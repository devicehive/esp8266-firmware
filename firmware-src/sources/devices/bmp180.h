/**
 * @file
 * @brief Simple communication with BMP180 pressure sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_BMP180_H_
#define _DEVICES_BMP180_H_

/**
 * @brief Measure pressure one time.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] pressure Pointer for storing pressure result measure in Pascals.
 * @param[out] temperature Pointer for storing temperature result measure in degree Celsius. Can be NULL.
 * @return Status value, one of DH_I2C_Status enum.
 */
int bmp180_read(int sda, int scl,
                int *pressure,
                float *temperature);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void bmp180_set_address(int address);

#endif /* _DEVICES_BMP180_H_ */
