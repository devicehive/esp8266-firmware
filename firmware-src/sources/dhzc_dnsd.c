/*
 * dhzc_dnsd.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "dhzc_dnsd.h"
#include "dns.h"
#include "swab.h"
#include "dhdebug.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>
#include <ets_forward.h>

#define MAX_CONNECTIONS 2

LOCAL unsigned int mDNSdConnected = 0;
LOCAL unsigned char mSendingInProgress = 0;
#define DNS_ANSWER_BUF_SIZE 1024
LOCAL char *mDNSAnswerBuffer;

LOCAL int ICACHE_FLASH_ATTR dnsd_answer(char *data, unsigned int len) {
	// always add response with host address data to the end
	DNS_HEADER *header = (DNS_HEADER *)data;
	header->answersNumber = htobe_u16(1);
	header->authoritiesNumber = 0;
	header->resourcesNumber = 0;
	header->flags.responseFlag = 1;
	header->flags.authoritiveAnswer = 0;
	header->flags.recursionAvailable = 1;
	header->flags.rcode = 0;

	struct ip_info info;
	if(wifi_get_ip_info(SOFTAP_IF, &info) == 0)
		info.ip.addr = 0;
	return len + dns_add_answer((uint8_t*)&data[len], NULL, NULL, DNS_TYPE_A, 60,
			sizeof(info.ip.addr), (uint8_t *)&info.ip.addr, NULL, NULL);
}

LOCAL void ICACHE_FLASH_ATTR dhzc_dnsd_disconnect_cb(void *arg) {
	mDNSdConnected--;
	dhdebug("dnsd disconnect, %d left", mDNSdConnected);
}

LOCAL void ICACHE_FLASH_ATTR dhzc_dnsd_sent_cb(void *arg) {
	struct espconn *conn = arg;
	mSendingInProgress = 0;
	if(conn->type & ESPCONN_TCP)
		espconn_disconnect(conn);
	dhdebug("dnsd sent");
}

LOCAL void ICACHE_FLASH_ATTR dhzc_dnsd_recv_cb(void *arg, char *data, unsigned short len) {
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

			if(espconn_send(conn, (uint8_t*)mDNSAnswerBuffer, rlen)) {
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

LOCAL void ICACHE_FLASH_ATTR dhzc_dnsd_reconnect_cb(void *arg, sint8 err) {
	mDNSdConnected--;
	dhdebug("dnsd connect error %d", err);
}

LOCAL void ICACHE_FLASH_ATTR dhzc_dnsd_connect_cb(void *arg) {
	struct espconn *conn = arg;
	mDNSdConnected++;
	if(mDNSdConnected > MAX_CONNECTIONS) {
		espconn_disconnect(conn);
		dhdebug("dnsd refuse");
		return;
	}
	espconn_regist_recvcb(conn, dhzc_dnsd_recv_cb);
	espconn_regist_disconcb(conn, dhzc_dnsd_disconnect_cb);
	espconn_regist_sentcb(conn, dhzc_dnsd_sent_cb);
	dhdebug("dnsd connected");
}

void ICACHE_FLASH_ATTR dhzc_dnsd_init(void) {
	mDNSAnswerBuffer = (char *)os_malloc(DNS_ANSWER_BUF_SIZE);

	esp_tcp *dnsdTcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	dnsdTcp->local_port = DNS_PORT;
	struct espconn *dnsdConnTCP = (struct espconn *)os_zalloc(sizeof(struct espconn ));
	dnsdConnTCP->type = ESPCONN_TCP;
	dnsdConnTCP->state = ESPCONN_NONE;
	dnsdConnTCP->proto.tcp = dnsdTcp;

	espconn_regist_connectcb(dnsdConnTCP, dhzc_dnsd_connect_cb);
	espconn_regist_reconcb(dnsdConnTCP, dhzc_dnsd_reconnect_cb);
	espconn_accept(dnsdConnTCP);

	esp_udp *dnsdUdp = (esp_udp *)os_zalloc(sizeof(esp_tcp));
	dnsdUdp->local_port = DNS_PORT;
	struct espconn *dnsdConnUDP = (struct espconn *)os_zalloc(sizeof(struct espconn ));
	dnsdConnUDP->type = ESPCONN_UDP;
	dnsdConnUDP->state = ESPCONN_NONE;
	dnsdConnUDP->proto.udp = dnsdUdp;

	espconn_regist_recvcb(dnsdConnUDP, dhzc_dnsd_recv_cb);
	espconn_regist_sentcb(dnsdConnUDP, dhzc_dnsd_sent_cb);
	espconn_create(dnsdConnUDP);
}
