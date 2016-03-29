/*****************************************************************************
 * Copyright (C) 2003 Anyka.
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
*/
#ifdef OS_WIN32

#ifndef _VME_UTIL_H
#define _VME_UTIL_H

#include "fwl_sysevent.h"
/**
* v24, vatc
*/

#define MAX_COM_CHANNELS       4


typedef enum {
    V24_CHANNEL     = 0,
    VATC_CHANNEL_1,
    VATC_CHANNEL_2,
    VATC_CHANNEL_3,
} vmeT_ComPortChan;


typedef void (*vmeT_ComEventCallback)     (vmeT_ComPortChan ComPortChan, vUINT32 ComEvent);



/*****************************************************************************
 * interface
 *****************************************************************************
*/
/**
* -----------------------------------------------------------------------------
* vme event queue
* -----------------------------------------------------------------------------
*/
void vme_util_CreateVMEEventQueue (void);
void vme_util_DestroyVMEEventQueue (void);
void vme_util_PutInVMEEventQueue (T_SYS_EVTID EvtCode, T_SYS_PARAM *pEvtParam);
vBOOL vme_util_PutUniqueVMEEventQueue (T_SYS_EVTID EvtCode, T_SYS_PARAM *pEvtParam);
vBOOL vme_util_GetFromVMEEventQueue (T_SYS_EVTID *pEvtCode, T_SYS_PARAM *pEvtParam);



/**
* -----------------------------------------------------------------------------
* application
* -----------------------------------------------------------------------------
*/
void vme_util_CloesAppl (void);


/**
 * ----------------------------------------------------------------------------
 * com ports
 * ----------------------------------------------------------------------------
*/
void vme_util_RegisterComportEventHandler (
    vmeT_ComEventCallback  fpV24,
    vmeT_ComEventCallback  fpVATC1,
    vmeT_ComEventCallback  fpVATC2,
    vmeT_ComEventCallback  fpVATC3
);

// vBOOL vme_util_OpenV24Port (vmeT_ComPortChan ComPortChan);
// void vme_utilCLoseV24Port (vmeT_ComPortChan ComPortChan);
// vINT16 vme_util_WriteV24Port (vmeT_ComPortChan ComPortChan, const vPDATA data, vINT16 count);
// vINT16 vme_util_ReadV24Port (vmeT_ComPortChan ComPortChan, vPDATA data, vINT16 count);
// vBOOL vme_util_v24Flush(vmeT_ComPortChan ComPortChan);
// vINT32 vme_util_v24GetBaudrate(vmeT_ComPortChan ComPortChan);
// vINT16 vme_util_v24GetLineStatus(vmeT_ComPortChan ComPortChan, VME_V24STATUS * status);
// vINT16 vme_util_vatcGetLineStatus(vmeT_ComPortChan ComPortChan, VME_VATCSTATUS * status);
// vINT16 vme_util_v24SetLineStatus(vmeT_ComPortChan ComPortChan, VME_V24STATUS * status);
// vINT16 vme_util_v24ClearReadBuffer(vmeT_ComPortChan ComPortChan);
// vINT16 vme_util_v24GetReadBufferLength(vmeT_ComPortChan ComPortChan);
// vINT32 vme_util_v24SetBaudrate(vmeT_ComPortChan ComPortChan, vINT32 baudrate);
// vINT16 vme_util_v24SetFlowControl(vmeT_ComPortChan ComPortChan, VME_V24FLOWCTRL fctrl);

/**
* -----------------------------------------------------------------------------
* flash file system
* -----------------------------------------------------------------------------
*/
//vFS_RESULT_STRUCT *vme_util_GetMemForFFSResult (void);
//void vme_util_FreeMemForFFSResult (vFS_RESULT_STRUCT *pRes);











#endif // _VME_UTIL_H
#endif
