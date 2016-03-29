/*****************************************************************************
 * Copyright (C) 2003 Anyka Co. Ltd
 *****************************************************************************
 *   Project: 
 *****************************************************************************
 * $Workfile: $
 * $Revision: $
 *     $Date: $
 *****************************************************************************
 * Description:
 *
 *****************************************************************************
*/
#ifdef OS_WIN32
#include <stdlib.h>
#include <assert.h>

#include "vme_interface.h"

#include "akdefine.h"

#include "winvme.h"
#include "fwl_pfdisplay.h"
#include "Fwl_display.h"


/*****************************************************************************
 * vdisplay
 *****************************************************************************
*/

T_pDATA pGB_dispBuffBackup = AK_NULL;

vVOID lcd_turn_on(DISPLAY_TYPE_DEV lcd)
{
    winvme_DisplayOn (lcd);
    return;
}

vVOID lcd_turn_off(DISPLAY_TYPE_DEV lcd)
{
    winvme_DisplayOff (lcd);
    return;
}

vVOID vInitDisplay(DISPLAY_TYPE_DEV lcd)
{
    winvme_InitDisplay (lcd);
    return;
}


T_U8 ddvdisplayhandler (vT_EvtCode* Event, vT_EvtParam** pParam)
{
    // dummy event handler
    return 0;
}


vVOID lcd_set_pixel(vUINT16 col, vUINT16 page, vUINT32 content)
{
    vUINT32 uiColor;
    vUINT8  r,g,b;

    // simulation supports only 12 bit color
    /* color: 0b00000000 00000000 RRRRGGGG BBBB0000 */

    b = (vUINT8)(content);
    g = (vUINT8)(content>>8);
    r = (vUINT8)(content>>16);

    uiColor = ((vUINT32)(((vUINT8)(r)|((vUINT16)((vUINT8)(g))<<8))|(((vUINT32)(vUINT8)(b))<<16)));

    // in the first step: only one LCD is supported
	if((col < Fwl_GetLcdWidth()) && (page < Fwl_GetLcdHeight()) )
	{
		winvme_SetPixel (DISPLAY_LCD_0, col, page, uiColor);
	}

    return;
}

vUINT32 lcd_get_pixel(DISPLAY_TYPE_DEV lcd, vUINT16 col, vUINT16 page)
{
    vUINT32 uiColor;
    vUINT8  r,g,b;
	vUINT32 ColorIndex;
	
    //if ((col < Fwl_GetLcdWidth(DISPLAY_LCD_0)) && (page < LCD0_HEIGHT))
    {
		if ( (lcd == DISPLAY_LCD_0) && (col < Fwl_GetLcdWidth()) && (page < Fwl_GetLcdHeight()))
		{
			winvme_GetPixel (DISPLAY_LCD_0, col, page, &uiColor);
		}
// 		else if( (lcd == LCD_1) && (col < Fwl_GetLcdWidth(LCD_1)) && (page < Fwl_GetLcdHeight(LCD_1)))
// 		{
// 			winvme_GetPixel (LCD_1, col, page, &uiColor);
// 		}

        // simulation supports only 12 bit color
        /* color: 0b00000000 00000000 RRRRGGGG BBBB0000 */
        r = ((vUINT8)(uiColor));   
        g = ((vUINT8)(((vUINT16)(uiColor)) >> 8));
        b = ((vUINT8)((uiColor)>>16));

        ColorIndex  = r;
		ColorIndex <<= 8;
		ColorIndex |= g;
		ColorIndex <<= 8;
		ColorIndex |= b;
        //*ColorIndex |= (vUINT16) (b & 0xF0);
        //*ColorIndex |= (vUINT16)((g & 0xF0)<<4);
        //*ColorIndex |= (vUINT16)((r & 0xF0)<<8);

    }

    return ColorIndex;
}

