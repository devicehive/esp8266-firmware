/*
 * rest.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <stdarg.h>
#include <c_types.h>
#include <osapi.h>
#include <mem.h>
#include "rest.h"
#include "dhrequest.h"
#include "dhcommands.h"
#include "dhsender_data.h"
#include "user_config.h"

static const char desription[] = "<html><body>This is firmware RESTfull API endpoint. "\
	"Please follow the firmware manual to use it.<br><a href='http://devicehive.com/' "\
	"target='_blank'>DeviceHive</a> Firmware v"FIRMWARE_VERSION"<br>"\
	"<a href='https://github.com/devicehive/esp8266-firmware' target='_blank'>"\
	"https://github.com/devicehive/esp8266-firmware</a></body></html>";

LOCAL void ICACHE_FLASH_ATTR rest_command_callback(CommandResultArgument cid,
		RESPONCE_STATUS status, REQUEST_DATA_TYPE data_type, ...) {
	va_list ap;
	va_start(ap, data_type);
	HTTP_ANSWER *answer = cid.arg;
	answer->ok = status == DHSTATUS_OK ? 1 : 0;
	if(data_type == RDT_CONST_STRING) { // optimization
		answer->content.data = va_arg(ap, const char *);;
		answer->content.len = os_strlen(answer->content.data);
	} else {
		SENDERDATA data;
		unsigned int data_len;
		unsigned int pin;
		dhsender_data_parse_va(ap, &data_type, &data, &data_len, &pin);
		char *buf = (char *)os_malloc(2*INTERFACES_BUF_SIZE); // do we really can have more?
		int res = dhsender_data_to_json(buf, 2*INTERFACES_BUF_SIZE, 0, data_type, &data,
				data_len, pin);
		if(res < 0) {
			os_free(buf);
			static char error[] = "Failed to build json";
			answer->ok = 0;
			answer->content.data = error;
			answer->content.len = sizeof(error) - 1;
			return;
		}
		answer->content.data = buf;
		answer->content.len = res;
		answer->free_content = 1;
	}
	va_end(ap);
}

HTTP_RESPONSE_STATUS ICACHE_FLASH_ATTR rest_handle(const char *path, const char *key,
		HTTP_CONTENT *content_in, HTTP_ANSWER *answer) {
	static const char cint[] = "/int";
	if(path[0] == 0) {
		answer->content.data = desription;
		answer->content.len = sizeof(desription) - 1;
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
			CommandResultArgument cid;
			cid.arg = answer;
			rest_command_callback(cid, DHSTATUS_ERROR, RDT_CONST_STRING, "Unknown command");
			return HRCS_ANSWERED;
		}
	}

	COMMAND_RESULT cb;
	cb.callback = rest_command_callback;
	cb.data.arg = (void*)answer;
	dhcommands_do(&cb, path, content_in->data, content_in->len);
	return HRCS_ANSWERED;
}
