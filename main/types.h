#ifndef	_INC_TYPES_H
#define	_INC_TYPES_H

#include	<sys/types.h>
#include	<stdint.h>
#include	<time.h>

#ifndef	byte
#define	byte		uint8_t
#endif

#ifndef	word
#define	word		uint32_t
#endif

#ifndef	dword
#define	dword		unsigned long
#endif

#ifndef	ulong
#define	ulong		unsigned long
#endif

#ifndef	Bool
#define	Bool		signed char
#endif

#ifndef	FALSE
#define	FALSE		0
#endif

#ifndef	TRUE
#define	TRUE		(!FALSE)
#endif

#define	IntToBool(v)	((v)?TRUE:FALSE)
#define	PRINT_BOOL(b)	((b) ? "T" : "F")

#define	TO_INT(x)	((x) - '0')
#define	TO_CHAR(x)	((x) + '0')

#define	ALIGN_BYTES		sizeof(int)

#define	ROUND_ALIGN(p)	\
	((((p)%ALIGN_BYTES) == 0) ? (p) : (((p)/ALIGN_BYTES)+1)*ALIGN_BYTES)

#ifndef	ON
#define	ON		TRUE
#endif
#ifndef	OFF
#define	OFF		FALSE
#endif


typedef	struct	tm		tTime;

#endif

