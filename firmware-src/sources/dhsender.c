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
#include <ctype.h>
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

LOCAL SENDER_JSON_DATA mDataToSend;
LOCAL unsigned int isCurrentNotification;
LOCAL int mSenderTook = 0;

SENDER_JSON_DATA * ICACHE_FLASH_ATTR dhsender_next(void *arg) {
	if(mSenderTook == 0) {
		if(dhsender_queue_take(&mDataToSend, &isCurrentNotification) == 0)
				return NULL;
		mSenderTook = DHSENDER_RETRY_COUNT;
		dhdebug("Sender start");
	}
	return &mDataToSend;
}

void ICACHE_FLASH_ATTR dhsender_current_fail() {
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

void ICACHE_FLASH_ATTR dhsender_current_success() {
	mSenderTook = 0;
}

void ICACHE_FLASH_ATTR dhsender_response(CommandResultArgument cid, RESPONCE_STATUS status, REQUEST_DATA_TYPE data_type, ...) {
	va_list ap;
	va_start(ap, data_type);
	dhstatistic_inc_responces_count();
	if(dhsender_queue_add(status == DHSTATUS_ERROR ? RT_RESPONCE_ERROR : RT_RESPONCE_OK, RNT_NOTIFICATION_NONE, data_type, cid.id, ap)) {
		dhsender_next(NULL);
	} else {
		dhstatistic_inc_responces_dropped_count();
		dhdebug("ERROR: No memory for response");
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
		dhdebug("ERROR: No memory for notification");
	}
	va_end(ap);
}
