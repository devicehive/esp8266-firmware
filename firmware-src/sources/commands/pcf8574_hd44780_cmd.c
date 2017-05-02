/**
 * @file
 * @brief HD44780 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/pcf8574_hd44780_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/pcf8574_hd44780.h"
#include "devices/pcf8574.h"
#include "DH/i2c.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>


/*
 * dh_handle_devices_pcf8574_hd44780_write() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_pcf8574_hd44780_write(COMMAND_RESULT *cmd_res, const char *command,
                                                               const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, PCF8574_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_ADDRESS | AF_DATA | AF_TEXT_DATA, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}

	if ((fields & (AF_DATA | AF_TEXT_DATA)) == 0 || info.data_len == 0) {
		dh_command_fail(cmd_res, "Text not specified");
		return; // FAILED
	}

	if (fields & AF_ADDRESS)
		pcf8574_set_address(info.address);

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	const int status = pcf8574_hd44780_write(DH_I2C_NO_PIN, DH_I2C_NO_PIN, info.data, info.data_len);
	err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		dh_command_done(cmd_res, "");
	}
}
