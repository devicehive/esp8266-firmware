/**
 * @file
 * @brief Simple communication with TM1637 LED 8 segment driver.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/tm1637.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"
#include "irom.h"

#include <osapi.h>
#include <ets_forward.h>

#if defined(DH_DEVICE_TM1637)

// segments table for digits
RO_DATA const uint8_t segments_table[] = {
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


/*
 * tm1636_write() implementation.
 */
int ICACHE_FLASH_ATTR tm1636_write(int sda, int scl, const char *text, size_t len)
{
	int i, j;
	char buf[6];

	// convert text to 8 segments digits
	os_memset(buf, 0, sizeof(buf));
	for (i = 0, j = 0; i < len; i++) {
		if (j >= sizeof(buf))
			return DH_I2C_WRONG_PARAMETERS;

		const int ch = text[i];
		int v;
		if (ch >= '0' && ch <= '9') {
			v = irom_char(&segments_table[ch - '0']);
		} else if (ch == ' ') {
			v = 0b00000000;
		} else if (ch == '-') {
			v = 0b00000010;
		} else {
			return DH_I2C_WRONG_PARAMETERS;
		}

		if ((i + 1) < len) {
			if ((text[i+1] == ':' || text[i+1] == '.')) {
				v |= 0b1;
				i++;
			}
		}

		buf[j++] = v;
	}

	// init pins
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("tm1636: failed to set up pins");
			return status;
		}
	}

	// write data to display register command
	status = dh_i2c_write(bitwise_reverse_byte(0x40), NULL, 0 , 1);
	if (status != DH_I2C_OK) {
		dhdebug("tm1636: failed to init");
		return status;
	}

	// write segments
	status = dh_i2c_write(bitwise_reverse_byte(0xC0), buf, sizeof(buf), 1);
	if (status != DH_I2C_OK) {
		dhdebug("tm1636: failed to write segments");
		return status;
	}

	// control display, max brightness
	status = dh_i2c_read(bitwise_reverse_byte(0x8F), NULL, 0);
	if (status != DH_I2C_OK) {
		dhdebug("tm1636: failed to control");
		return status;
	}

	return DH_I2C_OK;
}

#endif /* DH_DEVICE_TM1637 */
