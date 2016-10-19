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
/** Maximum domain name length for this server, including zero terminated char */
#define DNS_MAX_DOMAIN_LENGTH 128

/** DNS header
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
} DNS_HEADER;

/** DNS response header
 * all fields should be filled with big endian
 *  */
typedef struct __attribute__((packed)) {
	uint16_t type;
	uint16_t class;
	uint32_t ttl;
	uint16_t size;
	uint8_t data[];
} DNS_ANSWER;

typedef enum {
	DNS_TYPE_A = 0x01,
	DNS_TYPE_NS = 0x02,
	DNS_TYPE_CNAME = 0x05,
	DNS_TYPE_SOA = 0x06,
	DNS_TYPE_WKS = 0x0B,
	DNS_TYPE_PTR = 0x0C,
	DNS_TYPE_MX = 0x0F,
	DNS_TYPE_SRV = 0x21
} DNS_TYPE;

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

/**
 *	\brief				Fill response with one record.
 *	\param[out]	buf		Pointer to store record.
 *	\param[in]	name	Name field. Can be NULL, default offset (0x0C, to first name in response) will be written then.
 *						If specified, '.local' domain will be added at tail and saved as QFDN. Can not be more then DNS_MAX_DOMAIN_LENGTH.
 *	\param[in]	type	One of DNS_TYPE enum.
 *	\param[in]	ttl		Time to live in seconds.
 *	\param[in]	size	Length of data in bytes.
 *	\param[in]	data	Data.
 *	\return				Number of bytes written to buf.
 */
uint32_t dns_add_answer(uint8_t *buf, const uint8_t *name, DNS_TYPE type,
		uint32_t ttl, uint32_t size, const uint8_t *data);

/**
 *	\brief				Compare two QFDN.
 *	\param[in]	a		Pointer to one QFDN string.
 *	\param[in]	b		Pointer to another QFDN string.
 *	\return				Zero if strings are equal, non zero otherwise.
 */
uint32_t dns_cmp_qfdn(const uint8_t *a, const uint8_t *b);

#endif /* _DNS_H_ */
