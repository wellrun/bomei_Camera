/** 
 * @file arch_rtc.h
 * @brief rtc module control
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun
 * @date 2010.04.29
 * @version 1.0
 */
#ifndef         __ARCH_RTC_H__
#define         __ARCH_RTC_H__

/** @defgroup RTC RTC group
 *  @ingroup Drv_Lib
 */
/*@{*/

/**
 * @brief system time struction
 
 *   define system time
 */
typedef struct tagSYSTIME{
    T_U16   year;               ///< 4 byte: 1980-2099
    T_U8    month;              ///< 1-12 
    T_U8    day;                ///< 1-31 
    T_U8    hour;               ///< 0-23 
    T_U8    minute;             ///< 0-59 
    T_U8    second;             ///< 0-59 
    T_U16   milli_second;       ///< 0-999 
    T_U8    week;               ///< 0-6,  0: monday, 6: sunday
} T_SYSTIME, *T_pSYSTIME;

typedef enum {
    WU_GPIO  = (1<<0),
    WU_ALARM = (1<<1),
    WU_USB   = (1<<2),
    WU_VOICE = (1<<3)
} T_WU_TYPE;

/**
 *  rtc event callback handler
 */
typedef T_VOID (*T_fRTC_CALLBACK)(T_VOID);

/**
 * @brief   init rtc module
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param year [in] current year
 * @return  T_VOID
 */
T_VOID  rtc_init(T_U32 year);

/**
 * @brief   set rtc alarm event callback handler
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param cb [in]  the alarm event callback handler
 * @return  T_VOID
 */
T_VOID  rtc_set_callback(T_fRTC_CALLBACK cb);

/**
 * @brief   enter standby mode
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return  T_VOID
 */
T_VOID  rtc_enter_standby(T_VOID);

/**
 * @brief   exit standby mode and return the reason
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return  T_U32 the reason of exitting standby
 * @retval  low 8-bit is wakeup type, refer to T_WU_TYPE
 * @retval  upper 8-bit stands for gpio number if WGPIO wakeup
 */
T_U16   rtc_exit_standby(T_VOID);

/**
 * @brief  set wakeup type for exiting standby mode
 *
 * @author xuchang
 * @param type [in] wakeup type, WU_GPIO and WU_ALARM default opened
 * @return T_VOID
 */
T_VOID rtc_set_wakeup_type(T_WU_TYPE type);

/**
 * @brief   set wakeup gpio of standby mode
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param wgpio_mask [in]  the wakeup gpio value
 * @return      T_VOID
 */
T_VOID  rtc_set_wgpio(T_U32 wgpio_mask);

/**
 * @brief   set wakeuppin active leval
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param wpinLevel [in]  the wakeup signal active level 1:low active,0:high active
 * @return      T_VOID
 */
T_VOID  rtc_set_wpinLevel(T_BOOL wpinLevel);

/**
 * @brief   set rtc start count value in seconds
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param rtc_value [in] the rtc count to be set
 * @return      T_VOID
 */
T_VOID rtc_set_RTCcount(T_U32 rtc_value);

/**
 * @brief set rtc register value by system time
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param systime T_SYSTIME : system time structure
 * @return T_U32 day num
 */
T_VOID rtc_set_RTCbySystime(T_SYSTIME *systime);


/**
 * @brief   get rtc passed count in seconds
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_U32 the rtc count
 */
T_U32 rtc_get_RTCcount(T_VOID);

/**
 * @brief get current system time from rtc
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return system time structure
 */
T_SYSTIME rtc_get_RTCsystime(T_VOID);


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
T_VOID  rtc_set_alarmcount(T_U32 rtc_wakeup_value);

/**
 * @brief set alarm by system time
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param systime system time structure
 * @return T_U32: day num
 */
T_VOID rtc_set_AlarmBySystime(T_SYSTIME *systime);

/**
 * @brief get alarm count that has been set.
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_U32
 * @retval the alarm count in seconds
 */
T_U32   rtc_get_alarmcount(T_VOID);

/**
 * @brief get alarm system time from rtc
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_SYSTIME system time structure
 */
T_SYSTIME rtc_get_AlarmSystime(T_VOID);

/**
 * @brief enable or disable power down alarm
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param alarm_on [in]  enable power down alarm or not
 * @return T_VOID
 */
T_VOID  rtc_set_powerdownalarm(T_BOOL alarm_on);

/**
 * @brief query alarm status
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_BOOL
 * @retval AK_TRUE alarm has occured
 * @retval AK_FALSE alarm hasn't occured
 */
T_BOOL rtc_get_alarm_status(T_VOID);

/**
 * @brief watch dog function init
 * @author liao_zhijun
 * @date 2010-05-28
 * @param[in] feedtime T_U16 feedtime:watch dog feed time, feedtime unit:ms
 * @param[in] rst_level T_U8 rst_level:reset level for WAKEUP pin after watchdog feedtime expired
 * @return T_VOID
  */
T_VOID watchdog_init(T_U16 feedtime, T_U8 rst_level);

/**
 * @brief watch dog function start
 * @author liao_zhijun
 * @date 2010-05-28
 * @return T_VOID
  */
T_VOID watchdog_start(T_VOID);

/**
 * @brief watch dog function stop
 * @author liao_zhijun
 * @date 2010-05-28
 * @return T_VOID
  */
T_VOID watchdog_stop(T_VOID);

/**
 * @brief feed watch dog
 * @author liao_zhijun
 * @date 2010-05-28
 * @return T_VOID
 * @note this function must be called periodically, 
    otherwise watchdog will expired and reset.
  */
T_VOID watchdog_feed(T_VOID);

/*@}*/
#endif//__ARCH_RTC_H__

