#define	TRACE

#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>

#include 	"config.h"
#include 	<freertos/FreeRTOS.h>
#include 	<freertos/task.h>
#include 	<esp_system.h>
#include	<esp_tls.h>
#include 	<esp_log.h>
#include	<time.h>
#include	<sys/time.h>
#include	<cJSON.h>
#include	"httpc.h"
#include	"globals.h"
#include	"misc.h"
#include 	"debug.h"

#ifdef	USE_LOCALCA
extern	const	unsigned	char	ca_key_start[]		asm("_binary_ca_crt_start");
extern	const	unsigned	char	ca_key_end[]		asm("_binary_ca_crt_end");
#endif

extern	void
initialize_api(void)
{
#ifdef	USE_LOCALCA
	esp_tls_init_global_ca_store();
	esp_tls_set_global_ca_store(ca_key_start, ca_key_end - ca_key_start);
#endif
}

extern	Bool
api_device_new(
	esp_http_client_handle_t	client,
	char	*user,
	char	*pass,
	char	*device_id)
{
	Bool	rc;
	cJSON	*root
		,	*node;
	int		size
		,	status;

	rc = FALSE;
	sprintf(Message, "{\"user\":\"%s\",\"pass\":\"%s\",\"class\":\"jar-garden\"}", user, pass);
	if	( httpc_post(client, CONSOLE_HOST, "/devices", (uint8_t *)Message, strlen(Message)) != ESP_OK )	goto	err;
	status =  httpc_get_status(client);
	dbgprintf("status: %d", status);
	if	( ( size = httpc_get_data(client, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER) ) < 0 )	goto	err;
	dbgprintf("get: %s", ResponseMessage);
	if		( status != 200 )	goto	err;
	root = cJSON_Parse(ResponseMessage);
	if	( root )	{
		node = cJSON_GetObjectItemCaseSensitive(root, "code");
		if 		(	( cJSON_IsNumber(node) )
				&&	( node->valueint == 0  ))	{
			node = cJSON_GetObjectItemCaseSensitive(root, "uuid");
			if	( cJSON_IsString(node) )	{
				strncpy(device_id, node->valuestring, SIZE_UUID+1);
				dbgprintf("device uuid: %s", device_id);
				rc = TRUE;
			}
		}
		cJSON_Delete(root);
	}
  err:;
	return	(rc);
}

extern	Bool
api_session_new(
	esp_http_client_handle_t	client,
	char	*user,
	char	*pass,
	char	*device_id,
	char	*session_key)
{
	Bool	rc;
	cJSON	*root
		,	*node;
	int		size
		,	status;


	rc = FALSE;
	sprintf(Message, "{\"user\":\"%s\",\"pass\":\"%s\",\"device\":\"%s\"}", user, pass, device_id);
	if ( httpc_post(client, CONSOLE_HOST, "/session", (uint8_t *)Message, strlen(Message)) != ESP_OK )	goto	err;
	status =  httpc_get_status(client);
	dbgprintf("status: %d", status);
	if	( status != 200 )	goto	err;
	if	( ( size = httpc_get_data(client, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER) ) < 0 )	goto	err;
	dbgprintf("get: %s", ResponseMessage);
	root = cJSON_Parse(ResponseMessage);
	if	( root )	{
		node = cJSON_GetObjectItemCaseSensitive(root, "code");
		if 		(	( cJSON_IsNumber(node) )
				&&	( node->valueint == 0  ))	{
			node = cJSON_GetObjectItemCaseSensitive(root, "session_key");
			if		(	( node )
					&&	( cJSON_IsString(node) ) )	{
				strncpy(session_key, node->valuestring, SIZE_UUID+1);
				dbgprintf("session key: %s", session_key);
				rc = TRUE;
			}
		}
		cJSON_Delete(root);
	}
  err:;
	return	(rc);
}

extern	Bool
api_post_data(
	esp_http_client_handle_t	client,
	char	*path,
	char	*device_id,
	char	*session_key,
	uint8_t	*data,
	size_t	size)
{
	Bool	rc;
	cJSON	*root
		,	*node;
	int		status;

	char	real_path[SIZE_PATH+1];

	rc = FALSE;
	sprintf(real_path, "/device/%s%s", device_id, path);
	httpc_set_header(client, "X-SESSION-KEY", session_key);
	if ( httpc_post(client, CONSOLE_HOST, real_path, data, size) != ESP_OK )	goto	err;
	status =  httpc_get_status(client);
	dbgprintf("status: %d", status);
	if	( status != 201 )	goto	err;
	if	( httpc_get_data(client, (uint8_t *)ResponseMessage, SIZE_RESPONSE_BUFFER) < 0 )	goto	err;
	dbgprintf("get: %s", ResponseMessage);
	root = cJSON_Parse(ResponseMessage);
	if	( root )	{
		node = cJSON_GetObjectItemCaseSensitive(root, "code");
		if 		(	( cJSON_IsNumber(node) )
				&&	( node->valueint == 0  ))	{
			dbgmsg("OK");
			rc = TRUE;
		}
		cJSON_Delete(root);
	}
  err:;
	return	(rc);
}

extern	int
api_get_data(
	esp_http_client_handle_t	client,
	const	char	*path,
	char	*device_id,
	char	*session_key,
	uint8_t	*data,
	size_t	size)
{
	int		rc;
	int		status;

	static	char	real_path[SIZE_PATH+1];

	rc = -1;
	sprintf(real_path, "/device/%s%s", device_id, path);
	esp_http_client_set_header(client, "X-SESSION-KEY", session_key);
	if ( httpc_get(client, CONSOLE_HOST, real_path) != ESP_OK )	goto	err;
	status =  httpc_get_status(client);
	dbgprintf("status: %d", status);
	if	( status != 200 )	goto	err;
	rc = httpc_get_data(client, data, size);
	dbgprintf("get: %s", ResponseMessage);
  err:;
	return	(rc);
}

extern	Bool
api_exec_ota(void)
{
	char	real_path[SIZE_PATH+1];

ENTER_FUNC;
	sprintf(real_path, "/device/%s/farm", my_device_id);
LEAVE_FUNC;
	return	(httpc_ota(CONSOLE_HOST, real_path, my_session_key));
}

