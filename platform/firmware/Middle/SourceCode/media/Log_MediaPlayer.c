/**
 * @file Log_MediaPlayer.c
 * @brief Media Player Logic Implemetation for Multi-thread
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Xie_Wenzhong
 * @date 2011-3-17
 * @version 1.0
 */

#include "fwl_osfs.h"
#include "akError.h"
#include "Fwl_wave.h"
#include "Fwl_waveout.h"
#include "Fwl_display.h"
#include "Fwl_Graphic.h"
#include "Arch_mmu.h"
#include "Arch_Lcd.h"
#include "Hal_timer.h"
#include "Arch_gui.h"
#include "akos_api.h"
#include "File.h"
#include "Ctl_AviPlayer.h"
#include "Log_MediaPlayer.h"
#include "Lib_SDcodec.h"
#include "Lib_SdFilter.h"
#include "Eng_debug.h"
#include "Eng_ImgDec.h"
#include "Eng_String_UC.h"
#include "Fwl_osMalloc.h"
#include "video_stream_lib.h"
#include "Lib_Image_Api.h"
#include "Fwl_sysevent.h"
#include "AKAppMgr.h"
#include "AKVideoBGApp.h"
#include "AKAudioBGApp.h"
#include "Log_MediaStruct.h"
#include "Log_MediaAudio.h"
#include "Log_MediaVideo.h"
#include "Log_MediaDmx.h"
#include "Log_Mp3Player.h"
#include "media_player_lib.h"

#define CLOSE_THREAD_MAX_TIMES	4

#define TASK_STACK_SIZE				(20*1024)

T_eMEDIALIB_VIDEO_CODE g_tCurrVideoType;


/*****************************************************************************/

static T_pMT_MPLAYER s_hPlayer = AK_NULL;

#if 0
T_VOID DriveEvtCB_Demux(T_TIMER timer_id, T_U32 delay)
{
	AK_ASSERT_PTR_VOID(s_hPlayer->pDmx->thread, "s_hPlayer->pDmx->thread Pointer Is Invalid");
	GenrlThread_PostEvent(s_hPlayer->pDmx->thread, EVT_DMX_SCAN, AK_OR);
}

T_VOID DriveEvtCB_Audio(T_TIMER timer_id, T_U32 delay)
{
	AK_ASSERT_PTR_VOID(s_hPlayer->pAudio->thread, "s_hPlayer->pAudio->thread Pointer Is Invalid");
	GenrlThread_PostEvent(s_hPlayer->pAudio->thread, EVT_AUDIO_SCAN, AK_OR);
}

T_VOID DriveEvtCB_Video(T_TIMER timer_id, T_U32 delay)
{
	AK_ASSERT_PTR_VOID(s_hPlayer->pVideo->thread, "s_hPlayer->pVideo->thread Pointer Is Invalid");
	GenrlThread_PostEvent(s_hPlayer->pVideo->thread, EVT_VIDEO_SCAN, AK_OR);
}
#endif

/*****************************************************************************/

T_BOOL Media_QueryInfo(T_MEDIALIB_MEDIA_INFO* mediaInfo, T_pVOID fname, T_BOOL isFile)
{
	T_MEDIALIB_DMX_INFO info;
	
	memset(mediaInfo, 0, sizeof(T_MEDIALIB_MEDIA_INFO));
	
	Fwl_Print(C4, M_MEDIA, "Query Media Info");
	
	if (Dmx_QueryInfo(&info, fname, isFile))
	{
	
		mediaInfo->m_MediaType = info.m_MediaType;
		mediaInfo->m_bHasAudio = info.m_bHasAudio;
		mediaInfo->m_bHasVideo = info.m_bHasVideo;
		mediaInfo->m_bAllowSeek = info.m_bAllowSeek;
		mediaInfo->m_ulTotalTime_ms = info.m_ulTotalTime_ms;

		mediaInfo->m_VideoCode = info.m_VideoDrvType;
		mediaInfo->m_AudioCode = info.m_AudioType;

		return AK_TRUE;
	}
	
	return AK_FALSE;
}


T_BOOL Media_HasAudio(T_pVOID src, T_BOOL isFile)
{
	T_MEDIALIB_CHECK_OUTPUT ckOut;
	
	Fwl_Print(C4, M_MEDIA, "Query Audio");
	
	Media_CheckFile(&ckOut, src, isFile);
	
	return ckOut.m_bHasAudio;
}

T_BOOL Media_HasVideo(T_pVOID src, T_BOOL isFile)
{
	T_MEDIALIB_CHECK_OUTPUT ckOut;
	
	Fwl_Print(C4, M_MEDIA, "Query Video");
	
	Media_CheckFile(&ckOut, src, isFile);
	
	return ckOut.m_bHasVideo;
}

T_eMEDIALIB_MEDIA_TYPE Media_GetMediaType(T_pVOID src, T_BOOL isFile)	
{
	T_MEDIALIB_CHECK_OUTPUT ckOut;
	
	return Media_CheckFile(&ckOut, src, isFile);
}


