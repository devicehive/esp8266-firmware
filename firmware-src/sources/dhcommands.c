/*
 * dhcommands.cpp
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for executing server command
 *
 */
#include "dhcommands.h"
#include "dhsender_queue.h"
#include "commands/gpio_cmd.h"
#include "commands/adc_cmd.h"
#include "commands/uart_cmd.h"
#include "commands/spi_cmd.h"
#include "commands/i2c_cmd.h"
#include "commands/pwm_cmd.h"
#include "commands/onewire_cmd.h"
#include "commands/ws2812b_cmd.h"
#include "commands/dht_cmd.h"
#include "DH/adc.h"
#include "dhnotification.h"
#include "snprintf.h"
#include "dhcommand_parser.h"
#include "dhterminal.h"
#include "dhdebug.h"
#include "dhutils.h"
#include "commands/ds18b20_cmd.h"
#include "devices/dht.h"
#include "commands/bmp180_cmd.h"
#include "commands/bmp280_cmd.h"
#include "commands/bh1750_cmd.h"
#include "commands/mpu6050_cmd.h"
#include "commands/hmc5883l_cmd.h"
#include "devices/pcf8574.h"
#include "devices/pcf8574_hd44780.h"
#include "commands/mhz19_cmd.h"
#include "commands/lm75_cmd.h"
#include "commands/si7021_cmd.h"
#include "commands/ads1115_cmd.h"
#include "commands/pcf8591_cmd.h"
#include "commands/mcp4725_cmd.h"
#include "commands/ina219_cmd.h"
#include "devices/mfrc522.h"
#include "commands/pca9685_cmd.h"
#include "commands/mlx90614_cmd.h"
#include "commands/max6675_cmd.h"
#include "commands/max31855_cmd.h"
#include "commands/tm1637_cmd.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <ets_forward.h>

#if 1 // devices commands

/**
 * @brief Do "devices/pcf8574/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_pcf8574_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, PCF8574_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS | AF_PULLUP, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			pcf8574_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(dh_i2c_init_helper(cb, fields, &parse_pins))
		return;
	if(fields & AF_PULLUP) {
		const char *res = dh_i2c_error_string(pcf8574_write(PCF8574_NO_PIN, PCF8574_NO_PIN, parse_pins.pins_to_pullup, 0));
		if (res != 0) {
			dh_command_fail(cb, res);
			return;
		}
	}
	unsigned int pins;
	const char *res = dh_i2c_error_string(pcf8574_read(PCF8574_NO_PIN, PCF8574_NO_PIN, &pins));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_GPIO, 0, pins, system_get_time(), PCF8574_SUITABLE_PINS);
	}
}

/**
 * @brief Do "devices/pcf8574/write" command.
 */
static void ICACHE_FLASH_ATTR do_devices_pcf8574_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, PCF8574_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_ADDRESS | AF_SET | AF_CLEAR, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	} else if( (fields & (AF_SET | AF_CLEAR)) == 0) {
		dh_command_fail(cb, "Dummy request");
		return;
	} else if( (parse_pins.pins_to_set | parse_pins.pins_to_clear | PCF8574_SUITABLE_PINS)
	        != PCF8574_SUITABLE_PINS ) {
		dh_command_fail(cb, "Unsuitable pin");
		return;
	}
	if(fields & AF_ADDRESS)
		pcf8574_set_address(parse_pins.address);
	fields |= AF_ADDRESS;
	if(dh_i2c_init_helper(cb, fields, &parse_pins))
		return;
	const char *res = dh_i2c_error_string(pcf8574_write(PCF8574_NO_PIN, PCF8574_NO_PIN,
	        parse_pins.pins_to_set, parse_pins.pins_to_clear));
	if(res != 0) {
		dh_command_fail(cb, res);
	} else {
		dh_command_done(cb, "");
	}
}

/**
 * @brief Do "devices/pcf8574/hd44780/write" command.
 */
static void ICACHE_FLASH_ATTR do_devices_pcf8574_hd44780_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, PCF8574_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_ADDRESS | AF_DATA | AF_TEXT_DATA, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if((fields & (AF_DATA | AF_TEXT_DATA)) == 0 || parse_pins.data_len == 0) {
		dh_command_fail(cb, "Text not specified");
		return;
	}
	if(fields & AF_ADDRESS)
		pcf8574_set_address(parse_pins.address);
	fields |= AF_ADDRESS;
	if(dh_i2c_init_helper(cb, fields, &parse_pins))
		return;
	const char *res = dh_i2c_error_string(pcf8574_hd44780_write(PCF8574_NO_PIN, PCF8574_NO_PIN,
	        parse_pins.data, parse_pins.data_len));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		dh_command_done(cb, "");
	}
}


