/*
 * base64.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Base64 implementation.
 *
 */

#include <c_types.h>
#include "user_config.h"
#include "base64.h"

#ifdef DATAENCODEBASE64

LOCAL signed char ICACHE_FLASH_ATTR reverse_base64_table(char c) {
	if(c >= 'a') {
		if(c > 'z')
			return -1;
		return c - 'a' + 26;
	} else if (c >= 'A') {
		if(c > 'Z')
			return -1;
		return c - 'A';
	} else if(c >= '0') {
		if(c > '9')
			return -1;
		return c - '0' + 52;
	} else if(c == '+') {
		return 62;
	} else if(c == '/') {
		return 63;
	}
	return -1;
}

LOCAL char ICACHE_FLASH_ATTR base64_table(uint32_t c, unsigned int offset) {
	static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	return table[(c >> offset) & 0x3F];
}

unsigned int ICACHE_FLASH_ATTR base64_encode_length(unsigned int datalen) {
	return (datalen + 2) / 3 * 4;
}

int ICACHE_FLASH_ATTR base64_encode(const char *data, unsigned int datalen, char *out, unsigned int outlen) {
	if( base64_encode_length(datalen) > outlen || datalen == 0)
		return 0;
	int datapos = 0;
	int outpos = 0;
	uint32_t pc;
	while(datapos < datalen) {
		pc = data[datapos++] << 16;
		out[outpos++] = base64_table(pc, 18);
		if(datapos < datalen) {
			pc |= (uint32_t)data[datapos++] << 8;
			out[outpos++] = base64_table(pc, 12);
			if(datapos < datalen) {
				pc |= (uint32_t)data[datapos++];
				out[outpos++] = base64_table(pc, 6);
				out[outpos++] = base64_table(pc, 0);
			} else {
				out[outpos++] = base64_table(pc, 6);
				out[outpos++] = '=';
				break;
			}
		} else {
			out[outpos++] = base64_table(pc, 12);
			out[outpos++] = '=';
			out[outpos++] = '=';
			break;
		}
	}
	return outpos;
}

unsigned int ICACHE_FLASH_ATTR base64_decode_length(const char *data, unsigned int datalen) {
	if(datalen % 4 || datalen == 0)
		return 0;
	int len = datalen * 3 / 4;
	if(data[datalen - 1] == '=')
		len--;
	if(data[datalen - 2] == '=')
		len--;
	return len;
}

int ICACHE_FLASH_ATTR base64_decode(const char *data, unsigned int datalen, char *out, unsigned int outlen) {
	int el = base64_decode_length(data, datalen);
	if(outlen < el || el == 0)
		return 0;
	int datapos = 0;
	int outpos = 0;
	signed char t[4];
	uint32_t r;
	while(datapos < datalen) {
		t[0] = reverse_base64_table(data[datapos++]);
		t[1] = reverse_base64_table(data[datapos++]);
		t[2] = reverse_base64_table(data[datapos++]);
		t[3] = reverse_base64_table(data[datapos++]);
		if(t[0] == -1 || t[1] == -1)
			return 0;
		r = t[0] << 18 | (t[1] << 12);
		out[outpos++] = (r >> 16) & 0xFF;
		if(t[2] != -1)
			r |= (t[2] << 6);
		else if(datapos < datalen)
			return 0;
		if(t[3] != -1)
			r |= t[3] ;
		else if(datapos < datalen)
			return 0;
		if(t[2] != -1)
			out[outpos++] = (r >> 8) & 0xFF;
		if(t[3] != -1)
			out[outpos++] = r & 0xFF;
	}
	return outpos;
}

#endif //DATAENCODEBASE64
