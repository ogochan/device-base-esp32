#ifndef	__INC_WIFI_H__
#define	__INC_WIFI_H__

#include	"types.h"

extern	void	initialize_wifi(void);
extern	void	scan_wifi_start(void);
extern	void	scan_wifi_get(char *p);
extern	void	scan_wifi_stop(void);
extern	void	stop_wifi(void);
extern	void	connect_wifi(char *ssid, char *password);

#endif
