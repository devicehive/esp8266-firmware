/**
 * @file
 * @brief ADS1115 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/ads1115_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/ads1115.h"
#include "DH/i2c.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>


/*
 * dh_handle_devices_ads1115_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_ads1115_read(COMMAND_RESULT *cmd_res, const char *command,
                                                      const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	if (params_len) {
		const char *err_msg = parse_params_pins_set(params, params_len, &info,
				DH_ADC_SUITABLE_PINS, 0, AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
		if (fields & AF_ADDRESS)
			ads1115_set_address(info.address);
	}
	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	float values[4];
	int status = ads1115_read(DH_I2C_NO_PIN, DH_I2C_NO_PIN, values);
	const char *err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_STRING,
				"{\"0\":%f, \"1\":%f, \"2\":%f, \"3\":%f}",
				values[0], values[1], values[2], values[3]);
	}
}
