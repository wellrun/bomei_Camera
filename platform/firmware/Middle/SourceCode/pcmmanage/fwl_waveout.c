/*
*file:waveout.c
*description:pcm output interface
*author:GB
*date:2009-1-22
*/
#include "fwl_waveout.h"
#include "Fwl_osMalloc.h"
#include "Eng_Debug.h"
#include "Log_MediaPlayer.h"
#include "Lib_state.h"
#include "gpio_config.h"
#include "Lib_state_api.h"
#include "akos_api.h"
#include "hal_sound.h"
#include "hal_timer.h"
#include "arch_analog.h"
#include "fwl_pfAudio.h"
#include "VAapi.h"

#ifdef SUPPORT_VISUAL_AUDIO	
#include "Log_MediaVisualAudio.h"
#endif

#define PCM_WRITE(dst, src) *(dst) = (*(src) * wavOut.curVolume) >> 10

static T_WAVE_OUT wavOut = {0};	//oniy used in this file

T_SOUND_DRV *dac_drv = AK_NULL;

#ifdef SUPPORT_VISUAL_AUDIO
extern VA_Handle VA_VisualAudio_handle;
extern T_BOOL bVAShow_data_ready;	
#endif


static T_U16    DABufGetFreeLen(T_VOID);
static T_BOOL	DABufRemalloc(T_U16 OneBufSize, T_U32 DABufNum);

static T_S32 WaveOut_Lock(T_VOID)
{
	if (0 == wavOut.mutex)
	{
		wavOut.mutex = AK_Create_Semaphore(1, AK_PRIORITY);
	}
	
	return AK_Obtain_Semaphore(wavOut.mutex, AK_SUSPEND);
}

static T_S32 WaveOut_Unlock(T_VOID)
{
	if (0 == wavOut.mutex)
	{
		return -1;
	}
	
	return AK_Release_Semaphore(wavOut.mutex);
}

static T_VOID WaveOut_Init(T_WAVE_OUT* pWaveOut)
{
	memset(&pWaveOut->PCMFmt, 0, sizeof(T_PCM_FORMAT));
	
	pWaveOut->daBufMgr.OneBufSize 	= 5120;		// 5K
    pWaveOut->daBufMgr.DABufNum   	= 10;
	pWaveOut->daBufMgr.daState 		= eDA_STATE_STOP;
	pWaveOut->daBufMgr.timeStamp 	= INVALID_TIME_STAMP;
	pWaveOut->daBufMgr.pCurDaBuf 	= AK_NULL;
	pWaveOut->daBufMgr.datalen 		= 0;
    
    //pWaveOut->volume = 0;
    pWaveOut->track 				= eSOUND_TRACK_STEREO;
    pWaveOut->dataFinishedCB 		= AK_NULL; 
    pWaveOut->bWriteOver 			= AK_FALSE;
    pWaveOut->bPlayOver  			= AK_FALSE; 
	pWaveOut->bInit 				= AK_FALSE;	
}

static T_BOOL WaveOut_SetDABufMgr(T_DABUF_MGR *daBufMgr, T_PCM_FORMAT *fmt, T_U8 DABufNum)
{
	//calculate one buf size according to playing time: 30ms
    //At present the smallest unit DA can play is 512B,
    //so the results are added or reduced to divide 512 exactly.
	switch(fmt->sampleRate)
    {
    case 8000:
        daBufMgr->OneBufSize = 1024;
        break;
        
    case 11025:
        daBufMgr->OneBufSize = 1536;
        break;
        
    case 16000:
        daBufMgr->OneBufSize = 2048;
        break;
        
    case 22050:
        daBufMgr->OneBufSize = 2560;
        break;
        
    case 24000:
        daBufMgr->OneBufSize = 3072;
        break;
        
    case 32000:
        daBufMgr->OneBufSize = 4096;            
        break;
        
    case 44100:
        daBufMgr->OneBufSize = 5120;
        break;
        
    case 48000:
        daBufMgr->OneBufSize = 5632;
        break;

    default:
        daBufMgr->OneBufSize = (T_U16)(fmt->sampleRate
                                     * (fmt->sampleBits>>3)
                                     * 2		//channel equal to 2 even mono
                                     * 3/100);	//30ms/1000ms
                                     
        //should divide exactly 512 at present temporarily
        daBufMgr->OneBufSize = (daBufMgr->OneBufSize>>9)<<9;
        break;
    }
    
    if (0 == daBufMgr->OneBufSize)
    {
        Fwl_Print(C2, M_WAVE, "sampleRate = %d, sampleBits = %d, waveout open error for OneBufSize = 0!\n",fmt->sampleRate, fmt->sampleBits);
        
        return AK_FALSE;
    }
	
    daBufMgr->DABufNum 	= DABufNum;
	daBufMgr->daState 	= eDA_STATE_STOP;
	daBufMgr->timeStamp = INVALID_TIME_STAMP;
	daBufMgr->pCurDaBuf = AK_NULL;
	daBufMgr->datalen 	= 0;

	return AK_TRUE;
}

