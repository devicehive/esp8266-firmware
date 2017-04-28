/**
 * @file
 * @brief MLX90614 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_MLX90614_CMD_H_
#define _COMMANDS_MLX90614_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/mlx90614/read" command.
 */

void dh_handle_devices_mlx90614_read(COMMAND_RESULT *cmd_res, const char *command,
                                     const char *params, unsigned int params_len);

#endif /* _COMMANDS_MLX90614_CMD_H_ */