vVOID fillRect(DISPLAY_TYPE_DEV lcd, vUINT16 col, vUINT16 line, vUINT16 width, vUINT16 height, vUINT32 color)
//vVOID seFillRect(T_eLCD lcd, vUINT8 col1, vUINT8 col2, vUINT8 page1, vUINT8 page2, vUINT16 rgb)
{
    vUINT16 i,j,a,b;


    //if ((col1 < Fwl_GetLcdWidth(DISPLAY_LCD_0)) && (col2 < Fwl_GetLcdWidth(DISPLAY_LCD_0)) && (page1 < LCD0_HEIGHT) && (page2 < LCD0_HEIGHT))
    {
		for(i = col, a=0; a<width; i++, a++)
		{
			for(j = line, b=0; b<height; j++, b++ )
			{
				lcd_set_pixel( i, j, color);
			}
		}
    }

    return;
}

int LCD_REFRESH_RECT[4]={0};
vVOID lcd_refresh(DISPLAY_TYPE_DEV lcd, T_U16 left, T_U16 top, T_U16 width, T_U16 height)
{
	LCD_REFRESH_RECT[0] =left;
	LCD_REFRESH_RECT[1] =top;
	LCD_REFRESH_RECT[2] =width;
	LCD_REFRESH_RECT[3] =height;
    winvme_UpdateDisplay (lcd);
    return;
}

vINT8 incContrast(DISPLAY_TYPE_DEV lcd)
{
	return 0;
}

vINT8 decContrast(DISPLAY_TYPE_DEV lcd)
{
	return 0;
}
/*
void inc_backlight_brightness()
{
	return;
}

void dec_backlight_brightness()
{
	return;
}
*/
T_VOID sub_lcd_off()
{
}

T_VOID sLCD_HScroll()
{
}

T_VOID sLCD_Recover()
{
}


/*T_S16 lcd_backlight_recover_brightness(T_eLCD lcd, T_U16 brightness)
{
	return 1;
}
*/
T_U8 lcd_set_brightness(DISPLAY_TYPE_DEV lcd, T_U8 brightness)
{
	return 1;
}

T_VOID lcd_set_buf_height(DISPLAY_TYPE_DEV lcd, T_U32 buf_h)
{
    g_Graph.LCDHEIGHT[lcd] = (T_S16)buf_h;
}

T_VOID lcd_set_buf_width(DISPLAY_TYPE_DEV lcd, T_U32 buf_w)
{
    g_Graph.LCDWIDTH[lcd] = (T_S16)buf_w;
}

/**
 * @brief  	rotate the lcd
 * @author	lgj
 * @date	2005-08-18
 * @param[in] lcd selected LCD, must be DISPLAY_LCD_0 or LCD_1
 * @param[in] degree rotate degree
 * @return  T_VOID
 */
static DISP_DEGREE deg;
T_VOID lcd_rotate(DISPLAY_TYPE_DEV lcd, DISP_DEGREE degree)
{
    deg=degree;
}

/**
 * @brief  	return current lcd degree
 * @author	lgj
 * @date	2005-08-18
 * @param[in] lcd selected LCD, must be DISPLAY_LCD_0 or LCD_1
 * @return  T_eLCD_DEGREE
 * @retval  degree of LCD
 */
DISP_DEGREE lcd_degree(DISPLAY_TYPE_DEV lcd)
{
    return deg;
}


T_U32 lcd_get_buffer_height(DISPLAY_TYPE_DEV lcd)
{
    if(DISPLAY_LCD_0 == lcd)
    {
        return g_Graph.LCDHEIGHT[lcd];
    }
    
    return SLAVE_LCD_WIDTH;
}

T_U32 lcd_get_buffer_width(DISPLAY_TYPE_DEV lcd)
{
    if(DISPLAY_LCD_0 == lcd)
    {
        return g_Graph.LCDWIDTH[lcd];
    }
    
    return SLAVE_LCD_WIDTH;
}

T_VOID lcd_refresh_YUV1(DISPLAY_TYPE_DEV lcd, T_U8 *srcY, T_U8 *srcU,T_U8 *srcV,
                        T_U16 src_width, T_U16 src_height,
                        T_U16 left, T_U16 top,
					 T_U16 dst_width, T_U16 dst_height)
{
    lcd_refresh(lcd,left,top,dst_width,dst_height);
}
#endif