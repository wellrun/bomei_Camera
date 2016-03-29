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


#include <stdio.h>

#include "Fwl_public.h"
#include "Lib_event.h"
#include "Fwl_rtc.h"
#include "Eng_Time.h"
#include "gpio_config.h"
#include "drv_api.h"
#include "fwl_pfdisplay.h"
#include "fwl_display.h"
#include "Fwl_sys_detect.h"
#include "lib_state.h"
#include "lib_state_api.h"

T_FWL_RTC    rtcFwlData;


T_BOOL Fwl_RTCGPIOCheck(T_U8 pin)
{
#ifdef OS_ANYKA
#ifdef USE_EXTERN_RTC
    return external_rtc_gpio_check(pin);
#else
    return AK_FALSE; //for internal RTC, don't use gpio to wake up standby
#endif
#endif

#ifdef OS_WIN32
    return AK_FALSE;
#endif
}



T_U32   Fwl_RtcExitStandby(T_VOID)
{
    T_U32  nReason = 0;
	
#ifdef  OS_ANYKA
    nReason = rtc_exit_standby();

	Fwl_Print(C3, M_RTC, "Fwl_RtcExitStandby:");

	switch(nReason&0xf)
	{
		case WU_GPIO:
			Fwl_Print(C3, M_RTC, "gpio wakeup, gpio pin=%d\n", nReason>>8);
			break;

		case WU_ALARM:
			Fwl_Print(C3, M_RTC, "alarm wakeup\n");
			break;

		case WU_USB:
			Fwl_Print(C3, M_RTC, "usb wakeup\n");
			break;

		case WU_VOICE:
			Fwl_Print(C3, M_RTC, "voice wakeup\n");
			break; 		
	}

#ifdef USE_EXTERN_RTC
    if ((nReason == 0x4000) && Fwl_RTCGPIOCheck(gpio_get_wakeup_pin(nReason)))
    {
        nReason = 0;
    }
#endif
    
#endif
    return nReason;  
}


