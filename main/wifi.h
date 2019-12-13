#ifndef	__INC_WIFI_H__
#define	__INC_WIFI_H__

#include	"types.h"

extern	esp_err_t	initialize_wifi(void);
extern	void		wifi_ap_start(char *ssid, char *pass);
extern	void		wifi_scan_start(void);
extern	void		wifi_scan_get(char *p);
extern	void		wifi_scan_stop(void);
extern	void		wifi_stop(void);
extern	Bool		wifi_connect(char *ssid, char *password);
extern	Bool		wifi_is_valid(void);

#endif
