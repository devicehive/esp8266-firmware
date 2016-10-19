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

uint32_t ICACHE_FLASH_ATTR dns_add_answer(uint8_t *buf, const uint8_t *name, DNS_TYPE type,
		uint32_t ttl, uint32_t size, const uint8_t *data) {
	uint32_t pos;
	if(name) {
		pos = 1;
		char *sp = buf;
		uint32_t len = 0;
		while(*name) {
			if(pos >= DNS_MAX_DOMAIN_LENGTH)
				return 0;
			if(*name == '.') {
				*sp = len;
				len = 0;
				sp = &buf[pos];
			} else {
				buf[pos] = *name;
				len++;
			}
			pos++;
			name++;
		}
		*sp = len;
		buf[pos++] = 0x05;
		buf[pos++] = 'l';
		buf[pos++] = 'o';
		buf[pos++] = 'c';
		buf[pos++] = 'a';
		buf[pos++] = 'l';
		buf[pos++] = 0x00;
	} else {
		buf[pos++] = 0xC0;
		buf[pos++] = sizeof(DNS_HEADER);
	}
	DNS_ANSWER *resp = (DNS_ANSWER *)&buf[pos];
	resp->type = htobe_16(type);
	resp->class = htobe_16(1); // IN - class
	resp->ttl = htobe_32(ttl);
	resp->size = htobe_16(size);
	pos += sizeof(DNS_ANSWER);
	while(size--) {
		buf[pos] = *data;
		pos++;
		data++;
	}
	return pos;
}

uint32_t dns_cmp_qfdn(const uint8_t *a, const uint8_t *b) {
	do {
		if(*a != *b)
			return 1;
		a++;
		b++;
	} while(*a);
	if ((*a == 0) && (*b == 0))
		return 0;
	return 1;
}
