/**
 * @file nand_win32.h
 * @brief Simulate NAND in Win32
 *
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Micro-electronics Technology Co., Ltd.
 * @author 
 * @MODIFY  
 * @DATE    2010-6-7
 * @version 0.1.0
 * @
 */

#ifndef _NAND_WIN32_H_
#define _NAND_WIN32_H_

#include "anyka_types.h"

#ifdef OS_WIN32
T_PMEDIUM NandDisk_Initial();

T_BOOL InitWin32Nand(T_VOID);

#endif

#endif