#define DOFADE_INTERVAL_THRESHOLD   1000 //ms
static T_VOID WaveOut_DoFade(T_WAVE_OUT *pWaveOut)
{

	if (pWaveOut->fadeCtrl.CurNum < pWaveOut->fadeCtrl.FadeStep)
	{
		pWaveOut->fadeCtrl.CurNum++;
	}
	else
	{
		pWaveOut->fadeCtrl.CurNum = 0;
		
		switch (pWaveOut->fadeCtrl.FadeState) 
		{
		case FADE_STATE_IN:		// Fade IN
			if (pWaveOut->curVolume < pWaveOut->volume + pWaveOut->fadeCtrl.volStep)
				pWaveOut->curVolume += pWaveOut->fadeCtrl.volStep;
			else
				pWaveOut->curVolume = pWaveOut->volume;
			
			break;
			
		case FADE_STATE_OUT:	// Fade OUT			
			if (pWaveOut->curVolume > pWaveOut->fadeCtrl.volStep)
				pWaveOut->curVolume -= pWaveOut->fadeCtrl.volStep;	
			else 
				pWaveOut->curVolume = 0;

			break;
			
		default:
			Fwl_Print(C2, M_WAVE, "Fade IN/OUT ERROR");
			break;
		}

		if (pWaveOut->curVolume >= pWaveOut->volume
			|| pWaveOut->curVolume == 0)
		{
			Fwl_Print(C3, M_WAVE, "Fade %d End T: %d ,cur vol=%d\n\n"
				, pWaveOut->fadeCtrl.FadeState, get_tick_count(),pWaveOut->curVolume);
			pWaveOut->fadeCtrl.FadeState = FADE_STATE_NUL;
		}
		
	}
	
	
}

T_BOOL WaveOut_IsOpen(T_VOID)
{
	if (wavOut.bInit)
    	return AK_TRUE;

	return AK_FALSE;
}

T_BOOL WaveOut_EnDA(T_VOID)
{
	SOUND_INFO da_info;

	WaveOut_Lock();
	
	if (wavOut.bInit)
	{
		WaveOut_Unlock();
		return AK_TRUE;
	}
	
	WaveOut_Init(&wavOut);
	
	dac_drv = sound_create(SOUND_DAC, wavOut.daBufMgr.OneBufSize, wavOut.daBufMgr.DABufNum, wavOut.dataFinishedCB);
	if (!dac_drv)
	{
		Fwl_Print(C2, M_WAVE, "open sound_create fail!");
		wavOut.bInit = AK_FALSE;
		
		WaveOut_Unlock();
		
        return AK_FALSE;
	}
	
	sound_cleanbuf(dac_drv);    

	if (!sound_open(dac_drv))
	{	
		sound_delete(dac_drv);
		dac_drv = AK_NULL;
		
		WaveOut_Unlock();
		
		Fwl_Print(C3, M_WAVE, "sound_open() Failure");
		return AK_FALSE;
	}	
	
	da_info.BitsPerSample 	= 16;
    da_info.nChannel 		= 2;
    da_info.nSampleRate 	= 8000;	

    if (!sound_setinfo(dac_drv, &da_info))
    {
		sound_close(dac_drv);
		sound_delete(dac_drv);
		dac_drv = AK_NULL;
		
		WaveOut_Unlock();
		
		Fwl_Print(C2, M_WAVE, "sound_setinfo() Failure");
		return AK_FALSE;
	}
	
	analog_setsignal(INPUT_DAC, OUTPUT_HP, SIGNAL_CONNECT);	
	wavOut.bInit = AK_TRUE;

	analog_setchannel(ANALOG_HP, CHANNEL_STEREO);
	Fwl_AudioSetVolume(Fwl_GetAudioVolume());
	
	WaveOut_Unlock();
	
	return AK_TRUE;
}

T_VOID WaveOut_DisDA(T_VOID)
{
	WaveOut_Lock();
	
	if (!wavOut.bInit)
    {
		WaveOut_Unlock();
        return;
    }

	analog_setsignal(INPUT_DAC, OUTPUT_HP, SIGNAL_DISCONNECT);	

	sound_cleanbuf(dac_drv);
	sound_close(dac_drv);
	sound_delete(dac_drv);
	dac_drv = AK_NULL;

	wavOut.bInit = AK_FALSE;

	WaveOut_CloseFade();	
    WaveOut_Unlock();
}


