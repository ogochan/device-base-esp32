#define	TRACE

#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>
extern "C"{
#include 	"config.h"
#include 	<freertos/FreeRTOS.h>
#include 	<freertos/task.h>
#include 	<esp_log.h>
}
#include 	"BME280_Barometric.h"
#include 	"BME280_Humidity.h"
#include 	"BME280_Temp.h"
#include	"Sensors.h"
#include	"I2C.h"
#include	"SensorInfo.h"
#include	"sensor.h"
extern "C"{
#include	"types.h"
#include	"httpc.h"
#include	"globals.h"
#include	"misc.h"
#include 	"debug.h"
}

static	SenseBuffer	*sense_buff;

#define	TAG	"sensor"

static	char	*message_pointer;

static	void
make_one_sensor_message(
	time_t	timeNow,
	SensorInfo	*info,
	SenseBuffer	*buff)
{
	char	*q = message_pointer;
	tTime	tm_now;

ENTER_FUNC;

	gmtime_r(&timeNow, &tm_now);

	message_pointer += sprintf(message_pointer, "{\"sensor_name\":\"%s\",", info->name());
	message_pointer += sprintf(message_pointer, "\"data_class_name\":\"%s\",", info->data_class_name());
	message_pointer += sprintf(message_pointer, "\"data\":{");
	message_pointer += sprintf(message_pointer, "\"at\":\"%d-%d-%d %02d:%02d:%02d +0000\",",
							   (tm_now.tm_year+1900), (tm_now.tm_mon+1), tm_now.tm_mday,
							   tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);
	message_pointer += info->build(message_pointer, buff);
	message_pointer += sprintf(message_pointer, "}}");
	ESP_LOGI(TAG, "size = %d", strlen(q));
LEAVE_FUNC;
}

static	size_t
net_print(
	esp_http_client_handle_t	client,
	char	*str)
{
	return	(esp_http_client_write(client, str, strlen(str)));
}

#define	net_printf(s, fmt, ...)		\
{					\
	char	buf[128];		\
	sprintf(buf, fmt, __VA_ARGS__);	\
	net_print(s, buf);		\
}

static	Bool
post_http(
	char	*host,
	char	*path,
	char	*session_key)
{
ENTER_FUNC;
	uint8_t	*pp;
	size_t	size;
	int		s;
	SensorInfo	*info;
	esp_http_client_handle_t	client;
	esp_err_t	ret;
	time_t	n;
	Bool	rc;

	rc = FALSE;
	sense_buff->rewind_read();
	ESP_LOGI(TAG, "buff %d bytes use", sense_buff->used_size());
	client = initialize_httpc();
	if	( client != NULL )	{
		while	(  !sense_buff->is_end() )	{
			pp = sense_buff->mark_read();
			sense_buff->get(&n, sizeof(time_t));
			size = 0;
			size ++;	//	[
			while	( sense_buff->current_value() != 0xFF )	{
				message_pointer = Message;
				info = Sensors::item(sense_buff->get_value());
				make_one_sensor_message(n, info, sense_buff);
				size += strlen(Message);
				if	( sense_buff->current_value() != 0xFF )	{
					size ++;	//	,
				}
			}
			size ++;	//	]

			sense_buff->rewind_read(pp);
			httpc_set_header(client, "X-SESSION-KEY", session_key);
			httpc_set_header(client, "Content-Type", "application/json");
			ret = httpc_post_open(client, host, path, size);
			if	( ret != ESP_OK )	goto	err;
			sense_buff->get(&n, sizeof(time_t));
			net_print(client, "[");
			while	( sense_buff->current_value() != 0xFF )	{
				message_pointer = Message;
				info = Sensors::item(sense_buff->get_value());
				dbgprintf("info->id = %d", (int)info->id);
				make_one_sensor_message(n, info, sense_buff);
				ESP_LOGI(TAG, "Message:%s", Message);
				net_print(client, Message);
				if	( sense_buff->current_value() != 0xFF )	{
					net_print(client, ",");
				}
			}
			(void)sense_buff->get_value();
			net_print(client, "]");
			msleep(100);
		}
		httpc_close(client);
		finish_httpc(client);
		sense_buff->rewind_write();
		rc = TRUE;
	}
  err:;
LEAVE_FUNC;
	return	(rc);
}

extern	Bool
sensor_send_server(void)
{
	char	real_path[SIZE_PATH+1];
	Bool	rc;
ENTER_FUNC;
	if		( sensor_url == NULL )	{
		sprintf(real_path, "%s/device/%s/data", CONSOLE_HOST, my_device_id);
	} else {
		strcpy(real_path, sensor_url);
	}
	rc = post_http(NULL, real_path, my_session_key);
LEAVE_FUNC;
	return	(rc);
}

extern	Bool
sensor_collect(void)
{
	time_t	timeNow;

	time(&timeNow);

	Sensors::collect(timeNow, sense_buff);
	return	(TRUE);
}

extern	Bool
sensor_collect(
	SensorInfo	*info)
{
	time_t	timeNow;

ENTER_FUNC;
	time(&timeNow);

	Sensors::collect(timeNow, sense_buff, info);
LEAVE_FUNC;
	return	(TRUE);
}

extern	void
initialize_sensors(void)
{
	initialize_i2c();

	sense_buff = new SenseBuffer(SIZE_OF_SENSOR_BUFF);

	Sensors::init();
	Sensors::add((SensorInfo*)new BME280_Barometric());
	Sensors::add((SensorInfo*)new BME280_Humidity());
	Sensors::add((SensorInfo*)new BME280_Temp());
}

extern	void
start_sensors(void)
{
	sense_buff->rewind_write();
	Sensors::start();
}
