/**
 *	\file		irom.h
 *	\brief		Module for working with irom
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 *	\details	ESP8266 stores read only variables in RAM by default. It's
 *				possible to force compliller to store data in ROM, but reading
 *				from ROM is possible only per 32 bits and should be 4 bytes
 *				aligned. That's why direct access to this memory via system
 *				function causes chip to reboot. This module gives access to
 *				this data in easy way.
 */

#ifndef _IROM_H_
#define _IROM_H_

#include <c_types.h>

/** Store data in ROM memory. Access to this data will be slower, but amount
 * of free RAM will be bigger. Never read such variables directly, except 32 bits.*/
#define RO_DATA const ICACHE_RODATA_ATTR STORE_ATTR static

/** Checks if pointer stored in ROM */
#define ifrom(data) if((void *)data >= 0x40200000)

/**
 *	\brief				Read single char from ROM.
 *	\param[in]	buf		Point to data in ROM.
 *	\return 			Char.
 */
char irom_char(const char *rostr);

/**
 *	\brief				Read multiple bytes from ROM.
 *	\param[out]	buf		Point where to store data in RAM.
  *	\param[in]	addr	Point to data in ROM.
 *	\param[in]	len		Number of bytes to read.
 */
void irom_read(char *buf, const char *addr, unsigned int len);

#endif /* _IROM_H_ */
