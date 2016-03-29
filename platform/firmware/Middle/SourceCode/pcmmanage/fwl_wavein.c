/*
*file:waveout.c
*description:pcm input interface
*author:GB
*date:2009-2-24
*/
#include "fwl_wavein.h"
#include "Eng_Debug.h"
#include "fwl_osfs.h"
#include "fwl_osMalloc.h"
#include "hal_sound.h"
#include "arch_analog.h"

T_SOUND_DRV *adc_drv = AK_NULL;

static T_BOOL  waveInADOpenFlg = AK_FALSE;//xuyr Swd300000079

T_BOOL waveInSetStatus(T_pVOID pHandle,T_eWAVEIN_STATUS type,const T_VOID *pStatus)
{
    if (pHandle)
    {
		T_WAVE_IN *phandle = (T_WAVE_IN *)pHandle;
		switch(type)
		{
		case WAVEIN_SET_COUNT:
			phandle->waveInBufCount = *((T_U32*)pStatus);
			break;
		case WAVEIN_BUF_COUNT:
			phandle->waveInBufCount+=(*((T_U32*)pStatus))/phandle->OneBufSize;
			break;
		default:
			return AK_FALSE;
		}
	}
	return AK_TRUE;
}
T_BOOL waveInGetStatus(T_pVOID pHandle,T_eWAVEIN_STATUS type,T_VOID *pStatus)
{
    if (pHandle)
    {
		T_WAVE_IN *phandle = (T_WAVE_IN *)pHandle;
		switch(type)
		{
		case WAVEIN_CUR_TIME:
#ifdef OS_ANYKA
			*((T_U32*)pStatus)= 500*((T_U64)phandle->waveInBufCount*phandle->OneBufSize)/(phandle->waveInFmt.channel*phandle->waveInFmt.sampleRate);
#endif
			break;

		default:
			return AK_FALSE;
		}
	}
	return AK_TRUE;
}

T_BOOL waveInInit(T_VOID)
{
    return AK_TRUE;
}