T_U8 Media_GetVideoType(T_pVOID fname, T_BOOL isFile)
{
	T_MEDIALIB_DMX_INFO info;
	
	Fwl_Print(C4, M_MEDIA, "Query Video Type");
	
	if (Dmx_QueryInfo(&info, fname, isFile))
		return info.m_VideoDrvType;
	
	return VIDEO_DRV_UNKNOWN;
}

T_AUDIO_TYPE Media_GetAudioType(T_pVOID fname, T_BOOL isFile)
{
	T_MEDIALIB_DMX_INFO info;
	
	Fwl_Print(C4, M_MEDIA, "Query Audio Type");
	
	if (Dmx_QueryInfo(&info, fname, isFile))
		return info.m_AudioType;

	return _SD_MEDIA_TYPE_UNKNOWN;
}

T_U32 Media_GetTotalTime(T_pVOID fname, T_BOOL isFile)
{
	T_MEDIALIB_DMX_INFO info;

	memset(&info, 0, sizeof(T_MEDIALIB_DMX_INFO));
	
	Fwl_Print(C4, M_MEDIA, "Query Total Time");
	
	if (Dmx_QueryInfo(&info, fname, isFile))
	{
		return info.m_ulTotalTime_ms;
	}

	return 0;
}

T_pVOID Media_GetMetaInfo(T_MEDIALIB_META_INFO *pMetaInfo, T_pVOID src, T_SRC_TYPE type)
{
	T_hFILE 				hFile;	
	T_MEDIALIB_CB 			cbFunc;
	T_pVOID 				hMedia = AK_NULL;
	T_MEDIALIB_CHECK_OUTPUT ckOut;
	T_eMEDIALIB_MEDIA_TYPE 	mediaType;	


	AK_ASSERT_PTR(pMetaInfo, "MEDIA:	pMetaInfo Is Invalid ", AK_NULL);
	AK_ASSERT_PTR(src, "MEDIA:	src Is Invalid ", AK_NULL);
	AK_ASSERT_VAL(type < T_SRC_TYPE_NUM, "MEDIA:	type Is Invalid ", AK_NULL);
	
	Fwl_Print(C4, M_MEDIA, "Get Meta Info ... ...");	

	// Open File
	if (T_SRC_TYPE_PATH != type)
	{
		hFile = (T_hFILE)src;
	}
	else if (_FOPEN_FAIL == (hFile = Fwl_FileOpen(src, _FMODE_READ, _FMODE_READ)))	
	{
		Fwl_Print(C2, M_DMX, "Open File Failure");
		Printf_UC(src);
		
		return AK_NULL;
	}

	//if type is T_SRC_TYPE_FP / T_SRC_TYPE_PATH, type > 0;
	Dmx_SetMediaLibCB(&cbFunc, (T_BOOL)type, AK_FALSE);
	mediaType = MediaLib_CheckFile(&cbFunc, hFile, &ckOut);

	if (MEDIALIB_MEDIA_UNKNOWN == mediaType)
	{
		if (T_SRC_TYPE_PATH == type)
			Fwl_FileClose(hFile);
		
		return (T_pVOID)-1;	//return 0xffff ffff
	}
	
	hMedia = MediaLib_GetID3MetaInfo(&cbFunc, hFile, mediaType, pMetaInfo);	

	if (T_SRC_TYPE_PATH == type)
		Fwl_FileClose(hFile);

	return hMedia;
}


T_VOID Media_ReleaseMetaInfo(T_pVOID hMedia)
{
	T_MEDIALIB_CB cbFunc;

	AK_ASSERT_PTR_VOID(hMedia, "hMedia Is Invalid");
	
	Dmx_SetMediaLibCB(&cbFunc, AK_TRUE, AK_FALSE);
	MediaLib_ReleaseID3MetaInfo(&cbFunc, hMedia);	
}


T_pVOID Media_GetPicMetaInfo(T_pCWSTR filename, T_U8 **picBuf, T_U32 *picLen)
{
	T_MEDIALIB_CB	cbFunc;
	T_pFILE         hFile;
	T_pVOID 		ret;
	
	Dmx_SetMediaLibCB(&cbFunc, AK_TRUE, AK_FALSE);
	
	if (_FOPEN_FAIL == (hFile = Fwl_FileOpen(filename, _FMODE_READ, _FMODE_READ)))
    {
        Fwl_Print(C3, M_MEDIA, "Media_GetPicMetaInfo() --- Open File Failure!");
        return AK_NULL;
    }

	ret = MediaLib_GetPicMetaInfo(&cbFunc, (T_S32)hFile, picBuf, picLen);

	/*close file*/
    Fwl_FileClose(hFile);
    
	return ret;
}

T_VOID Media_ReleasePicMetaInfo(T_pVOID pMetapic)
{
	T_MEDIALIB_CB cbFunc;

	Dmx_SetMediaLibCB(&cbFunc, AK_TRUE, AK_FALSE);

	MediaLib_ReleasePicMetaInfo(&cbFunc, pMetapic);
}

