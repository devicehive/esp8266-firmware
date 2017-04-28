/**
 * @file
 * @brief HMC5883L command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_HMC5883L_CMD_H_
#define _COMMANDS_HMC5883L_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/hmc5883l/read" command.
 */
void dh_handle_devices_hmc5883l_read(COMMAND_RESULT *cmd_res, const char *command,
                                     const char *params, unsigned int params_len);

#endif /* _COMMANDS_HMC5883L_CMD_H_ */
