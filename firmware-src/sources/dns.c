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
LOCAL uint32_t to_fqdn(uint8_t *buf, const uint8_t *d) {
	uint32_t pos;
	pos = 1;
	char *sp = buf;
	uint32_t len = 0;
	while(*d) {
		if(pos >= DNS_MAX_DOMAIN_LENGTH)
			return 0;
		if(*d == '.') {
			*sp = len;
			len = 0;
			sp = &buf[pos];
		} else {
			buf[pos] = *d;
			len++;
		}
		pos++;
		d++;
	}
	*sp = len;
	buf[pos++] = 0x00;
	return pos;
}

LOCAL uint32_t to_fqdn_local(uint8_t *buf, const uint8_t *d) {
	uint32_t pos = to_fqdn(buf, d);
	pos--;
	buf[pos++] = 0x05;
	buf[pos++] = 'l';
	buf[pos++] = 'o';
	buf[pos++] = 'c';
	buf[pos++] = 'a';
	buf[pos++] = 'l';
	buf[pos++] = 0x00;
	return pos;
}

uint32_t ICACHE_FLASH_ATTR dns_add_answer(uint8_t *buf, const uint8_t *name1,
		const uint8_t *name2, DNS_TYPE type, uint32_t ttl, uint32_t size1,
		const uint8_t *data1, const uint8_t *data2) {
	uint32_t pos;
	if(name1) {
		if(name2) {
			pos = to_fqdn(buf, name1);
			pos--;
			pos += to_fqdn_local(&buf[pos], name2);
		} else {
			pos = to_fqdn_local(buf, name1);
		}

	} else {
		buf[pos++] = 0xC0;
		buf[pos++] = sizeof(DNS_HEADER);
	}
	DNS_ANSWER *resp = (DNS_ANSWER *)&buf[pos];
	resp->type = htobe_16(type);
	resp->class = htobe_16(1); // IN - class
	resp->ttl = htobe_32(ttl);
	// resp->size see below
	pos += sizeof(DNS_ANSWER);
	uint32_t dl;
	if(data1) {
		dl = size1;
		while(dl--) {
			buf[pos] = *data1;
			pos++;
			data1++;
		}
	}
	if(data2 == NULL) {
		resp->size = htobe_16(size1);
		return pos;
	}
	dl = to_fqdn_local(&buf[pos], data2);
	resp->size = htobe_16(dl + size1);
	pos += dl;
	return pos;
}

int ICACHE_FLASH_ATTR dns_cmp_fqdn_str(const uint8_t *fqdn, const uint8_t *str) {
	uint8_t dl = *fqdn++;
	if(dl == 0)
		return 0;
	while(*fqdn && *str) {
		if(*fqdn != *str)
			return 0;
		dl--;
		fqdn++;
		str++;
		if(dl == 0) {
			if(*str == 0) {
				if(*fqdn == 5) {
					if(fqdn[1] == 'l' && fqdn[2] == 'o' &&
							fqdn[3] == 'c' && fqdn[4] == 'a' && fqdn[5] == 'l')
						return 1;
				}
				return 0;
			} else if(*str != '.') {
				return 0;
			}
			dl = *fqdn;
			fqdn++;
			str++;
		}
	}
	return 0;
}
