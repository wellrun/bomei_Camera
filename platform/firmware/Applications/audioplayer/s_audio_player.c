
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Ctl_AudioPlayer.h"
#include "Ctl_APlayerList.h"
#include "Eng_Lyric.h"
#include "Fwl_oscom.h"
#include "Fwl_pfKeypad.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Fwl_osFS.h"
#include "Eng_TopBar.h"
#include "Lib_state.h"
#include "Eng_AkBmp.h"
#include "eng_dataconvert.h"
#include "Ctl_msgbox.h"
#include "lib_image_api.h"
#include "Eng_Graph.h"
#include "ctl_fm.h"
#include "fwl_pfdisplay.h"
#include "fwl_pfaudio.h"
#include "fwl_osfs.h"
#include "Lib_state_api.h"
#include "Fwl_waveout.h"
#include "fwl_display.h"
#include "Eng_ImgDec.h"
#include "Eng_ImgConvert.h"

#define     AUDIO_NAME_TAILSPACE            2

#define     AUDIOPLAYER_REFRESH_TIME        500

#define     HORIZONTAL_DISTANCE             10

#if (LCD_CONFIG_WIDTH == 800)   	 // for LCD 800 X 480
#define     VERTICAL_DISTANCE               4
#else 
#if (LCD_CONFIG_WIDTH == 480)   	// for LCD 480 X 272
#define     VERTICAL_DISTANCE               4
#else 								// for LCD 320 X 240
#define     VERTICAL_DISTANCE               2
#endif
#endif

#if (LCD_CONFIG_WIDTH == 800)   	 // for LCD 800 X 480
#define     BUTTON_SPACE                    10 
#else 
#if (LCD_CONFIG_WIDTH == 480)   	// for LCD 480 X 272
#define     BUTTON_SPACE                    10      
#else                           	 // for LCD 320 X 240
#define     BUTTON_SPACE                    1
#endif
#endif


#define     TIME_RECT_WIDTH                 55
#define     CURTIME_HEIGHT                  16

#define     TOTALTIME_HEIGHT                16
#define     LYRIC_HEIGHT                    (16 << 1)

#define     SMALL_SOUND_LEFT_POS            8
#define     SMALL_SOUND_TOP_POS             183
#define     BIG_SOUND_LEFT_POS              297
#define     BIG_SOUND_TOP_POS               183
#define     SOUND_BAR_LEFT_POS              26
#define     SOUND_BAR_TOP_POS               180

#define     MIN_VOLUME_NUM_LEFT_POS         10
#define     MIN_VOLUME_NUM_TOP_POS          204
#define     CUR_VOLUME_NUM_LEFT_POS         151
#define     CUR_VOLUME_NUM_TOP_POS          204
#define     MAX_VOLUME_NUM_LEFT_POS         292
#define     MAX_VOLUME_NUM_TOP_POS          204

#define     NAME_RECT_LEFT_SPACE            24
#define     NAME_SCROLL_TAIL_SPACE          6

#define     AUDIOPLAYER_REFRESH_NONE                    0x00000000
#define     AUDIOPLAYER_REFRESH_ALL                     0xffffffff
#define     AUDIOPLAYER_LR_REFRESH                      0x00000001
#define     AUDIOPLAYER_AUDITION_REFRESH                0x00000002
#define     AUDIOPLAYER_CYCLE_REFRESH                   0x00000004
#define     AUDIOPLAYER_AB_REFRESH                      0x00000008
#define     AUDIOPLAYER_SPEED_REFRESH                   0x00000010
#define     AUDIOPLAYER_PROGRESS_REFRESH                0x00000020
#define     AUDIOPLAYER_CURTIME_REFRESH                 0x00000040
#define     AUDIOPLAYER_TOTALTIME_REFRESH               0x00000080
#define     AUDIOPLAYER_VOLUME_REFRESH                  0x00000100
#define     AUDIOPLAYER_NAME_REFRESH                    0x00000200
#define     AUDIOPLAYER_LYRIC_REFRESH                   0x00000400
#define     AUDIOPLAYER_PLAY_REFRESH                    0x00000800
#define     AUDIOPLAYER_STOP_REFRESH                    0x00001000
#define     AUDIOPLAYER_PREVIOUS_REFRESH                0x00002000
#define     AUDIOPLAYER_NEXT_REFRESH                    0x00004000
#define     AUDIOPLAYER_VOLBTN_REFRESH                  0x00008000
#define     AUDIOPLAYER_CTLPANEL_REFRESH                0x00010000
#define     AUDIOPLAYER_PIC_REFRESH                		0x00020000


#define     MILLISECOND_PER_HOUR                        3600000
#define     MILLISECOND_PER_MINUTE                      60000
#define     MILLISECOND_PER_SECOND                      1000

typedef enum {
    AUDIO_PLAYER_TRACK_LEFT = 0,
    AUDIO_PLAYER_TRACK_RIGHT,
    AUDIO_PLAYER_TRACK_STEREO,
    AUDIO_PLAYER_TRACK_NUM
}T_AUDIO_PLAYER_TRACK;

/** audio player resource and rect */
typedef struct {
    T_RECT                  PlayerRect;     // mp3 player up background rect

    T_RECT                  PanelRect;      //mp3 control panel rect
    T_RECT                  PlayRect;       //mp3 play/pause button rect
    T_RECT                  StopRect;       //mp3 stop button rect
    T_RECT                  PreviousRect;   //mp3 previous button rect
    T_RECT                  NextRect;       //mp3 next button rect
    T_RECT                  VolBtnRect;     //mp3 volume/mute button rect
    
    T_RECT                  LRIconRect;
    T_RECT                  AuditionRect;   // mp3 player prep listen status rect
    T_RECT                  CycleRect;      // mp3 player cycle mode rect
    T_RECT                  ABRect;
    T_RECT                  SpeedRect;

    T_RECT                  ProgBarRect;
    T_RECT                  BarRect;
    T_RECT                  CurTimeRect;
    T_RECT                  TotalTimeRect;
    T_RECT                  LyricRect;
    T_RECT                  SmallSoundRect;
    T_RECT                  BigSoundRect;
    T_RECT                  SoundBarRect;
    T_RECT                  NameRect;
    T_RECT                  VolRct;
    
    T_pCDATA                pLR[3];
    T_pCDATA                pAudition[2];
    T_pCDATA                pCycle[4];      // mp3 player music cycle mode image
    T_pCDATA                pAB[3];         // mp3 player repeat status image
    T_pCDATA                pSpeed[11];     // mp3 player repeat status image
    T_pCDATA                pBarBack;
    T_pCDATA                pBar[15];
    T_pCDATA                pSmallSound;
    T_pCDATA                pBigSound;
    T_pCDATA                pBckgrnd;
    T_pCDATA                pPlay[2];       //mp3 player Play/Pause button image
    T_pCDATA                pVolume[2];     //mp3 player Volume/Mute button image
    T_pCDATA                pStop;          //mp3 player Stop button image
    T_pCDATA                pPrevious;      //mp3 player Previous button image
    T_pCDATA                pNext;          //mp3 player Next button image
    T_pCDATA                pPanel;         //mp3 player Control Panel image

    T_BOOL                  ProgRfrshAll;
    T_U32                   LastPlayTime;
}T_AUDIO_PLAYER_RES;

typedef struct {
    T_U16                   initFlag;       /* identify the control is initialized or not */
    T_BOOL                  bScroll;              /**AK_TRUE: enable scroll text */
    T_USTR_INFO             UText;          /*unicode text*/
    T_pDATA                 bmpData;       /* bmp data in anyka format */
    T_POINT                 point;                /**diplaying position*/
    T_RECT                  range;         /* location of title */
    T_U16                   chrBegin;       /* The first shown character's ID, this variable for scroll title */
    T_U16                   UTextLen;        /* text length */
    //T_U16                   DispTextMax;    /* maximum char number displayed*/
    T_S16                   TmCount;        /* time count for scroll the text automatically */
} T_AUDIO_NAME;

typedef struct {
    T_LYRIC_STRUCT          Lyric;          //audio's lyric
    T_BOOL                  LyricShwFlg;            //show lyric flag
    T_RECT                  LyricRect;
    T_AUDIO_PLAYER_RES      Resource;
    T_TIMER                 RefreshTimer;   //the id of refresh timer
    T_U32                   Refresh;
    T_U32                   ChngVlmFlg;      // in change volume flag
    T_AUDIO_NAME            AudioName;
    T_MSGBOX                msgbox;
#ifdef UI_USE_ICONMENU
    T_BOOL                  First;
#endif
	T_U32					InitEvent;		// This SM Initializing Event

	T_RECT                  ImgRect;
	T_USTR_FILE				filepath;
} T_AUDIO_PLAYER_PARM;

static T_AUDIO_PLAYER_PARM *pAudio_Player_Parm = AK_NULL;
static T_U16 FileNameOffset = 0;

static T_VOID AdPlyr_SetResRect(T_AUDIO_PLAYER_RES *pRes);
static T_VOID AdPlyr_GetRes(T_AUDIO_PLAYER_RES *pResource);
static T_BOOL AdPlyr_Show(T_VOID);
static T_BOOL AdPlyr_SetRefresh(T_U32 refresh);
static T_U32 AdPlyr_GetRefresh(T_VOID);
static T_BOOL AdPlyr_StartRefreshTimer(T_VOID);
static T_BOOL AdPlyr_StopRefreshTimer(T_VOID);
static T_BOOL AdPlyr_ShowTotalTime(T_AUDIO_PLAYER_RES *pResource);
static T_BOOL AdPlyr_ShowCurTime(T_AUDIO_PLAYER_RES *pResource);
static T_VOID AdPlyr_RefreshCallBack(T_VOID);
static T_VOID resume_audio_player(T_VOID);
static T_VOID suspend_audio_player(T_VOID);
static T_BOOL AdPlyr_GetLyric(T_pCWSTR pFilePath);
static T_BOOL AdPlyr_ShowLyric(T_AUDIO_PLAYER_RES *pResource);
static T_BOOL AdPlyr_ShowBckgrnd(T_AUDIO_PLAYER_RES *pResource);
static T_BOOL AdPlyr_ShowVolumeNum(T_AUDIO_PLAYER_RES *pRes);
static T_VOID AdPlyr_SetNameInfoCallBack(T_VOID);
static T_BOOL AdPlyr_ShowProgress(T_AUDIO_PLAYER_RES *pResource);

