/**
 * @file
 * @brief Software I2C implementation for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "DH/i2c.h"
#include "DH/gpio.h"
#include "DH/adc.h"
#include "user_config.h"

#include <c_types.h>
#include <osapi.h>
#include <gpio.h>
#include <user_interface.h>
#include <ets_forward.h>

#define I2C_DELAY_US         5
#define I2C_ERROR_TIMEOUT_US 50000

// module variables
static DHGpioPinMask mSDAPin = DH_GPIO_PIN(0);
static DHGpioPinMask mSCLPin = DH_GPIO_PIN(2);


/*
 * dh_i2c_error_string() implementation.
 */
const char* ICACHE_FLASH_ATTR dh_i2c_error_string(int status)
{
	switch(status) {
		case DH_I2C_OK:
			return 0;
		case DH_I2C_NOACK:
			return "no ACK response";
		case DH_I2C_WRONG_PARAMETERS:
			return "Wrong parameters";
		case DH_I2C_BUS_BUSY:
			return "Bus is busy";
		case DH_I2C_DEVICE_ERROR:
			return "Device error";
	}

	return 0;
}


/*
 * dh_i2c_init() implementation.
 */
int ICACHE_FLASH_ATTR dh_i2c_init(unsigned int sda_pin, unsigned int scl_pin)
{
	const DHGpioPinMask SDA = DH_GPIO_PIN(sda_pin);
	const DHGpioPinMask SCL = DH_GPIO_PIN(scl_pin);
	if (!(SDA&DH_GPIO_SUITABLE_PINS) || !(SCL&DH_GPIO_SUITABLE_PINS) || SDA == SCL)
		return DH_I2C_WRONG_PARAMETERS;

	mSDAPin = SDA;
	mSCLPin = SCL;
	dh_i2c_reinit();
	return DH_I2C_OK;
}


/*
 * dh_i2c_reinit() implementation.
 */
void ICACHE_FLASH_ATTR dh_i2c_reinit(void)
{
	dh_gpio_open_drain(mSDAPin | mSCLPin, 0);
	dh_gpio_prepare_pins(mSDAPin | mSCLPin, true/*disable_pwm*/);
	dh_gpio_pull_up(mSDAPin | mSCLPin, 0);
	gpio_output_set(mSDAPin | mSCLPin, 0, mSDAPin | mSCLPin, 0);
}


/**
 * @brief Set pin value.
 */
static int ICACHE_FLASH_ATTR i2c_set_pin(DHGpioPinMask pin_mask, int val)
{
	if (val) {
		gpio_output_set(pin_mask, 0, pin_mask, 0);
	} else {
		gpio_output_set(0, pin_mask, pin_mask, 0);
	}
	os_delay_us(I2C_DELAY_US);

	int i; // busy-wait loop
	for(i = 0; i < I2C_ERROR_TIMEOUT_US; i++) {
		const int vv = (gpio_input_get()&pin_mask) != 0;
		if (vv ^ val) {
			// values are still different
			os_delay_us(1); // wait a bit
		} else {
			return DH_I2C_OK;
		}
	}

	return DH_I2C_BUS_BUSY;
}


/**
 * @brief Start I2C communication.
 */
static int ICACHE_FLASH_ATTR i2c_start(void)
{
	int res;

	res = i2c_set_pin(mSDAPin, 1);
	if (res != DH_I2C_OK)
		return res;
	res = i2c_set_pin(mSCLPin, 1);
	if (res != DH_I2C_OK)
		return res;

	res = i2c_set_pin(mSDAPin, 0);
	if (res != DH_I2C_OK)
		return res;
	res = i2c_set_pin(mSCLPin, 0);
	if (res != DH_I2C_OK)
		return res;

	return DH_I2C_OK;
}


/**
 * @brief Stop I2C communication.
 */
static int ICACHE_FLASH_ATTR i2c_stop(void)
{
	int res;

	res = i2c_set_pin(mSDAPin, 0);
	if (res != DH_I2C_OK)
		return res;
	res = i2c_set_pin(mSCLPin, 1);
	if (res != DH_I2C_OK)
		return res;
	res = i2c_set_pin(mSDAPin, 1);
	if (res != DH_I2C_OK)
		return res;

	return DH_I2C_OK;
}