T_BOOL waveInOpen(T_pVOID *ppHandle, T_WAVE_IN *fmt)
{
	SOUND_INFO pcmInfo;
	T_BOOL	ret = AK_FALSE;
	
	if (ppHandle && fmt)
	{
		*ppHandle = (T_WAVE_IN*)Fwl_Malloc(sizeof(T_WAVE_IN));
		AK_ASSERT_PTR(*ppHandle, "malloc waveIn handle failed\n", AK_FALSE);

		((T_WAVE_IN *)(*ppHandle))->waveInFmt = fmt->waveInFmt;
		((T_WAVE_IN *)(*ppHandle))->inputSrc = fmt->inputSrc;
		((T_WAVE_IN *)(*ppHandle))->volume = fmt->volume;
		((T_WAVE_IN *)(*ppHandle))->waveInBufCount=0;
		//37只支持单声道
		if (0 != fmt->OneBufSize)
		{
			((T_WAVE_IN *)(*ppHandle))->OneBufSize = fmt->OneBufSize;
		}
		else
		{
			switch(fmt->waveInFmt.sampleRate)
			{
				case 8000:
					((T_WAVE_IN *)(*ppHandle))->OneBufSize = 512;
					break;
					
				case 11025:
					((T_WAVE_IN *)(*ppHandle))->OneBufSize = 768;
					break;
					
				case 16000:
					((T_WAVE_IN *)(*ppHandle))->OneBufSize = 1024;
					break;
					
				case 22050:
					((T_WAVE_IN *)(*ppHandle))->OneBufSize = 1280;
					break;
					
				case 24000:
					((T_WAVE_IN *)(*ppHandle))->OneBufSize = 1536;
					break;
					
				case 32000:
					((T_WAVE_IN *)(*ppHandle))->OneBufSize = 2048;			  
					break;
					
				case 44100:
					((T_WAVE_IN *)(*ppHandle))->OneBufSize = 2560;
					break;
					
				case 48000:
					((T_WAVE_IN *)(*ppHandle))->OneBufSize = 2816;
					break;
			
				default:
					((T_WAVE_IN *)(*ppHandle))->OneBufSize = fmt->waveInFmt.sampleRate
												 * 2//(waveOut.PCMFmt.sampleBits>>3)
												 //* 2//channel equal to 1
												 * 3/100;//30ms/1000ms
												 
					//should divide exactly 512 at present temporarily
					((T_WAVE_IN *)(*ppHandle))->OneBufSize = (((T_WAVE_IN *)(*ppHandle))->OneBufSize>>9)<<9;
					break;
			
			}

		}

		/*
		if (1 == ((T_WAVE_IN *)(*ppHandle))->waveInFmt.channel)
		{
			((T_WAVE_IN *)(*ppHandle))->OneBufSize=ONE_RECBUF_SIZE/2;
		}
		else
		{
			((T_WAVE_IN *)(*ppHandle))->OneBufSize=ONE_RECBUF_SIZE;
		}
		*/
		AK_DEBUG_OUTPUT("sound_open ,OneBufSize = %d\n", ((T_WAVE_IN *)(*ppHandle))->OneBufSize);
		
		pcmInfo.nSampleRate = fmt->waveInFmt.sampleRate;
		pcmInfo.nChannel = (T_U16)fmt->waveInFmt.channel;
		pcmInfo.BitsPerSample = (T_U16)fmt->waveInFmt.sampleBits; 
#ifdef OS_ANYKA

		adc_drv = sound_create(SOUND_ADC, ((T_WAVE_IN *)(*ppHandle))->OneBufSize, RECBUF_POINTER_ARRAY_SIZE, AK_NULL);

		AK_DEBUG_OUTPUT("sound_create :adc_drv:%d\n",adc_drv);
		
		sound_cleanbuf(adc_drv);

		ret = sound_open(adc_drv);
		AK_DEBUG_OUTPUT("sound_open ,ret = %d\n", ret); 	
		
//		analog_setsignal(INPUT_ALL, OUTPUT_ALL, SIGNAL_DISCONNECT);

		switch(fmt->inputSrc)
		{
		case eINPUT_SOURCE_MIC:
			analog_setgain_mic(6);
			analog_setsignal(INPUT_MIC, OUTPUT_ADC, SIGNAL_CONNECT);
			analog_setchannel(ANALOG_MIC, fmt->waveInFmt.channel-1);
			break;

		case eINPUT_SOURCE_LINEIN: 
			analog_setgain_linein(3);
			analog_setsignal(INPUT_LINEIN, OUTPUT_ADC, SIGNAL_CONNECT);
			analog_setchannel(ANALOG_LINEIN, fmt->waveInFmt.channel-1);
			break;
			
		default:
			AK_DEBUG_OUTPUT("wavein input source is error\n");
			return AK_FALSE;
		}

		ret = sound_setinfo(adc_drv, &pcmInfo);
		AK_DEBUG_OUTPUT("sound_setinfo ,ret = %d\n", ret);
		
		waveInADOpenFlg = AK_TRUE;
		
		AK_DEBUG_OUTPUT("wave in open ok\n");
#endif
		return AK_TRUE;
	}

	return AK_FALSE;
}


T_BOOL waveInSetVolume(T_pVOID pHandle, T_U32 volume)
{
	T_WAVE_IN *phandle = (T_WAVE_IN *)pHandle;
    if (phandle)
    {
        phandle->volume = (T_S32)volume;
        return AK_TRUE;
    }

    return AK_FALSE;
}

T_U32 waveInGetVolume(T_pVOID pHandle)
{
	T_WAVE_IN *phandle = (T_WAVE_IN *)pHandle;
    if (phandle)
    {
        return phandle->volume;
    }

    return 0;
}
/*
 *note:change pcm sample value for changing volume , and filter channel
 */
