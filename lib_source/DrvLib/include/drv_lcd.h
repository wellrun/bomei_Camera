/**
 * @file drv_lcd.h
 * @brief This file describe the interface of lcd module for lcd driver
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author lianGenhui
 * @date 2010-06-18
 * @version 2.0
 */

#ifndef __DRIVER_LCD_H__
#define __DRIVER_LCD_H__

#include "anyka_types.h"
#include "arch_lcd.h"



/** @defgroup driver_lcd lcd device driver
 *  @ingroup LCD
 */
/*@{*/
#ifdef __cplusplus
extern "C" {
#endif

/*
*
*@brief STNLCD info define.
*define the info of STNLCD
*/
typedef struct
{
    T_U16   cpl;    //Whether reverse gray or not
    T_U16   Wlh;    //VLINE pulse width( unit: vclk)
    T_U16   Wdly;   //Delay between VLINE and VCLK ( unit: vclk)
    T_U16   Wline;  //Blank time in one horizontal
    T_U8    *pName;
}T_STNLCD_INFO;

/**
 * @brief RGBLCD info define.
 * define the info of RGBLCD
 */
typedef struct 
{
    T_BOOL  isInterlace;        ///< Interlace(1) or progress(0)
    T_U8    PHVG_POL;           ///< PCLK(bit3)Hsync(bit2), Vsync(bit1), Gate(bite0) polarity
    T_U16   RGBorGBR;           ///< 0=rgb,1=gbr
    T_U16   Thlen;              ///< horizontal cycle, Unit PCLK
    T_U16   Thd;                ///< horizontal display period, Unit PCLK
    T_U8    Thf;                ///< Horizontal Front porch, Unit PCLK
    T_U8    Thpw;               ///< Horizontal Pulse width, Unit PCLK
    T_U8    Thb;                ///< Horizontal Back porch, Unit PCLK
    T_U16   Tvlen;              ///< Vertical cycle, Unit Hsync
    T_U16   Tvd;                ///< Vertical display period, Unit line
    T_U8    Tvf;                ///< Vertical Front porch, Unit line
    T_U8    Tvpw;               ///< Vertical Pulse width, Unit line
    T_U8    Tvb;                ///< Vertical Back porch, Unit line
    T_U8    *pName;
}T_RGBLCD_INFO;

/**
 * @brief lcd info define.
 * define the info of all lcd
 */
typedef struct
{
    T_U32   lcd_width;                      ///< pannel size width
    T_U32   lcd_height;                     ///< pannel size height

    T_U32   lcd_PClk_Hz;                    ///< Clock cycle, Unit Hz
    T_U8    lcd_BusWidth;                   ///< data bus width(8,9,16,18 or 24)  
    T_U8    lcd_type;                       ///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
    T_U8    lcd_color_sel;                  ///< 0x00 4K color, 0x01 64K/256K color
    T_U8    flag;
    T_VOID  *lcd_reginfo;                   ///< refer to T_RGBLCD_INFO or T_STNLCD_INFO define

    T_U32  (*lcd_read_id_func)(T_eLCD lcd); ///< only for MPU interface 
    T_VOID (*lcd_init_func)(T_eLCD lcd);    ///< init for lcd driver 
    T_VOID (*lcd_turn_on_func)(T_eLCD lcd);
    T_VOID (*lcd_turn_off_func)(T_eLCD lcd);
    T_VOID (*lcd_set_disp_address_func)(T_eLCD lcd, T_U32 x1, T_U32 y1, T_U32 x2, T_U32 y2);
    T_BOOL (*lcd_rotate_func)(T_eLCD lcd, T_eLCD_DEGREE degree);
    T_VOID (*lcd_start_dma_func)(T_eLCD lcd);
}T_LCD_FUNCTION_HANDLER;

typedef struct
{
    T_U32 DeviceID;
    T_LCD_FUNCTION_HANDLER *handler;
}T_LCD_INFO;

/**
 * @brief LCD MPU control define.
 * define mpu control data or command
 */
typedef enum
{
    LCD_MPU_CMD = 0,
    LCD_MPU_DATA
} T_eLCD_MPU_CTL;

#define LCD_NOT_USED        0xfffff
#define LCD_MAX_SUPPORT     20


/**
 * @brief   send command or data to the lcd device(for MPU)
 * @author  LianGenhui
 * @date    2010-06-18
 * @param[in]  lcd select the lcd, LCD_0 or LCD_1
 * @param[in]  ctl send command or data, refer to T_eLCD_MPU_CTL definition
 * @param  data the data or command want to send
 * @return  T_VOID
 */
T_VOID lcd_MPU_ctl (T_eLCD lcd, T_eLCD_MPU_CTL ctl, T_U32 data);

/**
 * @brief read back the lcd device's register or GRAM data(for MPU) 
 * this function is generally called after sending register or GRAM address
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd select the lcd, LCD_0 or LCD_1
 * @return  T_U32
 * @retval  the value of the lcd device's register or GRAM data
 */
T_U32 lcd_readback (T_eLCD lcd);

/**
 * @brief register lcd device
 * @author Liangenhui
 * @date 2010-06-18
 * @param[in] id lcd id
 * @param[in] handler lcd device pointer
 * @param[in] idx_sort_foward T_BOOL  idx_sort_foward  Ak_TRUE  index from 0 t0 19 , AK_FALSE index from 19 to 0
 * @return T_VOID
 */
T_BOOL lcd_reg_dev(T_U32 id, T_LCD_FUNCTION_HANDLER *handler,T_BOOL idx_sort_foward);


#ifdef __cplusplus
}
#endif

/*@}*/
#endif

