/**
 * @file
 * @brief Simple communication with MPU6050 accelerometer and gyroscope sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_MPU6050_H_
#define _DEVICES_MPU6050_H_

/** @brief Measurements in three dimensions. */
typedef struct {
	float X;	///< @brief X axis.
	float Y;	///< @brief Y axis.
	float Z;	///< @brief Z axis.
} MPU6050_XYZ;


/**
 * @brief Measure accelerometer and gyroscope data.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL.  Can be DH_I2C_NO_PIN.
 * @param[out] acceleromter Accelerometer data in metre per second squared. Can be NULL.
 * @param[out] gyroscope Gyroscope data in degree per second. Can be NULL.
 * @param[out] temparature Gyroscope data in degree per second. Can be NULL.
 * @return Status value, one of DH_I2C_Status enum.
 */
int mpu6050_read(int sda, int scl, MPU6050_XYZ *acceleromter, MPU6050_XYZ *gyroscope, float *temparature);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void mpu6050_set_address(int address);

#endif /* _DEVICES_MPU6050_H_ */
