/**
 * @file
 * @brief BMP280 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_BMP280_CMD_H_
#define _COMMANDS_BMP280_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/bmp280/read" command.
 */
void dh_handle_devices_bmp280_read(COMMAND_RESULT *cmd_res, const char *command,
                                   const char *params, unsigned int params_len);

#endif /* _COMMANDS_BMP280_CMD_H_ */