T_eMEDIALIB_MEDIA_TYPE Media_CheckFile(T_MEDIALIB_CHECK_OUTPUT *ckOut, T_pVOID src, T_BOOL isFile)
{
	T_hFILE 				hFile;	
	T_MEDIALIB_CB 			cbFunc;
	T_eMEDIALIB_MEDIA_TYPE 	mediaType;
	
	if (!isFile)	
	{
		hFile = (T_hFILE)src;
		Fwl_Print(C3, M_MPLAY, "Check File, Media Is A Buffer Type");
	}
	else
	{
		Fwl_Print(C3, M_MPLAY, "Check File, Open Media --- ");
		Printf_UC(src);
		
		// Open File
		if (_FOPEN_FAIL == (hFile = Fwl_FileOpen((T_pCWSTR)src, _FMODE_READ, _FMODE_READ)))
		{
			Fwl_Print(C2, M_MPLAY, "Open File Failure");
			
			return MEDIALIB_MEDIA_UNKNOWN;
		}
	}
	
	Dmx_SetMediaLibCB(&cbFunc, isFile, AK_FALSE);	
	mediaType = MediaLib_CheckFile(&cbFunc, hFile, ckOut);
	
	if (isFile)	
	{
		Fwl_FileClose(hFile);
	}

	return mediaType;
}


/*****************************************************************************/

T_BOOL MPlayer_HasAudio(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(s_hPlayer->pDmx, "s_hPlayer->pDmx Is Invalid", AK_FALSE);

	return s_hPlayer->pDmx->dmxInfo.m_bHasAudio;
}

T_BOOL MPlayer_HasVideo(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(s_hPlayer->pDmx, "s_hPlayer->pDmx Is Invalid", AK_FALSE);
	
	return s_hPlayer->pDmx->dmxInfo.m_bHasVideo;
}

T_BOOL MPlayer_AllowSeek(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(s_hPlayer->pDmx, "s_hPlayer->pDmx Is Invalid", AK_FALSE);

	Fwl_Print(C3, M_MPLAY, "Allow Seek: %d.\n", s_hPlayer->pDmx->dmxInfo.m_bAllowSeek);
	
	return s_hPlayer->pDmx->dmxInfo.m_bAllowSeek;
}

T_LEN MPlayer_GetWidth(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer Is Invalid ", -1);

	if (s_hPlayer->pDmx)
		return s_hPlayer->pDmx->dmxInfo.m_uWidth;
	else
		return -1;
}

T_LEN MPlayer_GetHeight(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer Is Invalid ", -1);

	if (s_hPlayer->pDmx)
		return s_hPlayer->pDmx->dmxInfo.m_uHeight;
	else
		return -1;
}

T_U32 MPlayer_GetTotalTime(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer NOT Initialized", 0);
	
	if (!s_hPlayer->pDmx)
	{
		Fwl_Print(C3, M_MPLAY, "s_hPlayer->pDmx Is Invalid");
		return 0;
	}
	
	return s_hPlayer->pDmx->dmxInfo.m_ulTotalTime_ms;
}

T_U32 MPlayer_GetCurTime(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer NOT Initialized.\n", 0);
	
	if (!s_hPlayer->pDmx)
	{
		Fwl_Print(C3, M_MPLAY, "Get Current Time, s_hPlayer->pDmx Is Invalid");
		return 0;
	}

	if (!s_hPlayer->pDmx->dmxInfo.m_bHasVideo
		&& s_hPlayer->pDmx->dmxInfo.m_bHasAudio
		&& MPLAYER_PLAY == s_hPlayer->status)
		WaveOut_GetStatus(&s_hPlayer->pDmx->curTime, WAVEOUT_CURRENT_TIME);
	
	return s_hPlayer->pDmx->curTime;
}

T_eMEDIALIB_MEDIA_TYPE MPlayer_GetMediaType(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer NOT Initialized.\n", MEDIALIB_MEDIA_UNKNOWN);
	
	if (s_hPlayer->pDmx)	
		return s_hPlayer->pDmx->dmxInfo.m_MediaType;

	Fwl_Print(C3, M_MPLAY, "NOT Media Is Playing");
	return MEDIALIB_MEDIA_UNKNOWN;
}

T_BOOL MPlayer_SetTrack(T_U8 track)
{
    AK_ASSERT_PTR(s_hPlayer, "MtMP_SetChannel(): s_hPlayer is null.\n", AK_FALSE);
    
    WaveOut_SetStatus(&track, WAVEOUT_TRACK);
    return AK_TRUE;
}

static T_BOOL MtMP_CreateVideoThread(T_pMT_MPLAYER hPlayer)
{	
	if (!IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_VIDEO))
	{
		T_U8 i = 0;
		
		CVideoBGApp_New(AK_NULL);
		
		do{
			AK_Sleep(2);
			Fwl_Print(C3, M_MPLAY, "Waiting Create Video Thread ... ...");
			
			if (++i > MAX_WAIT_MSG_NUM )
				return AK_FALSE;
			
		}while(!IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_VIDEO));
		
	}

	return AK_TRUE;
}

