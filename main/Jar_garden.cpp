//#define	TRACE

extern "C" {
#include 	"esp_log.h"
}

#include	"config.h"
#include	"debug.h"
#include	"utility.h"

#include	"SenseBuffer.h"

#include	"Jar_garden.h"

Jar_garden::Jar_garden()
{
}

void
Jar_garden::get(
	SenseBuffer *buff)
{
ENTER_FUNC;
LEAVE_FUNC;
}

size_t
Jar_garden::build(
	char	*p,
	SenseBuffer	*buff)
{
	float	data;

ENTER_FUNC;
LEAVE_FUNC;
	return	sprintf(p, "\"value\":\"reboot by OTA\"");
}

const	char	*
Jar_garden::name(void)
{
	return	"Jar_garden";
}

const	char	*
Jar_garden::data_class_name(void)
{
	return	"MachineStatus";
}

