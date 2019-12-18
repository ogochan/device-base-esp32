#ifndef	__INC_JAR_GARDEN_H__
#define	__INC_JAR_GARDEN_H__

#include	"SensorInfo.h"
#include	"SenseBuffer.h"

class Jar_garden:public SensorInfo
{
  public:
	virtual	void 	initialize() {};
	virtual	void	get(SenseBuffer *buff);
	virtual	size_t	build(char *p, SenseBuffer *buff);
	virtual	void	stop(int sec) {};
	virtual	void	start(void) {};
	virtual	const	char	*name(void);
	virtual	const	char	*unit(void) { return NULL; };
	virtual	const	char	*data_class_name(void);
	virtual	int	dimension(void) { return 4; };

	static	const	uint8_t	precision;

	esp_err_t	err_code;

	Jar_garden();
   	~Jar_garden();
};

#endif
