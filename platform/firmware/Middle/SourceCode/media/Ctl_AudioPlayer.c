#include "Ctl_AudioPlayer.h"
#include "Eng_AutoPowerOff.h"
#include "Eng_String.h"
#include "Fwl_osfs.h"
#include "Fwl_pfKeypad.h"
#include "Fwl_pfAudio.h"
#include "Eng_String.h"
#include "Eng_FileManage.h"
#include "Eng_TopBar.h"
#include "Eng_Math.h"
#include "Eng_DataConvert.h"
#include "Eng_FileManage.h"
#include "Ctl_Fm.h"
#include "fwl_keyhandler.h"
#include "Fwl_Initialize.h"
#include "eng_debug.h"
#include "fwl_oscom.h"
#include "Eng_Graph.h"
#include "AKAppMgr.h"
#include "Log_MediaPlayer.h"
#include "lib_sdfilter.h"
#include "eng_gblstring.h"
#include "Fwl_public.h"
#include "Fwl_osfs.h"
#include "Lib_state_api.h"
#include "hal_timer.h"
#include "Fwl_sd.h"
#include "Eng_ImgConvert.h"
#include "Media_Demuxer_Lib.h"
#include "Ctl_APlayerList.h"
#include "Fwl_Waveout.h"
#include "Fwl_tscrcom.h"

#ifdef SUPPORT_VISUAL_AUDIO	
#include "Log_MediaVisualAudio.h"
#endif

#define AUDIOPLAYER_AB_DISABLE      0xfffffffb

#define AUDIO_FAST_MIN              5000

#define AUDIO_PLAYER_TIMER          200
#define MIN_AB_REPEAT_INTERVAL		1000 	// ms

#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1)) //Just use for Chip_AK3753
#define AUDIO_SEEK_TIMER          500
#endif

T_AUDIOPLAYER *p_audio_player = AK_NULL;


T_FILE_TYPE AudioFileType[] = {
		/*FILE_TYPE_RMVB,FILE_TYPE_RM,*/
        FILE_TYPE_MP1, FILE_TYPE_MP2, FILE_TYPE_MP3,
        FILE_TYPE_AAC, FILE_TYPE_AMR, FILE_TYPE_WMA,FILE_TYPE_ASF, FILE_TYPE_MID,
        FILE_TYPE_MIDI, FILE_TYPE_ADPCM, FILE_TYPE_WAV, FILE_TYPE_M4A, FILE_TYPE_MP4,        
        FILE_TYPE_FLAC_NATIVE, FILE_TYPE_FLAC_OGG, FILE_TYPE_FLAC_OGA, FILE_TYPE_APE,
        FILE_TYPE_ADIF, FILE_TYPE_ADTS, FILE_TYPE_ALT, FILE_TYPE_NONE
    };

T_hSemaphore g_AudioPlayerSem;

// Set a flag to cancel user`s operation while autoswitch song. Bug ID :SD3700001521
static T_BOOL bAutoSwitch = AK_FALSE;

#ifdef OS_WIN32
T_VOID SetCallback(T_VOID);
#endif

static T_BOOL AudioPlayer_Forward(T_VOID);
static T_BOOL AudioPlayer_Forward_Test(T_VOID);
static T_BOOL AudioPlayer_Backward(T_VOID);
static T_BOOL AudioPlayer_Backward_Test(T_VOID);
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_StopStateHandle(T_AUDIOPLAYER_ACT action);
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_PlayStateHandle(T_AUDIOPLAYER_ACT action);
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_BackGroundPlayStateHandle(T_AUDIOPLAYER_ACT action);
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_AB_PlayStateHandle(T_AUDIOPLAYER_ACT action);
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_AuditionStateHandle(T_AUDIOPLAYER_ACT action);
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_PauseStateHandle(T_AUDIOPLAYER_ACT action);
//static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_MenuStateHandle(T_AUDIOPLAYER_ACT action);
//static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_ListStateHandle(T_AUDIOPLAYER_ACT action);

static T_AUDIOPLAYER_ACT AudioPlayer_MappingEvent(T_EVT_CODE event, T_EVT_PARAM *pEventParm);

static T_BOOL AudioPlayer_StartPlayTimer(T_U32 time);
static T_BOOL AudioPlayer_StopAuditionTimer(T_VOID);
static T_BOOL AudioPlayer_StartAuditionTimer(T_U32 time);

#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1)) //Just use for Chip_AK3753
static T_BOOL AudioPlayer_StartSeekTimer(T_AUDIOPLAYER_ACT action);
static T_BOOL AudioPlayer_StopSeekTimer(T_VOID);

static T_VOID audio_forward_callback_func(T_TIMER timer_id, T_U32 delay);
static T_VOID audio_backward_callback_func(T_TIMER timer_id, T_U32 delay);
#endif

static T_VOID AudioPlayer_FetchNextFile(T_U8 endType, T_USTR_FILE pFilePath);

static T_VOID audio_audition_callback_func(T_TIMER timer_id, T_U32 delay);
static T_VOID audio_play_callback_func(T_TIMER timer_id, T_U32 delay);

static T_VOID* AudioPlayer_GetPrevNext(T_AUDIOPLAYER_ACT action);
static T_AUDIOPLAYER_ACT AudioPlayer_UserKey_Handle(T_MMI_KEYPAD phyKey);

T_AUDIOPLAYER_INIT_RET AudioPlayer_Init(T_VOID)
{
    

    //if current player state is playing,return.
    if (AUDIOPLAYER_STATE_PLAY == AudioPlayer_GetCurState())
    {
        AK_ASSERT_VAL((AUDIOPLAYER_STATE_PLAY == p_audio_player->CurState), "AudioPlayer_Init(): CurState != BACKGROUNDPLAY", AUDIOPLAYER_INIT_ERROR);
        return AUDIOPLAYER_INIT_PLAY;          
    }

    //if p_audio_player is null, malloc space.
    AK_Obtain_Semaphore(g_AudioPlayerSem, AK_SUSPEND);//xuyr@
    if (AK_NULL == p_audio_player)
    {
        p_audio_player = (T_AUDIOPLAYER *)Fwl_Malloc(sizeof(T_AUDIOPLAYER));
        AK_ASSERT_PTR(p_audio_player, "AudioPlayer_Init(): malloc T_AUDIOPLAYER error", AUDIOPLAYER_INIT_ERROR);
    }
    Utl_MemSet(p_audio_player, 0x0, sizeof(T_AUDIOPLAYER));
	
    p_audio_player->pIconExplorer 	= AK_NULL;
    p_audio_player->Action 			= AUDIOPLAYER_ACT_NONE;
    p_audio_player->CurState 		= AUDIOPLAYER_STATE_STOP;
    p_audio_player->fState_Handle 	= AudioPlayer_StopStateHandle;
    p_audio_player->pMidiBuf 		= AK_NULL;
    p_audio_player->PlayTimer 		= ERROR_TIMER;
    p_audio_player->AuditionTimer 	= ERROR_TIMER;

#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1))//for Chip_ak3753 keypad ==1
	p_audio_player->SeekTimer 		= ERROR_TIMER;
#endif

    p_audio_player->CurTime 		= 0;
    p_audio_player->TotalTime 		= 0;
    p_audio_player->TimeBeforSeek 	= 0;
    p_audio_player->CurType 		= MEDIALIB_MEDIA_UNKNOWN;
    p_audio_player->Repeat_A 		= AUDIOPLAYER_AB_DISABLE;
    p_audio_player->Repeat_B 		= AUDIOPLAYER_AB_DISABLE;
    p_audio_player->Channel 		= AUDIOPLAYER_CHANNEL_STEREO;
    p_audio_player->Interface 		= AUDIOPLAYER_INTERFACE_MAIN;
    p_audio_player->SuspendFlag 	= 0;
    p_audio_player->fRefreshCallback 		= AK_NULL;
    p_audio_player->fGetLyricCallback 		= AK_NULL;
    p_audio_player->fSetNameInfoCallback 	= AK_NULL;
    Utl_MemSet(&p_audio_player->path, 0, sizeof(T_USTR_FILE));

	AK_Release_Semaphore(g_AudioPlayerSem);//xuyr@
	
    // exit fm
    //Ctl_FmFree();

    // disable auto power off
    AutoPowerOffDisable(FLAG_AUDIO);
    gb.AudioPlaySpeed = _SD_WSOLA_1_0;//_SD_TEMPO_05;
    // set play speed
    //Fwl_AudioSetPlaySpeed(gb.AudioPlaySpeed);
    //Fwl_AudioSetEQMode(p_audio_player->pMPlayer,gs.AudioToneMode);

	AK_Change_Priority(IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA)), 50);	
	
    return AUDIOPLAYER_INIT_OK;
}



/**
 * @brief   Destroy the AudioPlayer module.
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_VOID
 * @retval  T_VOID
 */
T_VOID AudioPlayer_Destroy(T_VOID)
{
    if (AK_NULL == p_audio_player)
    {
        return;
    }

    gb.AudioPreTime = 0;

    if(AK_TRUE == gb.bInExplorer)
    {
        p_audio_player->CurState = AUDIOPLAYER_STATE_STOP;
    }
    else
    {
        AudioPlayer_SaveCurrentPlayList(p_audio_player->pIconExplorer);
    }
    
    if (AUDIOPLAYER_STATE_PLAY != p_audio_player->CurState)
    {
        gb.AudioPlaySpeed = _SD_WSOLA_1_0;
        //Fwl_AudioSetPlaySpeed(p_audio_player->pMPlayer, gb.AudioPlaySpeed);

        // stop timer
        AudioPlayer_ChangeState(AUDIOPLAYER_STATE_STOP);
        AudioPlayer_StopAuditionTimer();
        AudioPlayer_StopPlayTimer();

        Fwl_AudioStop(T_END_TYPE_USER);
        //Fwl_AudioDisableDA();
        AK_DEBUG_OUTPUT("aud destory\n");
        
        if(gb.bInExplorer)
        {
            AudioPlayer_Free();
        }

        AutoPowerOffEnable(FLAG_AUDIO);
		MList_Close(eMEDIA_LIST_AUDIO);
    }
    else
    {
         // stop timer
        AudioPlayer_StopAuditionTimer();
        AudioPlayer_StopPlayTimer();
    }
}

/**
 * @brief   free the memory.
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_VOID
 * @retval  T_VOID
 */
T_VOID AudioPlayer_Free(T_VOID)
{
	AK_Obtain_Semaphore(g_AudioPlayerSem, AK_SUSPEND);//xuyr@
    if (AK_NULL == p_audio_player)
   	{
    	AK_Release_Semaphore(g_AudioPlayerSem);//xuyr@
        return;
    }
   
    // destory AUDIOPLAYER
    if (AK_NULL == p_audio_player->pIconExplorer)
    {
	    AK_Release_Semaphore(g_AudioPlayerSem);//xuyr@
        return;
    }

    IconExplorer_Free(p_audio_player->pIconExplorer);
    Fwl_Free(p_audio_player->pIconExplorer);
    p_audio_player = Fwl_Free(p_audio_player);

	AK_Release_Semaphore(g_AudioPlayerSem);//xuyr@

	Fwl_AudioDisableDA();
    AutoPowerOffEnable(FLAG_AUDIO);

	AK_Change_Priority(IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA)), 100);

	AK_DEBUG_OUTPUT("Freed Audio Player.\n");
}


