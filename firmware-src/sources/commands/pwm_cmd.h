/**
 * @file
 * @brief PWM command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_PWM_CMD_H_
#define _COMMANDS_PWM_CMD_H_

#include "user_config.h"
#if defined(DH_COMMANDS_PWM) // PWM command handlers
#include "dhsender_data.h"

/**
 * @brief Handle "pwm/control" command.
 */
void dh_handle_pwm_control(COMMAND_RESULT *cmd_res, const char *command,
                           const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_PWM */
#endif /* _COMMANDS_PWM_CMD_H_ */
