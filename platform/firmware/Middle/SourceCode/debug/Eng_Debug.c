/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name£ºEng_Debug.c
 * Function£ºThe file define some function for debuging code.
 
 *
 * Author£ºXC
 * Date£º2004-10-14
 * Version£º1.0		  
 *
 * Reversion: 
 * Author: liuyoubo
 * Date: 2006-04-14
 * Reversion: 
 * Author: ZhuSizhe
 * Date: 2006-08-29
**************************************************************************/
#include "Eng_String.h"
#include "Eng_Graph.h"
#include "Eng_Queue.h"
#include "Ctl_MsgBox.h"
#include "Fwl_pfDisplay.h"
#include "eng_debug.h"
#include "raminit.h"
#include "akos_api.h"

#ifdef SUPPORT_NAND_TRACE
#include "eng_nandtrace.h"
#endif	// end of #ifdef SUPPORT_NAND_TRACE


#ifdef OS_WIN32
#include "winvme.h"
#endif

#define DBG_TRACE_LOGLEN    	2048
#define MODULE_NAME_MAX_LEN     8
/********************************************************************
*																	*
*							DEBUG use assert						*
*																	*
********************************************************************/


static T_U8  g_dbgLevel = C3;
static T_CHR gb_printf_buf[DBG_TRACE_LOGLEN];

/*==================================================================================*/
T_S32 Fwl_SetDbgValveLevel(T_U8 level)
{
   g_dbgLevel = level;
   
   return 0;
}

T_U8  Fwl_GetDbgValveLevel(T_VOID)
{
   return g_dbgLevel;
}
/*==================================================================================*/


/**
 * @brief Check if the pointer is out of range
 * 
 * @author @b ZouMai
 * 
 * @author pyxue
 * @date 2001-09-21
 * @param T_pVOID ptr
 * @return T_BOOL
 * @retval AK_TRUE: legal
 * @retval AK_FALSE: illegal
 */
T_BOOL AkAssertCheckPointer(T_pCVOID ptr)
{
	return Ram_AssertCheckPointer(ptr);
}

/**
 * @brief Assert fail process in WIN32 OS
 * 
 * @author @b ZouMai
 * 
 * @author pyxue
 * @date 2002-12-27
 * @param T_pSTR message: user message
 * @param T_pSTR filename: file name of caller
 * @param T_U32 line: line ID of caller
 * @return T_VOID
 * @retval 
 */

T_VOID AkAssertDispMsg(T_pCSTR message, T_pCSTR filename, T_U32 line)
{
	Ram_AssertDispMsg(message, filename, line);
}


/********************************************************************
*																	*
*							DEBUG trace function					*
*																	*
********************************************************************/
#ifdef DEBUG_TRACE_FUNCTIONE
T_VOID AkFuncEnter(T_pCSTR funcname)
{
	Fwl_Print(C1, M_ENGINE, "Enter function: %s",funcname);
}

T_VOID AkFuncLeave(T_pCSTR funcname)
{
	Fwl_Print(C1, M_ENGINE, "Enter function: %s",funcname);
}

T_VOID AkFuncLeaveS(T_pCSTR funcname, T_pCSTR retval)
{
	Fwl_Print(C1, M_ENGINE, "Leave function: %s  and return %s",funcname,retval);
}

T_VOID AkFuncLeaveD(T_pCSTR funcname, T_S32 retval)
{
	Fwl_Print(C1, M_ENGINE, "Leave function: %s  and return %s",funcname,retval);
}
#endif	/* not define DEBUG_TRACE_FUNCTIONE */

/********************************************************************
*																	*
*							DEBUG trace serial						*
*																	*
********************************************************************/
#ifdef DEBUG_TRACE_SERIAL
T_VOID AkSerialReceive(T_S8 serialID, T_pCSTR data, T_U16 size)
{
	Fwl_Print(C1, M_ENGINE, "Read data from CHANNEL %d(%d): %s",serialID,size,data);
}

T_VOID AkSerialSend(T_S8 serialID, T_pCSTR data, T_U16 size)
{
	Fwl_Print(C1, M_ENGINE, "Write data to  CHANNEL %d(%d): %s\r\n",serialID,size,data);
}
#endif

