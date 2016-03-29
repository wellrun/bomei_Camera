/**
 * @file timer.c
 * @brief hardware timer source file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "interrupt.h"
#include "drv_api.h"
#include "drv_module.h"
#include "timer.h"

#ifdef OS_ANYKA

#define TICK_WITH_OS_AND_VTIMEER 0

//define timer register bits
#define TIMER_CLEAR_BIT                 (1<<30)
#define TIMER_FEED_BIT                  (1<<29)
#define TIMER_ENABLE_BIT                (1<<28)
#define TIMER_STATUS_BIT                (1<<27)
#define TIMER_READ_SEL_BIT              (1<<26)

//define pwm/pwm mode
#define MODE_PWM                        0x2
#define MODE_ONE_SHOT_TIMER             0x1
#define MODE_AUTO_RELOAD_TIMER          0x0       

//define timer frequency
#define TIMER_FREQ                      (25000000)

//max timer counter value
#define TIMER_MAX_COUNT                 0xffffffff
#define TIMER5_COUNT                 125000000

//max timer count for each hardware timer
#define TIMER_NUM_MAX                   6

//timer status
#define TIMER_INACTIVE                  0               /* inactive(available) */
#define TIMER_ACTIVE                    1               /* active */
#define TIMER_TIMEOUT                   2               /* time out */

typedef struct
{
        T_U8    state;                  /* TIMER_INACTIVE or TIMER_ACTIVE or TIMER_TIMEOUT */
        T_BOOL  loop;                   /* loop or not */
        T_U32   total_delay;            /* total delay, microseconds */
        T_U32   cur_delay;              /* current delay, microseconds */
        T_fTIMER_CALLBACK callback_func;/* callback function only for current timer */
} T_TIMER_DATA;


//hardware timer struct
typedef struct
{
        T_U32   interval; // interval time is ms
        T_U32   clock;    // interval clock
        T_BOOL  bStatus; //open or close
        T_BOOL  bInit;   // init flag
    T_TIMER_DATA timer_data[TIMER_NUM_MAX];
}T_HARDWARE_TIMER;

//harware timer assignment 
static T_HARDWARE_TIMER m_hardtimer[ HARDWARE_TIMER_NUM ] = { 
          {250, 0, AK_FALSE, AK_FALSE, {0}},  //undefine function
          {10,  0, AK_FALSE, AK_FALSE, {0}},   //instant, keypad,touchscr 
          {5,   0, AK_FALSE, AK_FALSE, {0}},   //normal, vtimer,akos
          {1,   0, AK_FALSE, AK_FALSE, {0}},   //watch dog 
          {5,   0, AK_FALSE, AK_FALSE, {0}},    //tick count 
        };

static const T_U32 timer_ctrl_reg1_grp[HARDWARE_TIMER_NUM] = {PWM_TIMER1_CTRL_REG1, PWM_TIMER2_CTRL_REG1, PWM_TIMER3_CTRL_REG1, PWM_TIMER4_CTRL_REG1, PWM_TIMER5_CTRL_REG1};
static const T_U32 timer_ctrl_reg2_grp[HARDWARE_TIMER_NUM] = {PWM_TIMER1_CTRL_REG2, PWM_TIMER2_CTRL_REG2, PWM_TIMER3_CTRL_REG2, PWM_TIMER4_CTRL_REG2, PWM_TIMER5_CTRL_REG2};
static const T_U32 timer_int_mask_grp[HARDWARE_TIMER_NUM] = {IRQ_MASK_TIMER1_BIT, IRQ_MASK_TIMER2_BIT, IRQ_MASK_TIMER3_BIT, IRQ_MASK_TIMER4_BIT, IRQ_MASK_TIMER5_BIT};

//variable contains the us tick count, increase at each timer5 interrupt
static volatile T_U64 s_tick_count_us = 0;
static volatile T_U64 s_tick_count_us_in_vtimer = 0;

