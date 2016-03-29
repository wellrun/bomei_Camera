/**
 * @file print.c
 * @brief print function file, provide functions to print infomation
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 */
#include <stdlib.h>
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "arch_uart.h"
#include "stdarg.h"
#include "drv_api.h"

#define KEY_BACKSPACE           0x08
#define KEY_ENTER               0x0d
#define KEY_ESC                 0x1b

#define CR                      0x0D
#define LF                      0x0A

#define DBG_TRACE_LOGLEN        2048
#define MODULE_NAME_MAX_LEN     8

/* default to enable print */
static T_CONSOLE_TYPE console_type=CONSOLE_UART;
static T_fPRINT_CALLBACK printf_callback = AK_NULL;
static T_U8 m_console_level=C3;
static T_UART_ID m_uart_id = uiUART0;

int puts(const char *s);

static T_CHR printf_buf[DBG_TRACE_LOGLEN];

/* return drvlib version */
T_pCSTR drvlib_version(T_VOID)
{
    return _DRV_LIB_VER;
}

/**
 * @brief  debug set callback
 * @author	
 * @date
 * @param[in] func callback function of console, can be a null one
 * @return  T_VOID
 */
T_VOID console_setcallback(T_fPRINT_CALLBACK func)
{
    printf_callback = func;
}

/**
 * @brief set forbidden level
 * @author
 * @date
 * @param[in] level new forbidden level
 * @return T_VOID
 */
T_VOID console_setlevel(T_U8 level)
{
    //set new level
    m_console_level = level;
}

extern int vsnprintf( char *buffer, size_t count, const char *format, va_list argptr );

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
T_S32 akprintf(T_U8 level, T_pCSTR mStr, T_pCSTR s, ...)
{
    va_list     args;

    if (m_console_level < level)
        return -1;
        
    va_start(args, s);
    vsnprintf(printf_buf, DBG_TRACE_LOGLEN, (const char*)s, args);   
    puts(printf_buf);
    va_end(args); 

    return 0;
}

/**
 * @brief debug console init
 * @author	
 * @date
 * @param[in] type type of console, refer to T_CONSOLE_TYPE definition
 * @param[in] uart baudrate, refer to Arch_uart.h definition
 * @param[in] T_U8,uart id:uiUART0--uiUART3
 * @return  T_VOID
 */
T_VOID console_init(T_UART_ID uart_id, T_CONSOLE_TYPE type, T_U32 baudrate)
{
    printf_callback = AK_NULL;
    
    if (type == CONSOLE_UART)
    {
        if (MAX_UART_NUM > uart_id)
        {
            m_uart_id = uart_id;
        }

        uart_init(uart_id, baudrate, get_asic_freq());
        uart_set_callback(uart_id, AK_NULL);
    }
#ifdef USBDEBUG_PRINT
    else if (type == CONSOLE_USB)
    {
        usbdebug_enable();
    }
#endif
    else
    {
        
    }
    
    console_type = type;
    
    //show driver lib version
    akprintf(C3, M_DRVSYS, "\n_DRV_LIB_VER=%s\n", _DRV_LIB_VER);    
}

/**
 * @brief release current console
 * @author	
 * @date
 * @return  T_VOID
 */
T_VOID console_free(T_VOID)
{
    if (console_type == CONSOLE_UART)
    {
        uart_free(m_uart_id);
    }
#ifdef USBDEBUG_PRINT
    else if (console_type == CONSOLE_USB)
    {
        usbdebug_disable();
    }
#endif
    else
    {
        
    }
    
    console_type = CONSOLE_NULL;

}

/**
 * @brief get a charactor from uart
 * @author	
 * @date
 * @return  T_S8
 * @retval  the charactor that was gotten
 */
