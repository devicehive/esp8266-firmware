/**
 * @file
 * @brief PCF8574 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/pcf8574_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/pcf8574.h"
#include "DH/i2c.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>

#if defined(DH_COMMANDS_PCF8574) && defined(DH_DEVICE_PCF8574)

/*
 * dh_handle_devices_pcf8574_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_pcf8574_read(COMMAND_RESULT *cmd_res, const char *command,
                                                      const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	if (params_len) {
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, PCF8574_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS | AF_PULLUP, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
		if (fields & AF_ADDRESS)
			pcf8574_set_address(info.address);
	}

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	if (fields & AF_PULLUP) {
		const int status = pcf8574_write(DH_I2C_NO_PIN, DH_I2C_NO_PIN, info.pins_to_pullup, 0);
		const char *err_msg = dh_i2c_error_string(status);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
	}

	unsigned int pins;
	const int status = pcf8574_read(DH_I2C_NO_PIN, DH_I2C_NO_PIN, &pins);
	const char *err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_GPIO, 0,
				pins, system_get_time(), PCF8574_SUITABLE_PINS);
	}
}


/*
 * dh_handle_devices_pcf8574_write() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_pcf8574_write(COMMAND_RESULT *cmd_res, const char *command,
                                                       const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, PCF8574_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_ADDRESS | AF_SET | AF_CLEAR, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	} else if (!(fields & (AF_SET | AF_CLEAR))) {
		dh_command_fail(cmd_res, "Dummy request");
		return; // FAILED
	} else if ((info.pins_to_set | info.pins_to_clear) & ~PCF8574_SUITABLE_PINS) {
		dh_command_fail(cmd_res, "Unsuitable pin");
		return;
	}

	if (fields & AF_ADDRESS)
		pcf8574_set_address(info.address);

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	const int status = pcf8574_write(DH_I2C_NO_PIN, DH_I2C_NO_PIN,
			info.pins_to_set, info.pins_to_clear);
	err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		dh_command_done(cmd_res, "");
	}
}

#endif /* DH_COMMANDS_PCF8574 && DH_DEVICE_PCF8574 */
