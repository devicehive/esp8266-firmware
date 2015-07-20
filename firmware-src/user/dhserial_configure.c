/*
 * dhserial_configure.c
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
#include "dhserial_configure.h"
#include "drivers/uart.h"
#include "dhserial.h"
#include "dhsettings.h"
#include "rand.h"
#include "snprintf.h"

char mComleaterBuff[48];

void ICACHE_FLASH_ATTR rand_password(char *buff, int minlen, int maxlen) {
	int num = minlen + rand() % (maxlen - minlen + 1);
	while(num--) {
		*buff++ =  0x21 + rand() % 0x5E;
	}
	*buff = 0;
}

void ICACHE_FLASH_ATTR rand_uuid(char *buff) {
	int i=0;
	for(i = 0 ; i<16; i++) {
		int d = rand() % 0x100;
		buff += snprintf(buff, 3, (d < 0x10) ? "0%X" : "%X", d);
		if(i == 3 || i == 5 || i == 7 || i == 9)
			*buff++ = '-';
	}
}

void ICACHE_FLASH_ATTR get_devicekey_cb(const char *key) {
	if(*key)
		dhsettings_set_devicehive_devicekey(key);
	uart_send_str("Configuring complete, store settings...");
	if(dhsettings_commit()) {
		uart_send_line("OK");
		uart_send_line("Rebooting...");
		system_restart();
	} else {
		uart_send_line("ERROR. Check debug output");
	}
	dhserial_set_mode(SM_NORMAL_MODE, 0, 0);
}

char * ICACHE_FLASH_ATTR generate_key(const char *pattern) {
	rand_password(mComleaterBuff, 8, 16);
	return mComleaterBuff;
}

void ICACHE_FLASH_ATTR get_deviceid_cb(const char *id) {
	dhsettings_set_devicehive_deviceid(id);
	uart_send_line("Enter DeviceHive DeviceKey. Press Tab button to generate random key (will be");
	uart_send_line("shown on screen). Leave empty to keep current");
	if(dhsettings_get_devicehive_devicekey()[0] == 0) {
		uart_send_line("Key wasn't stored before. Generating random, change it if you want");
	}
	dhserial_set_mode(SM_HIDDEN_INPUT_MODE, get_devicekey_cb, generate_key);
	if(dhsettings_get_devicehive_devicekey()[0] == 0) {
		dhserial_set_input(generate_key(""));
	}
}

char * ICACHE_FLASH_ATTR generate_uuid(const char *pattern) {
	rand_uuid(mComleaterBuff);
	return mComleaterBuff;
}

void ICACHE_FLASH_ATTR get_server_cb(const char *server) {
	char buf[os_strlen(server)];
	int pos = os_sprintf(buf, "%s", server);
	pos--;
	while(pos >= 0 && (buf[pos] == ' ' || buf[pos] == '/'))
		pos--;
	buf[pos + 1] = 0;

	dhsettings_set_devicehive_server(buf);
	uart_send_line("Enter DeviceHive DeviceId. Press Tab button to generate random uuid");
	dhserial_set_mode(SM_INPUT_MODE, get_deviceid_cb, generate_uuid);
	if(dhsettings_get_devicehive_deviceid()[0] == 0) {
		dhserial_set_input(generate_uuid(""));
	} else {
		dhserial_set_input(dhsettings_get_devicehive_deviceid());
	}
}

void ICACHE_FLASH_ATTR get_password_cb(const char *password) {
	if(*password)
		dhsettings_set_wifi_password(password);
	uart_send_line("Enter DeviceHive server URL");
	dhserial_set_mode(SM_INPUT_MODE, get_server_cb, 0);
	dhserial_set_input(dhsettings_get_devicehive_server());
}

void ICACHE_FLASH_ATTR get_ssid_cb(const char *ssid) {
	dhsettings_set_wifi_ssid(ssid);
	uart_send_line("Enter Wi-Fi network password. Leave empty to keep current");
	dhserial_set_mode(SM_HIDDEN_INPUT_MODE, get_password_cb, 0);
}

void ICACHE_FLASH_ATTR dhserial_configure_start() {
	uart_send_line("Welcome to the DeviceHive setup utility. Use Ctrl+C to interrupt.");
	uart_send_line("Enter Wi-Fi network SSID");
	dhserial_set_mode(SM_INPUT_MODE, get_ssid_cb, 0);
	dhserial_set_input(dhsettings_get_wifi_ssid());
}
