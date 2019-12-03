#define	TRACE
//#define	PWM

#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#ifdef	PWM
#include	"driver/ledc.h"
#else
#include	"driver/gpio.h"
#endif
#include	"esp_err.h"

#include 	"config.h"
#include	"types.h"
#include 	"debug.h"
#include	"fan.h"
#include	"globals.h"
#include	"misc.h"


extern	void
initialize_fan(void)
{
#ifdef	PWM
    ledc_timer_config_t fan_timer = {
        .speed_mode = FAN_TIMER_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = FAN_TIMER,
        .freq_hz = 1000
    };
    ledc_channel_config_t fan_channel = {
		.gpio_num   = FAN_PORT,
		.speed_mode = FAN_TIMER_MODE,
		.channel    = FAN_CHANNEL,
		.timer_sel  = FAN_TIMER,
		.duty       = 0,
		.hpoint     = 0,
	};

	ledc_timer_config(&fan_timer);
	ledc_channel_config(&fan_channel);
#else
	gpio_set_direction(FAN_PORT, GPIO_MODE_OUTPUT);
#endif
}

extern	void
fan_set_speed(
	int		speed)
{
#ifdef	PWM
	ledc_set_duty(FAN_TIMER_MODE, FAN_CHANNEL, speed);
	ledc_update_duty(FAN_TIMER_MODE, FAN_CHANNEL);
#else
	if	( speed > 0 )	{
		gpio_set_level(FAN_PORT, 1);
	} else {
		gpio_set_level(FAN_PORT, 0);
	}
#endif
}

extern	void
fan_switch(
	Bool	on)
{
#ifdef	PWM
	if	( on )	{
		ledc_set_duty(FAN_TIMER_MODE, FAN_CHANNEL, 255);
	} else {
		ledc_set_duty(FAN_TIMER_MODE, FAN_CHANNEL, 0);
	}
	ledc_update_duty(FAN_TIMER_MODE, FAN_CHANNEL);
#else
	if	( on )	{
		gpio_set_level(FAN_PORT, 1);
	} else {
		gpio_set_level(FAN_PORT, 0);
	}
#endif
}
