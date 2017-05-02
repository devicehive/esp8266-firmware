/**
 * @file
 * @brief Simple communication with INA219 power monitor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_INA219_H_
#define _DEVICES_INA219_H_

#include "user_config.h"
#if defined(DH_DEVICE_INA219)

/**
 * @brief Get measurements.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] voltage Input voltage in Volts.
 * @param[out] current Current in Amperes.
 * @param[out] power Total power since first call.
 * @return Status value, one of DH_I2C_Status enum.
 */
int ina219_read(int sda, int scl, float *voltage, float *current, float *power);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void ina219_set_address(int address);

/**
 * @brief Set reference shunt resistance.
 *
 * Default is 0.1 Ohm.
 *
 * @param[in] resistance Resistance in Ohms.
 * @return Status value, one of DH_I2C_Status enum.
 */
int ina219_set_shunt(float resistance);

#endif /* DH_DEVICE_INA219 */
#endif /* _DEVICES_INA219_H_ */
