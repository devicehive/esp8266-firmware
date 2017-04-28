/**
 * @file
 * @brief Simple communication with MAX6675 K-thermocouple temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/max6675.h"
#include "DH/gpio.h"
#include "DH/spi.h"
#include "dhutils.h"

#include <osapi.h>

// module variables
static int mCSPin = 15;

/*
 * max6675_read() implementation.
 */
const char* ICACHE_FLASH_ATTR max6675_read(int pin, float *temperature)
{
	if (pin == DH_SPI_NO_PIN) {
		dh_spi_set_cs_pin(mCSPin);
	} else {
		int ok = (pin != DH_SPI_NO_CS);
		if (ok)
			ok = (0 == dh_spi_set_cs_pin(pin));
		if (!ok)
			return "Wrong CS pin";
		mCSPin = pin;
	}
	dh_spi_set_mode(DH_SPI_CPOL0CPHA0);

	// start converting
	dh_gpio_write(DH_GPIO_PIN(mCSPin), 0);
	delay_ms(250);

	uint8_t buf[2];
	dh_spi_read(buf, sizeof(buf));

	if ((buf[0] & 0x80) || (buf[1] & 0x02))
		return "Protocol error";
	if (buf[1] & 0x04)
		return "Thermocouple is not connected";

	buf[1] &= 0xF8;
	int v = signedInt16be((const char*)buf, 0);
	*temperature = v / 32.0f;

	return NULL; // OK
}