static T_MMI_KEYPAD AdPlyr_HitButtonCallBack(T_POS x, T_POS y, T_EVT_PARAM *pEventParm);
extern T_BOOL AudioPlayer_IsSupportSeekType(T_eMEDIALIB_MEDIA_TYPE media_type);
#endif


/*---------------------- BEGIN OF STATE s_mp3_player ------------------------*/
void initaudio_player(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_AUDIOPLAYER_INIT_RET AudioInitRet = AUDIOPLAYER_INIT_ERROR;
    T_pCWSTR    pFilePath = AK_NULL;
    T_USTR_INFO uText;  /*unicode text*/

	//AK_DEBUG_OUTPUT("initaudio_player 1 MIC_TO_HP=%d,0x0800005c=0x%x",(*(volatile T_U32 *)0x0800005c) & 0x40000,(*(volatile T_U32 *)0x0800005c));//xuyr debug Swd200001023
    gb.bAudioPlaySM = AK_TRUE;

    Utl_UStrCpy(uText, Res_GetStringByID(eRES_STR_PUB_MUSIC_PLAYER));

    //FreqMgr_StateCheckIn(FREQ_FACTOR_AUDIO, FREQ_PRIOR_AUDIO);
	
    pAudio_Player_Parm = (T_AUDIO_PLAYER_PARM *)Fwl_Malloc(sizeof(T_AUDIO_PLAYER_PARM));
    AK_ASSERT_PTR_VOID(pAudio_Player_Parm, "initaudio_player(): malloc error");
    memset((void *)pAudio_Player_Parm, 0x0, sizeof(T_AUDIO_PLAYER_PARM));

    (&pAudio_Player_Parm->Resource)->pPanel = AK_NULL;

    (&pAudio_Player_Parm->Resource)->pPlay[0] = AK_NULL;

    (&pAudio_Player_Parm->Resource)->pPlay[1] = AK_NULL;


    // get item resource
    AdPlyr_GetRes(&pAudio_Player_Parm->Resource);
    // init the resource rect
    AdPlyr_SetResRect(&pAudio_Player_Parm->Resource);
    pAudio_Player_Parm->Resource.ProgRfrshAll = AK_TRUE;

    pAudio_Player_Parm->RefreshTimer = ERROR_TIMER;
    pAudio_Player_Parm->LyricShwFlg = AK_FALSE;
 
    AudioInitRet = AudioPlayer_Init();
    if (AUDIOPLAYER_INIT_PLAY == AudioInitRet)    
    {
        pFilePath = AudioPlayer_GetPlayAudioPath();
        if (pFilePath)
            AdPlyr_GetLyric(pFilePath);
    }
    else if (AUDIOPLAYER_INIT_ERROR == AudioInitRet)
    {
        AK_DEBUG_OUTPUT("Audio player init error\r\n");
        return;
    }

     // set the Fade in/out flag
    if (AudioPlayer_GetCurState() != AUDIOPLAYER_STATE_PLAY )
    {
         WaveOut_OpenFade();
    }

    AudioPlayer_SetRefreshCallback(AdPlyr_RefreshCallBack);
    AudioPlayer_SetFetchLyricCallback(AdPlyr_GetLyric);
    AdPlyr_StartRefreshTimer();
    AdPlyr_SetRefresh(AUDIOPLAYER_REFRESH_ALL);
    pAudio_Player_Parm->ChngVlmFlg = 0;

    pAudio_Player_Parm->AudioName.range = pAudio_Player_Parm->Resource.NameRect;
    //pAudio_Player_Parm->AudioName.DispTextMax = pAudio_Player_Parm->AudioName.range.width/g_Font.CWIDTH;
    pAudio_Player_Parm->AudioName.bmpData = (T_pDATA)(pAudio_Player_Parm->Resource.pBckgrnd);
    AudioPlayer_SetNameInfoCallback(AdPlyr_SetNameInfoCallBack);

    // refresh top bar
    TopBar_EnableMenuButton();

    TopBar_SetTitle(uText);

#ifdef UI_USE_ICONMENU
    pAudio_Player_Parm->First = AK_TRUE;
#else
    TopBar_Show(TB_REFRESH_ALL);
#endif

    AudioPlayer_SetHitButtonCallback(AdPlyr_HitButtonCallBack);

    m_regResumeFunc(resume_audio_player);
    m_regSuspendFunc(suspend_audio_player);

	//AK_DEBUG_OUTPUT("initaudio_player 2 MIC_TO_HP=%d,0x0800005c=0x%x",(*(volatile T_U32 *)0x0800005c) & 0x40000,(*(volatile T_U32 *)0x0800005c));//xuyr debug Swd200001023
#endif
}

void exitaudio_player(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    AK_DEBUG_OUTPUT("exitaudio_player() ... ...\n");
	//AK_DEBUG_OUTPUT("exitaudio_player 1 MIC_TO_HP=%d,0x0800005c=0x%x",(*(volatile T_U32 *)0x0800005c) & 0x40000,(*(volatile T_U32 *)0x0800005c));//xuyr debug Swd200001023

	if (AUDIOPLAYER_STATE_AB_PLAY == AudioPlayer_GetCurState()) 
		AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PLAY);
    
    //FreqMgr_StateCheckOut(FREQ_FACTOR_AUDIO);
	
    Lyric_Free(&pAudio_Player_Parm->Lyric);

    AdPlyr_StopRefreshTimer();
    AudioPlayer_Destroy();

    TopBar_DisableMenuButton();

    if (AudioPlayer_GetCurState() != AUDIOPLAYER_STATE_PLAY )
    {
         WaveOut_CloseFade();
    }

#ifdef UI_USE_ICONMENU
    TopBar_DisableShow();
#endif

    // if audio player is not at play STATE
    // destroy the audio player
    pAudio_Player_Parm  = Fwl_Free(pAudio_Player_Parm);
    // free lyric
    // release the resource
//        AK_DEBUG_OUTPUT("exitaudio_player2");
    gb.bAudioPlaySM = AK_FALSE;

	//AK_DEBUG_OUTPUT("exitaudio_player 2 MIC_TO_HP=%d,0x0800005c=0x%x",(*(volatile T_U32 *)0x0800005c) & 0x40000,(*(volatile T_U32 *)0x0800005c));//xuyr debug Swd200001023	
#endif
}

void paintaudio_player(void)
{
#ifdef SUPPORT_AUDIOPLAYER

#ifdef UI_USE_ICONMENU
    if (pAudio_Player_Parm->First == AK_TRUE)
    {
        TopBar_EnableShow();
        TopBar_Show(TB_REFRESH_ALL);
        pAudio_Player_Parm->First = AK_FALSE;
    }
#endif
    // show the interface
    AdPlyr_Show();

    //refresh top bar
    TopBar_Show(TB_REFRESH_AUDIO_STATUS);
    TopBar_Refresh();
#endif
}

