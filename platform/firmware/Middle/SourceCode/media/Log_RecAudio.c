#include "Fwl_osMalloc.h"
#include "Fwl_osCom.h"
#include "Eng_String_UC.h"
#include "lib_sdfilter.h"
#include "Ctl_AVIPlayer.h"
#include "fwl_waveout.h"
#include "fwl_wavein.h"
#include "eng_string.h"
#include "lib_image_api.h"
#include "arch_freq.h"
#include "akos_api.h"
#include "arch_mmc_sd.h"
#include "arch_mmu.h"
#include "string.h"
#include "eng_debug.h"
#include "hal_timer.h"
#include "fwl_pfaudio.h"
#include "l2_cache.h"
#include "File.h"
#include "Hal_Sysdelay.h"
#include "Log_RecAudio.h"

#define AUDIO_ONE_BUFFER_SIZE   (2048)

/*****************************************************************************/
#if (defined (SUPPORT_AUDIOREC) || defined (SUPPORT_FM))

//xuyr pps
#define RECAUDIO_PRE_PROCESS_A1 511639552
#define RECAUDIO_PRE_PROCESS_A2 -244645887
#define RECAUDIO_PRE_PROCESS_B0 248905728
#define RECAUDIO_PRE_PROCESS_B1 -497811968
#define RECAUDIO_PRE_PROCESS_B2 248905945
#define QCOEF 					28
#define Q_PREP_COEF 			(1<<QCOEF)
#define Q16 					(1<<16)

typedef struct 
{
	T_S32 y2;
	T_S32 y1;
	T_S32 x0;
	T_S32 x1;
} Pre_ProcessState; 

static Pre_ProcessState recaudio_pps;

static T_VOID recaudio_pre_process_init(Pre_ProcessState *st)
{
	st->y2 = 0;
	st->y1 = 0;
	st->x0 = 0;
	st->x1 = 0;
} 

static int recaudio_pre_process(Pre_ProcessState *st,
					T_S16 signal[], /* input/output signal */
					T_S16 lg)       /* lenght of signal    */
{
    T_S16 i;
	T_S32 x2;
    T_S32 L_tmp;
 
	//T_S32 a1 = AUDIO_RECORDER_PRE_PROCESS_A1;//(int)(1.906005859*Q_PREP_COEF+0.5);
	//T_S32 a2 = AUDIO_RECORDER_PRE_PROCESS_A2;//(int)(-0.911376953*Q_PREP_COEF+0.5);
	//T_S32 b0 = AUDIO_RECORDER_PRE_PROCESS_B0;//(int)(0.4636230465*2*Q_PREP_COEF+0.5);
	//T_S32 b1 = AUDIO_RECORDER_PRE_PROCESS_B1;//(int)(-0.92724705*2*Q_PREP_COEF+0.5);
    //T_S32 b2 = AUDIO_RECORDER_PRE_PROCESS_B2;//(int)(0.4636234515*2*Q_PREP_COEF+0.5);
	
    for (i = 0; i < lg; i++) 
	{
        x2 = st->x1;
        st->x1 = st->x0;
        st->x0 = signal[i];
		L_tmp = (T_S32)(((T_S64)st->y1 * RECAUDIO_PRE_PROCESS_A1) >>(16 + QCOEF - 16)) ;
		L_tmp += (T_S32)(((T_S64)st->y2 * RECAUDIO_PRE_PROCESS_A2) >>(16 + QCOEF - 16)) ;
		L_tmp += (T_S32)(((T_S64)st->x0 * RECAUDIO_PRE_PROCESS_B0) >>(0 + QCOEF - 16));
		L_tmp += (T_S32)(((T_S64)st->x1 * RECAUDIO_PRE_PROCESS_B1) >>(0 + QCOEF - 16));
		L_tmp += (T_S32)(((T_S64)x2 * RECAUDIO_PRE_PROCESS_B2) >>(0 + QCOEF - 16));
		signal[i] = (T_S16)(L_tmp >> 16);
		st->y2 = st->y1;
        st->y1 = L_tmp;
    }
	
    return 0;
}