/**
 * @brief Query Wave Out Fade In/Out Is Open?
 * @date	2011-01-04
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL
 * @retval	AK_FALSE	Fade Closed
 * @retval	AK_TRUE	Fade Opened
 */
T_BOOL WaveOut_IsFadeOpen(T_VOID)
{
	return (FADE_STATE_CLOSE != wavOut.fadeCtrl.FadeState);	
}

/**
 * @brief Disable Wave Out Fade In/Out
 * @date	2011-01-04
 * @author 	Xie_Wenzhong
 * @return 	T_VOID
 */
T_VOID WaveOut_CloseFade(T_VOID)
{
	if (FADE_STATE_CLOSE != wavOut.fadeCtrl.FadeState)
		wavOut.fadeCtrl.FadeState = FADE_STATE_CLOSE;
}

/**
 * @brief Enable Wave Out Fade In/Out
 * @date	2011-01-04
 * @author 	Xie_Wenzhong
 * @return 	T_VOID
 */
T_VOID WaveOut_OpenFade(T_VOID)
{
	if (FADE_STATE_CLOSE == wavOut.fadeCtrl.FadeState)
		wavOut.fadeCtrl.FadeState = FADE_STATE_NUL;

}


/**
 * @brief Set Fade In/Out In Audio Start/End
 * @date	2012-02-13
 * @author	Xie_Wenzhong
 * @param	time		[in]	Fade In/Out Time, ms
 * @param	fadeType[in]	Fade Type, FADE_STATE_IN / FADE_STATE_OUT
 * @return 	T_VOID	
 */
T_VOID	WaveOut_SetFade(T_U32 time, T_eFADE_STATE fadeType)
{
	if (FADE_STATE_CLOSE != wavOut.fadeCtrl.FadeState)
	{
		wavOut.fadeCtrl.FadeState	= fadeType;
		wavOut.fadeCtrl.CurNum 		= 0;
		wavOut.fadeCtrl.volStep		= 1;

		if (FADE_STATE_IN == fadeType)
			wavOut.curVolume = 0;
		
		if (1 != wavOut.PCMFmt.channel)
			wavOut.PCMFmt.channel = 2;
		
		if (wavOut.volume != 0)
		{
			T_DOUBLE rate;

			rate = wavOut.PCMFmt.sampleRate*time / (wavOut.PCMFmt.channel * 1000.0 * wavOut.volume);

			wavOut.fadeCtrl.FadeStep = (T_U16)rate;
			
			if (rate < 0.25)
				wavOut.fadeCtrl.volStep = 5;
			else if (rate < 0.33)
				wavOut.fadeCtrl.volStep = 4;
			else if (rate < 0.5)
				wavOut.fadeCtrl.volStep = 3;
			else if (rate < 0.9)
				wavOut.fadeCtrl.volStep = 2;

		}
		else
			wavOut.fadeCtrl.FadeState = FADE_STATE_NUL;

		Fwl_Print(C3, M_WAVE, "set fade %d, vol = %d, FadeStep = %d, volStep: %d, T: %d", fadeType, wavOut.volume, wavOut.fadeCtrl.FadeStep, wavOut.fadeCtrl.volStep, get_tick_count());
	}
}


T_BOOL WaveOut_Open(T_U8 DABufNum, const T_PCM_FORMAT *fmt, T_DataFinishedCB dataFinishedCB)
{
    SOUND_INFO da_info;

   	WaveOut_Lock();
	
	if (!wavOut.bInit)
	{
		WaveOut_Unlock();
		return AK_FALSE;
	}
	
    wavOut.PCMFmt = *fmt;   
	wavOut.refSampleRate = wavOut.PCMFmt.sampleRate;
  	if (!WaveOut_SetDABufMgr(&wavOut.daBufMgr, &wavOut.PCMFmt, DABufNum))
  	{
		WaveOut_Unlock();
  		return AK_FALSE;
  	}
  		
    //wavOut.volume 		= 0;
    wavOut.track 			= eSOUND_TRACK_STEREO;	// stero
    wavOut.dataFinishedCB 	= dataFinishedCB; 
    wavOut.bWriteOver 		= AK_FALSE;
    wavOut.bPlayOver  		= AK_FALSE;   
	
	if (!sound_realloc(dac_drv, wavOut.daBufMgr.OneBufSize, wavOut.daBufMgr.DABufNum, wavOut.dataFinishedCB))
	{
		WaveOut_Unlock();
		Fwl_Print(C2, M_WAVE, "sound_realloc() Failure\n");
		return AK_FALSE;
	}
	
	sound_cleanbuf(dac_drv);
	
    da_info.BitsPerSample	= fmt->sampleBits;
    da_info.nChannel		= 2;
    da_info.nSampleRate		= fmt->sampleRate;

	analog_setgain_hp(5);
	
	if (!sound_setinfo(dac_drv, &da_info))
		Fwl_Print(C2, M_WAVE, "sound_setinfo() FAILURE");

	analog_setconnect(INPUT_DAC, OUTPUT_HP, SIGNAL_CONNECT);	
	WaveOut_Unlock();
	
    return AK_TRUE;
}

