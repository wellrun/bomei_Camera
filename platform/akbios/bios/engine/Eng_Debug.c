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
#include "eng_debug.h"
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

#ifdef OS_WIN32
	#define MIN_RAM_ADDR	0x00001000  // The first address of RAM
	#define MAX_RAM_ADDR	0x2fffffff  // the end of RAM
	#define MIN_ROM_ADDR	0x00001000  // The first address of ROM
	#define MAN_ROM_ADDR	0x2fffffff  // the end of ROM
#else

    #define MIN_RAM_ADDR	0x80000000  // The first address of RAM

#ifdef SDRAM_MODE
#define MAX_RAM_ADDR (MIN_RAM_ADDR+(SDRAM_MODE<<20))
#else
#error "No define SDRAM_MODE"
#endif
    #define MIN_ROM_ADDR    0x10000000  // The first address of ROM
    #define MAN_ROM_ADDR    0x12000000  // the end of ROM 8M

#endif

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
	if ((((T_U32)ptr >= MIN_RAM_ADDR) && ((T_U32)ptr <= MAX_RAM_ADDR)) ||
		(((T_U32)ptr >= MIN_ROM_ADDR) && ((T_U32)ptr <= MAN_ROM_ADDR)))
		return AK_TRUE;
	else
	{
		Fwl_Print(C1, M_ENGINE, "AkAssertCheckPointer failed, ptr is:%x", (T_U32)ptr);
		return AK_FALSE;
	}
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
	T_U16		len = 0;

	Fwl_Print(C1, M_ENGINE, "AkAssertDispMsg %s, %s, %d\n", message, filename, line);

#ifdef ASSERT_DEATH_LOOP
	while(1);
#endif

#if defined(ASSERT_REBOOT) && defined(SUPPORT_PANNIC_REBOOT)
	System_Start(ASSERT_TYPE);
#endif
	return;		/* assert occur */

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
#if 0
int printf(const char *s, ...)
{
	T_S32		len = 0;
	va_list 	args;

	va_start(args, s);

	len = vsnprintf((char*)gb_printf_buf, DBG_TRACE_LOGLEN,(const char*)s, args);
	puts(gb_printf_buf);

	va_end(args);	
	 
	return 0;
}
#endif

T_S32 Fwl_Printf(T_pCSTR s, ...)
{
	T_S32		len = 0;
	va_list		args;

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
#endif

#ifdef OS_WIN32
// 	Winvme_SetCurrentPrintType(ePrintf);  // add by ygm 20060910
// 	if (Winvme_GetPrintFwlStatus())
// 	{
	EB_Printf((char *)gb_printf_buf); // add by ygm ,judge to print or not,20060910
// /	}
 
// 	Winvme_WriteTraceInfoFile((const char*)gb_printf_buf);	
#endif

#endif
	return len;
}

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
    
    gb_printf_buf[0] = (T_CHR)level;

    if (AK_NULL != mStr)
    {  
        len = (T_S32)strlen(mStr);
        
        strcpy(&gb_printf_buf[1], mStr);
        if (len < 6)
        {
            strcpy(&gb_printf_buf[len+1], "\t\t:");
            len += 4;  /*1 + 3*/
        }
        else if (len < 12)
        {
            strcpy(&gb_printf_buf[len+1], "\t:");
            len += 3;  /*1 + 2*/
        }
        else
        {
            gb_printf_buf[MODULE_NAME_MAX_LEN+1] = ':';
            len = 14; /*1 + 12 + 1*/
        }        
    }
    else
    {
        strcpy(&gb_printf_buf[1], "\t\t:");
        len = 4;    /*1 + 3*/
    }

    va_start(args, s);
#ifdef OS_ANYKA
    len += vsnprintf((char*)&gb_printf_buf[len], (DBG_TRACE_LOGLEN - 14), (const char*)s, args);
#else
    len += _vsnprintf((char*)&gb_printf_buf[len], (DBG_TRACE_LOGLEN - 14), (const char*)s, args);
#endif
    va_end(args); 
    strcat(gb_printf_buf, "\r\n");
    
#ifdef DEBUG_OUTPUT
    
#ifdef OS_ANYKA
	puts(gb_printf_buf);
#ifdef SUPPORT_NAND_TRACE
	nandPrintf(gb_printf_buf);
#endif	// end of #ifndef SUPPORT_NAND_TRACE

#endif
    
