#ifndef _LOG_REC_AUDIO_H_
#define _LOG_REC_AUDIO_H_

#include "Anyka_types.h"
#include "fwl_osfs.h"
#include "gbl_macrodef.h"

#include "Lib_Media_Global.h"
#include "Lib_Media_struct.h"
#include "Lib_Sdcodec.h"
#include "log_MediaEncode.h"
#include "fwl_wave.h"

/* encode time interval once*/
#define ENCODE_INTERVAL_MS  20

/*encode out buffer size*/
#define ENCODE_OUTBUF_SIZE  (10*1024)

/*alert data len,should less than ENCODE_OUTBUF_SIZE*/
#define ENCODE_ALERT_DATALEN (ENCODE_OUTBUF_SIZE - ONE_RECBUF_SIZE)

/*send pcm data len by one time*/
#define PCM_ONCE_SEND_LEN       (4*1024)

/*encode type*/
typedef enum{
    T_ENCODE_TYPE_AUDIO,
    T_ENCODE_TYPE_VIDEO
}T_ENCODE_TYPE;

typedef enum{
    eAUDIO_REC_STATE_INIT,
    eAUDIO_REC_STATE_RECORDING,
    eAUDIO_REC_STATE_STOP
}T_eAUDIO_REC_STATE;

typedef struct
{
    T_U8 *pBuf;//buf start addr
    T_U32 dataLen;//data len
    T_U32 bufSize;//buf total len
}T_ENCODE_OUTBUF;

typedef struct{
    T_pFILE         hFile;//record file handle
    T_pVOID         hMedia;//encode handle
    T_pVOID			pWaveIn;//get pcm handle 
    T_U32           curTime;//current record time(ms)
    T_eAUDIO_REC_STATE audEncState;//encode state
    T_AUDIO_REC_INPUT audEncInInfo;//encode info
    T_AUDIO_ENC_OUT_INFO audEndOutInfo;//encode format info
    T_AUDIO_ENC_BUF_STRC audEncBuf;//encode buffer info
    T_ENCODE_OUTBUF OutBuf;//encode out buffer
    T_BOOL          bEncodeInited;//encode if initialized    
    T_BOOL          bEncode;//if need to encode
    T_S32    		stopMutex;//stop mutex T_hSemaphore     
    T_S16			sample[2];
#ifdef SUPPORT_AUDIOREC_DENOICE
	T_pVOID         pAudFilter;//audio filter
#endif
}T_MEncoder;

typedef struct{
    T_ENCODE_TYPE enc_type;
    
    union{
        //use for audio encode
        T_AUDIO_REC_INPUT rec_input;

        //use for video encode
        T_MEDIALIB_INIT_INPUT media_input;
        T_MEDIALIB_INIT_CB media_cb;
    }enc_input;
    
}T_ENCODE_INPUT;


/*
*brief:Media Encoder init
*param1:pMEncoder[in],encode handle
*param2:recMode[in],record file format
*param3:sampleRate[in],record sample speed
*retval:AK_TRUE is success,or failed
*note:call this func before all other record func is called,and call only once before record
*/
T_BOOL MEncoder_Init(T_MEncoder *pMEncoder, T_eREC_MODE audioType, T_U32 sampleRate);

/*
*brief:Media Encoder start
*param1:pMEncoder[in],handle
*param2:hFile[in],encode file handle
*param3:input_src[in],record object
*retval:AK_TRUE is successed,or failed
*/
T_BOOL MEncoder_Start(T_MEncoder *pMEncoder, T_hFILE rec_fd, T_eINPUT_SOURCE input_src);


/*
*brief:Media Encoder stop
*param1:pMEncoder[in],encode handle
*retval:AK_TRUE is successed , or failed
*/
T_BOOL MEncoder_Stop(T_MEncoder* pMEncoder);

/*
*brief:Media Encoder Destroy for release some resource
*param1:pMEncoder[in],encode handle
*retval:AK_TRUE is successed, or failed
*note:match MEncoder_Init func,call this func after exiting record 
*/
T_BOOL MEncoder_Destroy(T_MEncoder* pMEncoder);

/*
*brief:Media Encoder encode process
*param1:pMEncoder[in],encode handle
*param2:bError[out],echo if error occur  
*retval:reture written to file data len 
*/
T_S32 MEncoder_HandleAudioEncode(T_MEncoder* pMEncoder, T_BOOL *bError);

/**
*@brief get current sample
*@author zhanglong
*@param
*@return AK_TURE
*@note sample number must be 2
*/
T_BOOL MEncoder_GetCurSample(T_MEncoder* pMEncoder, T_S16 sample[2]);



T_U32 MEncoder_GetCurTime(T_MEncoder* pMEncoder);


#endif	// _LOG_REC_AUDIO_H_

