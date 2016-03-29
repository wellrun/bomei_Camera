/**@file hal_mouse_ps2.c
 * @brief implement mouse.
 *
 * This file implement r/w interface of mouse.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2011-01-06
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "hal_Print.h"
#include "sysctl.h"
#include "hal_ps2.h"
#include "hal_mouse_ps2.h"
#include "drv_gpio.h"
#include "drv_api.h"
#include "drv_module.h"


static volatile T_MOUSE_PS2 s_tMousePs2 = {0};

static T_BOOL mouse_ps2_reset(T_VOID);
static T_BOOL mouse_ps2_enable_data(T_VOID);
static T_BOOL mouse_ps2_enable_uart(T_UART_ID uart_id,T_U32 rate);
static T_U8 mouse_ps2_uart_cb(T_VOID);
static T_S32 mouse_ps2_mv_x(T_U8 * mouse_data);
static T_S32 mouse_ps2_mv_y(T_U8 * mouse_data);
static T_S32 mouse_ps2_mv_z(T_U8 * mouse_data);
static T_eMOUSE_PS2_BTN mouse_ps2_btn(T_U8 * mouse_data);


/**
 * @brief   mouse init 
 *
 * mouse reset , get mouse clk,enable mouse data,enable uart receive
 * @author Huang Xin
 * @date 2011-01-27
 * @param uart_id[in] uart id
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL mouse_ps2_open(T_UART_ID uart_id)
{
    T_U32 clk_pin,data_pin;

    //clear s_tMousePs2 member except fMousePs2Cb 
    memset((T_U8*)&s_tMousePs2, 0, sizeof(T_MOUSE_PS2)-4);
    if(!gpio_get_uart_pin(uart_id, &clk_pin, &data_pin))
    {
        return AK_FALSE;
    }
    akprintf(C3, M_DRVSYS, "clk pin: %d, data pin: %d\n", clk_pin, data_pin);
    ps2_init(clk_pin,data_pin);
    if (!mouse_ps2_reset())
    {
        return AK_FALSE;
    }

    if(!mouse_ps2_enable_data())
    {
        return AK_FALSE;
    }
    s_tMousePs2.ucMode = MOUSE_PS2_MODE_STREAM;
    s_tMousePs2.bInitOK = AK_TRUE;
    s_tMousePs2.ucUartId = uart_id;
    s_tMousePs2.ulRate  = 1000000/ps2_get_clk();
    akprintf(C3, M_DRVSYS, "mouse id: %d, rate: %dHz\n", s_tMousePs2.ucId,s_tMousePs2.ulRate);
   
    if(!mouse_ps2_enable_uart(uart_id,s_tMousePs2.ulRate))
    {
        return AK_FALSE;
    }
    return AK_TRUE;
}

/**
 * @brief   mouse close
 *
 * close ps2 , uart free
 * @author Huang Xin
 * @date 2011-01-27
 * @param uart_id[in] uart id
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_VOID mouse_ps2_close(T_UART_ID uart_id)
{
    ps2_close();
    uart_free(uart_id);
    s_tMousePs2.bInitOK = AK_FALSE;
    s_tMousePs2.fMousePs2Cb = AK_NULL;
}

/**
 * @brief   set callback func
 *
 * called after mouse_ps2_open()
 * @author Huang Xin
 * @date 2011-01-27
 * @param cb[in] callback func
 * @return  T_VOID
 */
T_VOID mouse_ps2_set_cb(T_fMOUSE_PS2_CALLBACK cb)
{
    if (AK_NULL != cb)
        s_tMousePs2.fMousePs2Cb = cb;
}

/**
 * @brief   mouse reset
 *
 * reset mouse and get mouse ids
 * @author Huang Xin
 * @date 2011-01-27
 * @return  T_VOID
 */
static T_BOOL mouse_ps2_reset(T_VOID)
{
    T_U8 data;
    
    if (!ps2_write(MOUSE_PS2_CMD_RESET))
    {
        akprintf(C3, M_DRVSYS, "mouse_ps2_reset():send MOUSE_PS2_CMD_RESET fail\n");
        goto EXIT;
    }
    //check if 0xFF sended successful
    if(!ps2_read(&data))
    {        
        akprintf(C3, M_DRVSYS, "mouse_ps2_reset():read MOUSE_PS2_RESP_ACK fail\n");
        goto EXIT;
    }
    if (MOUSE_PS2_RESP_ACK != data)
    {        
        akprintf(C3, M_DRVSYS, "mouse_ps2_reset():reset fail 0x%x\n",data);
        goto EXIT;
    }
    // check if self tst success
    if(!ps2_read(&data))
    {        
        akprintf(C3, M_DRVSYS, "mouse_ps2_reset():read  MOUSE_PS2_SELF_TST_PASS fail\n");
        goto EXIT;
    }
    if (MOUSE_PS2_SELF_TST_PASS!= data)
    {        
        akprintf(C3, M_DRVSYS, "mouse_ps2_reset():self tst fail\n");
        goto EXIT;
    }
    // get mouse id
    if(!ps2_read(&data))
    {
        akprintf(C3, M_DRVSYS, "mouse_ps2_reset():get mouse id fail\n");
        goto EXIT;
    }    
    s_tMousePs2.ucId = data;
    
    return AK_TRUE;
EXIT:
    return AK_FALSE;
    
}

