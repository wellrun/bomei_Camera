/**
 * @file hal_timer.c
 * @brief Virtual timer function header file
 * This file provides virtual timer APIs: initialization, start timer, stop timer and
 * timer interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
  */
#include <string.h>
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "hal_timer.h"
#include "interrupt.h"
#include "drv_api.h"
#include "drv_module.h"
#include "timer.h"

/** @{@name Define the timer statu */
/** timer inactive(available) */
#define TIMER_INACTIVE                          0       
/** timer active */
#define TIMER_ACTIVE                            1
/** timer time out */
#define TIMER_TIMEOUT                           2               
/** @} */

#define VTIMER_MESSAGE                          3


/** @{@name Define the total timer num */
#define TIMER_NUM_MAX                           32
/** @} */

//virtual timer struct
typedef struct
{
        T_U32   state;                          /* TIMER_INACTIVE or TIMER_ACTIVE or TIMER_TIMEOUT */
        T_BOOL  loop;                           /* loop or not */
        T_U32   total_delay;                    /* total delay, microseconds */
        T_U32   cur_delay;                      /* current delay, microseconds */
        T_fVTIMER_CALLBACK callback_func;       /* callback function only for current timer */
} T_TIMER_DATA;

//akos task manager timer handler
extern T_VOID TMT_Timer0_Interrupt(T_VOID);

//virtual timer global variable
static T_TIMER_DATA     s_timer_data[TIMER_NUM_MAX];

//current virtual timer count
static T_S32 s_maxid = -1;

static T_VOID vtimer_callback(T_TIMER timer_id, T_U32 delay);
static T_VOID AK_callback(T_TIMER timer_id, T_U32 delay);
static T_VOID vtimer_handler(T_U32 *param, T_U32 len);

#ifdef OS_ANYKA
/********vtimer logic, hardware timer logic and system control init ********/
T_VOID vtimer_init( T_VOID )
{
    T_U32   i;

    /* init vtimer control logic */
    for ( i=0; i<TIMER_NUM_MAX; i++ )
    {
        s_timer_data[i].state = TIMER_INACTIVE;
        s_timer_data[i].callback_func = AK_NULL;
    }

    /* init vtimer system control logic  */
    DrvModule_Create_Task(DRV_MODULE_VTIMER);
    DrvModule_Map_Message(DRV_MODULE_VTIMER, VTIMER_MESSAGE, vtimer_handler);

    /* init hardware timer */
    timer_init();

    /* start system schedule timer and vtimer*/
#ifdef AKOS
    timer_start(uiTIMER2, timer_interval(uiTIMER2), AK_TRUE, AK_callback);
#endif // #ifdef AKOS
    timer_start(uiTIMER2, timer_interval(uiTIMER2), AK_TRUE, vtimer_callback);

}

T_VOID vtimer_free( T_VOID )
{
    T_U32   i;
    
    //reset hard timer
    timer_reset();

    /* reset vtimer control logic */
    for ( i=0; i<TIMER_NUM_MAX; i++ )
    {
        s_timer_data[i].state = TIMER_INACTIVE;
        s_timer_data[i].callback_func = AK_NULL;
    }    
}

/**
 * @brief Start timer
 * When the time reach, the vtimer callback function will be called. User must call function
 * vtimer_stop() to free the timer ID, in spite of loop is AK_TRUE or AK_FALSE.
 * Function vtimer_init() must be called before call this function
 * @author LiaoZhijun
 * @date 2010-05-27
 * @param T_U32 milli_sec: Specifies the time-out value, in millisecond. Caution, this value must can be divided by 10.
 * @param T_BOOL loop: loop or not
 * @param T_fVTIMER_CALLBACK callback_func: Timer callback function. If callback_func is
 *                              not AK_NULL, then this callback function will be called when time reach.
 * @return T_TIMER: timer ID, user can stop the timer by this ID
 * @retval ERROR_TIMER: failed
 */
