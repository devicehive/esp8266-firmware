/**
 * @file
 * @brief ADC hardware access layer for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DH_ADC_H_
#define _DH_ADC_H_

/**
 * @brief ADC suitable channels.
 */
#define DH_ADC_SUITABLE_PINS 0x01 // ADC0


/**
 * @brief Get ADC value.
 * @return ADC voltage in volts.
 */
float dh_adc_get_value(void);


/**
 * @brief Start ADC loop measurement.
 * @param[in] period_ms Measurement interval in milliseconds.
 */
void dh_adc_loop(unsigned int period_ms);


/**
 * @brief Callback function for loop measurement.
 * @param[in] value ADC voltage in volts.
 */
// TODO: consider to use callback function pointer
extern void dh_adc_loop_value_cb(float value);


#if 1 // command handlers
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

#endif // command handlers

#endif /* _DH_ADC_H_ */
