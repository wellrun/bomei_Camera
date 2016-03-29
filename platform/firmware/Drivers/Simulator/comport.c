/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name£ºcomport.c
 * Function£º Serial port operation
 *
 * Author£ºMiao Baoli
 * Date£º2004-05-16
 * Version£º1.0		  
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/
#ifdef   OS_WIN32
#include <stdio.h>
#include <assert.h>

#include "Windows.h"
#include "comport.h"
#include "Eng_Debug.h"
#include "hal_print.h"
/*****************************************************************************
 * defines
 *****************************************************************************
*/


#define MAX_COM_PORTS           5
#define MAX_PORTMODE_STRING     500


#define PORTNAME_PREFIX         "\\\\.\\COM"

#define COM_EVENT_MASK_ALL_EVENTS            (EV_CTS    | \
                                              EV_DSR    | \
                                              EV_RING   | \
                                              EV_RXCHAR)


typedef struct {
    HANDLE                      hPort;
    HANDLE                      hThread;
    DWORD                       dwPortNum;
    DCB                         Dcb;
    comportT_ComEventCallback   EventCallback;
    BOOL                        bStateRTS;
    BOOL                        bStateDTR;
} comportT_Port;




/*****************************************************************************
 * globals
 *****************************************************************************
*/

/**
array for available com ports
*/

comportT_Port   PortTab [MAX_COM_PORTS];

/*****************************************************************************
 * functions
 *****************************************************************************
*/
/*----------------------------------------------------------------	
 * brief : 	Judge the com port pointer is valid or not
 * author:	
 * param:	comportT_Port *pPort: The pointer of a com port 
 * retval:	TRUE: The pointer is valid; False: The pointer is invalid;
 * history:	
 -----------------------------------------------------------------*/
