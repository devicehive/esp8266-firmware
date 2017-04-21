/**
 * @file
 * @brief UART HAL for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "DH/uart.h"
#include "DH/adc.h"
#include "dhdebug.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include <ets_forward.h>

#define UART_BASE 0x60000000
#define UART_INTERUPTION_STATE_REGISTER (UART_BASE + 0x08)
#define UART_INTERUPTION_ENABLE_REGISTER (UART_BASE + 0x0C)
#define UART_INTERUPTION_REGISTER (UART_BASE + 0x10)
#define UART_STATUS_REGISTER (UART_BASE + 0x1C)
#define UART_DIV_REGISTER (UART_BASE + 0x14)
#define UART_CONFIGURATION_REGISTER0 (UART_BASE + 0x20)
#define UART_CONFIGURATION_REGISTER1 (UART_BASE + 0x24)

#define BUFFER_OVERFLOW_RESERVE INTERFACES_BUF_SIZE

// module variables
static DHUartDataMode mDataMode = DH_UART_MODE_IGNORE;
static uint8_t mUartBuf[INTERFACES_BUF_SIZE + BUFFER_OVERFLOW_RESERVE] = {0};
static size_t mUartBufPos = 0;
static os_timer_t mUartTimer;
static unsigned int mTimeoutMs = 250;
static int mBufInterrupt = false;
static os_timer_t mRecoverLEDTimer;
static int mKeepLED = false;

static void arm_buf_timer(void);


/**
 * @brief Buffer timer callback.
 */
static void ICACHE_FLASH_ATTR buf_timeout_cb(void *arg)
{
	size_t sz = mUartBufPos;
	if (sz > INTERFACES_BUF_SIZE)
		sz = INTERFACES_BUF_SIZE;
	dh_uart_buf_rcv_cb(mUartBuf, sz);

	ETS_UART_INTR_DISABLE();
	if (sz != mUartBufPos) {
		os_memmove(mUartBuf, &mUartBuf[sz],
		           mUartBufPos - sz);
		mUartBufPos -= sz;
	} else {
		mUartBufPos = 0;
	}
	ETS_UART_INTR_ENABLE();
	if (mUartBufPos)
		arm_buf_timer();
}


/**
 * Start buffer timer.
 */
static void ICACHE_FLASH_ATTR arm_buf_timer(void)
{
	os_timer_disarm(&mUartTimer);
	os_timer_setfn(&mUartTimer, buf_timeout_cb, NULL);
	os_timer_arm(&mUartTimer, (mTimeoutMs == 0 || mUartBufPos >= INTERFACES_BUF_SIZE) ? 1 : mTimeoutMs, 0);
}


/**
 * @brief UART RX interruption handler.
 */
static void int_cb(void *arg)
{
	if (READ_PERI_REG(UART_INTERUPTION_STATE_REGISTER) & BIT(0)) {
		const int rcvChar = READ_PERI_REG(UART_BASE) & 0xFF;
		WRITE_PERI_REG(UART_INTERUPTION_REGISTER, BIT(0));

		switch(mDataMode) {
		case DH_UART_MODE_PER_BYTE:
			dh_uart_char_rcv_cb(rcvChar);
			break;

		case DH_UART_MODE_PER_BUF:
			if (mUartBufPos >= sizeof(mUartBuf)) {
				return; // buffer overflow
			}
			mUartBuf[mUartBufPos++] = rcvChar;
			if (mBufInterrupt) {
				if (mUartBufPos <= INTERFACES_BUF_SIZE) {
					arm_buf_timer();
				}
			}
			break;

		case DH_UART_MODE_IGNORE:
			break;
		}
	} else {
		WRITE_PERI_REG(UART_INTERUPTION_REGISTER, 0xffff);
	}
}


/*
 * dh_uart_init() implementation.
 */
int ICACHE_FLASH_ATTR dh_uart_init(int speed, int databits, char parity, int stopbits)
{
	if (speed < 300 || speed > 230400)
		return -1; // bad speed
	if (databits < 5 || databits > 8)
		return -1; // bad databits
	if (parity != 'N' && parity != 'O' && parity != 'E')
		return -1; // bad parity
	if (stopbits < 1 || stopbits > 2)
		return -1; // bad stopbits

	ETS_UART_INTR_DISABLE();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	gpio_output_set(0, 0, 0, BIT(1) | BIT(3));
	PIN_PULLUP_EN(PERIPHS_IO_MUX_U0RXD_U);
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);

	WRITE_PERI_REG(UART_DIV_REGISTER, UART_CLK_FREQ / speed);
	WRITE_PERI_REG(UART_CONFIGURATION_REGISTER0, 0);
	SET_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, ((parity == 'E') ? 0 : 1)  <<  0);
	SET_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, ((parity == 'N') ? 0 : 1)  <<  1);
	SET_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, (databits - 5)  <<  2);
	SET_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, ((stopbits == 2) ? 3 : 1)  <<  4);
	SET_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, BIT(17) | BIT(18));
	CLEAR_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, BIT(17) | BIT(18));
	WRITE_PERI_REG(UART_CONFIGURATION_REGISTER1, BIT(0) | BIT(16) | BIT(23));
	ETS_UART_INTR_ATTACH(int_cb, 0);
	WRITE_PERI_REG(UART_INTERUPTION_REGISTER, 0xffff);
	SET_PERI_REG_MASK(UART_INTERUPTION_ENABLE_REGISTER, BIT(0));
	ETS_UART_INTR_ENABLE();

	return 0; // OK
}


/**
 * @brief Recover the LED state.
 */
static void ICACHE_FLASH_ATTR led_recover(void *arg)
{
	ETS_INTR_LOCK();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
	gpio_output_set(0, 0b0110, 0b0110, 0);
	ETS_INTR_UNLOCK();
}


/**
 * @brief Turn the LED off.
 */
static void ICACHE_FLASH_ATTR led_off(void)
{
	gpio_output_set(0, 0, 0, 0b0110);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	os_delay_us(10000);
}


/*
 * dh_uart_leds() implementation.
 */
void ICACHE_FLASH_ATTR dh_uart_leds(DHUartLedsMode mode)
{
	if (mode == DH_UART_LEDS_ON) {
		mKeepLED = true;
		led_recover(0);
	} else {
		mKeepLED = false;
		led_off();
	}
}


/**
 * @brief Send one char to the UART.
 */
static void ICACHE_FLASH_ATTR send_char(int ch)
{
	while ((READ_PERI_REG(UART_STATUS_REGISTER) >> 16) & 0xFF);
	WRITE_PERI_REG(UART_BASE, ch);
}


/*
 * dh_uart_send_str() implementation.
 */
void ICACHE_FLASH_ATTR dh_uart_send_str(const char *str)
{
	if (mDataMode != DH_UART_MODE_PER_BYTE)
		return;

	if (mKeepLED) {
		os_timer_disarm(&mRecoverLEDTimer);
		led_off();
	}

	while (*str)
		send_char(*str++);

	if (mKeepLED) {
		os_timer_setfn(&mRecoverLEDTimer, led_recover, NULL);
		os_timer_arm(&mRecoverLEDTimer, 20, 0);
	}
}


/*
 * dh_uart_send_line() implementation.
 */
void ICACHE_FLASH_ATTR dh_uart_send_line(const char *str)
{
	dh_uart_send_str(str);
	dh_uart_send_str("\r\n");
}


/*
 * dh_uart_send_buf() implementation.
 */
void ICACHE_FLASH_ATTR dh_uart_send_buf(const void *buf_, size_t len)
{
	if (mDataMode != DH_UART_MODE_PER_BUF)
		return;

	if (mKeepLED) {
		os_timer_disarm(&mRecoverLEDTimer);
		led_off();
	}

	const char *buf = (const char*)buf_;
	while (len--) {
		system_soft_wdt_feed();
		send_char(*buf++);
	}

	if (mKeepLED) {
		os_timer_setfn(&mRecoverLEDTimer, led_recover, NULL);
		os_timer_arm(&mRecoverLEDTimer, 20, 0);
	}
}


/*
 * dh_uart_set_mode() implementation.
 */
void ICACHE_FLASH_ATTR dh_uart_set_mode(DHUartDataMode mode)
{
	mDataMode = mode;
	if (mode == DH_UART_MODE_PER_BUF) {
		mUartBufPos = 0;
	} else if(mode == DH_UART_MODE_PER_BYTE) {
		mBufInterrupt = false;
	}
}


/*
 * dh_uart_set_callback_timeout() implementation.
 */
void ICACHE_FLASH_ATTR dh_uart_set_callback_timeout(unsigned int timeout_ms)
{
	mTimeoutMs = timeout_ms;
}


/*
 * dh_uart_get_callback_timeout() implementation.
 */
unsigned int ICACHE_FLASH_ATTR dh_uart_get_callback_timeout(void)
{
	return mTimeoutMs;
}


/*
 * dh_uart_get_buf() implementation.
 */
size_t ICACHE_FLASH_ATTR dh_uart_get_buf(void **buf)
{
	*buf = mUartBuf;
	return mUartBufPos;
}


/*
 * dh_uart_reset_buf() implementation.
 */
void ICACHE_FLASH_ATTR dh_uart_reset_buf(void)
{
	mUartBufPos = 0;
}


/*
 * dh_uart_enable_buf_interrupt() implementation.
 */
void ICACHE_FLASH_ATTR dh_uart_enable_buf_interrupt(int enable)
{
	mBufInterrupt = enable;
}


#ifdef DH_COMMANDS_UART // UART command handlers
#include "dhcommand_parser.h"
#include "dhterminal.h"
#include <user_interface.h>

/**
 * @brief UART initialization helper.
 * @return Non-zero if UART was initialized. Zero otherwise.
 */
static int ICACHE_FLASH_ATTR uart_init(COMMAND_RESULT *cmd_res, ALLOWED_FIELDS fields,
                                       const gpio_command_params *params, int is_int)
{
	if (fields & AF_UARTMODE) {
		if (params->uart_speed == 0 && is_int) {
			dh_uart_enable_buf_interrupt(false);
			dh_command_done(cmd_res, "");
			return 1;
		} else if(!!dh_uart_init(params->uart_speed, params->uart_bits,
		                         params->uart_partity, params->uart_stopbits)) {
			dh_command_fail(cmd_res, "Wrong UART mode");
			return 1;
		}
	}

	return 0;
}

/*
 * dh_handle_uart_write() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_write(COMMAND_RESULT *cmd_res, const char *command,
                                            const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_UARTMODE | AF_DATA, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else if (!uart_init(cmd_res, fields, &info, false)) {
		dh_uart_set_mode(DH_UART_MODE_PER_BUF);
		dh_uart_send_buf(info.data, info.data_len);
		dh_command_done(cmd_res, "");
	}
}


/**
 * dh_handle_uart_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_read(COMMAND_RESULT *cmd_res, const char *command,
                                           const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	if (params_len) {
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0,
				AF_UARTMODE | AF_DATA | AF_TIMEOUT, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return;
		}
		if (uart_init(cmd_res, fields, &info, false))
			return;
		if (fields & AF_TIMEOUT) {
			if (info.timeout > 1000) { // TODO: MAX timeout constant
				dh_command_fail(cmd_res, "Timeout out of range");
				return;
			}
			if (!(fields & AF_DATA)) {
				dh_command_fail(cmd_res, "Timeout can be specified only with data");
				return;
			}
		}
	}

	if (fields & AF_DATA) {
		dh_uart_set_mode(DH_UART_MODE_PER_BUF);
		dh_uart_send_buf(info.data, info.data_len);
		system_soft_wdt_feed();
		delay_ms((fields & AF_TIMEOUT) ? info.timeout : 250);
		system_soft_wdt_feed();
	}

	void *buf = 0;
	size_t len = dh_uart_get_buf(&buf);
	if (len > INTERFACES_BUF_SIZE)
		len = INTERFACES_BUF_SIZE;
	dh_command_done_buf(cmd_res, buf, len);
	dh_uart_set_mode(DH_UART_MODE_PER_BUF);
}


/**
 * dh_handle_uart_int() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_int(COMMAND_RESULT *cmd_res, const char *command,
                                          const char *params, unsigned int params_len)
{
	if (params_len) {
		gpio_command_params info;
		ALLOWED_FIELDS fields = 0;
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, dh_uart_get_callback_timeout(),
				AF_UARTMODE | AF_TIMEOUT, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return;
		}
		if ((fields & AF_TIMEOUT) && info.timeout > 5000) { // TODO: MAX timeout constant
			dh_command_fail(cmd_res, "Timeout out of range");
			return;
		}
		if (uart_init(cmd_res, fields, &info, true))
			return;
		if (fields & AF_TIMEOUT)
			dh_uart_set_callback_timeout(info.timeout);
	}

	dh_uart_set_mode(DH_UART_MODE_PER_BUF);
	dh_uart_enable_buf_interrupt(true);
	dh_command_done(cmd_res, "");
}


/**
 * dh_handle_uart_terminal() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_terminal(COMMAND_RESULT *cmd_res, const char *command,
                                               const char *params, unsigned int params_len)
{
	if (params_len) {
		dh_command_fail(cmd_res, "No parameters expected");
		return;
	}

	dhterminal_init();
	dh_command_done(cmd_res, "");
}

#endif /* DH_COMMANDS_UART */
