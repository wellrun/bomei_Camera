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
#include <stdlib.h>
#include <assert.h>
#include "windows.h"
#include "vme_interface.h"
#include "vme_engine.h"
#include "vme_util.h"
#include "winvme.h"
#include "comport.h"
#include "Fwl_EvtMailBox.h"
#include "fwl_sysevent.h"
#include "Fwl_tscrcom.h"

/*****************************************************************************
 * defines
 *****************************************************************************
*/

enum {
    VME_TIMER_NO_ACTIVE,
    VME_TIMER_ACTIVE
};


/**
* types for vme event queue
*/

typedef struct _vmeT_Event {
    struct _vmeT_Event * next;
    struct _vmeT_Event * prev;
    T_SYS_EVTID          vmeEvtCode;             // event code
    T_SYS_PARAM       * pvmeEvtParam;           // event parameter
} vmeT_Event, *vmeT_pEvent;

/*
typedef struct {
    CRITICAL_SECTION    CSEventQueue;
    vmeT_pEvent         pFirst;
    vmeT_pEvent         pLast;
} vmeT_EventQueue;
*/

typedef struct {
    vUINT32         vmeTimeOut;
    T_SYS_EVTID      vmeEvtCode;
    BOOL            bState;
} vmeT_Timer;


/**
* types for vatc channel
*/

typedef struct {
    DWORD                       dwPortNum;
    DWORD                       dwComPortHandle;
    vmeT_ComEventCallback       fpRegisteredVmeEventCallBack;
} vmeT_V24Chan;

#define EVTPARAM_VME_TSCRACT(_param) ((T_TOUCHSCR_ACTION)((_param)->s.Param3))


/*****************************************************************************
 * globals
 *****************************************************************************
*/

//vmeT_EventQueue     vmeEventQueue;

/**
* v24, vatc
*/

//vmeT_ComEventCallback      V24_EventCallback       = NULL;

// vmeT_V24Chan   TabV24Channels[MAX_COM_CHANNELS] = {
//     {COMPORT_V24,    0, NULL},
//     {COMPORT_VATC_1, 0, NULL},
//     {COMPORT_VATC_2, 0, NULL},
//     {COMPORT_VATC_3, 0, NULL}
// };



/*****************************************************************************
 * functions
 *****************************************************************************
*/
static vBOOL VME_TSCRACT_CompareEvtParam(const T_SYS_PARAM *evtParm1, const T_SYS_PARAM *evtParm2)
{
	if(EVTPARAM_VME_TSCRACT(evtParm1) == EVTPARAM_VME_TSCRACT(evtParm2))
	{
		return vTRUE;
	}
	return vFALSE;
}

static void FreeVMEEvent (vmeT_pEvent  pEvent)
{
    assert (pEvent != NULL);

    free (pEvent);
    pEvent  =   NULL;

    return;
}

static vmeT_pEvent AllocVMEEvent (void)
{
    vmeT_pEvent pEvent  =   NULL;

    pEvent  = malloc (sizeof (vmeT_Event));

    assert (pEvent != NULL);

    pEvent->next            = NULL;
    pEvent->prev            = NULL;
    pEvent->vmeEvtCode      = 0;
    pEvent->pvmeEvtParam    = NULL;

    return pEvent;
}

static void FreeVMEEventParam (T_SYS_PARAM *pEventParam)
{
    assert (pEventParam != NULL);

    free (pEventParam);

    pEventParam = NULL;

    return;
}

static T_SYS_PARAM *AllocVMEEventParam (void)
{
    T_SYS_PARAM *pEventParam;

    pEventParam = malloc (sizeof (T_SYS_PARAM));

    assert (pEventParam != NULL);

    return pEventParam;
}


// static vmeT_V24Chan *GetV24Channel (vmeT_ComPortChan ComPortChan)
// {
//     return &TabV24Channels[ComPortChan];
// }
// 

/**
* com port event handler
*/
// void V24Port_EventHandler (DWORD dwHandle, DWORD dwComEvent)
// {
//     int             i;
//     vmeT_V24Chan    *pV24Chan;
// 
// 
//     for (i = 0; i < MAX_COM_CHANNELS; i++)
//     {
//         pV24Chan = GetV24Channel (i);
//         if (dwHandle == pV24Chan->dwComPortHandle)
//         {
//             if (NULL != pV24Chan->fpRegisteredVmeEventCallBack)
//             {
//                 pV24Chan->fpRegisteredVmeEventCallBack (i, dwComEvent);
//             }
//         }
//     }
// 
//     return;
// }


