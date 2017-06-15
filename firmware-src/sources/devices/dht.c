/**
 * @file
 * @brief Simple communication with DHT11 humidity sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/dht.h"
#include "DH/onewire.h"
#include "DH/adc.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <user_interface.h>
#include <ets_forward.h>

#define DHT_PACKET_SIZE         5
#define DHT_RESET_LENGTH_US     25000
#define DHT_TIMEOUT_US          200

#if defined(DH_DEVICE_DHT11) || defined(DH_DEVICE_DHT22)

/**
 * @brief Read DHT packet of 5 bytes.
 */
static const char* ICACHE_FLASH_ATTR dht_read_pkt(int pin, uint8_t buf[DHT_PACKET_SIZE])
{
	if (pin != DH_ONEWIRE_NO_PIN) {
		if (!!dh_onewire_set_pin(pin)) {
			return "Failed to set up onewire pin";
		}
	}

	if (dht_read(buf, DHT_PACKET_SIZE) != DHT_PACKET_SIZE) {
		return "Failed to read data";
	}

	int cs = ((int)buf[0] + (int)buf[1] + (int)buf[2] + (int)buf[3]);
	if ((cs&0xFF) != (int)buf[4]) {
		return "Bad checksum";
	}

	return 0; // OK
}

#endif /* DH_DEVICE_DHT11 || DH_DEVICE_DHT22 */


#if defined(DH_DEVICE_DHT11)

/*
 * dht11_read() implementation.
 */
const char* ICACHE_FLASH_ATTR dht11_read(int pin, int *humidity, int *temperature)
{
	uint8_t buf[DHT_PACKET_SIZE];
	const char *err_msg = dht_read_pkt(pin, buf);
	if (err_msg != 0)
		return err_msg;

	*humidity = buf[0];
	if (temperature)
		*temperature = buf[2];

	return NULL; // OK
}

#endif /* DH_DEVICE_DHT11 */


#if defined(DH_DEVICE_DHT22)

/*
 * dht22_read() implementation.
 */
const char* ICACHE_FLASH_ATTR dht22_read(int pin, float *humidity, float *temperature)
{
	uint8_t buf[DHT_PACKET_SIZE];
	const char *err_msg = dht_read_pkt(pin, buf);
	if (err_msg != 0)
		return err_msg;

	*humidity = signedInt16be_sm((const char*)buf, 0) / 10.0f;
	if (temperature)
		*temperature = signedInt16be_sm((const char*)buf, 2) / 10.0f;

	return NULL; // OK
}

#endif /* DH_DEVICE_DHT22 */


#if defined(DH_COMMANDS_ONEWIRE)

/**
 * @brief Measure high-level interval.
 */
static unsigned int ICACHE_FLASH_ATTR dht_measure_high(DHGpioPinMask pin_mask)
{
	unsigned int counter = 0;
	while (!(gpio_input_get() & pin_mask)) {
		if (counter > DHT_TIMEOUT_US)
			return 0; // timeout!
		os_delay_us(1);
		counter++;
	}

	counter = 0;
	while ((gpio_input_get() & pin_mask)) {
		if (counter > DHT_TIMEOUT_US)
			return DHT_TIMEOUT_US;
		os_delay_us(1);
		counter++;
	}

	return counter;
}


/*
 * dht_read() implementation.
 */
int ICACHE_FLASH_ATTR dht_read(void *buf_, size_t len)
{
	const DHGpioPinMask pin_mask = DH_GPIO_PIN(dh_onewire_get_pin());
	if (0 == dh_onewire_reset(pin_mask, 1))
		return 0; // no device present

	ETS_INTR_LOCK();
	if (dht_measure_high(pin_mask) >= DHT_TIMEOUT_US) { // wait till response finish
		ETS_INTR_UNLOCK();
		return 0; // failed
	}
	size_t i;
	uint8_t *buf = (uint8_t*)buf_;
	for(i = 0; i/8 < len; i++) {
		unsigned int time = dht_measure_high(pin_mask);
		if (time >= DHT_TIMEOUT_US || time <= 10) {
			break;
		} else {
			const int bit = BIT(7 - i%8);
			if (time > 25)
				buf[i/8] |= bit;
			else
				buf[i/8] &= ~bit;
		}
		system_soft_wdt_feed();
	}

	ETS_INTR_UNLOCK();
	return i/8;
}

#endif /* DH_COMMANDS_ONEWIRE */
