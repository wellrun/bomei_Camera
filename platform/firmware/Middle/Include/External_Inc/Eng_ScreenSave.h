/**
 * @file Eng_ScreenSave.h
 * @brief This header file is for global data
 * 
 */

#ifndef __ENG_SCREEN_SAVE_H__
#define __ENG_SCREEN_SAVE_H__

#include "anyka_types.h"

T_VOID  UserCountDownReset(T_VOID);

T_VOID ScreenSaverCountSet(T_U32 count);
T_BOOL ScreenSaverCountDecrease(T_U32 millisecond);
T_VOID ScreenSaverDisable(T_VOID);
T_VOID ScreenSaverEnable(T_VOID);
T_BOOL ScreenSaverIsOn(T_VOID);

#endif /* __ENG_SCREEN_SAVE_H__ */
