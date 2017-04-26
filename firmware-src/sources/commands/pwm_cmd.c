/**
 * @file
 * @brief PWM command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/pwm_cmd.h"
#include "DH/pwm.h"
#include "DH/adc.h"

#ifdef DH_COMMANDS_PWM // PWM command handlers
#include "dhcommand_parser.h"
#include <user_interface.h>

/*
 * dh_handle_pwm_control() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_pwm_control(COMMAND_RESULT *cmd_res, const char *command,
                                             const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_VALUES | AF_PERIOD | AF_COUNT, &fields);

	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else if (!!dh_pwm_start(info.storage.uint_values,
	                          info.pin_value_readed,
	                          (fields & AF_PERIOD) ? info.periodus
	                                               : dh_pwm_get_period_us(),
	                          info.count)) {
		dh_command_fail(cmd_res, "Wrong parameters");
	} else {
		dh_command_done(cmd_res, ""); // OK
	}
}

#endif /* DH_COMMANDS_PWM */
