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

#include	<esp_log.h>

#ifdef	DEBUG
#define	STOP				while(true) {}
#define	PAUSE				delay(10*1000)
#else
#define	STOP				/* */
#define	PAUSE				/* */
#endif

#ifdef	TRACE
#define	ENTER_FUNC			ESP_LOGI("", "%s:%d:>%s", __FILE__, __LINE__, __func__);
#define	LEAVE_FUNC			ESP_LOGI("", "%s:%d:<%s", __FILE__, __LINE__, __func__);
#define	dbgmsg(s)			ESP_LOGI("", "%s:%d:%s", __FILE__, __LINE__, (s))
#define	dbgprintf(fmt,...)	ESP_LOGI("", "%s:%d:" fmt, __FILE__, __LINE__, __VA_ARGS__)
#else
#define	ENTER_FUNC
#define	LEAVE_FUNC
#define	dbgmsg(s)
#define	dbgprintf(fmt,...)
#endif

#endif
