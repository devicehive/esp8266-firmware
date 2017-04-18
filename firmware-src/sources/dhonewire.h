/**
 *	\file		dhonewire.h
 *	\brief		Software implementation of onewire interface for ESP8266.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHONEWIRE_H_
#define _DHONEWIRE_H_


/**
 *	\brief				Set pin for onewire, but not initialize it.
 *	\param[in]	cs_pin	Pin number.
 *	\return				Non zero value on success, zero on error.
 */
int dhonewire_set_pin(unsigned int pin);

/**
 *	\brief				Get pin that configured for onewire.
 *	\return				Pin number.
 */
int dhonewire_get_pin(void);

/**
 *	\brief					Write data to onewire bus.
 *	\details				Pin will be initialized automatically.
 *	\param[in]	buf			Buffer with data.
 *	\param[in]	len			Data length in bytes.
 *	\return					Non zero value on success, zero on error.
 */
int dhonewire_write(const char *buf, unsigned int len);

/**
 *	\brief					Read data from onewire bus.
 *	\details				Pin will NOT be initialized automatically, will use previous pin.
 *	\param[out]	buf			Buffer for data.
 *	\param[in]	len			Number of bytes to read, buffer have to be at least the same size.
 *	\return					Non zero value on success, zero on error.
 */
int dhonewire_read(char *buf, unsigned int len);

// command 0xF0 // SEARCH ROM

/**
 *	\brief					Search for onewire devices.
 *	\param[out]		buf		Buffer to store 64 bits addresses.
 *	\param[in,out]	len		Receives number of bytes allowed in buffer, returns number of copied bytes. Can return 0, that means no devices were found.
 *	\param[in]		command	Bus command for search, 0xF0 - SEARCH ROM, 0xEC - ALARM SEARCH.
 *	\param[in]		pin		Pin number for using during search.
 *	\return					Non zero value on success, zero on error.
 */
int dhonewire_search(char *buf, unsigned long *len, char command, unsigned int pin_number);

/**
 *	\brief						Enable interruption.
 *	\param[in]	search_pins		Pins that will awaiting for PRESENSE and then runs SEARCH command.
 *	\param[in]	disable_pins	Pin mask for disabling.
 *	\return						Non zero value on success, zero on error.
 */
int dhonewire_int(unsigned int search_pins, unsigned int disable_pins);

/**
 *	\brief						Callback for search result.
 *	\param[in]	is_ok			Non zero value on success, zero on error.
 *	\param[in]	pin_number		Pin number where found.
 *	\param[in]	buf				Buffer with 64 bits addresses.
 *	\param[in]	len				Buffer length in bytes.
 *	\return						Non zero value on success, zero on error.
 */
extern void dhonewire_search_result(unsigned int pin_number, char *buf, unsigned long len);

/**
 *	\brief				Read data from DHT like devices.
 *	\details			Checksum will not be checked.
 *	\param[out]	buf		Buffer for data.
 *	\param[in]	len		Buffer length in bytes.
 *	\return				Number of read bytes on success, zero on error.
 */
int dhonewire_dht_read(char *buf, unsigned int len);

/**
 *	\brief					Write data to WS2812b like device
 *	\param[in]	pin			1-wire pin for communication.
 *	\param[in]	buf			Pointer to buffer with data.
 *	\param[in]	len			Buffer length.
 */
void dhonewire_ws2812b_write(const char *buf, unsigned int len);

#endif /* _DHONEWIRE_H_ */
