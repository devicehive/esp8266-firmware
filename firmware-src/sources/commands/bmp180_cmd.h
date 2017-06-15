/**
 * @file
 * @brief BMP180 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_BMP180_CMD_H_
#define _COMMANDS_BMP180_CMD_H_

#include "user_config.h"
#if defined(DH_COMMANDS_BMP180) && defined(DH_DEVICE_BMP180)
#include "dhsender_data.h"

/**
 * @brief Handle "devices/bmp180/read" command.
 */
void dh_handle_devices_bmp180_read(COMMAND_RESULT *cmd_res, const char *command,
                                   const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_BMP180 && DH_DEVICE_BMP180 */
#endif /* _COMMANDS_BMP180_CMD_H_ */
