/**
 *	\file		ads1115.h
 *	\brief		Simple communication with ADS1115 ADC
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_ADS1115_H_
#define SOURCES_DEVICES_ADS1115_H_

#include "DH/i2c.h"

/** Default sensor i2c address*/
#define ADS1115_DEFAULT_ADDRESS 0x90
/** Do not initialize pin */
#define ADS1115_NO_PIN -1

/**
 *	\brief					Get ADC voltages.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	values		Pointer to four float values to store result in Volts.
 *	\return 				Status value, one of DH_I2C_Status enum.
 */
DH_I2C_Status ads1115_read(int sda, int scl, float *values);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void ads1115_set_address(int address);

#endif /* SOURCES_DEVICES_ADS1115_H_ */
