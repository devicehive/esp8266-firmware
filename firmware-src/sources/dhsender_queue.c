/*
 * dhsender_queue.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Queue for dhsender
 *
 */

#include <c_types.h>
#include <ets_sys.h>
#include <osapi.h>
#include <mem.h>
#include "dhsender_queue.h"
#include "user_config.h"
#include "dhdebug.h"
#include "dhgpio.h"
#include "snprintf.h"
#include "dhmem.h"
#include "dhsender_data.h"

LOCAL const char STATUS_OK[] = "OK";
LOCAL const char STATUS_ERROR[] = "Error";
LOCAL unsigned int mQueueMaxSize;

#define RESERVE_FOR_RESPONCE (mQueueMaxSize - 3)
#define MEM_RECOVER_THRESHOLD (mQueueMaxSize * 3 / 4)
#define MEMORY_RESERVER 10240
#define MAX_QUEUE_LENGTH 25

typedef struct {
	unsigned int				id;
	REQUEST_TYPE				type;
	REQUEST_NOTIFICATION_TYPE	notification_type;
	SENDERDATA					data;
	REQUEST_DATA_TYPE			data_type;
	unsigned int				data_len;
	unsigned int				pin;
} DHSENDER_QUEUE;

LOCAL DHSENDER_QUEUE *mQueue;
LOCAL int mQueueAddPos = 0;
LOCAL int mQueueTakePos = -1;
LOCAL unsigned int mQueueSize = 0;

int ICACHE_FLASH_ATTR dhsender_queue_add(REQUEST_TYPE type, REQUEST_NOTIFICATION_TYPE notification_type, REQUEST_DATA_TYPE data_type, unsigned int id, va_list ap) {
	ETS_INTR_LOCK();
	if(mQueueAddPos == mQueueTakePos) {
		ETS_INTR_UNLOCK();
		dhdebug("ERROR: no space for request");
		return 0;
	}
	mQueue[mQueueAddPos].id = id;
	mQueue[mQueueAddPos].type = type;
	mQueue[mQueueAddPos].data_type = data_type;
	mQueue[mQueueAddPos].notification_type = notification_type;
	dhsender_data_parse_va(ap, &data_type, &mQueue[mQueueAddPos].data,
			&mQueue[mQueueAddPos].data_len, &mQueue[mQueueAddPos].pin);
	if(mQueueTakePos < 0)
		mQueueTakePos = mQueueAddPos;
	if(mQueueAddPos >= mQueueMaxSize - 1)
		mQueueAddPos = 0;
	else
		mQueueAddPos++;
	mQueueSize++;
	if(mQueueSize > RESERVE_FOR_RESPONCE)
		dhmem_block();
	ETS_INTR_UNLOCK();
	return 1;
}

int ICACHE_FLASH_ATTR dhsender_queue_take(HTTP_REQUEST *out, unsigned int *is_notification) {
	if(mQueueTakePos < 0)
		return 0;
	ETS_INTR_LOCK();
	DHSENDER_QUEUE item;
	os_memcpy(&item, &mQueue[mQueueTakePos], sizeof(DHSENDER_QUEUE));
	if(mQueueTakePos >= mQueueMaxSize - 1)
		mQueueTakePos = 0;
	else
		mQueueTakePos++;
	if(mQueueAddPos == mQueueTakePos)
		mQueueTakePos = -1;
	mQueueSize--;
	ETS_INTR_UNLOCK();

	if(mQueueSize < MEM_RECOVER_THRESHOLD && dhmem_isblock())
		dhmem_unblock();

	char buf[HTTP_REQUEST_MIN_ALLOWED_PAYLOAD];
	if(dhsender_data_to_json(buf, sizeof(buf),
			item.notification_type == RNT_NOTIFICATION_GPIO, item.data_type,
			&item.data, item.data_len, item.pin) < 0) {
		snprintf(buf, sizeof(buf), "Failed to convert data to json");
		item.type = RT_RESPONCE_ERROR;
	}

	*is_notification = 0;
	switch(item.type) {
		case RT_RESPONCE_OK:
		case RT_RESPONCE_ERROR:
			dhrequest_create_update(out, item.id, (item.type == RT_RESPONCE_OK) ? STATUS_OK : STATUS_ERROR, buf);
			break;
		case RT_NOTIFICATION:
			*is_notification = 1;
			switch(item.notification_type) {
			case RNT_NOTIFICATION_GPIO:
				dhrequest_create_notification(out, "gpio/int", buf);
				break;
			case RNT_NOTIFICATION_ADC:
				dhrequest_create_notification(out, "adc/int", buf);
				break;
			case RNT_NOTIFICATION_UART:
				dhrequest_create_notification(out, "uart/int", buf);
				break;
			case RNT_NOTIFICATION_ONEWIRE:
				dhrequest_create_notification(out, "onewire/master/int", buf);
				break;
			default:
				dhdebug("ERROR: Unknown notification type of request %d", item.notification_type);
				return 0;
			}
			break;
		default:
			dhdebug("ERROR: Unknown type of request %d", item.type);
			return 0;
	}
	return 1;
}

unsigned int ICACHE_FLASH_ATTR dhsender_queue_length() {
	return mQueueSize;
}

void ICACHE_FLASH_ATTR dhsender_queue_init() {
	mQueueMaxSize = (system_get_free_heap_size() - MEMORY_RESERVER) / sizeof(DHSENDER_QUEUE);
	if(mQueueMaxSize > MAX_QUEUE_LENGTH)
		mQueueMaxSize = MAX_QUEUE_LENGTH;
	mQueue = (DHSENDER_QUEUE *)os_malloc(mQueueMaxSize * sizeof(DHSENDER_QUEUE));
	if(mQueue == 0) {
		dhdebug("ERROR: can not allocate memory for queue");
	} else {
		if(mQueueMaxSize < 10)
			dhdebug("Warning: queue is very shot - %u", mQueueMaxSize);
		else
			dhdebug("Queue created, size %u", mQueueMaxSize);
	}
}
