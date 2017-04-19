/**
 * @file
 * @brief Handle GPIO commands.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DH_CMD_GPIO_H_
#define _DH_CMD_GPIO_H_

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


#endif /* _DH_CMD_GPIO_H_ */
