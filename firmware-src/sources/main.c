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
#include "dhdebug.h"
#include "dhuart.h"
#include "dhsender_queue.h"
#include "dhterminal.h"
#include "dhsettings.h"
#include "dhconnector.h"
#include "dhap.h"
#include "DH/gpio.h"
#include "webserver.h"
#include "irom.h"
#include "uploadable_page.h"
#include "dhzc_dnsd.h"
#include "dhzc_web.h"
#include "mdnsd.h"

#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <user_interface.h>
#include <gpio.h>
#include <ets_forward.h>

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
	if(system_get_rst_info()->reason != REASON_EXT_SYS_RST) {
		reset_counter(0);
	} else {
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
				os_timer_arm(&mResetTimer, 1000, 0);
			}
		} else {
			reset_counter(0);
		}
	}

	system_restore();
	rtc_mem_check(0);
}

void ICACHE_FLASH_ATTR system_init_done(void) {
	if(dhsettings_get_wifi_mode() == WIFI_MODE_AP &&
			dhsettings_get_devicehive_deviceid()[0] &&
			mSpecialMode == 0) {
		mdnsd_start(dhsettings_get_devicehive_deviceid(), dhap_get_ip_info()->ip.addr);
	}
	dhdebug("Initialization completed");
}

void user_init(void) {
	int ever_saved;
	gpio_output_set(0, 0, 0, DH_GPIO_SUITABLE_PINS);
	dhsettings_init(&ever_saved);
	if(ever_saved == 0) { // if first run on this chip
		uploadable_page_delete();
		mSpecialMode = 1;
	}
	dhdebug("*****************************");
	if(mSpecialMode) { // if special mode was called by user or if there is no settings
		dhuart_leds(DHUART_LEDS_ON);
		dhdebug("Wi-Fi Zero Configuration Mode");
		dhap_init(WIFI_CONFIGURATION_SSID, NULL);
		dhzc_dnsd_init();
		dhzc_web_init();
		dhdebug("Zero configuration server is initialized");
	} else {
		if(dhsettings_get_wifi_mode() == WIFI_MODE_CLIENT) {
			dhsender_queue_init();
			dhconnector_init();
			dh_gpio_init();
		} else if(dhsettings_get_wifi_mode() == WIFI_MODE_AP) {
			dhap_init(dhsettings_get_wifi_ssid(), dhsettings_get_wifi_password());
		}
		webserver_init();
	}
	system_init_done_cb(system_init_done);
	dhterminal_init();
}
