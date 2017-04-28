/**
 * @file
 * @brief Simple communication with MLX90614 contactless IR temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_MLX90614_H_
#define _DEVICES_MLX90614_H_

/**
 * @brief Measure temperature one time.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] ambient Pointer for storing ambient temperature result measure in Celsius.
 * @param[out] object Pointer for storing object temperature result measure in Celsius.
 * @return Status value, one of DH_I2C_Status enum.
 */
int mlx90614_read(int sda, int scl, float *ambient, float *object);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void mlx90614_set_address(int address);

#endif /* _DEVICES_MLX90614_H_ */
