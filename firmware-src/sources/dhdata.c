/*
 * dhdata.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Data coder/encoder
 *
 */
#include "dhdata.h"
#include "user_config.h"
#include "dhutils.h"
#include "base64.h"

#if defined DATAENCODEBASE64 && defined DATAENCODEHEX
#error Only one data encode method are allowed
#elif !defined DATAENCODEBASE64  && !defined DATAENCODEHEX
#error No data encode method specified. Please define DATAENCODEBASE64 or DATAENCODEHEX
#endif


int ICACHE_FLASH_ATTR dhdata_encode(const char *data, unsigned int datalen, char *out, unsigned int outlen) {
#ifdef DATAENCODEBASE64
	return esp_base64_encode(data, datalen, out, outlen);
#else
	if(datalen*2 > outlen || datalen == 0)
		return 0;
	unsigned int datapos = 0;
	unsigned int outpos = 0;
	while(datalen--) {
		outpos += byteToHex(data[datapos], (char *)&out[outpos]);
		datapos++;
	}
	return outpos;
#endif // DATAENCODEBASE64
}

int ICACHE_FLASH_ATTR dhdata_decode(const char *data, unsigned int datalen, char *out, unsigned int outlen) {
#ifdef DATAENCODEBASE64
	return esp_base64_decode(data, datalen, out, outlen);
#else
	if(datalen % 2 || outlen < datalen / 2 || datalen == 0)
		return 0;
	char c;
	unsigned int datapos = 0;
	unsigned int outpos = 0;
	while(datalen) {
		if(hexToByte(&data[datapos], &c) != 2)
			return 0;
		datapos += 2;
		out[outpos++] = c;
		datalen -= 2;
	}
	return outpos;
#endif // DATAENCODEBASE64
}
