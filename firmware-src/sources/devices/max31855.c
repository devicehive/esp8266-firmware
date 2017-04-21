/*
 * max31855.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "max31855.h"
#include "DH/gpio.h"
#include "DH/spi.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

static int mCSPin = 15;

char * ICACHE_FLASH_ATTR max31855_read(int pin, float *temperature) {
	char buf[4];
	if(pin == MAX31855_NO_PIN) {
		dh_spi_set_cs_pin(mCSPin);
	} else {
		int ok = (pin != DH_SPI_NO_CS);
		if(ok)
			ok = (0 == dh_spi_set_cs_pin(pin));
		if(!ok)
			return "Wrong CS pin";
		mCSPin = pin;
	}
	dh_spi_set_mode(DH_SPI_CPOL0CPHA0);

	//start converting
	dh_gpio_write(DH_GPIO_PIN(mCSPin), 0);
	delay_ms(100);

	dh_spi_read((char *)&buf, sizeof(buf));

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
