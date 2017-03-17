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
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include "dhsettings.h"
#include "dhdebug.h"
#include "rand.h"
#include "user_config.h"
#include "dhconnector_websocket_api.h"

#define PAYLOAD_BUF_SIZE (DHSETTINGS_DEVICEID_MAX_LENGTH + DHSETTINGS_ACCESSKEY_MAX_LENGTH + 512)
#define WEBSOCKET_HEADER_MAX_SIZE 4
#define WEBSOCKET_MASK_SIZE 4

LOCAL dhconnector_websocket_send_proto mSendFunc;
LOCAL dhconnector_websocket_error mErrFunc;
LOCAL char mBuf[PAYLOAD_BUF_SIZE + WEBSOCKET_HEADER_MAX_SIZE + WEBSOCKET_MASK_SIZE];
LOCAL char *mPayLoadBuf = &mBuf[WEBSOCKET_HEADER_MAX_SIZE + WEBSOCKET_MASK_SIZE];
LOCAL int mPayLoadBufLen = 0;

LOCAL void ICACHE_FLASH_ATTR mask() {
	uint32_t *mask = (uint32_t *)&mBuf[WEBSOCKET_HEADER_MAX_SIZE];
	*mask = rand();
	int i, j;
	for(i = 0, j = 0; i < mPayLoadBufLen; i++, j++) {
		if(j >= WEBSOCKET_MASK_SIZE)
			j = 0;
		mPayLoadBuf[i] ^= mBuf[WEBSOCKET_HEADER_MAX_SIZE + j];
	}
}

LOCAL void ICACHE_FLASH_ATTR send_payload() {
	if(mPayLoadBufLen <= 0) {
		return;
	} else if(mPayLoadBufLen < 126) {
		mBuf[2] = 0x81; // final text frame
		mBuf[3] = 0x80 | mPayLoadBufLen; // masked, size
dhdebug_dump(&mBuf[2], mPayLoadBufLen + 2 + WEBSOCKET_MASK_SIZE);
		mask();
		mSendFunc(&mBuf[2], mPayLoadBufLen + 2 + WEBSOCKET_MASK_SIZE);
	} else { // if(mPayLoadBufLen < 65536) - buf is always smaller then 65536, so there is no implementation for buf more then 65535 bytes.
		mBuf[0] = 0x81; // final text frame
		mBuf[1] = 0x80 | 126; // masked, size in the next two bytes
		mBuf[2] = (mPayLoadBufLen >> 8) & 0xFF;
		mBuf[3] = mPayLoadBufLen & 0xFF;
dhdebug_dump(mBuf, mPayLoadBufLen + 4 + WEBSOCKET_MASK_SIZE);
		mask();
		mSendFunc(mBuf, mPayLoadBufLen + 4 + WEBSOCKET_MASK_SIZE);
	}
}

void ICACHE_FLASH_ATTR dhconnector_websocket_start(dhconnector_websocket_send_proto send_func,
		dhconnector_websocket_error err_func) {
	mSendFunc = send_func;
	mErrFunc = err_func;

	mPayLoadBufLen = dhconnector_websocket_api_start(mPayLoadBuf, PAYLOAD_BUF_SIZE);
	send_payload();
	// TODO check with timeout that we have connected
}

void ICACHE_FLASH_ATTR dhconnector_websocket_parse(const char *data, unsigned int len) {
	// check and response on ping
	if(len == 2) {
		if(data[0] == 0x89 && data[1] == 0x00) {// PING
			static char pong_buf[2] = {0x8A, 0x00}; // PONG
			mSendFunc(pong_buf, sizeof(pong_buf));
			return;
		}
	}

	// check data
	if(data[0] != 0x81) {
		// always expect final text frame
		dhdebug("WebSocket error - wrong header 0x%X", data[0]);
		mErrFunc();
		return;
	}
	if(data[1] & 0x80) {
		// always expect unmasked data
		dhdebug("WebSocket error - masked data from server");
		mErrFunc();
		return;
	}

	// check length
	unsigned int wslen = (data[1] & 0x7F);
	if(wslen == 127) {
		dhdebug("WebSocket error - cannot handle more then 65535 bytes");
		mErrFunc();
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
		mErrFunc();
		return;
	}

	// here we should have JSON in data and len variables
	dhdebug("WS got response");
dhdebug_dump(data, len);

	mPayLoadBufLen = dhconnector_websocket_api_communicate(data, len, mPayLoadBuf, PAYLOAD_BUF_SIZE);
	if(mPayLoadBufLen == DHCONNECT_WEBSOCKET_API_ERROR)
		mErrFunc();
	else if(mPayLoadBufLen > 0)
		send_payload();
}
