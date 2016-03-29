/**@file hal_ps2.c
 * @brief implement ps/2 protocol.
 *
 * This file implement r/w interface based on ps/2 protocol.
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

static volatile T_PS2 s_tPs2 = {0};

static T_U8 ps2_wait_for_read(T_VOID);
static T_U8 ps2_wait_for_write(T_VOID);
static T_VOID ps2_prepare_receive(T_VOID);
static T_VOID ps2_prepare_send(T_VOID);
static T_VOID ps2_pin_init(T_VOID);
static T_VOID ps2_inhibit_receive(T_VOID);
static T_VOID ps2_set_clk_dir(T_U8 dir);
static T_VOID ps2_set_data_dir(T_U8 dir);
static T_U8 ps2_get_bit();
static T_U8 ps2_set_bit(T_U8 bit);
static T_U8 ps2_check_parity(T_U8 data);
static T_VOID ps2_data_in_isr(T_U32 pin, T_U8 polarity);
static T_VOID ps2_data_out_isr(T_U32 pin, T_U8 polarity);


/**
 * @brief   ps2 init
 *
 * set clk pin ,data pin,and init these pin dir  pullup
 * @author Huang Xin
 * @date 2011-01-27
 * @param clk_pin[in] clk pin num
 * @param data_pin[in] data pin num
 * @return  T_VOID
 */
T_VOID ps2_init(T_U32 clk_pin,T_U32 data_pin)
{
    memset((T_U8*)&s_tPs2, 0 , sizeof(T_PS2));
    s_tPs2.ulClkPin = clk_pin;
    s_tPs2.ulDataPin = data_pin;
    ps2_pin_init();
    ps2_inhibit_receive();
}

/**
 * @brief   get ps2 clk
 *
 * called by ps2 mouse drv to get the mouse clk,called after mouse_ps2_open() ok 
 * @author Huang Xin
 * @date 2011-01-27
 * @return  T_U32
 * @retval  clk(us)
 */
T_U32 ps2_get_clk(T_VOID)
{
    return s_tPs2.ulClk;
}

/**
 * @brief   ps2 close
 * 
 * @author Huang Xin
 * @date 2011-01-27
 * @return  T_VOID
 */
T_VOID ps2_close(T_VOID)
{
    ps2_inhibit_receive();
    memset((T_U8*)&s_tPs2, 0, sizeof(T_PS2));
}

/**
 * @brief   read ps2 data
 *
 * read the data from ps2
 * @author Huang Xin
 * @date 2011-01-27 
 * @param data[out] read data
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL ps2_read(T_U8 *data)
{
    T_BOOL ret = AK_TRUE;
    
    if (ps2_wait_for_read())
    {
        *data = s_tPs2.ucRdBuf;
    }
    else
    {
        ret = AK_FALSE;
    }
    
    //akprintf(C3, M_DRVSYS, "ps2_read(): status = %x\n",s_tPs2.ucStatus);
    //bus is idle for receive
    ps2_prepare_receive();
    return ret;
}

/**
 * @brief   write ps2 data
 *
 * write the data to ps2
 * @author Huang Xin
 * @date 2011-01-27 
 * @param data[out] write data
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
T_BOOL ps2_write(T_U8 data)
{
    T_BOOL ret = AK_TRUE;
    
    s_tPs2.ucWrBuf = data;
    ps2_prepare_send();
    if (ps2_wait_for_write())
    {
        ret = AK_TRUE;
    }
    else
    {
        ret = AK_FALSE;
    }    
    //bus is idle for receive
    //akprintf(C3, M_DRVSYS, "ps2_write(): status = %x\n",s_tPs2.ucStatus);
    ps2_prepare_receive();
    return ret;
}

/**
 * @brief  wait read data ok
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
static T_U8 ps2_wait_for_read(T_VOID)
{
    while(1)
    {
        if (s_tPs2.ucStatus & PS2_TIMEOUT)
        {            
            akprintf(C3, M_DRVSYS, "read timeout\n");
            break;
        }
        if (s_tPs2.ucStatus & PS2_PARITY_ERROR)
        {
            akprintf(C3, M_DRVSYS, "parity error\n");
            break;
        }
        if ((s_tPs2.ucStatus & PS2_RD_BUF_FULL) && (!(s_tPs2.ucStatus & PS2_PARITY_ERROR)))
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief  wait write data ok
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @return  T_BOOL
 * @retval  AK_FALSE means failed
 * @retval  AK_TURE means successful
 */