//variable indicate wether timer5 is init or not
static T_BOOL m_bRTCStart = AK_FALSE;
//variable indicate how many timer clocks in one us 
static T_U32  m_clkPerUs = TIMER_FREQ / 1000000;
//variable indicate how many ticks in a MAX timer cycle
static T_U32  m_rtcCountUs = 0;

static T_BOOL timer_interrupt_handler(T_VOID);
static T_VOID timer5_interrupt_handler(T_VOID);

/**
 * @brief: Init timer, initial global variables, enable timer interrupt
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @return T_VOID
 * @retval
 */
T_VOID timer_init( T_VOID )
{
    T_U16   i, j;
    T_U32   clkPerMs;

    // init hardware timer struct
    for(j = 0; j < HARDWARE_TIMER_NUM; j++)
    {
        for ( i = 0; i < TIMER_NUM_MAX; i++ )
        {
            m_hardtimer[j].timer_data[i].state = TIMER_INACTIVE;
            m_hardtimer[j].timer_data[i].callback_func = AK_NULL;
        }
    }

    clkPerMs = TIMER_FREQ / 1000;
    m_clkPerUs = TIMER_FREQ / 1000000;
    m_rtcCountUs = TIMER5_COUNT / m_clkPerUs;
    
    for (i = 0; i < HARDWARE_TIMER_NUM; i++)
        m_hardtimer[i].clock = ( clkPerMs * m_hardtimer[i].interval );

    timer_reset();
        
    m_bRTCStart = AK_TRUE;
    s_tick_count_us = 0;

    //enable GPIO/timer interrupt
    int_register_irq(INT_VECTOR_TIMER, timer_interrupt_handler);

    /* start timer5 for tickcount */
    #if TICK_WITH_OS_AND_VTIMEER
    REG32(PWM_TIMER5_CTRL_REG1) = TIMER5_COUNT;
    REG32(PWM_TIMER5_CTRL_REG2) = TIMER_ENABLE_BIT | TIMER_FEED_BIT | (MODE_ONE_SHOT_TIMER << 24);
	#endif
    /* set init flag */
    for (i=0; i<HARDWARE_TIMER_NUM; i++)
    {
        m_hardtimer[i].bInit = AK_TRUE;
    }

    //enable timer5 interrupt
    INTR_ENABLE_L2(IRQ_MASK_TIMER5_BIT);

    return;
}

/**
 * @brief Start timer
 * When the time reach, the vtimer callback function will be called. User must call function
 * vtimer_stop() to free the timer ID, in spite of loop is AK_TRUE or AK_FALSE.
 * Function vtimer_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param T_U32 milli_sec: Specifies the time-out value, in millisecond. Caution, this value must can be divided by 10.
 * @param T_BOOL loop: loop or not
 * @param T_fVTIMER_CALLBACK callback_func: Timer callback function. If callback_func is
 *                              not AK_NULL, then this callback function will be called when time reach.
 * @return T_TIMER: timer ID, user can stop the timer by this ID
 * @retval ERROR_TIMER: failed
 */
