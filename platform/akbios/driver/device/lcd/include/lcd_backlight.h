/**
 * @FILENAME: lcd_backlight.h
 * @BRIEF LCD backlight driver head file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @VERSION 1.0
 * @REF
 */

#ifndef __LCD_BACKLIGHT_H__
#define __LCD_BACKLIGHT_H__


/**
 * @BRIEF Set brightness value
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @PARAM T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @PARAM T_U8 brightness: brightness value
 * @RETURN T_U8: new brightness value after setting
 * @RETVAL
 */
T_U8 lcdbl_set_brightness(T_eLCD lcd, T_U8 brightness);

#endif

