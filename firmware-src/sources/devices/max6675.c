/*
 * max6675.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "max6675.h"
#include "dhspi.h"
#include "dhgpio.h"

static int mCSPin = 15;

char * ICACHE_FLASH_ATTR max6675_read(int pin, float *temperature) {
	char buf[2];
	if(pin == MAX6675_NO_PIN) {
		dhspi_set_cs_pin(mCSPin);
	} else {
		int ok = (pin != DHSPI_NOCS);
		if(ok)
			ok = dhspi_set_cs_pin(pin);
		if(!ok)
			return "Wrong CS pin";
		mCSPin = pin;
	}

	//start converting
	dhgpio_write((1 << mCSPin), 0);
	os_delay_us(250000);

	dhspi_set_mode(SPI_CPOL0CPHA0);
	dhspi_read((char *)&buf, sizeof(buf));

	if ((buf[0] & 0x80) || (buf[1] & 0x02))
		return "Protocol error";
	if (buf[1] & 0x04)
		return "Thermocouple is not connected";

	int v = (buf[1] >> 3) & 0x1F;
	v += (((int)buf[0]) & 0x7F) * 0x20;
	*temperature = v / 4.0f;

	return NULL;
}
