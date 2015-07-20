/*
 * dhserial.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#ifndef USER_DHSERIAL_H_
#define USER_DHSERIAL_H_
#include <stdarg.h>

typedef enum {
	SM_NORMAL_MODE = 0,
	SM_DEBUG_MODE,
	SM_AWATING_MODE,
	SM_OUTPUT_MODE,
	SM_INPUT_MODE,
	SM_HIDDEN_INPUT_MODE
} DHSERIAL_MODE;

typedef void (*Input_Call_Back)(const char *line);
typedef char * (*Input_Autocompleater)(const char *pattern);

void dhserial_init();
void dhserial_debug(const char *pFormat, va_list ap);

const char *dhserial_get_history();
const char *dhserial_get_debug_ouput();
DHSERIAL_MODE dhserial_get_mode();
void dhserial_set_mode(DHSERIAL_MODE mode, Input_Call_Back inputcb, Input_Autocompleater complcb);
void dhserial_set_input(const char *line);

#endif /* USER_DHSERIAL_H_ */
