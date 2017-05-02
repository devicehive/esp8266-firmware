/**
 * @file
 * @brief DHT command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_DHT_CMD_H_
#define _COMMANDS_DHT_CMD_H_

#include "user_config.h"
#include "dhsender_data.h"
#if defined(DH_COMMANDS_ONEWIRE)

/**
 * @brief Handle "onewire/dht/read" command.
 */
void dh_handle_onewire_dht_read(COMMAND_RESULT *cmd_res, const char *command,
                                const char *params, unsigned int params_len);
#endif /* DH_COMMANDS_ONEWIRE */


#if defined(DH_COMMANDS_DHT11) && defined(DH_DEVICE_DHT11)
/**
 * @brief Handle "devices/dht11/read" command.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_dht11_read(COMMAND_RESULT *cmd_res, const char *command,
                                                    const char *params, unsigned int params_len);
#endif /* DH_COMMANDS_DHT11 && DH_DEVICE_DHT11 */


#if defined(DH_COMMANDS_DHT22) && defined(DH_DEVICE_DHT22)
/**
 * @brief Handle "devices/dht22/read" command.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_dht22_read(COMMAND_RESULT *cmd_res, const char *command,
                                                    const char *params, unsigned int params_len);
#endif /* DH_COMMANDS_DHT22 || DH_DEVICE_DHT22 */

#endif /* _COMMANDS_DHT_CMD_H_ */
