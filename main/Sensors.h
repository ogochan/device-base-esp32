#ifndef	__INC_SENSORS_H__
#define	__INC_SENSORS_H__

#include	"SensorInfo.h"

class	Sensors
{
  public:
	static	void	init(void);
	static	void	start(void);
	static	void	stop(int sec);
	static	void	collect(time_t n, SenseBuffer *buff);
	static	void	collect(time_t n, SenseBuffer *buff, SensorInfo *info);
	static	int		count(void)	{
		return	(nSensors);
	};
	static	SensorInfo	*item(uint8_t i)	{
		return	(_Sensors[i]);
	};
	static	SensorInfo	*add(SensorInfo *sensor)	{
		sensor->id = nSensors;
		_Sensors[nSensors ++] = sensor;
		ESP_LOGI("","sensor id = %d", (int)sensor->id);
		return	(sensor);
	};

  protected:
	static	uint8_t		nSensors;
	static	SensorInfo	*_Sensors[NR_SENSORS];
};

#endif
