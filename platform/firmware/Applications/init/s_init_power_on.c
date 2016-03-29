#include "Fwl_public.h"
#include "Fwl_Initialize.h"
#include "Fwl_osFS.h"
#include "Fwl_oscom.h"
#include "Fwl_pfAudio.h"
#include "Fwl_Image.h"
#include "Eng_Jpeg2Bmp.h"
#include "Eng_ImgConvert.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "lib_image_api.h"
#include "Eng_ImgDec.h"
#include "Fwl_Initialize.h"
#include "Ctl_AVIPlayer.h"
#include "Eng_AkBmp.h"
#include "Eng_DataConvert.h"
#include "fwl_keyhandler.h"
#include "fwl_osfs.h"
#include "Log_MediaPlayer.h"
#include "fwl_waveout.h"
#include "Eng_pwr_onoff_video.h"
#include "fwl_osfs.h"
#include "Lib_state_api.h"
#include "Fwl_pfdisplay.h"
#include "eng_imgconvert.h"
#include "Fwl_display.h"

#ifdef TOUCH_SCR
#include "fwl_tscrcom.h"
#endif


#define PON_IMG_SHOW_TIME       2   //seconds
#define PON_AUDIO_PLAY_TIME     10  //seconds


static T_POWER_PARM *pPoweronParm = AK_NULL;

extern T_pDATA pGB_dispBuffBackup;

static T_VOID AK_PowerOnVideoStop(T_VOID);
static T_VOID AK_PowerOnAVEndCB(T_END_TYPE endType);

static T_U8 PowerOn_EnterAlarm(T_EVT_PARAM *pEventParm)
{
	Fwl_Print(C3, M_POWER, "handleinit_power_on pPoweronParm->timer_id = %d, gs.PowerOnMedia = %d.\n", pPoweronParm->gif.timer, gs.PowerOnMedia);

	if (pPoweronParm->gif.bOpen)
    {
        PowerOnOff_CloseGif(&pPoweronParm->gif);     
    }        
    
    if (gs.PowerOnMedia == VIDEO_MEDIA)
    {
        AK_PowerOnVideoStop();
        // Fwl_FillSolid(HRGB_LAYER,COLOR_BLACK);
		// Fwl_RefreshDisplay();
    }
    else
    {
        AK_PowerOnOffAudioStop(pPoweronParm);
    }    
    
    m_triggerEvent(M_EVT_ALARM, pEventParm);        
    return 0;
}

static T_U8 PowerOn_EnterUsbDisk(T_EVT_PARAM *pEventParm)
{
	if (pPoweronParm->gif.timer != ERROR_TIMER)
    {
        if (gs.PowerOnMedia == VIDEO_MEDIA)
            AK_PowerOnVideoStop();
        else
        {
            PowerOnOff_CloseGif(&pPoweronParm->gif);
            AK_PowerOnOffAudioStop(pPoweronParm);
        }
    }
    
    m_triggerEvent(M_EVT_NEXT, pEventParm);
    return 0;

}

static T_VOID PowerOn_HandleUserKey(T_EVT_PARAM *pEventParm)
{
	T_MMI_KEYPAD phyKey;
	
	phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
	phyKey.pressType = (T_PRESS_TYPE)pEventParm->c.Param2;
	
	if (kbCLEAR != phyKey.keyID && PRESS_UP != phyKey.pressType)
	{
		if (pPoweronParm->audioPlay)
		{
			AK_PowerOnOffAudioStop(pPoweronParm);
		}
	
		if( pPoweronParm->videoPlay )
		{
			AK_PowerOnVideoStop();
	    }
			
		if (pPoweronParm->gif.bOpen)
		{
			PowerOnOff_CloseGif(&pPoweronParm->gif);
		}
	
		m_triggerEvent(M_EVT_NEXT, pEventParm);
	}
}


/*---------------------- BEGIN OF STATE s_init_power_on ------------------------*/
void initinit_power_on(void)
{
    Fwl_Print(C3, M_POWER, "power_on SM"); //"power on state. audio: %d, %s\n", gs.PonAudio, gs.PathPonAudio);
	
    pPoweronParm = (T_POWER_PARM *)Fwl_Malloc(sizeof(T_POWER_PARM));
    AK_ASSERT_PTR_VOID(pPoweronParm, "initinit_power_on(): malloc error");
	
	memset(pPoweronParm, 0, sizeof(T_POWER_PARM));
    pPoweronParm->gif.timer	= ERROR_TIMER;

#ifdef TOUCH_SCR
		if (0 != gs.matrixPtr.Y[2] || 0 != gs.matrixPtr.X[2])
		{
			// TSCR Is Calibrated, Enable TSCR
			Fwl_EnableTSCR();		
		}
#endif

    if (gb.bRtcPwrOn)
    {
		// power on by rtc, then show alarm
        return;
    }

	PowerOnOff_PlayMedia(gs.PowerOnMedia, gs.PathPonVideo, gs.PathPonAudio, gs.PathPonPic,
						pPoweronParm, AK_PowerOnAVEndCB);
}

