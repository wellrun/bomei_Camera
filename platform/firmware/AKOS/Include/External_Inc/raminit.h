/*
 * @(#)raminit.h
 * @date 2010/4/28
 * @version 1.0
 * @author DengZhou.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 */

#ifndef        _RAMINIT_H_
#define        _RAMINIT_H_

#include "mem_api.h"

#define M_RAM            "RAM"

#ifdef OS_WIN32
    #define MIN_RAM_ADDR    0x00001000  // The first address of RAM
    #define MAX_RAM_ADDR    0x2fffffff  // the end of RAM
    #define MIN_ROM_ADDR    0x00001000  // The first address of ROM
    #define MAN_ROM_ADDR    0x2fffffff  // the end of ROM
#else

    #define MIN_RAM_ADDR    0x30000000  // The first address of RAM

#ifdef SDRAM_MODE
#define MAX_RAM_ADDR        (MIN_RAM_ADDR+(SDRAM_MODE<<20))
#else
#error "No define SDRAM_MODE"
#endif
    #define MIN_ROM_ADDR    0x10000000  // The first address of ROM
    #define MAN_ROM_ADDR    0x12000000  // the end of ROM 8M

#endif


T_BOOL Ram_AssertCheckPointer(T_pCVOID ptr);

T_VOID Ram_AssertDispMsg(T_pCSTR message, T_pCSTR filename, T_U32 line);

#define RAM_ASSERT_VAL(_bool_, _msg_, _retval_)    if (!(_bool_)) { Ram_AssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return (_retval_); }
#define RAM_ASSERT_VAL_VOID(_bool_, _msg_)        if (!(_bool_)) { Ram_AssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return; }
#define RAM_ASSERT_PTR(_ptr_, _msg_, _retval_)    if (!Ram_AssertCheckPointer(_ptr_)) { Ram_AssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return (_retval_); }
#define RAM_ASSERT_PTR_VOID(_ptr_, _msg_)        if (!Ram_AssertCheckPointer(_ptr_)) { Ram_AssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return; }

T_S32 Ram_getaddr(T_U32 *minaddr, T_U32* maxaddr);

T_U32 Ram_getMMUaddr(T_VOID);

T_GLOBALMEMHandle Ram_GetGlobalMem(T_VOID);

T_U32  Ram_GetRamBufferSize(T_VOID);

T_U32  Ram_ReleaseMemory(T_VOID);

T_VOID Ram_MallocInit(T_VOID);

T_VOID Ram_MallocSupportMultithread(T_VOID);

#endif

