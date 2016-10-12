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
	t.uint = *(uint32_t *)(((uint32_t)rostr / 4) << 2);
	return t.chars[(uint32_t)rostr % 4];
}
