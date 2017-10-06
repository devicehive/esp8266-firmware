/**
 * @file
 * @brief Software implementation of onewire interface for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DH_ONEWIRE_H_
#define _DH_ONEWIRE_H_

#include "DH/gpio.h"

#include <c_types.h>

/**
 * @brief Do not initialize pin indicator.
 */
#define DH_ONEWIRE_NO_PIN (-1)


/**
 * @brief Set pin for onewire, but not initialize it.
 * @param[in] pin Pin number.
 * @return Zero on success.
 */
int dh_onewire_set_pin(unsigned int pin);


/**
 * @brief Get pin that configured for onewire.
 * @return Pin number.
 */
unsigned int dh_onewire_get_pin(void);


/**
 * @brief Reset onewire bus.
 * @param[in] pin_mask Pin mask to reset.
 * @param[in] exit_on_presence Flag to exit immediately if device found.
 * @param[in] reset_time_us reset signal length.
 * @return Non-zero if device is presented. Zero otherwise.
 */
int dh_onewire_reset(DHGpioPinMask pin_mask, int reset_time_us, int exit_on_presence);


/**
 * @brief Write data to onewire bus.
 *
 * Pin will be initialized automatically.
 * @param[in] buf Buffer with data to write.
 * @param[in] len Buffer length in bytes.
 * return Zero on success.
 */
int dh_onewire_write(const void *buf, size_t len);


/**
 * @brief Read data from onewire bus.
 *
 * Pin will NOT be initialized automatically, will use previous pin.
 *
 * @param[out] buf Buffer to read data to.
 * @param[in] len Number of bytes to read, buffer should be at least the same size.
 * @return Zero on success.
 */
int dh_onewire_read(void *buf, size_t len);


// command 0xF0 // SEARCH ROM

/**
 * @brief Search for onewire devices.
 * @param[out] buf Buffer to store 64 bits addresses.
 * @param[in,out] len Receives number of bytes allowed in buffer, returns number of copied bytes. Can return 0, that means no devices were found.
 * @param[in] command Bus command for search, 0xF0 - SEARCH ROM, 0xEC - ALARM SEARCH.
 * @param[in] pin Pin number for using during search.
 * @return Zero on success.
 */
int dh_onewire_search(void *buf, size_t *len,
                      int command, unsigned int pin);


/**
 * @brief Enable interruption.
 * @param[in] search_pins Pins that will awaiting for PRESENSE and then runs SEARCH command.
 * @param[in] disable_pins Pin mask for disabling.
 * @return Zero on success.
 */
int dh_onewire_int(DHGpioPinMask search_pins, DHGpioPinMask disable_pins);


/**
 * @brief Callback for search result.
 * @param[in] pin Pin number where found.
 * @param[in] buf Buffer with 64 bits addresses.
 * @param[in] len Buffer length in bytes.
 */
// TODO: consider to use callback function pointer
extern void dh_onewire_search_result(unsigned int pin, const void* buf, size_t len);

#endif /* _DH_ONEWIRE_H_ */
