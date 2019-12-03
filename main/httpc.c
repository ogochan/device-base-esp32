#define	TRACE

#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>

#include 	"config.h"
#include	<nvs.h>
#include	<nvs_flash.h>
#include 	<freertos/FreeRTOS.h>
#include 	<freertos/task.h>
#include	<freertos/event_groups.h>
#include	<freertos/semphr.h>
#include 	<esp_system.h>
#include	<esp_http_client.h>
#include	<esp_https_ota.h>
#include 	<esp_log.h>
#include	<time.h>
#include	<sys/time.h>
#include	<cJSON.h>
#include	"types.h"
#include	"httpc.h"
#include	"globals.h"
#include	"misc.h"
#include 	"debug.h"
#include	"version.h"

extern	const	char	server_crt_start[]	asm("_binary_server_crt_start");

static	SemaphoreHandle_t	http_semaphore = NULL;

static	esp_err_t
_http_event_handle(
	esp_http_client_event_t	*evt)
{
	switch(evt->event_id) {
	  case HTTP_EVENT_ERROR:
		ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
		break;
	  case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
		break;
	  case HTTP_EVENT_HEADER_SENT:
		ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
		break;
	  case HTTP_EVENT_ON_HEADER:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
		dbgprintf("%.*s", evt->data_len, (char*)evt->data);
		break;
	  case HTTP_EVENT_ON_DATA:
#if	0
		ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		if (!esp_http_client_is_chunked_response(evt->client)) {
			printf("%.*s", evt->data_len, (char*)evt->data);
		}
#endif
		break;
	  case HTTP_EVENT_ON_FINISH:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
		break;
	  case HTTP_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
		break;
	}
	return ESP_OK;
}

extern	esp_http_client_handle_t
initialize_httpc(void)
{
	esp_http_client_handle_t	client;

	esp_http_client_config_t config = {
		.url = CONSOLE_HOST,
		.event_handler = _http_event_handle,
#ifdef	USE_LOCALCA
		.use_global_ca_store = true,
#endif
		.cert_pem = (char *)server_crt_start
	};
	if		( http_semaphore == NULL )	{
		http_semaphore = xSemaphoreCreateBinary();
	} else {
		xSemaphoreTake(http_semaphore, portMAX_DELAY);
	}
	client = esp_http_client_init(&config);

	return	(client);
}

extern	esp_err_t
httpc_set_header(
	esp_http_client_handle_t client,
	char	*key,
	char	*value)
{
	esp_err_t	ret;

	if	( value ) {
		ret = esp_http_client_set_header(client, key, value);
	} else {
		ret = esp_http_client_delete_header(client, key);
	}
	return	(ret);
}

extern	esp_err_t
httpc_post_open(
	esp_http_client_handle_t client,
	char	*host,
	char	*path,
	size_t	size)
{
	esp_err_t	ret;

	if	( host )	{
		sprintf(url_buff, "%s%s", host, path);
	} else {
		strcpy(url_buff, path);
	}
	ESP_LOGI(TAG, "POST URL: %s", url_buff);
	ret = esp_http_client_set_url(client, url_buff);
	if ( ret != ESP_OK )	goto	err;
	ret = esp_http_client_set_method(client, HTTP_METHOD_POST);
	if ( ret != ESP_OK )	goto	err;

	ret = esp_http_client_open(client, size);
  err:;
	return	(ret);
}

extern	esp_err_t
httpc_get_open(
	esp_http_client_handle_t client,
	char	*host,
	char	*path)
{
	esp_err_t	ret;

	if	( host )	{
		sprintf(url_buff, "%s%s", host, path);
	} else {
		strcpy(url_buff, path);
	}
	ESP_LOGI(TAG, "GET  URL: %s", url_buff);
	ret = esp_http_client_set_url(client, url_buff);
	if ( ret != ESP_OK )	goto	err;
	ret = esp_http_client_set_method(client, HTTP_METHOD_GET);
	if ( ret != ESP_OK )	goto	err;


	ret = esp_http_client_open(client, 0);
  err:;
	return	(ret);
}

