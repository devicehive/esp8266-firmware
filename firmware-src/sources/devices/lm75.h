/**
 *	\file		lm75.h
 *	\brief		Simple communication with LM75 temperature sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_LM75_H_
#define SOURCES_DEVICES_LM75_H_

#include "dhi2c.h"

/** Default sensor i2c address*/
#define LM75_DEFAULT_ADDRESS 0x90
/** Do not initialize pin */
#define LM75_NO_PIN -1

/**
 *	\brief					Measure temperature one time.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	temperature	Pointer for storing temperature result measure in Celsius.
 *	\return 				Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS lm75_read(int sda, int scl, float *temperature);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void lm75_set_address(int address);

#endif /* SOURCES_DEVICES_LM75_H_ */