static T_BOOL MtMP_CreateAudioThread(T_pMT_MPLAYER hPlayer)
{
	IThread_Run(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIO));

	return AK_TRUE;
}

T_BOOL MPlayer_Play(T_U32 position)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer Is Invalid ", AK_FALSE);
	AK_ASSERT_PTR(s_hPlayer->pDmx, "s_hPlayer->pDmx Is Invalid ", AK_FALSE);

	if (s_hPlayer->pDmx->dmxInfo.m_bHasAudio 
		&& s_hPlayer->pAudio
		&& !MtMP_CreateAudioThread(s_hPlayer))
	{		
		Fwl_Print(C2, M_MPLAY, "Create Audio Thread Failure");
		
		return AK_FALSE;		
	}	
	
	if (s_hPlayer->pDmx->dmxInfo.m_bHasVideo && s_hPlayer->pVideo)
	{
		// This Is Just A Audio Player
		if (!s_hPlayer->pVideo->showCB)
		{
			s_hPlayer->pDmx->dmxInfo.m_bHasVideo = AK_FALSE;
			s_hPlayer->pVideo = Vs_FreeVS(s_hPlayer->pVideo);

			if (!s_hPlayer->pDmx->dmxInfo.m_bHasAudio)
				return AK_FALSE;
		}
		else if (!MtMP_CreateVideoThread(s_hPlayer))
		{	
			Fwl_Print(C2, M_MPLAY, "Create Video Thread Failure");
			
			return AK_FALSE;
		}

		// Close Audio Filter
		if (s_hPlayer->pDmx->dmxInfo.m_bHasVideo
			&& s_hPlayer->pAudio 
			&& s_hPlayer->pAudio->pSdFilt)
		{
			s_hPlayer->pAudio->pSdFilt = SdFilter_Close(s_hPlayer->pAudio->pSdFilt);
		}
	}	

#if CI37XX_PLATFORM	
	if (s_hPlayer->pVideo && s_hPlayer->pVideo->showCB)
	{
		MpuRefr_SetShowFrameCB(s_hPlayer->pVideo->showCB);
	}
#endif

	if (!MPlayer_Start(position))
		return AK_FALSE;

	return AK_TRUE;
}

T_VOID MPlayer_SetInitParm(T_MEDIALIB_INIT_INPUT *pInitIn, T_MEDIALIB_INIT_CB *pIinitCB)
{
	pInitIn->m_ChipType 			= MEDIALIB_CHIP_AK3751C;
    pInitIn->m_AudioI2S 			= I2S_UNUSE;

	pIinitCB->m_FunPrintf 			= (MEDIALIB_CALLBACK_FUN_PRINTF)AkDebugOutput;
    pIinitCB->m_FunLoadResource 	= AK_NULL;    
    pIinitCB->m_FunReleaseResource 	= AK_NULL;
}

static T_BOOL MPlayer_IsInit(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer Is NOT Initialized", AK_FALSE);
	
	return s_hPlayer->init;
}

T_pVOID MPlayer_GetPlayer(T_VOID)
{
	return s_hPlayer;
}

T_BOOL MPlayer_Init(T_VOID)
{
	T_MEDIALIB_INIT_INPUT initIn;
    T_MEDIALIB_INIT_CB initCB;

	//initliaze Mp3Player
	MP3Player_Init();
	
	if (AK_NULL != s_hPlayer)
	{
		MPlayer_Close();

		if (s_hPlayer->init)
		{
			VideoStream_Destroy();
			IAppMgr_DeleteEntry(AK_GetAppMgr(), AKAPP_CLSID_AUDIO);			
		}

		s_hPlayer = Fwl_FreeTrace(s_hPlayer);
	}		
	
	s_hPlayer = (T_pMT_MPLAYER)Fwl_Malloc(sizeof(T_MT_MPLAYER));
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer Malloc Failure ", AK_FALSE);	
	memset(s_hPlayer, 0, sizeof(T_MT_MPLAYER));	
	
	MPlayer_SetInitParm(&initIn, &initCB);

	if (!VideoStream_Init(&initIn, &initCB))
	{
		s_hPlayer = Fwl_FreeTrace(s_hPlayer);
		return AK_FALSE;
	}
	
	CAudioBGApp_New(AK_NULL);
	s_hPlayer->init = AK_TRUE;
	s_hPlayer->tmExit = ERROR_TIMER;
	
	return AK_TRUE;
}

T_VOID MPlayer_Destroy()
{
	AK_ASSERT_PTR_VOID(s_hPlayer, "s_hPlayer Is NOT Initialized")

	if (s_hPlayer->init)
	{
		VideoStream_Destroy();
		IAppMgr_DeleteEntry(AK_GetAppMgr(), AKAPP_CLSID_AUDIO);
		s_hPlayer->init = AK_FALSE;
	}

	s_hPlayer = Fwl_FreeTrace(s_hPlayer);
}

