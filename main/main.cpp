#define	TRACE
#define	MAIN

#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>
extern "C"{
#include 	"config.h"
#include 	<freertos/FreeRTOS.h>
#include 	<freertos/task.h>
#include	<time.h>
#include	<sys/time.h>
#include	"nvs.h"
#include	"misc.h"
#include	"wifi.h"
#include	"device_depend.h"
#include	"schedule_funcs.h"
#include	"api_client.h"
#include	"sntp.h"
#include	"schedule.h"
#include	"setup_mode.h"
#include	"globals.h"
#include 	"debug.h"
}

#include <esp_heap_alloc_caps.h>

#define	HEALTH_CHECK(check) if ( ( err = check ) != ESP_OK ) { goto err_stop; } else {  health_check &= ~err_mask ; err_mask >>= 1; }

static	void
main_task(
	void	*args)
{
	get_settings();
	msleep(1000);

	initialize_sntp();
	while	( !sntp_valid() )	{
		msleep(1000*10);
	}

	dbgmsg( "all preparation is done");
	start_schedule();
	finalize_schedules();
	esp_restart();
}

extern "C" void
app_main (void)
{
	int		count_reset;
	uint8_t	health_check;

ENTER_FUNC;
	health_check = 0x0;


	initialize_nvs();
	load_device_info();
	initialize_wifi();

	initialize_device();

	init_schedule_func();
	initialize_schedule();
	initialize_schedules();

	initialize_api();

	open_reset();
#if	1
	count_reset = get_reset();
#else
	count_reset = 3;
#endif
	dbgprintf( "reset count %d", count_reset);
	count_reset ++;
	set_reset((int32_t)count_reset);
	commit_reset();

	switch(count_reset) {
	  case	3:	//	setup mode
		show_status(STATUS_SETUP);
		SetupMode();
		break;
	  case	5:	//	all reset
		destroy_device_info();
		show_status(STATUS_ALL_RESET);
		count_reset = 0;
		set_reset(count_reset);
		commit_reset();
		msleep(1000*10);
		esp_restart();
		break;
	  default:
		show_status(STATUS_IDLE);
		msleep(1000*10);
		count_reset = 0;
		set_reset(count_reset);
		commit_reset();
		close_reset();
		if	( health_check == 0 )	{
			/*	OK	*/
			show_status(STATUS_RUN);
		} else {
			show_status(STATUS_FAULT);
			goto	stop;
		}
		dbgprintf( "%u bytes free after system initialize", (uint32_t)xPortGetFreeHeapSizeCaps(MALLOC_CAP_8BIT));
		
		if		( wifi_connect(ap_ssid, ap_pass) )	{
			while	( !wifi_is_valid() )	{
				msleep(1000);
			}
			dbgmsg( "network is OK");
			(void)xTaskCreate(main_task, "main_task", 8192, NULL, 5, NULL);
		} else {
			SetupMode();
		}
		break;
	}
  stop:
	STOP;
}

