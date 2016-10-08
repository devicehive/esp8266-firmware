/*
 * rest.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <c_types.h>
#include <osapi.h>
#include "rest.h"
#include "dhrequest.h"
//#include "dhcommands.h"

static const char desription[] = "<html><body>This is firmware RESTfull API endpoint. "\
	"Please follow the firmware manual to use it.<br><a href='http://devicehive.com/' "\
	"target='_blank'>DeviceHive</a> Firmware v"FIRMWARE_VERSION"<br>"\
	"<a href='https://github.com/devicehive/esp8266-firmware' target='_blank'>"\
	"https://github.com/devicehive/esp8266-firmware</a></body></html>";

HTTP_RESPONSE_STATUS ICACHE_FLASH_ATTR rest_handle(const char *path, const char *key,
		HTTP_CONTENT *content_in, HTTP_CONTENT *content_out) {
	static const char cint[] = "/int";
	if(path[0] == 0) {
		content_out->data = desription;
		content_out->len = sizeof(desription) - 1;
		return HRCS_ANSWERED;
	}
	if(dhrequest_current_accesskey()[0]) {
		if(key == 0) {
			return HRCS_UNAUTHORIZED;
		}
		if(os_strcmp(key, dhrequest_current_accesskey())) {
			return HRCS_UNAUTHORIZED;
		}
	}

	// prevent all commands with interruption
	int pathlen = os_strlen(path);
	if(pathlen >= sizeof(cint) - 1) {
		if(os_strncmp(&path[pathlen - sizeof(cint) + 1], cint, sizeof(cint) - 1) == 0) {
			return HRCS_NOT_FOUND;
		}
	}

	// TODO
	//dhcommands_do(0, path, content_in->data, content_in->len);
	return HRCS_NOT_FOUND;
}
