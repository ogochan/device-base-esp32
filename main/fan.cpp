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

void
FAN::initialize()
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
	fValid = false;
}

FAN::FAN()
{
	initialize();
}

void
FAN::get(
	SenseBuffer *buff)
{
	buff->put(&_speed, sizeof(int));
}

size_t
FAN::build(
	char	*p,
	SenseBuffer	*buff)
{
	int		speed;
ENTER_FUNC;
	buff->get(&speed, sizeof(int));
LEAVE_FUNC;
	return	sprintf(p, "\"value\":%d", speed);
}

const	char	*
FAN::name(void)
{
	return	"Fan";
}

const	char	*
FAN::data_class_name(void)
{
	return	"FanSpeed";
}

void
FAN::speed(
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
	this->_speed = speed;
}

int
FAN::speed()
{
	return	(this->_speed);
}

void
FAN::set_switch(
	Bool	on)
{
	if	( on )	{
		speed(255);
	} else {
		speed(0);
	}
}
