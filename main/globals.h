#ifndef	__INC__GLOBALS_H__
#define	__INC__GLOBALS_H__

#include	"types.h"

#define	STATUS_RUN			0
#define	STATUS_SETUP		1
#define	STATUS_ALL_RESET	2
#define	STATUS_IDLE			3
#define	STATUS_FAULT		4

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
GLOBAL	int		led_schedule_interval;
GLOBAL	char	*sensor_url;
GLOBAL	float	upper_temperature_limit;
GLOBAL	float	lower_temperature_limit;

#endif
