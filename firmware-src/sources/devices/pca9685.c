/*
 * pca9685.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "pca9685.h"
#include "dhi2c.h"
#include "dhgpio.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

static int mAddress = PCA9685_DEFAULT_ADDRESS;

DHI2C_STATUS ICACHE_FLASH_ATTR pca9685_control(int sda, int scl, float *pinsduty, unsigned int pinsmask, unsigned int periodus) {
	char buf[5];
	DHI2C_STATUS status;
	unsigned int i;

	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		if(pinsmask & (1 << i)) {
			if(pinsduty[i] > 100.0f || pinsduty[i] < 0.0f)
				return DHI2C_WRONG_PARAMETERS;
		}
	}
	if(periodus != PCA9685_NO_PERIOD && (periodus > 41667 ||  periodus < 655)) // hardware limits
		return DHI2C_WRONG_PARAMETERS;

	if(sda != PCA9685_NO_PIN && scl != PCA9685_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("pca9685: failed to set up pins");
			return status;
		}
	}

	if(periodus != PCA9685_NO_PERIOD) { // check if we need prescaller should be changed
		char pb[2];
		pb[0] = 0xFE; // Prescaler
		pb[1] = ((25 * periodus) / 4096.0f) - 0.5f; // value
		buf[0] =  0xFE; // Prescaler
		if((status = dhi2c_write(mAddress, buf, 1, 0)) != DHI2C_OK) {
			dhdebug("pca9685: failed to choose prescaler");
			return status;
		}
		if((status = dhi2c_read(mAddress, buf, 1)) != DHI2C_OK) {
			dhdebug("pca9685: failed read prescaler");
			return status;
		}
		if(pb[1] != buf[0]) {
			buf[0] = 0x00; // MODE1
			buf[1] = 0x31; // turn off oscillator, prescaler can be changed only then
			if((status = dhi2c_write(mAddress, buf, 2, 1)) != DHI2C_OK) {
				dhdebug("pca9685: failed to shutdown");
				return status;
			}
			if((status = dhi2c_write(mAddress, pb, 2, 1)) != DHI2C_OK) {
				dhdebug("pca9685: failed to set up frequency");
				return status;
			}
		}
	}

	buf[0] = 0x00; // MODE1
	buf[1] = 0x21; // allow auto increment, turn on oscillator
	if((status = dhi2c_write(mAddress, buf, 2, 1)) != DHI2C_OK) {
		dhdebug("pca9685: failed to init");
		return status;
	}

	buf[1] = 0; // LED_OFF_L
	buf[2] = 0; // LED_OFF_H
	unsigned short *v = (unsigned short *)&buf[3]; // LED_ON_HL
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		if(pinsmask & (1 << i)) {
			*v = pinsduty[i] * 40.95f;
			buf[0] = 0x06 + 4 * i;
			if((status = dhi2c_write(mAddress, buf, 5, 1)) != DHI2C_OK) {
				dhdebug("pca9685: failed to write");
				return status;
			}
		}
	}

	return DHI2C_OK;
}

void ICACHE_FLASH_ATTR pca9685_set_address(int address) {
	mAddress = address;
}
