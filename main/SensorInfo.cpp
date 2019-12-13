#define	TRACE
#include	"config.h"
#include	"debug.h"
#include	"SensorInfo.h"

const	uint8_t	SensorInfo::precision = 0;

SensorInfo::SensorInfo()
{
	fValid = false;
}

SensorInfo::~SensorInfo()
{
}
