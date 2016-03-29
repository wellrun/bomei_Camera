/*****************************************************************************
 * Copyright (C) 2003 Anyka Co. Ltd
 *****************************************************************************
 *   Project: 
 *****************************************************************************
 * $Workfile: $
 * $Revision: $
 *     $Date: $
 *****************************************************************************
 * Description:
 *
 *****************************************************************************
 * 10.04.2003    KR36712    add return values for vme_engine_Init()
 *
 * 15.04.2003    KR36712    add vme_engine_vatcSendEsc () for vatc interface
 *
 *****************************************************************************
*/
#ifdef OS_WIN32

#ifndef _VME_ENGINE_H
#define _VME_ENGINE_H

#include "fwl_vme.h"

/*****************************************************************************
 * defines
 *****************************************************************************
*/

#define MAX_V24_WRITEPACKAGE_SIZE       0x4000      // max send data blaock -> 16 k

/** 
* error values for vme_engine_Init ()
*/
typedef enum {
    VMEENG_OK  =   0,
    VMEENG_ERR_CANNOT_OPEN_V24,
    VMEENG_ERR_CANNOT_OPEN_ATC1,
    VMEENG_ERR_CANNOT_OPEN_ATC2,
    VMEENG_ERR_CANNOT_OPEN_ATC3
} enum_vmeeng_ret_t;



/*****************************************************************************
 * interface
 *****************************************************************************
*/

/**
* vme engine
*/
enum_vmeeng_ret_t vme_engine_Init (void);
void vme_engine_SetMainCallBack (vT_MainCallback pMainCallback);
void vme_engine_HandleEventQueue (void);
void vme_engine_Term (void);

#if 00
/**
* vatc
*/
VME_VATCCHANID vme_engine_vatcOpen(VME_VATC_CHAN Channel);
vINT16 vme_engine_vatcClose(VME_VATCCHANID ChanId);
vINT16 vme_engine_vatcRead(VME_VATCCHANID ChanId, vPDATA data, vINT16 count);
vINT16 vme_engine_vatcWrite(VME_VATCCHANID ChanId, const vPDATA data, vINT16 count);
vINT16 vme_engine_vatcWriteBlocked(VME_VATCCHANID ChanId, const vPDATA data, vINT16 count);
vINT16 vme_engine_vatcSendEsc(VME_VATCCHANID ChanId);
vINT16 vme_engine_vatcGetLineStatus(VME_VATCCHANID ChanId, VME_VATCSTATUS * status);
vINT16 vme_engine_vatcClearReadBuffer(VME_VATCCHANID ChanId);
vINT16 vme_engine_vatcGetReadBufferLength(VME_VATCCHANID ChanId);

/**
* v24
*/
VME_V24DEVHANDLE vme_engine_v24Open(void);
vINT16 vme_engine_v24Close(VME_V24DEVHANDLE devHandle);
vINT16 vme_engine_v24Write(VME_V24DEVHANDLE devHandle, const vPDATA data, vINT16 count);
vINT16 vme_engine_v24WriteBlocked(VME_V24DEVHANDLE devHandle, const vPDATA data, vINT16 count);
vINT16 vme_engine_v24Read(VME_V24DEVHANDLE devHandle, vPDATA data, vINT16 count);
vINT16 vme_engine_v24Flush(VME_V24DEVHANDLE devHandle);
vINT32 vme_engine_v24GetBaudrate(VME_V24DEVHANDLE devHandle);
vINT16 vme_engine_v24GetLineStatus(VME_V24DEVHANDLE devHandle, VME_V24STATUS * status);
vINT16 vme_engine_v24SetLineStatus(VME_V24DEVHANDLE devHandle, VME_V24STATUS * status);
vINT16 vme_engine_v24ClearReadBuffer(VME_V24DEVHANDLE devHandle);
vINT16 vme_engine_v24GetReadBufferLength(VME_V24DEVHANDLE devHandle);
vINT32 vme_engine_v24SetBaudrate(VME_V24DEVHANDLE devHandle, vINT32 baudrate);
vINT16 vme_engine_v24SetFlowControl(VME_V24DEVHANDLE devHandle, VME_V24FLOWCTRL fctrl);

#endif

/**
* fsflash
*/
vBOOL fsfflash_Init (void);

//extern T_U32				SynchEvent;

#endif // _VME_ENGINE_H
#endif
