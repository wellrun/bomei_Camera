
#include "Fwl_public.h"
#include "Eng_ScreenSave.h"
#include "Ctl_Msgbox.h"
#include "Eng_Alarm.h"
#include "Ctl_AudioPlayer.h"
#include "Fwl_Initialize.h"
#include "Ctl_Fm.h"
#include "Fwl_pfAudio.h"
#include "log_mediaplayer.h"
#include "Eng_AkBmp.h"
#include "Fwl_pfdisplay.h"
#include "Fwl_rtc.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Fwl_WaveOut.h"

typedef struct {
    T_MSGBOX        msgbox;
    T_pCDATA        SoundData;
    T_BOOL          ContinueAlarm;
    T_U32           audio_playsecond;
    T_RECT          MsgRect;
    T_pDATA         pMsgScreen;
    T_BOOL          bRefreshFlag;    
} T_ALARM_PARM;

static T_ALARM_PARM *pAlarmParm = AK_NULL;
static T_BOOL ScreenSaverSwitch = AK_TRUE;

extern T_pDATA p_menu_bckgrnd;

static T_VOID PubAlm_BackupDispMem(T_VOID)
{
	T_pDATA pDisBuf = AK_NULL;
	T_S16 x, y;
    int iIndex = 0;
    int k = 0;
	
#ifdef LCD_MODE_565
	pAlarmParm->pMsgScreen = (T_U8 *)Fwl_Malloc(pAlarmParm->MsgRect.width*pAlarmParm->MsgRect.height*2 + 32);
	AK_ASSERT_PTR_VOID(pAlarmParm->pMsgScreen, "initpub_alarm(): pMsgScreen malloc error");
	
	pDisBuf = (T_U8 *)Fwl_GetDispMemory565();
	for(x = pAlarmParm->MsgRect.top; x < pAlarmParm->MsgRect.top + pAlarmParm->MsgRect.height; x++)
	{
		iIndex = (x * Fwl_GetLcdWidth() + pAlarmParm->MsgRect.left) * 2;
		for(y = pAlarmParm->MsgRect.left; y < pAlarmParm->MsgRect.left + pAlarmParm->MsgRect.width; y++)
		{
			pAlarmParm->pMsgScreen[k++] = pDisBuf[iIndex++];
			pAlarmParm->pMsgScreen[k++] = pDisBuf[iIndex++];
		}
	}
#else // lcd_mode_888
	pAlarmParm->pMsgScreen = (T_U8 *)Fwl_Malloc(pAlarmParm->MsgRect.width*pAlarmParm->MsgRect.height*3 + 32);
	AK_ASSERT_PTR_VOID(pAlarmParm->pMsgScreen, "initpub_alarm(): pMsgScreen malloc error");
	
	pDisBuf = (T_U8 *)Fwl_GetDispMemory();
	for(x = pAlarmParm->MsgRect.top; x < pAlarmParm->MsgRect.top + pAlarmParm->MsgRect.height; x++)
	{
		iIndex = (x * Fwl_GetLcdWidth() + pAlarmParm->MsgRect.left) * 3;
		for(y = pAlarmParm->MsgRect.left; y < pAlarmParm->MsgRect.left + pAlarmParm->MsgRect.width; y++)
		{
			pAlarmParm->pMsgScreen[k++] = pDisBuf[iIndex++];
			pAlarmParm->pMsgScreen[k++] = pDisBuf[iIndex++];
			pAlarmParm->pMsgScreen[k++] = pDisBuf[iIndex++];
		}
	}
#endif
}