static BOOL IsPortPointerValid (comportT_Port *pPort)
{
    int i;

    assert (pPort != NULL);

    for (i = 0; i < MAX_COM_PORTS; i++)
    {
        if (pPort == &PortTab[i])
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*----------------------------------------------------------------	
 * brief : 	Judge the com port open or not
 * author:	
 * param:	comportT_Port *pPort:  The pointer of a com port 
 * retval:	TRUE: the port is open; FALSE: the port is close;
 * history:	
 -----------------------------------------------------------------*/
static BOOL IsPortOpened (comportT_Port *pPort)
{
    assert (pPort != NULL);

    if (INVALID_HANDLE_VALUE != pPort->hPort)
    {
        return TRUE;
    }
    return FALSE;
}

/*----------------------------------------------------------------	
 * brief : 	Get a com port
 * author:	
 * param:	
 * retval:	return The pointer of the port we get: we can get a port;
 * retval:	return NULL: all the com port has been used;
 * history:	
 -----------------------------------------------------------------*/
static comportT_Port *GetNewPort (void)
{
    int i;

    for (i = 0; i < MAX_COM_PORTS; i++)
    {
        if (INVALID_HANDLE_VALUE == PortTab[i].hPort)
        {
            return &PortTab[i];
        }
    }
    return NULL;
}

/*----------------------------------------------------------------	
 * brief : 	Free a com port
 * author:	
 * param:	comportT_Port *pPort: The pointer of the com port which is going to free.
 * retval:	
 * history:	
 -----------------------------------------------------------------*/
static void FreePort (comportT_Port *pPort)
{
    pPort->hPort = INVALID_HANDLE_VALUE;
}

/*----------------------------------------------------------------	
 * brief : 	change pointer type to DWORD type 
 * author:	
 * param:	comportT_Port *pPort: the com port pointer
 * retval:	return a DWORD num which changed from the com port pointer, the num is used in the windows application
 * history:	
 -----------------------------------------------------------------*/
static DWORD PortToHandle (comportT_Port *pPort)
{
    return (DWORD) pPort;
}

/*----------------------------------------------------------------	
 * brief : 	change DWORD type to pointer type
 * author:	
 * param:	DWORD dwHandle: dwHandle is used in the windows application
 * retval:	return a comportT_Port pointer which point to the com port structrue 
 * history:	
 -----------------------------------------------------------------*/
static comportT_Port *HandleToPort (DWORD dwHandle)
{
    return (comportT_Port *)dwHandle;
}

/**
* thread for port events
*/
/*----------------------------------------------------------------	
 * brief : 	read the com port event and deal with the event
 * author:	
 * param:	PVOID pData: the pointer to the data buffer
 * retval:	-1: the port pointer is invalid; -2: pending error; -3: com port error; 0: treat with event
 * history:	
 -----------------------------------------------------------------*/
DWORD WINAPI EventReader (PVOID pData)
{
    comportT_Port   *pPort;
    DWORD           dwEventMask;
    DWORD           dwHandle;
    OVERLAPPED      ovl = {0, 0, 0, 0, CreateEvent( NULL, TRUE, FALSE, NULL ) };

    pPort = (comportT_Port *)pData;

    if (INVALID_HANDLE_VALUE == pPort->hPort)
    {
        return -1;
    }

    while (1)
    {
        if (!WaitCommEvent (pPort->hPort, &dwEventMask, &ovl))
        {
            if ( ERROR_IO_PENDING == GetLastError() )
            {
                DWORD dwBytesTransferred=0;
                if ( !GetOverlappedResult( pPort->hPort, &ovl, &dwBytesTransferred, TRUE ) )
                {
                    CloseHandle ( ovl.hEvent );
                    return -2;
                }
            }
            else
            {
                CloseHandle ( ovl.hEvent );
                return -3;
            }
        }          

        dwHandle = PortToHandle (pPort);

        if ( dwEventMask & EV_ERR )    { pPort->EventCallback( dwHandle, EV_ERR );    }
        if ( dwEventMask & EV_BREAK )  { pPort->EventCallback( dwHandle, EV_BREAK );  }
        if ( dwEventMask & EV_CTS )    { pPort->EventCallback( dwHandle, EV_CTS );    }
        if ( dwEventMask & EV_DSR )    { pPort->EventCallback( dwHandle, EV_DSR );    }
        if ( dwEventMask & EV_RING )   { pPort->EventCallback( dwHandle, EV_RING );   }
        if ( dwEventMask & EV_RXCHAR ) 
		{
			pPort->EventCallback( dwHandle, EV_RXCHAR );
		}

        if ( dwEventMask & EV_RLSD )   { pPort->EventCallback( dwHandle, EV_RLSD );   }
        if ( dwEventMask & EV_RXFLAG ) { pPort->EventCallback( dwHandle, EV_RXFLAG ); }
        if ( dwEventMask & EV_TXEMPTY ){ pPort->EventCallback( dwHandle, EV_TXEMPTY );}

    }

    CloseHandle (ovl.hEvent);
    return 0;
}


/*****************************************************************************
 * interface
 *****************************************************************************
*/
/*----------------------------------------------------------------	
 * brief : 	The com port initialition
 * author:	
 * param:	
 * retval:	
 * history:	
 -----------------------------------------------------------------*/
void comport_Init (void)
{
    int i;

    for (i = 0; i < MAX_COM_PORTS; i++)
    {
        // reset PortTab
        PortTab[i].hPort            =   INVALID_HANDLE_VALUE;
        PortTab[i].hThread          =   INVALID_HANDLE_VALUE;
        PortTab[i].dwPortNum        =   0;
        PortTab[i].EventCallback    =   NULL;
    }    

    return;
}

/*----------------------------------------------------------------	
 * brief : 	open the com port
 * author:	
 * param:	DWORD dwPortNum:  the com port num
 * param:       DWORD dwBaudRate: The com port baudrate
 * param:       comportT_ComEventCallback pEventCallback: The call back function of the com port
 * param:       DWORD * pdwPortHandle:
 * retval:	
 * history:	
 -----------------------------------------------------------------*/
DWORD comport_Open (
    DWORD                       dwPortNum, 
    DWORD                       dwBaudRate, 
    comportT_ComEventCallback   pEventCallback, 
    DWORD                     * pdwPortHandle
)
{
    comportT_Port   *pPort;
    char            strPortName[_MAX_PATH];
    DWORD           dwThreadId;
    COMMTIMEOUTS    ComTimeOut;
	BOOL			fSuccess;

    /**
    * get structure for port parameters
    */
    pPort = GetNewPort ();
    if (NULL == pPort)
    {
        return ERR_NO_PORT_RESOURCE;
    }

    pPort->dwPortNum        = dwPortNum;
    pPort->EventCallback    = pEventCallback;

    /**
    * build filename for requested port
    */
    sprintf (strPortName, "%s%d", PORTNAME_PREFIX, dwPortNum);

    /**
    * open port
    */
    pPort->hPort    =   CreateFile (
                            strPortName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_OVERLAPPED,
                            NULL
                        );

    if (INVALID_HANDLE_VALUE == pPort->hPort)
    {
        FreePort (pPort);
        return ERR_PORT_CANNOT_BE_OPENED;
    }

    /**
    * set port configuration
    */
	fSuccess = GetCommState( pPort->hPort, &pPort->Dcb );
	if (!fSuccess) {
	  // Handle the error.
	  Fwl_Print(C2, M_DRVSYS, "GetCommState failed with error %d.\n", GetLastError());
	  return ERR_PORT_CANNOT_BE_OPENED;
	}

	pPort->Dcb.BaudRate = dwBaudRate;
	pPort->Dcb.ByteSize = 8;
	pPort->Dcb.Parity	= NOPARITY;
	pPort->Dcb.StopBits = ONESTOPBIT;

    fSuccess = SetCommState (pPort->hPort, &pPort->Dcb);
	if (!fSuccess) {
	  // Handle the error.
	  Fwl_Print(C2, M_DRVSYS, "SetCommState failed with error %d.\n", GetLastError());
	  return ERR_PORT_CANNOT_BE_OPENED;
	}

    pPort->bStateDTR    =   TRUE;
    pPort->bStateRTS    =   TRUE;

    /**
    * set eventmask
    */
    PurgeComm (pPort->hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
    SetCommMask (pPort->hPort, COM_EVENT_MASK_ALL_EVENTS);

    /**
    * set timouts for operations with com port
    */
    ComTimeOut.ReadIntervalTimeout          =   0;
    ComTimeOut.ReadTotalTimeoutConstant     =   20;
    ComTimeOut.ReadTotalTimeoutMultiplier   =   0;
    ComTimeOut.WriteTotalTimeoutConstant    =   0;
    ComTimeOut.WriteTotalTimeoutMultiplier  =   0;


    SetCommTimeouts (pPort->hPort, &ComTimeOut);

    /**
    * create thread for port events
    */
    pPort->hThread  =   CreateThread (
                            NULL,           // pointer to thread security attributes
                            0,              // stack size
                            EventReader,    // thread service routine
                            pPort,          // argument for new thread -> pointer port description
                            0,              // creation flags
                            &dwThreadId     // pointer to returned thread identifier
                        );

    if (INVALID_HANDLE_VALUE == pPort->hPort)
    {
        FreePort (pPort);
        return ERR_CANNOT_CREATE_EVENTTHREAD;
    }

    /**
    * ok, port is open
    */
    *pdwPortHandle = PortToHandle (pPort);

    return COMPORT_OK;
}

/*----------------------------------------------------------------	
 * brief : 	Close the com port 
 * author:	
 * param:	DWORD dwPortHandle: com port handle in windows
 * retval:	
 * history:	
 -----------------------------------------------------------------*/
void comport_Close (DWORD dwPortHandle)
{
    comportT_Port   *pPort;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return;

    FlushFileBuffers (pPort->hPort);
    PurgeComm (pPort->hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

    /**
    * stop thread for port events
    */
    if (INVALID_HANDLE_VALUE != pPort->hThread)
    {
        TerminateThread(pPort->hThread, 0);
        CloseHandle (pPort->hThread  );
    }

    /**
    * close port
    */
    if (INVALID_HANDLE_VALUE != pPort->hPort)
    {
        CloseHandle (pPort->hPort);
    }

    FreePort (pPort);

    return;
}

/*----------------------------------------------------------------	
 * brief : 	read data from the com port
 * author:	
 * param:	DWORD dwPortHandle: the com port handle in windows
 * param:       BYTE *pReadBuffer: a pointer of a data buffer,which will store the data read from the com port
 * param:       DWORD dwNumberOfBytesToRead: the quantity of data expect to be read
 * param:       DWORD *pdwBytesRead: the quantity of data which have read
 * retval:	return ERR_NO_PORT_RESOURCE: The port is close or the port pointer is invalid;
                return COMPORT_OK: can read data from the com port
 * history:	
 -----------------------------------------------------------------*/
DWORD comport_Read (DWORD dwPortHandle, BYTE *pReadBuffer, DWORD dwNumberOfBytesToRead, DWORD *pdwBytesRead)
{
    comportT_Port   *pPort;
    OVERLAPPED      ovl = { 0, 0, 0, 0, CreateEvent( NULL, TRUE, FALSE, NULL ) };
    DWORD           dwRet = COMPORT_OK;

//	  assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));
    assert (pReadBuffer != NULL);

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return ERR_NO_PORT_RESOURCE;

    // attempt an asynchronous read operation
    if (!ReadFile(pPort->hPort, pReadBuffer, dwNumberOfBytesToRead, pdwBytesRead, &ovl))
    {
        DWORD dwErrorCode = GetLastError();

        // deal with the error code
        if (dwErrorCode == ERROR_IO_PENDING)
        {
            // if there was still a problem ...
            if (!GetOverlappedResult(pPort->hPort, &ovl, pdwBytesRead, TRUE))
            {
                dwRet   = ERR_READ_FROM_PORT;
            }
        }
    }

    CloseHandle ( ovl.hEvent );

    return COMPORT_OK;
}

/*----------------------------------------------------------------	
 * brief : 	write data to the com port
 * author:	
 * param:	DWORD dwPortHandle: the com port handle in windows
 * param:       BYTE *pWriteBuffer: the pointer of the data buffer which will be wrote to the com port 
 * param:       DWORD dwNumberOfBytesToWrite: the quantity of data which is expect to write
 * retval:	return ERR_NO_PORT_RESOURCE:the pointer of the port is close or the pointer is invalid;
                return dwBytesWritten(the quantity of data has been wrote): can write data to the com port
 * history:	
 -----------------------------------------------------------------*/
DWORD comport_Write (DWORD dwPortHandle, BYTE *pWriteBuffer, DWORD dwNumberOfBytesToWrite)
{
    DWORD           dwBytesWritten = 0;
    comportT_Port   *pPort;
    OVERLAPPED      ovl = { 0, 0, 0, 0, CreateEvent( NULL, TRUE, FALSE, NULL ) };

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));
    assert (pWriteBuffer != NULL);

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return ERR_NO_PORT_RESOURCE;

    // attempt an asynchronous write operation
    if (!WriteFile(pPort->hPort, pWriteBuffer, dwNumberOfBytesToWrite, &dwBytesWritten, &ovl))
    {
        DWORD dwErrorCode = GetLastError();

        // deal with the error code
        if (dwErrorCode == ERROR_IO_PENDING)
        {
            // if there was still a problem ...
            if (!GetOverlappedResult(pPort->hPort, &ovl, &dwBytesWritten, TRUE))
            {
                assert (FALSE);
            }
        }
    }

    FlushFileBuffers (pPort->hPort);
    CloseHandle ( ovl.hEvent );
    
    return dwBytesWritten;
}

/*----------------------------------------------------------------	
 * brief : 	set the baudrate of the com port
 * author:	
 * param:	DWORD dwPortHandle: the com port handle in windows
 * param:       DWORD dwBaudRate: baudrate of the com port
 * retval:	return ERR_NO_PORT_RESOURCE:the pointer of the port is close or the pointer is invalid;
                return COMPORT_OK: the com port can set baudrate 
 * history:	
 -----------------------------------------------------------------*/
DWORD comport_SetBaudRate (DWORD dwPortHandle, DWORD dwBaudRate)
{
    comportT_Port   *pPort;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return ERR_NO_PORT_RESOURCE;

    pPort->Dcb.BaudRate = dwBaudRate;

    if (!SetCommState (pPort->hPort, &pPort->Dcb))
    {
        assert (FALSE);
    }

    return COMPORT_OK;
}

/*----------------------------------------------------------------	
 * brief : 	get the baudrate of the com port
 * author:	
 * param:	DWORD dwPortHandle: the com port handle in windows
 * retval:	return ERR_NO_PORT_RESOURCE:the pointer of the port is close or the pointer is invalid;
                return baudrate: the com port has been opened  
 * history:	
 -----------------------------------------------------------------*/
DWORD comport_GetBaudRate (DWORD dwPortHandle)
{
    comportT_Port   *pPort;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return ERR_NO_PORT_RESOURCE;

    return pPort->Dcb.BaudRate;
}

/*----------------------------------------------------------------	
 * brief : 	flush the com port buffer
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	ERR_NO_PORT_RESOURCE: port pointer invalid; ERR_FLUSH: flush error; COMPORT_OK: flush sucess
 * history:	
 -----------------------------------------------------------------*/
DWORD comport_Flush (DWORD dwPortHandle)
{
    comportT_Port   *pPort;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return ERR_NO_PORT_RESOURCE;

    if (!FlushFileBuffers (pPort->hPort))
    {
        return ERR_FLUSH;
    }

    return COMPORT_OK;
}

/*----------------------------------------------------------------	
 * brief : 	get the com port DSR status
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	FALSE: can not get the DSR status; TRUE: get the DSR status
 * history:	
 -----------------------------------------------------------------*/
BOOL comport_GetStateDSR (DWORD dwPortHandle)
{
    comportT_Port   *pPort;
    DWORD           dwModeStat;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return FALSE;

    if (!GetCommModemStatus (pPort->hPort, &dwModeStat))
    {
        assert (FALSE);
    }
    
    return (dwModeStat & MS_DSR_ON) ? TRUE : FALSE;
}

/*----------------------------------------------------------------	
 * brief : 	get the com port CTS status
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	FALSE: can not get the CTS status ; TRUE: get the CTS status
 * history:	
 -----------------------------------------------------------------*/
BOOL comport_GetStateCTS (DWORD dwPortHandle)
{
    comportT_Port   *pPort;
    DWORD           dwModeStat;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return FALSE;

    if (!GetCommModemStatus (pPort->hPort, &dwModeStat))
    {
        assert (FALSE);
    }
    
    return (dwModeStat & MS_CTS_ON) ? TRUE : FALSE;
}

/*----------------------------------------------------------------	
 * brief : 	get the com port RING status
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	FALSE: can not get the RING status ; TRUE: get the RING status
 * history:	
 -----------------------------------------------------------------*/
BOOL comport_GetStateRING (DWORD dwPortHandle)
{
    comportT_Port   *pPort;
    DWORD           dwModeStat;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return FALSE;

    if (!GetCommModemStatus (pPort->hPort, &dwModeStat))
    {
        assert (FALSE);
    }
    
    return (dwModeStat & MS_RING_ON) ? TRUE : FALSE;
}

/*----------------------------------------------------------------	
 * brief : 	get the com port RTS status
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	FALSE: can not get the RTS status ; TRUE: get the RTS status
 * history:	
 -----------------------------------------------------------------*/
DWORD comport_SetStateRTS (DWORD dwPortHandle, BOOL bState)
{
    comportT_Port   *pPort;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return ERR_NO_PORT_RESOURCE;

    if (!EscapeCommFunction(pPort->hPort, bState ? SETRTS : CLRRTS))
    {
        assert (FALSE);
    }

    pPort->bStateRTS = bState;

    return COMPORT_OK;
}

/*----------------------------------------------------------------	
 * brief : 	get the com port DTR status
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	FALSE: can not get the DTR status ; TRUE: get the DTR status
 * history:	
 -----------------------------------------------------------------*/
DWORD comport_SetStateDTR (DWORD dwPortHandle, BOOL bState)
{
    comportT_Port   *pPort;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return ERR_NO_PORT_RESOURCE;

    if (!EscapeCommFunction(pPort->hPort, bState ? SETDTR : CLRDTR))
    {
        assert (FALSE);
    }

    pPort->bStateDTR = bState;

    return COMPORT_OK;
}

/*----------------------------------------------------------------	
 * brief : 	get the com port RTS status
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	FALSE: can not get the RTS status ; TRUE: get the RTS status
 * history:	
 -----------------------------------------------------------------*/
BOOL comport_GetStateRTS (DWORD dwPortHandle)
{
    comportT_Port   *pPort;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));
  
	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return FALSE;
	
    return pPort->bStateRTS;
}

