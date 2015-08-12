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

int ICACHE_FLASH_ATTR dhnotification_gpio_read_to_json(char *out, unsigned int value) {
	int len = os_sprintf(out, "{");
	int i;
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		const unsigned int pin = 1 << i;
		const pinvalue = (value & pin) ? 1 : 0;
		if(DHGPIO_SUITABLE_PINS & pin) {
			len += os_sprintf(&out[len], (i == 0) ? "\"%d\":\"%d\"" : ", \"%d\":\"%d\"", i, pinvalue);
		}
	}
	return len + os_sprintf(&out[len], "}");
}

void ICACHE_FLASH_ATTR dhgpio_int_timeout(unsigned int caused_pins) {
	if(dhmem_isblock())
		return;
	const unsigned int gpio_state = dhgpio_read();
	char gpiostatebuff[512]; // 16 gpio inputs with '"10":"1", 'x2, format - 10*32=320 + {,}, null terminated char etc
	char *ptr = gpiostatebuff;
	ptr += os_sprintf(ptr, "{\"caused\":[");
	int i;
	int comma = 0;
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		const unsigned int pin = 1 << i;
		if(DHGPIO_SUITABLE_PINS & pin == 0)
			continue;
		if( pin & caused_pins) {
			ptr += os_sprintf(ptr, comma?", \"%d\"":"\"%d\"", i);
			comma = 1;
		}
	}
	ptr += os_sprintf(ptr, "], \"state\":");
	ptr += dhnotification_gpio_read_to_json(ptr, gpio_state);
	os_sprintf(ptr, "}");
	dhsender_notification("gpio/int", gpiostatebuff);
}

void ICACHE_FLASH_ATTR dhadc_loop_value(float value){
	if(dhmem_isblock())
		return;
	char adcvalue[48];
	snprintf(adcvalue, sizeof(adcvalue), "{\"0\":\"%f\"}", value);
	dhsender_notification("adc/int", adcvalue);
}

char * ICACHE_FLASH_ATTR dhnotification_prepare_data_parameters(char *buf, unsigned int bufsize, const char *data, unsigned int len) {
	const unsigned int pos = os_sprintf(buf, "{\"data\":\"");
	const unsigned int res = dhdata_encode(data, len, &buf[pos], bufsize - pos - 3);
	if(res == 0)
		return "Failed to convert data in base64";
	os_sprintf( &buf[pos + res], "\"}");
	return 0;
}

void ICACHE_FLASH_ATTR dhuart_buf_rcv(const char *buf, unsigned int len) {
	if(dhmem_isblock())
		return;
	char notification[128 + len * 2];
	char *res = dhnotification_prepare_data_parameters(notification, sizeof(notification), buf, len);
	if(res) {
		dhdebug(res);
		return;
	}
	dhsender_notification("uart/int", notification);
}
