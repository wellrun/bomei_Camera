
#include "Fwl_public.h"
#include "Eng_ScreenSave.h"
#include "Eng_KeyMapping.h"
#include "Eng_MsgQueue.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfAudio.h"
#include "Ctl_Fm.h"
#include "Fwl_pfKeypad.h"
#include "Fwl_osFS.h"
#include "Eng_topbar.h"
#include "Ctl_AudioPlayer.h"
#include "gpio_config.h"
#include "Fwl_Initialize.h"
#include "fwl_usb.h"
#include "Lib_state_api.h"
#include "fwl_sd.h"
#include "Fwl_sys_detect.h"
#include "svc_medialist.h"
#include "Ctl_RecAudio.h"
#include "fwl_usb_host.h"
#include "arch_freq.h"
#include "Eng_IdleThread.h"
#include "Eng_BatWarn.h"
#include "Eng_AutoPowerOff.h"
#include "Ctl_AviPlayer.h"
#include "Fwl_power.h"

#ifdef OS_ANYKA
#include "hal_gpio.h"
#include "drv_gpio.h"
#include "gpio_config.h"
#endif



static T_U8 TBUpdtCnt = 0;
static T_eMPLAYER_STATUS AudioPlayStatus = MPLAYER_CLOSE;

extern T_BOOL gb_lastHasDownEvt;

extern T_VOID KeyLightCountDecrease(T_U32 millisecond);
extern T_VOID DD_CameraRecorder_SaveFile(T_VOID);

#ifdef TOUCH_SCR
extern T_BOOL TscrIsCalibrated(T_VOID);
#endif


unsigned char ddsysstarthandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    Fwl_Print(C3, M_CTRL, "-------------------- SYSTER START EVENT ---------------\r\n");

    return 1;
}

unsigned char ddpubtimerhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    static T_U32 num = 0;

    if (++num >= PRINT_TIMES_SEC)
    {
        Fwl_Print(C3, M_PUBLIC, "*** State = %d, memory remain = %d ***", SM_GetCurrentSM(), Fwl_GetRemainRamSize());
#ifdef OS_ANYKA
        Fwl_Print(C3, M_PUBLIC, "*** asic = %d, cpu usage = %d ***\n\n", get_asic_freq(), 100-Idle_GetCpuIdle());
#endif
        num = 0;
    }

    //when use akosd.a and open it
    //AK_List_Task();
    //AK_Task_clear_time();
    
    AK_Check_Task_Stack();
    Fwl_RamBeyondMonitor(6);

    KeyLightCountDecrease((*pEventParm)->w.Param2);

    if (BatWarnIsEnable())
        BatWarnCountDecrease((*pEventParm)->w.Param2);
    if (AutoPowerOffIsEnable())
        AutoPowerOffCountDecrease((*pEventParm)->w.Param2);
    if (ScreenSaverIsOn() && (!Fwl_TvoutIsOpen()) && !gb_lastHasDownEvt)
        ScreenSaverCountDecrease((*pEventParm)->w.Param2);

    TopBar_ShowTimeDecrease((*pEventParm)->w.Param2);


    /* battery voltage check */
    MonitorBatteryVoltage((*pEventParm)->w.Param2);
    
    /**--------BEGIN: top bar----------*/
    /**Update battery icon*/
    TopBar_UpdateBattIcon();

    /**Scroll title text*/
    TopBar_TitleTextScroll();

    /*Video text scroll*/
#ifdef SUPPORT_VIDEOPLAYER
    AVIPlayer_SetTextOffset();
#endif

    /**Update time in top bar*/
    TBUpdtCnt++;
    if (TBUpdtCnt >= 30)
    {
        /**Show topbar*/
        TopBar_Show(TB_REFRESH_TITLE);     // Update time in topbar
        TopBar_Refresh();

        TBUpdtCnt = 0;
    }

    if (AudioPlayStatus != MPlayer_GetStatus())
    {
        AudioPlayStatus = MPlayer_GetStatus();

        TopBar_Show(TB_REFRESH_AUDIO_STATUS);
        TopBar_Refresh();
    }    

    return 1;
}

