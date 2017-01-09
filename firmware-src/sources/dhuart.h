/**
 *	\file		dhuart.h
 *	\brief		UART HAL for ESP8266.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 *	\details	UART Hardware access layer for ESP8266. Can operate in two modes: DUM_PER_BYTE or DUM_PER_BUF.
 *				In DUM_PER_BYTE all writing operations with strings are allowed, but data operations are disabled.
 *				Each received byte cause dhuart_char_rcv callback. In DUM_PER_BUF all writing operations with data
 *				are allowed, but char operations are disabled. Callback dhuart_buf_rcv will be called after number
 *				of byte in buffer is greate or eqaul INTERFACES_BUF_SIZE or last byte was received later than
 *				specified timeout.
 */

#ifndef _DHUART_H_
#define _DHUART_H_

/** UART mode. */
typedef enum {
	DUM_IGNORE,		///< Ignore input data, can print everything.
	DUM_PER_BYTE,	///< Receive data byte by byte, dedicated callback on each byte, some send data function is disabled.
	DUM_PER_BUF		///< Receive data per buffer, dedicated callback with buffer with some timeout which disabled by default, some send data function is disabled.
} DHUART_DATA_MODE;

typedef enum {
	DHUART_LEDS_ON,
	DHUART_LEDS_OFF
} DHUART_LEDS_MODE;

/**
 *	\brief					Initializes UART.
 *	\param[in]	speed		Bitrate. 115200 or 19200 for example.
 *	\param[in]	databits	Number of bits in byte for UART. From 5 to 8.
 *	\param[in]	parity		Use parity bit or not. Char value: N(not), E(even) or O(odd).
 *	\param[in]	stopbits 	Number of stop bits: 1 or 2.
 *	\return					Non zero value on success. Zero on error.
 */
int dhuart_init(unsigned int speed, unsigned int databits, char parity, unsigned int stopbits);

/**
 *	\brief					Keep TX LEDs always on.
 *	\details				Some modules has LEDs which are connected to TX pins. This method allows to keep this LEDs on.
 *	\param[in]	on			LEDs mode.
 */
void dhuart_leds(DHUART_LEDS_MODE mode);

/**
 *	\brief				Write string to UART.
 *	\details			Doesn't works only in DUM_PER_BUF mode.
 *	\param[in]	str		String, ie bytes array that should be written. Will transmit buffer until first null char.
 */
void dhuart_send_str(const char *str);

/**
 *	\brief				Write string to UART and add "\r\n" to the end of string.
 *	\details			Doesn't work in DUM_PER_BUF mode.
 *	\param[in]	str		String, ie bytes array that should be written. Will transmit buffer until first null char.
 */
void dhuart_send_line(const char *str);

/**
 *	\brief				Write char array with specified size to UART.
 *	\details			Doesn't works only in DUM_PER_BYTE mode.
 *	\param[in]	buf		Chars array.
 *	\param[in]	len		Number of chars in buf.
 */
void dhuart_send_buf(const char *buf, unsigned int len);

/**
 *	\brief				Set current operating mode.
 *	\details			Buffer is cleaned up on setting DUM_PER_BUF mode.
 *	\param[in]	mode	New operating mode.
 */
void dhuart_set_mode(DHUART_DATA_MODE mode);

/**
 *	\brief				Set timeout for callback.
 *	\details			This timeout means how many ms without receiving bytes should pass before calling callback.
 *						Make sense only when mode is DUM_PER_BUF and callback is enabled.
 *	\param[in]	timeout	Timeout in ms.
 */
void dhuart_set_callback_timeout(unsigned int timeout);

/**
 *	\brief				Get receiving buffer.
 *	\param[out]	buf		Pointer where pointer to buffer should be stored.
 *	\return				Number of bytes in buffer.
 */
unsigned int dhuart_get_buf(char ** buf);

/**
 *	\brief				Clean up buffer.
 */
void dhuart_reset_buf();

/**
 *	\brief				Enable or disable DUM_PER_BUF callbacks.
 *	\details			Disabled by default.
 *	\param[in]	enable	Non zero value for enabling, zero for disabling.
  */
void dhuart_enable_buf_interrupt(int enable);

/**
 *	\brief		Get current timeout value.
 *	\return		Timeout value in milliseconds.
 */
unsigned int dhuart_get_callback_timeout();

/**
 *	\brief			Callback declaration for DUM_PER_BYTE mode.
 *	\param[in]	c	Char that was received.
 */
extern void dhuart_char_rcv(char c);

/**
 *	\brief			Callback declaration for DUM_PER_BUF mode.
 *	\param[in]	buf	Data that was received.
 *	\param[in]	len	Size of data.
 */
extern void dhuart_buf_rcv(const char *buf, unsigned int len);

#endif /* _DHUART_H_ */
