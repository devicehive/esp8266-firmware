/**
 * @file
 * @brief GPIO hardware access layer for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DH_GPIO_H_
#define _DH_GPIO_H_

#include <c_types.h>

/**
 * @brief Pin mask type.
 */
typedef uint32_t DHGpioPinMask;


/**
 * @brief Get pin mask.
 * @param[in] num Pin number [0..DH_GPIO_PIN_COUNT).
 */
#define DH_GPIO_PIN(num) ((DHGpioPinMask)BIT(num))


/**
 * @brief Total number of GPIO pins.
 */
#define DH_GPIO_PIN_COUNT 16


/**
 * @brief Bitwise pin mask for pins that can be used.
 *
 * Pins: `GPIO0..GPIO5`, `GPIO12..GPIO15`.
 */
#define DH_GPIO_SUITABLE_PINS ( DH_GPIO_PIN(0) | DH_GPIO_PIN(1) | DH_GPIO_PIN(2) \
                              | DH_GPIO_PIN(3) | DH_GPIO_PIN(4) | DH_GPIO_PIN(5) \
                              | DH_GPIO_PIN(12)| DH_GPIO_PIN(13) \
                              | DH_GPIO_PIN(14)| DH_GPIO_PIN(15) )


/**
 * @brief Initialize GPIO sub-system.
 */
void dh_gpio_init(void);


/**
 * @brief Prepare pins for using as GPIO.
 *
 * Only suitable pins can be used.
 *
 * @param[in] pin_mask Bitwise pin mask to use as GPIO pins.
 * @param[in] disable_pwm Flag to disable PWM if it was enabled before.
 */
void dh_gpio_prepare_pins(DHGpioPinMask pins, bool disable_pwm);


/**
 * @brief Enable/disable open-drain switch for GPIO output.
 *
 * Only suitable pins can be used.
 *
 * @param[in] pins_enable Bitwise pin mask to enable.
 * @param[in] pins_disable Bitwise pin mask to disable.
 */
void dh_gpio_open_drain(DHGpioPinMask pins_enable,
                        DHGpioPinMask pins_disable);


/**
 * @brief Enable/disable pull-up GPIO pins.
 *
 * Only suitable pins can be used.
 *
 * @param[in] pins_enable Bitwise pin mask to enable.
 * @param[in] pins_disable Bitwise pin mask to disable.
 */
void dh_gpio_pull_up(DHGpioPinMask pins_enable,
                     DHGpioPinMask pins_disable);


/**
 * @brief Set GPIO outputs state.
 * @param[in] pins_set Bitwise pin mask for switching to high level.
 * @param[in] pins_unset Bitwise pin mask for switching to low level.
 * @return Zero on success.
 */
int dh_gpio_write(DHGpioPinMask pins_set,
                  DHGpioPinMask pins_unset);


/**
 * @brief Initializes GPIO pins for input.
 * @param[in] pins_init Bitwise pin mask for pins that should be switched to input mode without touching pull up state.
 * @param[in] pins_pullup Bitwise pin mask for pins that should be switched to input mode with enabled pull up.
 * @param[in] pins_nopull Bitwise pin mask for pins that should be switched to input mode with disable pull up.
 * @return Zero on success.
 */
int dh_gpio_input(DHGpioPinMask pins_init,
                  DHGpioPinMask pins_pullup,
                  DHGpioPinMask pins_nopull);


/**
 * @brief Read GPIO pins state.
 * @return Bitwise bit mask of input.
 */
DHGpioPinMask dh_gpio_read(void);


/**
 * @brief Enable GPIO interruption.
 * @param[in] pins_disable Bitwise pin mask for disabling interruption.
 * @param[in] pins_rising Bitwise pin mask for rising edge interruption.
 * @param[in] pins_falling Bitwise pin mask for falling edge interruption.
 * @param[in] pins_both Bitwise pin mask for rising and falling edge interruption.
 * @param[in] timeout_ms Timeout in milliseconds, that will pass after interruption occurred.
 *            During this period module will collect data about interruptions on other pins.
 * @return Zero on success.
 */
int dh_gpio_subscribe_int(DHGpioPinMask pins_disable,
                          DHGpioPinMask pins_rising,
                          DHGpioPinMask pins_falling,
                          DHGpioPinMask pins_both,
                          unsigned int timeout_ms);


/**
 * @brief Get current GPIO interruption timeout.
 * @return Timeout in milliseconds.
 */
unsigned int dh_gpio_get_timeout(void);


/**
 * @brief GPIO Interruption callback.
 * @param[in] caused_pins Bitwise pin mask with pins that was trigger interruption.
 */
// TODO: consider to use callback function pointer
extern void dh_gpio_int_cb(DHGpioPinMask caused_pins);


/**
 * @brief Enable GPIO extra interruption.
 * @param[in] pins_disable Bitwise pin mask for disabling interruption.
 * @param[in] pins_rising Bitwise pin mask for rising edge interruption.
 * @param[in] pins_falling Bitwise pin mask for falling edge interruption.
 * @param[in] pins_both Bitwise pin mask for rising and falling edge interruption.
 * @return Zero on success.
 */
int dh_gpio_subscribe_extra_int(DHGpioPinMask pins_disable,
                                DHGpioPinMask pins_rising,
                                DHGpioPinMask pins_falling,
                                DHGpioPinMask pins_both);


/**
 * @brief Extra interruption callback.
 * @param[in] caused_pins Bitwise pin mask with pins that was trigger interruption.
 */
// TODO: consider to use callback function pointer
extern void dh_gpio_extra_int_cb(DHGpioPinMask caused_pins);

#endif /* _DH_GPIO_H_ */
