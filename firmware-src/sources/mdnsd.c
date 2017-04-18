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
#include "swab.h"
#include "snprintf.h"
#include "dhdebug.h"
#include "user_config.h"

#define MDNS_TTL (60 * 60)
#define MDNS_MAX_PACKET_LENGTH 1024

LOCAL const uint8_t MDNS_SERVICE[] = MDNS_SERVICE_NAME;
LOCAL const uint16_t MDNS_SERVICE_PORT = HTTPD_PORT;
LOCAL const uint8_t MDNS_DISCOVERY[] = "_services._dns-sd._udp";
LOCAL const uint8_t *mName;
LOCAL unsigned long mAddr;
LOCAL struct espconn mMDNSdConn = { 0 };
LOCAL struct ip_addr mMDNSAddr = { 0 };
LOCAL esp_udp mDNSdUdp;
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
	if(len > MDNS_MAX_PACKET_LENGTH || len < sizeof(DNS_HEADER) || mSendingInProgress)
		return;

	DNS_HEADER *request = (DNS_HEADER *)data;
	if(request->id != 0 || request->all_flags !=0 )
		return;

	uint8_t responsebuff[MDNS_MAX_PACKET_LENGTH];
	DNS_HEADER *response = (DNS_HEADER *)responsebuff;

	uint16_t qd = betoh_u16(request->questionsNumber);
	uint32_t i;
	uint32_t offset = 0;
	uint32_t data_len = len - sizeof(DNS_HEADER);
	uint32_t alen = 0;
	uint32_t answersNumber = 0;
	for(i = 0; i < qd && offset < data_len; i++) {
		uint32_t qdend = offset;
		while(request->data[qdend]) {
			if((request->data[qdend] & 0xC0) == 0xC0) {
				offset = ((uint32_t)(request->data[qdend] & 0x3F)) * 0x100 + (uint32_t)request->data[qdend + 1];
				offset -= sizeof(DNS_HEADER);
				if(offset >= qdend) // bad request
					return;
				qdend++;
				break;
			}
			qdend += request->data[qdend] + 1;
			if(qdend > data_len) // bad request
				return;
		}
		qdend++;
		// check first byte of class and if FQDN was given by offset due wrong packet,
		// it makes sure that string is null terminated.
		if(request->data[qdend] == 0) {
			switch(request->data[qdend + 1]) {
			case DNS_TYPE_A:
				if(dns_cmp_fqdn_str(&request->data[offset], mName, NULL)) {
					alen += dns_add_answer(&response->data[alen], NULL, mName,
							DNS_TYPE_A,	MDNS_TTL, sizeof(mAddr), (uint8_t *) &mAddr,
							NULL, NULL);
					answersNumber++;
				}
				break;
			case DNS_TYPE_PTR:
				if(dns_cmp_fqdn_str(&request->data[offset], MDNS_DISCOVERY, NULL)) {
					alen += dns_add_answer(&response->data[alen], NULL,
							MDNS_DISCOVERY, DNS_TYPE_PTR, MDNS_TTL, 0, NULL, NULL,
							MDNS_SERVICE);
					answersNumber++;
				}else if(dns_cmp_fqdn_str(&request->data[offset], MDNS_SERVICE, NULL)) {
					alen += dns_add_answer(&response->data[alen], NULL,
							MDNS_SERVICE, DNS_TYPE_PTR, MDNS_TTL, 0, NULL, mName,
							MDNS_SERVICE);
					answersNumber++;
				}
				break;
			case DNS_TYPE_SRV:
				if(dns_cmp_fqdn_str(&request->data[offset], mName, MDNS_SERVICE)) {
					SRV_DATA srv;
					srv.port = htobe_u16( MDNS_SERVICE_PORT );
					srv.priority = 0;
					srv.weigth = 0;
					alen += dns_add_answer(&response->data[alen], mName,
							MDNS_SERVICE, DNS_TYPE_SRV, MDNS_TTL, sizeof(SRV_DATA),
							(uint8_t *)&srv, NULL, mName);
					answersNumber++;
				}
				break;
			case DNS_TYPE_TXT:
				if(dns_cmp_fqdn_str(&request->data[offset], mName, MDNS_SERVICE)) {
					static uint8_t txt[] = "\x00version = "FIRMWARE_VERSION;
					txt[0] = sizeof(txt) - 1;
					alen += dns_add_answer(&response->data[alen], mName,
							MDNS_SERVICE, DNS_TYPE_TXT, MDNS_TTL, sizeof(txt),
							txt, NULL, NULL);
					answersNumber++;
				}
				break;
			default:
				break;
			}
		}

		if(alen > sizeof(responsebuff)) {
			dhdebug("ERROR: mdns response buffer overflow %d/%d - %d answers",
					alen, sizeof(responsebuff), answersNumber);
			return;
		}

		offset = qdend + 4; // four bytes for QTYPE and QCLASS
	}

	if(answersNumber) {
		// if we have answer(s), fill dns header and send
		os_memset(response, 0, sizeof(DNS_HEADER));
		response->flags.authoritiveAnswer = 1;
		response->flags.responseFlag = 1;
		response->answersNumber = htobe_u16(answersNumber);
		announce(responsebuff, alen + sizeof(DNS_HEADER));
	}
}

LOCAL void ICACHE_FLASH_ATTR mdnsd_sent_cb(void *arg) {
	mSendingInProgress = 0;
}

int ICACHE_FLASH_ATTR mdnsd_start(const char *name, unsigned long addr) {
	if(mMDNSdConn.proto.udp)
		mdnsd_stop();

	mName = name;
	mAddr = addr;

	mMDNSAddr.addr = addr;
	mDNSdUdp.local_port = MDNS_PORT;
	mMDNSdConn.type = ESPCONN_UDP;
	mMDNSdConn.state = ESPCONN_NONE;
	mMDNSdConn.proto.udp = &mDNSdUdp;
	mMDNSdConn.reverse = NULL;
	mSendingInProgress = 0;

	if(espconn_regist_recvcb(&mMDNSdConn, mdnsd_recv_cb) ||
		espconn_regist_sentcb(&mMDNSdConn, mdnsd_sent_cb)) {
		mMDNSdConn.proto.udp = NULL;
		dhdebug("mDNSd failed to start");
		return 0;
	}
	if(espconn_igmp_join(&mMDNSAddr, &mMulticastIP)) {
		mMDNSdConn.proto.udp = NULL;
		dhdebug("mDNSd failed to join igmp group");
		return 0;
	}
	if(espconn_create(&mMDNSdConn)) {
		espconn_igmp_leave(&mMDNSAddr, &mMulticastIP);
		mMDNSdConn.proto.udp = NULL;
		dhdebug("mDNS failed to listen UDP socket");
		return 0;
	}
	dhdebug("mDNSd is started");
	return 1;
}

void ICACHE_FLASH_ATTR mdnsd_stop(void) {
	if(mMDNSdConn.proto.udp) {
		espconn_disconnect(&mMDNSdConn);
		espconn_delete(&mMDNSdConn);
		espconn_igmp_leave(&mMDNSAddr, &mMulticastIP);
		mMDNSdConn.proto.udp = NULL;
		dhdebug("mDNSd is stopped");
	}
}