static T_VOID PubAlm_RestoreDispMem(T_VOID)
{
	T_pDATA pDisBuf = AK_NULL;
	T_S16 x, y;
    int iIndex = 0;
    int k = 0;
	
#ifdef LCD_MODE_565
	pDisBuf = (T_U8 *)Fwl_GetDispMemory565();

	for(x = pAlarmParm->MsgRect.top; x < pAlarmParm->MsgRect.top + pAlarmParm->MsgRect.height; x++)
	{
		iIndex = (x * Fwl_GetLcdWidth() + pAlarmParm->MsgRect.left) * 2;
		for(y = pAlarmParm->MsgRect.left; y < pAlarmParm->MsgRect.left + pAlarmParm->MsgRect.width; y++)
		{
			pDisBuf[iIndex++] = pAlarmParm->pMsgScreen[k++];
			pDisBuf[iIndex++] = pAlarmParm->pMsgScreen[k++];
		}
	}
#else // lcd_mode_888
	pDisBuf = (T_U8 *)Fwl_GetDispMemory();

	for(x = pAlarmParm->MsgRect.top; x < pAlarmParm->MsgRect.top + pAlarmParm->MsgRect.height; x++)
	{
		iIndex = (x * Fwl_GetLcdWidth() + pAlarmParm->MsgRect.left) * 3;
		for(y = pAlarmParm->MsgRect.left; y < pAlarmParm->MsgRect.left + pAlarmParm->MsgRect.width; y++)
		{
			pDisBuf[iIndex++] = pAlarmParm->pMsgScreen[k++];
			pDisBuf[iIndex++] = pAlarmParm->pMsgScreen[k++];
			pDisBuf[iIndex++] = pAlarmParm->pMsgScreen[k++];
		}
	}
#endif
}

static T_BOOL PubAlm_PlayIsEnable(T_VOID)
{
    if (!Fwl_AudioIsPlaying())
    {
        return AK_TRUE;
    }
    else
    {
    	AK_DEBUG_OUTPUT("PUBALM: Audio Channel Is Busy.\n");
        return AK_FALSE;
    }
}

static void PubAlm_Show(void)
{
    //refresh background image 
    if(pAlarmParm->bRefreshFlag)
    {      
        if(p_menu_bckgrnd != AK_NULL)
        {
            Fwl_AkBmpDrawFromString(HRGB_LAYER, 0, 0, p_menu_bckgrnd,  AK_NULL, AK_FALSE);
        }
        else
        {
            AK_DEBUG_OUTPUT("PUBALM:	PubAlm_Show() backgrand image is null");
        }        
    }    

    MsgBox_Show(&pAlarmParm->msgbox);

    Fwl_RefreshDisplay();

    pAlarmParm->bRefreshFlag = AK_FALSE;
}

/*---------------------- BEGIN OF STATE s_pub_alarm ------------------------*/
void initpub_alarm(void)
{    
	T_USTR_INFO utmpstr;

    AK_DEBUG_OUTPUT("PUBALM:	RTC alarm!!\n");

    gb.bAlarming = AK_TRUE;
    pAlarmParm = (T_ALARM_PARM *)Fwl_Malloc(sizeof(T_ALARM_PARM));
    AK_ASSERT_PTR_VOID(pAlarmParm, "initpub_alarm(): malloc error");

    pAlarmParm->pMsgScreen 		= AK_NULL;
    pAlarmParm->ContinueAlarm 	= AK_TRUE;
    ScreenSaverSwitch 			= ScreenSaverIsOn();

    /* output message box to select delay or not */
    utmpstr[0] = 0;
    MsgBox_InitStr(&pAlarmParm->msgbox, 0, GetCustomTitle(ctHINT), utmpstr, MSGBOX_QUESTION | MSGBOX_YESNO);

    Utl_UStrCpy(utmpstr, GetCustomString(csCLK_ALARM));
    MsgBox_AddLine(&pAlarmParm->msgbox, utmpstr);
    Utl_UStrCpy(utmpstr, GetCustomString(csCLK_DELAY));
    MsgBox_AddLine(&pAlarmParm->msgbox, utmpstr);
    MsgBox_GetRect(&pAlarmParm->msgbox, &pAlarmParm->MsgRect);  
    pAlarmParm->bRefreshFlag = AK_TRUE;
    MsgBox_SetRefresh(&pAlarmParm->msgbox, CTL_REFRESH_ALL);
//    pAlarmParm->pData = Res_StaticLoad(AK_NULL, eRES_BMP_MENU_BACKGROUND, &imgLen);      
    Menu_LoadRes();
        
    PubAlm_Show();    

    AudioPlayer_Stop();
    WaveOut_CloseFade();
    
    AK_DEBUG_OUTPUT("PUBALM:	SCREEN SAVER: %d\n", ScreenSaverSwitch);
    if(ScreenSaverSwitch)
    {
        ScreenSaverDisable();
    }

    if (PubAlm_PlayIsEnable())
    {
		AK_DEBUG_OUTPUT("PUBALM:	Init Alarm\n");
        PlayAlarmSound();
        //Fwl_SetAudioPlayStatus(AK_FALSE);       //don't set to playing state when alarming
    }

    pAlarmParm->audio_playsecond = ALARM_RING_TIME;

	PubAlm_BackupDispMem();
}

