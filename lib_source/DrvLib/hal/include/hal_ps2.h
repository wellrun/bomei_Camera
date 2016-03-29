/**@file hal_ps2.h
 * @brief implement ps/2 protocol.
 *
 * This file provide r/w interface based on ps/2 protocol.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2011-01-06
 * @version 1.0
 */

#ifndef __HAL_PS2_H
#define __HAL_PS2_H


#ifdef __cplusplus
extern "C" {
#endif

#define PS2_DATA_IN     1
#define PS2_DATA_OUT    0
//ps2 status
#define PS2_RD_BUF_FULL         (1<<0)
#define PS2_WR_BUF_FULL         (1<<1)    
#define PS2_TIMEOUT             (1<<2)
#define PS2_PARITY_ERROR        (1<<3)

typedef struct _PS2_INFO
{  
    T_U8    ucRdBuf;
    T_U8    ucWrBuf;
    T_U8    ucStatus;
    T_U32   ulClkPin;
    T_U32   ulDataPin;
    T_U32   ulClk;
}T_PS2,*T_pPS2;

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
T_VOID ps2_init(T_U32 clk_pin,T_U32 data_pin);
/**
 * @brief   get ps2 clk
 *
 * called by ps2 mouse drv to get the mouse clk,called after mouse_ps2_open() ok 
 * @author Huang Xin
 * @date 2011-01-27
 * @return  T_U32
 * @retval  clk(us)
 */
T_U32 ps2_get_clk(T_VOID);
/**
 * @brief   ps2 close
 * 
 * @author Huang Xin
 * @date 2011-01-27
 * @return  T_VOID
 */
T_VOID ps2_close(T_VOID);
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
T_BOOL ps2_read(T_U8 *data);
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

T_BOOL ps2_write(T_U8 data);


#ifdef __cplusplus
}
#endif

#endif
