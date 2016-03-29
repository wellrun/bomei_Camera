/**
 * @file drv_timer.h
 * @brief hardware timer function header file.
 *
 * This file provides hardware timer APIs: start timer, stop timer and
 * timer interrupt handler.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun
 * @date 2010-04-15
 * @version 1.0
 */

#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus


/** @defgroup HTimer Hardware Timer group
 *      ingroup Timer
 */

/*@{*/

#include "anyka_types.h"

/** @{@name ERROR TIMER Define
 *	
 */
#define ERROR_TIMER             -1
/** @} */

/**
 * @brief hardware timer define
 *   define all the hardware timer
 */
 
typedef enum
{
    uiTIMER0 = 0,           ///< TIMER1
    uiTIMER1,               ///< TIMER2
    uiTIMER2,               ///< TIMER3
    uiTIMER3,               ///< TIMER4
    uiTIMER4,               ///< TIMER5

    HARDWARE_TIMER_NUM      ///< MAX TIMER number
} T_TIMER_ID;

/**
 * @brief: Timer Callback type define.
 */
typedef T_VOID (*T_fTIMER_CALLBACK)(T_TIMER timer_id, T_U32 delay);

/**
 * @brief Start timer
 *
 * When the time reach, the timer callback function will be called. 
 * Function vtimer_init() must be called before call this function
 * @author liao_zhijun
 * @date 2010-04-15
 * @param[in]  hardware_timer hardware timer ID, uiTIMER0~uiTIMER3, cannot be uiTIMER4, because it is used for tick count
 * @param[in]  milli_sec Specifies the time-out value, in millisecond. Caution, this value must can be divided by 20.
 * @param[in] loop loop or not
 * @param[in] callback_func Timer callback function. If callback_func is
 *      not AK_NULL, then this callback function will be called when time reach.
 * @return T_TIMER timer ID user can stop the timer by this ID
 * @retval ERROR_TIMER: failed
 */
T_TIMER timer_start(T_TIMER_ID hardware_timer, T_U32 milli_sec, T_BOOL loop, T_fTIMER_CALLBACK callback_func);

/**
 * @brief Stop timer
 *
 * Function vtimer_init() must be called before call this function
 * @author liao_zhijun
 * @date 2010-04-15
 * @param[in] timer_id Timer ID, uiTIMER0~uiTIMER3, cannot be uiTIMER4, because it is used for tick count
 * @return T_VOID
 */
T_VOID timer_stop(T_TIMER timer_id);

/**
 * @brief watch dog function init
 * @author liao_zhijun
 * @date 2014-05-28
 * @param T_U16 feedtime:watch dog feed time, feedtime unit:ms, MAX = 171798ms
 * @return T_VOID
  */
T_VOID watchdog_timer_start(T_U32 feed_time);

/**
 * @brief watch dog feed
 * @author liao_zhijun
 * @date 2014-05-28
 * @return T_VOID
  */
T_VOID watchdog_timer_feed(T_VOID);

/**
 * @brief watch dog stop
 * @author liao_zhijun
 * @date 2014-05-28
 * @return T_VOID
  */
T_VOID watchdog_timer_stop(T_VOID);


/*@}*/

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // #ifndef __DRV_TIMER_H__

