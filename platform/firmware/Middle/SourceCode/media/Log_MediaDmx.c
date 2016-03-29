/** 
 * @file Log_MediaDmx.c
 * @brief Media Demuxer Logic Implemetation for Multi-thread
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
#include "Arch_mmu.h"
#include "Arch_Lcd.h"
#include "Hal_timer.h"
#include "AkAppMgr.h"
#include "Eng_callback.h"
#include "Eng_String.h"
#include "Eng_String_UC.h"
#include "akos_api.h"
#include "File.h"
#include "Ctl_AviPlayer.h"

#include "Log_MediaPlayer.h"
#include "Lib_SDcodec.h"
#include "Lib_SdFilter.h"
#include "Eng_debug.h"
#include "Fwl_osMalloc.h"
#include "video_stream_lib.h"
#include "Lib_Image_Api.h"
#include "video_stream_lib.h"

#include "Log_MediaStruct.h"
#include "Log_MediaVideo.h"
#include "Log_MediaDmx.h"
#include "Log_MediaAudio.h"
#include "Log_MediaVisualAudio.h"


#define FILL_AUDIO_TIMES		0x00000020	// 32
#define FILL_VIDEO_TIMES		0x0000000C	// 12

#define AUDIO_DATA_TURN_SIZE 	0x00004000	// 16K


/*****************************************************************************/

static T_BOOL sd_is_busy(T_VOID)
{
    return AK_FALSE;
}

T_S32 Buf_Exist(T_pVOID pFile)
{
	if (AK_NULL == pFile)
		return 0;
	
	return 1;
}

T_S32 Buf_Write(T_S32 hbuf, T_pVOID pdata, T_S32 size)
{    
    return 0;
}

T_S32 Buf_Read(T_S32 hbuf, T_pVOID pdata, T_S32 size)
{
    T_MEDIALIB_BUFFER *mediaBuf = (T_MEDIALIB_BUFFER*)hbuf;

    if (size > 0)
    {
        if (mediaBuf->bufLen - mediaBuf->bufPos >= (T_U32)size)
        {
        	Utl_MemCpy(pdata, mediaBuf->pBuf + mediaBuf->bufPos, size);
            mediaBuf->bufPos += size;
            return size;
        }
        else
        {
            T_U32 len;
			len = mediaBuf->bufLen - mediaBuf->bufPos;
            Utl_MemCpy(pdata, mediaBuf->pBuf + mediaBuf->bufPos, len);
            mediaBuf->bufPos = mediaBuf->bufLen;

            return len;
        }
    }
    return 0;
}

T_S32 Buf_Seek(T_S32 hbuf, T_S32 offset, T_S32 whence)
{
    T_MEDIALIB_BUFFER *mediaBuf = (T_MEDIALIB_BUFFER*)hbuf;

	switch (whence)
	{
	case _FSEEK_SET:
		if ((T_U32)offset < mediaBuf->bufLen)
        {
            mediaBuf->bufPos = offset; 
        }
        else
        {
            mediaBuf->bufPos = mediaBuf->bufLen;       
        }
		
        return mediaBuf->bufPos;
		break;

	case _FSEEK_CUR:
		if (offset > 0)
        {
            if ((T_U32)offset < mediaBuf->bufLen - mediaBuf->bufPos)
            {
                mediaBuf->bufPos += offset;
            }
            else
            {
                mediaBuf->bufPos = mediaBuf->bufLen;    
            }            
        }
        else
        {
            if ((T_U32)(-offset) < mediaBuf->bufPos)
            {
                mediaBuf->bufPos += offset;
            }
            else
            {
                mediaBuf->bufPos = 0;
            }
        }
		
        return mediaBuf->bufPos;
		break;
		
	case _FSEEK_END:
		if (offset < 0)
        {
            if ((T_U32)(-offset) < mediaBuf->bufLen)
            {
                mediaBuf->bufPos = mediaBuf->bufLen + offset;
            }
            else
            {
                mediaBuf->bufPos = 0;
            }    
        }
        else
        {
            mediaBuf->bufPos = mediaBuf->bufLen;
        }
		
        return mediaBuf->bufPos;
		break;

	default:
		return 0;
		break;
	}
}

T_S32 Buf_Tell(T_S32 hbuf)
{
    return Buf_Seek(hbuf, 0, _FSEEK_CUR);
}

T_U32 Buf_GetLength(T_S32 hbuf)
{
	T_MEDIALIB_BUFFER *mediaBuf = (T_MEDIALIB_BUFFER*)hbuf;

    return mediaBuf->bufLen;
}

static T_VOID UnmapAddr( T_U32 addr, T_U32 size)       //取消映射
{
	return;
}

static T_U32 RegBaseAddr( T_U32 addr, T_U32 size)
{
	return addr;
}

static T_pVOID IBufToRBuf( T_pVOID mem)
{
	return mem;
}

static T_VOID MMU_InvalidateDCacheFunc(T_VOID)
{	
	MMU_Clean_Invalidate_Dcache();
}


