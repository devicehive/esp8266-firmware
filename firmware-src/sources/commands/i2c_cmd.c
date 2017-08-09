/**
 * @file
 * @brief I2C command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/i2c_cmd.h"
#include "DH/i2c.h"
#include "DH/gpio.h"
#include "DH/adc.h"

#ifdef DH_COMMANDS_I2C // I2C command handlers
#include "dhcommand_parser.h"
#include <user_interface.h>

/*
 * dh_i2c_init_helper() implementation.
 */
int ICACHE_FLASH_ATTR dh_i2c_init_helper(COMMAND_RESULT *cmd_res, ALLOWED_FIELDS fields, const gpio_command_params *params)
{
	if ((fields & AF_ADDRESS) == 0) {
		dh_command_fail(cmd_res, "Address not specified");
		return 1; // FAILED
	}

	int init = ((fields & AF_SDA) != 0) + ((fields & AF_SCL) != 0);
	if (init == 2) {
		const int res = dh_i2c_init(params->SDA, params->SCL);
		const char *err = dh_i2c_error_string(res);
		if (err != 0) {
			dh_command_fail(cmd_res, err);
			return 1; // FAILED
		}
	} else if(init == 1) {
		dh_command_fail(cmd_res, "Only one pin specified");
		return 1; // FAILED
	} else {
		dh_i2c_reinit();
	}

	return 0; // continue
}


/*
 * dh_handle_i2c_master_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_i2c_master_read(COMMAND_RESULT *cmd_res, const char *command,
                                                 const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_DATA | AF_ADDRESS | AF_COUNT, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return;
	}

	if (!(fields & AF_COUNT))
		info.count = 2;
	if (info.count == 0 || info.count > INTERFACES_BUF_SIZE) {
		dh_command_fail(cmd_res, "Wrong read size");
		return;
	}

	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return;

	// write
	if (fields & AF_DATA) {
		int res = dh_i2c_write(info.address, info.data, info.data_len, 0);
		err_msg = dh_i2c_error_string(res);
		if (err_msg) {
			dh_command_fail(cmd_res, err_msg);
			return;
		}
	}

	// read
	int res = dh_i2c_read(info.address, info.data, info.count);
	err_msg = dh_i2c_error_string(res);
	if (err_msg) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		dh_command_done_buf(cmd_res, info.data, info.count);
	}
}


/*
 * dh_handle_i2c_master_write() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_i2c_master_write(COMMAND_RESULT *cmd_res, const char *command,
                                                  const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_DATA | AF_ADDRESS, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return;
	}

	if((fields & AF_DATA) == 0) {
		dh_command_fail(cmd_res, "Data not specified");
		return;
	}

	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return;

	const int res = dh_i2c_write(info.address, info.data, info.data_len, 1);
	err_msg = dh_i2c_error_string(res);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		dh_command_done(cmd_res, "");
	}
}

#endif /* DH_COMMANDS_I2C */