unsigned char handleaudio_player(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_AUDIOPLAYER_HANDLE_RET    ret;

    if (IsPostProcessEvent(event))
    {
        AdPlyr_SetRefresh(AUDIOPLAYER_REFRESH_ALL);
        return 1;
    }

	if (M_EVT_1 == event)
	{
		pAudio_Player_Parm->InitEvent = event;
	}
	
    ret = AudioPlayer_Handler(event, pEventParm);
	
    switch (ret)
    {
    case AUDIOPLAYER_HANDLE_STAY:
    case AUDIOPLAYER_HANDLE_NONE:
        break;
		
    case AUDIOPLAYER_HANDLE_AUDIOSWITCH:
        AdPlyr_SetRefresh(AUDIOPLAYER_REFRESH_ALL);
        AdPlyr_SetNameInfoCallBack();
        break;
		
    case AUDIOPLAYER_HANDLE_MENU:
        if (AudioPlayer_GetCurState() != AUDIOPLAYER_STATE_STOP)
        {
            pEventParm->p.pParam1 = (T_pVOID)AudioPlayer_GetPlayAudioPath();
            pEventParm->p.pParam2 = (T_pVOID)AudioPlayer_GetPlayAudioName();
        }
        else
        {
        	memset(pAudio_Player_Parm->filepath, 0, sizeof(T_USTR_FILE));
        	AudioPlayer_GetFocusFilePath(pAudio_Player_Parm->filepath);

            pEventParm->p.pParam1 = (T_pVOID)pAudio_Player_Parm->filepath;
            pEventParm->p.pParam2 = (T_pVOID)AudioPlayer_GetFocusName();
        }

		pEventParm->p.pParam3 = (T_pVOID)pAudio_Player_Parm->InitEvent;
        m_triggerEvent(M_EVT_MENU, pEventParm);
        AdPlyr_SetRefresh(AUDIOPLAYER_REFRESH_ALL);
        break;
		
    case AUDIOPLAYER_HANDLE_STATECHANGE:
        AdPlyr_SetRefresh(AUDIOPLAYER_LR_REFRESH \
                | AUDIOPLAYER_SPEED_REFRESH | AUDIOPLAYER_AUDITION_REFRESH \
                | AUDIOPLAYER_AB_REFRESH | AUDIOPLAYER_TOTALTIME_REFRESH \
                | AUDIOPLAYER_PROGRESS_REFRESH| AUDIOPLAYER_CURTIME_REFRESH \
                | AUDIOPLAYER_NAME_REFRESH | AUDIOPLAYER_LYRIC_REFRESH \
                | AUDIOPLAYER_PLAY_REFRESH /*|AUDIOPLAYER_PIC_REFRESH*/);
        pAudio_Player_Parm->ChngVlmFlg = 0; // kill volume progress bar at once
        break;
		
    case AUDIOPLAYER_HANDLE_VOICEREFRESH:
        pAudio_Player_Parm->ChngVlmFlg = 10;
        AdPlyr_SetRefresh(AUDIOPLAYER_AUDITION_REFRESH | AUDIOPLAYER_HANDLE_VOICEREFRESH);
        break;
		
    case AUDIOPLAYER_HANDLE_SWITCH:
        AdPlyr_SetRefresh(AUDIOPLAYER_REFRESH_ALL);
        AdPlyr_SetNameInfoCallBack();
        AdPlyr_RefreshCallBack();
		pAudio_Player_Parm->ChngVlmFlg = 0; // kill volume progress bar at once
        break;
		
    case AUDIOPLAYER_HANDLE_PROGRESSREFRESH:
         AdPlyr_SetRefresh(AUDIOPLAYER_PROGRESS_REFRESH | AUDIOPLAYER_TOTALTIME_REFRESH \
                | AUDIOPLAYER_CURTIME_REFRESH | AUDIOPLAYER_LYRIC_REFRESH);
        break;
		
    case AUDIOPLAYER_HANDLE_EXIT:
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
		
    case AUDIOPLAYER_HANDLE_EXITHOME:
        m_triggerEvent(M_EVT_Z09COM_SYS_RESET, pEventParm);
        break;
		
    default:
        if (ret == AUDIOPLAYER_HANDLE_ERROR)
        {
            if (gb.bInExplorer == AK_TRUE)
                MsgBox_InitAfx(&pAudio_Player_Parm->msgbox, 2, ctFAILURE, csAUDIO_FILE_ERROR, MSGBOX_INFORMATION);
            else
                MsgBox_InitAfx(&pAudio_Player_Parm->msgbox, 1, ctFAILURE, csAUDIO_FILE_ERROR, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pAudio_Player_Parm->msgbox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudio_Player_Parm->msgbox);
        }
        break;
    }

    if (M_EVT_PUB_TIMER == event)
    {
       if (pAudio_Player_Parm->AudioName.bScroll == AK_TRUE)
           AdPlyr_SetRefresh(AUDIOPLAYER_NAME_REFRESH/*|AUDIOPLAYER_PIC_REFRESH*/);
    }

    if (VME_EVT_TIMER == event)
    {
        if (pEventParm->w.Param1 == (T_U32)pAudio_Player_Parm->RefreshTimer)
        {
            if (pAudio_Player_Parm->ChngVlmFlg > 0)
            {
                pAudio_Player_Parm->ChngVlmFlg--;
                AdPlyr_SetRefresh(AUDIOPLAYER_VOLUME_REFRESH \
                        | AUDIOPLAYER_TOTALTIME_REFRESH \
                        | AUDIOPLAYER_LYRIC_REFRESH);
            }
			
            AdPlyr_SetRefresh(AUDIOPLAYER_PROGRESS_REFRESH \
                	| AUDIOPLAYER_CURTIME_REFRESH);
            if (AK_TRUE == pAudio_Player_Parm->LyricShwFlg)
                AdPlyr_SetRefresh(AUDIOPLAYER_LYRIC_REFRESH);
		}
    }
#endif
    return 0;
}


#ifdef SUPPORT_AUDIOPLAYER

static T_BOOL AdPlyr_SetRefresh(T_U32 refresh)
{
    AK_ASSERT_PTR(pAudio_Player_Parm, "AdPlyr_SetRefresh(): pAudio_Player_Parm null", AK_FALSE);

    if (AUDIOPLAYER_REFRESH_NONE != refresh)
    {
        pAudio_Player_Parm->Refresh |= refresh;
        if (AUDIOPLAYER_REFRESH_ALL == refresh)
            pAudio_Player_Parm->Resource.ProgRfrshAll = AK_TRUE;
    }
    else
    {
        pAudio_Player_Parm->Refresh = refresh;
    }

    return AK_TRUE;
}

static T_U32 AdPlyr_GetRefresh(T_VOID)
{
    AK_ASSERT_PTR(pAudio_Player_Parm, "AdPlyr_GetRefresh(): pAudio_Player_Parm null", AK_FALSE);

    return pAudio_Player_Parm->Refresh;
}

static T_BOOL AdPlyr_ShowTrack(T_AUDIO_PLAYER_RES *pResource)
{
    T_U32 i;

    AK_ASSERT_PTR(pResource, "AdPlyr_ShowTrack(): pResource null", AK_FALSE);

    i = AUDIO_PLAYER_TRACK_STEREO;

    Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->LRIconRect.left, pResource->LRIconRect.top, pResource->pLR[i], AK_NULL, AK_FALSE);

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowAudition(T_AUDIO_PLAYER_RES *pResource)
{
    T_U32 i;
    T_AUDIOPLAYER_STATE state;

    AK_ASSERT_PTR(pResource, "AdPlyr_ShowAudition(): pResource null", AK_FALSE);

    state = AudioPlayer_GetCurState();
    i = (state == AUDIOPLAYER_STATE_AUDITION) ? 1 : 0;
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->AuditionRect.left, pResource->AuditionRect.top, pResource->pAudition[i], AK_NULL, AK_FALSE);

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowCycle(T_AUDIO_PLAYER_RES *pResource)
{
    AK_ASSERT_PTR(pResource, "AdPlyr_ShowCycle(): pResource null", AK_FALSE);

    Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->CycleRect.left, pResource->CycleRect.top, pResource->pCycle[gs.AudioRepMode], AK_NULL, AK_FALSE);

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowAB(T_AUDIO_PLAYER_RES *pResource)
{
    T_U32 i;

    AK_ASSERT_PTR(pResource, "AdPlyr_ShowAB(): pResource null", AK_FALSE);

    if (AudioPlayer_IsMarkPointA())
        i = AudioPlayer_IsMarkPointB() ? 2 : 1;
    else
        i = 0;
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->ABRect.left, pResource->ABRect.top, pResource->pAB[i], AK_NULL, AK_FALSE);

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowSpeed(T_AUDIO_PLAYER_RES *pResource)
{
    AK_ASSERT_PTR(pResource, "AdPlyr_ShowSpeed(): pResource null", AK_FALSE);

    Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->SpeedRect.left, pResource->SpeedRect.top, pResource->pSpeed[gb.AudioPlaySpeed], AK_NULL, AK_FALSE);

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowStopButton(T_AUDIO_PLAYER_RES *pResource)
{
    AK_ASSERT_PTR(pResource, "AdPlyr_ShowStopButton(): pResource null", AK_FALSE);

    Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->StopRect.left, pResource->StopRect.top, pResource->pStop, AK_NULL, AK_FALSE);

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowPreviousButton(T_AUDIO_PLAYER_RES *pResource)
{
    AK_ASSERT_PTR(pResource, "AdPlyr_ShowPreviousButton(): pResource null", AK_FALSE);

    Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->PreviousRect.left, pResource->PreviousRect.top, pResource->pPrevious, AK_NULL, AK_FALSE);

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowNextButton(T_AUDIO_PLAYER_RES *pResource)
{
    AK_ASSERT_PTR(pResource, "AdPlyr_ShowNextButton(): pResource null", AK_FALSE);

    Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->NextRect.left, pResource->NextRect.top, pResource->pNext, AK_NULL, AK_FALSE);

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowVolumeButton(T_AUDIO_PLAYER_RES *pResource)
{
    AK_ASSERT_PTR(pResource, "AdPlyr_ShowVolumeButton(): pResource null", AK_FALSE);

	if (0 == Fwl_GetAudioVolume())
	{
		Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->VolBtnRect.left, pResource->VolBtnRect.top, pResource->pVolume[1], AK_NULL, AK_FALSE);
	}
	else
	{
		Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->VolBtnRect.left, pResource->VolBtnRect.top, pResource->pVolume[0], AK_NULL, AK_FALSE);
	}

    return AK_TRUE;
}


static T_BOOL AdPlyr_ShowPlayButton(T_AUDIO_PLAYER_RES *pResource)
{
    T_AUDIOPLAYER_STATE curstat;
    T_RECT BckRect;
    T_RECT PnlRect;

    AK_ASSERT_PTR(pResource, "AdPlyr_ShowPlayButton(): pResource null", AK_FALSE);

    curstat = AudioPlayer_GetCurState();

    //the part of backgroud rect 
    BckRect.left =  pResource->PlayRect.left;
    BckRect.top = pResource->PanelRect.top;
    BckRect.width = pResource->PlayRect.width;
    BckRect.height = pResource->PanelRect.height;

    //the part of control panel rect 
    PnlRect.left =  pResource->PlayRect.left - pResource->PanelRect.left;
    PnlRect.top = 0;
    PnlRect.width = pResource->PlayRect.width;
    PnlRect.height = pResource->PanelRect.height;

    //need to fresh the backgroud , control panel, play button in order, otherwise the play button shows abnormally    
    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pResource->PlayRect.left, pResource->PanelRect.top, &BckRect, pResource->pBckgrnd, AK_NULL, AK_FALSE);

    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pResource->PlayRect.left, pResource->PanelRect.top, &PnlRect, pResource->pPanel, &g_Graph.TransColor, AK_FALSE);


    if (curstat == AUDIOPLAYER_STATE_PLAY || curstat == AUDIOPLAYER_STATE_AUDITION
        || curstat == AUDIOPLAYER_STATE_FORWARD || curstat == AUDIOPLAYER_STATE_BACKWARD
        || curstat == AUDIOPLAYER_STATE_AB_PLAY)
    {
        //show pause button
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->PlayRect.left, pResource->PlayRect.top, pResource->pPlay[1], &g_Graph.TransColor, AK_FALSE);
    }
    else if (curstat == AUDIOPLAYER_STATE_PAUSE || curstat == AUDIOPLAYER_STATE_STOP)
    {
        //show play button
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->PlayRect.left, pResource->PlayRect.top, pResource->pPlay[0], &g_Graph.TransColor, AK_FALSE);
    }
    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowControlPanel(T_AUDIO_PLAYER_RES *pResource)
{
    AK_ASSERT_PTR(pResource, "AdPlyr_ShowControlPanel(): pResource null", AK_FALSE);

    Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->PanelRect.left, pResource->PanelRect.top, pResource->pPanel, &g_Graph.TransColor, AK_FALSE);

    return AK_TRUE;
}


