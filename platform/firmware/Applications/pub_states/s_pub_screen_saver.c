
#include "Fwl_public.h"
#include "Fwl_pfDisplay.h"
#include "Eng_ScreenSave.h"
#include "Eng_AutoPowerOff.h"
#include "Eng_BatWarn.h"
#include "Eng_Alarm.h"
#include "Ctl_Fm.h"
#include "fwl_power.h"
#include "Fwl_pfAudio.h"
#include "Lib_state.h"
#include "Fwl_rtc.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Ctl_AudioPlayer.h"
#include "Fwl_waveout.h"
#include "Fwl_usb_host.h"
#include "svc_medialist.h"
#include "Ctl_RecAudio.h"
#include "Fwl_Power.h"
#ifdef SUPPORT_VISUAL_AUDIO	
#include "Log_MediaVisualAudio.h"
#endif

static T_BOOL gScreenSaverStatus = AK_FALSE;
static T_U32 WakeupType = WAKE_NULL;
extern T_S16			CurValue;
extern T_AUDIOPLAYER *p_audio_player;
static T_BOOL gIsEnableStandby = AK_FALSE;

static T_VOID pub_screen_saver_standby(T_VOID)
{
    if ( MPLAYER_PAUSE > MPlayer_GetStatus()
		&& Ctl_RecAudio_IsStoped())
    {
		if (WaveOut_IsOpen())
		{
			Fwl_AudioDisableDA();
		}	

#ifdef SUPPORT_VISUAL_AUDIO	
		if (VisualAudio_IsInit())
		{
			/**exit screen saver*/
			VME_ReTriggerEvent((vT_EvtSubCode)M_EVT_WAKE_SAVER, (T_U32)WAKE_NULL);
			return;
		}
#endif

		if (!(Fwl_UseExternCharge()
#ifdef OS_ANYKA			
#ifdef USB_HOST	 //连接中和已经连接成功USB HOST都不能进入standby
			|| (eM_s_usb_host == SM_GetCurrentSM()) \
			|| Fwl_UsbHostIsConnect()
#endif
#endif
			)&& (0 == (MEDIA_LIST_STATE_RUNNING & MList_State(eMEDIA_LIST_AUDIO)))
			&& (0 == (MEDIA_LIST_STATE_RUNNING & MList_State(eMEDIA_LIST_VIDEO)))
			)
	    {
			if (gIsEnableStandby)
			{
				Fwl_SetChipStandby();
				gIsEnableStandby = AK_FALSE;
			} 
		}
	}
}

/*---------------------- BEGIN OF STATE s_pub_screen_saver ------------------------*/
void initpub_screen_saver(void)
{
    //AK3631进screen save之前如果有被checkIn为一个最高频率的因子，则进standby后唤不醒。
    //因此暂时加一个较低频率的因子。待查明原因之后再去除此临时修改。
    //FreqMgr_StateCheckIn(FREQ_FACTOR_STANDBY, FREQ_PRIOR_HIGH);

	Fwl_Print(C3, M_PUBLIC, "screen saver entry.\n");

#ifdef SUPPORT_VISUAL_AUDIO	
	if(MPLAYER_PLAY == MPlayer_GetStatus()
	  && gs.bPlayer2Saver 
	  && gs.bVisualAudio)
	{
		VisualAudio_Init();
		TopBar_DisableShow();
	}
	else
	{
		Fwl_DisplayBacklightOff();
    	Fwl_DisplayOff();
	}
#else
    Fwl_DisplayBacklightOff();
    Fwl_DisplayOff();
#endif

    gScreenSaverStatus = AK_TRUE;

    WakeupType = WAKE_NULL;
    
    //FreqMgr_StateCheckIn(FREQ_FACTOR_SCR_SAVE, FREQ_PRIOR_SCR);
   	gIsEnableStandby = AK_TRUE;
	
	pub_screen_saver_standby();
}

