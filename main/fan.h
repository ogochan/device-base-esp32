#ifndef	__INC_FAN_H__
#define	__INC_FAN_H__

#include	"SensorInfo.h"
#include	"SenseBuffer.h"

class FAN:public SensorInfo
{
  public:
	virtual	void 	initialize();
	virtual	void	get(SenseBuffer *buff);
	virtual	size_t	build(char *p, SenseBuffer *buff);
	virtual	void	stop(int sec) {};
	virtual	void	start(void) {};
	virtual	const	char	*name(void);
	virtual	const	char	*unit(void) { return NULL; };
	virtual	const	char	*data_class_name(void);
	virtual	int	dimension(void) { return 1; };

	static	const	uint8_t	precision;
	void	speed(int speed);
	int		speed();
	void	set_switch(Bool on);

	esp_err_t	err_code;

	FAN();
   	~FAN();

  protected:
	int		_speed;
};

#endif
