#define	TRACE

extern "C" {
#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include 	"config.h"
#include	<nvs.h>
#include 	<freertos/FreeRTOS.h>
#include 	<freertos/task.h>
#include	<time.h>
#include	<sys/time.h>
#include	<driver/ledc.h>
#include	<cJSON.h>
#include	"neopixel.h"
#include	"types.h"
#include	"misc.h"
#include	"api_client.h"
#include	"device_depend.h"
#include	"httpc.h"
#include	"schedule_funcs.h"
#include	"globals.h"
#include 	"debug.h"
}
#include	"light.h"
#include	"fan.h"
#include	"utility.h"
#include	"SensorInfo.h"
#include	"BME280_Temp.h"
#include	"Sensors.h"
#include	"sensor.h"

#ifdef	HAVE_FAN
static	FAN		*fan;
#endif
static	Light	*light;

typedef	struct	{
	int8_t	no;
	uint8_t	red;
	uint8_t	green;
	uint8_t	blue;
	uint8_t	white;
}	LED;
typedef	struct	{
	int		at;
	LED		led;
}	tScheduleEvent;

typedef	struct	{
	uint8_t		wday;
	uint8_t		nev;
	tScheduleEvent	*ev;
}	tSchedule;

extern	void
initialize_device(void)
{
	initialize_sensors();

	light = new Light();
	Sensors::add(light);
	dbgprintf("light id = %d", (int)light->id);

#ifdef	HAVE_FAN
	fan = new FAN();
	Sensors::add(fan);
	dbgprintf("fan id = %d", (int)fan->id);
#endif
	start_sensors();
}

extern	void
show_status(
	int		status)
{
	switch	(status)	{
	  case	STATUS_RUN:
		light->color(0, 0, 0, 0);
		break;
	  case	STATUS_SETUP:
		light->color(0, 64, 128, 64);
		break;
	  case	STATUS_ALL_RESET:
		light->color(128, 0, 64, 64);
		break;
	  case	STATUS_IDLE:
		light->color(128, 128, 0, 0);
		break;
	  case	STATUS_FAULT:
		light->color(128,   0, 0, 0);
		break;
	}
}

static	tSchedule	Schedules[MAX_TIMER_EVENTS];
static	int			nSchedules;

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

static	void
reset_led_schedule(void)
{
	while ( nSchedules > 0 )	{
		nSchedules --;
		free(Schedules[nSchedules].ev);
	}
}

static	void
push_led_schedule(
	uint8_t	wday,
	int		nev,
	tScheduleEvent	*ev)
{
	Schedules[nSchedules].wday = wday;
	Schedules[nSchedules].nev = (uint8_t)nev;
	Schedules[nSchedules].ev = ev;
	nSchedules ++;
}

static	void
get_led_schedule(void)
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
	tScheduleEvent	*ev = NULL;
	int		nev
		,	nsc;
	

