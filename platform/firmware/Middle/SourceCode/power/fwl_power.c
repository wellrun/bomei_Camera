/**
 * @file fwl_power.c
 * @brief power function file, provide functions to control the system power
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 * @ref AK3223 technical manual.
 */
#include "fwl_power.h"
#include "misc.h"
#include "fwl_pfdisplay.h"
#include "fwl_oscom.h"
#include "eng_debug.h"
#include "eng_screensave.h"
#include "Ctl_msgbox.h"
#ifdef SUPPORT_JAVA
#include "ujLibApi.h"
#include "ujPort.h"
#endif
#include "AKAppMgr.h"
#include "eng_gblstring.h"
#include "eng_math.h"
#include "eng_debug.h"
#include "ctl_audioplayer.h"
#include "fwl_waveout.h"
#include "ctl_AVIPlayer.h"
#include "eng_MsgQueue.h"
#include "eng_BatWarn.h"
#include "gpio_config.h"
#include "arch_freq.h"
#include "arch_uart.h"
#include "hal_gpio.h"
#include "arch_interrupt.h"
#include "fwl_pfaudio.h"
#include "arch_lcd.h"
#include "Lib_state_api.h"
#include "Fwl_usb_host.h"
#include "drv_gpio.h"
#include "arch_analog.h"
#include "arch_rtc.h"
#include "Fwl_sys_detect.h"
#include "fwl_usb_host.h"
#include "Hal_sysdelay.h"
#include "Fwl_sys_detect.h"

#include "Lib_state.h"
#include "Arch_pmu.h"
#include "Fwl_KeyHandler.h"
#include "Fwl_rtc.h"
#include "Eng_AutoPowerOff.h"

#define BATTERY_MONITOR_INTERVAL        4   //second
#define BATTERY_WARP_VOLTAGE            300 //大公模客户机因二极管导致AD读取值与电池实际值偏差300mV 

#define ALARM_MAX                 0xFFFFFFFF

extern T_VOID get_wGpio_Mask(T_U32 *wgpio_mask);

static T_VOID sys_power_off(T_VOID);
static T_VOID SaveBatteryVoltage(T_U32 voltage);
static T_U32 ReadBatteryVoltage(T_VOID);

#ifdef OS_ANYKA
T_BOOL is_wakeup_by_touchscreen(T_U32 reason)
{
#if 0
    T_U32 wgpio_mask;
       
    wgpio_mask = (1<< get_wGpio_Bit(GPIO_TSCR_ADC));
    if(reason & (wgpio_mask<<1))
        return AK_TRUE;
#endif
    return AK_FALSE;
}


/**
 * @BRIEF Monitor battery voltage
 *      called in ddpubtimerhandler
 * @AUTHOR 
 * @DATE 2008-04-24
 * @PARAM[in] T_U32 millisecond
 * @RETURN T_VOID
 * @RETVAL: 
 */