T_TIMER timer_start(T_TIMER_ID hardware_timer, T_U32 milli_sec, T_BOOL loop, T_fTIMER_CALLBACK callback_func)
{
    T_U16   i;
    T_U32 mode;

    /* check timer id */
    if(hardware_timer > uiTIMER4)
        return ERROR_TIMER;
    
    /* check init flag */
    if(AK_FALSE == m_hardtimer[hardware_timer].bInit)
        return ERROR_TIMER;

    /* disable gpio&timer interrupt to avoid reenter*/
    INTR_DISABLE(IRQ_MASK_SYS_MODULE_BIT);
    
    for (i = 0; i < TIMER_NUM_MAX; i++ )
    {
        if (m_hardtimer[hardware_timer].timer_data[i].state == TIMER_INACTIVE)
        {
            break;
        }
    }

    //No free timer
    if( i == TIMER_NUM_MAX) //TIMER_NUM_MAX - 1 is reserved timer
    {
        akprintf(C3, M_DRVSYS, "no more timer %d!!!!!!!!!!!!!!!!!!\r\n", i);
        INTR_ENABLE(IRQ_MASK_SYS_MODULE_BIT);
        return ERROR_TIMER;
    }
    else
    {
        m_hardtimer[hardware_timer].timer_data[i].state = TIMER_ACTIVE;
        m_hardtimer[hardware_timer].timer_data[i].loop = loop;
        m_hardtimer[hardware_timer].timer_data[i].total_delay = milli_sec;
        m_hardtimer[hardware_timer].timer_data[i].cur_delay = 0;
        m_hardtimer[hardware_timer].timer_data[i].callback_func = callback_func;

        if( m_hardtimer[hardware_timer].bStatus == AK_FALSE )
        {
            m_hardtimer[hardware_timer].bStatus = AK_TRUE;

            mode = MODE_AUTO_RELOAD_TIMER;

            REG32(timer_ctrl_reg1_grp[hardware_timer]) = m_hardtimer[hardware_timer].clock;
            REG32(timer_ctrl_reg2_grp[hardware_timer]) = TIMER_ENABLE_BIT | TIMER_FEED_BIT | (mode << 24);
            INTR_ENABLE_L2(timer_int_mask_grp[hardware_timer]);
        }
    }
    
    INTR_ENABLE(IRQ_MASK_SYS_MODULE_BIT);

    return (hardware_timer * TIMER_NUM_MAX + i);
}

/**
 * @brief Stop timer
 * Function vtimer_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param T_TIMER timer_id: Timer ID
 * @return T_VOID
 * @retval
 */
T_VOID timer_stop(T_TIMER timer_id)
{
    T_U16   i, hardware_timer;

    if(timer_id < 0 || timer_id >= HARDWARE_TIMER_NUM * TIMER_NUM_MAX)
    {
        akprintf(C3, M_DRVSYS, "stop the invalid timer!\r\n");
        return;
    }

    hardware_timer = timer_id / TIMER_NUM_MAX;
    timer_id = timer_id % TIMER_NUM_MAX;

    /* check init flag */
    if(AK_FALSE == m_hardtimer[hardware_timer].bInit)
        return;


    INTR_DISABLE(IRQ_MASK_SYS_MODULE_BIT);

    m_hardtimer[hardware_timer].timer_data[timer_id].state = TIMER_INACTIVE;

    for(i = 0; i < TIMER_NUM_MAX; i++ )
    {
        if(m_hardtimer[hardware_timer].timer_data[i].state != TIMER_INACTIVE)
        {
            INTR_ENABLE(IRQ_MASK_SYS_MODULE_BIT);       
            return;
        }
    }

    INTR_DISABLE_L2(timer_int_mask_grp[hardware_timer]);
    REG32(timer_ctrl_reg2_grp[hardware_timer]) = 0;

    m_hardtimer[hardware_timer].bStatus = AK_FALSE;

    INTR_ENABLE(IRQ_MASK_SYS_MODULE_BIT);

    return;
}

/**
 * @brief Timer interrupt handler
 * If chip detect that timer counter reach 0, this function will be called.
 * Function vtimer_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param T_VOID
 * @return T_VOID

 *      NOTE    1. 若使能计数, 则当计算到0后, 还会继续递减(从最大值开始);
 *                         若不使能计数, 则保护为禁止计数时的值;
 *                      2. 若计数值为0, 但使能计数, 此时打开中断, 会引起中断的产生;
 *                         若计数值为0, 但禁止计数, 此时打开中断, 不会引起中断的产生;
 *                      3. a) 读中断状态寄存器(0x20000014), 并不会清除其TIMER BIT的值,
 *                                也不会清控制寄存器(0x20090014等)的BIT30(中断状态);
 *                         b) 读TIMER的控制寄存器(0x20090014等), 会清除中断状态寄存器的TIMER BIT的值,
 *                                同时也清控制寄存器(0x20090014等)的BIT30(中断状态);
 *                                直接将这一位设为0也能达到同样效果.
 *                      4. 进入中断后立即读计数值, 并不为0, 因为计数不会自动停止.
 *                      5. 若使用中断, 则计数值不能太小, 否则中断将占用大部分CPU时间而使程序运行异常.
 *                              (除中断函数外其它函数无法运行);


 */

