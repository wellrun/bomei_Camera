/**@file hal_i_mouse_ps2.h
 * @brief  ps2 mouse interface .
 *
 * This file provide interface of ps2 mouse.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2011-01-06
 * @version 1.0
 */

#ifndef __HAL_I_MOUSE_PS2_H
#define __HAL_I_MOUSE_PS2_H

#include "arch_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    MOUSE_PS2_BTN_NON,
    MOUSE_PS2_BTN_LEFT,
    MOUSE_PS2_BTN_RIGHT,
    MOUSE_PS2_BTN_MIDDLE,
    MOUSE_PS2_BTN_4,
    MOUSE_PS2_BTN_5
}T_eMOUSE_PS2_BTN;

typedef enum
{
    MOUSE_PS2_ID_STD = 0x0,
    MOUSE_PS2_ID_SCROLLING = 0x3,
    MOUSE_PS2_ID_5BTN_SCROLLING
}T_eMOUSE_PS2_ID;

typedef struct _MOUSE_DATA
{
    T_S32 lMvX;
    T_S32 lMvY;
    T_S32 lMvZ;
    T_eMOUSE_PS2_BTN eBtn;
}T_MOUSE_DATA,*T_pMOUSE_DATA;


typedef T_VOID (*T_fMOUSE_PS2_CALLBACK)(T_pMOUSE_DATA data);

/**
 * @brief   mouse init 
 *
 * mouse reset , get mouse clk,enable mouse data,enable uart receive
 * @author Huang Xin
 * @date 2011-01-27
 * @param[in] uart_id uart id
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL mouse_ps2_open(T_UART_ID uart_id);
/**
 * @brief   mouse close
 *
 * close ps2 , uart free
 * @author Huang Xin
 * @date 2011-01-27
 * @param[in] uart_id uart id
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_VOID mouse_ps2_close(T_UART_ID uart_id);

/**
 * @brief   set callback func
 *
 * called after mouse_ps2_open()
 * @author Huang Xin
 * @date 2011-01-27
 * @param[in] cb callback func
 * @return  T_VOID
 */
T_VOID mouse_ps2_set_cb(T_fMOUSE_PS2_CALLBACK cb);



#ifdef __cplusplus
}
#endif

#endif