extern T_VOID Sd_SetAudioCB(T_AUDIO_FILTER_CB_FUNS* audioCB);
extern T_VOID Sd_SetCodecCB(T_AUDIO_CB_FUNS *cbFun);

/*
*brief:throw data from buf
*param1:pMEncoder[in],handle
*param2:throwtime[in],times(ms)
*retval: T_VOID
*/
T_VOID GetOffBufData(T_MEncoder* pMEncoder, T_U32 throwtime)
{
	T_U32 totaldata;
	T_U32 tempdata;
	
	totaldata = (throwtime*2*(pMEncoder->audEncInInfo.enc_in_info.m_nSampleRate)*(pMEncoder->audEncInInfo.enc_in_info.m_nChannel))/1000;
	tempdata = 0;
	
	while (1)
    {
		if (waveInRead(pMEncoder->pWaveIn, (T_U8 **)(&pMEncoder->audEncBuf.buf_in), &pMEncoder->audEncBuf.len_in))
		{
			if (pMEncoder->audEncBuf.len_in > 1)
			{
				tempdata += pMEncoder->audEncBuf.len_in;						
			}
			
			if (tempdata >= totaldata)				
			{	
				Fwl_Print(C3, M_AUDIO, "GetOffBufData tempdata=%ld,totaldata=%ld\n",tempdata,totaldata);
				return;			
			}  
		}
	}
}

static T_VOID MEncoder_SetAudioRecInfo(T_AUDIO_REC_INPUT *audioRec, T_eREC_MODE audioType, T_U32 sampleRate)
{
    Sd_SetCodecCB(&audioRec->cb_fun);

    audioRec->enc_in_info.m_BitsPerSample            = 16;
    audioRec->enc_in_info.m_nChannel                 = 1;
    
    switch (audioType)
    {
    case eRECORD_MODE_AMR:        
        audioRec->enc_in_info.m_nSampleRate             = AUDIOREC_SAMPLE_RATE_AMR;
        audioRec->enc_in_info.m_private.m_amr_enc.mode  = AMR_ENC_MR515;
        audioRec->enc_in_info.m_Type                    = _SD_MEDIA_TYPE_AMR;
        break;
        
    case eRECORD_MODE_ADPCM_IMA:
        audioRec->enc_in_info.m_nSampleRate             = sampleRate;
        audioRec->enc_in_info.m_private.m_adpcm.enc_bits= 4;
        audioRec->enc_in_info.m_Type                    = _SD_MEDIA_TYPE_ADPCM_IMA;
        break;

    case eRECORD_MODE_MP3:
        audioRec->enc_in_info.m_nSampleRate             = sampleRate;
        audioRec->enc_in_info.m_Type                    = _SD_MEDIA_TYPE_MP3;
        audioRec->enc_in_info.m_private.m_mp3.bitrate   = 0;
        audioRec->enc_in_info.m_private.m_mp3.mono_from_stereo = 0;
        break;
    case eRECORD_MODE_AAC:
        audioRec->enc_in_info.m_nSampleRate             = sampleRate;
        audioRec->enc_in_info.m_Type                    = _SD_MEDIA_TYPE_AAC;
        break;
        
    case eRECORD_MODE_WAV:
    default:
        audioRec->enc_in_info.m_nSampleRate             = sampleRate;
        audioRec->enc_in_info.m_Type                    = _SD_MEDIA_TYPE_PCM;
        break;
    }
}

