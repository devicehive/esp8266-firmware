/*
 * mdnsd.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */


#include "dns.h"
#include <c_types.h>

uint32_t ICACHE_FLASH_ATTR htobe_32(uint32_t n) {
	uint32_t res;
	uint8_t *p = (uint8_t *)&res;
	p[3] = n & 0xFF;
	n >>= 8;
	p[2] = n & 0xFF;
	n >>= 8;
	p[1] = n & 0xFF;
	n >>= 8;
	p[0] = n & 0xFF;
	return res;
}

uint16_t ICACHE_FLASH_ATTR htobe_16(uint16_t n) {
	uint16_t res;
	uint8_t *p = (uint8_t *)&res;
	p[1] = n & 0xFF;
	n >>= 8;
	p[0] = n & 0xFF;
	return res;
}

uint16_t ICACHE_FLASH_ATTR betoh_16(uint16_t n) {
	uint16_t res;
	uint8_t *p = (uint8_t *)&n;
	res = p[0];
	res <<= 8;
	res |= p[1];
	return res;
}
