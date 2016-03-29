#ifdef OS_WIN32

#include "anyka_types.h"
#include "arch_rtc.h"
#include "time.h"
#include <windows.h>

/** 
 * @file arch_rtc.h
 * @brief rtc module control
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2007.9.7
 * @version 1.0
 */

/** @defgroup RTC RTC group
 *	@ingroup Drv_Lib
 */
/*@{*/


/**
 * @brief   init rtc module
 *
 * @author
 * @date
 * @return  T_VOID
 */
T_VOID 	rtc_init(T_U32 year)
{

}

/**
 * @brief   set rtc event callback handler
 *
 * @author
 * @date
 * @param[in]   cb the callback handler
 * @return  T_VOID
 */
T_VOID 	rtc_set_callback(T_fRTC_CALLBACK cb)
{

}


/**
 * @brief   enter standby mode
 *
 * @author
 * @date
 * @return  T_VOID
 */
T_VOID  rtc_enter_standby(T_VOID)
{

}

/**
 * @brief   exit standby mode and return the reason
 *
 * @author
 * @date
 * @return  T_U32
 * @retval  the reason of exitting standby
 */
T_U16   rtc_exit_standby(T_VOID)
{
    return 0;
}

/**
 * @brief   set wakeup gpio of standby mode
 *
 * @author
 * @date
 * @param[in]   wgpio_mask the wakeup gpio value
 * @return      T_VOID
 */
T_VOID 	rtc_set_wgpio(T_U32 wgpio_mask)
{

}

/**
 * @brief   set rtc start count value in seconds
 *
 * @author
 * @date
 * @param[in]   rtc_value the rtc count to be set
 * @return      T_VOID
 */
T_VOID rtc_set_RTCcount(T_U32 rtc_value)
{

}

/**
 * @brief   get rtc passed count in seconds
 *
 * @author
 * @date
 * @return T_U32
 * @retval the rtc count
 */
T_U32 rtc_get_RTCcount(T_VOID)
{
    return time(NULL) - 0x12f71400; // adjust beginning time
}

/**
 * @brief   set rtc alarm count.
 *
 * when the rtc count reaches to the alarm  count, 
 * AK chip is woken up if in standby mode and rtc interrupt happens.
 * @author
 * @date
 * @param[in] rtc_wakeup_value alarm count in seconds
 * @return T_VOID
 */
T_VOID  rtc_set_alarmcount(T_U32 rtc_wakeup_value)
{

}

/**
 * @brief get alarm count that has been set.
 * @author
 * @date
 * @return T_U32
 * @retval the alarm count in seconds
 */
T_U32   rtc_get_alarmcount(T_VOID)
{
    return 0;
}

/**
 * @brief enable or disable power down alarm
 * @author
 * @date
 * @param[in] alarm_on enable power down alarm or not
 * @return T_VOID
 */
T_VOID  rtc_set_powerdownalarm(T_BOOL alarm_on)
{

}

/*@}*/
#endif