/**
 * @brief Do "devices/mfrc522/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_mfrc522_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	if(paramslen) {
		gpio_command_params parse_pins;
		ALLOWED_FIELDS fields = 0;
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_CS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_CS) {
			if(MFRC522_Set_CS(parse_pins.CS) != MFRC522_STATUS_OK) {
				dh_command_fail(cb, "Unsuitable pin");
				return;
			}
		}
	}
	MFRC522_PCD_Init();
	uint8_t bufferATQA[2];
	uint8_t bufferSize = sizeof(bufferATQA);
	MFRC522_StatusCode result = MFRC522_PICC_RequestA(bufferATQA, &bufferSize);
	if(result == MFRC522_STATUS_OK || result == MFRC522_STATUS_COLLISION) {
		MFRC522_Uid *uid = MFRC522_Get_Uid();
		result = MFRC522_PICC_Select(uid, 0);
		if(result == MFRC522_STATUS_OK) {
			char hexbuf[uid->size * 2 + 1];
			unsigned int i;
			for(i = 0; i < uid->size; i++)
			byteToHex(uid->uidByte[i], &hexbuf[i * 2]);
			hexbuf[sizeof(hexbuf) - 1] = 0;
			cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING,
			                "{\"uid\":\"0x%s\", \"type\":\"%s\"}", hexbuf,
			                MFRC522_PICC_GetTypeName(MFRC522_PICC_GetType(uid->sak)));
			MFRC522_PCD_AntennaOff();
			return;
		}
	}
	MFRC522_PICC_HaltA();
	MFRC522_PCD_AntennaOff();
	dh_command_fail(cb, MFRC522_GetStatusCodeName(result));
}

/**
 * @brief Do "devices/mfrc522/mifare/read" and "devices/mfrc522/mifare/write" commands.
 */
static void ICACHE_FLASH_ATTR do_devices_mfrc522_mifare_read_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	const int check = os_strcmp(command, "devices/mfrc522/mifare/read");
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, DH_ADC_SUITABLE_PINS, 0,
			AF_CS | AF_ADDRESS | AF_KEY | (check ? AF_DATA : 0), &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if(fields & AF_CS) {
		if(MFRC522_Set_CS(parse_pins.CS) != MFRC522_STATUS_OK) {
			dh_command_fail(cb, "Unsuitable pin");
			return;
		}
	}
	if((fields & AF_ADDRESS) == 0) {
		dh_command_fail(cb, "Block address not specified");
		return;
	}
	if((fields & AF_KEY) == 0) {
		// default key
		os_memset(parse_pins.storage.key.key_data, 0xFF, MF_KEY_SIZE);
		parse_pins.storage.key.key_len = MF_KEY_SIZE;
	} else if(parse_pins.storage.key.key_len != MF_KEY_SIZE) {
		dh_command_fail(cb, "Wrong key length");
		return;
	}
	if(check) {
		if((fields & AF_DATA) == 0) {
			dh_command_fail(cb, "Data not specified");
			return;
		} else if(parse_pins.data_len != 16) {
			dh_command_fail(cb, "Data length should be 16 bytes");
			return;
		}
	}
	MFRC522_PCD_Init();
	uint8_t bufferATQA[2];
	uint8_t bufferSize = sizeof(bufferATQA);
	MFRC522_StatusCode result = MFRC522_PICC_RequestA(bufferATQA, &bufferSize);
	if(result == MFRC522_STATUS_OK || result == MFRC522_STATUS_COLLISION) {
		MFRC522_Uid *uid = MFRC522_Get_Uid();
		result = MFRC522_PICC_Select(uid, 0);
		MIFARE_Key key;
		os_memcpy(key.keyByte, parse_pins.storage.key.key_data, MF_KEY_SIZE);
		if(result == MFRC522_STATUS_OK) {
			result = MFRC522_PCD_Authenticate(PICC_CMD_MF_AUTH_KEY_A, parse_pins.address, &key, uid);
			if(result == MFRC522_STATUS_OK) {
				uint8_t len = (sizeof(parse_pins.data) > 0xFF) ? 0xFF : sizeof(parse_pins.data);
				if(check)
					result = MFRC522_MIFARE_Write(parse_pins.address, (uint8_t*)parse_pins.data, parse_pins.data_len);
				else
					result = MFRC522_MIFARE_Read(parse_pins.address, (uint8_t*)parse_pins.data, &len);
				if(result == MFRC522_STATUS_OK) {
					parse_pins.count = len;
					if(check)
						dh_command_done(cb, "");
					else
						dh_command_done_buf(cb, parse_pins.data, parse_pins.count);
					MFRC522_PICC_HaltA();
					MFRC522_PCD_StopCrypto1();
					MFRC522_PCD_AntennaOff();
					return;
				}
			}
		}
	}
	MFRC522_PICC_HaltA();
	MFRC522_PCD_StopCrypto1();
	MFRC522_PCD_AntennaOff();
	dh_command_fail(cb, MFRC522_GetStatusCodeName(result));
}

#endif // devices commands