static T_pVOID Dmx_Malloc(T_U32 size)
{
	return (void *)Fwl_MallocAndTrace((size), ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

static T_pVOID Dmx_Free(T_pVOID var)
{
   return Fwl_FreeAndTrace(var, ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

/*****************************************************************************/

/*
 *@brief: Set Media Lib Callback Function
 *@param[in] cbFunc, MEDIALIB Callback Struct
 *@param[in] isFile,  Is for File Or Buffer
 *@retval:T_VOID
 */
T_MEDIALIB_CB* Dmx_SetMediaLibCB(T_MEDIALIB_CB *cbFunc, T_BOOL isFile, T_BOOL openPRN)
{
	AK_ASSERT_PTR(cbFunc, "Input Parameter Is Invalid ", AK_NULL);
	
	memset((T_U8*)cbFunc, 0, sizeof(T_MEDIALIB_CB));

	if (openPRN)
		cbFunc->m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)AkDebugOutput;
	else
		cbFunc->m_FunPrintf = AK_NULL;	
	
	if (isFile)
	{
	    cbFunc->m_FunRead 	= (MEDIALIB_CALLBACK_FUN_READ)Fwl_FileRead;
	    cbFunc->m_FunWrite 	= (MEDIALIB_CALLBACK_FUN_WRITE)Fwl_FileWrite;
	    cbFunc->m_FunSeek 	= (MEDIALIB_CALLBACK_FUN_SEEK)Fwl_FileSeekEx;
	    cbFunc->m_FunTell 	= Fwl_FileTell;
		cbFunc->m_FunFileHandleExist	= (MEDIALIB_CALLBACK_FUN_FILE_HANDLE_EXIST)File_Exist;
		cbFunc->m_FunFileGetLength		= (MEDIALIB_CALLBACK_FUN_FILE_GET_LENGTH)Fwl_GetFileLen;
	}
	else 	// Buffer Data
	{
		cbFunc->m_FunRead 	= Buf_Read;
	    cbFunc->m_FunWrite 	= Buf_Write;
	    cbFunc->m_FunSeek 	= Buf_Seek;
	    cbFunc->m_FunTell 	= Buf_Tell;
		cbFunc->m_FunFileHandleExist	= (MEDIALIB_CALLBACK_FUN_FILE_HANDLE_EXIST)Buf_Exist;
		cbFunc->m_FunFileGetLength		= (MEDIALIB_CALLBACK_FUN_FILE_GET_LENGTH)Buf_GetLength;
	}
	
    cbFunc->m_FunMalloc = Dmx_Malloc;
    cbFunc->m_FunFree 	= (MEDIALIB_CALLBACK_FUN_FREE)Dmx_Free;
	
    cbFunc->m_FunLoadResource 		= AK_NULL;
    cbFunc->m_FunReleaseResource 	= AK_NULL;
	
    cbFunc->m_FunRtcDelay 			= (MEDIALIB_CALLBACK_FUN_RTC_DELAY)AK_Sleep;
    cbFunc->m_FunDMAMemcpy 			= (MEDIALIB_CALLBACK_FUN_DMA_MEMCPY)memcpy;
    cbFunc->m_FunMMUInvalidateDCache = MMU_InvalidateDCache;
    cbFunc->m_FunCleanInvalidateDcache = MMU_InvalidateDCacheFunc; // MMU_Clean_Invalidate_Dcache;
    cbFunc->m_FunCheckDecBuf 		= Vs_Yuv2RgbAvail;
#ifdef OS_ANYKA
	cbFunc->m_FunFileSysIsBusy		= sd_is_busy;
#endif
	
	cbFunc->m_FunMapAddr			= RegBaseAddr;
	cbFunc->m_FunUnmapAddr			= UnmapAddr;
	cbFunc->m_FunDMAFree			= (MEDIALIB_CALLBACK_FUN_DMA_FREE)Dmx_Free;
	cbFunc->m_FunDMAMalloc			= Dmx_Malloc;
	cbFunc->m_FunVaddrToPaddr		= IBufToRBuf;
	cbFunc->m_FunRegBitsWrite		= (MEDIALIB_CALLBACK_FUN_REG_BITS_WRITE)RegBitsWriteCB;

	cbFunc->m_FunVideoHWLock		= AK_NULL;
	cbFunc->m_FunVideoHWUnlock		= AK_NULL;

	return cbFunc;
}


#define TriggerEvent2Task(appID,event) IAppMgr_PostUniqueEvt(AK_GetAppMgr(),appID,event)

T_VOID DriveEvtCB_Demux(T_TIMER timer_id, T_U32 delay)
{
	T_SYS_MAILBOX mailbox;
	
    mailbox.event = EVT_DMX_SCAN;
	TriggerEvent2Task(AKAPP_CLSID_MEDIA, &mailbox);
}

static T_VOID DriveEvtCB_DelayExit(T_TIMER timer_id, T_U32 delay)
{
	T_SYS_MAILBOX mailbox;
	
    mailbox.event = EVT_DMX_EXIT;
	TriggerEvent2Task(AKAPP_CLSID_MEDIA, &mailbox);
}


T_VOID DriveEvtCB_Video(T_TIMER timer_id, T_U32 delay)
{
	T_SYS_MAILBOX mailbox; 
	
    mailbox.event = EVT_VIDEO_SCAN;
	TriggerEvent2Task(AKAPP_CLSID_VIDEO, &mailbox);
}

T_VOID DriveEvtCB_Audio(T_TIMER timer_id, T_U32 delay)
{
	T_SYS_MAILBOX mailbox; 
	
    mailbox.event = EVT_AUDIO_SCAN;
	TriggerEvent2Task(AKAPP_CLSID_AUDIO, &mailbox);
}


T_BOOL CThread_StartTimer(T_TIMER *timer, T_U32 millSec, T_fVTIMER_CALLBACK cbFunc)
{
	AK_ASSERT_PTR(timer, "timer Parameter Is Invalid ", AK_FALSE);
	
	if (ERROR_TIMER == *timer
		&& ERROR_TIMER == (*timer = vtimer_start(millSec, AK_TRUE, cbFunc)))
	{
		return AK_FALSE;
	}
	else
	{
		return AK_TRUE;
	}
}

T_VOID CThread_StopTimer(T_TIMER *pTimer)
{
	AK_ASSERT_PTR_VOID(pTimer, "pTimer Parameer Is Invalid ");
	
	if(ERROR_TIMER != *pTimer)
	{
		vtimer_stop(*pTimer);
		*pTimer = ERROR_TIMER;
	}
}

static T_U32 Dmx_WriteAudio2Tail(T_MEDIALIB_STRUCT hMedia, T_pVOID hSD, T_U8* buf, T_U32 bufLen)
{
	T_U32 writeLen = 0;
	T_U32 packLen = MediaLib_Dmx_GetAudioDataSize(hMedia);
	
	do{		
		//对于flv可能出现Demuxer_GetAudioData的返回值与Demuxer_GetAudioDataSize不相同的情况
		packLen = MediaLib_Dmx_GetAudioData(hMedia, buf + writeLen, packLen);
		if(!packLen )
		{
			Fwl_Print(C3, M_DMX, "Can NOT Get Audio Data, Set Audio Decoder _SD_BM_LIVE MODE.\n" );
			_SD_SetBufferMode(hSD, _SD_BM_LIVE);	// _SD_BM_LIVE _SD_BM_NORMAL
			break;
		}
			
		bufLen	 -= packLen;
		writeLen += packLen;

		packLen = MediaLib_Dmx_GetAudioDataSize(hMedia); 
		
	}while (writeLen < AUDIO_DATA_TURN_SIZE && packLen < bufLen);

	_SD_Buffer_Update(hSD, writeLen);

	return writeLen;
}

static T_U32 Dmx_WriteAudio2Both(T_MEDIALIB_STRUCT hMedia, T_pVOID hSD, T_AUDIO_BUFFER_CONTROL *audioBuf, T_U32 packLen)
{
	T_pDATA tmp = Fwl_Malloc(packLen);	
	AK_ASSERT_PTR(tmp, "DEMUX:	Malloc Audio temp Buffer Failure", 0);

	packLen = MediaLib_Dmx_GetAudioData(hMedia, tmp, packLen);
	if (!packLen)
	{
		Fwl_Print(C2, M_DMX, "_SD_BUFFER_WRITABLE_TWICE State, Set Audio Decoder _SD_BM_LIVE MODE.\n" );
		_SD_SetBufferMode(hSD, _SD_BM_LIVE);	// _SD_BM_LIVE _SD_BM_NORMAL
		Fwl_FreeTrace(tmp);
		
		return 0;
	}		
				
	memcpy(audioBuf->pwrite, tmp, audioBuf->free_len);
	memcpy(audioBuf->pstart, tmp + audioBuf->free_len, packLen - audioBuf->free_len);

	_SD_Buffer_Update(hSD, packLen);
	Fwl_FreeTrace(tmp);
	
	return packLen;
}

T_U32 Dmx_GetAudio2Decoder(T_MEDIALIB_STRUCT hMedia, T_pVOID hSD)
{
	T_U32 audio_bytes = 0;
	T_AUDIO_BUFFER_CONTROL bufInfo;
	T_U32 packLen;
	T_S32 try_times = FILL_AUDIO_TIMES;
	T_U16 sizeOfBuf  = sizeof(T_AUDIO_BUFFER_CONTROL);
	T_U8 ret;

	AK_ASSERT_PTR(hMedia, "DEMUX: hMedia Is Invalid", 0);	
	AK_ASSERT_PTR(hSD, "DEMUX: hSD Is Invalid", 0);	
	
	do
	{	
		if (MediaLib_Dmx_CheckAudioEnd(hMedia)
			|| MEDIALIB_DMX_PLAY != MediaLib_Dmx_GetStatus(hMedia)
			)
		{
			//AK_DEBUG_OUTPUT("NA");
			break;
		}		

		// Get Package Length
		packLen = MediaLib_Dmx_GetAudioDataSize(hMedia);
		
		memset(&bufInfo, 0, sizeOfBuf);
		ret = _SD_Buffer_Check(hSD, &bufInfo);	// Buffer Length: 128K
#if 0
		if (_SD_BUFFER_WRITABLE == ret)
		 	AK_DEBUG_OUTPUT("W: %d\n", bufInfo.free_len);
		else if (_SD_BUFFER_WRITABLE_TWICE == ret)
		 	AK_DEBUG_OUTPUT("B:	%d\n", bufInfo.free_len + bufInfo.start_len);
#endif		
		if (packLen <= bufInfo.free_len)		
		{
			audio_bytes += Dmx_WriteAudio2Tail(hMedia, hSD, bufInfo.pwrite, bufInfo.free_len);
		}
		else if (packLen <= bufInfo.free_len + bufInfo.start_len)
		{
			// Data Length > Tail Space
			audio_bytes += Dmx_WriteAudio2Both(hMedia, hSD, &bufInfo, packLen);
		}
		else
		{
			// Data Length > Free Space
			break;
		}		
	}while (--try_times > 0);
	
	return audio_bytes;
}

T_U32 Dmx_GetVideo2Decoder(T_MEDIALIB_STRUCT hMedia, T_pVOID hVS)
{
	T_pDATA	videoBuf 	= AK_NULL;
	T_U32	videoBytes	= 0;
	T_U32	frameLen 	= 0;
	T_U8	i 			= 0;

	AK_ASSERT_PTR(hMedia, "DEMUX:	hMedia Parameter Is Invalid ", 0);
	AK_ASSERT_PTR(hVS, "DEMUX:	hVS Parameter Is Invalid ", 0);
		
	frameLen = MediaLib_Dmx_GetVideoFrameSize(hMedia);
	videoBuf = VideoStream_GetAddr(hVS, frameLen);
		
	while(videoBuf && i++ < FILL_VIDEO_TIMES)
	{		
		if(!MediaLib_Dmx_GetVideoFrame(hMedia, videoBuf, &frameLen))
		{
			Fwl_Print(C2, M_DMX, "Get Video Frame Failure.\n");
			return videoBytes;
		}
				
		if(!VideoStream_UpdateAddr(hVS, videoBuf, frameLen))//更新视频数据
			Fwl_Print(C2, M_DMX, "Can NOT Update Video Address.\n");

		videoBytes += frameLen;
		frameLen = MediaLib_Dmx_GetVideoFrameSize(hMedia);
		videoBuf = VideoStream_GetAddr(hVS, frameLen);
	}

	return videoBytes;
}


T_pVOID Dmx_CloseDemux(T_pDEMUXER dmx)
{
	AK_ASSERT_PTR(dmx, "DEMUX:	dmx Parameter Is Invalid ", AK_NULL);

	if (dmx->hMedia)
	{
		MediaLib_Dmx_Close(dmx->hMedia);	
		dmx->hMedia = AK_NULL;
	}

	if (dmx->isFile)
	{
		if (_FOPEN_FAIL != (T_hFILE)dmx->hFile)
		{
			Fwl_Print(C4, M_DMX, "Close File ... ...");
			Fwl_FileClose(dmx->hFile);
			Fwl_Print(C4, M_DMX, "Success");
			dmx->hFile = _FOPEN_FAIL;
		}
	}
	else
	{
		Fwl_FreeTrace((T_pVOID)dmx->hFile);
	}	
	
	return Fwl_FreeTrace(dmx);
}

static T_BOOL DmxMgr_StartVideoTimer(T_pDEMUXER dmx, T_pVIDEO_DEC video)
{
	T_U32 interval;
	T_pMT_MPLAYER pMPlayer = MPlayer_GetPlayer();
	
	if (MPLAYER_FAST == pMPlayer->status)
	{
		interval = VIDEO_FAST_INTERVAL;
	}
	else
	{
		interval = VIDEO_SCAN_INTERVAL;
	}
		
	if (dmx->dmxInfo.m_bHasVideo && ERROR_TIMER == video->timer)
	{
		DriveEvtCB_Video(0, 0);
		if (!CThread_StartTimer(&video->timer, interval, DriveEvtCB_Video))
		{
			Fwl_Print(C2, M_DMX, "Start Video Timer Failure");
		
			return AK_FALSE;
		}
		else 
			Fwl_Print(C4, M_DMX, "Start Video Timer 0x%x", video->timer);
	}
	
	return AK_TRUE;
}

static T_BOOL DmxMgr_StartAudioTimer(T_pDEMUXER dmx, T_pAUDIO_DEC audio)
{
	if (dmx->dmxInfo.m_bHasAudio && ERROR_TIMER == audio->timer)
	{
		DriveEvtCB_Audio(0, 0);
		if (!CThread_StartTimer(&audio->timer, AUDIO_SCAN_INTERVAL, DriveEvtCB_Audio))
		{
			Fwl_Print(C2, M_DMX, "Start audio Timer Failure");
		
			return AK_FALSE;
		}
		else 
			Fwl_Print(C4, M_DMX, "Start audio Timer 0x%x", audio->timer);
	}
	
	return AK_TRUE;
}


static T_U32 Dmx_DemuxData(T_pDEMUXER dmx, T_pAUDIO_DEC audio, T_pVIDEO_DEC video)
{	
	T_U32 readBytes = 0;
	
	AK_ASSERT_PTR(dmx, "DEMUX:	dmx Is Invalid ", 0);	
		
	if (dmx->dmxInfo.m_bHasAudio)
	{
		AK_ASSERT_PTR(audio, "DEMUX:	audio Is Invalid ", 0);
		
		readBytes += Dmx_GetAudio2Decoder(dmx->hMedia, audio->hSD);
		// Fwl_Print(C3, M_DMX, "A2D: %d\n", readBytes);		
	}
	
	if (dmx->dmxInfo.m_bHasVideo && !MediaLib_Dmx_CheckVideoEnd(dmx->hMedia))
	{
		AK_ASSERT_PTR(video, "DEMUX:	video Is Invalid ", 0);
		
		readBytes += Dmx_GetVideo2Decoder(dmx->hMedia, video->hVS);
		//Fwl_Print(C4, M_DMX, "V2D %d\n", Dmx_GetVideo2Decoder(dmx->hMedia, video->hVS));
	}
	
	if (ERROR_TIMER != dmx->timer)
	{
		DmxMgr_StartVideoTimer(dmx, video);
		DmxMgr_StartAudioTimer(dmx, audio);
	}

	return readBytes;
}


T_pVOID Dmx_Open(T_MEDIALIB_DMX_INFO *info, T_hFILE hFile,
					T_BOOL isFile, T_eMEDIALIB_MEDIA_TYPE mediaType, T_BOOL openPRN)
{
	T_pVOID hMedia;
	T_MEDIALIB_DMX_OPEN_INPUT dmxIn;
	T_MEDIALIB_DMX_OPEN_OUTPUT dmxOut;
	
	Dmx_SetMediaLibCB(&dmxIn.m_CBFunc, isFile, openPRN);
	dmxIn.m_hMediaSource 	= hFile;
	dmxIn.m_MediaType 		= mediaType;
	
	// Open Demuxer
	hMedia = MediaLib_Dmx_Open(&dmxIn, &dmxOut);
	if (!hMedia)
	{
		Fwl_Print(C2, M_DMX, "Dmx_GetInfo() -- Open Demuxer Failure");
		return AK_NULL;
	}	
	
	// Get Media Infomation
	if (!MediaLib_Dmx_GetInfo(hMedia, info))
	{
		Fwl_Print(C2, M_DMX, "Get Media Infomation Failure");
		
		MediaLib_Dmx_Close(hMedia);
		
		return AK_NULL;
	}
	
	Fwl_Print(C3, M_DMX, "Get Media Infomation Success");
	return hMedia;
}

static T_BOOL Dmx_Init(T_pDEMUXER* ppDmx, T_hFILE hFile, T_BOOL isFile)
{
	AK_ASSERT_PTR(ppDmx, "DEMUX:	ppDmx Is Invalid", AK_FALSE);
	
	*ppDmx = (T_pDEMUXER)Fwl_Malloc(sizeof(T_DEMUXER));
	AK_ASSERT_PTR(*ppDmx, "DEMUX:	Malloc *ppDmx FAILURE", AK_FALSE);
	
	memset(*ppDmx, 0, sizeof(T_DEMUXER));
	(*ppDmx)->hFile 	= hFile;
	(*ppDmx)->isFile 	= isFile;
	(*ppDmx)->timer 	= ERROR_TIMER;	
	
	return AK_TRUE;
}

T_pVOID Dmx_QueryInfoEx(T_MEDIALIB_DMX_INFO *pInfo, T_pVOID fname, T_BOOL isFile)
{
	T_hFILE hFile;
	T_pVOID hMedia;
		
	// Open File
	if (!isFile)
		hFile = (T_hFILE)fname;
	else if (_FOPEN_FAIL == (hFile = Fwl_FileOpen(fname, _FMODE_READ, _FMODE_READ)))	
	{
		Fwl_Print(C2, M_DMX, "Open File Failure");
		Printf_UC(fname);
		
		return AK_NULL;
	}	

	hMedia = Dmx_Open(pInfo, hFile, isFile, MEDIALIB_MEDIA_UNKNOWN, AK_FALSE);
	if (!hMedia)
	{
		Fwl_Print(C2, M_DMX, "Get Media Infomation Failure");
	}

	if (isFile)
		Fwl_FileClose(hFile);
	else
		Fwl_FreeTrace((T_pVOID)hFile);
	
	return hMedia;
}

T_BOOL Dmx_QueryInfo(T_MEDIALIB_DMX_INFO *pInfo, T_pVOID fname, T_BOOL isFile)
{
	T_MEDIALIB_STRUCT hMedia;

	hMedia = Dmx_QueryInfoEx(pInfo, fname, isFile);
	if (hMedia)
	{
		MediaLib_Dmx_Close(hMedia);
	
		return AK_TRUE;
	}

	return AK_FALSE;
}


/*****************************************************************************/
T_VOID DmxMgr_CloseVideo(T_pMT_MPLAYER pPlayer)
{
	T_U8 i = 0;
	T_SYS_MAILBOX mailbox;

	AK_ASSERT_PTR_VOID(pPlayer, "DEMUX:	pPlayer Is Invalid");
	AK_ASSERT_PTR_VOID(pPlayer->pDmx, "DEMUX:	pPlayer->pDmx Is Invalid");
	AK_ASSERT_PTR_VOID(pPlayer->pVideo, "DEMUX:	pPlayer->pVideo Is Invalid");
	
	pPlayer->pDmx->dmxInfo.m_bHasVideo = AK_FALSE;
		
	mailbox.event = EVT_VIDEO_CLOSE;
	IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_VIDEO, &mailbox);
	
	do{
		AK_Sleep(2);		
	}while(pPlayer->pVideo && ++i < MAX_WAIT_MSG_NUM);
}

T_VOID DmxMgr_CloseAudio(T_pMT_MPLAYER pPlayer)
{
	T_U8 i = 0;
	T_SYS_MAILBOX mailbox;

	AK_ASSERT_PTR_VOID(pPlayer, "DEMUX:	pPlayer Is Invalid");
	AK_ASSERT_PTR_VOID(pPlayer->pDmx, "DEMUX:	pPlayer->pDmx Is Invalid");
	AK_ASSERT_PTR_VOID(pPlayer->pAudio, "DEMUX:	pPlayer->pAudio Is Invalid");
	
	pPlayer->pDmx->dmxInfo.m_bHasAudio = AK_FALSE;

#ifdef MEDIA_FAST_SWITCH
	mailbox.event = EVT_AUDIO_STOP;
#else
	mailbox.event = EVT_AUDIO_CLOSE;
#endif
	IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);

#ifndef MEDIA_FAST_SWITCH
	do
	{
		AK_Sleep(2);
	}while(pPlayer->pAudio && ++i < MAX_WAIT_MSG_NUM);
	
	IThread_Suspend(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIO));

	Fwl_Print(C3, M_DMX, "Audio Thread Suspend %d", i);
#endif
}


