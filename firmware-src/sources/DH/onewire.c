/**
 * @file
 * @brief Software implementation of onewire interface for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "DH/onewire.h"
#include "DH/adc.h"
#include "dhdebug.h"

#include <eagle_soc.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include <ets_sys.h>
#include <user_interface.h>
#include <ets_forward.h>

// module variables
static unsigned int mOneWirePin = 0;
static DHGpioPinMask mIntPins = 0;
static unsigned int mIntErrorCount = 0;
static unsigned int mWaitSearchPins = 0;
static os_timer_t mIntTimer;

#define ONEWIRE_MAX_INT_SEARCH_ATTEMPS  5
#define ONEWIRE_MAX_INT_DELAY_MS        20
#define ONEWIRE_RESET_LENGHT_US         640


/**
 * @brief Lock interruptions.
 */
static void ICACHE_FLASH_ATTR lock_int(void)
{
	if (mIntPins)
		dh_gpio_subscribe_extra_int(mIntPins, 0, 0, 0);
}


/**
 * @brief Unlock interruptions.
 */
static void ICACHE_FLASH_ATTR unlock_int(void)
{
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, mIntPins);
	dh_gpio_subscribe_extra_int(0, 0, mIntPins, 0);
}


/*
 * dh_onewire_set_pin() implementation.
 */
int ICACHE_FLASH_ATTR dh_onewire_set_pin(unsigned int pin)
{
	if (!(DH_GPIO_PIN(pin) & DH_GPIO_SUITABLE_PINS))
		return -1; // unsuitable pin

	mOneWirePin = pin;
	return 0; // OK
}


/*
 * dh_onewire_get_pin() implementation.
 */
unsigned int ICACHE_FLASH_ATTR dh_onewire_get_pin(void)
{
	return mOneWirePin;
}


/**
 * @brief Reset onewire bus.
 * @return Non-zero if device is presented. Zero otherwise.
 */
int ICACHE_FLASH_ATTR dh_onewire_reset(DHGpioPinMask pin_mask, int exit_on_presence)
{
	system_soft_wdt_feed();
	const int pinstate = (gpio_input_get() & pin_mask) !=0;
	dh_gpio_open_drain(pin_mask, 0);
	dh_gpio_pull_up(pin_mask, 0);
	dh_gpio_prepare_pins(pin_mask, 1);
	if (!pinstate) {
		gpio_output_set(pin_mask, 0, pin_mask, 0);
		os_delay_us(500);
	}
	if (!(gpio_input_get() & pin_mask))
		return 0;

	// send RESET
	gpio_output_set(0, pin_mask, pin_mask, 0);
	os_delay_us(ONEWIRE_RESET_LENGHT_US);
	gpio_output_set(pin_mask, 0, pin_mask, 0);

	// check RESPONSE
	int i, presence = 0;
	for (i = 0; i < 480; i++) { // wait at least 480 usec
		if (!presence && i < 240) {
			if (i > 15) {
				if (!(gpio_input_get() & pin_mask))
					presence = 1;
			}
		} else if(exit_on_presence) {
			if (presence) {
				if (gpio_input_get() & pin_mask)
					return 1;
			} else {
				return 0;
			}
		}
		os_delay_us(1);
	}

	return presence;
}


/**
 * @brief Do 1 bit communication.
 */
static int ICACHE_FLASH_ATTR onewire_act_bit(int bit, DHGpioPinMask pin_mask)
{
	// time slot start
	char res = 0;
	ETS_INTR_LOCK();
	gpio_output_set(0, pin_mask, pin_mask, 0);
	os_delay_us(5);

	if (bit) {
		gpio_output_set(pin_mask, 0, pin_mask, 0);
		os_delay_us(5);
		if (gpio_input_get() & pin_mask)
			res = bit;
		ETS_INTR_UNLOCK();
		os_delay_us(80);
	} else {
		os_delay_us(85);
		gpio_output_set(pin_mask, 0, pin_mask, 0);
		ETS_INTR_UNLOCK();
	}

	os_delay_us(5); // pause between time slots
	return res;
}


/**
 * @brief Do 1 byte communication.
 */
static int ICACHE_FLASH_ATTR onewire_act_byte(int byte, DHGpioPinMask pin_mask)
{
	system_soft_wdt_feed();

	int i, res = 0;
	for (i = 0; i < 8; i++) // lsb-first
		res |= onewire_act_bit(byte & BIT(i), pin_mask);

	return res;
}


/*
 * dh_onewire_write() implementation.
 */
int ICACHE_FLASH_ATTR dh_onewire_write(const void *buf_, size_t len)
{
	lock_int();

	const DHGpioPinMask pin_mask = DH_GPIO_PIN(mOneWirePin);
	int present = dh_onewire_reset(pin_mask, 0);
	if (!present) {
		unlock_int();
		return -1; // failed
	}

	const uint8_t *buf = (const uint8_t*)buf_;
	while (len--)
		onewire_act_byte(*buf++, pin_mask);

	unlock_int();
	return 0; // OK
}


