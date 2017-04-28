/**
 * @file
 * @brief Simple communication with DS18B20 temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/ds18b20.h"
#include "DH/onewire.h"
#include "dhutils.h"

#include <osapi.h>


/*
 * ds18b20_read() implementation.
 */
const char* ICACHE_FLASH_ATTR ds18b20_read(int pin, float *temperature)
{
	if (pin != DH_ONEWIRE_NO_PIN) {
		if(!!dh_onewire_set_pin(pin)) {
			return "Failed to set up onewire pin";
		}
	}

	uint8_t buf[8];
	buf[0] = 0xCC;
	buf[1] = 0x44; // start measure, use defaults
	if (!!dh_onewire_write(buf, 2)) {
		return "No response";
	}

	delay_ms(750); // maximum possible time for measure

	// buf[0] = 0xCC;
	buf[1] = 0xBE; // read memory
	if (!!dh_onewire_write(buf, 2)) {
		return "Failed to read";
	}
	if (!!dh_onewire_read(buf, sizeof(buf))) {
		return "Failed to read data";
	}

	const int16_t *t = (const int16_t *)&buf[0];
	*temperature = (t[0] / 16.0f); // default precision

	return NULL; // OK
}
