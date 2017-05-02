/**
 * @file
 * @brief PCF8574 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_PCF8574_CMD_H_
#define _COMMANDS_PCF8574_CMD_H_

#include "user_config.h"
#if defined(DH_COMMANDS_PCF8574) && defined(DH_DEVICE_PCF8574)
#include "dhsender_data.h"

/**
 * @brief Handle "devices/pcf8574/read" command.
 */
void dh_handle_devices_pcf8574_read(COMMAND_RESULT *cmd_res, const char *command,
                                    const char *params, unsigned int params_len);


/**
 * @brief Handle "devices/pcf8574/write" command.
 */
void dh_handle_devices_pcf8574_write(COMMAND_RESULT *cmd_res, const char *command,
                                     const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_PCF8574 && DH_DEVICE_PCF8574 */
#endif /* _COMMANDS_PCF8574_CMD_H_ */
