/**
 * @file hal_timer.h
 * @brief Virtual timer function header file
 *
 * This file provides virtual timer APIs: initialization, start timer, stop timer and
 * timer interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Liao_Zhijun
 * @date 2010-04-15
 * @version 1.0
 */

#ifndef __HAL_TIMER_H__
#define __HAL_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

/** @defgroup Timer Timer group
 *	@ingroup Drv_Lib
 */
/*@{*/
/*@}*/


/** @defgroup VTimer VTimer group
 *	@ingroup Timer
 */
/*@{*/

/** @name ERROR TIMER Define
 *	
 */
/*@{*/
#define ERROR_TIMER			-1
/*@}*/

/**
 * @brief: Timer Callback type define.
 */
typedef T_VOID (*T_fVTIMER_CALLBACK)(T_TIMER timer_id, T_U32 delay);

/**
 * @brief  Init virtual timer and hardware timer. 
 *         and then open the hardware timer interrupt;
 * @author liaozhijun
 * @date 2010-04-06
 * @return T_VOID
 */
T_VOID vtimer_init( T_VOID );

/**
 * @brief  free virtual timer and hardware timer.
 * @author liaozhijun
 * @date 2010-04-06
 * @return T_VOID
 */
T_VOID vtimer_free( T_VOID );

/**
 * @brief Start vtimer
 *
 * When the time reaches, the vtimer callback function will be called. User must call function
 * vtimer_stop() to free the timer ID, in spite of loop is AK_TRUE or AK_FALSE.
 * Function  must called vtimer_init() before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @param milli_sec [in] Specifies the time-out value, in millisecond. Caution, this value must can be divided by 5.
 * @param loop [in] oop or not
 * @param callback_func [in] Timer callback function. If callback_func is
 *				not AK_NULL, then this callback function will be called when time reaches.
 * @return T_TIMER
 * @retval timer_ID user can stop the timer by this ID
 * @retval ERROR_TIMER start failed
 */
T_TIMER vtimer_start(T_U32 milli_sec, T_BOOL loop, T_fVTIMER_CALLBACK callback_func);

/**
 * @brief Stop vtimer
 *
 * Function  must called vtimer_init() before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @param timer_id [in] Timer ID
 * @return T_VOID
 */
T_VOID vtimer_stop(T_TIMER timer_id);

/**
 * @brief Get Timer total delay, count by ms
 *
 * Function  must called vtimer_init() before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @param timerID [in] Timer ID
 * @return T_U32 total delay of that timer
 */
T_U32 vtimer_get_time( T_TIMER timerID );

/**
 * @brief Get Timer current value, count by ms
 *
 * Function  must called vtimer_init() before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @param timerID [in] Timer ID
 * @return T_U32 current value of that timer
 */
T_U32 vtimer_get_cur_time( T_TIMER timerID );

/**
 * @brief Get unused timer number.
 *
 * Function  user should call vtimer_init() before calling this function
 * @author liaozhijun
 * @date 2010-04-06
 * @return T_U32 the number of unused timer
 */
T_U32 vtimer_validate_count(T_VOID);


/**
 * @brief Get ms level tick count from hardware timer
 *
 * tick count value is calculated from timer5
 * Function vtimer_init() must be called before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @return T_U32
 * @retval tick_count whose unit is ms
 */
T_U32 get_tick_count(T_VOID);

/**
 * @brief Get us level tick count from hardware timer
 *
 * tick count value is calculated from timer5.
 * Function vtimer_init() must be called before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @return T_U64
 * @retval tick_count whose unit is us
 */
T_U64 get_tick_count_us(T_VOID);


#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

/*@}*/
#endif // #ifndef __HAL_TIMER_H__

