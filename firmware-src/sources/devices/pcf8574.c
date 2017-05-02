/**
 * @file
 * @brief Simple communication with PCF8574 GPIO extender.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/pcf8574.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>

/** @brief Default sensor i2c address. */
#define PCF8574_DEFAULT_ADDRESS 0x4E

// module variables
static int mAddress = PCF8574_DEFAULT_ADDRESS;


/*
 * pcf8574_read() implementation.
 */
int ICACHE_FLASH_ATTR pcf8574_read(int sda, int scl, unsigned int *pins)
{
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("pcf8574: failed to set up pins");
			return status;
		}
	}

	uint8_t buf;
	if ((status = dh_i2c_read(mAddress, &buf, 1)) != DH_I2C_OK) {
		dhdebug("pcf8574: failed to read");
		return status;
	}

	*pins = buf;
	return DH_I2C_OK;
}


/*
 * pcf8574_write() implementation.
 */
int ICACHE_FLASH_ATTR pcf8574_write(int sda, int scl, unsigned int pins_to_set, unsigned int pins_to_clear)
{
	if (pins_to_set & pins_to_clear)
		return DH_I2C_WRONG_PARAMETERS;

	int status;
	unsigned int current_state;
	if ((status = pcf8574_read(sda, scl, &current_state)) != DH_I2C_OK) {
		return status;
	}

	uint8_t buf = ((current_state | pins_to_set) & ~pins_to_clear) & PCF8574_SUITABLE_PINS;
	if ((status = dh_i2c_write(mAddress, &buf, 1, 1)) != DH_I2C_OK) {
		dhdebug("pcf8574: failed to write");
		return status;
	}

	return DH_I2C_OK;
}


/*
 * pcf8574_set() implementation.
 */
int ICACHE_FLASH_ATTR pcf8574_set(int sda, int scl, unsigned int pins)
{
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("pcf8574: failed to set up pins");
			return status;
		}
	}

	uint8_t buf = pins & PCF8574_SUITABLE_PINS;
	if ((status = dh_i2c_write(mAddress, &buf, 1, 1)) != DH_I2C_OK) {
		dhdebug("pcf8574: failed to write");
		return status;
	}

	return DH_I2C_OK;
}


/*
 * pcf8574_set_address() implementation.
 */
void ICACHE_FLASH_ATTR pcf8574_set_address(int address)
{
	mAddress = address;
}
