/**
 * @file Eng_BatWarn.h
 * @brief This header file is for global data
 * 
 */

#ifndef __ENG_BAT_WARN_H__
#define __ENG_BAT_WARN_H__

#include "anyka_types.h"

T_VOID BatWarnCountSet(T_U32 count);
T_BOOL BatWarnCountDecrease(T_U32 second);
T_VOID BatWarnDisable(T_VOID);
T_VOID BatWarnEnable(T_VOID);
//T_BOOL BatWarnOn(T_VOID);
T_BOOL BatWarnIsEnable(T_VOID);
T_U32 BatWarnGetCount(T_VOID);


#endif /* __ENG_BAT_WARN_H__ */
