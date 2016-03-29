/*
*file:waveout.c
*description:pcm output interface
*author:GB
*date:2009-1-22
*/
#ifndef _WAVE_OUT_H__
#define _WAVE_OUT_H__

#include "fwl_wave.h"

//T_BOOL waveOutInit(T_VOID);

//T_BOOL waveOutUninit(T_VOID);

/**
 * @brief Set Refrence Sample Rate For Calculate Current Time When Tempo Filter
 * @date	2008-04-10
 * @author Xie_Wenzhong
 * @param	sampleRate	[in]	Sample Rate After Audio Tempo Filter
 * @return 	T_VOID
 */
T_VOID WaveOut_SetRefSampRate(T_U32 sampleRate, T_U32 curTime);

/**
 * @brief Query Wave Out Fade In/Out Is Open?
 * @date	2011-01-04
 * @author 	Xie_Wenzhong
 * @return 	T_BOOL
 * @retval	AK_FALSE	Fade Closed
 * @retval	AK_TRUE	Fade Opened
 */
T_BOOL WaveOut_IsFadeOpen(T_VOID);

/**
 * @brief Disable Wave Out Fade In/Out
 * @date	2011-01-04
 * @author 	Xie_Wenzhong
 * @return 	T_VOID
 */
T_VOID WaveOut_CloseFade(T_VOID);

/**
 * @brief Enable Wave Out Fade In/Out
 * @date	2011-01-04
 * @author 	Xie_Wenzhong
 * @return 	T_VOID
 */
T_VOID WaveOut_OpenFade(T_VOID);

T_BOOL WaveOut_EnDA(T_VOID);
T_VOID WaveOut_DisDA(T_VOID);

T_BOOL WaveOut_Open(T_U8 DABufNum, const T_PCM_FORMAT *fmt, T_DataFinishedCB dataFinishedCB);

T_S32 WaveOut_Write(T_VOID *pBuf, T_U32 len, T_U32 timeStamp);

T_BOOL WaveOut_GetStatus(T_VOID *pStatus, T_eWAVEOUT_STATUS type);

T_BOOL WaveOut_SetStatus(const T_VOID *pStatus, T_eWAVEOUT_STATUS type);

T_VOID WaveOut_CleanBuf(T_VOID);

T_BOOL WaveOut_Close(T_VOID);

T_BOOL WaveOut_SwitchTrack(T_VOID);


/**
 * @brief Set Fade In/Out In Audio Start/End
 * @date	2012-02-13
 * @author	Xie_Wenzhong
 * @param	time		[in]	Fade In/Out Time, ms
 * @param	fadeType[in]	Fade Type, FADE_STATE_IN / FADE_STATE_OUT
 * @return 	T_VOID	
 */
T_VOID	WaveOut_SetFade(T_U32 time, T_eFADE_STATE fadeType);

//date: 2009-10-9
//brief: added for CMMB, cleanning rudimental data after changing channel
T_VOID WaveOut_ResetBuf(T_VOID);

T_BOOL  WaveOut_IsOpen(T_VOID);

T_VOID WaveOut_CloseDac(T_VOID);




#endif
