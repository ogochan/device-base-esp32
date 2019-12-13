#define	TRACE

extern "C" {
#include 	"esp_log.h"
}

#include	"config.h"
#include	"debug.h"
#include	"utility.h"

#include	"SensorInfo.h"
#include	"Sensors.h"

#define	TAG	"Sensors"

uint8_t		Sensors::nSensors;
SensorInfo	*Sensors::_Sensors[NR_SENSORS];

void
Sensors::init(void)
{
	nSensors = 0;
}

void
Sensors::stop(
	int		sec)
{
	int		i;

ENTER_FUNC;
	for	( i = 0 ; i < nSensors ; i ++ )	{
		if		( Sensors::item(i)->fValid )	{
			Sensors::item(i)->stop(sec);
		}
	}
LEAVE_FUNC;
}

void
Sensors::start(void)
{
	int		i;

ENTER_FUNC;

	for	( i = 0 ; i < nSensors ; i ++ )	{
		dbgprintf("%s", Sensors::item(i)->name());
		if		( Sensors::item(i)->fValid )	{
			Sensors::item(i)->start();
		}
	}
LEAVE_FUNC;
}

void
Sensors::collect(
	time_t	n,
	SenseBuffer	*buff,
	SensorInfo	*info)
{
ENTER_FUNC;
	buff->check_space();
	buff->put(&n, sizeof(time_t));
	if	( info != NULL )	{
		dbgprintf("info->id = %d", (int)info->id);
		buff->set_value(info->id);
		info->get(buff);
	}
	buff->set_value(0xFF);
LEAVE_FUNC;
}

void
Sensors::collect(
	time_t	n,
	SenseBuffer	*buff)
{
	SensorInfo	*info;
	uint8_t	i;

ENTER_FUNC;
	buff->check_space();
	buff->put(&n, sizeof(time_t));
	for( i = 0; i< Sensors::count(); i++ ){
		info = Sensors::item(i);
		dbgprintf("info(%d)->id = %d", i, (int)info->id);
		if	( info->fValid )	{
			buff->set_value(info->id);
			info->get(buff);
		}
	}
	buff->set_value(0xFF);
LEAVE_FUNC;
}

