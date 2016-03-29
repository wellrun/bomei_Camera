/**@file dev_paddle.h
 * @brief lcd module, for lcd buffer and registeration operations
 *
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @date 2007.12.20
 * @version 1.0
 */
#ifndef __DEV_PADDLE_H__
#define __DEV_PADDLE_H__

#define PADDLE_SCAN_TIME   20

typedef struct 
{
    T_U8  num[16];
    T_U8  press[16];
} T_PADDLE_KEYPAD;

typedef T_VOID (*PADDLE_CALLBACK_PT)(T_S32 nPaddleIndex, T_U32  DataPin, T_U32 BlPin, T_U32 ClkPin);

typedef struct 
{
    PADDLE_CALLBACK_PT normalmode_callback_func;
    PADDLE_CALLBACK_PT gamemode_callback_func;
} T_PADDLE_CALLBACK_FUNC;

/*@}*/
#endif

