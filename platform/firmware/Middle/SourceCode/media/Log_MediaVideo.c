/**
 * @file Log_MediaVideo.c
 * @brief Video Decoder Logic Implemetation for Multi-thread
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Xie_Wenzhong
 * @date 2011-3-17
 * @version 1.0
 */

#include "fwl_osfs.h"
#include "akError.h"

#include "Fwl_display.h"
#include "Fwl_VME.h"
#include "Arch_mmu.h"
#include "Arch_Lcd.h"
#include "Hal_timer.h"
#include "Fwl_Wave.h"
#include "Fwl_WaveOut.h"
#include "akos_api.h"
#include "AkAppMgr.h"
#include "File.h"
#include "Fwl_ImageLib.h"
#include "Log_MediaPlayer.h"
#include "Lib_SDcodec.h"
#include "Lib_SdFilter.h"
#include "Eng_debug.h"
#include "Fwl_osMalloc.h"
#include "video_stream_lib.h"
#include "Lib_Image_Api.h"
#include "Eng_Math.h"

#include "Log_MediaStruct.h"
#include "Log_MediaVideo.h"
#include "Log_MediaDmx.h"

#define VIDEO_MEM_MIN	0x00032000	// 200k
#define VIDEO_MEM_MAX	0x00080000	// 512K	// 0x00200000	// 2.0M	0x00380000	// 3.5M
#define FF_FR_MIN_SLICE	1000 // 1000ms


extern T_eMEDIALIB_VIDEO_CODE g_tCurrVideoType;

T_BOOL Vs_Yuv2RgbAvail(T_pDATA pBuff)
{  
#ifdef OS_ANYKA
	if (//MEDIALIB_VIDEO_H264 != g_tCurrVideoType 
		//&& MEDIALIB_VIDEO_RV != g_tCurrVideoType
		//&& 
		MEDIALIB_VIDEO_UNKNOWN != g_tCurrVideoType
#if CI37XX_PLATFORM		
		&& !MpuRefr_CheckFrameFinish(pBuff)
#else		
		&& !lcd_check_frame_finish(pBuff)
#endif		
		)
	{
		return AK_FALSE;
	}		
#endif
    return AK_TRUE; 
}

static T_VOID Vs_SetLibParm(T_VIDEOLIB_OPEN_INPUT *open_input, const T_MEDIALIB_DMX_INFO *dmxInfo)
{
	T_U32 reslv;
	T_U32 bitRate;
	
	reslv = dmxInfo->m_uWidth * dmxInfo->m_uHeight;
	reslv = reslv > VIDEO_MEM_MIN ? reslv : VIDEO_MEM_MIN;
	
	bitRate = (dmxInfo->m_ulVideoBitrate >> 20) > 1 ? (dmxInfo->m_ulVideoBitrate >> 20) : 1;
	bitRate = bitRate > 2 ? (bitRate>>1) : bitRate; 
	
	g_tCurrVideoType  		= dmxInfo->m_VideoDrvType;
	open_input->m_VideoType = dmxInfo->m_VideoDrvType;
	open_input->m_ulBufSize = reslv*bitRate > VIDEO_MEM_MAX ? VIDEO_MEM_MAX : reslv*bitRate;

	Fwl_Print(C3, M_VDEC, "Video Decode Buffer Size: %d K", (open_input->m_ulBufSize>>10));
	
	//if(dmxInfo->m_bHasAudio)
		open_input->m_bNeedSYN 	= AK_TRUE;
	//else
	//	open_input->m_bNeedSYN 	= AK_FALSE;
	
	open_input->m_bFixedStream 	= AK_TRUE;
	
	open_input->m_uWidth 		= dmxInfo->m_uWidth;
	open_input->m_uHeight 		= dmxInfo->m_uHeight;
}

