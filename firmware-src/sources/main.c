/*
 * user_main.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: DeviceHive firmware for ESP8266
 *
 */

#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <user_interface.h>
#include "gpio.h"
#include "dhcommands.h"
#include "dhdebug.h"
#include "dhsender_queue.h"
#include "dhterminal.h"
#include "dhsettings.h"
#include "dhap.h"
#include "webserver.h"

typedef struct {
	unsigned int magic;
	unsigned int resetCounter;
} RESET_COUNTER;
#define RESET_COUNTER_MAGIC 0x12345678
#define RESET_COUNTER_RTC_ADDRESS 64
#define RESET_NUM 3

LOCAL os_timer_t mResetTimer;
LOCAL unsigned int mSpecialMode = 0;

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
	enum flash_size_map size_map = system_get_flash_size_map();
	uint32 rf_cal_sec = 0;

	switch (size_map) {
	case FLASH_SIZE_4M_MAP_256_256:
		rf_cal_sec = 128 - 8;
		break;

	case FLASH_SIZE_8M_MAP_512_512:
		rf_cal_sec = 256 - 5;
		break;

	case FLASH_SIZE_16M_MAP_512_512:
	case FLASH_SIZE_16M_MAP_1024_1024:
		rf_cal_sec = 512 - 5;
		break;

	case FLASH_SIZE_32M_MAP_512_512:
	case FLASH_SIZE_32M_MAP_1024_1024:
		rf_cal_sec = 1024 - 5;
		break;

	default:
		rf_cal_sec = 0;
		break;
	}

	return rf_cal_sec;
}

LOCAL void ICACHE_FLASH_ATTR reset_counter(void *arg) {
	RESET_COUNTER counter;
	counter.magic = RESET_COUNTER_MAGIC;
	counter.resetCounter = 0;
	system_rtc_mem_write(RESET_COUNTER_RTC_ADDRESS, &counter, sizeof(counter));
}

extern int rtc_mem_check(int f);

void user_rf_pre_init(void) {
	RESET_COUNTER counter;
	system_rtc_mem_read(64, &counter, sizeof(counter));
	if(counter.magic == RESET_COUNTER_MAGIC && counter.resetCounter <= RESET_NUM) {
		counter.resetCounter++;
		if(counter.resetCounter == RESET_NUM) {
			reset_counter(0);
			mSpecialMode = 1;
		} else {
			system_rtc_mem_write(RESET_COUNTER_RTC_ADDRESS, &counter, sizeof(counter));
			os_timer_disarm(&mResetTimer);
			os_timer_setfn(&mResetTimer, (os_timer_func_t *)reset_counter, NULL);
			os_timer_arm(&mResetTimer, 3000, 0);
		}
	} else {
		reset_counter(0);
	}

	system_restore();
	rtc_mem_check(0);
}

void user_init(void) {
	if(mSpecialMode) {
		system_set_os_print(0);
		dhsettings_init();
		dhap_init();
	} else {
		dhterminal_init();
		dhdebug("*****************************");
		dhsender_queue_init();
		dhsettings_init();
		dhconnector_init(dhcommands_do);
		dhgpio_init();
		webserver_init();
		dhdebug("Initialization completed");
	}
}
