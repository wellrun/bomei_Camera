/**
 * @file
 * @brief ANYKA software
 * this file will constraint the access to the bottom layer sound function,
 * avoid resource competition. Also, this file os for porting to
 * different OS
 *
 * @author ZouMai
 * @date 2001-4-20
 * @version 1.0
 */
#include "fwl_vme.h"
#include "Fwl_pfAudio.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osFS.h"
#include "Eng_String.h"
#include "Gbl_Global.h"
#include "Lib_event.h"
#include "Fwl_Initialize.h"
#include "Eng_ImgConvert.h"
#include "hal_timer.h"
#include "eng_topbar.h"
#include "Eng_DataConvert.h"
#include "gpio_config.h"

#include "eng_debug.h"
#include "Ctl_Fm.h"
#include "AKAppMgr.h"

#include "Fwl_waveout.h"
#include "hal_gpio.h"
#include "hal_fm.h"
#include "arch_analog.h"
#include "arch_freq.h"
#include "Lib_SdFilter.h"
#include "Log_MediaPlayer.h"
#include "Log_MediaAudio.h"
#include "Log_Mp3Player.h"
#include "Ctl_AudioPlayer.h"
#include "Fwl_sys_detect.h"


#define IMPLEMENT_AUDIO_SUPPORT

#ifdef OS_WIN32
#include "fwl_oscom.h"
#include "w_melody.h"
#endif

#define AUDIO_VOL_STEP 53


static T_U16 gSysAudioVolume;
static T_BOOL gSysAudioVolumeStatus;    //allow system control volume flag

T_VOID Fwl_AudioVolumeInit(T_VOID)
{
    gSysAudioVolume = gs.SysVolume;
    Fwl_AudioSetVolume(gSysAudioVolume);
    gSysAudioVolumeStatus = AK_TRUE;
}

T_VOID Fwl_AudioVolumeFree(T_VOID)
{
    if (gSysAudioVolume != gs.SysVolume)
    {
        gs.SysVolume = gSysAudioVolume;
    }

	
    gSysAudioVolumeStatus = AK_FALSE;
}

T_U16 Fwl_AudioVolumeAdd(T_VOID)
{
    if (gSysAudioVolume < AK_VOLUME_MAX)
    {
        ++gSysAudioVolume;
    }
    
    return gSysAudioVolume;
}

T_U16 Fwl_AudioVolumeSub(T_VOID)
{
    if (gSysAudioVolume > 0)
    {
        --gSysAudioVolume;
    }
    
    return gSysAudioVolume;
}

T_U16 Fwl_GetAudioVolume(T_VOID)
{
    
    return gSysAudioVolume;
}

T_BOOL Fwl_GetAudioVolumeStatus(T_VOID)
{
    return gSysAudioVolumeStatus;
}

T_VOID Fwl_SetAudioVolumeStatus(T_BOOL status)
{
    gSysAudioVolumeStatus = status;
}

T_BOOL Fwl_AudioEnableDA(T_VOID)
{
    if (WaveOut_IsOpen())
    {
	   Fwl_AudioSetVolume(Fwl_GetAudioVolume());
	   return AK_TRUE;
    }

    return WaveOut_EnDA();
}

T_VOID Fwl_AudioDisableDA(T_VOID)
{
    if (!WaveOut_IsOpen())
        return;

    WaveOut_DisDA();
}

T_BOOL  Fwl_AudioSetVolume(T_U16 volume)
{
    T_U16 vol;

    AK_DEBUG_OUTPUT("vol=$%d\n", volume);
    if (volume > AK_VOLUME_MAX)
        return AK_FALSE;
	
    if (volume == 0)
    {
        vol = 0;
        WaveOut_SetStatus(&vol, WAVEOUT_VOLUME);
    }
    else
    {
		//audio volume control        
        vol = volume * AUDIO_VOL_STEP;
        AK_DEBUG_OUTPUT("volume = %d.\n", vol);
        WaveOut_SetStatus(&vol, WAVEOUT_VOLUME);
    }

    gSysAudioVolume = volume;

    return AK_TRUE;
    
}

/**
 * @brief mute
 *      mute but not to change gSysAudioVolume
 * @author zhengwenbo
 * @date 2008-11-8
 * @param T_VOID
 * @return T_BOOL
 * @retval 
 */
T_BOOL  Fwl_AudioMute(T_VOID)
{

    Fwl_AudioSetVolume(0);
    
    return AK_TRUE;
}

T_BOOL  Fwl_AudioSetEQMode(T_EQ_MODE mode)
{
	T_SYS_MAILBOX mailbox;		

	mailbox.event = EVT_AUDIO_FILTER;
	mailbox.param.w.Param1 = _SD_FILTER_EQ;
	mailbox.param.w.Param2 = mode;

	IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);

	//for bug.SD3700001634, solve audioplayer`s speed button can`t change in immediately
	if (_SD_EQ_MODE_NORMAL != mode)
	{
		gb.AudioPlaySpeed = _SD_WSOLA_1_0;	
	}
	
	return AK_TRUE;
}