/*
*brief:Media Encoder init
*param1:pMEncoder[in],encode handle
*param2:recMode[in],record file format
*param3:sampleRate[in],record sample speed
*retval:AK_TRUE is success,or failed
*note:call this func before all other record func is called,and call only once before record
*/
T_BOOL MEncoder_Init(T_MEncoder *pMEncoder, T_eREC_MODE recMode, T_U32 sampleRate)
{
    AK_ASSERT_PTR(pMEncoder, "MEncoder_Init:pMEncoder is NULL\n", AK_FALSE);
    
    if (pMEncoder->bEncodeInited)
    {
        AK_DEBUG_OUTPUT("audio encode have been initialed\n");
        return AK_FALSE;
    }
    else if (pMEncoder)
    {
        pMEncoder->OutBuf.pBuf = (T_U8*)Fwl_Malloc(ENCODE_OUTBUF_SIZE);
        AK_ASSERT_PTR(pMEncoder->OutBuf.pBuf, "encode out buffer malloc failed\n", AK_FALSE);
        Utl_MemSet(pMEncoder->OutBuf.pBuf , 0, ENCODE_OUTBUF_SIZE);

        pMEncoder->OutBuf.bufSize = ENCODE_OUTBUF_SIZE;

		MEncoder_SetAudioRecInfo(&pMEncoder->audEncInInfo, recMode, sampleRate);
        AK_DEBUG_OUTPUT("@record type=%d\n",pMEncoder->audEncInInfo.enc_in_info.m_Type);

        if (_SD_MEDIA_TYPE_PCM !=  pMEncoder->audEncInInfo.enc_in_info.m_Type)
        {
            pMEncoder->bEncode = AK_TRUE;
        }
        else
        {
            pMEncoder->bEncode = AK_FALSE;
        }
        pMEncoder->hFile = FS_INVALID_HANDLE;
        pMEncoder->hMedia = AK_NULL;
        pMEncoder->pWaveIn = AK_NULL;
        pMEncoder->curTime = 0;
        pMEncoder->audEncBuf.buf_out = pMEncoder->OutBuf.pBuf;
        pMEncoder->audEncBuf.len_out = ENCODE_OUTBUF_SIZE;
        pMEncoder->audEncBuf.buf_in = AK_NULL;
        pMEncoder->audEncBuf.len_in = 0;
        pMEncoder->audEncState = eAUDIO_REC_STATE_INIT;
#ifdef SUPPORT_AUDIOREC_DENOICE
		pMEncoder->pAudFilter = AK_NULL;
#endif
        Utl_MemSet(&pMEncoder->audEndOutInfo, 0, sizeof(T_AUDIO_ENC_OUT_INFO));

        pMEncoder->stopMutex = AK_Create_Semaphore(1, AK_PRIORITY);
        waveInInit();

        pMEncoder->bEncodeInited = AK_TRUE;

        return AK_TRUE;
    }

	AK_DEBUG_OUTPUT("media encoder is inited successly\n");
	
	return AK_FALSE;
}


