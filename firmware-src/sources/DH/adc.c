/**
 * @file
 * @brief ADC hardware access layer for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "DH/adc.h"

#include <ets_sys.h>
#include <os_type.h>
#include <osapi.h>
#include <user_interface.h>
#include <ets_forward.h>

// module variables
static os_timer_t mTimer;

/*
 * dh_adc_get_value() implementation.
 */
float ICACHE_FLASH_ATTR dh_adc_get_value(void)
{
	return system_adc_read() / 1024.0f;
}


/**
 * @brief Timeout callback.
 */
static void ICACHE_FLASH_ATTR timeout_cb(void *arg)
{
	// call external handler
	dh_adc_loop_value_cb(dh_adc_get_value());
}


/*
 * dh_adc_loop() implementation.
 */
void ICACHE_FLASH_ATTR dh_adc_loop(unsigned int period_ms)
{
	os_timer_disarm(&mTimer);
	if (period_ms) {
		os_timer_setfn(&mTimer, timeout_cb, NULL);
		os_timer_arm(&mTimer, period_ms, 1);
	}
}


#if 1 // command handlers
#include "dhcommand_parser.h"
#include <user_interface.h>

/**
 * Minimum notification timeout, milliseconds.
 */
#define MIN_TIMEOUT_MS 250


/**
 * Maximum notification timeout, milliseconds.
 */
#define MAX_TIMEOUT_MS 0x7fffff


/*
 * dh_handle_adc_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_adc_read(COMMAND_RESULT *cmd_res, const char *command,
                                          const char *params, unsigned int params_len)
{
	if (params_len) {
		gpio_command_params info;
		ALLOWED_FIELDS fields = 0;
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0, AF_READ, &fields);

		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}

		if (info.pins_to_read != DH_ADC_SUITABLE_PINS) {
			dh_command_fail(cmd_res, "Unknown ADC channel");
			return; // FAILED
		}
	}

	// OK, report ADC value
	cmd_res->callback(cmd_res->data, DHSTATUS_OK,
	                  RDT_FLOAT, dh_adc_get_value());
}


/*
 * dh_handle_adc_int() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_adc_int(COMMAND_RESULT *cmd_res, const char *command,
                                         const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0, AF_VALUES, &fields);
	// TODO: use AF_TIMEOUT here?

	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	} else if (info.pin_value_readed != DH_ADC_SUITABLE_PINS) {
		dh_command_fail(cmd_res, "Unknown ADC channel");
		return; // FAILED
	}

	const unsigned int timeout = info.storage.uint_values[0];
	if ((timeout != 0 && timeout < MIN_TIMEOUT_MS) || timeout > MAX_TIMEOUT_MS) {
		dh_command_fail(cmd_res, "Wrong period");
	} else {
		dh_adc_loop(timeout);
		dh_command_done(cmd_res, "");
	}
}

#endif // command handlers
