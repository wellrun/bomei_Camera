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
 * 15.04.2003    KR36712    add VATC_CHANNEL_2, VATC_CHANNEL_3
 *                          add vme_engine_vatcSendEsc () for vatc interface
 *
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
#include "vme_event.h"
#include "Fwl_oscom.h"
#include "Gbl_Global.h"

// #include "eng_ip.h"

// for AK_Activate_HISR,
#include "akos_api.h"
#include "fwl_osMalloc.h"
//
// #include "mux_api.h"

extern vVOID VME_Main(vVOID);
extern T_S32 Ak_AppFrame_Init( T_VOID );


/*****************************************************************************
 * defines
 *****************************************************************************
*/

#define MAX_VATC_CHANNEL        3

/*****************************************************************************
 * globals
 *****************************************************************************
*/

/**
VME general
*/
vT_MainCallback     fpMainCallback  = NULL;

/**
keypad
*/
vBOOL               bKeyPadIsOpen   = vFALSE;

HANDLE                SynchEvent;

/**
vatc
*/
vINT16 TabVatcDeviceHandle[MAX_VATC_CHANNEL] = {
    0,                                              // eVATC ATC1   => TabDeviceHandle[0]
    0,                                              // eVATC ATC2   => TabDeviceHandle[1]
    0                                               // eVATC ATC3   => TabDeviceHandle[2]
};

/**
v24
*/
vINT16  V24Handle = 0;




T_U8 gb_V24Buffer[2000];
//unsigned char *V24buf = gb_V24Buffer;
T_S32 gb_V24ComHead=0, gb_V24ComTail=0;

/*****************************************************************************
 * functions
 *****************************************************************************
*/

void ResetEventxx(void)
{
    SetEvent(SynchEvent);
}
#if 0
static VME_VATCCHANID GetNewVatcChanId (VME_VATC_CHAN VatcChan)
{
    switch (VatcChan)
    {
        case eVATC_ATC1:
            TabVatcDeviceHandle [0] = VATC_CHANNEL_1;
            return (vINT16)VATC_CHANNEL_1;
        case eVATC_ATC2:
            TabVatcDeviceHandle [1] = VATC_CHANNEL_2;
            return (vINT16)VATC_CHANNEL_2;
        case eVATC_ATC3:
            TabVatcDeviceHandle [2] = VATC_CHANNEL_3;
            return (vINT16)VATC_CHANNEL_3;
    }
    assert (FALSE);
    return 0;
}

static BOOL ExistVatcChanId (VME_VATC_CHAN VatcChan)
{
    switch (VatcChan)
    {
        case eVATC_ATC1:
            return TabVatcDeviceHandle [0] == VATC_CHANNEL_1 ? TRUE : FALSE;
        case eVATC_ATC2:
            return TabVatcDeviceHandle [1] == VATC_CHANNEL_2 ? TRUE : FALSE;
        case eVATC_ATC3:
            return TabVatcDeviceHandle [2] == VATC_CHANNEL_3 ? TRUE : FALSE;
    }
    assert (FALSE);
    return FALSE;
}

