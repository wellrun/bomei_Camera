/**
 * @filename usb_anyka.c
 * @brief: AK3223M how to use usb device of anyka.
 *
 * This file describe frameworks of anyka device.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */
#include <stdio.h>
#ifdef OS_ANYKA
#include    "anyka_cpu.h"
#include    "anyka_types.h"
#include    "usb_slave_drv.h"
#include    "hal_usb_s_debug.h"
#include    "hal_usb_s_std.h"
#include    "usb_common.h"
#include    "drv_api.h"

#include "interrupt.h"

//********************************************************************


#define MAXSIZE_CONSOLEBUFFER   2048

static T_U8 console_buffer[MAXSIZE_CONSOLEBUFFER];
static T_U32 buffer_head = 0, buffer_tail = 0;

static void Fwl_usb_debug_receive(T_VOID);

//********************************************************************
/**
* @brief enable usb debug
* @author liao_zhijun
* @date 2010-06-17
*
* @return T_BOOL
*/
T_BOOL usbdebug_enable(T_VOID)
{
    usbcdc_init();
    usbcdc_set_callback(Fwl_usb_debug_receive, AK_NULL);
    usbcdc_set_datapool(console_buffer, MAXSIZE_CONSOLEBUFFER);
    usbcdc_enable();

    buffer_head = 0;
    buffer_tail = 0;

    return AK_TRUE;
    
}

//********************************************************************
/**
* @brief disable usb debug
* @author liao_zhijun
* @date 2010-06-17
*
* @return T_BOOL
*/
T_VOID usbdebug_disable(T_VOID)
{
    usbcdc_disable();

    return;
}

static void Fwl_usb_debug_receive(T_VOID)
{
    T_S32 ret;
    
    do
    {
        ret = usbcdc_read((T_U8 *)console_buffer + buffer_tail, 1); 
        if(ret > 0)
        {
            buffer_tail++;
            buffer_tail &= (MAXSIZE_CONSOLEBUFFER - 1);
        }
        
    }while(ret > 0);
}

T_U32 usbdebug_getstring(T_U8 *str, T_U32 len)
{
    T_U32 chr_count = 0;
    
    while(buffer_head != buffer_tail)
    {
        str[chr_count++] =  console_buffer[buffer_head++];
        buffer_head &= (MAXSIZE_CONSOLEBUFFER - 1);

        if(chr_count >= len)
            break;
    }

    return chr_count;
}

/**
* @brief enable usb debug
* @author liao_zhijun
* @date 2010-06-17
*
* @param str string to print
* @param len string length
* @return T_BOOL
*/
void usbdebug_printf(T_U8 *str, T_U32 len)
{
    if(str == AK_NULL || len == 0)
        return;

    usbcdc_write(str, len);

    return;
}



//********************************************************************
#endif
