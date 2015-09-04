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
#include "snprintf.h"

static const char REGISTER_JSON[] =
		"{\"name\":\"%s\",\"key\":\"%s\",\"status\":\"Online\",\"deviceClass\":{\"name\":\"ESP Class\",\"version\":\"1.0\",\"offlineTimeout\":\"120\"}}";
static const char UPDATE_JSON[] =
		"{\"status\":\"%s\",\"result\":%s%s%s}";
static const char NOTIFICATION_JSON[] =
		"{\"notification\":\"%s\",\"parameters\":%s%s%s}";

static const char HTTP_REQUEST_PATTERN[] =
		"%s %s%s%s%s%s HTTP/1.0\r\nAuth-DeviceID: %s\r\nAuth-DeviceKey: %s\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %s\r\n\r\n%s";

LOCAL char mServer[DHSETTINGS_SERVER_MAX_LENGTH];
LOCAL char mDeviceId[DHSETTINGS_DEVICEID_MAX_LENGTH];
LOCAL char mDeviceKey[DHSETTINGS_DEVICEKEY_MAX_LENGTH];

LOCAL unsigned short mServerLen;
LOCAL unsigned short mDeviceIdLen;
LOCAL unsigned short mDeviceKeyLen;

void ICACHE_FLASH_ATTR dhrequest_load_settings() {
	mServerLen = snprintf(mServer, sizeof(mServer), "%s", dhsettings_get_devicehive_server());
	mDeviceIdLen = snprintf(mDeviceId, sizeof(mDeviceId), "%s", dhsettings_get_devicehive_deviceid());
	mDeviceKeyLen = snprintf(mDeviceKey, sizeof(mDeviceKey), "%s", dhsettings_get_devicehive_devicekey());
}

const char *dhrequest_current_server() {
	return mServer;
}

void ICACHE_FLASH_ATTR dhrequest_create_register(HTTP_REQUEST *buf) {
	char lenbuf[16];
	char jsonbuf[sizeof(REGISTER_JSON) - 4 + DHSETTINGS_DEVICEID_MAX_LENGTH + DHSETTINGS_DEVICEKEY_MAX_LENGTH];
	const unsigned int jsonbuflen = snprintf(jsonbuf, sizeof(jsonbuf), REGISTER_JSON, mDeviceId, mDeviceKey);
	snprintf(lenbuf, sizeof(lenbuf), "%d", jsonbuflen);
	buf->len = snprintf(buf->data, sizeof(buf->data), HTTP_REQUEST_PATTERN, "PUT", mServer, "/device/", mDeviceId, "", "", mDeviceId, mDeviceKey, lenbuf, jsonbuf);
	dhdebug("Register request created, %d/%d", buf->len + 1, sizeof(buf->data));
}

void ICACHE_FLASH_ATTR dhrequest_create_poll(HTTP_REQUEST *buf, const char *timestamp) {
	dhrequest_update_poll(buf, timestamp);
}

void ICACHE_FLASH_ATTR dhrequest_update_poll(HTTP_REQUEST *old, const char *timestamp) {
	const unsigned int timestamplength = os_strlen(timestamp);
	const unsigned int len = sizeof(HTTP_REQUEST_PATTERN) - 20 + 3 + mServerLen + 8 + mDeviceIdLen + 24 + timestamplength + mDeviceIdLen + mDeviceKeyLen + 1;
	if(old) {
		if(old->len + 1 == len) {
			os_memcpy(&old->data[4 + mServerLen + 8 + mDeviceIdLen + 24], timestamp, timestamplength);
			dhdebug("Poll request updated");
			return;
		}
	}
	old->len = snprintf(old->data, sizeof(old->data), HTTP_REQUEST_PATTERN, "GET", mServer, "/device/", mDeviceId, "/command/poll?timestamp=", timestamp, mDeviceId, mDeviceKey, "0", "");
	dhdebug("Poll request created, %d/%d", old->len + 1, sizeof(old->data));
}

void ICACHE_FLASH_ATTR dhrequest_create_update(HTTP_REQUEST *buf, unsigned int commandId, const char *status, const char *result) {
	char idbuf[16];
	snprintf(idbuf, sizeof(idbuf), "%u", commandId);
	char jsonbuf[sizeof(UPDATE_JSON) - 6 + os_strlen(status) + os_strlen(result)];
	char extra[] = "\"";
	if(result[0] == '{')
		extra[0] = 0;
	const unsigned int jsonbuflen = snprintf(jsonbuf, sizeof(jsonbuf), UPDATE_JSON, status, extra, result, extra);
	char lenbuf[16];
	snprintf(lenbuf, sizeof(lenbuf), "%d", jsonbuflen);
	buf->len = snprintf(buf->data, sizeof(buf->data), HTTP_REQUEST_PATTERN, "PUT", mServer, "/device/", mDeviceId, "/command/", idbuf, mDeviceId, mDeviceKey, lenbuf, jsonbuf);
	dhdebug("Update request created, %d/%d", buf->len + 1, sizeof(buf->data));
}

void ICACHE_FLASH_ATTR dhrequest_create_info(HTTP_REQUEST *buf) {
	buf->len = snprintf(buf->data, sizeof(buf->data), HTTP_REQUEST_PATTERN, "GET", mServer, "/info", "", "", "", mDeviceId, mDeviceKey, "0", "");
	dhdebug("Info request created, %d/%d", buf->len + 1, sizeof(buf->data));
}

void ICACHE_FLASH_ATTR dhrequest_create_notification(HTTP_REQUEST *buf, const char *name, const char *parameters) {
	char jsonbuf[sizeof(NOTIFICATION_JSON) - 6 + os_strlen(name) + os_strlen(parameters)];
	char extra[] = "\"";
	if(parameters[0] == '{')
		extra[0] = 0;
	const unsigned int jsonbuflen = snprintf(jsonbuf, sizeof(jsonbuf), NOTIFICATION_JSON, name, extra, parameters, extra);
	char lenbuf[16];
	snprintf(lenbuf, sizeof(lenbuf), "%d", jsonbuflen);
	buf->len = snprintf(buf->data, sizeof(buf->data), HTTP_REQUEST_PATTERN, "POST", mServer, "/device/", mDeviceId, "/notification", "", mDeviceId, mDeviceKey, lenbuf, jsonbuf);
	dhdebug("Notification created, %d/%d", buf->len + 1, sizeof(buf->data));
}
