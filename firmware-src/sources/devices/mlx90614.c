/*
 * mlx90614.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "mlx90614.h"
#include "dhi2c.h"
#include "dhdebug.h"
#include "dhutils.h"

static int mAddress = MLX90614_DEFAULT_ADDRESS;

DHI2C_STATUS ICACHE_FLASH_ATTR mlx90614_read(int sda, int scl, float *ambient, float *object) {
	char buf[2];
	DHI2C_STATUS status;
	int raw_temperature;
	if(sda != MLX90614_NO_PIN && scl != MLX90614_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("mlx90614: failed to set up pins");
			return status;
		}
	}

    buf[0] = 0x06; // Ta register
    if((status = dhi2c_write(mAddress, buf, 1, 0)) != DHI2C_OK) {
		dhdebug("mlx90614: failed to get Ta register");
		return status;
	}
	if((status = dhi2c_read(mAddress, buf, 2)) != DHI2C_OK) {
		dhdebug("mlx90614: failed to read Ta");
		return status;
	}
	dhdebug_dump(buf, 2);
	raw_temperature = signedInt16le(buf, 0);
	*ambient = raw_temperature * 0.02f - 273.15f;

    buf[0] = 0x07; // Tobj1 register
    if((status = dhi2c_write(mAddress, buf, 1, 0)) != DHI2C_OK) {
		dhdebug("mlx90614: failed to get Tobj1 register");
		return status;
	}
	if((status = dhi2c_read(mAddress, buf, 2)) != DHI2C_OK) {
		dhdebug("mlx90614: failed to read Tobj1");
		return status;
	}
	dhdebug_dump(buf, 2);
	raw_temperature = signedInt16le(buf, 0);
	*object = raw_temperature * 0.02f - 273.15f;
	return DHI2C_OK;
}

void ICACHE_FLASH_ATTR mlx90614_set_address(int address) {
	mAddress = address;
}