static T_VOID timer_handler(T_TIMER_ID hardware_timer)
{
    T_U16 i = 0;
    
    for (i = 0 ; i < TIMER_NUM_MAX; i++)
    {
        if (m_hardtimer[hardware_timer].timer_data[i].state == TIMER_ACTIVE)
        {
            m_hardtimer[hardware_timer].timer_data[i].cur_delay += m_hardtimer[hardware_timer].interval;
            
            if (m_hardtimer[hardware_timer].timer_data[i].cur_delay >= m_hardtimer[hardware_timer].timer_data[i].total_delay)
            {
                if (m_hardtimer[hardware_timer].timer_data[i].loop)
                {
                    m_hardtimer[hardware_timer].timer_data[i].cur_delay = 0;
                }
                else
                {
                    /* do not set state as TIMER_INACTIVE here, else this timer ID will be allocated by
                        another process, in this case, call vtimer_stop() will cause mistake */
//                      timer_data[i].state = TIMER_INACTIVE;
                    m_hardtimer[hardware_timer].timer_data[i].state = TIMER_TIMEOUT;
                }

                if (m_hardtimer[hardware_timer].timer_data[i].callback_func != AK_NULL)
                {
                    m_hardtimer[hardware_timer].timer_data[i].callback_func(
                        hardware_timer * TIMER_NUM_MAX + i, 
                        m_hardtimer[hardware_timer].timer_data[i].total_delay);
                }
            }
        }
    }
}

/**
 * @brief: gpio and timer interrupt handler
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
static T_BOOL timer_interrupt_handler(T_VOID)
{               
    if( *( volatile T_U32* )INT_SYS_MODULE_REG & INT_STATUS_TIMER1_BIT )
    {               
        REG32(PWM_TIMER1_CTRL_REG2) |= TIMER_CLEAR_BIT;
        timer_handler(uiTIMER0);
    }
    else if( *( volatile T_U32* )INT_SYS_MODULE_REG & INT_STATUS_TIMER2_BIT )
    {
        REG32(PWM_TIMER2_CTRL_REG2) |= TIMER_CLEAR_BIT;
        timer_handler(uiTIMER1);
    }
    else if( *( volatile T_U32* )INT_SYS_MODULE_REG & INT_STATUS_TIMER3_BIT )
    {
        REG32(PWM_TIMER3_CTRL_REG2) |= TIMER_CLEAR_BIT;
        s_tick_count_us_in_vtimer += m_hardtimer[uiTIMER2].interval;
        timer_handler(uiTIMER2);
    }
    else if( *( volatile T_U32* )INT_SYS_MODULE_REG & INT_STATUS_TIMER4_BIT )
    {
        REG32(PWM_TIMER4_CTRL_REG2) |= TIMER_CLEAR_BIT;
        timer_handler(uiTIMER3);
    }
    else if( *( volatile T_U32* )INT_SYS_MODULE_REG & INT_STATUS_TIMER5_BIT )
    {
        REG32(PWM_TIMER5_CTRL_REG2) |= TIMER_CLEAR_BIT;
        #if TICK_WITH_OS_AND_VTIMEER
        timer5_interrupt_handler();
		#else
        timer_handler(uiTIMER4);
		#endif
    }
    else
    {
        return AK_FALSE;
    }
    
    return AK_TRUE;
}



/**
 * @brief Timer4 interrupt handler
 * If chip detect that timer counter reach 0, this function will be called.
 * Function vtimer_init() must be called before call this function
 * @author MiaoBaoli
 * @date 2004-09-22
 * @param T_VOID
 * @return T_VOID

 *      NOTE    1. 若使能计数, 则当计算到0后, 还会继续递减(从最大值开始);
 *                         若不使能计数, 则保护为禁止计数时的值;
 *                      2. 若计数值为0, 但使能计数, 此时打开中断, 会引起中断的产生;
 *                         若计数值为0, 但禁止计数, 此时打开中断, 不会引起中断的产生;
 *                      3. a) 读中断状态寄存器(0x20000014), 并不会清除其TIMER BIT的值,
 *                                也不会清控制寄存器(0x20090014等)的BIT30(中断状态);
 *                         b) 读TIMER的控制寄存器(0x20090014等), 会清除中断状态寄存器的TIMER BIT的值,
 *                                同时也清控制寄存器(0x20090014等)的BIT30(中断状态);
 *                                直接将这一位设为0也能达到同样效果.
 *                      4. 进入中断后立即读计数值, 并不为0, 因为计数不会自动停止.
 *                      5. 若使用中断, 则计数值不能太小, 否则中断将占用大部分CPU时间而使程序运行异常.
 *                              (除中断函数外其它函数无法运行);


 */

