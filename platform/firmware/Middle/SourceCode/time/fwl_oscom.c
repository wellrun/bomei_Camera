/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: Fwl_osCOM.c
* Function: This file will constraint the access to the bottom layer common
            os system, avoid resource competition. Also, this file os for
            porting to different OS
*
* Author: Zou Mai
* Date: 2001-06-14
* Version: 1.0
*
* Revision: 
* Author: 
* Date: 
***************************************************************************/

#include "Fwl_public.h"
#include "Fwl_EvtMailBox.h"
#include "Fwl_osCom.h"
#include "fwl_vme.h"
#include "hal_timer.h"
#include "arch_mmu.h"
#include "raminit.h"
#include "hal_sysdelay.h"
#include "drv_timer.h"

extern T_TIMER                HandsetTimer  ;
extern volatile T_BOOL        Handset1sInsert   ;

T_S32 AK_Semaphore_Information(T_hSemaphore, T_U32 *, T_U8 *, T_U32 *, T_U32 *);

/**
 * @brief Initial timer
 * 
 * @author JianhuaLiao
 * @date 2005-09-12
 * @retval 
 */
T_VOID Fwl_InitTimer(T_VOID)
{
    vtimer_init();
}

static T_VOID Fwl_Timer_Callback(T_TIMER timer_id, T_U32 delay)
{
    T_SYS_MAILBOX mailbox;
    mailbox.event = SYS_EVT_TIMER;
    if (gb.s_public_timer_id == timer_id)
    {
        mailbox.event = SYS_EVT_PUB_TIMER;
    }
    
    mailbox.param.w.Param1 = timer_id;
    mailbox.param.w.Param2 = delay;

    //AK_PostUniqueEvent(&mailbox, Fwl_CompareTimerEvtParam);
    //AK_PostUniqueEventToHead(&mailbox, Fwl_CompareTimerEvtParam);
    AK_PostEventEx(&mailbox, Fwl_CompareTimerEvtParam, AK_TRUE, AK_FALSE, AK_FALSE);

    
    return;
}


T_TIMER Fwl_SetTimer(T_U32 milliSeconds, T_BOOL loop)
{
    return vtimer_start(milliSeconds, loop, Fwl_Timer_Callback);
}


/**
 * @brief Set a timer in seconds
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param T_U32 seconds: Time out period(second).
 * @param T_U8 subEvent: Timer ID
 * @return T_VOID
 * @retval void
 */
T_TIMER Fwl_SetTimerSecond(T_U32 seconds, T_BOOL loop)
{
    return Fwl_SetTimer(seconds*1000, loop);

}

/**
 * @brief Set a timer in milliseconds
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param T_S16 timeout: Time out period(millisecond).
 * @param T_S16 timerid: Timer ID
 * @return T_VOID
 * @retval void
 */
T_TIMER Fwl_SetTimerMilliSecond(T_U32 milliSeconds, T_BOOL loop)
{
    return Fwl_SetTimer(milliSeconds, loop);
}

/**
 * @brief Stop a timer
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param T_TIMER timerHandle: timer handler
 * @return T_BOOL
 * @retval
 */
T_BOOL Fwl_StopTimer(T_TIMER timer)
{
    vtimer_stop(timer);
    return AK_TRUE;
}

T_VOID Fwl_InitMMU()
{
#ifndef OS_WIN32
    MMU_Init(Ram_getMMUaddr());
#endif
}


T_BOOL Fwl_CompareTimerEvtParam(const T_SYS_MAILBOX *pMailBox1, const T_SYS_MAILBOX *pMailBox2)
{
    if (pMailBox1->event == pMailBox2->event)
    {
        if(EVTPARAM_TIMERID(&pMailBox1->param) == EVTPARAM_TIMERID(&pMailBox2->param))
        {
            return AK_TRUE;
        }
    }
    
    return AK_FALSE;
}


T_U32 Fwl_GetTickCount(T_VOID)
{
    return get_tick_count();
}


/**
 * @brief millisecond delay
 * @author 
 * @date 2010-10-14
 * @param[in] minisecond minisecond delay number
 * @return T_VOID
 */

 T_VOID Fwl_MiniDelay(T_U32 minisecond)
 {
 	mini_delay(minisecond);
 }

/**
 * @brief vtimer_start
 * @author 
 * @date 2010-10-14
 * @param[in] timer start
 * @return T_VOID
 */

T_TIMER Fwl_SetMSTimerWithCallback(T_U32 milli_sec, T_BOOL loop, T_fVTIMER_CALLBACK callback_func)
{
	return vtimer_start(milli_sec, loop, callback_func);
}



T_U32 AK_Get_SemVal(T_hSemaphore semaphore)
{
	T_U32 cnt = 0;
#ifdef OS_ANYKA
	T_U8  supsend = 0;
	T_U32 wait = 0;
	T_U32 firsttask = 0;
	
    AK_Semaphore_Information(semaphore, &cnt, &supsend, &wait, &firsttask);
#endif
	return cnt;
}


T_S32 AK_Try_Obtain_Semaphore(T_hSemaphore semaphore, T_U32 suspend)
{
	T_U32 cnt = AK_Get_SemVal(semaphore);

	//AkDebugOutput(":::%d_%d_%d_%d\n",cnt,supsend,wait,firsttask);
	if (cnt > 0)
	{
		AK_Obtain_Semaphore(semaphore, suspend);
	}

	return cnt;
}


/**
 * @brief watch dog function init
 * @author liao_zhijun
 * @date 2014-05-28
 * @param T_U16 feedtime:watch dog feed time, feedtime unit:ms
 * @return T_VOID
  */
T_VOID Fwl_Watchdog_Timer_Start(T_U32 feed_time)
{
	watchdog_timer_start(feed_time);
}

/**
 * @brief watch dog feed
 * @author liao_zhijun
 * @date 2014-05-28
 * @return T_VOID
  */
T_VOID Fwl_Watchdog_Timer_Feed(T_VOID)
{
	watchdog_timer_feed();
}

/**
 * @brief watch dog stop
 * @author liao_zhijun
 * @date 2014-05-28
 * @return T_VOID
  */
T_VOID Fwl_Watchdog_Timer_Stop(T_VOID)
{
	watchdog_timer_stop();
}



