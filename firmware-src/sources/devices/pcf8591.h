/**
 *	\file		pcf8591.h
 *	\brief		Simple communication with PCF8591 ADC/DAC
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_PCF8591_H_
#define SOURCES_DEVICES_PCF8591_H_

#include "dhi2c.h"

/** Default sensor i2c address*/
#define PCF8591_DEFAULT_ADDRESS 0x90
/** Do not initialize pin */
#define PCF8591_NO_PIN -1

/**
 *	\brief					Get ADC voltages.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	values		Pointer to four float values to store result in Volts.
 *	\return 				Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS pcf8591_read(int sda, int scl, float *values);

/**
 *	\brief					Set DAC voltages.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	value		Voltage in Volts.
 *	\return 				Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS pcf8591_write(int sda, int scl, float value);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void pcf8591_set_address(int address);

/**
 *	\brief					Set reference voltage in volts.
 *	\note					Default is 3.3V.
 *	\param[in]	voltage		Voltage in Volts.
 *	\return 				Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS pcf8591_set_vref(float voltage);

#endif /* SOURCES_DEVICES_PCF8591_H_ */
