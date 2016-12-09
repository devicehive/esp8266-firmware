/**
 *	\file		ina219.h
 *	\brief		Simple communication with INA219 power monitor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_INA219_H_
#define SOURCES_DEVICES_INA219_H_

#include "dhi2c.h"

/** Default sensor i2c address*/
#define INA219_DEFAULT_ADDRESS 0x80
/** Do not initialize pin */
#define INA219_NO_PIN -1

/**
 *	\brief					Set DAC voltages.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	voltage		Input voltage in Volts.
 *	\param[out]	current		Current in Amperes.
 *	\param[out]	power		Total power since first call.
 *	\return 				Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS ina219_read(int sda, int scl, float *voltage, float *current, float *power);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void ina219_set_address(int address);

/**
 *	\brief					Set reference shunt resistance.
 *	\note					Default is 0.1 Ohm.
 *	\param[in]	resistance	Resistance in Ohms.
 *	\return 				Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS ina219_set_shunt(float resistance);

#endif /* SOURCES_DEVICES_INA219_H_ */