T_VOID MPlayer_HandleEvent(T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
	if (MPLAYER_CLOSE != s_hPlayer->status)
		DmxMgr_HandleDemux(s_hPlayer, eEvent, pEvtParam);
	
	if (EVT_DMX_EXIT == eEvent)		
	{
		if (MPLAYER_CLOSE == s_hPlayer->status && AK_NULL == s_hPlayer->pDmx)
		{
			T_SYS_MAILBOX mailbox;
			
#ifdef MEDIA_FAST_SWITCH
			mailbox.event = EVT_AUDIO_CLOSE;				
			IAppMgr_PostUniqueEvt(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);
#endif			
			mailbox.event = EVT_VIDEO_EXIT;				
			IAppMgr_PostUniqueEvt(AK_GetAppMgr(), AKAPP_CLSID_VIDEO, &mailbox);		

			AK_Sleep(2);
			
#ifdef MEDIA_FAST_SWITCH
			IThread_Suspend(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIO));
#endif			
			Fwl_Print(C3, M_MPLAY, "Close Decoder Handle, Exit");
		}

		if (ERROR_TIMER != s_hPlayer->tmExit)
		{
			vtimer_stop(s_hPlayer->tmExit);
			s_hPlayer->tmExit = ERROR_TIMER;
		}

	}

	
}


T_BOOL MPlayer_Open(T_pVOID src, T_BOOL isFile)
{
	T_U8 i = 0;
	T_hFILE hFile;	
	
	if (!MPlayer_IsInit())
	{
		Fwl_Print(C2, M_MPLAY, "Media Lib NOT Initialized");

		return AK_FALSE;
	}

	if (!isFile)	
	{
		hFile = (T_hFILE)src;
		Fwl_Print(C3, M_MPLAY, "Media Is A Buffer Type tick: %d", get_tick_count());
	}
	else
	{
		Fwl_Print(C3, M_MPLAY, "Open File --- tick: %d ", get_tick_count());
		Printf_UC(src);
		// Open File
		hFile = Fwl_FileOpen((T_pCWSTR)src, _FMODE_READ, _FMODE_READ);
		if (_FOPEN_FAIL == hFile)
		{
			Fwl_Print(C2, M_MPLAY, "Open File Failure");
			return AK_FALSE;
		}
	}
#ifdef OS_ANYKA
	while (s_hPlayer->pDmx)
	{	
		if (++i > MAX_WAIT_MSG_NUM)
		{
			//DmxMgr_Close(s_hPlayer);
			//s_hPlayer->pDmx = Fwl_FreeTrace(s_hPlayer->pDmx);
			Fwl_Print(C2, M_MPLAY, "XXXXXXXXXXXXXXXX    MPlayer Is Running!    XXXXXXXXXXXXXXXXXX\n");
			return AK_FALSE;
		}
		MPlayer_Close();
		AK_Sleep(20);
	}

	if(!DmxMgr_DecodeHeader(hFile, s_hPlayer, isFile))
	{
		s_hPlayer->pDmx = Dmx_CloseDemux(s_hPlayer->pDmx);
		
		Fwl_Print(C2, M_MPLAY, "Decode Media Header Failure");

		return AK_FALSE;
	}

	s_hPlayer->status = MPLAYER_OPEN;

	return AK_TRUE;
#else
	return AK_FALSE;
#endif

}

T_VOID MPlayer_SetEndCB(T_fEND_CB endCB)
{
    AK_ASSERT_PTR_VOID(s_hPlayer, "MPlayer_SetEndCB(): s_hPlayer is null");    
    
    s_hPlayer->endCB = endCB;
}


T_VOID MPlayer_SetShowFrameCB(T_fSHOWFRAME_CB pShowFrame)
{
    AK_ASSERT_PTR_VOID(s_hPlayer, "MPlayer_SetShowFrameCB(): s_hPlayer is null");

	if (!s_hPlayer->pVideo)
	{
		Fwl_Print(C3, M_MPLAY, "Video Decoder NOT Opened, Can NOT Set Show Frame CB");
		return;
	}
	
	s_hPlayer->pVideo->showCB = pShowFrame;
}


T_pVOID MPlayer_GetAudioDecoder(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer Is Invalid ", AK_NULL);

	return s_hPlayer->pAudio;
}

T_pVOID MPlayer_GetVideoDecoder(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer Is Invalid ", AK_NULL);

	return s_hPlayer->pVideo;
}

T_pVOID MPlayer_GetMediaInfo(T_VOID)
{
	AK_ASSERT_PTR(s_hPlayer, "s_hPlayer NOT Initialized.\n", AK_NULL);
	AK_ASSERT_PTR(s_hPlayer->pDmx, "s_hPlayer->pDmx Is Invalid", AK_NULL);
	
	return &s_hPlayer->pDmx->dmxInfo;
}

T_BOOL MPlayer_GetMetaInfo(T_MEDIALIB_META_INFO** metaInfo)
{
	T_MEDIALIB_DMX_INFO* dmxInfo;

	dmxInfo = MPlayer_GetMediaInfo();
	if(dmxInfo)
	{
		*metaInfo = dmxInfo->m_pMetaInfo;
		return AK_TRUE;
	}

	return AK_FALSE;
}

