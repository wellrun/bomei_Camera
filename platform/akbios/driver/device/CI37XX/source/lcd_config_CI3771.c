/**
 * @FILENAME: lcd_config_CI7802.c
 * @BRIEF lcd_config_CI7802 file
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR 
 * @DATE 2008-07-24
 * @VERSION 1.0
 * @REF
 */
#ifdef CI7802_PLATFORM

#include "akdefine.h"
#include "lcd.h"


/**
 * @BRIEF select lcd type
 * @AUTHOR guoshaofeng
 * @DATE 2008-07-24
 * @RETURN E_TS_TYPE, touch screen type, resistant or capacitive
 * @RETVAL
 */
E_LCD_TYPE lcd_select_type(T_VOID)
{
    return E_LCD_TYPE_RGB;
}

/**
 * @BRIEF  get rgblcd info
 * @AUTHOR guoshaofeng
 * @DATE 2008-07-24
 * @RETURN T_RGBLCD_INFO, rgblcd info struct define
 * @RETVAL
 */
T_RGBLCD_INFO *Rgblcd_get_info(T_VOID)
{
	printf("get rgb info!\n\n");

//    return &(SUPPORT_RGBLCD_TABLE[RGBLCD_LQ043T3DX02]);
}

#endif // endif CI7802_PLATFORM

