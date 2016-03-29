/**
 * @file eng_debug.h
 * @brief This header file is for debug & trace function prototype
 * @author ANYKA
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @version 1.0
 */

#ifndef __ENG_DEBUG_H__
/**
 * @def __ENG_DEBUG_H__
 *
 */
#define __ENG_DEBUG_H__

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "anyka_types.h"
//#include "gbl_macrodef.h"

/**
 * @def AK_EMPTY
 *
 */
#define AK_EMPTY

#ifdef PLATFORM_DEBUG_VER
#define AK_Feed_Watchdog(x)	AK_Feed_WatchdogEx(x, (T_pSTR)(__FILE__),(T_U32)(__LINE__))
#else
#define AK_Feed_Watchdog(x)	
#endif

/*==============================================================================*/
/*------------------>   Module name defination begin   <------------------*/

#define M_AKFRAME         "AKFRAME"
#define M_ALARM           "ALARM"
#define M_CAMERA          "CAMERA"
#define M_CALENDAR        "CALENDAR"
#define M_CLOCK           "CLOCK"
#define M_COMMON          "COMMON"
#define M_CTRL         	  "CTRL"
#define M_DISPLAY         "DISPLAY"
#define M_ENGINE          "ENGINE"
#define M_EXAMPLE         "EXAMPLE"
#define M_EBOOK           "EBOOK"
#define M_EMAP            "EMAP"
#define M_EXPLORER        "EXPLORER"
#define M_FM              "FM"
#define M_FONT			  "FONT"
#define M_FONT_LIB        "FONT_LIB"
#define M_FS              "FS"
#define M_FWL             "FWL"
#define M_GAME            "GAME"
#define M_GBA             "GBA"
#define M_GE_LIB		  "GE_LIB"
#define M_GE			  "GE"
#define M_IMG_LIB		  "IMG_LIB"
#define M_IMAGE           "IMAGE"
#define M_2D			  "2D"
#define M_INIT            "INIT"
#define M_KEY_PAD         "KEYPAD"
#define M_MEDIA			  "Media"
#define M_MLIST			  "MList"
#define M_MPLAY			  "MPlayer"
#define M_DMX			  "Demux"
#define M_ADEC			  "AudioDEC"
#define M_VDEC			  "VideoDEC"
#define M_WAVE			  "WavOUT"
#define M_AUDIO			  "AUDIO"
#define M_AB			  "ABPLAY"
#define M_VIDEO           "VIDEO"
#define M_MENCODE         "MENCODE"
#define M_MDETECT         "MDETECT"
#define M_MENU            "MENU"
#define M_MUX             "MUX"
#define M_NES             "NES"
#define M_OFFICE          "OFFICE"
#define M_POWER           "POWER"
#define M_PREPROC         "PREPROC"
#define M_PUBLIC          "PUBLIC"
#define M_RAM_MLC         "RAM_MLC"
#define M_SD           	  "SD"
#define M_SETTING         "SETTING"
#define M_SNES            "SNES"
#define M_STANDBY         "STANDBY"
#define M_THUMB           "THUMB"
#define M_TOOL            "TOOL"
#define M_UPSYS           "UPSYS"
#define M_USB             "USB"
#define M_REC_A		  	  "REC_A"
#define M_RTC		  	  "RTC"
#define M_FLASH		  	  "FLASH"

#define M_VZOOM           "VZOOM"
#define M_VFONT           "VFONT"
#define M_NETWORK         "NETWORK"


/*------------------->   Module name defination end   <------------------*/
/*==============================================================================*/


/*==============================================================================*/
/*------------------->   Fwl_Print Level defination begin   <------------------*/
#define C1 1    /*Fatal error message*/
#define C2 2    /*Error message*/
#define C3 3    /*Common message*/
#define C4 4    /*temp message*/
/*-------------------->   Fwl_Print Level defination end   <-------------------*/
/*==============================================================================*/

#define PRINT_TIMES_SEC		2	// How Many Seconds Print Debug Info

/*==============================================================================*/
T_S32 Fwl_SetDbgValveLevel(T_U8 level);
T_U8  Fwl_GetDbgValveLevel(T_VOID);
/*==============================================================================*/

/** @defgroup DBGI Debug interface 
 * @ingroup ENG
 */
/*@{*/

// data point check, not only for NULL, but also for thoes points that are
// out of range. We will define different data point ranges for different platforms
T_BOOL AkAssertCheckPointer(T_pCVOID ptr);

// deal with failure condition
T_VOID AkAssertDispMsg(T_pCSTR message, T_pCSTR filename, T_U32 line);

#define AK_ASSERT_VAL(_bool_, _msg_, _retval_)    if (!(_bool_)) { AkAssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return (_retval_); }
#define AK_ASSERT_VAL_VOID(_bool_, _msg_)        if (!(_bool_)) { AkAssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return; }
#define AK_ASSERT_PTR(_ptr_, _msg_, _retval_)    if (!AkAssertCheckPointer(_ptr_)) { AkAssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return (_retval_); }
#define AK_ASSERT_PTR_VOID(_ptr_, _msg_)        if (!AkAssertCheckPointer(_ptr_)) { AkAssertDispMsg(_msg_, __FILE__, (T_U32)__LINE__); return; }


#define AK_DEBUG_OUTPUT     AkDebugOutput
/********************************************************************
*                                                                    *
*                            DEBUG trace function                    *
*                                                                    *
********************************************************************/

#ifdef DEBUG_TRACE_FUNCTIONE
/**
*@brief reference debug_trace_function:AkFuncEnter(T_pCTSTR funcname)
*whether the function is enter and print the function name 
*@author 
*@date
*@param[in] funcname            function's name 
*@return T_VOID
*/
    T_VOID AkFuncEnter(T_pCTSTR funcname);
