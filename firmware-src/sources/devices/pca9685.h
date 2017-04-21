/**
 *	\file		pca9685.h
 *	\brief		Simple communication with PCA9685 PWM LED controller
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_PCA9685_H_
#define SOURCES_DEVICES_PCA9685_H_

#include "DH/i2c.h"

/** Default i2c address*/
#define PCA9685_DEFAULT_ADDRESS 0x80
/** Do not initialize pin */
#define PCA9685_NO_PIN -1
/** Do not change frequency */
#define PCA9685_NO_PERIOD 0
/** Pins which are allowed for this chip */
#define PCA9685_SUITABLE_PINS 0xFFFF

/**
 *	\brief					Measure pressure one time.
 *	\param[in]	sda			Pin for I2C's SDA. Can be PCA9685_NO_PIN to use current.
 *	\param[in]	scl			Pin for I2C's SCL. Can be PCA9685_NO_PIN to use current.
 *	\param[in]	pinsduty	Array with pins duty cycles for each pin.
 *	\param[in]	pinsmask	Bitwise pins mask with pins that should be enabled, this pins have to have correct value in pinsduty array.
 *	\param[in]	periodus	PWM period. Can be PCA9685_NO_PERIOD to use current.
 *	\return 				Status value, one of DH_I2C_Status enum.
 */
DH_I2C_Status pca9685_control(int sda, int scl, float *pinsduty, unsigned int pinsmask, unsigned int periodus);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void pca9685_set_address(int address);

#endif /* SOURCES_DEVICES_PCA9685_H_ */
