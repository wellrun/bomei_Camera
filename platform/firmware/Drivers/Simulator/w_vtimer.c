/**
 * @file vtimer.c
 * @brief Virtual timer function header file
 * This file provides virtual timer APIs: initialization, start timer, stop timer and
 * timer interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ZouMai
 * @date 2004-09-22
 * @version 1.0
 * @ref AK3223 technical manual.
 */

#ifdef OS_WIN32

#include "winvme.h"
#include "hal_timer.h"
#include "Eng_Debug.h"
#include <time.h>

T_U32 watch_dog;

#define TIMER_NUM_MAX				18    /* 0~3 as second timer, 4~11 as millisecond timer.*/

#define PHYSICAL_TIMER				10	  /* interrupt every 10 millisecond */

#define TIMER_INACTIVE				0		/* inactive(available) */
#define TIMER_ACTIVE				1		/* active */
#define TIMER_TIMEOUT				2		/* time out */

typedef struct
{
	T_U8	state;				/* TIMER_INACTIVE or TIMER_ACTIVE or TIMER_TIMEOUT */
	T_BOOL	loop;				/* loop or not */
	T_U32	total_delay;		/* total delay, microseconds */
	T_U32	cur_delay;			/* current delay, microseconds */
	T_fVTIMER_CALLBACK	callback_func;	/* callback function only for current timer */
} T_TIMER_DATA;


static T_TIMER_DATA	timer_data[TIMER_NUM_MAX];
volatile static T_U32 s_tick_count_main = 0;

/**
 * @brief Start the physical 2 timers
 * When system clock is changed, user should call this function to restart physical timer
 * @author ZouMai
 * @date 2004-09-22
 * @param T_fTIMER_CALLBACK callback_func: Timer callback function
 * @param T_U32 sys_clk: system clock
 * @return T_VOID
 * @retval
 */
T_VOID vtimer_init( T_VOID )
{
	T_U16	i;
	
	for ( i=0; i<TIMER_NUM_MAX; i++ )
	{
		timer_data[i].state = TIMER_INACTIVE;
	}

	/* start physical timer */
	winvme_StartTimer(PHYSICAL_TIMER, 0);

	return;
}

/**
 * @brief Start the physical 2 timers
 * When system clock is changed, user should call this function to restart physical timer
 * @author Junhua Zhao
 * @date 2005-05-31
 * @param T_fTIMER_CALLBACK callback_func: Timer callback function
 * @param T_U32 sys_clk: system clock
 * @return T_VOID
 * @retval
 */
T_VOID vtimer_change( T_U32 sys_clk)
{
	//s_system_clock = sys_clk;
}

T_TIMER vtimer_start_reserved(T_U32 milli_sec, T_BOOL loop, T_fVTIMER_CALLBACK callback_func)
{
	//use the last timerid as reserved
	if( timer_data[TIMER_NUM_MAX- 1].state == TIMER_ACTIVE )
	{
		return ERROR_TIMER;
	}
	else
	{
		timer_data[TIMER_NUM_MAX- 1].state = TIMER_ACTIVE;
		timer_data[TIMER_NUM_MAX- 1].loop = loop;
		timer_data[TIMER_NUM_MAX- 1].total_delay = milli_sec;
		timer_data[TIMER_NUM_MAX- 1].cur_delay = 0;
		timer_data[TIMER_NUM_MAX- 1].callback_func = callback_func;
	}
	return TIMER_NUM_MAX- 1;
}


/**
 * @brief Start timer
 * When the time reach, the vtimer callback function will be called. User must call function
 * vtimer_stop() to free the timer ID, in spite of loop is AK_TRUE or AK_FALSE.
 * Function vtimer_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-22
 * @param T_U32 milli_sec: Millisecond number of the timer
 * @param T_BOOL loop: loop or not
 * @param T_fVTIMER_CALLBACK callback_func: Timer callback function. If callback_func is
 *				not AK_NULL, then this callback function will be called wen time reach,
 *				else the callback function appointed by function vtimer_init() will be called.
 * @return T_TIMER: timer ID, user can stop the timer by this ID
 * @retval ERROR_TIMER: failed
 */
T_TIMER vtimer_start(T_U32 milli_sec, T_BOOL loop, T_fVTIMER_CALLBACK callback_func)
{
	T_U16	i;

	for ( i=0; i<TIMER_NUM_MAX-1; i++ )
	{
		if (timer_data[i].state == TIMER_INACTIVE)
		{
			break;
		}
	}

	//No free timer
	if( i == TIMER_NUM_MAX-1 )
	{
        AkDebugOutput("no more timer!\n");
		return ERROR_TIMER;
	}
	else
	{
		timer_data[i].state = TIMER_ACTIVE;
		timer_data[i].loop = loop;
		timer_data[i].total_delay = milli_sec;
		timer_data[i].cur_delay = 0;
		timer_data[i].callback_func = callback_func;
	}
	return i;
}

T_U32 vtimer_get_time( T_TIMER timerID )
{
	return timer_data[ timerID ].total_delay;
}

T_U32 vtimer_get_cur_time( T_TIMER timerID )
{
	return timer_data[ timerID ].cur_delay;
}

/**
 * @brief Stop timer
 * Function vtimer_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-22
 * @param T_TIMER timer_id: Timer ID
 * @return T_VOID
 * @retval
 */
T_VOID vtimer_stop(T_TIMER timer_id)
{
	if(timer_id < TIMER_NUM_MAX)
	{
		timer_data[timer_id].state = TIMER_INACTIVE;
	}
	return;
}

/**
 * @brief Timer interrupt handler for WIN32
 * If chip detect that timer counter reach 0, this function will be called.
 * Function vtimer_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-22
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE: timer interrupt occur
 */
T_BOOL vtimer_interrupt_handler_WIN32(int timer_id)
{
	T_U16	i;
	T_BOOL	occur = AK_FALSE;

	if (timer_id == 0)
	{
		for (i = 0; i < TIMER_NUM_MAX; i++)
		{
			if (timer_data[i].state == TIMER_ACTIVE)
			{
				timer_data[i].cur_delay += PHYSICAL_TIMER;
				if (timer_data[i].cur_delay >= timer_data[i].total_delay)
				{
					if (timer_data[i].loop)
					{
						timer_data[i].cur_delay = 0;
					}
					else
					{
						/* do not set state as TIMER_INACTIVE here, else this timer ID will be allocated by
							another process, in this case, call vtimer_stop() will cause mistake */
//						timer1_data[i].state = TIMER_INACTIVE;
						timer_data[i].state = TIMER_TIMEOUT;
					}
					if (timer_data[i].callback_func != AK_NULL)
					{
						timer_data[i].callback_func(i, timer_data[i].total_delay);
					}
				}
			}
		}
		occur = AK_TRUE;
		s_tick_count_main += PHYSICAL_TIMER;
    }

	return occur;
}

T_VOID vtimer_start_RTC( void )
{
}

T_VOID vtimer_stop_RTC( void )
{
}

/**
 * @brief Get tick count
 * Function vtimer_init() must be called before call this function
 * @author ZouMai
 * @date 2004-09-22
 * @param T_VOID
 * @return T_U32: tick count
 * @retval
 */

T_U32 get_tick_count(T_VOID)
{
    return s_tick_count_main;
}

/**
 * @brief Reset tick count as 0
 * @author ZouMai
 * @date 2004-09-22
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID reset_tick_count(T_VOID)
{
	s_tick_count_main = 0;
}
// java add this funtion for getting the number of valid timer. 07.9.21
T_U32 vtimer_validate_count(T_VOID  )
{
	return 8;
}
#endif
/* end of file */
