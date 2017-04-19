/*
 * tm1637.c
 *
 * Copyright 2017 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "tm1636.h"
#include "irom.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>
#include <ets_forward.h>

RO_DATA const unsigned char segments_table[] = {
	0b11111100,	// 0
	0b01100000,	// 1
	0b11011010,	// 2
	0b11110010,	// 3
	0b01100110,	// 4
	0b10110110,	// 5
	0b10111110,	// 6
	0b11100000,	// 7
	0b11111110,	// 8
	0b11110110	// 9
};

DHI2C_STATUS ICACHE_FLASH_ATTR tm1636_write(int sda, int scl, const char *text, unsigned int len) {
	int i, j;
	DHI2C_STATUS status;
	char v;
	char buf[6];

	// convert text to 8 segments digits
	os_memset(buf, 0, sizeof(buf));
	for(i = 0, j = 0; i < len; i++) {
		if(j >= sizeof(buf))
			return DHI2C_WRONG_PARAMETERS;
		if(text[i] >= 0x30 && text[i] <= 0x39) {
			v = irom_char(&segments_table[text[i] - 0x30]);
		} else if(text[i] == ' ') {
			v = 0b00000000;
		} else if(text[i] == '-') {
			v = 0b00000010;
		} else {
			return DHI2C_WRONG_PARAMETERS;
		}
		if((i + 1) < len) {
			if((text[i + 1] == ':' || text[i + 1] == '.')) {
				v |= 0b1;
				i++;
			}
		}
		buf[j] = v;
		j++;
	}

	// init pins
	if(sda != TM1636_NO_PIN && scl != TM1636_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("tm1636: failed to set up pins");
			return status;
		}
	}

	// write data to display register command
	status = dhi2c_write(bitwise_reverse_byte(0x40), NULL, 0 , 1);
	if(status != DHI2C_OK) {
		dhdebug("tm1636: failed to init");
		return status;
	}

	// write segments
	status = dhi2c_write(bitwise_reverse_byte(0xC0), buf, sizeof(buf), 1);
	if(status != DHI2C_OK) {
		dhdebug("tm1636: failed to write segments");
		return status;
	}

	// control display, max brightness
	status = dhi2c_read(bitwise_reverse_byte(0x8F), NULL, 0);
	if(status != DHI2C_OK) {
		dhdebug("tm1636: failed to control");
		return status;
	}

	return DHI2C_OK;
}
