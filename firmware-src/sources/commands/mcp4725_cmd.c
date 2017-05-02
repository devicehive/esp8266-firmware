/**
 * @file
 * @brief MCP4725 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/mcp4725_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/mcp4725.h"
#include "DH/i2c.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>

#if defined(DH_COMMANDS_MCP4725) && defined(DH_DEVICE_MCP4725)

/*
 * dh_handle_devices_mcp4725_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_mcp4725_write(COMMAND_RESULT *cmd_res, const char *command,
                                                       const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_ADDRESS | AF_REF | AF_FLOATVALUES, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}
	if (info.pin_value_readed != 1) {
		dh_command_fail(cmd_res, "Unsuitable pin");
		return; // FAILED
	}
	if (fields & AF_ADDRESS)
		mcp4725_set_address(info.address);
	if (fields & AF_REF) {
		const int status = mcp4725_set_vref(info.ref);
		err_msg = dh_i2c_error_string(status);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
	}

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	const int status = mcp4725_write(DH_I2C_NO_PIN, DH_I2C_NO_PIN, info.storage.float_values[0]);
	const char *res = dh_i2c_error_string(status);
	if (res != 0) {
		dh_command_fail(cmd_res, res);
	} else {
		dh_command_done(cmd_res, "");
	}
}

#endif /* DH_COMMANDS_MCP4725 && DH_DEVICE_MCP4725 */
