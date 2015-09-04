/*
 * dhsender.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for sending notifications and command's responses to DeviceHive server
 *
 */

#include <stdarg.h>
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include <espconn.h>
#include "user_config.h"
#include "dhsender.h"
#include "dhrequest.h"
#include "dhdebug.h"
#include "dhesperrors.h"
#include "dhutils.h"
#include "dhstatistic.h"

#define DHSENDER_RETRY_COUNT 5

LOCAL struct espconn mDHSender = {0};
LOCAL os_timer_t mRepeatTimer = {0};
LOCAL unsigned char mStopped = 0;
LOCAL HTTP_REQUEST mSenderRequest;
LOCAL unsigned int isCurrentNotification;
LOCAL int mSenderTook = 0;

LOCAL void dhsender_next(void *arg);

LOCAL void ICACHE_FLASH_ATTR dhsender_arm_timer(unsigned int ms) {
	os_timer_disarm(&mRepeatTimer);
	os_timer_setfn(&mRepeatTimer, (os_timer_func_t *)dhsender_next, NULL);
	os_timer_arm(&mRepeatTimer, ms, 0);
}

LOCAL void ICACHE_FLASH_ATTR dhsender_next(void *arg) {
	if(mStopped)
		return;
	if(mSenderTook == 0) {
		if(dhsender_queue_take(&mSenderRequest, &isCurrentNotification))
			mSenderTook = DHSENDER_RETRY_COUNT;
		else
			return;
	}
	sint8 cr = espconn_connect(&mDHSender);
	if(cr == ESPCONN_ISCONN) {
		return;
	} else if (cr != ESPCONN_OK) {
		dhesperrors_espconn_result("Sender espconn_connect failed:", cr);
		dhsender_arm_timer(RETRY_CONNECTION_INTERVAL_MS);
	} else {
		dhdebug("Sender start");
	}
}

LOCAL void ICACHE_FLASH_ATTR decrementSenderTook() {
	if(mSenderTook) {
		if(mSenderTook == 1) {
			dhdebug("WARNING: Request is not delivered after %u attempts", DHSENDER_RETRY_COUNT);
			if(isCurrentNotification)
				dhstatistic_inc_notifications_dropped_count();
			else
				dhstatistic_inc_responces_dropped_count();
		}
		mSenderTook--;
	}
}

LOCAL void ICACHE_FLASH_ATTR senderDisconnectCb(void *arg) {
	if(mSenderTook == 0) {
		dhsender_arm_timer(DHREQUEST_PAUSE_MS);
	} else {
		decrementSenderTook();
		dhsender_arm_timer(RETRY_CONNECTION_INTERVAL_MS);
	}
}

LOCAL void ICACHE_FLASH_ATTR senderErrorCb(void *arg, sint8 err) {
	dhesperrors_espconn_result("Sender error occurred:", err);
	decrementSenderTook();
	dhsender_arm_timer(RETRY_CONNECTION_INTERVAL_MS);
	dhstatistic_inc_network_errors_count();
}

LOCAL void ICACHE_FLASH_ATTR senderRecvCb(void *arg, char *data, unsigned short len) {
	dhstatistic_add_bytes_received(len);
	const char *rc = find_http_responce_code(data, len);
	if (rc) { // HTTP
		if (*rc == '2') { // HTTP responce code 2xx - Success
			mSenderTook = 0;
			dhdebug("Sender received OK");
		} else {
			dhdebug("Sender HTTP response bad status %c%c%c", rc[0],rc[1],rc[2]);
			dhdebug(data);
			dhdebug("--------------------------------------");
			dhstatistic_server_errors_count();
		}
	} else {
		dhdebug("Sender received wrong HTTP magic");
		dhstatistic_server_errors_count();
	}
	espconn_disconnect(&mDHSender);
}

LOCAL void ICACHE_FLASH_ATTR senderConnectCb(void *arg) {
	int res;
	if( (res = espconn_send(&mDHSender, mSenderRequest.data, mSenderRequest.len)) != ESPCONN_OK) {
		dhesperrors_espconn_result("sender espconn_send failed:", res);
		espconn_disconnect(&mDHSender);
	} else {
		dhstatistic_add_bytes_sent(mSenderRequest.len);
	}
}

void ICACHE_FLASH_ATTR dhsender_init(ip_addr_t *ip, int port) {
	static esp_tcp tcp;
	static int local_port = -1;
	if(local_port == -1)
		local_port = espconn_port();
	mDHSender.type = ESPCONN_TCP;
	mDHSender.state = ESPCONN_NONE;
	mDHSender.proto.tcp = &tcp;
	mDHSender.proto.tcp->local_port = local_port;
	mDHSender.proto.tcp->remote_port = port;
	os_memcpy(mDHSender.proto.tcp->remote_ip, &ip->addr, sizeof(ip->addr));
	espconn_regist_connectcb(&mDHSender, senderConnectCb);
	espconn_regist_recvcb(&mDHSender, senderRecvCb);
	espconn_regist_reconcb(&mDHSender, senderErrorCb);
	espconn_regist_disconcb(&mDHSender, senderDisconnectCb);
	mStopped = 0;
	dhsender_arm_timer(DHREQUEST_PAUSE_MS);
}

void ICACHE_FLASH_ATTR dhsender_stop_repeat() {
	mStopped = 1;
	os_timer_disarm(&mRepeatTimer);
}

void ICACHE_FLASH_ATTR dhsender_response(unsigned int id, RESPONCE_STATUS status, REQUEST_DATA_TYPE data_type, ...) {
	va_list ap;
	va_start(ap, data_type);
	dhstatistic_inc_responces_count();
	if(dhsender_queue_add(status == DHSTATUS_ERROR ? RT_RESPONCE_ERROR : RT_RESPONCE_OK, RNT_NOTIFICATION_NONE, data_type, id, ap)) {
		dhsender_next(NULL);
	} else {
		dhstatistic_inc_responces_dropped_count();
		dhdebug("ERROR: No memory for response.");
	}
	va_end(ap);
}

void ICACHE_FLASH_ATTR dhsender_notification(REQUEST_NOTIFICATION_TYPE type, REQUEST_DATA_TYPE data_type, ...) {
	va_list ap;
	va_start(ap, data_type);
	dhstatistic_inc_notifications_count();
	if(dhsender_queue_add(RT_NOTIFICATION, type, data_type, 0, ap)) {
		dhsender_next(NULL);
	} else {
		dhstatistic_inc_notifications_dropped_count();
		dhdebug("ERROR: No memory for notification.");
	}
	va_end(ap);
}