T_VOID DmxMgr_StopTimer(T_pMT_MPLAYER pPlayer)
{
	IThread* thread;
	
	AK_ASSERT_PTR_VOID(pPlayer, "DEMUX:	pPlayer NOT Initialized.\n");
	AK_ASSERT_PTR_VOID(pPlayer->pDmx, "DEMUX:	pPlayer->pDmxr Is Invalid.\n");

	Fwl_Print(C4, M_DMX, "Stop demux Timer: 0x%x ... ...", pPlayer->pDmx->timer);
	CThread_StopTimer(&pPlayer->pDmx->timer);

	thread = IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA);
	if (thread)
		AK_Reset_Queue(IThread_GetQueue(thread));

	if (pPlayer->pDmx->dmxInfo.m_bHasVideo
		&& pPlayer->pVideo)
	{
		Fwl_Print(C4, M_DMX, "Stop video Timer: 0x%x ... ...", pPlayer->pVideo->timer);	
		CThread_StopTimer(&pPlayer->pVideo->timer);

		thread = IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_VIDEO);
		if (thread)
			AK_Reset_Queue(IThread_GetQueue(thread));

	}

	if (pPlayer->pDmx->dmxInfo.m_bHasAudio
		&& pPlayer->pAudio)
	{
		Fwl_Print(C4, M_DMX, "Stop Audio Timer: 0x%x ... ...", pPlayer->pAudio->timer);	
		CThread_StopTimer(&pPlayer->pAudio->timer);

		thread = IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIO);
		if (thread)
			AK_Reset_Queue(IThread_GetQueue(thread));

	}
}

