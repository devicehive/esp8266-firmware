/**
 *	\file		dhstatistic.h
 *	\brief		Module fo collecting statistic data
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHSTATISTIC_H_
#define _DHSTATISTIC_H_

/** Struct will all statistic data*/
typedef struct {
		unsigned long long bytesSent;			///< Total bytes sent.
		unsigned long long bytesReceived;		///< Total bytes received.
		unsigned int networkErrors;				///< Number of network errors.
		unsigned int httpdRequestsCount;			///< Number of REST requests.
		unsigned int httpdErrorsCount;			///< Number of REST errors.
		unsigned int wifiLosts;					///< Number of Wi-Fi disconnects.
		unsigned int serverErrors;				///< Number of errors from server.
		unsigned int notificationsTotal;		///< Attempts number of creating notifications.
		unsigned int responcesTotal;			///< Attempts number of creating responses.
		unsigned int notificationsDroppedCount;	///< Number of dropped notifications.
		unsigned int responcesDroppedCount;		///< Number of dropped responses.
		unsigned int localRestRequestsCount;	///< Number of requests received via local REST.
		unsigned int localRestResponcesErrors;	///< Number of errors in responses to local REST.
} DHSTATISTIC;

/**
 *	\brief				Add some numver of sent bytes.
 *	\param[in]	bytes	Number of bytes to add.
 */
void dhstatistic_add_bytes_sent(unsigned int bytes);

/**
 *	\brief				Add some numver of received bytes.
 *	\param[in]	bytes	Number of bytes to add.
 */
void dhstatistic_add_bytes_received(unsigned int bytes);

/**
 *	\brief				Increment number of network errors.
 */
void dhstatistic_inc_network_errors_count();

/**
 *	\brief				Increment number of REST requests.
 */
void dhstatistic_inc_httpd_requests_count();

/**
 *	\brief				Increment number of REST errors.
 */
void dhstatistic_inc_httpd_errors_count();

/**
 *	\brief				Increment number of Wi-Fi disconnections.
 */
void dhstatistic_inc_wifi_lost_count();

/**
 *	\brief				Increment number of server errors.
 */
void dhstatistic_inc_server_errors_count();

/**
 *	\brief				Increment number of notifications.
 */
void dhstatistic_inc_notifications_count();

/**
 *	\brief				Increment number of dropped notifications.
 */
void dhstatistic_inc_notifications_dropped_count();

/**
 *	\brief				Increment number of responses.
 */
void dhstatistic_inc_responces_count();

/**
 *	\brief				Increment number of dropped responses.
 */
void dhstatistic_inc_responces_dropped_count();

/**
 *	\brief				Increment number REST requests.
 */
void dhstatistic_inc_local_rest_requests_count();

/**
 *	\brief				Increment number of REST requests which were answered with error.
 */
void dhstatistic_inc_local_rest_responses_errors();

/**
 *	\brief				Return statistic.
 *	\return				Pointer to DHSTATISTIC with data.
 */
const DHSTATISTIC* dhstatistic_get_statistic();

#endif /* _DHSTATISTIC_H_ */
