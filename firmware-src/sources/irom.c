/*
 * irom.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Helper module to work with rom.
 *
 */

#include "irom.h"

typedef union {
	uint32_t uint;
	uint8_t chars[4];
} FourChars;

char irom_char(const char *rostr) {
	FourChars t;
	t.uint = *(uint32_t *)((uint32_t)rostr & ~0b11);
	return t.chars[(uint32_t)rostr & 0b11];
}

void ICACHE_FLASH_ATTR irom_read(char *buf, const char *addr, unsigned int len) {
	while(len) {
		if(((uint32_t)addr & 0b11) || len < 4) {
			*buf = irom_char(addr);
			buf++;
			addr++;
			len--;
		} else {
			*((uint32_t *)buf) = *((uint32_t *)addr);
			buf += 4;
			addr += 4;
			len -= 4;
		}
	}
}

int ICACHE_FLASH_ATTR irom_cmp(char *buf, const char *addr, unsigned int len) {
	while(len) {
		if(((uint32_t)addr & 0b11) || len < 4) {
			if(*buf != irom_char(addr))
				return 1;
			buf++;
			addr++;
			len--;
		} else {
			if(*((uint32_t *)buf) != *((uint32_t *)addr))
				return 1;
			buf += 4;
			addr += 4;
			len -= 4;
		}
	}
	return 0;
}
