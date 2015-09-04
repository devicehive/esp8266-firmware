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

LOCAL const char STATUS_OK[] = "OK";
LOCAL const char STATUS_ERROR[] = "Error";
LOCAL unsigned int mQueueMaxSize;

#define RESERVE_FOR_RESPONCE (mQueueMaxSize - 3)
#define MEM_RECOVER_THRESHOLD (mQueueMaxSize * 3 / 4)
#define MEMORY_RESERVER 10240

typedef struct {
	unsigned int caused;
	unsigned int state;
	unsigned int timestamp;
} GPIO_DATA;

typedef union {
	char array[INTERFACES_BUF_SIZE];
	const char *string;
	float adc;
	GPIO_DATA gpio;
} SENDERDATA;

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
	switch(data_type) {
		case RDT_CONST_STRING:
			mQueue[mQueueAddPos].data.string = va_arg(ap, const char *);
			mQueue[mQueueAddPos].data_len = sizeof(const char *);
			break;
		case RDT_GPIO:
			mQueue[mQueueAddPos].data.gpio.caused = va_arg(ap, unsigned int);
			mQueue[mQueueAddPos].data.gpio.state = va_arg(ap, unsigned int);
			mQueue[mQueueAddPos].data.gpio.timestamp = va_arg(ap, unsigned int);
			mQueue[mQueueAddPos].data_len = sizeof(GPIO_DATA);
			break;
		case RDT_FLOAT:
			mQueue[mQueueAddPos].data.adc = (float)va_arg(ap, double);
			mQueue[mQueueAddPos].data_len = sizeof(float);
			break;
		case RDT_SEARCH64:
			mQueue[mQueueAddPos].pin = va_arg(ap, unsigned int);
			/* no break */
		case RDT_DATA_WITH_LEN:
		{
			const char *s = va_arg(ap, char *);
			char *d = mQueue[mQueueAddPos].data.array;
			unsigned int l = va_arg(ap, unsigned int);
			mQueue[mQueueAddPos].data_len = l;
			while(l--)
				*d++ = *s++;
		}
			break;
		default:
			mQueue[mQueueAddPos].data.string = "ERROR: Unknown request data type.";
			mQueue[mQueueAddPos].type = RDT_CONST_STRING;
			dhdebug("ERROR: Unknown request data type %d", data_type);
	}
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

LOCAL unsigned int ICACHE_FLASH_ATTR gpio_state(char *buf, unsigned int buflen, unsigned int state) {
	unsigned int len = snprintf(buf, buflen, "{");
	unsigned int i;
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		const unsigned int pin = 1 << i;
		const pinvalue = (state & pin) ? 1 : 0;
		if(DHGPIO_SUITABLE_PINS & pin) {
			len += snprintf(&buf[len], buflen - len, (i == 0) ? "\"%d\":\"%d\"" : ", \"%d\":\"%d\"", i, pinvalue);
		}
	}
	return len + snprintf(&buf[len], buflen - len, "}");
}

LOCAL void ICACHE_FLASH_ATTR gpio_notification(char *buf, unsigned int buflen, const GPIO_DATA *data) {
	unsigned int len = snprintf(buf, buflen, "{\"caused\":[");
	unsigned int i;
	int comma = 0;
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		const unsigned int pin = 1 << i;
		if(DHGPIO_SUITABLE_PINS & pin == 0)
			continue;
		if( pin & data->caused) {
			len += snprintf(&buf[len], buflen - len, comma?", \"%d\"":"\"%d\"", i);
			comma = 1;
		}
	}
	len += snprintf(&buf[len], buflen - len, "], \"state\":");
	len += gpio_state(&buf[len], buflen - len, data->state);
	snprintf(&buf[len], buflen - len, ", \"tick\":\"%u\"}", data->timestamp);
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
	const char *result = buf;
	switch(item.data_type) {
		case RDT_CONST_STRING:
			result = item.data.string;
			break;
		case RDT_DATA_WITH_LEN:
		{
			const unsigned int pos = snprintf(buf, sizeof(buf), "{\"data\":\"");
			const unsigned int res = dhdata_encode(item.data.array, item.data_len, &buf[pos], sizeof(buf) - pos - 3);
			if(res == 0) {
				result = "Failed to convert data in base64";
				item.type = RT_RESPONCE_ERROR;
			} else {
				snprintf(&buf[pos + res], sizeof(buf) - pos, "\"}");
			}
			break;
		}
		case RDT_FLOAT:
			snprintf(buf, sizeof(buf), "{\"0\":\"%f\"}", item.data.adc);
			break;
		case RDT_GPIO:
			if(item.notification_type == RNT_NOTIFICATION_GPIO)
				gpio_notification(buf, sizeof(buf), &item.data.gpio);
			else
				gpio_state(buf, sizeof(buf), item.data.gpio.state);
			break;
		case RDT_SEARCH64:
		{
			unsigned int i;
			unsigned int len = snprintf(buf, sizeof(buf), "{\"found\":[");
			if(item.data_len) {
				len += snprintf(&buf[len], sizeof(buf) - len, "\"");
				for(i = 0; i < item.data_len; i++) {
					if(i % 8 == 0 &&  i != 0) {
						len += snprintf(&buf[len], sizeof(buf) - len, "\", \"");
					}
					if(len + 2 < sizeof(buf))
						len += byteToHex(item.data.array[item.data_len - i - 1], &buf[len]);
				}
				len += snprintf(&buf[len], sizeof(buf) - len, "\"");
			}
			snprintf(&buf[len], sizeof(buf) - len, "], \"pin\":\"%d\"}", item.pin);
			break;
		}
		default:
			dhdebug("ERROR: Unknown data type of request %d", item.data_type);
			return 0;
	}
	*is_notification = 0;
	switch(item.type) {
		case RT_RESPONCE_OK:
		case RT_RESPONCE_ERROR:
			dhrequest_create_update(out, item.id, (item.type == RT_RESPONCE_OK) ? STATUS_OK : STATUS_ERROR, result);
			break;
		case RT_NOTIFICATION:
			*is_notification = 1;
			switch(item.notification_type) {
			case RNT_NOTIFICATION_GPIO:
				dhrequest_create_notification(out, "gpio/int", result);
				break;
			case RNT_NOTIFICATION_ADC:
				dhrequest_create_notification(out, "adc/int", result);
				break;
			case RNT_NOTIFICATION_UART:
				dhrequest_create_notification(out, "uart/int", result);
				break;
			case RNT_NOTIFICATION_ONEWIRE:
				dhrequest_create_notification(out, "onewire/master/int", result);
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
