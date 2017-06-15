/**
 * @file
 * @brief Simple communication with HMC5883L compass sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_HMC5883L_H_
#define _DEVICES_HMC5883L_H_

#include "user_config.h"
#if defined(DH_DEVICE_HMC5883L)

/** Measurements in three dimensions. */
typedef struct {
	float X;	///< @brief X axis.
	float Y;	///< @brief Y axis.
	float Z;	///< @brief Z axis.
} HMC5883L_XYZ;


/** @brief Axis overflow */
#define HMC5883l_OVERFLOWED -1.0e8f


/**
 * @brief Measure compass data.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] compass Compass data in normalized vector. If axis is overflowed during measure, HMC5883l_OVERFLOWED is a value.
 * @return NULL on success, text description on error.
 */
int hmc5883l_read(int sda, int scl, HMC5883L_XYZ *compass);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void hmc5883l_set_address(int address);

#endif /* DH_DEVICE_HMC5883L */
#endif /* _DEVICES_HMC5883L_H_ */
