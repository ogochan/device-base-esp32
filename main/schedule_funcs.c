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
#include	"schedule_funcs.h"
#include	"globals.h"
#include	"misc.h"
#include 	"debug.h"


static	int				n_check_funcs = 0;
static	CHECK_FUNC		check_funcs[N_TIMER_EVENTS];

extern	void
init_schedule_func(void)
{
	n_check_funcs = 0;
}

extern	void
register_schedule_func(
	CHECK_FUNC	func)
{
	check_funcs[n_check_funcs] = func;
	n_check_funcs ++;
}

extern	Bool
check_schedule_funcs(
	tTime	*now)
{
	int		i;
	Bool	ret;

	ret = FALSE;
	for	( i = 0; i < n_check_funcs; i ++ )	{
		if	( check_funcs[i](now) )	{
			ret = TRUE;
		}
	}
	return	(ret);
}
