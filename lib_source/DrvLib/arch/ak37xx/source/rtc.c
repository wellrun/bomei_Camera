/** @file rtc.c
 *  @brief rtc module control
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun 
 * @date 2010.05.28
 * @version 1.0
 */
#include "anyka_cpu.h" 
#include "anyka_types.h"
#include "drv_api.h"
#include "interrupt.h"
#include "arch_rtc.h"
#include "l2.h"
#include "dac.h"
#include "standby.h"
#include "drv_module.h"

//set and get bist macro; bs:start bit position, be:end bit positon
#define BIT_MASK(__bf) (((1U << ((be ## __bf) - (bs ## __bf) + 1)) - 1U) << (bs ## __bf)) 
#define SET_BITS(__dst, __bf, __val) \
        ((__dst) = ((__dst) & ~(BIT_MASK(__bf))) | \
        (((__val) << (bs ## __bf)) & (BIT_MASK(__bf)))) 
#define GET_BITS(__src, __bf)   \
        (((__src) & BIT_MASK(__bf)) >> (bs ## __bf))

//RTC_CONFIG_REG bits define
#define bsDataAddr              0
#define beDataAddr              13
#define bsIntRegAddr            14
#define beIntRegAddr            16
#define bsRWmodeAddr            17
#define beRWmodeAddr            21

//RealTime Register1 bits define
#define bsSecAddr              0
#define beSecAddr              5
#define bsMinAddr              6
#define beMinAddr              11

//RealTime Register2 bits define
#define bsHourAddr              0
#define beHourAddr              4
#define bsDayMonthAddr          5
#define beDayMonthAddr          9
#define bsDayWeekAddr           10
#define beDayWeekAddr           12

//RealTime Register3 bits define
#define bsMonthAddr             0
#define beMonthAddr             3
#define bsYearAddr              4
#define beYearAddr              10

//watchdog time configuration bits define
#define bsWDTimeAddr            0
#define beWDTimeAddr            12

//RTC_BACK_DATA_REG bits define
#define bsBackDataAddr          0
#define beBackDataAddr          13

//RTC module includes 6 internal registers defined as follows
#define REALTIME1_REG           0
#define REALTIME2_REG           1
#define REALTIME3_REG           2
#define ALARMTIME1_REG          3
#define ALARMTIME2_REG          4
#define ALARMTIME3_REG          5
#define WDOG_TIMER_REG          6
#define RTC_SETTING_REG         7

#define ALARM_ENABLE            (1<<13)

//rtc setting bits
#define TIMEOUT_ERR_CLR         (1<<13)
#define TIMEOUT_ERR_STAT        (1<<12)
#define TIMEOUT_ERR_EN          (1<<11)

#define WDOG_TIMER_SEL          (1<<10)
#define REAL_TIME_WR            (1<<3)
#define REAL_TIME_RE            (1<<4)

#define WAKEUP_EN               (1<<2)
#define TIMER_STATUS_CLEAR      (1<<1)
#define ALARM_STATUS_CLEAR      (1<<0)

#define MPU_RB_EN               (1<<29)
#define MPU_EAD_NOW             (1<<28)
#define MPU_R_EN                (1<<27)
#define MPU_MIAN_CS             (1<<26)
#define MPU_SUB_CS              (1<<25)
#define MPU_RB_A0               (1<<24)

//RTC 按键唤醒
//#define KEY_PAD_WAKEUP          (1<<30) 
#define KEY_PAD_WAKEUP_EN          (1<<2) 

//RTC Module Command
#define WRITE_MODE              0x14
#define READ_MODE               0x15

#define RTC_ENABLE_WAKEUP_FROM_STANDBY          (0x1<<16)
#define RTC_ENTER_STANDBY_MODE                  (0x1<<13)

//watchdog function bits
#define WD_ENABLE                               (0x1<<13)
#define WD_DISABLE                              (0x0<<13)

/*
    对于AK880x，设置rtc寄存器时rtc ready interrupt不能被mask，否则读不到status
    但如果一直打开rtc ready interrupt又会一直有中断来
    所以设置rtc寄存器时的顺序为：
    1.mask system control interrupt
    2.unmask rtc ready interrupt
    3.set rtc register and check rtc ready status
    4.mask rtc ready interrupt
    5.unmask system control interrupt
*/
#define RTC_ENABLE_RDY_INTR     \
        {store_all_int(); \
        REG32(INT_SYS_MODULE_REG) |= (1<<8); \
        while(!(REG32(INT_SYS_MODULE_REG)|(1<<24)));\
        REG32(RTC_CONFIG_REG) |= (1<<25);}
#define RTC_DISABLE_RDY_INTR     \
        REG32(RTC_CONFIG_REG) &= ~(1<<25); \
        {REG32(INT_SYS_MODULE_REG) &= ~(1<<8); \
        restore_all_int();}
        
        
//clear all wakeup gpio status
/*
#define CLEAR_ALL_WGPIO (REG32(RTC_WAKEUP_GPIO_C_REG1) = 0xffffffff, \
REG32(RTC_WAKEUP_GPIO_C_REG2) = 0xffffffff)
*/

//default wgpio polarity value, must be set according to actual harware
#define DEFAULT_POL             0x00

//default wakeup pin level
#define DEFAULT_WPIN_LEVEL      0

//rtc message, used in message map
#define RTC_MESSAGE             3

#define RTC_PROTECT \
do{ \
    DrvModule_Protect(DRV_MODULE_RTC); \
}while(0)

#define RTC_UNPROTECT \
do{ \
    DrvModule_UnProtect(DRV_MODULE_RTC);\
}while(0)

/*
wakup pin active level. Because rtc_set_wpinLevel() is a new interface,
if user doesn't call it.Driver must set a default value according to actual hardware
*/
static T_BOOL   s_bSetWpinLevel = AK_FALSE;

//rtc call back function
static T_fRTC_CALLBACK m_RTCCallback = AK_NULL;

//rtc gpio status clear control reg
static const T_U32 rtc_gpio_c_control_reg[] = {
    RTC_WAKEUP_GPIO_C_REG1,RTC_WAKEUP_GPIO_C_REG2
};
//rtc gpio enable control reg
static const T_U32 rtc_gpio_e_control_reg[] = {
    RTC_WAKEUP_GPIO_E_REG1,RTC_WAKEUP_GPIO_E_REG2
};


static T_U32    read_rtc_inter_reg(T_U8 reg);
static T_VOID   write_rtc_inter_reg(T_U8 reg, T_U32 value);

static T_BOOL   rtc_interrupt_handler(T_VOID);
static T_VOID   rtc_callback(T_U32 *param, T_U32 len);

static T_SYSTIME CalcSystimeByRTC(T_U32 RealTimeRet1, T_U32 RealTimeRet2, T_U32 RealTimeRet3);
static T_VOID   ConvertSecondsToSysTime(T_U32 seconds, T_SYSTIME *SysTime);
static T_U32    ConvertTimeToSeconds(T_SYSTIME *SysTime);
static T_U32 keypadgpio_wakeup_type(T_VOID);

/**
 * @convert seconds counted from 1980-01-01 00:00:00 to system time
 * @author YiRuoxiang
 * @date 2006-02-16
 * @param  the seconds want to added, which are counted from 1980-01-01 00:00:00
 * @the seconds can't be more than 4102444799 (2099-12-31 23:59:59)
 * @param T_SYSTIME *SysTime: system time structure pointer
 * @return T_U8
 * @retval if system time is more than 2099-12-31 23:59:59, return AK_FALSE
 *          else return AK_TRUE;
 */
static T_VOID ConvertSecondsToSysTime(T_U32 seconds, T_SYSTIME *SysTime)
{
    T_U32 second_intv, minute_intv, hour_intv, dayofw_intv, day_intv;
    T_U32 year_intv, day_total, year_leap;
    T_U8 month_std_day[12]  = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U8 month_leap_day[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U8 i;
    T_U16       nYearDist = 0;

    SysTime->year      = 1980;  //修改时间起点，要修改最大秒数和闰年数
    SysTime->month     = 1;
    SysTime->week      = 1; // 1980.1.1 is Tuesday
    SysTime->day       = 1;
    SysTime->hour      = 0;
    SysTime->minute    = 0;
    SysTime->second    = 0;
    nYearDist              = 3;
 
    second_intv = seconds % 60;
    minute_intv = (seconds % 3600) / 60;
    hour_intv   = (seconds % 86400) / 3600;
    dayofw_intv = (seconds % 604800) / 86400;
    day_total   = seconds / 86400;  //day_total is still exact!!
    year_intv   = day_total / 365;  //难点是year_intv的计算
    year_leap   = (year_intv + nYearDist) / 4;   //算的是肯定会经过的闰年数，如修改时间起点，如1970年改为1980年，括号中加的数为该年离即将来临的闰年的差减1，如1970后的为1972，为2-1=1；1980后的为1984，为4-1=3
    day_intv = day_total - year_leap * 366 - (year_intv - year_leap) * 365;

    if (day_intv > 366)
    {
        year_intv -= 1;
        year_leap -= 1;
        SysTime->year += (T_U16)year_intv;
        if ((SysTime->year % 4) == 0)
        {
            day_intv = day_total - year_leap * 366 - (year_intv - year_leap) * 365;
        }
        else
        {
            day_intv = day_total - year_leap * 366 - (year_intv - year_leap) * 365 - 1;
        }
    }
    else
    {
        SysTime->year += (T_U16)year_intv;
    }
    
    if ((SysTime->second + second_intv) < 60)
    {
        SysTime->second += (T_U8)second_intv;
    }
    else
    {
        SysTime->second = SysTime->second + (T_U8)second_intv - 60;
    }

    if ((SysTime->minute + minute_intv) < 60)
    {
        SysTime->minute += (T_U8)minute_intv;
    }
    else
    {
        SysTime->minute += (T_U8)minute_intv - 60;
    }

    if ((SysTime->hour + hour_intv) < 24)
    {
        SysTime->hour += (T_U8)hour_intv;
    }
    else
    {
        SysTime->hour += (T_U8)hour_intv - 24;
    }
    
    if ((SysTime->week + dayofw_intv) < 7)
    {
        SysTime->week += (T_U8)dayofw_intv;
    }
    else
    {
        SysTime->week += (T_U8)dayofw_intv - 7;
    }

    if ((SysTime->year % 4) == 0)
    {
        for (i = 0; i <= 11; i++)
        {
            if (day_intv >= month_leap_day[i])
            {
                day_intv -= month_leap_day[i];
            }
            else
            {
                SysTime->month += i;
                SysTime->day   += (T_U8)day_intv;
                break;
            }
        }
    }
    else
    {
        for (i = 0; i <= 11; i++)
        {
            if (day_intv >= month_std_day[i])
            {
                day_intv -= month_std_day[i];
            }
            else
            {
                SysTime->month += i;
                SysTime->day   += (T_U8)day_intv;
                break;
            }
        }
    }
    return;
}
/**
 * @convert system time to seconds counted from 1980-01-01 00:00:00
 * @author YiRuoxiang
 * @date 2006-02-17
 * @param T_SYSTIME SysTime: system time structure
 * @return T_U32: seconds counted from 1980-01-01 00:00:00
 */
static T_U32 ConvertTimeToSeconds(T_SYSTIME *SysTime)
{
    T_U16 i;
    T_U16 year;
    T_U8 month;
    T_U8 day;
    T_U8 month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U32 daynum = 0;
    T_U32 baseyear = 0;
    
    baseyear = 1980;

    //check param
    if ((SysTime->year < baseyear) || (SysTime->year > 2099) || (SysTime->month < 1) || (SysTime->month > 12) || \
       (SysTime->day < 1) || (SysTime->day > 31) || (SysTime->hour > 23) || (SysTime->minute > 59) || (SysTime->second > 59))
    {
        akprintf(C3, M_DRVSYS, "it is a wrong systime format\n");
        akprintf(C3, M_DRVSYS, "year = %d, month = %d, day = %d, hour = %d, minute = %d, second = %d\n",
            SysTime->year, SysTime->month, SysTime->day, SysTime->hour, SysTime->minute, SysTime->second);
        return 0;
    }
 
    year  = SysTime->year;
    month = SysTime->month;
    day   = SysTime->day - 1;

    for (i = baseyear; i < year; i++)
    {
        if (!(i % 4 == 0 && i % 100 != 0 || i % 400 == 0))
             daynum += 365;
        else
             daynum += 366;
    }
    for (i = 1; i < month; i++)
    {
        if (i == 2 && (year % 4 == 0 && year % 100 != 0 || year % 400 == 0))
             daynum += 29;
        else
             daynum += month_day[i-1];
    }
    return ((daynum + day) * 86400 + SysTime->hour * 3600 + SysTime->minute * 60 + SysTime->second);
}

/**
 * @brief get system time from rtc
 * @author liao_zhijun
 * @date 2010-05-28
 * @param RealTimeRet1[in]: rtc register1 value
 * @param RealTimeRet2[in]: rtc register2 value
 * @return T_SYSTIME SysTime: system time structure
 */
static T_SYSTIME CalcSystimeByRTC(T_U32 RealTimeRet1, T_U32 RealTimeRet2, T_U32 RealTimeRet3)
{
    T_SYSTIME systime;   

    systime.year =  ((RealTimeRet3 >> 4) & 0x7f) + 1980;
    systime.month = RealTimeRet3 & 0xf;
    systime.day = (RealTimeRet2 >> 5) & 0x1f;
    systime.week = ((RealTimeRet2 >> 10) & 0x7);
    if(systime.week > 0)
    {
        systime.week -= 1;
    }
    
    systime.hour = RealTimeRet2 & 0x1f;
    systime.minute = (RealTimeRet1 >> 6) &0x3f;
    systime.second = RealTimeRet1 & 0x3f;

#ifdef RTC_DEBUG
    akprintf(C3, M_DRVSYS, "CalcSystimeByRTC(): year = %d, month = %d, day = %d, hour = %d, minute = %d \
        second = %d, week = %d\n", systime.year, systime.month, systime.day, \
        systime.hour, systime.minute, systime.second, systime.week);
#endif    

    return systime;
}


/**
 * @brief   init rtc module
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param year [in] current year
 * @return  T_VOID
 */
T_VOID rtc_init(T_U32 year)
{
    T_U32 ret;

    //open rtc module
    REG32(RTC_CONFIG_REG) |= (1<<24);
    //enable wakeup function and clear wake up signal status
    ret = read_rtc_inter_reg(RTC_SETTING_REG);

    //check timeout error and clear
    if(ret | TIMEOUT_ERR_STAT)
    {
        ret &= ~TIMEOUT_ERR_STAT;
        ret |= TIMEOUT_ERR_CLR;
    }

    ret |= TIMEOUT_ERR_EN | WAKEUP_EN | ALARM_STATUS_CLEAR;
    write_rtc_inter_reg(RTC_SETTING_REG, ret);
}


/**
 * @brief   rtc call back function used in drv module
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param param: param passed from message, not used here
 * @param len: param length, not used here
 * @return  T_VOID
 */
T_VOID rtc_callback(T_U32 *param, T_U32 len)
{
    if (AK_NULL != m_RTCCallback)
    {    
        m_RTCCallback();
    }    
}

/**
 * @brief   set rtc event callback handler
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param cb [in]  the callback handler
 * @return  T_VOID
 */
T_VOID rtc_set_callback(T_fRTC_CALLBACK cb)
{
    m_RTCCallback = cb;

    //create task
    DrvModule_Create_Task(DRV_MODULE_RTC);
    DrvModule_Map_Message(DRV_MODULE_RTC, RTC_MESSAGE, rtc_callback);

    INTR_ENABLE_L2(IRQ_MASK_RTC_ALARM_BIT);

    int_register_irq(INT_VECTOR_RTC, rtc_interrupt_handler);   
}

//10ms timeout, but there is register access, timeout value maybe various
#define MAX_WAIT_TIME       (get_cpu_freq()/7/100) 

//check if intern rtc works or not
static T_BOOL test_rtc_inter_reg(T_U8 reg)
{
    T_U32 status;
    T_U32 ConfigValue = 0, timeout = MAX_WAIT_TIME;
    T_BOOL ret=AK_TRUE;
    
    RTC_ENABLE_RDY_INTR   

    //read back  reg
    ConfigValue = REG32(RTC_CONFIG_REG);
    SET_BITS(ConfigValue, RWmodeAddr, READ_MODE);
    SET_BITS(ConfigValue, IntRegAddr, reg);
    REG32(RTC_CONFIG_REG) = ConfigValue;
    do {
        status = REG32(INT_SYS_MODULE_REG);
    }while(!(status & INT_STATUS_RTC_READY_BIT) && timeout--);
        
    RTC_DISABLE_RDY_INTR

    return ret;
}

//read rtc interner register
T_U32 read_rtc_inter_reg(T_U8 reg)
{
    T_U32 status;
    T_U32 ConfigValue = 0, timeout=MAX_WAIT_TIME;

    RTC_ENABLE_RDY_INTR    

    //read back  reg
    ConfigValue = REG32(RTC_CONFIG_REG);
    SET_BITS(ConfigValue, RWmodeAddr, READ_MODE);
    SET_BITS(ConfigValue, IntRegAddr, reg);
    REG32(RTC_CONFIG_REG) = ConfigValue;

    do {
        status = REG32(INT_SYS_MODULE_REG);
    }while(!(status & INT_STATUS_RTC_READY_BIT) && timeout--);
    
    RTC_DISABLE_RDY_INTR
    
    //wait 1/32K s here
    us_delay(312);

    if (!timeout)
        return 0;
    return GET_BITS(REG32(RTC_BACK_DAT_REG), BackDataAddr);
}

//write rtc interner register
T_VOID write_rtc_inter_reg(T_U8 reg, T_U32 value)
{
    T_U32 status;
    T_U32 ConfigValue = 0, timeout=MAX_WAIT_TIME;
        
    RTC_ENABLE_RDY_INTR

    //write setdata to wakeupset_reg
    ConfigValue = REG32(RTC_CONFIG_REG);
    SET_BITS(ConfigValue, DataAddr, value);
    SET_BITS(ConfigValue, RWmodeAddr, WRITE_MODE);
    SET_BITS(ConfigValue, IntRegAddr, reg);
    REG32(RTC_CONFIG_REG) = ConfigValue;
    do {
        status = REG32(INT_SYS_MODULE_REG);
    }while(!(status & INT_STATUS_RTC_READY_BIT) && timeout--);

    RTC_DISABLE_RDY_INTR
    
    return;
}

//rtc callback function, called by IRQ interrupt handler
static T_BOOL   rtc_interrupt_handler(T_VOID)
{       
    T_U32 ret, reg;

    reg = REG32(INT_SYS_MODULE_REG);

    if((reg & INT_STATUS_RTC_READY_BIT))
    {
        REG32(INT_SYS_MODULE_REG) &= ~IRQ_MASK_RTC_READY_BIT;
        //akprintf(C3, M_DRVSYS, "error rtc int!!\n");
    }

    if((reg & INT_STATUS_RTC_ALARM_BIT) == 0)
        return AK_FALSE;
    
    /* the int_status bit ust be cleared here, do not delete it*/
    ret = read_rtc_inter_reg(RTC_SETTING_REG);
    ret |= ALARM_STATUS_CLEAR;
    write_rtc_inter_reg(RTC_SETTING_REG, ret);

    //drv module send message
    DrvModule_Send_Message(DRV_MODULE_RTC, RTC_MESSAGE, AK_NULL);

    return AK_TRUE;
}


/**
 * @brief   enter standby mode
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return  T_VOID
 */
T_VOID rtc_enter_standby(T_VOID)
{
    T_U32 ret,i;
    T_U32 asic_freq, state=0;   //state 1: cpu2x, 2:cpu3x, 0:cpu=asic
    T_U32 lcd_readback_reg;
    if (test_rtc_inter_reg(0))
    {
        
        ret = read_rtc_inter_reg(RTC_SETTING_REG);
        ret |= WAKEUP_EN;
        write_rtc_inter_reg(RTC_SETTING_REG, ret);
    }
    //NOTE:fix chip bug, IR drop problem. to avoid this, enter standby with low clock.
    asic_freq = get_asic_freq();
    if (AK_FALSE == set_asic_freq(ASIC_FREQ_BY_DIV16))
    {
        if (AK_TRUE == is_cpu_2x())
        {
            set_cpu_2x_asic(AK_FALSE);
            state = 1;
        }
        else if (AK_TRUE == is_cpu_3x())
        {
            set_cpu_3x_asic(MAIN_CLK, AK_FALSE);
            state = 2;
        }
        set_asic_freq(ASIC_FREQ_BY_DIV16);  //decrease asic clock again
    }

    REG32(CLOCK_DIV_REG) |= RTC_ENABLE_WAKEUP_FROM_STANDBY;  //enable RTC WAKEUP
    l2_specific_exebuf(enter_standby, 0);

    //disable voice wakeup
    REG32(ANALOG_CTRL_REG4) |= (1 << 25);
    REG32(ANALOG_CTRL_REG4) |= (1 << 26);      //power off
    REG32(USB_DETECT_CTRL_REG) &= ~(1 << 5);   //disable voice wakeup

    //Pull down VCM2 and set VCM2 at he dischareging state
    REG32(ANALOG_CTRL_REG3) |= (0x1FU << PTM_DCHG) | PL_VCM2;

    //restore clock setting
    set_asic_freq(asic_freq);

    if (state == 1)
        set_cpu_2x_asic(AK_TRUE);
    else if (state == 2)
        set_cpu_3x_asic(MAIN_CLK, AK_TRUE);
}

/**
 * @brief   exit standby mode and return the reason
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return  T_U32 the reason of exitting standby
 * @retval  low 8-bit is wakeup type, refer to T_WU_TYPE
 * @retval  upper 8-bit stands for gpio number if WGPIO wakeup
 */
 T_U16 rtc_exit_standby(T_VOID)
{
    T_U32  reason;
    
    //usb wakeup
    if(REG32(USB_DETECT_STATUS_REG) & (1<<0))
    {
        REG32(USB_DETECT_CTRL_REG) |= (1<<2);
        akprintf(C2, M_DRVSYS, "wakeup by usb vbus\n");
        return WU_USB;
    }
    //voice wakeup
    if (REG32(USB_DETECT_STATUS_REG) & (1<<1))
    {
        REG32(USB_DETECT_CTRL_REG) |= (1<<3);
        REG32(ANALOG_CTRL_REG3) |= (1 << 3);//poweroff vcm3
        akprintf(C2, M_DRVSYS, "wakeup by voice\n");
        return WU_VOICE;
    }

    reason = keypadgpio_wakeup_type();
    
    if (0 == reason)
    {
        akprintf(C2, M_DRVSYS, "wakeup by alarm\n");
        return WU_ALARM;
    }
    if (86 == reason)
    {
        REG32(ANALOG_CTRL_REG1) &= ~KEY_PAD_WAKEUP_EN;  //disable ain keypad input wakeup 
        akprintf(C2, M_DRVSYS, "wakeup by ain\n");
        return  (WU_AIN);
    }
    else
    {
        akprintf(C2, M_DRVSYS, "wakeup by wgpio, gpio is %d \n", reason);
        return (WU_GPIO|(reason<<8));
    }
}
//获取唤醒类型。GPIO或者AIN唤醒方式
static T_U32 keypadgpio_wakeup_type(T_VOID)
{
    T_U32 reason = 0;
    T_U32 gpio_reg1 = 0;
    T_U32 gpio_reg2 = 0;
    T_U32 pin = 0;
    
    gpio_reg1 = REG32(RTC_WAKEUP_GPIO_S_REG1) & REG32(RTC_WAKEUP_GPIO_E_REG1);
    gpio_reg2 = REG32(RTC_WAKEUP_GPIO_S_REG2) & REG32(RTC_WAKEUP_GPIO_E_REG2);

    if  (0 != gpio_reg1)
    {
        reason = gpio_reg1;
    }
    else if(0 != gpio_reg2)
    {
        reason = gpio_reg2;
    }
    else if (( gpio_reg1 | gpio_reg2) == 0)
    {
        return 0;
    }

    pin = gpio_get_wakeup_pin(reason);
    
    REG32(RTC_WAKEUP_GPIO_C_REG1) = 0xffffffff;
    REG32(RTC_WAKEUP_GPIO_C_REG1) = 0;
    REG32(RTC_WAKEUP_GPIO_C_REG2) = 0xffffffff;
    REG32(RTC_WAKEUP_GPIO_C_REG2) = 0;
    return pin;
}
/**
 * @brief   set wakeup gpio of standby mode
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param wgpio_mask [in]  the wakeup gpio value
 * @return      T_VOID
 */
T_VOID rtc_set_wgpio(T_U32 wgpio_mask, T_U32 ctreg)
{

    if (wgpio_mask > 31)
        return;
        
    if ((1 == ctreg) && (30 == wgpio_mask))
    {
        REG32(ANALOG_CTRL_REG1) |= KEY_PAD_WAKEUP_EN;  //打开按键唤醒功能；
    }
    //enable RTC wakeup function and clear wakupgpio status.
    REG32(rtc_gpio_c_control_reg[ctreg]) |= (1<<wgpio_mask);
    REG32(rtc_gpio_c_control_reg[ctreg]) &= ~(1<<wgpio_mask);

    REG32(rtc_gpio_e_control_reg[ctreg]) |= (1<<wgpio_mask);
}

/**
 * @brief   set adc wakeup cfg of standby mode
 *
 * @author liu_huadong
 * @date 2011-05-19
 * @param wgpio_mask [in]  the wakeup gpio value
 * @return      T_VOID
 */

T_VOID ADC_CfgVoiceW(T_U8 freq, T_U8 ref, T_U8 time)
{

    REG32(ANALOG_CTRL_REG4) &= ~(((T_U32)0x3) << 30);    
    REG32(ANALOG_CTRL_REG4) &= ~(0x7 << 27);   
    REG32(ANALOG_CTRL_REG4) &= ~(0x3 << 12);//25   37
    REG32(ANALOG_CTRL_REG4) |= ((freq & 0x3) << 30);    
    REG32(ANALOG_CTRL_REG4) |= ((ref & 0x7) << 27);    
    REG32(ANALOG_CTRL_REG4) |= ((time & 0x3) << 12);

}

/**
 * @brief  open or close voice wakeup function
 *
 * @author liao_Zhijun
 * @date 2011-02-28
 * @return  T_VOID
 */
static T_VOID voice_wakeup(T_BOOL enable)
{

    //REG32(ANALOG_CTRL_REG4) |= (1 << 24);    //poweroff mic_n   
    REG32(ANALOG_CTRL_REG4) |= (1 << 23);    //poweroff mic_p    
    REG32(ANALOG_CTRL_REG4) &= ~(0x7 << 2);  // set no input to adc2
    REG32(ANALOG_CTRL_REG3) &= ~(0x7 << 12); // set hp mute       
    
    //power off voice wakeup before    
    REG32(ANALOG_CTRL_REG4) |= (1 << 26);    //power down voice wake up    
    REG32(ANALOG_CTRL_REG3) |= (0x4 << 12);  // set hp from mic    
    REG32(ANALOG_CTRL_REG3) |= (0x1 << 17);  // HP with 3k to gnd    
    
    mini_delay(5);
    
    REG32(ANALOG_CTRL_REG3) &= ~(0x1 << 2);  // no pl_vcm2 with 2k to gnd    
    REG32(ANALOG_CTRL_REG4) &= ~(0x1<<0);    // no Pl_vcm3 with 2k to gnd   
    REG32(ANALOG_CTRL_REG3) &= ~(0x1F << 4); //disable descharge for VCM2    
    REG32(ANALOG_CTRL_REG3) &= ~(1 << 0);    // power on bias   
    REG32(ANALOG_CTRL_REG3) &= ~(1 << 1);    // power on vcm2  
    REG32(ANALOG_CTRL_REG3) &= ~(1 << 23);   // vcm3 from refrec1.5    
    REG32(ANALOG_CTRL_REG3) &= ~(1 << 3);    // power on  vcm3            
    REG32(USB_DETECT_CTRL_REG) &= ~(1 << 5);    //disable voice wakeup interrupt    
    REG32(USB_DETECT_CTRL_REG) |= (1 << 3);     //cle ar voice wakeup interrupt status    
    REG32(USB_DETECT_CTRL_REG) &= ~(1 << 3);     
    
    mini_delay(500);    
    
    REG32(ANALOG_CTRL_REG3) |= (1 << 1);     // power off vcm2    
    REG32(ANALOG_CTRL_REG3) |= (1 << 3);     // power off vcm3    
    REG32(ANALOG_CTRL_REG3) |= (1 << 0);     // power off bias           
    REG32(ANALOG_CTRL_REG3) &= ~(0x7 << 12); // set hp mute    
    REG32(ANALOG_CTRL_REG3) &= ~(0x1 << 17); //no HP with 3k to gnd            
    REG32(USB_DETECT_CTRL_REG) &= ~(1 << 1);    //rising    
    
    ADC_CfgVoiceW(0x2, 0x5, 0x3);             
    
    REG32(USB_DETECT_CTRL_REG) |= (1 << 5);      //enable voice wakeup interrupt    
    REG32(ANALOG_CTRL_REG4) &= ~(1 << 25);
    REG32(ANALOG_CTRL_REG4) &= ~(1 << 26);    //power on pd_vw, vcm3 from avcc    15

    mini_delay(10);
}


/**
 * @brief   usb vbus wakeup control.
 *
 * @author Huang Xin
 * @date 2010-12-16
 * @param enable [IN] AK_TRUE enable wakeup by usb vbus; AK_FALSE disable wakeup by usb vbus
 * @return  T_VOID
 */
static T_VOID usb_vbus_wakeup(T_BOOL enable)
{
    //open usb detect
    if(enable)
    {
		if ((REG32(MUL_FUN_CTL_REG3) >> 31) & 0x01)
		{
	        REG32(USB_DETECT_CTRL_REG) |= (1 << 0);    //falling-trig 
		}
		else
		{
	        REG32(USB_DETECT_CTRL_REG) &= ~(1 << 0);    //rising-trig 
		}
		

        REG32(USB_DETECT_CTRL_REG) |= (1 << 2);     //clear usb det
        REG32(USB_DETECT_CTRL_REG) &= ~(1 << 2);

        REG32(USB_DETECT_CTRL_REG) |= (1 << 4);     //enable usb detect
    }
    else
    {
        REG32(USB_DETECT_CTRL_REG) |= (1 << 2);     //clear usb det
        REG32(USB_DETECT_CTRL_REG) &= ~(1 << 2);

        REG32(USB_DETECT_CTRL_REG) &= ~(1 << 4);    //disable usb detect
    }
}

/**
 * @brief  set wakeup type for exiting standby mode
 *
 * @author xuchang
 * @param type [in] wakeup type, WU_GPIO and WU_ALARM default opened
 * @return T_VOID
 */
T_VOID rtc_set_wakeup_type(T_WU_TYPE type)
{
    if (type & WU_USB)  //usb vbus wakeup
        usb_vbus_wakeup(AK_TRUE);     
    else
        usb_vbus_wakeup(AK_FALSE);     
        
    if (type & WU_VOICE) //voice wakeup
        voice_wakeup(AK_TRUE);    
}

/**
 * @brief   set wakeuppin active leval
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param wpinLevel [in]  the wakeup signal active level 1:low active,0:high active
 * @return      T_VOID
 */
T_VOID  rtc_set_wpinLevel(T_BOOL wpinLevel)
{
    T_U32 ret = 0;
    T_U32 bit;

    //init rtc
    REG32(RTC_CONFIG_REG) |= (1<<24);
    //check rtc works or not
    if (!test_rtc_inter_reg(0))
        return;
        
    if(wpinLevel) bit = 8;
    else bit = 9;

    ret = read_rtc_inter_reg(RTC_SETTING_REG);
    ret |= (1<<bit);
    write_rtc_inter_reg(RTC_SETTING_REG, ret);

    do
    {
        ret = read_rtc_inter_reg(RTC_SETTING_REG);
    }
    while(ret & (1<<bit));

    return;
}

/**
 * @brief set rtc register value by system time
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param systime T_SYSTIME : system time structure
 * @return T_U32 day num
 */
T_VOID rtc_set_RTCbySystime(T_SYSTIME *systime)
{
    T_U32 RealTimeData1 = 0;
    T_U32 RealTimeData2 = 0;
    T_U32 RealTimeData3 = 0;
    T_U32 rtc_setting = 0;
    
    if ((systime->year < 1980) || (systime->year > 2099) || (systime->month < 1) || (systime->month > 12) || \
       (systime->day < 1) || (systime->day > 31) || (systime->hour > 23) || (systime->minute > 59) \
       || (systime->second > 59) || (systime->week > 6))
    {
        akprintf(C3, M_DRVSYS, "year = %d, month = %d, day = %d, hour = %d, minute = %d, second = %d, week = %d\n", \
            systime->year, systime->month, systime->day, systime->hour, systime->minute, systime->second, systime->week);
        akprintf(C3, M_DRVSYS, "it is a wrong systime format, set system time fail\n");

        return;
    }

    RTC_PROTECT;

    //check rtc works or not
    if (!test_rtc_inter_reg(0))
    {
        RTC_UNPROTECT;
        return;
    }
    
    //config Real Time Reg1
    SET_BITS(RealTimeData1, SecAddr, systime->second);
    SET_BITS(RealTimeData1, MinAddr, systime->minute);

    //config Real Time Reg2
    SET_BITS(RealTimeData2, HourAddr, systime->hour);
    SET_BITS(RealTimeData2, DayMonthAddr, systime->day);
    SET_BITS(RealTimeData2, DayWeekAddr, (systime->week+1));

    //config Real Time Reg3
    SET_BITS(RealTimeData3, MonthAddr, systime->month);
    SET_BITS(RealTimeData3, YearAddr, (systime->year-1980));

    //write RealTime register
    write_rtc_inter_reg(REALTIME1_REG, RealTimeData1);
    write_rtc_inter_reg(REALTIME2_REG, RealTimeData2);
    write_rtc_inter_reg(REALTIME3_REG, RealTimeData3);

    rtc_setting = read_rtc_inter_reg(RTC_SETTING_REG);
    rtc_setting |= REAL_TIME_WR;
    write_rtc_inter_reg(RTC_SETTING_REG, rtc_setting);

    do
    {
        rtc_setting = read_rtc_inter_reg(RTC_SETTING_REG);
    }
    while(rtc_setting & REAL_TIME_WR);

    RTC_UNPROTECT;
}


T_VOID rtc_set_RTCcount(T_U32 rtc_value)
{
    T_SYSTIME systime;

    memset(&systime, 0x00, sizeof(systime));
    ConvertSecondsToSysTime(rtc_value, &systime);

#ifdef RTC_DEBUG    
    akprintf(C3, M_DRVSYS, "rtc_set_RTCcount(): year = %d, month = %d, day = %d, hour = %d, minute = %d \
        second = %d, week = %d\n", systime.year, systime.month, systime.day, \
        systime.hour, systime.minute, systime.second, systime.week);
#endif

    rtc_set_RTCbySystime(&systime);

    return ;
}


/**
 * @brief set alarm by system time
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param systime system time structure
 * @return T_U32: day num
 */
T_VOID rtc_set_AlarmBySystime(T_SYSTIME *systime)
{
    T_U32 RealTimeData1 = 0;
    T_U32 RealTimeData2 = 0;
    T_U32 RealTimeData3 = 0;

    if ((systime->year < 1980) || (systime->year > 2099) || (systime->month < 1) || (systime->month > 12) || \
       (systime->day < 1) || (systime->day > 31) || (systime->hour > 23) || (systime->minute > 59) \
       || (systime->second > 59) || (systime->week > 6))
    {
        akprintf(C3, M_DRVSYS, "year = %d, month = %d, day = %d, hour = %d, minute = %d, second = %d, week = %d\n", \
            systime->year, systime->month, systime->day, systime->hour, systime->minute, systime->second, systime->week);
        akprintf(C3, M_DRVSYS, "it is a wrong systime format, set system time fail\n");

        return;
    }

    RTC_PROTECT;

    //config Alarm Time Reg1
    SET_BITS(RealTimeData1, SecAddr, systime->second);
    SET_BITS(RealTimeData1, MinAddr, systime->minute);
    RealTimeData1 |= ALARM_ENABLE;

    //config Alarm Time Reg2
    SET_BITS(RealTimeData2, HourAddr, systime->hour);
    SET_BITS(RealTimeData2, DayMonthAddr, systime->day);
    RealTimeData2 |= ALARM_ENABLE;

    //config Alarm Time Reg3
    SET_BITS(RealTimeData3, MonthAddr, systime->month);
    SET_BITS(RealTimeData3, YearAddr, systime->year - 1980);
    RealTimeData3 |= ALARM_ENABLE;

    //write Alarm Time register
    write_rtc_inter_reg(ALARMTIME1_REG, RealTimeData1);
    write_rtc_inter_reg(ALARMTIME2_REG, RealTimeData2);
    write_rtc_inter_reg(ALARMTIME3_REG, RealTimeData3);

    RTC_UNPROTECT;
}


/**
 * @brief   set rtc alarm count.
 *
 * when the rtc count reaches to the alarm  count, 
 * AK chip is woken up if in standby mode and rtc interrupt happens.
 * @author liao_zhijun
 * @date 2010-04-29
 * @param rtc_wakeup_value [in] alarm count in seconds
 * @return T_VOID
 */
 T_VOID rtc_set_alarmcount(T_U32 rtc_wakeup_value)
{
    T_SYSTIME systime;

    memset(&systime, 0x00, sizeof(systime));
    ConvertSecondsToSysTime(rtc_wakeup_value, &systime);

#ifdef RTC_DEBUG    
    akprintf(C3, M_DRVSYS, "rtc_set_alarmcount(): year = %d, month = %d, day = %d, hour = %d, minute = %d \
        second = %d, week = %d\n", systime.year, systime.month, systime.day, \
        systime.hour, systime.minute, systime.second, systime.week);
#endif

    rtc_set_AlarmBySystime(&systime);

    return ;
}

/**
 * @brief enable or disable power down alarm
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param alarm_on [in]  enable power down alarm or not
 * @return T_VOID
 */
T_VOID rtc_set_powerdownalarm(T_BOOL alarm_on)
{
    /* four standard steps(if alarm_on)
            a. to write current time and alarm time
            b. to clear alarm status bit
            c. to enable the alarm function
            d.power down
            in this version, 'a' can be omitted
            note: should check corresponding status bits to confirm the 
                    operation has been implemented successfully
    */
    T_U32 setdata = 0;

    RTC_PROTECT;

    //read back  wakeupset_reg
    setdata = read_rtc_inter_reg(RTC_SETTING_REG);

    if (AK_TRUE == alarm_on)
    {
        //clear alarm status, enable alarm function, set wakeuppin active gpio
        setdata |= WAKEUP_EN;
    }
    else
    {
        //disable the alarm function
        setdata &= ~WAKEUP_EN;               
    }

    //write setdata to wakeupset_reg
    write_rtc_inter_reg(RTC_SETTING_REG, setdata);

    RTC_UNPROTECT;
    return;
}

/**
 * @brief get system time from rtc
 * @author liao_zhijun
 * @date 2010-05-28
 * @param 
 * @return T_SYSTIME SysTime: system time structure
 */
T_SYSTIME rtc_get_RTCsystime(T_VOID)
{
    T_U32 RealTimeRet1, RealTimeRet2, RealTimeRet3;
    T_SYSTIME systime;
    T_U32 rtc_setting;

    RTC_PROTECT;
    
    rtc_setting = read_rtc_inter_reg(RTC_SETTING_REG);
    rtc_setting |= REAL_TIME_RE;
    write_rtc_inter_reg(RTC_SETTING_REG, rtc_setting);

    //read RealTime register 
    RealTimeRet1 = read_rtc_inter_reg(REALTIME1_REG);
    RealTimeRet2 = read_rtc_inter_reg(REALTIME2_REG);
    RealTimeRet3 = read_rtc_inter_reg(REALTIME3_REG);

    systime = CalcSystimeByRTC(RealTimeRet1, RealTimeRet2, RealTimeRet3);

    RTC_UNPROTECT;

    return systime;
}

/**
 * @brief   get rtc passed count in seconds
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_U32 the rtc count
 */
T_U32 rtc_get_RTCcount( T_VOID )
{
    T_U32 rtc_count = 0;
    T_SYSTIME systime;

    systime = rtc_get_RTCsystime();
    rtc_count = ConvertTimeToSeconds(&systime);
    
    return rtc_count;
}

/**
 * @brief get system time from rtc
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param T_VOID
 * @return T_SYSTIME system time structure
 */
T_SYSTIME rtc_get_AlarmSystime(T_VOID)
{
    T_U32 TimeRet1, TimeRet2, TimeRet3;
    T_SYSTIME systime;

    RTC_PROTECT;

    //read RealTime register 1
    TimeRet1 = read_rtc_inter_reg(ALARMTIME1_REG);
    TimeRet2 = read_rtc_inter_reg(ALARMTIME2_REG);
    TimeRet3 = read_rtc_inter_reg(ALARMTIME3_REG);
        
    systime = CalcSystimeByRTC(TimeRet1, TimeRet2, TimeRet3);

    RTC_UNPROTECT;

    return systime;
}

/**
 * @brief get alarm count that has been set.
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_U32
 * @retval the alarm count in seconds
 */
T_U32 rtc_get_alarmcount(T_VOID)
{
    T_U32 rtc_count = 0;
    T_SYSTIME systime;

    systime = rtc_get_AlarmSystime();
    rtc_count = ConvertTimeToSeconds(&systime);
    
    return rtc_count;
}

/**
 * @brief query alarm status
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_BOOL
 * @retval AK_TRUE alarm has occured
 * @retval AK_FALSE alarm hasn't occured
 */
T_BOOL rtc_get_alarm_status()
{
    return AK_FALSE;
}

#if 0
/**
 * @brief watch dog function init
 * @author liao_zhijun
 * @date 2010-05-28
 * @param T_U16 feedtime:watch dog feed time, feedtime unit:ms, wd_pulse <= feedtime
 * @param T_U8 rst_level:reset level for WAKEUP pin after watchdog feedtime expired
 * @return T_VOID
  */
T_VOID watchdog_init(T_U16 feedtime, T_U8 rst_level)
{
    T_U32 ret = 0;

    if (feedtime >8000)
        feedtime = 8000;    //max 8s
    feedtime = feedtime * 1024 / 1000 - 1;

    if (feedtime == 0) return;

    RTC_PROTECT;

    //select watch dog
    ret = read_rtc_inter_reg(RTC_SETTING_REG);
    ret |= (WDOG_TIMER_SEL) ;
    ret |= (1<<5)|(1<<2);
    write_rtc_inter_reg(RTC_SETTING_REG, ret);
   
    //set WAKEUP pin level
    rtc_set_wpinLevel(1-rst_level);    
        
    //set watchdog feetime disalbe watchdog
    ret = WD_DISABLE;
    SET_BITS(ret, WDTimeAddr, feedtime);
    write_rtc_inter_reg(WDOG_TIMER_REG, ret);

    RTC_UNPROTECT;

    return;
}

/**
 * @brief watch dog function start
 * @author liao_zhijun
 * @date 2010-05-28
 * @return T_VOID
  */
T_VOID watchdog_start(T_VOID)
{
    T_U32 ret = 0;

    RTC_PROTECT;

    ret = read_rtc_inter_reg(WDOG_TIMER_REG);
    ret |= WD_ENABLE;
    write_rtc_inter_reg(WDOG_TIMER_REG, ret);

    RTC_UNPROTECT;

    return;
}

/**
 * @brief watch dog function stop
 * @author liao_zhijun
 * @date 2010-05-28
 * @return T_VOID
  */
T_VOID watchdog_stop(T_VOID)
{
    T_U32 ret = 0;

    RTC_PROTECT;

    ret = read_rtc_inter_reg(WDOG_TIMER_REG);
    ret &= ~WD_ENABLE;
    write_rtc_inter_reg(WDOG_TIMER_REG, ret);

    RTC_UNPROTECT;

    return;
}

/**
 * @brief feed watch dog
 * @author liao_zhijun
 * @date 2010-05-28
 * @return T_VOID
 * @note this function must be called periodically, 
    otherwise watchdog will expired and reset.
  */
T_VOID watchdog_feed(T_VOID)
{
    T_U32 ret = 0;

    RTC_PROTECT;

    ret = read_rtc_inter_reg(RTC_SETTING_REG);
    ret |= (1<<6);
    write_rtc_inter_reg(RTC_SETTING_REG, ret);

    RTC_UNPROTECT;

    return;
}

#endif
