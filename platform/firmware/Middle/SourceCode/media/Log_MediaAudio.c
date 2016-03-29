/**
 * @file Log_MediaAudio.c
 * @brief Audio Decoder Logic Implemetation for Multi-thread
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Xie_Wenzhong
 * @date 2011-3-17
 * @version 1.0
 */

#include "Anyka_types.h"
#include "Gbl_Global.h"
#include "AkAppMgr.h"
#include "Fwl_Wave.h"
#include "Fwl_WaveOut.h"
#include "Fwl_pfAudio.h"
#include "Fwl_osfs.h"
#include "Lib_Media_global.h"
#include "Lib_SdCodec.h"
#include "Lib_SdFilter.h"
#include "Media_Demuxer_Lib.h"
#include "Eng_debug.h"
#include "Fwl_osMalloc.h"
#include "akos_api.h"
#include "Fwl_vme.h"
#include "Lib_Event.h"
#include "Log_MediaAudio.h"
#include "Log_MediaDmx.h"


#define AUDIO_DATA_BUFFER_128K 	0x00020000	// 128K
#define AUDIO_DEC_BUF_SIZE 		0x00004000	// 16K

#define DA_BUFFER_NUM			10			// 10
#define FADE_OUT_NUM			500

#define AUDIO_DEC_BUF_MAX		(0xC0<<10)	// 192K
#define AUDIO_DEC_BUF_SIZE_MP3	(0x20<<10)	// 32K
#define AUDIO_DEC_BUF_SIZE_AMR	(0x12<<10)	// 12K

#define LIMIT_APE_SAMPLE		44100
#define LIMIT_WMA_BITRATE		324000

#define FILTER_SIZE_MAX   (64 * 2)

/*audio filter buffer size*/
#define FILTER_BUF_SIZE    (2048)

/*get bit value in char*/
#define CHAR_BIT(c,nbit) (((c)>>(nbit))&0x01)
#define SD_ENABLE_EQ	0x01
#define SD_ENABLE_TEMPO	0x02
#define SD_ENABLE_VOICECHANGE	0x04

extern T_GLOBAL_S  gs;

extern T_S32 Buf_Seek(T_S32 hbuf, T_S32 offset, T_S32 whence);
extern T_S32 Buf_Tell(T_S32 hbuf);

const T_U8 Filter_idx[] = 
{
    0,     	//_SD_FILTER_UNKNOWN
    0x0,   	//_SD_FILTER_EQ
    0x1,    //_SD_FILTER_WSOLA
	0,		//_SD_FILTER_RESAMPLE,
	0,		//_SD_FILTER_3DSOUND,
	0,		//_SD_FILTER_DENOICE,
	0,		//_SD_FILTER_AGC,
	0x2,	//_SD_FILTER_VOICECHANGE,
	0,		//_SD_FILTER_PCMMIXER
};

static T_BOOL Sd_IsDecLimit(const T_MEDIALIB_DMX_INFO* dmxInfo)
{
	AK_ASSERT_PTR(dmxInfo, "dmxInfo Is Invalid ", AK_FALSE);

	if (dmxInfo->m_MediaType == MEDIALIB_MEDIA_APE)
	{
		T_U32 compressionLevel = 0;
	
		// Get Ape compressionLevel from m_szData byte 8~11
		compressionLevel = (((T_U32)dmxInfo->m_szData[8] & 0xFF)
							| (((T_U32)dmxInfo->m_szData[9] & 0xFF) << 8)
							| (((T_U32)dmxInfo->m_szData[10] & 0xFF) << 16)
							| (((T_U32)dmxInfo->m_szData[11] & 0xFF) << 24));

		if(compressionLevel == 5000)
		{
			Fwl_Print(C2, M_DMX, "This APE compression is: c%d, too big!\n",
								compressionLevel);
			return AK_TRUE;
		}
		else if(dmxInfo->m_nSamplesPerSec > LIMIT_APE_SAMPLE)
		{
			Fwl_Print(C2, M_DMX, "This APE SamplesRate is: %d, too big to support!\n",
								dmxInfo->m_nSamplesPerSec);
			return AK_TRUE;
		}
	}

	// wma bitRate limit 320kps
	if (dmxInfo->m_MediaType == MEDIALIB_MEDIA_ASF
		&& dmxInfo->m_ulAudioBitRate > LIMIT_WMA_BITRATE)
	{
		Fwl_Print(C2, M_DMX, "This WMA BitRate over 320kps, too big\n");

		return AK_TRUE;
	}

	return AK_FALSE;
}

