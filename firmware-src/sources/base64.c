/**
 * @file
 * @brief Base64 encode/decode.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "base64.h"
#include "user_config.h"

#ifdef DATAENCODEBASE64

/**
 * @brief Base64 decoding table.
 * @param[in] ch Input character.
 * @return 6-bits integer or -1 in case of error.
 */
static int ICACHE_FLASH_ATTR reverse_base64_table(int ch)
{
	if (ch >= 'A' && ch <= 'Z')
		return ch - 'A';
	if (ch >= 'a' && ch <= 'z')
		return ch - 'a' + 26;
	if (ch >= '0' && ch <= '9')
		return ch - '0' + 52;
	if (ch == '+')
		return 62;
	if (ch == '/')
		return 63;

	return -1; // bad input
}


/**
 * @brief Base64 encoding table.
 * @param[in] reg Shift register.
 * @param[in] off Offset.
 */
static inline int ICACHE_FLASH_ATTR base64_table(uint32_t reg, int offset)
{
	static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	return table[(reg >> offset) & 0x3F]; // 6-bits
}


/*
 * base64_encode_length() implementation.
 */
size_t ICACHE_FLASH_ATTR base64_encode_length(size_t data_len)
{
	return (data_len + 2)/3 * 4; // round_up(len/3)*4
}


/*
 * base64_encode() implementation.
 */
int ICACHE_FLASH_ATTR base64_encode(const void *data_, size_t data_len,
                                    char *text, size_t text_len)
{
	if (!data_len)
		return 0; // nothing to encode

	// check we have enough space for output
	if (base64_encode_length(data_len) > text_len)
		return 0;

	const uint8_t *data = (const uint8_t*)data_;
	char* const text_base = text;

	while (data_len != 0)
	{
		uint32_t reg = ((uint32_t)*data++) << 16; data_len--;
		*text++ = base64_table(reg, 18);
		if (data_len != 0) {
			reg |= ((uint32_t)*data++) << 8; data_len--;
			*text++ = base64_table(reg, 12);
			if (data_len != 0) {
				reg |= (uint32_t)*data++; data_len--;
				*text++ = base64_table(reg, 6);
				*text++ = base64_table(reg, 0);
			} else {
				*text++ = base64_table(reg, 6);
				*text++ = '='; // padding
				break;
			}
		} else {
			*text++ = base64_table(reg, 12);
			*text++ = '='; // padding
			*text++ = '='; // padding
			break;
		}
	}

	return text - text_base;
}


/*
 * base64_decode_length() implementation.
 */
size_t ICACHE_FLASH_ATTR base64_decode_length(const char *text, size_t text_len)
{
	if (!text_len)
		return 0; // nothing to decode
	if (text_len%4)
		return 0; // bad text

	size_t data_len = (text_len/4) * 3;
	if (text[text_len-1] == '=')
		data_len--;
	if (text[text_len-2] == '=')
		data_len--;

	return data_len;
}


/*
 * base64_decode() implementation.
 */
int ICACHE_FLASH_ATTR base64_decode(const char *text, size_t text_len,
                                    void *data_base, size_t data_len)
{
	if (!text_len || text_len%4)
		return 0; // nothing to decode

	// check we have enough space for output
	if (base64_decode_length(text, text_len) > data_len)
		return 0;

	uint8_t *data = (uint8_t*)data_base;
	while (text_len != 0) {
		const int t0 = reverse_base64_table(*text++); text_len--;
		const int t1 = reverse_base64_table(*text++); text_len--;
		if (t0 < 0 || t1 < 0)
			return 0; // bad data
		uint32_t reg = ((uint32_t)t0 << 18)
		             | ((uint32_t)t1 << 12);
		*data++ = (reg >> 16) & 0xFF;

		const int ch2 = *text++; text_len--;
		if (ch2 == '=')
			break;
		const int t2 = reverse_base64_table(ch2);
		if (t2 < 0)
			return 0; // bad data
		reg |= ((uint32_t)t2 << 6);
		*data++ = (reg >> 8) & 0xFF;

		const int ch3 = *text++; text_len--;
		if (ch3 == '=')
			break;
		const int t3 = reverse_base64_table(ch3);
		if (t3 < 0)
			return 0; // bad data
		reg |= ((uint32_t)t3);
		*data++ = reg & 0xFF;
	}

	return data - (uint8_t*)data_base;
}

#endif // DATAENCODEBASE64
