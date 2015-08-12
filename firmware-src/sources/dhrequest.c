/*
 * dhrequest.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for creating HTTP requests for DeviceHive server
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include "dhrequest.h"
#include "user_config.h"
#include "dhdebug.h"
#include "dhsettings.h"
#include "dhmem.h"

static const char REGISTER_JSON[] =
		"{\"name\":\"%s\",\"key\":\"%s\",\"status\":\"Online\",\"deviceClass\":{\"name\":\"ESP Class\",\"version\":\"1.0\",\"offlineTimeout\":\"120\"}}";
static const char UPDATE_JSON[] =
		"{\"status\":\"%s\",\"result\":%s%s%s}";
static const char NOTIFICATION_JSON[] =
		"{\"notification\":\"%s\",\"parameters\":%s%s%s}";

static const char HTTP_REQUEST_PATTERN[] =
		"%s %s%s%s%s%s HTTP/1.0\r\nAuth-DeviceID: %s\r\nAuth-DeviceKey: %s\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %s\r\n\r\n%s";

char mServer[DHSETTINGS_SERVER_MAX_LENGTH];
char mDeviceId[DHSETTINGS_DEVICEID_MAX_LENGTH];
char mDeviceKey[DHSETTINGS_DEVICEKEY_MAX_LENGTH];

unsigned short mServerLen;
unsigned short mDeviceIdLen;
unsigned short mDeviceKeyLen;

void ICACHE_FLASH_ATTR dhrequest_load_settings() {
	mServerLen = os_sprintf(mServer, "%s", dhsettings_get_devicehive_server());
	mDeviceIdLen = os_sprintf(mDeviceId, "%s", dhsettings_get_devicehive_deviceid());
	mDeviceKeyLen = os_sprintf(mDeviceKey, "%s", dhsettings_get_devicehive_devicekey());
}

const char *dhrequest_current_server() {
	return mServer;
}

HTTP_REQUEST * ICACHE_FLASH_ATTR dhrequest_create_register() {
	char lenbuf[16];
	char jsonbuf[sizeof(REGISTER_JSON) - 4 + mDeviceKeyLen + mDeviceIdLen];
	const unsigned int jsonbuflen = os_sprintf(jsonbuf, REGISTER_JSON, mDeviceId, mDeviceKey);
	const unsigned int lenbuflen = os_sprintf(lenbuf, "%d", jsonbuflen);
	const unsigned int len = sizeof(HTTP_REQUEST_PATTERN) - 20 + 3 + mServerLen + 8 + mDeviceIdLen + mDeviceIdLen + mDeviceKeyLen + lenbuflen + jsonbuflen;
	HTTP_REQUEST *res = dhmem_malloc_request(len + sizeof(HTTP_REQUEST));
	if(res == NULL) {
		dhdebug("Error: no memory for register request");
		return NULL;
	}
	res->len = os_sprintf(res->body, HTTP_REQUEST_PATTERN, "PUT", mServer, "/device/", mDeviceId, "", "", mDeviceId, mDeviceKey, lenbuf, jsonbuf);
	dhdebug("Register request created, %d/%d", len, res->len+1);
	res->next = NULL;
	return res;
}

HTTP_REQUEST * ICACHE_FLASH_ATTR dhrequest_create_poll(const char *timestamp) {
	return dhrequest_update_poll(NULL, timestamp);
}

HTTP_REQUEST *dhrequest_update_poll(HTTP_REQUEST *old, const char *timestamp) {
	const unsigned int timestamplength = os_strlen(timestamp);
	const unsigned int len = sizeof(HTTP_REQUEST_PATTERN) - 20 + 3 + mServerLen + 8 + mDeviceIdLen + 24 + timestamplength + mDeviceIdLen + mDeviceKeyLen + 1;
	HTTP_REQUEST *next;
	if(old) {
		if(old->len + 1 == len) {
			os_memcpy(&old->body[4 + mServerLen + 8 + mDeviceIdLen + 24], timestamp, timestamplength);
			dhdebug("Poll request updated");
			return old;
		}
		next = old->next;
		dhrequest_free(old);
	} else {
		next = NULL;
	}
	HTTP_REQUEST *res = dhmem_malloc_request(len + sizeof(HTTP_REQUEST));
	if(res == NULL) {
		dhdebug("Error: no memory for poll request");
		return NULL;
	}
	res->len = os_sprintf(res->body, HTTP_REQUEST_PATTERN, "GET", mServer, "/device/", mDeviceId, "/command/poll?timestamp=", timestamp, mDeviceId, mDeviceKey, "0", "");
	dhdebug("Poll request created, %d/%d", len, res->len+1);
	res->next = next;
	return res;
}

HTTP_REQUEST * ICACHE_FLASH_ATTR dhrequest_create_update(int commandId, const char *status, const char *result) {
	char idbuf[16];
	const unsigned int idbuflen = os_sprintf(idbuf, "%d", commandId);
	char jsonbuf[sizeof(UPDATE_JSON) - 6 + os_strlen(status) + os_strlen(result)];
	char extra[] = "\"";
	if(result[0] == '{')
		extra[0] = 0;
	const unsigned int jsonbuflen = os_sprintf(jsonbuf, UPDATE_JSON, status, extra, result, extra);
	char lenbuf[16];
	const unsigned int lenbuflen = os_sprintf(lenbuf, "%d", jsonbuflen);
	const unsigned int len = sizeof(HTTP_REQUEST_PATTERN) - 20 + 3 + mServerLen + 8 + mDeviceIdLen + 9 + idbuflen + mDeviceIdLen + mDeviceKeyLen + lenbuflen + jsonbuflen;
	HTTP_REQUEST *res = dhmem_malloc_request(len + sizeof(HTTP_REQUEST));
	if(res == NULL) {
		dhdebug("Error: no memory for update request");
		return NULL;
	}
	res->len = os_sprintf(res->body, HTTP_REQUEST_PATTERN, "PUT", mServer, "/device/", mDeviceId, "/command/", idbuf, mDeviceId, mDeviceKey, lenbuf, jsonbuf);
	dhdebug("Update request created, %d/%d", len, res->len+1);
	res->next = NULL;
	return res;
}

HTTP_REQUEST * ICACHE_FLASH_ATTR dhrequest_create_info() {
	const int len = sizeof(HTTP_REQUEST_PATTERN) - 20 + 3 + mServerLen + 5 + mDeviceIdLen + mDeviceKeyLen +1;
	HTTP_REQUEST *res = dhmem_malloc_request(len + sizeof(HTTP_REQUEST));
	if(res == NULL) {
		dhdebug("Error: no memory for info request");
		return NULL;
	}
	res->len = os_sprintf(res->body, HTTP_REQUEST_PATTERN, "GET", mServer, "/info", "", "", "", mDeviceId, mDeviceKey, "0", "");
	dhdebug("Info request created, %d/%d", len, res->len+1);
	res->next = NULL;
	return res;
}

HTTP_REQUEST * ICACHE_FLASH_ATTR dhrequest_create_notification(const char *name, const char *parameters) {
	char jsonbuf[sizeof(NOTIFICATION_JSON) - 6 + os_strlen(name) + os_strlen(parameters)];
	char extra[] = "\"";
	if(parameters[0] == '{')
		extra[0] = 0;
	const unsigned int jsonbuflen = os_sprintf(jsonbuf, NOTIFICATION_JSON, name, extra, parameters, extra);
	char lenbuf[16];
	const unsigned int lenbuflen = os_sprintf(lenbuf, "%d", jsonbuflen);
	const unsigned int len = sizeof(HTTP_REQUEST_PATTERN) - 20 + 4 + mServerLen + 8 + mDeviceIdLen + 13 + mDeviceIdLen + mDeviceKeyLen + lenbuflen + jsonbuflen;
	HTTP_REQUEST *res = dhmem_malloc_request(len + sizeof(HTTP_REQUEST));
	if(res == NULL) {
		dhdebug("Error: no memory for notification");
		return NULL;
	}
	res->len = os_sprintf(res->body, HTTP_REQUEST_PATTERN, "POST", mServer, "/device/", mDeviceId, "/notification", "", mDeviceId, mDeviceKey, lenbuf, jsonbuf);
	dhdebug("Notification created, %d/%d", len, res->len+1);
	res->next = NULL;
	return res;
}

void ICACHE_FLASH_ATTR dhrequest_free(HTTP_REQUEST *request) {
	dhmem_free_request(request);
}

const char *ICACHE_FLASH_ATTR dhrequest_find_http_responce_code(const char *data, unsigned short len) {
	unsigned short pos = sizeof(uint32);
	if (len > sizeof(uint32) && *(uint32 *) data == 0x50545448) { // HTTP
		while (pos < len)
			if (data[pos++] == ' ')
				break;
		return &data[pos];
	}
	return NULL;
}