static T_BOOL AdPlyr_ShowCurTime(T_AUDIO_PLAYER_RES *pResource)
{
    T_USTR_INFO     uText;
    T_STR_INFO      tmpstr;
    T_RECT          range;
    T_U32           TotalTime = 0;
    T_U32           CurTime = 0;

    AK_ASSERT_PTR(pResource, "AdPlyr_ShowCurTime(): pResource null", AK_FALSE);

    CurTime = AudioPlayer_GetPlayTime();
    TotalTime = AudioPlayer_GetTotalTime();
    if (CurTime > TotalTime)
    {
        AudioPlayer_TuneTotalTime();
        AdPlyr_ShowTotalTime(pResource);
    }

    range = pResource->CurTimeRect;
    
    //if (TotalTime >= MILLISECOND_PER_HOUR && AK_FALSE == pAudio_Player_Parm->LyricShwFlg)
    if (TotalTime >= MILLISECOND_PER_HOUR)
    {
        sprintf(tmpstr, "%02d:%02d:%02d", (unsigned int)(CurTime/MILLISECOND_PER_HOUR), \
                (unsigned int)(CurTime%MILLISECOND_PER_HOUR/MILLISECOND_PER_MINUTE), \
                (unsigned int)(CurTime%MILLISECOND_PER_MINUTE/MILLISECOND_PER_SECOND));
    }
    else
    {
        sprintf(tmpstr, "%02d:%02d", (unsigned int)(CurTime/MILLISECOND_PER_MINUTE), \
                (unsigned int)(CurTime%MILLISECOND_PER_MINUTE/MILLISECOND_PER_SECOND));
    }

    memset(uText, 0, sizeof(uText));
    Eng_StrMbcs2Ucs(tmpstr, uText);
    range.width = (T_U16)UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText));
    range.left = pResource->ProgBarRect.left;

    //add 20 pixel at here, for the width of string is changing
    if (pResource->PlayerRect.width-range.left-range.width>20)
        range.width+=20;

    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, range.left, range.top, &range, pResource->pBckgrnd, AK_NULL, AK_FALSE);
    Fwl_DispString(HRGB_LAYER, range.left, range.top, tmpstr, Utl_StrLen(tmpstr), COLOR_BLACK, CURRENT_FONT_SIZE);

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowTotalTime(T_AUDIO_PLAYER_RES *pResource)
{
    T_USTR_INFO     uText;
    T_STR_INFO      tmpstr;
    T_RECT          range;
    T_U32           TotalTime = 0;
    T_AUDIOPLAYER_STATE state;

    AK_ASSERT_PTR(pResource, "AdPlyr_ShowTotalTime(): pResource null", AK_FALSE);

    TotalTime = AudioPlayer_GetTotalTime();
    state = AudioPlayer_GetCurState();
    if (state == AUDIOPLAYER_STATE_STOP)
    {
        TotalTime = AudioPlayer_GetTotalTimeAtStopState();
    }

    range = pResource->TotalTimeRect;
    
	// Update The Backgroup Area of Total Time
    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, range.left, range.top, &range, pResource->pBckgrnd, AK_NULL, AK_FALSE);
    
    //if (TotalTime >= MILLISECOND_PER_HOUR && AK_FALSE == pAudio_Player_Parm->LyricShwFlg)
    if (TotalTime >= MILLISECOND_PER_HOUR)
    {
        sprintf(tmpstr, "%02d:%02d:%02d", (unsigned int)(TotalTime/MILLISECOND_PER_HOUR), \
               (unsigned int)(TotalTime%MILLISECOND_PER_HOUR/MILLISECOND_PER_MINUTE), \
               (unsigned int)(TotalTime%MILLISECOND_PER_MINUTE/MILLISECOND_PER_SECOND));
    }
    else
    {
        sprintf(tmpstr, "%02d:%02d", (unsigned int)(TotalTime/MILLISECOND_PER_MINUTE), \
                (unsigned int)(TotalTime%MILLISECOND_PER_MINUTE/MILLISECOND_PER_SECOND));
    }

    memset(uText, 0, sizeof(uText));
    Eng_StrMbcs2Ucs(tmpstr, uText);
    range.width = (T_U16)UGetSpeciStringWidth(uText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uText));
    range.left = Fwl_GetLcdWidth() - pResource->ProgBarRect.left - range.width;

    Fwl_DispString(HRGB_LAYER, range.left, range.top, tmpstr, Utl_StrLen(tmpstr), COLOR_BLACK, CURRENT_FONT_SIZE);

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowVolumeNum(T_AUDIO_PLAYER_RES *pRes)
{
    T_S8    tmpStr[4] = {0};
    T_U16    Ustrtmp[8] = {0};
    T_POS   posX = 0;
    T_POS   posY = 0;
    T_U16   UStrLen = 0;
    T_U32   volValue = 0;
    T_U32   UStrWidth = 0;

    // show min volume num
    posX = pRes->VolRct.left;
    posY = pRes->CurTimeRect.top;
    sprintf(tmpStr, "%d", 0);
    Eng_StrMbcs2Ucs(tmpStr, Ustrtmp);
    UStrLen = (T_U16)Utl_UStrLen(Ustrtmp);
    Fwl_UDispSpeciString(HRGB_LAYER, posX, posY, Ustrtmp, COLOR_BLACK, CURRENT_FONT_SIZE, UStrLen);

    // show cur volume num
    volValue = (T_U32)Fwl_GetAudioVolume();
    sprintf(tmpStr, "%d", (unsigned int)volValue);
    Eng_StrMbcs2Ucs(tmpStr, Ustrtmp);
    UStrLen = (T_U16)Utl_UStrLen(Ustrtmp);
    UStrWidth = UGetSpeciStringWidth(Ustrtmp, CURRENT_FONT_SIZE, UStrLen);
    posX = (T_POS)(pRes->VolRct.left + (pRes->VolRct.width - UStrWidth) / 2);
    posY = pRes->CurTimeRect.top;

    Fwl_UDispSpeciString(HRGB_LAYER, posX, posY, Ustrtmp, COLOR_BLACK, CURRENT_FONT_SIZE, UStrLen);

    // show cur volume num
    sprintf(tmpStr, "%d", AK_VOLUME_MAX);
    Eng_StrMbcs2Ucs(tmpStr, Ustrtmp);
    UStrLen = (T_U16)Utl_UStrLen(Ustrtmp);
    UStrWidth = UGetSpeciStringWidth(Ustrtmp, CURRENT_FONT_SIZE, UStrLen);
    posX = (T_POS)(pRes->VolRct.left + pRes->VolRct.width - UStrWidth);
    posY = pRes->CurTimeRect.top;

    Fwl_UDispSpeciString(HRGB_LAYER, posX, posY, Ustrtmp, COLOR_BLACK, CURRENT_FONT_SIZE, UStrLen);

    return AK_TRUE;
}


static T_BOOL AdPlyr_ShowVolume(T_AUDIO_PLAYER_RES *pRes)
{
    T_RECT  range;
    T_U32   i;
    T_U32   BarW;
    T_U32   volValue = 0;
    T_POS   posX = 0;
    T_POS   posY = 0;

    AK_ASSERT_PTR(pRes, "AdPlyr_ShowVolume(): pResource null", AK_FALSE);

    // refresh background 
    range.left  = pRes->VolRct.left;
    range.top   = pRes->VolRct.top;
    range.width = pRes->VolRct.width;
    range.height= pRes->VolRct.height+20;
    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pRes->VolRct.left, pRes->VolRct.top, &range, pRes->pBckgrnd, AK_NULL, AK_FALSE);

    // show small sound icon and big sound icon
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->SmallSoundRect.left, pRes->SmallSoundRect.top, pRes->pSmallSound, &g_Graph.TransColor, AK_FALSE);
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->BigSoundRect.left, pRes->BigSoundRect.top, pRes->pBigSound, &g_Graph.TransColor, AK_FALSE);

    //show sound bar
    range.left = pRes->SoundBarRect.left - pRes->ProgBarRect.left;
    range.top = 0;
    range.width = pRes->SoundBarRect.width;
    range.height = pRes->SoundBarRect.height;
    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pRes->SoundBarRect.left, pRes->SoundBarRect.top, &range, pRes->pBarBack, AK_NULL, AK_FALSE);
    
    volValue = (T_U32)Fwl_GetAudioVolume();
    BarW = (pRes->SoundBarRect.width * volValue) / AK_VOLUME_MAX;

    for (i = 0; i < BarW; i++)
    {
        posX = (T_POS)(pRes->SoundBarRect.left + i);
        posY = pRes->SoundBarRect.top + 1;
        Fwl_AkBmpDrawFromString(HRGB_LAYER, posX, posY, pRes->pBar[i % 15], AK_NULL, AK_FALSE);
    }

    pAudio_Player_Parm->Resource.ProgRfrshAll = AK_TRUE;

    // show volume num
    AdPlyr_ShowVolumeNum(pRes);

	//dengzhou 判断是否静音
	AdPlyr_ShowVolumeButton(pRes);
    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowProgress(T_AUDIO_PLAYER_RES *pResource)
{
    T_U32 i, k;
    T_U32  BarW;
    T_U32  PlayTime;
    T_U32  TotalTime;
    T_RECT range;
//    T_POS   barPosX = 0;
//    T_POS   barPosY = 0;

    AK_ASSERT_PTR(pResource, "AdPlyr_ShowProgress(): pResource null", AK_FALSE);

    PlayTime = AudioPlayer_GetPlayTime();
    TotalTime = AudioPlayer_GetTotalTime();
	
	if (PlayTime > TotalTime)
	{
		AudioPlayer_TuneTotalTime();
		AdPlyr_ShowTotalTime(pResource);
		TotalTime = AudioPlayer_GetTotalTime();
	}

    if (pAudio_Player_Parm->Resource.ProgRfrshAll \
            || pResource->LastPlayTime > PlayTime
            || AudioPlayer_GetCurState() != AUDIOPLAYER_STATE_PLAY)
    {
        range.left = pResource->PlayerRect.left;
        range.top = pResource->SoundBarRect.top;
        range.width = pResource->PlayerRect.width;
        range.height = pResource->SoundBarRect.height;
        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, range.left, range.top, &range, pResource->pBckgrnd, AK_NULL, AK_FALSE);

        Fwl_AkBmpDrawFromString(HRGB_LAYER, pResource->ProgBarRect.left, pResource->ProgBarRect.top, pResource->pBarBack, AK_NULL, AK_FALSE);
    }

    BarW = (T_U32)((0 != TotalTime) ? (((T_U64)pResource->ProgBarRect.width * PlayTime) / TotalTime) : 0);//xuyr Swd200001270

    if ((0 == TotalTime) || (pResource->LastPlayTime > PlayTime) \
        || (pAudio_Player_Parm->Resource.ProgRfrshAll) \
        || (AudioPlayer_GetCurState() != AUDIOPLAYER_STATE_PLAY))
    {
        k = 0;
    }
    else
    {    
        k = (T_U32)(((T_U64)pResource->ProgBarRect.width * pResource->LastPlayTime) / TotalTime);//xuyr Swd200001270
    }

    for(i = k; i<BarW; i++)
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, (T_U16)(pResource->BarRect.left + i), pResource->BarRect.top, pResource->pBar[i % 15], AK_NULL, AK_FALSE);
    }

    pResource->LastPlayTime = PlayTime;
   	pAudio_Player_Parm->Resource.ProgRfrshAll = AK_FALSE;
	
    return AK_TRUE;
}