/*----------------------------------------------------------------	
 * brief : 	get the com port DTR status
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	FALSE: can not get the DTR status ; TRUE: get the DTR status
 * history:	
 -----------------------------------------------------------------*/
BOOL comport_GetStateDTR (DWORD dwPortHandle)
{
    comportT_Port   *pPort;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));
  
	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return FALSE;
	
    return pPort->bStateDTR;
}

/*----------------------------------------------------------------	
 * brief : 	clear the receive data buffer
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	
 * history:	
 -----------------------------------------------------------------*/
void comport_ClearRXBuffer (DWORD dwPortHandle)
{
    comportT_Port   *pPort;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return;

    if (!PurgeComm (pPort->hPort, PURGE_RXCLEAR))
    {
        assert (FALSE);
    }

    return;
}

/*----------------------------------------------------------------	
 * brief : 	get data the receive data buffer
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	
 * history:	
 -----------------------------------------------------------------*/
DWORD comport_PeekRXBuffer (DWORD dwPortHandle)
{
    comportT_Port   *pPort;
    COMSTAT         ComStat;
    DWORD           dwComError;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return ERR_NO_PORT_RESOURCE;

    if (!ClearCommError (pPort->hPort, &dwComError, &ComStat))
    {
        assert (FALSE);
    }

    return ComStat.cbInQue;    
}

/*----------------------------------------------------------------	
 * brief : 	set the com port control with handshake
 * author:	
 * param:	DWORD dwPortHandle: the com port handle
 * retval:	FALSE: can't set handshake; COMPORT_OK: set handshake success
 * history:	
 -----------------------------------------------------------------*/
DWORD comport_SetRTSControlHandshake (DWORD dwPortHandle)
{
    comportT_Port   *pPort;

//    assert ((pPort = HandleToPort (dwPortHandle)) != NULL);
//    assert (IsPortPointerValid (pPort));
//    assert (IsPortOpened (pPort));

	if ((pPort = HandleToPort (dwPortHandle)) == NULL ||
		!IsPortPointerValid (pPort) ||
		!IsPortOpened (pPort))
		return ERR_NO_PORT_RESOURCE;

    pPort->Dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;

    if (!SetCommState (pPort->hPort, &pPort->Dcb))
    {
        assert (FALSE);
    }

    return COMPORT_OK;
}
#endif