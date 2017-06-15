/**
 * @file
 * @brief ADC command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/adc_cmd.h"
#include "DH/adc.h"

#ifdef DH_COMMANDS_ADC // ADC command handlers
#include "dhcommand_parser.h"
#include <user_interface.h>

/**
 * Minimum notification timeout, milliseconds.
 */
#define MIN_TIMEOUT_MS 250


/**
 * Maximum notification timeout, milliseconds.
 */
#define MAX_TIMEOUT_MS 0x7fffff


/*
 * dh_handle_adc_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_adc_read(COMMAND_RESULT *cmd_res, const char *command,
                                          const char *params, unsigned int params_len)
{
	if (params_len) {
		gpio_command_params info;
		ALLOWED_FIELDS fields = 0;
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0, AF_READ, &fields);

		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}

		if (info.pins_to_read != DH_ADC_SUITABLE_PINS) {
			dh_command_fail(cmd_res, "Unknown ADC channel");
			return; // FAILED
		}
	}

	// OK, report ADC value
	cmd_res->callback(cmd_res->data, DHSTATUS_OK,
	                  RDT_FLOAT, dh_adc_get_value());
}


/*
 * dh_handle_adc_int() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_adc_int(COMMAND_RESULT *cmd_res, const char *command,
                                         const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0, AF_VALUES, &fields);
	// TODO: use AF_TIMEOUT here?

	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	} else if (info.pin_value_readed != DH_ADC_SUITABLE_PINS) {
		dh_command_fail(cmd_res, "Unknown ADC channel");
		return; // FAILED
	}

	const unsigned int timeout = info.storage.uint_values[0];
	if ((timeout != 0 && timeout < MIN_TIMEOUT_MS) || timeout > MAX_TIMEOUT_MS) {
		dh_command_fail(cmd_res, "Wrong period");
	} else {
		dh_adc_loop(timeout);
		dh_command_done(cmd_res, "");
	}
}

#endif /* DH_COMMANDS_ADC */
