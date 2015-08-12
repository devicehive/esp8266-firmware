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

#ifndef USER_DHUART_H_
#define USER_DHUART_H_

/** UART mode. */
typedef enum {
	DUM_PER_BYTE,	///< Receive data byte by byte.
	DUM_PER_BUF		///< Receive data per buffer.
} DHUART_DATA_MODE;

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
 *	\brief				Write string to UART.
 *	\details			Works only in DUM_PER_BYTE mode.
 *	\param[in]	str		String, ie bytes array that should be written. Will transmit buffer until first null char.
 */
void dhuart_send_str(const char *str);

/**
 *	\brief				Write string to UART and add "\r\n" to the end of string.
 *	\details			Works only in DUM_PER_BYTE mode.
 *	\param[in]	str		String, ie bytes array that should be written. Will transmit buffer until first null char.
 */
void dhuart_send_line(const char *str);

/**
 *	\brief				Write char array with specified size to UART.
 *	\details			Works only in DUM_PER_BUF mode.
 *	\param[in]	buf		Chars array.
 *	\param[in]	len		Number of chars in buf.
 */
void dhuart_send_buf(const char *buf, unsigned int len);

/**
 *	\brief				Set current operating mode.
 *	\param[in]	mode	New operating mode.
 *	\param[in]	timeout	Timeout in ms which should pass without receiving to generate callback with read bytes. Can be set only with DUM_PER_BUF mode.
 */
void dhuart_set_mode(DHUART_DATA_MODE mode, unsigned int timeout);

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
unsigned int dhuart_get_timeout();

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

#endif /* USER_DHUART_H_ */
