/**
 * @file
 * @brief MLX90614 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/mlx90614_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/mlx90614.h"
#include "DH/i2c.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>

#if defined(DH_COMMANDS_MLX90614) && defined(DH_DEVICE_MLX90614)

/*
 * dh_handle_devices_mlx90614_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_mlx90614_read(COMMAND_RESULT *cmd_res, const char *command,
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
			return; // FAILED
		}
		if (fields & AF_ADDRESS)
			mlx90614_set_address(info.address);
	}

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	float ambient, object;
	const int status = mlx90614_read(DH_I2C_NO_PIN, DH_I2C_NO_PIN, &ambient, &object);
	const char *err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_STRING,
				"{\"ambient\":%f, \"object\":%f}", ambient, object);
	}
}

#endif /* DH_COMMANDS_MLX90614 && DH_DEVICE_MLX90614 */
