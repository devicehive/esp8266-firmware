/**
 *	\file		mpu6050.h
 *	\brief		Simple communication with MPU6050 accelerometer and gyroscope sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_MPU6050_H_
#define SOURCES_DEVICES_MPU6050_H_

/** Value for returning on error*/
#define MPU6050_ERROR -274
/** Default sensor i2c address*/
#define MPU6050_DEFAULT_ADDRESS 0xD0
/** Do not initialize pin */
#define MPU6050_NO_PIN -1

typedef struct {
	float X;
	float Y;
	float Z;
} MPU6050_XYZ;

/**
 *	\brief						Measure accelerometer and gyroscope data.
 *	\param[in]	sda				Pin for I2C's SDA.
 *	\param[in]	scl				Pin for I2C's SCL.
 *	\param[out]	acceleromter	Accelerometer data in metre per second squared.
 *	\param[out]	gyroscope		Gyroscope data in degree per second.
 *	\return 					Sensor temperature in Celcius.
 */
float mpu6050_read(int sda, int scl, MPU6050_XYZ *acceleromter, MPU6050_XYZ *gyroscope);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void mpu6050_set_address(int address);

#endif /* SOURCES_DEVICES_MPU6050_H_ */
