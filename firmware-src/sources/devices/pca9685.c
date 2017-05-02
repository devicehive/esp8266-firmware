/**
 * @file
 * @brief Simple communication with PCA9685 PWM LED controller
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/pca9685.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>

#if defined(DH_DEVICE_PCA9685)

/** @brief Default i2c address*/
#define PCA9685_DEFAULT_ADDRESS 0x80

// module variables
static int mAddress = PCA9685_DEFAULT_ADDRESS;


/*
 * pca9685_control() implementation.
 */
int ICACHE_FLASH_ATTR pca9685_control(int sda, int scl, const float pins_duty[DH_GPIO_PIN_COUNT], DHGpioPinMask pins, unsigned int period_us)
{
	int i;
	for (i = 0; i < DH_GPIO_PIN_COUNT; i++) {
		if (pins & DH_GPIO_PIN(i)) {
			if (pins_duty[i] > 100.0f || pins_duty[i] < 0.0f)
				return DH_I2C_WRONG_PARAMETERS;
		}
	}
	if (period_us != PCA9685_NO_PERIOD && (period_us > 41667 || period_us < 655)) // hardware limits
		return DH_I2C_WRONG_PARAMETERS;

	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("pca9685: failed to set up pins");
			return status;
		}
	}

	uint8_t buf[5];
	if (period_us != PCA9685_NO_PERIOD) { // check if prescaller should be changed
		uint8_t pb[2];
		pb[0] = 0xFE; // Prescaler
		pb[1] = ((25 * period_us) / 4096.0f) - 0.5f; // value // TODO: do we really need float here?
		buf[0] =  0xFE; // Prescaler
		if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
			dhdebug("pca9685: failed to choose prescaler");
			return status;
		}
		if ((status = dh_i2c_read(mAddress, buf, 1)) != DH_I2C_OK) {
			dhdebug("pca9685: failed read prescaler");
			return status;
		}

		if (pb[1] != buf[0]) {
			buf[0] = 0x00; // MODE1
			buf[1] = 0x31; // turn off oscillator, prescaler can be changed only then
			if ((status = dh_i2c_write(mAddress, buf, 2, 1)) != DH_I2C_OK) {
				dhdebug("pca9685: failed to shutdown");
				return status;
			}
			if ((status = dh_i2c_write(mAddress, pb, 2, 1)) != DH_I2C_OK) {
				dhdebug("pca9685: failed to set up frequency");
				return status;
			}
		}
	}

	buf[0] = 0x00; // MODE1
	buf[1] = 0x21; // allow auto increment, turn on oscillator
	if ((status = dh_i2c_write(mAddress, buf, 2, 1)) != DH_I2C_OK) {
		dhdebug("pca9685: failed to init");
		return status;
	}

	buf[1] = 0; // LED_OFF_L
	buf[2] = 0; // LED_OFF_H
	uint16_t *v = (uint16_t *)&buf[3]; // LED_ON_HL
	for (i = 0; i < DH_GPIO_PIN_COUNT; i++) {
		if (pins & DH_GPIO_PIN(i)) {
			*v = pins_duty[i] * 40.95f;
			buf[0] = 0x06 + 4 * i;
			if ((status = dh_i2c_write(mAddress, buf, 5, 1)) != DH_I2C_OK) {
				dhdebug("pca9685: failed to write");
				return status;
			}
		}
	}

	return DH_I2C_OK;
}


/*
 * pca9685_set_address() implementation.
 */
void ICACHE_FLASH_ATTR pca9685_set_address(int address)
{
	mAddress = address;
}

#endif /* DH_DEVICE_PCA9685 */
