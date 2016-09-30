/*
 * dht.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "dht.h"
#include "dhonewire.h"
#include "dhdebug.h"

#define DHT_PACKET_SIZE 5

LOCAL int ICACHE_FLASH_ATTR dht_read(int pin, char *buf) {
	if (!dhonewire_set_pin(pin)) {
		dhdebug("dht: failed to set up onewire pin");
		return DHT_ERROR;
	}
	if (dhonewire_dht_read(buf, DHT_PACKET_SIZE) != DHT_PACKET_SIZE ) {
		dhdebug("dht: failed to read data");
		return DHT_ERROR;
	}
	char cs = ((int)buf[0] + (int)buf[1] + (int)buf[2] + (int)buf[3]) & 0xFF;
	if (cs != buf[4]) {
		dhdebug("dht: bad checksum");
		return DHT_ERROR;
	}
	return 0;
}

int ICACHE_FLASH_ATTR dht11_read(int pin, int *temperature) {
	unsigned char buf[DHT_PACKET_SIZE];
	if (dht_read(pin, buf) == DHT_ERROR)
		return DHT_ERROR;
	if (temperature)
		*temperature = buf[2];
	return buf[0];
}

float ICACHE_FLASH_ATTR dht22_read(int pin, float *temperature) {
	unsigned char buf[DHT_PACKET_SIZE];
	if (dht_read(pin, buf) == DHT_ERROR)
		return DHT_ERROR;

	float humidity = signedInt16(buf, 0) / 10.0f;
	float temp = signedInt16(buf, 2) / 10.0f;
	if (temperature)
		*temperature = temp;
	return humidity;
}
