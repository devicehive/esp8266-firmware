/**
 *	\file		irom.h
 *	\brief		Module for working with irom
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef _IROM_H_
#define _IROM_H_

#include <c_types.h>

/** Store data in ROM memory. Access to this data will be slower, but amount
 * of free RAM will be bigger. Never read such variables directly.*/
#define RO_DATA const ICACHE_RODATA_ATTR STORE_ATTR static

/** Checks if pointer stored in ROM */
#define ifrom(data) if((void *)data >= 0x40200000)

/**
 *	\brief				Read single char from ROM.
 *	\param[in]	buf		Point to data in ROM.
 *	\return 			Char.
 */
char irom_char(const char *rostr);

#endif /* _IROM_H_ */