unsigned char dduserkeyhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    T_MMI_KEYPAD phyKey;
    T_BOOL      isShowUi = AK_FALSE;
    M_STATES current_sm;
    
    UserCountDownReset();

    if (AK_GetScreenSaverStatus())
        return 1;

    phyKey.keyID = (*pEventParm)->c.Param1;
    phyKey.pressType = (*pEventParm)->c.Param2;

    /**For switch between title and time in top bar*/
    TopBar_SetTimeShowFlag(AK_FALSE); // Show title in top bar when any user's key

    TopBar_ResetShowTimeCount();
    current_sm = SM_GetCurrentSM();
    
    // when recorder is runing, display will use asyn refresh, can not refresh by syn mode
    if (eM_s_camera_recorder == current_sm)
    {
        isShowUi = AK_FALSE;
    }
    else
    {
        isShowUi = AK_TRUE;
    }

    if (isShowUi)
    {
        /**Show topbar*/
        TopBar_Show(TB_REFRESH_ALL);
        TopBar_Refresh();
    }
    
    switch (MappingPublicKey(phyKey))
    {
    case fkPUBLIC_KEY_LOCK:
        if (!gb.PubMsgAllow)
        {
            return 0;
        }

        if (!gb_UserkeyValid)
        {
            return 1;
        }
        
        //if not standby states, disable  key lock funciton
        if (eM_s_stdb_standby != current_sm)
        {
            return 0;
        }

        if (gb.KeyLocked)
        {
            SetKeyLightCount(gs.KeyLightTM);
            gb.KeyLocked = AK_FALSE;
            
            if (isShowUi)
            {
                MsgQu_Push(GetCustomTitle(ctHINT), GetCustomString(csKEY_UNLOCKED), MSGBOX_INFORMATION, MSGBOX_DELAY_0);
                if (!gb.InPublicMessage)  
                {
                    VME_ReTriggerUniqueEvent(M_EVT_Z05COM_MSG, (vUINT32)AK_NULL);
                }
            }
        }
        else
        {
            SetKeyLightCount(0);
            gb.KeyLocked = AK_TRUE;
            
            if (isShowUi)
            {
                MsgQu_Push(GetCustomTitle(ctHINT), GetCustomString(csKEY_LOCKED), MSGBOX_INFORMATION, MSGBOX_DELAY_0);
                if (!gb.InPublicMessage)
                    VME_ReTriggerUniqueEvent(M_EVT_Z05COM_MSG, (vUINT32)AK_NULL);
            }
        }

        Fwl_KeyStop();
        
        current_sm = SM_GetCurrentSM();
        if (eM_s_init_power_on != current_sm)
        {
            return 0;
        }
        else
        {
            return 1;
        }
        break;
        
    case fkPUBLIC_VOICE_UP:            
        if (!gb.KeyLocked)
        {
            if (Fwl_GetAudioVolumeStatus())
            {
                Fwl_AudioVolumeAdd();
                Fwl_AudioSetVolume(Fwl_GetAudioVolume());
                return 1;
            }
        }
        break;
        
    case fkPUBLIC_VOICE_DOWN:            
        if (!gb.KeyLocked)
        {
            if (Fwl_GetAudioVolumeStatus())
            {
                Fwl_AudioVolumeSub();
                Fwl_AudioSetVolume(Fwl_GetAudioVolume());
                return 1;
            }
        }
        break;
        
    default:
        break;
    }

    if (gb.KeyLocked && eM_s_pub_alarm != SM_GetCurrentSM())
    {
        /**not display key lock message box if key type is press up*/
        if (PRESS_UP == phyKey.pressType)
        {
            return 0;
        }

        if(!gb.InPublicMessage)
        {
            if (gb.PubMsgAllow)
            {
                /**avoid overload s_pub_pre_message*/
                gb.PubMsgAllow = AK_FALSE;

                if (isShowUi)
                {
                    MsgQu_Push(GetCustomTitle(ctHINT), GetCustomString(csKEY_LOCKED), MSGBOX_INFORMATION, MSGBOX_DELAY_0);
                    VME_ReTriggerUniqueEvent(M_EVT_Z05COM_MSG, (vUINT32)AK_NULL);
                }
            }
        }
        else if (phyKey.keyID == kbOK || phyKey.keyID == kbCLEAR)
        {
            VME_ReTriggerUniqueEvent(M_EVT_PRE_EXIT, (vUINT32)AK_NULL);
        }

        return 0;
    }

    return 1;
}