static BOOL IsValidVatcChanId (VME_VATCCHANID VatcChanId)
{
    int i;

    if( MAX_CHANNEL_NUM == 1 && VatcChanId == V24_CHANNEL )
    {
        return TRUE;
    }

    if (VatcChanId == 0)
    {
        return FALSE;
    }

    for (i = 0; i < MAX_VATC_CHANNEL; i++)
    {
        if (TabVatcDeviceHandle[i] == VatcChanId)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static void FreeVatcChanId (VME_VATCCHANID VatcChanId)
{
    int i;

    for (i = 0; i < MAX_VATC_CHANNEL; i++)
    {
        if (TabVatcDeviceHandle[i] == VatcChanId)
        {
            TabVatcDeviceHandle[i] = 0;
            return;
        }
    }
    assert (FALSE);
    return;
}

void V24Port_ComEventHandler (vmeT_ComPortChan ComPortChan, vUINT32 ComEvent)
{
    switch (ComEvent)
    {
        case EV_RXCHAR:
            vme_event_Send_VME_EVT_V24_RECEIVED_DATA_AVAIL ((vUINT8)V24Handle);
            break;
        case EV_CTS:
        case EV_DSR:
            {
                VME_V24STATUS   status;
                vme_util_v24GetLineStatus (V24_CHANNEL, &status);
                vme_event_Send_VME_EVT_V24_STATUS_LINES_CHANGED ((vUINT8)V24Handle, (vUINT8)status.DTR, (vUINT8)status.RTS);
            }
            break;
        default:
            break;
    }
    return;
}

void VATCPort_ComEventHandler (vmeT_ComPortChan ComPortChan, vUINT32 ComEvent)
{
    T_U8    rxChar;

    switch (ComEvent)
    {
        case EV_RXCHAR:
            while(vme_util_ReadV24Port(ComPortChan, &rxChar, 1)==1)
            {
                gb_VComBuffer[ gb_VComTail ] = rxChar;
                gb_VComTail++;
                gb_VComTail %= MAXSIZE_VCOMBUFFER;

                //if( ((gb_bOnNetwork)/* && (buf == 0x7e)*/)        // under gb_bOnNetwork mode, not only read ppp data but also read at command
                /*if(gb_bOnNetwork)
                {
                    if(gb_bOnNetPPP)//NET_PPP*/
                if(!gb.bMuxMode)
                {
                    if(gb_net_phase == NET_PPP)
                    {
                        if(rxChar == PPP_FLAG)
                        {
                            vme_event_Send_VME_EVT_VATC_RECEIVED_DATA_AVAIL ((T_U8)ComPortChan);
                            continue;
                        }
                    }
                    else if(gb_net_phase == NET_AT || gb_net_phase == NET_OFF_HOLD)
                    {
                        if(rxChar == AT_END_FLAG)
                        {
                            vme_event_Send_VME_EVT_VATC_RECEIVED_DATA_AVAIL ((T_U8)ComPortChan);
                            continue;
                        }
                    }
                    //}
                    else
                    {
                        if(rxChar == AT_END_FLAG||SMS_SEND_FLAG == rxChar)
                        {
                            vme_event_Send_VME_EVT_VATC_RECEIVED_DATA_AVAIL ((T_U8)ComPortChan);
                            continue;
                        }
                    }
                }
                else
                {
                    if(rxChar == CMUX_FLAG_BASIC_MODE)        
                    {
                        vme_event_Send_VME_EVT_VATC_RECEIVED_MUX_DATA((T_U8)ComPortChan);
                    }
                    continue;

                }
            }
            //vme_event_Send_VME_EVT_VATC_RECEIVED_DATA_AVAIL ((vUINT8)ComPortChan);
            break;
        case EV_RING:
            {
                VME_VATCSTATUS   status;
                vme_util_vatcGetLineStatus (ComPortChan, &status);
                vme_event_Send_VME_EVT_VATC_STATUS_LINES_CHANGED ((vUINT8)ComPortChan, (vUINT8)status.RI, (vUINT8)status.CD);
            }
            break;
        default:
            break;
    }
    return;
}
#endif

/*****************************************************************************
 * interface
 *****************************************************************************
*/

enum_vmeeng_ret_t vme_engine_Init (void)
{

    // create event-queue
    vme_util_CreateVMEEventQueue ();

    // init members
    bKeyPadIsOpen   =   vFALSE;

    // init flash file system simulation
//  fsfflash_Init ();
    // call VME_Main () from userware

    Fwl_MallocInit();
    Fwl_InitTimer();

    VME_Main();

    SynchEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    // start AppFrame Init, for win32, added by Dingliwei at Sep,17th,2007
    Ak_AppFrame_Init();

    WaitForSingleObject(SynchEvent, INFINITE);
    CloseHandle(SynchEvent);
    // send event VME_EVT_SYSSTART
    vme_event_Send_VME_EVT_SYSSTART();
    
//    WaitForSingleObject(SynchEvent, INFINITE);

   return VMEENG_OK;
//     return engRet;
}

void vme_engine_SetMainCallBack (vT_MainCallback pMainCallback)
{
    fpMainCallback  = pMainCallback;

    return;
}


void vme_engine_HandleEventQueue (void)
{
    T_SYS_EVTID     EvtCode;
    T_SYS_PARAM     EvtParam;
//    vBOOL           EvtParamExist;

    // check event queue for events to userware
    // call MainCallback
    if (vTRUE == vme_util_GetFromVMEEventQueue (&EvtCode, &EvtParam))
    {
        assert (fpMainCallback != NULL);
        if (NULL != fpMainCallback)
        {

            // pre process for events
            switch (VME_EVT_MAINCODE (EvtCode))
            {
                case VME_EVT_KEYPAD:
                    break;

                default:
                    break;
            }


            fpMainCallback ((vT_EvtCode)EvtCode, (vT_EvtParam*)&EvtParam );


            // post process for events
            switch (VME_EVT_MAINCODE (EvtCode))
            {
                default:
                    break;
            }

        }
    }

    return;
}


void vme_engine_Term (void)
{
    fpMainCallback = NULL;

//     vme_utilCLoseV24Port (V24_CHANNEL);
//     vme_utilCLoseV24Port (VATC_CHANNEL_1);
//     vme_utilCLoseV24Port (VATC_CHANNEL_2);
//     vme_utilCLoseV24Port (VATC_CHANNEL_3);

    vme_util_DestroyVMEEventQueue ();

    vme_util_CloesAppl ();

    return;
}

/*****************************************************************************
 * keypad
 *****************************************************************************
*/
#if 0
/*****************************************************************************
 * vatc
 *****************************************************************************
*/

VME_VATCCHANID vme_engine_vatcOpen(VME_VATC_CHAN Channel)
{
    gb_VComHead = 0;
    gb_VComTail = 0;

    if (ExistVatcChanId (Channel))
    {
        return -1;
    }

    return GetNewVatcChanId (Channel);
}

vINT16 vme_engine_vatcClose(VME_VATCCHANID ChanId)
{
    if (!IsValidVatcChanId (ChanId))
    {
        return -1;
    }

    FreeVatcChanId (ChanId);

    return 0;
}

vINT16 vme_engine_vatcRead(VME_VATCCHANID ChanId, vPDATA data, vINT16 count)
{
    int i=0;

    if (!IsValidVatcChanId (ChanId))
    {
        return -1;
    }

    while( gb_VComHead != gb_VComTail )
    {
        if( i>=count )
        {
            break;
        }
        data[i++] = gb_VComBuffer[gb_VComHead++];
        gb_VComHead %= MAXSIZE_VCOMBUFFER;
    }
    return i;
    //return vme_util_ReadV24Port (ChanId, data, count);
}

vINT16 vme_engine_vatcReadLine(VME_VATCCHANID ChanId, vPDATA data, vINT16 count)
{
    int i=0;
    T_U32 tempHead = gb_VComHead;

    //if (!IsValidVatcChanId (ChanId))
    //{
    //    return -1;
    //}
 
    while( tempHead != gb_VComTail )
    {
        if( i>=count )
        {
            break;
        }
        data[i++] = gb_VComBuffer[tempHead++];
        tempHead %= MAXSIZE_VCOMBUFFER;

        if( data[i-1] == 0x0a || data[i-1] == '>' )
        {
            data[ i ] = 0;
            gb_VComHead = tempHead;
            return i;
        }
    }

    return 0;
    //return vme_util_ReadV24Port (ChanId, data, count);
}

vINT16 vme_engine_vatcWrite(VME_VATCCHANID ChanId, const vPDATA data, vINT16 count)
{
    vINT16 BytesWritten = 0;

    if (!IsValidVatcChanId (ChanId))
    {
        return -1;
    }

    BytesWritten = vme_util_WriteV24Port (ChanId, data, count);

    if (BytesWritten < count)
    {
        /**
        * attention: type cast of V24Handle is a open problem from vme interface
        */
        vme_event_Send_VME_EVT_VATC_WRITE_CALLBACK ((vUINT8)ChanId);
    }

    return (BytesWritten);
}

vINT16 vme_engine_vatcWriteBlocked(VME_VATCCHANID ChanId, const vPDATA data, vINT16 count)
{
    vINT16 BytesWritten = 0;

    if (!IsValidVatcChanId (ChanId))
    {
        return -1;
    }

    BytesWritten = vme_util_WriteV24Port (ChanId, data, count);

    return (BytesWritten);
}


vINT16 vme_engine_vatcSendEsc(VME_VATCCHANID ChanId)
{
    if (!IsValidVatcChanId (ChanId))
    {
        return -1;
    }

    // wait 1 sec -> send "+++" -> wait 1 sec
    Sleep (1100);

    vme_util_WriteV24Port (ChanId, "+++", 3);

    Sleep (1100);

    return 0;
}

vINT16 vme_engine_vatcGetLineStatus(VME_VATCCHANID ChanId,VME_VATCSTATUS * status)
{
    if (!IsValidVatcChanId (ChanId))
    {
        return -1;
    }

    return vme_util_vatcGetLineStatus (ChanId, status);
}

vINT16 vme_engine_vatcClearReadBuffer(VME_VATCCHANID ChanId)
{
    if (!IsValidVatcChanId (ChanId))
    {
        return -1;
    }

    return vme_util_v24ClearReadBuffer (ChanId);
}

vINT16 vme_engine_vatcGetReadBufferLength(VME_VATCCHANID ChanId)
{
    if (!IsValidVatcChanId (ChanId))
    {
        return -1;
    }

    return vme_util_v24GetReadBufferLength (ChanId);
}


/*****************************************************************************
 * v24
 * com ports on PC are already opened
 * this are logical functions
 *****************************************************************************
*/


VME_V24DEVHANDLE vme_engine_v24Open(void)
{
    gb_V24ComHead = 0;
    gb_V24ComTail = 0;

    if (V24Handle > 0)
    {
        return -1;
    }
    V24Handle++;

    return V24Handle;
}

vINT16 vme_engine_v24Close(VME_V24DEVHANDLE devHandle)
{
    if (V24Handle != devHandle)
    {
        return -1;
    }

    V24Handle--;

    return 0;
}

vINT16 vme_engine_v24Write(VME_V24DEVHANDLE devHandle, const vPDATA data, vINT16 count)
{
    vINT16 BytesWritten = 0;

    if (V24Handle != devHandle)
    {
        return -1;
    }

    BytesWritten = vme_util_WriteV24Port (V24_CHANNEL, data, count);

    if (BytesWritten < count)
    {
        /**
        * attention: type cast of V24Handle is a open problem from vme interface
        */
        //vme_event_Send_VME_EVT_V24_WRITE_CALLBACK ((vUINT8)V24Handle);
    }

    return (BytesWritten);
}

vINT16 vme_engine_v24WriteBlocked(VME_V24DEVHANDLE devHandle, const vPDATA data, vINT16 count)
{
    vINT16 BytesWritten = 0;

    if (V24Handle != devHandle)
    {
        return -1;
    }

    BytesWritten = vme_util_WriteV24Port (V24_CHANNEL, data, count);


    return (BytesWritten);
}

vINT16 vme_engine_v24Read(VME_V24DEVHANDLE devHandle, vPDATA data, vINT16 count)
{
    if (V24Handle != devHandle)
    {
        return -1;
    }

    return vme_util_ReadV24Port (V24_CHANNEL, data, count);
}

vINT16 vme_engine_v24Flush(VME_V24DEVHANDLE devHandle)
{
    if (V24Handle != devHandle)
    {
        return -1;
    }

    if (vTRUE != vme_util_v24Flush(V24_CHANNEL))
    {
        return -1;
    }

    return 0;
}

vINT32 vme_engine_v24GetBaudrate(VME_V24DEVHANDLE devHandle)
{
    if (V24Handle != devHandle)
    {
        return -1;
    }

    return vme_util_v24GetBaudrate (V24_CHANNEL);
}

vINT16 vme_engine_v24GetLineStatus(VME_V24DEVHANDLE devHandle,VME_V24STATUS * status)
{
    if (V24Handle != devHandle)
    {
        return -1;
    }

    return vme_util_v24GetLineStatus (V24_CHANNEL, status);
}

vINT16 vme_engine_v24SetLineStatus(VME_V24DEVHANDLE devHandle, VME_V24STATUS * status)
{
    if (V24Handle != devHandle)
    {
        return -1;
    }

    return vme_util_v24SetLineStatus (V24_CHANNEL, status);
}

vINT16 vme_engine_v24ClearReadBuffer(VME_V24DEVHANDLE devHandle)
{
    if (V24Handle != devHandle)
    {
        return -1;
    }

    return vme_util_v24ClearReadBuffer (V24_CHANNEL);
}


vINT16 vme_engine_v24GetReadBufferLength(VME_V24DEVHANDLE devHandle)
{
    if (V24Handle != devHandle)
    {
        return -1;
    }

    return vme_util_v24GetReadBufferLength (V24_CHANNEL);
}

vINT16 vme_engine_v24SetFlowControl(VME_V24DEVHANDLE devHandle, VME_V24FLOWCTRL fctrl)
{
    if (V24Handle != devHandle)
    {
        return -1;
    }

    /**
    * PC simulation sopports only eFCTRL_HARD
    */
    if (eFCTRL_HARD != fctrl)
    {
        return -1;
    }

    return vme_util_v24SetFlowControl (V24_CHANNEL, fctrl);
}


vINT32 vme_engine_v24SetBaudrate(VME_V24DEVHANDLE devHandle, vINT32 baudrate)
{
    if (V24Handle != devHandle)
    {
        return -1;
    }

    return vme_util_v24SetBaudrate (V24_CHANNEL, baudrate);
}
#endif

/*****************************************************************************
 * timer
 *****************************************************************************
*/


#endif
