/**
 * @file Log_MediaVideo.c
 * @brief  VisualAudio display
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author 
 * @date 
 * @version 1.0
 */

#ifndef _LOG_MEDIA_VISUALAUDIO_H_
#define _LOG_MEDIA_VISUALAUDIO_H_
#define EVT_VISUAL_REFRESH	0x2020

T_VOID Log_LockVA_Audio(T_VOID);
T_VOID Log_UnLockVA_Audio(T_VOID);

T_VOID VA_Fill_AudioBuf(T_S16* src,T_U32 len);

T_VOID VA_Increase_Draw_Type(T_VOID);

T_BOOL VisualAudio_Init(T_VOID);

T_BOOL VisualAudio_IsInit(T_VOID);

T_VOID VisualAudio_Realease(T_VOID);

#endif


