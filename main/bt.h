#ifndef	__INC_BT_H__
#define	__INC_BT_H__

#include	"types.h"

#define		BT_GET				1
#define		BT_POST				2
	
typedef	struct	{
	int		method;
	char	*path;
	int		(*func)(char *path, char *body, Bool debug);
}	BT_HandlerTable_s;

extern	void	initialize_nvs(void);
extern	void	initialize_bt(const char *deviceName);
extern	void	bt_send(uint8_t *data, size_t len);
extern	void	bt_send_string(char *str);
extern	int		bt_getc(void);
extern	size_t	bt_recv(uint8_t *data, size_t len);
extern	size_t	bt_recv_str(char *data, int delim, size_t len);
extern	void	bt_set_handler(int method, char *path, int (*func)(char *, char *, Bool));
extern	void	start_bt(size_t sStack);

#endif