RO_DATA struct {
	const char *name;
	void (*func)(COMMAND_RESULT*, const char*, const char*, unsigned int);
} g_command_table[] =
{
#ifdef DH_COMMANDS_GPIO
	{"gpio/write", dh_handle_gpio_write},
	{"gpio/read", dh_handle_gpio_read},
	{"gpio/int", dh_handle_gpio_int},
#endif /* DH_COMMANDS_GPIO */

#ifdef DH_COMMANDS_ADC
	{"adc/read", dh_handle_adc_read},
	{"adc/int", dh_handle_adc_int},
#endif /* DH_COMMANDS_ADC */

#ifdef DH_COMMANDS_PWM
	{"pwm/control", dh_handle_pwm_control},
#endif /* DH_COMMANDS_PWM */

#ifdef DH_COMMANDS_UART
	{"uart/write", dh_handle_uart_write},
	{"uart/read", dh_handle_uart_read},
	{"uart/int", dh_handle_uart_int},
	{"uart/terminal", dh_handle_uart_terminal},
#endif /* DH_COMMANDS_UART */

#ifdef DH_COMMANDS_I2C
	{ "i2c/master/read", dh_handle_i2c_master_read},
	{ "i2c/master/write", dh_handle_i2c_master_write},
#endif /* DH_COMMANDS_I2C */

#ifdef DH_COMMANDS_SPI
	{ "spi/master/read", dh_handle_spi_master_read},
	{ "spi/master/write", dh_handle_spi_master_write},
#endif /* DH_COMMANDS_SPI */

#ifdef DH_COMMANDS_ONEWIRE
	{ "onewire/master/read", dh_handle_onewire_master_read},
	{ "onewire/master/write", dh_handle_onewire_master_write},
	{ "onewire/master/search", dh_handle_onewire_master_search},
	{ "onewire/master/alarm", dh_handle_onewire_master_search},
	{ "onewire/master/int", dh_handle_onewire_master_int},
	{ "onewire/dht/read", dh_handle_onewire_dht_read},
	{ "onewire/ws2812b/write", dh_handle_onewire_ws2812b_write},
#endif /* DH_COMMANDS_ONEWIRE */

	{ "devices/ds18b20/read", dh_handle_devices_ds18b20_read},
	{ "devices/dht11/read", dh_handle_devices_dht11_read},
	{ "devices/dht22/read", dh_handle_devices_dht22_read},
	{ "devices/bmp180/read", dh_handle_devices_bmp180_read},
	{ "devices/bmp280/read", dh_handle_devices_bmp280_read},
	{ "devices/bh1750/read", dh_handle_devices_bh1750_read},
	{ "devices/mpu6050/read", dh_handle_devices_mpu6050_read},
	{ "devices/hmc5883l/read", dh_handle_devices_hmc5883l_read},
	{ "devices/pcf8574/read", do_devices_pcf8574_read},
	{ "devices/pcf8574/write", do_devices_pcf8574_write},
	{ "devices/pcf8574/hd44780/write", do_devices_pcf8574_hd44780_write},
	{ "devices/mhz19/read", dh_handle_devices_mhz19_read},
	{ "devices/lm75/read", dh_handle_devices_lm75_read},
	{ "devices/si7021/read", dh_handle_devices_si7021_read},
	{ "devices/ads1115/read", dh_handle_devices_ads1115_read},
	{ "devices/pcf8591/read", dh_handle_devices_pcf8591_read},
	{ "devices/pcf8591/write", dh_handle_devices_pcf8591_write},
	{ "devices/mcp4725/write", dh_handle_devices_mcp4725_write},
	{ "devices/ina219/read", dh_handle_devices_ina219_read},
	{ "devices/mfrc522/read", do_devices_mfrc522_read},
	{ "devices/mfrc522/mifare/read", do_devices_mfrc522_mifare_read_write},
	{ "devices/mfrc522/mifare/write", do_devices_mfrc522_mifare_read_write},
	{ "devices/pca9685/control", dh_handle_devices_pca9685_control},
	{ "devices/mlx90614/read", dh_handle_devices_mlx90614_read},
	{ "devices/max6675/read", dh_handle_devices_max6675_read},
	{ "devices/max31855/read", dh_handle_devices_max31855_read},
	{ "devices/tm1637/write", dh_handle_devices_tm1637_write}
};


/**
 * @brief Number of commands in the table.
 */
#define NUM_OF_COMMANDS (sizeof(g_command_table)/sizeof(g_command_table[0]))


void ICACHE_FLASH_ATTR dhcommands_do(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen) {
	int i;

	dhdebug("Got command: %s %d", command, cb->data.id);
	for (i = 0; i < NUM_OF_COMMANDS; ++i) {
		if (0 == os_strcmp(command, g_command_table[i].name)) {
			g_command_table[i].func(cb, command, params, paramslen);
			return; // done
		}
	}

	dh_command_fail(cb, "Unknown command");
}
