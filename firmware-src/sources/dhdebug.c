/*
 * dhdebug.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for debug output
 *
 */
#include <stdarg.h>
#include <osapi.h>
#include <c_types.h>
#include <eagle_soc.h>
#include <gpio.h>
#include <os_type.h>
#include "dhdebug.h"
#include "dhterminal.h"
#include "user_config.h"
#include "dhutils.h"

void dhdebug_ram(const char *fmt, ...) {
	va_list    ap;
	va_start(ap, fmt);
	dhterminal_debug(fmt, ap);
	va_end(ap);
}

void ICACHE_FLASH_ATTR dhdebug_dump(const char *data, unsigned int len) {
	if(len == 0)
		return;
	const unsigned char byte_per_line = 16;
	char textbuf[byte_per_line + 1];
	char hexbuf[byte_per_line * 3 + 1];
	char address[9] = "00000000";
	int pos = 0;
	dhdebug("Dump at 0x%X, length %u", (unsigned int)data, len);
	do {
		byteToHex((pos / 0x1000000) & 0xFF, &address[0]);
		byteToHex((pos / 0x10000) & 0xFF, &address[2]);
		byteToHex((pos / 0x100) & 0xFF, &address[4]);
		byteToHex(pos & 0xFF, &address[6]);
		unsigned int i;
		for(i = 0; i < byte_per_line; i++) {
			if(pos < len) {
				char c = data[pos++];
				byteToHex(c, &hexbuf[i * 3]);
				hexbuf[i * 3 + 2] = ' ';
				if(c < 0x20 || c >= 0x7F)
					textbuf[i] = '.';
				else
					textbuf[i] = c;
				textbuf[i + 1] = 0;
			} else {
				hexbuf[i * 3] = ' ';
				hexbuf[i * 3 + 1] = ' ';
				hexbuf[i * 3 + 2] = ' ';
			}
			hexbuf[i * 3 + 3] = 0;
		}
		dhdebug("%s:  %s  %s", address, hexbuf, textbuf);
	} while (pos < len);
}