T_BOOL  Fwl_AudioSetPlaySpeed(T_WSOLA_TEMPO speed)
{
	T_SYS_MAILBOX mailbox;		

	mailbox.event = EVT_AUDIO_FILTER;
	mailbox.param.w.Param1 = _SD_FILTER_WSOLA;
	mailbox.param.w.Param2 = speed;

	IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);
	return AK_TRUE;
}

T_BOOL  Fwl_AudioSetVoiceChange(T_PITCH_MODES mode)
{
	T_SYS_MAILBOX mailbox;		

	mailbox.event = EVT_AUDIO_FILTER;
	mailbox.param.w.Param1 = _SD_FILTER_VOICECHANGE;
	mailbox.param.w.Param2 = mode;

	IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);

	//for bug.SD3700001634, solve audioplayer`s speed button can`t change in immediately
	if (PITCH_NORMAL != mode)
	{
		gb.AudioPlaySpeed = _SD_WSOLA_1_0;	
	}
	
	return AK_TRUE;
}

T_BOOL  Fwl_AudioSetTrack(T_U8 track)
{
    AK_DEBUG_OUTPUT("Fwl_AudioSetTrack()  track = %d.\n", track);

    return MPlayer_SetTrack(track);
}

T_BOOL  Fwl_AudioChangeTrack(T_VOID)
{
    return WaveOut_SwitchTrack();
}


T_BOOL  Fwl_AudioOpenFile(T_pCWSTR fileName)
{
	AK_ASSERT_PTR(fileName, "fileName Is INVALID", AK_FALSE);
	
    if (MPLAYER_END < MPlayer_GetStatus())
    {
        Fwl_AudioStop(T_END_TYPE_USER);
    }
	
    return MPlayer_Open((T_pVOID)fileName, AK_TRUE);
}

T_BOOL Fwl_AudioOpenBuffer(T_pCDATA Buff, T_U32 BuffLen)
{
    T_MEDIALIB_BUFFER* pMediaBuf;

	AK_ASSERT_PTR(Buff, "Buff Is INVALID", AK_FALSE);
	
	pMediaBuf = Fwl_Malloc(sizeof(T_MEDIALIB_BUFFER));
	AK_ASSERT_PTR(pMediaBuf, "Play Buffer Malloc pMediaBuf Failure", AK_FALSE);

    if (MPLAYER_END < MPlayer_GetStatus())
    {
        Fwl_AudioStop(T_END_TYPE_USER);
    }    

    pMediaBuf->pBuf 	= Buff;
    pMediaBuf->bufLen 	= BuffLen;
	pMediaBuf->bufPos 	= 0;
	
    return MPlayer_Open(pMediaBuf, AK_FALSE);
}

T_BOOL Fwl_MP3AudioOpenBuffer(T_pCDATA Buff, T_U32 BuffLen)
{
    T_MEDIALIB_BUFFER* pMediaBuf;

	AK_ASSERT_PTR(Buff, "Buff Is INVALID", AK_FALSE);
	
	pMediaBuf = Fwl_Malloc(sizeof(T_MEDIALIB_BUFFER));
	AK_ASSERT_PTR(pMediaBuf, "Play Buffer Malloc pMediaBuf Failure", AK_FALSE);

    if (MPLAYER_END < MPlayer_GetStatus())
    {
        Fwl_AudioStop(T_END_TYPE_USER);
    }    

    pMediaBuf->pBuf 	= Buff;
    pMediaBuf->bufLen 	= BuffLen;
	pMediaBuf->bufPos 	= 0;

    return MP3MPlayer_Open(pMediaBuf);
}

T_BOOL  Fwl_AudioPlay(T_AUDIOPLAYER *pAPlayer)
{
//	AK_ASSERT_PTR(pAPlayer, "pAPlayer Is INVALID", AK_FALSE);

	MPlayer_SetEndCB(audio_stop_callback);

	if (WaveOut_IsFadeOpen())
	{
		WaveOut_SetFade(1000, FADE_STATE_IN);
	}	

	if (_SD_WSOLA_1_0 != gb.AudioPlaySpeed)
    	Fwl_AudioSetPlaySpeed(gb.AudioPlaySpeed);

	if (_SD_EQ_MODE_NORMAL != gs.AudioToneMode)
    	Fwl_AudioSetEQMode(gs.AudioToneMode);

	if (PITCH_NORMAL != gs.AudioPitchMode)
    	Fwl_AudioSetVoiceChange(gs.AudioPitchMode);
	
	return MPlayer_Play(0);
}

