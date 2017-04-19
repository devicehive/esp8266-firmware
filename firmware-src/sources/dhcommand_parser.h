/**
 *	\file		dhcommand_parser.h
 *	\brief		JSON command parameters parser.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHCOMMAND_PARSER_H_
#define _DHCOMMAND_PARSER_H_

#include <c_types.h>

#include "DH/gpio.h"
#include "user_config.h"

/** Structure with parsing result */
typedef struct {
	uint16_t pins_to_set;							///< Bitwise mask with pin marked as 1.
	uint16_t pins_to_clear;							///< Bitwise mask with pin marked as 0.
	uint16_t pins_to_init;							///< Bitwise mask with pin marked as inti.
	uint16_t pins_to_pullup;						///< Bitwise mask with pin marked as pullup.
	uint16_t pins_to_nopull;						///< Bitwise mask with pin marked as nopull.
	uint16_t pins_to_disable;						///< Bitwise mask with pin marked as disabled.
	uint16_t pins_to_rising;						///< Bitwise mask with pin marked as rising.
	uint16_t pins_to_falling;						///< Bitwise mask with pin marked as falling.
	uint16_t pins_to_both;							///< Bitwise mask with pin marked as both.
	uint16_t pins_to_read;							///< Bitwise mask with pin marked as read.
	uint16_t pins_to_presence;						///< Bitwise mask with pin marked as presence.
	uint32_t periodus;								///< converted frequency field value.
	uint32_t count;									///< count field value.
	union {
		uint32_t uint_values[DH_GPIO_PIN_COUNT];///< Pin values.
		float float_values[DH_GPIO_PIN_COUNT];	///< Pin values.
		struct {
			uint8_t key_data[(DH_GPIO_PIN_COUNT-1) * 4];///< Key for authentication.
			uint8_t key_len;						///< Key length.
		} key;
	} storage;
	uint16_t pin_value_readed;						///< Bitwise mask with read pin values.
	uint32_t uart_speed;							///< Speed for UART from mode field.
	uint8_t	uart_bits;								///< Bits per byte for UART from mode field.
	char uart_partity;								///< Parity for UART from mode field.
	uint8_t uart_stopbits;							///< Stop bits for UART from mode field.
	char data[INTERFACES_BUF_SIZE];					///< Decoded data.
	uint32_t data_len;								///< Decoded data length.
	uint32_t timeout;								///< timeout field value.
	uint32_t address;								///< address field value.
	uint32_t SDA;									///< SDA field value.
	uint32_t SCL;									///< SCL field value.
	uint32_t spi_mode;								///< Mode for SPI.
	uint32_t CS;									///< CS field value.
	uint32_t pin;									///< pin field value
	float ref;									///< ref field value
} gpio_command_params;

/** Flag based enum with available parameters fields. */
typedef enum {
	AF_SET = 0x01,			///< Read pins with 1 value.
	AF_CLEAR = 0x02,		///< Read pins with 0 value.
	AF_INIT = 0x04,			///< Read pins with init value.
	AF_PULLUP = 0x08,		///< Read pins with pullup value.
	AF_NOPULLUP = 0x10,		///< Read pins with nopullup value.
	AF_DISABLE = 0x20,		///< Read pins with disable value.
	AF_RISING = 0x40,		///< Read pins with rising value.
	AF_FALLING = 0x80,		///< Read pins with falling value.
	AF_BOTH = 0x100,		///< Read pins with both value.
	AF_READ = 0x200,		///< Read pins with read value.
	AF_PRESENCE = 0x400,	///< Read pins with presence value.
	AF_PERIOD = 0x800,		///< Read frequency field.
	AF_COUNT = 0x1000,		///< Read count field.
	AF_VALUES = 0x2000,		///< Read pins values.
	AF_FLOATVALUES = 0x4000,///< Read pins values.
	AF_UARTMODE = 0x8000,	///< Read mode field for UART.
	AF_DATA = 0x10000,		///< Read data field.
	AF_TEXT_DATA = 0x20000,	///< Read text field and store as data.
	AF_TIMEOUT = 0x40000,	///< Read timeout field.
	AF_ADDRESS = 0x80000,	///< Read address field.
	AF_SDA = 0x100000,		///< Read SDA field.
	AF_SCL = 0x200000,		///< Read SCL field.
	AF_SPIMODE = 0x400000,	///< Read mode field for SPI.
	AF_CS = 0x800000,		///< Read CS field.
	AF_PIN = 0x1000000,		///< Read pin field.
	AF_REF = 0x2000000,		///< Read ref field.
	AF_KEY = 0x4000000,		///< Read key field.
} ALLOWED_FIELDS;

/**
 *	\brief						Handle remote command.
 *	\param[in]	params			Pointer to JSON text.
 *	\param[in]	paramslen		Length of JSON in bytes.
 *	\param[out]	out				Pointer to gpio_command_params that will be filled with data.
 *	\param[in]	all				Bitwise pins mask for 'all' value.
 *	\param[in]	timeout			Timeout default value, if it wasn't specified in JSON.
 *	\param[in]	fields			Fields flags that can be read from parameters.
 *	\param[out]	readedfields	Pointer to variable where read parameter flags will be stored.
 */
char *parse_params_pins_set(const char *params, unsigned int paramslen, gpio_command_params *out, unsigned int all, unsigned int timeout, ALLOWED_FIELDS fields, ALLOWED_FIELDS *readedfields);

#endif /* _DHCOMMAND_PARSER_H_ */
