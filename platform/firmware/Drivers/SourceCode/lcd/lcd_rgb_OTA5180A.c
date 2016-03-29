/**
 * @FILENAME: lcd_rgb_OTA5180A.c
 * @BRIEF LCD driver file
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR LianGenhui
 * @DATE 2010-07-12
 * @VERSION 1.0
 * @REF
 * @Note:    MODE: 
 *            LCD Module : 480X272 RGB
 *            Screen Size: 
 *            Interface: 8 bits
 */ 
#include "akdefine.h"
#include "drv_lcd.h"

#include "drv_api.h"
#include "platform_hd_config.h"

#ifdef USE_LCD_RGB_OTA5180A

#define LCD_OTA5180A_ID 0x00005180

static T_RGBLCD_INFO SUPPORT_RGB_OTA5180A_TABLE[] = 
{
    //Interlace, HVG_Pol,RGB_BIT, PClk_Hz,Thlen_PClk,Thd_PClk,Thf_PClk,Thpw_PClk,Thb_PClk,Tvlen_HCLK,Tvd_HCLK,Tvf_PClk,Tvpw_PClk,Tvb_PClk,pName
    AK_FALSE,  0x07,    0,      1716,       1440,     14,       10,      252,        288,      272,    3,       3,      10,       "OTA5180A"
};

static T_U32 lcd_OTA5180A_read_id(T_eLCD lcd)
{
    return LCD_OTA5180A_ID;
}

static T_VOID lcd_OTA5180A_init(T_eLCD lcd)
{
    //AkDebugOutput(lcd_OTA5180A_init);
}

static T_VOID lcd_OTA5180A_turn_on(T_eLCD lcd)
{

}
static T_VOID lcd_OTA5180A_turn_off(T_eLCD lcd)
{
 
}


static T_LCD_FUNCTION_HANDLER OTA5180A_function_handler =
{
    480,            ///< pannel size width
    272,            ///< pannel size height
    27000000,           ///< Clock cycle, Unit Hz
    8,               ///< data bus width(8,9,16,18 or 24)  
    0x02,            ///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
    0x01,            ///< 0x00 4K color, 0x01 64K/256K color
    0x01,            ///< no use
    SUPPORT_RGB_OTA5180A_TABLE,    ///< refer to T_RGBLCD_INFO define
    
    lcd_OTA5180A_read_id,         ///< only for MPU interface 
    lcd_OTA5180A_init,        ///< init for lcd driver 
    lcd_OTA5180A_turn_on,        
    lcd_OTA5180A_turn_off,            
    AK_NULL,
    AK_NULL,
    AK_NULL,
};


static int lcd_OTA5180A_reg(void)
{
    lcd_reg_dev(LCD_OTA5180A_ID, &OTA5180A_function_handler, AK_TRUE);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(lcd_OTA5180A_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif





