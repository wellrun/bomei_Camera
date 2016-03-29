/**
 * @file drv_keypad.h
 * @brief keypad module, for keypad register
 *
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @date 2005.12.20
 * @version 1.0
 */
#ifndef __DRV_KEYPAD_H__
#define __DRV_KEYPAD_H__

#include "hal_keypad.h"

/** @defgroup Keypad Keypad group
 *  @ingroup Drv_Lib
 */


/** @defgroup drv_keypad Keypad driver group
 *  @ingroup Keypad
 */
/*@{*/


#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus


/**
 * @brief: Keypad callback define.
 */
typedef T_VOID (*T_f_H_KEYPAD_CALLBACK)(const T_KEYPAD *keypad); 

/**
 * @brief  key handler define
 */
typedef struct
{
    T_VOID (*KeyPadInit)(T_f_H_KEYPAD_CALLBACK callback_func, const T_VOID *keypad_parm);//keypad init function
    
    T_S32  (*KeyPadScan)(T_VOID); //directly scan function, get the key value currently pressed
    T_VOID (*KeyPadEnIntr)(T_VOID);//enable keypad interrupt
    T_VOID (*KeyPadDisIntr)(T_VOID);//disable keypad interrupt
    
    T_eKEY_PRESSMODE (*GetMode)(T_VOID);//get curent keypad mode
    T_BOOL (*SetMode)(T_eKEY_PRESSMODE press_mode);//set press mode
    
    T_VOID (*SetDelay)(T_S32 keydown_delay, T_S32 keyup_delay, T_S32 keylong_delay, T_S32 powerkey_long_delay, T_S32 loopkey_delay);//adjust keypad param
} T_KEYPAD_HANDLE;

/**
 * @brief register keypad scan mode
 *
 * @author Miaobaoli
 * @date 2004-09-21
 * @param[in] index keypad index
 * @param[in] handler keypad handler 
 * @return T_BOOL
 */
T_BOOL keypad_reg_scanmode(T_U32 index, T_KEYPAD_HANDLE *handler);


#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

/*@}*/
#endif // #ifndef __DRV_KEYPAD_H__

