/**
 *	\file		dnsd.h
 *	\brief		DNS structs and helper functions
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */


#ifndef _DNS_H_
#define _DNS_H_

#include <c_types.h>

/** Standart DNS port */
#define DNS_PORT 53
/** Standart MDNS port */
#define MDNS_PORT 5353
/** Standart MDNS multicast address */
#define MDNS_IP ((224 << 0) | (0 << 8) | (0 << 16) | (251 << 24))

/** DNS request header
 * all fields should be filled with big endian
 * */
typedef struct __attribute__((packed)) {
	uint16_t id;
	union {
		struct __attribute__((packed)) {
			unsigned recursionDesired : 1;
			unsigned truncatedMessage : 1;
			unsigned authoritiveAnswer : 1;
			unsigned opcode : 4;
			unsigned responseFlag : 1;
			unsigned rcode : 4;
			unsigned resevered : 2;
			unsigned primaryServer : 1;
			unsigned recursionAvailable : 1;
		} flags;
		uint16_t all_flags;
	};
	uint16_t questionsNumber;
	uint16_t answersNumber;
	uint16_t authoritiesNumber;
	uint16_t resourcesNumber;
	uint8_t data[];
} DNS_REQUEST;

/** DNS response header
 * all fields should be filled with big endian
 *  */
typedef struct __attribute__((packed)) {
	uint16_t nameOffset;
	uint16_t type;
	uint16_t class;
	uint32_t ttl;
	uint16_t ipSize;
	uint32_t ip;
} DNS_RESPONSE;

/**
 *	\brief				Convert host 32 bit value to 32 bit big endian value
 *	\param[in]	n		Host value.
 *	\return				Big endian value.
 */
uint32_t htobe_32(uint32_t n);

/**
 *	\brief				Convert host 16 bit value to 16 bit big endian value
 *	\param[in]	n		Host value.
 *	\return				Big endian value.
 */
uint16_t htobe_16(uint16_t n);

/**
 *	\brief				Convert 16 bit big endian value to host 16 bit value
 *	\param[in]	n		Big endian value.
 *	\return				Host value.
 */
uint16_t betoh_16(uint16_t n);

#endif /* _DNS_H_ */
