/**
 * @file Fwl_osCom.h
 * @brief This header file is for OS related function prototype
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @version 1.0
 */

#ifndef __FWL_OS_COM_H__
#define __FWL_OS_COM_H__

#include "Gbl_Global.h"

#include "Fwl_sysevent.h"


/** @defgroup FWL_OSCOM OS related interface
    @ingroup FWL
 */
/*@{*/

/** macro definition for invalid timer ID*/
#define INVALID_TIMERID -1
/** macro definition for function to extract time id from event parameter*/
#define EVTPARAM_TIMERID(_param) ((T_TIMER)((_param)->w.Param1))
/** macro definition for function of starting timer in seconds*/
#define Fwl_SetSecondTimerWithCallback(_sec,_loop, _callbackFunc) vtimer_start(_sec*1000,_loop, _callbackFunc)


/**
 * @brief Initial timer
 * 
 * @author JianhuaLiao
 * @date 2005-09-12
 * @param[in] T_VOID
 * @return void
 */
T_VOID Fwl_InitTimer(T_VOID);


/**
 * @brief Set a timer in seconds
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param[in] seconds   Time out period(second).
 * @param[in] loop    loop or not
 * @return timer ID, user can stop the timer by this ID
 */
 T_TIMER Fwl_SetTimerSecond(T_U32 seconds, T_BOOL loop);
//#define Fwl_SetTimerSecond(seconds, loop) Fwl_SetTimer((seconds)*1000, loop)

/**
 * @brief Set a timer in milliseconds
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param[in] seconds   Time out period(millisecond).
 * @param[in] loop    loop or not
 * @return timer ID, user can stop the timer by this ID
 */
T_TIMER Fwl_SetTimerMilliSecond(T_U32 milliSeconds, T_BOOL loop);
//#define Fwl_SetTimerMilliSecond(milliSeconds, loop) Fwl_SetTimer(milliSeconds, loop)

/**
 * @brief Stop a timer
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param[in] timerHandle   timer handler
 * @return always be true
 */
T_BOOL	Fwl_StopTimer(T_TIMER timerHandle);

/**
 * @brief retrieves the number of milliseconds that have elapsed 
 *		  since the system was started
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param[in] T_VOID
 * @return the number of milliseconds that have elapsed 
 *		   since the system was started
 */
T_U32	Fwl_GetTickCount(T_VOID);

/**
 * @brief switch on vibration
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param[in] T_VOID
 * @return void
 */
T_VOID  Fwl_VibrateOn(T_VOID);

/**
 * @brief switch off vibration
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param[in] T_VOID
 * @return void
 */
T_VOID  Fwl_VibrateOff(T_VOID);

/**
 * @brief Compare the timer ids which are extracted from the event parameters
 * 
 * @author ZouMai
 * @date 2001-06-18
 * @param[in] evtParm1   the first event parameter
 * @param[in] evtParm2   the second event parameter
 * @return same or different
 */
T_BOOL Fwl_CompareTimerEvtParam(const T_SYS_MAILBOX *pMailBox1, const T_SYS_MAILBOX *pMailBox2);

/**
 * @brief MMUTT init
 * 
 * @author sunyunfeng
 * @date 2007-04-27
 * @param[in] T_VOID
 * @return T_VOID
 */

T_VOID Fwl_InitMMU(T_VOID);

/**
 * @brief millisecond delay
 * @author 
 * @date 2010-10-14
 * @param[in] minisecond minisecond delay number
 * @return T_VOID
 */

 T_VOID Fwl_MiniDelay(T_U32 minisecond);

 /**
 * @brief vtimer_start
 * @author 
 * @date 2010-10-14
 * @param[in] timer start
 * @return T_VOID
 */
typedef T_VOID (*T_TIMER_CALLBACK)(T_TIMER timer_id, T_U32 delay);  //follow T_fVTIMER_CALLBACK
T_TIMER Fwl_SetMSTimerWithCallback(T_U32 milli_sec, T_BOOL loop, T_TIMER_CALLBACK callback_func);


/**
 * @brief watch dog function init
 * @author liao_zhijun
 * @date 2014-05-28
 * @param T_U16 feedtime:watch dog feed time, feedtime unit:ms
 * @return T_VOID
  */
T_VOID Fwl_Watchdog_Timer_Start(T_U32 feed_time);

/**
 * @brief watch dog feed
 * @author liao_zhijun
 * @date 2014-05-28
 * @return T_VOID
  */
T_VOID Fwl_Watchdog_Timer_Feed(T_VOID);

/**
 * @brief watch dog stop
 * @author liao_zhijun
 * @date 2014-05-28
 * @return T_VOID
  */
T_VOID Fwl_Watchdog_Timer_Stop(T_VOID);


/*@}*/
#endif


