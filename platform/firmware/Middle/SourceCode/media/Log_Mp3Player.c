/**
 * @file Log_Mp3Player.c
 * @brief Audio Decoder Logic Implemetation for Multi-thread
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author He_yuanlong
 * @date 2011-3-17
 * @version 1.0
 */

#include "Anyka_types.h"
#include "Gbl_Global.h"
#include "AkAppMgr.h"
#include "Fwl_WaveOut.h"
#include "Fwl_wave.h"
#include "Fwl_waveout.h"
#include "Fwl_sysevent.h"
#include "Fwl_pfAudio.h"

#include "Lib_Media_global.h"
#include "Lib_SdCodec.h"
#include "Eng_debug.h"
#include "Fwl_osMalloc.h"
#include "akos_api.h"
#include "Fwl_vme.h"
#include "Lib_Event.h"
#include "fwl_osfs.h"
#include "akError.h"
#include "Arch_mmu.h"
#include "Hal_timer.h"
#include "File.h"

#include "Log_MediaPlayer.h"
#include "Log_MediaStruct.h"
#include "Log_MediaAudio.h" 
#include "Log_Mediadmx.h"
#include "media_player_lib.h"
#include "Log_Mp3Player.h"

#define TASK_STACK_SIZE				(20*1024)

#define AUDIO_DEC_BUF_SIZE 		0x00004000	// 16K
#define DA_BUFFER_NUM			10			// 10
extern T_GLOBAL_S  gs;

static T_pMT_MP3PLAYER s_Mp3Player = AK_NULL;

/***********************MP3 DECODE*********************/
static T_BOOL Sd_Mp3SetDecInput(T_AUDIO_DECODE_INPUT *input)
{ 
	AK_ASSERT_PTR(input, "Input Parameter Is Invalid", AK_FALSE);
	
	Sd_SetCodecCB(&input->cb_fun);
	memset(&input->m_info, 0, sizeof(T_AUDIO_IN_INFO));
	
	input->m_info.m_Type 		= _SD_MEDIA_TYPE_MP3;
	input->m_info.m_SampleRate 	= 32000;
	input->m_info.m_Channels 		= 1;
	input->m_info.m_BitsPerSample 	= 16;
	return AK_TRUE;
}

static T_VOID Sd_Mp3SetAudInfo(T_MEDIALIB_AUDIO_INFO *pAudioInfo)
{
	pAudioInfo->m_SampleRate 	= 32000;	
	pAudioInfo->m_Channels 		= 2;
	pAudioInfo->m_BitsPerSample = 16;
	pAudioInfo->m_Type 			= _SD_MEDIA_TYPE_MP3;
	pAudioInfo->m_BitRate 		= 128000;
}

static T_VOID Sd_Mp3SeekPos(T_pMT_MP3PLAYER pPlayer, T_U32 startPos)
{
	T_AUDIO_SEEK_INFO seekInfo;

	AK_ASSERT_PTR_VOID(pPlayer, "Parameter pPlayer Is Invalid");
	AK_ASSERT_PTR_VOID(pPlayer->pMp3Aud, "Parameter pPlayer->pAudio Is Invalid");
	
	memset(&seekInfo, 0, sizeof(T_AUDIO_SEEK_INFO));
	seekInfo.real_time = startPos;
	
	_SD_Decode_Seek(pPlayer->pMp3Aud->hMP3, &seekInfo);
}

T_BOOL Sd_Mp3InitDecoder(T_pMP3AUD_DEC* ppMp3Aud)
{
	AK_ASSERT_PTR(ppMp3Aud, "ppMp3Aud Parameter ERROR", AK_FALSE);
	
	*ppMp3Aud= (T_pMP3AUD_DEC)Fwl_Malloc(sizeof(T_MP3AUD_DEC));	
	AK_ASSERT_PTR(*ppMp3Aud, "player->pMp3Aud Malloc FAILURE ", AK_FALSE);	
	memset(*ppMp3Aud, 0, sizeof(T_MP3AUD_DEC));
	
	(*ppMp3Aud)->timer = ERROR_TIMER;
	Sd_Mp3SetAudInfo(&(*ppMp3Aud)->audioInfo);
	
	return AK_TRUE;
}

