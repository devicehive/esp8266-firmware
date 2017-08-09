/**
 * @file
 * @brief MHZ19 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_MHZ19_CMD_H_
#define _COMMANDS_MHZ19_CMD_H_

#include "user_config.h"
#if defined(DH_COMMANDS_MHZ19) && defined(DH_DEVICE_MHZ19)
#include "dhsender_data.h"

/**
 * @brief Handle "devices/mhz19/read" command.
 */
void dh_handle_devices_mhz19_read(COMMAND_RESULT *cmd_res, const char *command,
                                  const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_MHZ19 && DH_DEVICE_MHZ19 */
#endif /* _COMMANDS_MHZ19_CMD_H_ */