T_BOOL AudioPlayer_OpenFile(T_pWSTR pFilepath, T_U32 FileId)
{
    T_U32  time;
    T_ICONEXPLORER_ITEM *p;
    T_U16 disk = UNICODE_C;
	T_USTR_FILE     filename;
    T_USTR_FILE     path;
	
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_OpenFile(): p_audio_player null", AK_FALSE);
    AK_ASSERT_PTR(pFilepath, "AudioPlayer_OpenFile(): pFilepath null", AK_FALSE);
//    AK_ASSERT_VAL((FileId != 0), "AudioPlayer_OpenFile(): FileId == 0", AK_FALSE);

    Fwl_AudioStop(T_END_TYPE_USER);
    p_audio_player->Repeat_A 		= AUDIOPLAYER_AB_DISABLE;
    p_audio_player->Repeat_B 		= AUDIOPLAYER_AB_DISABLE;
    p_audio_player->CurTime 		= 0;
    p_audio_player->TotalTime 		= 0;
    p_audio_player->TimeBeforSeek 	= 0;
    p_audio_player->PlayFileId 		= 0;
	p_audio_player->bAllowSeek		= AK_FALSE;
	
    Utl_MemSet((T_U8 *)&p_audio_player->path, 0, sizeof(T_USTR_FILE));
    Utl_MemSet((T_U8 *)p_audio_player->PlayFileName, 0, sizeof(T_USTR_FILE));
    p_audio_player->CurType = MEDIALIB_MEDIA_UNKNOWN;
    //Utl_MemSet(&p_audio_player->CurMetaInfo, 0, sizeof(_SD_T_META_INFO));
    p_audio_player->Channel = AUDIOPLAYER_CHANNEL_STEREO;
    p_audio_player->SuspendFlag = AK_FALSE;
   
    // get text pointer
	Utl_USplitFilePath(pFilepath, path, filename);
	if (0 != Utl_UStrCmpN(filename, (T_U16 *)AudioPlayer_GetFocusName(), (T_U16)Utl_UStrLen(AudioPlayer_GetFocusName())))
	{
		memset(&pFilepath[0], 0, sizeof(pFilepath));
		return AK_FALSE;
	}
	
    if (!Fwl_AudioOpenFile((T_pWSTR)pFilepath))
    {
        AK_DEBUG_OUTPUT("open file failed\n");
        return AK_FALSE;
    }
    
    if (!MPlayer_HasAudio())
    {
        MPlayer_Close();
        return AK_FALSE;
    }

	p_audio_player->bAllowSeek = MPlayer_AllowSeek();
	/*
    if (media_info.m_AudioInfo.m_Type == _SD_MEDIA_TYPE_MP3)
    {
        // find song info in baseset first
        if (AudioPlayer_GetInfo(pFilepath,&p_audio_player->PalySongInfo) == AK_FALSE)
        {
            AudioPlayer_GetMetaInfo(pFilepath,&p_audio_player->PalySongInfo);
            AK_DEBUG_OUTPUT("---- AudioPlayer_GetMetaInfo\n");
        }
    }
	*/
    p_audio_player->TotalTime 	= MPlayer_GetTotalTime();
    p_audio_player->CurType 	= MPlayer_GetMediaType();	
    p_audio_player->PlayFileId 	= FileId;
    p = IconExplorer_GetItem(p_audio_player->pIconExplorer, FileId);
    AK_ASSERT_PTR(p->pText, "AudioPlayer_OpenFile(): pFileName null", AK_FALSE);
    Utl_UStrCpyN(p_audio_player->PlayFileName, p->pText, sizeof(T_USTR_FILE)/2);
    Utl_UStrCpyN(p_audio_player->path, (T_U16 *)pFilepath, sizeof(T_USTR_FILE)/2);

    // can play the song, so need play timer
    AudioPlayer_StartPlayTimer(AUDIO_PLAYER_TIMER);

    // get lyric
    if (p_audio_player->fGetLyricCallback != AK_NULL)
    {
        p_audio_player->fGetLyricCallback(pFilepath);
    }

	
	if (!Fwl_AudioPlay(p_audio_player))
	{
		AK_DEBUG_OUTPUT("play failed\n");
		//WaveOut_CloseFade();
		return AK_FALSE;
	}

#ifdef SUPPORT_VISUAL_AUDIO
	if (VisualAudio_IsInit())
	{
		VA_Increase_Draw_Type(); //switch Visual Audio Draw Type
	}
#endif		

    // start audition timer if in audition state
    if (p_audio_player->CurState == AUDIOPLAYER_STATE_AUDITION && gb.AudioPreTime > 0)
    {
        if ((T_U32)(gb.AudioPreTime*1000) < p_audio_player->TotalTime)
        {
            time = gb.AudioPreTime*1000;
            AudioPlayer_StartAuditionTimer(time);
        }
    }

	//WaveOut_OpenFade();
    
    AK_DEBUG_OUTPUT("+++++++++++++++++++++++++++++++++++\n");
	MList_UpdatePlayInfo(((T_INDEX_CONTENT *)(p->pContent))->id, eMEDIA_LIST_AUDIO);
    AK_DEBUG_OUTPUT("-----------------------------------\n");
    
    disk = GetFilePathDisk((T_pWSTR)pFilepath);
	
    return AK_TRUE;
}



T_VOID AudioPlayer_BSeekToA(T_VOID)
{	
	AK_Obtain_Semaphore(g_AudioPlayerSem, AK_SUSPEND);
	
	if (AK_NULL == p_audio_player)
	{	
		AK_Release_Semaphore(g_AudioPlayerSem);
		Fwl_Print(C3, M_AB, "p_audio_player null\n");
		return;
	}

    if (AUDIOPLAYER_AB_DISABLE != p_audio_player->Repeat_A \
        && AUDIOPLAYER_AB_DISABLE != p_audio_player->Repeat_B
        //&& AUDIOPLAYER_STATE_AB_PLAY == p_audio_player->CurState
        && AUDIOPLAYER_STATE_PAUSE != p_audio_player->CurState
        && MPlayer_GetCurTime() >= p_audio_player->Repeat_B )
    {
		Fwl_Print(C3, M_AB, "From B Seek To A\n");
		
		Fwl_AudioPause();
		
		if (!Fwl_AudioSeek(p_audio_player->Repeat_A/1000*1000+500))
        {
			Fwl_Print(C3, M_AB, "Seek Error\n");
        }
    }

	AK_Release_Semaphore(g_AudioPlayerSem);
}


static T_AUDIOPLAYER_ACT APlayer_MarkAB(T_VOID)
{
	T_AUDIOPLAYER_ACT action;
	
	if (AUDIOPLAYER_STATE_BACKWARD == p_audio_player->CurState)
    {
        return AUDIOPLAYER_ACT_MOVEBACK;
    }
	
   	if(p_audio_player->bAllowSeek
		&& (p_audio_player->CurState == AUDIOPLAYER_STATE_PLAY
            || p_audio_player->CurState == AUDIOPLAYER_STATE_AB_PLAY))
    {           
        AK_Obtain_Semaphore(g_AudioPlayerSem, AK_SUSPEND);

		if ((p_audio_player->Repeat_A == AUDIOPLAYER_AB_DISABLE) 
			&& (p_audio_player->Repeat_B == AUDIOPLAYER_AB_DISABLE))
        {
			// Mark A
        	action = AUDIOPLAYER_ACT_MARK_A;
            p_audio_player->Repeat_A = p_audio_player->CurTime;
			
			AK_DEBUG_OUTPUT("AB:	Mark A: %d.\n", p_audio_player->CurTime);
        }
        else if ((p_audio_player->Repeat_B == AUDIOPLAYER_AB_DISABLE)
		&& (p_audio_player->CurTime > p_audio_player->Repeat_A))
        {
			// Mark B
			if (p_audio_player->CurTime < p_audio_player->Repeat_A + MIN_AB_REPEAT_INTERVAL)
            	p_audio_player->Repeat_B = p_audio_player->Repeat_A + MIN_AB_REPEAT_INTERVAL;
			else
				p_audio_player->Repeat_B = p_audio_player->CurTime;
			
            action = AUDIOPLAYER_ACT_PLAY_AB;
			
			AK_DEBUG_OUTPUT("AB:	Mark B: %d.\n", p_audio_player->CurTime);
        }
        else
        {
			// Cancel Repeat
            p_audio_player->Repeat_A = p_audio_player->Repeat_B = AUDIOPLAYER_AB_DISABLE;
            action = AUDIOPLAYER_ACT_STOP_PLAY_AB;
			
			AK_DEBUG_OUTPUT("AB:	Cancel.\n");
        }

        AK_Release_Semaphore(g_AudioPlayerSem);
        }
	
	return action;
}

static T_AUDIOPLAYER_ACT APlayer_UserKeyHandleOK(T_MMI_KEYPAD *phyKey)
{
	if (phyKey->pressType == PRESS_LONG)
    {
        Fwl_KeyStop();
        return AUDIOPLAYER_ACT_STOP;
    }
    else if (phyKey->pressType == PRESS_SHORT)
    {
        if (p_audio_player->CurState == AUDIOPLAYER_STATE_PLAY \
            || p_audio_player->CurState == AUDIOPLAYER_STATE_AB_PLAY \
            || p_audio_player->CurState == AUDIOPLAYER_STATE_AUDITION)
        {
            return AUDIOPLAYER_ACT_PAUSE;
        }
        else if (p_audio_player->CurState == AUDIOPLAYER_STATE_STOP)
        {
            return AUDIOPLAYER_ACT_PLAY;
        }
        else if(p_audio_player->CurState == AUDIOPLAYER_STATE_PAUSE)
        {
            return AUDIOPLAYER_ACT_STOP_PAUSE;
        }
    }

    return AUDIOPLAYER_ACT_NONE;
}

static T_AUDIOPLAYER_ACT APlayer_UserKeyHandleLeft(T_MMI_KEYPAD *phyKey)
{
	 if (phyKey->pressType == PRESS_SHORT)
    {
		//ab play
		return APlayer_MarkAB();
    }

	
    return AUDIOPLAYER_ACT_NONE;
}

static T_AUDIOPLAYER_ACT APlayer_UserKeyHandleRight(T_MMI_KEYPAD *phyKey)
{

    
    return AUDIOPLAYER_ACT_NONE;
}

