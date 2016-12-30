/*
 * dhsettings.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for storing settings in flash
 *
 */

#include <c_types.h>
#include <osapi.h>
#include <spi_flash.h>
#include <mem.h>
#include "dhsettings.h"
#include "snprintf.h"
#include "crc32.h"
#include "dhdebug.h"

// using main and backup storage to keep old setting in case of power loss during writing
#define ESP_SETTINGS_MAIN_SEC   0x7A
#define ESP_SETTINGS_BACKUP_SEC (ESP_SETTINGS_MAIN_SEC + 1)

typedef struct {
	char ssid[DHSETTINGS_SSID_MAX_LENGTH];
	char password[DHSETTINGS_PASSWORD_MAX_LENGTH];
	char server[DHSETTINGS_SERVER_MAX_LENGTH];
	char deviceId[DHSETTINGS_DEVICEID_MAX_LENGTH];
	char accessKey[DHSETTINGS_ACCESSKEY_MAX_LENGTH];
	WIFI_MODE mode;
} DH_SETTINGS_DATA;

typedef struct {
	uint32_t crc;
	union {
		uint8_t storage[SPI_FLASH_SEC_SIZE - sizeof(uint32_t)]; // 4092+4 bytes from crc = 4K, make this structure like sector size
		DH_SETTINGS_DATA data;
	};
} DH_SETTINGS;

static DH_SETTINGS_DATA mSettingsData = {0};

LOCAL uint32_t ICACHE_FLASH_ATTR getStorageCrc(DH_SETTINGS *storage) {
	return crc32(storage->storage, sizeof(storage->storage));
}

int ICACHE_FLASH_ATTR dhsettings_init(int *exist) {
	*exist = 1;
	DH_SETTINGS *settings = (DH_SETTINGS *)os_malloc(sizeof(DH_SETTINGS));
	if(settings == NULL) {
		dhdebug("Failed to read settings, no RAM.");
		return 0;
	}
	SpiFlashOpResult res;
	res = spi_flash_read(ESP_SETTINGS_MAIN_SEC * SPI_FLASH_SEC_SIZE, (uint32 *)settings, sizeof(DH_SETTINGS));
	int read = 1;
	if (res != SPI_FLASH_RESULT_OK) {
		dhdebug("Could not read settings from main storage %d, recover from backup", res);
		read = 0;
	} else if(getStorageCrc(settings) != settings->crc) {
		dhdebug("Main storage data corrupted or never saved, recover from backup");
		read = 0;
	}
	if(read == 0) {
		res = spi_flash_read(ESP_SETTINGS_BACKUP_SEC * SPI_FLASH_SEC_SIZE, (uint32 *)settings, sizeof(DH_SETTINGS));
		if (res != SPI_FLASH_RESULT_OK) {
			dhdebug("Could not read settings from backup storage %d", res);
		} else if(getStorageCrc(settings) != settings->crc) {
			dhdebug("Backup storage data corrupted or never saved, using empty settings");
			os_memset(settings, 0, sizeof(DH_SETTINGS));
			*exist = 0;
		} else {
			read = 1;
			dhdebug("Settings successfully loaded from backup storage");
		}
	} else {
		dhdebug("Settings successfully loaded from main storage");
	}
	os_memcpy(&mSettingsData, &settings->data, sizeof(DH_SETTINGS_DATA));
	os_free(settings);
	return read;
}

