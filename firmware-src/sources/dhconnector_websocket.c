/*
 * dhconnector_websocket.cpp
 *
 * Copyright 2017 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for connecting to remote DeviceHive server via WebSocket
 *
 */

#include "dhconnector_websocket.h"
#include "dhsettings.h"
#include "dhdebug.h"
#include "rand.h"
#include "user_config.h"
#include "dhconnector_websocket_api.h"
#include "dhsettings.h"
#include "dhutils.h"
#include "dhsender_data.h"
#include "dhsender.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <ets_forward.h>
#include <espconn.h>

#define PAYLOAD_BUF_SIZE (MAX( \
	ROUND_KB(DHSETTINGS_DEVICEID_MAX_LENGTH + DHSETTINGS_KEY_MAX_LENGTH + 512), \
	SENDER_JSON_MAX_LENGTH))
#define WEBSOCKET_HEADER_MAX_SIZE 4
#define WEBSOCKET_MASK_SIZE 4
#define WEBSOCKET_CONNECTION_TIMEOUT_MS 60000
#define WEBSOCKET_PING_TIMEOUT_MS 120000

LOCAL dhconnector_websocket_send_proto mSendFunc;
LOCAL dhconnector_websocket_error mErrFunc;
LOCAL char mBuf[PAYLOAD_BUF_SIZE + WEBSOCKET_HEADER_MAX_SIZE + WEBSOCKET_MASK_SIZE];
LOCAL char *mPayLoadBuf = &mBuf[WEBSOCKET_HEADER_MAX_SIZE + WEBSOCKET_MASK_SIZE];
LOCAL int mPayLoadBufLen = 0;
LOCAL os_timer_t mTimeoutTimer;

LOCAL void ICACHE_FLASH_ATTR error() {
	os_timer_disarm(&mTimeoutTimer);
	mErrFunc();
	dhsender_current_fail();
}

LOCAL void ICACHE_FLASH_ATTR timeout(void *arg) {
	dhdebug("WebSocket timeout");
	error();
}

LOCAL void ICACHE_FLASH_ATTR arm_timeout_timer(unsigned int ms) {
	os_timer_disarm(&mTimeoutTimer);
	os_timer_setfn(&mTimeoutTimer, (os_timer_func_t *)timeout, NULL);
	os_timer_arm(&mTimeoutTimer, ms, 0);
}

LOCAL void ICACHE_FLASH_ATTR mask(void) {
	uint32_t *mask = (uint32_t *)&mBuf[WEBSOCKET_HEADER_MAX_SIZE];
	*mask = rand();
	int i, j;
	for(i = 0, j = 0; i < mPayLoadBufLen; i++, j++) {
		if(j >= WEBSOCKET_MASK_SIZE)
			j = 0;
		mPayLoadBuf[i] ^= mBuf[WEBSOCKET_HEADER_MAX_SIZE + j];
	}
}

LOCAL int ICACHE_FLASH_ATTR send_payload() {
	if(mPayLoadBufLen <= 0) {
		return 0;
	} else if(mPayLoadBufLen < 126) {
		mBuf[2] = 0x81; // final text frame
		mBuf[3] = 0x80 | mPayLoadBufLen; // masked, size
		mask();
		return mSendFunc(&mBuf[2], mPayLoadBufLen + 2 + WEBSOCKET_MASK_SIZE);
	} else { // if(mPayLoadBufLen < 65536) - buf is always smaller then 65536, so there is no implementation for buf more then 65535 bytes.
		mBuf[0] = 0x81; // final text frame
		mBuf[1] = 0x80 | 126; // masked, size in the next two bytes
		mBuf[2] = (mPayLoadBufLen >> 8) & 0xFF;
		mBuf[3] = mPayLoadBufLen & 0xFF;
		mask();
		return mSendFunc(mBuf, mPayLoadBufLen + 4 + WEBSOCKET_MASK_SIZE);
	}
}

LOCAL void ICACHE_FLASH_ATTR check_queue(void) {
	SENDER_JSON_DATA *data = dhsender_next();
	if(data) {
		dhdebug("Sender start with %d bytes", data->jsonlen);
		// sizeof(data->json) always is equal or lower then PAYLOAD_BUF_SIZE
		os_memcpy(mPayLoadBuf, data->json, data->jsonlen);
		mPayLoadBufLen = data->jsonlen;
		if(send_payload())
			dhsender_current_success();
	}
}

void ICACHE_FLASH_ATTR dhconnector_websocket_start(dhconnector_websocket_send_proto send_func,
		dhconnector_websocket_error err_func) {
	mSendFunc = send_func;
	mErrFunc = err_func;

	dhsender_set_cb(check_queue);

	mPayLoadBufLen = dhconnector_websocket_api_start(mPayLoadBuf, PAYLOAD_BUF_SIZE);
	send_payload();
	arm_timeout_timer(WEBSOCKET_CONNECTION_TIMEOUT_MS);
}

void ICACHE_FLASH_ATTR dhconnector_websocket_parse(const char *data, unsigned int len) {
	// check and response on ping
	if(len == 2) {
		if(data[0] == 0x89 && data[1] == 0x00) {// PING
			if(dhconnector_websocket_api_check()) {
				arm_timeout_timer(WEBSOCKET_PING_TIMEOUT_MS);
			}
			static char pong_buf[2] = {0x8A, 0x00}; // PONG
			mSendFunc(pong_buf, sizeof(pong_buf));
			return;
		}
	}

	// check data
	if((data[0] & 0x80) == 0) {
		// always expect final text frame
		dhdebug("WebSocket error - wrong header 0x%X", data[0]);
		error();
		return;
	}
	if(data[1] & 0x80) {
		// always expect unmasked data
		dhdebug("WebSocket error - masked data from server");
		error();
		return;
	}

	// check length
	unsigned int wslen = (data[1] & 0x7F);
	if(wslen == 127) {
		dhdebug("WebSocket error - cannot handle more then 65535 bytes");
		error();
		return;
	} else if(wslen == 126) {
		wslen = data[2];
		wslen <<= 8;
		wslen |= data[3];
		data += 4;
		len -= 4;
	} else if(wslen < 126) {
		data += 2;
		len -= 2;
	}
	if(wslen != len) {
		// it is final frame, we checked before, received and header lengths should be equal
		dhdebug("WebSocket error - length mismatch");
		// TODO here could be several messages.
		dhdebug_dump(data, len);
		error();
		return;
	}

	// here we should have JSON in data and len variables
	mPayLoadBufLen = dhconnector_websocket_api_communicate(data, len, mPayLoadBuf, PAYLOAD_BUF_SIZE);
	if(mPayLoadBufLen > 0) {
		send_payload();
	} else if(mPayLoadBufLen == DHCONNECT_WEBSOCKET_API_ERROR) {
		error();
	} else { // successfully connected
		arm_timeout_timer(WEBSOCKET_PING_TIMEOUT_MS);
		// if we have data to send, we can do it
		check_queue();
	}
}