/**
 * @brief   enable mouse data
 *
 * enable mouse to send data
 * @author Huang Xin
 * @date 2011-01-27
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
static T_BOOL mouse_ps2_enable_data(T_VOID)
{
    T_U8 data;
    
    if(!ps2_write(MOUSE_PS2_CMD_ENABLE_DATA))
    {
        akprintf(C3, M_DRVSYS, "mouse_ps2_enable_data():send MOUSE_PS2_CMD_ENABLE_DATA fail\n");
        goto EXIT;
    }
    //check if enable data success
    if(!ps2_read(&data))
    {        
        akprintf(C3, M_DRVSYS, "mouse_ps2_enable_data():read MOUSE_PS2_RESP_ACK fail\n");
        goto EXIT;
    }
    if (MOUSE_PS2_RESP_ACK != data)
    {        
        akprintf(C3, M_DRVSYS, "mouse_ps2_enable_data(): fail 0x%x\n",data);
        goto EXIT;
    }
    return AK_TRUE;
EXIT:
    return AK_FALSE;
}

/**
 * @brief   mouse enable uart 
 *
 * enable uart to receive mouse data
 * @author Huang Xin
 * @date 2011-01-27
 * @param uart_id[in] uart id 
 * @param rate[in] baud rate
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
static T_BOOL mouse_ps2_enable_uart(T_UART_ID uart_id,T_U32 rate)
{    
    if(!uart_init(uart_id, rate, get_asic_freq()))
    {
        akprintf(C3, M_DRVSYS, "uart init fail\n");
        return AK_FALSE;
    }
    uart_setdataparity(uart_id, 1, 0);
    uart_set_datapool(uart_id, (T_U8 *)s_tMousePs2.BufRcv, MOUSE_PS2_BUF_RCV_LEN);  
    uart_set_callback(uart_id, mouse_ps2_uart_cb);
    return AK_TRUE;
}

/**
 * @brief   uart callback
 *
 * call uart_read() to get mouse data and parse data then call mouse callback,if data len is invalid, re-init mouse
 * @author Huang Xin
 * @date 2011-01-27
 * @return  T_U8
 * @retval  0 means failed
 * @retval  1 means successful
 */
static T_U8 mouse_ps2_uart_cb(T_VOID)
{
    T_U8 len = 0;
    T_U8 mouse_data[4] = {0};
    T_MOUSE_DATA data = {0};
    
    len = uart_read(uiUART3, mouse_data, 4);
    
    if (MOUSE_PS2_STD_DATA_LEN == len || MOUSE_PS2_INTELLI_DATA_LEN == len)
    {
        akprintf(C3, M_DRVSYS, "[%d %d %d]",mouse_ps2_mv_x(mouse_data),mouse_ps2_mv_y(mouse_data),mouse_ps2_btn(mouse_data));
        data.lMvX = mouse_ps2_mv_x(mouse_data);
        data.lMvY = mouse_ps2_mv_y(mouse_data);
        data.lMvZ = mouse_ps2_mv_z(mouse_data);
        data.eBtn = mouse_ps2_btn(mouse_data);
        if(AK_NULL != s_tMousePs2.fMousePs2Cb)
        {
            s_tMousePs2.fMousePs2Cb(&data);
        }
    }
    else
    {
        akprintf(C3, M_DRVSYS, "len:%d,reset\n",len);
        mouse_ps2_open(s_tMousePs2.ucUartId);
        return 0;
    }
    return 1;
}

/**
 * @brief   get mouse move x
 *
 * @author Huang Xin
 * @date 2011-01-27
 * @param mouse_data[in] mouse data packet
 * @return  T_S32
 * @retval  move_x
 */
static T_S32 mouse_ps2_mv_x(T_U8 * mouse_data)
{  
   T_S32 mv_x = 0;
   
   //x sign bit = 1, move left
   if(mouse_data[0]&0x10)
   {
       mv_x = -(256 - mouse_data[1]);
   }
   else
   {
       mv_x = mouse_data[1];
   }
   return(mv_x);
}

/**
 * @brief   get mouse move y
 *
 * @author Huang Xin
 * @date 2011-01-27
 * @param mouse_data[in] mouse data packet
 * @return  T_S32
 * @retval  move_y
 */
static T_S32 mouse_ps2_mv_y(T_U8 * mouse_data)
{      
   T_S32 mv_y = 0;

   //y sign bit = 1, move down
   if(mouse_data[0]&0x20)                         
   {
       mv_y = -(256 - mouse_data[2]);
   }
   else
   {
       mv_y = mouse_data[2];
   }
   return(mv_y);
}

/**
 * @brief   get mouse move z
 *
 * @author Huang Xin
 * @date 2011-01-27
 * @param mouse_data[in] mouse data packet
 * @return  T_S32
 * @retval  move_z
 */
static T_S32 mouse_ps2_mv_z(T_U8 * mouse_data)
{  
   T_S32 mv_z = 0;
   
   return(mv_z);
}

/**
 * @brief   get mouse btn status
 *
 * @author Huang Xin
 * @date 2011-01-27
 * @param mouse_data[in] mouse data packet
 * @return T_eMOUSE_PS2_BTN
 * @retval  btn status
 */
static T_eMOUSE_PS2_BTN mouse_ps2_btn(T_U8 * mouse_data)
{  
    //left btn down
    if(mouse_data[0]&0x01)            
    {
        return MOUSE_PS2_BTN_LEFT;
    }
    //right btn down
    if(mouse_data[0]&0x02)    
    {
        return MOUSE_PS2_BTN_RIGHT;
    }  
    //middle btn down
    if(mouse_data[0]&0x04)             
    {
        return MOUSE_PS2_BTN_MIDDLE;
    }     
    else                     
    {
        return MOUSE_PS2_BTN_NON;
    }     
}
