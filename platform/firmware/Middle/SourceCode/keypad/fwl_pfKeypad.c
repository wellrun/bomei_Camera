/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: fwl_pfKeypad.c
* Function: frame work level of key
*
* Author: 
* Date: 2001-04-14
* Version: 1.0
*
* Revision: 
* Author: 
* Date: 
***************************************************************************/
/**
 * @brief Stop keypad timer
 * Function keypad_init() must be called before call this function
 * @author YiRuoxiang
 * @date 2006-02-09
 * @param T_VOID
 * @return T_VOID
 */

#include "anyka_types.h"
#include "fwl_pfkeypad.h" 
#include "fwl_keyhandler.h"
#include "hal_keypad.h"

T_VOID Fwl_KeyOpen(T_VOID)
{
    keypad_open();
    Fwl_keypadEnableIntr();
}


T_VOID Fwl_KeyStop(T_VOID)
{
    keypad_keystop();
}

/**
 * @brief set keypad mode
 * @author 
 * @date 2010-10-13
 * @return AK_TRUE
 */
T_BOOL Fwl_KeypadSetSingleMode(T_VOID)
{
#if (KEYPAD_TYPE == 0)
    return keypad_set_pressmode(eSINGLE_PRESS);
#else 
    return  AK_TRUE;
#endif//end of (KEYPAD_TYPE == 0)
}




T_BOOL Fwl_KeypadSetMultipleMode(T_VOID)
{
#if (KEYPAD_TYPE == 0)
    return keypad_set_pressmode(eMULTIPLE_PRESS);
#else 
    return  AK_TRUE;
#endif//end of (KEYPAD_TYPE == 0)
}


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
T_VOID Fwl_KeypadSetDelay(T_S32 keydown_delay, T_S32 keyup_delay, T_S32 keylong_delay, T_S32 powerkey_long_delay, T_S32 loopkey_delay)
{
    keypad_set_delay(keydown_delay, keyup_delay, keylong_delay, powerkey_long_delay, loopkey_delay);
}


T_VOID Fwl_GameKeypadSetDelay(T_VOID)
{    
    Fwl_KeypadSetDelay(DEFAULT_KEYDOWN_DELAY, DEFAULT_KEYUP_DELAY, DEFAULT_LONG_KEY_DELAY, DEFAULT_POWERKEY_LONG_DELAY, DEFAULT_LOOP_KEY_DELAY);
}

T_VOID Fwl_keypadEnableIntr(T_VOID)
{
#ifdef OS_ANYKA    
    keypad_enable_intr();
#endif
}

T_VOID Fwl_keypadDisableIntr(T_VOID)
{
#ifdef OS_ANYKA
    keypad_disable_intr();
#endif
}





