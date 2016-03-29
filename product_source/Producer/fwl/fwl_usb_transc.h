/**
 * @filename hal_usb_mass.h
 * @brief: AK3223M Mass Storage of usb.
 *
 * This file describe mass storage protocol of usb disk.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */

#ifndef __HAL_USB_MASS_H__
#define __HAL_USB_MASS_H__

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DATA_STAGE_NONE = 0,
    DATA_STAGE_RECV,
    DATA_STAGE_SEND
}
E_DATA_PHASE;

typedef struct tagCmdResult
{
    T_U8 status;
    
    T_U16 data_stage;
    T_U32 data_count;

    T_U32 ack_result;
}
T_CMD_RESULT;

//ret: AK_TRUE, continue, AK_FALSE, quit usb_main; compatible to module burn
typedef T_BOOL (*T_FCBK_CMD)(T_U8 data[], T_U32 len, T_CMD_RESULT *result);
typedef T_BOOL (*T_FCBK_RCV)(T_U8 buf[], T_U32 len);
typedef T_BOOL (*T_FCBK_SND)(T_U8 buf[], T_U32 len);

typedef struct tagTRANSC
{
    T_U32 cmd;
    T_FCBK_CMD fCmd;
    T_FCBK_RCV fRcv;
    T_FCBK_SND fSnd;
}
T_ANYKA_TRANSC;

//********************************************************************
/*@}*/                      
#ifdef __cplusplus    
}                     
#endif                
                      
#endif                
                      
                      
                      
                      
                      
                      
                      
                      
                      