T_BOOL Sd_Mp3OpenDecoder(T_pMT_MP3PLAYER pPlayer)
{
	T_AUDIO_DECODE_INPUT input;	
	
	AK_ASSERT_PTR(pPlayer, "Input Parameter Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(pPlayer->pMp3Aud, "pAudio Is Invalid ", AK_FALSE); 

	memset(&input, 0, sizeof(T_AUDIO_DECODE_INPUT));
	
	// Set Audio Decoder Input Infomation
	if (!Sd_Mp3SetDecInput(&input))
	{
		AK_DEBUG_OUTPUT("MT_MPlayer:	Set Audio Decoder Input Information Failure.\n");
		
		return AK_FALSE;
	}

	// Open Audio Decoder
	pPlayer->pMp3Aud->hMP3 = _SD_Decode_Open(&input, &pPlayer->pMp3Aud->decOut);
	if(!pPlayer->pMp3Aud->hMP3)
	{
		AK_DEBUG_OUTPUT("MT_MPlayer:	Audio Thread Open SD CODEC Failure.\n");
		
		return AK_FALSE;
	}
	_SD_SetBufferMode(pPlayer->pMp3Aud->hMP3, _SD_BM_LIVE);	// _SD_BM_LIVE _SD_BM_NORMAL
		
	// Malloc Audio Decoder Output Buffer
	pPlayer->pMp3Aud->decOut.m_ulSize = AUDIO_DEC_BUF_SIZE;
	pPlayer->pMp3Aud->decOut.m_pBuffer = (T_U8*)Fwl_Malloc(pPlayer->pMp3Aud->decOut.m_ulSize);	
	if(!pPlayer->pMp3Aud->decOut.m_pBuffer)
	{
		AK_DEBUG_OUTPUT("MT_MPlayer:	Malloc SD Decode Output Buffer Failure.\n ");
		
		_SD_Decode_Close(pPlayer->pMp3Aud->hMP3);
		
		return AK_FALSE;		
	}
	
	AK_DEBUG_OUTPUT("_SD_Decode_Open sucess~\n");
	return AK_TRUE;
}

T_U32 Mp3_GetAudio2Decoder(T_hFILE hFile, T_pVOID hMP3)
{
	T_AUDIO_BUFFER_CONTROL bufInfo;
	T_U32 packLen;
	T_U16 sizeOfBuf  = sizeof(T_AUDIO_BUFFER_CONTROL);
	T_U8 ret;
	T_MEDIALIB_BUFFER *fname = (T_MEDIALIB_BUFFER *)hFile;
	
	AK_ASSERT_PTR(hMP3, "MT_MPlayer: hSD Is Invalid", 0);	
	AK_DEBUG_OUTPUT("Mp3_GetAudio2Decoder buflen%d, bufpos%d, buf%d\n",
					fname->bufLen, fname->bufPos, fname->pBuf);
	
	packLen = fname->bufLen;
		
	memset(&bufInfo, 0, sizeOfBuf);
	ret = _SD_Buffer_Check(hMP3, &bufInfo);	// Buffer Length: 128K
	
	if (packLen <= bufInfo.free_len)		
	{
		memcpy(bufInfo.pwrite, fname->pBuf, packLen);
		_SD_Buffer_Update(hMP3, packLen);
	}
	else if (packLen <= bufInfo.free_len + bufInfo.start_len)
	{
	//	 Data Length > Tail Space
		memcpy(bufInfo.pwrite, fname->pBuf, bufInfo.free_len);
		memcpy(bufInfo.pstart, fname->pBuf + bufInfo.free_len, packLen - bufInfo.free_len);		
		_SD_Buffer_Update(hMP3, packLen);
	}
	else
	{
		return AK_FALSE;
	}
	_SD_SetBufferMode(hMP3, _SD_BM_LIVE); // _SD_BM_LIVE _SD_BM_NORMAL	

	return packLen;
}

T_BOOL Sd_Mp3DecAppointFrame(T_pMT_MP3PLAYER pPlayer, T_U32 startPos, T_hFILE hFile)
{
	T_U16 	decNum = 0;
	T_BOOL 	retVal = AK_FALSE;
	
	AK_ASSERT_PTR(pPlayer, "Parameter pPlayer Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(pPlayer->pMp3Aud, "Parameter pPlayer->pAudio Is Invalid", AK_FALSE);
	
	Sd_Mp3SeekPos(pPlayer, startPos);

	//Mp3 copy audio data to decode`s buffer
	if (0 < Mp3_GetAudio2Decoder(hFile, pPlayer->pMp3Aud->hMP3))
	{
		while(NORM_DEC_FAIL_NUM > ++decNum
			&& 0 >= _SD_Decode(pPlayer->pMp3Aud->hMP3, &pPlayer->pMp3Aud->decOut));
	
		if (NORM_DEC_FAIL_NUM <= decNum)
		{
			retVal = AK_FALSE;
		}
		else
		{
			retVal = AK_TRUE;
		}

		pPlayer->pMp3Aud->decOut.m_ulDecDataSize = 0;
		_SD_Buffer_Clear(pPlayer->pMp3Aud->hMP3);	
	}
	else
	{
		retVal = AK_FALSE;
	}
	
	return retVal;
}

T_pVOID Sd_Mp3Close(T_pMP3AUD_DEC pMp3Aud)
{
	AK_ASSERT_PTR(pMp3Aud, "pAudio Is Invalid ", AK_NULL);

	_SD_Decode_Close(pMp3Aud->hMP3);

	pMp3Aud->decOut.m_pBuffer = Fwl_FreeTrace(pMp3Aud->decOut.m_pBuffer);
	
	return Fwl_FreeTrace(pMp3Aud);
}

T_BOOL Sd_Mp3PreDecode(T_hFILE hFile, T_pMT_MP3PLAYER player)
{
	T_PCM_FORMAT fmt;
		
	AK_ASSERT_PTR(player, "Input Parameter Is Invalid", AK_FALSE);	
	AK_ASSERT_PTR(player->pMp3Aud, "player->pAudio Is Invalid", AK_FALSE);
	
#if 0		
	SdFilter_Init(&player->pAudio->pSdFilt);	
	SdFilter_Open(player->pAudio->pSdFilt, &player->pAudio->audioInfo);
#endif

	if (!Sd_Mp3DecAppointFrame(player, 0, hFile))
	{
		AK_DEBUG_OUTPUT("MtMP:	Audio Predecode Failure\n");
	
		return AK_FALSE;
	}
	
	AK_DEBUG_OUTPUT("Decode frame Channels %d, BitsPerSample %d, SampleRate %d\n",
					player->pMp3Aud->decOut.m_Channels,
					player->pMp3Aud->decOut.m_BitsPerSample,
					player->pMp3Aud->decOut.m_SampleRate);
	
	player->pMp3Aud->decNum = (T_U8)((player->pMp3Aud->decOut.m_SampleRate/6) >> 6);

	WaveOut_GetStatus(&fmt, WAVEOUT_FORMAT);

	if (fmt.channel 	!= player->pMp3Aud->decOut.m_Channels
		|| fmt.sampleBits 	!= player->pMp3Aud->decOut.m_BitsPerSample
		|| fmt.sampleRate 	!= player->pMp3Aud->decOut.m_SampleRate)
	{
		WaveOut_Close();
		AK_DEBUG_OUTPUT("clean DA Buffer!\n");
	}

	return AK_TRUE;
}

T_VOID Sd_MP3Start(T_pMT_MP3PLAYER pPlayer, T_U32 startPos)
{	
	Sd_Mp3SeekPos(pPlayer, startPos);

	if (!Sd_OpenWaveOut(&pPlayer->pMp3Aud->decOut, startPos))
	{
		AK_DEBUG_OUTPUT("MtMP:		Open Wave Out Failure.\n");
		
		pPlayer->status = MPLAYER_ERR;
		return;
	}

	pPlayer->status = MPLAYER_WAVEOUT;
	AK_DEBUG_OUTPUT("MtMP:		Open Wave Out OK.\n");
	
}

T_VOID Sd_MP3Stop(T_pMP3AUD_DEC pAudio)
{
	if (AK_NULL == pAudio 
		|| AK_NULL == pAudio->hMP3
		|| !_SD_Buffer_Clear(pAudio->hMP3))
	{
		AK_DEBUG_OUTPUT("Mp3_AudioBG:	Clear Mp3SD Buffer Failure.\n");				
	}			
	
	AK_DEBUG_OUTPUT("Clear SD Buffer\n");
	// WaveOut_Close();
}
 
static T_VOID Sd_Mp3Output(T_pMP3AUD_DEC pAudio)
{
	T_S32 dataLen 	= (T_S32)pAudio->decOut.m_ulDecDataSize; // 64

	do
	{
		dataLen -= WaveOut_Write(pAudio->decOut.m_pBuffer + pAudio->decOut.m_ulDecDataSize - dataLen,
								dataLen, INVALID_TIME_STAMP);
		if (dataLen > 0)
			AK_Sleep(5);
		
	}while(dataLen > 0);
}

T_VOID Sd_Mp3HandleAudDec(T_pMT_MP3PLAYER pPlayer)
{	
	T_pMP3AUD_DEC pAudio	= pPlayer->pMp3Aud;
	T_S32 		retval 	= 1;
	T_U32 		freeSpace;
	T_U8		decNum = 0;
	static T_U8 failNum = 0;

	while (WaveOut_GetStatus(&freeSpace, WAVEOUT_SPACE_SIZE) && freeSpace		// wait for DA
		&& decNum++ < pAudio->decNum
		&& 0 <= (retval = _SD_Decode(pAudio->hMP3, &pAudio->decOut)) )         // 128 Bytes per time
	{
		if (retval > 0)
		{
			Sd_Mp3Output(pAudio);
			failNum = 0;
		}
	}

	if (0 > retval || ++failNum > NORM_DEC_FAIL_NUM)
	{	
		AK_DEBUG_OUTPUT("MtMP:	NOT More Audio Data to Decode, Exit Audio Decoder Thread.\n");

		CThread_StopTimer(&pAudio->timer);
		
		if((ERROR_TIMER == pPlayer->pMp3Aud->timer))
		AK_DEBUG_OUTPUT("MtMP:	Close Player By Audio Decoder End.\n");		
	}		
}

/************************MP3 PLAY*********************/
T_BOOL MP3Player_Init(T_VOID)
{
	if (AK_NULL != s_Mp3Player)
	{
		MP3Player_Close();
		s_Mp3Player = Fwl_FreeTrace(s_Mp3Player);
	}		
	
	s_Mp3Player = (T_pMT_MP3PLAYER)Fwl_Malloc(sizeof(T_MT_MP3PLAYER));
	AK_ASSERT_PTR(s_Mp3Player, "s_hPlayer Malloc Failure ", AK_FALSE);	
	memset(s_Mp3Player, 0, sizeof(T_MT_MP3PLAYER));
	
	s_Mp3Player->init = AK_TRUE;
	s_Mp3Player->decOpen = AK_FALSE;
	s_Mp3Player->hFile = AK_FALSE;

	return AK_TRUE;
}

static T_BOOL MP3_CreateAudioThread(T_VOID)
{ 
	IThread_Run(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIO));
	
	return AK_TRUE;
}

static T_VOID DriveEvtCB_Mp3Audio(T_TIMER timer_id, T_U32 delay)
{
	T_SYS_MAILBOX mailbox; 

	//Audio Decoder start decode mp3 data and waveOutwrite 
    mailbox.event = EVT_AUDIO_MP3SCAN;
	IAppMgr_PostUniqueEvt(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);
}

T_pVOID MP3Player_GetPlayer(T_VOID)
{
	return s_Mp3Player;
}

T_BOOL MP3Player_OpenDec(T_VOID)
{
	AK_ASSERT_PTR(s_Mp3Player, "Input Parameter Is Invalid", AK_FALSE);
	
	if(!s_Mp3Player->decOpen)
	{
		if(!Sd_Mp3InitDecoder(&s_Mp3Player->pMp3Aud))
			return AK_FALSE;

		if (!Sd_Mp3OpenDecoder(s_Mp3Player))
		{
			AK_DEBUG_OUTPUT("MT_MPlayer:	Open Audio Decoder Failure.\n");			
			s_Mp3Player->pMp3Aud = Fwl_FreeTrace(s_Mp3Player->pMp3Aud);	
			return AK_FALSE;
		}
		
		AK_DEBUG_OUTPUT("Open Decode Channels %d, BitsPerSample %d, SampleRate %d\n",
						s_Mp3Player->pMp3Aud->decOut.m_Channels,
						s_Mp3Player->pMp3Aud->decOut.m_BitsPerSample,
						s_Mp3Player->pMp3Aud->decOut.m_SampleRate);
		s_Mp3Player->decOpen = AK_TRUE;
	}
	
	if (s_Mp3Player->pMp3Aud && !MP3_CreateAudioThread())
	{		
		AK_DEBUG_OUTPUT("MtMP:	Mp3 Create Audio Thread Failure.\n");			
		return AK_FALSE;		
	}	
	
	Fwl_AudioEnableDA();
	return AK_TRUE;
}

T_BOOL MP3MPlayer_Open(T_pVOID src)
{
	if (!s_Mp3Player->init)
	{
		AK_DEBUG_OUTPUT("Media Lib NOT Initialized.\n");

		return AK_FALSE;
	}	

	if(s_Mp3Player->hFile)
	{
		s_Mp3Player->hFile = (T_hFILE)Fwl_FreeTrace((T_pVOID)s_Mp3Player->hFile);
	}
	
	AK_DEBUG_OUTPUT("MtMP:	Media Is A Buffer Type.\n");
	s_Mp3Player->hFile = (T_hFILE)src;

#ifdef OS_ANYKA
	// Open Audio PreDecoder	
	if(!Sd_Mp3PreDecode(s_Mp3Player->hFile, s_Mp3Player))
	{
		AK_DEBUG_OUTPUT("MtMP:	Open Mp3Audio Decode Failure.\n");

		s_Mp3Player->hFile = (T_hFILE)Fwl_FreeTrace((T_pVOID)s_Mp3Player->hFile);

		return AK_FALSE;
	}

	s_Mp3Player->status = MPLAYER_OPEN;	
	return AK_TRUE;
#else
	return AK_FALSE;
#endif

}

static T_BOOL MP3_Mgr_Stop(T_pMT_MP3PLAYER hPlayer)
{	
	T_SYS_MAILBOX mailbox;
	
	AK_ASSERT_PTR(hPlayer, "hPlayer Is Invalid", AK_FALSE);

	if (hPlayer->pMp3Aud)
	{
		AK_DEBUG_OUTPUT("Stop Audio Timer: 0x%x ... ...\n", hPlayer->pMp3Aud->timer);	
		CThread_StopTimer(&hPlayer->pMp3Aud->timer);
#ifdef OS_ANYKA
		AK_Reset_Queue(IThread_GetQueue(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIO)));
#endif
	}

	mailbox.event = EVT_AUDIO_MP3STOP;
	IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);
	AK_Sleep(10);

	hPlayer->status = MPLAYER_STOP;
	
	AK_DEBUG_OUTPUT("MTMP:	Mp3Player Stoped.\n");

	return AK_TRUE;
}

