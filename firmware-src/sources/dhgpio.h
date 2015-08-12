/**
 *	\file		dhgpio.h
 *	\brief		GPIO hardware access layer for ESP8266 firmware.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef USER_DHGPIO_H_
#define USER_DHGPIO_H_

/** Bitwise pin mask for pin that can be used. */
#define DHGPIO_SUITABLE_PINS 0b1111000000111111 // GPIO0-GPIO5, GPIO12-GPIO15
/** Last pin number. */
#define DHGPIO_MAXGPIONUM 15

/**
 *	\brief					Prepare pins for using as GPIO.
 *	\param[in]	pin_mask	Bitwise pin mask.
 *	\param[in]	disable_pwm	Disable PWM if it was enabled before.
 */
void dhgpio_prepare_pins(unsigned int pin_mask, int disable_pwm);

/**
 *	\brief							Enabling/disabling pull up switch for GPIO output.
 *	\param[in]	pin_mask_set_od		Bitwise pin mask for enabling.
 *	\param[in]	pin_mask_unset_od	Bitwise pin mask for disabling.
 */
void dhgpio_open_drain(unsigned int pin_mask_set_od, unsigned int pin_mask_unset_od);

/**
 *	\brief								Enabling/disabling pull up GPIO pins.
 *	\param[in]	pin_mask_pullup			Bitwise pin mask for enabling.
 *	\param[in]	pin_mask_disablepull	Bitwise pin mask for disabling.
 */
void dhgpio_pull(unsigned int pin_mask_pullup, unsigned int pin_mask_disablepull);

/**
 *	\brief					Set GPIO outputs state.
 *	\param[in]	set_mask	Bitwise pin mask for switching to high level.
 *	\param[in]	unset_mask	Bitwise pin mask for switching to low level.
 *	\return 				Non zero value on success, zero on failure.
 */
int dhgpio_write(unsigned int set_mask, unsigned int unset_mask);

/**
 *	\brief					Initializes GPIO pins for input.
 *	\param[in]	init_mask	Bitwise pin mask for pins that should be switched to input mode without touching pull up state.
 *	\param[in]	pollup_mask	Bitwise pin mask for pins that should be switched to input mode with enabled pull up.
 *	\param[in]	nopoll_mask	Bitwise pin mask for pins that should be switched to input mode with disable pull up.
 *	\return 				Non zero value on success, zero on failure.
 */
int dhgpio_init(unsigned int init_mask, unsigned int pollup_mask, unsigned int nopoll_mask);

/**
 *	\brief		Read GPIO pins state
 *	\return 	Bitwise bit mask of input.
 */
unsigned int dhgpio_read();

/**
 *	\brief						Enbale GPIO interruption.
 *	\param[in]	disable_mask	Bitwise pin mask for disabling interruption.
 *	\param[in]	rising_mask		Bitwise pin mask for rising edge interruption.
 *	\param[in]	falling_mask	Bitwise pin mask for falling edge interruption..
 *	\param[in]	both_mask		Bitwise pin mask for rising and falling edge interruption.
 *	\param[in]	timeout			Timeout in milliseconds, that will pass after interruption occurred. During this period module will collect data about interruptions on other pins.
 *	\return 					Non zero value on success, zero on failure.
 */
int dhgpio_int(unsigned int disable_mask, unsigned int rising_mask, unsigned int falling_mask, unsigned int both_mask, unsigned int timeout);

/**
 *	\brief			Get current GPIO inerruption timeout.
 *	\return 		Timeout in milliseconds.
 */
unsigned int dhgpio_get_timeout();

/**
 *	\brief					Interruption callback.
 *	\param[in] caused_pins	Bitwise pin mask with pins that was trigger interruption.
 */
extern void dhgpio_int_timeout(unsigned int caused_pins);

#endif /* USER_DHGPIO_H_ */
