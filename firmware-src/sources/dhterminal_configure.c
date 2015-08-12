/*
 * dhterminal_configure.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: 'configure' command implementation
 *
 */

#include <osapi.h>
#include <user_interface.h>
#include "dhterminal_configure.h"
#include "dhuart.h"
#include "dhterminal.h"
#include "dhsettings.h"
#include "rand.h"
#include "snprintf.h"
#include "dhutils.h"

char mComleaterBuff[48];

LOCAL void ICACHE_FLASH_ATTR get_devicekey_cb(const char *key) {
	if(*key)
		dhsettings_set_devicehive_devicekey(key);
	dhuart_send_str("Configuring complete, store settings...");
	if(dhsettings_commit()) {
		dhuart_send_line("OK");
		dhuart_send_line("Rebooting...");
		system_restart();
	} else {
		dhuart_send_line("ERROR. Not saved. Check debug output.");
	}
	dhterminal_set_mode(SM_NORMAL_MODE, 0, 0, 0, 0);
}

LOCAL char * ICACHE_FLASH_ATTR generate_key(const char *pattern) {
	const int minlen = 8;
	const int maxlen = 16;
	char *buf = mComleaterBuff;
	int num = minlen + rand() % (maxlen - minlen + 1);
	while(num--) {
		char c = 0x21 + rand() % 0x5C; // 0x21 - 0x7C
		// removing unsuitable chars
		if(c == '"')
			c = '}'; // 0x7D
		else if (c == '\\')
			c = '~'; // 0x7E
		*buf++ =  c;
	}
	*buf = 0;
	return mComleaterBuff;
}

LOCAL int ICACHE_FLASH_ATTR deviceid_filter(char c) {
	if(c == '-' || c == '_' || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'))
		return 1;
	return 0;
}

LOCAL int ICACHE_FLASH_ATTR devicekey_filter(char c) {
	if(c <= 0x20 || c >= 0x7F || c == '"' || c == '\\')
			return 0;
	return 1;
}

LOCAL int ICACHE_FLASH_ATTR url_filter(char c) {
	if(c <= 0x20 || c >= 0x7F)
		return 0;
	return 1;
}

LOCAL void ICACHE_FLASH_ATTR get_deviceid_cb(const char *id) {
	dhsettings_set_devicehive_deviceid(id);
	dhuart_send_line("Enter DeviceHive DeviceKey. Press Tab button to generate random key (will be");
	dhuart_send_line("shown on screen). Leave empty to keep current. Do not use \\ and \" chars.");
	if(dhsettings_get_devicehive_devicekey()[0] == 0) {
		dhuart_send_line("Key wasn't stored before. Generating random, change it if you want.");
	}
	dhterminal_set_mode(SM_HIDDEN_INPUT_MODE, get_devicekey_cb, generate_key, devicekey_filter, DHSETTINGS_DEVICEKEY_MAX_LENGTH);
	if(dhsettings_get_devicehive_devicekey()[0] == 0) {
		dhterminal_set_input(generate_key(""));
	}
}

LOCAL char * ICACHE_FLASH_ATTR generate_deviceid(const char *pattern) {
	int mComleaterBuffPos = snprintf(mComleaterBuff, sizeof(mComleaterBuff), "esp-device-");
	int i;
	for(i = 0; i < 8 && mComleaterBuffPos < sizeof(mComleaterBuff) - 1; i++) {
		mComleaterBuffPos += snprintf(&mComleaterBuff[mComleaterBuffPos], sizeof(mComleaterBuff) - mComleaterBuffPos, "%x", rand() % 16);
	}
	return mComleaterBuff;
}

LOCAL void ICACHE_FLASH_ATTR get_server_cb(const char *server) {
	const unsigned int slen = os_strlen(server) + 1;
	char buf[slen];
	int pos = snprintf(buf, slen, "%s", server);
	pos--;
	while(pos >= 0 && (buf[pos] == ' ' || buf[pos] == '/'))
		pos--;
	buf[pos + 1] = 0;

	dhsettings_set_devicehive_server(buf);
	dhuart_send_line("Enter DeviceHive DeviceId. Press Tab button to generate random.");
	dhuart_send_line("Allowed chars are A-Za-z0-9_-");
	dhterminal_set_mode(SM_INPUT_MODE, get_deviceid_cb, generate_deviceid, deviceid_filter, DHSETTINGS_DEVICEID_MAX_LENGTH);
	if(dhsettings_get_devicehive_deviceid()[0] == 0) {
		dhterminal_set_input(generate_deviceid(""));
	} else {
		dhterminal_set_input(dhsettings_get_devicehive_deviceid());
	}
}

LOCAL void ICACHE_FLASH_ATTR get_password_cb(const char *password) {
	if(*password)
		dhsettings_set_wifi_password(password);
	dhuart_send_line("Enter DeviceHive API URL.");
	dhterminal_set_mode(SM_INPUT_MODE, get_server_cb, 0, url_filter, DHSETTINGS_SERVER_MAX_LENGTH);
	dhterminal_set_input(dhsettings_get_devicehive_server());
}

LOCAL void ICACHE_FLASH_ATTR get_ssid_cb(const char *ssid) {
	dhsettings_set_wifi_ssid(ssid);
	dhuart_send_line("Enter Wi-Fi network password. Leave empty to keep current.");
	dhterminal_set_mode(SM_HIDDEN_INPUT_MODE, get_password_cb, 0, 0, DHSETTINGS_PASSWORD_MAX_LENGTH);
}

void ICACHE_FLASH_ATTR dhterminal_configure_start() {
	dhuart_send_line("Welcome to the DeviceHive setup utility. Use Ctrl+C to interrupt.");
	dhuart_send_line("Enter Wi-Fi network SSID.");
	dhterminal_set_mode(SM_INPUT_MODE, get_ssid_cb, 0, 0, DHSETTINGS_SSID_MAX_LENGTH);
	dhterminal_set_input(dhsettings_get_wifi_ssid());
}
