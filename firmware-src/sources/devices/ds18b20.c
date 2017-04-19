/*
 * ds18b20.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "ds18b20.h"
#include "dhonewire.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

char * ICACHE_FLASH_ATTR ds18b20_read(int pin, float *temperature) {
	char in_buf[8];
	char out_buf[2];
	int16_t *res = (int16_t *)&in_buf[0];
	out_buf[0] = 0xCC;
	if(pin != DS18B20_NO_PIN) {
		if(!dhonewire_set_pin(pin)) {
			return "Failed to set up onewire pin";
		}
	}
	out_buf[1] = 0x44; // start measure, use defaults
	if(!dhonewire_write(out_buf, sizeof(out_buf))) {
		return "No response";
	}

	delay_ms(750); // maximum possible time for measure

	out_buf[1] = 0xBE; // read memory
	if(!dhonewire_write(out_buf, sizeof(out_buf))) {
		return "Failed to read";
	}
	if(!dhonewire_read(in_buf, sizeof(in_buf))) {
		return "Failed to read data";
	}
	*temperature = (*res / 16.0f); // default precision
	return NULL;
}
