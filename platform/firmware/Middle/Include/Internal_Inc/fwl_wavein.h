/*
*file:waveout.c
*description:pcm input interface
*author:GB
*date:2009-2-24
*/
#ifndef _WAVE_IN_H__
#define _WAVE_IN_H__

#include "anyka_types.h"


#define RECBUF_POINTER_ARRAY_SIZE 60
#define ONE_RECBUF_SIZE (4*1024)

#include "fwl_wave.h"
//typedef struct
//{
//    T_U32 sampleRate;
//    T_U32 sampleBits;
//    T_U32 channel;
//}T_WAVE_IN_FORMAT;
//typedef enum
//{
//    eINPUT_SOURCE_MIC,
//    eINPUT_SOURCE_LINEIN,
//    eINPUT_SOURCE_UNKNOW
//}T_eINPUT_SOURCE;
//typedef struct
//{
//    T_WAVE_IN_FORMAT waveInFmt;
//    T_U32 volume;
//    T_eINPUT_SOURCE inputSrc;
//}T_WAVE_IN;

//xuyr Swd300000069
typedef enum
{
	WAVEIN_SET_COUNT,
	WAVEIN_BUF_COUNT,
	WAVEIN_CUR_TIME
}T_eWAVEIN_STATUS;

/*
*brief:waveIn init function
*param:T_VOID
*retval:T_BOOL
*note:before using others wavein interface this func retval must be AK_TRUE 
*/
T_BOOL waveInInit(T_VOID);

/*
*brief:waveIn open function
*param[out]:ppHandle
*param[in]:desired wave format
*retval:AK_TRUE indicate open success,or failed
*/
T_BOOL waveInOpen(T_pVOID *ppHandle, T_WAVE_IN *fmt);

/*
*brief:waveIn read pcm data function
*param[in]:pHandle
*param[out]:ppBuf,pcm data addr
*param[out]:pBufLen,buffer len
*retval:AK_TRUE indicate read successed,or failed
*/
T_BOOL waveInRead(T_pVOID pHandle, T_U8 **ppBuf, T_U32 *pBufLen);

/*
*brief:waveIn set status
*param[in]:pHandle
*param[in]: type to set
*param[in]: value to set
*retval:AK_TRUE indicate successed,or failed
*/
T_BOOL waveInSetStatus(T_pVOID pHandle,T_eWAVEIN_STATUS type,const T_VOID *pStatus);//xuyr Swd300000069
/*
*brief:waveIn get status
*param[in]:pHandle
*param[in]: type to get
*param[in]: address to put value in
*retval:AK_TRUE indicate successed,or failed
*/
T_BOOL waveInGetStatus(T_pVOID pHandle,T_eWAVEIN_STATUS type,T_VOID *pStatus);//xuyr Swd300000069
	
/*
*brief:waveIn close function
*param[in]:pHandle
*retval:AK_TRUE indicate close  success,or failed
*/
T_BOOL waveInClose(T_pVOID pHandle);

/*
*brief:waveIn setvolume function
*param[in]:pHandle
*param[in]:volume 
*retval:AK_TRUE indicate setvolume  success,or failed
*/
T_BOOL waveInSetVolume(T_pVOID pHandle, T_U32 volume);

/*
*brief:waveIn getvolume function
*param[in]:pHandle
*retval:volume value
*/
T_U32 waveInGetVolume(T_pVOID pHandle);

/*
*brief:waveIn destroy function
*param: T_VOID
*retval:AK_TRUE indicate setInput  success,or failed
*/
T_BOOL waveInDestroy(T_VOID);

#endif
