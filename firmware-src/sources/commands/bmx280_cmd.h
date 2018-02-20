/**
 * @file
 * @brief BMP280 and BME280 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_BMX280_CMD_H_
#define _COMMANDS_BMX280_CMD_H_

#include "user_config.h"
#if defined(DH_COMMANDS_BMX280) && defined(DH_DEVICE_BMX280)
#include "dhsender_data.h"

/**
 * @brief Handle "devices/bmp280/read" command.
 */
void dh_handle_devices_bmp280_read(COMMAND_RESULT *cmd_res, const char *command,
                                   const char *params, unsigned int params_len);

/**
 * @brief Handle "devices/bme280/read" command.
 */
void dh_handle_devices_bme280_read(COMMAND_RESULT *cmd_res, const char *command,
                                   const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_BMX280 && DH_DEVICE_BMX280 */
#endif /* _COMMANDS_BMX280_CMD_H_ */
