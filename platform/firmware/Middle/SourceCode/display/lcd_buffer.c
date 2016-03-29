/**
 * @FILENAME: lcd_buffer.c
 * @BRIEF LCD device list file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR linhexi
 * @DATE 2007-04-023
 * @VERSION 1.0
 * @REF
 */
#include <string.h>
#include "anyka_types.h"
#include "eng_graph.h"
#include "Ctl_AVIPlayer.h"
#include "Eng_Debug.h"
#include "hal_print.h"
#include "fwl_osmalloc.h"
#include "arch_gui.h"

#include "Fwl_display.h"

#define SCALE_USE_HARDWARE

//dynamically allocated display buffer
T_pDATA pGB_dispBuffBackup = AK_NULL;
T_pDATA gb_DisplayBuffer_Temp = AK_NULL;

/**
 * @brief Set pixel
 * Set the color of the specified pixel
 * @author qxj
 * @date 2005-07-18
 * @param T_U16 x, T_U16 y: the coordinate of the pixel
 * @param T_COLOR color: the color of the pixel
 * @return T_VOID
 */

T_U8*  lcd_get_disp_buf(T_VOID);

T_VOID lcd_set_pixel( T_U16 x, T_U16 y, T_COLOR color)
{
    T_U8 *p;
    T_U16 lcd_w;
    T_U16 lcd_h;


   lcd_w=Fwl_GetLcdWidth();
   lcd_h=Fwl_GetLcdHeight();
 
#ifdef SUPPORT_TVOUT
	//if (TvOut_IsOpen())
	
	if(Fwl_TvoutIsOpen())
	{
		if ((x >= (lcd_w > 720 ? lcd_w : 720)) || (y >= (lcd_h > 288 ? lcd_h : 288)))
    	{
    		akprintf(C2, M_DRVSYS,  "set point out of TV, error!\n"); 
    		akprintf(C2, M_DRVSYS,  "x=%d,y=%d,widht=%d,height=%d\n",x,y,lcd_w,lcd_h); 
    		return;
    	}
	}
	else
#endif 
	{
	    if(x > lcd_w || y > lcd_h)
	    {
	    	akprintf(C2, M_DRVSYS,  "set point out of LCD, error!\n"); 
	    	akprintf(C2, M_DRVSYS,  "x=%d,y=%d,widht=%d,height=%d\n",x,y,lcd_w,lcd_h); 
	    	return;
	    }
	}
	p = lcd_get_disp_buf() + (y * lcd_w + x) * 3;

    *p++ = color >> 16;     // R
    *p++ = color >> 8;      // G
    *p++ = color;           // B
}


/**
 * @brief Set display buffer
 * Set display buffer with given content
 * @author qxj
 * @date 2005-07-18
 * @param T_U16 left, T_U16 top: the coordinate of the rect in LCD
 * @param T_U16 width, T_U16 height: the width and right of the content
 * @param const T_U8 *content: the content that will be set to display buffer
 * @return T_VOID
 */
T_VOID lcd_set_disp_buf(T_U16 left, T_U16 top, T_U16 width, T_U16 height, const T_U8 *content)
{
    T_U32 x, y, iIndex, k = 0;
    T_U8 *p, *q;
    T_U16 lcd_w;
    T_U16 lcd_h;

	T_U8 *bufDisplay;


    lcd_w=Fwl_GetLcdWidth();
    lcd_h=Fwl_GetLcdHeight();

	if(content == AK_NULL)
	{
    	akprintf(C2, M_DRVSYS,  "set content is NULL, error!\n"); 
        return;
    }

    if(left + width > lcd_w || top + height > lcd_h)
	{
		akprintf(C2, M_DRVSYS,  "set content out of lcd, error!\n"); 
    	return;
	}

	bufDisplay = lcd_get_disp_buf();

	iIndex = (top * lcd_w + left) * 3;
	k = lcd_w * 3;
	q = (T_U8 *)content;
	
    for( x=0; x<height ; x++, iIndex+=k )
	{
		// p = &gb_DisplayBuffer[0] + iIndex;
		p = bufDisplay + iIndex;
        for( y=0; y<width; y++ )
        {
            *p++ = *q++;
			*p++ = *q++;
			*p++ = *q++;
        }
	}
}