/*
 * dh_onewire_read() implementation.
 */
int ICACHE_FLASH_ATTR dh_onewire_read(void *buf_, size_t len) {
	lock_int();

	const DHGpioPinMask pin_mask = DH_GPIO_PIN(mOneWirePin);
	uint8_t *buf = (uint8_t*)buf_;
	while (len--)
		*buf++ = onewire_act_byte(0xFF, pin_mask);

	unlock_int();
	return 0; // OK
}


/**
 * @brief Check onewire CRC.
 * @return Zero on success.
 */
static int ICACHE_FLASH_ATTR dh_onewire_check_crc(const uint8_t data[8])
{
	int seed = 0;

	int i, j;
	for (i = 0; i < 8; i++) {
		int b = data[i];
		for (j = 0; j < 8; j++) {
			if((seed ^ b) & 0x01)
				seed = (seed >> 1) ^ 0x8C;
			else
				seed >>= 1;
			b >>= 1;
		}
	}

	return seed;
}


/*
 * dh_onewire_search() implementation.
 */
int ICACHE_FLASH_ATTR dh_onewire_search(void *buf_, size_t *len, int command, unsigned int pin)
{
	uint8_t address[8] = {0};
	if (*len < sizeof(address))
		return -1; // too few space in buffer

	const DHGpioPinMask pin_mask = DH_GPIO_PIN(pin);
	int lastAmbiguity = 8*sizeof(address);
	char *buf = (char*)buf_;
	size_t copyied = 0;

	lock_int();

	do {
		if (!dh_onewire_reset(pin_mask, 0)) {
			unlock_int();
			if (lastAmbiguity == 8*sizeof(address)) {
				*len = 0; // devices are not connected
				return 0;
			}
			return -1; // something wrong
		}

		onewire_act_byte(command, pin_mask);
		int i, ambiguity = -1;
		for (i = 0; i < 8*sizeof(address); i++) {
			if (!(gpio_input_get() & pin_mask)) {
				unlock_int();
				return -1; // lost
			}

			const int byte = i/8;
			const int bit = BIT(i%8);

			const int bit1 = onewire_act_bit(0x1, pin_mask);
			const int bit2 = onewire_act_bit(0x1, pin_mask);
			if (bit1 && bit2) {
				unlock_int();
				if (i == 0 && lastAmbiguity == 8*sizeof(address)) {
					*len = 0; // first interaction, no devices found
					return 0; // OK, no devices
				}
				// dhdebug("Onewire error while search at %u bit", i);
				return -1; // something wrong
			} else if(bit1 && !bit2) {
				address[byte] |= bit;
				onewire_act_bit(0x1, pin_mask);
			} else if(!bit1 && bit2) {
				address[byte] &= ~bit;
				onewire_act_bit(0x0, pin_mask);
			} else {
				if (i < lastAmbiguity) {
					address[byte] &= ~bit;
					onewire_act_bit(0x0, pin_mask);
					ambiguity = i;
				} else {
					address[byte] |= bit;
					onewire_act_bit(0x1, pin_mask);
				}
			}
		}
		lastAmbiguity = ambiguity;

		if (0 == dh_onewire_check_crc(address)) {
			os_memcpy(buf, address, sizeof(address));
			buf += sizeof(address);
			copyied += sizeof(address);
			if (copyied +  sizeof(address) > *len)
				break; // no more space
		} else {
			unlock_int();
			return -1; // bad CRC
		}
	} while (lastAmbiguity >= 0);

	*len = copyied;
	unlock_int();
	return 0; // OK
}


/**
 * @brief Timer callback.
 */
static void ICACHE_FLASH_ATTR onewire_int_search(void *arg)
{
	int i;
	for (i = 0; i < DH_GPIO_PIN_COUNT; i++) {
		const DHGpioPinMask pin = DH_GPIO_PIN(i);
		if (pin & mWaitSearchPins) {
			uint8_t buf[INTERFACES_BUF_SIZE];
			size_t len = sizeof(buf);
			int res = dh_onewire_search(buf, &len, 0xF0, i);
			if (len == 0)
				res = 1; // not found
			if (0 == res)
				dh_onewire_search_result(i, buf, len);
			else
				mIntErrorCount++;
			if (0 == res || mIntErrorCount > ONEWIRE_MAX_INT_SEARCH_ATTEMPS) {
				mIntErrorCount = 0;
				mWaitSearchPins &= ~pin;
			}

			// re-arm timer to avoid long operating in timer interruption.
			if (mWaitSearchPins) {
				os_timer_disarm(&mIntTimer);
				os_timer_setfn(&mIntTimer, onewire_int_search, NULL);
				os_timer_arm(&mIntTimer, ONEWIRE_MAX_INT_DELAY_MS, 0);
			}
			break;
		}
	}
}


