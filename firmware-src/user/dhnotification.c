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
#include "dhnotification.h"
#include "user_config.h"
#include "snprintf.h"

os_timer_t mADCTimer;

LOCAL void gpio_int(void *arg) {
	const unsigned int gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
	const unsigned int gpio_state = dhgpio_read();
	char gpiostatebuff[512]; // 32 gpio inputs with '"10":"1", ' format - 10*32=320 + {,}, null terminated char etc
	char *ptr = gpiostatebuff;
	ptr += os_sprintf(ptr, "{\"caused\":[");
	int i;
	int comma = 0;
	for(i = 0; i < 32; i++) {
		const unsigned int pin = 1 << i;
		if(DHGPIO_SUITABLE_PINS & pin == 0)
			continue;
		if( pin & gpio_status) {
			ptr += os_sprintf(ptr, comma?", \"%d\"":"\"%d\"", i);
			comma = 1;
		}
	}
	ptr += os_sprintf(ptr, "], \"state\":");
	ptr += dhgpio_read_to_json(ptr, gpio_state);
	os_sprintf(ptr, "}");
	dhsender_notification("gpio", gpiostatebuff);
}

LOCAL void ICACHE_FLASH_ATTR send_adc_data() {
	char adcvalue[48];
	snprintf(adcvalue, sizeof(adcvalue), "{\"0\":\"%f\"}", dhadc_get_value()/1024.0f);
	dhsender_notification("adc", adcvalue);
}


void ICACHE_FLASH_ATTR dhnotification_adc(unsigned int period) {
	os_timer_disarm(&mADCTimer);
	if(period) {
			os_timer_setfn(&mADCTimer, (os_timer_func_t *)send_adc_data, NULL);
			os_timer_arm(&mADCTimer, period, 1);
	}
}

void ICACHE_FLASH_ATTR dhnotification_init() {
	ETS_GPIO_INTR_ATTACH(gpio_int, NULL);
	ETS_GPIO_INTR_ENABLE();
}