T_VOID lcd_set_disp_buf565(T_U16 left, T_U16 top, T_U16 width, T_U16 height, const T_U8 *content)
{
    T_U32 x, iIndex, k = 0;
	T_U8 *p;
    T_U32 lcd_buf_w;
    T_U32 lcd_buf_h;
    T_U8 *buf = AK_NULL;


	lcd_buf_w=Fwl_GetLcdWidth();
	lcd_buf_h=Fwl_GetLcdHeight();

	if(content == AK_NULL)
	{
    	akprintf(C2, M_DRVSYS,  "set content is NULL, error!\n"); 
        return;
    }

    if(left + width > lcd_buf_w || top + height > lcd_buf_h)
	{
		akprintf(C2, M_DRVSYS,  "set content out of lcd, error!\n"); 
    	return;
	}
	
    buf = lcd_get_disp_buf();
    iIndex = ( top * lcd_buf_w + left ) * 2;
    k = lcd_buf_w * 2;
    for( x=0; x<height; x++, iIndex+=k)
    {
        p = (T_U8 *)content + iIndex;
        memcpy(buf, p, width*2);
        buf += width*2;
    }  
}



T_VOID lcd_copy_disp_buf565(const T_U8 *content)
{
    T_U32 lcd_buf_w;
    T_U32 lcd_buf_h;
    T_U8 *buf = lcd_get_disp_buf();
    
	if((AK_NULL == content) || (AK_NULL == buf))
	{
    	akprintf(C2, M_DRVSYS,  "set content is NULL, error!\n"); 
        return;
    }
    
	lcd_buf_w = Fwl_GetLcdWidth();
	lcd_buf_h = Fwl_GetLcdHeight();
    memcpy(buf, content, lcd_buf_w*lcd_buf_h*2);
}


/**
 * @brief Get the address of the display buffer
 * @author qxj
 * @date 2005-07-18
 * @param T_eLCD lcd: selected LCD, must be DISPLAY_LCD_0 or LCD_1
 * @return T_U8 *: the address of the display buffer
 */
T_U8*  lcd_get_disp_buf(T_VOID)
{
	T_LAYER_INFO  stLayerInfo;
	
	Fwl_GetLayerInfo(HRGB_LAYER, &stLayerInfo);
	
	return stLayerInfo.pBufLayer;	 
}


T_VOID lcd_clean(T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_COLOR color)
{

	Fwl_FillRect(HRGB_LAYER,left,top,width,height,color);
}


/**
 * @brief: Copy RGB888 buffer to RGB565 buffer in the part of lcd buffer with format convertion
 * 
 *  更新RGBlcd的Gram,同时转换888 2 565,(Gram 就是565buffer)
 *  转换后的565数据存放在与起始位置
 * @author 
 * @modify date 2008-11-26 
 * @param T_U8 *pBuffer565: destation buffer, format is RGB565
 * @param T_U8 *pBuffer888: source buffer, format is RGB888
 * @param T_U16 left, T_U16 top, T_U16 width, T_U16 height: 
 * @return T_VOID
 */
T_VOID lcd_MPU_FlushGram(T_U8 *pBuffer565, const T_U8 *pBuffer888, T_U32 left,T_U32 top, T_U32 width, T_U32 height)
{
    T_U32 i,k;
    T_U32 x,y;
    T_U32 lcd_width;
    
    k = 0; // RGB565
    lcd_width=Fwl_GetLcdWidth();

    for (y=top; y<(top+height); y++ )
    {
        i = (y * lcd_width + left) * 3; // RGB888  
        
        for (x=0; x<width; x++, i+=3)
        {
            pBuffer565[k++] = ((pBuffer888[i + 1] & 0x1C) << 3) | ((pBuffer888[i + 2] & 0xF8) >> 3);       // low 8 bit, G and B
            pBuffer565[k++] = (pBuffer888[i] & 0xF8) | ((pBuffer888[i + 1] & 0xE0) >> 5);        // high 8 bit, R and G
        }
    }
}