T_VOID MP3_Mgr_CloseAudio(T_pMT_MP3PLAYER pPlayer)
{
	T_U8 i = 0;
	T_SYS_MAILBOX mailbox;

	AK_ASSERT_PTR_VOID(pPlayer, "pPlayer Is Invalid");
	AK_ASSERT_PTR_VOID(pPlayer->pMp3Aud, "pPlayer->pAudio Is Invalid");
			
	mailbox.event = EVT_AUDIO_MP3CLOSE;
	
	do
	{
		IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);
		AK_Sleep(5);
	}while(pPlayer->pMp3Aud && ++i < MAX_WAIT_MSG_NUM);
	
#if 1

	IThread_Suspend(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIO));
#else
	if (i >= MAX_WAIT_MSG_NUM)
	{
		AK_DEBUG_OUTPUT("DMX: Sent MSG to Close Audio Thread Failure, Close Directly Audio Thread.\n");
		IAppMgr_DeleteEntry(AK_GetAppMgr(), AKAPP_CLSID_AUDIO);
		AK_Sleep(2);
	}		
#endif	
	AK_DEBUG_OUTPUT("MtMP:	Mp3 Deleted Audio Thread %d.\n", i);
}

T_BOOL MP3Mgr_Close(T_pMT_MP3PLAYER pPlayer)
{	
	MP3_Mgr_Stop(pPlayer);
	
	if (AK_NULL != pPlayer->pMp3Aud)
	{
		MP3_Mgr_CloseAudio(pPlayer);
	}

	if(pPlayer->hFile)
	{
		pPlayer->hFile = (T_hFILE)Fwl_FreeTrace((T_pVOID)pPlayer->hFile);		
	}
	
	pPlayer->status = MPLAYER_CLOSE;
	pPlayer->decOpen = AK_FALSE;

	AK_DEBUG_OUTPUT("MtMP:	Mp3_Mgr_Close() Exit.\n");	
	return AK_TRUE;
}

