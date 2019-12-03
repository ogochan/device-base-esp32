#define	TRACE

#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>

#include 	"config.h"
#include 	<freertos/FreeRTOS.h>
#include 	<freertos/task.h>
#include	<freertos/event_groups.h>
#include 	<esp_system.h>
#include	<esp_tls.h>
#include 	<esp_log.h>
#include	<time.h>
#include	<sys/time.h>
#include	<cJSON.h>
#include	"bt.h"
#include	"wifi.h"
#include	"httpc.h"
#include	"schedule.h"
#include	"globals.h"
#include	"misc.h"
#include 	"debug.h"

#ifdef	USE_LOCALCA
extern	const	unsigned	char	ca_key_start[]		asm("_binary_ca_crt_start");
extern	const	unsigned	char	ca_key_end[]		asm("_binary_ca_crt_end");
#endif

extern	void
initialize_api(void)
{
#ifdef	USE_LOCALCA
	esp_tls_init_global_ca_store();
	esp_tls_set_global_ca_store(ca_key_start, ca_key_end - ca_key_start);
#endif
}

extern	Bool
api_device_new(
	esp_http_client_handle_t	client,
	char	*user,
	char	*pass,
	char	*device_id)
{
	Bool	rc;
	cJSON	*root
		,	*node;
	int		size
		,	status;

	rc = FALSE;
	sprintf(Message, "{\"user\":\"%s\",\"pass\":\"%s\",\"class\":\"jar-garden\"}", user, pass);
	if	( httpc_post(client, CONSOLE_HOST, "/devices", (uint8_t *)Message, strlen(Message)) != ESP_OK )	goto	err;
	status =  httpc_get_status(client);
	ESP_LOGI(TAG, "status: %d", status);
	if	( ( size = httpc_get_data(client, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER) ) < 0 )	goto	err;
	ESP_LOGI(TAG, "get: %s", ResponseMessage);
	if		( status != 200 )	goto	err;
	root = cJSON_Parse(ResponseMessage);
	if	( root )	{
		node = cJSON_GetObjectItemCaseSensitive(root, "code");
		if 		(	( cJSON_IsNumber(node) )
				&&	( node->valueint == 0  ))	{
			node = cJSON_GetObjectItemCaseSensitive(root, "uuid");
			if	( cJSON_IsString(node) )	{
				strncpy(device_id, node->valuestring, SIZE_UUID+1);
				ESP_LOGI(TAG, "device uuid: %s", device_id);
				rc = TRUE;
			}
		}
		cJSON_Delete(root);
	}
  err:;
	return	(rc);
}

extern	Bool
api_session_new(
	esp_http_client_handle_t	client,
	char	*user,
	char	*pass,
	char	*device_id,
	char	*session_key)
{
	Bool	rc;
	cJSON	*root
		,	*node;
	int		size
		,	status;


	rc = FALSE;
	sprintf(Message, "{\"user\":\"%s\",\"pass\":\"%s\",\"device\":\"%s\"}", user, pass, device_id);
	if ( httpc_post(client, CONSOLE_HOST, "/session", (uint8_t *)Message, strlen(Message)) != ESP_OK )	goto	err;
	status =  httpc_get_status(client);
	ESP_LOGI(TAG, "status: %d", status);
	if	( status != 200 )	goto	err;
	if	( ( size = httpc_get_data(client, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER) ) < 0 )	goto	err;
	ESP_LOGI(TAG, "get: %s", ResponseMessage);
	root = cJSON_Parse(ResponseMessage);
	if	( root )	{
		node = cJSON_GetObjectItemCaseSensitive(root, "code");
		if 		(	( cJSON_IsNumber(node) )
				&&	( node->valueint == 0  ))	{
			node = cJSON_GetObjectItemCaseSensitive(root, "session_key");
			if		(	( node )
					&&	( cJSON_IsString(node) ) )	{
				strncpy(session_key, node->valuestring, SIZE_UUID+1);
				ESP_LOGI(TAG, "session key: %s", session_key);
				rc = TRUE;
			}
		}
		cJSON_Delete(root);
	}
  err:;
	return	(rc);
}