unsigned char ddtouchscreenhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    //if(TvOut_IsOpen())
    if (Fwl_GetDispalyType() != DISPLAY_LCD_0)
    {
        return 0;
    }
    
    UserCountDownReset();

    if (AK_GetScreenSaverStatus())
        return 1;

    /**For switch between title and time in top bar*/
    TopBar_SetTimeShowFlag(AK_FALSE); // Show title in top bar when any user's touch

    TopBar_ResetShowTimeCount();

    if (gb.KeyLocked && eM_s_pub_alarm != SM_GetCurrentSM())
    {
        if(!gb.InPublicMessage)
        {
            if (gb.PubMsgAllow)
            {
                /**avoid overload s_pub_pre_message*/
                gb.PubMsgAllow = AK_FALSE;
                if (eM_s_camera_recorder != SM_GetCurrentSM())
                {
                    MsgQu_Push(GetCustomTitle(ctHINT), GetCustomString(csKEY_LOCKED), MSGBOX_INFORMATION, MSGBOX_DELAY_0);
                    VME_ReTriggerUniqueEvent(M_EVT_Z05COM_MSG, (vUINT32)AK_NULL);
                }
            }
        }

        return 0;
    }

    return 1;
}

unsigned char ddsdcbmsghandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{     
    AudioPlayer_AutoSwitch((T_U8)(*pEventParm)->lParam);  
    
    return 0;
}

unsigned char ddaudioabplay(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    AudioPlayer_BSeekToA();
    return 0;
}

unsigned char ddaudiostop(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    //Fwl_Print(C3, M_CTRL, "ddaudiostop 1 %x",pEventParm);
    if(AK_NULL != pEventParm 
        && AK_NULL != (*pEventParm) 
        && (vUINT32)AK_NULL != (*pEventParm)->lParam 
    )
    {//xuyr add 991
        //Fwl_Print(C3, M_CTRL, "ddaudiostop 2 &p_audio_player=%x",(*pEventParm)->lParam);
        if (AUDIOPLAYER_STATE_BACKGROUNDPLAY == (*(T_AUDIOPLAYER **)((*pEventParm)->lParam))->OldState)
        {
            //Fwl_Print(C3, M_CTRL, "ddaudiostop 3");
            AudioPlayer_Destroy();
            AudioPlayer_Free();
            //Fwl_Print(C3, M_CTRL, "ddaudiostop 4");
        }
        return 0;
    }
    else
    {    
        AudioPlayer_Stop();
        
        Fwl_Print(C3, M_PREPROC, "ddaudiostop(): return main menu\n");
        *event = M_EVT_Z09COM_SYS_RESET;
        return 1;
    }
}

/**
* event: M_EVT_AUDIO_RECORD_STOP:
* event parameter: lParam
* parameter value: 
*        Value         Meaning
*           0           indicate the event is posted by other modules ,e.g. alarm clock/auto power off ,etc.
*/
unsigned char ddaudiorecordstop(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
#if (defined(SUPPORT_AUDIOREC)||defined(SUPPORT_FM))

    if (!Ctl_RecAudio_IsStoped())
    {
        AK_DEBUG_OUTPUT("@ddaudiorecordstop:receive stop record event\n");
        
        if (Ctl_RecAudio_IsBgRec(eINPUT_SOURCE_LINEIN))
        {
            AK_DEBUG_OUTPUT("+stop FM background record\n");
        }
        else
        {
            Ctl_RecAudio_Stop(Ctl_RecAudio_IsExistMedium());
        }
    }
#endif

    return 0;
}

