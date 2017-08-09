/**
 * @file
 * @brief Module to collect various statistic data.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DHSTATISTIC_H_
#define _DHSTATISTIC_H_


/**
 * @brief Various statistic data.
 */
struct DHStat {
	unsigned long long bytesSent;           ///< Total bytes sent.
	unsigned long long bytesReceived;       ///< Total bytes received.
	unsigned int networkErrors;             ///< Number of network errors.

	unsigned int httpdRequestsCount;        ///< Number of REST requests.
	unsigned int httpdErrorsCount;          ///< Number of REST errors.

	unsigned int wifiLosts;                 ///< Number of Wi-Fi disconnects.

	unsigned int serverErrors;              ///< Number of errors from server.
	unsigned int notificationsTotal;        ///< Number of attempts to create notifications.
	unsigned int responcesTotal;            ///< Number of attempts to create responses.
	unsigned int notificationsDroppedCount; ///< Number of dropped notifications.
	unsigned int responcesDroppedCount;     ///< Number of dropped responses.

	unsigned int localRestRequestsCount;    ///< Number of requests received via local REST.
	unsigned int localRestResponcesErrors;  ///< Number of errors in responses to local REST.
};


/**
 * @brief Get global statistics.
 */
const struct DHStat* dhstat_get(void);


/**
 * @brief Add some number of bytes sent.
 * @param[in] stat Statistics to update.
 * @param[in] no_bytes Number of bytes sent.
 */
void dhstat_add_bytes_sent(unsigned int no_bytes);


/**
 * @brief Add some number of bytes received.
 * @param[in] stat Statistics to update.
 * @param[in] no_bytes Number of bytes received.
 */
void dhstat_add_bytes_received(unsigned int no_bytes);


/**
 * @brief Increment number of network errors.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_network_error(void);


/**
 * @brief Increment number of REST requests.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_httpd_request(void);


/**
 * @brief Increment number of REST errors.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_httpd_error(void);


/**
 * @brief Increment number of Wi-Fi disconnections.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_wifi_lost(void);


/**
 * @brief Increment number of server errors.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_server_error(void);


/**
 * @brief Increment number of notifications.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_notification(void);


/**
 * @brief Increment number of dropped notifications.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_notification_dropped(void);


/**
 * @brief Increment number of responses.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_responce(void);


/**
 * @brief Increment number of dropped responses.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_responce_dropped(void);


/**
 * @brief Increment number REST requests.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_local_rest_request(void);


/**
 * @brief Increment number of REST requests which were answered with error.
 * @param[in] stat Statistics to update.
 */
void dhstat_got_local_rest_response_error(void);


#endif /* _DHSTATISTIC_H_ */