/********************************************************************
*																	*
*							DEBUG output function					*
*																	*
********************************************************************/
#ifdef PLATFORM_DEBUG_VER
int printf(const char *s, ...)
{
	T_S32		len = 0;
	va_list 	args;

	va_start(args, s);
#ifdef OS_ANYKA
	len = vsnprintf((char*)gb_printf_buf, DBG_TRACE_LOGLEN,(const char*)s, args);
#else
	len = _vsnprintf((char*)gb_printf_buf, DBG_TRACE_LOGLEN,(const char*)s, args);
#endif
	va_end(args);	
	 
#ifdef DEBUG_OUTPUT

#ifdef OS_ANYKA
	puts(gb_printf_buf);
	//putch('\n');
#endif

#ifdef OS_WIN32
//	   Winvme_SetCurrentPrintType(ePrintf);  // add by ygm 20060910
//	   if (Winvme_GetPrintFwlStatus())
//	   {
	EB_Printf((char *)gb_printf_buf); // add by ygm ,judge to print or not,20060910
//	   }
//	   Winvme_WriteTraceInfoFile((const char*)gb_printf_buf);  
#endif

#endif
	return 0;
}


#endif

/*******************************************************************************
 * @brief Output the trace info to platform trace port
 * @author 
 * @date 2008-01-18
 * @param T_U8 level: message level
 * @param T_pCSTR mStr: module name
 * @param T_pCSTR s: format string, same format with ANSI C format string
 * @return >=0 if trace success, <0 failed
 * @remark Max trace len is 2048(DBG_TRACE_LOGLEN)
   | - | - - - - - - - - | : | - - - - - - - -
     |           |                    |
  level(1)  module name(<8)          message(...)
 *******************************************************************************/
T_S32 Fwl_Print(T_U8 level, T_pCSTR mStr, T_pCSTR s, ...)
{
    va_list     args;
    T_S32       len;

    if (level > g_dbgLevel)
    {
         return 0;
    }
    
    if (AK_NULL != mStr)
    {  
        len = (T_S32)strlen(mStr);
        
        strcpy(&gb_printf_buf[0], mStr);
        if (len < 4)
        {
            strcpy(&gb_printf_buf[len], "\t\t:");
            len += 3;
        }
        else if (len < 8)
        {
            strcpy(&gb_printf_buf[len], "\t:");
            len += 2;
        }
        else
        {
            gb_printf_buf[MODULE_NAME_MAX_LEN] = ':';
            len = 9; /*8 + 1*/
        }        
    }
    else
    {
        strcpy(&gb_printf_buf[0], "\t\t:");
        len = 3;
    }

    va_start(args, s);
#ifdef OS_ANYKA
    len += vsnprintf((char*)&gb_printf_buf[len], (DBG_TRACE_LOGLEN - 9), (const char*)s, args);
#else
    len += _vsnprintf((char*)&gb_printf_buf[len], (DBG_TRACE_LOGLEN - 9), (const char*)s, args);
#endif
    va_end(args); 
    
    if (gb_printf_buf[strlen(gb_printf_buf)-1] != '\n')
        strcat(gb_printf_buf, "\r\n");
    
#ifdef DEBUG_OUTPUT
    
#ifdef OS_ANYKA
	puts(gb_printf_buf);
#endif
    
#ifdef OS_WIN32
    EB_Printf((char *)&gb_printf_buf[0]); // add by ygm ,judge to print or not,20060910
#endif
    
#endif
    return len;
}


T_S32 Fwl_VPrint(T_U8 level, T_pCSTR mStr, T_pCSTR s, va_list args)
{
    T_S32 len;

    if (level > g_dbgLevel)
    {
         return 0;
    }
        
    if (AK_NULL != mStr)
    {  
        len = (T_S32)strlen(mStr);
        
        strcpy(&gb_printf_buf[0], mStr);
        if (len < 4)
        {
            strcpy(&gb_printf_buf[len], "\t\t:");
            len += 3;
        }
        else if (len < 8)
        {
            strcpy(&gb_printf_buf[len], "\t:");
            len += 2;
        }
        else
        {
            gb_printf_buf[MODULE_NAME_MAX_LEN] = ':';
            len = 9; /*8 + 1*/
        }        
    }
    else
    {
        strcpy(&gb_printf_buf[0], "\t\t:");
        len = 3;
    }
#ifdef OS_ANYKA
    len += vsnprintf((char*)&gb_printf_buf[len], (DBG_TRACE_LOGLEN - 9), (const char*)s, args);
#else
    len += _vsnprintf((char*)&gb_printf_buf[len], (DBG_TRACE_LOGLEN - 9), (const char*)s, args); 
#endif
    strcat(gb_printf_buf, "\r\n");
    
#ifdef DEBUG_OUTPUT
    
#ifdef OS_ANYKA
    puts(gb_printf_buf);
#endif
    
#ifdef OS_WIN32
//     Winvme_SetCurrentPrintType(ePrintf);  // add by ygm 20060910
//     if (Winvme_GetPrintFwlStatus())
//     {
    EB_Printf((char *)&gb_printf_buf[0]); // add by ygm ,judge to print or not,20060910
//     }
    
//     Winvme_WriteTraceInfoFile((const char*)&gb_printf_buf[1]);  
#endif
    
#endif
    return len;
}


