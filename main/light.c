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

#define	RGBW_LED_R_PORT		21
#define	RGBW_LED_G_PORT		22
#define	RGBW_LED_B_PORT		23
#define	RGBW_LED_W_PORT		4
#define	NEOPIXEL_LED_PORT	18

#define	RGBW_LED_TIMER_MODE		LEDC_HIGH_SPEED_MODE
#define	RGBW_LED_TIMER			LEDC_TIMER_0
#define	RGBW_LED_CH_R	0
#define	RGBW_LED_CH_G	1
#define	RGBW_LED_CH_B	2
#define	RGBW_LED_CH_W	3

// NeoPixel configure
#define	NEOPIXEL_SK6812
#define	NEOPIXEL_RMT_CHANNEL		RMT_CHANNEL_1

static	Bool				rgbw_need_update;
static	pixel_settings_t	Px;

extern	void
light_init(void)
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 1000,
        .speed_mode = RGBW_LED_TIMER_MODE,
        .timer_num = RGBW_LED_TIMER
    };
    ledc_channel_config_t ledc_channel[] = {
        {
            .channel    = LEDC_CHANNEL_0,
            .duty       = 0,
            .gpio_num   = RGBW_LED_CH_R,
            .speed_mode = RGBW_LED_TIMER_MODE,
            .hpoint     = 0,
            .timer_sel  = RGBW_LED_TIMER
        },{
            .channel    = LEDC_CHANNEL_1,
            .duty       = 0,
            .gpio_num   = RGBW_LED_CH_G,
            .speed_mode = RGBW_LED_TIMER_MODE,
            .hpoint     = 0,
            .timer_sel  = RGBW_LED_TIMER
        },{
            .channel    = LEDC_CHANNEL_2,
            .duty       = 0,
            .gpio_num   = RGBW_LED_CH_B,
            .speed_mode = RGBW_LED_TIMER_MODE,
            .hpoint     = 0,
            .timer_sel  = RGBW_LED_TIMER
        },{
            .channel    = LEDC_CHANNEL_3,
            .duty       = 0,
            .gpio_num   = RGBW_LED_CH_W,
            .speed_mode = RGBW_LED_TIMER_MODE,
            .hpoint     = 0,
            .timer_sel  = RGBW_LED_TIMER
        },
	};

	if		(  nr_rgbw_led > 0 )	{
		ledc_timer_config(&ledc_timer);
		for (int ch = 0; ch < 4; ch++) {
			ledc_channel_config(&ledc_channel[ch]);
		}
	}
	rgbw_need_update = FALSE;
	if		( nr_neopixel_led > 0 )	{
		neopixel_init(NEOPIXEL_LED_PORT, NEOPIXEL_RMT_CHANNEL);

		Px.pixels = (uint8_t *)malloc(sizeof(uint32_t) * nr_neopixel_led);
		memclear(Px.pixels,sizeof(uint32_t) * nr_neopixel_led);
		Px.pixel_count = nr_neopixel_led;

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
		np_show(&Px, NEOPIXEL_RMT_CHANNEL);
	}
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
	if		( nr_rgbw_led > 0 )	{
		if	( no < nr_rgbw_led )	{
			ledc_set_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_R, r);
			ledc_set_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_G, g);
			ledc_set_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_B, b);
			ledc_set_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_W, w);
			rgbw_need_update = TRUE;
		}
		no -= nr_rgbw_led;
	}
	if		( nr_neopixel_led > 0 )	{
		np_set_pixel_rgbw(&Px, no, r, g, b, w);
	}
}

extern	void
light_update(void)
{
	if ( nr_rgbw_led > 0 )	{
		if	( rgbw_need_update )	{
			ledc_update_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_R);
			ledc_update_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_G);
			ledc_update_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_B);
			ledc_update_duty(RGBW_LED_TIMER_MODE, RGBW_LED_CH_W);
			rgbw_need_update = FALSE;
		}
	}
	if	( nr_neopixel_led > 0 )	{
		np_show(&Px, NEOPIXEL_RMT_CHANNEL);
	}
}
			 