T_VOID MonitorBatteryVoltage(T_U32 millisecond)
{
    T_U32 second = millisecond / 1000;
    T_S32 voltage, value;
    T_MSGBOX  msgbox;
    T_RECT    msgRect;
    static T_U8 BatMinCount = 0;
    static T_U8 BatWarnCount = 0;
#ifdef USB_HOST
    static T_U8 UHostBatWarnCount = 0;
#endif
    static T_U32 time_counter = 0;
    static T_U32 num = 0;

    time_counter += second;

    if ((time_counter % BATTERY_MONITOR_INTERVAL) == 0)
    {
        return;
    }
    
    value = ReadBatteryVoltage();
    SaveBatteryVoltage(value);
    
    voltage = Fwl_GetBatteryVoltage();
    if (++num >= PRINT_TIMES_SEC)
    {
        Fwl_Print(C3, M_PUBLIC, "battery voltage = %d", voltage);
        num = 0;
    }
    
    /**avoid wrong judgement if voltage wave*/
    if (voltage <=  BATTERY_VALUE_MIN)
    {
        BYTE_INC(BatMinCount);
    }
    else
    {
        BatMinCount = 0;
    }

    if (voltage <= BATTERY_VALUE_WARN)
    {
        BYTE_INC(BatWarnCount);
    }
    else
    {
        BatWarnCount = 0;
    }

#ifdef USB_HOST
    if ((AK_TRUE == gb.bUDISKAvail) && (AK_NULL != gb.driverUDISK))
    {
        if (voltage <= BATTERY_VALUE_WARN_USBHOST)
        {
            BYTE_INC(UHostBatWarnCount);
        }
        else
        {
            UHostBatWarnCount = 0;
        }

        if (UHostBatWarnCount >= BATTERY_KILL_WAVE
            && !Fwl_UseExternCharge())
        {
#ifdef SUPPORT_VIDEOPLAYER
            if (AK_TRUE == AVIPlayer_IsPlaying())
            {
                ConsolePrint(7, "Fwl_power.c, go to stop AVIPlayer = %d\n", voltage);
                AVIPlayer_Stop(T_END_TYPE_ERR);
            }
#endif
            VME_ReTriggerEvent(M_EVT_WAKE_SAVER, WAKE_NULL);

#if 0
            if (gb.PubMsgAllow)
            {
                MsgQu_Push(GetCustomTitle(ctWARNING), GetCustomString(csUSBDISK_BAT_LOW), MSGBOX_INFORMATION, MSGBOX_DELAY_2);
                if (!gb.InPublicMessage)
                    VME_ReTriggerUniqueEvent(M_EVT_Z05COM_MSG, AK_NULL);
            }
#else           
            MsgBox_InitAfx(&msgbox, 1, ctWARNING, csUSBDISK_BAT_LOW, MSGBOX_INFORMATION);    
            MsgBox_Show(&msgbox);
            MsgBox_GetRect(&msgbox, &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);
            AK_Sleep(200);
#endif
            Fwl_UsbDisk_POP_UP();

            UHostBatWarnCount = 0;
            
            return;
        }
    }
#endif

    if (BatMinCount >= BATTERY_KILL_WAVE
        && !Fwl_UseExternCharge())
    {
        /**exit screen saver*/
        VME_ReTriggerEvent(M_EVT_WAKE_SAVER, WAKE_NULL);
#if 0
        if (gb.PubMsgAllow)
        {
            MsgQu_Push(GetCustomTitle(ctWARNING), GetCustomString(csLOW_BATTERY), MSGBOX_INFORMATION, MSGBOX_DELAY_1);
            if (!gb.InPublicMessage)
                VME_ReTriggerUniqueEvent(M_EVT_Z05COM_MSG, AK_NULL);
        }
#endif
        if (eM_s_pub_switch_off != SM_GetCurrentSM())
        {
            MsgBox_InitAfx(&msgbox, 1, ctWARNING, csLOW_BATTERY_POWEROFF, MSGBOX_INFORMATION);    
            MsgBox_Show(&msgbox);
            MsgBox_GetRect(&msgbox, &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);

            mini_delay(1000);
        }
        
#if (defined (SUPPORT_AUDIOREC) || defined (SUPPORT_FM))
        m_triggerEvent(M_EVT_AUDIO_RECORD_STOP, AK_NULL);
#endif

#ifdef CAMERA_SUPPORT
        m_triggerEvent(M_EVT_VIDEO_RECORD_STOP, AK_NULL);
#endif


        m_triggerEvent(M_EVT_Z09COM_SYS_RESET, AK_NULL);
        m_triggerEvent(M_EVT_Z99COM_POWEROFF, AK_NULL);
        //VME_ReTriggerEvent(M_EVT_Z99COM_POWEROFF, AK_NULL);    
        //VME_ReTriggerEvent(M_EVT_Z09COM_SYS_RESET, AK_NULL);
    }

    else if (BatWarnCount >= BATTERY_KILL_WAVE 
            && !Fwl_UseExternCharge()
            && gs.LowBatTM != 0)
    {
        if (BatWarnIsEnable() == AK_FALSE)
        {
            BatWarnEnable();
            BatWarnCountSet(10);
            gb.PowerLowBattery = AK_TRUE;
        }
    }
    else
    {
        BatWarnDisable();
        gb.PowerLowBattery = AK_FALSE;
    }

}


/**
 * @BRIEF Read real-time battery voltage
 *      
 * @AUTHOR Guohui
 * @DATE 2008-04-24
 * @PARAM[in] T_VOID
 * @RETURN T_U32
 * @RETVAL: real time voltage of battery
 */
static T_U32 ReadBatteryVoltage(T_VOID)
{
    T_U32 value, ret;    


    value =  analog_getvalue_bat();
        
    ret = BATTERY_VOL_FORMULA(value);

    return ret;
}

static T_VOID sys_power_off(T_VOID)
{
    Fwl_DeInitFs();
#ifdef CI37XX_PLATFORM
    while(1)
    {
        gpio_set_pin_level(GPIO_POWER_OFF, GPIO_LEVEL_LOW);
        rtc_set_wpinLevel(0);
    }

#endif
    
    //*(volatile T_U32 *)0x20000000 = 1<<10 | 1<<1;
}

