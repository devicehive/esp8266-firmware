/*
 * dhutils.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: utils for firmware
 *
 */
#include "dhutils.h"

#include <osapi.h>
#include <ets_forward.h>

int ICACHE_FLASH_ATTR strToFloat(const char *ptr, float *result) {
	float res = 0.0f;
	float sign = 1.0f;
	float fract = 1.0f;
	char found = 0;
	int pos = 0;
	while (1) {
		if(ptr[pos] == '+') {
			sign = 1.0f;
		} else if(ptr[pos] == '-') {
			sign = -1.0f;
		} else if(ptr[pos] == '.' || ptr[pos] == ',') {
			fract = 0.1f;
		} else if(ptr[pos] >= '0' && ptr[pos] <= '9') {
			const unsigned char v = ptr[pos] - 0x30;
			if(fract == 1.0L) {
				res *= 10.0f;
				res += v;
			} else {
				res += ((float) v) * fract;
				fract = fract / 10.0f;
			}
			found = 1;
		} else {
			if(found) {
				*result = res * sign;
				return pos;
			}
			return 0;
		}
		pos++;
	}
}

int ICACHE_FLASH_ATTR strToUInt(const char *ptr, unsigned int *result) {
	#define MAX_INT (~(unsigned int)0)
	unsigned char d;
	unsigned int res = 0;
	unsigned int pos = 0;
	while ( (d = ptr[pos] - 0x30) < 10) {
		pos++;
		if((unsigned int)res < MAX_INT/10 || ((unsigned int)res == MAX_INT/10 && d <= MAX_INT%10))
			res = res*10 + d;
		else
			return 0;
	}
	if(pos)
		*result = res;
	return pos;
}

int ICACHE_FLASH_ATTR strToInt(const char *ptr, int *result) {
	unsigned char d;
	int res = 0;
	int sign = 1;
	unsigned int pos = 0;
	if(ptr[pos] == '-') {
		sign = -1;
		pos++;
	}
	while ( (d = ptr[pos] - 0x30) < 10) {
		pos++;
		res = res*10 + d;
		if(res < 0)
			return 0;
	}
	if(sign < 0 && pos == 1)
		pos = 0;
	if(pos)
		*result = res * sign;
	return pos;
}

/*
 * byteToHex() implementation.
 */
int ICACHE_FLASH_ATTR byteToHex(uint8_t val, char *hex_out)
{
	const int b0 = (val >> 4) & 0x0F; // high nibble
	const int b1 =  val       & 0x0F; // low nibble

	// WARNING: there is no (hex_out != 0) check!
	hex_out[0] = (b0 < 10) ? (b0 + '0') : (b0 - 10 + 'A');
	hex_out[1] = (b1 < 10) ? (b1 + '0') : (b1 - 10 + 'A');

	return 2;
}


/**
 * @brief Convert hexadecimal character into nibble.
 *
 * [Nibble](https://en.wikipedia.org/wiki/Nibble) is a 4-bits value.
 *
 * - ['A'..'Z'] are converted to [10..15]
 * - ['a'..'z'] are converted to [10..15]
 * - ['0'..'9'] are converted to [0..9]
 *
 * @param[in] ch Character to convert.
 * @return Decimal value in range [0..15] or `-1` in case of error.
 */
static inline int hex2nibble(int ch)
{
	if ('a' <= ch && ch <= 'f')
		return ch - 'a' + 10;
	if ('A' <= ch && ch <= 'F')
		return ch - 'A' + 10;
	if ('0' <= ch && ch <= '9')
		return (ch - '0');

	return -1; // unknown character
}


/*
 * hexToByte() implementation.
 */
int ICACHE_FLASH_ATTR hexToByte(const char *hex, uint8_t *val_out)
{
	// high nibble
	const int b0 = hex2nibble(hex[0]);
	if (b0 < 0)
		return 0;

	// low nibble
	const int b1 = hex2nibble(hex[1]);
	if (b1 < 0) {
		*val_out = b0; // WARNING: no (val_out != 0) check
		return 1;
	}

	// WARNING: no (val_out != 0) check
	*val_out = (b0<<4) | b1;
	return 2;
}


const char *ICACHE_FLASH_ATTR find_http_responce_code(const char *data, unsigned short len) {
	unsigned short pos = sizeof(uint32);
	if(len > sizeof(uint32) && *(uint32 *) data == 0x50545448) { // HTTP
		while (pos < len)
			if(data[pos++] == ' ')
				break;
		return &data[pos];
	}
	return NULL;
}

void ICACHE_FLASH_ATTR delay_ms(unsigned int ms) {
	while( ms >= 65) {
		os_delay_us(65000);
		ms -= 65;
	}
	if(ms)
		os_delay_us(ms * 1000);
}
