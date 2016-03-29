
/**
 * @FILENAME: lcd_CI3771.c
 * @BRIEF lcd driver file
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR zhengwenbo
 * @DATE 2008-07-14
 */

#ifdef CI37XX_PLATFORM
#include "lcd.h"
#include "arch_lcd.h"

T_VOID lcd_device_init(T_VOID)
{
    lcd_initial();
    //lcd_set_initial_flag();
}

#endif // endif CI37XX_PLATFORM

