/*
 * pcf8574_hd44780.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "pcf8574_hd44780.h"
#include "pcf8574.h"

#define PIN_RS (1 << 0)
#define PIN_RW (1 << 1)
#define PIN_E (1 << 2)
#define PIN_BACKLIGHT (1 << 3)
#define PIN_D4 (1 << 4)
#define PIN_D5 (1 << 5)
#define PIN_D6 (1 << 6)
#define PIN_D7 (1 << 7)

LOCAL DHI2C_STATUS ICACHE_FLASH_ATTR pcf8574_hd44780_write_half(int sda,
		int scl, char half, int is_command) {
	DHI2C_STATUS status;
	char pins = ((half & (1 << 3)) ? PIN_D7 : 0) | ((half & (1 << 2)) ? PIN_D6 : 0)
				| ((half & (1 << 1)) ? PIN_D5 : 0) | ((half & (1 << 0)) ? PIN_D4 : 0);
	pins |= PIN_BACKLIGHT | (is_command ? 0 : PIN_RS);
	// set enable HIGH
	if((status = pcf8574_set(sda, scl, pins | PIN_E)) != DHI2C_OK)
		return status;
	os_delay_us(1);
	// set enable to LOW
	if((status = pcf8574_set(sda, scl, pins)) != DHI2C_OK)
		return status;
	return DHI2C_OK;
}

LOCAL DHI2C_STATUS ICACHE_FLASH_ATTR pcf8574_hd44780_write_byte(int sda,
		int scl, char byte, int is_command) {
	DHI2C_STATUS status;
	if((status = pcf8574_hd44780_write_half(sda, scl, (byte  >> 4) & 0x0F, is_command)) != DHI2C_OK)
		return status;
	if((status = pcf8574_hd44780_write_half(sda, scl, byte & 0x0F, is_command)) != DHI2C_OK)
		return status;
	return DHI2C_OK;
}

LOCAL DHI2C_STATUS ICACHE_FLASH_ATTR pcf8574_hd44780_set_line(int sda,
		int scl, unsigned int line) {
	DHI2C_STATUS status;
	switch (line) {
	case 0:
		if( (status = pcf8574_hd44780_write_byte(sda, scl, 0x80 | 0x14, 1))
				!= DHI2C_OK)
			return status;
		break;
	case 1:
		if( (status = pcf8574_hd44780_write_byte(sda, scl, 0x80 | 0x40, 1))
				!= DHI2C_OK)
			return status;
		break;
	case 2:
		if( (status = pcf8574_hd44780_write_byte(sda, scl, 0x80 | 0x14, 1))
				!= DHI2C_OK)
			return status;
		break;
	case 3:
		if( (status = pcf8574_hd44780_write_byte(sda, scl, 0x80 | 0x54, 1))
				!= DHI2C_OK)
			return status;
		break;
	default:
		break;
	}
	return DHI2C_OK;
}

DHI2C_STATUS ICACHE_FLASH_ATTR pcf8574_hd44780_write(int sda, int scl, const char *text, unsigned int len) {
	DHI2C_STATUS status;
	int i;
	const static char init_data[] = {
		0b00101100, // function set
		0b00001100, // display on
		0b00000001, // cursor clear
		0b00000110  // entry mode set
	};

	// clear enable
	if((status = pcf8574_set(sda, scl, ~((char)(PIN_E | PIN_RW)))) != DHI2C_OK)
		return status;

	// initialization
	for(i = 0; i < 3; i++) {
		if((status = pcf8574_hd44780_write_half(sda, scl, 0b0011, 1)) != DHI2C_OK)
			return status;
		os_delay_us(5000);
	}
	if((status = pcf8574_hd44780_write_half(sda, scl, 0b0010, 1)) != DHI2C_OK)
		return status;
	os_delay_us(100);

	// configure
	for(i = 0; i < sizeof(init_data); i++) {
		if((status = pcf8574_hd44780_write_byte(sda, scl, init_data[i], 1)) != DHI2C_OK)
			return status;
		os_delay_us(2000);
	}

	int line = 0;
	int ch = 0;
	// write text to display RAM
	for(i = 0; i < len; i++) {
		if(ch == 20 || text[i] == '\n') {
			line++;
			if(ch == 20 && text[i] == '\n')
				line++;
			if(line > 3)
				break;
			if((status = pcf8574_hd44780_set_line(sda, scl, line)) != DHI2C_OK)
				return status;
			ch = 0;
			if(text[i] == '\n')
				continue;
		}
		if((status = pcf8574_hd44780_write_byte(sda, scl, text[i], 0)) != DHI2C_OK)
			return status;
		ch++;
	}

	return DHI2C_OK;
}
