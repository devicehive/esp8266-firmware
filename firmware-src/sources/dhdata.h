/**
 *	\file		dhdata.h
 *	\brief		Encode binary data to text.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef DHDATA_H_
#define DHDATA_H_

/**
 *	\brief				Encode binary data to text.
 *	\details			Function check if output buffer enough for text and start writing to it only this check. Return zero immediately otherwise.
 *	\param[in]	data	Pointer to binary data.
 *	\param[in]	datalen	Data size in bytes.
 *	\param[out]	out		Pointer to output buffer.
 *	\param[in]	outlen	Output buffer size in bytes.
 *	\return 			Number of chars that was copied to output buffer.
 */
int dhdata_encode(const char *data, unsigned int datalen, char *out, unsigned int outlen);

/**
 *	\brief				Decode text to binary data.
 *	\details			Function check if output buffer enough for data and start writing to it only this check. Return zero immediately otherwise.
 *	\param[in]	data	Pointer to encoded text.
 *	\param[in]	datalen	Encoded text size in bytes.
 *	\param[out]	out		Pointer to output buffer.
 *	\param[in]	outlen	Output buffer size in bytes.
 *	\return 			Number of bytes that was copied to output buffer.
 */
int dhdata_decode(const char *data, unsigned int datalen, char *out, unsigned int outlen);

#endif /* DHDATA_H_ */