T_BOOL WaveOut_Close(T_VOID)
{
	WaveOut_Lock();
	
    if (!wavOut.bInit)
    {
		WaveOut_Unlock();
		
        Fwl_Print(C2, M_WAVE, "waveout open yet!\n");
        return AK_FALSE;
    }

	sound_cleanbuf(dac_drv);

	wavOut.daBufMgr.pCurDaBuf 	= AK_NULL;
	wavOut.daBufMgr.datalen 	= 0;
    //memset(&wavOut.PCMFmt, 0, sizeof(T_PCM_FORMAT));
    //memset(&wavOut.fadeCtrl, 0, sizeof(T_FADE_CTRL));
	WaveOut_Unlock();
	
    return AK_TRUE;
}

T_BOOL WaveOut_SwitchTrack(T_VOID)
{
	if (!wavOut.bInit)
    {
        Fwl_Print(C2, M_WAVE, "waveout has not been opened.\n");
        return AK_FALSE;
    }

	wavOut.track = (wavOut.track+1) % eSOUND_TRACK_NUM;
	
	Fwl_Print(C2, M_WAVE, "Sound Switch Track");
	
	return AK_TRUE;
}

/**
 * @brief Stereo PCM Write to DA
 * @date	2012-01-29
 * @author  
 * @param	pDst	[in]	Destination PCM Buffer Address
 * @param	pSrc		[in]	Source PCM Dada Address
   @param	remLen	[in]	the Free len of current DA buf
 * @param	curLen	[in]	the un-Consumed len of input data
 * @return 	T_U16	Output PCM Data Length
 */
static T_U16 WaveOut_StereoOutput(T_S16 *pDst, T_S16 *pSrc, T_U16 remLen, T_U16 curLen)
{
	T_U16 i;
	T_U16 minlen;
	
#ifdef SUPPORT_VISUAL_AUDIO	
	T_S32* pVaData = (T_S32*)vaGetAudioBuf(VA_VisualAudio_handle);
#endif
	minlen = curLen<remLen ? curLen : remLen;
	
	for (i = 0; i < minlen>>2; i++)	// minlen/4 means 2 channel and 16bit/sample
	{
		if (FADE_STATE_IN == wavOut.fadeCtrl.FadeState
			|| FADE_STATE_OUT == wavOut.fadeCtrl.FadeState)
		{
			WaveOut_DoFade(&wavOut);
		}
		
		switch(wavOut.track)
		{					 
		case eSOUND_TRACK_STEREO:

#ifdef SUPPORT_VISUAL_AUDIO
			if(VisualAudio_IsInit())
				*pVaData++ = (T_S32)((pSrc[0] + pSrc[1]) >> 1);
#endif
			PCM_WRITE(pDst++, pSrc++);
			PCM_WRITE(pDst++, pSrc++);
			break;
			
		case eSOUND_TRACK_LEFT:	
			PCM_WRITE(pDst++, pSrc++);
			*pDst++ = 0;
			pSrc++;
			break;
			
		case eSOUND_TRACK_RIGHT:
			*pDst++ = 0;
			pSrc++;
			PCM_WRITE(pDst++, pSrc++);
			break;
			
		default:
			Fwl_Print(C2, M_WAVE, "output channel error!");
			return 0;
			break;
		}
	}

	return minlen;	
}

/**
 * @brief Mono PCM Write to DA
 * @date	2012-01-29
 * @author 
 * @param	pDst	[in]	Destination PCM Buffer Address
 * @param	pSrc		[in]	Source PCM Dada Address
   @param	remLen	[in]	the Free len of current DA buf
 * @param	curLen	[in]	the un-Consumed len of input data
 * @return 	T_U16	Output PCM Data Length
 */