static T_AUDIOPLAYER_ACT APlayer_UserKeyHandleUp(T_MMI_KEYPAD phyKey)
{
	if (phyKey.pressType == PRESS_LONG)
    {
		//backward
        if ((p_audio_player->CurState == AUDIOPLAYER_STATE_PLAY
	          || (p_audio_player->CurState == AUDIOPLAYER_STATE_PAUSE
	              && p_audio_player->Repeat_B == AUDIOPLAYER_AB_DISABLE))
             	  && p_audio_player->bAllowSeek)
        {
            p_audio_player->TimeBeforSeek = p_audio_player->CurTime;
            return AUDIOPLAYER_ACT_BACKWARD;
        }
        else
        {
            Fwl_KeyStop();
        }
    } 
	else if (phyKey.pressType == PRESS_SHORT)
	{
		AK_DEBUG_OUTPUT("ZZZZ:	RIGHT SHORT State: %d.\n", p_audio_player->CurState);
		if (AUDIOPLAYER_STATE_BACKWARD == p_audio_player->CurState)
		{
			return AUDIOPLAYER_ACT_MOVEBACK;
		}
		else if (p_audio_player->Interface == AUDIOPLAYER_INTERFACE_MAIN \
			 	 && (p_audio_player->CurState == AUDIOPLAYER_STATE_PLAY \
	          	 || p_audio_player->CurState == AUDIOPLAYER_STATE_STOP \
				 || p_audio_player->CurState == AUDIOPLAYER_STATE_PAUSE)) //
        {
            return AUDIOPLAYER_ACT_SWITCH_PREV;
        }
	}
    else if (phyKey.pressType == PRESS_UP
			 && AUDIOPLAYER_STATE_BACKWARD == p_audio_player->CurState)
    {
        return AUDIOPLAYER_ACT_STOP_BACKWARD;
    }

    return AUDIOPLAYER_ACT_NONE;
}

static T_AUDIOPLAYER_ACT APlayer_UserKeyHandleDown(T_MMI_KEYPAD phyKey)
{
	if (phyKey.pressType == PRESS_LONG)
	{//forward
		AK_DEBUG_OUTPUT("ZZZZ:	RIGHT LONG State: %d.\n", p_audio_player->CurState);
		if ( (p_audio_player->CurState == AUDIOPLAYER_STATE_PLAY
			  || (p_audio_player->CurState == AUDIOPLAYER_STATE_PAUSE
				  && p_audio_player->Repeat_B == AUDIOPLAYER_AB_DISABLE))
			 && p_audio_player->bAllowSeek)
		{
			p_audio_player->TimeBeforSeek = p_audio_player->CurTime;
			return AUDIOPLAYER_ACT_FORWARD;
		}
		else
		{
		   Fwl_KeyStop();
		}
	}
	else if (phyKey.pressType == PRESS_SHORT)
	{
		AK_DEBUG_OUTPUT("ZZZZ:	RIGHT SHORT State: %d.\n", p_audio_player->CurState);
		if (AUDIOPLAYER_STATE_FORWARD == p_audio_player->CurState)
		{
			return AUDIOPLAYER_ACT_MOVEFORWARE;
		}
		else if (p_audio_player->Interface == AUDIOPLAYER_INTERFACE_MAIN \
					&& (p_audio_player->CurState == AUDIOPLAYER_STATE_PLAY \
					|| p_audio_player->CurState == AUDIOPLAYER_STATE_STOP \
					|| p_audio_player->CurState == AUDIOPLAYER_STATE_PAUSE))
		{
			
			{
				return AUDIOPLAYER_ACT_SWITCH_NEXT;
			}
		}
	}
	else if (phyKey.pressType == PRESS_UP
		&& AUDIOPLAYER_STATE_FORWARD == p_audio_player->CurState)
	{
		return AUDIOPLAYER_ACT_STOP_FORWARD;
	}
    
    return AUDIOPLAYER_ACT_NONE;
}

static T_AUDIOPLAYER_ACT AudioPlayer_UserKey_Handle(T_MMI_KEYPAD phyKey)
{
    T_AUDIOPLAYER_ACT action = AUDIOPLAYER_ACT_NONE;
	// AK_DEBUG_OUTPUT("Calling "__func__"() ...\n");

	//Disable User`s handle while autoswitch song
	if(bAutoSwitch)
		return AUDIOPLAYER_ACT_NONE;

#if (KEYPAD_NUM == 7)
    if (phyKey.keyID == kbUP)
    {
        if (PRESS_SHORT != phyKey.pressType)
        {
            return action;
        }

        phyKey.keyID = kbVOICE_UP;
        if (Fwl_GetAudioVolumeStatus())
            Fwl_AudioSetVolume(Fwl_AudioVolumeAdd());
    }

    if (phyKey.keyID == kbDOWN)
    {
        if (PRESS_SHORT != phyKey.pressType)
        {
            return action;
        }

        phyKey.keyID = kbVOICE_DOWN;
        if (Fwl_GetAudioVolumeStatus())
            Fwl_AudioSetVolume(Fwl_AudioVolumeSub());
    }
#endif
    
    switch (phyKey.keyID)
    {
    case kbVOICE_UP:
    case kbVOICE_DOWN:
        action = AUDIOPLAYER_ACT_VOL_CHANGE;
        break;
		
    case kbUP:
        return APlayer_UserKeyHandleUp(phyKey);
        break;
		
    case kbDOWN:
		return APlayer_UserKeyHandleDown(phyKey);
		break;
		
    case kbCLEAR:
        if (phyKey.pressType == PRESS_LONG)
        {
            action = AUDIOPLAYER_ACT_EXITHOME;
        }
        else
        {
            action = AUDIOPLAYER_ACT_EXIT;
        }
        break;
		
    case kbOK:
        return APlayer_UserKeyHandleOK(&phyKey);
        break;
		
    case kbMENU:
        if (phyKey.pressType == PRESS_SHORT)
        {
            action = AUDIOPLAYER_ACT_CONFIG;
        }
        break;
		
    case kbLEFT:
        return APlayer_UserKeyHandleLeft(&phyKey);
        break;
		
    case kbRIGHT:
        return APlayer_UserKeyHandleRight(&phyKey);
        break;
		
    default:
    	break;
    }

    return action;

}

static T_AUDIOPLAYER_ACT APlayer_TScrHandle(T_AUDIOPLAYER* pAPlayer, T_EVT_PARAM *pEventParm)
{
	T_POS x, y;
	
    x = (T_POS)pEventParm->s.Param2;
    y = (T_POS)pEventParm->s.Param3;

    switch (pEventParm->s.Param1) 
    {
    case eTOUCHSCR_UP:
        /* if the point(x,y) hit in the control buttons rect,  
                transform it to the corresponding key */
        return AudioPlayer_UserKey_Handle(pAPlayer->fHitButtonCallback(x, y, pEventParm));
        break;
		
    case eTOUCHSCR_DOWN:
         break;
		 
    case eTOUCHSCR_MOVE:
         break;
		 
    default:
         break;
    }

	return AUDIOPLAYER_ACT_NONE;
}


// mapping event to audio player action
static T_AUDIOPLAYER_ACT AudioPlayer_MappingEvent(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_MMI_KEYPAD phyKey;
    T_AUDIOPLAYER_ACT action = AUDIOPLAYER_ACT_NONE;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_MappingEvent(): p_audio_player null", AUDIOPLAYER_ACT_NONE);

    if(M_EVT_NEXT == event || M_EVT_1 == event)
    {
        if (AUDIOPLAYER_STATE_PLAY == p_audio_player->CurState)
        {
            p_audio_player->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;

            AudioPlayer_StartPlayTimer(AUDIO_PLAYER_TIMER);
            p_audio_player->fSetNameInfoCallback();
        }
        else
        {
            if (gb.bInExplorer == AK_TRUE)
            {
                T_FILELIST FileList;

                p_audio_player->pIconExplorer = (T_ICONEXPLORER *)Fwl_Malloc(sizeof(T_ICONEXPLORER));
                AK_ASSERT_PTR(p_audio_player->pIconExplorer, "AudioPlayer_MappingEvent(): pIconExplorer malloc error", AK_FALSE);

                if (!FileList_Init(&FileList, AUDIOPLAYER_MAX_ITEM_QTY, FILELIST_SORT_NONE,  AudioPlayer_IsSupportFile))
                {
                    AK_DEBUG_OUTPUT("\nfile list init fail\n");
                    return AUDIOPLAYER_ACT_NONE;
                }

                if (FileList_Add(&FileList, _T(AUDIOLIST_TMP_FILE), FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER) != FILELIST_ADD_SUCCESS)
                    AK_DEBUG_OUTPUT("AudioPlayer_Init enter a null file\r\n");

                MenuStructInit(p_audio_player->pIconExplorer);
                FileList_ToIconExplorer(&FileList, p_audio_player->pIconExplorer, eINDEX_TYPE_AUDIO, p_audio_player->pathExplorer);
                FileList_Free(&FileList);
            }
            else
            {
                p_audio_player->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
            }

            return AUDIOPLAYER_ACT_PLAY;
        }
    }
    else if (M_EVT_2 == event)
    {
        action =  AUDIOPLAYER_ACT_AUDITION;
    }    
    else if (M_EVT_USER_KEY == event)
    {
    	// key event mapping
        phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_PRESS_TYPE)pEventParm->c.Param2;

        action = AudioPlayer_UserKey_Handle(phyKey);
    }    
    else if (M_EVT_TOUCH_SCREEN == event)
    {
    	 // touch screen event mapping
    	action = APlayer_TScrHandle(p_audio_player, pEventParm);
    }
	
    return action;
}


static T_AUDIOPLAYER_HANDLE_RET APlayer_Switch(T_AUDIOPLAYER_ACT action)
{
	T_USTR_FILE    pFilePath = {0};
	T_INDEX_CONTENT *pcontent = AK_NULL;
	
	//AudioPlayer_Suspend();
    IconExplorer_SetFocus(p_audio_player->pIconExplorer, p_audio_player->PlayFileId);
    pcontent = (T_INDEX_CONTENT *)AudioPlayer_GetPrevNext(action);

    if (AK_NULL != pcontent)
    {
		MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
    }

    if (0 == pFilePath[0] && gb.bInExplorer)
	{
		Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
	}
    
    if (0 != pFilePath[0] && p_audio_player->CurState == AUDIOPLAYER_STATE_PLAY)
    {
    	T_U32   FileId;
    	
        FileId = IconExplorer_GetItemFocusId(p_audio_player->pIconExplorer);
                
        if (!FileMgr_CheckFileIsExist(pFilePath))
        {
            MsgBox_InitAfx(&p_audio_player->MsgBox, 1, ctFAILURE, csFILE_NOT_EXIST, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&p_audio_player->MsgBox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&p_audio_player->MsgBox);
            AudioPlayer_Stop();

            IconExplorer_DelItem(p_audio_player->pIconExplorer, FileId);
        }
        else if (!AudioPlayer_OpenFile(pFilePath, FileId))
        {
			if (0 == pFilePath[0])
				MsgBox_InitAfx(&p_audio_player->MsgBox, 1, ctFAILURE, csNOT_IN_AUDIOLIST, MSGBOX_INFORMATION);
			else	
				MsgBox_InitAfx(&p_audio_player->MsgBox, 1, ctFAILURE, csAUDIO_FILE_ERROR, MSGBOX_INFORMATION);
			
            MsgBox_SetDelay(&p_audio_player->MsgBox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&p_audio_player->MsgBox);
            AudioPlayer_Stop();

            IconExplorer_DelItem(p_audio_player->pIconExplorer, FileId);
        }
    }
    else if (0 != pFilePath[0] && p_audio_player->CurState == AUDIOPLAYER_STATE_PAUSE)
    {
        AudioPlayer_Stop();
    }
    return AUDIOPLAYER_HANDLE_SWITCH;
}


