/*
 * max6675.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "max6675.h"
#include "DH/gpio.h"
#include "DH/spi.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

static int mCSPin = 15;

char * ICACHE_FLASH_ATTR max6675_read(int pin, float *temperature) {
	char buf[2];
	if(pin == MAX6675_NO_PIN) {
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
	delay_ms(250);

	dh_spi_read((char *)&buf, sizeof(buf));

	if((buf[0] & 0x80) || (buf[1] & 0x02))
		return "Protocol error";
	if(buf[1] & 0x04)
		return "Thermocouple is not connected";

	buf[1] &= 0xF8;
	int v = signedInt16be(buf, 0);
	*temperature = v / 32.0f;

	return NULL;
}
