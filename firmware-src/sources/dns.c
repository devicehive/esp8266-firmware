/*
 * mdnsd.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "dns.h"
#include "swab.h"

const uint8_t LOCAL_DOMAIN[] = "local";

LOCAL uint32_t to_fqdn(uint8_t *buf, const uint8_t *d) {
	uint32_t pos;
	pos = 1;
	uint8_t *sp = buf;
	uint32_t len = 0;
	while(*d) {
		if(*d == '.') {
			*sp = len;
			len = 0;
			sp = &buf[pos];
		} else {
			buf[pos] = *d;
			if(len >= DNS_MAX_OCTET_LEN)
				break;
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
	uint8_t i;
	pos--;
	buf[pos++] = sizeof(LOCAL_DOMAIN) - 1;
	for(i = 0; i < sizeof(LOCAL_DOMAIN) - 1; i++)
		buf[pos++] = LOCAL_DOMAIN[i];
	buf[pos++] = 0x00;
	return pos;
}

uint32_t ICACHE_FLASH_ATTR dns_add_answer(uint8_t *buf, const uint8_t *name1,
		const uint8_t *name2, DNS_TYPE type, uint32_t ttl, uint32_t size1,
		const uint8_t *data1, const uint8_t *data2, const uint8_t *data3) {
	uint32_t pos = 0;
	if(name1 == NULL && name2 == NULL) {
		buf[pos++] = 0xC0;
		buf[pos++] = sizeof(DNS_HEADER);
	} else {
		if(name1)
			pos = to_fqdn(&buf[pos], name1);
		if(name2) {
			if(name1)
				pos--;
			pos += to_fqdn_local(&buf[pos], name2);
		}
	}
	DNS_ANSWER *resp = (DNS_ANSWER *)&buf[pos];
	resp->type = htobe_u16(type);
	resp->class = htobe_u16(1); // IN - class
	resp->ttl = htobe_u32(ttl);
	// resp->size see below
	pos += sizeof(DNS_ANSWER);
	uint32_t datapos = pos;
	if(data1) {
		while(size1--) {
			buf[pos] = *data1;
			pos++;
			data1++;
		}
	}
	if(data2)
		pos += to_fqdn(&buf[pos], data2);
	if(data3) {
		if(data2)
			pos--;
		pos += to_fqdn_local(&buf[pos], data3);
	}
	resp->size = htobe_u16(pos - datapos);
	return pos;
}

LOCAL uint8_t inline to_lower(uint8_t c) {
	if(c > 'Z')
		return c;
	if(c < 'A')
		return c;
	return (c | 0x20);
}

int ICACHE_FLASH_ATTR dns_cmp_fqdn_str(const uint8_t *fqdn,
		const uint8_t *str1, const uint8_t *str2) {
	uint8_t dl = *fqdn++;
	const uint8_t *str1begin = str1;
	if(dl == 0)
		return 0;
	while(*fqdn && *str1) {
		if(to_lower(*fqdn) != to_lower(*str1))
			return 0;
		dl--;
		fqdn++;
		str1++;
		if(dl == 0) {
			if(*str1 == 0 || (str1 - str1begin) >= DNS_MAX_OCTET_LEN) {
				if(str2) {
					str1 = str2;
					str2 = NULL;
					dl = *fqdn;
					fqdn++;
					continue;
				} else {
					if(*fqdn == sizeof(LOCAL_DOMAIN) - 1) {
						fqdn++;
						for(dl = 0; dl < sizeof(LOCAL_DOMAIN) - 1; dl++) {
							if(fqdn[dl] != LOCAL_DOMAIN[dl])
								return 0;
						}
						return 1;
					}
					return 0;
				}
			} else if(*str1 != '.') {
				return 0;
			}
			dl = *fqdn;
			fqdn++;
			str1++;
		}
	}
	return 0;
}
