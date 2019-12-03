#ifndef	__INC_SCHEDULE_H__
#define	__INC_SCHEDULE_H__

typedef	struct	{
	int8_t	no;
	uint8_t	red;
	uint8_t	green;
	uint8_t	blue;
	uint8_t	white;
}	LED;
typedef	struct	{
	int		at;
	LED		led;
}	tScheduleEvent;

typedef	struct	{
	uint8_t		wday;
	uint8_t		nev;
	tScheduleEvent	*ev;
}	tSchedule;

extern	void	reset_schedule(void);
extern	void	push_schedule(uint8_t wday, int nev, tScheduleEvent *ev);
extern	void	initialize_schedule(void);
extern	void	start_schedule(void);

#endif
