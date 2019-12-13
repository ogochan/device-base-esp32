#ifndef	_INC_SENSOR_INFO_H_
#define	_INC_SENSOR_INFO_H_

#include	<stdlib.h>
#include	<stdint.h>
#include	<string.h>

extern "C" {
#include 	"esp_err.h"
}

#include	"SenseBuffer.h"

typedef	uint8_t		byte;

class SensorInfo
{
  public:
	virtual void	initialize(void){};
	virtual	void	get(SenseBuffer *buff) {};
	virtual	size_t	build(char *p, SenseBuffer *buff) { return 0; };
	virtual	void	stop(int sec) {};
	virtual	void	start(void) {};
	virtual	const	char	*name(void) { return NULL; };
	virtual	const	char	*unit(void) { return NULL; };
	virtual	const	char	*data_class_name(void) { return NULL; };
	virtual	int	dimension(void) { return 1; };
	bool	fValid;
	virtual	int	get_ledno(void) { return 0; };

	static	const	uint8_t	precision;
	esp_err_t	err_code;
	uint8_t		id;

	SensorInfo();
	~SensorInfo();
};

#endif
