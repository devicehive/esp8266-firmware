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
#define MAX_DOMAIN_LENGTH 128
LOCAL const uint8_t LOCALSTR[] = "local";
LOCAL const uint8_t MULTIFQDN[] = "\x9_services\x07_dns-sd\x04_udp\x05local";

LOCAL struct espconn mMDNSdConn = { 0 };
LOCAL struct ip_addr mMDNSAddr = { 0 };
LOCAL mSendingInProgress = 0;
LOCAL struct ip_addr mMulticastIP = { MDNS_IP };
LOCAL uint8_t mAnnounce[MAX_DOMAIN_LENGTH + sizeof(LOCALSTR) + 1 + sizeof(DNS_REQUEST) + sizeof(DNS_RESPONSE)];
LOCAL uint32_t mAnnounceLength = 0;
LOCAL const uint8_t *mFQDN = 0;
LOCAL uint32_t mFQDNLength = 0;

LOCAL void ICACHE_FLASH_ATTR announce() {
	dhdebug("mDNSd announce");
	mSendingInProgress = 1;

	*(unsigned long *)mMDNSdConn.proto.udp->remote_ip = MDNS_IP;
	mMDNSdConn.proto.udp->remote_port = MDNS_PORT;
	*(unsigned long *)mMDNSdConn.proto.udp->local_ip = mMulticastIP.addr;
	mMDNSdConn.proto.udp->local_port = MDNS_PORT;
	espconn_send(&mMDNSdConn, mAnnounce, mAnnounceLength);
}

LOCAL void ICACHE_FLASH_ATTR mdnsd_recv_cb(void *arg, char *data, unsigned short len) {
	if(len > 1024 || len < mFQDNLength || mSendingInProgress)
		return;

	DNS_REQUEST *req = (DNS_REQUEST *)data;
	if(req->id != 0 || req->all_flags !=0)
		return;

	uint16_t qd = betoh_16(req->questionsNumber);
	uint32_t i;
	uint32_t offset = 0;
	uint32_t data_len = len - sizeof(DNS_REQUEST);
	for(i = 0; i < qd && offset < data_len; i++) {
		uint32_t qdend = offset;
		while(req->data[qdend]) {
			qdend += req->data[qdend] + 1;
			if(qdend > data_len) // bad request
				return;
		}
		qdend++;

		if(qdend - offset == mFQDNLength) {
			if(os_memcmp(&req->data[offset], mFQDN, mFQDNLength) == 0) {
				announce();
				return;
			}
		}
		if(qdend - offset == sizeof(MULTIFQDN)) {
			if(os_memcmp(&req->data[offset], MULTIFQDN, sizeof(MULTIFQDN)) == 0) {
				announce();
				return;
			}
		}

		offset = qdend + 4; // four bytes for QTYPE and QCLASS
	}
}

LOCAL void ICACHE_FLASH_ATTR mdnsd_sent_cb(void *arg) {
	mSendingInProgress = 0;
}

int ICACHE_FLASH_ATTR mdnsd_start(const char *name, unsigned long addr) {
	static esp_udp mDNSdUdp;

	uint32_t namelen = os_strlen(name);
	if(namelen > MAX_DOMAIN_LENGTH - 1) {
		dhdebug("mDNS name too long");
		return 0;
	}

	// generate mDNS response packet
	os_memset(&mAnnounce, 0, sizeof(mAnnounce));
	DNS_REQUEST *req = (DNS_REQUEST *)mAnnounce;
	req->flags.authoritiveAnswer = 1;
	req->flags.responseFlag = 1;
	req->answersNumber = htobe_16( 1 );
	req->data[0] = namelen;
	os_memcpy(&req->data[1], name, namelen);
	mAnnounceLength = 1 + namelen;
	req->data[mAnnounceLength] = sizeof(LOCALSTR) - 1;
	os_memcpy(&req->data[1 + mAnnounceLength], LOCALSTR, sizeof(LOCALSTR) - 1);
	mAnnounceLength += sizeof(LOCALSTR);
	req->data[mAnnounceLength] = 0;
	mAnnounceLength++;
	// ignore nameOffset field for response
	DNS_RESPONSE *resp = (DNS_RESPONSE *)&req->data[mAnnounceLength
			- sizeof(((DNS_RESPONSE *)0)->nameOffset)];
	resp->type = htobe_16(1);  // A - host address
	resp->class = htobe_16(1); // IN - class
	resp->ttl = htobe_32(MDNS_TTL);
	resp->ipSize = htobe_16(sizeof(resp->ip));
	resp->ip = addr;
	mAnnounceLength += sizeof(DNS_REQUEST) + sizeof(DNS_RESPONSE) - sizeof(((DNS_RESPONSE *)0)->nameOffset);
	mFQDN = &req->data[0];
	mFQDNLength = namelen + 1 + sizeof(LOCALSTR) + 1;

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
	announce();
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
