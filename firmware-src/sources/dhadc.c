/*
 * dhadc.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include "dhadc.h"

os_timer_t mADCTimer;

float ICACHE_FLASH_ATTR dhadc_get_value(){
	return system_adc_read()/1024.0f;
}

LOCAL void ICACHE_FLASH_ATTR send_adc_data(void *arg) {
	dhadc_loop_value(dhadc_get_value());
}

void ICACHE_FLASH_ATTR dhadc_loop(unsigned int period) {
	os_timer_disarm(&mADCTimer);
	if(period) {
		os_timer_setfn(&mADCTimer, (os_timer_func_t *)send_adc_data, NULL);
		os_timer_arm(&mADCTimer, period, 1);
	}
}