extern	Bool
api_post_data(
	esp_http_client_handle_t	client,
	char	*path,
	char	*device_id,
	char	*session_key,
	uint8_t	*data,
	size_t	size)
{
	Bool	rc;
	cJSON	*root
		,	*node;
	int		status;

	char	real_path[SIZE_PATH+1];

	rc = FALSE;
	sprintf(real_path, "/device/%s%s", device_id, path);
	httpc_set_header(client, "X-SESSION-KEY", session_key);
	if ( httpc_post(client, CONSOLE_HOST, real_path, data, size) != ESP_OK )	goto	err;
	status =  httpc_get_status(client);
	ESP_LOGI(TAG, "status: %d", status);
	if	( status != 201 )	goto	err;
	if	( httpc_get_data(client, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER) < 0 )	goto	err;
	ESP_LOGI(TAG, "get: %s", ResponseMessage);
	root = cJSON_Parse(ResponseMessage);
	if	( root )	{
		node = cJSON_GetObjectItemCaseSensitive(root, "code");
		if 		(	( cJSON_IsNumber(node) )
				&&	( node->valueint == 0  ))	{
			ESP_LOGI(TAG, "OK");
			rc = TRUE;
		}
		cJSON_Delete(root);
	}
  err:;
	return	(rc);
}

extern	int
api_get_data(
	esp_http_client_handle_t	client,
	char	*path,
	char	*device_id,
	char	*session_key,
	uint8_t	*data,
	size_t	size)
{
	int		rc;
	int		status;

	char	real_path[SIZE_PATH+1];

	rc = -1;
	sprintf(real_path, "/device/%s%s", device_id, path);
	esp_http_client_set_header(client, "X-SESSION-KEY", session_key);
	if ( httpc_get(client, CONSOLE_HOST, real_path) != ESP_OK )	goto	err;
	status =  httpc_get_status(client);
	ESP_LOGI(TAG, "status: %d", status);
	if	( status != 200 )	goto	err;
	rc = httpc_get_data(client, data, size);
	ESP_LOGI(TAG, "get: %s", ResponseMessage);
  err:;
	return	(rc);
}

extern	void
api_get_device_info(void)
{
	esp_http_client_handle_t	client;
	int		size;
ENTER_FUNC;
	client = initialize_httpc();
	do	{
		size = api_get_data(client, "/spec", my_device_id, my_session_key, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER);
	}	while	( size < 0 );
	finish_httpc(client);
LEAVE_FUNC;
}

extern	void
api_get_settings(void)
{
	esp_http_client_handle_t	client;
	int		size;
	cJSON	*root
		,	*node
		,	*sensor;
ENTER_FUNC;
	client = initialize_httpc();
	do	{
		size = api_get_data(client, "/settings", my_device_id, my_session_key, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER);
	}	while	( size < 0 );

	//	defaults
	sensor_interval = DEFAULT_SENSE_INTERVAL;
	setenv("TZ", "UTC", 1);
	sensor_url = NULL;

	root = cJSON_Parse(ResponseMessage);
	if	( root )	{
		node = cJSON_GetObjectItemCaseSensitive(root, "timezone");
		if		(	( node )
				&&	( cJSON_IsString(node) ) )	{
			setenv("TZ", node->valuestring, 1);
			ESP_LOGI(TAG, "TZ=%s", getenv("TZ"));
			tzset();
		}
		sensor = cJSON_GetObjectItemCaseSensitive(root, "sensor");
		if		(	( sensor )
				&&	( cJSON_IsObject(sensor) ) )	{
			node = cJSON_GetObjectItemCaseSensitive(sensor, "interval");
			if		(	( node )
					&&	( cJSON_IsNumber(node) ) )	{
				sensor_interval = node->valueint;
			}
			node = cJSON_GetObjectItemCaseSensitive(sensor, "post_url");
			if		(	( node )
					&&	( cJSON_IsString(node) ) )	{
				if	( *node->valuestring != 0 )	{
					sensor_url = strdup(node->valuestring);
				}
			}
		}
		cJSON_Delete(root);
	}
	finish_httpc(client);
LEAVE_FUNC;
}

static	Bool
check_wday(
	cJSON	*root,
	char	*wday)
{
	cJSON	*node;

	node = cJSON_GetObjectItemCaseSensitive(root, wday);
	return	(	( node != NULL )
			&&	( cJSON_IsBool(node) )
			&&	( cJSON_IsTrue(node) ) );
}

