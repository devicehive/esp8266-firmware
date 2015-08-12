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
#include "dhsettings.h"
#include "snprintf.h"
#include "crc32.h"
#include "dhdebug.h"

// using main and backup storage to keep old setting in case of power loss during writing
#define ESP_SETTINGS_MAIN_SEC   0x3C
#define ESP_SETTINGS_BACKUP_SEC (ESP_SETTINGS_MAIN_SEC + 1)

typedef struct {
	uint32_t crc;
	union {
		uint8_t storage[SPI_FLASH_SEC_SIZE - sizeof(uint32_t)]; // 4092+4 bytes from crc = 4K, make this structure like sector size
		struct {
			char ssid[DHSETTINGS_SSID_MAX_LENGTH];
			char password[DHSETTINGS_PASSWORD_MAX_LENGTH];
			char server[DHSETTINGS_SERVER_MAX_LENGTH];
			char deviceId[DHSETTINGS_DEVICEID_MAX_LENGTH];
			char deviceKey[DHSETTINGS_DEVICEKEY_MAX_LENGTH];
		} data;
	};
} DH_SETTINGS;

DH_SETTINGS mSettings = {0};

uint32_t ICACHE_FLASH_ATTR getStorageCrc() {
	return crc32(mSettings.storage, sizeof(mSettings.storage));
}

int ICACHE_FLASH_ATTR dhsettings_init() {
	SpiFlashOpResult res;
	res = spi_flash_read(ESP_SETTINGS_MAIN_SEC * SPI_FLASH_SEC_SIZE, (uint32 *)&mSettings, sizeof(mSettings));
	int read = 1;
	if (res != SPI_FLASH_RESULT_OK) {
		dhdebug("Could not read settings from main storage %d, recover from backup", res);
		read = 0;
	} else if(getStorageCrc() != mSettings.crc) {
		dhdebug("Main storage data corrupted or never saved, recover from backup");
		read = 0;
	}
	if(read == 0) {
		res = spi_flash_read(ESP_SETTINGS_BACKUP_SEC * SPI_FLASH_SEC_SIZE, (uint32 *)&mSettings, sizeof(mSettings));
		if (res != SPI_FLASH_RESULT_OK) {
			dhdebug("Could not read settings from backup storage %d", res);
		} else if(getStorageCrc() != mSettings.crc) {
			dhdebug("Backup storage data corrupted or never saved, using empty settings");
			os_memset(&mSettings, 0, sizeof(mSettings));
		} else {
			read = 1;
			dhdebug("Settings successfully loaded from backup storage");
		}
	} else {
		dhdebug("Settings successfully loaded from main storage");
	}
	return read;
}

int ICACHE_FLASH_ATTR dhsettings_commit() {
	int res = 1;
	mSettings.crc = getStorageCrc();
	if(spi_flash_erase_sector(ESP_SETTINGS_MAIN_SEC) == SPI_FLASH_RESULT_OK) {
		if(spi_flash_write(ESP_SETTINGS_MAIN_SEC* SPI_FLASH_SEC_SIZE, (uint32 *)&mSettings, sizeof(mSettings)) != SPI_FLASH_RESULT_OK) {
			dhdebug("Writing to main storage failed");
			res = 0;
		}
	} else {
		dhdebug("Erasing of main storage failed");
		res = 0;
	}
	if(spi_flash_erase_sector(ESP_SETTINGS_BACKUP_SEC) == SPI_FLASH_RESULT_OK) {
		if(spi_flash_write(ESP_SETTINGS_BACKUP_SEC * SPI_FLASH_SEC_SIZE, (uint32 *)&mSettings, sizeof(mSettings)) != SPI_FLASH_RESULT_OK) {
			dhdebug("Writing to backup storage failed");
			res = 0;
		}
	} else {
		dhdebug("Erasing of backup storage failed");
		res = 0;
	}
	if(res)
		dhdebug("Settings successfully wrote");
	return res;
}

int ICACHE_FLASH_ATTR dhsettings_clear() {
	int res = 1;
	if(spi_flash_erase_sector(ESP_SETTINGS_MAIN_SEC) != SPI_FLASH_RESULT_OK) {
		dhdebug("Erasing of main storage failed");
		res = 0;
	}
	if(spi_flash_erase_sector(ESP_SETTINGS_BACKUP_SEC) != SPI_FLASH_RESULT_OK) {
		dhdebug("Erasing of backup storage failed");
		res = 0;
	}
	if(res) {
		os_memset(&mSettings, 0, sizeof(mSettings));
		dhdebug("Settings successfully cleared");
	}
	return res;
}

const char * ICACHE_FLASH_ATTR dhsettings_get_wifi_ssid() {
	return mSettings.data.ssid;
}

const char * ICACHE_FLASH_ATTR dhsettings_get_wifi_password() {
	return mSettings.data.password;
}

const char * ICACHE_FLASH_ATTR dhsettings_get_devicehive_server() {
	return mSettings.data.server;
}

const char * ICACHE_FLASH_ATTR dhsettings_get_devicehive_deviceid() {
	return mSettings.data.deviceId;
}

const char * ICACHE_FLASH_ATTR dhsettings_get_devicehive_devicekey() {
	return mSettings.data.deviceKey;
}

void ICACHE_FLASH_ATTR set_arg(char *arg, size_t argSize, const char *value) {
	const int len = snprintf(arg, argSize, "%s", value) + 1;
	if(len < argSize)
		os_memset(&arg[len], 0, argSize - len);
}

void ICACHE_FLASH_ATTR dhsettings_set_wifi_ssid(const char *ssid) {
	set_arg(mSettings.data.ssid, sizeof(mSettings.data.ssid), ssid);
}

void ICACHE_FLASH_ATTR dhsettings_set_wifi_password(const char *pass) {
	set_arg(mSettings.data.password, sizeof(mSettings.data.password), pass);
}

void ICACHE_FLASH_ATTR dhsettings_set_devicehive_server(const char *server) {
	set_arg(mSettings.data.server, sizeof(mSettings.data.server), server);
}

void ICACHE_FLASH_ATTR dhsettings_set_devicehive_deviceid(const char *id) {
	set_arg(mSettings.data.deviceId, sizeof(mSettings.data.deviceId), id);
}

void ICACHE_FLASH_ATTR dhsettings_set_devicehive_devicekey(const char *key) {
	set_arg(mSettings.data.deviceKey, sizeof(mSettings.data.deviceKey), key);
}
