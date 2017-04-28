/**
 * @file
 * @brief LM75 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_LM75_CMD_H_
#define _COMMANDS_LM75_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/lm75/read" command.
 */
void dh_handle_devices_lm75_read(COMMAND_RESULT *cmd_res, const char *command,
                                 const char *params, unsigned int params_len);

#endif /* _COMMANDS_LM75_CMD_H_ */
