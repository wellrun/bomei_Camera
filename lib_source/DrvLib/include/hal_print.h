/**
 * @file  hal_print.h
 * @brief Define structures and functions of print.c
 * 
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2006-01-16
 * @version 1.0
 */
#ifndef _PRINT_H_
#define _PRINT_H_

#include "arch_uart.h"

/** @defgroup Debug_Print Print group
 *ingroup Drv_Lib
 */
/*@{*/

typedef enum{
    CONSOLE_UART=0,         /* print via uart */
    CONSOLE_USB,            /* print via usb */
    CONSOLE_NULL            /* disable print */
}T_CONSOLE_TYPE;

#define M_DRVSYS            "DRVLIB" /* module name */

#define C1 1    /*Fatal error message*/
#define C2 2    /*Error message*/
#define C3 3    /*Common message*/

typedef T_S32 (* T_fPRINT_CALLBACK)(T_U8 level, T_pCSTR mStr, T_pCSTR s, ...);

/**
 * @brief get drvlib version
 * @return T_pCSTR
 */
T_pCSTR drvlib_version(T_VOID);

/**
 * @brief  debug set callback
 * @author	
 * @date
 * @param[in] func callback function of console, can be a null one
 * @return  T_VOID
 */
T_VOID console_setcallback(T_fPRINT_CALLBACK func);

/**
 * @brief set forbidden level
 * @author
 * @date
 * @param[in] level new forbidden level
 * @return T_VOID
 */
T_VOID console_setlevel(T_U8 level);

/**
 * @brief debug console init
 * @author	
 * @date
 * @param[in] type type of console, refer to T_CONSOLE_TYPE definition
 * @param[in] baudrate uart baudrate, refer to Arch_uart.h definition
 * @param[in] T_UART_ID,uart id
 * @return  T_VOID
 */
T_VOID console_init(T_UART_ID uart_id, T_CONSOLE_TYPE type, T_U32 baudrate);

/**
 * @brief release current console
 * @author	
 * @date
 * @return  T_VOID
 */
T_VOID console_free(T_VOID);

/**
 * @brief get a charactor from uart
 * @author	
 * @date
 * @return  T_S8
 * @retval  the charactor that was gotten
 */
T_S8   getch(T_VOID);

/**
 * @brief get string
 * @param T_S8 *buf, the buffer for the string
 * @param T_S32 n, the length of the string
 * @retval T_VOID
 */
//T_VOID  gets(T_S8 * buf, T_S32 n);

/**
 * @brief get integer in decimal style
 * @author	
 * @date
 * @param[in] def default value
 * @return T_U32
 * @retval integer that was gotten
 */
T_U32   getul10(T_U32 def);

/**
 * @brief get integer in hex style
 * @author	
 * @date
 * @param[in] def default value
 * @return T_U32
 * @retval integer that was gotten
 */
T_U32   getul(T_U32 def);

/**
 * @brief anyka specific printf
 * @author
 * @date
 * @param[in] level forbidden level
 * @param[in] mStr module string
 * @param[in] s format string
 * @return T_S32
 * @retval 0 is print ok, -1 is forbidden to print
 */
T_S32 akprintf(T_U8 level, T_pCSTR mStr, T_pCSTR s, ...);


/*@}*/
#endif /* _PRINT_H_ */