static T_VOID AdPlyr_SetNameInfoCallBack(T_VOID)
{
    T_pCWSTR pFocusName = AK_NULL;
    T_U16 width = 0;

    if (AK_NULL == pAudio_Player_Parm)
        return;

    pFocusName = AudioPlayer_GetPlayAudioName();
    if (AK_NULL == pFocusName)
        pFocusName = AudioPlayer_GetFocusName();

    if (pFocusName == AK_NULL)
    {
        pAudio_Player_Parm->AudioName.initFlag = AK_FALSE;
        pAudio_Player_Parm->AudioName.UText[0] = 0;
        pAudio_Player_Parm->AudioName.bScroll= AK_FALSE;
        pAudio_Player_Parm->AudioName.UTextLen = 0;
    }
    else
    {
        pAudio_Player_Parm->AudioName.initFlag = AK_TRUE;

        /*tanslate ansi string to unicode string*/
        Utl_UStrCpy(pAudio_Player_Parm->AudioName.UText, (T_pWSTR)pFocusName);
        pAudio_Player_Parm->AudioName.UTextLen = (T_U16)Utl_UStrLen(pFocusName);

        width = (T_U16)UGetSpeciStringWidth(pAudio_Player_Parm->AudioName.UText, CURRENT_FONT_SIZE, pAudio_Player_Parm->AudioName.UTextLen);
        if (width > pAudio_Player_Parm->AudioName.range.width)
            pAudio_Player_Parm->AudioName.bScroll= AK_TRUE;
        else
            pAudio_Player_Parm->AudioName.bScroll= AK_FALSE;

        pAudio_Player_Parm->AudioName.point.y= pAudio_Player_Parm->Resource.NameRect.top + ((pAudio_Player_Parm->Resource.NameRect.height - 16) >> 1);
        if (pAudio_Player_Parm->AudioName.bScroll == AK_TRUE)
            pAudio_Player_Parm->AudioName.point.x = pAudio_Player_Parm->Resource.NameRect.left ;
        else
            pAudio_Player_Parm->AudioName.point.x = pAudio_Player_Parm->Resource.NameRect.left + ((pAudio_Player_Parm->Resource.NameRect.width - width) >> 1);
    }
}



T_VOID AdPlyr_NameSetOffset(T_VOID)
{
    FileNameOffset++;
    if (FileNameOffset + AUDIO_NAME_TAILSPACE > pAudio_Player_Parm->AudioName.UTextLen)
    {
        FileNameOffset = 0;
    }
}
static T_BOOL AdPlyr_ShowName(T_AUDIO_PLAYER_RES *pResource)
{
    T_U16* pFocusName = AK_NULL;
    T_POS left;
    T_POS top;
    //T_U32 maxlen;

    pFocusName = pAudio_Player_Parm->AudioName.UText;
    //maxlen = pAudio_Player_Parm->AudioName.DispTextMax;
    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pAudio_Player_Parm->AudioName.range.left, pAudio_Player_Parm->AudioName.range.top, \
            &pAudio_Player_Parm->AudioName.range, pAudio_Player_Parm->Resource.pBckgrnd, AK_NULL, AK_FALSE);

    if (pAudio_Player_Parm->AudioName.initFlag == AK_TRUE)
    {
        top = pAudio_Player_Parm->AudioName.point.y;
        left = pAudio_Player_Parm->AudioName.point.x;
        if (pAudio_Player_Parm->AudioName.bScroll == AK_TRUE)
        {
            AdPlyr_NameSetOffset();
            Fwl_UScrollDispString(HRGB_LAYER, pFocusName, left, top, pAudio_Player_Parm->AudioName.UTextLen,
                   FileNameOffset, pAudio_Player_Parm->AudioName.range.width, COLOR_BLACK, CURRENT_FONT_SIZE);
        }
        else
        {
            Fwl_UDispSpeciString(HRGB_LAYER, left, top, pFocusName, COLOR_BLACK, CURRENT_FONT_SIZE, pAudio_Player_Parm->AudioName.UTextLen);
        }
    }

    return AK_TRUE;
}

static T_BOOL AdPlyr_ShowLyric(T_AUDIO_PLAYER_RES *pResource)
{
    T_RECT          LyricRect;
    T_pCDATA        pBckgrnd = AK_NULL;
    T_U32           CurPos = 0;

    LyricRect = pAudio_Player_Parm->LyricRect;
    LyricRect.left -= 3;
    LyricRect.width += 6;
    pBckgrnd = pResource->pBckgrnd;
    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, LyricRect.left, LyricRect.top, &LyricRect, pBckgrnd, AK_NULL, AK_FALSE);

    if (AK_TRUE == pAudio_Player_Parm->LyricShwFlg && AUDIOPLAYER_STATE_STOP != AudioPlayer_GetCurState())
    {
        CurPos = AudioPlayer_GetPlayTime();
        Lyric_Show(&pAudio_Player_Parm->Lyric, CurPos);

        return AK_TRUE;
    }

    return AK_FALSE;
}

static T_BOOL AdPlyr_ShowBckgrnd(T_AUDIO_PLAYER_RES *pResource)
{
    T_RECT range;

    range = pResource->PlayerRect;

    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, range.left, range.top, &range, pResource->pBckgrnd, AK_NULL, AK_FALSE);

    return AK_TRUE;
}


////////////////////////////////////////////////////////////////////////////
static T_BOOL Adplyr_showPic(T_AUDIO_PLAYER_RES *pResource)
{
	T_pDATA 	pBuf=AK_NULL;

	pBuf = AudioPlayer_GetAudioImage(AudioPlayer_GetPlayAudioPath(), 
				pAudio_Player_Parm->ImgRect.width, pAudio_Player_Parm->ImgRect.height);
	
	// Update The Backgroup Area
	Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pAudio_Player_Parm->ImgRect.left, pAudio_Player_Parm->ImgRect.top, 
								&(pAudio_Player_Parm->ImgRect), pResource->pBckgrnd, AK_NULL, AK_FALSE);
	Fwl_AkBmpDrawFromString(HRGB_LAYER, pAudio_Player_Parm->ImgRect.left, pAudio_Player_Parm->ImgRect.top, pBuf, &g_Graph.TransColor, AK_FALSE);
	
	if (pBuf != AK_NULL)
	{
		Fwl_Free(pBuf);		
	}	
	return AK_TRUE;
}



///////////////////////////////////////////////////////////////////////////

