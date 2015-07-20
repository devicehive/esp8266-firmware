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

struct espconn mDHSender = {0};
os_timer_t mRepeatTimer = {0};
HTTP_REQUEST *mRequestsQueue = NULL;

LOCAL void ICACHE_FLASH_ATTR remove_first() {
	if(mRequestsQueue == NULL)
		return;
	HTTP_REQUEST *request = mRequestsQueue;
	mRequestsQueue = request->next;
	dhrequest_free(request);
}

LOCAL void ICACHE_FLASH_ATTR remove_half() {
	if(mRequestsQueue == NULL)
		return;
	HTTP_REQUEST *request = mRequestsQueue;
	int i, len = 1;
	while(request->next) {
		len++;
		request =request->next;
	}
	len = len/2;
	for(i=0; i<len; i++)
		remove_first();
}

LOCAL void ICACHE_FLASH_ATTR dhsender_next(void *arg) {
	if(mRequestsQueue == NULL)
		return;
	sint8 cr = espconn_connect(&mDHSender);
	if (cr != ESPCONN_OK) {
		if(cr == ESPCONN_ISCONN)
			return;
		dhesperrors_espconn_result("Sender espconn_connect failed:", cr);
		os_timer_disarm(&mRepeatTimer);
		os_timer_setfn(&mRepeatTimer, (os_timer_func_t *)dhsender_next, NULL);
		os_timer_arm(&mRepeatTimer, RETRY_CONNECTION_INTERVAL_MS, 0);
	} else {
		dhdebug("Sender start");
	}
}

LOCAL void ICACHE_FLASH_ATTR senderDisconnectCb(void *arg) {
	dhdebug("Sender disconnect");
	remove_first();
	dhsender_next(NULL);
}

LOCAL void ICACHE_FLASH_ATTR senderErrorCb(void *arg, sint8 err) {
	dhesperrors_espconn_result("Sender error occurred:", err);
	dhsender_next(NULL);
}

LOCAL void ICACHE_FLASH_ATTR senderRecvCb(void *arg, char *data, unsigned short len) {
	const char *rc = dhrequest_find_http_responce_code(data, len);
	if (rc) { // HTTP
		if (*rc == '2') { // HTTP responce code 2xx - Success
			dhdebug("Sender received OK");
		} else {
			dhdebug("Sender HTTP response bad status %c%c%c", rc[0],rc[1],rc[2]);
			dhdebug(data);
			dhdebug("--------------------------------------");
		}
	} else {
		dhdebug("Sender received wrong HTTP magic");
	}
	espconn_disconnect(&mDHSender);
}

LOCAL void ICACHE_FLASH_ATTR senderConnectCb(void *arg) {
	dhdebug("Sender connected");
	int res;
	if( (res = espconn_sent(&mDHSender, mRequestsQueue->body, mRequestsQueue->len)) != ESPCONN_OK) {
		dhesperrors_espconn_result("senderConnectCb failed:", res);
		espconn_disconnect(&mDHSender);
	}
}

void ICACHE_FLASH_ATTR dhsender_addrequest(HTTP_REQUEST *request) {
	if(mRequestsQueue) {
		HTTP_REQUEST *last = mRequestsQueue;
		while(last->next)
			last = last->next;
		last->next = request;
	} else {
		mRequestsQueue = request;
		dhsender_next(NULL);
	}
}

void ICACHE_FLASH_ATTR dhsender_response(int id, const char *status, const char *result) {
	HTTP_REQUEST *request = dhrequest_create_update(id, status, result);
	while(request == NULL) {
		if(mRequestsQueue == NULL) {
			dhdebug("Error: dhsender_response memory leak detected");
			return;
		}
		dhdebug("Error: no memory, drop old requests");
		remove_half();
		request = dhrequest_create_update(id, status, result);
	}
	dhsender_addrequest(request);
}

void ICACHE_FLASH_ATTR dhsender_notification(const char *name, const char *parameters) {
	HTTP_REQUEST *request = dhrequest_create_notification(name, parameters);
	while(request == NULL) {
		if(mRequestsQueue == NULL) {
			dhdebug("Error: dhsender_notification memory leak detected");
			return;
		}
		dhdebug("Error: no memory, drop old requests");
		remove_half();
		request = dhrequest_create_notification(name, parameters);
	}
	dhsender_addrequest(request);
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
}
