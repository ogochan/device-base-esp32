#ifndef	__INC_LIGHT_H__
#define	__INC_LIGHT_H__

extern	void	initialize_light(void);
extern	void	light_set_color(int no, int r, int g, int b, int w);
extern	void	light_switch(Bool on);
extern	void	light_update(void);
extern	void	light_set_all_color(int r, int g, int b, int w);

#endif