static T_BOOL DmxMgr_Pause(T_pMT_MPLAYER pPlayer)
{
	DmxMgr_StopTimer(pPlayer);

	if(!MediaLib_Dmx_Pause(pPlayer->pDmx->hMedia))
	{
		pPlayer->status = MPLAYER_ERR;
		return AK_FALSE;
	}

	AK_Sleep(40);
	pPlayer->status = MPLAYER_PAUSE;
	return AK_TRUE;
}

static T_BOOL DmxMgr_Resume(T_pMT_MPLAYER pPlayer)
{	
	if (pPlayer->pDmx->dmxInfo.m_bHasVideo && pPlayer->pVideo->hVS)
		VideoStream_Resume(pPlayer->pVideo->hVS);
	
	if (!pPlayer->pDmx->dmxInfo.m_bHasAudio)
			pPlayer->pDmx->zeroTime = get_tick_count() - pPlayer->pDmx->curTime;
		
	if (!MediaLib_Dmx_Resume(pPlayer->pDmx->hMedia))
	{
		Fwl_Print(C2, M_DMX, "Resume Failure");
		return AK_FALSE;
	}

	if (MEDIALIB_DMX_END == MediaLib_Dmx_GetStatus(pPlayer->pDmx->hMedia))
	{
		DmxMgr_StartVideoTimer(pPlayer->pDmx, pPlayer->pVideo);
		DmxMgr_StartAudioTimer(pPlayer->pDmx, pPlayer->pAudio);
	}

	if (!CThread_StartTimer(&pPlayer->pDmx->timer, DMX_SCAN_INTERVAL, DriveEvtCB_Demux))
	{	
		Fwl_Print(C2, M_DMX, "Set Demuxer Timer Failure");
	
		return AK_FALSE;
	}
	
	pPlayer->status = MPLAYER_PLAY;
	Fwl_Print(C4, M_DMX, "Start Demuxer Timer 0x%x", pPlayer->pDmx->timer);

	return AK_TRUE;
}