/*****************************************************************************
 * interface
 *****************************************************************************
*/
/**
* -----------------------------------------------------------------------------
* vme event queue
* -----------------------------------------------------------------------------
*/
void vme_util_CreateVMEEventQueue (void)
{
	MB_Init();
}

void vme_util_DestroyVMEEventQueue (void)
{
}


void vme_util_PutInVMEEventQueue (T_SYS_EVTID EvtCode, T_SYS_PARAM *pEvtParam)
{
	T_SYS_MAILBOX mailbox;

	mailbox.event = EvtCode;
	mailbox.param = *pEvtParam;

	AK_PostEvent( &mailbox );

    return;
}

vBOOL vme_util_PutUniqueVMEEventQueue (T_SYS_EVTID EvtCode, T_SYS_PARAM *pEvtParam)
{
	T_SYS_MAILBOX mailbox;

	mailbox.event = EvtCode;
	mailbox.param = *pEvtParam;
	if (EvtCode==SYS_EVT_TSCR)
	{
		if (0 == AK_PostTerminalInputEventEx( &mailbox, VME_TSCRACT_CompareEvtParam, 
			                                    AK_TRUE,AK_FALSE ))
		{
			return vTRUE;
		}
		else
		{
			return vFALSE;
		}
	}
	else
	{
		AK_PostTerminalInputEventEx( &mailbox, AK_NULL, AK_TRUE,AK_FALSE);
	}
	return vTRUE;
}

vBOOL vme_util_GetFromVMEEventQueue (T_SYS_EVTID *pEvtCode, T_SYS_PARAM *pEvtParam)
{
	T_SYS_MAILBOX mailbox;

	/* Xiao Jianting for win32 2007-09-14	if( !GetEventMailbox( &mailbox ) )*/
	if( !MB_GetEvent( &mailbox, AK_TRUE) )
	{
		if( !MB_GetEvent( &mailbox, AK_FALSE) )
		{
			return vFALSE;
		}
	}


	*pEvtCode = mailbox.event;

	*pEvtParam = mailbox.param;
	return vTRUE;
}




/**
* -----------------------------------------------------------------------------
*
* -----------------------------------------------------------------------------
*/
void vme_util_CloesAppl (void)
{
    winvme_CloesAppl ();
}
#if 0
/**
* -----------------------------------------------------------------------------
* com ports
* -----------------------------------------------------------------------------
*/
void vme_util_RegisterComportEventHandler (
    vmeT_ComEventCallback  fpV24,
    vmeT_ComEventCallback  fpVATC1,
    vmeT_ComEventCallback  fpVATC2,
    vmeT_ComEventCallback  fpVATC3
)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (V24_CHANNEL);
    pV24Chan->fpRegisteredVmeEventCallBack    = fpV24;

    pV24Chan = GetV24Channel (VATC_CHANNEL_1);
    pV24Chan->fpRegisteredVmeEventCallBack = fpVATC1;

    pV24Chan = GetV24Channel (VATC_CHANNEL_2);
    pV24Chan->fpRegisteredVmeEventCallBack = fpVATC2;

    pV24Chan = GetV24Channel (VATC_CHANNEL_3);
    pV24Chan->fpRegisteredVmeEventCallBack = fpVATC3;

    return;
}


vBOOL vme_util_OpenV24Port (vmeT_ComPortChan ComPortChan)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    if (COMPORT_OK != comport_Open (pV24Chan->dwPortNum, COMPORT_DEFAULT_BAUDRATE, V24Port_EventHandler, &pV24Chan->dwComPortHandle))
    {
        return FALSE;
    }

    return TRUE;
}

void vme_utilCLoseV24Port (vmeT_ComPortChan ComPortChan)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    comport_Close (pV24Chan->dwComPortHandle);
    pV24Chan->dwComPortHandle = 0;
    return;
}


vINT16 vme_util_WriteV24Port (vmeT_ComPortChan ComPortChan, const vPDATA data, vINT16 count)
{
    DWORD           dwBytesWritten  = 0;
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    dwBytesWritten  = comport_Write (pV24Chan->dwComPortHandle, data, (DWORD)count);

    return (vINT16)dwBytesWritten;
}