static T_U16 WaveOut_MonoOutput(T_S16 *pDst, T_S16 *pSrc, T_U16 remLen, T_U16 curLen)
{
	T_U16 i;
	T_U16 minlen;

#ifdef SUPPORT_VISUAL_AUDIO	
	T_S32* pVaData = (T_S32*)vaGetAudioBuf(VA_VisualAudio_handle);
#endif

	minlen = curLen<<1 < remLen ? curLen<<1 : remLen;
		 
	for (i = 0; i < minlen>>2; i++)
	{
		if (FADE_STATE_IN == wavOut.fadeCtrl.FadeState
			|| FADE_STATE_OUT == wavOut.fadeCtrl.FadeState)
		{
			WaveOut_DoFade(&wavOut);
		}
		
		switch(wavOut.track)
		{					 
		case eSOUND_TRACK_STEREO:
#ifdef SUPPORT_VISUAL_AUDIO
			if(VisualAudio_IsInit())
				*pVaData++ = (T_S32)pSrc[0];
#endif
			PCM_WRITE(pDst++, pSrc);
			PCM_WRITE(pDst++, pSrc++);
			break;
			
		case eSOUND_TRACK_LEFT:	
			PCM_WRITE(pDst++, pSrc++);
			*pDst++ = 0;
			break;
			
		case eSOUND_TRACK_RIGHT:
			*pDst++ = 0;
			PCM_WRITE(pDst++, pSrc++);
			break;
			
		default:
			Fwl_Print(C2, M_WAVE, "output channel error!");
			return 0;
			break;
		}
	}		
	
	return minlen;
}

/**
 * @brief write data,and start up da send data by L2DA_Send_Data() depend on DA_STATE
 * @date	2012-01-04
 * @author 
 * @param	pBuf	[in]	Source PCM Buffer Address
 * @param	len		[in]	Source PCM Dada Length
 * @param	timeStamp[in]	PCM  Time Stamp, INVALID_TIME_STAMP for NOT
 * @return 	T_S32	Output PCM Data Length
 */
T_S32 WaveOut_Write(T_VOID *pBuf, T_U32 len, T_U32 timeStamp)
{
    T_U32 daFreeLen;	// the Free len of current DA buf
    T_U16 unconsmLen;	// the un-Consumed len of input data
    T_U16 writeLen;
    T_S16 *pDst 	= AK_NULL;
    T_S16 *pSrc 	= AK_NULL;
	
	AK_ASSERT_PTR(pBuf, "WAV_OUT:	pBuf Is Invalid", 0);
	
    if (0 == len || !wavOut.bInit)
    {
		Fwl_Print(C4, M_WAVE, "Invalid Data %d", len);
        return 0;
    }
	
	unconsmLen = (T_U16)len;

    //write data into da buf
	do
	{
		if (AK_NULL == wavOut.daBufMgr.pCurDaBuf)
		{
			if (!sound_getbuf(dac_drv, &wavOut.daBufMgr.pCurDaBuf, &daFreeLen))
	        {
	        	Fwl_Print(C4, M_WAVE, "write error");
				return len - unconsmLen;
			}

			wavOut.daBufMgr.datalen = wavOut.daBufMgr.OneBufSize - (T_U16)daFreeLen;
		}
		else
			daFreeLen = wavOut.daBufMgr.OneBufSize - wavOut.daBufMgr.datalen;
		
		pDst = (T_S16*)(wavOut.daBufMgr.pCurDaBuf + wavOut.daBufMgr.datalen);
		pSrc = (T_S16*)((T_U8*)pBuf + len - unconsmLen);		
		
		if (2 == wavOut.PCMFmt.channel)
		{
			// stereo
			writeLen = WaveOut_StereoOutput(pDst, pSrc, (T_U16)daFreeLen, unconsmLen);
			unconsmLen -= writeLen;
		}
		else
		{	
			// mono
			writeLen = WaveOut_MonoOutput(pDst, pSrc, (T_U16)daFreeLen, unconsmLen);
			unconsmLen -= writeLen>>1;
		}

		wavOut.daBufMgr.datalen += writeLen;
		
		if (wavOut.daBufMgr.datalen >= wavOut.daBufMgr.OneBufSize)
		{
#ifdef SUPPORT_VISUAL_AUDIO	
			if(VisualAudio_IsInit())
				bVAShow_data_ready = AK_TRUE;
#endif
			sound_endbuf(dac_drv, wavOut.daBufMgr.datalen);

			wavOut.totalAudioLen += 1;			
	        wavOut.daBufMgr.pCurDaBuf = AK_NULL;
		}
	}while(unconsmLen != 0);

    return len - unconsmLen;
}

/**
 * @brief Set Refrence Sample Rate For Calculate Current Time When Tempo Filter
 * @date	2008-04-10
 * @author 	Xie_Wenzhong
 * @param	sampleRate	[in]	Sample Rate After Audio Tempo Filter
 * @return 	T_VOID
 */
T_VOID WaveOut_SetRefSampRate(T_U32 sampleRate, T_U32 curTime)
{
	wavOut.refSampleRate = sampleRate;
	WaveOut_SetStatus(&curTime, WAVEOUT_CURRENT_TIME);
}


/*
 *get state according to state type
 */
