/*
 * dhstatistic.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for collecting statistic
 *
 */

#include <c_types.h>
#include "dhstatistic.h"

LOCAL DHSTATISTIC mStatistic = {0};

void ICACHE_FLASH_ATTR dhstatistic_add_bytes_sent(unsigned int bytes) {
	mStatistic.bytesSent += bytes;
}

void ICACHE_FLASH_ATTR dhstatistic_add_bytes_received(unsigned int bytes) {
	mStatistic.bytesReceived += bytes;
}

void ICACHE_FLASH_ATTR dhstatistic_inc_network_errors_count() {
	mStatistic.networkErrors++;
}

void dhstatistic_inc_httpd_requests_count() {
	mStatistic.httpdRequestsCount++;
}

void dhstatistic_inc_httpd_errors_count() {
	mStatistic.httpdErrorsCount++;
}

void ICACHE_FLASH_ATTR dhstatistic_inc_wifi_lost_count() {
	mStatistic.wifiLosts++;
}

void ICACHE_FLASH_ATTR dhstatistic_server_errors_count() {
	mStatistic.serverErrors++;
}

void ICACHE_FLASH_ATTR dhstatistic_inc_notifications_count() {
	mStatistic.notificationsTotal++;
}

void ICACHE_FLASH_ATTR dhstatistic_inc_notifications_dropped_count() {
	mStatistic.notificationsDroppedCount++;
}

void ICACHE_FLASH_ATTR dhstatistic_inc_responces_count() {
	mStatistic.responcesTotal++;
}

void ICACHE_FLASH_ATTR dhstatistic_inc_responces_dropped_count() {
	mStatistic.responcesDroppedCount++;
}

const DHSTATISTIC* ICACHE_FLASH_ATTR dhstatistic_get_statistic() {
	return &mStatistic;
}
