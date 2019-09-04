#ifndef	__INC__GLOBALS_H__
#define	__INC__GLOBALS_H__

#ifdef	MAIN
#define	GLOBAL	/*	*/
#else
#define	GLOBAL	extern
#endif

GLOBAL	int		nr_rgbw_led;
GLOBAL	int		nr_neopixel_led;

#endif
