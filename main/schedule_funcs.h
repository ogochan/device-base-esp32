#ifndef	__INC_SCHEDULE_FUNCS_H__
#define	__INC_SCHEDULE_FUNCS_H__

typedef	Bool (*CHECK_FUNC)(tTime *now);

extern	void	init_schedule_func(void);
extern	void	register_schedule_func(CHECK_FUNC func);
extern	Bool	check_schedule_funcs(tTime *now);

#endif
