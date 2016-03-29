#ifndef __LCD_H__
#define __LCD_H__

#include "akdefine.h"
#include "drv_api.h"


#define MAIN_LCD_WIDTH		240
#define MAIN_LCD_HEIGHT	    320

#define SLAVE_LCD_WIDTH	    0	    //no slave lcd
#define SLAVE_LCD_HEIGHT	0

/**
 * @brief Refresh the LCD
 * Send the display buffer to specified LCD and refresh the screen
 * @author qxj
 * @date 2005-07-18
 * @param T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @param T_U16 left, T_U16 top: the coordinate of the rect that will be refreshed
 * @param T_U16 width, T_U16 height: the width and height of the refresh rect
 * @return T_VOID
 */
//T_VOID lcd_refresh_RGB(T_eLCD lcd, T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_U8 *disp_buf);

#endif

