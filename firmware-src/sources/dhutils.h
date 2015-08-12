/**
 *	\file		dhutils.h
 *	\brief		Utils for firmware.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DHUTILS_H_
#define SOURCES_DHUTILS_H_

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
 *	\brief				Convert byte value to two hex chars.
 *	\param[in]	byte	Value for convert.
 *	\param[out]	hexout	Pointer to two chars buffer for output.
 *	\return				Number of written chars. Always 2.
 */
int byteToHex(unsigned char byte, char *hexout);

/**
 *	\brief				Convert string to byte.
 *	\param[in]	hex		String to convert.
 *	\param[out]	byteout	Pointer to one char for output.
 *	\return				Number of characters that was used from string: 1, 2 or 0 on error.
 */
int hexToByte(const char *hex, unsigned char *byteout);

#endif /* SOURCES_DHUTILS_H_ */
