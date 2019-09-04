#define	TRACE
#define	MAIN

#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>
extern "C"{
#include "nvs.h"
#include "nvs_flash.h"
#include 	"freertos/FreeRTOS.h"
#include 	"freertos/task.h"
#include	"freertos/event_groups.h"
#include 	"esp_system.h"
#include	"esp_wifi.h"
#include	"esp_event_loop.h"
#include	"esp_bt.h"
#include	"esp_bt_main.h"
#include	"esp_bt_device.h"
#include	"esp_spp_api.h"
#include	"esp_gap_bt_api.h"
#include 	"esp_log.h"
#include	"time.h"
#include	"sys/time.h"
#include	"cJSON.h"
#include	"bt.h"
#include	"driver/ledc.h"
#include	"wifi.h"
#include	"light.h"
#include	"globals.h"
}

#include 	"config.h"
#include 	"debug.h"

#define DEVICE_NAME		"Jar-garden"

#define	TEST
#define	HAVE_NET
#define	HAVE_LIGHT

static esp_err_t
example_net_event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

static	char	ssid[33];
static	char	pass[65];

static	int
_wifi_scan(
	char	*path,
	char	*body,
	Bool	debug)
{
ENTER_FUNC;
	scan_wifi_get(body);
	if	( debug )	{
		ESP_LOGI(TAG, "%s", body);
	} else {
		bt_send_string(body);
		bt_send_string("\n\n");
	}
LEAVE_FUNC;
	return	(0);
}

#define	ERROR(msg)		{		\
 ESP_LOGI(TAG, "%s", (msg));	\
 goto error;					\
}

static	int
_wifi_config(
	char	*path,
	char	*body,
	Bool	debug)
{
	cJSON	*root
		,	*_ssid
		,	*_pass;
	
	ESP_LOGI(TAG, "config: %s", body);
	root = cJSON_Parse(body);
	if ( root != NULL )	{
		_ssid = cJSON_GetObjectItemCaseSensitive(root, "ssid");
		if ( cJSON_IsString(_ssid) )	{
			strcpy(ssid, _ssid->valuestring);
		} else {
			strcpy(ssid, "** none **");
			ERROR("not string ssid");
		}
		_pass = cJSON_GetObjectItemCaseSensitive(root, "pass");
		if	( cJSON_IsString(_pass) )	{
			strcpy(pass, _pass->valuestring);
		} else {
			strcpy(pass, "** none **");
			ERROR("not string pass");
		}
		ESP_LOGI(TAG, "ssid: %s", ssid);
		ESP_LOGI(TAG, "pass: %s", pass);
	} else {
		const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGI(TAG, "Error before: %s\n", error_ptr);
        }
	}
	cJSON_Delete(root);
  error:
	
	return	(0);
}

static	int
_wifi_connect(
	char	*path,
	char	*body,
	Bool	debug)
{
	stop_wifi();
	connect_wifi(ssid, pass);
	
	return	(0);
}

static	int
_light_set(
	char	*path,
	char	*body,
	Bool	debug)
{
	return	0;
}

#include <esp_heap_alloc_caps.h>

#define	BODY_BUFFER_SIZE		1024
static	char	body_buffer[BODY_BUFFER_SIZE];

#ifdef	HAVE_LIGHT
#ifdef	TEST
static	void
test_light()
{
	uint8_t	fact = 0;
	int i;

ENTER_FUNC;
	while (1) {
		usleep(1000*100);
		for	( i = 0 ; i < 25 ; i ++ )	{
			light_set_color(i,
							(( i + fact ) & 0x08) ? 255: 0,
							(( i + fact ) & 0x04) ? 255: 0,
							(( i + fact ) & 0x02) ? 255: 0,
							(( i + fact ) & 0x01) ? 255: 0);
		}
		dbgmsg("*");
		light_update();
		fact ++;
	}
LEAVE_FUNC;
}
#endif
#endif

extern "C" void
app_main (void)
{
ENTER_FUNC;
#ifdef	TEST
	nr_rgbw_led = 0;
	nr_neopixel_led = 25;
#endif

	initialize_nvs();

#ifdef	HAVE_NET
	initialize_wifi();
	scan_wifi_start();

	bt_set_handler(BT_GET, "/wifi/scan", _wifi_scan);
	bt_set_handler(BT_POST, "/wifi/config", _wifi_config);
	bt_set_handler(BT_POST, "/wifi/connect", _wifi_config);
	
	//initialize_bt(DEVICE_NAME);

	//start_bt(4096);

	ESP_LOGI(TAG, "%d bytes free after system initialize", (int)xPortGetFreeHeapSizeCaps(MALLOC_CAP_8BIT));
#ifdef	TEST
	_wifi_scan(NULL, body_buffer, true);
	_wifi_config(NULL, "{ \"ssid\": \"LANLANLAN24\", \"pass\": \"8309f815520d2\"}", true);
	_wifi_connect(NULL, NULL, true);

#endif

#endif
#ifdef	HAVE_LIGHT
	light_init();
#ifdef	TEST
	test_light();
#endif
#endif
}

