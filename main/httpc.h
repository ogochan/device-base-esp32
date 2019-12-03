#ifndef	__INC_HTTPC_H__
#define	__INC_HTTPC_H__

#include	"esp_http_client.h"
#include	"types.h"

extern	esp_http_client_handle_t	initialize_httpc(void);
extern	void		finish_httpc(esp_http_client_handle_t client);
extern	esp_err_t	httpc_set_header(esp_http_client_handle_t client, char *key, char *value);
extern	esp_err_t	httpc_post(esp_http_client_handle_t client, char *host, char *path, uint8_t *data, size_t len);
extern	esp_err_t	httpc_post_open(esp_http_client_handle_t client, char *host, char *path, size_t size);
extern	esp_err_t	httpc_close(esp_http_client_handle_t client);
extern	esp_err_t	httpc_get(esp_http_client_handle_t client, char *host, char *path);
extern	int			httpc_get_data(esp_http_client_handle_t client, uint8_t *buff, size_t len);
extern	int			httpc_get_status(esp_http_client_handle_t client);
extern	void		httpc_ota(char *host, char *path, char *session_key);

#endif
