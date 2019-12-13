#ifndef __BME280_BAROMETRIC_H__
#define __BME280_BAROMETRIC_H__

#include	"I2C.h"
#include	"SenseBuffer.h"

#include	"BME280.h"

class BME280_Barometric:public BME280
{
  public:
	virtual	void	get(SenseBuffer *buff);
	virtual	size_t	build(char *p, SenseBuffer *buff);
	virtual	void	stop(int sec) {};
	virtual	void	start(void) {};
	virtual	const	char	*name(void);
	virtual	const	char	*unit(void);
	virtual	const	char	*data_class_name(void);

	static	const	uint8_t	precision = 0;
	esp_err_t	err_code;

};

#endif // __BME280_BAROMETRIC_H__
