/**
 * @file
 * @brief DHT command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_DHT_CMD_H_
#define _COMMANDS_DHT_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "onewire/dht/read" command.
 */
void dh_handle_onewire_dht_read(COMMAND_RESULT *cmd_res, const char *command,
                                const char *params, unsigned int params_len);

#endif /* _COMMANDS_DHT_CMD_H_ */
