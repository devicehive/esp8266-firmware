/**
 *	\file		crc32.h
 *	\brief		CRC32 calculation module
 *	\author		Gary S. Brown
 *	\date		1986
 *	\copyright	Public Domain
 */

#ifndef _CRC32_H_
#define _CRC32_H_
#include <c_types.h>

/**
 *	\brief				Calculate CRC32.
 *	\param[in]	buf		Point to data.
 *	\param[in]	size	Data size in bytes.
 *	\return 			Calculated CRC32 value.
 */
uint32_t crc32(const void *buf, size_t size);

#endif /* _CRC32_H_ */
