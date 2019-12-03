#define	TRACE

#include 	<stdio.h>
#include	<sys/time.h>
#include	<string.h>

#include 	"freertos/FreeRTOS.h"
#include 	"freertos/task.h"
#include	"freertos/event_groups.h"

#include 	"esp_system.h"
#include	"esp_wifi.h"
#include	"esp_event.h"
#include	"esp_event_loop.h"
#include 	"esp_log.h"
#include	"time.h"

#include	"lwip/err.h"
#include	"lwip/sys.h"

#include 	"config.h"
#include	"wifi.h"
#include	"types.h"
#include	"globals.h"
#include	"misc.h"
#include 	"debug.h"

#define	NUM_APS				32

#define	WIFI_CONNECTED_BIT	BIT0

static	EventGroupHandle_t	wifi_event_group;
static	int					s_retry_num = 0;
static	wifi_ap_record_t	aps[NUM_APS];
static	char				*scan_buffer;
static	Bool	wifi_started;

static	esp_err_t
wifi_event_handler(
	void* arg,
	esp_event_base_t event_base, 
	int32_t event_id,
	void* event_data)
{
	ip_event_got_ip_t	*event = (ip_event_got_ip_t*)event_data;
	wifi_event_ap_staconnected_t *wifi_event = (wifi_event_ap_staconnected_t *) event_data;

	switch(event_id) {
	  case SYSTEM_EVENT_STA_START:
        //esp_wifi_connect();
        break;
	  case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->ip_info.ip));
		wifi_started = TRUE;
        break;
	  case SYSTEM_EVENT_STA_DISCONNECTED:
		//esp_wifi_connect();
		ESP_LOGI(TAG,"connect to the AP fail\n");
		break;
	  case	WIFI_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(wifi_event->mac), wifi_event->aid);
		break;
	  case	WIFI_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(wifi_event->mac), wifi_event->aid);
		break;
	  default:
        break;
    }
    return ESP_OK;
}

static	wifi_config_t wifi_config;

extern	esp_err_t
initialize_wifi(void)
{
	esp_err_t err;

	tcpip_adapter_init();
	wifi_event_group = xEventGroupCreate();

	ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
	wifi_init_config_t	cfg = WIFI_INIT_CONFIG_DEFAULT();
	ERROR_CHECK( esp_wifi_init(&cfg) );
    ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL) );
    ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL) );
	ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM));

	wifi_started = FALSE;
  err_exit:
    return	(err);
}

extern	void
wifi_ap_start(
	char	*ssid,
	char	*pass)
{
    wifi_config_t wifi_config = {
        .ap = {
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
	strcpy((char *)wifi_config.ap.ssid, ssid);
	wifi_config.ap.ssid_len = strlen( ssid);
	strcpy((char *)wifi_config.ap.password, pass);

    if	( strlen(pass) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    //ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
             my_ssid, my_pass);
	wifi_started = TRUE;
}

extern	void
wifi_scan_start(void)
{
    ESP_ERROR_CHECK( esp_wifi_start() );
}

extern	void
wifi_scan_get(
	char	*p)
{
	wifi_scan_config_t	scan_config = {
		.ssid = NULL,
		.bssid = NULL,
		.channel = 0,
		.show_hidden = 0,
		.scan_type = WIFI_SCAN_TYPE_ACTIVE,
	};
	uint16_t	num;
	uint16_t	gnum;
	char	*q;
	q = p;
	ESP_ERROR_CHECK( esp_wifi_scan_start(&scan_config, true) );
	ESP_ERROR_CHECK( esp_wifi_scan_get_ap_num(&num) );
	ESP_LOGI(TAG, "AP number = %d", (int)num);
	gnum = NUM_APS;
	ESP_ERROR_CHECK( esp_wifi_scan_get_ap_records(&gnum, aps) );
	ESP_LOGI(TAG, "AP get = %d", (int)gnum);
	*p ++ = '[';
	for	( int i = 0 ; i < gnum; i ++ )	{
		if ( i > 0 )	{
			*p ++ = ',';
		}
		p += sprintf(p, "{\"ssid\": \"%s\", \"rssi\": %d}", aps[i].ssid, (int)aps[i].rssi);
		ESP_LOGI(TAG, "SSID: %s rssi %d", aps[i].ssid, (int)aps[i].rssi);
	}
	*p ++ = ']';
	*p = 0;
	ESP_ERROR_CHECK( esp_wifi_scan_stop() );
}

extern	void
wifi_connect(
	char	*ssid,
	char	*password)
{
	strcpy((char *)wifi_config.sta.ssid, ssid);
	strcpy((char *)wifi_config.sta.password, password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );

    ESP_ERROR_CHECK(esp_wifi_start() );
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s", ssid, password);
    ESP_ERROR_CHECK(esp_wifi_connect() );

}

extern	Bool
wifi_is_valid(void)
{
	return	(wifi_started);
}

extern	void
wifi_stop(void)
{
	if	( wifi_started )	{
		ESP_ERROR_CHECK( esp_wifi_stop() );
		wifi_started = FALSE;
	}
}
