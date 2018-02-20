/**
 * @file
 * @brief BMP280 and BME280 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/i2c_cmd.h"
#include "DH/i2c.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>
#include "../devices/bmx280.h"
#include "bmx280_cmd.h"

#if defined(DH_COMMANDS_BMX280) && defined(DH_DEVICE_BMX280)

/*
 * Sensor initialization
 */
LOCAL int ICACHE_FLASH_ATTR dh_handle_devices_bmx280_prepare(COMMAND_RESULT *cmd_res, const char *command,
                                                              const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	if (params_len) {
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return 0; // FAILED
		}
		if (fields & AF_ADDRESS)
			bmx280_set_address(info.address);
	}

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return 0; // FAILED
	return 1;
}


/*
 * dh_handle_devices_bmp280_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_bmp280_read(COMMAND_RESULT *cmd_res, const char *command,
                                                     const char *params, unsigned int params_len)
{
	if(!dh_handle_devices_bmx280_prepare(cmd_res, command, params, params_len))
		return;
	float temperature;
	float pressure;
	const int status = bmx280_read(DH_I2C_NO_PIN, DH_I2C_NO_PIN, &pressure, &temperature, 0);
	const char *err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_JSON,
				"{\"temperature\":%f, \"pressure\":%f}", temperature, pressure);
	}
}

/*
 * dh_handle_devices_bme280_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_bme280_read(COMMAND_RESULT *cmd_res, const char *command,
                                                     const char *params, unsigned int params_len)
{
	if(!dh_handle_devices_bmx280_prepare(cmd_res, command, params, params_len))
		return;
	float temperature;
	float pressure;
	float humidity;
	const int status = bmx280_read(DH_I2C_NO_PIN, DH_I2C_NO_PIN, &pressure, &temperature, &humidity);
	const char *err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_JSON,
				"{\"temperature\":%f, \"pressure\":%f, \"humidity\":%f}", temperature, pressure, humidity);
	}
}

#endif /* DH_COMMANDS_BMX280 && DH_DEVICE_BMX280 */
