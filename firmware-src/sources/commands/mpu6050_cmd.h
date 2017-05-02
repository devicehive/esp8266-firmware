/**
 * @file
 * @brief MPU6050 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_MPU6050_CMD_H_
#define _COMMANDS_MPU6050_CMD_H_

#include "user_config.h"
#if defined(DH_COMMANDS_MPU6050) && defined(DH_DEVICE_MPU6050)
#include "dhsender_data.h"

/**
 * @brief Handle "devices/mpu6050/read" command.
 */
void dh_handle_devices_mpu6050_read(COMMAND_RESULT *cmd_res, const char *command,
                                    const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_MPU6050 && DH_DEVICE_MPU6050 */
#endif /* _COMMANDS_MPU6050_CMD_H_ */