unsigned char ddvideorecordstop(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    if (eM_s_camera_recorder == SM_GetCurrentSM())
    {
#ifdef CAMERA_SUPPORT
        gb.ChangePath = AK_TRUE;
        DD_CameraRecorder_SaveFile();
#endif
         //这个事件可以让其他应用知道触发了录像退出
        (*pEventParm)->c.Param1 = EVT_VIDEO_REC_STOP;
        Fwl_Print(C3, M_PREPROC, "ddvideorecordstop():return main menu\n");

        *event = M_EVT_Z09COM_SYS_RESET;
    }
    
    return 1;
}

unsigned char ddemptymsghandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    return 0;
}

unsigned char ddwakesaverhandler(T_EVT_CODE *event, T_EVT_PARAM **pEventParm)
{
    if (AK_GetScreenSaverStatus())
        return 1;
    else
        return 0;
}

unsigned char ddalarmhandler(vT_EvtCode* event, vT_EvtParam** pEventParm)
{
#ifdef OS_ANYKA

    Fwl_Print(C3, M_CTRL, "ddalarmhandler1 event = %d.\n", *event );
    if(eM_s_init_system == SM_GetCurrentSM() ||eM_s_init_power_on  == SM_GetCurrentSM())
    {
        *event = M_EVT_1;
        (*pEventParm)->c.Param8 = EVT_RTC_TRIGGER;   
        Fwl_Print(C3, M_CTRL, "ddalarmhandler2 event = %d.\n", *event );

        return 1;

    }
    else
    {
        if (eM_s_usb_camera != SM_GetCurrentSM())
        {
            *event = M_EVT_Z09COM_SYS_RESET;
            (*pEventParm)->c.Param8 = EVT_RTC_TRIGGER;   
            Fwl_Print(C3, M_CTRL, "ddalarmhandler3 event = %d, M_EVT_Z09COM_SYS_RESET = %d.\n", *event, M_EVT_Z09COM_SYS_RESET);        
        }
        
        return 1;
    }
    
#endif
    return 1;
}

unsigned char ddsdcardhandler(vT_EvtCode* event, vT_EvtParam** pEventParm)
{
    T_BOOL SdMountState = AK_FALSE;

    //Fwl_Print(C3, M_PREPROC, "ddsdchandler,sd type[%d]\n",(*pEventParm)->c.Param2);

    if (EVT_SD_PLUG_IN == (*pEventParm)->c.Param1)
    {
        SdMountState = Fwl_Sd_HwInit((*pEventParm)->c.Param2);
        if (SdMountState) //sd init OK
        {
            SdMountState = Fwl_Sd_Mount((*pEventParm)->c.Param2);
            if (!SdMountState) //Mount sd fail ,destory sd card
            {
                Fwl_Sd_HwDestory((*pEventParm)->c.Param2);
            }
        }
    }
    else
    {
        SdMountState = Fwl_Sd_UnMount((*pEventParm)->c.Param2);
        Fwl_Sd_HwDestory((*pEventParm)->c.Param2);
    }
    

    (*pEventParm)->c.Param3 = SdMountState;
    m_triggerEvent(M_EVT_SD_MOUNT_STATE, *pEventParm);

    UserCountDownReset();
    
    return 1;
}