T_AUDIOPLAYER_HANDLE_RET AudioPlayer_Handler(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_AUDIOPLAYER_ACT   action;
    T_AUDIOPLAYER_HANDLE_RET    ret = AUDIOPLAYER_HANDLE_NONE;
   	
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_Handler(): p_audio_player null", AUDIOPLAYER_HANDLE_ERROR);
	
    // mapping event to audio player action
    action = AudioPlayer_MappingEvent(event, pEventParm);
		
    switch (action)
    {
    case AUDIOPLAYER_ACT_NONE:
        break;
		
    case AUDIOPLAYER_ACT_EXIT:
        ret = AUDIOPLAYER_HANDLE_EXIT;
        break;
		
    case AUDIOPLAYER_ACT_EXITHOME:
        ret = AUDIOPLAYER_HANDLE_EXITHOME;
        break;
		
    case AUDIOPLAYER_ACT_CONFIG:
        ret = AUDIOPLAYER_HANDLE_MENU;
        break;
		
    case AUDIOPLAYER_ACT_VOL_CHANGE:
        ret = AUDIOPLAYER_HANDLE_VOICEREFRESH;
        break;
		
    case AUDIOPLAYER_ACT_AUDITION:
        if (AudioPlayer_StartAudition())
        {
            ret = AUDIOPLAYER_HANDLE_STATECHANGE;
        }
        break;
		
    case AUDIOPLAYER_ACT_STOP:                 /**< go to stop state */
        AudioPlayer_Stop();
		AK_DEBUG_OUTPUT("Stoped Audio Player\n");
        return AUDIOPLAYER_HANDLE_STATECHANGE;
        break;
		
    case AUDIOPLAYER_ACT_SWITCH_PREV:
    case AUDIOPLAYER_ACT_SWITCH_NEXT:
		return APlayer_Switch(action);
        break;
		
    default:
		AK_DEBUG_OUTPUT("Calling fState_Handle(%d).\n", action);
        ret = p_audio_player->fState_Handle(action);
        break;
    }

    return ret;
}

T_AUDIOPLAYER_STATE AudioPlayer_GetCurState(T_VOID)
{
    if (AK_NULL != p_audio_player)
    {
        return p_audio_player->CurState;
    }
    else
    {
        return AUDIOPLAYER_STATE_NONE;
    }
}

T_AUDIOPLAYER_STATE AudioPlayer_GetOldState(T_VOID)
{
    if (AK_NULL != p_audio_player)
    {
        return p_audio_player->OldState;
    }
    else
    {
        return AUDIOPLAYER_STATE_NONE;
    }
}

static T_VOID AudioPlayer_FetchNextFile(T_U8 endType, T_USTR_FILE pFilePath)
{
    T_ICONEXPLORER_ITEM *p;
	T_INDEX_CONTENT *pcontent = AK_NULL;
	
    AK_ASSERT_PTR_VOID(p_audio_player, "AudioPlayer_FetchNextFile(): p_audio_player null");
	AK_ASSERT_PTR_VOID(pFilePath, "AudioPlayer_FetchNextFile(): pFilePath null");

    // get file path
    if (IconExplorer_GetItemQty(p_audio_player->pIconExplorer) > 0)
    {
		IconExplorer_SetFocus(p_audio_player->pIconExplorer, p_audio_player->PlayFileId);
		
		// Current Play Media ERROR
		if (T_END_TYPE_ERR == endType)
		{
			pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer);
			IconExplorer_MoveFocus(p_audio_player->pIconExplorer, ICONEXPLORER_DIRECTION_DOWN);

			// Return NULL PATH When ONLY ONE Media 
			if (pcontent == (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer))
			{
				memset(&pFilePath[0], 0, sizeof(pFilePath));
				return;
			}
			
			if (FILELIST_FETCH_REPEAT_SINGLE != gs.AudioRepMode)
				IconExplorer_MoveFocus(p_audio_player->pIconExplorer, ICONEXPLORER_DIRECTION_UP);
		}   
		
        //switch at pre play state,
        switch (p_audio_player->CurState)
        {
        case AUDIOPLAYER_STATE_PAUSE:
            if (p_audio_player->OldState != AUDIOPLAYER_STATE_AUDITION)
            {
                memset(pFilePath, 0, sizeof(T_USTR_FILE));
                AK_DEBUG_OUTPUT("AudioPlayer_FetchNextFile exception in pause state %d\r\n", p_audio_player->CurState);
                break;
            }
            
        // audition play must use sequence mode to get file
        case AUDIOPLAYER_STATE_AUDITION:
            p = IconExplorer_GetItemFocus(p_audio_player->pIconExplorer);
            if ((p != AK_NULL) && (p->pNext != AK_NULL))
            {
                IconExplorer_MoveFocus(p_audio_player->pIconExplorer, ICONEXPLORER_DIRECTION_DOWN);
                pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer);
                if (AK_NULL != pcontent)
				{
					MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
				}

				if (0 == pFilePath[0] && gb.bInExplorer)
				{
					Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
				}
            }
            else
            {
                memset(pFilePath, 0, sizeof(T_USTR_FILE));
            }
            IconExplorer_SetRefresh(p_audio_player->pIconExplorer, ICONEXPLORER_REFRESH_ITEM);
            break;
			
        case AUDIOPLAYER_STATE_PLAY:
        case AUDIOPLAYER_STATE_BACKGROUNDPLAY:
        case AUDIOPLAYER_STATE_FORWARD:
        case AUDIOPLAYER_STATE_BACKWARD:
            if (gs.AudioRepMode == FILELIST_FETCH_REPEAT_SINGLE)
            {
                pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer);
                if (AK_NULL != pcontent)
				{
					MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
				}

				if (0 == pFilePath[0] && gb.bInExplorer)
				{
					Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
				}
            }
            else if (gs.AudioRepMode == FILELIST_FETCH_RANDOM)
            {
                T_U32 i, count = 0, random;

                p = IconExplorer_GetItemFocus(p_audio_player->pIconExplorer);
                while (count++ < 3)
                {
                    random = Fwl_GetRand(IconExplorer_GetItemQty(p_audio_player->pIconExplorer));
                    for (i=0; i<random; i++)
                        IconExplorer_MoveFocus(p_audio_player->pIconExplorer, ICONEXPLORER_DIRECTION_DOWN);

                    if (p != IconExplorer_GetItemFocus(p_audio_player->pIconExplorer))
                        break;
                }

                pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer);
                if (AK_NULL != pcontent)
				{
					MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
				}

				if (0 == pFilePath[0] && gb.bInExplorer)
				{
					Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
				}
            }
            else if (gs.AudioRepMode == FILELIST_FETCH_REPEAT)
            {
                IconExplorer_MoveFocus(p_audio_player->pIconExplorer, ICONEXPLORER_DIRECTION_DOWN);
                pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer);
                if (AK_NULL != pcontent)
				{
					MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
				}

				if (0 == pFilePath[0] && gb.bInExplorer)
				{
					Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
				}
            }
            else
            {
                p = IconExplorer_GetItemFocus(p_audio_player->pIconExplorer);
                if ((p != AK_NULL) && (p->pNext != AK_NULL))
                {
                    IconExplorer_MoveFocus(p_audio_player->pIconExplorer, ICONEXPLORER_DIRECTION_DOWN);
                    pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer);
                    if (AK_NULL != pcontent)
					{
						MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
					}

					if (0 == pFilePath[0] && gb.bInExplorer)
					{
						Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
					}
                }
                else
                {
                    memset(pFilePath, 0, sizeof(T_USTR_FILE));
                }
            }

            IconExplorer_SetRefresh(p_audio_player->pIconExplorer, ICONEXPLORER_REFRESH_ITEM);
            break;
			
        case AUDIOPLAYER_STATE_STOP:
        case AUDIOPLAYER_STATE_AB_PLAY:
        default:
            memset(pFilePath, 0, sizeof(T_USTR_FILE));
            AK_DEBUG_OUTPUT("AudioPlayer_FetchNextFile exception %d\r\n", p_audio_player->CurState);
            break;
        }
    }
    else
    {
        memset(pFilePath, 0, sizeof(T_USTR_FILE));
    }

}

static T_VOID* AudioPlayer_GetPrevNext(T_AUDIOPLAYER_ACT action)
{
    T_ICONEXPLORER_DIRECTION dir;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_GetPrev(): p_audio_player null", AK_NULL);

    dir = (action == AUDIOPLAYER_ACT_SWITCH_PREV) ? ICONEXPLORER_DIRECTION_UP : ICONEXPLORER_DIRECTION_DOWN;
    IconExplorer_MoveFocus(p_audio_player->pIconExplorer, dir);
    IconExplorer_SetRefresh(p_audio_player->pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer);
}

T_BOOL AudioPlayer_StartAudition(T_VOID)
{
    T_USTR_FILE 	pFilePath = {0};
    T_U32 			FileId;	
    T_INDEX_CONTENT *pcontent = AK_NULL;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_StartAudition(): p_audio_player null", AK_FALSE);

    Fwl_AudioStop(T_END_TYPE_USER);
    AudioPlayer_StopPlayTimer();
    AudioPlayer_StopAuditionTimer();
    IconExplorer_SetFocusByIndex(p_audio_player->pIconExplorer, 0);

	pcontent = IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer);
	if (AK_NULL != pcontent)
	{
		MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
	}

	if (0 == pFilePath[0] && gb.bInExplorer)
	{
		Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
	}
	
    if (0 != pFilePath[0])
    {
        do
        {
			FileId = IconExplorer_GetItemFocusId(p_audio_player->pIconExplorer);
    
	        if (AudioPlayer_OpenFile(pFilePath, FileId))
	        {
	        	p_audio_player->fSetNameInfoCallback();
	            p_audio_player->fRefreshCallback();	            
	            AudioPlayer_ChangeState(AUDIOPLAYER_STATE_AUDITION);
	            
	            if ((T_U32)(gb.AudioPreTime*1000) < p_audio_player->TotalTime)
	            {
	                AudioPlayer_StartAuditionTimer(gb.AudioPreTime*1000);
	            }
	            
	            return AK_TRUE;	            
	        }
	        else
	        {
	        	memset(pFilePath, 0, sizeof(T_USTR_FILE));
				pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentNextById(p_audio_player->pIconExplorer, FileId);
				if (AK_NULL != pcontent)
				{
					MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
				}

				if (0 == pFilePath[0] && gb.bInExplorer)
				{
					Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
				}
				
	            IconExplorer_DelItem(p_audio_player->pIconExplorer, FileId); 
	        }            
        } while(0 != pFilePath[0]);
    }
    
    AudioPlayer_Stop(); 
    
    return AK_FALSE;
}

