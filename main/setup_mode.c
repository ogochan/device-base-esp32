#define	TRACE

#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>

#include 	"config.h"
#include "nvs.h"
#include "nvs_flash.h"
#include 	"freertos/FreeRTOS.h"
#include 	"freertos/task.h"
#include	"freertos/event_groups.h"
#include 	<esp_system.h>
#include	<esp_http_server.h>
#include 	"esp_log.h"
#include	"time.h"
#include	"sys/time.h"
#include	"cJSON.h"
#include	"bt.h"
#include	"wifi.h"
#include	"light.h"
#include	"fan.h"
#include	"httpc.h"
#include	"sntp.h"
#include	"api_client.h"
#include	"setup_mode.h"
#include	"globals.h"
#include	"misc.h"
#include 	"debug.h"


static	void
_wifi_scan(
	char	*buff)
{
	wifi_scan_start();
	wifi_scan_get(buff);
	ESP_LOGI(TAG, "%s", buff);
}

static	esp_err_t
bt_wifi_scan(void)
{
	_wifi_scan(ResponseMessage);
	bt_send_string(ResponseMessage);
	bt_send_string("\n\n");

    return ESP_OK;
}

static	esp_err_t
http_wifi_scan(
	httpd_req_t	*req)
{
	_wifi_scan(ResponseMessage);
    httpd_resp_send(req, ResponseMessage, strlen(ResponseMessage));

    return ESP_OK;
}

static	void
_wifi_config(
	char	*body)
{
	cJSON	*root
		,	*node;
	
	ESP_LOGI(TAG, "config: %s", body);
	root = cJSON_Parse(body);
	if ( root != NULL )	{
		node = cJSON_GetObjectItemCaseSensitive(root, "ssid");
		if ( cJSON_IsString(node) )	{
			strcpy(ap_ssid, node->valuestring);
		} else {
			strcpy(ap_ssid, "** none **");
			ERROR("not string ssid");
		}
		node = cJSON_GetObjectItemCaseSensitive(root, "pass");
		if	( cJSON_IsString(node) )	{
			strcpy(ap_pass, node->valuestring);
		} else {
			strcpy(ap_pass, "** none **");
			ERROR("not string pass");
		}
		ESP_LOGI(TAG, "ssid: %s", ap_ssid);
		ESP_LOGI(TAG, "pass: %s", ap_pass);
	} else {
		const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGI(TAG, "Error before: %s\n", error_ptr);
        }
	}
	cJSON_Delete(root);
  error:;
}

static	esp_err_t
http_wifi_config(
	httpd_req_t	*req)
{
	int		ret;

	ret = httpd_req_recv(req, ResponseMessage, req->content_len);
	ResponseMessage[req->content_len] = 0;
	//ESP_LOGI(TAG, "%s", ResponseMessage);
	_wifi_config(ResponseMessage);

    httpd_resp_send(req, "OK", 3);
    return ESP_OK;
}