/*
*brief:Media Encoder start
*param1:pMEncoder[in],handle
*param2:hFile[in],encode file handle
*param3:input_src[in],record object
*retval:AK_TRUE is successed,or failed
*/
T_BOOL MEncoder_Start(T_MEncoder *pMEncoder, T_hFILE rec_fd, T_eINPUT_SOURCE input_src)
{
    T_WAVE_IN waveInInput;
        
	if (AK_NULL == pMEncoder || !pMEncoder->bEncodeInited)
	{
		AK_DEBUG_OUTPUT("@arg is failed\n");
		return AK_FALSE;
	}

	AK_ASSERT_VAL(FS_INVALID_HANDLE != rec_fd, "FS_INVALID_HANDLE == rec_fd", AK_FALSE);

    pMEncoder->hFile = rec_fd;

    AK_DEBUG_OUTPUT("bEncode=%d\n",pMEncoder->bEncode);
    if (pMEncoder->bEncode)
    {    
#ifdef OS_ANYKA

        pMEncoder->hMedia = _SD_Encode_Open(&pMEncoder->audEncInInfo, &pMEncoder->audEndOutInfo);
        AK_ASSERT_PTR(pMEncoder->hMedia, "MEncoder_Start:hMedia is NULL\n",AK_FALSE);
#endif
    }

	recaudio_pre_process_init(&recaudio_pps);
	
	memset(&waveInInput,0,sizeof(waveInInput));
    waveInInput.waveInFmt.channel = pMEncoder->audEncInInfo.enc_in_info.m_nChannel;
    waveInInput.waveInFmt.sampleBits = pMEncoder->audEncInInfo.enc_in_info.m_BitsPerSample;
    waveInInput.waveInFmt.sampleRate = pMEncoder->audEncInInfo.enc_in_info.m_nSampleRate;
    waveInInput.volume = 1024;
    waveInInput.inputSrc = input_src;
	waveInInput.OneBufSize = AUDIO_ONE_BUFFER_SIZE;

    waveInOpen(&pMEncoder->pWaveIn, &waveInInput);
    GetOffBufData(pMEncoder, 500); //get off 500ms

#ifdef SUPPORT_AUDIOREC_DENOICE
	//暂时限制非8K采样率的不降噪(会死机)
	//if (8000 == waveInInput.waveInFmt.sampleRate) 
	if (eINPUT_SOURCE_MIC == input_src && gs.bAudioRecDenoice)
	{
		T_AUDIO_FILTER_INPUT filter_input;
		
		Sd_SetAudioCB(&filter_input.cb_fun);
		filter_input.m_info.m_Type = _SD_FILTER_DENOICE;
		filter_input.m_info.m_SampleRate = waveInInput.waveInFmt.sampleRate;
		filter_input.m_info.m_Channels = waveInInput.waveInFmt.channel;
		filter_input.m_info.m_BitsPerSample = waveInInput.waveInFmt.sampleBits;
		filter_input.m_info.m_Private.m_NR.NR_Level = 1;
		filter_input.m_info.m_Private.m_NR.ASLC_ena = 1;
		pMEncoder->pAudFilter = _SD_Filter_Open(&filter_input);
		AK_DEBUG_OUTPUT("@open sound filter : pMEncoder->pAudFilter is %d\n", pMEncoder->pAudFilter);
	}
	else
	{
		pMEncoder->pAudFilter = AK_NULL;
	}
#endif	

    pMEncoder->audEncState = eAUDIO_REC_STATE_RECORDING;
    AK_DEBUG_OUTPUT("MEncoder start\n");
	
    return AK_TRUE;
    
}


T_BOOL MEncoder_Pause(T_MEncoder* pMEncoder)
{
	return AK_FALSE;
}


/*
*brief:Media Encoder stop
*param1:pMEncoder[in],encode handle
*retval:AK_TRUE is successed , or failed
*/
T_BOOL MEncoder_Stop(T_MEncoder* pMEncoder)
{    
    T_U32 write_len = 0;
    T_BOOL ret = AK_FALSE;
    
    AK_ASSERT_PTR(pMEncoder, "MEncoder_Stop pMEncoder is NULL\n",AK_FALSE);
    
    AK_Obtain_Semaphore(pMEncoder->stopMutex, AK_SUSPEND);

    if(eAUDIO_REC_STATE_RECORDING == pMEncoder->audEncState)
    {        
        if (waveInClose(pMEncoder->pWaveIn))
        {
#ifdef SUPPORT_AUDIOREC_DENOICE
			if (AK_NULL != pMEncoder->pAudFilter)
			{
				_SD_Filter_Close(pMEncoder->pAudFilter);
				AK_DEBUG_OUTPUT("@close sound filter\n");
				pMEncoder->pAudFilter = AK_NULL;
			}
#endif

            if (pMEncoder->bEncode)
            {
#ifdef OS_ANYKA
                if (_SD_Encode_Close(pMEncoder->hMedia))
                {
                    pMEncoder->audEncState = eAUDIO_REC_STATE_STOP;
                    ret = AK_TRUE;
                }
#endif
            }
            else
            {
                pMEncoder->audEncState = eAUDIO_REC_STATE_STOP;
                ret = AK_TRUE;
            }
            
            if (pMEncoder->OutBuf.dataLen > 0)
            {
                write_len = Fwl_FileWrite(pMEncoder->hFile, pMEncoder->OutBuf.pBuf, pMEncoder->OutBuf.dataLen);
                if (write_len!=pMEncoder->OutBuf.dataLen)
                {
                    AK_DEBUG_OUTPUT("MEncoder_Stop: write data error\n");
                }
                pMEncoder->OutBuf.dataLen = 0;
            }
            
            pMEncoder->hFile = FS_INVALID_HANDLE;
            pMEncoder->hMedia = AK_NULL;
            pMEncoder->pWaveIn = AK_NULL;
            pMEncoder->curTime = 0;
            pMEncoder->audEncBuf.buf_out = pMEncoder->OutBuf.pBuf;
            pMEncoder->audEncBuf.len_out = ENCODE_OUTBUF_SIZE;
            pMEncoder->audEncBuf.buf_in = AK_NULL;
            pMEncoder->audEncBuf.len_in = 0;
        }       
        
    }
    else
    {
        ret = AK_TRUE;
    }

    AK_Release_Semaphore(pMEncoder->stopMutex);
    if (AK_FALSE == ret)
    {
        AK_DEBUG_OUTPUT("MEncoder_Stop error!!!\n");
    }

    return ret;
}


