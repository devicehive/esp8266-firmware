/**
 * @file
 * @brief HD44780 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_PCF8574_HD44780_CMD_H_
#define _COMMANDS_PCF8574_HD44780_CMD_H_

#include "dhsender_data.h"

/**
 * @brief Handle "devices/pcf8574/hd44780/write" command.
 */
void dh_handle_devices_pcf8574_hd44780_write(COMMAND_RESULT *cmd_res, const char *command,
                                             const char *params, unsigned int params_len);

#endif /* _COMMANDS_PCF8574_HD44780_CMD_H_ */
