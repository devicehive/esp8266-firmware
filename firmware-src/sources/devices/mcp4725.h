/**
 *	\file		mcp4725.h
 *	\brief		Simple communication with MCP4725 DAC
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_MCP4725_H_
#define SOURCES_DEVICES_MCP4725_H_

#include "DH/i2c.h"

/** Default sensor i2c address*/
#define MCP4725_DEFAULT_ADDRESS 0xC0
/** Do not initialize pin */
#define MCP4725_NO_PIN -1

/**
 *	\brief					Set DAC voltages.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	value		Voltage in Volts.
 *	\return 				Status value, one of DH_I2C_Status enum.
 */
DH_I2C_Status mcp4725_write(int sda, int scl, float value);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void mcp4725_set_address(int address);

/**
 *	\brief					Set reference voltage in volts.
 *	\note					Default is 3.3V.
 *	\param[in]	voltage		Voltage in Volts.
 *	\return 				Status value, one of DH_I2C_Status enum.
 */
DH_I2C_Status mcp4725_set_vref(float voltage);

#endif /* SOURCES_DEVICES_MCP4725_H_ */
