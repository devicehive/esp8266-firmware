/**
 * @file
 * @brief UART command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_UART_CMD_H_
#define _COMMANDS_UART_CMD_H_

#include "user_config.h"
#if defined(DH_COMMANDS_UART) // UART command handlers
#include "dhsender_data.h"

/**
 * @brief Handle "uart/write" command.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_write(COMMAND_RESULT *cmd_res, const char *command,
                                            const char *params, unsigned int params_len);


/**
 * @brief Handle "uart/read" command.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_read(COMMAND_RESULT *cmd_res, const char *command,
                                           const char *params, unsigned int params_len);


/**
 * @brief Handle "uart/int" command.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_int(COMMAND_RESULT *cmd_res, const char *command,
                                          const char *params, unsigned int params_len);


/**
 * @brief Handle "uart/terminal" command.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_terminal(COMMAND_RESULT *cmd_res, const char *command,
                                               const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_UART */
#endif /* _COMMANDS_UART_CMD_H_ */
