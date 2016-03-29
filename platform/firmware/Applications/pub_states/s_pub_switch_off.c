#include "Fwl_public.h"
#include "Fwl_Initialize.h"
#include "Fwl_osMalloc.h"
#include "Fwl_pfAudio.h"
#include "Fwl_pfKeypad.h"
#include "Fwl_osFS.h"
#include "lib_image_api.h"
#include "Eng_ImgConvert.h"
#include "Eng_Jpeg2Bmp.h"
#include "Fwl_Image.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Ctl_AudioPlayer.h"
#include "eng_topbar.h"
#include "Lib_state.h"
#include "Eng_AkBmp.h"
#include "Ctl_AVIPlayer.h"
#include "Ctl_Fm.h"
#include "eng_string.h"
#include "fwl_keyhandler.h"
#include "fwl_power.h"
#include "fwl_oscom.h"
//#include "Misc.h"
#include "Fwl_pfdisplay.h"
#include "Fwl_rtc.h"
#include "Eng_pwr_onoff_video.h"
#include "fwl_osfs.h"
#include "Lib_state_api.h"
#include "eng_imgconvert.h"
#include "fwl_display.h"
#include "Fwl_usb_host.h"
#include "Eng_IdleThread.h"
#include "svc_medialist.h"


#define POFF_IMG_SHOW_TIME      2       //2s
#define POFF_AUDIO_PLAY_TIME    5       //5s

#define SWITCHOFF_PLAYING_CHECKUSB

extern T_pDATA pGB_dispBuffBackup;
extern 	T_BOOL sys_usb_detect(T_VOID);

//static  T_BOOL    PoffVflag = 0;

static T_POWER_PARM *pPowerParm = AK_NULL;


extern T_VOID StandbyFree(T_VOID);
extern T_VOID PublicTimerStop(T_VOID);
extern T_VOID AK_ExitMtvPlayer(T_VOID);
extern T_VOID AK_ExitKokPlayer(T_VOID);
static T_VOID AK_PowerOffVideoStop(T_VOID);
static T_VOID AK_PowerOffAVEndCB(T_END_TYPE endType);
/*---------------------- BEGIN OF STATE s_pub_switch_off ------------------------*/

T_VOID PowerOnOff_PrintChar(T_U32 charID)
{
	Fwl_FillSolid(HRGB_LAYER, POWER_ON_BACK_COLOR);
    Fwl_UDispSpeciString(HRGB_LAYER, \
            (T_POS)((Fwl_GetLcdWidth() - UGetSpeciStringWidth((T_U16 *)GetCustomString(charID), CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(GetCustomString(charID))))/2), \
            (T_POS)((Fwl_GetLcdHeight() - g_Font.CHEIGHT)/2), \
            (T_U16 *)GetCustomString(charID), \
            COLOR_YELLOW, CURRENT_FONT_SIZE, \
            (T_U16)Utl_UStrLen(GetCustomString(charID)));
	
    Fwl_RefreshDisplay();
}

static T_VOID PowerOff_ShowCharging(T_VOID)
{
	T_U32 len, i;
	T_LEN   width = 0;
    T_LEN   height= 0;
	T_pCDATA battaryData;

	Fwl_Print(C3, M_POWER, "Adaptor charging...\n");
	
    gb.PowerOffStatus = AK_TRUE;
    Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);

    for (i = eRES_BMP_CHARGE_ANIMTN1; i <= eRES_BMP_CHARGE_ANIMTN3; i++)
    {
        if (!Fwl_UseExternCharge())
        {
			Fwl_DisplayBacklightOff();
            Fwl_DisplayOff();
            VME_Terminate();
        }

        if (!Fwl_ChargeVoltageFull())
        {
            battaryData = Res_GetBinResByID(AK_NULL, AK_TRUE, i, &len);
        }
        else
        {
            battaryData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CHARGE_ANIMTN3, &len);
        }

        AKBmpGetInfo(battaryData, &width, &height, AK_NULL);
        Fwl_AkBmpDrawFromString(HRGB_LAYER, (Fwl_GetLcdWidth()-width)/2, (Fwl_GetLcdHeight()-height)/2, \
                    battaryData, &g_Graph.TransColor, AK_FALSE);
        Fwl_RefreshDisplay();
		
        Fwl_MiniDelay(1000);

        if (i == eRES_BMP_CHARGE_ANIMTN3)
            i = eRES_BMP_CHARGE_ANIMTN1 - 1;
    }
    
}

