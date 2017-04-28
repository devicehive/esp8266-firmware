/**
 * @file
 * @brief PCA9685 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/pca9685_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/pca9685.h"
#include "DH/i2c.h"

#include "dhcommand_parser.h"
#include <user_interface.h>

/*
 * dh_handle_devices_pca9685_control() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_pca9685_control(COMMAND_RESULT *cmd_res, const char *command,
                                                         const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, PCA9685_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_ADDRESS | AF_FLOATVALUES | AF_PERIOD, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}
	if (fields & AF_ADDRESS)
		pca9685_set_address(info.address);

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	const int status = pca9685_control(DH_I2C_NO_PIN, DH_I2C_NO_PIN,
			info.storage.float_values, info.pin_value_readed,
			(fields & AF_PERIOD) ? info.periodus : PCA9685_NO_PERIOD);
	err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		dh_command_done(cmd_res, "");
	}
}
