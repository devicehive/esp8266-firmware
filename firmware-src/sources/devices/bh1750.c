/*
 * bh1750.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "bh1750.h"
#include "dhi2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

static int mAddress = BH1750_DEFAULT_ADDRESS;

DHI2C_STATUS ICACHE_FLASH_ATTR bh1750_read(int sda, int scl, float *illuminance) {
	char buf[2];
	DHI2C_STATUS status;
	if(sda != BH1750_NO_PIN && scl != BH1750_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("bh1750: failed to set up pins");
			return status;
		}
	}
	buf[0] = 0x21; // One Time High Resolution (0.5 lx)
	if((status = dhi2c_write(mAddress, buf, 1, 0)) != DHI2C_OK) {
		dhdebug("bh1750: failed to measure");
		return status;
	}

	delay_ms(180);
	if((status = dhi2c_read(mAddress, buf, 2)) != DHI2C_OK) {
		dhdebug("bh1750: failed to read");
		return status;
	}

	*illuminance = (float)unsignedInt16be(buf, 0) / 1.2f / 2.0f;
	return DHI2C_OK;
}

void ICACHE_FLASH_ATTR bh1750_set_address(int address) {
	mAddress = address;
}
