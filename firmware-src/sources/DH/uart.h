/**
 * @file
 * @brief UART HAL for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 *
 * UART Hardware access layer for ESP8266. Can operate in two modes: DH_UART_MODE_PER_BYTE or DH_UART_MODE_PER_BUF.
 *
 * In DH_UART_MODE_PER_BYTE all writing operations with strings are allowed, but data operations are disabled.
 * Each received byte cause dh_uart_char_rcv callback.
 *
 * In DH_UART_MODE_PER_BUF all writing operations with data are allowed, but char operations are disabled.
 * Callback dh_uart_buf_rcv will be called after number of byte in buffer is greater or equal
 * INTERFACES_BUF_SIZE or last byte was received later than specified timeout.
 */
#ifndef _DH_UART_H_
#define _DH_UART_H_

#include <c_types.h>
#include "user_config.h"

/**
 * @brief UART mode.
 */
typedef enum {
	DH_UART_MODE_IGNORE,     ///< @brief Ignore input data, can print everything.
	DH_UART_MODE_PER_BYTE,   ///< @brief Receive data byte by byte. @details Dedicated callback on each byte, some send data function is disabled.
	DH_UART_MODE_PER_BUF     ///< @brief Receive data per buffer. @details Dedicated callback with buffer with some timeout which disabled by default, some send data function is disabled.
} DHUartDataMode;


/**
 * @brief UART LED mode.
 */
typedef enum {
	DH_UART_LEDS_ON,
	DH_UART_LEDS_OFF
} DHUartLedsMode;


/**
 * @brief Initialize UART module.
 * @param[in] speed Bitrate. For example: 115200 or 19200.
 * @param[in] databits Number of bits in byte for UART. From 5 to 8.
 * @param[in] parity Use parity bit or not. Char value: 'N'(not), 'E'(even) or 'O'(odd).
 * @param[in] stopbits Number of stop bits: 1 or 2.
 * @return Zero on success.
 */
int dh_uart_init(int speed, int databits, char parity, int stopbits);


/**
 * @brief Keep TX LEDs always on.
 *
 * Some modules has LEDs which are connected to TX pins.
 * This method allows to keep this LEDs on.
 *
 * @param[in] on LEDs mode.
 */
void dh_uart_leds(DHUartLedsMode mode);


/**
 * @brief Write string to UART.
 * @warning Doesn't work in DH_UART_MODE_PER_BUF mode.
 * @param[in] str String, i.e. byte array that should be written.
 *                Will transmit buffer until first null char.
 */
void dh_uart_send_str(const char *str);


/**
 * @brief Write string to UART and add "\r\n" to the end.
 * @warning Doesn't work in DH_UART_MODE_PER_BUF mode.
 * @param[in] str String, i.e. byte array that should be written.
 *                Will transmit buffer until first null char.
 */
void dh_uart_send_line(const char *str);


/**
 * @brief Write byte array with specified size.
 * @warning Doesn't work in DH_UART_MODE_PER_BYTE mode.
 * @param[in] buf Byte array.
 * @param[in] len Number of bytes in buffer.
 */
void dh_uart_send_buf(const void *buf, size_t len);


/**
 * @brief Set current operating mode.
 * @details Buffer is cleaned up on setting DH_UART_MODE_PER_BUF mode.
 * @param[in] mode New operating mode.
 */
void dh_uart_set_mode(DHUartDataMode mode);


/**
 * @brief Set timeout for callback.
 *
 * This timeout means how many ms without receiving bytes
 * should pass before calling callback.
 * Make sense only when mode is DH_UART_MODE_PER_BUF and callback is enabled.
 *
 * @param[in] timeout_ms Timeout in milliseconds.
 */
void dh_uart_set_callback_timeout(unsigned int timeout_ms);


/**
 * @brief Get current callback timeout.
 * @return Timeout in milliseconds.
 */
unsigned int dh_uart_get_callback_timeout(void);


/**
 * @brief Get receiving buffer.
 * @param[out] buf Pointer where pointer to buffer is stored.
 * @return Number of bytes in buffer.
 */
size_t dh_uart_get_buf(void **buf);


/**
 * @brief Clear receiving buffer.
 */
void dh_uart_reset_buf(void);


/**
 * @brief Enable/disable DH_UART_MODE_PER_BUF callbacks.
 *
 * Callbacks are disabled by default.
 *
 * @param[in] enable Non zero value for enabling, zero for disabling.
  */
void dh_uart_enable_buf_interrupt(int enable);


/**
 * @brief Callback declaration for DH_UART_MODE_PER_BYTE mode.
 * @param[in] ch Received character.
 */
extern void dh_uart_char_rcv_cb(int ch);


/**
 * @brief Callback declaration for DH_UART_MODE_PER_BUF mode.
 * @param[in] buf Data that was received.
 * @param[in] len Size of data in bytes.
 */
extern void dh_uart_buf_rcv_cb(const void *buf, size_t len);


#ifdef DH_COMMANDS_UART // UART command handlers
#include "dhsender_data.h"


/**
 * @brief Handle "uart/write" command.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_write(COMMAND_RESULT *cmd_res, const char *command,
                                            const char *params, unsigned int params_len);


/**
 * @brief Handle "uart/read" command.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_read(COMMAND_RESULT *cmd_res, const char *command,
                                           const char *params, unsigned int params_len);


/**
 * @brief Handle "uart/int" command.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_int(COMMAND_RESULT *cmd_res, const char *command,
                                          const char *params, unsigned int params_len);


/**
 * @brief Handle "uart/terminal" command.
 */
void ICACHE_FLASH_ATTR dh_handle_uart_terminal(COMMAND_RESULT *cmd_res, const char *command,
                                               const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_UART */
#endif /* _DH_UART_H_ */
