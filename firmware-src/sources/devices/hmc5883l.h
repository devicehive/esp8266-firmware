/**
 *	\file		hmc5883l.h
 *	\brief		Simple communication with HMC5883L compass sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_HMC5883L_H_
#define SOURCES_DEVICES_HMC5883L_H_

#include "dhi2c.h"

/** Default sensor i2c address*/
#define HMC5883L_DEFAULT_ADDRESS 0x3C
/** Do not initialize pin */
#define HMC5883L_NO_PIN -1
/** Axis overflow */
#define HMC5883l_OVERFLOWED -100000000.0f

/** Struct for results in three dimensions */
typedef struct {
	float X;	///< X axis
	float Y;	///< Y axis
	float Z;	///< Z axis
} HMC5883L_XYZ;

/**
 *	\brief						Measure compass data.
 *	\param[in]	sda				Pin for I2C's SDA.
 *	\param[in]	scl				Pin for I2C's SCL.
 *	\param[out]	compass			Compass data in normalized vector. If axis is overflowed during measure, HMC5883l_OVERFLOWED is a value.
 *	\return 					NULL on success, text description on error.
 */
DHI2C_STATUS hmc5883l_read(int sda, int scl, HMC5883L_XYZ *compass);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void hmc5883l_set_address(int address);

#endif /* SOURCES_DEVICES_HMC5883L_H_ */