T_BOOL WaveOut_GetStatus(T_VOID *pStatus, T_eWAVEOUT_STATUS type)
{
    if (!wavOut.bInit)
    {
        Fwl_Print(C2, M_WAVE, "waveout has not been opened");
        return AK_FALSE;
    }

    switch (type)
    {
    case WAVEOUT_REMAIN_DATA:
        *((T_U32*)pStatus) = wavOut.daBufMgr.DABufNum*wavOut.daBufMgr.OneBufSize - DABufGetFreeLen();
        break;
        
    case WAVEOUT_SPACE_SIZE:
        *((T_U32*)pStatus) = DABufGetFreeLen();
        break;
        
    case WAVEOUT_PLAY_OVER:
		if (wavOut.bWriteOver
            && wavOut.daBufMgr.daState == eDA_STATE_STOP        
        	&& DABufGetFreeLen() == wavOut.daBufMgr.DABufNum * wavOut.daBufMgr.OneBufSize)	// has no data in DA buf
        {
			wavOut.bPlayOver = AK_TRUE;
            if (wavOut.dataFinishedCB != AK_NULL)
            {
                wavOut.dataFinishedCB();
            }
        }
        
		
        *((T_BOOL*)pStatus) = wavOut.bPlayOver;
        break;

    case WAVEOUT_CURRENT_TIME:			
		{
			T_U32 fullBufCount = 0;

			fullBufCount = sound_getnum_fullbuf(dac_drv);
			wavOut.totalAudioLen = (wavOut.totalAudioLen>fullBufCount) ? wavOut.totalAudioLen : fullBufCount;
			*((T_U32*)pStatus) = (T_U32)(250.0*(wavOut.totalAudioLen-fullBufCount)*wavOut.daBufMgr.OneBufSize
										/(wavOut.refSampleRate));
			 // AK_DEBUG_OUTPUT("DA BUF SIZE: %d\n",wavOut.daBufMgr.OneBufSize);
		}
			/*
            //calculate the current time from begin
            *((T_U32*)pStatus) = (T_U32)(250.0*wavOut.totalAudioLen*wavOut.daBufMgr.OneBufSize
                                /(wavOut.PCMFmt.sampleRate));
            //AK_DEBUG_OUTPUT("wavOut.curTime=%d\n",*((T_U32*)pStatus));
       	*/
        break;

	case WAVEOUT_ONE_BUFSIZE:
        *((T_U32*)pStatus) = wavOut.daBufMgr.OneBufSize;
        break;

	case WAVEOUT_FORMAT:
		memcpy((void *)pStatus, (void *)&wavOut.PCMFmt, sizeof(T_PCM_FORMAT));
        break;
      
    default:
        break;
    }

    return AK_TRUE;   
}

/*
 *set state according to state type
 */
T_BOOL WaveOut_SetStatus(const T_VOID *pStatus, T_eWAVEOUT_STATUS type)
{
    if (!wavOut.bInit)
    {
        Fwl_Print(C2, M_WAVE, "waveout has not been opened.\n");
        return AK_FALSE;
    }

    switch (type)
    {
    case WAVEOUT_VOLUME:
        wavOut.volume = *((T_U16*)pStatus);

		//在fade状态，不允许设置音量
		if (!(FADE_STATE_IN == wavOut.fadeCtrl.FadeState
			|| FADE_STATE_OUT == wavOut.fadeCtrl.FadeState))
			wavOut.curVolume = wavOut.volume;
		break;   
        
    case WAVEOUT_TRACK:
        wavOut.track = *((T_U16*)pStatus);
        break;
        
    case WAVEOUT_WRITE_OVER:            
        wavOut.bWriteOver = *((T_BOOL*)pStatus);
        //AK_DEBUG_OUTPUT("write over=%d\n",wavOut.bWriteOver);
        if (wavOut.bWriteOver
            && wavOut.daBufMgr.daState == eDA_STATE_STOP        
        	&& DABufGetFreeLen() == wavOut.daBufMgr.DABufNum * wavOut.daBufMgr.OneBufSize)	// has no data in DA buf
        {
			wavOut.bPlayOver = AK_TRUE;
            if (wavOut.dataFinishedCB != AK_NULL)
            {
                wavOut.dataFinishedCB();
            }
        }
                     
        break;
        
    case WAVEOUT_CURRENT_TIME:
        if (wavOut.daBufMgr.OneBufSize != 0)
        {
            wavOut.totalAudioLen = (T_U32)((*((T_U32*)pStatus))*(T_U64)wavOut.refSampleRate
                                /(250*wavOut.daBufMgr.OneBufSize));
        }
        break;

    case WAVEOUT_TOTAL_BUFSIZE:
        if (*((T_U32*)pStatus)/wavOut.daBufMgr.OneBufSize > DA_BUF_MAX_NUM)
        {
            Fwl_Print(C2, M_WAVE, "DA buf num can not be bigger than %d", DA_BUF_MAX_NUM);
            return AK_FALSE;
        }
        
        if (*((T_U32*)pStatus)/wavOut.daBufMgr.OneBufSize != wavOut.daBufMgr.DABufNum)
        {
			DABufRemalloc(wavOut.daBufMgr.OneBufSize, *((T_U32*)pStatus)/wavOut.daBufMgr.OneBufSize);
        }
        break;

    default:
        break;
    
    }
    
    return AK_TRUE; 
}

