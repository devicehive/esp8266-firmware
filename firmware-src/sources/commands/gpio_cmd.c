/**
 * @file
 * @brief GPIO command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/gpio_cmd.h"
#include "DH/gpio.h"

#ifdef DH_COMMANDS_GPIO // GPIO command handlers
#include "dhcommand_parser.h"
#include <user_interface.h>

/**
 * Minimum notification timeout, milliseconds.
 */
#define MIN_TIMEOUT_MS 50


/**
 * Maximum notification timeout, milliseconds.
 */
#define MAX_TIMEOUT_MS 0x7fffff


/*
 * dh_handle_gpio_write() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_gpio_write(COMMAND_RESULT *cmd_res, const char *command,
                                            const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_GPIO_SUITABLE_PINS,
			0, AF_SET | AF_CLEAR, &fields);

	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else if (!(fields & (AF_SET | AF_CLEAR))) {
		dh_command_fail(cmd_res, "Dummy request");
	} else if (!!dh_gpio_write(info.pins_to_set, info.pins_to_clear)) {
		dh_command_fail(cmd_res, "Unsuitable pin");
	} else {
		dh_command_done(cmd_res, "");
	}
}


/*
 * dh_handle_gpio_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_gpio_read(COMMAND_RESULT *cmd_res, const char *command,
                                           const char *params, unsigned int params_len)
{
	if (params_len) {
		gpio_command_params info;
		ALLOWED_FIELDS fields = 0;
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_GPIO_SUITABLE_PINS, 0,
				AF_INIT | AF_PULLUP | AF_NOPULLUP, &fields);

		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}

		// initialize GPIO as input
		if (!!dh_gpio_input(info.pins_to_init,
		                    info.pins_to_pullup,
		                    info.pins_to_nopull)) {
			dh_command_fail(cmd_res, "Wrong initialization parameters");
			return; // FAILED
		}
	}

	// OK, read GPIO input
	cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_GPIO,
	                  0, dh_gpio_read(), system_get_time(),
	                  DH_GPIO_SUITABLE_PINS);
}


/*
 * dh_handle_gpio_int() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_gpio_int(COMMAND_RESULT *cmd_res, const char *command,
                                          const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_GPIO_SUITABLE_PINS, dh_gpio_get_timeout(),
			AF_DISABLE | AF_RISING | AF_FALLING | AF_BOTH | AF_TIMEOUT, &fields);

	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else if (fields == 0) {
		dh_command_fail(cmd_res, "Wrong action");
	} else if (info.timeout < MIN_TIMEOUT_MS || info.timeout > MAX_TIMEOUT_MS) {
		dh_command_fail(cmd_res, "Timeout out of range");
	} else if (!!dh_gpio_subscribe_int(info.pins_to_disable,
	                                   info.pins_to_rising,
	                                   info.pins_to_falling,
	                                   info.pins_to_both,
	                                   info.timeout)) {
		dh_command_fail(cmd_res, "Unsuitable pin");
	} else {
		dh_command_done(cmd_res, "");
	}
}

#endif /* DH_COMMANDS_GPIO */
