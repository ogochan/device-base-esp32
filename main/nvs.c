#include	<esp_system.h>
#include	<nvs.h>
#include	<nvs_flash.h>

#include 	"config.h"
#include	"types.h"
#include	"misc.h"
#include	"nvs.h"
#include 	"debug.h"

static	nvs_handle_t	reset_handle;
static	nvs_handle_t	device_info_handle;

extern	esp_err_t
initialize_nvs(void)
{
	esp_err_t err;
	
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return	(err);
}

extern	void
open_device_info(void)
{
	esp_err_t err;

	err = nvs_open("deviceinfo", NVS_READWRITE, &device_info_handle);
	ESP_ERROR_CHECK(err);
}

extern	void
set_device_id(
	char	*uuid)
{
	esp_err_t err;
	err = nvs_set_str(device_info_handle, "device_id", uuid);
	ESP_ERROR_CHECK(err);
}

extern	void
get_device_id(
	char	*uuid)
{
	esp_err_t	err;
	size_t		size;
	memclear(uuid, SIZE_UUID + 1);
	err = nvs_get_str(device_info_handle, "device_id", uuid, &size);
}

extern	void
set_session_key(
	char	*uuid)
{
	esp_err_t err;
	err = nvs_set_str(device_info_handle, "session_key", uuid);
	ESP_ERROR_CHECK(err);
}

extern	void
get_session_key(
	char	*uuid)
{
	esp_err_t	err;
	size_t		size;
	memclear(uuid, SIZE_UUID + 1);
	err = nvs_get_str(device_info_handle, "session_key", uuid, &size);
}

extern	void
set_my_ssid(
	char	*ssid)
{
	esp_err_t err;
	err = nvs_set_str(device_info_handle, "my_ssid", ssid);
	ESP_ERROR_CHECK(err);
}

extern	void
get_my_ssid(
	char	*ssid)
{
	esp_err_t	err;
	size_t		size;
	memclear(ssid, SIZE_SSID + 1);
	err = nvs_get_str(device_info_handle, "my_ssid", ssid, &size);
	if		(	( err != ESP_OK )
			||	( *ssid == 0 ) )	{
		strcpy(ssid, DEFAULT_SSID);
	}
}

extern	void
set_my_pass(
	char	*pass)
{
	esp_err_t err;
	err = nvs_set_str(device_info_handle, "my_pass", pass);
	ESP_ERROR_CHECK(err);
}

extern	void
get_my_pass(
	char	*pass)
{
	esp_err_t	err;
	size_t		size;
	memclear(pass, SIZE_PASS + 1);
	err = nvs_get_str(device_info_handle, "my_pass", pass, &size);
	if		(	( err != ESP_OK )
			||	(  *pass == 0 ) )	{
		strcpy(pass, DEFAULT_PASS);
	}
}

extern	void
set_ap_ssid(
	char	*ssid)
{
	esp_err_t err;
	err = nvs_set_str(device_info_handle, "ap_ssid", ssid);
	ESP_ERROR_CHECK(err);
}

extern	void
get_ap_ssid(
	char	*ssid)
{
	esp_err_t	err;
	size_t		size;
	memclear(ssid, SIZE_SSID + 1);
	err = nvs_get_str(device_info_handle, "ap_ssid", ssid, &size);
	if	( err != ESP_OK )	{
		strcpy(ssid, DEFAULT_AP_SSID);
	}
}

extern	void
set_ap_pass(
	char	*pass)
{
	esp_err_t err;
	err = nvs_set_str(device_info_handle, "ap_pass", pass);
	ESP_ERROR_CHECK(err);
}

extern	void
get_ap_pass(
	char	*pass)
{
	esp_err_t	err;
	size_t		size;
	memclear(pass, SIZE_PASS + 1);
	err = nvs_get_str(device_info_handle, "ap_pass", pass, &size);
	if	( err != ESP_OK )	{
		strcpy(pass, DEFAULT_AP_PASS);
	}
}

extern	void
commit_device_info(void)
{
	esp_err_t err;

	err = nvs_commit(device_info_handle);
    ESP_ERROR_CHECK(err);
}

extern	void
close_device_info(void)
{
	nvs_close(device_info_handle);
}

extern	void
open_reset(void)
{
	esp_err_t err;

	err = nvs_open("reset", NVS_READWRITE, &reset_handle);
	ESP_ERROR_CHECK(err);
}

extern	int
get_reset(void)
{
	esp_err_t err;
	uint8_t		ret;

	ret = 0;
	err = nvs_get_u8(reset_handle, "count", &ret);
	if	( err == ESP_ERR_NVS_NOT_FOUND) {
		ret = 0;
	}
	return	(ret);
}

extern	void
set_reset(
	int		arg)
{
	esp_err_t err;
	uint8_t		ret = (uint8_t)arg;

	err = nvs_set_u8(reset_handle, "count", ret);
    ESP_ERROR_CHECK(err);
}
	
extern	void
commit_reset(void)
{
	esp_err_t err;

	err = nvs_commit(reset_handle);
    ESP_ERROR_CHECK(err);
}

extern	void
close_reset(void)
{
	nvs_close(reset_handle);
}