static T_BOOL Sd_SetDecInfo(T_AUDIO_IN_INFO *info, T_hFILE hFile,
							const T_MEDIALIB_DMX_INFO *dmxInfo, T_BOOL isFile)
{
	AK_ASSERT_PTR(info, "AudioDEC:	Input Parameter Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(dmxInfo, "AudioDEC:	Input Parameter Is Invalid", AK_FALSE);	

	memset(info, 0, sizeof(T_AUDIO_IN_INFO));
	
	info->m_Type 			= dmxInfo->m_AudioType;
	info->m_SampleRate 		= dmxInfo->m_nSamplesPerSec;
	info->m_Channels 		= dmxInfo->m_nChannels;
	info->m_BitsPerSample 	= dmxInfo->m_wBitsPerSample;
	info->m_szData 			= (T_U8*)dmxInfo->m_szData;
	info->m_szDataLen		= dmxInfo->m_cbSize;

	if (_SD_MEDIA_TYPE_PCM == info->m_Type)
	{
		info->m_InbufLen = AUDIO_DEC_BUF_MAX;
	}
	else if (_SD_MEDIA_TYPE_MP3 == info->m_Type
		|| _SD_MEDIA_TYPE_AAC == info->m_Type
		|| _SD_MEDIA_TYPE_WMA == info->m_Type
		|| _SD_MEDIA_TYPE_AC3 == info->m_Type)		
	{
		info->m_InbufLen = AUDIO_DEC_BUF_SIZE_MP3;
	}
	else if (_SD_MEDIA_TYPE_AMR == info->m_Type)
	{
		info->m_InbufLen = AUDIO_DEC_BUF_SIZE_AMR;
	}

	if (_SD_MEDIA_TYPE_MIDI == info->m_Type)
	{
		if (isFile)
		{
			//filelen
			Fwl_FileSeek(hFile, 0, SEEK_END);
			info->m_Private.m_midi.nFileSize = Fwl_FileTell(hFile);
		}
		else
		{
			Buf_Seek(hFile, 0, SEEK_END);
			info->m_Private.m_midi.nFileSize = Buf_Tell(hFile);
		}
	}

	return AK_TRUE;
}

static T_BOOL Sd_SetDecInput(T_AUDIO_DECODE_INPUT *input, T_hFILE hFile,
							const T_MEDIALIB_DMX_INFO *dmxInfo, T_BOOL isFile)
{
	AK_ASSERT_PTR(input, "AudioDEC:	Input Parameter Is Invalid", AK_FALSE);
	
	Sd_SetCodecCB(&input->cb_fun);
	return Sd_SetDecInfo(&input->m_info, hFile, dmxInfo, isFile);
}


static T_VOID Sd_SetAudioInfo(T_MEDIALIB_AUDIO_INFO *pAudioInfo,
								const T_MEDIALIB_DMX_INFO *pDmxInfo)
{
	pAudioInfo->m_SampleRate 	= pDmxInfo->m_nSamplesPerSec;	
	pAudioInfo->m_Channels 		= pDmxInfo->m_nChannels;
	pAudioInfo->m_BitsPerSample = pDmxInfo->m_wBitsPerSample;
	pAudioInfo->m_Type 			= pDmxInfo->m_AudioType;
	pAudioInfo->m_BitRate 		= pDmxInfo->m_ulAudioBitRate;
}

static T_VOID Sd_SeekPos(T_pMT_MPLAYER pPlayer, T_U32 startPos)
{
	T_AUDIO_SEEK_INFO seekInfo;
	T_AUDIO_SEEK_INFO* pSeek_Info;
	
	AK_ASSERT_PTR_VOID(pPlayer, "AudioDEC:	pPlayer Is Invalid");
	AK_ASSERT_PTR_VOID(pPlayer->pDmx, "AudioDEC:	pPlayer->pDmx Is Invalid");
	AK_ASSERT_PTR_VOID(pPlayer->pAudio, "AudioDEC:	pPlayer->pAudio Is Invalid");

	pSeek_Info = MediaLib_Dmx_GetAudioSeekInfo(pPlayer->pDmx->hMedia);
	if (AK_NULL == pSeek_Info)
	{
		memset(&seekInfo, 0, sizeof(T_AUDIO_SEEK_INFO));
		seekInfo.real_time = startPos;
		pSeek_Info = &seekInfo;
	}

	_SD_Decode_Seek(pPlayer->pAudio->hSD, pSeek_Info);
}

static T_pVOID Aud_Malloc(T_U32 size)
{
	return (void *)Fwl_MallocAndTrace((size), ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

static T_pVOID Aud_Free(T_pVOID var)
{
   return Fwl_FreeAndTrace(var, ((T_S8*)(__FILE__)), ((T_U32)__LINE__));
}

T_VOID Sd_SetAudioCB(T_AUDIO_FILTER_CB_FUNS* audioCB)
{
	AK_ASSERT_PTR_VOID(audioCB, "AudioDEC:	audioCB Is Invalid ");
	memset(audioCB, 0, sizeof(T_AUDIO_FILTER_CB_FUNS));
	
	audioCB->Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)Aud_Malloc;
    audioCB->Free 	= (MEDIALIB_CALLBACK_FUN_FREE)Aud_Free;
    audioCB->printf = (MEDIALIB_CALLBACK_FUN_PRINTF)AkDebugOutput;
	audioCB->delay	= (MEDIALIB_CALLBACK_FUN_RTC_DELAY)AK_Sleep;
}

T_U32 Sd_BufFreeSpace(T_pVOID hSD)
{
	T_AUDIO_BUFFER_CONTROL bufInfo;

	AK_ASSERT_PTR(hSD, "AudioDEC:	hSD Is Invalid", 0);
	
	switch(_SD_Buffer_Check(hSD, &bufInfo))
	{
	case _SD_BUFFER_WRITABLE:
		return bufInfo.free_len;
		break;

	case _SD_BUFFER_WRITABLE_TWICE:
		return bufInfo.free_len + bufInfo.start_len;
		break;

	default:
		return 0;
		break;
	}
}


/*****************************************************************************/

static T_VOID SdFilter_SetInfo(T_AUDIO_FILTER_IN_INFO* pInInfo, 
						const T_pAUDIO_DEC pAudio,
						T_AUDIO_FILTER_TYPE type, T_U8 mode)
{
	pInInfo->m_Type 			= type;
	pInInfo->m_Channels 		= pAudio->decOut.m_Channels;	
	pInInfo->m_SampleRate 		= pAudio->decOut.m_SampleRate;
    pInInfo->m_BitsPerSample	= 16;
	
	switch (pInInfo->m_Type)
	{
	case _SD_FILTER_EQ:
		pInInfo->m_Private.m_eq.eqmode = mode;
		break;

	case _SD_FILTER_WSOLA:
		pInInfo->m_Private.m_wsola.tempo 			= mode;
		pInInfo->m_Private.m_wsola.arithmeticChoice = _SD_WSOLA_ARITHMATIC_0;
		break;

	case _SD_FILTER_VOICECHANGE:
		pInInfo->m_Private.m_pitch.pitchMode = mode;
		break;

	default:
		break;
	}
}

static T_BOOL SdFilter_SetParm(T_pVOID hFilt, const T_pAUDIO_DEC pAudio,
							T_AUDIO_FILTER_TYPE type, T_U8 mode)
{
	T_AUDIO_FILTER_IN_INFO inInfo;

	AK_ASSERT_PTR(hFilt, "AudioDEC:	hFilt Is Invalid Handle ", AK_FALSE);

	SdFilter_SetInfo(&inInfo, pAudio, type, mode);
    
    return (T_BOOL)_SD_Filter_SetParam(hFilt, &inInfo);
}

static T_BOOL SdFilter_Init(T_pAUDIO_FILTER* ppFilter)
{
	T_U8 i;
	
	AK_ASSERT_PTR(ppFilter, "AudioDEC:	ppFilter Is Invalid", AK_FALSE);
	
	(*ppFilter) = (T_pAUDIO_FILTER)Fwl_Malloc(sizeof(T_AUDIO_FILTER));
	AK_ASSERT_PTR(*ppFilter, "AudioDEC:	Filter Malloc Failure", AK_FALSE);
	memset(*ppFilter, 0, sizeof(T_AUDIO_FILTER));

	for (i=0; i<AUDIO_FILTER_NUM; ++i)
    {
    	if (_SD_FILTER_EQ == i + _SD_FILTER_EQ)
			(*ppFilter)->filter[i].mode = _SD_EQ_MODE_NORMAL;
		else if (_SD_FILTER_WSOLA == i + _SD_FILTER_EQ)
			(*ppFilter)->filter[i].mode = _SD_WSOLA_1_0;
		else
			(*ppFilter)->filter[i].mode = PITCH_NORMAL;
    }
	
	(*ppFilter)->filtBuf.bufLen = FILTER_BUF_SIZE;
	(*ppFilter)->filtBuf.pBuf = Fwl_Malloc((*ppFilter)->filtBuf.bufLen);
	if (!(*ppFilter)->filtBuf.pBuf)
	{
		Fwl_Print(C2, M_ADEC, "Malloc Filter Buffer Failure");
		*ppFilter = Fwl_FreeTrace(*ppFilter);
		
		return AK_FALSE;		
	}

	return AK_TRUE;
}

static T_BOOL SdFilter_Open(T_pAUDIO_FILTER pFilter, const T_pAUDIO_DEC pAudio)
{
	T_U8 i;
	T_AUDIO_FILTER_INPUT filterIn;

	AK_ASSERT_PTR(pFilter, "AudioDEC:	pFilter Is Invalid ", AK_FALSE);
	
	Sd_SetAudioCB(&filterIn.cb_fun);

    for (i=0; i<AUDIO_FILTER_NUM; ++i)
    {		
    	if(2 == i)
    	{
    		SdFilter_SetInfo(&filterIn.m_info, pAudio, 
							_SD_FILTER_VOICECHANGE, pFilter->filter[i].mode);			
    	}
		else
		{
			SdFilter_SetInfo(&filterIn.m_info, pAudio, 
							i+_SD_FILTER_EQ, pFilter->filter[i].mode);
		}
		
        pFilter->filter[i].hFilt = _SD_Filter_Open(&filterIn); 
		
        Fwl_Print(C3, M_ADEC, "Audio Filter[%d] = 0x%X.\n", i, pFilter->filter[i].hFilt);  
    }

	if (_SD_MEDIA_TYPE_APE == pAudio->audioInfo.m_Type)
		pFilter->flag &= 0xF8;	// Close EQ / Tempo / pitch

	return AK_TRUE;
}

static T_U32 Sd_Filter(T_pAUDIO_DEC pAudio, T_U32 pos)
{
    T_BOOL bFirstFilter = AK_TRUE;
    T_U32 i=0;
    T_AUDIO_FILTER_BUF_STRC filtBuf;
    T_S32 ret=0;
    	
    filtBuf.buf_in 	= pAudio->decOut.m_pBuffer + pos;
    filtBuf.len_in 	= (pAudio->decOut.m_ulDecDataSize - pos > FILTER_SIZE_MAX) ?
						FILTER_SIZE_MAX : pAudio->decOut.m_ulDecDataSize - pos;
	
    filtBuf.buf_out	= pAudio->pSdFilt->filtBuf.pBuf;
    filtBuf.len_out	= FILTER_BUF_SIZE;
	
    for (i = 0; i < AUDIO_FILTER_NUM; i++)
    {                    
        if (CHAR_BIT(pAudio->pSdFilt->flag, i))
        {
        	Fwl_Print(C4, M_ADEC, "%d", i);
            if (bFirstFilter)
            {
                bFirstFilter = AK_FALSE;
            }
            else
            {
            	filtBuf.buf_out = filtBuf.buf_in;                 
				
				filtBuf.buf_in = pAudio->pSdFilt->filtBuf.pBuf;
				filtBuf.len_in = ret;
				//len_out在_SD_Filter_Control中会被改掉，是输入输出值
				//filtBuf.len_out = AUDIO_FILTER_BUFFER_SIZE;
            }
			
            ret = _SD_Filter_Control(pAudio->pSdFilt->filter[i].hFilt, &filtBuf);
        }
    }
	
    do
	{	
		ret -= WaveOut_Write((T_U8*)filtBuf.buf_out + filtBuf.len_out - ret,
							ret, INVALID_TIME_STAMP);
		if (ret > 0)
			AK_Sleep(5);
		
	}while(ret > 0);

	return filtBuf.len_in;
}

T_pVOID SdFilter_Close(T_pAUDIO_FILTER pFilter)
{
	T_U8 i;

	AK_ASSERT_PTR(pFilter, "AudioDEC:	pFilter Is Invalid ", AK_FALSE);

	Fwl_Print(C3, M_ADEC, "Close Audio Filter.\n");
	for (i=0; i<AUDIO_FILTER_NUM; ++i)
	{
		_SD_Filter_Close(pFilter->filter[i].hFilt);
	}
	
	pFilter->filtBuf.pBuf = Fwl_FreeTrace(pFilter->filtBuf.pBuf);
	return Fwl_FreeTrace(pFilter);
}

/*****************************************************************************/

static T_VOID Sd_Output(T_pAUDIO_DEC pAudio)
{
	T_S32 dataLen 	= (T_S32)pAudio->decOut.m_ulDecDataSize; // 64

	if (pAudio->pSdFilt && (pAudio->pSdFilt->flag & (SD_ENABLE_EQ|SD_ENABLE_TEMPO|SD_ENABLE_VOICECHANGE)))
	{
		while(dataLen > 0)
		{
			dataLen -= Sd_Filter(pAudio, pAudio->decOut.m_ulDecDataSize - dataLen);
		}
	}
	else
	{
		do
		{
			dataLen -= WaveOut_Write(pAudio->decOut.m_pBuffer + pAudio->decOut.m_ulDecDataSize - dataLen,
									dataLen, INVALID_TIME_STAMP);
			if (dataLen > 0)
				AK_Sleep(5);
			
		}while(dataLen > 0);
	}
}

static T_BOOL Sd_InitDecoder(T_pAUDIO_DEC* ppAudio, const T_MEDIALIB_DMX_INFO* pDmxInfo)
{
	AK_ASSERT_PTR(ppAudio, "ppAudio Parameter ERROR", AK_FALSE);
	
	*ppAudio = (T_pAUDIO_DEC)Fwl_Malloc(sizeof(T_AUDIO_DEC));
	AK_ASSERT_PTR(*ppAudio, "player->pAudio Malloc FAILURE ", AK_FALSE);
	
	memset(*ppAudio, 0, sizeof(T_AUDIO_DEC));
	(*ppAudio)->timer = ERROR_TIMER;
	Sd_SetAudioInfo(&(*ppAudio)->audioInfo, pDmxInfo);
	
	return AK_TRUE;
}

static T_BOOL Sd_OpenDecoder(T_pAUDIO_DEC pAudio, T_pDEMUXER dmx)
{
	T_AUDIO_DECODE_INPUT input;	

	AK_ASSERT_PTR(pAudio, "pAudio Is Invalid ", AK_FALSE);	
	AK_ASSERT_PTR(dmx, "dmx Is Invalid ", AK_FALSE);

	// This audio whether had limit
	if(Sd_IsDecLimit(&dmx->dmxInfo))
		return AK_FALSE;
	
	memset(&input, 0, sizeof(T_AUDIO_DECODE_INPUT));
	
	// Set Audio Decoder Input Infomation
	if (!Sd_SetDecInput(&input, (T_hFILE)dmx->hFile, &dmx->dmxInfo, dmx->isFile))
	{
		Fwl_Print(C2, M_ADEC, "Set Audio Decoder Input Information Failure");
		dmx->dmxInfo.m_bHasAudio = AK_FALSE;
		
		return AK_FALSE;
	}

	// Open Audio Decoder
	pAudio->hSD = _SD_Decode_Open(&input, &pAudio->decOut);
	if(!pAudio->hSD)
	{
		Fwl_Print(C2, M_ADEC, "Open SD CODEC Failure");
		
		return AK_FALSE;
	}
	_SD_SetBufferMode(pAudio->hSD, _SD_BM_NORMAL);	// _SD_BM_LIVE _SD_BM_NORMAL
		
	// Malloc Audio Decoder Output Buffer
	pAudio->decOut.m_ulSize = AUDIO_DEC_BUF_SIZE;
	pAudio->decOut.m_pBuffer = (T_U8*)Fwl_Malloc(pAudio->decOut.m_ulSize);	
	if(!pAudio->decOut.m_pBuffer)
	{
		Fwl_Print(C2, M_ADEC, "Malloc SD Decode Output Buffer Failure");
		
		_SD_Decode_Close(pAudio->hSD);
		
		return AK_FALSE;		
	}
	
	pAudio->decNum = (T_U8)((pAudio->decOut.m_SampleRate/6) >> 6);
	
	return AK_TRUE;
}

/*****************************************************************************/

T_VOID Sd_SetCodecCB(T_AUDIO_CB_FUNS *cbFun)
{
	AK_ASSERT_PTR_VOID(cbFun, "Input Parameter Is Invalid ");
	memset(cbFun, 0, sizeof(T_AUDIO_CB_FUNS));
	
	Sd_SetAudioCB((T_AUDIO_FILTER_CB_FUNS*)cbFun);
}

T_BOOL Sd_DecAppointFrame(T_pMT_MPLAYER pPlayer, T_U32 startPos)
{
	T_U16 	decNum = 0;
	T_BOOL 	retVal = AK_FALSE;

	AK_ASSERT_PTR(pPlayer, "Parameter pPlayer Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(pPlayer->pDmx, "Parameter pPlayer->pDmx Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(pPlayer->pAudio, "Parameter pPlayer->pAudio Is Invalid", AK_FALSE);
	
	if (0 > MediaLib_Dmx_Start(pPlayer->pDmx->hMedia, startPos))
		return retVal;
	
	Sd_SeekPos(pPlayer, startPos);
	
	if (0 < Dmx_GetAudio2Decoder(pPlayer->pDmx->hMedia, pPlayer->pAudio->hSD))
	{
		while(NORM_DEC_FAIL_NUM > ++decNum
			&& 0 >= _SD_Decode(pPlayer->pAudio->hSD, &pPlayer->pAudio->decOut));

		if (NORM_DEC_FAIL_NUM <= decNum)
		{
			Fwl_Print(C2, M_ADEC, "Audio Decode Failure");
			retVal = AK_FALSE;
		}
		else
		{
			retVal = AK_TRUE;
		}
		
		pPlayer->pAudio->decOut.m_ulDecDataSize = 0;
		_SD_Buffer_Clear(pPlayer->pAudio->hSD);
	}
	
	MediaLib_Dmx_Stop(pPlayer->pDmx->hMedia);
	
	return retVal;
}

T_BOOL Sd_OpenWaveOut(const T_AUDIO_DECODE_OUT *decOut, T_U32 beginTime)
{
	T_PCM_FORMAT fmt;
	
	AK_ASSERT_PTR(decOut, "decOut Is Invalid Parameter", AK_FALSE);	
	
	if (!Fwl_AudioEnableDA())
		return AK_FALSE;
	
	WaveOut_GetStatus(&fmt, WAVEOUT_FORMAT);
	if (fmt.channel 	!= decOut->m_Channels
		|| fmt.sampleBits 	!= decOut->m_BitsPerSample
		|| fmt.sampleRate 	!= decOut->m_SampleRate)
	{
		Fwl_Print(C3, M_ADEC, "Reset Sample Rate\n");

		fmt.channel 	= decOut->m_Channels;
    	fmt.sampleBits 	= decOut->m_BitsPerSample;
    	fmt.sampleRate 	= decOut->m_SampleRate;

		if(!WaveOut_Open(DA_BUFFER_NUM, &fmt, AK_NULL))
    	{
			Fwl_Print(C2, M_ADEC, "Wave Out Open Failure.\n");

			return AK_FALSE;
    	}
	}    
	
    return WaveOut_SetStatus(&beginTime, WAVEOUT_CURRENT_TIME);
}

T_BOOL Sd_OpenAudioPlayer(T_pMT_MPLAYER player)
{
	T_PCM_FORMAT fmt;
	
	AK_ASSERT_PTR(player, "Input Parameter Is Invalid", AK_FALSE);	
	
#ifdef MEDIA_FAST_SWITCH
	if (player->pAudio)
		return AK_TRUE;
#endif

	if(!Sd_InitDecoder(&player->pAudio, &player->pDmx->dmxInfo))
		return AK_FALSE;

	AK_ASSERT_PTR(player->pAudio, "player->pAudio Is Invalid", AK_FALSE);

	if (!Sd_OpenDecoder(player->pAudio, player->pDmx))
	{
		Fwl_Print(C2, M_ADEC, "Open Audio Decoder Failure");
		
		player->pAudio = Fwl_FreeTrace(player->pAudio);
		
		return AK_FALSE;
	}
	
	if (!Sd_DecAppointFrame(player, 0))
	{
		Fwl_Print(C2, M_ADEC, "Audio Predecode Failure");
		
		player->pAudio = Sd_Close(player->pAudio);
		
		return AK_FALSE;
	}

	WaveOut_GetStatus(&fmt, WAVEOUT_FORMAT);
	
	if (fmt.channel 	!= player->pAudio->decOut.m_Channels
		|| fmt.sampleBits 	!= player->pAudio->decOut.m_BitsPerSample
		|| fmt.sampleRate 	!= player->pAudio->decOut.m_SampleRate)
	{
		WaveOut_ResetBuf();
	}
	else
		WaveOut_CleanBuf();
	
	return AK_TRUE;
}

T_VOID Sd_Start(T_pMT_MPLAYER pPlayer, T_U32 startPos)
{	
	Sd_SeekPos(pPlayer, startPos);

	if (!Sd_OpenWaveOut(&pPlayer->pAudio->decOut, startPos))
	{
		if (pPlayer->pDmx->dmxInfo.m_bHasVideo)
		{
			pPlayer->pAudio = Sd_Close(pPlayer->pAudio);
			pPlayer->pDmx->dmxInfo.m_bHasAudio = AK_FALSE;
		}
		else
		{
			pPlayer->status = MPLAYER_ERR;
			pPlayer->endCB(T_END_TYPE_ERR);
		}

		Fwl_Print(C2, M_ADEC, "Open Wave Out Failure");
		return;
	}
	
	pPlayer->status = MPLAYER_WAVEOUT;
	Fwl_Print(C4, M_ADEC, "Open Wave Out OK");
	
	DriveEvtCB_Demux(0, 0);
			
	if (pPlayer->pAudio->pSdFilt && (pPlayer->pAudio->pSdFilt->flag & SD_ENABLE_TEMPO))
		WaveOut_SetRefSampRate(pPlayer->pAudio->decOut.m_SampleRate * 10 / (pPlayer->pAudio->pSdFilt->filter[_SD_FILTER_WSOLA-_SD_FILTER_EQ].mode + 5), startPos);

}

T_VOID Sd_Stop(T_pAUDIO_DEC pAudio)
{
	if (AK_NULL == pAudio 
		|| AK_NULL == pAudio->hSD
		|| !_SD_Buffer_Clear(pAudio->hSD))
	{
		Fwl_Print(C2, M_ADEC, "Clear SD Buffer Failure");				
	}
	
	pAudio->bFadeOpen = AK_FALSE;
	Fwl_Print(C3, M_ADEC, "Clear SD Buffer");
	// WaveOut_Close();
}

T_pVOID Sd_Close(T_pAUDIO_DEC pAudio)
{
	AK_ASSERT_PTR(pAudio, "pAudio Is Invalid ", AK_NULL);

	_SD_Decode_Close(pAudio->hSD);

	if (pAudio->pSdFilt)
	{
		pAudio->pSdFilt = SdFilter_Close(pAudio->pSdFilt);
	}
	
	pAudio->decOut.m_pBuffer = Fwl_FreeTrace(pAudio->decOut.m_pBuffer);
	
	return Fwl_FreeTrace(pAudio);
}

T_VOID Sd_HandleAudioDec(T_pMT_MPLAYER pPlayer)
{	
	T_pDEMUXER 	pDmx	= pPlayer->pDmx;
	T_pAUDIO_DEC pAudio	= pPlayer->pAudio;
	T_S32 		retval 	= 1;
	T_U32 		freeSpace;
	T_U8		decNum = 0;
	static T_U8 failNum = 0;
	
	while (WaveOut_GetStatus(&freeSpace, WAVEOUT_SPACE_SIZE) && freeSpace		// wait for DA
		&& decNum++ < pAudio->decNum
		&& 0 <= (retval = _SD_Decode(pAudio->hSD, &pAudio->decOut)) )			// 128 Bytes per time
	{
		if (retval > 0)
		{
			Sd_Output(pAudio);
			failNum = 0;
		}

		if (!pAudio->bFadeOpen
		&& MPlayer_GetCurTime() + FADE_OUT_NUM > MPlayer_GetTotalTime())
		{
			WaveOut_SetFade(FADE_OUT_NUM>>2, FADE_STATE_OUT);
			pAudio->bFadeOpen = AK_TRUE;
		}
	}	
	
	if (!pDmx->dmxInfo.m_bHasVideo
		&& (0 > retval || ++failNum > NORM_DEC_FAIL_NUM))
	{
		T_END_TYPE endType;
		
		Fwl_Print(C3, M_ADEC, "NOT More Audio Data to Decode, Exit Audio Decoder Thread");
		CThread_StopTimer(&pAudio->timer);

		if (!pDmx->hMedia)
			return;
		
		if (MediaLib_Dmx_CheckAudioEnd(pDmx->hMedia))
			endType = T_END_TYPE_NORMAL;
		else
			endType = T_END_TYPE_ERR;
		
		if (pPlayer->endCB)
		{
			pPlayer->endCB(endType);
			pPlayer->endCB = AK_NULL;
		}
		else
		{
			Fwl_Print(C3, M_ADEC, "Close Player By Audio Decoder End.\n");
			DmxMgr_HandleEvt(EVT_PLAY_CLOSE, 0);
		}

		AK_Sleep(20);
	}		
}

T_BOOL Sd_SetFilter(T_pVOID hAudioDec, T_AUDIO_FILTER_TYPE type, T_U8 mode)
{
	T_U32 curTime = 0;
	T_pAUDIO_DEC pAudio = (T_pAUDIO_DEC)hAudioDec;
	
    AK_ASSERT_PTR(pAudio, "Sd_SetFilter(): pAudio is Invalid", AK_FALSE);
	curTime = MPlayer_GetCurTime();

	if (!pAudio->pSdFilt)
	{
		if (!SdFilter_Init(&pAudio->pSdFilt))
			return AK_FALSE;
		
		if (!SdFilter_Open(pAudio->pSdFilt, pAudio))
		{
			Fwl_Print(C2, M_ADEC, "Sound Filter Open Failure\n");
			
			pAudio->pSdFilt->filtBuf.pBuf = Fwl_FreeTrace(pAudio->pSdFilt->filtBuf.pBuf);
			pAudio->pSdFilt = Fwl_FreeTrace(pAudio->pSdFilt);

			return AK_FALSE;
		}
	}
	
	AK_ASSERT_PTR(pAudio->pSdFilt->filter[Filter_idx[type]].hFilt, "AudioDEC:	Audio Filter Handler Is Invalid", AK_FALSE);
    
    if (pAudio->pSdFilt->filter[Filter_idx[type]].mode == mode
		|| _SD_MEDIA_TYPE_APE == pAudio->audioInfo.m_Type)
    {
        return AK_TRUE;
    }

	if (!SdFilter_SetParm(pAudio->pSdFilt->filter[Filter_idx[type]].hFilt, pAudio, type, mode))
	{
		Fwl_Print(C2, M_ADEC, "Set Audio Filter Paramter Failure.\n");
		return AK_FALSE;
	}
	
	switch(type)
	{
	case _SD_FILTER_EQ:			
		if (_SD_EQ_MODE_NORMAL == mode)
		{			
			pAudio->pSdFilt->flag &= 0xFE;	// Close EQ
 		}
		else
    	{  
    		pAudio->pSdFilt->flag &= 0xF9;	// Close Tempo & tom
    		pAudio->pSdFilt->filter[Filter_idx[_SD_FILTER_WSOLA]].mode = _SD_WSOLA_1_0;
			pAudio->pSdFilt->filter[Filter_idx[_SD_FILTER_VOICECHANGE]].mode = PITCH_NORMAL;
    		gb.AudioPlaySpeed = _SD_WSOLA_1_0;					
			gs.AudioPitchMode = PITCH_NORMAL;
			
        	pAudio->pSdFilt->flag |= SD_ENABLE_EQ; 	// Open EQ	
        	WaveOut_SetRefSampRate(pAudio->decOut.m_SampleRate, curTime);
     	}
		break;

	case _SD_FILTER_WSOLA:
		if (_SD_WSOLA_1_0 == mode)
    	{    		
        	pAudio->pSdFilt->flag &= 0xFD;	// Close Tempo
        }		
        else
        {						
        	pAudio->pSdFilt->flag &= 0xFA;	// Close EQ & Close tom

			pAudio->pSdFilt->filter[Filter_idx[_SD_FILTER_EQ]].mode = _SD_EQ_MODE_NORMAL;
			pAudio->pSdFilt->filter[Filter_idx[_SD_FILTER_VOICECHANGE]].mode = PITCH_NORMAL;
        	gs.AudioToneMode = _SD_EQ_MODE_NORMAL;
			gs.AudioPitchMode = PITCH_NORMAL;
			
        	pAudio->pSdFilt->flag |= SD_ENABLE_TEMPO;	// Open Tempo	
       	}
       	
		WaveOut_SetRefSampRate(pAudio->decOut.m_SampleRate * 10 / (mode + 5), curTime);
		break;

	case _SD_FILTER_VOICECHANGE: 		
		if (PITCH_NORMAL == mode)
		{			
			pAudio->pSdFilt->flag &= 0xFB;	// Close voiceChange
		}
		else
		{  
			pAudio->pSdFilt->flag &= 0xFC;	// Close Tempo & eq

			pAudio->pSdFilt->filter[Filter_idx[_SD_FILTER_EQ]].mode = _SD_EQ_MODE_NORMAL;
			pAudio->pSdFilt->filter[Filter_idx[_SD_FILTER_WSOLA]].mode = _SD_WSOLA_1_0;
			gb.AudioPlaySpeed = _SD_WSOLA_1_0;			
			gs.AudioToneMode  = _SD_EQ_MODE_NORMAL;
			
			pAudio->pSdFilt->flag |= SD_ENABLE_VOICECHANGE;	// Open Voice Change	
			WaveOut_SetRefSampRate(pAudio->decOut.m_SampleRate, curTime);
		}
		break;

	default:
		Fwl_Print(C3, M_ADEC, "Unsupport Audio Filter Type.\n");
		return AK_FALSE; 
		break;
    } 
	
	pAudio->pSdFilt->filter[Filter_idx[type]].mode = mode;
	Fwl_Print(C3, M_ADEC, "Set Filter Type: %d Successed.\n", type);
	
    return AK_TRUE;
}


/*
 * End of File
 */
