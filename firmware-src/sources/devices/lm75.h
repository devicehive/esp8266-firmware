/**
 * @file
 * @brief Simple communication with LM75 temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_LM75_H_
#define _DEVICES_LM75_H_

#include "user_config.h"
#if defined(DH_DEVICE_LM75)

/**
 * @brief Measure temperature one time.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] temperature Pointer for storing temperature result measure in Celsius.
 * @return Status value, one of DH_I2C_Status enum.
 */
int lm75_read(int sda, int scl, float *temperature);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void lm75_set_address(int address);

#endif /* DH_DEVICE_LM75 */
#endif /* _DEVICES_LM75_H_ */