extern	esp_err_t
httpc_close(
	esp_http_client_handle_t client)
{
	esp_err_t	ret;

	ret = esp_http_client_close(client);
	return	(ret);
}

extern	esp_err_t
httpc_post(
	esp_http_client_handle_t client,
	char	*host,
	char	*path,
	uint8_t	*data,
	size_t	len)
{
	esp_err_t	ret;

	sprintf(url_buff, "%s%s", host, path);
	ret = esp_http_client_set_url(client, url_buff);
	if ( ret != ESP_OK )	goto	err;
	ret = esp_http_client_set_post_field(client, (char *)data, len);
	if ( ret != ESP_OK )	goto	err;
	ret = esp_http_client_set_method(client, HTTP_METHOD_POST);
	if ( ret != ESP_OK )	goto	err;
	ret = esp_http_client_perform(client);
  err:;
	xSemaphoreGive(http_semaphore);
	return	(ret);
}

extern	esp_err_t
httpc_get(
	esp_http_client_handle_t client,
	char	*host,
	char	*path)
{
	esp_err_t	ret;

	sprintf(url_buff, "%s%s", host, path);
#ifdef	TRACE
	ESP_LOGI(TAG, "URL: %s", url_buff);
#endif
	ret = esp_http_client_set_url(client, url_buff);
	if ( ret != ESP_OK )	goto	err;
	ret = esp_http_client_set_method(client, HTTP_METHOD_GET);
	if ( ret != ESP_OK )	goto	err;
	ret = esp_http_client_perform(client);
  err:;
	xSemaphoreGive(http_semaphore);
	return	(ret);
}

extern	int
httpc_get_data(
	esp_http_client_handle_t client,
	uint8_t	*buff,
	size_t	len)
{
	return	esp_http_client_read(client, (char *)buff, len);
}

extern	int
httpc_get_status(
	esp_http_client_handle_t client)
{
	return	esp_http_client_get_status_code(client);
}

extern	void
finish_httpc(
	esp_http_client_handle_t client)
{
	esp_http_client_cleanup(client);
	xSemaphoreGive(http_semaphore);
}

#define HASH_LEN 32 /* SHA-256 digest length */

#if	0
static void print_sha256 (const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s: %s", label, hash_print);
}
#endif

static	Bool	ota_running;

typedef	struct	{
	char	host[SIZE_HOST+1];
	char	path[SIZE_PATH+1];
	char	session_key[SIZE_UUID+1];
}	ota_args;

