/**
 * @file Log_MediaAudio.h
 * @brief Audio Decoder  Interface for Multi-thread Implementation
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Xie_Wenzhong
 * @date 2011-3-17
 * @version 1.0
 */

#ifndef _LOG_MEDIA_AUDIO_H_
#define _LOG_MEDIA_AUDIO_H_

#include "Anyka_types.h"
#include "Lib_Media_Struct.h"
#include "Lib_Media_Global.h"
#include "Media_Demuxer_lib.h"
#include "Lib_SdFilter.h"

#include "Fwl_sysevent.h"
#include "Log_MediaStruct.h"

T_VOID Sd_SetCodecCB(T_AUDIO_CB_FUNS *cbFun);

T_BOOL Sd_OpenAudioPlayer(T_pMT_MPLAYER player);
T_VOID Sd_HandleAudioDec(T_pMT_MPLAYER pPlayer);

T_BOOL Sd_OpenWaveOut(const T_AUDIO_DECODE_OUT *decOut, T_U32 beginTime);

T_pVOID Sd_Close(T_pAUDIO_DEC pAudio);
T_VOID Sd_Start(T_pMT_MPLAYER pPlayer, T_U32 startPos);
T_VOID Sd_Stop(T_pAUDIO_DEC pAudio);

/**
 * @brief Set Current Media Player EQ or Speed
 *
 * @author 	Xie_Wenzhong
 * @param	hAudioDec	[in]	Audio Decoder Pointer, Can Retrieve With MPlayer_GetAudioDecoder()
 * @param	type		[in]	Filter Type:  _SD_FILTER_EQ / _SD_FILTER_WSOLA	
 * @param	mode		[in]	EQ Mode or Tempo Value
 * @return 	T_BOOL
 * @retval	AK_FALSE	Failure
 * @retval	AK_TRUE	Success
 */
T_BOOL Sd_SetFilter(T_pVOID hAudioDec, T_AUDIO_FILTER_TYPE type, T_U8 mode);

T_pVOID SdFilter_Close(T_pAUDIO_FILTER pFilter);

#endif // _LOG_MEDIA_AUDIO_H_