vINT16 vme_util_ReadV24Port (vmeT_ComPortChan ComPortChan, vPDATA data, vINT16 count)
{
    DWORD           dwRet;
    DWORD           dwBytesRead;
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    dwRet   = comport_Read (pV24Chan->dwComPortHandle, (BYTE *)data, (DWORD)count, &dwBytesRead);
    if (dwRet != COMPORT_OK)
    {
        assert (dwRet != COMPORT_OK);
        return 0;
    }

    return (vINT16)dwBytesRead;
}

vBOOL vme_util_v24Flush(vmeT_ComPortChan ComPortChan)
{
    DWORD           dwRet;
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    dwRet   = comport_Flush (pV24Chan->dwComPortHandle);
    if (dwRet != COMPORT_OK)
    {
        assert (dwRet != COMPORT_OK);
        return vFALSE;
    }
    return vTRUE;
}

vINT32 vme_util_v24GetBaudrate(vmeT_ComPortChan ComPortChan)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    return (vINT32)comport_GetBaudRate (pV24Chan->dwComPortHandle);
}

vINT16 vme_util_v24GetLineStatus(vmeT_ComPortChan ComPortChan, VME_V24STATUS * status)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    status->DCD_avail       =   0;
    status->RI_avail        =   0;

    status->DTR_DSR_avail   =   TRUE;
    status->RTS_CTS_avail   =   TRUE;


    /**
    * attention: we simulate vme interface in module
    *            from view of the module: input line CTS = output line from PC RTS
    *                                     input line DSR = output line from PC DTR
    *
    *            the PC simulates a DCE now, from this follows RTS = CTS, DTR = DSR
    */

    status->DTR             =   comport_GetStateDSR (pV24Chan->dwComPortHandle);
    status->RTS             =   comport_GetStateCTS (pV24Chan->dwComPortHandle);

    status->CTS             =   comport_GetStateRTS (pV24Chan->dwComPortHandle);
    status->DSR             =   comport_GetStateDTR (pV24Chan->dwComPortHandle);

    return 0;
}

vINT16 vme_util_vatcGetLineStatus(vmeT_ComPortChan ComPortChan, VME_VATCSTATUS * status)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);


    status->CD  =   0;  // raku: exists line for pc simulation????
    status->RI  =   comport_GetStateRING (pV24Chan->dwComPortHandle);

    return 0;
}

vINT16 vme_util_v24SetLineStatus(vmeT_ComPortChan ComPortChan, VME_V24STATUS * status)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    /**
    * attention: we simulate vme interface in module
    *            from view of the module: input line CTS = output line from PC RTS
    *                                     input line DSR = output line from PC DTR
    *
    *            the PC simulates a DCE now, from this follows RTS = CTS, DTR = DSR
    */

    comport_SetStateDTR (pV24Chan->dwComPortHandle, status->SetDSR ? TRUE : FALSE);
    comport_SetStateRTS (pV24Chan->dwComPortHandle, status->SetCTS ? TRUE : FALSE);

    return 0;
}


vINT16 vme_util_v24ClearReadBuffer(vmeT_ComPortChan ComPortChan)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    comport_ClearRXBuffer (pV24Chan->dwComPortHandle);
    return 0;
}


vINT16 vme_util_v24GetReadBufferLength(vmeT_ComPortChan ComPortChan)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    return (vINT16)comport_PeekRXBuffer (pV24Chan->dwComPortHandle);
}

vINT16 vme_util_v24SetFlowControl(vmeT_ComPortChan ComPortChan, VME_V24FLOWCTRL fctrl)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    if (eFCTRL_HARD == fctrl) {
        comport_SetRTSControlHandshake (pV24Chan->dwComPortHandle);
        return 0;
    }
    return -1;
}


vINT32 vme_util_v24SetBaudrate(vmeT_ComPortChan ComPortChan, vINT32 baudrate)
{
    vmeT_V24Chan    *pV24Chan;

    pV24Chan = GetV24Channel (ComPortChan);

    comport_SetBaudRate (pV24Chan->dwComPortHandle, (DWORD)baudrate);
    return baudrate;
}
#endif

/**
* -----------------------------------------------------------------------------
* flash file system
* -----------------------------------------------------------------------------
*/
/*
vFS_RESULT_STRUCT *vme_util_GetMemForFFSResult (void)
{
    vFS_RESULT_STRUCT   *pRes;

    pRes =  malloc (sizeof (vFS_RESULT_STRUCT));

    assert (pRes != NULL);

    return pRes;
}


void vme_util_FreeMemForFFSResult (vFS_RESULT_STRUCT *pRes)
{
    assert (pRes != NULL);

    free (pRes);

    return;
}

*/
#endif