T_BOOL AudioPlayer_StopAudition(T_VOID)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_StopAudition(): p_audio_player null", AK_FALSE);

    AudioPlayer_StopAuditionTimer();
    if (AUDIOPLAYER_STATE_PAUSE != p_audio_player->CurState)
        AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PLAY);
    else if (p_audio_player->OldState == AUDIOPLAYER_STATE_AUDITION)    //预听暂停后关闭了预听，再按OK播放时候，如果OldState还是AUDIOPLAYER_STATE_AUDITION，处理会混乱。
        p_audio_player->OldState = AUDIOPLAYER_STATE_PLAY;
    gb.AudioPreTime = 0;

    return AK_TRUE;
}


T_VOID AudioPlayer_Stop_Step1(T_VOID)
{
	if (AK_NULL == p_audio_player)
    {
        return;
    }

    gb.AudioPreTime = 0;

    Fwl_AudioStop(T_END_TYPE_USER);
    AudioPlayer_StopPlayTimer();
    AudioPlayer_StopAuditionTimer();
    
    p_audio_player->Repeat_A = AUDIOPLAYER_AB_DISABLE;
    p_audio_player->Repeat_B = AUDIOPLAYER_AB_DISABLE;
    p_audio_player->CurTime = 0;
    p_audio_player->TotalTime = 0;
    p_audio_player->TimeBeforSeek = 0;
    p_audio_player->PlayFileId = 0;
    Utl_MemSet((T_U8 *)&p_audio_player->path, 0, sizeof(T_USTR_FILE));
    Utl_MemSet((T_U8 *)p_audio_player->PlayFileName, 0, sizeof(T_USTR_FILE));
    p_audio_player->CurType = _SD_MEDIA_TYPE_UNKNOWN;
//    Utl_MemSet(&p_audio_player->CurMetaInfo, 0, sizeof(_SD_T_META_INFO));
    p_audio_player->Channel = AUDIOPLAYER_CHANNEL_STEREO;
    p_audio_player->SuspendFlag = AK_FALSE;

    AutoPowerOffEnable(FLAG_AUDIO);
    AudioPlayer_ChangeState(AUDIOPLAYER_STATE_STOP);
}

T_VOID AudioPlayer_Stop_Step2(T_VOID)
{
	if (AK_NULL == p_audio_player)
    {
        return;
    }

    if (AUDIOPLAYER_STATE_BACKGROUNDPLAY == p_audio_player->OldState)
    {
        AudioPlayer_Destroy();
        AudioPlayer_Free(); 		
    }
}

T_VOID AudioPlayer_Stop(T_VOID)
{
	AudioPlayer_Stop_Step1();
    AudioPlayer_Stop_Step2();    
}

T_BOOL AudioPlayer_AutoSwitch(T_U8 endType)
{
    T_BOOL 			ret = AK_FALSE;
    T_USTR_FILE 	pFilePath = {0};
    T_U32   		FileId;
    T_INDEX_CONTENT *pcontent = AK_NULL;
	T_U32 			curFocusId;

	AK_Obtain_Semaphore(g_AudioPlayerSem, AK_SUSPEND);
	if(AK_NULL == p_audio_player)
	{
		AK_Release_Semaphore(g_AudioPlayerSem);
		AK_DEBUG_OUTPUT("AudioPlayer_Switch(): p_audio_player null");
		return ret;
	}
	
    bAutoSwitch = AK_TRUE;	
    AudioPlayer_StopAuditionTimer();
	AudioPlayer_StopPlayTimer();

    Fwl_AudioStop(endType);
    p_audio_player->fRefreshCallback();
    
	curFocusId = IconExplorer_GetItemFocusId(p_audio_player->pIconExplorer);

	AudioPlayer_FetchNextFile(endType, pFilePath);
    if (0 != pFilePath[0])
    {
    	AK_DEBUG_OUTPUT("auto switch ***filepath = 0x%x.\n", pFilePath);
		
        while(0 != pFilePath[0] && !ret)
        {
            /*totaltime < 1s*/
			if(p_audio_player->TotalTime / 1000 == 0)
            {
                if(FILELIST_FETCH_REPEAT_SINGLE == gs.AudioRepMode 
                    || 1 == IconExplorer_GetItemQty(p_audio_player->pIconExplorer))
                {
                    ret = AK_FALSE;
                    break;
                }
            }
			
            FileId = IconExplorer_GetItemFocusId(p_audio_player->pIconExplorer);

            if (!(ret = AudioPlayer_OpenFile(pFilePath, FileId)))
            {
            	memset(pFilePath, 0, sizeof(T_USTR_FILE));
                pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentNextById(p_audio_player->pIconExplorer, FileId);
				if (AK_NULL != pcontent)
				{
					MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
				}

				if (0 == pFilePath[0] && gb.bInExplorer)
				{
					Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
				}
				
                IconExplorer_DelItem(p_audio_player->pIconExplorer, FileId);
            }
        }

    }
	IconExplorer_SetFocus(p_audio_player->pIconExplorer, curFocusId);

    p_audio_player->fSetNameInfoCallback();
    if (!ret)
    {   
        AK_DEBUG_OUTPUT("stop *****\n");
        AudioPlayer_Stop_Step1();
		if (AUDIOPLAYER_STATE_BACKGROUNDPLAY == p_audio_player->OldState)
		{
			VME_ReTriggerEvent(M_EVT_AUDIO_STOP,(T_U32)&p_audio_player);
			AK_DEBUG_OUTPUT("AudioPlayer_Stop VME_ReTriggerEvent M_EVT_AUDIO_STOP &p_audio_player=%x",&p_audio_player);
		}

        if(!(gb.bAudioPlaySM))
        {
            WaveOut_CloseFade();
        }
        
    }
    else
    {
        // play, background play and forward will go to the end of the song
        // play and backgrond play not change the state
        if (AUDIOPLAYER_STATE_FORWARD == AudioPlayer_GetCurState())
        {
            // forward to the end of the song, turn to next song and turn to play state
            AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PLAY);
        }
    }
	
	bAutoSwitch = AK_FALSE;
	AK_Release_Semaphore(g_AudioPlayerSem);//xuyr@

    return ret;
}

T_BOOL AudioPlayer_IsPlayingFile(T_pCWSTR pFilepath)
{
    // if  p_audio_player not null
    // check the file is playing or not
   
    if ((AK_NULL == p_audio_player )||(_SD_MEDIA_TYPE_UNKNOWN == p_audio_player->CurType))
        return AK_FALSE;

    if (Utl_UStrCmp(p_audio_player->path, (T_U16 *)pFilepath) == 0)
        return AK_TRUE;

    return AK_FALSE;
}

T_BOOL AudioPlayer_ChangeState(T_AUDIOPLAYER_STATE NewState)
{
    T_BOOL ret = AK_TRUE;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_ChangeState(): p_audio_player null", AK_FALSE);
    AK_ASSERT_VAL((NewState < AUDIOPLAYER_STATE_NUM && NewState > AUDIOPLAYER_STATE_NONE), \
                "AudioPlayer_ChangeState(): newstate error", AK_FALSE);

    p_audio_player->OldState = p_audio_player->CurState;
    p_audio_player->CurState = NewState;

	AK_DEBUG_OUTPUT("APLAYER:	State Change --- %d -> %d.\n", p_audio_player->OldState, NewState);
    switch (NewState)
    {
    case AUDIOPLAYER_STATE_STOP:
        p_audio_player->fRefreshCallback();
        p_audio_player->fSetNameInfoCallback();
        p_audio_player->fState_Handle = AudioPlayer_StopStateHandle;
    	break;
	
    case AUDIOPLAYER_STATE_PLAY:
        //AudioPlayer_StartPlayTimer(AUDIO_PLAYER_TIMER);
        //p_audio_player->fRefreshCallback();
        p_audio_player->fState_Handle = AudioPlayer_PlayStateHandle;

//    AK_DEBUG_OUTPUT("change state from stop to play");

//    FreqMgr_StateCheckIn(FREQ_FACTOR_AUDIO, FREQ_PRIOR_AUDIO);
        break;

    case AUDIOPLAYER_STATE_BACKGROUNDPLAY:
        p_audio_player->fState_Handle = AudioPlayer_BackGroundPlayStateHandle;
    	break;
	
    case AUDIOPLAYER_STATE_AB_PLAY:
        p_audio_player->fState_Handle = AudioPlayer_AB_PlayStateHandle;
    	break;
	
    case AUDIOPLAYER_STATE_AUDITION:
        p_audio_player->fState_Handle = AudioPlayer_AuditionStateHandle;
    	break;
	
    case AUDIOPLAYER_STATE_PAUSE:
        // p_audio_player->fRefreshCallback();
        p_audio_player->fState_Handle = AudioPlayer_PauseStateHandle;
    	break;
	
    case AUDIOPLAYER_STATE_FORWARD:
        p_audio_player->fState_Handle = AudioPlayer_ForwardStateHandle;
    	break;
	
    case AUDIOPLAYER_STATE_BACKWARD:
        p_audio_player->fState_Handle = AudioPlayer_BackwardStateHandle;
    	break;
	
    /* case AUDIOPLAYER_STATE_MENU:
       p_audio_player->fState_Handle = AudioPlayer_MenuStateHandle;
        break;
        case AUDIOPLAYER_STATE_LIST:
            p_audio_player->fState_Handle = AudioPlayer_ListStateHandle;
            break;
            */
    default:
       	ret = AK_FALSE;
       	AK_DEBUG_OUTPUT("AudioPlayer_ChangeState error\r\n");
   		break;
    }

//    TopBar_Show(TB_REFRESH_AUDIO_STATUS);
//    TopBar_Refresh();
    return ret;
}

static T_BOOL AudioPlayer_Forward(T_VOID)
{
    // validity judge
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_Forward(): p_audio_player null", AK_FALSE);

    //Fwl_AudioPause();
	AK_DEBUG_OUTPUT("Calling AudioPlayer_Forward() ... ...\n");
	
    if (p_audio_player->TotalTime > 0)
    {
        if ((p_audio_player->CurTime + AUDIO_FAST_MIN) < p_audio_player->TotalTime)
        {
            p_audio_player->CurTime += AUDIO_FAST_MIN;
        }
        else
        {
			//dengzhou 
			//当seek时间大于总时间时，跳到下一首
            p_audio_player->CurTime = 0;//p_audio_player->TotalTime - 1;
            Fwl_KeyStop();
            AudioPlayer_ForwardStateHandle(AUDIOPLAYER_ACT_SWITCH_NEXT);
        }
    }

    return AK_TRUE;
}

static T_BOOL AudioPlayer_Forward_Test(T_VOID)
{
   // T_U32 CurPlayTime = 0;

    // validity judge
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_Forward_Test(): p_audio_player null", AK_FALSE);

	AK_DEBUG_OUTPUT("Calling AudioPlayer_Forward_Test() ... ...\n");
	
    //Fwl_AudioPause();
    //if (AK_FALSE == AudioPlayer_IsSupportSeekType(p_audio_player->pMPlayer->mediaInfo.m_MediaType))
    if(!p_audio_player->bAllowSeek)//xuyr modi
    {
        return AK_FALSE;
    }
    if (p_audio_player->TotalTime > 0)
    {
        if (p_audio_player->CurTime + AUDIO_FAST_MIN <= p_audio_player->TotalTime)
        {
            //CurPlayTime = p_audio_player->CurTime + AUDIO_FAST_MIN;
            return AK_TRUE;
        }
        else
        {
            return AK_FALSE;
        }
    }

    return AK_FALSE;
}


