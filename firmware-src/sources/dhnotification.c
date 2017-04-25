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
#include "dhnotification.h"
#include "dhsender.h"
#include "DH/gpio.h"
#include "DH/adc.h"
#include "user_config.h"
#include "snprintf.h"
#include "dhdata.h"
#include "dhdebug.h"
#include "dhstatistic.h"
#include "dhmem.h"

#include <ets_sys.h>
#include <os_type.h>
#include <osapi.h>
#include <user_interface.h>

void ICACHE_FLASH_ATTR dh_gpio_int_cb(DHGpioPinMask caused_pins) {
	if(dhmem_isblock()) {
		dhstat_got_notification_dropped();
		return;
	}
	dhsender_notification(RNT_NOTIFICATION_GPIO, RDT_GPIO, caused_pins, dh_gpio_read(), system_get_time(), DH_GPIO_SUITABLE_PINS);
}

void ICACHE_FLASH_ATTR dh_adc_loop_value_cb(float value){
	if(dhmem_isblock()) {
		dhstat_got_notification_dropped();
		return;
	}
	dhsender_notification(RNT_NOTIFICATION_ADC, RDT_FLOAT, value);
}

void ICACHE_FLASH_ATTR dh_uart_buf_rcv_cb(const void *buf, size_t len) {
	if(dhmem_isblock()) {
		dhstat_got_notification_dropped();
		return;
	}

	dhsender_notification(RNT_NOTIFICATION_UART, RDT_DATA_WITH_LEN, buf, len);
}


/*
 * dh_onewire_search_result() implementation.
 */
void ICACHE_FLASH_ATTR dh_onewire_search_result(unsigned int pin, const void *buf, size_t len)
{
	if (dhmem_isblock()) {
		dhstat_got_notification_dropped();
		return;
	}
	dhsender_notification(RNT_NOTIFICATION_ONEWIRE, RDT_SEARCH64, pin, buf, len);
}
