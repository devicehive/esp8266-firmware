/**
 * @file
 * @brief Simple communication with LM75 temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/lm75.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

/** @brief Default sensor i2c address*/
#define LM75_DEFAULT_ADDRESS 0x90

// module variables
static int mAddress = LM75_DEFAULT_ADDRESS;


/*
 * lm75_read() implementation.
 */
int ICACHE_FLASH_ATTR lm75_read(int sda, int scl, float *temperature)
{
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("lm75: failed to set up pins");
			return status;
		}
	}

	uint8_t buf[2];
	buf[0] = 0x0; // get temperature
	if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("lm75: failed to write get temperature command");
		return status;
	}
	if ((status = dh_i2c_read(mAddress, buf, 2)) != DH_I2C_OK) {
		dhdebug("lm75: failed to read temperature");
		return status;
	}

	const int raw = signedInt16be(buf, 0);
	*temperature = (float)raw / 256.0f;
	return DH_I2C_OK;
}


/*
 * lm75_set_address() implementation.
 */
void ICACHE_FLASH_ATTR lm75_set_address(int address)
{
	mAddress = address;
}
