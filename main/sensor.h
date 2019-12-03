#ifndef	__INC_SENSOR_H__
#define	__INC_SENSOR_H__

#include	"types.h"

extern	void	initialize_sensors(void);
extern	void	start_sensors(void);
extern	Bool	sensor_send_server(void);
extern	Bool	sensor_collect(void);

#endif
