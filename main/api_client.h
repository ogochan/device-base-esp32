#ifndef	__INC_API_CLIENT_H__
#define	__INC_API_CLIENT_H__

#include	<esp_http_client.h>
#include	"types.h"

extern	void	initialize_api(void);
extern	Bool	api_device_new(esp_http_client_handle_t client, char *user, char *pass, char *device_id);
extern	Bool	api_session_new(esp_http_client_handle_t client, char *user, char *pass, char *device_id, char *session_key);
extern	Bool	api_post_data(esp_http_client_handle_t client, char *path, char *device_id, char *session_key, uint8_t *data, size_t size);
extern	int		api_get_data(esp_http_client_handle_t client, char *path, char *device_id, char *session_key, uint8_t *data, size_t size);
extern	void	api_get_device_info(void);
extern	void	api_get_settings(void);
extern	void	api_get_schedule(void);
extern	void	api_exec_ota(void);

#endif
