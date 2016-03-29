/**@file hal_mouse_ps2.h
 * @brief implement mouse.
 *
 * This file provide interface of mouse.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2011-01-06
 * @version 1.0
 */

#ifndef __HAL_MOUSE_PS2_H
#define __HAL_MOUSE_PS2_H

#include "hal_i_mouse_ps2.h"

#ifdef __cplusplus
extern "C" {
#endif

//mouse ps2 command
#define MOUSE_PS2_CMD_RESET             (0xff)
#define MOUSE_PS2_CMD_SET_DEF           (0xf6)
#define MOUSE_PS2_CMD_DISABLE_DATA      (0xf5)
#define MOUSE_PS2_CMD_ENABLE_DATA       (0xf4)
#define MOUSE_PS2_CMD_SET_SAMPLE        (0xf3)
#define MOUSE_PS2_CMD_GET_ID            (0xf2)
#define MOUSE_PS2_CMD_SET_REMOTE        (0xf0)
#define MOUSE_PS2_CMD_SET_WRAP          (0xee)
#define MOUSE_PS2_CMD_RESET_WRAP        (0xec)
#define MOUSE_PS2_CMD_READ_DATA         (0xeb)
#define MOUSE_PS2_CMD_SET_STREAM        (0xea)
#define MOUSE_PS2_CMD_REQ_STATUS        (0xe9)
#define MOUSE_PS2_CMD_SET_RES           (0xe8)
#define MOUSE_PS2_CMD_SET_SCAL2         (0xe7)
#define MOUSE_PS2_CMD_SET_SCAL1         (0xe6)

#define MOUSE_PS2_SELF_TST_PASS     (0xaa)

#define MOUSE_PS2_RESP_ACK      (0xfa)
#define MOUSE_PS2_STD_DATA_LEN      3
#define MOUSE_PS2_INTELLI_DATA_LEN  4
#define MOUSE_PS2_BUF_RCV_LEN       16

//T_eMOUSE_PS2_MODE
typedef enum
{
    MOUSE_PS2_MODE_RESET,
    MOUSE_PS2_MODE_STREAM,
    MOUSE_PS2_MODE_REMOTE,
    MOUSE_PS2_MODE_WRAP
}T_eMOUSE_PS2_MODE;


//T_MOUSE_PS2
typedef struct _MOUSE_PS2_
{      
    T_BOOL  bInitOK;
    T_U32   ulRate;
    T_U8    ucId;
    T_U8    ucMode;
    T_U8    ucUartId;
    T_U8    BufRcv[MOUSE_PS2_BUF_RCV_LEN];  //recv buffer
    T_fMOUSE_PS2_CALLBACK fMousePs2Cb;      //callback func
}T_MOUSE_PS2,*T_pMOUSE_PS2;


#ifdef __cplusplus
}
#endif

#endif
