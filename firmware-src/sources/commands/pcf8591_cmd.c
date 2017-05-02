/**
 * @file
 * @brief PCF8591 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/pcf8591_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/pcf8591.h"
#include "DH/i2c.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>

#if defined(DH_COMMANDS_PCF8591) && defined(DH_DEVICE_PCF8591)

/*
 * dh_handle_devices_pcf8591_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_pcf8591_read(COMMAND_RESULT *cmd_res, const char *command,
                                                      const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	if (params_len) {
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS | AF_REF, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
		if (fields & AF_ADDRESS)
			pcf8591_set_address(info.address);

		if (fields & AF_REF) {
			const int status = pcf8591_set_vref(info.ref);
			err_msg = dh_i2c_error_string(status);
			if (err_msg != 0) {
				dh_command_fail(cmd_res, err_msg);
				return; // FAILED
			}
		}
	}

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	float values[4];
	const int status = pcf8591_read(DH_I2C_NO_PIN, DH_I2C_NO_PIN, values);
	const char *err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_STRING,
				"{\"0\":%f, \"1\":%f, \"2\":%f, \"3\":%f}",
				values[0], values[1], values[2], values[3]);
	}
}


/*
 * dh_handle_devices_pcf8591_write() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_pcf8591_write(COMMAND_RESULT *cmd_res, const char *command,
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
		return;
	}
	if (fields & AF_ADDRESS)
		pcf8591_set_address(info.address);

	if (fields & AF_REF) {
		const int status = pcf8591_set_vref(info.ref);
		err_msg = dh_i2c_error_string(status);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
	}

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	const int status = pcf8591_write(DH_I2C_NO_PIN, DH_I2C_NO_PIN, info.storage.float_values[0]);
	err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		dh_command_done(cmd_res, "");
	}
}

#endif /* DH_COMMANDS_PCF8591 && DH_DEVICE_PCF8591 */
