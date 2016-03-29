/**
 * @file Log_MediaVideo.c
 * @brief  Video Decoder Interface for Multi-thread
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Xie_Wenzhong
 * @date 2011-3-17
 * @version 1.0
 */

#ifndef _LOG_MEDIA_VIDEO_H_
#define _LOG_MEDIA_VIDEO_H_

#include "anyka_types.h"
#include "Fwl_sysevent.h"
#include "Log_MediaStruct.h"


T_BOOL Vs_Yuv2RgbAvail(T_pDATA pBuff);
T_pVOID Vs_FreeVS(T_pVIDEO_DEC video);
T_pVOID Vs_Close(T_pVIDEO_DEC pVideo);

T_BOOL Vs_OpenVideoDecoder(T_pMT_MPLAYER hPlayer);
T_VOID Vs_HandleDecode(T_pMT_MPLAYER pPlayer);

T_pVOID Vs_PrepareDec(T_VOID);


//used by video lib
T_BOOL VD_EXfunc_JPEG2YUV(T_U8 *srcJPEG, T_U32 srcSize, T_U8 *dstYUV, T_S32 *pWidth, T_S32 *pHeight); 

#if CI37XX_PLATFORM

// MPU Refresh Interface
const T_pVOID MpuRefr_IsInit(T_VOID);
T_BOOL MpuRefr_Init(T_VOID);
T_VOID MpuRefr_SetShowFrameCB(T_fSHOWFRAME_CB 	ShowCB);

T_VOID MpuRefr_Refresh(T_VIDEO_DECODE_OUT *decOut);
T_BOOL MpuRefr_CheckFrameFinish(const T_pDATA pBuf);
T_BOOL MpuRefr_Free(T_VOID);
T_VOID MpuRefr_Stop(T_VOID);

#endif	// CI37XX_PLATFORM

#endif	// _LOG_MEDIA_VIDEO_H_
