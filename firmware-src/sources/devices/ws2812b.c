/*
 * ws2812b.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include <ets_sys.h>
#include <gpio.h>
#include "ws2812b.h"
#include "dhgpio.h"

static int mPin = (1 << 2);

LOCAL inline void ICACHE_FLASH_ATTR ws2812b_sleep(unsigned tacts) {
	unsigned f, r;
	asm volatile("rsr %0, ccount" : "=r"(f));
	f += tacts;
	do {
		asm volatile("rsr %0, ccount" : "=r"(r));
	} while(r < f);
}

char * ICACHE_FLASH_ATTR ws2812b_write(int pin, const char *buf, unsigned int len) {
	int i, j;
	// send reset to line
	if(pin != WS2812B_NO_PIN) {
		if(pin < 0 || pin > DHGPIO_MAXGPIONUM || ((1 << pin) & DHGPIO_SUITABLE_PINS) == 0)
			return "Wrong pin";
		mPin = (1 << pin);
	}

	// init number of CPU tacts for delay
	// delays are lower that spec on 50ns due to GPIO latency, anyway spec allows 150ns jitter.
	const int freq = system_get_cpu_freq(); // MHz
	unsigned T0H = 350 * freq / 1000;
	unsigned T1H = 800 * freq / 1000;
	unsigned T0L = 750 * freq / 1000;
	unsigned T1L = 400 * freq / 1000;

	ETS_GPIO_INTR_DISABLE();

	// send reset
	gpio_output_set(mPin, 0, mPin, 0);
	os_delay_us(10);
	gpio_output_set(0, mPin, 0, 0);
	os_delay_us(50);
	gpio_output_set(mPin, 0, 0, 0);

	// send each byte
	for(i = 0; i < len; i++) {
		const char b = buf[i];
		for(j = 1; j <= 0x80; j <<= 1) {
			if(b & j) {
					ws2812b_sleep(T1H);
					gpio_output_set(0, mPin, 0, 0);
					ws2812b_sleep(T1L);
					gpio_output_set(mPin, 0, 0, 0);
				} else {
					ws2812b_sleep(T0H);
					gpio_output_set(0, mPin, 0, 0);
					ws2812b_sleep(T0L);
					gpio_output_set(mPin, 0, 0, 0);
				}
			}
	}

	ETS_GPIO_INTR_ENABLE();

	return NULL;
}
