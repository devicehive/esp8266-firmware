/**
 * @file
 * @brief GPIO command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_GPIO_CMD_H_
#define _COMMANDS_GPIO_CMD_H_

#include "user_config.h"

#ifdef DH_COMMANDS_GPIO // GPIO command handlers
#include "dhsender_data.h"

/**
 * @brief Handle "gpio/write" command.
 */
void dh_handle_gpio_write(COMMAND_RESULT *cmd_res, const char *command,
                          const char *params, unsigned int params_len);


/**
 * @brief Handle "gpio/read" command.
 */
void dh_handle_gpio_read(COMMAND_RESULT *cmd_res, const char *command,
                         const char *params, unsigned int params_len);


/**
 * @brief Handle "gpio/int" command.
 */
void dh_handle_gpio_int(COMMAND_RESULT *cmd_res, const char *command,
                        const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_GPIO */
#endif /* _COMMANDS_GPIO_CMD_H_ */
