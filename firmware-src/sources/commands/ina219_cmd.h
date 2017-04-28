/**
 * @file
 * @brief INA219 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_INA219_CMD_H_
#define _COMMANDS_INA219_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/ina219/read" command.
 */

void dh_handle_devices_ina219_read(COMMAND_RESULT *cmd_res, const char *command,
                                     const char *params, unsigned int params_len);

#endif /* _COMMANDS_INA219_CMD_H_ */