/**
 * @brief Write one byte to I2C bus.
 */
static int ICACHE_FLASH_ATTR i2c_writebyte(int ch)
{
	int res, i;

	// msb-first
	for (i = 7; i >= 0; i--) {
		res = i2c_set_pin(mSDAPin, (ch>>i)&1);
		if (res != DH_I2C_OK)
			return res;
		res = i2c_set_pin(mSCLPin, 1);
		if (res != DH_I2C_OK)
			return res;
		os_delay_us(I2C_DELAY_US);
		res = i2c_set_pin(mSCLPin, 0);
		if (res != DH_I2C_OK)
			return res;
	}

	// check ACK
	gpio_output_set(mSDAPin, 0, mSDAPin, 0);
	os_delay_us(I2C_DELAY_US);
	res = i2c_set_pin(mSCLPin, 1);
	if (res != DH_I2C_OK)
		return res;
	os_delay_us(I2C_DELAY_US);
	const int nack = (gpio_input_get() & mSDAPin) != 0;
	res = i2c_set_pin(mSCLPin, 0);
	if (res != DH_I2C_OK)
		return res;
	if (nack)
		return DH_I2C_NOACK;

	return DH_I2C_OK;
}


/**
 * @brief Read one byte from I2C bus.
 */
static int ICACHE_FLASH_ATTR i2c_readbyte(int *byte, int ack_bit)
{
	int res, i;

	gpio_output_set(mSDAPin, 0, mSDAPin, 0);
	os_delay_us(I2C_DELAY_US);

	// msb-first
	for (i = 7; i >= 0; i--) {
		res = i2c_set_pin(mSCLPin, 1);
		if (res != DH_I2C_OK)
			return res;
		os_delay_us(I2C_DELAY_US);
		const int bit = (gpio_input_get() & mSDAPin) != 0;
		*byte |= bit << i;
		res = i2c_set_pin(mSCLPin, 0);
		if (res != DH_I2C_OK)
			return res;
	}

	// send ACK
	res = i2c_set_pin(mSDAPin, ack_bit);
	if (res != DH_I2C_OK)
		return res;
	res = i2c_set_pin(mSCLPin, 1);
	if (res != DH_I2C_OK)
		return res;
	os_delay_us(I2C_DELAY_US);
	res = i2c_set_pin(mSCLPin, 0);
	if (res != DH_I2C_OK)
		return res;

	return DH_I2C_OK;
}


/**
 * @brief Do I2C bus communication.
 */
static int ICACHE_FLASH_ATTR i2c_act(unsigned int address, uint8_t *buf, size_t len, int send_stop, int is_read)
{
	if (len > INTERFACES_BUF_SIZE || address > 0xFF)
		return DH_I2C_WRONG_PARAMETERS;

	// start
	int res = i2c_start();
	if (res != DH_I2C_OK)
		return res;

	// write address
	res = i2c_writebyte(is_read ? (address |  0x01)
	                            : (address & ~0x01));
	if (res != DH_I2C_OK) {
		i2c_stop();
		return res;
	}

	while (len--) {
		system_soft_wdt_feed();
		if (is_read) {
			int byte = 0;
			res = i2c_readbyte(&byte, 0 == len);
			*buf++ = byte;
		} else {
			res = i2c_writebyte(*buf++);
		}
		if (res != DH_I2C_OK) {
			i2c_stop();
			return res;
		}
	}

	if (send_stop) {
		return i2c_stop();
	} else {
		return DH_I2C_OK;
	}
}


/*
 * dh_i2c_write() implementation.
 */
int ICACHE_FLASH_ATTR dh_i2c_write(unsigned int address, const void *buf, size_t len, int send_stop)
{
	return i2c_act(address, (uint8_t*)buf, len, send_stop, 0);
}


/*
 * dh_i2c_read() implementation.
 */
int ICACHE_FLASH_ATTR dh_i2c_read(unsigned int address, void *buf, size_t len)
{
	return i2c_act(address, (uint8_t*)buf, len, 1/*send_stop*/, 1);
}
