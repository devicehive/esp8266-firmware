/**
 * @file
 * @brief MAX31855 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_MAX31855_CMD_H_
#define _COMMANDS_MAX31855_CMD_H_

#include "user_config.h"
#if defined(DH_COMMANDS_MAX31855) && defined(DH_DEVICE_MAX31855)
#include "dhsender_data.h"

/**
 * @brief Handle "devices/max31855/read" command.
 */
void dh_handle_devices_max31855_read(COMMAND_RESULT *cmd_res, const char *command,
                                     const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_MAX31855 && DH_DEVICE_MAX31855 */
#endif /* _COMMANDS_MAX31855_CMD_H_ */
