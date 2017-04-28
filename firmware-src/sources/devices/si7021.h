/**
 * @file
 * @brief Simple communication with SI7021 relative humidity and temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_SI7021_H_
#define _DEVICES_SI7021_H_

/**
 * @brief Measure temperature and relative humidity one time.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] humidity Pointer for storing relative humidity result measure in percents.
 * @param[out] temperature Pointer for storing temperature result measure in degree Celsius. Can be NULL.
 * @return Status value, one of DH_I2C_Status enum.
 */
int si7021_read(int sda, int scl, float *humidity, float *temperature);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void si7021_set_address(int address);

#endif /* _DEVICES_SI7021_H_ */
