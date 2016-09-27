/*
 * dht11.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "dht11.h"
#include "dhonewire.h"
#include "dhdebug.h"

int ICACHE_FLASH_ATTR dht11_read(int pin, int *temperature) {
	unsigned char buf[5];
	if (!dhonewire_set_pin(pin)) {
		dhdebug("dht11: failed to set up onewire pin");
		return DHT11_ERROR;
	}
	if (dhonewire_dht_read(buf, sizeof(buf)) != sizeof(buf)) {
		dhdebug("dht11: failed to read data");
		return DHT11_ERROR;
	}
	char cs = (buf[0] + buf[1] + buf[2] + buf[3]) & 0xFF;
	if (cs != buf[4]) {
		dhdebug("dht11: bad checksum");
		return DHT11_ERROR;
	}
	if (temperature)
		*temperature = buf[2];
	return buf[0];
}
