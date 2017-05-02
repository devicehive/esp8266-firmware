/**
 * @file
 * @brief Simple communication with HD44780 like displays via PCF8574 GPIO extender with I2C bus.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/pcf8574_hd44780.h"
#include "devices/pcf8574.h"
#include "DH/i2c.h"

#include <osapi.h>
#include <ets_forward.h>

#if defined(DH_DEVICE_PCF8574_HD44780)

#define PIN_RS        BIT(0)
#define PIN_RW        BIT(1)
#define PIN_E         BIT(2)
#define PIN_BACKLIGHT BIT(3)
#define PIN_D4        BIT(4)
#define PIN_D5        BIT(5)
#define PIN_D6        BIT(6)
#define PIN_D7        BIT(7)


/**
 * @brief Write a half of byte.
 */
static int ICACHE_FLASH_ATTR hd44780_write_half(int sda, int scl, int half, int is_command)
{
	/*unsigned int pins = ((half & BIT(3)) ? PIN_D7 : 0)
	                  | ((half & BIT(2)) ? PIN_D6 : 0)
	                  | ((half & BIT(1)) ? PIN_D5 : 0)
	                  | ((half & BIT(0)) ? PIN_D4 : 0);*/
	unsigned int pins = half << 4; // NOTE: check D4,D5,D6,D7 position
	pins |= PIN_BACKLIGHT | (is_command ? 0 : PIN_RS);

	// set enable HIGH
	int status;
	if ((status = pcf8574_set(sda, scl, pins | PIN_E)) != DH_I2C_OK)
		return status;
	os_delay_us(1);

	// set enable to LOW
	if ((status = pcf8574_set(sda, scl, pins)) != DH_I2C_OK)
		return status;

	return DH_I2C_OK;
}


/**
 * @brief Write a byte.
 */
static int ICACHE_FLASH_ATTR hd44780_write_byte(int sda, int scl, int byte, int is_command)
{
	int status;
	if ((status = hd44780_write_half(sda, scl, (byte >> 4) & 0x0F, is_command)) != DH_I2C_OK)
		return status;
	if ((status = hd44780_write_half(sda, scl, byte & 0x0F, is_command)) != DH_I2C_OK)
		return status;

	return DH_I2C_OK;
}


/**
 * @brief Set output line.
 */
static int ICACHE_FLASH_ATTR hd44780_set_line(int sda, int scl, unsigned int line)
{
	int status;

	switch (line) {
	case 0:
		if ((status = hd44780_write_byte(sda, scl, 0x80 | 0x14, 1)) != DH_I2C_OK)
			return status;
		break;

	case 1:
		if ((status = hd44780_write_byte(sda, scl, 0x80 | 0x40, 1)) != DH_I2C_OK)
			return status;
		break;

	case 2:
		if ((status = hd44780_write_byte(sda, scl, 0x80 | 0x14, 1)) != DH_I2C_OK)
			return status;
		break;

	case 3:
		if ((status = hd44780_write_byte(sda, scl, 0x80 | 0x54, 1)) != DH_I2C_OK)
			return status;
		break;

	default:
		return DH_I2C_WRONG_PARAMETERS;
	}

	return DH_I2C_OK;
}


/*
 * pcf8574_hd44780_write() implementation.
 */
int ICACHE_FLASH_ATTR pcf8574_hd44780_write(int sda, int scl, const char *text, int len)
{
	// clear enable
	int status;
	if ((status = pcf8574_set(sda, scl, ~((uint8_t)(PIN_E | PIN_RW)))) != DH_I2C_OK)
		return status;

	// initialization
	int i;
	for (i = 0; i < 3; i++) {
		if ((status = hd44780_write_half(sda, scl, 0b0011, 1)) != DH_I2C_OK)
			return status;
		os_delay_us(5000);
	}
	if ((status = hd44780_write_half(sda, scl, 0b0010, 1)) != DH_I2C_OK)
		return status;
	os_delay_us(100);

	// configure
	const static uint8_t init_data[] = {
		0b00101100, // function set
		0b00001100, // display on
		0b00000001, // cursor clear
		0b00000110  // entry mode set
	};
	for (i = 0; i < sizeof(init_data); i++) {
		if ((status = hd44780_write_byte(sda, scl, init_data[i], 1)) != DH_I2C_OK)
			return status;
		os_delay_us(2000);
	}

	int line = 0;
	int ch = 0;
	// write text to display RAM
	for (i = 0; i < len; i++) {
		int nla = (text[i] == '\n');
		int nlc = (i+1 < len) ? (text[i] == '\\' && text[i+1] == 'n') : 0;
		if (ch == 20 || nla || nlc) {
			line++;
			if (ch == 20 && (nla || nlc))
				line++;
			if (line > 3)
				break;
			if ((status = hd44780_set_line(sda, scl, line)) != DH_I2C_OK)
				return status;
			ch = 0;
			if (nlc)
				i++;
			if (nla || nlc)
				continue;
		}
		if ((status = hd44780_write_byte(sda, scl, text[i], 0)) != DH_I2C_OK)
			return status;
		ch++;
	}

	return DH_I2C_OK;
}

#endif /* DH_DEVICE_PCF8574_HD44780 */
