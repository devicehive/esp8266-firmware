/**
 * @file
 * @brief SI7021 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_SI7021_CMD_H_
#define _COMMANDS_SI7021_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/si7021/read" command.
 */
void dh_handle_devices_si7021_read(COMMAND_RESULT *cmd_res, const char *command,
                                   const char *params, unsigned int params_len);

#endif /* _COMMANDS_SI7021_CMD_H_ */
