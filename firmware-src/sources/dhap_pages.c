/*
 * dhap_pages.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <c_types.h>
#include <osapi.h>
#include <mem.h>
#include "dhap_pages.h"
#include "snprintf.h"
#include "dhsettings.h"
#include "rand.h"
#include "user_config.h"
#include "irom.h"

#define DHAP_PAGE_TITLE_META  "<title>DeviceHive ESP8266 Configuration</title><meta name='viewport' content='width=device-width, initial-scale=1.0'>"
#define DHAP_PAGE_MAX_SIZE 4096

RO_DATA char DHAP_PAGE_ERROR[] = "<html><head>"DHAP_PAGE_TITLE_META"</head><body><font color='red'>%s<br>Go <a href='javascript:history.back()'>back</a> to try again.</font></body></html>";
RO_DATA char DHAP_PAGE_OK[] =  "<html><head>"DHAP_PAGE_TITLE_META"</head><body><font color='green'>Configuration was saved. System will reboot shortly.</font></body></html>";
RO_DATA char DHAP_PAGE_FORM[] =  "<html>" \
							"<head>"DHAP_PAGE_TITLE_META\
							"<style type='text/css'>input{width:100%%;}input[type=submit]{width:30%%;}</style></head>"\
							"<body><form method='post'>"\
								"Wi-Fi SSID:<br>"\
								"<input type='text' name='ssid' value='%s'><br><br>"\
								"Wi-Fi Password(leave empty to keep current):<br>"\
								"<input type='password' name='pass'><br><br>"\
								"DeviceHive API Url (Example "DEFAULT_SERVER"):<br>"\
								"<input type='text' name='url' value='%s'><br><br>"\
								"DeviceId (allowed chars are A-Za-z0-9- ):<br>"\
								"<input type='text' name='id' value='%s'><br><br>"\
								"AccessKey (leave empty to keep current, allowed chars are A-Za-z0-9/+= ):<br>"\
								"<input type='password' name='key'><br><br>"\
								"<input type='submit' value='Apply'>"\
							"</form></body>"\
						"</html>";
LOCAL char *mPageBuffer = 0;

LOCAL char * ICACHE_FLASH_ATTR init() {
	if(mPageBuffer == 0)
		mPageBuffer = (char *)os_malloc(DHAP_PAGE_MAX_SIZE);
	return mPageBuffer;
}

const char * ICACHE_FLASH_ATTR dhap_pages_error(const char *error, unsigned int *len) {
	if(init() == 0)
		return 0;
	*len = snprintf(mPageBuffer, DHAP_PAGE_MAX_SIZE, DHAP_PAGE_ERROR, error);
	return mPageBuffer;
}

const char * ICACHE_FLASH_ATTR dhap_pages_ok(unsigned int *len) {
	*len = sizeof(DHAP_PAGE_OK) - 1;
	return DHAP_PAGE_OK;
}

LOCAL void ICACHE_FLASH_ATTR esc_cpystr(char *dst, const char *src) {
	const char esc_quote[] = "&#39;";
	while(*src) {
		if(*src == '\'')
			dst += snprintf(dst, sizeof(esc_quote), "%s", esc_quote);
		else
			*dst++ = *src;
		src++;
	}
	*dst = 0;
}

LOCAL unsigned int ICACHE_FLASH_ATTR esc_len(const char *str) {
	int res = 0;
	while(*str) {
		res++;
		if(*str == '\'')
			res += 4;
		str++;
	}
	return res;
}

const char * ICACHE_FLASH_ATTR dhap_pages_form(unsigned int *len) {
	if(init() == 0)
		return 0;
	const char *ssid = dhsettings_get_wifi_ssid();
	const char *server = dhsettings_get_devicehive_server();
	const char *deviceid = dhsettings_get_devicehive_deviceid();
	unsigned int esc_ssid_len = esc_len(ssid);
	unsigned int esc_server_len = esc_len(server);
	unsigned int esc_deviceid_len = esc_len(deviceid);
	if(deviceid[0] == 0)
		esc_deviceid_len = rand_generate_deviceid(0);
	char esc_ssid[esc_ssid_len + 1];
	char esc_server[esc_server_len + 1];
	char esc_deviceid[esc_deviceid_len + 1];
	esc_cpystr(esc_ssid, ssid);
	esc_cpystr(esc_server, server);
	if(deviceid[0])
		esc_cpystr(esc_deviceid, deviceid);
	else
		esc_deviceid_len = rand_generate_deviceid(esc_deviceid);
	*len = snprintf(mPageBuffer, DHAP_PAGE_MAX_SIZE, DHAP_PAGE_FORM, esc_ssid, esc_server, esc_deviceid);
	return mPageBuffer;
}