static T_BOOL AudioPlayer_Backward(T_VOID)
{
    // validity judge
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_Backward(): p_audio_player null", AK_FALSE);

    //Fwl_AudioPause();
	AK_DEBUG_OUTPUT("Calling AudioPlayer_Backward() ... ...\n");
	
    if (p_audio_player->TotalTime > 0)
    {
        if (p_audio_player->CurTime > AUDIO_FAST_MIN)
        {
            p_audio_player->CurTime -= AUDIO_FAST_MIN;
        }
        else
        {
            p_audio_player->CurTime = 0;
            Fwl_KeyStop();
			
#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1))
			AudioPlayer_StopSeekTimer();
#endif			
			// Waiting "LEFT" Key UP.
            // AudioPlayer_BackwardStateHandle(AUDIOPLAYER_ACT_STOP_BACKWARD);
        }
    }

    return AK_TRUE;
}

static T_BOOL AudioPlayer_Backward_Test(T_VOID)
{
    //T_U32 CurPlayTime = 0;

    // validity judge
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_Backward_Test(): p_audio_player null", AK_FALSE);

	AK_DEBUG_OUTPUT("Calling AudioPlayer_Backward_Test() ... ...\n");
	
    //Fwl_AudioPause();
    //if (AK_FALSE == AudioPlayer_IsSupportSeekType(p_audio_player->pMPlayer->mediaInfo.m_MediaType))
    if(!p_audio_player->bAllowSeek)//xuyr modi
    {
        return AK_FALSE;
    }
    if (p_audio_player->TotalTime > 0)
    {
        if (p_audio_player->CurTime - AUDIO_FAST_MIN > 0)
        {
            //p_audio_player->CurTime = p_audio_player->CurTime - AUDIO_FAST_MIN;
            return AK_TRUE;
        }    
        else
        {
            AK_DEBUG_OUTPUT("AudioPlayer_Backward_Test seek error\r\n");
            return AK_FALSE;
        }
    }

    return AK_FALSE;
}

// stop state handle
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_StopStateHandle(T_AUDIOPLAYER_ACT action)
{
    T_AUDIOPLAYER_HANDLE_RET ret = AUDIOPLAYER_HANDLE_NONE;
    T_USTR_FILE pFilePath = {0};
    T_INDEX_CONTENT *pcontent = AK_NULL;
    T_U32   FileId;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_StopStateHandle(): p_audio_player null", AUDIOPLAYER_HANDLE_ERROR);

    AK_DEBUG_OUTPUT("AudioPlayer_StopStateHandle");

	AK_DEBUG_OUTPUT("Calling AudioPlayer_StopStateHandle() ... ...\n");
	
    switch (action)
    {
    case AUDIOPLAYER_ACT_PLAY:
		pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer);

		if (AK_NULL != pcontent)
		{
			MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
		}

		if (0 == pFilePath[0] && gb.bInExplorer)
		{
			Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
		}
		
        if (0 != pFilePath[0])
        {
            FileId = IconExplorer_GetItemFocusId(p_audio_player->pIconExplorer);

            /*some very short file's stopcallback perhaps happen between AudioPlayer_OpenFile and AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PLAY);
                          , it will make the order of playback in disorder, so suspend audio back application before open file*/
            // IThread_Suspend(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA));
            if (AudioPlayer_OpenFile(pFilePath, FileId))
            {
                //AudioPlayer_StartPlayTimer(AUDIO_PLAYER_TIMER);
                p_audio_player->fRefreshCallback();
                AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PLAY);
                ret = AUDIOPLAYER_HANDLE_STATECHANGE;
            }
            else
            {
				if (!pFilePath[0])
				{
					MsgBox_InitAfx(&p_audio_player->MsgBox, 1, ctFAILURE, csNOT_IN_AUDIOLIST, MSGBOX_INFORMATION);
					MsgBox_SetDelay(&p_audio_player->MsgBox, MSGBOX_DELAY_0);
					m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&p_audio_player->MsgBox);
				}
				else if (!FileMgr_CheckFileIsExist(pFilePath))
                {
                    MsgBox_InitAfx(&p_audio_player->MsgBox, 1, ctFAILURE, csFILE_NOT_EXIST, MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&p_audio_player->MsgBox, MSGBOX_DELAY_0);
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&p_audio_player->MsgBox);
                }
                else
                {
                    ret = AUDIOPLAYER_HANDLE_ERROR;
                }

                IconExplorer_DelItem(p_audio_player->pIconExplorer, FileId);
            }
            // IThread_Run(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA));
        }

        p_audio_player->fSetNameInfoCallback();

        break;
		
    default:
        break;
    }

    return ret;
}

// play state handle
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_PlayStateHandle(T_AUDIOPLAYER_ACT action)
{
    T_AUDIOPLAYER_HANDLE_RET ret = AUDIOPLAYER_HANDLE_NONE;
    T_BOOL SeekTest = AK_FALSE;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_PlayStateHandle(): p_audio_player null", AUDIOPLAYER_HANDLE_ERROR);

	AK_DEBUG_OUTPUT("Calling AudioPlayer_PlayStateHandle() ... ...\n");
	
    switch (action)
    {
    case AUDIOPLAYER_ACT_MARK_A:
        ret = AUDIOPLAYER_HANDLE_STATECHANGE;
        break;
		
    case AUDIOPLAYER_ACT_PLAY_AB:
        AudioPlayer_ChangeState(AUDIOPLAYER_STATE_AB_PLAY);
        ret= AUDIOPLAYER_HANDLE_STATECHANGE;
        break;
		
    case AUDIOPLAYER_ACT_FORWARD:
        AudioPlayer_StopPlayTimer();
        Fwl_AudioPause();

        SeekTest = AudioPlayer_Forward_Test();
        if (AK_TRUE == SeekTest)
        {
            AudioPlayer_ChangeState(AUDIOPLAYER_STATE_FORWARD);
            ret= AUDIOPLAYER_HANDLE_STATECHANGE;
			
#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1))
			AudioPlayer_StartSeekTimer(AUDIOPLAYER_ACT_FORWARD);
#endif
        }
        else
        {
            Fwl_KeyStop();
            Fwl_AudioResume();
            AudioPlayer_StartPlayTimer(AUDIO_PLAYER_TIMER);
        }
        break;
		
    case AUDIOPLAYER_ACT_BACKWARD:
        AudioPlayer_StopPlayTimer();
        Fwl_AudioPause();

        SeekTest = AudioPlayer_Backward_Test();
        if (AK_TRUE == SeekTest)
        {
            AudioPlayer_ChangeState(AUDIOPLAYER_STATE_BACKWARD);
            ret= AUDIOPLAYER_HANDLE_STATECHANGE;
			
#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1))
			AudioPlayer_StartSeekTimer(AUDIOPLAYER_ACT_BACKWARD);
#endif
        }
        else
        {
            Fwl_KeyStop();
            Fwl_AudioResume();
            AudioPlayer_StartPlayTimer(AUDIO_PLAYER_TIMER);
        }
        break;
		
    case AUDIOPLAYER_ACT_PAUSE:
        AudioPlayer_StopPlayTimer();
        Fwl_AudioPause();
        AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PAUSE);
        ret= AUDIOPLAYER_HANDLE_STATECHANGE;
        break;
		
    case AUDIOPLAYER_ACT_STOP_PLAY_AB:
        ret= AUDIOPLAYER_HANDLE_STATECHANGE;
        break;
		
    default:
        break;
    }

    return ret;
}

// background play state handle
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_BackGroundPlayStateHandle(T_AUDIOPLAYER_ACT action)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_BackGroundPlayStateHandle(): p_audio_player null", AUDIOPLAYER_HANDLE_ERROR);

	AK_DEBUG_OUTPUT("Calling AudioPlayer_BackGroundPlayStateHandle() ... ...\n");
	
    return AUDIOPLAYER_HANDLE_NONE;
}

// AB play state handle
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_AB_PlayStateHandle(T_AUDIOPLAYER_ACT action)
{
    T_AUDIOPLAYER_HANDLE_RET ret = AUDIOPLAYER_HANDLE_NONE;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_AB_PlayStateHandle(): p_audio_player null", AUDIOPLAYER_HANDLE_ERROR);

	AK_DEBUG_OUTPUT("Calling AudioPlayer_AB_PlayStateHandle() ... ...\n");
	
    switch (action)
    {
    case AUDIOPLAYER_ACT_STOP_PLAY_AB:
        AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PLAY);
		
        ret= AUDIOPLAYER_HANDLE_STATECHANGE;
        break;
		
    case AUDIOPLAYER_ACT_PAUSE:
        AudioPlayer_StopPlayTimer();
		AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PAUSE);
        Fwl_AudioPause();
		
        ret= AUDIOPLAYER_HANDLE_STATECHANGE;
        break;
		
    default:
        break;
    }

    return ret;
}

// audition state handle
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_AuditionStateHandle(T_AUDIOPLAYER_ACT action)
{
    T_AUDIOPLAYER_HANDLE_RET ret = AUDIOPLAYER_HANDLE_NONE;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_AuditionStateHandle(): p_audio_player null", AUDIOPLAYER_HANDLE_ERROR);

	AK_DEBUG_OUTPUT("Calling AudioPlayer_AuditionStateHandle() ... ...\n");
	
    switch (action)
    {
    case AUDIOPLAYER_ACT_PAUSE:
        AudioPlayer_StopPlayTimer();
        AudioPlayer_StopAuditionTimer();
        Fwl_AudioPause();
        AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PAUSE);
		
        ret = AUDIOPLAYER_HANDLE_STATECHANGE;
        break;

    default:
        break;
    }

    return ret;
}

