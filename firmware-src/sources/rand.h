/**
 *	\file			rand.h
 *	\brief			Random implementation.
 *	\details		Pseudo-randomized numbers generator based on linear congruential generator.
 *	\author			Nikolay Khabarov
 *	\date			2015
 *	\copyright		Public Domain
 */

#ifndef USER_RAND_H_
#define USER_RAND_H_

/** Maximum random values count.*/
#define RAND_MAX 32767

/**
 *	\brief	Generate random value.
 *	\return	Value in range of 0..(RAND_MAX - 1).
 */
int rand();

#endif /* USER_RAND_H_ */
