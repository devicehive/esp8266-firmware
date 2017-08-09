/**
 * @file
 * @brief Onewire command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/onewire_cmd.h"
#include "DH/onewire.h"
#include "DH/adc.h"

#ifdef DH_COMMANDS_ONEWIRE // onewire command handlers
#include "dhcommand_parser.h"
#include <user_interface.h>

#include <osapi.h>
#include <ets_forward.h>

/**
 * @brief Initialization helper.
 * @return Non-zero if onewire was initialized. Zero otherwise.
 */
int ICACHE_FLASH_ATTR dh_onewire_init_helper(COMMAND_RESULT *cmd_res, ALLOWED_FIELDS fields,
                                             const gpio_command_params *params)
{
	if (fields & AF_PIN) {
		if (!!dh_onewire_set_pin(params->pin)) {
			dh_command_fail(cmd_res, "Wrong onewire pin");
			return 1; // FAILED
		}
	}

	return 0; // continue
}


/**
 * @brief Handle "onewire/master/read" command.
 */
void ICACHE_FLASH_ATTR dh_handle_onewire_master_read(COMMAND_RESULT *cmd_res, const char *command,
                                                     const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_PIN | AF_DATA | AF_COUNT, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}
	if ((fields & AF_COUNT) == 0 || info.count == 0 || info.count > INTERFACES_BUF_SIZE) {
		dh_command_fail(cmd_res, "Wrong read size");
		return; // FAILED
	}
	if ((fields & AF_DATA) == 0) {
		dh_command_fail(cmd_res, "Command for reading is not specified");
		return;
	}
	if (dh_onewire_init_helper(cmd_res, fields, &info))
		return; // FAILED
	if (!!dh_onewire_write(info.data, info.data_len)) {
		dh_command_fail(cmd_res, "No response");
		return; // FAILED
	}
	dh_onewire_read(info.data, info.count);
	dh_command_done_buf(cmd_res, info.data, info.count);
}


/**
 * @brief Handle "onewire/master/write" commands.
 */
void ICACHE_FLASH_ATTR dh_handle_onewire_master_write(COMMAND_RESULT *cmd_res, const char *command,
                                                      const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_PIN | AF_DATA, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}
	if (dh_onewire_init_helper(cmd_res, fields, &info))
		return; // FAILED
	if (!!dh_onewire_write(info.data, info.data_len)) {
		dh_command_fail(cmd_res, "No response");
		return; // FAILED
	}

	dh_command_done(cmd_res, "");
}


/**
 * Handle "onewire/master/search" or "onewire/master/alarm" commands.
 */
void ICACHE_FLASH_ATTR dh_handle_onewire_master_search(COMMAND_RESULT *cmd_res, const char *command,
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
		if(dh_onewire_init_helper(cmd_res, fields, &info))
			return; // FAILED
	}

	int check = os_strcmp(command, "onewire/master/search");
	size_t data_len = sizeof(info.data);
	if (!!dh_onewire_search(info.data, &data_len, (check == 0) ? 0xF0 : 0xEC, dh_onewire_get_pin()))
		dh_command_fail(cmd_res, "Error during search");
	else
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_SEARCH64, dh_onewire_get_pin(), info.data, data_len);
}


/**
 * @brief Handle "onewire/master/int" command.
 */
void ICACHE_FLASH_ATTR dh_handle_onewire_master_int(COMMAND_RESULT *cmd_res, const char *command,
                                                    const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;

	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_GPIO_SUITABLE_PINS, dh_gpio_get_timeout(),
			AF_DISABLE | AF_PRESENCE, &fields);
	if (err_msg != 0)
		dh_command_fail(cmd_res, err_msg);
	else if (fields == 0)
		dh_command_fail(cmd_res, "Wrong action");
	else if (!!dh_onewire_int(info.pins_to_presence, info.pins_to_disable))
		dh_command_fail(cmd_res, "Unsuitable pin");
	else
		dh_command_done(cmd_res, "");
}

#endif /* DH_COMMANDS_ONEWIRE */