void exitpub_screen_saver(void)
{
   // Fwl_Print(C3, M_PUBLIC, "exit1... , pll vale=%d\n" , get_pll_value());    
     //set_pll_value(124);
   // AFwl_Print(C3, M_PUBLIC, "exit2... , pll vale=%d\n" , get_pll_value());
#ifdef SUPPORT_VISUAL_AUDIO	
	if(VisualAudio_IsInit())
	{
		VisualAudio_Realease();
		TopBar_EnableShow();
	}
	else
	{
   		Fwl_DisplayOn(); 
	}
#else
	Fwl_DisplayOn(); 
#endif
	if(gs.bPlayer2Saver)
	  	Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
	
	Fwl_RefreshDisplay();	
	AK_Sleep(30);

    gScreenSaverStatus = AK_FALSE;

    if (WAKE_PWROFF == (WakeupType & WAKE_PWROFF)) //if auto power-off, need to exit standby mode
    {
        AutoPowerOffCountSet(0);        //power off immediately
    }

    if (WAKE_BATT_WARN == (WakeupType & WAKE_BATT_WARN))
    {
        BatWarnCountSet(0); // for low battery warning
    }
        /**cancel rtc for auto power off*/
        //modifyed for new alarm clock
    if (gb.RtcType != RTC_ALARM)
    {
        T_U32     alarm_time;
        alarm_time = GetAlarmDataMinValid(AK_TRUE);
        Fwl_SetAlarmRtcCount(alarm_time);
        gb.RtcType = RTC_ALARM;
    }

    //FreqMgr_StateCheckOut(FREQ_FACTOR_SCR_SAVE);
    
    //Fwl_DisplayOn();    //put forward
#ifdef SUPPORT_SYS_SET
    if(SM_GetCurrentSM() ==eM_s_set_disp_brightness)
    {
     	Fwl_SetBrightness(DISPLAY_LCD_0, (T_S8)CurValue);
    }
	else
#endif
	{
		Fwl_DisplayBacklightOn(gs.LcdBrightness);
	}
	
	Set_key_valid(AK_TRUE);
	
    Fwl_Print(C3, M_PUBLIC, "screen saver exit.\n");
}

void paintpub_screen_saver(void)
{
#ifdef SUPPORT_VISUAL_AUDIO	
	if (VisualAudio_IsInit())
		Fwl_RefreshDisplay();
	
	return;
#endif
}

unsigned char handlepub_screen_saver(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U32 EventType = WAKE_NULL;

#ifdef SUPPORT_VISUAL_AUDIO	
	if (EVT_VISUAL_REFRESH == event)
		return 0;
#endif

    if (event != M_EVT_Z06COM_SCREEN_SAVER
		&& event != M_EVT_RTC
		&& IsPostProcessEvent(event))
    {
        return 1;
    }

    switch (event)
    {
    case M_EVT_WAKE_SAVER:
        WakeupType |= pEventParm->w.Param1;
        
        UserCountDownReset();
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
		
#ifdef SUPPORT_VISUAL_AUDIO	
	case M_EVT_TOUCH_SCREEN:
		if (!VisualAudio_IsInit())
			break;
#endif	
    case M_EVT_USER_KEY:
    //case M_EVT_TOUCH_SCREEN:
        WakeupType |= WAKE_GPIO;

        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
//  case M_EVT_1:
//      FreqMgr_SetChipFreq(DEFAULT_SLEEP_CLOCK);
    default:
        break;
    }

    if (M_EVT_RTC == event)
    {
        WakeupType |= pEventParm->w.Param1;
        
        UserCountDownReset();
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        
        if (!gb.bAlarming)
        {
            EventType = RTC_ALARM;   
            VME_ReTriggerEvent((vT_EvtSubCode)M_EVT_RTC, (T_U32)(&EventType));
        }
        
        return 0;
    }
	
    pub_screen_saver_standby();

//   Fwl_Print(C3, M_PUBLIC, "Leave Handle Screen saver event---------\n");

    return 0;
}

T_BOOL AK_GetScreenSaverStatus(T_VOID)
{
    return gScreenSaverStatus;
}