static	void
_led_update(
	char	*body)
{
	cJSON	*root
		,	*_red
		,	*_green
		,	*_blue
		,	*_white
		,	*_no;
	int		red
		,	green
		,	blue
		,	white
		,	no;
	
	ESP_LOGI(TAG, "config: %s", body);
	root = cJSON_Parse(body);
	if ( root != NULL )	{
		_red = cJSON_GetObjectItemCaseSensitive(root, "red");
		if ( cJSON_IsString(_red) )	{
			red = atoi(_red->valuestring);
		} else
		if	( cJSON_IsNumber(_red) )	{
			red = _red->valueint;
		} else {
			red = 0;
		}
		_green = cJSON_GetObjectItemCaseSensitive(root, "green");
		if	( cJSON_IsString(_green) )	{
			green = atoi(_green->valuestring);
		} else
		if	( cJSON_IsNumber(_green) )	{
			green = _green->valueint;
		} else {
			green = 0;
		}
		_blue = cJSON_GetObjectItemCaseSensitive(root, "blue");
		if	( cJSON_IsString(_blue) )	{
			blue = atoi(_blue->valuestring);
		} else
		if	( cJSON_IsNumber(_blue) )	{
			blue = _blue->valueint;
		} else {
			blue = 0;
		}
		_white = cJSON_GetObjectItemCaseSensitive(root, "white");
		if	( cJSON_IsString(_white) )	{
			white = atoi(_white->valuestring);
		} else
		if	( cJSON_IsNumber(_white) )	{
			white = _white->valueint;
		} else {
			white = 0;
		}
		_no = cJSON_GetObjectItemCaseSensitive(root, "no");
		if	( cJSON_IsString(_no) )	{
			no = atoi(_no->valuestring);
		} else
		if	( cJSON_IsNumber(_no) )	{
			no = _no->valueint;
		} else {
			no = -1;
		}

		ESP_LOGI(TAG, "red: %d", red);
		ESP_LOGI(TAG, "green: %d", green);
		ESP_LOGI(TAG, "blue: %d", blue);
		ESP_LOGI(TAG, "white: %d", white);
		ESP_LOGI(TAG, "no: %d", no);
		if	( no < 0 )	{
			light_set_all_color(red, green, blue, white);
		} else {
			light_set_color(no, red, green, blue, white);
		}
	} else {
		const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGI(TAG, "Error before: %s\n", error_ptr);
        }
	}
	cJSON_Delete(root);
}

static	esp_err_t
http_led_update(
	httpd_req_t	*req)
{
	int		ret;

	ret = httpd_req_recv(req, ResponseMessage, req->content_len);
	ResponseMessage[req->content_len] = 0;

	_led_update(ResponseMessage);

    httpd_resp_send(req, "OK", 3);
    return ESP_OK;
}

static	void
_fan_update(
	char	*body)
{
	cJSON	*root
		,	*_switch;
	Bool	on;
	
	ESP_LOGI(TAG, "config: %s", body);
	root = cJSON_Parse(body);
	on = FALSE;
	if ( root != NULL )	{
		_switch = cJSON_GetObjectItemCaseSensitive(root, "switch");
		if ( cJSON_IsBool(_switch) )	{
			if	( cJSON_IsTrue(_switch) )	{
				on = TRUE;
				fan_switch(on);
			} else
			if	( cJSON_IsFalse(_switch) )	{
				on = FALSE;
				fan_switch(on);
			}
		}
		ESP_LOGI(TAG, "fan: %s", on ? "on": "off");
	} else {
		const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGI(TAG, "Error before: %s\n", error_ptr);
        }
	}
	cJSON_Delete(root);
}

static	esp_err_t
http_fan_update(
	httpd_req_t	*req)
{
	int		ret;

	ret = httpd_req_recv(req, ResponseMessage, req->content_len);
	ResponseMessage[req->content_len] = 0;

	_fan_update(ResponseMessage);

    httpd_resp_send(req, "OK", 3);
    return ESP_OK;
}

static	Bool
_device_new(
	char	*body)
{
	cJSON	*root
		,	*node;
	char	user[SIZE_USER + 1]
		,	pass[SIZE_PASS + 1];
	Bool	ret;
	esp_http_client_handle_t	client;
	Bool	rc;

	ESP_LOGI(TAG, "config: %s", body);
	ret = FALSE;
	*user = 0;
	*pass = 0;
	*my_device_id = 0;
	*my_session_key = 0;
	rc = TRUE;

	root = cJSON_Parse(body);
	if ( root == NULL )	{
		rc = FALSE;
	} else {
		node = cJSON_GetObjectItemCaseSensitive(root, "user");
		if ( cJSON_IsString(node) )	{
			strncpy(user, node->valuestring, SIZE_USER+1);
		}
		node = cJSON_GetObjectItemCaseSensitive(root, "pass");
		if ( cJSON_IsString(node) )	{
			strncpy(pass, node->valuestring, SIZE_PASS+1);
		}
		cJSON_Delete(root);
		if		(	( *user != 0 )
				&&	( *pass != 0 ) )	{
			client = initialize_httpc();
			rc = api_device_new(client, user, pass, my_device_id);
			if	( rc )	{
				rc = api_session_new(client, user, pass, my_device_id, my_session_key);
				if	( rc )	{
					open_device_info();
					set_device_id(my_device_id);
					set_session_key(my_session_key);
					commit_device_info();
					close_device_info();
				}
			}
			finish_httpc(client);
		}
	}
	return	(ret);
}

