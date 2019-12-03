#ifndef __INC_CONFIG_H__
#define __INC_CONFIG_H__

#include		"config.common.h"

#define TAG	"Jar-garden"

#define	SIZE_RESPONSE_BUFFER	1023
#define	SIZE_MESSAGE_BUFFER		1023

#define	SIZE_USER				63
#define	SIZE_PASS				63
#define	SIZE_UUID				36
#define	SIZE_SSID				32
#define	SIZE_IP					15

#define	NUMBER_OF_SENSORS		5
#define	SIZE_OF_SENSOR_BUFF		256
#define	SIZE_HOST				255
#define	SIZE_URL				255
#define	SIZE_PATH				255

/* SoftAP */
#define	DEFAULT_SSID	"JAR-GARDEN"
#define	DEFAULT_PASS	"jar-garden"
#define	DEFAULT_AP_SSID	"LANLANLAN24"
#define	DEFAULT_AP_PASS	"8309f815520d2"

/* folloger */
/*
#define	HOST		"54.238.177.112"
#define	PORT		7001
#define	UUID		"9e2f23f8-28da-437d-aa43-b8e92469880a"
*/

#define	HOST			"192.168.8.1"
#define	PORT			2000
#define	UUID			"9e2f23f8-28da-437d-aa43-b8e92469880a"

#define	MAX_STA_CONN	2

/* LAN */
/*
#define	HOST		"10.1.252.14"
#define	PORT		2000
#define	UUID		"9e2f23f8-28da-437d-aa43-b8e92469880a"
#define	MY_SSID		"LANLANLAN24"
#define	MY_PASS		"8309f815520d2"
*/

#endif // __CONFIG_H__
