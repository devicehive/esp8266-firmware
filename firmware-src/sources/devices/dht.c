/*
 * dht.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "dht.h"
#include "DH/onewire.h"
#include "DH/adc.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <user_interface.h>
#include <ets_forward.h>

#define DHT_PACKET_SIZE         5
#define DHT_RESET_LENGTH_US     25000
#define DHT_TIMEOUT_US          200

LOCAL char * ICACHE_FLASH_ATTR dht_read(int pin, char *buf) {
	if(pin != DHT_NO_PIN) {
		if (!!dh_onewire_set_pin(pin)) {
			return "Failed to set up onewire pin";
		}
	}
	if(dhonewire_dht_read(buf, DHT_PACKET_SIZE) != DHT_PACKET_SIZE ) {
		return "Failed to read data";
	}
	char cs = ((int)buf[0] + (int)buf[1] + (int)buf[2] + (int)buf[3]) & 0xFF;
	if(cs != buf[4]) {
		return "Bad checksum";
	}
	return 0;
}

char * ICACHE_FLASH_ATTR dht11_read(int pin, int *humidity, int *temperature) {
	char buf[DHT_PACKET_SIZE];
	char *r = dht_read(pin, buf);
	if(r)
		return r;
	*humidity = (uint8_t)buf[0];
	if(temperature)
		*temperature = (uint8_t)buf[2];
	return NULL;
}

char * ICACHE_FLASH_ATTR dht22_read(int pin, float *humidity, float *temperature) {
	char buf[DHT_PACKET_SIZE];
	char *r = dht_read(pin, buf);
	if(r)
		return r;

	*humidity = signedInt16be_sm(buf, 0) / 10.0f;
	if(temperature)
		*temperature = signedInt16be_sm(buf, 2) / 10.0f;
	return NULL;
}


LOCAL unsigned int ICACHE_FLASH_ATTR donewire_dht_measure_high(unsigned int pin) {
	unsigned int counter = 0;
	while((gpio_input_get() & pin) == 0) {
		if(counter > DHT_TIMEOUT_US)
			return 0;
		os_delay_us(1);
		counter++;
	}
	counter = 0;
	while((gpio_input_get() & pin)) {
		if(counter > DHT_TIMEOUT_US)
			return DHT_TIMEOUT_US;
		os_delay_us(1);
		counter++;
	}
	return counter;
}

int ICACHE_FLASH_ATTR dhonewire_dht_read(char *buf, unsigned int len) {
	const unsigned int pin = DH_GPIO_PIN(dh_onewire_get_pin());
	if (dh_onewire_reset(pin, 1) == 0)
		return 0;
	ETS_INTR_LOCK();
	unsigned int i;
	if(donewire_dht_measure_high(pin) >= DHT_TIMEOUT_US) { // wait till response finish
		ETS_INTR_UNLOCK();
		return 0;
	}
	for(i = 0; i / 8 < len; i++) {
		unsigned int time = donewire_dht_measure_high(pin);
		if(time >= DHT_TIMEOUT_US || time <= 10) {
			break;
		} else {
			const char bit = 1 << (7 - i % 8);
			if(time > 25)
				buf[i / 8] |= bit;
			else
				buf[i / 8] &= ~bit;
		}
		system_soft_wdt_feed();
	}
	ETS_INTR_UNLOCK();
	return i / 8;
}


/**
 * @brief Do "onewire/dht/read" command.
 */
void ICACHE_FLASH_ATTR dh_handle_onewire_dht_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_PIN, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(dh_onewire_init_helper(cb, fields, &parse_pins))
			return;
	}
	parse_pins.count = dhonewire_dht_read(parse_pins.data, sizeof(parse_pins.data));
	if(parse_pins.count)
		dh_command_done_buf(cb, parse_pins.data, parse_pins.count);
	else
		dh_command_fail(cb, "No response");
}

