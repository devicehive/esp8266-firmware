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

#include <c_types.h>
#include <osapi.h>
#include "dhutils.h"

int ICACHE_FLASH_ATTR strToFloat(const char *ptr, float *result) {
	float res = 0.0f;
	float sign = 1.0f;
	float fract = 1.0f;
	char found = 0;
	int pos = 0;
	while (1) {
		if (ptr[pos] == '+') {
			sign = 1.0f;
		} else if (ptr[pos] == '-') {
			sign = -1.0f;
		} else if (ptr[pos] == '.' || ptr[pos] == ',') {
			fract = 0.1f;
		} else if (ptr[pos] >= '0' && ptr[pos] <= '9') {
			const unsigned char v = ptr[pos] - 0x30;
			if (fract == 1.0L) {
				res *= 10.0f;
				res += v;
			} else {
				res += ((float) v) * fract;
				fract = fract / 10.0f;
			}
			found = 1;
		} else {
			if (found) {
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

int ICACHE_FLASH_ATTR byteToHex(unsigned char byte, char *hexout) {
	const unsigned char b0 = byte / 0x10;
	const unsigned char b1 = byte & 0xF;
	hexout[0] = (b0 < 10) ? b0 + '0' : (b0 - 10 + 'A');
	hexout[1] = (b1 < 10) ? b1 + '0' : (b1 - 10 + 'A');
	return 2;
}

LOCAL int ICACHE_FLASH_ATTR hexchar(const char c) {
	if (c > 0x60) {
		if(c > 0x66)
			return -1;
		return c - 0x57;
	} else if (c > 0x40) {
		if (c > 0x46)
			return -1;
		return c - 0x37;
	} else if (c > 0x2F) {
		if (c > 0x39)
			return -1;
		return c - 0x30;
	} else
		return -1;
}

int ICACHE_FLASH_ATTR hexToByte(const char *hex, unsigned char *byteout) {
	const int b0 = hexchar(hex[0]);
	if(b0 != -1) {
		const int b1 = hexchar(hex[1]);
		if(b1 == -1) {
			*byteout = b0;
			return 1;
		} else {
			*byteout = b0 * 0x10 + b1;
			return 2;
		}
	}
	return 0;
}

const char *ICACHE_FLASH_ATTR find_http_responce_code(const char *data, unsigned short len) {
	unsigned short pos = sizeof(uint32);
	if (len > sizeof(uint32) && *(uint32 *) data == 0x50545448) { // HTTP
		while (pos < len)
			if (data[pos++] == ' ')
				break;
		return &data[pos];
	}
	return NULL;
}

unsigned int ICACHE_FLASH_ATTR unsignedInt16be(const char *buf, int pos) {
	return ((int)buf[pos] * 0x100 + (int)buf[pos + 1]);
}

int ICACHE_FLASH_ATTR signedInt16be(const char *buf, int pos) {
	int r = unsignedInt16be(buf, pos);
	if (r <= 0x7FFF)
		return r;
	return r - 0x10000;
}

int ICACHE_FLASH_ATTR signedInt16be_sm(const char *buf, int pos) {
	int r = unsignedInt16be(buf, pos);
	if (r <= 0x7FFF)
		return r;
	return -(r & 0x7FFF);
}

unsigned int ICACHE_FLASH_ATTR unsignedInt16le(const char *buf, int pos) {
	return ((int)buf[pos] + (int)buf[pos + 1] * 0x100);
}

int ICACHE_FLASH_ATTR signedInt16le(const char *buf, int pos) {
	int r = unsignedInt16le(buf, pos);
	if (r <= 0x7FFF)
		return r;
	return r - 0x10000;
}

void ICACHE_FLASH_ATTR delay_ms(unsigned int ms) {
	while( ms >= 65) {
		os_delay_us(65000);
		ms -= 65;
	}
	if(ms)
		os_delay_us(ms * 1000);
}