static T_BOOL DmxMgr_Stop(T_pMT_MPLAYER hPlayer)
{
	AK_ASSERT_PTR(hPlayer, "DEMUX:	hPlayer Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(hPlayer->pDmx, "DEMUX:	hPlayer->pDmx Is Invalid", AK_FALSE);
	
	DmxMgr_StopTimer(hPlayer);
	
	if (!MediaLib_Dmx_Stop(hPlayer->pDmx->hMedia))
		Fwl_Print(C2, M_DMX, "Stop DMX Failure");
		
	if (hPlayer->pDmx->dmxInfo.m_bHasVideo)
	{
		T_SYS_MAILBOX mailbox;
	
		mailbox.event = EVT_VIDEO_STOP;
		IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_VIDEO, &mailbox);
		AK_Sleep(2);
	}

	if (hPlayer->pDmx->dmxInfo.m_bHasAudio)
	{
		T_SYS_MAILBOX mailbox;
	
		mailbox.event = EVT_AUDIO_STOP;
		IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);
		AK_Sleep(2);
	}

	hPlayer->status = MPLAYER_STOP;
	hPlayer->pDmx->curTime = 0;
	
	Fwl_Print(C3, M_DMX, "MPlayer Stoped");

	return AK_TRUE;
}

static T_S32 DmxMgr_Start(T_pMT_MPLAYER pPlayer, T_S32 startPos)
{
	AK_ASSERT_PTR(pPlayer->pDmx, "DEMUX:	pDmx Is Invalid ", -1);	
	AK_ASSERT_PTR(pPlayer->pDmx->hMedia, "DEMUX:	hMedia Is Invalid ", -1);
	
	if (startPos >= (T_S32)MPlayer_GetTotalTime()
		|| startPos < 0)
	{
		Fwl_Print(C3, M_DMX, "Seek Position Overflow, Reset to Zero");
		startPos = 0;
	}	

	pPlayer->status = MPLAYER_SEEKING;
	
	if(0 > (startPos = MediaLib_Dmx_Start(pPlayer->pDmx->hMedia, startPos)))
	{
		Fwl_Print(C2, M_DMX, "MediaLib Start Failure");
		
		pPlayer->status = MPLAYER_ERR;
		
		return MPLAYER_ERROR;
	}	

	pPlayer->status = MPLAYER_SEEKED;
	
	pPlayer->pDmx->curTime 		= startPos;
	pPlayer->pDmx->startTime 	= startPos;
	
	if (pPlayer->pDmx->dmxInfo.m_bHasAudio)
	{
		T_U32 num = 0;	
		T_SYS_MAILBOX mailbox;		

		mailbox.event = EVT_AUDIO_START;
		mailbox.param.w.Param1 = startPos;
	
		IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);

		do
		{
			AK_Sleep(5);
			
		}while (MPLAYER_SEEKED == pPlayer->status 
			&& ++num < MAX_WAIT_MSG_NUM);
		
		if (MPLAYER_ERR == pPlayer->status)
		{
			return MPLAYER_ERROR;
		}

		if (num >= MAX_WAIT_MSG_NUM)
		{
			Fwl_Print(C2, M_DMX, "Wave Out NOT Opened, NUM: %d\n", num);
		}		
	}
		
	if (!CThread_StartTimer(&pPlayer->pDmx->timer, DMX_SCAN_INTERVAL, DriveEvtCB_Demux))
	{
		Fwl_Print(C2, M_DMX, "Set Demuxer Timer Failure");

		MediaLib_Dmx_Stop(pPlayer->pDmx->hMedia);
		
		pPlayer->status = MPLAYER_ERR;
		
		return MPLAYER_ERROR;
	}

	if (!pPlayer->pDmx->dmxInfo.m_bHasAudio)
		pPlayer->pDmx->zeroTime = get_tick_count() - startPos;
	
	pPlayer->status = MPLAYER_PLAY;

	Fwl_Print(C2, M_DMX, "Start Play Position %d, tick: %d\n\n", startPos, get_tick_count());
	return startPos;
}

