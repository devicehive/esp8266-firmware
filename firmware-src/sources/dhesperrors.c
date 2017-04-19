/*
 * dhesperrors.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for printing error description in debug output
 *
 */
#include "dhesperrors.h"
#include "dhdebug.h"

#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>

void ICACHE_FLASH_ATTR dhesperrors_disconnect_reason(const char *descrption, uint8 reason) {
	char *errdescr = 0;
	switch(reason) {
	case REASON_UNSPECIFIED:
		errdescr = "UNSPECIFIED";
		break;
	case REASON_AUTH_EXPIRE:
		errdescr = "AUTH_EXPIRE";
		break;
	case REASON_AUTH_LEAVE:
		errdescr = "AUTH_LEAVE";
		break;
	case REASON_ASSOC_EXPIRE:
		errdescr = "ASSOC_EXPIRE";
		break;
	case REASON_ASSOC_TOOMANY:
		errdescr = "ASSOC_TOOMANY";
		break;
	case REASON_NOT_AUTHED:
		errdescr = "NOT_AUTHED";
		break;
	case REASON_NOT_ASSOCED:
		errdescr = "NOT_ASSOCED";
		break;
	case REASON_ASSOC_LEAVE:
		errdescr = "ASSOC_LEAVE";
		break;
	case REASON_ASSOC_NOT_AUTHED:
		errdescr = "ASSOC_NOT_AUTHED";
		break;
	case REASON_DISASSOC_PWRCAP_BAD:
		errdescr = "DISASSOC_PWRCAP_BAD";
		break;
	case REASON_DISASSOC_SUPCHAN_BAD:
		errdescr = "DISASSOC_SUPCHAN_BAD";
		break;
	case REASON_IE_INVALID:
		errdescr = "IE_INVALID";
		break;
	case REASON_MIC_FAILURE:
		errdescr = "MIC_FAILURE";
		break;
	case REASON_4WAY_HANDSHAKE_TIMEOUT:
		errdescr = "4WAY_HANDSHAKE_TIMEOUT";
		break;
	case REASON_GROUP_KEY_UPDATE_TIMEOUT:
		errdescr = "GROUP_KEY_UPDATE_TIMEOUT";
		break;
	case REASON_IE_IN_4WAY_DIFFERS:
		errdescr = "IE_IN_4WAY_DIFFERS";
		break;
	case REASON_GROUP_CIPHER_INVALID:
		errdescr = "GROUP_CIPHER_INVALID";
		break;
	case REASON_PAIRWISE_CIPHER_INVALID:
		errdescr = "PAIRWISE_CIPHER_INVALID";
		break;
	case REASON_AKMP_INVALID:
		errdescr = "AKMP_INVALID";
		break;
	case REASON_UNSUPP_RSN_IE_VERSION:
		errdescr = "UNSUPP_RSN_IE_VERSION";
		break;
	case REASON_INVALID_RSN_IE_CAP:
		errdescr = "INVALID_RSN_IE_CAP";
		break;
	case REASON_802_1X_AUTH_FAILED:
		errdescr = "802_1X_AUTH_FAILED";
		break;
	case REASON_CIPHER_SUITE_REJECTED:
		errdescr = "CIPHER_SUITE_REJECTED";
		break;
	case REASON_BEACON_TIMEOUT:
		errdescr = "BEACON_TIMEOUT";
		break;
	case REASON_NO_AP_FOUND:
		errdescr = "NO_AP_FOUND";
		break;
	}
	if(errdescr)
		dhdebug("%s %s", descrption, errdescr);
	else
		dhdebug("%s %d", descrption, reason);
}

void ICACHE_FLASH_ATTR dhesperrors_espconn_result(const char *descrption, int reason) {
	char *errdescr = 0;
	switch(reason) {
	case ESPCONN_OK:
		errdescr = "No error";
		break;
	case ESPCONN_MEM:
		errdescr = "Out of memory error";
		break;
	case ESPCONN_TIMEOUT:
		errdescr = "Timeout";
		break;
	case ESPCONN_RTE:
		errdescr = "Routing problem";
		break;
	case ESPCONN_INPROGRESS:
		errdescr = "Operation in progress";
		break;
	case ESPCONN_ABRT:
		errdescr = "Connection aborted";
		break;
	case ESPCONN_RST:
		errdescr = "Connection reset";
		break;
	case ESPCONN_CLSD:
		errdescr = "Connection closed";
		break;
	case ESPCONN_CONN:
		errdescr = "Not connected";
		break;
	case ESPCONN_ARG:
		errdescr = "Illegal argument";
		break;
	case ESPCONN_ISCONN:
		errdescr = "Already connected";
		break;
	case ESPCONN_HANDSHAKE:
		errdescr = "SSL handshake failed";
		break;
	case ESPCONN_SSL_INVALID_DATA:
		errdescr = "SSL application invalid";
		break;
	}
	if(errdescr)
		dhdebug("%s %s", descrption, errdescr);
	else
		dhdebug("%s %d", descrption, reason);
}

void ICACHE_FLASH_ATTR dhesperrors_wifi_state(const char *descrption, uint8 reason) {
	char *errdescr = 0;
	switch(reason) {
	case EVENT_STAMODE_CONNECTED:
		errdescr = "STAMODE_CONNECTED";
		break;
	case EVENT_STAMODE_DISCONNECTED:
		errdescr = "STAMODE_DISCONNECTED";
		break;
	case EVENT_STAMODE_AUTHMODE_CHANGE:
		errdescr = "STAMODE_AUTHMODE_CHANGE";
		break;
	case EVENT_STAMODE_GOT_IP:
		errdescr = "STAMODE_GOT_IP";
		break;
	case EVENT_SOFTAPMODE_STACONNECTED:
		errdescr = "SOFTAPMODE_STACONNECTED";
		break;
	case EVENT_SOFTAPMODE_STADISCONNECTED:
		errdescr = "SOFTAPMODE_STADISCONNECTED";
		break;
	case EVENT_MAX:
		errdescr = "EVENT_MAX";
		break;
	}
	if(errdescr)
		dhdebug("%s %s", descrption, errdescr);
	else
		dhdebug("%s %d", descrption, reason);
}
