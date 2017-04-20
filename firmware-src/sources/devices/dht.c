/*
 * dht.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "dht.h"
#include "dhonewire.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

#define DHT_PACKET_SIZE 5

LOCAL char * ICACHE_FLASH_ATTR dht_read(int pin, char *buf) {
	if(pin != DHT_NO_PIN) {
		if(!dhonewire_set_pin(pin)) {
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