/**
*@brief reference debug_trace_function:AkFuncLeave(T_pCTSTR funcname)
*whether the function is leave and print the function name 
*@author 
*@date
*@param[in] funcname            function's name 
*@return T_VOID
*/
    T_VOID AkFuncLeave(T_pCTSTR funcname);
/**
*@brief reference debug_trace_function:AkFuncLeaveS(T_pCTSTR funcname)    
*whether the function is leave and print the function name and retval
*@author 
*@date
*@param[in] funcname            function's name 
*@return T_VOID
*/    
    T_VOID AkFuncLeaveS(T_pCTSTR funcname, T_pCSTR retval);
/**
*@brief reference debug_trace_function:AkFuncLeaveS(T_pCTSTR funcname)
*whether the function is leave and print the function name and retval
*@author 
*@date
*@param[in] funcname            function's name 
*@return T_VOID
*/
    T_VOID AkFuncLeaveD(T_pCTSTR funcname, T_S32 retval);

    #define AK_FUNCTION_ENTER(_funcname_)                AkFuncEnter(_funcname_)
    #define AK_FUNCTION_LEAVE(_funcname_)                AkFuncLeave(_funcname_)
    #define AK_FUNCTION_RET_STR(_funcname_, _retval_)    AkFuncLeaveS(_funcname_, _retval_)
    #define AK_FUNCTION_RET_INT(_funcname_, _retval_)    AkFuncLeaveD(_funcname_, _retval_)
#else    // not define DEBUG_TRACE_FUNCTIONE
    #define AK_FUNCTION_ENTER(_funcname_)                AK_EMPTY
    #define AK_FUNCTION_LEAVE(_funcname_)                AK_EMPTY
    #define AK_FUNCTION_RET_STR(_funcname_, _retval_)    AK_EMPTY
    #define AK_FUNCTION_RET_INT(_funcname_, _retval_)    AK_EMPTY
#endif    // end of define DEBUG_TRACE_FUNCTIONE

/********************************************************************
*                                                                    *
*                            DEBUG trace serial                        *
*                                                                    *
********************************************************************/
#ifdef DEBUG_TRACE_SERIAL
/**
*@brief definite debug_trace_function:AkSerialReceive(T_S8 serialID, T_pCSTR data, T_U16 size)
*show data which is read from ,and show data's size and data also 
*@author 
*@date
*@param[in] serialID        which data is read from     
*@param[in] data            data
*@param[in] size            data'size
*@return T_VOID
*/
    T_VOID AkSerialReceive(T_S8 serialID, T_pCSTR data, T_U16 size);
/**
*@brief definite debug_trace_function:AkSerialSend(T_S8 serialID, T_pCSTR data, T_U16 size)
*show data which is writen to,and show data's size and data also 
*@author 
*@date
*@param[in] serialID        which data is writen to     
*@param[in] data            data
*@param[in] size            data'size
*@return T_VOID
*/
    T_VOID AkSerialSend(T_S8 serialID, T_pCSTR data, T_U16 size);

    #define AK_SERIAL_RECEIVE(_sid_, _msg_, _size_)        AkSerialReceive(_sid_, _msg_, _size_)
    #define AK_SERIAL_SEND(_sid_, _msg_, _size_)        AkSerialSend(_sid_, _msg_, _size_)
#else    // not define DEBUG_TRACE_SERIAL
    #define AK_SERIAL_RECEIVE(_sid_, _msg_, _size_)        AK_EMPTY
    #define AK_SERIAL_SEND(_sid_, _msg_, _size_)        AK_EMPTY
#endif    // end of define DEBUG_TRACE_SERIAL

/********************************************************************
*                                                                    *
*                            DEBUG output function                    *
*                                                                    *
********************************************************************/

/**
 * @brief Output the trace info to platform trace port
 * @author Ljh
 * @date 2005-05-09
 * @param[in] s 
 * @param[in] ...
 * @return >=0 if Printf success, <0 failed
**/
T_S32 Fwl_Print(T_U8 level, T_pCSTR mStr, T_pCSTR s, ...);
T_S32 Fwl_VPrint(T_U8 level, T_pCSTR mStr, T_pCSTR s, va_list args);

T_VOID AkDebugOutput(const T_U8 *s, ...);


//========== stronger console debug window 
/* *******************************************************************

                优点：By ZMJ
                
                	1。模拟器上支持断点调试打印
                	2。模拟器上主程序死亡，打印窗口依然活跃
                	3。模拟器上支持彩色打印
                	4。模拟器上支持多窗口打印
                	5。模拟器上点击窗口即暂停

                	6。开发板上支持多线程打印，无乱码
                	7。开发板上支持级别打印
                	
                	
******************************************************************* */


#define MIN_CONSOLE_PRINT_LEVEL		9  //该级别的打印不显示
#define MAX_CONSOLE_PRINT_LEVEL		0  //该级别的打印一直显示

#ifdef OS_WIN32
	#ifndef OPEN_CONSOLE_PRINTER
		#define OPEN_CONSOLE_PRINTER
		#include "zmjDump.h"
	#else
	int		ConsolePrint(int LLD, const char *format, ...);
	int		InitConsolePrinter(int LLD, int bStreamFile);
	void	ExitConsolePrinter(int LLD);
	#endif
#endif

#ifdef OS_ANYKA
	int		ConsolePrint(int LLD, const char *format, ...);
	int		InitConsolePrinter(int LLD, int bStreamFile);
	void	ExitConsolePrinter(int LLD);
#endif

T_VOID	SetConsolePrinterLevel(int level);

		


/*@}*/


#endif

