#define	TRACE

#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	"driver/ledc.h"
#include	"esp_err.h"

#include 	"config.h"
#include	"types.h"
#include 	"debug.h"
#include	"light.h"
#include	"neopixel.h"
#include	"globals.h"
#include	"misc.h"

#define	NEOPIXEL_NEW_DRIVER

static	Bool				rgbw_need_update;
static	pixel_settings_t	Px;

extern	void
initialize_light(void)
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 1000,
        .speed_mode = RGBW_LED_TIMER_MODE,
        .timer_num = RGBW_LED_TIMER
    };
    ledc_channel_config_t ledc_channel[] = {
        {
            .channel    = RGBW_LED_CH_R,
            .duty       = 0,
            .gpio_num   = RGBW_LED_R_PORT,
            .speed_mode = RGBW_LED_TIMER_MODE,
            .hpoint     = 0,
            .timer_sel  = RGBW_LED_TIMER
        },{
            .channel    = RGBW_LED_CH_G,
            .duty       = 0,
            .gpio_num   = RGBW_LED_G_PORT,
            .speed_mode = RGBW_LED_TIMER_MODE,
            .hpoint     = 0,
            .timer_sel  = RGBW_LED_TIMER
        },{
            .channel    = RGBW_LED_CH_B,
            .duty       = 0,
            .gpio_num   = RGBW_LED_B_PORT,
            .speed_mode = RGBW_LED_TIMER_MODE,
            .hpoint     = 0,
            .timer_sel  = RGBW_LED_TIMER
        },{
            .channel    = RGBW_LED_CH_W,
            .duty       = 0,
            .gpio_num   = RGBW_LED_W_PORT,
            .speed_mode = RGBW_LED_TIMER_MODE,
            .hpoint     = 0,
            .timer_sel  = RGBW_LED_TIMER
        },
	};

#if	(NR_RGBW_LED > 0)
	ledc_timer_config(&ledc_timer);
	for (int ch = 0; ch < 4; ch++) {
		ledc_channel_config(&ledc_channel[ch]);
	}
#endif
	rgbw_need_update = FALSE;
#if	(NR_NEOPIXEL_LED > 0)
#if	0
	gpio_set_direction(NEOPIXEL_LED_PORT, GPIO_MODE_OUTPUT);
	gpio_set_level(NEOPIXEL_LED_PORT, 0);
	msleep(2);
#endif

#ifndef	NEOPIXEL_NEW_DRIVER
	neopixel_init(NEOPIXEL_LED_PORT, NEOPIXEL_RMT_CHANNEL);
#endif
	memset(&Px, 0, sizeof(Px));
	Px.pixels = (uint8_t *)malloc(sizeof(uint32_t) * NR_NEOPIXEL_LED);
	memclear(Px.pixels,sizeof(uint32_t) * NR_NEOPIXEL_LED);
	Px.pixel_count = NR_NEOPIXEL_LED;

	memset(&Px.timings, 0, sizeof(Px.timings));
	Px.timings.mark.level0 = 1;
	Px.timings.space.level0 = 1;
	Px.timings.mark.duration0 = 12;
#ifdef	NEOPIXEL_WS2812
	strcpy(px.color_order, "GRB");
	Px.nbits = 24;
	Px.timings.mark.duration1 = 14;
	Px.timings.space.duration0 = 7;
	Px.timings.space.duration1 = 16;
	Px.timings.reset.duration0 = 600;
	Px.timings.reset.duration1 = 600;
#endif
#ifdef	NEOPIXEL_SK6812
	strcpy(Px.color_order, "GRBW");
	Px.nbits = 32;
	Px.timings.mark.duration1 = 12;
	Px.timings.space.duration0 = 6;
	Px.timings.space.duration1 = 18;
	Px.timings.reset.duration0 = 900;
	Px.timings.reset.duration1 = 900;
#endif
	Px.brightness = 0x80;
#ifdef	NEOPIXEL_NEW_DRIVER
	Px.pin = NEOPIXEL_LED_PORT;
	Px.rmtChannel = NEOPIXEL_RMT_CHANNEL;
	neopixel_init(&Px);
#endif
	msleep(10);
	np_clear(&Px);
#ifdef	NEOPIXEL_NEW_DRIVER
	np_show(&Px);
#else
	np_show(&Px, NEOPIXEL_RMT_CHANNEL);
#endif
#endif
}

extern	void
light_switch(
	Bool	on)
{
}

extern	void
light_set_color(
	int		no,
	int		r,
	int		g,
	int		b,
	int		w)
{
#if	(NR_RGBW_LED > 0)
	ledc_set_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_R, r);
	ledc_set_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_G, g);
	ledc_set_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_B, b);
	ledc_set_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_W, w);
	rgbw_need_update = TRUE;
	no -= NR_RGBW_LED;
#endif
#if	(NR_NEOPIXEL_LED > 0 )
	np_set_pixel_rgbw(&Px, no, r, g, b, w);
#endif
}

extern	void
light_update(void)
{
#if	(NR_RGBW_LED > 0)
	if	( rgbw_need_update )	{
		ledc_update_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_R);
		ledc_update_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_G);
		ledc_update_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_B);
		ledc_update_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_W);
		rgbw_need_update = FALSE;
	}
#endif
#if	(NR_NEOPIXEL_LED > 0 )
#ifdef	NEOPIXEL_NEW_DRIVER
	np_show(&Px);
#else
	np_show(&Px, NEOPIXEL_RMT_CHANNEL);
#endif
#endif
}
			 
extern	void
light_set_all_color(
	int		r,
	int		g,
	int		b,
	int		w)
{
	int		i;

	for ( i = 0 ; i < ( NR_RGBW_LED + NR_NEOPIXEL_LED ) ; i ++ )	{
		light_set_color(i, r, g, b, w);
	}
	light_update();
	msleep(200);
}
