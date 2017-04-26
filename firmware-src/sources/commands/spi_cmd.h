/**
 * @file
 * @brief SPI command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_SPI_CMD_H_
#define _COMMANDS_SPI_CMD_H_

#include "user_config.h"

#ifdef DH_COMMANDS_SPI // SPI command handlers
#include "dhsender_data.h"

/**
 * @brief Handle "spi/master/read" command.
 */
void dh_handle_spi_master_read(COMMAND_RESULT *cmd_res, const char *command,
                               const char *params, unsigned int params_len);


/**
 * @brief Handle "spi/master/write" command.
 */
void dh_handle_spi_master_write(COMMAND_RESULT *cmd_res, const char *command,
                                const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_SPI */
#endif /* _COMMANDS_SPI_CMD_H_ */
