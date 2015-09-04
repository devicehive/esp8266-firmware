/**
 *	\file		base64.h
 *	\brief		Base64 encode/decode implementation.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	Public Domain
 */

#ifndef _DATAENCODEBASE64_H_
#define _DATAENCODEBASE64_H_

#include "user_config.h"

#ifdef DATAENCODEBASE64

/**
 *	\brief				Encode binary data to text with Base64.
 *	\details			Function check if output buffer enough for text and start writing to it only this check. Return zero immediately otherwise.
 *	\param[in]	data	Pointer to binary data.
 *	\param[in]	datalen	Data size in bytes.
 *	\param[out]	out		Pointer to output buffer.
 *	\param[in]	outlen	Output buffer size in bytes.
 *	\return 			Number of chars that was copied to output buffer.
 */
int base64_encode(const char *data, unsigned int datalen, char *out, unsigned int outlen);

/**
 *	\brief				Report how many bytes will take encoded data.
 *	\param[in]	datalen	Binary data size in bytes.
 *	\return 			Base64 encoded size in bytes.
 */
unsigned int base64_encode_length(unsigned int datalen);

/**
 *	\brief				Decode text with Base64 to binary data.
 *	\details			Function check if output buffer enough for data and start writing to it only this check. Return zero immediately otherwise.
 *	\param[in]	data	Pointer to Base64 encoded text.
 *	\param[in]	datalen	Base64 encoded text size in bytes.
 *	\param[out]	out		Pointer to output buffer.
 *	\param[in]	outlen	Output buffer size in bytes.
 *	\return 			Number of bytes that was copied to output buffer.
 */
int base64_decode(const char *data, unsigned int datalen, char *out, unsigned int outlen);

/**
 *	\brief				Report how many bytes will take decoded data.
 *	\param[in]	data	Pointer to Base64 encoded text.
 *	\param[in]	datalen	Base64 encoded text size in bytes.
 *	\return 			Decoded binary data size in bytes.
 */
unsigned int base64_decode_length(const char *data, unsigned int datalen);

#endif //DATAENCODEBASE64

#endif /* _DATAENCODEBASE64_H_ */
