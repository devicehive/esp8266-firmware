/**
 * @file
 * @brief Simple communication with MCP4725 DAC.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_MCP4725_H_
#define _DEVICES_MCP4725_H_

#include "user_config.h"
#if defined(DH_DEVICE_MCP4725)

/**
 * @brief Set DAC voltages.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] value Voltage in Volts.
 * @return Status value, one of DH_I2C_Status enum.
 */
int mcp4725_write(int sda, int scl, float value);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void mcp4725_set_address(int address);


/**
 * @brief Set reference voltage in volts.
 *
 * Default is 3.3V.
 *
 * @param[in] voltage Voltage in Volts.
 * @return Status value, one of DH_I2C_Status enum.
 */
int mcp4725_set_vref(float voltage);

#endif /* DH_DEVICE_MCP4725 */
#endif /* _DEVICES_MCP4725_H_ */
