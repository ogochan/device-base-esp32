#ifndef __INC_CONFIG_H__
#define __INC_CONFIG_H__

#include		"config.common.h"

#define TAG	"Jar-garden"

#define	BT_BUFFER_SIZE			512

/* folloger */
/*
#define	HOST		"54.238.177.112"
#define	PORT		7001
#define	UUID		"9e2f23f8-28da-437d-aa43-b8e92469880a"
#define	MY_SSID		"LANLANLAN24"
#define	MY_PASS		"8309f815520d2"
*/

/* SoftAP */

#define	HOST		"192.168.8.1"
#define	PORT		2000
#define	UUID		"9e2f23f8-28da-437d-aa43-b8e92469880a"
#define	MY_SSID		"UAVLANAP"
#define	MY_PASS		"uavlanpass"


/* LAN */
/*
#define	HOST		"10.1.252.14"
#define	PORT		2000
#define	UUID		"9e2f23f8-28da-437d-aa43-b8e92469880a"
#define	MY_SSID		"LANLANLAN24"
#define	MY_PASS		"8309f815520d2"
*/
#define	GAS_SENSE_TIMING		100	// milli second
#define	VOLTAGE_SENSE_TIMING		1000	// milli second

#endif // __CONFIG_H__