static T_BOOL AdPlyr_Show(T_VOID)
{
    T_AUDIO_PLAYER_RES *pResource;
    T_U32               RfrshFlg = AUDIOPLAYER_REFRESH_NONE;

    AK_ASSERT_PTR(pAudio_Player_Parm, "AdPlyr_Show(): pAudio_Player_Parm null", AK_FALSE);

    pResource = &pAudio_Player_Parm->Resource;
    RfrshFlg = AdPlyr_GetRefresh();

    if (RfrshFlg == AUDIOPLAYER_REFRESH_ALL)
        AdPlyr_ShowBckgrnd(pResource);

    //control panel
    if (RfrshFlg & AUDIOPLAYER_CTLPANEL_REFRESH)
         AdPlyr_ShowControlPanel(pResource);

    //right buttons
    if (RfrshFlg & AUDIOPLAYER_LR_REFRESH)
        AdPlyr_ShowTrack(pResource);
    if (RfrshFlg & AUDIOPLAYER_AUDITION_REFRESH)
        AdPlyr_ShowAudition(pResource);
    if (RfrshFlg & AUDIOPLAYER_CYCLE_REFRESH)
        AdPlyr_ShowCycle(pResource);
    if (RfrshFlg & AUDIOPLAYER_AB_REFRESH)
        AdPlyr_ShowAB(pResource);
    if (RfrshFlg & AUDIOPLAYER_SPEED_REFRESH
		&& AudioPlayer_GetCurState()!=AUDIOPLAYER_STATE_AUDITION
		&& !(AudioPlayer_GetCurState()==AUDIOPLAYER_STATE_PAUSE
		&& AudioPlayer_GetOldState()==AUDIOPLAYER_STATE_AUDITION))
        AdPlyr_ShowSpeed(pResource);
    if (RfrshFlg & AUDIOPLAYER_NAME_REFRESH)
        AdPlyr_ShowName(pResource);

    //left buttons
    if (RfrshFlg & AUDIOPLAYER_PLAY_REFRESH)
        AdPlyr_ShowPlayButton(pResource);
    if (RfrshFlg & AUDIOPLAYER_STOP_REFRESH)
        AdPlyr_ShowStopButton(pResource);
    if (RfrshFlg & AUDIOPLAYER_PREVIOUS_REFRESH)
        AdPlyr_ShowPreviousButton(pResource);
    if (RfrshFlg & AUDIOPLAYER_NEXT_REFRESH)
        AdPlyr_ShowNextButton(pResource);
    if (RfrshFlg & AUDIOPLAYER_VOLBTN_REFRESH)
        AdPlyr_ShowVolumeButton(pResource);

    
    if (pAudio_Player_Parm->ChngVlmFlg > 0)
    {
        //if (RfrshFlg & AUDIOPLAYER_VOLUME_REFRESH)
            AdPlyr_ShowVolume(pResource);
    }
    else
    {
        if (RfrshFlg & AUDIOPLAYER_LYRIC_REFRESH)
            AdPlyr_ShowLyric(pResource);
        if (RfrshFlg & AUDIOPLAYER_TOTALTIME_REFRESH)
            AdPlyr_ShowTotalTime(pResource);
        if (RfrshFlg & AUDIOPLAYER_CURTIME_REFRESH)
            AdPlyr_ShowCurTime(pResource);
        if (RfrshFlg & AUDIOPLAYER_PROGRESS_REFRESH)
            AdPlyr_ShowProgress(pResource);
    }

    //////////////////////////////////////////////////////////////
	if (RfrshFlg & AUDIOPLAYER_PIC_REFRESH)
    	Adplyr_showPic(pResource);
    //////////////////////////////////////////////////////////////
    
    if (AUDIOPLAYER_REFRESH_NONE != RfrshFlg)
    {
        AdPlyr_SetRefresh(AUDIOPLAYER_REFRESH_NONE);
        Fwl_RefreshDisplay();
    }

    return AK_TRUE;
}

static T_BOOL AdPlyr_GetLyric(T_pCWSTR pFilePath)
{
    T_USTR_FILE file_ext, file_path;
    T_U16       *pUstring;
    T_pFILE     fp;
    T_U32       fsize;//, ustrLen;
    T_U8        *lyric_buf = AK_NULL;
    T_RECT      lyric_rect;
    T_BOOL      ret = AK_FALSE;

    if (AK_NULL == pAudio_Player_Parm)
        return AK_FALSE;

    pAudio_Player_Parm->LyricShwFlg = AK_FALSE;

    Lyric_Free(&pAudio_Player_Parm->Lyric);
    memset((void *)(&pAudio_Player_Parm->Lyric), 0x00, sizeof(T_LYRIC_STRUCT));

    Utl_USplitFileName((T_pWSTR)pFilePath, file_path, file_ext);
    Utl_UStrCat(file_path, _T(".LRC"));

    if ((fsize = Fwl_FileGetSize(file_path)) > 0)
    {
        lyric_buf = (T_U8 *)Fwl_Malloc(fsize+2);
        AK_ASSERT_PTR(lyric_buf, "AdPlyr_GetLyric():lyric_buf malloc fail", AK_FALSE);
        
        pUstring = (T_U16 *)Fwl_Malloc((fsize << 1) + 2);

        if (AK_NULL == pUstring)
        {
            lyric_buf = Fwl_Free(lyric_buf);
            
            AK_DEBUG_OUTPUT("AdPlyr_GetLyric(): pUstring malloc error");

            return AK_FALSE;
        }
        
        
        memset((void *)lyric_buf, 0x00, fsize+2);

        fp = Fwl_FileOpen(file_path, _FMODE_READ, _FMODE_READ);
        AK_ASSERT_VAL((fp != _FOPEN_FAIL), "AdPlyr_GetLyric(): open lyric error", AK_FALSE);
        Fwl_FileRead(fp, lyric_buf, fsize);
        Fwl_FileClose(fp);

        lyric_rect = pAudio_Player_Parm->LyricRect;
        Lyric_String2Unc((const T_S8 *)lyric_buf, pUstring);
        ret = Lyric_ParseBuf(pUstring, &pAudio_Player_Parm->Lyric, lyric_rect);

        lyric_buf = Fwl_Free(lyric_buf);
        pUstring = Fwl_Free(pUstring);

        if (AK_TRUE == ret)
            pAudio_Player_Parm->LyricShwFlg = AK_TRUE;
    }
    AdPlyr_SetRefresh(AUDIOPLAYER_LYRIC_REFRESH);

    return ret;
}


static T_BOOL AdPlyr_StartRefreshTimer(T_VOID)
{
    AK_ASSERT_PTR(pAudio_Player_Parm, "AdPlyr_StartRefreshTimer(): pAudio_Player_Parm null", AK_FALSE);

    if(ERROR_TIMER != pAudio_Player_Parm->RefreshTimer)
    {
        Fwl_StopTimer(pAudio_Player_Parm->RefreshTimer);
        pAudio_Player_Parm->RefreshTimer = ERROR_TIMER;
    }
    pAudio_Player_Parm->RefreshTimer = Fwl_SetTimerMilliSecond(AUDIOPLAYER_REFRESH_TIME, AK_TRUE);
    AK_DEBUG_OUTPUT("AdPlyr_StartRefreshTimer(): RefreshTimer = %d.\n", pAudio_Player_Parm->RefreshTimer);

    return AK_TRUE;
}

static T_BOOL AdPlyr_StopRefreshTimer(T_VOID)
{
    AK_ASSERT_PTR(pAudio_Player_Parm, "AdPlyr_StopRefreshTimer(): pAudio_Player_Parm null", AK_FALSE);

    if(ERROR_TIMER != pAudio_Player_Parm->RefreshTimer)
    {
        Fwl_StopTimer(pAudio_Player_Parm->RefreshTimer);
        pAudio_Player_Parm->RefreshTimer = ERROR_TIMER;
    }

    return AK_TRUE;
}