T_VOID VME_Terminate(T_VOID)
{
    sys_power_off();
}

/**
 * @BRIEF Save real-time battery voltage in data array
 *      
 * @AUTHOR 
 * @DATE 2008-04-24
 * @PARAM[in] T_U32 voltage: real-time battery voltage
 * @RETURN T_VOID
 * @RETVAL: 
 */
T_VOID SaveBatteryVoltage(T_U32 voltage)
{
#ifdef OS_ANYKA
    gb.Voltage[gb.VoltageIdx] = voltage;
#else
    gb.Voltage[gb.VoltageIdx] = 1000;
#endif
    gb.VoltageIdx = (gb.VoltageIdx + 1) % SAVE_VOLTAGE_NUM;
}

/**
 * @BRIEF Get battery voltage
 *      its return value is a average value
 * @AUTHOR 
 * @DATE 2008-04-24
 * @PARAM[in] T_VOID
 * @RETURN T_U32
 * @RETVAL: battery voltage
 */
T_U32 Fwl_GetBatteryVoltage(T_VOID)
{
    T_U32 i;
    T_U32 sum;
    int sumcount;
    T_U32 VCheck;
    T_U32 voltage;
    T_U32 minVal = 0xFFFFFFFF;
    T_U32 maxVal = 0;

    VCheck = 40;
    sum = 0;
    sumcount = 0;
    for (i=0; i<SAVE_VOLTAGE_NUM; i++)
    {
        if (gb.Voltage[i] != BATTERY_VALUE_INVALID)
        {
            minVal = gb.Voltage[i]<minVal ? gb.Voltage[i] : minVal;
            maxVal = gb.Voltage[i]>maxVal ? gb.Voltage[i] : maxVal;
            sum += gb.Voltage[i];
            sumcount++;
        }
    }

    if (sumcount == SAVE_VOLTAGE_NUM)
    {
        voltage = (sum-minVal-maxVal) >> SAVE_VOLTAGE_SHIFT;         //SAVE_VOLTAGE_NUM == 16, >>4 == /16
    }
    else if (sumcount != 0)
    {
        voltage = sum / sumcount;
    }
    else
    {
        voltage = ReadBatteryVoltage();
    }

	return voltage;
}



/**
 * @brief Judge whether use extern charge
 * 
 * @author Guohui
 * @date 2008-4-24
 * @param T_VOID
 * @return T_BOOL
 * @retval 1: charge is in   0:charge is out
 */
T_BOOL Fwl_UseExternCharge(T_VOID)
{
    return usb_is_connected();
}


/**
 * @brief Judge whether charging is finished
 * 
 * @author Guohui
 * @date 2008-4-24
 * @param T_VOID
 * @return T_BOOL
 * @retval 1: charge finish   0: charge not finish
 */
T_BOOL Fwl_ChargeVoltageFull(T_VOID)
{
    T_BOOL ret;
    T_U32 chg_voltage;

#ifdef USE_PMU
    ret = charger_is_over();
#else
    chg_voltage = Fwl_GetBatteryVoltage();
    if (chg_voltage > BATTERY_VALUE_CHANGE)
    {
        ret = AK_TRUE;
    }
    else
    {
        ret = AK_FALSE;
    }
#endif

    return ret;
}

#endif

/**
 * @brief set rtc and enter standby
 * 
 * @author 
 * @date 
 * @param T_VOID
 * @return T_U32
 * @retval the reason of exiting from standby
 */
