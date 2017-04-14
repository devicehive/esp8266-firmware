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
/** Maximum domain name octet length without zero terminated char for this server */
#define DNS_MAX_OCTET_LEN 60

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

/** DNS data for SRV record. */
typedef struct __attribute__((packed)) {
	uint16_t priority;
	uint16_t weigth;
	uint16_t port;
	uint8_t target[];
} SRV_DATA;

/** DNS record types enumeration. */
typedef enum {
	DNS_TYPE_A = 0x01,
	DNS_TYPE_NS = 0x02,
	DNS_TYPE_CNAME = 0x05,
	DNS_TYPE_SOA = 0x06,
	DNS_TYPE_WKS = 0x0B,
	DNS_TYPE_PTR = 0x0C,
	DNS_TYPE_MX = 0x0F,
	DNS_TYPE_TXT = 0x10,
	DNS_TYPE_SRV = 0x21
} DNS_TYPE;


/**
 *	\brief				Fill response with one record.
 *	\param[out]	buf		Pointer to store record.
 *	\param[in]	name1	Name field. Can be NULL, default offset (0x0C, to first name in response) will be written then and name2 ignored.
 *	\param[in]	name2	Name field. Will be saved FQDN encoded with top level domain 'local'.
 *	\param[in]	type	One of DNS_TYPE enum.
 *	\param[in]	ttl		Time to live in seconds.
 *	\param[in]	size1	Length of data in bytes.
 *	\param[in]	data1	First peace of data. Will be saved as is. Can be NULL.
 *	\param[in]	data2	Second piece of data.Should be zero terminated string. Will be saved FQDN encoded. Can be NULL.
 *	\param[in]	data3	Third piece of data.Should be zero terminated string. Will be saved FQDN encoded and with top level domain 'local'. Can be NULL.
 *	\return				Number of bytes written to buf.
 */
uint32_t dns_add_answer(uint8_t *buf, const uint8_t *name1,
		const uint8_t *name2, DNS_TYPE type, uint32_t ttl, uint32_t size1,
		const uint8_t *data1, const uint8_t *data2, const uint8_t *data3);

/**
 *	\brief				Compare FQDN with domain name.
 *	\details			String main contain of two parts. Should be without '.local' top level domain.
 *	\param[in]	fqdn	Pointer to FQDN string.
 *	\param[in]	str1	First part of str, before first dot.
 *	\param[in]	str2	Pointer to domain name to compare. Can be NULL.
 *	\return				Non zero if equal, zero otherwise.
 */
int dns_cmp_fqdn_str(const uint8_t *fqdn, const uint8_t *prefix, const uint8_t *str);

#endif /* _DNS_H_ */
