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
#include "devices/ds18b20.h"

typedef struct {
	unsigned int magic;
	unsigned int resetCounter;
} RESET_COUNTER;
#define RESET_COUNTER_MAGIC 0x12345678
#define RESET_COUNTER_RTC_ADDRESS 64
#define RESET_NUM 3

LOCAL os_timer_t mResetTimer;
LOCAL unsigned int mSpecialMode = 0;

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
		int dht11_temperature;
		float dht22_temperature;
		dhgpio_write(1 << 4, 0); // power up sensors
		os_delay_us(300000); //wait sensors
		float ds18b20_temperature = ds18b20_read(1);
		int dht11_humiduty = dht11_read(2, &dht11_temperature);
		float dht22_humiduty = dht22_read(5, &dht22_temperature);
		float bmp180_temperature;
		int bmp180_pressure = bmp180_read(12, 14, &bmp180_temperature);

		dhdebug("-----------------------------");
		dhdebug("ds18b20 temperature = %f C", ds18b20_temperature);
		dhdebug("dht11 humidity %d %%, temperature %d C", dht11_humiduty, dht11_temperature);
		dhdebug("dht22 humidity %f %%, temperature %f C", dht22_humiduty, dht22_temperature);
		dhdebug("bmp180 pressure %d Pa, temperature %f C", bmp180_pressure, bmp180_temperature);
		dhdebug("-----------------------------");
		dhgpio_initialize(1 << 1, 0, 0);

		dhterminal_init();
		dhdebug("*****************************");
		dhsender_queue_init();
		dhsettings_init();
		dhconnector_init(dhcommands_do);
		dhgpio_init();
		dhdebug("Initialization completed");
	}
}
