/**
 * @file
 * @brief I2C command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_I2C_CMD_H_
#define _COMMANDS_I2C_CMD_H_

#include "user_config.h"

#ifdef DH_COMMANDS_I2C // I2C command handlers
#include "dhcommand_parser.h"
#include "dhsender_data.h"

/**
 * @brief Helper function to initialize I2C bus.
 * @return Non-zero if I2C was initialized. Zero otherwise.
 */
int dh_i2c_init_helper(COMMAND_RESULT *cmd_res, ALLOWED_FIELDS fields,
                       const gpio_command_params *params);


/**
 * @brief Handle "i2c/master/read" command.
 */
void dh_handle_i2c_master_read(COMMAND_RESULT *cmd_res, const char *command,
                               const char *params, unsigned int params_len);


/**
 * @brief Handle "i2c/master/write" command.
 */
void dh_handle_i2c_master_write(COMMAND_RESULT *cmd_res, const char *command,
                                const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_I2C */
#endif /* _COMMANDS_I2C_CMD_H_ */
