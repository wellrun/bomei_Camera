/**
 * @file Log_MediaDmx.h
 * @brief Demuxer  Interface for Multi-thread Implementation
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Xie_Wenzhong
 * @date 2011-3-17
 * @version 1.0
 */

#ifndef _LOG_MEDIA_DMX_H_
#define _LOG_MEDIA_DMX_H_

#include "Anyka_types.h"
#include "Fwl_Sysevent.h"
#include "Media_Demuxer_lib.h"
#include "Log_MediaStruct.h"

#define EVT_DMX_START	0x1001
#define EVT_DMX_STOP	0x1002
#define EVT_DMX_PAUSE	0x1004
#define EVT_DMX_RESUME	0x1008
#define EVT_DMX_AB		0x1011
#define EVT_DMX_CLOSE	0x10FF
#define EVT_DMX_SCAN	0x1100
#define EVT_DMX_EXIT	0x1101
#define EVT_DMX_FF		0x1201
#define EVT_DMX_FR		0x1202


T_U32 Dmx_GetVideo2Decoder(T_MEDIALIB_STRUCT hMedia, T_pVOID hVS);
T_pVOID Dmx_CloseDemux(T_pDEMUXER dmx);
T_BOOL Dmx_QueryInfo(T_MEDIALIB_DMX_INFO *pInfo, T_pVOID fname, T_BOOL isFile);
T_U32 Dmx_GetAudio2Decoder(T_MEDIALIB_STRUCT hMedia, T_pVOID hSD);
T_pVOID Dmx_Open(T_MEDIALIB_DMX_INFO *info, T_hFILE hFile,
					T_BOOL isFile, T_eMEDIALIB_MEDIA_TYPE mediaType, T_BOOL openPRN);

T_VOID CThread_StopTimer(T_TIMER *pTimer);
T_BOOL CThread_StartTimer(T_TIMER *timer, T_U32 millSec, T_fVTIMER_CALLBACK cbFunc);

T_VOID DriveEvtCB_Demux(T_TIMER timer_id, T_U32 delay);


T_VOID DmxMgr_CloseVideo(T_pMT_MPLAYER pPlayer);
T_VOID DmxMgr_HandleDemux (T_pMT_MPLAYER pPlayer, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam);
T_BOOL DmxMgr_DecodeHeader(T_hFILE hFile, T_pMT_MPLAYER hPlayer, T_BOOL isFile);
T_VOID DmxMgr_StopTimer(T_pMT_MPLAYER pPlayer);
T_pVOID Dmx_QueryInfoEx(T_MEDIALIB_DMX_INFO *pInfo, T_pVOID fname, T_BOOL isFile);
T_BOOL DmxMgr_Close(T_pMT_MPLAYER pPlayer);
T_VOID DmxMgr_HandleEvt(T_EVT_CODE event, T_U32 param);

#endif 	// _LOG_MEDIA_DMX_H_

