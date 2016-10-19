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
#include "snprintf.h"
#include "dhdebug.h"

#define MDNS_TTL (60 * 60)

LOCAL const  uint8_t MDNS_SERVICE[] = "_esp8266-devicehive._tcp";
LOCAL const  uint8_t MDNS_DISCOVERY[] = "_services._dns-sd._udp";
LOCAL const uint8_t *mName;
LOCAL unsigned long mAddr;
LOCAL struct espconn mMDNSdConn = { 0 };
LOCAL struct ip_addr mMDNSAddr = { 0 };
LOCAL mSendingInProgress = 0;
LOCAL struct ip_addr mMulticastIP = { MDNS_IP };

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

	DNS_HEADER *request = (DNS_HEADER *)data;
	if(request->id != 0 || request->all_flags !=0 )
		return;

	uint8_t responsebuff[512];
	DNS_HEADER *response = (DNS_HEADER *)responsebuff;
	os_memset(response, 0, sizeof(DNS_HEADER));
	response->flags.authoritiveAnswer = 1;
	response->flags.responseFlag = 1;
	response->answersNumber = htobe_16( 1 );

	uint16_t qd = betoh_16(request->questionsNumber);
	uint32_t i;
	uint32_t offset = 0;
	uint32_t data_len = len - sizeof(DNS_HEADER);
	for(i = 0; i < qd && offset < data_len; i++) {
		uint32_t qdend = offset;
		while(request->data[qdend]) {
			qdend += request->data[qdend] + 1;
			if(qdend > data_len) // bad request
				return;
		}
		qdend++;
dhdebug("mdns got %s", &request->data[offset]);
		if(dns_cmp_fqdn_str(&request->data[offset], mName)) {
			uint32_t alen = dns_add_answer(response->data, mName, NULL,
					DNS_TYPE_A,	MDNS_TTL, sizeof(mAddr), (uint8_t *) &mAddr,
					NULL);
			announce(responsebuff, alen + sizeof(DNS_HEADER));
			return;
		}
		if(request->data[qdend] == 0 && request->data[qdend + 1] == DNS_TYPE_PTR) {
			if(dns_cmp_fqdn_str(&request->data[offset], MDNS_DISCOVERY)) {
				uint32_t alen = dns_add_answer(response->data, MDNS_DISCOVERY,
						NULL, DNS_TYPE_PTR, MDNS_TTL, 0, NULL, MDNS_SERVICE);
				announce(responsebuff, alen + sizeof(DNS_HEADER));
				return;
			}

			if(dns_cmp_fqdn_str(&request->data[offset], MDNS_SERVICE)) {
				SRV_DATA srv;
				srv.port = htobe_16( 80 );
				srv.priority = 0;
				srv.weigth = 0;
				uint32_t alen = dns_add_answer(response->data, mName,
						MDNS_SERVICE, DNS_TYPE_SRV, MDNS_TTL, sizeof(SRV_DATA),
						(uint8_t *)&srv, mName);
				response->resourcesNumber = htobe_16( 1 );
				alen += dns_add_answer(&response->data[alen], mName, NULL,
						DNS_TYPE_A,	MDNS_TTL, sizeof(mAddr), (uint8_t *)&mAddr,
						NULL);
				announce(responsebuff, alen + sizeof(DNS_HEADER));
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
	if(os_strlen(name) >= DNS_MAX_DOMAIN_LENGTH)
		return 0;

	mName = name;
	mAddr = addr;

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
