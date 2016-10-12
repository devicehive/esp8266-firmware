/*
 * dhap_dnsd.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>
#include "dhap_dnsd.h"
#include "dhdebug.h"

#define MAX_CONNECTIONS 2
#define DNSD_PORT 53

LOCAL unsigned int mDNSdConnected = 0;
LOCAL unsigned char mSendingInProgress = 0;
#define DNS_ANSWER_BUF_SIZE 1024
LOCAL char *mDNSAnswerBuffer;

// struct declaration for little endian systems
// all fields should be filled with big endian
typedef struct __attribute__((packed)) {
	uint16_t id;
	unsigned recursionDesired : 1;
	unsigned truncatedMessage : 1;
	unsigned authoritiveAnswer : 1;
	unsigned opcode : 4;
	unsigned responseFlag : 1;
	unsigned rcode : 4;
	unsigned resevered : 2;
	unsigned primaryServer : 1;
	unsigned recursionAvailable : 1;
	uint16_t questionsNumber;
	uint16_t answersNumber;
	uint16_t authoritiesNumber;
	uint16_t resourcesNumber;
} DNS_HEADER;

typedef struct __attribute__((packed)) {
	uint16_t nameOffset;
	uint16_t type;
	uint16_t class;
	uint32_t ttl;
	uint16_t ipSize;
	uint32_t ip;
} DNS_ANSWER;

LOCAL uint16_t ICACHE_FLASH_ATTR htobe_16(uint16_t n) {
	uint32_t res;
	uint8_t *p = (uint8_t *) &res;
	p[1] = n % 0x100;
	n /= 0x100;
	p[0] = n % 0x100;
	return res;
}

LOCAL int ICACHE_FLASH_ATTR dnsd_answer(char *data, unsigned int len) {
	// always add response with host address data to the end
	DNS_HEADER *header = (DNS_HEADER *)data;
	DNS_ANSWER *answer = (DNS_ANSWER *)(&data[len]);
	header->answersNumber = htobe_16(1);
	header->authoritiesNumber = 0;
	header->resourcesNumber = 0;
	header->responseFlag = 1;
	header->authoritiveAnswer = 0;
	header->recursionAvailable = 1;
	header->rcode = 0;

	answer->nameOffset = htobe_16(0xC000 | sizeof(DNS_HEADER));
	answer->type = htobe_16(1);  // A - host address
	answer->class = htobe_16(1); // IN - class
	answer->ttl = 0;
	answer->ipSize = htobe_16(sizeof(answer->ip));
	answer->ip = 0;

	struct ip_info info;
	if(wifi_get_ip_info(SOFTAP_IF, &info))
		answer->ip = info.ip.addr;

	return len + sizeof(DNS_ANSWER);
}

LOCAL void ICACHE_FLASH_ATTR dhap_dnsd_disconnect_cb(void *arg) {
	mDNSdConnected--;
	dhdebug("dnsd disconnect, %d left", mDNSdConnected);
}

LOCAL void ICACHE_FLASH_ATTR dhap_dnsd_sent_cb(void *arg) {
	struct espconn *conn = arg;
	mSendingInProgress = 0;
	if(conn->type & ESPCONN_TCP)
		espconn_disconnect(conn);
	dhdebug("dnsd sent");
}

LOCAL void ICACHE_FLASH_ATTR dhap_dnsd_recv_cb(void *arg, char *data, unsigned short len) {
	struct espconn *conn = arg;
	if(mSendingInProgress || len + sizeof(DNS_ANSWER) > DNS_ANSWER_BUF_SIZE) {
		if(mSendingInProgress)
			dhdebug("Got %u bytes, but dnsd is busy, drop", len);
		else
			dhdebug("Got %u bytes, too big for this dnsd", len);
		if(conn->type & ESPCONN_TCP)
			espconn_disconnect(conn);
		return;
	}
	dhdebug("dnsd received %d bytes", len);
	os_memcpy(mDNSAnswerBuffer, data, len);
	unsigned int rlen = dnsd_answer(mDNSAnswerBuffer, len);
	if(rlen) {
		mSendingInProgress = 1;

		remot_info *premot = NULL;
		if(espconn_get_connection_info(conn, &premot, 0) == ESPCONN_OK) {
			if(conn->type & ESPCONN_TCP) {
				conn->proto.tcp->remote_port = premot->remote_port;
				os_memcpy(conn->proto.tcp->remote_ip, premot->remote_ip,
						sizeof(premot->remote_ip));
			}
			if(conn->type & ESPCONN_UDP) {
				conn->proto.udp->remote_port = premot->remote_port;
				os_memcpy(conn->proto.udp->remote_ip, premot->remote_ip,
						sizeof(premot->remote_ip));
			}

			if(espconn_send(conn, mDNSAnswerBuffer, rlen)) {
				dhdebug("Failed to send response");
				mSendingInProgress = 0;
				if(conn->type & ESPCONN_TCP)
					espconn_disconnect(conn);
			}
		} else {
			dhdebug("Failed to get connection info");
		}
	} else {
		dhdebug("Wrong dns request");
		if(conn->type & ESPCONN_TCP)
			espconn_disconnect(conn);
	}
}

LOCAL void ICACHE_FLASH_ATTR dhap_dnsd_reconnect_cb(void *arg, sint8 err) {
	mDNSdConnected--;
	dhdebug("dnsd connect error %d", err);
}

LOCAL void ICACHE_FLASH_ATTR dhap_dnsd_connect_cb(void *arg) {
	struct espconn *conn = arg;
	mDNSdConnected++;
	if(mDNSdConnected > MAX_CONNECTIONS) {
		espconn_disconnect(conn);
		dhdebug("dnsd refuse");
		return;
	}
	espconn_regist_recvcb(conn, dhap_dnsd_recv_cb);
	espconn_regist_disconcb(conn, dhap_dnsd_disconnect_cb);
	espconn_regist_sentcb(conn, dhap_dnsd_sent_cb);
	dhdebug("dnsd connected");
}

void ICACHE_FLASH_ATTR dhap_dnsd_init() {
	mDNSAnswerBuffer = (char *)os_malloc(DNS_ANSWER_BUF_SIZE);

	esp_tcp *dnsdTcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	dnsdTcp->local_port = DNSD_PORT;
	struct espconn *dnsdConnTCP = (struct espconn *)os_zalloc(sizeof(struct espconn ));
	dnsdConnTCP->type = ESPCONN_TCP;
	dnsdConnTCP->state = ESPCONN_NONE;
	dnsdConnTCP->proto.tcp = dnsdTcp;

	espconn_regist_connectcb(dnsdConnTCP, dhap_dnsd_connect_cb);
	espconn_regist_reconcb(dnsdConnTCP, dhap_dnsd_reconnect_cb);
	espconn_accept(dnsdConnTCP);

	esp_udp *dnsdUdp = (esp_udp *)os_zalloc(sizeof(esp_tcp));
	dnsdUdp->local_port = DNSD_PORT;
	struct espconn *dnsdConnUDP = (struct espconn *)os_zalloc(sizeof(struct espconn ));
	dnsdConnUDP->type = ESPCONN_UDP;
	dnsdConnUDP->state = ESPCONN_NONE;
	dnsdConnUDP->proto.udp = dnsdUdp;

	espconn_regist_recvcb(dnsdConnUDP, dhap_dnsd_recv_cb);
	espconn_regist_sentcb(dnsdConnUDP, dhap_dnsd_sent_cb);
	espconn_create(dnsdConnUDP);
}