T_S8 getch(T_VOID)
{
    T_S8 c=0;

    if (console_type == CONSOLE_UART)
        uart_read_chr( m_uart_id, (T_U8 *)&c );
#ifdef USBDEBUG_PRINT
    else if (console_type == CONSOLE_USB)
    {
        
        while(usbdebug_getstring((T_U8 *)&c, 1) == 0);
        return c;
    }
#endif
    else
    {
        puts("not support input, dead!!\n");\
        while(1);
        return 0;
    }
    
    return c;
}

T_S32 putch( T_S8 ch)
{
    if (console_type == CONSOLE_UART)
        return uart_write_chr( m_uart_id, ch );
#ifdef USBDEBUG_PRINT
    else if (console_type == CONSOLE_USB)
    {
        //uart_write_chr( CONSOLE_PORT, ch );
        usbdebug_printf((T_U8 *)&ch, 1);
        return 0;
    }
#endif
    else
    {
        return 0;
    }
}

int puts(const char *s)
{
    T_U32   written_num = 0;
    
    if (console_type == CONSOLE_UART)
        return uart_write_str( m_uart_id, ( T_U8* )s );
#ifdef USBDEBUG_PRINT
    else if (console_type == CONSOLE_USB)   
    {
        usbdebug_printf(( T_U8* )s, strlen(s));       
        return written_num;
    }
#endif
    else
    {
        return 0;
    }
}

T_VOID gets (T_S8 * buf, T_S32 n)
{
    int i;

    for(i = 0;i< n; i ++)
        {
            buf[i] = (char )getch();
            if (buf[i] == CR){
                //putch(LF);
                putch(CR);
                break;
            }
            else
            {
                 if(buf[i] == KEY_BACKSPACE)
                 {
                    if (i >= 1)
                        buf[i-1] = 0;
            else
            {
                buf[i]= 0;
                i --;
                        continue;
                    }

                    putch(KEY_BACKSPACE);
                    putch(' ');
                    putch(KEY_BACKSPACE);
                    i -= 2;
                    continue;
                 }
                else
                    putch(buf[i]);
           }
        }

        if (i == n)
        {
            //putch(LF);
            putch(CR);
        }

        buf[i] = '\0';
        return;
}

/**
 * @brief get integer in decimal style
 * @author	
 * @date
 * @param[in] def default value
 * @return T_U32
 * @retval integer that was gotten
 */
T_U32 getul10(T_U32 def)
{
    char buf[11];
    char *p = buf;

    gets((T_S8 *)buf,11);

    if (buf[0] == '\0')
        return def;

    buf[10] = '\0';
    if ((buf[0]== '0' ) && ((buf[1]== 'x')||(buf[1] == 'X')) )
        p = &buf[2];

    return strtoul(p, 0, 10);
}

/**
 * @brief get integer in hex style
 * @author	
 * @date
 * @param[in] def default value
 * @return T_U32
 * @retval integer that was gotten
 */
T_U32 getul(T_U32 def)
{
    char buf[11];
    char *p = buf;

    gets((T_S8 *)buf,11);

    if (buf[0] == '\0')
        return def;

    buf[10] = '\0';
    if ((buf[0]== '0' ) && ((buf[1]== 'x')||(buf[1] == 'X')) ) // hex
    {
        p = &buf[2];
        return strtoul(p, 0, 16);
    }
    else    //decimal
    {
        return strtoul(p, 0, 10);
    }
}
T_VOID print_x(T_U32 hex)
{
	T_S32 i;
	T_S8 chHex ;

	for(i = 32 ;i !=0 ; i -=4)
	{
	    chHex = (hex >> (i-4)) & 0xf;
        if(chHex < 10)
        {
            chHex += '0'; 
        }
        else
        {
            chHex += 'a' -10;
        }
	    putch(chHex);
    }
}

T_VOID akerror(T_S8 *aStr, T_U32 nHex, T_BOOL bNewline)
{
    puts((const char*)aStr);
    if (nHex)
    {
        print_x(nHex);
    }
    if (bNewline)
    {
        putch('\n');
    }
}


