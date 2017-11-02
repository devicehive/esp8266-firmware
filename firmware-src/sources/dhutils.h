/**
 *	\file		dhutils.h
 *	\brief		Utils for firmware.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHUTILS_H_
#define _DHUTILS_H_

#include "swab.h"


/** Find maximum value */
#define MAX(x, y) (((x) < (y)) ? (y) : (x))

/** Round value to KiB */
#define ROUND_KB(x) ((((x) + 1023) / 1024) * 1024)

/**
 *	\brief				Convert string to float.
 *	\details			Read string till first non digit character or null terminated char.
 *	\param[in]	ptr		Input string.
 *	\param[out]	result	Point to output value.
 *	\return				Number of character from input that was used to write in output buffer, ie 0 on error.
 */
int strToFloat(const char *ptr, float *result);

/**
 *	\brief				Convert string to unsigned integer.
 *	\details			Read string till first non digit character or null terminated char.
 *	\param[in]	ptr		Input string.
 *	\param[out]	result	Point to output value.
 *	\return				Number of character from input that was used to write in output buffer, ie 0 on error.
 */
int strToUInt(const char *ptr, unsigned int *result);

/**
 *	\brief				Convert string to signed integer.
 *	\details			Read string till first non digit character or null terminated char.
 *	\param[in]	ptr		Input string.
 *	\param[out]	result	Point to output value.
 *	\return 			Number of character from input that was used to write in output buffer, ie 0 on error.
 */
int strToInt(const char *ptr, int *result);


/**
 * @brief Convert byte value to hexadecimal string.
 *
 * Output is in the range `[0..9A..F]`.
 *
 * @param[in] val Value for convert.
 * @param[out] hex_out Pointer to output buffer. Should be at least 2 characters in length.
 * @return Number of characters written. Always 2.
 */
int byteToHex(uint8_t val, char *hex_out);


/**
 * @brief Convert hexadecimal string to byte.
 * @param[in] hex String to convert. Should be at least 2 characters in length.
 * @param[out] val_out Pointer to byte where output result is stored.
 * @return Number of characters that was used from string: 1, 2 or 0 on error.
 */
int hexToByte(const char *hex, uint8_t *val_out);


/**
 *	\brief					Util function to find out response code in HTTP response.
 *	\param[in]	data		Pointer to HTTP response data.
 *	\param[in]	len			HTTP response data length.
 *	\return					Pointer to HTTP response code
 */
const char *find_http_responce_code(const char *data, unsigned short len);


/**
 * @brief Read unsigned 16-bits integer from pointer. Big-endian.
 * @param[in] buf Pointer to data.
 * @param[in] pos Offset in data.
 * @return Result value.
 */
static inline unsigned int unsignedInt16be(const char *buf, int pos) {
	return betoh_u16(*(const uint16_t*)(buf + pos));
}


/**
 * @brief Read signed 16-bits integer from pointer. Big-endian.
 * @param[in] buf Pointer to data.
 * @param[in] pos Offset in data.
 * @return Result value.
 */
static inline int signedInt16be(const char *buf, int pos) {
	return (int16_t)betoh_u16(*(const uint16_t*)(buf + pos));
}


/**
 * @brief Read signed 16-bits integer from pointer in sign-magnitude representation. Big-endian.
 * @param[in] buf Pointer to data.
 * @param[in] pos Offset in data.
 * @return Result value.
 */
static inline int signedInt16be_sm(const char *buf, int pos) {
	int r = unsignedInt16be(buf, pos);
	if(r <= 0x7FFF)
		return r;
	return -(r & 0x7FFF);
}


/**
 * @brief Read unsigned 16-bits integer from pointer. Little-endian.
 * @param[in] buf Pointer to data.
 * @param[in] pos Offset in data.
 * @return Result value.
 */
static inline unsigned int unsignedInt16le(const char *buf, int pos) {
	return letoh_u16(*(const uint16_t*)(buf + pos));
}


/**
 * @brief Read signed 16-bits integer from pointer. Little-endian.
 * @param[in] buf Pointer to data.
 * @param[in] pos Offset in data.
 * @return Result value.
 */
static inline int signedInt16le(const char *buf, int pos) {
	return (int16_t)letoh_u16(*(const uint16_t*)(buf + pos));
}


/**
 * @brief Delay in milliseconds.
 */
void delay_ms(unsigned int ms);

/**
 * @brief Reverse bits in byte.
 * @param[in] v Byte.
 * @return Byte in reverse bit order.
 */
static inline uint8_t bitwise_reverse_byte(uint8_t v)
{
	return ((v * 0x0802LU & 0x22110LU) | (v * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
}

/**
 * @brief Convert char to lower case.
 * @param[in] c Char.
 * @return Char in lower case.
 */
char to_lower(char c);

/**
 * @brief Case insensitive string comparison with specified length.
 * @param[in] s1 First string.
 * @param[in] s2 Second string.
 * @param[in] n Maximum string length to compare.
 * @return Relationship between the strings, zero if equal.
 */
int strncasecmp(const char *s1, const char *s2, int n);

#endif /* _DHUTILS_H_ */
