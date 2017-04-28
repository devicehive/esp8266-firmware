/**
 * @file
 * @brief DS18B20 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/ds18b20_cmd.h"
#include "commands/onewire_cmd.h"
#include "devices/ds18b20.h"
#include "DH/onewire.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>


/*
 * dh_handle_devices_ds18b20_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_ds18b20_read(COMMAND_RESULT *cmd_res, const char *command,
                                                      const char *params, unsigned int params_len)
{
	if (params_len) {
		gpio_command_params info;
		ALLOWED_FIELDS fields = 0;
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0, AF_PIN, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
		if (dh_onewire_init_helper(cmd_res, fields, &info))
			return; // FAILED
	}

	float temperature;
	const char *err_msg = ds18b20_read(DH_ONEWIRE_NO_PIN, &temperature);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"temperature\":%f}", temperature);
	}
}
