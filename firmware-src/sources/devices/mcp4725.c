/*
 * mcp4725.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "mcp4725.h"
#include "dhi2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

static int mAddress = MCP4725_DEFAULT_ADDRESS;
static float mVoltage = 3.3f;

DHI2C_STATUS ICACHE_FLASH_ATTR mcp4725_write(int sda, int scl, float value) {
	char buf[3];
	DHI2C_STATUS status;
	if(value < 0.0f || value > mVoltage)
		return DHI2C_WRONG_PARAMETERS;
	if(sda != MCP4725_NO_PIN && scl != MCP4725_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("mcp4725: failed to set up pins");
			return status;
		}
	}

	uint16_t v = (uint16_t)(value / mVoltage * 4095.0f);

	buf[0] = 0x40;  // Config(write DAC register)
	buf[1] = ((v >> 4) & 0xFF);
	buf[2] = ((v & 0xF) << 4); // LSB bits
	if((status = dhi2c_write(mAddress, buf, 3, 1)) != DHI2C_OK) {
		dhdebug("mcp4725: failed to write");
		return status;
	}
	return DHI2C_OK;
}

void ICACHE_FLASH_ATTR mcp4725_set_address(int address) {
	mAddress = address;
}

DHI2C_STATUS ICACHE_FLASH_ATTR mcp4725_set_vref(float voltage) {
	if(mVoltage <= 0.0f)
		return DHI2C_WRONG_PARAMETERS;
	mVoltage = voltage;
	return DHI2C_OK;
}