T_VOID PowerOff_Close(T_VOID)
{
	Fwl_StopTimer(pPowerParm->timer_off);
    pPowerParm->timer_off = ERROR_TIMER;

	AK_Feed_Watchdog(0);

    //close_keypadlight();
    Fwl_MiniDelay(500);

    Fwl_Print(C3, M_POWER, "*******power off******\n");

    if (gb.ResetAfterPowerOff)
    {
        Fwl_DisplayBacklightOff();
        Fwl_DisplayOff();
		
        gb.ResetAfterPowerOff = AK_FALSE;
        VME_Reset();//正常开机
    }
    else
    {
#ifdef OS_ANYKA
		
        if (Fwl_UseExternCharge())
        {
            PowerOff_ShowCharging();
        }
        else
#endif // OS_ANYKA
        {
            Fwl_DisplayBacklightOff();
            Fwl_DisplayOff();
            VME_Terminate();
        }
    }
	
    gb.PowerOffFlag = AK_FALSE;
}

static T_VOID PowerOff_HandleUserKey(T_EVT_PARAM *pEventParm)
{
	T_MMI_KEYPAD phyKey;
	
	phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
	phyKey.pressType = (T_PRESS_TYPE)pEventParm->c.Param2;
	
	if ((kbCLEAR != phyKey.keyID) && (PRESS_UP != phyKey.pressType))
	{
		//stop poweroff-audio if press anykey
		AK_PowerOnOffAudioStop(pPowerParm);
			
		if (pPowerParm->gif.bOpen)
		{
			PowerOnOff_CloseGif(&pPowerParm->gif);
		}
	
		if (ERROR_TIMER == pPowerParm->timer_off)
		{
			pPowerParm->timer_off = Fwl_SetTimerSecond(1, AK_TRUE);
		}
	}
}

void initpub_switch_off(void)
{
    T_U32 RtcCount = 0;
    T_U32 AlarmData = 0;
	T_U32 clock;

	MList_SuspendAll();
    TopBar_DisableShow();
    m_regResumeFunc(TopBar_DisableShow);

    Fwl_Print(C3, M_POWER, "power off SM");

    gb.PowerOffFlag = AK_TRUE;
    gb.PubMsgAllow = AK_FALSE;

    if(Fwl_GetBatteryVoltage() <= BATTERY_VALUE_MIN)
    {
        Fwl_MiniDelay(5);
    }

    pPowerParm = (T_POWER_PARM *)Fwl_Malloc(sizeof(T_POWER_PARM));
    AK_ASSERT_PTR_VOID(pPowerParm, "initpub_switch_off(): malloc error");
	
	memset(pPowerParm,0,sizeof(T_POWER_PARM));
	pPowerParm->timer_off = ERROR_TIMER;
	pPowerParm->gif.timer = ERROR_TIMER;
	
    PowerOnOff_PrintChar(csSWITCH_OFF);

    AudioPlayer_Stop();
    Fwl_AudioVolumeFree();

	PowerOnOff_PlayMedia(gs.PowerOffMedia, gs.PathPoffVideo, gs.PathPoffAudio, gs.PathPoffPic,
						pPowerParm, AK_PowerOffAVEndCB);
			
    if (!pPowerParm->videoPlay)
    {
        pPowerParm->timer_off = Fwl_SetTimerMilliSecond(1000, AK_TRUE);
    }

#ifndef SWITCHOFF_PLAYING_CHECKUSB
        StandbyFree();
#endif

//    PublicTimerStop();
    SetKeyLightCount(0);  //turn off key light

    /* set alarm clock to RTC_ALARM_MASK_REG */
    /**alarm will not happen if alarm happen when power off, so must delay the alarm
        if alarm happens during power off*/
    RtcCount = Fwl_RTCGetCount();
    AlarmData = GetAlarmDataMinValid(AK_FALSE);
	
    if ((AlarmData >= RtcCount)  && (AlarmData < (RtcCount + POWER_OFF_ALARM_DELAY)))
    {
        /**alarm is delay if alarm is conflict with power off*/
        clock = RtcCount + POWER_OFF_ALARM_DELAY;
        gs.AlarmDelay = AK_TRUE;
    }
    else
    {   
        clock = AlarmData;  //for new alarm clock
    }

    Fwl_SetAlarmRtcCount(clock); // set RTC_ALARM_MASK_REG
    gb.RtcType = RTC_ALARM;

	gs.sysbooterrstatus = AK_FALSE;//正常关机
	
    SaveUserdata();
}