static	esp_err_t
http_device_new(
	httpd_req_t	*req)
{
	int		ret;

	ret = httpd_req_recv(req, ResponseMessage, req->content_len);
	ResponseMessage[req->content_len] = 0;
	if	( _device_new(ResponseMessage) )	{
		httpd_resp_send(req, "OK", 3);
	} else {
		httpd_resp_send(req, "NG", 3);
	}
    return ESP_OK;
}

static	const	httpd_uri_t	handlers[] = {
    {
		.uri      = "/wifi/scan",
		.method   = HTTP_GET,
		.handler  = http_wifi_scan,
		.user_ctx = NULL,
    },
    { .uri      = "/wifi/config",
      .method   = HTTP_POST,
      .handler  = http_wifi_config,
      .user_ctx = NULL,
    },
    { .uri      = "/led/update",
      .method   = HTTP_POST,
      .handler  = http_led_update,
      .user_ctx = NULL,
    },
    { .uri      = "/fan/update",
      .method   = HTTP_POST,
      .handler  = http_fan_update,
      .user_ctx = NULL,
    },
    { .uri      = "/device",
      .method   = HTTP_POST,
      .handler  = http_device_new,
      .user_ctx = NULL,
    },
};

static	void
initialize_setup_mode(void)
{
    httpd_handle_t	hd;
    httpd_config_t	config = HTTPD_DEFAULT_CONFIG();
    int		i;

    config.server_port = 80;
    config.max_open_sockets = 2;

    if (httpd_start(&hd, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Started HTTP server on port: '%d'", config.server_port);
        ESP_LOGI(TAG, "Max URI handlers: '%d'", config.max_uri_handlers);
        ESP_LOGI(TAG, "Max Open Sessions: '%d'", config.max_open_sockets);
        ESP_LOGI(TAG, "Max Header Length: '%d'", HTTPD_MAX_REQ_HDR_LEN);
        ESP_LOGI(TAG, "Max URI Length: '%d'", HTTPD_MAX_URI_LEN);
        ESP_LOGI(TAG, "Max Stack Size: '%d'", config.stack_size);

		for (i = 0; i < sizeof(handlers)/sizeof(httpd_uri_t); i++) {
			ESP_LOGI(TAG, "path %s", handlers[i].uri);
			if	( httpd_register_uri_handler(hd, &handlers[i]) != ESP_OK )	{
				ESP_LOGW(TAG, "register uri failed for %d", i);
				return;
			}
		}
	}

}

extern	void
load_device_info(void)
{
	open_device_info();

	get_device_id(my_device_id);
	get_session_key(my_session_key);
	get_my_ssid(my_ssid);
	get_my_pass(my_pass);
	get_ap_ssid(ap_ssid);
	get_ap_pass(ap_pass);

	close_device_info();
	ESP_LOGI(TAG, "ap ssid    : %s", ap_ssid);
	ESP_LOGI(TAG, "ap pass    : %s", ap_pass);
	ESP_LOGI(TAG, "device uuid: %s", my_device_id);
	ESP_LOGI(TAG, "session key: %s", my_session_key);
}

extern	void
destroy_device_info(void)
{
	open_device_info();

	memclear(my_ssid, SIZE_SSID + 1);
	memclear(my_pass, SIZE_PASS + 1);
	memclear(my_device_id, SIZE_UUID + 1);
	memclear(my_session_key, SIZE_UUID + 1);
	memclear(ap_ssid, SIZE_SSID + 1);
	memclear(ap_pass, SIZE_PASS + 1);

	set_my_ssid(my_ssid);
	set_my_pass(my_pass);
	set_device_id(my_device_id);
	set_session_key(my_session_key);
	set_ap_ssid(ap_ssid);
	set_ap_pass(ap_pass);

	close_device_info();
}

extern	void
SetupMode(void)
{
ENTER_FUNC;
	initialize_setup_mode();
LEAVE_FUNC;
}

