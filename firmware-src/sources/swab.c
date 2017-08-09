/** @file
 * @brief Helper module to work with big-endian/little-endian numbers.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Sergey Polichnoy <pilatuz@gmail.com>
 */
#include "swab.h"


/**
 * @brief Swap byte-order of 32-bits value.
 * @param[in] x Input value: `0xAABBCCDD`.
 * @return Output value: `0xDDCCBBAA`.
 */
uint32_t ICACHE_FLASH_ATTR swab_u32(uint32_t x)
{
	return (((x>>24)&0xFF) << 0)
	     | (((x>>16)&0xFF) << 8)
	     | (((x>>8)&0xFF) << 16)
	     | (((x>>0)&0xFF) << 24);
}


/**
 * @brief Swap byte-order of 16-bits value.
 * @param[in] x Input value: `0xAABB`.
 * @return Output value: `0xBBAA`.
 */
uint16_t ICACHE_FLASH_ATTR swab_u16(uint16_t x)
{
	return (((x>>8)&0xFF) << 0)
	     | (((x>>0)&0xFF) << 8);
}
