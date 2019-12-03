#define	TRACE

#include 	"config.h"
#include 	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<unistd.h>	//	for usleep

#include 	"esp_system.h"
#include 	"esp_log.h"
#include	"time.h"
#include	"sys/time.h"
#include	"globals.h"

#include 	"debug.h"
#include	"misc.h"

extern	void
msleep(
	int		ms)
{
	usleep(ms*1000);
}

extern	int
hex2int(
	char	*hex,
	size_t	len)
{
	int		val;
	uint8_t	c;

	val = 0;
	while	( len >  0 )	{
		if		(	( *hex >= '0' )
				&&	( *hex <= '9' ) )	{
			c = *hex - '0';
		} else
		if		(	( *hex >= 'a' )
				&&	( *hex <= 'f' ) )	{
			c = *hex - 'a' + 10;
		} else
		if		(	( *hex >= 'A' )
				&&	( *hex <= 'F' ) )	{
			c = *hex - 'A' + 10;
		} else {
			c = 0;
		}
		val = ( val << 4 ) | c;
		hex ++;
		len --;
	}
	return	(val);
}
extern	int
str2int(
	char	*str,
	size_t	len)
{
	int		val;
	uint8_t	c;

	val = 0;
	while	( len >  0 )	{
		if		(	( *str >= '0' )
				&&	( *str <= '9' ) )	{
			c = (*str) - '0';
		} else {
			c = 0;
		}
		val = val * 10 + c;
		str ++;
		len --;
	}
	return	(val);
}
