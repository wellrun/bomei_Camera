/**
 * @file Log_MediaAudio.h
 * @brief Mp3 Decoder and Player Interface for Multi-thread Implementation
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author He_Yuanlong
 * @date 2011-8-29
 * @version 1.0
 */

#ifndef _LOG_MEDIA_MP3_H_
#define _LOG_MEDIA_MP3_H_

#include "Anyka_types.h"
#include "Lib_Media_Struct.h"
#include "Lib_Media_Global.h"

#include "Fwl_sysevent.h"
#include "Log_MediaStruct.h"


T_BOOL Sd_Mp3InitDecoder(T_pMP3AUD_DEC* ppMp3Aud);

T_BOOL Sd_Mp3OpenDecoder(T_pMT_MP3PLAYER pPlayer);

T_BOOL Sd_Mp3PreDecode(T_hFILE hFile, T_pMT_MP3PLAYER player);

T_VOID Sd_Mp3HandleAudDec(T_pMT_MP3PLAYER pPlayer);

T_pVOID Sd_Mp3Close(T_pMP3AUD_DEC pMp3Aud);

T_pVOID MP3Player_GetPlayer(T_VOID);

T_VOID Sd_MP3Start(T_pMT_MP3PLAYER pPlayer, T_U32 startPos);

T_VOID Sd_MP3Stop(T_pMP3AUD_DEC pAudio);

#endif // _LOG_MEDIA_MP3_H_
