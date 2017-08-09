/**
 * @file
 * @brief Onewire WS2812B command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/ws2812b_cmd.h"
#include "commands/onewire_cmd.h"
#include "DH/onewire.h"
#include "DH/adc.h"

#ifdef DH_COMMANDS_ONEWIRE // onewire command handlers
#include "dhcommand_parser.h"
#include <user_interface.h>

#include <osapi.h>
#include <ets_forward.h>

/**
 * Write data to WS2812b device.
 */
/*static*/ void ws2812b_write(const void *buf_, size_t len, DHGpioPinMask pin_mask,
                          unsigned int T, unsigned int S, unsigned int L)
{
	// due to the strict timings, this method should be in IRAM,
	// don't mark it with ICACHE_FLASH_ATTR

	// send each byte in real time
	unsigned int ss, se, r;
	const uint8_t *buf = (const uint8_t*)buf_;
	while (len--) {
		int j, v = *buf++;
		for (j = 0x80; j > 0; j >>= 1) {
			GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pin_mask);
			asm volatile("rsr %0, ccount" : "=r"(r));
			ss = r + ((v & j) ? L : S);
			se = r + T;
			do {
				asm volatile("rsr %0, ccount" : "=r"(r));
			} while(r < ss);
			GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pin_mask);
			do {
				asm volatile("rsr %0, ccount" : "=r"(r));
			} while(r < se);
		}
	}
}


/**
 * @brief Write data to WS2812b like device
 * @param[in] pin 1-wire pin for communication.
 * @param[in] buf Pointer to buffer with data.
 * @param[in] len Buffer length in bytes.
 */
static void ICACHE_FLASH_ATTR onewire_ws2812b_write(const void *buf, size_t len)
{
	// init number of CPU tacts for delay
	// delays are lower that spec on 50ns due to GPIO latency, anyway spec allows 150ns jitter.
	const int freq = system_get_cpu_freq(); // MHz
	const unsigned S = 350 * freq / 1000;
	const unsigned L = 750 * freq / 1000;

	ETS_INTR_LOCK();

	// send reset, low more then 50 ms
	const DHGpioPinMask pin = DH_GPIO_PIN(dh_onewire_get_pin());
	gpio_output_set(0, pin, pin, 0); // low and initialize
	os_delay_us(50);
	ws2812b_write(buf, len, pin, S + L, S, L);

	ETS_INTR_UNLOCK();
}


/**
 * @brief Handle "onewire/ws2812b/write" command.
 */
// TODO: move this code to dedicated device file!
void ICACHE_FLASH_ATTR dh_handle_onewire_ws2812b_write(COMMAND_RESULT *cmd_res, const char *command,
                                                       const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;

	const char *err_msg = parse_params_pins_set(params, params_len, &info,
			DH_ADC_SUITABLE_PINS, 0, AF_PIN | AF_DATA, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}
	if (!(fields & AF_DATA)) {
		dh_command_fail(cmd_res, "Data not specified");
		return; // FAILED
	}
	if (dh_onewire_init_helper(cmd_res, fields, &info))
		return; // FAILED

	onewire_ws2812b_write(info.data, info.data_len);
	dh_command_done(cmd_res, "");
}

#endif /* DH_COMMANDS_ONEWIRE */
