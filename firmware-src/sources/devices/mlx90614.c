/**
 * @file
 * @brief Simple communication with MLX90614 contactless IR temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/mlx90614.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>

/** @brief Default sensor i2c address*/
#define MLX90614_DEFAULT_ADDRESS 0xB4

// module variable
static int mAddress = MLX90614_DEFAULT_ADDRESS;


/*
 * mlx90614_read() implementation.
 */
int ICACHE_FLASH_ATTR mlx90614_read(int sda, int scl, float *ambient, float *object)
{
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("mlx90614: failed to set up pins");
			return status;
		}
	}

	uint8_t buf[2];
	buf[0] = 0x06; // Ta register
	if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("mlx90614: failed to get Ta register");
		return status;
	}
	if ((status = dh_i2c_read(mAddress, buf, 2)) != DH_I2C_OK) {
		dhdebug("mlx90614: failed to read Ta");
		return status;
	}

	int raw = signedInt16le((const char*)buf, 0);
	*ambient = raw * 0.02f - 273.15f;

	buf[0] = 0x07; // Tobj1 register
	if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("mlx90614: failed to get Tobj1 register");
		return status;
	}
	if ((status = dh_i2c_read(mAddress, buf, 2)) != DH_I2C_OK) {
		dhdebug("mlx90614: failed to read Tobj1");
		return status;
	}

	raw = signedInt16le((const char*)buf, 0);
	*object = raw * 0.02f - 273.15f;

	return DH_I2C_OK;
}


/*
 * mlx90614_set_address() implementation.
 */
void ICACHE_FLASH_ATTR mlx90614_set_address(int address)
{
	mAddress = address;
}
