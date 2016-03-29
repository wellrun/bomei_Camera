/**
 * @file Eng_Melody.h
 * @brief This header file is for EMS melody process function prototype
 *
 */
#ifdef OS_WIN32

#ifndef __UTL_IMELODY_H__
#define __UTL_IMELODY_H__

#include "akdefine.h"

typedef T_VOID (*T_fMELODY_CALLBACK)(T_BOOL user_stop);

T_BOOL  iMelodyPlay(T_pSTR melodyBuf, T_U16 loop, T_fMELODY_CALLBACK callback);
T_VOID  iMelodyStop(T_VOID);
T_BOOL  iMelodyIsPlaying(T_VOID);
T_BOOL  ConvertMIDI2Melody(T_pCDATA midiData, T_pSTR melodyData);

#endif
#endif
