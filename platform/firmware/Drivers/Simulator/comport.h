/*****************************************************************************
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
#ifdef  OS_WIN32
#ifndef _COMPORT_H
#define _COMPORT_H


/** 
* error values
*/
enum {
    COMPORT_OK  =   0,
    ERR_NO_PORT_RESOURCE,
    ERR_PORT_CANNOT_BE_OPENED,
    ERR_CANNOT_CREATE_EVENTTHREAD,
    ERR_READ_FROM_PORT,
    ERR_FLUSH
};


typedef void (*comportT_ComEventCallback)     (DWORD dwHandle, DWORD dwEvent);



void comport_Init (void);
DWORD comport_Open (
    DWORD                       dwPortNum, 
    DWORD                       dwBaudRate, 
    comportT_ComEventCallback   pEventCallback, 
    DWORD                     * pdwPortHandle
);


void comport_Close (DWORD dwPortHandle);
DWORD comport_Read (DWORD dwPortHandle, BYTE *pReadBuffer, DWORD dwNumberOfBytesToRead, DWORD *pdwBytesRead);
DWORD comport_Write (DWORD dwPortHandle, BYTE *pWriteBuffer, DWORD dwNumberOfBytesToWrite);
DWORD comport_SetBaudRate (DWORD dwPortHandle, DWORD dwBaudRate);
DWORD comport_GetBaudRate (DWORD dwPortHandle);
DWORD comport_Flush (DWORD dwPortHandle);
BOOL comport_GetStateCTS (DWORD dwPortHandle);
BOOL comport_GetStateDSR (DWORD dwPortHandle);
BOOL comport_GetStateRING (DWORD dwPortHandle);
DWORD comport_SetStateRTS (DWORD dwPortHandle, BOOL bState);
DWORD comport_SetStateDTR (DWORD dwPortHandle, BOOL bState);
BOOL comport_GetStateRTS (DWORD dwPortHandle);
BOOL comport_GetStateDTR (DWORD dwPortHandle);
void comport_ClearRXBuffer (DWORD dwPortHandle);
DWORD comport_PeekRXBuffer (DWORD dwPortHandle);
DWORD comport_SetRTSControlHandshake (DWORD dwPortHandle);


#endif // _COMPORT_H
#endif