void exitpub_alarm(void)
{
	AK_DEBUG_OUTPUT("PUBALM:	Exit Pub Alarm.\n");
	
    /* refresh last screen before exit */
    if (!gs.LatestIsDayAlarm \
        && ((Fwl_RTCGetCount() % ONE_DAY_SECOND) >= gs.AlarmClock.DayAlarm) \
        && (gs.AlarmClock.DayAlarm > gs.AlarmClock.WeekAlarm))
    {
        if (gs.AlarmClock.DayAlarmType == DAY_ALARM_ONCE)
        {
            gs.AlarmClock.DayAlarmType = DAY_ALARM_CLOSE;
        }
    }

    if (pAlarmParm->pMsgScreen != AK_NULL)
    {
        PubAlm_RestoreDispMem();
        pAlarmParm->pMsgScreen = Fwl_Free(pAlarmParm->pMsgScreen);
    }

	if (pAlarmParm->ContinueAlarm == AK_TRUE)
	{
        AlarmDelayProc(ALARM_INTERVAL_TIME);    //delay 3min
	}
	else
	{
        AlarmPostProc();
    }

    if (GetAlarmPlayStatus())
    {
        StopPlayAlarmSound();
    }

	Fwl_AudioDisableDA();	

    if(ScreenSaverSwitch)
        ScreenSaverEnable();

    gb.bAlarming = AK_FALSE;

    // Before leaving, it should refresh LCD's buffer to previous contents.
    // Calling Fwl_RefreshDisplay() to refresh the buffer of 565.
    //Fwl_RefreshDisplay();    

    pAlarmParm = Fwl_Free(pAlarmParm);
}

void paintpub_alarm(void)
{
    PubAlm_Show();
}

unsigned char handlepub_alarm(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    if (M_EVT_ALARM == event		// M_EVT_RTC
		&& gs.LatestIsDayAlarm) 	// day alarm power on
    {
        AK_DEBUG_OUTPUT("PUBALM:	*****It's Day Alarm*****\n");
        if (gs.AlarmClock.DayAlarmType == DAY_ALARM_ONCE)
        {
            gs.AlarmClock.DayAlarmType = DAY_ALARM_CLOSE;
			pAlarmParm->ContinueAlarm = AK_FALSE;
            AK_DEBUG_OUTPUT("PUBALM:	DayAlarmType: %d.\n", gs.AlarmClock.DayAlarmType);
        }
    }    

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pAlarmParm->msgbox, CTL_REFRESH_ALL);
        pAlarmParm->bRefreshFlag = AK_TRUE;
        return 1;
    }

    if (!Fwl_AudioIsPlaying() && !GetAlarmPlayStatus())
    {
		AK_DEBUG_OUTPUT("PUBALM:	handle Alarm\n");
        PlayAlarmSound();
    }

#ifdef CAMERA_SUPPORT
    if (gb.VideoIsRecording)
    {
        if (0 == pAlarmParm->audio_playsecond--)
        {
            pAlarmParm->ContinueAlarm = AK_FALSE;
            m_triggerEvent(M_EVT_EXIT, pEventParm);
        }
    }
    else
#endif    
    if (event == M_EVT_PUB_TIMER
		&& 0 == pAlarmParm->audio_playsecond--)
    {
        m_triggerEvent(M_EVT_EXIT, pEventParm);
    }     

	// Handle Message Box
    switch(MsgBox_Handler(&pAlarmParm->msgbox, event, pEventParm))
    {
    case eNext:
        pAlarmParm->ContinueAlarm = AK_FALSE;
		
    case eReturn:
        pAlarmParm->bRefreshFlag = AK_TRUE;
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;

    default:
        break;
    }

    return 0;
}

/*
 * End of File
 */
