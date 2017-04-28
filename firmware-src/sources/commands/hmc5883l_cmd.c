/**
 * @file
 * @brief HMC5883L command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/hmc5883l_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/hmc5883l.h"
#include "DH/i2c.h"
#include "DH/adc.h"
#include "snprintf.h"

#include "dhcommand_parser.h"
#include <user_interface.h>

/*
 * dh_handle_devices_hmc5883l_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_hmc5883l_read(COMMAND_RESULT *cmd_res, const char *command,
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
			hmc5883l_set_address(info.address);
	}

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	HMC5883L_XYZ compass;
	const int status = hmc5883l_read(DH_I2C_NO_PIN, DH_I2C_NO_PIN, &compass);
	const char *err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}

	// TODO: support for NaN in snprintf!
	char floatbufx[10] = "NaN";
	char floatbufy[10] = "NaN";
	char floatbufz[10] = "NaN";
	if (compass.X != HMC5883l_OVERFLOWED)
		snprintf(floatbufx, sizeof(floatbufx), "%f", compass.X);
	if (compass.Y != HMC5883l_OVERFLOWED)
		snprintf(floatbufy, sizeof(floatbufy), "%f", compass.Y);
	if (compass.Z != HMC5883l_OVERFLOWED)
		snprintf(floatbufz, sizeof(floatbufz), "%f", compass.Z);
	cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_STRING,
			"{\"magnetometer\":{\"X\":%s, \"Y\":%s, \"Z\":%s}}",
			floatbufx, floatbufy, floatbufz);
}
