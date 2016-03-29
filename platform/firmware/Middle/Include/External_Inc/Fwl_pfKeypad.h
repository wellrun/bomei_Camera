
#ifndef _FWL_PFKEYPAD_H_
#define _FWL_PFKEYPAD_H_

#include "akdefine.h"

T_VOID Fwl_KeyOpen(T_VOID);

/**
 * @brief Stop keypad timer
 * Function keypad_init() must be called before call this function
 * @author YiRuoxiang
 * @date 2006-02-09
 * @param T_VOID
 * @return T_VOID
 */
T_VOID Fwl_KeyStop(T_VOID);

T_BOOL Fwl_KeypadSetSingleMode(T_VOID);
T_BOOL Fwl_KeypadSetMultipleMode(T_VOID);




/**
 * @brief set keypad delay
 * @author 
 * @date 2010-10-13
 * @param keydown_delay
 * @param keyup_delay
 * @param keylong_delay
 * @param powerkey_long_delay
 * @param loopkey_delay
 * @return AK_TRUE
 */
T_VOID Fwl_KeypadSetDelay(T_S32 keydown_delay, T_S32 keyup_delay, T_S32 keylong_delay, T_S32 powerkey_long_delay, T_S32 loopkey_delay);
T_VOID Fwl_GameKeypadSetDelay(T_VOID);
T_VOID Fwl_keypadEnableIntr(T_VOID);
T_VOID Fwl_keypadDisableIntr(T_VOID);


#endif

