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
*/
#ifdef OS_WIN32

#ifndef _VME_EVENT_H
#define _VME_EVENT_H

#include "Fwl_tscrcom.h"

/** 
* vme system events
*/
void vme_event_Send_VME_EVT_SYSSTART (void);
void vme_event_Send_VME_EVT_RETRIGGER (vT_EvtSubCode event, vUINT32 param);

/** 
* keypad events
*/
// java adds this funtion for java user key process. 07.9.21
void vme_event_Send_VME_EVT_KEY( int KeyMes , int Key );
/**
* touch screen events
*/
void vme_event_Send_VME_EVT_TOUCHSCR_ACTION (T_TOUCHSCR_ACTION action, vUINT16 x, vUINT16 y, vUINT32 TimeStamp);

/** 
* V24 events
*/
void vme_event_Send_VME_EVT_V24_WRITE_CALLBACK (vUINT8 DeviceHandle);
void vme_event_Send_VME_EVT_V24_RECEIVED_DATA_AVAIL (vUINT8 DeviceHandle);
void vme_event_Send_VME_EVT_V24_STATUS_LINES_CHANGED (vUINT8 DeviceHandle, vUINT8 Dtr, vUINT8 Rts);


/** 
* vatc events
*/
void vme_event_Send_VME_EVT_VATC_WRITE_CALLBACK (vUINT8 DeviceHandle);
void vme_event_Send_VME_EVT_VATC_RECEIVED_DATA_AVAIL (vUINT8 DeviceHandle);
void vme_event_Send_VME_EVT_VATC_STATUS_LINES_CHANGED (vUINT8 DeviceHandle, vUINT8 Ring, vUINT8 Dcd);
void vme_event_Send_VME_EVT_VATC_RECEIVED_MUX_DATA (vUINT8 DeviceHandle);

/** 
* fsfflash events
*/
//void vme_event_Send_VME_EVT_FSFLASH_FINISH (vFS_RESULT_STRUCT *pRes);


#endif // _VME_EVENT_H
#endif