static T_VOID timer5_interrupt_handler(T_VOID)
{
    extern T_U32 pc_lr;

    /* restart timer5 */
    REG32(PWM_TIMER5_CTRL_REG1) = TIMER5_COUNT;
    REG32(PWM_TIMER5_CTRL_REG2) = TIMER_ENABLE_BIT | TIMER_FEED_BIT | (MODE_ONE_SHOT_TIMER << 24);

    s_tick_count_us += m_rtcCountUs;

    akprintf(C1, M_DRVSYS, "[lr: %x]\n", pc_lr);
}

static T_U32 timer_read_current_count(T_TIMER_ID timer_id)
{
    T_U32 count;
    
    //select read current count mode
    REG32(timer_ctrl_reg2_grp[timer_id]) |= TIMER_READ_SEL_BIT;

    count = REG32(timer_ctrl_reg1_grp[timer_id]);

    //recover read mode
    REG32(timer_ctrl_reg2_grp[timer_id]) &= ~TIMER_READ_SEL_BIT;

    return count;
}
/**
 * @brief Get tick count by ms
 * Function vtimer_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param T_VOID
 * @return T_U32: tick count
 * @retval
 */
T_U32 get_tick_count(T_VOID)
{
    T_U32 tick = 0;
    T_U32 ret = 0;
    T_U64 CurUs = 0;

    if( !m_bRTCStart )
    {
        return ret;
    }

    irq_mask();
    #if TICK_WITH_OS_AND_VTIMEER
    
    tick = timer_read_current_count(uiTIMER4);        
    tick &= TIMER_MAX_COUNT;

    //!!!notice
    //if timer interrupt comes here, need to reverse the tick count
    //otherwise we may get wrong tick
    if(REG32(PWM_TIMER5_CTRL_REG2) & (TIMER_STATUS_BIT))
    {
        tick = 0;
    }

    CurUs = ( TIMER5_COUNT - tick ) / m_clkPerUs + s_tick_count_us;
	#else
    tick = timer_read_current_count(uiTIMER2);        
    tick &= TIMER_MAX_COUNT;

    //!!!notice
    //if timer interrupt comes here, need to reverse the tick count
    //otherwise we may get wrong tick
    if(REG32(PWM_TIMER3_CTRL_REG2) & (TIMER_STATUS_BIT))
    {
        tick = 0;
    }

    CurUs = ( m_hardtimer[uiTIMER2].clock - tick ) / m_clkPerUs + s_tick_count_us_in_vtimer*1000;	
	#endif
    ret = (T_U32)(CurUs / 1000);
    
    irq_unmask();

    return ret;
}