T_BOOL MP3Player_HandleAudio(T_U32 position)//T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
	if (MPLAYER_CLOSE != s_Mp3Player->status)
	{
		T_U32 num = 0;	
		T_SYS_MAILBOX mailbox;
		
		mailbox.event = EVT_AUDIO_MP3START;
		mailbox.param.w.Param1 = position;
	
		IAppMgr_PostUniqueEvt2Head(AK_GetAppMgr(), AKAPP_CLSID_AUDIO, &mailbox);
		
 		do
		{
			AK_Sleep(10);		
			
		}while(MPLAYER_ERR != s_Mp3Player->status 
			&& MPLAYER_WAVEOUT != s_Mp3Player->status 
			&& ++num < MAX_WAIT_MSG_NUM);

		AK_DEBUG_OUTPUT("waveOpen num:%d\n",num);

		if (MPLAYER_ERR == s_Mp3Player->status)
		{			
			AK_DEBUG_OUTPUT("waveout open err~!\n");
			return AK_FALSE;
		}
	}

	if(s_Mp3Player->hFile)
	{
		//write mp3 data to decode buffer
		Mp3_GetAudio2Decoder(s_Mp3Player->hFile, s_Mp3Player->pMp3Aud->hMP3);
	}
	else
	{
		AK_DEBUG_OUTPUT("MP3_Player:	Mp3 buf is NULL!~\n");
		return AK_FALSE;
	}
	
	if (ERROR_TIMER == s_Mp3Player->pMp3Aud->timer)
	{
		if (!CThread_StartTimer(&s_Mp3Player->pMp3Aud->timer, AUDIO_SCAN_INTERVAL, DriveEvtCB_Mp3Audio))
		{
			AK_DEBUG_OUTPUT("\nMT_MPlayer:	Start Mp3 audio Timer Failure.\n");
			return AK_FALSE;
		}
		else 
			AK_DEBUG_OUTPUT("\nStart Mp3 audio Timer 0x%x.\n", s_Mp3Player->pMp3Aud->timer);
	}
	
	s_Mp3Player->status = MPLAYER_PLAY;
	
	AK_DEBUG_OUTPUT("MtMP:  Mp3 Start Play Position %d.\n", position);
	return AK_TRUE;
}

