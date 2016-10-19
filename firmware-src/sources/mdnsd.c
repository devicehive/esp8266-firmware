/*
 * mdnsd.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include <ip_addr.h>
#include "mdnsd.h"
#include "dns.h"
#include "dhdebug.h"

#define MDNS_TTL (60 * 60)

LOCAL const  uint8_t SERVICE_DISCOVERY_QFDN[] = "\x09_services\x07_dns-sd\x04_udp\x05local";

LOCAL struct espconn mMDNSdConn = { 0 };
LOCAL struct ip_addr mMDNSAddr = { 0 };
LOCAL mSendingInProgress = 0;
LOCAL struct ip_addr mMulticastIP = { MDNS_IP };
LOCAL uint8_t mAnnounce[DNS_MAX_DOMAIN_LENGTH + 7 + sizeof(DNS_HEADER) + sizeof(DNS_ANSWER)];
LOCAL uint32_t mAnnounceLength = 0;

LOCAL void ICACHE_FLASH_ATTR announce(const uint8_t *data, uint32_t len) {
	mSendingInProgress = 1;

	*(unsigned long *)mMDNSdConn.proto.udp->remote_ip = MDNS_IP;
	mMDNSdConn.proto.udp->remote_port = MDNS_PORT;
	*(unsigned long *)mMDNSdConn.proto.udp->local_ip = mMulticastIP.addr;
	mMDNSdConn.proto.udp->local_port = MDNS_PORT;
	espconn_send(&mMDNSdConn, (uint8_t *)data, len);
}

LOCAL void ICACHE_FLASH_ATTR mdnsd_recv_cb(void *arg, char *data, unsigned short len) {
	if(len > 1024 || len < sizeof(DNS_HEADER) || mSendingInProgress)
		return;

	DNS_HEADER *req = (DNS_HEADER *)data;
	if(req->id != 0 || req->all_flags !=0 )
		return;

	uint16_t qd = betoh_16(req->questionsNumber);
	uint32_t i;
	uint32_t offset = 0;
	uint32_t data_len = len - sizeof(DNS_HEADER);
	for(i = 0; i < qd && offset < data_len; i++) {
		uint32_t qdend = offset;
		while(req->data[qdend]) {
			qdend += req->data[qdend] + 1;
			if(qdend > data_len) // bad request
				return;
		}
		qdend++;

		if(dns_cmp_qfdn(((DNS_HEADER*)&mAnnounce)->data, &req->data[offset]) == 0) {
			announce(mAnnounce, mAnnounceLength);
			continue;
		}

		offset = qdend + 4; // four bytes for QTYPE and QCLASS
	}
}

LOCAL void ICACHE_FLASH_ATTR mdnsd_sent_cb(void *arg) {
	mSendingInProgress = 0;
}

int ICACHE_FLASH_ATTR mdnsd_start(const char *name, unsigned long addr) {
	static esp_udp mDNSdUdp;

	// generate mDNS response packet
	os_memset(&mAnnounce, 0, sizeof(mAnnounce));
	DNS_HEADER *req = (DNS_HEADER *)mAnnounce;
	req->flags.authoritiveAnswer = 1;
	req->flags.responseFlag = 1;
	req->answersNumber = htobe_16( 1 );

	mAnnounceLength = dns_add_answer(req->data, name,
			DNS_TYPE_A, MDNS_TTL, sizeof(addr), (uint8_t *)&addr);
	if(mAnnounceLength == 0) {
		dhdebug("Name is too long");
		return 0;
	}
	mAnnounceLength += sizeof(DNS_HEADER);

	mMDNSAddr.addr = addr;
	mDNSdUdp.local_port = MDNS_PORT;
	mMDNSdConn.type = ESPCONN_UDP;
	mMDNSdConn.state = ESPCONN_NONE;
	mMDNSdConn.proto.udp = &mDNSdUdp;
	mMDNSdConn.reverse = NULL;
	mSendingInProgress = 0;

	if(espconn_regist_recvcb(&mMDNSdConn, mdnsd_recv_cb) == 0) {
		if(espconn_regist_sentcb(&mMDNSdConn, mdnsd_sent_cb) == 0) {
			if(espconn_igmp_join(&mMDNSAddr, &mMulticastIP) == 0) {
				if(espconn_create(&mMDNSdConn)) {
					espconn_igmp_leave(&mMDNSAddr, &mMulticastIP);
					mMDNSdConn.proto.udp = NULL;
					dhdebug("mDNS failed to UDP listen socket");
					return 0;
				}
			}
		}
	}

	dhdebug("mDNSd is started");
	return 1;
}

void ICACHE_FLASH_ATTR mdnsd_stop() {
	if(mMDNSdConn.proto.udp) {
		espconn_disconnect(&mMDNSdConn);
		espconn_delete(&mMDNSdConn);
		espconn_igmp_leave(&mMDNSAddr, &mMulticastIP);
		mMDNSdConn.proto.udp = NULL;
	}
	dhdebug("mDNSd is stopped");
}
