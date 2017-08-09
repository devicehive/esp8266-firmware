/**
 * @file
 * @brief SI7021 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/si7021_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/si7021.h"
#include "DH/i2c.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>

#if defined(DH_COMMANDS_SI7021) && defined(DH_DEVICE_SI7021)

/*
 * dh_handle_devices_si7021_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_si7021_read(COMMAND_RESULT *cmd_res, const char *command,
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
			si7021_set_address(info.address);
	}

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	float temperature;
	float humidity;
	const int status = si7021_read(DH_I2C_NO_PIN, DH_I2C_NO_PIN, &humidity, &temperature);
	const char *err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_JSON,
				"{\"temperature\":%f, \"humidity\":%f}", temperature, humidity);
	}
}

#endif /* DH_COMMANDS_SI7021 && DH_DEVICE_SI7021 */