static T_VOID AdPlyr_SetResRect(T_AUDIO_PLAYER_RES *pRes)
{
    T_POS   baseTop = 3 * Fwl_GetLcdHeight() / 4;
 //   T_POS   baseLeft = Fwl_GetLcdWidth() / 2;
 //   T_LEN   radius = 0;
    T_LEN   frameWidth = 0;
    T_LEN   tmpLen = 0;
        
    AK_ASSERT_PTR_VOID(pRes, "AdPlyr_SetResRect(): pResource null");

    pRes->PlayerRect.left = 0;
    pRes->PlayerRect.top = TOP_BAR_HEIGHT;
    pRes->PlayerRect.width = Fwl_GetLcdWidth();
    pRes->PlayerRect.height = Fwl_GetLcdHeight() - TOP_BAR_HEIGHT;

    //control panel rect, use standard bmp image
    AKBmpGetInfo(pRes->pPanel, &pRes->PanelRect.width, &pRes->PanelRect.height, AK_NULL);
    pRes->PanelRect.left = (Fwl_GetLcdWidth() - pRes->PanelRect.width)/2;
    pRes->PanelRect.top = baseTop - pRes->PanelRect.height - VERTICAL_DISTANCE;


    // play/pause button rect, use standard bmp image
    AKBmpGetInfo(pRes->pPlay[0], &pRes->PlayRect.width, &pRes->PlayRect.height, AK_NULL);
    pRes->PlayRect.left = pRes->PanelRect.left;
    pRes->PlayRect.top =  pRes->PanelRect.top  - (pRes->PlayRect.height - pRes->PanelRect.height) / 2;

    // stop button rect
    AKBmpGetInfo(pRes->pStop, &pRes->StopRect.width, &pRes->StopRect.height, AK_NULL);
    pRes->StopRect.left = pRes->PlayRect.left + pRes->PlayRect.width + BUTTON_SPACE;
    pRes->StopRect.top =  pRes->PanelRect.top + VERTICAL_DISTANCE/2 - (pRes->StopRect.height - pRes->PanelRect.height) / 2;

    // previous button rect
    AKBmpGetInfo(pRes->pPrevious, &pRes->PreviousRect.width, &pRes->PreviousRect.height, AK_NULL);
    pRes->PreviousRect.left = pRes->StopRect.left + pRes->StopRect.width + BUTTON_SPACE;
    pRes->PreviousRect.top =  pRes->PanelRect.top + VERTICAL_DISTANCE/2 - (pRes->PreviousRect.height - pRes->PanelRect.height) / 2;

    // next button rect
    AKBmpGetInfo(pRes->pNext, &pRes->NextRect.width, &pRes->NextRect.height, AK_NULL);
    pRes->NextRect.left = pRes->PreviousRect.left + pRes->PreviousRect.width + BUTTON_SPACE;
    pRes->NextRect.top =  pRes->PanelRect.top + VERTICAL_DISTANCE/2 - (pRes->NextRect.height - pRes->PanelRect.height) / 2;

    // volume/mute button rect
    AKBmpGetInfo(pRes->pVolume[0], &pRes->VolBtnRect.width, &pRes->VolBtnRect.height, AK_NULL);
    pRes->VolBtnRect.left = pRes->NextRect.left + pRes->NextRect.width + BUTTON_SPACE;
    pRes->VolBtnRect.top =  pRes->PanelRect.top + VERTICAL_DISTANCE/2 - (pRes->VolBtnRect.height - pRes->PanelRect.height) / 2;

    // speed button rect
    AKBmpGetInfo(pRes->pSpeed[0], &pRes->SpeedRect.width, &pRes->SpeedRect.height, AK_NULL);

#if (LCD_CONFIG_WIDTH==800)  //800 x 480
    pRes->SpeedRect.left =  pRes->PanelRect.left + pRes->PanelRect.width 
                                                 - pRes->SpeedRect.width 
                                                 - 2*BUTTON_SPACE;
#else
#if (LCD_CONFIG_WIDTH==480)  //480 x 272
    pRes->SpeedRect.left =  pRes->PanelRect.left + pRes->PanelRect.width 
                                                 - pRes->SpeedRect.width 
                                                 - 2*BUTTON_SPACE;                                             
#else //320 x 240
    pRes->SpeedRect.left =  pRes->PanelRect.left + pRes->PanelRect.width 
                                                 - pRes->SpeedRect.width 
                                                 - 12*BUTTON_SPACE;
#endif
#endif

    pRes->SpeedRect.top = pRes->PanelRect.top + VERTICAL_DISTANCE/2 - (pRes->SpeedRect.height - pRes->PanelRect.height) / 2;

    // A->B button rect
    AKBmpGetInfo(pRes->pAB[0], &pRes->ABRect.width, &pRes->ABRect.height, AK_NULL);
    pRes->ABRect.left = pRes->SpeedRect.left - pRes->SpeedRect.width - BUTTON_SPACE;
    pRes->ABRect.top = pRes->PanelRect.top + VERTICAL_DISTANCE/2 - (pRes->ABRect.height - pRes->PanelRect.height) / 2;

    //Cycle button rect
    AKBmpGetInfo(pRes->pCycle[0], &pRes->CycleRect.width, &pRes->CycleRect.height, AK_NULL);
    pRes->CycleRect.left = pRes->ABRect.left - pRes->ABRect.width - BUTTON_SPACE;
    pRes->CycleRect.top = pRes->PanelRect.top + VERTICAL_DISTANCE/2 - (pRes->CycleRect.height - pRes->PanelRect.height) / 2;

    // audition button rect
    AKBmpGetInfo(pRes->pAudition[0], &pRes->AuditionRect.width, &pRes->AuditionRect.height, AK_NULL);
    pRes->AuditionRect.left = pRes->CycleRect.left - pRes->CycleRect.width - BUTTON_SPACE;
    pRes->AuditionRect.top = pRes->PanelRect.top + VERTICAL_DISTANCE/2 - (pRes->AuditionRect.height - pRes->PanelRect.height) / 2;

    //LR button rect
    AKBmpGetInfo(pRes->pLR[0], &pRes->LRIconRect.width, &pRes->LRIconRect.height, AK_NULL);
    pRes->LRIconRect.left = pRes->AuditionRect.left - pRes->AuditionRect.width - BUTTON_SPACE;
    pRes->LRIconRect.top = pRes->PanelRect.top + VERTICAL_DISTANCE/2 - (pRes->LRIconRect.height - pRes->PanelRect.height) / 2;

    AKBmpGetInfo(pRes->pBarBack, &pRes->ProgBarRect.width, &pRes->ProgBarRect.height, AK_NULL);
    pRes->ProgBarRect.left = (Fwl_GetLcdWidth() - pRes->ProgBarRect.width)/2;
    pRes->ProgBarRect.top = baseTop;
    frameWidth = pRes->ProgBarRect.left;

    AKBmpGetInfo(pRes->pBar[0], &pRes->BarRect.width, &pRes->BarRect.height, AK_NULL);
    pRes->BarRect.left = pRes->ProgBarRect.left;
    pRes->BarRect.top = baseTop + 1;

    pRes->CurTimeRect.left = frameWidth;
    pRes->CurTimeRect.top = baseTop + pRes->ProgBarRect.height + 4;
    pRes->CurTimeRect.width = TIME_RECT_WIDTH;
    pRes->CurTimeRect.height = CURTIME_HEIGHT;

    pRes->TotalTimeRect.left = Fwl_GetLcdWidth() - frameWidth - TIME_RECT_WIDTH;
    pRes->TotalTimeRect.top = baseTop + pRes->ProgBarRect.height + 4;
    pRes->TotalTimeRect.width = TIME_RECT_WIDTH;
    pRes->TotalTimeRect.height = TOTALTIME_HEIGHT;

    //pRes->NameRect.left = pRes->PlayerRect.left + NAME_RECT_LEFT_SPACE;
    //pRes->NameRect.top = pRes->PlayerRect.top;
    //pRes->NameRect.width = pRes->PlayerRect.width - NAME_RECT_LEFT_SPACE*2;
    //pRes->NameRect.height = pRes->PlayerRect.height - pRes->LRIconRect.top;

    pRes->NameRect.left = pRes->PlayerRect.left + NAME_RECT_LEFT_SPACE;
    pRes->NameRect.top = pRes->PlayerRect.top;
    pRes->NameRect.width = pRes->PlayerRect.width - NAME_RECT_LEFT_SPACE*2;
    pRes->NameRect.height = 30;

    //small sound icon 
    AKBmpGetInfo(pRes->pSmallSound, &pRes->SmallSoundRect.width, &pRes->SmallSoundRect.height, AK_NULL);
    pRes->SmallSoundRect.left = pRes->ProgBarRect.left;
    tmpLen = (pRes->ProgBarRect.height - pRes->SmallSoundRect.height) / 2;
    pRes->SmallSoundRect.top = pRes->ProgBarRect.top + tmpLen;

    //big sound icon
    AKBmpGetInfo(pRes->pBigSound, &pRes->BigSoundRect.width, &pRes->BigSoundRect.height, AK_NULL);
    pRes->BigSoundRect.left = pRes->ProgBarRect.left + pRes->ProgBarRect.width - pRes->BigSoundRect.width;
    tmpLen = (pRes->ProgBarRect.height - pRes->BigSoundRect.height) / 2;
    pRes->BigSoundRect.top = pRes->ProgBarRect.top + tmpLen;

    //sound bar rect
    pRes->SoundBarRect.left = pRes->SmallSoundRect.left + pRes->SmallSoundRect.width + HORIZONTAL_DISTANCE/2;
    pRes->SoundBarRect.top = pRes->ProgBarRect.top;
    tmpLen = pRes->SmallSoundRect.width + pRes->BigSoundRect.width + HORIZONTAL_DISTANCE;
    pRes->SoundBarRect.width = pRes->ProgBarRect.width - tmpLen;
    pRes->SoundBarRect.height = pRes->ProgBarRect.height;

    //volume display area: small sound icon \ volume bar \big sound icon \volume number
    pRes->VolRct.left = pRes->ProgBarRect.left;
    pRes->VolRct.top = pRes->ProgBarRect.top;
    pRes->VolRct.width = pRes->ProgBarRect.width;
    pRes->VolRct.height= pRes->CurTimeRect.top + pRes->CurTimeRect.height - pRes->ProgBarRect.top;

    pAudio_Player_Parm->LyricRect.left = frameWidth + TIME_RECT_WIDTH;
    pAudio_Player_Parm->LyricRect.top = baseTop + pRes->ProgBarRect.height + 4;
    pAudio_Player_Parm->LyricRect.width = Fwl_GetLcdWidth() - 2 * (frameWidth + TIME_RECT_WIDTH);
    pAudio_Player_Parm->LyricRect.height = LYRIC_HEIGHT;

	pAudio_Player_Parm->ImgRect.width = 60;
    pAudio_Player_Parm->ImgRect.height = 60;
    pAudio_Player_Parm->ImgRect.left = (Fwl_GetLcdWidth()-pAudio_Player_Parm->ImgRect.width)>>1;
    pAudio_Player_Parm->ImgRect.top = pRes->NameRect.top+pRes->NameRect.height+4;
    
}


static T_VOID AdPlyr_GetRes(T_AUDIO_PLAYER_RES *pResource)
{
    T_U32 i;
    T_U32 len;

    AK_ASSERT_PTR_VOID(pResource, "AdPlyr_GetRes(): pResource null");

    for (i=0; i<3; i++)
        pResource->pLR[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_LEFTSOUND + i, &len);
    for (i=0; i<2; i++)
        pResource->pAudition[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_NOAUDITION + i, &len);
    for (i=0; i<4; i++)
        pResource->pCycle[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_NORMAL_CYC + i, &len);
    for (i=0; i<3; i++)
        pResource->pAB[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_A0B0 + i, &len);
    for (i=0; i<11; i++)
        pResource->pSpeed[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_SPEED05 + i, &len);
    pResource->pBarBack = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_BAR_BACKGROUND, &len);
    for (i=0; i<15; i++)
        pResource->pBar[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_BAR0 + i, &len);

    for (i=0; i<2; i++)
    {
		pResource->pPlay[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_AUDIO_PLAY + i, &len);
    }

    for (i=0; i<2; i++)
    {
         pResource->pVolume[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_VOLUME + i, &len);
    }

    pResource->pPanel = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_AUDIO_CONTROL_PANEL, &len);
    
    pResource->pStop = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_STOP, &len);

    pResource->pPrevious = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_PREVIOUS, &len);

    pResource->pNext = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_NEXT, &len);

    pResource->pSmallSound = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_SMALLSOUND, &len);
    pResource->pBigSound = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_BIGSOUND, &len);

    pResource->pBckgrnd = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_AUDIO_BACKGROUND, &len);
}

