/**
 * @file
 * @brief MAX6675 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_MAX6675_CMD_H_
#define _COMMANDS_MAX6675_CMD_H_

#include "user_config.h"
#include "dhsender_data.h"

/**
 * @brief Handle "devices/max6675/read" command.
 */
void dh_handle_devices_max6675_read(COMMAND_RESULT *cmd_res, const char *command,
                                    const char *params, unsigned int params_len);

#endif /* _COMMANDS_MAX6675_CMD_H_ */
