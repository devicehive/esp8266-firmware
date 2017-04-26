/**
 * @file
 * @brief DHT command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/dht_cmd.h"
#include "commands/onewire_cmd.h"
#include "devices/dht.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>


/*
 * dh_handle_onewire_dht_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_onewire_dht_read(COMMAND_RESULT *cmd_res, const char *command,
                                                  const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	if (params_len) {
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0, AF_PIN, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
		if (dh_onewire_init_helper(cmd_res, fields, &info))
			return; // FAILED
	}

	info.count = dhonewire_dht_read(info.data, sizeof(info.data));
	if (info.count)
		dh_command_done_buf(cmd_res, info.data, info.count);
	else
		dh_command_fail(cmd_res, "No response");
}
