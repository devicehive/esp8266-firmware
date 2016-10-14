/**
 *	\file		mpu6050.h
 *	\brief		Simple communication with MPU6050 accelerometer and gyroscope sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_MPU6050_H_
#define SOURCES_DEVICES_MPU6050_H_

#include "dhi2c.h"

/** Default sensor i2c address*/
#define MPU6050_DEFAULT_ADDRESS 0xD0
/** Do not initialize pin */
#define MPU6050_NO_PIN -1

/** Struct for results in three dimensions */
typedef struct {
	float X;	///< X axis
	float Y;	///< Y axis
	float Z;	///< Z axis
} MPU6050_XYZ;

/**
 *	\brief						Measure accelerometer and gyroscope data.
 *	\param[in]	sda				Pin for I2C's SDA.
 *	\param[in]	scl				Pin for I2C's SCL.
 *	\param[out]	acceleromter	Accelerometer data in metre per second squared. Can be NULL.
 *	\param[out]	gyroscope		Gyroscope data in degree per second. Can be NULL.
 *	\param[out]	temparature		Gyroscope data in degree per second. Can be NULL.
 *	\return 					NULL on success, text description on error.
 */
DHI2C_STATUS mpu6050_read(int sda, int scl, MPU6050_XYZ *acceleromter, MPU6050_XYZ *gyroscope, float *temparature);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void mpu6050_set_address(int address);

#endif /* SOURCES_DEVICES_MPU6050_H_ */