static T_pVOID Vs_Malloc(T_U32 size)
{
	return (void *)Fwl_MallocAndTrace((size), ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

static T_pVOID Vs_Free(T_pVOID var)
{
   return Fwl_FreeAndTrace(var, ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

static T_pVOID Vs_CreateMutexCB(T_VOID)
{
	return (T_pVOID)AK_Create_Semaphore(1, AK_PRIORITY);
}

static T_VOID Vs_ReleaseMutexCB(T_pVOID mutex)
{
	AK_Delete_Semaphore((T_hSemaphore)mutex);
}

static T_S32 Vs_LockCB(T_pVOID mutex, T_S32 timeOut)
{
	if (AK_SUCCESS == AK_Obtain_Semaphore((T_hSemaphore)mutex, timeOut))
		return AK_TRUE;

	return AK_FALSE;
}

static T_S32 Vs_UnlockCB(T_pVOID mutex)
{
	if (AK_SUCCESS == AK_Release_Semaphore((T_hSemaphore)mutex))
		return AK_TRUE;

	return AK_FALSE;
}

static T_VOID Vs_SetVideoLibCB(T_VIDEOLIB_CB *videoCB)
{
	AK_ASSERT_PTR_VOID(videoCB, "VideoDEC:	videoCB Parameter Is Invalid ");
	memset(videoCB, 0, sizeof(T_VIDEOLIB_CB));
	
	videoCB->m_FunPrintf 	= (MEDIALIB_CALLBACK_FUN_PRINTF)AkDebugOutput;
	
	videoCB->m_FunMalloc 	= (MEDIALIB_CALLBACK_FUN_MALLOC)Vs_Malloc;
	videoCB->m_FunFree 		= (MEDIALIB_CALLBACK_FUN_FREE)Vs_Free;

	videoCB->m_FunMMUInvalidateDCache 	= MMU_InvalidateDCache;	
	videoCB->m_FunCleanInvalidateDcache = MMU_Clean_Invalidate_Dcache;	
	videoCB->m_FunCheckDecBuf 			= Vs_Yuv2RgbAvail;	
	videoCB->m_FunRtcDelay				= (MEDIALIB_CALLBACK_FUN_RTC_DELAY)AK_Sleep; 
	
	videoCB->m_FunMutexCreate			= Vs_CreateMutexCB;
	videoCB->m_FunMutexRelease			= Vs_ReleaseMutexCB;
	videoCB->m_FunMutexLock				= Vs_LockCB;
	videoCB->m_FunMutexUnlock			= Vs_UnlockCB;
}

static T_pVOID Vs_OpenVS(T_MEDIALIB_DMX_INFO *dmxInfo)
{
	T_VIDEOLIB_OPEN_INPUT open_input;
	memset(&open_input, 0, sizeof(T_VIDEOLIB_OPEN_INPUT));
	
	AK_ASSERT_PTR(dmxInfo, "VideoDEC:	dmxInfo Parameter Is Invalid.\n", AK_NULL);

	Vs_SetLibParm(&open_input, dmxInfo);	
	Vs_SetVideoLibCB(&open_input.m_CBFunc);

	Fwl_Print(C4, M_VDEC, "VB_Rate: %d.\n", dmxInfo->m_ulVideoBitrate);
		
	return VideoStream_Open(&open_input);
}

T_pVOID Vs_FreeVS(T_pVIDEO_DEC video)
{
	AK_ASSERT_PTR(video, "VideoDEC:	video Parameter Is Invalid ", AK_NULL);

	VideoStream_Close(video->hVS);

	return Fwl_FreeTrace(video);
}

T_pVOID Vs_PrepareDec(T_VOID)
{
#if CI37XX_PLATFORM	
	MpuRefr_Init();
	AK_Sleep(1); 
#endif
	
	return  MPlayer_GetPlayer();
}

T_pVOID Vs_Close(T_pVIDEO_DEC pVideo)
{
	AK_ASSERT_PTR(pVideo, "VideoDEC:	pVideo Parameter Is Invalid ", AK_NULL);

	MpuRefr_Stop();
	if(!VideoStream_Close(pVideo->hVS))
		Fwl_Print(C2, M_VDEC, "Close Viedo Stream Failure.\n");
			
	pVideo->hVS = AK_NULL;
	
	return Fwl_FreeTrace(pVideo);
}

static T_pVOID Vs_DecodeFirstFrame(T_pDEMUXER dmx)
{
	T_U32 frameSize;
	T_pDATA video_buf = AK_NULL;
	T_pVOID hVS;
		
	// Open Video Stream Decoder
	hVS = Vs_OpenVS(&dmx->dmxInfo);
	if (!hVS)
	{
		Fwl_Print(C2, M_VDEC, "open video Stream Failuer.\n");
		return AK_NULL;
	}	
	
	frameSize = MediaLib_Dmx_GetFirstVideoSize(dmx->hMedia);

	video_buf = VideoStream_GetAddr(hVS, frameSize);
	if( !video_buf
		|| !MediaLib_Dmx_GetFirstVideo(dmx->hMedia, video_buf, &frameSize)
		|| !VideoStream_UpdateAddr(hVS, video_buf, frameSize)
		|| 1 != VideoStream_SYNDecodeHeader(hVS))					// Decode First Frame
	{
		Fwl_Print(C2, M_VDEC, "Decode First Frame Failure.\n ");
	
		VideoStream_Close(hVS);

		return AK_NULL;	
	}

	Fwl_Print(C4, M_VDEC, "Decode First Frame Success.\n");	
	
	return hVS;
}


T_BOOL Vs_OpenVideoDecoder(T_pMT_MPLAYER hPlayer)
{
	AK_ASSERT_PTR(hPlayer, "VideoDEC:	Input Parameter Is Invalid ", AK_FALSE);
	
#ifdef MEDIA_FAST_SWITCH
	if (hPlayer->pVideo)
	{
		MpuRefr_Stop();
		VideoStream_Reset(hPlayer->pVideo->hVS);

		return AK_TRUE;
	}
#else
	while (hPlayer->pVideo)
	{
		if (IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_VIDEO))
		{
			T_SYS_MAILBOX mailbox;
			
			mailbox.event = EVT_VIDEO_CLOSE;
			IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_VIDEO, &mailbox);
			AK_Sleep(2);
		}
		else 
		{			
			hPlayer->pVideo = Vs_Close(hPlayer->pVideo);
		}	
	}
#endif	
	
	hPlayer->pVideo = (T_pVIDEO_DEC)Fwl_Malloc(sizeof(T_VIDEO_DEC));
	AK_ASSERT_PTR(hPlayer->pVideo, "VideoDEC:	pVideo Malloc Failure", AK_FALSE);
	memset(hPlayer->pVideo, 0, sizeof(T_VIDEO_DEC));
	
	// Decode First Frame
	hPlayer->pVideo->hVS = Vs_DecodeFirstFrame(hPlayer->pDmx);
	if(!hPlayer->pVideo->hVS)
	{
		Fwl_Print(C2, M_VDEC, "Decode First Frame Failure.\n ");
		
		hPlayer->pVideo = Fwl_FreeTrace(hPlayer->pVideo);		
	
		return AK_FALSE;
	}
	
	hPlayer->pVideo->timer 		= ERROR_TIMER;

	Fwl_Print(C3, M_VDEC, "Decode First Frame Success.\n");

	return AK_TRUE;
}

T_VOID Vs_HandleDecode(T_pMT_MPLAYER pPlayer)
{
	T_pDEMUXER pDmx;
	T_pVIDEO_DEC pVideo;
	T_U32 time;
	T_S32 stamp;
	static T_U32 failNum = 0;
		
	pDmx 	= pPlayer->pDmx;
	pVideo 	= pPlayer->pVideo;

	if (!pDmx || !pVideo)
		return;

	pVideo->videoOut.m_pBuffer = AK_NULL;
	
	switch (pPlayer->status)
	{
	case MPLAYER_PLAY:
		if (pDmx->dmxInfo.m_bHasAudio)
		{
			WaveOut_GetStatus(&time, WAVEOUT_CURRENT_TIME);
			
			// When Audio Time is Less Than Video Time, 
			// The REST Video Will Drive By  pDmx->curTime += VIDEO_SCAN_INTERVAL
			if (pDmx->curTime < time)
				pDmx->curTime = time;
			else
				pDmx->curTime += VIDEO_SCAN_INTERVAL;
		}
		else
		{
			pDmx->curTime = get_tick_count() - pDmx->zeroTime;
		}

		time = VideoStream_SYNDecode(pVideo->hVS, &pVideo->videoOut, pDmx->curTime);
		
		break;

	case MPLAYER_FAST:
		do{
			stamp = VideoStream_GetNextPts(pVideo->hVS);
			
			if (stamp < 0
				|| FF_FR_MIN_SLICE < Fwl_Abs(stamp - pDmx->curTime))
				break;

			VideoStream_SpecialDecode(pVideo->hVS, &pVideo->videoOut, AK_TRUE);
		}while (AK_TRUE);
		
		if (stamp >= 0)		
			pDmx->curTime = VideoStream_SpecialDecode(pVideo->hVS, &pVideo->videoOut, AK_FALSE);

		break;

	default:
		Fwl_Print(C3, M_VDEC, "MPlayer Is Error Status");
		break;
	}
	
	if (pVideo->videoOut.m_pBuffer)
	{
		
#if CI37XX_PLATFORM	
		MpuRefr_Refresh(&pVideo->videoOut);
#else		
		pVideo->showCB(pVideo->videoOut.m_pBuffer, pVideo->videoOut.m_pBuffer_u, pVideo->videoOut.m_pBuffer_v,\
				pVideo->videoOut.m_uDispWidth, pVideo->videoOut.m_uDispHeight, pVideo->videoOut.m_uOriWidth, pVideo->videoOut.m_uOriHeight);
		
#endif		
		failNum = 0;	
	}	
	else if (++failNum > MAX_DEC_FAIL_NUM
		|| (MediaLib_Dmx_CheckVideoEnd(pDmx->hMedia) 
			&& VIDEO_STREAM_NODATA_ERR == VideoStream_GetLastError(pVideo->hVS)))			
	{		
		Fwl_Print(C3, M_VDEC, "NOT More Video Data to Decode, Exit Video Decoder Thread, Fail Num: %d.\n", failNum);
		DmxMgr_StopTimer(pPlayer);
		
		VideoStream_Reset(pVideo->hVS);
		
		if (pPlayer->endCB)
		{
			pPlayer->endCB(T_END_TYPE_NORMAL);
			AK_Sleep(10);
		}
		else
		{
			Fwl_Print(C3, M_VDEC, "Close Player By Video Decoder End.\n");
			DmxMgr_HandleEvt(EVT_PLAY_CLOSE, 0);
		}
	}
	else 	
		Fwl_Print(C4, M_VDEC, "VD Failure");
}



/*****************************************************************************/
//used by video lib
T_BOOL VD_EXfunc_JPEG2YUV(T_U8 *srcJPEG, T_U32 srcSize, T_U8 *dstYUV, T_S32 *pWidth, T_S32 *pHeight) 
{
	if (AK_NULL == dstYUV)
	{
		T_U8 y_h_samp	= 0;
		T_U8 y_v_samp	= 0;
		T_U16 width		= 0;
		T_U16 height	= 0;

		if (!Img_JpegInfo(srcJPEG, srcSize, &width, &height, &y_h_samp, &y_v_samp))
		{
			Fwl_Print(C2, M_VDEC, "JPEG INFO FAILURE");
			return AK_FALSE;
		}

		*pWidth 	= (T_S32)width;
		*pHeight 	= (T_S32)height;
	}
	
	else if (!Img_VideoJPEG2YUV_Mutex(srcJPEG, srcSize, dstYUV, pWidth, pHeight))
	{
		Fwl_Print(C2, M_VDEC, "JPEG 2 YUV FAILURE");
		return AK_FALSE;
	}	

	return AK_TRUE;
}


/*
 * End of File
 */