/*
 * dh_gpio_extra_int_cb() implementation.
 */
void ICACHE_FLASH_ATTR dh_gpio_extra_int_cb(DHGpioPinMask caused_pins)
{
	os_timer_disarm(&mIntTimer);
	mWaitSearchPins |= caused_pins;
	os_timer_setfn(&mIntTimer, onewire_int_search, NULL);
	os_timer_arm(&mIntTimer, ONEWIRE_MAX_INT_DELAY_MS, 0);
}


/*
 * dh_onewire_int() implementation.
 */
int ICACHE_FLASH_ATTR dh_onewire_int(DHGpioPinMask search_pins, DHGpioPinMask disable_pins)
{
	int res = dh_gpio_subscribe_extra_int(disable_pins, 0, search_pins, 0);
	if (!!res)
		return res; // failed to subscribe

	mIntPins |= search_pins;
	mIntPins &= ~disable_pins;
	return 0; // OK
}


#ifdef DH_COMMANDS_ONEWIRE // onewire command handlers
#include "dhcommand_parser.h"
#include <user_interface.h>


/**
 * @brief Initialization helper.
 * @return Non-zero if onewire was initialized. Zero otherwise.
 */
int ICACHE_FLASH_ATTR dh_onewire_init_helper(COMMAND_RESULT *cmd_res, ALLOWED_FIELDS fields,
                                             const gpio_command_params *params)
{
	if (fields & AF_PIN) {
		if (!!dh_onewire_set_pin(params->pin)) {
			dh_command_fail(cmd_res, "Wrong onewire pin");
			return 1; // FAILED
		}
	}

	return 0; // continue
}


/**
 * @brief Handle "onewire/master/read" command.
 */
void ICACHE_FLASH_ATTR dh_handle_onewire_master_read(COMMAND_RESULT *cmd_res, const char *command,
                                                     const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_PIN | AF_DATA | AF_COUNT, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}
	if ((fields & AF_COUNT) == 0 || info.count == 0 || info.count > INTERFACES_BUF_SIZE) {
		dh_command_fail(cmd_res, "Wrong read size");
		return; // FAILED
	}
	if ((fields & AF_DATA) == 0) {
		dh_command_fail(cmd_res, "Command for reading is not specified");
		return;
	}
	if (dh_onewire_init_helper(cmd_res, fields, &info))
		return; // FAILED
	if (!!dh_onewire_write(info.data, info.data_len)) {
		dh_command_fail(cmd_res, "No response");
		return; // FAILED
	}
	dh_onewire_read(info.data, info.count);
	dh_command_done_buf(cmd_res, info.data, info.count);
}


/**
 * @brief Handle "onewire/master/write" commands.
 */
void ICACHE_FLASH_ATTR dh_handle_onewire_master_write(COMMAND_RESULT *cmd_res, const char *command,
                                                      const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_PIN | AF_DATA, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}
	if (dh_onewire_init_helper(cmd_res, fields, &info))
		return; // FAILED
	if (!!dh_onewire_write(info.data, info.data_len)) {
		dh_command_fail(cmd_res, "No response");
		return; // FAILED
	}

	dh_command_done(cmd_res, "");
}


/**
 * Handle "onewire/master/search" or "onewire/master/alarm" commands.
 */
void ICACHE_FLASH_ATTR dh_handle_onewire_master_search(COMMAND_RESULT *cmd_res, const char *command,
                                                       const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	if (params_len) {
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0, AF_PIN, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
		if(dh_onewire_init_helper(cmd_res, fields, &info))
			return; // FAILED
	}

	int check = os_strcmp(command, "onewire/master/search");
	size_t data_len = sizeof(info.data);
	if (!!dh_onewire_search(info.data, &data_len, (check == 0) ? 0xF0 : 0xEC, dh_onewire_get_pin()))
		dh_command_fail(cmd_res, "Error during search");
	else
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_SEARCH64, dh_onewire_get_pin(), info.data, data_len);
}


/**
 * @brief Handle "onewire/master/int" command.
 */
void ICACHE_FLASH_ATTR dh_handle_onewire_master_int(COMMAND_RESULT *cmd_res, const char *command,
                                                    const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;

	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_GPIO_SUITABLE_PINS, dh_gpio_get_timeout(),
			AF_DISABLE | AF_PRESENCE, &fields);
	if (err_msg != 0)
		dh_command_fail(cmd_res, err_msg);
	else if (fields == 0)
		dh_command_fail(cmd_res, "Wrong action");
	else if (!!dh_onewire_int(info.pins_to_presence, info.pins_to_disable))
		dh_command_fail(cmd_res, "Unsuitable pin");
	else
		dh_command_done(cmd_res, "");
}


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
	const DHGpioPinMask pin = DH_GPIO_PIN(mOneWirePin);
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
