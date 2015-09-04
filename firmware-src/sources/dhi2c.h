/**
 *	\file		dhi2c.h
 *	\brief		Software implementation of I2C bus for ESP8266.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHI2C_H_
#define _DHI2C_H_

/** I2C functions return status */
typedef enum {
	DHI2C_OK,					///< Success.
	DHI2C_NOACK,				///< ACK response did not receive on bus.
	DHI2C_WRONG_PARAMETERS,		///< Wrong parameters.
	DHI2C_TIMEOUT				///< Module wasn't able to set desired level on bus during timeout time.
} DHI2C_STATUS;

/**
 *	\brief				Prepare pins for usage with I2C.
 *	\param[in]	sda_pin	Pin number for SDA line.
 *	\param[in]	scl_pin	Pin number for SCL line.
 *	\return				Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS dhi2c_init(unsigned int sda_pin, unsigned int scl_pin);

/**
 *	\brief				Prepare last successfully prepared pins for usage with I2C.
 */
void dhi2c_reinit();

/**
 *	\brief					Write data to I2C bus.
 *	\param[in]	address		Device address.
 *	\param[in]	buf			Buffer with data.
 *	\param[in]	len			Data length in bytes.
 *	\param[in]	sendstop	Send STOP after writing if this parameter has non zero value, do not send if parameter is zero.
 *	\return					Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS dhi2c_write(unsigned int address, const char *buf, unsigned int len, int sendstop);

/**
 *	\brief					Read data from I2C bus.
 *	\param[in]	address		Device address.
 *	\param[out]	buf			Buffer with data.
 *	\param[in]	len			Data length in bytes.
 *	\return					Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS dhi2c_read(unsigned int address, char *buf, unsigned int len);

#endif /* _DHI2C_H_ */
