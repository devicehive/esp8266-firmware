/**
 * @file
 * @brief Onewire command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_ONEWIRE_CMD_H_
#define _COMMANDS_ONEWIRE_CMD_H_

#include "user_config.h"

#ifdef DH_COMMANDS_ONEWIRE // onewire command handlers
#include "dhsender_data.h"
#include "dhcommand_parser.h"

/**
 * @brief Initialization helper.
 * @return Non-zero if onewire was initialized. Zero otherwise.
 */
int dh_onewire_init_helper(COMMAND_RESULT *cmd_res, ALLOWED_FIELDS fields,
                           const gpio_command_params *params);


/**
 * @brief Handle "onewire/master/read" command.
 */
void dh_handle_onewire_master_read(COMMAND_RESULT *cmd_res, const char *command,
                                   const char *params, unsigned int params_len);


/**
 * @brief Handle "onewire/master/write" commands.
 */
void dh_handle_onewire_master_write(COMMAND_RESULT *cmd_res, const char *command,
                                    const char *params, unsigned int params_len);


/**
 * Handle "onewire/master/search" or "onewire/master/alarm" commands.
 */
void dh_handle_onewire_master_search(COMMAND_RESULT *cmd_res, const char *command,
                                     const char *params, unsigned int params_len);


/**
 * @brief Handle "onewire/master/int" command.
 */
void dh_handle_onewire_master_int(COMMAND_RESULT *cmd_res, const char *command,
                                  const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_ONEWIRE */
#endif /* _COMMANDS_ONEWIRE_CMD_H_ */