// pause state handle
static T_AUDIOPLAYER_HANDLE_RET AudioPlayer_PauseStateHandle(T_AUDIOPLAYER_ACT action)
{
    T_AUDIOPLAYER_HANDLE_RET ret = AUDIOPLAYER_HANDLE_NONE;
    T_U32 time;
    T_U32 tPlayTime = 0;
//    T_BOOL SeekTest = AK_FALSE;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_PauseStateHandle(): p_audio_player null", AUDIOPLAYER_HANDLE_ERROR);

	AK_DEBUG_OUTPUT("Calling AudioPlayer_PauseStateHandle() ... ...\n");
	
    switch (action)
    {
    case AUDIOPLAYER_ACT_STOP_PAUSE:
		p_audio_player->SuspendFlag=AK_FALSE;//xuyr Swd200001299
		
        if (p_audio_player->OldState == AUDIOPLAYER_STATE_AUDITION)
        {
            if ((T_U32)(gb.AudioPreTime*1000) < p_audio_player->TotalTime)
            {
                time = gb.AudioPreTime*1000;
            }
            else
            {
                time = p_audio_player->TotalTime;
            }
            tPlayTime = AudioPlayer_GetPlayTime();
            time = (time >= tPlayTime + 1) ? (time - tPlayTime) : 1;
            AudioPlayer_StartAuditionTimer(time);
        }

		if (p_audio_player->OldState == AUDIOPLAYER_STATE_PLAY
			&& 0 < p_audio_player->TimeBeforSeek
			&& p_audio_player->TimeBeforSeek != p_audio_player->CurTime)
		{
			if (Fwl_AudioSeek(p_audio_player->CurTime))
	        {
	            AK_DEBUG_OUTPUT("AudioPlayer_pause is stopped! Seek ok!");
	        }

	        p_audio_player->TimeBeforSeek = 0;
		}
		
        AudioPlayer_StartPlayTimer(AUDIO_PLAYER_TIMER);
        Fwl_AudioResume();

        switch (p_audio_player->OldState)
        {
            case AUDIOPLAYER_STATE_AUDITION:
                AudioPlayer_ChangeState(AUDIOPLAYER_STATE_AUDITION);
                break;
            case AUDIOPLAYER_STATE_AB_PLAY:
                AudioPlayer_ChangeState(AUDIOPLAYER_STATE_AB_PLAY);
                break;
            default:
                AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PLAY);
                break;
        }
        ret= AUDIOPLAYER_HANDLE_STATECHANGE;
        break;
		
    /*case AUDIOPLAYER_ACT_FORWARD:
	       SeekTest = AudioPlayer_Forward();
	       if (AK_TRUE == SeekTest)
	       {
	           AudioPlayer_ChangeState(AUDIOPLAYER_STATE_FORWARD);
	           ret= AUDIOPLAYER_HANDLE_STATECHANGE;
	       }
	       break;
       case AUDIOPLAYER_ACT_BACKWARD:
	        SeekTest = AudioPlayer_Backward();
	        if (AK_TRUE == SeekTest)
	        {
	            AudioPlayer_ChangeState(AUDIOPLAYER_STATE_BACKWARD);
	            ret= AUDIOPLAYER_HANDLE_STATECHANGE;
	        }
	        break;*/
	        
    default:
        break;
    }

    return ret;
}

// forward state handle
T_AUDIOPLAYER_HANDLE_RET AudioPlayer_ForwardStateHandle(T_AUDIOPLAYER_ACT action)
{
    T_AUDIOPLAYER_HANDLE_RET ret = AUDIOPLAYER_HANDLE_NONE;
    T_BOOL ForwardRet = AK_FALSE;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_ForwardStateHandle(): p_audio_player null", AUDIOPLAYER_HANDLE_ERROR);

	AK_DEBUG_OUTPUT("Calling AudioPlayer_ForwardStateHandle() ... ...\n");
    switch (action)
    {
    case AUDIOPLAYER_ACT_MOVEFORWARE:
        ForwardRet = AudioPlayer_Forward();
		
        ret = (AK_TRUE == ForwardRet) ? AUDIOPLAYER_HANDLE_PROGRESSREFRESH : AUDIOPLAYER_HANDLE_NONE;
        break;
		
    case AUDIOPLAYER_ACT_STOP_FORWARD:

#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1))
		AudioPlayer_StopSeekTimer();
#endif
        if (Fwl_AudioSeek(p_audio_player->CurTime))
        {
            AK_DEBUG_OUTPUT("AudioPlayer_Forward is stopped! Seek ok!");
        }
        else
        {
            p_audio_player->CurTime = p_audio_player->TimeBeforSeek;
            AK_DEBUG_OUTPUT("AudioPlayer_Forward is stopped! But Seek failed!");
        }

        p_audio_player->TimeBeforSeek = 0;

        if (AUDIOPLAYER_STATE_PLAY == p_audio_player->OldState)
        {
            AudioPlayer_StartPlayTimer(AUDIO_PLAYER_TIMER);
            // Fwl_AudioResume();
            AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PLAY);
            ret= AUDIOPLAYER_HANDLE_STATECHANGE;
        }
        else if (AUDIOPLAYER_STATE_PAUSE == p_audio_player->OldState)
        {
            AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PAUSE);
            ret= AUDIOPLAYER_HANDLE_STATECHANGE;
        }
        break;
		
	case AUDIOPLAYER_ACT_SWITCH_NEXT:                 /**< go to stop state */

#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1))
		AudioPlayer_StopSeekTimer();
#endif
        AudioPlayer_AutoSwitch(T_END_TYPE_USER);
		
        ret = AUDIOPLAYER_HANDLE_STATECHANGE;
    default:
        break;
    }

    return ret;
}

// backward state handle
T_AUDIOPLAYER_HANDLE_RET AudioPlayer_BackwardStateHandle(T_AUDIOPLAYER_ACT action)
{
    T_AUDIOPLAYER_HANDLE_RET ret = AUDIOPLAYER_HANDLE_NONE;
    T_BOOL BackwardRet = AK_FALSE;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_BackwardStateHandle(): p_audio_player null", AUDIOPLAYER_HANDLE_ERROR);

    switch (action)
    {
    case AUDIOPLAYER_ACT_MOVEBACK:
        BackwardRet = AudioPlayer_Backward();
		
        ret = (AK_TRUE == BackwardRet) ? AUDIOPLAYER_HANDLE_PROGRESSREFRESH : AUDIOPLAYER_HANDLE_NONE;
        break;
		
    case AUDIOPLAYER_ACT_STOP_BACKWARD:

#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1))
		AudioPlayer_StopSeekTimer();
#endif

        if (Fwl_AudioSeek(p_audio_player->CurTime))
        {
            AK_DEBUG_OUTPUT("AudioPlayer_Backward is stopped! Seek ok!");
        }
        else
        {
            p_audio_player->CurTime = p_audio_player->TimeBeforSeek;
            AK_DEBUG_OUTPUT("AudioPlayer_Backward is stopped! But Seek failed!");
        }

        p_audio_player->TimeBeforSeek = 0;

        if (AUDIOPLAYER_STATE_PLAY == p_audio_player->OldState)
        {
            AudioPlayer_StartPlayTimer(AUDIO_PLAYER_TIMER);
            // Fwl_AudioResume();
            AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PLAY);
            ret= AUDIOPLAYER_HANDLE_STATECHANGE;
        }
        else if (AUDIOPLAYER_STATE_PAUSE == p_audio_player->OldState)
        {
            AudioPlayer_ChangeState(AUDIOPLAYER_STATE_PAUSE);
            ret= AUDIOPLAYER_HANDLE_STATECHANGE;
        }
        break;
		
    default:
        break;
    }

    return ret;
}



/*****************************************************************************/


static T_BOOL AudioPlayer_StartAuditionTimer(T_U32 time)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_StartAuditionTimer(): p_audio_player null", AK_FALSE);

    if(ERROR_TIMER != p_audio_player->AuditionTimer)
    {
        Fwl_StopTimer(p_audio_player->AuditionTimer);
        p_audio_player->AuditionTimer = ERROR_TIMER;
    }

	if (_SD_WSOLA_1_0 != gb.AudioPlaySpeed)
	{
    	Fwl_AudioSetPlaySpeed(_SD_WSOLA_1_0);
		gb.AudioPlaySpeed = _SD_WSOLA_1_0;
	}

    p_audio_player->AuditionTimer = Fwl_SetMSTimerWithCallback(time + 700, AK_FALSE, audio_audition_callback_func);
    AK_DEBUG_OUTPUT("AuditionTimer=%d\n", p_audio_player->AuditionTimer);

    return AK_TRUE;
}

static T_BOOL AudioPlayer_StopAuditionTimer(T_VOID)
{
	AK_DEBUG_OUTPUT("Calling AudioPlayer_StopAuditionTimer() ... ...\n");

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_StopAuditionTimer(): p_audio_player null", AK_FALSE);

    if(ERROR_TIMER != p_audio_player->AuditionTimer)
    {
        Fwl_StopTimer(p_audio_player->AuditionTimer);
        p_audio_player->AuditionTimer = ERROR_TIMER;
    }

    return AK_FALSE;
}


static T_BOOL AudioPlayer_StartPlayTimer(T_U32 time)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_StartPlayTimer(): p_audio_player null", AK_FALSE);

    if(ERROR_TIMER != p_audio_player->PlayTimer)
    {
        Fwl_StopTimer(p_audio_player->PlayTimer);
        p_audio_player->PlayTimer = ERROR_TIMER;
    }
	
    p_audio_player->PlayTimer = Fwl_SetMSTimerWithCallback(time, AK_TRUE, audio_play_callback_func);
    AK_DEBUG_OUTPUT("AudioPlayer_StartPlayTimer(): PlayTimer = %d.\n", p_audio_player->PlayTimer);

    return AK_TRUE;
}

T_BOOL AudioPlayer_StopPlayTimer(T_VOID)
{
	AK_DEBUG_OUTPUT("Calling AudioPlayer_StopPlayTimer() ... ...\n");
	
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_StopPlayTimer(): p_audio_player null", AK_FALSE);

    if(ERROR_TIMER != p_audio_player->PlayTimer)
    {
        Fwl_StopTimer(p_audio_player->PlayTimer);
        p_audio_player->PlayTimer = ERROR_TIMER;
    }

    return AK_FALSE;
}

#if (defined(CHIP_AK3753) && (KEYPAD_TYPE == 1)) //Just use for Chip_AK3753
static T_BOOL AudioPlayer_StartSeekTimer(T_AUDIOPLAYER_ACT action)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_StartSeekTimer(): p_audio_player null", AK_FALSE);
	
    if(ERROR_TIMER != p_audio_player->SeekTimer)
    {
        Fwl_StopTimer(p_audio_player->SeekTimer);
        p_audio_player->SeekTimer = ERROR_TIMER;
    }
	
	if(AUDIOPLAYER_ACT_FORWARD == action)
    {	
	    p_audio_player->SeekTimer = Fwl_SetMSTimerWithCallback(AUDIO_SEEK_TIMER, AK_TRUE, audio_forward_callback_func);
	    AK_DEBUG_OUTPUT("AudioPlayer_StartForwardTimer(): PlayTimer = %d.\n", p_audio_player->SeekTimer);
	}
	else if(AUDIOPLAYER_ACT_BACKWARD == action)
	{
	    p_audio_player->SeekTimer = Fwl_SetMSTimerWithCallback(AUDIO_SEEK_TIMER, AK_TRUE, audio_backward_callback_func);
	    AK_DEBUG_OUTPUT("AudioPlayer_StartBackwardTimer(): PlayTimer = %d.\n", p_audio_player->SeekTimer);
	}
	
	return AK_TRUE;
}

static T_BOOL AudioPlayer_StopSeekTimer(T_VOID)
{
	AK_DEBUG_OUTPUT("Calling AudioPlayer_StopSeekTimer() ... ...\n");
	
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_StopSeekTimer(): p_audio_player null", AK_FALSE);

    if(ERROR_TIMER != p_audio_player->SeekTimer)
    {
        Fwl_StopTimer(p_audio_player->SeekTimer);
        p_audio_player->SeekTimer = ERROR_TIMER;
    }

    return AK_FALSE;
}

/*****************************************************************************/