T_BOOL MEncoder_Destroy(T_MEncoder* pMEncoder)
{
    AK_ASSERT_PTR(pMEncoder, "MEncoder_Destroy is  NULL\n", AK_FALSE);

    if (pMEncoder->bEncodeInited)
    {
        Fwl_Free(pMEncoder->OutBuf.pBuf);
        AK_Delete_Semaphore(pMEncoder->stopMutex);
        waveInDestroy();
        
        pMEncoder->bEncodeInited = AK_FALSE;
    }
    
    return AK_TRUE;        
}


/*
*brief:Media Encoder encode process
*param1:pMEncoder[in],encode handle
*param2:bError[out],echo if error occur  
*retval:reture written to file data len 
*/
T_S32 MEncoder_HandleAudioEncode(T_MEncoder* pMEncoder, T_BOOL *pError)
{
    T_U32 encode_len = 0;
    T_U32 pcm_len = 0;
    T_U32 write_len = 0;    
    
    AK_ASSERT_PTR(pMEncoder, "MEncoder_HandleAudioEncode:pMEncoder is NULL\n", AK_FALSE);

    *pError = AK_FALSE;

    AK_Obtain_Semaphore(pMEncoder->stopMutex, AK_SUSPEND);
    if (eAUDIO_REC_STATE_RECORDING == pMEncoder->audEncState)
    {
        while (waveInRead(pMEncoder->pWaveIn, (T_U8 **)(&pMEncoder->audEncBuf.buf_in), &pMEncoder->audEncBuf.len_in))
        {
			recaudio_pre_process(&recaudio_pps, pMEncoder->audEncBuf.buf_in, (T_S16)(pMEncoder->audEncBuf.len_in>>1));//xuyr pps

			if (pMEncoder->audEncBuf.len_in > 1)
			{
				pMEncoder->sample[0] = *((T_S16 *)(pMEncoder->audEncBuf.buf_in));
				pMEncoder->sample[1] = *((T_S16 *)(pMEncoder->audEncBuf.buf_in) + 1);
			}
			
#ifdef SUPPORT_AUDIOREC_DENOICE
			if(AK_NULL != pMEncoder->pAudFilter)
			{
				T_AUDIO_FILTER_BUF_STRC filter_buf;
				
				filter_buf.buf_in  = (pMEncoder->audEncBuf.buf_in);
				filter_buf.len_in  = (pMEncoder->audEncBuf.len_in);
				filter_buf.buf_out = (pMEncoder->audEncBuf.buf_in);
				filter_buf.len_out = (pMEncoder->audEncBuf.len_in);
				
				_SD_Filter_Control(pMEncoder->pAudFilter,&filter_buf);
			}
#endif			

            pcm_len = pMEncoder->audEncBuf.len_in;
            
            if (pMEncoder->bEncode)
            {
#ifdef OS_ANYKA
                encode_len = _SD_Encode(pMEncoder->hMedia, &pMEncoder->audEncBuf);
                pMEncoder->OutBuf.dataLen += pMEncoder->audEncBuf.len_out;
#endif
            }
            else
            {
                Utl_MemCpy(pMEncoder->audEncBuf.buf_out, pMEncoder->audEncBuf.buf_in, pcm_len);
                pMEncoder->OutBuf.dataLen += pcm_len;
            }
            /*calc cur time*/			
			waveInSetStatus(pMEncoder->pWaveIn, WAVEIN_BUF_COUNT, &pcm_len);
			waveInGetStatus(pMEncoder->pWaveIn, WAVEIN_CUR_TIME, &pMEncoder->curTime);

			/*write file ,update writepos*/
            if (pMEncoder->OutBuf.dataLen >= ENCODE_ALERT_DATALEN)
            {
                write_len = Fwl_FileWrite(pMEncoder->hFile, pMEncoder->OutBuf.pBuf, pMEncoder->OutBuf.dataLen);
                if (write_len != pMEncoder->OutBuf.dataLen)
                {
                    *pError = AK_TRUE;
                }
                AK_DEBUG_OUTPUT("=>%d\n", write_len);
                pMEncoder->OutBuf.dataLen = 0;
                pMEncoder->audEncBuf.buf_out = pMEncoder->OutBuf.pBuf;
            }
            else
            {
                pMEncoder->audEncBuf.buf_out = pMEncoder->OutBuf.pBuf+ pMEncoder->OutBuf.dataLen;
            }
        }
    }
    AK_Release_Semaphore(pMEncoder->stopMutex);

    return write_len;
}


