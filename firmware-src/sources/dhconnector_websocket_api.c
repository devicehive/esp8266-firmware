/*
 * dhconnector_websocket_api.cpp
 *
 * Copyright 2017 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: DeviceHive WebSocket protocol implementation
 *
 */
#include "dhconnector_websocket_api.h"
#include "dhsettings.h"
#include "irom.h"
#include "snprintf.h"
#include "dhdebug.h"
#include "rand.h"
#include "user_config.h"
#include "dhcommands.h"
#include "dhsender.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include <json/jsonparse.h>
#include <ets_forward.h>


int ICACHE_FLASH_ATTR dhconnector_websocket_api_start(char *buf, unsigned int maxlen) {
	RO_DATA char template[] =
			"{"
				"\"action\":\"authenticate\","
				"\"accessKey\":\"%s\""
			"}";
	return snprintf(buf, maxlen, template, dhsettings_get_devicehive_accesskey());
}

int ICACHE_FLASH_ATTR dhconnector_websocket_api_communicate(const char *in, unsigned int inlen, char *out, unsigned int outmaxlen) {
	int type;
	int status_not_success = 1;
	char action[32];
	char command[128];
	const char *params = 0;
	unsigned int paramslen = 0;
	unsigned int id = 0;
	action[0] = 0;
	command[0] = 0;
	struct jsonparse_state jparser;
	jsonparse_setup(&jparser, in, inlen);
	while (jparser.pos < jparser.len) {
		type = jsonparse_next(&jparser);
		if(type == JSON_TYPE_PAIR_NAME) {
			if(jsonparse_strcmp_value(&jparser, "status") == 0) {
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR) {
					status_not_success = jsonparse_strcmp_value(&jparser, "success");
				}
			} else if(jsonparse_strcmp_value(&jparser, "action") == 0) {
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR)
					jsonparse_copy_value(&jparser, action, sizeof(action));
			} else if(jsonparse_strcmp_value(&jparser, "command") == 0) {
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR)
					jsonparse_copy_value(&jparser, command, sizeof(command));
			} else if(jsonparse_strcmp_value(&jparser, "id") == 0) {
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR)
					id = jsonparse_get_value_as_ulong(&jparser);
			} else if(jsonparse_strcmp_value(&jparser, "parameters") == 0) {
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR) {
					// there is an issue with extracting subjson with jparser->vstart or jparser_copy_value
					params = &jparser.json[jparser.pos - 1];
					if(*params == '{') {
						int end = jparser.pos;
						while(end < jparser.len && jparser.json[end] != '}') {
							end++;
						}
						paramslen = end - jparser.pos + 2;
						jparser.pos += paramslen;
					}
				}
			}
		} else if(type == JSON_TYPE_ERROR) {
			if(jparser.pos > 0 && jparser.len - jparser.pos >= 3) { // fix issue with parsing null value
				if(os_strncmp(&jparser.json[jparser.pos - 1], "null", 4) == 0) {
					jparser.pos += 3;
					jparser.vtype = JSON_TYPE_NULL;
					continue;
				}
			}
			break;
		}
	}

	if(os_strcmp(action, "command/insert") == 0) {
		COMMAND_RESULT cb;
		cb.callback = dhsender_response;
		cb.data.id = id;
		dhcommands_do(&cb, command, params, paramslen);
		return 0;
	} else if(os_strcmp(action, "authenticate") == 0) {
		RO_DATA char template[] =
				"{"
					"\"action\":\"device/save\","
					"\"deviceId\":\"%s\","
					"\"deviceKey\":\"%s\","
					"\"device\":{"
						"\"name\":\"%s\","
						"\"key\":\"%s\","
						"\"status\":\"Online\","
						"\"deviceClass\":{"
							"\"name\":\"ESP Class\","
							"\"version\":\""FIRMWARE_VERSION"\","
							"\"offlineTimeout\":\"900\""
							"}"
					"}"
				"}";
		char dk[9];
		if(status_not_success) {
			dhdebug("Failed to authenticate");
			return DHCONNECT_WEBSOCKET_API_ERROR;
		}
		snprintf(dk, sizeof(dk), "%s", dhsettings_get_devicehive_accesskey());
		return snprintf(out, outmaxlen, template,
				dhsettings_get_devicehive_deviceid(), dk,
				dhsettings_get_devicehive_deviceid(), dk);
	} else if(os_strcmp(action, "device/save") == 0) {
		RO_DATA char template[] =
				"{"
					"\"action\":\"command/subscribe\","
					"\"deviceGuids\":[\"%s\"]"
				"}";
		if(status_not_success) {
			dhdebug("Failed to save device");
			return DHCONNECT_WEBSOCKET_API_ERROR;
		}
		return snprintf(out, outmaxlen, template,
				dhsettings_get_devicehive_deviceid());
	}
	return 0;
}