/**
 * @brief Get tick count by us
 * Function vtimer_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @param T_VOID
 * @return T_U64: tick count by us
 * @retval
 */
T_U64 get_tick_count_us(T_VOID)
{
    T_U32 tick = 0;
    T_U64 CurUs = 0;

    if( !m_bRTCStart )
    {
        return CurUs;
    }

    irq_mask();
    #if TICK_WITH_OS_AND_VTIMEER
    tick = timer_read_current_count(uiTIMER4);        
    tick &= TIMER_MAX_COUNT;

    //!!!notice
    //if timer interrupt comes here, need to reverse the tick count
    //otherwise we may get wrong tick
    if(REG32(PWM_TIMER5_CTRL_REG2) & (TIMER_STATUS_BIT))
    {
        tick = 0;
    }
    
    CurUs = ( TIMER5_COUNT - tick ) / m_clkPerUs + s_tick_count_us;
    #else
	tick = timer_read_current_count(uiTIMER2);        
	tick &= TIMER_MAX_COUNT;

	//!!!notice
	//if timer interrupt comes here, need to reverse the tick count
	//otherwise we may get wrong tick
	if(REG32(PWM_TIMER3_CTRL_REG2) & (TIMER_STATUS_BIT))
	{
		tick = 0;
	}

    CurUs = ( m_hardtimer[uiTIMER2].clock - tick ) / m_clkPerUs + s_tick_count_us_in_vtimer*1000;	
   
    #endif
    
    irq_unmask();

    return CurUs;
}

/**
 * @brief: reset timer register to default value
 * @author Liao_Zhijun
 * @date 2010-05-27
 * @return T_VOID
 * @retval
 */
T_VOID timer_reset(T_VOID)
{
    T_U32 i;

    for(i = 0; i < 5; i++)
    {
        REG32(timer_ctrl_reg1_grp[i]) = 0;
        REG32(timer_ctrl_reg2_grp[i]) = 0;
    }
}


T_U32 timer_interval(T_TIMER_ID hardware_timer)
{
    return m_hardtimer[hardware_timer].interval;
}

/**
 * @brief watch dog function init
 * @author liao_zhijun
 * @date 2014-05-28
 * @param T_U16 feedtime:watch dog feed time, feedtime unit:ms, MAX = 171798ms
 * @return T_VOID
  */
T_VOID watchdog_timer_start(T_U32 feed_time)
{
    T_U32 count;

    irq_mask();

    /* start timer4 for watchdog */
    REG32(PWM_TIMER4_CTRL_REG1) = (TIMER_FREQ / 1000) * feed_time;
    REG32(PWM_TIMER4_CTRL_REG2) = TIMER_ENABLE_BIT | TIMER_FEED_BIT | (MODE_ONE_SHOT_TIMER << 24);

    REG32(CPU_TIMER_REG) = (1<<3);

    irq_unmask();
}

/**
 * @brief watch dog feed
 * @author liao_zhijun
 * @date 2014-05-28
 * @return T_VOID
  */
T_VOID watchdog_timer_feed(T_VOID)
{
    irq_mask();
    REG32(PWM_TIMER4_CTRL_REG2) |= TIMER_FEED_BIT;
    irq_unmask();
}

/**
 * @brief watch dog stop
 * @author liao_zhijun
 * @date 2014-05-28
 * @return T_VOID
  */
T_VOID watchdog_timer_stop(T_VOID)
{
    irq_mask();
    REG32(CPU_TIMER_REG) = 0;
    REG32(PWM_TIMER4_CTRL_REG1) = 0;
    REG32(PWM_TIMER4_CTRL_REG2) = 0;
    irq_unmask();
}

#endif//#ifdef OS_ANYKA

/* end of file */

