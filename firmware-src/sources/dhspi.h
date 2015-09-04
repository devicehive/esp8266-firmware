/**
 *	\file		dhspi.h
 *	\brief		Implementation of SPI bus for ESP8266.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHSPI_H_
#define _DHSPI_H_

/** SPI operation mode */
typedef enum {
	SPI_CPOL0CPHA0 = 0,		///< Low clock polarity, front edge
	SPI_CPOL0CPHA1 = 1,		///< Low clock polarity, rear edge
	SPI_CPOL1CPHA0 = 2,		///< High clock polarity, front edge
	SPI_CPOL1CPHA1 = 3,		///< High clock polarity, rear edge
} DHSPI_MODE;

/** Virtual pin for CS. It means that module will not set up CS line automatically. */
#define DHSPI_NOCS (~(uint32_t)0U)

/**
 *	\brief				Set SPI mode
 *	\param[in]	mode	SPI mode, can be use DHSPI_MODE enum.
 *	\return				Non zero value on success, zero on error.
 */
int dhspi_set_mode(unsigned int mode);

/**
 *	\brief				Set CS pim
 *	\param[in]	cs_pin	Pin number. Can be DHSPI_NOCS.
 *	\return				Non zero value on success, zero on error.
 */
int dhspi_set_cs_pin(unsigned int cs_pin);

/**
 *	\brief					Write data to SPI bus.
 *	\details				Pins will be initialized automatically.
 *	\param[in]	buf			Buffer with data.
 *	\param[in]	len			Data length in bytes.
 *	\param[in]	disable_cs	Non zero value means that CS should be disabled after operation, zero value means not.
 */
void dhspi_write(const char *buf, unsigned int len, int disable_cs);

/**
 *	\brief					Read data from I2C bus.
 *	\details				Pins will be initialized automatically.
 *	\param[out]	buf			Buffer with data.
 *	\param[in]	len			Data length in bytes.
 */
void dhspi_read(char *buf, unsigned int len);

#endif /* _DHSPI_H_ */