T_VOID  Fwl_AudioStop(T_END_TYPE endType)
{
	if (MPLAYER_END > MPlayer_GetStatus())
		return;
	
    if (WaveOut_IsFadeOpen()
		&& T_END_TYPE_NORMAL != endType)
    {
		WaveOut_SetFade(50, FADE_STATE_OUT);

		AK_Sleep(20);
    }
	
	AudioPlayer_StopPlayTimer();
	
    MPlayer_Close();
    
}

T_BOOL  Fwl_AudioPause(T_VOID)
{
    if (WaveOut_IsFadeOpen())
    {
    	WaveOut_SetFade(100, FADE_STATE_OUT);
		AK_Sleep(30);
    }

	return MPlayer_Pause();    
}

T_BOOL  Fwl_AudioResume(T_VOID)
{
    Fwl_AudioSetVolume(Fwl_GetAudioVolume());

	if (WaveOut_IsFadeOpen())
	{
		WaveOut_SetFade(1000, FADE_STATE_IN);
	}
	
	return MPlayer_Resume();
}

T_BOOL  Fwl_AudioSeek(T_U32 milliSecond)
{
    if (WaveOut_IsFadeOpen())
    {
    	WaveOut_SetFade(100, FADE_STATE_OUT);
		AK_Sleep(30);
    }

    return MPlayer_Seek(milliSecond);
}

T_BOOL Fwl_AudioGetTotalTimeFromBuffer(T_U8 *Buff, T_U32 BuffLen, T_U32 *totalTime)
{
	T_MEDIALIB_BUFFER* pMediaBuf;
	
	pMediaBuf = Fwl_Malloc(sizeof(T_MEDIALIB_BUFFER));
	AK_ASSERT_PTR(pMediaBuf, "Malloc pMediaBuf Failure", AK_FALSE);
	
	pMediaBuf->pBuf 	= Buff;
    pMediaBuf->bufLen 	= BuffLen;
	pMediaBuf->bufPos 	= 0;

	if (0 < (*totalTime = Media_GetTotalTime(pMediaBuf, AK_FALSE)))
		return AK_TRUE;

	return AK_FALSE;
}

T_BOOL Fwl_AudioGetTotalTimeFromFile(T_pCWSTR fileName, T_U32 *totalTime)
{
	AK_ASSERT_PTR(fileName, "fileName Is INVALID", AK_FALSE);
	
	if (0 < (*totalTime = Media_GetTotalTime((T_pVOID)fileName, AK_TRUE)))
		return AK_TRUE;

	return AK_FALSE;
}

T_BOOL Fwl_AudioGetMediaTypeFromFile(T_pCWSTR fileName, T_U8 *MediaType)
{
	AK_ASSERT_PTR(fileName, "fileName Is INVALID", AK_FALSE);
	
	*MediaType = Media_GetAudioType((T_pVOID)fileName, AK_TRUE);

	if (MEDIALIB_AUDIO_UNKNOWN != *MediaType)
		return AK_TRUE;
	
	return AK_FALSE;
}

T_BOOL Fwl_AudioIsPlaying(T_VOID)
{
    return MPLAYER_END < MPlayer_GetStatus();
}

T_U8 Fwl_GetMediatypeByFiletype(T_FILE_TYPE Filetype)
{
    switch (Filetype)
    {
    case FILE_TYPE_MP3:
 //   case FILE_TYPE_MPEG:
        return _SD_MEDIA_TYPE_MP3;
 
    case FILE_TYPE_ADPCM:
        return _SD_MEDIA_TYPE_ADPCM_MS;
		
    case FILE_TYPE_WAV:
        return _SD_MEDIA_TYPE_PCM;		
	
    case FILE_TYPE_WMA:
    case FILE_TYPE_ASF:
        return _SD_MEDIA_TYPE_WMA; //_SD_MEDIA_TYPE_LPC_WMA
 
#if (SDRAM_MODE >= 16)
	case FILE_TYPE_MID:
       return _SD_MEDIA_TYPE_MIDI;
#endif

	case FILE_TYPE_AMR:
		return _SD_MEDIA_TYPE_AMR;

	case FILE_TYPE_AC3:
		return _SD_MEDIA_TYPE_AC3;
		
	case FILE_TYPE_AAC:
    case FILE_TYPE_ADIF:
    case FILE_TYPE_ADTS:
    case FILE_TYPE_M4A:
    case FILE_TYPE_MP4:
        return _SD_MEDIA_TYPE_AAC;
		
    case FILE_TYPE_FLAC_NATIVE:
        return _SD_MEDIA_TYPE_FLAC;
		
    case FILE_TYPE_FLAC_OGA:
    case FILE_TYPE_FLAC_OGG:
        return _SD_MEDIA_TYPE_OGG_FLAC;
		
    case FILE_TYPE_APE:
        return _SD_MEDIA_TYPE_APE;
		
	//case FILE_TYPE_RMVB:
	//case FILE_TYPE_RM:
    //    return _SD_MEDIA_TYPE_RA8LBR;

    default: 
		return _SD_MEDIA_TYPE_UNKNOWN;
    }
}

/* end of file */