T_VOID WaveOut_CloseDac(T_VOID)
{
	if (AK_NULL != dac_drv)
	{
		sound_cleanbuf(dac_drv);
	}
}

T_VOID WaveOut_ResetBuf(T_VOID)
{
    //sometimes this fun is loaded before waveOutOpen is loaded by other thread
    //so must judge if wave_mutex is initialized.
    if (!wavOut.bInit)
    {
        Fwl_Print(C2, M_WAVE, "WaveOut_ResetBuf: waveout not open yet!");
        return;
    }
	
    sound_cleanbuf(dac_drv);

	wavOut.daBufMgr.pCurDaBuf 	= AK_NULL;
	wavOut.daBufMgr.datalen 	= 0;
}

T_VOID WaveOut_CleanBuf()
{
    //sometimes this fun is loaded before waveOutOpen is loaded by other thread
    //so must judge if wave_mutex is initialized.
    if (!wavOut.bInit)
    {
        Fwl_Print(C2, M_WAVE, "waveout not open yet!");
        return;
    }

	wavOut.daBufMgr.pCurDaBuf 	= AK_NULL;
	wavOut.daBufMgr.datalen 	= 0;
}

static T_U16 DABufGetFreeLen(T_VOID)
{
	T_VOID *pDAbuf 		= AK_NULL;
	T_U32 DAbufLen 		= 0;
	T_U16 fullBufCount 	= 0;
	T_U16 FreeLen 		= 0;

	fullBufCount = (T_U16)sound_getnum_fullbuf(dac_drv);	

	if (!sound_getbuf(dac_drv, &pDAbuf, &DAbufLen))
    {
		DAbufLen = 0;
	}

	FreeLen = (wavOut.daBufMgr.DABufNum - fullBufCount - 1) * wavOut.daBufMgr.OneBufSize + (T_U16)DAbufLen;

	return FreeLen;
}

static T_BOOL DABufRemalloc(T_U16 OneBufSize, T_U32 DABufNum)
{
	if (!wavOut.bInit)
	{
		return AK_FALSE;
	}

	wavOut.daBufMgr.DABufNum 	= (T_U8)DABufNum;
	wavOut.daBufMgr.OneBufSize 	= OneBufSize;

	sound_realloc(dac_drv, wavOut.daBufMgr.OneBufSize, wavOut.daBufMgr.DABufNum, wavOut.dataFinishedCB);

	if (wavOut.dataFinishedCB != AK_NULL)
    {
    	wavOut.dataFinishedCB();
    }

    sound_cleanbuf(dac_drv);

	wavOut.daBufMgr.pCurDaBuf = AK_NULL;
	wavOut.daBufMgr.datalen = 0;

	return AK_TRUE;
}


#ifdef OS_WIN32

/**
 * @brief   get buffer address and buffer len, which can be used to fill or retrieve sound data
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @param[out] pbuf return buffer address or AK_NULL
 * @param[out] len return buffer len or 0
 * @return  T_BOOL
 * @retval  AK_TRUE  get buffer successful
 * @retval  AK_FALSE get buffer failed
 * @note    if sound_create failed or no buffer to return, it will return failed
 */
T_BOOL sound_getbuf (T_SOUND_DRV *handler, T_VOID **pbuf, T_U32 *len)
{
	return AK_FALSE;
}

/**
 * @brief set one buffer end\n
 * after call sound_getbuf and finish the operation of sound data,call this function
 * @author    LianGenhui
 * @date     2010-06-30 
 * @param[in] handler handler of the sound device  
 * @param[in] len     buffer len(use for write)
 * @return  T_BOOL
 * @retval  AK_TRUE  successful
 * @retval  AK_FALSE longer than one buffer's len
 */
T_BOOL sound_endbuf(T_SOUND_DRV *handler, T_U32 len)
{
	return AK_FALSE;
}

/**
 * @brief   clean sound buffer
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  T_VOID
 */
T_VOID sound_cleanbuf (T_SOUND_DRV *handler)
{
	return;
}

