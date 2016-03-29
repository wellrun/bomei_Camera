/**
 * @file w_usb.c
 * @brief USB operation funcitons for PC version
 * This file provides usb operation functions for PC version
 * All of these operations are empty funcitons
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Miaobaoli
 * @date 2005-09-22
 * @version 1.0
 * @ref AK3223 technical manual.
 */
#ifdef OS_WIN32

#include "anyka_types.h"
#include "drv_api.h"

T_U8 usb_getstate(T_VOID)
{
    return USB_NOTUSE;
}
#endif