static T_BOOL waveInDataConvert(T_pVOID pHandle, T_VOID * const pBuf, T_U32 bufLen)
{
#define AD_REFERCE   (1024)
    T_U32  i=0;
    T_S16 *pTmpBuf=AK_NULL;
    T_U32 nSample = 0;
	T_WAVE_IN *phandle = (T_WAVE_IN *)pHandle;
    
    if (phandle)
    {
    	if (AD_REFERCE == (phandle->volume))
    	{
			return AK_TRUE;
		}
#if 0
        pTmpBuf = (T_S16*)pBuf;
		if (2 == phandle->waveInFmt.channel)
		{
			nSample = bufLen/2;
		}
		else
		{
			T_S16 *p = AK_NULL;
			T_S16 *q = AK_NULL;
		/*channel=1,remove right channel,
		(because of mono channel is that set left channel,and left channel contain validate data,but right channel	zero )*/
			p = pTmpBuf + 1;
			q = pTmpBuf + 2;
			for (i=0; i<bufLen/4-1; i++)
			{
				*p++ = *q;
				q += 2;
			}
			nSample = bufLen/4;
		}
#endif
        pTmpBuf = (T_S16*)pBuf;
        nSample = bufLen>>1;
        
        /*change sample value for changing volume*/
        for (i=0; i<nSample; i++)
        {
        	T_S32 tempv;
			tempv = (T_S32)((*pTmpBuf) *(phandle->volume)/AD_REFERCE);
			if (tempv>32767)
			{
				tempv = 32767;
			}
			if (tempv<-32768)
			{
				tempv = -32768;
			}
		    *pTmpBuf = (T_S16)tempv;
		    pTmpBuf ++;
        }
        return AK_TRUE;        
    }

    return AK_FALSE;
}

T_BOOL waveInRead(T_pVOID pHandle, T_U8 **ppBuf, T_U32 *pBufLen)
{
    T_WAVE_IN *phandle = (T_WAVE_IN *)pHandle;

#ifdef OS_ANYKA
    if (phandle && ppBuf && pBufLen)
    {
        switch(phandle->inputSrc)
        {
        case eINPUT_SOURCE_MIC:
        case eINPUT_SOURCE_LINEIN:
			if (waveInADOpenFlg)
			{
	            if (sound_getbuf(adc_drv, ppBuf, pBufLen))
	            {
	                //AK_DEBUG_OUTPUT("wave in read data bufLen=%d\n", bufLen);

					
			        waveInDataConvert(phandle, *ppBuf, *pBufLen);
					/*

			        if (1 == phandle->waveInFmt.channel)
			        {	                        
			            *pBufLen = phandle->OneBufSize;
			        }
			        */

					sound_endbuf(adc_drv, *pBufLen);
	                
	                return AK_TRUE;
	            }
			}
			else
			{
				*pBufLen = 0;
				return AK_FALSE;
			}
            break;
            
        default:
            AK_DEBUG_OUTPUT("wavein input source is error\n");
            break;
        }
    }
#endif
    return AK_FALSE;
}


T_BOOL waveInClose(T_pVOID pHandle)
{
	T_WAVE_IN *phandle = (T_WAVE_IN *)pHandle;
	
    if (phandle)
    {
#ifdef OS_ANYKA
        switch(phandle->inputSrc)
        {
        case eINPUT_SOURCE_MIC:
			analog_setsignal(INPUT_MIC, OUTPUT_ADC, SIGNAL_DISCONNECT);
            break;

        case eINPUT_SOURCE_LINEIN:
			analog_setsignal(INPUT_LINEIN, OUTPUT_ADC, SIGNAL_DISCONNECT);
            break;
            
        default:
            AK_DEBUG_OUTPUT("wavein input source is error\n");
            break;
        }


		sound_cleanbuf(adc_drv);
		sound_close(adc_drv);
		waveInADOpenFlg = AK_FALSE;

		sound_delete(adc_drv);
		
		AK_DEBUG_OUTPUT("sound_delete :adc_drv:%d\n",adc_drv);
#endif

        Fwl_Free(phandle);
        phandle = AK_NULL;
        AK_DEBUG_OUTPUT("wave in close success\n");
        return AK_TRUE;
    }

    AK_DEBUG_OUTPUT("wave in close failed\n");
    return AK_FALSE;
}   

T_BOOL waveInDestroy(T_VOID)
{
    return AK_TRUE;
}


