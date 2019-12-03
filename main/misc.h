#ifndef	__INC_MISC_H__
#define	__INC_MISC_H__

#include	<string.h>

extern	void	msleep(int ms);
extern	int		hex2int(char *hex, size_t len);
extern	int		str2int(char *str, size_t len);

#define	strlcmp(p,q)			strncmp((p),(q),strlen(q))
#define	memclear(buff,size)		memset((buff),0,(size))
#define	ERROR_CHECK(check)		if ((err = check) != ESP_OK) goto err_exit;
#define	ERROR(msg)		{		\
 dbgprintf("%s", (msg));		\
 goto error;					\
}


#endif