void exitpub_switch_off(void)
{
    TopBar_EnableShow();
    pPowerParm = Fwl_Free(pPowerParm);

	//释放TV OUT 播放关机视频时的内存
	pGB_dispBuffBackup = Fwl_Free(pGB_dispBuffBackup);
}

void paintpub_switch_off(void)
{
    if( !pPowerParm->videoPlay )
        Fwl_RefreshDisplay();
}

unsigned char handlepub_switch_off(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    static T_U8 counter =0;
    
#ifdef SWITCHOFF_PLAYING_CHECKUSB
	if ((event == M_EVT_Z09COM_SYS_RESET)
		&& sys_usb_detect())
	{
       //stop poweroff-audio if press anykey
       	Fwl_Print(C3, M_POWER, "switch off,usb plug in");
	    if (pPowerParm->videoPlay)
	   	{
			AK_PowerOffVideoStop();
			pPowerParm->videoPlay = AK_FALSE;
		}	
	   
        AK_PowerOnOffAudioStop(pPowerParm);

        if (pPowerParm->gif.bOpen)
        {
			PowerOnOff_CloseGif(&pPowerParm->gif);
        }

		if (ERROR_TIMER != pPowerParm->timer_off)
		{
			Fwl_StopTimer(pPowerParm->timer_off);
        	pPowerParm->timer_off = ERROR_TIMER;
		}

		m_triggerEvent(M_EVT_EXIT, pEventParm);
		m_triggerEvent(event, pEventParm);

		return 0;
	}
#endif
	
    if (pPowerParm->videoPlay 
		&& (eReturn == Eng_PwrOnOff_Video_Handle(event, pEventParm)
			|| pPowerParm->playEnd) )
    {
        AK_PowerOffVideoStop();
    }    

    switch (event)
    {
    case M_EVT_USER_KEY:
		PowerOff_HandleUserKey(pEventParm);
        break;
        
    case VME_EVT_TIMER:
        if (pEventParm->w.Param1 == (T_U32)pPowerParm->timer_off
        	&& !pPowerParm->gif.bOpen
            && !pPowerParm->audioPlay
            && !pPowerParm->videoPlay)
        {
#ifdef USB_HOST
			if ((AK_TRUE == gb.bUDISKAvail) && (AK_NULL != gb.driverUDISK))
			{
				Fwl_Usb_DisconnectDisk();		
			}
#endif

			PowerOff_Close();
        }
		else if (pEventParm->w.Param1 == (T_U32)pPowerParm->gif.timer)
        {
            PowerOnOff_HandleGif(pPowerParm);
        }
        
        break;

    case M_EVT_PUB_TIMER:
        if (pPowerParm->audioPlay)
        {
            if(counter >= POFF_AUDIO_PLAY_TIME)
            {
                AK_PowerOnOffAudioStop(pPowerParm);
                if (pPowerParm->gif.bOpen)
        		{
					PowerOnOff_CloseGif(&pPowerParm->gif);
        		}
            }
            else
                counter++;
        }
        break;

    default:
        break;
    }

    return 0;
}

static T_VOID AK_PowerOffAVEndCB(T_END_TYPE endType)
{
    Fwl_Print(C3, M_POWER, "power off end cb");
    
    if (VIDEO_MEDIA == gs.PowerOffMedia)
    {
        pPowerParm->playEnd = AK_TRUE;
        Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
        Fwl_Print(C3, M_POWER, "END OF POWER OFF VIDEO!");
    }
    else if (AUDIO_MEDIA == gs.PowerOffMedia || PIC_AUDIO_MEDIA == gs.PowerOffMedia)
    {
        AK_PowerOnOffAudioStop(pPowerParm);
    }
}

static T_VOID AK_PowerOffVideoStop(T_VOID)
{
    if (pPowerParm->timer_off != ERROR_TIMER)
    {
        Fwl_StopTimer(pPowerParm->timer_off);
		pPowerParm->timer_off = ERROR_TIMER;
    }
	
    Eng_PwrOnOff_Video_Free();
    pPowerParm->videoPlay = AK_FALSE;

	// Trigger Event VME_EVT_TIMER to Exec PowerOff_Close()
    pPowerParm->timer_off = Fwl_SetTimerSecond(1, AK_TRUE);
}

/* end of files */
