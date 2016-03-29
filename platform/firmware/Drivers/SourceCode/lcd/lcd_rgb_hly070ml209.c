/**
 * @FILENAME: lcd_rgb_hsd0701dw1.c
 * @BRIEF LCD driver file
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR LianGenhui
 * @DATE 2010-07-12
 * @VERSION 1.0
 * LCD Module : 800*480 RGB
 * Interface: 24 bits
 */ 
#include "akdefine.h"
#include "drv_lcd.h"

#include "drv_api.h"
#include "platform_hd_config.h"


#ifdef USE_LCD_RGB_HLY070ML209

#define LCD_HLY070ML209_ID 0xff006000

static T_RGBLCD_INFO SUPPORT_RGB_HLY070ML209_TABLE[] = 
{
    AK_FALSE,   //Interlace
    0xe,        //HVG_Pol
    0,          //RGB_BIT
    1056,        //Thlen_PClk
    800,        //Thd_PClk
    210,         //Thf_PClk
    20,         //Thpw_PClk
    46,         //Thb_PClk
    525,        //Tvlen_HCLK
    480,        //Tvd_HCLK
    22,         //Tvf_PClk
    10,          //Tvpw_PClk
    23,         //Tvb_PClk
    "hsd0701dw1" //pName
};

static T_U32 lcd_hly070ml209_read_id(T_eLCD lcd)
{
    return LCD_HLY070ML209_ID;
}

static T_VOID lcd_hly070ml209_init(T_eLCD lcd)
{
}

static T_VOID lcd_hly070ml209_turn_on(T_eLCD lcd)
{
}
static T_VOID lcd_hly070ml209_turn_off(T_eLCD lcd)
{
}


static T_LCD_FUNCTION_HANDLER hly070ml209_function_handler =
{
    800,       ///< pannel size width
    480,       ///< pannel size height
    40000000,  ///< Clock cycle, Unit Hz
    24,        ///< data bus width(8,9,16,18 or 24)  
    0x02,      ///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
    0x01,      ///< 0x00 4K color, 0x01 64K/256K color
    0x0,       ///< no use
    SUPPORT_RGB_HLY070ML209_TABLE,    ///< refer to T_RGBLCD_INFO define
    
    lcd_hly070ml209_read_id,     ///< only for MPU interface 
    lcd_hly070ml209_init,        ///< init for lcd driver 
    lcd_hly070ml209_turn_on,        
    lcd_hly070ml209_turn_off,            
    AK_NULL,
    AK_NULL,
    AK_NULL,

};


static int lcd_hly070ml209_reg(void)
{
    lcd_reg_dev(LCD_HLY070ML209_ID, &hly070ml209_function_handler, AK_TRUE);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(lcd_hly070ml209_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif






