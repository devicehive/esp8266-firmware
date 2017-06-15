/**
 * @file
 * @brief UART command handler.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/uart_cmd.h"
#include "DH/uart.h"
#include "DH/adc.h"

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