/**
 * @brief RTC(real time clock) callback function
 * @author YiRuoxiang
 * @date 2006-04-03
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
static T_VOID rtc_callback_func(T_VOID)
{
    T_EVT_PARAM EventParm;
    T_U32 wakeup_type = WAKE_NULL;
    T_SYS_MAILBOX mailbox;    

    Fwl_Print(C3, M_RTC, "\n\n\n*******************rtc_callback_func***********\n\n\n");

#ifdef USE_EXTERN_RTC
    external_rtc_clear_alarm();
#endif

    EventParm.w.Param1 = (Fwl_GetAlmRTCCount()%ONE_DAY_SECOND);

    if (gb.PowerOffStatus == AK_TRUE)
    {
        Fwl_DisplayBacklightOff();
        Fwl_DisplayOff();

        VME_Reset();
    }

    switch (gb.RtcType)
    {
        case RTC_POWEROFF: // auto power off
            wakeup_type |= WAKE_PWROFF;
            break;
        case RTC_BATT_WARN: // low battery warning
            wakeup_type |= WAKE_BATT_WARN;
            break;
        case RTC_ALARM: // alarm
        default:
            wakeup_type |= WAKE_ALARM;
            if (!gb.bAlarming)
            {
				// usb state, no response alarm
				if (gb.InUsbMessage)
				{
					return;
				}
				//send sys event 
                mailbox.event = SYS_EVT_RTC;
                AK_PostUniqueEvent( &mailbox, AK_NULL);
            }
            break;
    }
#ifdef OS_ANYKA
#ifdef CAMERA_SUPPORT
	if (eM_s_camera_recorder == SM_GetCurrentSM())
	    VME_ReTriggerEvent(M_EVT_VIDEO_RECORD_STOP, AK_NULL);
#endif
#if (defined (SUPPORT_AUDIOREC) || defined (SUPPORT_FM))
		VME_ReTriggerUniqueEvent((vT_EvtSubCode)M_EVT_AUDIO_RECORD_STOP, 0);
#endif

#endif
    VME_ReTriggerEvent((vT_EvtSubCode)M_EVT_WAKE_SAVER, (T_U32)wakeup_type);
}



T_VOID 	Fwl_SetRtcCallBack(T_fRTC_CALLBACK cb)
{
#ifdef USE_EXTERN_RTC
    external_rtc_set_callback(rtc_callback_func);      
#else            
    rtc_set_callback(rtc_callback_func);      
#endif
}

/**
 * @brief RTC Init
 * @author WuShanwei
 * @date 2007-08-07
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID    Fwl_RTCInit(T_VOID)
{
#ifdef OS_ANYKA
    T_POWER_ON_TYPE  power_on_type; 
    T_U32 rtc;
    T_SYSTIME systime;
  
#ifdef USE_EXTERN_RTC
 	T_BOOL  bException = AK_FALSE;
#endif

    systime.year = SYSTEM_DEFAULT_YEAR;
    systime.month = 1;
    systime.day = 1;
    systime.hour = 0;
    systime.minute = 0;
    systime.second = 0;
    systime.milli_second = 0;
    systime.week = 1;

#ifdef USE_EXTERN_RTC
    i2c_pin_cfg(GPIO_I2C_SCL,GPIO_I2C_SDA);

    bException = external_rtc_CheckPower();

    if(bException)
    {
        gs.bSetRTCcount = AK_FALSE;
    }
#endif


#ifdef USE_EXTERN_RTC
    external_rtc_clear_alarm();

#else            
    rtc_init(gs.SysTimeYear);  
#endif
    systime = GetSysTime();

    SetSysTime(&systime);


#ifdef USE_EXTERN_RTC
    external_rtc_SetPowrFlag();
#endif
    
    Fwl_SetRtcCallBack(rtc_callback_func);
    //get system time
    systime = GetSysTime();
	
    if( (systime.year < MIN_YEAR) || (systime.year > MAX_YEAR))
    {
        Time_Init(&systime, SYSTEM_DEFAULT_YEAR);
        SetSysTime(&systime);
        systime = GetSysTime();
    }

    rtc =  Fwl_RTCGetCount();

    Fwl_Print(C3, M_RTC, "init rtc");        
  
    power_on_type = IsAlarmPowerOn(rtc, gs.AlarmClock.DayAlarm, gs.AlarmClock.WeekAlarm);

    if(power_on_type > 0)
    {
       gb.bRtcPwrOn = AK_TRUE;
    }
    else
    {
       gb.bRtcPwrOn = AK_FALSE;
    }
    
   Fwl_Print(C3, M_RTC, "power_on_type: %d, gs.AlarmDelay = %d\n", power_on_type, gs.AlarmDelay);
    if (gs.AlarmDelay)
    {
        gs.AlarmDelay = AK_FALSE;
        rtc_callback_func();
    }
    else if(power_on_type)    
    {
        rtc_callback_func();
    }
    else
    {
        T_U32 alarm_time;
        alarm_time = GetAlarmDataMinValid(AK_FALSE);

        Fwl_RTCSetAlarm(Fwl_RTCGetCount(), alarm_time, AK_TRUE, RTCTYPE_ALARM);

        gb.RtcType = RTC_ALARM;
    }

#endif 
}

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
T_S8    Fwl_RTCSetAlarm(T_U32 CurrentValue, T_U32 AlarmValue, T_BOOL PwDnEnable, T_eRTCType RTCType)
{

    T_U32 DisValue;

    DisValue = AlarmValue - CurrentValue;
    
#ifdef OS_ANYKA
    Fwl_SetAlarmRtcCount(Fwl_RTCGetCount() + DisValue);

    if(PwDnEnable)
    {
        Fwl_rtc_set_powerdownalarm(AK_TRUE);
    }
    else
    {
        Fwl_rtc_set_powerdownalarm(AK_FALSE);
    }
#endif 
    return 0;
}



T_VOID Fwl_rtc_set_powerdownalarm(T_BOOL flag)
{
#ifndef USE_EXTERN_RTC
     rtc_set_powerdownalarm(flag);  
#endif
}

T_VOID Fwl_SetAlarmRtcCount(T_U32 count)
{
#ifdef OS_ANYKA

#ifdef USE_EXTERN_RTC
    external_rtc_set_alarmcount(count);
#else
    rtc_set_alarmcount(count);
#endif

#endif // OS_ANYKA
}

T_VOID Fwl_RTCSetCount(T_U32 rtc_val)
{
#ifdef OS_ANYKA

#ifdef USE_EXTERN_RTC
    external_rtc_set_RTCcount( rtc_val);
#else
     //comment the function of rtc ,for  there is serious problem with rtc casing system  die
    rtc_set_RTCcount( rtc_val);
#endif    

#endif
}
T_U32 Fwl_RTCGetCount(T_VOID)
{
#ifdef OS_ANYKA

#ifdef USE_EXTERN_RTC
    return external_rtc_get_RTCcount();
#else
    return rtc_get_RTCcount();
#endif
#endif    
	return 0;
}

T_U32 Fwl_GetAlmRTCCount(T_VOID)
{
#ifdef OS_ANYKA
#ifdef USE_EXTERN_RTC
    return external_rtc_get_alarmcount();
#else
    return rtc_get_alarmcount();
#endif
#else
    return 0;
#endif
}