T_eMPLAYER_STATUS MPlayer_GetStatus(T_VOID)
{
	if (AK_NULL == s_hPlayer)
		return MPLAYER_CLOSE;

	return s_hPlayer->status;
}


T_BOOL MPlayer_Start(T_U32 pos)
{
	T_U8 i = 0;	
	
	DmxMgr_HandleEvt(EVT_PLAY_START, pos);
	
	do
	{
		AK_Sleep(10);
		
		if (MPLAYER_ERR == s_hPlayer->status)
		{
			Fwl_Print(C2, M_MPLAY, "Start Playing Media Failure");
			
			return AK_FALSE;
		}
		
		if (MPLAYER_SEEKING == s_hPlayer->status)
		{
			// Waiting Demuxer Seek
			AK_DEBUG_OUTPUT(".");
			continue;
		}
		
		if (MPLAYER_SEEKED == s_hPlayer->status)
		{
			// Waiting Open WaveOut
			AK_DEBUG_OUTPUT("&");
		}

		if (MPLAYER_WAVEOUT == s_hPlayer->status)
		{
			// Waiting Open WaveOut
			AK_DEBUG_OUTPUT("@");
		}
		
		if (++i > MAX_WAIT_MSG_NUM)
		{
			Fwl_Print(C2, M_MPLAY, "Start Player Timeout");
			
			IThread_Terminate(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA));
			
			DmxMgr_Close(s_hPlayer);
			
			IThread_Run(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA));
			
			return AK_FALSE;
		}		
	}while(MPLAYER_PLAY != s_hPlayer->status);

	return AK_TRUE;
}

T_BOOL MPlayer_Pause(T_VOID)
{
	T_U16 i = 0;
	
	DmxMgr_HandleEvt(EVT_PLAY_PAUSE, 0);

	do{
		AK_Sleep(2);
		
	} while (MPLAYER_PAUSE != s_hPlayer->status && i++ < MAX_WAIT_MSG_NUM);

	if (MPLAYER_PAUSE == s_hPlayer->status)
		return AK_TRUE;
	else
		return AK_FALSE;
}

T_BOOL MPlayer_Resume(T_VOID)
{
	T_U16 i = 0;
	
	DmxMgr_HandleEvt(EVT_PLAY_RESUME, 0);

	do{
		AK_Sleep(2);
		
	} while (MPLAYER_PLAY != s_hPlayer->status && i++ < MAX_WAIT_MSG_NUM);

	if (MPLAYER_PLAY == s_hPlayer->status)
		return AK_TRUE;
	else
		return AK_FALSE;

}

T_BOOL MPlayer_Stop(T_VOID)
{
	T_U16 i = 0;
	
	DmxMgr_HandleEvt(EVT_PLAY_STOP, 0);

	do{
		AK_Sleep(2);
		
	} while (MPLAYER_STOP != s_hPlayer->status && i++ < MAX_WAIT_MSG_NUM);

	if (MPLAYER_STOP == s_hPlayer->status)
		return AK_TRUE;
	else
		return AK_FALSE;
}

T_BOOL MPlayer_FastForward(T_U32 pos)
{
	T_U16 i = 0;
	
	DmxMgr_HandleEvt(EVT_PLAY_FF, pos);

	do{
		AK_Sleep(2);
		
	} while (MPLAYER_FAST != s_hPlayer->status && i++ < MAX_WAIT_MSG_NUM);

	if (MPLAYER_FAST == s_hPlayer->status)
		return AK_TRUE;
	else
		return AK_FALSE;
}

T_BOOL MPlayer_FastRewind(T_U32 pos)
{
	T_U16 i = 0;
	
	DmxMgr_HandleEvt(EVT_PLAY_FR, pos);

	do{
		AK_Sleep(2);
		
	} while (MPLAYER_FAST != s_hPlayer->status && i++ < MAX_WAIT_MSG_NUM);

	if (MPLAYER_FAST == s_hPlayer->status)
		return AK_TRUE;
	else
		return AK_FALSE;
}


T_BOOL MPlayer_Close(T_VOID)
{
	T_U8 i = 0;	
	T_hTask hAudio;
	T_hTask hVideo;
	T_hTask hMedia;
	T_hTask hCurrt;

	if (MPLAYER_CLOSE == s_hPlayer->status)
	{
		Fwl_Print(C3, M_MPLAY, "MPlayer Has Been Closed\n");
		return AK_TRUE;
	}
	
	DmxMgr_HandleEvt(EVT_PLAY_CLOSE, 0);
	
	hAudio = IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIO));
	hVideo = IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_VIDEO));
	hMedia = IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA));
	hCurrt = AK_GetCurrent_Task();

	if (hCurrt == hAudio
		|| hCurrt == hVideo
		|| hCurrt == hMedia)
	{
		return AK_TRUE;
	}
	
	do{		
		AK_Sleep(1);
		
	} while (MPLAYER_CLOSE != s_hPlayer->status && i++ < MAX_WAIT_MSG_NUM);

	Fwl_Print(C3, M_MPLAY, "Player Close %d, tick: %d\n\n", i, get_tick_count());
	
	if (MPLAYER_CLOSE == s_hPlayer->status)
		return AK_TRUE;
	else
		return AK_FALSE;
}

