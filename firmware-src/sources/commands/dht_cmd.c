/**
 * @file
 * @brief DHT command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/dht_cmd.h"
#include "commands/onewire_cmd.h"
#include "devices/dht.h"
#include "DH/onewire.h"
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

	info.count = dht_read(info.data, sizeof(info.data));
	if (info.count)
		dh_command_done_buf(cmd_res, info.data, info.count);
	else
		dh_command_fail(cmd_res, "No response");
}



/*
 * dh_handle_devices_dht11_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_dht11_read(COMMAND_RESULT *cmd_res, const char *command,
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

	int temperature;
	int humidity;
	const char *err_msg = dht11_read(DH_ONEWIRE_NO_PIN, &humidity, &temperature);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_STRING,
				"{\"temperature\":%d, \"humidity\":%d}", temperature, humidity);
	}
}


/*
 * dh_handle_devices_dht22_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_dht22_read(COMMAND_RESULT *cmd_res, const char *command,
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
	float humidity;
	const char *err_msg = dht22_read(DH_ONEWIRE_NO_PIN, &humidity, &temperature);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_STRING,
				"{\"temperature\":%f, \"humidity\":%f}", temperature, humidity);
	}
}