#ifdef OS_WIN32
//     Winvme_SetCurrentPrintType(ePrintf);  // add by ygm 20060910
//     if (Winvme_GetPrintFwlStatus())
//     {
    EB_Printf((char *)&gb_printf_buf[1]); // add by ygm ,judge to print or not,20060910
//     }
    
//     Winvme_WriteTraceInfoFile((const char*)&gb_printf_buf[1]);  
#endif
    
#endif
    return len;
}


/**
 * @brief Output the trace info to platform trace port, for interrupt used.
 * @author LiChenjie
 * @date 2008-01-31
 * @param[in] s 
 * @param[in] ...
 * @return >=0 if Printf success, <0 failed
**/
#define DBG_INTR_TRACE_INTR_LOGLEN    	128
static T_CHR gb_intr_printf_buf[DBG_INTR_TRACE_INTR_LOGLEN];
T_S32 Fwl_Print_Intr(T_U8 level, T_pCSTR mStr, T_pCSTR s, ...)
{
    va_list     args;
    T_S32       len;
    
    gb_intr_printf_buf[0] = (T_CHR)level;

    if (AK_NULL != mStr)
    {  
        len = (T_S32)strlen(mStr);
        
        strcpy(&gb_intr_printf_buf[1], mStr);
        if (len < 3)
        {
            strcpy(&gb_intr_printf_buf[len+1], "\t\t:");
            len += 4;  /*1 + 3*/
        }
        else if (len < 8)
        {
            strcpy(&gb_intr_printf_buf[len+1], "\t:");
            len += 3;  /*1 + 2*/
        }
        else
        {
            gb_intr_printf_buf[MODULE_NAME_MAX_LEN+1] = ':';
            len = 10; /*1 + 8 + 1*/
        }        
    }
    else
    {
        strcpy(&gb_intr_printf_buf[1], "\t\t:");
        len = 4;    /*1 + 3*/
    }

    va_start(args, s);
#ifdef OS_ANYKA
    len += vsnprintf((char*)&gb_intr_printf_buf[len], 
                     (DBG_INTR_TRACE_INTR_LOGLEN - 10), (const char*)s, args);  
#else
    len += _vsnprintf((char*)&gb_intr_printf_buf[len], 
                      (DBG_INTR_TRACE_INTR_LOGLEN - 10), (const char*)s, args);
#endif
    va_end(args); 
    strcat(gb_intr_printf_buf, "\r\n");
    
#ifdef DEBUG_OUTPUT
    
#ifdef OS_ANYKA
	puts(&gb_intr_printf_buf[1]);
#endif
    
#ifdef OS_WIN32
//     Winvme_SetCurrentPrintType(ePrintf);  // add by ygm 20060910
//     if (Winvme_GetPrintFwlStatus())
//     {
    EB_Printf((char *)&gb_intr_printf_buf[1]); // add by ygm ,judge to print or not,20060910
//     }
    
//     Winvme_WriteTraceInfoFile((const char*)&gb_intr_printf_buf[1]);  
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
        
    gb_printf_buf[0] = (T_CHR)level;

    if (AK_NULL != mStr)
    {  
        len = (T_S32)strlen(mStr);
        
        strcpy(&gb_printf_buf[1], mStr);
        if (len < 6)
        {
            strcpy(&gb_printf_buf[len+1], "\t\t:");
            len += 4;  /*1 + 3*/
        }
        else if (len < 12)
        {
            strcpy(&gb_printf_buf[len+1], "\t:");
            len += 3;  /*1 + 2*/
        }
        else
        {
            gb_printf_buf[MODULE_NAME_MAX_LEN+1] = ':';
            len = 14; /*1 + 12 + 1*/
        }        
    }
    else
    {
        strcpy(&gb_printf_buf[1], "\t\t:");
        len = 4;    /*1 + 3*/
    }
#ifdef OS_ANYKA
    len += vsnprintf((char*)&gb_printf_buf[len], (DBG_TRACE_LOGLEN - 14), (const char*)s, args);
#else
    len += _vsnprintf((char*)&gb_printf_buf[len], (DBG_TRACE_LOGLEN - 14), (const char*)s, args); 
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
    EB_Printf((char *)&gb_printf_buf[1]); // add by ygm ,judge to print or not,20060910
//     }
    
//     Winvme_WriteTraceInfoFile((const char*)&gb_printf_buf[1]);  
#endif
    
#endif
    return len;
}



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
    putch('\n');
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


//add end