/**
 * @brief   create a sound driver, it will malloc sound buffer and init L2\n
 *          the callback function will be called when a read or write buffer complete, it can be AK_NULL and do nothing.
 * @author    LianGenhui
 * @date    2010-06-30
 * @param[in]  driver sound driver, refer to SOUND_DRIVER
 * @param[in]  OneBufSize buffer size
 * @param[in]  DABufNum buffer number
 * @param[in]  callback callback function or AK_NULL 
 * @return  T_SOUND_DRV
 * @retval  AK_NULL created failed
 */
T_SOUND_DRV *sound_create (SOUND_DRIVER driver, T_U32 OneBufSize, T_U32 DABufNum, T_fSOUND callback)
{
	return AK_NULL;
}
/**
 * @brief  realloc buffer for giving sound driver
 * @author    liao_zhijun
 * @date    2010-11-02
 * @param[in] handler handler of the sound device
 * @param[in]  OneBufSize buffer size
 * @param[in]  DABufNum buffer number
 * @param[in]  callback callback function or AK_NULL 
 * @return  T_BOOL
 * @retval  AK_TRUE  realloc successful
 * @retval  AK_FALSE realloc failed
 */
T_BOOL sound_realloc (T_SOUND_DRV *handler,T_U32 OneBufSize, T_U32 DABufNum, T_fSOUND callback)
{
	return AK_TRUE;
}


/**
 * @brief   Set sound sample rate, channel, bits per sample of the sound device
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @param[in] info     refer to SOUND_INFO
 * @return  T_BOOL
 * @retval  AK_TRUE set successful
 * @retval  AK_FALSE set failed
 */
T_BOOL sound_setinfo (T_SOUND_DRV *handler, SOUND_INFO *info)
{
	return AK_TRUE;
}

/**
 * @brief  delete sound driver and Free sound buffer
 * @author LianGenhui
 * @date   2010-06-30
 * @param[in] handler handler of the sound device
 */
T_VOID sound_delete (T_SOUND_DRV *handler)
{
	return;
}

/**
 * @brief  open a sound device and it can be used
 * @author LianGenhui
 * @date   2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  T_BOOL
 * @retval  AK_TRUE  open successful
 * @retval  AK_FALSE open failed
 */
T_BOOL sound_open (T_SOUND_DRV *handler)
{
	return AK_TRUE;
}

/**
 * @brief   Close a sound device
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  T_BOOL
 * @retval  AK_TRUE close successful
 * @retval  AK_FALSE close failed
 */
T_BOOL sound_close (T_SOUND_DRV *handler)
{
	return AK_TRUE;
}

/**
 * @brief get the number of buffers which have been filled or retrieved sound data 
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] handler handler of the sound device  
 * @return  T_U32
 * @retval  value the value will from 0 to the number when create a sound set 
  */
T_U32 sound_getnum_fullbuf(T_SOUND_DRV *handler)
{
	return 0;
}

/**
 * @brief   Set headphone gain,available for aspen3s later
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] gain for normal mode, 
            AK98XX: must be 0~8.0 for mute,1~8 for 0.1 time to 0.8 time
            AK37XX: must be 0~5, 5 is max gain
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL analog_setgain_hp (T_U8 gain)
{
	return AK_TRUE;
}

/**
 * @brief   connect or disconnect the signal between input and output signal. 
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] analog_in refer to ANALOG_SIGNAL_INPUT
 * @param[in] analog_out refer to ANALOG_SIGNAL_OUTPUT
 * @param[in] state SIGNAL_OPEN or SIGNAL_CLOSE
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL analog_setsignal(ANALOG_SIGNAL_INPUT analog_in, ANALOG_SIGNAL_OUTPUT analog_out, ANALOG_SIGNAL_STATE state)
{
	return AK_TRUE;
}
/**
 * @brief   connect or disconnect the signal between input and output signal. 
 * @author  LHD
 * @date    2011-08-16
 * @param[in] analog_in refer to ANALOG_SIGNAL_INPUT
 * @param[in] analog_out refer to ANALOG_SIGNAL_OUTPUT
 * @param[in] state SIGNAL_OPEN or SIGNAL_CLOSE
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL analog_setconnect(ANALOG_SIGNAL_INPUT analog_in, ANALOG_SIGNAL_OUTPUT analog_out,ANALOG_SIGNAL_STATE state)
{
	return AK_TRUE;
}

/**
 * @brief   set analog module channel to be MONO or STEREO
 * @author  LianGenhui
 * @date    2010-06-30
 * @param[in] module refer to ANALOG_CHANNEL
 * @param[in] state CHANNEL_MONO or CHANNEL_STEREO
 * @return  T_BOOL
 * @retval  AK_TRUE  operation successful
 * @retval  AK_FALSE operation failed
 */
T_BOOL analog_setchannel(ANALOG_CHANNEL module, ANALOG_CHANNEL_STATE state)
{
	return AK_TRUE;
}

#endif // OS_WIN32


