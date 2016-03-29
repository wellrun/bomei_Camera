/**
 * @file Fwl_pfAudio.h
 * @brief This header file is for framework layer sound function prototype
 *
 */
#ifndef __FWL_PF_AUDIO_H__
#define __FWL_PF_AUDIO_H__

#include "anyka_types.h"
#include "Fwl_sysevent.h"
#include "Log_MediaPlayer.h"
#include "Eng_String.h"
#include "Log_RecAudio.h"
#include "Lib_SdFilter.h"
#include "Ctl_AudioPlayer.h"

#ifndef MP3_PLAYER_STATUS
#define MP3_PLAYER_STATUS           0
#define MP3_PLAYER_PLAY             1
#define MP3_PLAYER_PAUSE            2
#define MP3_PLAYER_FORWARD          3
#define MP3_PLAYER_BACKWARD         4
#define MP3_PRE_LISTEN              5
#endif


T_VOID Fwl_AudioVolumeInit(T_VOID);
T_VOID Fwl_AudioVolumeFree(T_VOID);
T_U16  Fwl_AudioVolumeAdd(T_VOID);
T_U16  Fwl_AudioVolumeSub(T_VOID);
T_U16  Fwl_GetAudioVolume(T_VOID);
T_BOOL Fwl_GetAudioVolumeStatus(T_VOID);
T_VOID Fwl_SetAudioVolumeStatus(T_BOOL status);

T_VOID Fwl_AudioDisableDA(T_VOID);
T_BOOL Fwl_AudioEnableDA(T_VOID);

T_BOOL Fwl_AudioSetVolume(T_U16 volume);
T_BOOL Fwl_AudioMute(T_VOID);
T_BOOL Fwl_AudioSetEQMode(T_EQ_MODE mode);
T_BOOL Fwl_AudioSetPlaySpeed(T_WSOLA_TEMPO speed);
T_BOOL Fwl_AudioSetVoiceChange(T_PITCH_MODES mode);
T_BOOL Fwl_AudioSetTrack(T_U8 channel);
T_BOOL Fwl_AudioChangeTrack(T_VOID);
T_BOOL Fwl_AudioOpenBuffer(T_pCDATA Buff, T_U32 BuffLen);
T_BOOL Fwl_MP3AudioOpenBuffer(T_pCDATA Buff, T_U32 BuffLen);
T_BOOL Fwl_AudioOpenFile(T_pCWSTR fileName);
T_BOOL Fwl_AudioPlay(T_AUDIOPLAYER *pAPlayer);
T_VOID Fwl_AudioStop(T_END_TYPE endType);
T_BOOL Fwl_AudioPause(T_VOID);
T_BOOL Fwl_AudioResume(T_VOID);
T_BOOL Fwl_AudioSeek(T_U32 milliSecond);
T_BOOL Fwl_AudioGetTotalTimeFromBuffer(T_U8 *Buff, T_U32 BuffLen, T_U32 *totalTime);
T_BOOL Fwl_AudioGetTotalTimeFromFile(T_pCWSTR fileName, T_U32 *totalTime);
T_BOOL Fwl_AudioGetMediaTypeFromFile(T_pCWSTR fileName, T_U8 *MediaType);

T_BOOL Fwl_AudioIsPlaying(T_VOID);
T_U8   Fwl_GetMediatypeByFiletype(T_FILE_TYPE Filetype);


T_U32 Fwl_AudioGetSyncTime(T_VOID);


#endif