#define WAKE_CONFIG_REG_NUM 2
T_U32 Fwl_SetChipStandby(T_VOID)
{
    T_U32 poff_second, alarm, almrtc, rtc;
    T_U32 BatWarnTime;
    T_U32 reason = 0;
    T_U32 mask[WAKE_CONFIG_REG_NUM] = {0,};
    T_U32 i, j;
#ifdef OS_ANYKA
    T_WU_TYPE wakeup_type;    
    T_U32 times;
#endif

    Fwl_Print(C3, M_POWER, "Set chip To standby..\n");
    Set_key_valid(AK_FALSE);
#ifdef USB_HOST
    if ((!Fwl_UseExternCharge()
        || (Fwl_UseExternCharge() && Fwl_UsbHostIsConnect())
        || eM_s_usb_host == SM_GetCurrentSM())
            && 0 != gs.LowBatTM 
            && BatWarnIsEnable()
        )
#else
    if (!Fwl_UseExternCharge()
        && 0 != gs.LowBatTM
        && BatWarnIsEnable())
#endif
    {
        BatWarnTime = BatWarnGetCount();
    }
    else
    {
        BatWarnTime = ALARM_MAX;
    }
#ifdef USB_HOST
    if ((!Fwl_UseExternCharge()
        || (Fwl_UseExternCharge() && Fwl_UsbHostIsConnect())
        || eM_s_usb_host == SM_GetCurrentSM()) 
            && 0 != gs.PoffTM 
            && AutoPowerOffIsEnable()
        )
#else
    if (!Fwl_UseExternCharge()
        && 0 != gs.PoffTM 
        && AutoPowerOffIsEnable())
#endif
    {
        poff_second = AutoPowerOffGetCount();
    }
    else
    {
        poff_second = ALARM_MAX;
    }

    almrtc = Fwl_GetAlmRTCCount();
    rtc = Fwl_RTCGetCount();
    alarm = (almrtc >= rtc) ? almrtc - rtc : ALARM_MAX;

    Fwl_Print(C3, M_POWER, "alarm = %d, poff_second = %d, BatWarnTime = %d\n",
                                    alarm, poff_second, BatWarnTime);
#ifdef USB_HOST
    if (((alarm > poff_second) || (alarm > BatWarnTime)) 
            && (!Fwl_UseExternCharge() 
                || (Fwl_UseExternCharge() && Fwl_UsbHostIsConnect()) 
                || eM_s_usb_host == SM_GetCurrentSM())
        )
#else
    if (((alarm > poff_second) || (alarm > BatWarnTime)) 
            && !Fwl_UseExternCharge())
#endif
    {
        if (0 != gs.LowBatTM
            && BatWarnIsEnable()
            && BatWarnTime < poff_second)
        {
            /**set rtc for low battery warning*/
            Fwl_Print(C3, M_POWER, "set rtc for low battery waring\n");
            gb.RtcType = RTC_BATT_WARN;
 
            Fwl_rtc_set_powerdownalarm(AK_FALSE);

            Fwl_SetAlarmRtcCount(Fwl_RTCGetCount() + BatWarnTime);
        }
        else if (gs.PoffTM != 0
            && AutoPowerOffIsEnable()
            && (poff_second < alarm))    
        {
             //if auto power-off is on, should do as below before enter standby
            /**set rtc for auto power off*/
            Fwl_Print(C3, M_POWER, "set rtc for power off\n");
            gb.RtcType = RTC_POWEROFF;  // set rtc for auto power off

            Fwl_rtc_set_powerdownalarm(AK_FALSE);//disable high-pulse output of MCU wakeup pin to dispel the conflict with GPIO_POWER_OFF's low level

            Fwl_SetAlarmRtcCount(Fwl_RTCGetCount() + poff_second);
        }
        
    }
    
    /**set wake-up gpio*/

    get_wGpio_Mask(mask);
    
    for (i = 0; i < sizeof(mask)/sizeof(mask[0]); i++)
    {
		for(j = 0; j < 32; j++)
		{
			if((mask[i] >> j) & 0x01) //the  bit  that  setting to "1"
			{
    			rtc_set_wgpio(j, i);
			}
		}
    }


#ifdef OS_ANYKA

    if (gs.bVoiceWakeup)
    {
        wakeup_type = WU_USB|WU_ALARM|WU_GPIO|WU_VOICE;
    }
    else
    {
        wakeup_type = WU_USB|WU_ALARM|WU_GPIO;
    }

    rtc_set_wakeup_type(wakeup_type);    
    if (wakeup_type & WU_VOICE) //voice wakeup
    {
        for(times=0; times<40; times++)
        {
            mini_delay(100);
            if(Is_key_valid())
            {
                VME_ReTriggerEvent((vT_EvtSubCode)M_EVT_WAKE_SAVER, (T_U32)WAKE_GPIO);
                return 0;
            }
        }
    }
#endif


    mini_delay(6); 
    rtc_enter_standby(); 
       
    reason = Fwl_RtcExitStandby();
    mini_delay(30);     //wait for 30ms to output M_EVT_USER_KEY
    

    if (WU_ALARM != (reason&0xf))  // reason is not rtc, because wake up event wll be sent in rtc callback
    {
        /**wake up from screen saver*/
        VME_ReTriggerEvent((vT_EvtSubCode)M_EVT_WAKE_SAVER, (T_U32)WAKE_GPIO);
    }
    return 0;
}



