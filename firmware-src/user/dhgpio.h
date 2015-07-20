/*
 * dhgpio.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#ifndef USER_DHGPIO_H_
#define USER_DHGPIO_H_

#define DHGPIO_SUITABLE_PINS 0b1111000000111111 // GPIO0-GPIO5, GPIO12-GPIO15

#define PIN_GPIOO (1 << 0)
#define PIN_GPIO1 (1 << 1)
#define PIN_GPIO2 (1 << 2)
#define PIN_GPIO3 (1 << 3)
#define PIN_GPIO4 (1 << 4)
#define PIN_GPIO5 (1 << 5)
#define PIN_GPIO6 (1 << 6)
#define PIN_GPIO7 (1 << 7)
#define PIN_GPIO8 (1 << 8)
#define PIN_GPIO9 (1 << 9)
#define PIN_GPIO10 (1 << 10)
#define PIN_GPIO11 (1 << 11)
#define PIN_GPIO12 (1 << 12)
#define PIN_GPIO13 (1 << 13)
#define PIN_GPIO14 (1 << 14)
#define PIN_GPIO15 (1 << 15)


int dhgpio_write(unsigned int set_mask, unsigned int unset_mask);
int dhgpio_init(unsigned int init_mask, unsigned int pollup_mask, unsigned int nopoll_mask);
unsigned int dhgpio_read();
int dhgpio_int(unsigned int disable_mask, unsigned int rising_mask, unsigned int falling_mask, unsigned int both_mask, unsigned int low_mask, unsigned int high_mask);
int dhgpio_read_to_json(char *out, unsigned int value);

#endif /* USER_DHGPIO_H_ */
