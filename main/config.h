#include "config.default.h"

#include	"hardware.h"

/* SoftAP */
#define	DEFAULT_SSID			"JAR-GARDEN"
#define	DEFAULT_PASS			"jar-garden"
#define	DEFAULT_AP_SSID			"LANLANLAN24"
#define	DEFAULT_AP_PASS			"8309f815520d2"

#define	MAX_STA_CONN	2

#define	CONSOLE_HOST			"https://10.1.254.11:7002"
#define	NR_RGBW_LED				0
#define	NR_NEOPIXEL_LED			25
#define	NR_LED					(NR_RGBW_LED + NR_NEOPIXEL_LED)
#define	NR_SENSORS				10

#define	MAX_TIMER_EVENTS		100

#define	DEFAULT_SENSE_INTERVAL	3600

#define	N_TIMER_EVENTS			10

#define	USE_LOCALCA

#define	HAVE_SENSORS

#ifdef	HAVE_SENSORS
#define	HAVE_FAN
#endif
