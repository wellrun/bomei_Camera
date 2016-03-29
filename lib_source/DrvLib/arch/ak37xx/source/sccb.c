/**
 * @file sccb.c
 * @brief SCCB interface driver, define SCCB interface APIs.
 * This file provides SCCB APIs: SCCB initialization, write data to SCCB & read data from SCCB.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @version 1.0
 * @ref AK3210M technical manual.
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"

#ifdef OS_ANYKA

T_VOID sccb_init(T_U32 pin_scl, T_U32 pin_sda)
{
    i2c_init(pin_scl, pin_sda);
}

T_BOOL sccb_write_data(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size)
{
     return i2c_write_data(daddr, raddr, data, size);
}

T_BOOL sccb_write_data3(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size)
{
     return i2c_write_data2(daddr, raddr, data, size);
}

T_BOOL sccb_write_data4(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size)
{
     return i2c_write_data3(daddr, raddr, data, size);
}

T_U8 sccb_read_data(T_U8 daddr, T_U8 raddr)
{
    T_U8 readdata = 0;

     i2c_read_data(daddr, raddr, &readdata, 1);

     return readdata;
}

T_BOOL sccb_read_data2(T_U8 daddr, T_U8 raddr, T_U8 *data, T_U32 size)
{
    return i2c_read_data(daddr, raddr, data, size);
}

T_BOOL sccb_read_data3(T_U8 daddr, T_U16 raddr, T_U8 *data, T_U32 size)
{
    return i2c_read_data2(daddr, raddr, data, size);
}

T_BOOL sccb_read_data4(T_U8 daddr, T_U16 raddr, T_U16 *data, T_U32 size)
{
    return i2c_read_data3(daddr, raddr, data, size);
}

#endif

/* end of file */
