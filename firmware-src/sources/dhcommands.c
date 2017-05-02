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
#include "dhdebug.h"

#include "commands/gpio_cmd.h"
#include "commands/adc_cmd.h"
#include "commands/uart_cmd.h"
#include "commands/spi_cmd.h"
#include "commands/i2c_cmd.h"
#include "commands/pwm_cmd.h"
#include "commands/onewire_cmd.h"
#include "commands/ws2812b_cmd.h"
#include "commands/dht_cmd.h"
#include "commands/ds18b20_cmd.h"
#include "commands/bmp180_cmd.h"
#include "commands/bmp280_cmd.h"
#include "commands/bh1750_cmd.h"
#include "commands/mpu6050_cmd.h"
#include "commands/hmc5883l_cmd.h"
#include "commands/pcf8574_cmd.h"
#include "commands/pcf8574_hd44780_cmd.h"
#include "commands/mhz19_cmd.h"
#include "commands/lm75_cmd.h"
#include "commands/si7021_cmd.h"
#include "commands/ads1115_cmd.h"
#include "commands/pcf8591_cmd.h"
#include "commands/mcp4725_cmd.h"
#include "commands/ina219_cmd.h"
#include "commands/mfrc522_cmd.h"
#include "commands/pca9685_cmd.h"
#include "commands/mlx90614_cmd.h"
#include "commands/max6675_cmd.h"
#include "commands/max31855_cmd.h"
#include "commands/tm1637_cmd.h"

#include <osapi.h>
#include <user_interface.h>
#include <ets_forward.h>

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
	{ "devices/pcf8574/read", dh_handle_devices_pcf8574_read},
	{ "devices/pcf8574/write", dh_handle_devices_pcf8574_write},
	{ "devices/pcf8574/hd44780/write", dh_handle_devices_pcf8574_hd44780_write},
	{ "devices/mhz19/read", dh_handle_devices_mhz19_read},
	{ "devices/lm75/read", dh_handle_devices_lm75_read},
	{ "devices/si7021/read", dh_handle_devices_si7021_read},
	{ "devices/ads1115/read", dh_handle_devices_ads1115_read},
	{ "devices/pcf8591/read", dh_handle_devices_pcf8591_read},
	{ "devices/pcf8591/write", dh_handle_devices_pcf8591_write},
	{ "devices/mcp4725/write", dh_handle_devices_mcp4725_write},
	{ "devices/ina219/read", dh_handle_devices_ina219_read},
	{ "devices/mfrc522/read", dh_handle_devices_mfrc522_read},
	{ "devices/mfrc522/mifare/read", dh_handle_devices_mfrc522_mifare_read_write},
	{ "devices/mfrc522/mifare/write", dh_handle_devices_mfrc522_mifare_read_write},
	{ "devices/pca9685/control", dh_handle_devices_pca9685_control},
	{ "devices/mlx90614/read", dh_handle_devices_mlx90614_read},
	{ "devices/max6675/read", dh_handle_devices_max6675_read},
	{ "devices/max31855/read", dh_handle_devices_max31855_read},
	{ "devices/tm1637/write", dh_handle_devices_tm1637_write},

	{ "-", 0 } // END
};


/**
 * @brief Number of commands in the table.
 */
#define NUM_OF_COMMANDS (sizeof(g_command_table)/sizeof(g_command_table[0]))


void ICACHE_FLASH_ATTR dhcommands_do(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen) {
	int i;

	dhdebug("Got command: %s %d", command, cb->data.id);
	for (i = 0; i < NUM_OF_COMMANDS; ++i) {
		if (0 == os_strcmp(command, g_command_table[i].name) && g_command_table[i].func) {
			g_command_table[i].func(cb, command, params, paramslen);
			return; // done
		}
	}

	dh_command_fail(cb, "Unknown command");
}
