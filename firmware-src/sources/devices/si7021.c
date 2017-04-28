/**
 * @file
 * @brief Simple communication with SI7021 relative humidity and temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/si7021.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>

/** @brief Default sensor i2c address*/
#define SI7021_DEFAULT_ADDRESS 0x80

// module variables
static int mAddress = SI7021_DEFAULT_ADDRESS;


/*
 * si7021_read() implementation.
 */
int ICACHE_FLASH_ATTR si7021_read(int sda, int scl, float *humidity, float *temperature)
{
	int status;
	if(sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("si7021: failed to set up pins");
			return status;
		}
	}

	uint8_t buf[2];
	buf[0] = 0xE5; // read humidity, hold master
	if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("si7021: failed to write get humidity");
		return status;
	}
	if ((status = dh_i2c_read(mAddress, buf, 2)) != DH_I2C_OK) {
		dhdebug("si7021: failed to read humidity");
		return status;
	}
	*humidity = ((float)unsignedInt16be((const char*)buf, 0)) * 125.0f / 65536.0f - 6.0f;

	if (temperature) {
		buf[0] = 0xE0; // read temperature from humidity measurement
		if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
			dhdebug("si7021: failed to write get temperature");
			return status;
		}
		if ((status = dh_i2c_read(mAddress, buf, 2)) != DH_I2C_OK) {
			dhdebug("si7021: failed to read temperature");
			return status;
		}
		*temperature = ((float)unsignedInt16be((const char *)buf, 0)) * 175.72f / 65536.0f - 46.85f;
	}

	return DH_I2C_OK;
}


/*
 * si7021_set_address() implementation.
 */
void ICACHE_FLASH_ATTR si7021_set_address(int address)
{
	mAddress = address;
}
