/**
 * @file
 * @brief MFRC522 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_MFRC522_CMD_H_
#define _COMMANDS_MFRC522_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/mfrc522/read" command.
 */
void dh_handle_devices_mfrc522_read(COMMAND_RESULT *cmd_res, const char *command,
                                    const char *params, unsigned int params_len);


/**
 * @brief Handle "devices/mfrc522/mifare/read" and "devices/mfrc522/mifare/write" commands.
 */
void dh_handle_devices_mfrc522_mifare_read_write(COMMAND_RESULT *cmd_res, const char *command,
                                                 const char *params, unsigned int params_len);

#endif /* _COMMANDS_MFRC522_CMD_H_ */
