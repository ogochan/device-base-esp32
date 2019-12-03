#define	TRACE

#include 	"config.h"
#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>
#include 	<freertos/FreeRTOS.h>
#include 	<freertos/task.h>
#include 	<esp_log.h>
#include	<time.h>

#include	"types.h"
#include	"light.h"
#include	"fan.h"
#include	"sensor.h"
#include	"api_client.h"
#include	"schedule.h"
#include	"globals.h"
#include	"misc.h"
#include 	"debug.h"

static	tSchedule	Schedules[MAX_TIMER_EVENTS];
static	int			nSchedules;

extern	void
reset_schedule(void)
{
	while ( nSchedules > 0 )	{
		nSchedules --;
		free(Schedules[nSchedules].ev);
	}
}

extern	void
push_schedule(
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
led_clear(
	LED		*led)
{
	int		i;
	for	( i = 0 ; i < NR_LED ; i ++ )	{
		led[i].no = i;
		led[i].red = 0;
		led[i].green = 0;
		led[i].blue = 0;
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


static	void
check_led(
	tTime	*now)
{
	int		i;
	LED		led[NR_LED];
	static	LED	last_led[NR_LED];
	Bool	fan;

	if	( now == NULL )	{
		led_clear(last_led);
	} else {
		current_status(now, led);
		if		( memcmp(last_led, led, sizeof(LED) * NR_LED)  != 0 )	{
			for	( i = 0 ; i < NR_LED ; i ++ )	{
				light_set_color(i, led[i].red, led[i].green, led[i].blue, led[i].white);
				if		(	( led[i].red > 0 )
						||	( led[i].green > 0 )
						||	( led[i].blue > 0 )
						||	( led[i].white > 0 ) )	{
					fan = TRUE;
				} else {
					fan = FALSE;
				}
				dbgprintf("%d: %d %d %d %d",
						  (int)led[i].no,
						  (int)led[i].red,
						  (int)led[i].green,
						  (int)led[i].blue,
						  (int)led[i].white);
			}
			light_update();
			fan_switch(fan);
			memcpy(last_led, led, sizeof(LED) * NR_LED);
		}
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
check_ota(
	tTime	*now)
{
	static	int		ota_min;
	int		at;
	static	Bool	done;

	if		( now == NULL )	{
		ota_min = random() % 1438 + 1;
		dbgprintf("OTA check at %02d:%02d", ota_min / 60, ota_min % 60);
		done = FALSE;
	} else {
		at = now->tm_hour * 60 + now->tm_min;
		if		(	( !done )
				&&	( at > ota_min ) )	{
			api_exec_ota();
			msleep(1000);
			done = TRUE;
		} else {
			if	( at < ota_min )	{
				done = FALSE;
			}
		}
	}
}

static	void
check_time(void)
{
	time_t	timeNow;
	tTime	now;

	time(&timeNow);
	localtime_r(&timeNow, &now);

	check_led(&now);
	check_send(&now);
	check_sensor(&now);
	check_ota(&now);
}

extern	void
initialize_schedule(void)
{
	nSchedules = 0;
	reset_schedule();
}

static	void
event_task(
	void	*args)
{
	time_t	timeNow;

	time(&timeNow);

	srandom(timeNow);

	check_led(NULL);
	check_send(NULL);
	check_sensor(NULL);
	check_ota(NULL);

	while	(true)	{
		check_time();
		msleep(1000);
	}
}

extern	void
start_schedule(void)
{
    xTaskCreate(event_task, "event_task", 4096, NULL, 5, NULL);
}