static T_VOID audio_forward_callback_func(T_TIMER timer_id, T_U32 delay)
{
    T_SYS_MAILBOX   mailbox;
	
	mailbox.event = SYS_EVT_USER_KEY;
	mailbox.param.c.Param1 = kbRIGHT;
	mailbox.param.c.Param2 = PRESS_SHORT;

	AK_PostEventEx(&mailbox, AK_NULL, AK_TRUE, AK_FALSE, AK_TRUE);
}

static T_VOID audio_backward_callback_func(T_TIMER timer_id, T_U32 delay)
{
    T_SYS_MAILBOX   mailbox;
	
	mailbox.event = SYS_EVT_USER_KEY;
	mailbox.param.c.Param1 = kbLEFT;
	mailbox.param.c.Param2 = PRESS_SHORT;

	AK_PostEventEx(&mailbox, AK_NULL, AK_TRUE, AK_FALSE, AK_TRUE);
}
#endif

static T_VOID audio_audition_callback_func(T_TIMER timer_id, T_U32 delay)
{
    AK_ASSERT_PTR_VOID(p_audio_player, "audio_audition_callback_func(): p_audio_player null");

    if (p_audio_player->AuditionTimer == timer_id && gb.AudioPreTime > 0)
    {
        // audition play
        AudioPlayer_StopAuditionTimer();

        //Fwl_PostEvent2AudioTask(MEDIA_EVT_SWITCH);
        VME_ReTriggerEvent(M_EVT_SDCB_MESSAGE, (vUINT32)T_END_TYPE_USER);
    }
}

static T_VOID audio_play_callback_func(T_TIMER timer_id, T_U32 delay)
{
	T_SYS_MAILBOX mail;
	
    AK_ASSERT_PTR_VOID(p_audio_player, "audio_play_callback_func(): p_audio_player null");

	if (p_audio_player->Repeat_A != AUDIOPLAYER_AB_DISABLE
		&& p_audio_player->Repeat_A > MPlayer_GetCurTime())
	{
		p_audio_player->CurTime = p_audio_player->Repeat_A;
	}
	else
	{
		p_audio_player->CurTime = MPlayer_GetCurTime();
	}
	
    if (p_audio_player->PlayTimer == timer_id && p_audio_player->TotalTime > 0)
    {        
        // ab play, if reach point b, seek to a
        if (p_audio_player->Repeat_A != AUDIOPLAYER_AB_DISABLE \
            && p_audio_player->Repeat_B != AUDIOPLAYER_AB_DISABLE \
            && p_audio_player->CurTime >= p_audio_player->Repeat_B)
        {        	        	
        	// AK_DEBUG_OUTPUT("A Time: %d, B Time: %d, Current Time: %d.\n", p_audio_player->Repeat_A, p_audio_player->Repeat_B, p_audio_player->CurTime);

			// AudioPlayer_StopPlayTimer();
			
			// Exec AudioPlayer_BSeekToA();
			// m_triggerEvent(M_EVT_AUDIO_ABPLAY, AK_NULL);
			mail.event = SYS_EVT_AUDIO_ABPLAY;
			AK_PostUniqueEventToHead(&mail, AK_NULL);
            
			// DmxMgr_HandleEvt(EVT_PLAY_AB, AK_NULL);				
        }
    }
}

/*
* @brief audio end callback function
*    it is called in interrupt service function, be careful to call lcd refresh function specially in AK36XX platform
*/
T_VOID audio_stop_callback(T_END_TYPE endType)
{
    AK_DEBUG_OUTPUT("endType = %d.\r\n", endType);
	
    VME_ReTriggerEvent(M_EVT_SDCB_MESSAGE, (vUINT32)endType);
}


/*****************************************************************************/


T_U32 AudioPlayer_GetPlayTime(T_VOID)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_GetPlayTime(): p_audio_player null", 0);

    return p_audio_player->CurTime;
}

T_U32 AudioPlayer_GetTotalTime(T_VOID)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_GetTotalTime(): p_audio_player null", 0);

    return p_audio_player->TotalTime;
}

T_pCWSTR AudioPlayer_GetPlayAudioName(T_VOID)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_GetPlayAudioName(): p_audio_player null", AK_NULL);

    if (0 == p_audio_player->PlayFileId)
    {
        return AK_NULL;
    }

    return p_audio_player->PlayFileName;
}

T_pCWSTR AudioPlayer_GetPlayAudioPath(T_VOID)
{
    if ((AK_NULL == p_audio_player) || (0 == p_audio_player->PlayFileId))
    {
        return AK_NULL;
    }

    return p_audio_player->path;
}


T_VOID AudioPlayer_GetFocusFilePath(T_pWSTR pFilePath)
{
	T_INDEX_CONTENT *pcontent = AK_NULL;
	
    AK_ASSERT_PTR_VOID(p_audio_player, "AudioPlayer_GetFocusFilePath(): p_audio_player null");
    AK_ASSERT_PTR_VOID(pFilePath, "AudioPlayer_GetFocusFilePath(): pFilePath null");

    pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(p_audio_player->pIconExplorer);

    if (AK_NULL != pcontent)
	{
		MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
	}

	if (0 == pFilePath[0] && gb.bInExplorer)
	{
		Utl_UStrCpyN(pFilePath, p_audio_player->pathExplorer, sizeof(T_USTR_FILE)/2);
	}
}

T_pCWSTR AudioPlayer_GetFocusName(T_VOID)
{
    T_ICONEXPLORER_ITEM *p;

    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_GetFocusName(): p_audio_player null", AK_NULL);

    p = IconExplorer_GetItemFocus(p_audio_player->pIconExplorer);
    if (p == AK_NULL)
        return AK_NULL;

    return p->pText;
}

T_ICONEXPLORER *AudioPlayer_GetIconExplorer(T_VOID)
{
    if (AK_NULL == p_audio_player)
    {
        AK_DEBUG_OUTPUT("AudioPlayer_GetIconExplorer(): p_audio_player null\n");
        return AK_NULL;
    }

    return p_audio_player->pIconExplorer;
}

T_BOOL AudioPlayer_TuneTotalTime(T_VOID)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_SetTotalTime(): p_audio_player null", AK_FALSE);

    if (p_audio_player->CurTime > p_audio_player->TotalTime)
    {
        p_audio_player->TotalTime = p_audio_player->CurTime;
        return AK_TRUE;
    }

    return AK_FALSE;
}

T_BOOL AudioPlayer_SetRefreshCallback(T_fAUDIOPLAYER_REFRESH_CALLBACK callbackfunc)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_SetSwichRefreshCallback(): p_audio_player null", AK_FALSE);

    p_audio_player->fRefreshCallback = callbackfunc;

    return AK_TRUE;
}

T_BOOL AudioPlayer_SetFetchLyricCallback(T_fAUDIOPLAYER_GET_LYRIC_CALLBACK callbackfunc)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_SetGetLyricCallback(): p_audio_player null", AK_FALSE);

    p_audio_player->fGetLyricCallback = callbackfunc;

    return AK_TRUE;
}

T_BOOL AudioPlayer_SetNameInfoCallback(T_fAUDIOPLAYER_SET_NAMEINFO_CALLBACK callbackfunc)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_SetNameInfoCallback(): p_audio_player null", AK_FALSE);

    p_audio_player->fSetNameInfoCallback = callbackfunc;

    return AK_TRUE;

}

T_BOOL AudioPlayer_SetHitButtonCallback(T_fAUDIOPLAYER_HIT_BUTTON_CALLBACK callbackfunc)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_HitButtonCallback(): p_audio_player null", AK_FALSE);

    p_audio_player->fHitButtonCallback = callbackfunc;

    return AK_TRUE;

}

T_BOOL AudioPlayer_IsMarkPointA(T_VOID)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_IsMarkPointA(): p_audio_player null", AK_FALSE);

    if (p_audio_player->Repeat_A != 0xfffffffb)
    {
        return AK_TRUE;
    }

    return AK_FALSE;
}

T_BOOL AudioPlayer_IsMarkPointB(T_VOID)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_IsMarkPointB(): p_audio_player null", AK_FALSE);

    if (p_audio_player->Repeat_B != 0xfffffffb)
    {
        return AK_TRUE;
    }

    return AK_FALSE;
}

T_U32 AudioPlayer_GetSuspendFlag(T_VOID)
{
    if (AK_NULL == p_audio_player)
    {
        return AK_FALSE;
    }

    return p_audio_player->SuspendFlag;
}


/*****************************************************************************/

#if 0
T_BOOL AudioPlayer_IsSupportSeekType(T_eMEDIALIB_MEDIA_TYPE media_type)
{
    T_BOOL retval = AK_TRUE;
/*
    switch(media_type)
    {
        defalut:
            break;

    }
*/
    return retval;    
}


T_VOID AudioPlayer_ChangName(T_VOID)
{
    p_audio_player->fSetNameInfoCallback();
}


T_pSONG_INFO AudioPlayer_GetPlayAudioInfo(T_VOID)
{
    if ((AK_NULL == p_audio_player) || (0 == p_audio_player->PlayFileId))
    {
        return AK_NULL;
    }

    return &p_audio_player->PalySongInfo;
}


T_BOOL AudioPlayer_SetPlayTime(T_U32 CurTime)
{
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_GetPlayTime(): p_audio_player null", 0);
   
    p_audio_player->CurTime = CurTime;

    return AK_TRUE;
}


T_U32  AudioPlayer_GetPlayFileType(T_VOID)
{
    // if  p_audio_player not null
    AK_ASSERT_PTR(p_audio_player, "AudioPlayer_GetPlayFileType(): p_audio_player null", _SD_MEDIA_TYPE_UNKNOWN);

    return p_audio_player->CurType;
}


// must use suspend first in background play state
// in audio player state can not use it
T_VOID AudioPlayer_Resume(T_VOID)
{
    // if p_audio_player not null and at pause state
    //|| AUDIOPLAYER_STATE_BACKGROUNDPLAY != p_audio_player->CurState 
     if ((AK_NULL == p_audio_player) || (AK_FALSE == p_audio_player->SuspendFlag))
     {
         return;
     }
        
    //AudioPlayer_StartPlayTimer();
    Fwl_AudioResume();
    p_audio_player->SuspendFlag = AK_FALSE;

    // refresh top bar
    TopBar_Show(TB_REFRESH_AUDIO_STATUS);
    TopBar_Refresh();
}


// must use suspend in background play state
// in audio player state can not use it
T_VOID AudioPlayer_Suspend(T_VOID)
{
    if (AK_NULL == p_audio_player) //(|| AUDIOPLAYER_STATE_BACKGROUNDPLAY != p_audio_player->CurState)
    {
        return;
    }

    //AudioPlayer_StopPlayTimer();
    // pause mp3
    Fwl_AudioPause();
    p_audio_player->SuspendFlag = AK_TRUE;

    // refresh top bar
    TopBar_Show(TB_REFRESH_AUDIO_STATUS);
    TopBar_Refresh();
}


T_AUDIOPLAYER_STATE AudioPlayer_GetOldState(T_VOID)
{
    if (p_audio_player)
    {
        return p_audio_player->OldState;
    }
    else
    {
        return AUDIOPLAYER_STATE_NONE;
    }
}
#endif