T_TIMER vtimer_start(T_U32 milli_sec, T_BOOL loop, T_fVTIMER_CALLBACK callback_func)
{
    T_S32 i;
        
    DrvModule_Protect(DRV_MODULE_VTIMER);
    
    for ( i= 0; i<TIMER_NUM_MAX; i++ )
    {
        if (s_timer_data[i].state == TIMER_INACTIVE)
        {
            break;
        }
    }
    
    if( i == TIMER_NUM_MAX )
    {
        akprintf(C1, M_DRVSYS, "no more timer!!!!!!!!!!!!!!!!!!\r\n");
        i = ERROR_TIMER;
    }
    else
    {
        //record the max timer id
        if (i > s_maxid)
                s_maxid = i;

        //set the timer data
        s_timer_data[i].state = TIMER_ACTIVE;
        s_timer_data[i].loop = loop;
        s_timer_data[i].total_delay = milli_sec;
        s_timer_data[i].cur_delay = 0;
        s_timer_data[i].callback_func = callback_func;
    }
        
    DrvModule_UnProtect(DRV_MODULE_VTIMER);
    
    return i;
}

//get timer total delay
T_U32 vtimer_get_time( T_TIMER timerID )
{
    T_U32 total_time = 0;
    
    if (ERROR_TIMER != timerID
        && timerID < TIMER_NUM_MAX)
    {
       total_time = s_timer_data[ timerID ].total_delay; 
    }

   return total_time;
        
}

//get timer current delay
T_U32 vtimer_get_cur_time( T_TIMER timerID )
{
    T_U32 cur_time = 0;
    
    if (ERROR_TIMER != timerID
        && timerID < TIMER_NUM_MAX)
    {
       cur_time = s_timer_data[ timerID ].cur_delay;
    }
    
    return cur_time;
}

/**
 * @brief Stop timer
 * Function vtimer_init() must be called before call this function
 * @author MiaoBaoli
 * @author LiaoZhijun
 * @date 2010-05-27
 * @return T_VOID
 * @retval
 */
T_VOID vtimer_stop(T_TIMER timer_id)
{
    if(timer_id < 0 || timer_id >= TIMER_NUM_MAX)
    {
        akprintf(C2, M_DRVSYS, "stop the invalid timer!\r\n");
        return;
    }
        
    DrvModule_Protect(DRV_MODULE_VTIMER);

    s_timer_data[timer_id].state = TIMER_INACTIVE;
    s_timer_data[timer_id].callback_func = AK_NULL;

    if (timer_id == s_maxid)
    {
        while (s_maxid>=0 && s_timer_data[s_maxid].state == TIMER_INACTIVE)
            s_maxid--;
    }
                
    DrvModule_UnProtect(DRV_MODULE_VTIMER);       
    return;
}

/**
*       Return the number of valid timers, added by java function 07.9.20
*     Sometimes timers were not closed after having been used. Add this 
*     API so that you can know how many valid timers you can used still.
*/
T_U32 vtimer_validate_count(T_VOID  )
{
    T_U32 cnt = 0;
    T_U32 i;
    
    for ( i = 0; i<TIMER_NUM_MAX; i++ )
    {
        if (s_timer_data[i].state == TIMER_INACTIVE)
        {
            cnt++;
        }
    }

    return cnt;
}

//vtimer handler, used in task callback
T_VOID vtimer_handler(T_U32 *param, T_U32 len)
{
    T_S32 i;
    T_fVTIMER_CALLBACK  callback_func;
    T_TIMER_DATA *p = AK_NULL;

    for (i = 0; i <= s_maxid; i++)
    {
        p = &(s_timer_data[i]);
        if (p->state == TIMER_ACTIVE)
        {
            p->cur_delay += timer_interval(uiTIMER2);
            if (p->cur_delay >= p->total_delay)
            {
                if (p->loop)
                {
                    p->cur_delay = 0;
                }
                else
                {
                    /* do not set state as TIMER_INACTIVE here, else this timer ID will be allocated by
                    another process, in this case, call vtimer_stop() will cause mistake */
                    //p->state = TIMER_INACTIVE;
                    DrvModule_Protect(DRV_MODULE_VTIMER);
                    if (TIMER_ACTIVE == p->state)
                        p->state = TIMER_TIMEOUT;
                    DrvModule_UnProtect(DRV_MODULE_VTIMER);
                }

                callback_func = p->callback_func;
                if (AK_NULL != callback_func)
                {
                    callback_func(i, p->total_delay);
                }
            }
        }
    }
}

static T_VOID vtimer_callback(T_TIMER timer_id, T_U32 delay)
{
    DrvModule_Send_Message(DRV_MODULE_VTIMER, VTIMER_MESSAGE, AK_NULL);
}

#ifdef AKOS
/* system task schedule timer callback */
static T_VOID AK_callback(T_TIMER timer_id, T_U32 delay)
{
    TMT_Timer0_Interrupt();
}
#endif // #ifndef AKOS

#endif//#ifdef OS_ANYKA

