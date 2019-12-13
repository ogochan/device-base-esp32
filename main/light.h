#ifndef	__INC_LIGHT_H__
#define	__INC_LIGHT_H__
extern	"C"	{
//#include	"neopixel.h"
}
#include	"SensorInfo.h"
#include	"SenseBuffer.h"

class Light:public SensorInfo
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
	virtual	int	dimension(void) { return 4; };

	static	const	uint8_t	precision;
	void	color(int no, int r, int g, int b, int w);
	void	update();
	void	color(int r, int g, int b, int w);

	esp_err_t	err_code;

	Light();
   	~Light();

  protected:
#if	(NR_RGBW_LED > 0)
	Bool				rgbw_need_update;
	uint8_t				rgbw_RGBW[4];
#endif
#if	(NR_NEOPIXEL_LED > 0)
	pixel_settings_t	Px;
#endif

};

#endif
