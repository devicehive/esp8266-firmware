/**
 * @file
 * @brief ADC command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _COMMANDS_ADC_CMD_H_
#define _COMMANDS_ADC_CMD_H_

#include "user_config.h"
#if defined(DH_COMMANDS_ADC) // ADC command handlers
#include "dhsender_data.h"

/**
 * @brief Handle "adc/read" command.
 */
void dh_handle_adc_read(COMMAND_RESULT *cmd_res, const char *command,
                        const char *params, unsigned int params_len);


/**
 * @brief Handle "adc/int" command.
 */
void dh_handle_adc_int(COMMAND_RESULT *cmd_res, const char *command,
                       const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_ADC */
#endif /* _COMMANDS_ADC_CMD_H_ */