T_BOOL MEncode_SetEncodeType(T_MEncoder* pMEncoder, T_AUDIO_TYPE audioRecType)
{
    pMEncoder->audEncInInfo.enc_in_info.m_Type = audioRecType;
    if (_SD_MEDIA_TYPE_PCM != audioRecType)
    {
        pMEncoder->bEncode = AK_TRUE;
    }
    else
    {
        pMEncoder->bEncode = AK_FALSE;
    }

    return AK_TRUE;
}

T_AUDIO_TYPE MEncode_GetEncodeType(T_MEncoder* pMEncoder)
{
    return pMEncoder->audEncInInfo.enc_in_info.m_Type;
}

T_BOOL  MEncoder_SetVolume(T_MEncoder* pMEncoder,T_U32 volume)
{
    return waveInSetVolume(pMEncoder->pWaveIn, volume);
}

T_U32  MEncoder_GetVolume(T_MEncoder* pMEncoder)
{
    return waveInGetVolume(pMEncoder->pWaveIn);
}

T_eAUDIO_REC_STATE MEncoder_GetState(T_MEncoder* pMEncoder)
{
    return pMEncoder->audEncState;
}

T_BOOL MEncoder_GetInfo(T_MEncoder* pMEncoder, T_MEDIALIB_REC_INFO *pInfo)
{

	return AK_FALSE;
}

T_U32 MEncoder_GetCurTime(T_MEncoder* pMEncoder)
{
    return pMEncoder->curTime;
}

T_BOOL MEncoder_GetCurSample(T_MEncoder* pMEncoder, T_S16 sample[2])
{
	AK_ASSERT_PTR(pMEncoder, "MEncoder_GetCurSample is  NULL\n", AK_FALSE);

    if (eAUDIO_REC_STATE_RECORDING == pMEncoder->audEncState)
    {
	    sample[0] = pMEncoder->sample[0];
	    sample[1] = pMEncoder->sample[1];
    }
    
	return AK_TRUE;
}

#endif

