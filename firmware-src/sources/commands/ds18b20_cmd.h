/**
 * @file
 * @brief DS18B20 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_DS18B20_CMD_H_
#define _COMMANDS_DS18B20_CMD_H_

#include "user_config.h"
#if defined(DH_COMMANDS_DS18B20) && defined(DH_DEVICE_DS18B20)
#include "dhsender_data.h"

/**
 * @brief Handle "devices/ds18b20/read" command.
 */
void dh_handle_devices_ds18b20_read(COMMAND_RESULT *cmd_res, const char *command,
                                    const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_DS18B20 && DH_DEVICE_DS18B20 */
#endif /* _COMMANDS_DS18B20_CMD_H_ */
