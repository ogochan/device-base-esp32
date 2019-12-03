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
	static	int	count(void)	{
		return	(nSensors);
	};
	static	SensorInfo	*item(uint8_t i)	{
		return	(_Sensors[i]);
	};
	static	void	add(SensorInfo *sensor)	{
		_Sensors[nSensors ++] = sensor;
	};

  protected:
	static	uint8_t		nSensors;
	static	SensorInfo	*_Sensors[NUMBER_OF_SENSORS];
};

#endif
