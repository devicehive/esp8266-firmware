/**
 *	\file		httpd.h
 *	\brief		Module for handling uploadable web page.
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef _UPLOADABLE_PAGE_H_
#define _UPLOADABLE_PAGE_H_

/** Uploadable page functions return status. */
typedef enum {
	UP_STATUS_OK, 				///< Successfully done.
	UP_STATUS_INTERNAL_ERROR,	///< Error while saving in ROM / no memory.
	UP_STATUS_WRONG_CALL,		///< Method is not allowed to call at this point.
	UP_STATUS_OVERFLOW			///< ROM storage is overflowed.
} UP_STATUS;

/**
 *	\brief				Get the content of uploadable page.
 *	\details			Content is stored in ROM, it can be read only per 4 bytes.
 *	\param[in]	data	Pointer where to store data length in bytes.
 *	\return				Pointer in ROM memory.
 */
const char *uploadable_page_get(unsigned int *len);

/**
 *	\brief				Destroy page data.
 *	\details			Physically page will be kept without first byte.
 *	\return				One of UP_STATUS statuses.
 */
UP_STATUS uploadable_page_delete();

/**
 *	\brief				Initialize flashing new web page procedure.
 *	\details			Pointer for writing will be moved to very beginning. This
 *						mode will be exited	if no writes operation happen in last
 *						UPLOADABLE_PAGE_TIMEOUT_MS.
 *	\return				One of UP_STATUS statuses.
 */
UP_STATUS uploadable_page_begin();

/**
 *	\brief					Write piece of data. Address increments internally.
 *	\details				Data is saved per 4 KiB blocks. Smaller buffer will be saved
 *							internally and will be written when 4 KiB is collected.
 *	\param[in]	data		Pointer to data.
 *	\param[in]	data_len	Data length.
 *	\return					One of UP_STATUS statuses.
 */
UP_STATUS uploadable_page_put(const char *data, unsigned int data_len);

/**
 *	\brief				Flash all remains data and leave out flashing procedure.
 *	\return				One of UP_STATUS statuses.
 */
UP_STATUS uploadable_page_finish();

#endif /* _UPLOADABLE_PAGE_H_ */
