/**
 * @file
 * @brief MCP4725 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_MCP4725_CMD_H_
#define _COMMANDS_MCP4725_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/mcp4725/write" command.
 */
void dh_handle_devices_mcp4725_write(COMMAND_RESULT *cmd_res, const char *command,
                                     const char *params, unsigned int params_len);

#endif /* _COMMANDS_MCP4725_CMD_H_ */