LOCAL int ICACHE_FLASH_ATTR dhsettings_write(const DH_SETTINGS_DATA * data) {
	int res = 1;
	DH_SETTINGS *settings = (DH_SETTINGS *)os_malloc(sizeof(DH_SETTINGS));
	if(settings == NULL) {
		dhdebug("Failed to write settings, no RAM.");
		return 0;
	}
	os_memset(settings, 0x0, sizeof(DH_SETTINGS));
	if(data)
		os_memcpy(&settings->data, data, sizeof(DH_SETTINGS_DATA));
	settings->crc = getStorageCrc(settings);
	if(spi_flash_erase_sector(ESP_SETTINGS_MAIN_SEC) == SPI_FLASH_RESULT_OK) {
		if(spi_flash_write(ESP_SETTINGS_MAIN_SEC* SPI_FLASH_SEC_SIZE, (uint32 *)settings, sizeof(DH_SETTINGS)) != SPI_FLASH_RESULT_OK) {
			dhdebug("Writing to main storage failed");
			res = 0;
		}
	} else {
		dhdebug("Erasing of main storage failed");
		res = 0;
	}
	if(spi_flash_erase_sector(ESP_SETTINGS_BACKUP_SEC) == SPI_FLASH_RESULT_OK) {
		if(spi_flash_write(ESP_SETTINGS_BACKUP_SEC * SPI_FLASH_SEC_SIZE, (uint32 *)settings, sizeof(DH_SETTINGS)) != SPI_FLASH_RESULT_OK) {
			dhdebug("Writing to backup storage failed");
			res = 0;
		}
	} else {
		dhdebug("Erasing of backup storage failed");
		res = 0;
	}
	if(res)
		dhdebug("Settings successfully wrote");
	os_free(settings);
	return res;
}


int ICACHE_FLASH_ATTR dhsettings_commit() {
	return dhsettings_write(&mSettingsData);
}

int ICACHE_FLASH_ATTR dhsettings_clear(int force) {
	os_memset(mSettingsData, 0, sizeof(mSettingsData));
	if(force) {
		if(spi_flash_erase_sector(ESP_SETTINGS_MAIN_SEC) == SPI_FLASH_RESULT_OK &&
				spi_flash_erase_sector(ESP_SETTINGS_BACKUP_SEC) == SPI_FLASH_RESULT_OK) {
			return 1;
		}
		return 0;
	}
	return dhsettings_write(NULL);
}

WIFI_MODE ICACHE_FLASH_ATTR dhsettings_get_wifi_mode() {
	return mSettingsData.mode;
}

const char * ICACHE_FLASH_ATTR dhsettings_get_wifi_ssid() {
	return mSettingsData.ssid;
}

const char * ICACHE_FLASH_ATTR dhsettings_get_wifi_password() {
	return mSettingsData.password;
}

const char * ICACHE_FLASH_ATTR dhsettings_get_devicehive_server() {
	return mSettingsData.server;
}

const char * ICACHE_FLASH_ATTR dhsettings_get_devicehive_deviceid() {
	return mSettingsData.deviceId;
}

const char * ICACHE_FLASH_ATTR dhsettings_get_devicehive_accesskey() {
	return mSettingsData.accessKey;
}

LOCAL void ICACHE_FLASH_ATTR set_arg(char *arg, size_t argSize, const char *value) {
	const int len = snprintf(arg, argSize, "%s", value) + 1;
	if(len < argSize)
		os_memset(&arg[len], 0, argSize - len);
}

void ICACHE_FLASH_ATTR dhsettings_set_wifi_mode(WIFI_MODE mode) {
	mSettingsData.mode = mode;
}

void ICACHE_FLASH_ATTR dhsettings_set_wifi_ssid(const char *ssid) {
	set_arg(mSettingsData.ssid, sizeof(mSettingsData.ssid), ssid);
}

void ICACHE_FLASH_ATTR dhsettings_set_wifi_password(const char *pass) {
	set_arg(mSettingsData.password, sizeof(mSettingsData.password), pass);
}

void ICACHE_FLASH_ATTR dhsettings_set_devicehive_server(const char *server) {
	set_arg(mSettingsData.server, sizeof(mSettingsData.server), server);
}

void ICACHE_FLASH_ATTR dhsettings_set_devicehive_deviceid(const char *id) {
	set_arg(mSettingsData.deviceId, sizeof(mSettingsData.deviceId), id);
}

void ICACHE_FLASH_ATTR dhsettings_set_devicehive_accesskey(const char *key) {
	set_arg(mSettingsData.accessKey, sizeof(mSettingsData.accessKey), key);
}

int ICACHE_FLASH_ATTR dhsettings_deviceid_filter(char c) {
	if(c == '-' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'))
		return 1;
	return 0;
}

int ICACHE_FLASH_ATTR dhsettings_accesskey_filter(char c) {
	if(c == '=' || c == '+' || c == '/' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'))
			return 1;
	return 0;
}

int ICACHE_FLASH_ATTR dhsettings_server_filter(char c) {
	if(c <= 0x20 || c >= 0x7F)
		return 0;
	return 1;
}

