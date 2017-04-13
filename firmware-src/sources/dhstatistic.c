/**
 * @file
 * @brief Module to collect various statistic data.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "dhstatistic.h"
#include <c_types.h>


// global statistics instance
static struct DHStat g_stat = {0};


/*
 * @brief Get global statistics.
 */
const struct DHStat* ICACHE_FLASH_ATTR dhstat_get()
{
	return &g_stat;
}


/*
 * @brief Add some number of bytes sent.
 * @param[in] stat Statistics to update.
 * @param[in] no_bytes Number of bytes sent.
 */
void ICACHE_FLASH_ATTR dhstat_add_bytes_sent(unsigned int no_bytes)
{
	g_stat.bytesSent += no_bytes;
}


/*
 * @brief Add some number of bytes received.
 * @param[in] stat Statistics to update.
 * @param[in] no_bytes Number of bytes received.
 */
void ICACHE_FLASH_ATTR dhstat_add_bytes_received(unsigned int no_bytes)
{
	g_stat.bytesReceived += no_bytes;
}


/*
 * @brief Increment number of network errors.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_network_error(void)
{
	g_stat.networkErrors += 1;
}


/*
 * @brief Increment number of REST requests.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_httpd_request(void)
{
	g_stat.httpdRequestsCount++;
}


/*
 * @brief Increment number of REST errors.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_httpd_error(void)
{
	g_stat.httpdErrorsCount++;
}


/*
 * @brief Increment number of Wi-Fi disconnections.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_wifi_lost(void)
{
	g_stat.wifiLosts++;
}


/*
 * @brief Increment number of server errors.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_server_error(void)
{
	g_stat.serverErrors++;
}


/*
 * @brief Increment number of notifications.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_notification(void)
{
	g_stat.notificationsTotal++;
}


/*
 * @brief Increment number of dropped notifications.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_notification_dropped(void)
{
	g_stat.notificationsDroppedCount++;
}


/*
 * @brief Increment number of responses.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_responce(void)
{
	g_stat.responcesTotal++;
}


/*
 * @brief Increment number of dropped responses.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_responce_dropped(void)
{
	g_stat.responcesDroppedCount++;
}


/*
 * @brief Increment number REST requests.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_local_rest_request(void)
{
	g_stat.localRestRequestsCount++;
}


/*
 * @brief Increment number of REST requests which were answered with error.
 * @param[in] stat Statistics to update.
 */
void ICACHE_FLASH_ATTR dhstat_got_local_rest_response_error(void)
{
	g_stat.localRestResponcesErrors++;
}