static T_VOID AdPlyr_RefreshCallBack(T_VOID)
{
    if (pAudio_Player_Parm)
    {
         AdPlyr_SetRefresh(AUDIOPLAYER_TOTALTIME_REFRESH | AUDIOPLAYER_LYRIC_REFRESH \
                | AUDIOPLAYER_LR_REFRESH | AUDIOPLAYER_AUDITION_REFRESH \
                | AUDIOPLAYER_AB_REFRESH | AUDIOPLAYER_SPEED_REFRESH \
                | AUDIOPLAYER_NAME_REFRESH | AUDIOPLAYER_PLAY_REFRESH
                | AUDIOPLAYER_PIC_REFRESH);
         pAudio_Player_Parm->Resource.ProgRfrshAll = AK_TRUE;
    }
}

static T_VOID resume_audio_player(T_VOID)
{
    T_USTR_INFO uText;          /*unicode text*/

	AK_DEBUG_OUTPUT("Calling resume_audio_player() ... ...\n");
	
    Utl_UStrCpy(uText, Res_GetStringByID(eRES_STR_PUB_MUSIC_PLAYER));

    //重新获取图像资源
    AdPlyr_GetRes(&pAudio_Player_Parm->Resource);

    TopBar_EnableMenuButton();
    
    TopBar_SetTitle(uText);
    TopBar_Show(TB_REFRESH_ALL);
    //TopBar_Refresh();
    
    AdPlyr_StartRefreshTimer();
	
    AdPlyr_SetRefresh(AUDIOPLAYER_REFRESH_ALL);
	
	gs.bPlayer2Saver = AK_FALSE;
}

static T_VOID suspend_audio_player(T_VOID)
{
	AK_DEBUG_OUTPUT("Calling suspend_audio_player() ... ...\n");

	gs.bPlayer2Saver = AK_TRUE;

    AdPlyr_StopRefreshTimer();
    
    //TopBar_DisableMenuButton();

    /* Stop audio backward or forward when suspend*/
    if (AUDIOPLAYER_STATE_BACKWARD == AudioPlayer_GetCurState())
    {
        AudioPlayer_BackwardStateHandle(AUDIOPLAYER_ACT_STOP_BACKWARD);
    }
    if (AUDIOPLAYER_STATE_FORWARD == AudioPlayer_GetCurState())
    {
        AudioPlayer_ForwardStateHandle(AUDIOPLAYER_ACT_STOP_FORWARD);
    }
	
}
extern T_AUDIOPLAYER *p_audio_player;
//handle touch sreen events, either transforming it to key evnet, or do corresponding  process
static T_MMI_KEYPAD AdPlyr_HitButtonCallBack(T_POS x, T_POS y, T_EVT_PARAM *pEventParm)
{
    T_AUDIO_PLAYER_RES *pResource;
    T_MMI_KEYPAD phyKey;
    T_RECT rect;

    phyKey.keyID = kbNULL;
    phyKey.pressType = PRESS_SHORT;

    pResource = &pAudio_Player_Parm->Resource;

    //get the rect of cancel button
    rect = TopBar_GetRectofCancelButton();

    //hit cancel button
    if (PointInRect(&rect, x, y))
    {
        phyKey.keyID = kbCLEAR;
        phyKey.pressType = PRESS_SHORT;
    }

    
    //hit menu button icon
    if (AK_TRUE == TopBar_GetMenuButtonState())
    {
        TopBar_GetRectofMenuButton(&rect);
        if (PointInRect(&rect, x, y))
        {
            phyKey.keyID = kbMENU;
        }
    }

    //hit play/pause button
    if (PointInRect(&pResource->PlayRect, x, y))
    {
        phyKey.keyID = kbOK;
    }

    //hit stop button
    if (PointInRect(&pResource->StopRect, x, y))
    {
        phyKey.keyID = kbOK;
        phyKey.pressType = PRESS_LONG;
    }

    //hit previous button
    if (PointInRect(&pResource->PreviousRect, x, y))
    {
        phyKey.keyID = kbUP;
    }

    //hit next button
    if (PointInRect(&pResource->NextRect, x, y))
    {
        phyKey.keyID = kbDOWN;
    }

    //hit volume button
    if (PointInRect(&pResource->VolBtnRect, x, y))
    {
        phyKey.keyID = kbVOICE_UP;
    }

    //hit A->B mode button
    if (PointInRect(&pResource->ABRect, x, y))
    {
        phyKey.keyID = kbLEFT;
    }
    
    //hit repeat mode button, go to the  s_audio_list_repeat_mode
    if (PointInRect(&pResource->CycleRect, x, y))
    {
		// Cancel Repeat Mode When Player Is Triggered From Explorer/Recorder
		if (pAudio_Player_Parm->InitEvent != M_EVT_1)
		{
        	m_triggerEvent(M_EVT_REPEAT_MODE, AK_NULL);
		}
    }

    //hit tone mode button
    if (PointInRect(&pResource->LRIconRect, x, y))
    {
        m_triggerEvent(M_EVT_TONE_MODE, AK_NULL);
    }

    //hit pre listen mode button, go to the  s_audio_list_pre_time
    if (PointInRect(&pResource->AuditionRect, x, y))
    {
        m_triggerEvent(M_EVT_PRE_TIME, AK_NULL);
    }

    //hit  mode button, go to the  s_audio_set_speed
    if (PointInRect(&pResource->SpeedRect, x, y)
		&& AudioPlayer_GetCurState() != AUDIOPLAYER_STATE_AUDITION
		&& !(AudioPlayer_GetCurState()==AUDIOPLAYER_STATE_PAUSE
		&& AudioPlayer_GetOldState()==AUDIOPLAYER_STATE_AUDITION))
    {
        m_triggerEvent(M_EVT_SET_SPEED, AK_NULL);
    }

    //hit  progress rect, go to the  correspongding position
    if(p_audio_player->bAllowSeek
	   && AudioPlayer_GetCurState() != AUDIOPLAYER_STATE_AB_PLAY
	   && !(p_audio_player->OldState == AUDIOPLAYER_STATE_AB_PLAY
	   && p_audio_player->CurState  == AUDIOPLAYER_STATE_PAUSE))
    {
	    if (PointInRect(&pResource->ProgBarRect, x, y)
	        && (pAudio_Player_Parm->ChngVlmFlg == 0)
	        && gb.AudioPreTime == 0)
	    {
	        T_U32 SeekTime = 0;
	        T_U32 TotalTime = 0;
	        T_SEEK_TYPE seek_dir;
	        T_U32 curTime = 0;

	        TotalTime = AudioPlayer_GetTotalTime();

	        SeekTime = (T_U32)((T_U64)TotalTime * (x - pResource->ProgBarRect.left) / pResource->ProgBarRect.width);//xuyr Swd200001270

	        if (AK_NULL != p_audio_player && SeekTime > 0)
	        {
		        if(AudioPlayer_GetCurState() == AUDIOPLAYER_STATE_PAUSE)
				{
					p_audio_player->TimeBeforSeek = p_audio_player->CurTime;
					p_audio_player->CurTime = SeekTime;
					AdPlyr_SetRefresh(AUDIOPLAYER_PROGRESS_REFRESH | AUDIOPLAYER_TOTALTIME_REFRESH \
	                | AUDIOPLAYER_CURTIME_REFRESH | AUDIOPLAYER_LYRIC_REFRESH);
					return phyKey;
				}
				
	            if (SeekTime > curTime)
	            {
	                seek_dir = SEEK_TYPE_FORWARD;
	            }
	            else
	            {
	                seek_dir = SEEK_TYPE_BACKWARD;
	            }
	            Fwl_AudioPause();
	            Fwl_AudioSeek(SeekTime);
	        }
	    }
    }//xuyr
	
    //hit  sound bar, adjust volume number
    if (PointInRect(&pResource->SoundBarRect, x, y))
    {
        if (pAudio_Player_Parm->ChngVlmFlg > 0)
        {
            T_S16 SeekVol = 0;

           // the volume rang is integer from 0 to AK_VOLUME_MAX
            SeekVol = (AK_VOLUME_MAX * (x - pResource->SoundBarRect.left) % pResource->SoundBarRect.width == 0)
                        ? (AK_VOLUME_MAX * (x - pResource->SoundBarRect.left) / pResource->SoundBarRect.width)
                        : (AK_VOLUME_MAX * (x - pResource->SoundBarRect.left) / pResource->SoundBarRect.width) + 1;

            if (SeekVol < 0)
            {
                SeekVol = 0;
            }
            else if (SeekVol > AK_VOLUME_MAX)
            {
                SeekVol = AK_VOLUME_MAX;
            }
            
            if (SeekVol != Fwl_GetAudioVolume())
            {
                Fwl_AudioSetVolume(SeekVol);
            }

            //delay the volume bar turnning into progress bar
            pAudio_Player_Parm->ChngVlmFlg = 10;
        }
    }
	//hit the small soune rect, turn the volume to 0
    else if (PointInRect(&pResource->SmallSoundRect, x, y))
    {
        if (pAudio_Player_Parm->ChngVlmFlg > 0)
        {
            if (0 != Fwl_GetAudioVolume())
            {
                Fwl_AudioSetVolume(0);
            }
            
            //delay the volume bar turnning into progress bar
            pAudio_Player_Parm->ChngVlmFlg = 10;
        }
    }
	//hit the big sound rect, turn the volume to 0
    else if (PointInRect(&pResource->BigSoundRect, x, y))
    {
        if (pAudio_Player_Parm->ChngVlmFlg > 0)
        {
            if (AK_VOLUME_MAX != Fwl_GetAudioVolume())
            {
                Fwl_AudioSetVolume(AK_VOLUME_MAX);
            }

            //delay the volume bar turnning into progress bar
            pAudio_Player_Parm->ChngVlmFlg = 10;
        }
    }
	else
	{
		AdPlyr_SetRefresh(AUDIOPLAYER_TOTALTIME_REFRESH \
                        | AUDIOPLAYER_CURTIME_REFRESH \
                        | AUDIOPLAYER_LYRIC_REFRESH \
                        | AUDIOPLAYER_PROGRESS_REFRESH);
		pAudio_Player_Parm->ChngVlmFlg = 0; // kill volume progress bar at once
	}

    return phyKey;
}


#endif
