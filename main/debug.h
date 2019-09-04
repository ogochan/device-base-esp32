/*
 * libmondai -- MONTSUQI data access library
 * Copyright (C) 1989-2017 Ogochan.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
*	debug macros
*/

#ifndef	__INC_DEBUG_H
#define	__INC_DEBUG_H

#include	<stdio.h>

#ifdef	DEBUG
#define	STOP				while(true) {}
#define	PAUSE				delay(10*1000)
#else
#define	STOP				/* */
#define	PAUSE				/* */
#endif

#ifdef	TRACE
#define	DEBUG_PRINTS(s)		printf("%s", (s))
#define	DEBUG_PRINTI(s)		printf("%d", (s))
#define	DEBUG_PRINTLN(s)	printf("%s\n", (s))
#else
#define	DEBUG_PRINTS(...)
#define	DEBUG_PRINTI(...)
#define	DEBUG_PRINTLN(...)
#endif

#define	dbgmsg(s)			\
do {						\
	DEBUG_PRINTS("M:");		\
	DEBUG_PRINTS(__FILE__);	\
	DEBUG_PRINTS(":");		\
	DEBUG_PRINTI(__LINE__);	\
	DEBUG_PRINTS(":");		\
	DEBUG_PRINTLN(s);		\
}	while(0)

#define	ENTER_FUNC			\
do {						\
	DEBUG_PRINTS("M:");		\
	DEBUG_PRINTS(__FILE__);	\
	DEBUG_PRINTS(":");		\
	DEBUG_PRINTI(__LINE__);	\
	DEBUG_PRINTS(":>");		\
	DEBUG_PRINTLN(__func__);	\
}	while(0)
#define	LEAVE_FUNC			\
do {						\
	DEBUG_PRINTS("M:");		\
	DEBUG_PRINTS(__FILE__);	\
	DEBUG_PRINTS(":");		\
	DEBUG_PRINTI(__LINE__);	\
	DEBUG_PRINTS(":<");		\
	DEBUG_PRINTLN(__func__);	\
}	while(0)
#define	dbgprintf(fmt,...)	\
{							\
	char	_buff[80];		\
	sprintf(_buff, fmt,  __VA_ARGS__);			\
	dbgmsg(_buff);			\
}	while(0);

#endif