unsigned char ddusbhandler(vT_EvtCode* event, vT_EvtParam** pEventParm)
{
#ifdef SPIBOOT
    if(eM_s_set_sys_update == SM_GetCurrentSM())//当前更新不能进入usb
    {
        return 0;
    }
#endif
    if (((EVT_USB_PLUG_IN == (*pEventParm)->c.Param1)))
    {
        if (USB_DISK_MODE == Fwl_UsbGetMode())  //usb device
        {
            //AK_DEBUG_OUTPUT("m_triggerEvent(M_EVT_USB_IN, pEventParm)\n");
            m_triggerEvent(M_EVT_USB_IN, AK_NULL);
        }
        else if (USB_CAMERA_MODE == Fwl_UsbGetMode())// pc camera
        {
#ifdef CAMERA_SUPPORT    
            //AK_DEBUG_OUTPUT("m_triggerEvent(M_EVT_USB_CAMERA, pEventParm)\n");
            m_triggerEvent(M_EVT_USB_CAMERA, AK_NULL);            
#endif  
        }
			
        return 1;
    }
    else
    {
        if (EVT_USB_PLUG_OUT == (*pEventParm)->c.Param1)
        {
        
            if (USB_CAMERA_MODE == Fwl_UsbGetMode())
            {
                *event = M_EVT_Z09COM_SYS_RESET;
                return 1;
            }
            else if (USB_DISK_MODE == Fwl_UsbGetMode())//disk mode
            {
                *event = M_EVT_USB_OUT;
                return 1;
            }
            
        }
        return 0;
    }
    
    return 1;
}

unsigned char ddpublicdetecthandler(vT_EvtCode* event, vT_EvtParam** pEventParm)
{
    T_SYS_MAILBOX mailbox;
    T_SYS_EVTID sys_event;
    
    if ((M_EVT_SDIO_DETECT == *event) || (M_EVT_SDMMC_DETECT == *event))
    {
        sys_event = SYS_EVT_SD_PLUG;
    }
    else //USB
    {
        sys_event = SYS_EVT_USB_PLUG;
    }

#ifdef TOUCH_SCR    //若触摸屏未校准过，则不响应USB/SD的插入 
    if ((eM_s_tscr_calibrate == SM_GetCurrentSM() && !TscrIsCalibrated())
        && 
        ((SYS_EVT_USB_PLUG == sys_event && EVT_USB_PLUG_IN == (*pEventParm)->c.Param1)
          || (SYS_EVT_SD_PLUG == sys_event && EVT_SD_PLUG_IN == (*pEventParm)->c.Param1))
        )
    {
        return 0;
    }
#endif

    //pre , 除去usbdisk和usb camera 状态 机下，拔出usb线
    if (!((sys_event == SYS_EVT_USB_PLUG) && (EVT_USB_PLUG_OUT == (*pEventParm)->c.Param1)
        && (Fwl_UsbGetMode()!= USB_HOST_MODE)))
    {
#ifdef CAMERA_SUPPORT
        gb.ChangePath = AK_TRUE;
        DD_CameraRecorder_SaveFile(); //video
#endif
#if (defined (SUPPORT_AUDIOREC) || defined (SUPPORT_FM))
        if (!Ctl_RecAudio_IsStoped() && !Ctl_RecAudio_IsExistMedium())
        {
            VME_ReTriggerEvent(M_EVT_WAKE_SAVER, WAKE_NULL);
            VME_ReTriggerEvent(M_EVT_AUDIO_RECORD_STOP, 1);
        }
#endif

        //send msg
        VME_ReTriggerEvent(M_EVT_Z09COM_SYS_RESET, 0);
    }
    
    mailbox.event = sys_event;
    mailbox.param.c.Param1 = (*pEventParm)->c.Param1;
    mailbox.param.c.Param2 = (*pEventParm)->c.Param2;
    AK_PostUniqueEvent(&mailbox, AK_NULL);
    
    return 0;
}

unsigned char ddpiniofliphandler(vT_EvtCode* event, vT_EvtParam** pEventParm)
{
    if (PINIO_EARPHONE_EVENT == (*pEventParm)->w.Param1 
        && 0 == (*pEventParm)->w.Param2)
    {// earphone out
    }
    
    return 1;
}


