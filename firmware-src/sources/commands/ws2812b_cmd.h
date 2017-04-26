/**
 * @file
 * @brief Onewire WS2812B command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_WS2812B_CMD_H_
#define _COMMANDS_WS2812B_CMD_H_

#include "user_config.h"

#ifdef DH_COMMANDS_ONEWIRE // onewire WS2812B command handlers
#include "dhsender_data.h"
#include "dhcommand_parser.h"

/**
 * @brief Handle "onewire/ws2812b/write" command.
 */
void dh_handle_onewire_ws2812b_write(COMMAND_RESULT *cmd_res, const char *command,
                                     const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_ONEWIRE */
#endif /* _COMMANDS_WS2812B_CMD_H_ */
