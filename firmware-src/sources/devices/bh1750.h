/**
 * @file
 * @brief Simple communication with BH1750 illuminance sensor
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_BH1750_H_
#define _DEVICES_BH1750_H_

#include "user_config.h"
#if defined(DH_DEVICE_BH1750)

/**
 * @brief Measure illuminance one time.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] illuminance Illuminance in lux.
 * @return Status value, one of DH_I2C_Status enum.
 */
int bh1750_read(int sda, int scl, float illuminance[1]);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void bh1750_set_address(int address);

#endif /* DH_DEVICE_BH1750 */
#endif /* _DEVICES_BH1750_H_ */