ENTER_FUNC;
	client = initialize_httpc();
	do	{
		size = api_get_data(client, "/schedule", my_device_id, my_session_key, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER);
		if		( size < 0 )	{
			msleep(1000);
		}
	}	while	( size < 0 );
	ResponseMessage[size] = 0;

	reset_led_schedule();

	ev = NULL;
	root = cJSON_Parse(ResponseMessage);
	if	( root )	{
		nsc = 0;
		cJSON_ArrayForEach(entry, root)	{
			nsc ++;
		}
		ESP_LOGI("", "nsc = %d", nsc);
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
			ESP_LOGI("", "nev = %d", nev);
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
			push_led_schedule(sc_wday, nev, ev);
			for	( i = 0; i < nev ; i ++ )	{
				ESP_LOGI("", "%d (%d) %d %d %d %d",
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

static	void
led_clear(
	LED		*led)
{
	int		i;
	for	( i = 0 ; i < NR_LED ; i ++ )	{
		led[i].no = i;
		led[i].red = 0;
		led[i].green = 0;
		led[i].blue = 0;
		led[i].white = 0;
	}
}

static	void
current_status(
	tTime	*now,
	LED		*led)
{
	int		i
		,	j;
	tScheduleEvent	*ev;
	int		at
		,	no;

	at = now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec;
	led_clear(led);

	for	( i = 0; i < nSchedules; i ++ )	{
		if		( ( Schedules[i].wday & ( 0x80 >> now->tm_wday ) ) != 0 )	{
			ev = Schedules[i].ev;
			for	( j = 0 ; j < Schedules[i].nev ; j ++ )	{
				if		( ev[j].at <= at )	{
					if		( ( no = ev[j].led.no ) >= 0 )	{
						led[no] = ev[j].led;
					} else {
						for	( no = 0; no < NR_LED; no ++ )	{
							led[no] = ev[j].led;
							led[no].no = no;
						}
					}
				} else
					break;
			}
		}
	}
}

extern	void
get_settings(void)
{
	esp_http_client_handle_t	client;
	int		size;
	cJSON	*root
		,	*node
		,	*sensor
		,	*control;

ENTER_FUNC;
	client = initialize_httpc();
	do	{
		size = api_get_data(client, "/settings", my_device_id, my_session_key, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER);
		if		( size < 0 )	{
			msleep(1000);
		}
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
			dbgprintf("TZ=%s", getenv("TZ"));
			tzset();
		}
		control = cJSON_GetObjectItemCaseSensitive(root, "control");
		if		(	( control )
				&&	( cJSON_IsObject(control) ) )	{
			node = cJSON_GetObjectItemCaseSensitive(control, "upper_temperature_limit");
			if		(	( node )
					&&	( cJSON_IsNumber(node) ) )	{
				upper_temperature_limit = node->valuedouble;
			} else {
				upper_temperature_limit = 31.0;
			}
			node = cJSON_GetObjectItemCaseSensitive(control, "lower_temperature_limit");
			if		(	( node )
					&&	( cJSON_IsNumber(node) ) )	{
				lower_temperature_limit = node->valuedouble;
			} else {
				lower_temperature_limit = 28.0;
			}
			node = cJSON_GetObjectItemCaseSensitive(control, "led_schedule_interval");
			if		(	( node )
					&&	( cJSON_IsNumber(node) ) )	{
				led_schedule_interval = node->valueint;
			} else {
				led_schedule_interval = 3600;
			}
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


static	void
check_led(
	tTime	*now)
{
	int		i;
	LED		led[NR_LED];
	static	LED	last_led[NR_LED];

	if	( now == NULL )	{
		nSchedules = 0;
		led_clear(last_led);
		get_led_schedule();
	} else {
		current_status(now, led);
		if		( memcmp(last_led, led, sizeof(LED) * NR_LED)  != 0 )	{
			for	( i = 0 ; i < NR_LED ; i ++ )	{
				light->color(i, led[i].red, led[i].green, led[i].blue, led[i].white);
				dbgprintf("%d: %d %d %d %d",
						  (int)led[i].no,
						  (int)led[i].red,
						  (int)led[i].green,
						  (int)led[i].blue,
						  (int)led[i].white);
			}
			light->update();
			sensor_collect(light);
			memcpy(last_led, led, sizeof(LED) * NR_LED);
		}
	}
}

static	void
check_led_schedule(
	tTime	*now)
{
	int		at;
	static	int		last;

	if	( now == NULL )	{
		last = 0;
	} else {
		at = now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec;
		if		( at - last > led_schedule_interval )	{
			if	( last > 0 )	{
				get_led_schedule();
			}
			last = at;
			dbgprintf("last = %d", last);
		}
	}
}

#ifdef	HAVE_SENSORS
static	void
check_sensor(
	tTime	*now)
{
	int		at;
	static	int		last;
	
	if		( now == NULL )	{
		last = 0;
	} else {
		at = now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec;
		if		( last > ( at % sensor_interval ) )	{
			sensor_collect();
		}
		last = at % sensor_interval;
	}
}

static	void
check_send(
	tTime	*now)
{
	static	int		send_min;
	static	Bool	sent;
	
	if		( now == NULL )	{
		send_min = random() % 59 + 1;
		dbgprintf("sensor data send every %d min", send_min);
		sent = FALSE;
	} else {
		if		( now->tm_min == send_min )	{
			if		( !sent )	{
				sent = sensor_send_server();
			}
		} else {
			sent = FALSE;
		}
	}
}
#endif

#ifdef	HAVE_FAN
static	void
check_fan(
	tTime	*now)
{
	static	SenseBuffer	*sense_buff;
	static	SensorInfo	*sensor;
	float	temp;
	int		speed;

	if		( now == NULL )	{
		sense_buff = new SenseBuffer(16);
		sensor = (SensorInfo *)new BME280_Temp();
		dbgprintf("fan on  = %s", ftos(upper_temperature_limit, 0, 1));
		dbgprintf("fan off = %s", ftos(lower_temperature_limit, 0, 1));
	} else {
		if	( now->tm_sec == 0 )	{
			sense_buff->rewind_write();
			sensor->get(sense_buff);
			sense_buff->rewind_read();
			sense_buff->get(&temp, sizeof(float));
			dbgprintf("Temp = %s", ftos(temp, 0, 1));
			speed = fan->speed();
			if		(  temp > upper_temperature_limit )	{
				fan->set_switch(TRUE);
			}
			if		(  temp < lower_temperature_limit )	{
				fan->set_switch(FALSE);
			}
			if		( speed != fan->speed() )	{
				sensor_collect(fan);
			}
		}
	}
}
#endif

extern	void
initialize_schedules(void)
{

	register_schedule_func(check_led);
	register_schedule_func(check_led_schedule);
#ifdef	HAVE_SENSORS
	register_schedule_func(check_send);
	register_schedule_func(check_sensor);
#endif
#ifdef	HAVE_FAN
	register_schedule_func(check_fan);
#endif

}
