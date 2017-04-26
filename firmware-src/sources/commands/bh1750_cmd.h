/**
 * @file
 * @brief BH1750 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_BH1750_CMD_H_
#define _COMMANDS_BH1750_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/bh1750/read" command.
 */

void dh_handle_devices_bh1750_read(COMMAND_RESULT *cmd_res, const char *command,
                                   const char *params, unsigned int params_len);

#endif /* _COMMANDS_BH1750_CMD_H_ */
