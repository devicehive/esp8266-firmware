/**
 *	\file		dhterminal.h
 *	\brief		Unix like terminal implementation.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHTERMINAL_H_
#define _DHTERMINAL_H_
#include <stdarg.h>

/** Current terminal mode. */
typedef enum {
	SM_NORMAL_MODE = 0,		///< Command input mode.
	SM_DEBUG_MODE,			///< Debug print mode.
	SM_AWATING_MODE,		///< Awaiting for command result.
	SM_OUTPUT_MODE,			///< Performing command that print to terminal.
	SM_INPUT_MODE,			///< Ask user to type line.
	SM_HIDDEN_INPUT_MODE	///< Ask user to type line, but hide input chracters.
} DHTERMINAL_MODE;

/** Callback prototype for input operation. */
typedef void (*Input_Call_Back)(const char *line);
/** Callback prototype for autocompleter. */
typedef char * (*Input_Autocompleter)(const char *pattern);
/** Callback prototype for input filter. */
typedef int (*Input_Filter_Call_Back)(char c);


/**
 *	\brief		Initializes terminal.
 */
void dhterminal_init();

/**
 *	\brief				Print data in debug.
 *	\details 			If mode is SM_DEBUG_MODE, data will be printed in terminal, otherwise it will be save in inner buffer.
 *	\param[in]	pFormat	Format.
 *	\param[in]	ap		Data for format.
 */
void dhterminal_debug(const char *pFormat, va_list ap);

/**
 *	\brief		Get terminal history.
 *	\return 	Pointer to char array. Each command ends with null terminated char, whole buffer ends with two null terminated chars.
 */
const char *dhterminal_get_history();

/**
 *	\brief		Get debug lines buffer.
 *	\return 	Pointer to buffer that end with null terminated char.
 */
const char *dhterminal_get_debug_ouput();

/**
 *	\brief		Get current mode.
 *	\return 	Current mode.
 */
DHTERMINAL_MODE dhterminal_get_mode();

/**
 *	\brief					Set current mode.
 *	\details				Callbacks and maxlength make sense in SM_INPUT_MODE and SM_HIDDEN_INPUT_MODE.
 *	\param[in]	mode		Format.
 *	\param[in]	inputcb		Callback for function that will be called whec user finish typing.
 *	\param[in]	complcb		Callback for autocompleter when user press Tab button. Can be NULL.
 *	\param[in]	filtercb	Callback for function that filters input characters. Can be NULL.
 *	\param[in]	maxlength	Maximum lenght of input line.
 */
void dhterminal_set_mode(DHTERMINAL_MODE mode, Input_Call_Back inputcb, Input_Autocompleter complcb, Input_Filter_Call_Back filtercb, int maxlength);

/**
 *	\brief				Replace current terminal input line.
 *	\param[in]	line	New line.
 */
void dhterminal_set_input(const char *line);

#endif /* _DHTERMINAL_H_ */
