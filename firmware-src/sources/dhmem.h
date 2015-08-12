/**
 *	\file		dhmem.h
 *	\brief		Memory manager helper.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 *	\details	Observer current memory status, and blocks firmware from generating new objects until memory will be free.
 */

#ifndef SOURCES_DHMEM_H_
#define SOURCES_DHMEM_H_

#include "dhrequest.h"

/**
 *	\brief				Allocate memory for request.
 *	\param[in]	size	Request size in bytes.
 *	\return				Pointer to allocated memory on success or NULL on failure.
 */
HTTP_REQUEST *dhmem_malloc_request(unsigned int size);

/**
 *	\brief				Free request memory.
 *	\param[in]	r		Pointer to request
 */
void dhmem_free_request(HTTP_REQUEST *r);

/**
 *	\brief		Check memory status good and new request allocation is possible.
 *	\return		Non zero if memory status is bad and allocation of new requests should be blocked. Zero otherwise.
 */
int dhmem_isblock();

/**
 *	\brief		Callback that will be called when allocation is possible again.
 */
extern void dhmem_unblock();

#endif /* SOURCES_DHMEM_H_ */