T_BOOL MP3Player_Play(T_U32 position)
{
	T_U8 i = 0;	
		
	AK_ASSERT_PTR(s_Mp3Player, "s_hPlayer Is Invalid ", AK_FALSE); 	

	if(!MP3Player_HandleAudio(position))
	{
		AK_DEBUG_OUTPUT("MP3Player:	 Start Playing Mp3 is Fail!\n");
					
		return AK_FALSE;
	}

	AK_DEBUG_OUTPUT("MtMP:	Mp3 Player Is Playing %d.\n", i);

	return AK_TRUE;
}

T_BOOL MP3Player_Close(T_VOID)
{
	T_U8 i = 0;	
	T_BOOL threadReset = AK_FALSE;
	
	do{
		if (++i >= MAX_WAIT_MSG_NUM )
		{		
			AK_DEBUG_OUTPUT("MtMP:	Mp3 Normal Close Failure, Reset Media Thread.\n");

			// Maybe Dead Lock When FS NOT Return
			IThread_Terminate(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA)); 
			IThread_Run(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MEDIA));
			
			threadReset = AK_TRUE;
			AK_Sleep(20);
		}
		
		MP3Mgr_Close(s_Mp3Player);
		AK_Sleep(20);
		
	} while (MPLAYER_CLOSE != s_Mp3Player->status && !threadReset );
	
	AK_DEBUG_OUTPUT("MtMP:	Mp3Player Close %d.\n", i);	
	return AK_TRUE;
}


/*
 * End of File
 */
