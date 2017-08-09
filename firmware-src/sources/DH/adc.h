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

#endif /* _DH_ADC_H_ */
