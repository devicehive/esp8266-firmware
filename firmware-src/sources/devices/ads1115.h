/**
 * @file
 * @brief Simple communication with ADS1115 ADC.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _SOURCES_DEVICES_ADS1115_H_
#define _SOURCES_DEVICES_ADS1115_H_

/**
 * @brief Get ADC voltages.
 * @param[in] sda Pin for I2C's SDA.
 * @param[in] scl Pin for I2C's SCL.
 * @param[out] values Pointer to four float values to store result in Volts.
 * @return Status value, one of DH_I2C_Status enum.
 */
int ads1115_read(int sda, int scl, float values[4]);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void ads1115_set_address(int address);

#endif /* _SOURCES_DEVICES_ADS1115_H_ */
