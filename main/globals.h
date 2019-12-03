#ifndef	__INC__GLOBALS_H__
#define	__INC__GLOBALS_H__

#include	"types.h"

#ifdef	MAIN
#define	GLOBAL	/*	*/
#else
#define	GLOBAL	extern
#endif

GLOBAL	char	ResponseMessage[SIZE_RESPONSE_BUFFER+1];
GLOBAL	char	Message[SIZE_MESSAGE_BUFFER+1];
GLOBAL	char	my_device_id[SIZE_UUID + 1]
	,			my_session_key[SIZE_UUID + 1]
	,			ap_ssid[SIZE_SSID + 1]
	,			ap_pass[SIZE_PASS + 1]
	,			my_ssid[SIZE_SSID + 1]
	,			my_pass[SIZE_PASS + 1];
GLOBAL	char	url_buff[SIZE_URL + 1];

GLOBAL	int		sensor_interval;
GLOBAL	char	*sensor_url;

#endif
