/**
 * @file
 * @brief MPU6050 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/mpu6050_cmd.h"
#include "commands/i2c_cmd.h"
#include "devices/mpu6050.h"
#include "DH/i2c.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>

/*
 * dh_handle_devices_mpu6050_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_mpu6050_read(COMMAND_RESULT *cmd_res, const char *command,
                                                      const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	if (params_len) {
		const char* err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
		if (fields & AF_ADDRESS)
			mpu6050_set_address(info.address);
	}

	fields |= AF_ADDRESS;
	if (dh_i2c_init_helper(cmd_res, fields, &info))
		return; // FAILED

	MPU6050_XYZ acc;
	MPU6050_XYZ gyro;
	float temperature;
	const int status = mpu6050_read(DH_I2C_NO_PIN, DH_I2C_NO_PIN, &acc, &gyro, &temperature);
	const char *err_msg = dh_i2c_error_string(status);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_STRING,
				"{\"temperature\":%f, \"acceleration\":{\"X\":%f, \"Y\":%f, \"Z\":%f}, \"rotation\":{\"X\":%f, \"Y\":%f, \"Z\":%f}}",
				temperature, acc.X, acc.Y, acc.Z, gyro.X, gyro.Y, gyro.Z);
	}
}
