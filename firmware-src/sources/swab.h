/** @file
 * @brief Helper module to work with big-endian/little-endian numbers.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Sergey Polichnoy <pilatuz@gmail.com>
 */
#ifndef _SWAB_H_
#define _SWAB_H_

#include <c_types.h>


/**
 * @brief Swap byte-order of 32-bits value.
 * @param[in] x Input value: `0xAABBCCDD`.
 * @return Output value: `0xDDCCBBAA`.
 */
uint32_t swab_u32(uint32_t x);


/**
 * @brief Swap byte-order of 16-bits value.
 * @param[in] x Input value: `0xAABB`.
 * @return Output value: `0xBBAA`.
 */
uint16_t swab_u16(uint16_t x);


#if 1 // CPU is little-endian

/**
 * @brief Convert host to big-endian (32-bits).
 * @param[in] h Host value.
 * @return Big-endian value.
 */
static inline uint32_t htobe_u32(uint32_t h)
{
	return swab_u32(h);
}


/**
 * @brief Convert big-endian to host (32 bits).
 * @param[in] be Big-endian value.
 * @return Host value.
 */
static inline uint32_t betoh_u32(uint32_t be)
{
	return swab_u32(be);
}


/**
 * @brief Convert host to big-endian (16 bits).
 * @param[in] h Host value.
 * @return Big-endian value.
 */
static inline uint16_t htobe_u16(uint16_t h)
{
	return swab_u16(h);
}


/**
 * @brief Convert big-endian to host (16 bits).
 * @param[in] be Big-endian value.
 * @return Host value.
 */
static inline uint16_t betoh_u16(uint16_t be)
{
	return swab_u16(be);
}



/**
 * @brief Convert host to little-endian (32-bits).
 * @param[in] h Host value.
 * @return Little-endian value.
 */
static inline uint32_t htole_u32(uint32_t h)
{
	return h;
}


/**
 * @brief Convert little-endian to host (32 bits).
 * @param[in] le Little-endian value.
 * @return Host value.
 */
static inline uint32_t letoh_u32(uint32_t le)
{
	return le;
}


/**
 * @brief Convert host to little-endian (16 bits).
 * @param[in] h Host value.
 * @return Little-endian value.
 */
static inline uint16_t htole_u16(uint16_t h)
{
	return h;
}


/**
 * @brief Convert little-endian to host (16 bits).
 * @param[in] le Little-endian value.
 * @return Host value.
 */
static inline uint16_t letoh_u16(uint16_t le)
{
	return le;
}

#else // CPU is big-endian

/**
 * @brief Convert host to big-endian (32-bits).
 * @param[in] h Host value.
 * @return Big-endian value.
 */
static inline uint32_t htobe_u32(uint32_t h)
{
	return h;
}


/**
 * @brief Convert big-endian to host (32 bits).
 * @param[in] be Big-endian value.
 * @return Host value.
 */
static inline uint32_t betoh_u16(uint32_t be)
{
	return be;
}


/**
 * @brief Convert host to big-endian (16 bits).
 * @param[in] h Host value.
 * @return Big-endian value.
 */
static inline uint16_t htobe_u16(uint16_t h)
{
	return h;
}


/**
 * @brief Convert big-endian to host (16 bits).
 * @param[in] be Big-endian value.
 * @return Host value.
 */
static inline uint16_t betoh_u16(uint16_t be)
{
	return be;
}


/**
 * @brief Convert host to little-endian (32-bits).
 * @param[in] h Host value.
 * @return Little-endian value.
 */
static inline uint32_t htole_u32(uint32_t h)
{
	return swab_u32(h);
}


/**
 * @brief Convert little-endian to host (32 bits).
 * @param[in] le Little-endian value.
 * @return Host value.
 */
static inline uint32_t letoh_u32(uint32_t le)
{
	return swab_u32(le);
}


/**
 * @brief Convert host to little-endian (16 bits).
 * @param[in] h Host value.
 * @return Little-endian value.
 */
static inline uint16_t htole_u16(uint16_t h)
{
	return swab_u16(h);
}


/**
 * @brief Convert little-endian to host (16 bits).
 * @param[in] le Little-endian value.
 * @return Host value.
 */
static inline uint16_t letoh_u16(uint16_t le)
{
	return swab_u16(le);
}

#endif // CPU

#endif /* _SWAB_H_ */
