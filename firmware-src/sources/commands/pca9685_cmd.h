/**
 * @file
 * @brief PCA9685 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_PCA9685_CMD_H_
#define _COMMANDS_PCA9685_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/pca9685/control" command.
 */
void dh_handle_devices_pca9685_control(COMMAND_RESULT *cmd_res, const char *command,
                                       const char *params, unsigned int params_len);

#endif /* _COMMANDS_PCA9685_CMD_H_ */