T_BOOL MPlayer_Seek(T_U32 param)
{
	if (MPlayer_Stop())
	{
		WaveOut_SetFade(1000, FADE_STATE_IN);
		if (MPlayer_Start(param))
			return AK_TRUE;
		else
			return AK_FALSE ;
	}
	else
	{
		return AK_FALSE;
	}
}

T_BOOL MPlayer_GetFrameYuv(T_pVOID fname, T_BOOL isFile, T_pDATA pBuf, T_LEN width, T_LEN height, T_S32 pos)
{
	T_S32 				vsRet 	= 0;
	T_BOOL				ret		= AK_FALSE;
	T_VIDEO_DECODE_OUT 	decOut;
	
	memset(&decOut, 0, sizeof(T_VIDEO_DECODE_OUT));

	if (!MPlayer_Open(fname, isFile))
	{
		Fwl_Print(C3, M_MPLAY, "Open MPlayer Failure");
		
		s_hPlayer->status = MPLAYER_ERR;
		
		return AK_FALSE;
	}

	if (0 <= (pos = MediaLib_Dmx_Start(s_hPlayer->pDmx->hMedia, pos))
		&& 0 < Dmx_GetVideo2Decoder(s_hPlayer->pDmx->hMedia, s_hPlayer->pVideo->hVS))
	{
		while(0 > (vsRet = VideoStream_SYNDecode(s_hPlayer->pVideo->hVS, &decOut, pos))
			|| !decOut.m_pBuffer)
		{
			if (VIDEO_STREAM_NODATA_ERR == VideoStream_GetLastError(s_hPlayer->pVideo->hVS))
				break;
		
			pos += 40;
		}

		if (vsRet >= 0 && decOut.m_pBuffer)
		{
			T_RECT srcWin;
			T_RECT dstWin;

			Fwl_InitRect(&srcWin, 0, 0, decOut.m_uDispWidth, decOut.m_uOriHeight);
			Fwl_InitRect(&dstWin, 0, 0, width, height);
		
			ret = Fwl_YuvZoom(decOut.m_pBuffer, decOut.m_pBuffer_u, decOut.m_pBuffer_v,	srcWin.width, &srcWin,\
				pBuf, width, YUV420, &dstWin);
		}
		else
		{
			Fwl_Print(C3, M_MPLAY, "Decode Frame Failure");
			ret = AK_FALSE;
		}
	}	

	s_hPlayer->pVideo 	= Vs_FreeVS(s_hPlayer->pVideo);
	s_hPlayer->pAudio 	= Sd_Close(s_hPlayer->pAudio);
	s_hPlayer->pDmx		= Dmx_CloseDemux(s_hPlayer->pDmx);
	
	s_hPlayer->status 	= MPLAYER_CLOSE;
	s_hPlayer->endCB 	= AK_NULL;
	
	return ret;
}


/*****************************************************************************/
#if 0
T_BOOL GenrlThread_StartTimer(T_pGENRL_THREAD thread, T_U32 millSec, T_fVTIMER_CALLBACK cbFunc)
{
	AK_ASSERT_PTR(thread, "Input Parameter Is Invalid ", AK_FALSE);
	// AK_ASSERT_PTR(cbFunc, "Input Parameter Is Invalid ", AK_FALSE);
	// AK_DEBUG_OUTPUT("Start timer\n");
	if (ERROR_TIMER == thread->timer
		&& ERROR_TIMER == (thread->timer = vtimer_start(millSec, AK_TRUE, cbFunc)))
	{
		return AK_FALSE;
	}
	else
	{
		return AK_TRUE;
	}
}

T_VOID GenrlThread_StopTimer(T_pGENRL_THREAD thread)
{
	AK_ASSERT_PTR_VOID(thread, "Input Parameer Is Invalid ");
	
	if(ERROR_TIMER != thread->timer)
	{
		vtimer_stop(thread->timer);
		thread->timer = ERROR_TIMER;
	}
}

T_pTHREAD_PARM 	GenrlThread_Init(T_fThreadEntryCB entry, T_U8* name, T_U32 argc, T_pVOID argv, T_U8 priority, T_U32 slice)
{
	T_pTHREAD_PARM threadParm;
	
	AK_ASSERT_PTR((T_pVOID)entry, "CB Function Parameter Is Invalid ", AK_NULL);
	AK_ASSERT_PTR(argv, "ARGV Parameter Is Invalid ", AK_NULL);

	threadParm = (T_pTHREAD_PARM)Fwl_Malloc(sizeof(T_THREAD_PARM));
	AK_ASSERT_PTR(threadParm, "Malloc threadParm Failure ", AK_NULL);	
	
	threadParm->argc = argc;
	threadParm->argv = argv;
	threadParm->auto_start = AK_START;
	threadParm->fEntry = entry;
	memcpy(threadParm->name, name, strlen(name)<FUNC_NAME_LEN ? strlen(name) : FUNC_NAME_LEN-1);
	threadParm->preempt = AK_PREEMPT;
	threadParm->priority = priority;
	threadParm->time_slice = slice;
	threadParm->stackAddr = Fwl_Malloc(TASK_STACK_SIZE);
	threadParm->stack_size = TASK_STACK_SIZE;

	if(AK_NULL == threadParm->stackAddr)
	{
		AK_DEBUG_OUTPUT("GenrlThread:	Thread Stack Malloc Failure.\n");
		Fwl_FreeTrace(threadParm);
		return AK_NULL;
	}
	threadParm->name[strlen(threadParm->name) + 1] = '\0';
	
	return threadParm;

}