#ifdef OS_WIN32
/*******************************************************************************
 * @brief Output the trace info to platform trace port
 * @author Ljh
 * @date 2005-05-09
 * @param T_U32 *mID: Module ID, identify the module call this function
 * @param T_pCSTR format: format string, same format with ANSI C format string
 * @return >=0 if trace success, <0 failed
 * @remark Max trace len is 2048(DBG_TRACE_LOGLEN)
 *******************************************************************************/
T_S32 Dbg_Trace(T_U32 mID, T_pCSTR format, ...)
{

	static T_S8	log[DBG_TRACE_LOGLEN+1];
	T_S32		len = 0;
	va_list		args;

	va_start(args, format);
	len = vsprintf(log, format, args);
	va_end(args);
	AK_ASSERT_VAL(len<DBG_TRACE_LOGLEN, "Dbg_Trace vsprintf len exceed buffer len, warning!!!!!!!!!!!!!!!!!!!!!!", -1);


	Fwl_Print(C3, M_ENGINE, log);
	return len;

}
#endif

T_VOID AkDebugOutput(const T_U8 *s, ...)
{
    T_S32       len = 0;
    va_list     args;

    va_start(args, s);
#ifdef OS_ANYKA
    len = vsnprintf((char*)gb_printf_buf, DBG_TRACE_LOGLEN,(const char*)s, args);
#else
    len = _vsnprintf((char*)gb_printf_buf, DBG_TRACE_LOGLEN,(const char*)s, args);
#endif
    va_end(args);   
     
#ifdef DEBUG_OUTPUT

#ifdef OS_ANYKA
    puts(gb_printf_buf);
    //putch('\n');
#endif

#ifdef OS_WIN32
//     Winvme_SetCurrentPrintType(ePrintf);  // add by ygm 20060910
//     if (Winvme_GetPrintFwlStatus())
//     {
    EB_Printf((char *)gb_printf_buf); // add by ygm ,judge to print or not,20060910
//     }
//     Winvme_WriteTraceInfoFile((const char*)gb_printf_buf);  
#endif

#endif
    return;
}

#ifndef OPEN_CONSOLE_PRINTER

static T_hSemaphore gprinter_hSemaphore;
static T_S16 gprinter_nLevel;

int		InitConsolePrinter(int LLD, int bStreamFile)
{
	gprinter_nLevel = MIN_CONSOLE_PRINT_LEVEL;
	gprinter_hSemaphore = AK_Create_Semaphore(1, AK_PRIORITY);
	return 0;
}

void	ExitConsolePrinter(int LLD)
{
	AK_Delete_Semaphore(gprinter_hSemaphore);
}

T_VOID	SetConsolePrinterLevel(int level)
{

	if(gprinter_nLevel==(T_S16)level)
		return;

	AK_Obtain_Semaphore(gprinter_hSemaphore, AK_SUSPEND);
	gprinter_nLevel = (T_S16)level;
	AK_Release_Semaphore(gprinter_hSemaphore);

	ConsolePrint(0, "SetConsolePrinterLevel=%d\n", gprinter_nLevel);
}

int		ConsolePrint(int LLD, const char *format, ...)
{
    T_S32       len = 0;
    va_list     args;
	char		msg[312];
	
	if((T_S16)LLD >= gprinter_nLevel)//MIN_CONSOLE_PRINT_LEVEL==9
		return 0;//priority print

    va_start(args, format);
	#ifdef OS_ANYKA
    len = vsnprintf((char*)msg, 312,(const char*)format, args);
	#else
    len = _vsnprintf((char*)msg, 312,(const char*)format, args);
	#endif
    va_end(args);   
     
	#ifdef OS_ANYKA
	AK_Obtain_Semaphore(gprinter_hSemaphore, AK_SUSPEND);
    puts(msg);
	AK_Release_Semaphore(gprinter_hSemaphore);
	#endif


	#ifdef OS_WIN32
    EB_Printf((char *)msg); // add by ygm ,judge to print or not,20060910
	#endif

	return 0;
}

#endif




//add end


