/*
 * dhrand.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Typical random implementation
 *
 */

#include <user_interface.h>
#include "rand.h"
#include "snprintf.h"

static unsigned long seed = 0;

int ICACHE_FLASH_ATTR rand() {
	if(seed == 0) {
		seed = system_get_time();
	}
	seed = seed * 1103515245 + 12345;
	return (unsigned int)(seed/65536) % RAND_MAX;
}

unsigned int ICACHE_FLASH_ATTR rand_generate_key(char *buf) {
	const int minlen = 8;
	const int maxlen = 16;
	if(buf == 0)
		return maxlen;
	int num = minlen + rand() % (maxlen - minlen + 1);
	unsigned int bufpos = 0;
	while(num--) {
		char c = 0x21 + rand() % 0x5C; // 0x21 - 0x7C
		// removing unsuitable chars
		if(c == '"')
			c = '}'; // 0x7D
		else if(c == '\\')
			c = '~'; // 0x7E
		buf[bufpos++] =  c;
	}
	buf[bufpos] = 0;
	return bufpos;
}

unsigned int ICACHE_FLASH_ATTR rand_generate_deviceid(char *buf) {
	const char prefix[] = "esp-device-";
	const unsigned int len = 16;
	if(buf == 0)
		return len + sizeof(prefix) - 1;
	unsigned int bufpos = snprintf(buf, sizeof(prefix), prefix);
	int i;
	for(i = 0; i < len; i++) {
		bufpos += snprintf(&buf[bufpos], 2, "%x", rand() % 16);
	}
	return bufpos;
}