T_BOOL GenrlThread_Create(T_pGENRL_THREAD thread)
{
	AK_ASSERT_PTR(thread, "Input Parameter Is Invalid ", AK_FALSE);
	
	AK_DEBUG_OUTPUT("\nargc: %d, priority: %d, time_slice: %d\n", thread->parm->argc, thread->parm->priority, thread->parm->time_slice);

	thread->evGroup = AK_Create_Event_Group();
	if (AK_IS_INVALIDHANDLE(thread->evGroup) )
	{
		AK_DEBUG_OUTPUT("GenrlThread:	Create Event Group Failure.\n");		
		
		return AK_FALSE;
	}
	
	thread->task = AK_Create_Task((T_pVOID)thread->parm->fEntry, thread->parm->name, 
								thread->parm->argc, thread->parm->argv,
								thread->parm->stackAddr, thread->parm->stack_size, 
								thread->parm->priority, thread->parm->time_slice,
								thread->parm->preempt, thread->parm->auto_start);
	if (AK_IS_INVALIDHANDLE(thread->task))
	{
		AK_DEBUG_OUTPUT("GenrlThread:	Create Thread Failure.\n");
		
		AK_Delete_Event_Group(thread->evGroup);
		
		return AK_FALSE;
	}	

	return AK_TRUE;
}

T_BOOL	GenrlThread_Destroy(T_pGENRL_THREAD thread)
{
	AK_ASSERT_PTR(thread, "Input Parameter Is Invalid ", AK_FALSE);	
		
	if(AK_SUCCESS != AK_Delete_Task(thread->task))
	{
		// AK_DEBUG_OUTPUT("GenrlThread:	Delete Task Failure, Terminate Task.\n");
		return AK_FALSE;
	}
	
	if(AK_SUCCESS != AK_Delete_Event_Group(thread->evGroup))
		AK_DEBUG_OUTPUT("GenrlThread:	Delete Event Group Failure.\n");	
	
	AK_DEBUG_OUTPUT("GenrlThread:	Delete Thread %X. ... ...\n", thread);
	
	thread->parm->stackAddr = Fwl_FreeTrace(thread->parm->stackAddr);
	thread->parm = Fwl_FreeTrace(thread->parm);
	thread = Fwl_FreeTrace(thread);

	return AK_TRUE;
}
/*
T_BOOL	GenrlThread_PostEvent(T_pGENRL_THREAD thread, T_U32 event, T_OPTION operation)
{
	return AK_Set_Events(thread->evGroup, event, operation);
}
*/
T_VOID GenrlThread_Exit(T_pGENRL_THREAD thread)
{
	T_U32 times = 0;
	
	AK_ASSERT_PTR_VOID(thread, "Input Parameter Is Invalid ");
	
	// Close Timer
	GenrlThread_StopTimer(thread);
	
	do
	{
		// Exit Thread
		GenrlThread_PostEvent(thread, EVT_THREAD_EXIT, AK_OR);
		AK_Sleep(10);
		
	}while(!GenrlThread_Destroy(thread) && ++times < CLOSE_THREAD_MAX_TIMES);
	if (times >= CLOSE_THREAD_MAX_TIMES)
	{
		AK_DEBUG_OUTPUT("GenrlThread:	Delete Task Failure, Terminate Task.\n");
		AK_Terminate_Task(thread->task);
		if(AK_SUCCESS != AK_Delete_Event_Group(thread->evGroup))
		AK_DEBUG_OUTPUT("GenrlThread:	Delete Event Group Failure.\n");	
	
		AK_DEBUG_OUTPUT("GenrlThread:	Delete Thread %X. ... ...\n", thread);
	
		thread->parm->stackAddr = Fwl_FreeTrace(thread->parm->stackAddr);
		thread->parm = Fwl_FreeTrace(thread->parm);
		thread = Fwl_FreeTrace(thread);
	}
	
	if(CLOSE_THREAD_MAX_TIMES <= times)
		AK_DEBUG_OUTPUT("GenrlThread:	Exit Thread Failure.\n");
}
#endif

/*****************************************************************************/
#ifdef OS_WIN32
T_VOID MMU_InvalidateDCache (T_VOID)
{

}

T_VOID MMU_Clean_Invalidate_Dcache (T_VOID)
{

}

T_S32 AK_Reset_Queue(T_hQueue queue)
{
	return 0;
}
#endif

/*
 * End of File
 */