void exitinit_power_on(void)
{
	AK_PowerOnOffAudioStop(pPoweronParm);

	if (pPoweronParm->gif.bOpen)
    {
        PowerOnOff_CloseGif(&pPoweronParm->gif);     
    } 

    pPoweronParm = Fwl_Free(pPoweronParm);

	//释放TV OUT 播放开机视频时的内存
	pGB_dispBuffBackup = Fwl_Free(pGB_dispBuffBackup);
}

void paintinit_power_on(void)
{
    if(!pPoweronParm->videoPlay)
        Fwl_RefreshDisplay();
}

unsigned char handleinit_power_on(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    static T_BOOL PonVideoPause = AK_FALSE;
	
    // do not use "IsPostProcessEvent()" function here
    // if use this function, the program will be dead
    
    if (M_EVT_1 == event && EVT_RTC_TRIGGER == pEventParm->c.Param8)
    {       
        return PowerOn_EnterAlarm(pEventParm);
    }

	/** if usb device plug in*/
    if (event == M_EVT_Z09COM_SYS_RESET && EVT_USB_PLUG_IN == pEventParm->c.Param8)
    {
		return PowerOn_EnterUsbDisk(pEventParm);
    }

    if (PonVideoPause)
    {
        PonVideoPause = AK_FALSE;
        Eng_PwrOnOff_Video_Resume();
        return 0;
    }
	
#if 0
    if (event == M_EVT_Z09COM_SYS_RESET)
    {
        if ((EVT_SD_PLUG_IN == pEventParm->c.Param1) || (EVT_SD_PLUG_OUT == pEventParm->c.Param1))
        {
           Fwl_AutoMountSD(AK_FALSE);
        }
    }
#endif    
    
    if (pPoweronParm->videoPlay)
    {
        if (eReturn == Eng_PwrOnOff_Video_Handle(event, pEventParm)
			|| pPoweronParm->playEnd)
        {
			AK_PowerOnVideoStop();
			
            m_triggerEvent(M_EVT_NEXT, pEventParm);
            return 1;
        }
    }

    if (pPoweronParm->videoPlay || pPoweronParm->audioPlay)
    {
		if ( M_EVT_PUB_TIMER == event )
		{
			static T_S32 s_timercount = 0;

			if ( s_timercount++ > PONOFF_VIDEO_PLAY_TIME )
			{
				s_timercount = 0;

				if ( pPoweronParm->videoPlay )
				{
					AK_PowerOnVideoStop();
				}

				if ( pPoweronParm->audioPlay )
				{
					AK_PowerOnOffAudioStop(pPoweronParm);
				}

				m_triggerEvent( M_EVT_NEXT, pEventParm );

				return 1;
			}
		}
    }

    switch (event)
    {
    case VME_EVT_TIMER:
        if (pEventParm->w.Param1 == (T_U32)pPoweronParm->gif.timer)
        {
            PowerOnOff_HandleGif(pPoweronParm);
        }
        break;



#ifdef TOUCH_SCR
	case M_EVT_TOUCH_SCREEN:
	{
		if(eTOUCHSCR_UP == pEventParm->s.Param1)
		{
			if (pPoweronParm->audioPlay
				&& (AUDIO_MEDIA == gs.PowerOnMedia || PIC_AUDIO_MEDIA == gs.PowerOnMedia))
			{
				AK_PowerOnOffAudioStop(pPoweronParm);
				Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
				Fwl_Print(C3, M_POWER, "END OF POWER ON AUDIO 2!");					
				m_triggerEvent(M_EVT_NEXT, pEventParm);
			}
		}			
		break;
	}
#endif   

    case M_EVT_USER_KEY:
        PowerOn_HandleUserKey(pEventParm);
        break;
		
    default:
        break;
    }

	if (!pPoweronParm->gif.bOpen && !pPoweronParm->videoPlay && !pPoweronParm->audioPlay)
	{
		m_triggerEvent(M_EVT_NEXT, pEventParm);
	}

    return 0;
}


static T_VOID AK_PowerOnAVEndCB(T_END_TYPE endType)
{
    Fwl_Print(C3, M_POWER, "power on end cb\n");
	
    if (VIDEO_MEDIA == gs.PowerOnMedia)
    {
        pPoweronParm->playEnd = AK_TRUE;
		
        Fwl_Print(C3, M_POWER, "END OF POWER ON VIDEO!\n");        
    }
    else if (AUDIO_MEDIA == gs.PowerOnMedia || PIC_AUDIO_MEDIA == gs.PowerOnMedia)
    {   
    	m_triggerEvent(M_EVT_NEXT, AK_NULL);
    }
}

T_VOID AK_PowerOnOffAudioStop(T_POWER_PARM *pPower)
{
    if (pPower && pPower->audioPlay)
    {
		Fwl_Print(C3, M_POWER, "AK_PowerOnOffAudioStop\n");
		
		WaveOut_SetFade(200, FADE_STATE_OUT);
		AK_Sleep(40);
		
        MPlayer_Close();
        
		Fwl_AudioDisableDA();

        pPower->audioPlay = AK_FALSE;
    }
}

static T_VOID AK_PowerOnVideoStop(T_VOID)
{
    Eng_PwrOnOff_Video_Free();
    pPoweronParm->videoPlay = AK_FALSE;
}

/* end of files */
