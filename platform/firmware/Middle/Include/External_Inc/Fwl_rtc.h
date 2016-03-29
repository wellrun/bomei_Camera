/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.    
 *  
 * File Name£ºFwl_rtc.c
 * Function£ºprocess rtc task including alarm,auto-power off/on and calendar.
 *
 * Author£ºWuShanwei
 * Date£º2007-08-08
 * Version£º1.0          
 *
 * Reversion:
 * Author:
 * Date:
**************************************************************************/

#ifndef __FWL_RTC_H__
/**
 * @def __FWL_RTC_H__
 *
 */
#define __FWL_RTC_H__

#ifdef USE_EXTERN_RTC

#define MAX_YEAR 2099       /* max year of edit*/
#define MIN_YEAR 2000       /* min year of edit*/

#else

#define MAX_YEAR 2050       /* max year of edit*/
#define MIN_YEAR 1980       /* min year of edit*/

#endif

#define RTC_POWER_OFF_NEED_TIME 15
/** file path definition for global variables related to rtc*/
#define RTC_GS_PATH _T(DRI_A"rtc_gs")

#define SYSTEM_DEFAULT_YEAR    2008 

typedef enum {
    RTCTYPE_ALARM = 0,
    RTCTYPE_CALENDAR,
    RTCTYPE_ALAANDCAL,
    RTCTYPE_STANDBYCYCDET,
    RTCTYPE_MAX
} T_eRTCType;


typedef struct {
    T_U32        RTCCounterValueOld;        /* RTC counter value before enter stand by    */
    T_eRTCType    RTCAlarmType;            /* alarm type user setting                    */
    T_BOOL        RTCPdEnType ;            /* RTC power down enable type                */

    T_SYSTIME    RTCSysTimeBackup;        /* System time backup                        */
    T_U32        RTCRTCCntBackup;        /* RTC value backup                            */
    T_U32        RTCAlarmCntBackup;        /* RTC alarm value backup                    */

    
} T_FWL_RTC;


typedef T_VOID (*T_fRTC_CALLBACK)(T_VOID);/* Define RTC callback function type */


T_U32   Fwl_RtcExitStandby(T_VOID);

T_BOOL Fwl_RTCGPIOCheck(T_U8 pin);

/**
 * @brief RTC Init
 * @author WuShanwei
 * @date 2007-08-07
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID    Fwl_RTCInit(T_VOID);

/**
 * @ config RTC alarm 
 * @author WuShanwei
 * @date 2007-08-07
 * @param 
     CurrentValue: current rtc time 
     AlarmValue: alarm time 
     PwDnEnable: 1-power down enable 
    T_eRTCType: RTC alarm type: RTCTYPE_ALARM or RTCTYPE_CALENDAR
 * @return 
     0 - OK
 * @retval
 */
T_S8    Fwl_RTCSetAlarm(T_U32 CurrentValue, T_U32 AlarmValue, T_BOOL PwDnEnable, T_eRTCType RTCType);

/**
 * @ Cancel alarm and calendar
 * @author QuChanghong
 * @date 2007-08-07
 * @param  T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID Fwl_RTCCancelAlarm(T_VOID);


/** 
 * @brief save user data(gs variable) related to rtc
 * @author WuShanwei
 * @date 2007-09-01
 * @param[in] T_VOID 
 * @return success or not
 */
T_BOOL RTC_SaveUserData(T_VOID);

T_VOID Fwl_RTCSetCount(T_U32 rtc_val);
T_VOID Fwl_SetAlarmRtcCount(T_U32 count);
T_U32 Fwl_GetAlmRTCCount(T_VOID);
T_U32 Fwl_RTCGetCount(T_VOID);

T_VOID Fwl_rtc_set_powerdownalarm(T_BOOL flag);

T_VOID 	Fwl_SetRtcCallBack(T_fRTC_CALLBACK cb);
#endif