static	void
ota_task(
	ota_args	*args)
{
	Bool	fatal;
	Bool	need_update;
    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;
    esp_http_client_handle_t client;
    size_t	binary_file_length;
	int		data_read;
	esp_app_desc_t	new_app_info;

ENTER_FUNC;
	dbgprintf("Starting OTA: %s%s", args->host, args->path);

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        dbgprintf("Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
				  configured->address, running->address);
        dbgmsg("(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    dbgprintf("Running partition type %d subtype %d (offset 0x%08x)",
			  running->type, running->subtype, running->address);
	
	fatal = FALSE;
	need_update = TRUE;

	client = initialize_httpc();
    if	( client == NULL )	{
        dbgmsg("Failed to initialise HTTP connection");
		fatal = TRUE;
		goto	finish;
    }

	esp_http_client_set_header(client, "X-SESSION-KEY", args->session_key);
    err = httpc_get_open(client, args->host, args->path);
    if	(err != ESP_OK) {
        dbgprintf("Failed to open HTTP connection: %s", esp_err_to_name(err));
		fatal = TRUE;
		goto	finish;
    }
    esp_http_client_fetch_headers(client);

    update_partition = esp_ota_get_next_update_partition(NULL);
	if	( update_partition )	{
		dbgprintf("Writing to partition subtype %d at offset 0x%x",
				  update_partition->subtype, update_partition->address);
	}

	
	data_read = esp_http_client_read(client, Message, SIZE_MESSAGE_BUFFER);
	if	( data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t) )	{
		// check current version with downloading
		memcpy(&new_app_info, &Message[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
		dbgprintf("New firmware version: %s", new_app_info.version);

		esp_app_desc_t	running_app_info;
		if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
			dbgprintf("Running firmware version: %s", running_app_info.version);
		}

		const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
		esp_app_desc_t invalid_app_info;
		if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
			dbgprintf("Last invalid firmware version: %s", invalid_app_info.version);
		}
		// check current version with last invalid partition
		if	( last_invalid_app != NULL )	{
			if	( !memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) ) {
				dbgmsg("New version is the same as invalid version.");
				dbgprintf("Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
				dbgmsg("The firmware has been rolled back to the previous version.");
				need_update = FALSE;
			}
		}
		
		if		( !strncmp(new_app_info.project_name, PROJECT_NAME, sizeof(new_app_info.project_name)) )	{
			if	( strncmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) <= 0 )	{
				dbgmsg("Current running version is the same as a new. We will not continue the update.");
				need_update = FALSE;
			}
			if	( need_update )	{
				err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
				if (err == ESP_OK) {
					dbgmsg("esp_ota_begin succeeded");
				} else {
					dbgprintf("esp_ota_begin failed (%s)", esp_err_to_name(err));
					fatal = TRUE;
					need_update = FALSE;
				}
			}
		} else {
			dbgmsg("not match project_name. We will not continue the update.");
			need_update = FALSE;
		}
	} else {
		dbgmsg("Error: SSL data read error");
		fatal = TRUE;
	}
	dbgprintf("need_update %s", need_update ? "TRUE" : "FALSE" );
	if	( need_update )	{
		binary_file_length = 0;
		do	{
			err = esp_ota_write( update_handle, (const void *)Message, data_read);
			if (err != ESP_OK) {
				fatal = TRUE;
				break;
			}
			binary_file_length += data_read;
			dbgprintf("Written image length %d", binary_file_length);
			data_read = esp_http_client_read(client, Message, SIZE_MESSAGE_BUFFER);
		}	while	( data_read > 0 );
		if	( data_read == 0 )	{
			dbgmsg("Connection closed,all data received");
			dbgprintf("Total Write binary data length : %d", binary_file_length);
			if ( esp_http_client_is_complete_data_received(client) != true ) {
				dbgmsg("Error in receiving complete file");
				fatal = TRUE;
			}
			if (esp_ota_end(update_handle) != ESP_OK) {
				dbgmsg("esp_ota_end failed!");
				fatal = TRUE;
			}
		
			err = esp_ota_set_boot_partition(update_partition);
			if (err != ESP_OK) {
				dbgprintf("esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
				fatal = TRUE;
			}
		} else {
			dbgmsg("received package is not fit len");
			fatal = TRUE;
		}
	}
  finish:;
	if	( client )	{
		httpc_close(client);
		finish_httpc(client);
	}
	if	( fatal )	{
		dbgmsg("Exiting task due to fatal error...");
	} else {
		if	( need_update )	{
			dbgmsg("Prepare to restart system!");
			esp_restart();
			msleep(5000);
		}
	}
	ota_running = FALSE;
	vTaskDelete(NULL);
LEAVE_FUNC;
    return ;
}

extern	void
httpc_ota(
	char	*host,
	char	*path,
	char	*session_key)
{
	static	ota_args	args;

	ota_running = TRUE;
	ESP_LOGI(TAG, "OTA URI: %s%s", host, path);

	strcpy(args.host, host);
	strcpy(args.path, path);
	strcpy(args.session_key, session_key);
    (void)xTaskCreate(ota_task, "ota_task", 8192, &args, 5, NULL);
}
