/*
 * rand.c
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

static unsigned long seed = 0;

int ICACHE_FLASH_ATTR rand() {
	if(seed == 0) {
		seed = system_get_time();
	}
	seed = seed * 1103515245 + 12345;
	return (unsigned int)(seed/65536) % RAND_MAX;
}