static T_S32 DmxMgr_FastPlay(T_pMT_MPLAYER pPlayer, T_S32 startPos, T_U8 direct)
{
	AK_ASSERT_PTR(pPlayer->pDmx, "DEMUX:	pDmx Is Invalid ", -1);	
	AK_ASSERT_PTR(pPlayer->pDmx->hMedia, "DEMUX:	hMedia Is Invalid ", -1);
	
	if (!pPlayer->pDmx->dmxInfo.m_bHasVideo)
	{
		Fwl_Print(C3, M_DMX, "No Video, Can NOT Fast Play");

		pPlayer->status = MPLAYER_ERR;
		
		return MPLAYER_ERROR;
	}
	
	if (startPos >= (T_S32)MPlayer_GetTotalTime())
	{
		Fwl_Print(C3, M_DMX, "Seek Position Overflow, Reset to Zero");
		startPos = 0;
	}	

	pPlayer->status = MPLAYER_SEEKING;

	if (!MediaLib_Dmx_FF_FR(pPlayer->pDmx->hMedia, startPos, direct))
		Fwl_Print(C2, M_DMX, "Set Fast Play Failure\n\n");
	
	// Clear Video Decoder Buffer
	VideoStream_Reset(pPlayer->pVideo->hVS);
	pPlayer->status = MPLAYER_SEEKED;
	
	pPlayer->pDmx->curTime 		= startPos;
	pPlayer->pDmx->startTime 	= startPos;
	
	DriveEvtCB_Demux(0, 0);
	if (!CThread_StartTimer(&pPlayer->pDmx->timer, DMX_SCAN_INTERVAL, DriveEvtCB_Demux))
	{
		Fwl_Print(C2, M_DMX, "Set Demuxer Timer Failure");

		MediaLib_Dmx_Stop(pPlayer->pDmx->hMedia);
		
		pPlayer->status = MPLAYER_ERR;
		
		return MPLAYER_ERROR;
	}

	if (!pPlayer->pDmx->dmxInfo.m_bHasAudio)
		pPlayer->pDmx->zeroTime = get_tick_count() - startPos;
	
	pPlayer->status = MPLAYER_FAST;

	Fwl_Print(C2, M_DMX, "Start Fast Play Position %d, tick: %d\n\n", startPos, get_tick_count());
	
	return startPos;
}

