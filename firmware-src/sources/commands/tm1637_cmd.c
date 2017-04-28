/**
 * @file
 * @brief TM1637 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/tm1637_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/tm1637.h"
#include "DH/i2c.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>

/*
 * dh_handle_devices_tm1637_write() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_tm1637_write(COMMAND_RESULT *cmd_res, const char *command,
                                                      const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_DATA | AF_TEXT_DATA, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}
	if ((fields & (AF_DATA | AF_TEXT_DATA)) == 0 || info.data_len == 0) {
		dh_command_fail(cmd_res, "Text not specified");
		return; // FAILED
	}

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	const int status = tm1636_write(DH_I2C_NO_PIN, DH_I2C_NO_PIN, info.data, info.data_len);
	err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		dh_command_done(cmd_res, "");
	}
}
