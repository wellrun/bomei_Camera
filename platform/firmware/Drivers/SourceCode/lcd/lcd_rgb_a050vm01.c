/**
 * @FILENAME: lcd_rgb_tm050rdz00.c
 * @BRIEF LCD driver file
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR LianGenhui
 * @DATE 2010-07-12
 * @VERSION 1.0
 * @REF
 * @Note:	MODE: 
 *			LCD Module : 400X272 RGB
 *			Screen Size: 
 *			Interface: 18 bits
 */ 
#include "akdefine.h"
#include "drv_lcd.h"

#include "drv_api.h"
#include "platform_hd_config.h"

#ifdef USE_LCD_RGB_A050VM01

#define LCD_A050VM01_ID 0xff005005


static T_RGBLCD_INFO SUPPORT_RGB_A050VM01_TABLE[] = 
{
    //Interlace, HVG_Pol,RGB_BIT, PClk_Hz,Thlen_PClk,Thd_PClk,Thf_PClk,Thpw_PClk,Thb_PClk,Tvlen_HCLK,Tvd_HCLK,Tvf_PClk,Tvpw_PClk,Tvb_PClk,pName
	//AK_FALSE,  0x6, 	0,		 894,	   800, 	7,		80,	 7, 	   525, 	 480,	 5, 	  35,	  5,	   "tm050rdz00"
	AK_FALSE,  0xe,     0,       928,      800,     7,      114, 7,        525,      480,    5,       35,     5,       "A050VM01"

}; 


static T_U32 lcd_a050vm01_read_id(T_eLCD lcd)
{
	return LCD_A050VM01_ID;
}

static T_VOID lcd_a050vm01_init(T_eLCD lcd)
{
}

static T_VOID lcd_a050vm01_turn_on(T_eLCD lcd)
{
}
static T_VOID lcd_a050vm01_turn_off(T_eLCD lcd)
{
}


static T_LCD_FUNCTION_HANDLER a050vm01_function_handler =
{
	800,			///< pannel size width
	480,			///< pannel size height
    25000000,   		///< Clock cycle, Unit Hz
    24,       		///< data bus width(8,9,16,18 or 24)  
    0x02,			///< interface type(0x01 MPU, 0x02 RGB, 0x03 TV-out)
    0x01,			///< 0x00 4K color, 0x01 64K/256K color
	0x0,			///< no use
	SUPPORT_RGB_A050VM01_TABLE,	///< refer to T_RGBLCD_INFO define
	
	lcd_a050vm01_read_id, 	///< only for MPU interface 
	lcd_a050vm01_init,		///< init for lcd driver 
	lcd_a050vm01_turn_on,		
	lcd_a050vm01_turn_off,			
	AK_NULL,
	AK_NULL,
	AK_NULL,

};


static int lcd_a050vm01_reg(void)
{
	lcd_reg_dev(LCD_A050VM01_ID, &a050vm01_function_handler, AK_TRUE);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(lcd_a050vm01_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif





