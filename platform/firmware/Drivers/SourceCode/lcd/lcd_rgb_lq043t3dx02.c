/**
 * @FILENAME: lcd_rgb_lq043t3dx02.c
 * @BRIEF LCD driver file
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR LianGenhui
 * @DATE 2010-07-12
 * @VERSION 1.0
 * @REF
 * @Note:    MODE: 
 *            LCD Module : 400X272 RGB
 *            Screen Size: 
 *            Interface: 18 bits
 */ 
#include "akdefine.h"
#include "drv_lcd.h"

#include "drv_api.h"
#include "platform_hd_config.h"

#ifdef USE_LCD_RGB_LQ043T3DX02

#define LCD_LQ043T3DX02_ID 0xff004302


static T_RGBLCD_INFO SUPPORT_RGB_LQ043T3DX02_TABLE[] = 
{
    //Interlace, HVG_Pol,RGB_BIT, PClk_Hz,Thlen_PClk,Thd_PClk,Thf_PClk,Thpw_PClk,Thb_PClk,Tvlen_HCLK,Tvd_HCLK,Tvf_PClk,Tvpw_PClk,Tvb_PClk,pName
    AK_FALSE,  0xe,    0,      525,       480,     2,       41,      2,        286,      272,    2,       10,      2,       "LQ043T3DX02"
};

static T_U32 lcd_lq043t3dx02_read_id(T_eLCD lcd)
{
    return LCD_LQ043T3DX02_ID;
}

static T_VOID lcd_lq043t3dx02_init(T_eLCD lcd)
{
    //AkDebugOutput(lcd_lq043t3dx02_init);
}

static T_VOID lcd_lq043t3dx02_turn_on(T_eLCD lcd)
{
}
static T_VOID lcd_lq043t3dx02_turn_off(T_eLCD lcd)
{
}


static T_LCD_FUNCTION_HANDLER lq043t3dx02_function_handler =
{
    480,            ///< pannel size width
    272,            ///< pannel size height
    9000000,           ///< Clock cycle, Unit Hz
    18,               ///< data bus width(8,9,16,18 or 24)  
    0x02,            ///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
    0x01,            ///< 0x00 4K color, 0x01 64K/256K color
    0x0,            ///< no use
    SUPPORT_RGB_LQ043T3DX02_TABLE,    ///< refer to T_RGBLCD_INFO define
    
    lcd_lq043t3dx02_read_id,         ///< only for MPU interface 
    lcd_lq043t3dx02_init,        ///< init for lcd driver 
    lcd_lq043t3dx02_turn_on,        
    lcd_lq043t3dx02_turn_off,            
    AK_NULL,
    AK_NULL,
    AK_NULL,
};


static int lcd_lq043t3dx02_reg(void)
{
    lcd_reg_dev(LCD_LQ043T3DX02_ID, &lq043t3dx02_function_handler, AK_TRUE);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(lcd_lq043t3dx02_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif





