/**
 *	\file		httpd.h
 *	\brief		Simple HTTP server.
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef _HTTPD_H_
#define _HTTPD_H_

/** Callback return code */
typedef enum {
	HRCS_ANSWERED,			///< Response was sent, content field is set.
	HRCS_NOT_FINISHED,		///< Request is not finished.
	HRCS_NOT_FOUND,			///< Path not found.
	HRCS_INTERNAL_ERROR,	///< Internal error happens.
	HRCS_BAD_REQUEST,		///< Request is not correct.
	HRCS_NOT_IMPLEMENTED,	///< Method is not implemented.
	HRCS_UNAUTHORIZED,		///< Unauthorized.
	HRCS_OPTIONS			///< Options.
} HTTP_RESPONSE_STATUS;

/** Content descriptor */
typedef struct {
	const char *data;  		///< Data to data.
	unsigned int len;		///< Data length.
} HTTP_CONTENT;

/** Struct for HRCS_ANSWERED data */
typedef struct {
	HTTP_CONTENT content;		///< Data to return.
	unsigned ok : 1;			///< Is response with 2xx code? True by default.
	unsigned free_content : 1;	///< Is data was malloced, need to be free? False by default.
} HTTP_ANSWER;

/** Callback prototype for requests. */
typedef HTTP_RESPONSE_STATUS (*HttpRequestCb)(const char *path, const char *key,
		HTTP_CONTENT *content_in, HTTP_ANSWER *answer);

/**
 *	\brief		Initialize HTTP daemon
 */
void httpd_init(HttpRequestCb get_cb, HttpRequestCb post_cb);

/**
 *	\brief				Force server to redirect and answer only to single host
 *	\param[in]	host	Pointer to host name, or zero to disable redirect.
 */
void httpd_redirect(const char *host);

#endif /* _HTTPD_H_ */
