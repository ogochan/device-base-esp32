#define	TRACE
#define	MAIN

#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>
extern "C"{
#include 	"config.h"
#include	<nvs.h>
#include 	<freertos/FreeRTOS.h>
#include 	<freertos/task.h>
#include	<freertos/event_groups.h>
#include 	<esp_system.h>
#include	<esp_event_loop.h>
#include 	<esp_log.h>
#include	<time.h>
#include	<sys/time.h>
#include	"cJSON.h"
#include	<driver/ledc.h>
#include	"nvs.h"
#include	"bt.h"
#include	"misc.h"
#include	"wifi.h"
#include	"light.h"
#include	"fan.h"
#include	"api_client.h"
#include	"sensor.h"
#include	"sntp.h"
#include	"schedule.h"
#include	"setup_mode.h"
#include	"globals.h"
#include 	"debug.h"
}

//#define	TEST
#define	HAVE_NET
#define	HAVE_LIGHT

#if	0
static	int
_wifi_connect(
	char	*path,
	char	*body,
	Bool	debug)
{
	wifi_stop();
	wifi_connect(ssid, pass);
	
	return	(0);
}
#endif

#include <esp_heap_alloc_caps.h>

#define	BODY_BUFFER_SIZE		1024
static	char	body_buffer[BODY_BUFFER_SIZE];

#define	HEALTH_CHECK(check) if ( ( err = check ) != ESP_OK ) { goto err_stop; } else {  health_check &= ~err_mask ; err_mask >>= 1; }

extern "C" void
app_main (void)
{
	esp_err_t err;
	int		count_reset;
	uint8_t	health_check
		,	err_mask;

ENTER_FUNC;
	health_check = 0xFF;
	err_mask = 0x80;

	initialize_light();
	initialize_fan();

	HEALTH_CHECK(initialize_nvs());
	load_device_info();
	HEALTH_CHECK(initialize_wifi());
	initialize_sensors();
	initialize_api();
	initialize_schedule();

	open_reset();
#if	0
	count_reset = get_reset();
#else
	count_reset = 3;
#endif
	dbgprintf( "reset count %d", count_reset);
	switch(count_reset) {
	  case	3:	//	setup mode
		light_set_all_color(0, 64, 128, 64);
		SetupMode();
		break;
	  case	5:	//	all reset
		destroy_device_info();
		light_set_all_color(128, 0, 64, 64);
		msleep(1000*10);
		esp_restart();
		break;
	  default:
		count_reset ++;
		set_reset((int32_t)count_reset);
		commit_reset();
		light_set_all_color(128, 128, 0, 0);
		msleep(1000*10);
		break;
	}
	count_reset = 0;
	set_reset(count_reset);
	commit_reset();
	close_reset();
	dbgprintf( "reset count %d", count_reset);
	if	( health_check == 0 )	{
		/*	OK	*/
		light_set_all_color(0, 0, 0, 0);
	} else {
	}

	//
	//	normal start here
	//
	light_set_all_color(0, 0, 0, 0);
	
	//	note need check connectable
	wifi_connect(ap_ssid, ap_pass);
	while	( !wifi_is_valid() )	{
		msleep(1000);
	}
	dbgmsg( "network is OK");

	api_get_settings();
	msleep(1000);
	api_exec_ota();
	msleep(1000);
	start_sensors();

	initialize_sntp();
	while	( !sntp_valid() )	{
		msleep(1000*10);
	}
	dbgmsg( "all preparation is done");
	initialize_schedule();
	api_get_schedule();
	start_schedule();

#if	0
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	wifi_scan_start();

	bt_set_handler(BT_GET, "/wifi/scan", _wifi_scan);
	bt_set_handler(BT_POST, "/wifi/config", _wifi_config);
	bt_set_handler(BT_POST, "/wifi/connect", _wifi_config);
	
	initialize_bt(DEVICE_NAME);

	start_bt(4096);
#endif
	
	dbgprintf( "%d bytes free after system initialize", (int)xPortGetFreeHeapSizeCaps(MALLOC_CAP_8BIT));
#if	0
	_wifi_scan(NULL, body_buffer, true);
	_wifi_config(NULL, "{ \"ssid\": \"LANLANLAN24\", \"pass\": \"8309f815520d2\"}", true);
	_wifi_connect(NULL, NULL, true);

#endif
  err_stop:
	STOP;
}

