/**
 * @file
 * @brief Simple communication with BMP280 and BME280 sensors.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_BMX280_H_
#define _DEVICES_BMX280_H_

#include "user_config.h"
#if defined(DH_DEVICE_BMX280)

/**
 * @brief Measure pressure one time.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] pressure Pointer for storing pressure result measure in Pascals.
 * @param[out] temperature Pointer for storing temperature result measure in degree Celsius. Can be NULL.
 * @param[out] humidity Pointer for storing humidity result measure in %RH. Can be NULL.
 * @return Status value, one of DH_I2C_Status enum.
 */
int bmx280_read(int sda, int scl,
                float *pressure,
                float *temperature,
                float *humidity);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void bmx280_set_address(int address);

#endif /* DH_DEVICE_BMX280 */
#endif /* _DEVICES_BMX280_H_ */
