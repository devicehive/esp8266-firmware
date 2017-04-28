/**
 * @file
 * @brief TM1637 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_TM1637_CMD_H_
#define _COMMANDS_TM1637_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/tm1637/write" command.
 */
void dh_handle_devices_tm1637_write(COMMAND_RESULT *cmd_res, const char *command,
                                    const char *params, unsigned int params_len);

#endif /* _COMMANDS_TM1637_CMD_H_ */