static T_U8 ps2_wait_for_write(T_VOID)
{
    while(1)
    {
        if (s_tPs2.ucStatus & PS2_TIMEOUT)
        {            
            akprintf(C3, M_DRVSYS, "write timeout\n");
            break;
        }
        if (!(s_tPs2.ucStatus & PS2_WR_BUF_FULL))
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief  prepare ps2 receive
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @return  T_VOID
 */
static T_VOID ps2_prepare_receive(T_VOID)
{    
    //akprintf(C3, M_DRVSYS, "prepare receive\n");
    s_tPs2.ucStatus &= ~(PS2_RD_BUF_FULL);

    //set data dir input    
    ps2_set_data_dir(0);
    //set clk dir input    
    ps2_set_clk_dir(0);
    //falling edge triggered interrupt ,set data in interrupt service routine
    gpio_register_int_callback(s_tPs2.ulClkPin, 0, 1, ps2_data_in_isr);
}

/**
 * @brief  prepare ps2 send
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @return  T_VOID
 */
static T_VOID ps2_prepare_send(T_VOID)
{    
    // akprintf(C3, M_DRVSYS, "prepare send\n");
    s_tPs2.ucStatus |= PS2_WR_BUF_FULL;
    //pull down clk
    ps2_inhibit_receive();
    //delay 100us
    us_delay(100);
    //rising edge triggered interrupt ,set data out interrupt service routine
    gpio_register_int_callback(s_tPs2.ulClkPin, 1, 1, ps2_data_out_isr);

    //data out and pull down data
    ps2_set_data_dir(GPIO_DIR_OUTPUT);

    //release clk line 
    ps2_set_clk_dir(GPIO_DIR_INPUT);

}
/**
 * @brief init ps2 pin dir and pull up
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @return  T_VOID
 */
static T_VOID ps2_pin_init(T_VOID)
{
    gpio_set_pin_as_gpio (s_tPs2.ulClkPin);
    gpio_set_pin_as_gpio (s_tPs2.ulDataPin);
    gpio_set_pin_dir(s_tPs2.ulClkPin, GPIO_DIR_INPUT);
    gpio_set_pin_dir(s_tPs2.ulDataPin, GPIO_DIR_INPUT);
    gpio_set_pull_up_r(s_tPs2.ulClkPin, 1);
    gpio_set_pull_up_r(s_tPs2.ulDataPin, 1);
}

/**
 * @brief  inhibit ps2 receive
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @return  T_VOID
 */
static T_VOID ps2_inhibit_receive(T_VOID)
{
    //disable ulClkPin int
    gpio_int_control(s_tPs2.ulClkPin, 0);
    //pull down clk pin
    gpio_set_pin_dir(s_tPs2.ulClkPin, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(s_tPs2.ulClkPin, 0);    
}

/**
 * @brief  set ps2 clk pin dir
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @return  T_VOID
 */
static T_VOID ps2_set_clk_dir(T_U8 dir)
{
    //input
    if (0 == dir)
    {
        gpio_set_pin_dir(s_tPs2.ulClkPin, GPIO_DIR_INPUT);
    }
    //output
    else
    {
        gpio_set_pin_dir(s_tPs2.ulClkPin, GPIO_DIR_OUTPUT);
    }
}

/**
 * @brief  set ps2 data pin dir
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @return  T_VOID
 */
static T_VOID ps2_set_data_dir(T_U8 dir)
{
    //input
    if (0 == dir)
    {
        gpio_set_pin_dir(s_tPs2.ulDataPin, GPIO_DIR_INPUT);
    }
    //output
    else
    {
        gpio_set_pin_dir(s_tPs2.ulDataPin, GPIO_DIR_OUTPUT);
        gpio_set_pin_level(s_tPs2.ulDataPin, 0);
    }
}

/**
 * @brief  get one bit from ps2
 *
 * @author Huang Xin
 * @date 2011-01-27  
 * @param bit[in] bit to get
 * @return  T_U8
 */
static T_U8 ps2_get_bit()
{
    T_U8 bit;
    T_U32 time = 0;

    //wait for clock to low
    while(gpio_get_pin_level(s_tPs2.ulClkPin))
    {
        if(time >= 5000) 
        {
            return PS2_TIMEOUT;
        }
        time++;
    }
    //get bit
    bit = gpio_get_pin_level(s_tPs2.ulDataPin);
    // wait for clock to high
    time = 0;
    while(!gpio_get_pin_level(s_tPs2.ulClkPin))
    {
        if(time >= 5000) 
        {
            return PS2_TIMEOUT;
        }
        time++;
    }
    return bit;
}

/**
 * @brief  set one bit to ps2
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @param bit[in] bit to set
 * @return  T_U8
 */
static T_U8 ps2_set_bit(T_U8 bit)
{
    T_U32 time = 0;

    //wait for clock to low
    while(gpio_get_pin_level(s_tPs2.ulClkPin))
    {
        if(time >= 5000) 
        {
            return PS2_TIMEOUT;
        }
        time++;
    }
    //set bit
    gpio_set_pin_level(s_tPs2.ulDataPin, bit);
    // wait for clock to high 
    time = 0;
    while(!gpio_get_pin_level(s_tPs2.ulClkPin))
    {
        if(time >= 5000) 
        {
            return PS2_TIMEOUT;
        }
        time++;
    }
    return 1;
}

/**
 * @brief  check parity bit
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @param data[in] one byte data
 * @return  T_U8
 */
static T_U8 ps2_check_parity(T_U8 data)
{
    T_U8 i, p = 1;

    //odd parity check
    for(i = 0; i < 8; i++)
    {
        if(data & (1<<i))
            p = !p;
    }
    return p;
}

/**
 * @brief  data in isr
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @param pin[in] pin num
 * @param polarity[in] high or low
 * @return  T_VOID
 */
static T_VOID ps2_data_in_isr(T_U32 pin, T_U8 polarity)
{
    T_U8 buf_shift = 0;
    T_U8 i,p,s;

    //inhibit gpio int
    gpio_int_control(pin, 0);
    //get start bit 
    buf_shift = ps2_get_bit();
    if((PS2_TIMEOUT == buf_shift) || (1 == buf_shift))
    {        
        goto ERROR;
    }

    //get data bits
    for(i = 0; i < 8; i++)
    {            
        buf_shift >>= 1;
        switch(ps2_get_bit())
        {
            case 1:    
                buf_shift |= 0x80;
                break;
            case 0: 
                break;
            default:
                goto ERROR;
        }        
    }

    //get  parity bit
    p = ps2_get_bit();
    if(p == PS2_TIMEOUT)
    {
        goto ERROR;
    }
    if(p != ps2_check_parity(buf_shift))
    {
        s_tPs2.ucStatus |= PS2_PARITY_ERROR;
    }
    //get stop bit 
    s = ps2_get_bit();
    if (PS2_TIMEOUT == s || 0 == s)
    {        
        goto ERROR;
    }
    //save data
    s_tPs2.ucRdBuf = buf_shift;
    s_tPs2.ucStatus |= PS2_RD_BUF_FULL;
    return;
ERROR:
    s_tPs2.ucStatus |= PS2_TIMEOUT;
    return;

}

/**
 * @brief  data out isr
 *
 * @author Huang Xin
 * @date 2011-01-27 
 * @param pin[in] pin num
 * @param polarity[in] high or low
 * @return  T_VOID
 */
static T_VOID ps2_data_out_isr(T_U32 pin, T_U8 polarity)
{
    T_U8 buf_shift = 0;
    T_U8 i,p,ack;
    T_U64 ullStartTime = 0;
    
    //inhibit gpio int
    gpio_int_control(pin, 0);
    //get data
    buf_shift = s_tPs2.ucWrBuf;
    //get parity bit 
    p = ps2_check_parity(buf_shift);

    //output data bits 
    for(i = 0; i < 8; i++)
    {    
        if(PS2_TIMEOUT == ps2_set_bit(buf_shift&0x01))
        {
            goto ERROR;
        }
        //start point for calculating rate  
        if (0 == ullStartTime && 0 == s_tPs2.ulClk)
        {
            ullStartTime = get_tick_count_us();
        }
        buf_shift >>= 1;
    }

    //output  parity bit
    if(PS2_TIMEOUT == ps2_set_bit(p))
    {
        goto ERROR;
    }

    //output stop bit
    if(PS2_TIMEOUT == ps2_set_bit(1))
    {
        goto ERROR;
    }
    //set data dir  input 
    ps2_set_data_dir(0);
    //input ack bit
    ack = ps2_get_bit();

    //end point for calculating rate , 10 clk past 
    if (0 == s_tPs2.ulClk)
    {
        s_tPs2.ulClk = (T_U32)((get_tick_count_us() - ullStartTime)/10);
    }
    if(PS2_TIMEOUT == ack || 0 != ack)
    {
        goto ERROR;
    }
    //clr buf full status
    s_tPs2.ucStatus &= ~(PS2_WR_BUF_FULL);
    return;
ERROR:
    s_tPs2.ucStatus |= PS2_TIMEOUT;
    return;    
}

