/*
 * ds18b20.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "ds18b20.h"
#include "dhonewire.h"
#include "dhdebug.h"

float ICACHE_FLASH_ATTR ds18b20_read(int pin) {
	char in_buf[8];
	char out_buf[2];
	int16_t *res = (int16_t *)&in_buf[0];
	out_buf[0] = 0xCC;
	if (pin != DS18B20_NO_PIN) {
		if (!dhonewire_set_pin(pin)) {
			dhdebug("ds18b20: failed to set up onewire pin");
			return DS18B20_ERROR;
		}
	}
	out_buf[1] = 0x44; // start measure, use defaults
	if (!dhonewire_write(out_buf, sizeof(out_buf))) {
		dhdebug("ds18b20: failed write start measure command");
		return DS18B20_ERROR;
	}

	os_delay_us(750*1000); // maximum possible time for measure

	out_buf[1] = 0xBE; // read memory
	if (!dhonewire_write(out_buf, sizeof(out_buf))) {
		dhdebug("ds18b20: failed to send read memory command");
		return DS18B20_ERROR;
	}
	if (!dhonewire_read(in_buf, sizeof(in_buf))) {
		dhdebug("ds18b20: failed to read data");
		return DS18B20_ERROR;
	}
	return (*res / 16.0f); // default precision
}