T_BOOL DmxMgr_Close(T_pMT_MPLAYER pPlayer)
{	
	AK_ASSERT_PTR(pPlayer->pDmx, "DEMUX:	pPlayer->pDmx Is Invalid", AK_FALSE);	

	MpuRefr_Stop();
	
	DmxMgr_StopTimer(pPlayer);

#ifndef MEDIA_FAST_SWITCH
	if (pPlayer->pDmx->dmxInfo.m_bHasVideo
		&& AK_NULL != pPlayer->pVideo)
	{
		DmxMgr_CloseVideo(pPlayer);
	}	
#endif

	if (pPlayer->pDmx->dmxInfo.m_bHasAudio
		&& AK_NULL != pPlayer->pAudio)
	{
		DmxMgr_CloseAudio(pPlayer);
	}

	pPlayer->pDmx 	= Dmx_CloseDemux(pPlayer->pDmx);
	
	pPlayer->status = MPLAYER_CLOSE;
	pPlayer->endCB 	= AK_NULL;

	if (ERROR_TIMER == pPlayer->tmExit)
	{
		pPlayer->tmExit = vtimer_start(1200, AK_FALSE, DriveEvtCB_DelayExit);
	}
	
	return AK_TRUE;
}

T_BOOL DmxMgr_DecodeHeader(T_hFILE hFile, T_pMT_MPLAYER hPlayer, T_BOOL isFile)
{
	T_pDEMUXER pDmx;
	// Open Demuxer And Get Information
	if (!Dmx_Init(&pDmx, hFile, isFile)
		|| AK_NULL == (pDmx->hMedia = Dmx_Open(&pDmx->dmxInfo, hFile, isFile, MEDIALIB_MEDIA_UNKNOWN, AK_TRUE)))
	{	
		Fwl_Print(C2, M_DMX, "Open Demuxer Failure");
		hPlayer->pDmx = pDmx;
		return AK_FALSE;
	}
	
	hPlayer->pDmx = pDmx;

#if CI37XX_PLATFORM	
	if (pDmx->dmxInfo.m_bHasVideo
		&& VIDEO_DRV_FLV263 != pDmx->dmxInfo.m_VideoDrvType
		&& VIDEO_DRV_MPEG != pDmx->dmxInfo.m_VideoDrvType
		&& VIDEO_DRV_H263 != pDmx->dmxInfo.m_VideoDrvType
		&& VIDEO_DRV_MJPEG != pDmx->dmxInfo.m_VideoDrvType)
	{
		pDmx->dmxInfo.m_bHasVideo = AK_FALSE;
		
		Fwl_Print(C3, M_DMX, "Unsupported Video Type");		
	}
#endif

	// Open Video Decoder
	if (pDmx->dmxInfo.m_bHasVideo
		&& !Vs_OpenVideoDecoder(hPlayer))
	{
		Fwl_Print(C2, M_DMX, "Open Video Decoder Failure");
		
		if (!pDmx->dmxInfo.m_bHasAudio)
		{
			return AK_FALSE;
		}
		
		pDmx->dmxInfo.m_bHasVideo = AK_FALSE;
		Fwl_Print(C3, M_DMX, "Play Audio ONLY");
	}

	// Open Audio Decoder
	if (pDmx->dmxInfo.m_bHasAudio
		&& !Sd_OpenAudioPlayer(hPlayer))
	{
		Fwl_Print(C2, M_DMX, "Open Audio Decode Failure");

		if (!pDmx->dmxInfo.m_bHasVideo)
		{
			return AK_FALSE;
		}
		
		pDmx->dmxInfo.m_bHasAudio = AK_FALSE;
		Fwl_Print(C3, M_DMX, "Play Video ONLY");		
	}
	
	if (!pDmx->dmxInfo.m_bHasAudio 
		&& !pDmx->dmxInfo.m_bHasVideo)
	{
		Fwl_Print(C3, M_DMX, "Neither Audio Nor Video");
	
		return AK_FALSE;
	}
	
	return AK_TRUE;
}

