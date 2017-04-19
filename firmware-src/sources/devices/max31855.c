/*
 * max31855.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "max31855.h"
#include "dhspi.h"
#include "dhutils.h"
#include "dhgpio.h"

#include <osapi.h>
#include <c_types.h>

static int mCSPin = 15;

char * ICACHE_FLASH_ATTR max31855_read(int pin, float *temperature) {
	char buf[4];
	if(pin == MAX31855_NO_PIN) {
		dhspi_set_cs_pin(mCSPin);
	} else {
		int ok = (pin != DHSPI_NOCS);
		if(ok)
			ok = dhspi_set_cs_pin(pin);
		if(!ok)
			return "Wrong CS pin";
		mCSPin = pin;
	}
	dhspi_set_mode(SPI_CPOL0CPHA0);

	//start converting
	dhgpio_write((1 << mCSPin), 0);
	delay_ms(100);

	dhspi_read((char *)&buf, sizeof(buf));

	if(buf[3] & 0x01)
		return "Open circuit";
	if(buf[3] & 0x02)
		return "Short to GND";
	if(buf[3] & 0x03)
		return "Short to Vcc";

	buf[1] &= 0xFC;
	int v = signedInt16be(buf, 0);
	*temperature = v / 16.0f;

	return NULL;
}