/*
title: "schedule"
type: "array"
minItems: 1
maxItems: 10
items:
  type: "object"
  properties:
    day_of_week:
      type: "object"
      properties:
        Sun:
          type: "boolean"
        Mon:
          type: "boolean"
        Tue:
          type: "boolean"
        Wed:
          type: "boolean"
        Thu:
          type: "boolean"
        Fri:
          type: "boolean"
        Sat:
          type: "boolean"
    LED:
      type: "array"
      minItems: 1
      maxItems: 10
      items:
        type: "object"
        properties:
          at:
            type: "object"
            properties:
              hour:
                type: "integer"
              min:
                type: "integer"
              sec:
                type: "integer"
          color:
            type: "object"
            properties:
              number:
                type: "integer"
              red:
                type: "integer"
                format: "int32"
              green:
                type: "integer"
                format: "int32"
              blue:
                type: "integer"
                format: "int32"
              white:
                type: "integer"
                format: "int32"
*/
extern	void
api_get_schedule(void)
{
	esp_http_client_handle_t	client;
	int		size
		,	i;
	cJSON	*root
		,	*node
		,	*entry
		,	*wday
		,	*led
		,	*event;
	uint8_t	sc_wday;
	tScheduleEvent	*ev;
	int		nev
		,	nsc;
	

ENTER_FUNC;
	client = initialize_httpc();
	do	{
		size = api_get_data(client, "/schedule", my_device_id, my_session_key, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER);
	}	while	( size < 0 );

	reset_schedule();

	root = cJSON_Parse(ResponseMessage);
	if	( root )	{
		nsc = 0;
		cJSON_ArrayForEach(entry, root)	{
			nsc ++;
		}
		ESP_LOGI(TAG, "nsc = %d", nsc);
		cJSON_ArrayForEach(entry, root)	{
			wday = cJSON_GetObjectItemCaseSensitive(entry, "day_of_week");
			sc_wday = 0;
			if		( check_wday(wday, "Sun") )	{
				sc_wday |= 0x80;
			}
			if		( check_wday(wday, "Mon") )	{
				sc_wday |= 0x40;
			}
			if		( check_wday(wday, "Tue") )	{
				sc_wday |= 0x20;
			}
			if		( check_wday(wday, "Wed") )	{
				sc_wday |= 0x10;
			}
			if		( check_wday(wday, "Thu") )	{
				sc_wday |= 0x08;
			}
			if		( check_wday(wday, "Fri") )	{
				sc_wday |= 0x04;
			}
			if		( check_wday(wday, "Sat") )	{
				sc_wday |= 0x02;
			}
			led = cJSON_GetObjectItemCaseSensitive(entry, "LED");
			nev = 0;
			cJSON_ArrayForEach(event, led)	{
				nev ++;
			}
			ESP_LOGI(TAG, "nev = %d", nev);
			ev = (tScheduleEvent *)malloc(nev * sizeof(tScheduleEvent));
			i = 0;
			cJSON_ArrayForEach(event, led)	{
				node = cJSON_GetObjectItemCaseSensitive(event, "at");
				ev[i].at = 0;
				if		(	( node )
						&&	( cJSON_IsString(node) ) )	{
					ev[i].at =
						  str2int(&node->valuestring[0], 2) * 3600
						+ str2int(&node->valuestring[3], 2) * 60
						+ str2int(&node->valuestring[6], 2);
				}
				node = cJSON_GetObjectItemCaseSensitive(event, "number");
				ev[i].led.no = 0;
				if		(	( node )
						&&	( cJSON_IsNumber(node) ) )	{
					ev[i].led.no = node->valueint;
				}
				node = cJSON_GetObjectItemCaseSensitive(event, "color");
				ev[i].led.red = 0;
				ev[i].led.green = 0;
				ev[i].led.blue = 0;
				if		(	( node )
						&&	( cJSON_IsString(node) ) )	{
					ev[i].led.red = hex2int(&node->valuestring[1], 2);
					ev[i].led.green = hex2int(&node->valuestring[3], 2);
					ev[i].led.blue = hex2int(&node->valuestring[5], 2);
				}
				ev[i].led.white = 0;
				node = cJSON_GetObjectItemCaseSensitive(event, "white");
				if		(	( node )
						&&	( cJSON_IsNumber(node) ) )	{
					ev[i].led.white = node->valueint;
				}
				i ++;
			}
			push_schedule(sc_wday, nev, ev);
			for	( i = 0; i < nev ; i ++ )	{
				ESP_LOGI(TAG, "%d (%d) %d %d %d %d",
						 (int)ev[i].at,
						 (int)ev[i].led.no,
						 (int)ev[i].led.red,
						 (int)ev[i].led.green,
						 (int)ev[i].led.blue,
						 (int)ev[i].led.white);
			}
		}
		cJSON_Delete(root);
	}
	finish_httpc(client);
LEAVE_FUNC;
}


extern	void
api_exec_ota(void)
{
	char	real_path[SIZE_PATH+1];

ENTER_FUNC;
	sprintf(real_path, "/device/%s/farm", my_device_id);
	httpc_ota(CONSOLE_HOST, real_path, my_session_key);
LEAVE_FUNC;
}

