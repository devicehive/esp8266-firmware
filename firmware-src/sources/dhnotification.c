/*
 * dhnotification.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for catching hardware events and preparing notification to DeviceHive server
 *
 */

#include <ets_sys.h>
#include <os_type.h>
#include <osapi.h>

#include "dhsender.h"
#include "dhgpio.h"
#include "dhadc.h"
#include "dhnotification.h"
#include "user_config.h"
#include "snprintf.h"
#include "dhdata.h"
#include "dhdebug.h"
#include "dhstatistic.h"

void ICACHE_FLASH_ATTR dhgpio_int_timeout(unsigned int caused_pins) {
	if(dhmem_isblock()) {
		dhstatistic_inc_notifications_dropped_count();
		return;
	}
	dhsender_notification(RNT_NOTIFICATION_GPIO, RDT_GPIO, caused_pins, dhgpio_read(), system_get_time());
}

void ICACHE_FLASH_ATTR dhadc_loop_value(float value){
	if(dhmem_isblock()) {
		dhstatistic_inc_notifications_dropped_count();
		return;
	}
	dhsender_notification(RNT_NOTIFICATION_ADC, RDT_FLOAT, value);
}

void ICACHE_FLASH_ATTR dhuart_buf_rcv(const char *buf, unsigned int len) {
	if(dhmem_isblock()) {
		dhstatistic_inc_notifications_dropped_count();
		return;
	}
	dhsender_notification(RNT_NOTIFICATION_UART, RDT_DATA_WITH_LEN, buf, len);
}

 void ICACHE_FLASH_ATTR dhonewire_search_result(unsigned int pin_number, char *buf, unsigned long len) {
	 if(dhmem_isblock()) {
		dhstatistic_inc_notifications_dropped_count();
		return;
	}
	dhsender_notification(RNT_NOTIFICATION_ONEWIRE, RDT_SEARCH64, pin_number, buf, len);
 }
