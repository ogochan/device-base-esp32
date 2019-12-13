#define	TRACE

extern "C" {
#include 	"esp_log.h"
}

#include	"config.h"
#include	"debug.h"
#include	"SensorInfo.h"
#include	"SenseBuffer.h"
#include	"Sensors.h"

#define	TAG	"SenseBuffer"

/*
uint8_t	SenseBuffer::_buff[SIZE_OF_SENSOR_BUFF];
uint8_t	*SenseBuffer::_w_pointer;
uint8_t	*SenseBuffer::_r_pointer;
*/
void
SenseBuffer::check_space(void)
{
	SensorInfo	*info;
	size_t		size
		,	shift;
	int		i;

	size = 0;
	for( i = 0; i< Sensors::count(); i++ ){
		info = Sensors::item(i);
		if	( info->fValid )	{
			size += info->dimension() * sizeof(float) + 1;
		}
	}
	size ++;
	ESP_LOGI(TAG, "use %d bytes", size);
	while	( ( SIZE_OF_SENSOR_BUFF - ( _w_pointer - _buff ) )  < size )	{
		_r_pointer = _buff + sizeof(time_t);
		while	( *_r_pointer != 0xFF )	{
			info = Sensors::item((int)*_r_pointer);
			_r_pointer ++;
			_r_pointer += info->dimension() * sizeof(float);
		}
		_r_pointer ++;
		shift = _r_pointer - _buff;
		ESP_LOGI(TAG, "shift %d bytes", shift);
		memmove(_r_pointer, _buff, shift);
		_w_pointer -= shift;
	}
}