T_VOID DmxMgr_HandleDemux(T_pMT_MPLAYER pPlayer, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
	T_pDEMUXER pDmx;
	T_pAUDIO_DEC pAudio;
	T_pVIDEO_DEC pVideo;
	T_pVOID hMedia;
	
	pDmx 	= pPlayer->pDmx;
	pAudio 	= pPlayer->pAudio;
	pVideo 	= pPlayer->pVideo;

	if (!pDmx || !pDmx->hMedia )
		return;
	
	hMedia 	= pDmx->hMedia;

	switch(eEvent)
	{
	case EVT_DMX_SCAN:
		if (MEDIALIB_DMX_PLAY == MediaLib_Dmx_GetStatus(hMedia)
			|| MEDIALIB_DMX_FF == MediaLib_Dmx_GetStatus(hMedia)
			|| MEDIALIB_DMX_FR == MediaLib_Dmx_GetStatus(hMedia))
		{
			// AK_DEBUG_OUTPUT("DMX");
			Dmx_DemuxData(pDmx, pAudio, pVideo);
		}

		if((!pDmx->dmxInfo.m_bHasAudio && MediaLib_Dmx_CheckVideoEnd(hMedia))
			|| (!pDmx->dmxInfo.m_bHasVideo && MediaLib_Dmx_CheckAudioEnd(hMedia))
			|| (MediaLib_Dmx_CheckVideoEnd(hMedia) && MediaLib_Dmx_CheckAudioEnd(hMedia)))
		{
			Fwl_Print(C3, M_DMX, "Stop DMX Timer: 0x%X", pDmx->timer);
			CThread_StopTimer(&pDmx->timer);
			Fwl_Print(C4, M_DMX, "DMX Timer: 0x%XStop", pDmx->timer);
		}

		break;
		
	case EVT_DMX_CLOSE:
		Fwl_Print(C3, M_DMX, "EVT_DMX_CLOSE");
		DmxMgr_Close(pPlayer);
		break;
		
	case EVT_DMX_PAUSE:
		Fwl_Print(C3, M_DMX, "EVT_DMX_PAUSE");
		DmxMgr_Pause(pPlayer);
		break;

	case EVT_DMX_RESUME:
		Fwl_Print(C3, M_DMX, "EVT_DMX_RESUME");
		DmxMgr_Resume(pPlayer);
		break;

	case EVT_DMX_STOP:
		Fwl_Print(C3, M_DMX, "EVT_DMX_STOP");
		DmxMgr_Stop(pPlayer);
		break;	
		
	case EVT_DMX_START:
		Fwl_Print(C3, M_DMX, "EVT_DMX_START");		
		
		if (0 > DmxMgr_Start(pPlayer, pEvtParam->w.Param1))
		{
			AK_ASSERT_PTR_VOID((T_pVOID)(pPlayer->endCB), "DEMUX:	pPlayer->endCB Is Invalid");
			pPlayer->endCB(T_END_TYPE_NORMAL);
		}		
		break;

	case EVT_DMX_FF:
		Fwl_Print(C3, M_DMX, "EVT_DMX_FF");
		DmxMgr_FastPlay(pPlayer, pEvtParam->w.Param1, MEDIALIB_DMX_FF);
		break;	

	case EVT_DMX_FR:
		Fwl_Print(C3, M_DMX, "EVT_DMX_FR");
		DmxMgr_FastPlay(pPlayer, pEvtParam->w.Param1, MEDIALIB_DMX_FR);
		break;

	default:
		break;
	}
}

T_VOID DmxMgr_HandleEvt(T_EVT_CODE event, T_U32 param)
{
	T_SYS_MAILBOX mailbox;       
	
	switch (event)
	{
	case EVT_PLAY_PAUSE:
		Fwl_Print(C3, M_MPLAY, "EVT_PLAY_PAUSE");
		mailbox.event = EVT_DMX_PAUSE;
		break;
		
	case EVT_PLAY_RESUME:
		Fwl_Print(C3, M_MPLAY, "EVT_PLAY_RESUME");
		mailbox.event = EVT_DMX_RESUME;
		break;
			
	case EVT_PLAY_STOP:
		Fwl_Print(C3, M_MPLAY, "EVT_PLAY_STOP");
		mailbox.event = EVT_DMX_STOP;
		break;
		
	case EVT_PLAY_START:
		Fwl_Print(C3, M_MPLAY, "EVT_PLAY_START");
		mailbox.event = EVT_DMX_START;
		break;

	case EVT_PLAY_FF:
		Fwl_Print(C3, M_MPLAY, "EVT_PLAY_FF");
		mailbox.event = EVT_DMX_FF;
		break;

	case EVT_PLAY_FR:
		Fwl_Print(C3, M_MPLAY, "EVT_PLAY_FR");
		mailbox.event = EVT_DMX_FR;
		break;

	case EVT_PLAY_CLOSE:
		Fwl_Print(C3, M_MPLAY, "EVT_PLAY_CLOSE tick: %d\n\n", get_tick_count());
		mailbox.event = EVT_DMX_CLOSE;
		break;
			
	default:
		return;
		break;
	}
	
	mailbox.param.w.Param1 = param;
	IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_MEDIA, &mailbox);
}


/* 
 * End of File
 */
