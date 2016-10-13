/*
 * bh1750.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "bh1750.h"
#include "dhi2c.h"
#include "dhdebug.h"
#include "dhutils.h"

static int mAddress = BH1750_DEFAULT_ADDRESS;

float ICACHE_FLASH_ATTR bh1750_read(int sda, int scl) {
	char buf[2];
	unsigned int raw_illuminance;
	if (sda != BH1750_NO_PIN || scl != BH1750_NO_PIN) {
		if (dhi2c_init(sda, scl) != DHI2C_OK) {
			dhdebug("bh1750: failed to set up pins");
			return BH1750_ERROR;
		}
	}
	buf[0] = 0x21; // One Time High Resolution (0.5 lx)
	if (dhi2c_write(mAddress, buf, 1, 0) != DHI2C_OK) {
		dhdebug("bh1750: failed to measure");
		return BH1750_ERROR;
	}

	os_delay_us(180000);
	if (dhi2c_read(mAddress, buf, 2) != DHI2C_OK) {
		dhdebug("bh1750: failed to read");
		return BH1750_ERROR;
	}
	raw_illuminance = unsignedInt16(buf, 0);

	return (float)raw_illuminance / 1.2f / 2.0f;
}

void ICACHE_FLASH_ATTR bh1750_set_address(int address) {
	mAddress = address;
}
