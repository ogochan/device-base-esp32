#define	TRACE

extern	"C"	{
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
#include	"neopixel.h"
#include	"api_client.h"
#include	"schedule_funcs.h"
#include	"schedule.h"
#include	"globals.h"
#include	"misc.h"
#include 	"debug.h"
}
#include	"light.h"
#include	"fan.h"
#include	"sensor.h"

extern	"C"	{

static	Bool
check_ota(
	tTime	*now)
{
	static	int		ota_min;
	int		at;
	static	Bool	done;
	Bool	ret;

	ret = FALSE;
	if		( now == NULL )	{
		ota_min = random() % 1438 + 1;
		dbgprintf("OTA check  at %02d:%02d", ota_min / 60, ota_min % 60);
		done = FALSE;
	} else {
		at = now->tm_hour * 60 + now->tm_min;
		//ota_min = at - 1;
		if		(	( !done )
				&&	( at > ota_min ) )	{
			ret = api_exec_ota();
			done = TRUE;
		} else {
			if	( at < ota_min )	{
				done = FALSE;
			}
		}
	}
	return	(ret);
}

extern	void
initialize_schedule(void)
{
	register_schedule_func(check_ota);
}

extern	void
start_schedule(void)
{
	time_t	timeNow;
	tTime	now;
	Bool	stop;

	time(&timeNow);

	srandom(timeNow);

	check_schedule_funcs(NULL);

	stop = FALSE;
	while	( !stop )	{
		time(&timeNow);
		localtime_r(&timeNow, &now);

		stop = check_schedule_funcs(&now);

		msleep(1000);
	}
}
}	//	extern C
