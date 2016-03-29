/**
 * @file Eng_AutoPowerOff.h
 * @brief This header file is for global data
 * 
 */

#ifndef __ENG_AUTO_POWER_OFF_H__
#define __ENG_AUTO_POWER_OFF_H__

#include "anyka_types.h"


typedef enum {
    FLAG_AUDIO		= 1,
    FLAG_VIDEO		= 2,
    FLAG_IMG		= 4,
    FLAG_EBK		= 8,
    FLAG_FM			= 16,
    FLAG_RECAUDIO	= 32,
    FLAG_CAMERA		= 64,
    FLAG_UVC		= 128,
    FLAG_FLASH		= 256,
    FLAG_NETWORK	= 512
} T_AUTOPOWEROFF_FLAG;


T_VOID AutoPowerOffCountSet(T_U32 count);
T_BOOL AutoPowerOffCountDecrease(T_U32 millisecond);
T_VOID AutoPowerOffDisable(T_AUTOPOWEROFF_FLAG flag);
T_VOID AutoPowerOffEnable(T_AUTOPOWEROFF_FLAG flag);
T_BOOL AutoPowerOffIsEnable(T_VOID);
T_U32 AutoPowerOffGetCount(T_VOID);

#endif /* __ENG_AUTO_POWER_OFF_H__ */
