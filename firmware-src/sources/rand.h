/**
 *	\file			rand.h
 *	\brief			Random implementation.
 *	\details		Pseudo-randomized numbers generator based on linear congruential generator.
 *	\author			Nikolay Khabarov
 *	\date			2015
 *	\copyright		Public Domain
 */

#ifndef _RAND_H_
#define _RAND_H_

/** Maximum random values count.*/
#define RAND_MAX 32767

/**
 *	\brief	Generate random value.
 *	\return	Value in range of 0..(RAND_MAX - 1).
 */
int rand();

/**
 *	\brief			Generate random device key.
 *	\details		Key length is variable.
 *	\param[out] buf	Pointer to buf where data will be copied. Should be at least 17 bytes. Pass zero to emulate and find out max size.
 *	\return			Number of char was copied to buf, excluding null terminated.
 */
unsigned int rand_generate_key(char *buf);

/**
 *	\brief			Generate random device id.
 *	\param[out] buf	Pointer to buf where data will be copied. Should be at least 20 bytes. Pass zero to emulate and find out max size.
 *	\return			Number of char was copied to buf, excluding null terminated, always 19.
 */
unsigned int rand_generate_deviceid(char *buf);

#endif /* _RAND_H_ */
