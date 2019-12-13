//#define	TRACE

#include 	"config.h"
#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<time.h>
#include	<sys/time.h>

#include 	<freertos/FreeRTOS.h>
#include 	<freertos/task.h>
#include	<freertos/event_groups.h>
#include 	<esp_system.h>
#include	<esp_event.h>
#include 	<esp_log.h>
#include 	<esp_attr.h>
#include 	<esp_sleep.h>
#include 	<nvs_flash.h>
#include	<esp_sntp.h>

#include	"types.h"
#include	"sntp.h"
#include	"globals.h"
#include	"misc.h"
#include 	"debug.h"

#define	SYNC_SLEEP		5
#define	RETRY_COUNT		20
#define	MIN_NTP_SLEEP	64
#define	MAX_NTP_SLEEP	1024

static	Bool	time_valid;

static	void
time_sync_notification_cb(struct timeval *tv)
{
	time_valid = TRUE;
}

static	void
sync_time(void)
{
	int		sleep_second;
	time_t	after
		,	before;
	struct	tm	timeinfo = { 0 };
	char	buf[50];
	int		retry;

	sleep_second = MIN_NTP_SLEEP;
	while	(TRUE)	{
		sntp_init();
		time(&before);
		localtime_r(&before, &timeinfo);
		//ESP_LOGI(TAG, "before time %s", asctime_r(&timeinfo, buf));
		
		retry = 0;
		while	( ( sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET ) && ( ++retry < RETRY_COUNT ) ) {
			//ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, RETRY_COUNT);
			msleep(SYNC_SLEEP * 1000);
		}
		time(&after);
		localtime_r(&after, &timeinfo);
		dbgprintf("after time %s", asctime_r(&timeinfo, buf));
		sntp_stop();
		if	( retry < RETRY_COUNT )	{
			if	( (int)after == (int)before + SYNC_SLEEP * retry )	{
				if	( sleep_second < MAX_NTP_SLEEP )	{
					sleep_second *= 2;
				}
			} else {
				if	( sleep_second > MIN_NTP_SLEEP )	{
					sleep_second /= 2;
				}
			}
		}
		dbgprintf("NTP sleep %d seconds", sleep_second);
		msleep(sleep_second * 1000);
	}
}

extern	Bool
sntp_valid(void)
{
	return	(time_valid);
}

extern	void
initialize_sntp(void)
{
    dbgmsg("Initializing SNTP");

	time_valid = FALSE;
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp.nict.jp");
    sntp_setservername(1, "time.nist.gov");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    xTaskCreate(&sync_time, "ntp_task", 2048, NULL, 5, NULL);

}
