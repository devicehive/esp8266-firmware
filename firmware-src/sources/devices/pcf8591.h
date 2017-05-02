/**
 * @file
 * @brief Simple communication with PCF8591 ADC/DAC.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_PCF8591_H_
#define _DEVICES_PCF8591_H_

/**
 * @brief Get ADC voltages.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] values Pointer to four float values to store result in Volts.
 * @return Status value, one of DH_I2C_Status enum.
 */
int pcf8591_read(int sda, int scl, float values[4]);


/**
 * @brief Set DAC voltages.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[in] value Voltage in Volts.
 * @return Status value, one of DH_I2C_Status enum.
 */
int pcf8591_write(int sda, int scl, float value);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void pcf8591_set_address(int address);


/**
 * @brief Set reference voltage in volts.
 *
 * Default is 3.3V.
 *
 * @param[in] voltage Reference voltage in Volts.
 * @return Status value, one of DH_I2C_Status enum.
 */
int pcf8591_set_vref(float voltage);

#endif /* _DEVICES_PCF8591_H_ */
