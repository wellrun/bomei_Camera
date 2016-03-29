/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: akwap_context.c
* Function: This file will constraint the access to the bottom layer display
            function, avoid resource competition. Also, this file os for
            porting to different OS
*
* Author: Zou Mai
* Date: 2001-04-14
* Version: 1.0
*
* Revision: 
* Author: 
* Date: 
***************************************************************************/
#include "Fwl_pfDisplay.h"
#include "Eng_Math.h"
#include "fwl_power.h"
#include "anyka_types.h"
#include "eng_debug.h"
#include "gpio_config.h"
#include "arch_lcd.h"
#include "hal_gpio.h"
#include "arch_gui.h"
#include "Eng_ImgConvert.h"
#include "Fwl_display.h"
#include "ImageLayer.h"
#include "Fwl_OsMalloc.h"

#ifndef OS_ANYKA
extern  lcd_refresh(DISPLAY_TYPE_DEV lcd, T_U16 left, T_U16 top, T_U16 width, T_U16 height);
#endif

#define GETABS(var) ((var)<0?(0-(var)):(var))

extern T_VOID lcd_set_pixel(T_U16 x, T_U16 y, T_COLOR color);
extern T_VOID lcd_clean(T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_COLOR color);
extern T_U8 *lcd_get_disp_buf(T_VOID);


/**
 * @brief  
 *
 * @author zeven zeng 
 * @date 2009-11-20
 * @param  layer
 * @param  T_COLOR color
 * @return T_VOID
 * @retval 
 */
T_VOID Fwl_FillSolid(HLAYER layer, T_COLOR color)
{
	T_LAYER_INFO  stLayerInfo;

	Fwl_GetLayerInfo(layer , &stLayerInfo);
	Fwl_FillRect(layer,0,0,stLayerInfo.stLayerRect.width,
		stLayerInfo.stLayerRect.height,color);
}

T_VOID Fwl_InvalidateRect(T_POS left, T_POS top, T_LEN width, T_LEN height)
{    
    Fwl_RefreshDisplay();
}

T_VOID Fwl_Invalidate(T_VOID)
{	
#ifdef OS_ANYKA
	Fwl_RefreshDisplay();
#else
    lcd_refresh(DISPLAY_LCD_0, 0, 0, Fwl_GetLcdWidth(), Fwl_GetLcdHeight());
#endif // OS_ANYKA
}

/**
 * @brief Draw a pixel
 *
 * @author ZouMai
 * @date 2001-4-20
  * @param[in] hLayer    handle of layer
 * @param T_POS x the point to be draw
 * @param  T_POS y the point to be draw
 * @param  T_COLOR color Display color
 * @return T_BOOL
 * @retval
 */
T_BOOL Fwl_SetPixel(HLAYER hLayer, T_POS x, T_POS y, T_COLOR color)
{
	T_POINT point;

	point.x = x;
	point.y = y;

	return	ImgLay_SetPixel( hLayer,  point,  color);
}

/**
 * @brief Get the color of the point
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_POS x x coordination of the point
 * @param  T_POS y Y coordination of the point
 * @return T_COLOR
 * @retval The color of this point
 */
T_COLOR Fwl_GetPixel(HLAYER hLayer, T_POS x, T_POS y)
{
	T_POINT point;
	
    AK_ASSERT_VAL((x>=0),"x cannot be less than 0\r\n",0);
    AK_ASSERT_VAL((y>=0),"y cannot be less than 0\r\n",0);

	point.x = x;
	point.y = y;
	
	return ImgLay_GetPixel(hLayer, point);
}

/**
 * @brief Fill  a rectangle with a specified color
 *
 * @author Baoli.Miao
 * @date 2001-4-20
 * @param T_POS left the left pointer of the rect.
 * @param T_POS top  the top pointer of the rect.
 * @param  T_LEN width rect width.
 * @param  T_LEN height rect height.
 * @param  T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_FillSolidRect(HLAYER hLayer, T_POS left, T_POS top, T_LEN width, T_LEN height, T_COLOR color)
{
#ifdef OS_WIN32
    T_POS    right = left + width;
    T_POS    bottom = top + height;

    T_POS i , j;
    if (right == 0 || bottom== 0)
           return;
	
    AK_ASSERT_VAL_VOID(left>=0, "Fwl_FillSolidRect():left");
    AK_ASSERT_VAL_VOID(top>=0, "Fwl_FillSolidRect():top");
    AK_ASSERT_VAL_VOID(width>=0, "Fwl_FillSolidRect():width");
    AK_ASSERT_VAL_VOID(height>=0, "Fwl_FillSolidRect():height");

    AK_ASSERT_VAL_VOID( (left + width) <= Fwl_GetLcdWidth(), "display buffer Length > Fwl_GetLcdWidth()\r\n" );
    AK_ASSERT_VAL_VOID( (top + height) <= Fwl_GetLcdHeight(), "display buffer Height >Fwl_GetLcdHeight()\r\n" );

    for (j=top; j<bottom; ++j)
    {
        for (i=left; i<right; ++i)
        {
            lcd_set_pixel(i, j, color);
        }
    }
	
    return;
#endif

#ifdef OS_ANYKA
    lcd_clean( left, top,width, height,  color);
#endif
}

/**
 * @brief Draw 3D rectangle
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_POS left Left to the rect.
 * @param  T_POS top Top to the rect.
 * @param  T_LEN width Rect width.
 * @param  T_LEN height Rect height.
 * @param  T_S8 flag Display mode.
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_Draw3DRect(HLAYER  hLayer, T_POS left, T_POS top, T_LEN width, T_LEN height, T_S8 flag)
{
    T_COLOR dark = RGB2AkColor(0, 0, 0, (T_U8)g_Graph.LCDCOLOR[DISPLAY_LCD_0]);
    T_COLOR bright = RGB2AkColor(255,255,255, (T_U8)g_Graph.LCDCOLOR[DISPLAY_LCD_0]);
	
    if (flag)        /* out */
    {
        Fwl_DrawLine(hLayer, left, top, (T_POS)(left+width-1), top, bright);
        Fwl_DrawLine(hLayer, left, (T_POS)(top+height-1), (T_POS)(left+width-1), (T_POS)(top+height-1), dark);
        Fwl_DrawLine(hLayer, left, top, left, (T_POS)(top+height-1), bright);
        Fwl_DrawLine(hLayer, (T_POS)(left+width-1), top, (T_POS)(left+width-1), (T_POS)(top+height-1), dark);
    }
    else
    {
        Fwl_DrawLine(hLayer, left, top, (T_POS)(left+width-1), top, dark);
        Fwl_DrawLine(hLayer, left, (T_POS)(top+height-1), (T_POS)(left+width-1), (T_POS)(top+height-1), bright);
        Fwl_DrawLine(hLayer, left, top, left, (T_POS)(top+height-1), dark);
        Fwl_DrawLine(hLayer, (T_POS)(left+width-1), top, (T_POS)(left+width-1), (T_POS)(top+height-1), bright);
    }

    return;
}

/**
 * @brief Fill a solid triangle.
 * dir: 0: U->D, 1: D->U, 2: L->R, 3: R->L
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_POS left 
 * @param T_POS top
 * @param  T_LEN width
 * @param  T_LEN height
 * @param  T_TRIANGLE_DIRECTION dir Draw direction
 * @param  T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_FillSolidTria(HLAYER hLayer, T_POS left,T_POS top, T_LEN width, T_LEN height, T_TRIANGLE_DIRECTION dir, T_COLOR color)
{
    T_S16   mid;
    T_S16   thick = 2;
    T_LEN   i;

    if (dir == 0 || dir == 1)         // up down
    {
        mid = left + width / 2 - 1;
		
        if (width % 2 == 1)
        {
            mid = left + width / 2;
            thick = 1;
        }
		
        if (dir == 1)
        {
            for (i = 0; i < height; i++)
                Fwl_FillSolidRect(hLayer, (T_POS)(mid - (width-thick)/2*(i+1)/height), (T_POS)(top + i),
                                  (T_LEN)((width-thick)/2*(i+1)/height*2+thick),     1, color);
        }
        else if (dir == 0)
        {
            for (i = 0; i < height; i++)
                Fwl_FillSolidRect(hLayer, (T_POS)(mid - (width-thick)/2*(i+1)/height), (T_POS)(top + height - i - 1),
                                  (T_LEN)((width-thick)/2*(i+1)/height*2+thick),     1, color);
        }
    }
    else if (dir == 2 || dir == 3)
    {
        mid = top + height / 2 - 1;
		
        if (height % 2 == 1)
        {
            mid = top + height / 2;
            thick = 1;
        }
		
        if (dir == 3)
        {
            for (i = 0; i < width; i++)
                Fwl_FillSolidRect(hLayer, (T_POS)(left + i), (T_POS)(mid - (height-thick)/2*(i+1)/width),
                                  1,      (T_LEN)((height-thick)/2*(i+1)/width*2+thick), color);
        }
        else if (dir == 2)
        {
            for (i = 0; i < width; i++)
                Fwl_FillSolidRect(hLayer, (T_POS)(left + width - i - 1), (T_POS)(mid - (height-thick)/2*(i+1)/width),
                                  1,      (T_LEN)((height-thick)/2*(i+1)/width*2+thick), color);
        }
    }

    return;
}


//(x, y) is the center of the circle. 
T_BOOL Fwl_DrawRadio(HLAYER layer, T_POS x, T_POS y, T_LEN radius, 
					 T_BOOL focus, T_COLOR color)
{
	T_POINT	point;

	point.x = x;
	point.y = y;

	return ImgLay_DrawRadio( layer,  point,  radius,  focus,  color);
}

T_S16 Fwl_DisplayBacklightOn(T_BOOL RadioTxCtrl)
{
#ifdef OS_WIN32
    return 0;
#else
    if (gs.LcdBrightness < 1 
		|| gs.LcdBrightness > LCD_BKL_BRIGHTNESS_MAX)
    {
        gs.LcdBrightness = LCD_BKL_BRIGHTNESS_MAX;
    }
	
    return Fwl_SetBrightness(DISPLAY_LCD_0, gs.LcdBrightness);
#endif
}

T_S16 Fwl_DisplayBacklightOff(T_VOID)
{
#ifdef OS_WIN32
    Fwl_FillSolidRect(HRGB_LAYER,0,0,Fwl_GetLcdWidth(),Fwl_GetLcdHeight(),0);
    return 0;
#else
    return Fwl_SetBrightness(DISPLAY_LCD_0, 0);
#endif
}

T_VOID Fwl_CleanScreen(T_COLOR color)
{
#ifdef OS_WIN32
    Fwl_FillSolidRect( HRGB_LAYER, 0, 0, Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), color );
#endif

#ifdef OS_ANYKA
    lcd_clean( 0, 0, Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), color);
#endif
}

T_U8 *Fwl_GetDispMemory(T_VOID)
{
#ifdef OS_WIN32
    extern unsigned char LCD0Memory[];

    return LCD0Memory;
#else
    return lcd_get_disp_buf();
#endif
}

T_U8 *Fwl_GetDispMemory565(T_VOID)
{
#ifdef OS_WIN32
    extern unsigned char LCD0Memory[];

    return LCD0Memory;
#else
    return lcd_get_disp_buf();
#endif
}


/*
    insert a yuv buf to a larger one!
    notes:
       yuv 4:2:2, the width had beeter be even number;
       yuv 4:2:0, both the width and height had beeter be even number.
       otherwise, the locations of the Ubuf and Vbuf be inserted will have a litter offset.
*/
T_VOID Fwl_InsertYUV2BckGrndYUV(const T_U8 *ybuf, const T_U8 *ubuf,const T_U8 *vbuf, T_S16 BckGrndWidth, T_S16 BckGrndHeight,
	T_U8 *YuvBuf, T_S16 left, T_S16 top, T_S16 width, T_S16 height, T_COLOR *bkColor, T_U8 trans)
{
    T_S16 i,j, rows ;
    T_U8 *yicon = YuvBuf;
    T_U8 *uicon = YuvBuf + width * height;
    T_U8 *vicon = YuvBuf + width * height * 5 / 4;
    T_U8 *y , *u, *v, *ydst,*udst,*vdst;
    T_POS yuvLeft;
    T_POS yuvTop;
    T_LEN yuvWidth;
    T_LEN yuvHeight;
    T_LEN imgWidth;
    T_LEN imgHeight;
    T_U8  tR = 0, tG = 0, tB = 0;
    T_U8  tY = 0, tU = 0, tV = 0;

#ifndef OS_WIN32
    if ((LCD_0_DEGREE == Fwl_GetLcdDegree()) || (LCD_180_DEGREE ==  Fwl_GetLcdDegree()))
    {
        yuvLeft     = left;
        yuvTop      = top;
        yuvWidth    = width;
        yuvHeight   = height;
        imgWidth    = BckGrndWidth;
        imgHeight   = BckGrndHeight;
    }
    else
#endif
    {
        yuvLeft     = left;
        yuvTop      = top;
        yuvWidth    = width ;
        yuvHeight   = height;
        imgWidth    = BckGrndWidth;
        imgHeight   = BckGrndHeight;
    }

    y = (T_U8 *)ybuf;
    u = (T_U8 *)ubuf;
    v = (T_U8 *)vbuf;

    if (bkColor != AK_NULL)
    {
    	AkColor2RGB( *bkColor, 24, &tR, &tG, &tB);

        tY = (77*tR + 150*tG + 29*tB)>>8;
        tU = ((-43*tR -85*tG + 128*tB)>>8) + 128;
        tV = ((128*tR - 107*tG - 21*tB)>>8) + 128;
    }

    for (rows = 0 ; rows < yuvHeight ; rows++ ) //loop for rows
    {
        T_U16   uvpos;

        uvpos = Eng_Get_UVBufPositon_YUV420((T_POS)(yuvTop + rows), (T_POS)yuvLeft, (T_LEN)imgWidth);

        ydst = y + imgWidth * (yuvTop + rows) + yuvLeft;
        udst = u + uvpos;
        vdst = v + uvpos;

        for (i = 0; i < (yuvWidth / 2); i++) //loop for a row
        {
            for (j=0; j<2; j++, ydst++, yicon++ )//count 0 to 1 for four Y
            {
                //This pixel isn't the transparent color
                //if( *yicon > 40 || *uicon<108 || *uicon>148 || *vicon<108 || *vicon>148 )
                if ((*yicon != tY))
                {
                    *ydst = (T_U8)(((T_U16)*ydst * (T_U16)trans + (T_U16)*yicon * (T_U16)(0xff - trans)) >> 8);
                }
            }

            if (((yuvTop + rows) % 2  == 0))
            {
                if ((*uicon != tU) || (*vicon != tV))
                {
                    *udst = (T_U8)(((T_U16)*udst * (T_U16)trans + (T_U16)*uicon * (T_U16)(0xff - trans)) >> 8);
                    *vdst = (T_U8)(((T_U16)*vdst * (T_U16)trans + (T_U16)*vicon * (T_U16)(0xff - trans)) >> 8);
                }

                uicon++;
                udst++;
                vicon++;
                vdst++;
            }

        }
    }
}


/**
 * @brief save parameter 'msgRect' specify display region
 *
 * @author hoube
 * @date 2012-4-20
 * @param  msgRect
 * @return address that  save display region data
 * @retval T_pDATA
 */
T_pDATA Fwl_SaveDisplayRegion(T_RECT msgRect)
{
	T_pDATA lcd_buffer = AK_NULL;
	T_pDATA pLcdBuf = AK_NULL;
	
	T_S16 x = 0;
	T_S16 y = 0;
    int iIndex = 0;
    int k = 0;
///1.	
#ifdef OS_ANYKA    
#ifdef LCD_MODE_565
	/**if default display buffer is RGB565*/
	AK_DEBUG_OUTPUT("MSGBUF:%d,w:%d,h:%d",msgRect.width*msgRect.height*2,msgRect.width,msgRect.height);
	lcd_buffer = (T_U8 *)Fwl_Malloc(msgRect.width*msgRect.height*2);
	if (lcd_buffer != AK_NULL)
	{
		pLcdBuf = Fwl_GetDispMemory565();
		for(x = msgRect.top; x < msgRect.top + msgRect.height; x++)
		{
			iIndex = (x * Fwl_GetLcdWidth() + msgRect.left) * 2;
			for(y = msgRect.left; y < msgRect.left + msgRect.width; y++)
			{
				lcd_buffer[k++] = pLcdBuf[iIndex++];
				lcd_buffer[k++] = pLcdBuf[iIndex++];
			}
		}
	}
#elif LCD_MODE_888
	/**if default display buffer is RGB888*/
	lcd_buffer = (T_U8 *)Fwl_Malloc(msgRect.width*msgRect.height*3);
	if (lcd_buffer != AK_NULL)
	{
		pLcdBuf = Fwl_GetDispMemory();
		for(x = msgRect.top; x < msgRect.top + msgRect.height; x++)
		{
			iIndex = (x * Fwl_GetLcdWidth() + msgRect.left) * 3;
			for(y = msgRect.left; y < msgRect.left + msgRect.width; y++)
			{
				lcd_buffer[k++] = pLcdBuf[iIndex++];
				lcd_buffer[k++] = pLcdBuf[iIndex++];
				lcd_buffer[k++] = pLcdBuf[iIndex++];
			}
		}
	}
#else
	error "not add current suspensory color space code!!!"
#endif  
#endif

#ifdef OS_WIN32
    lcd_buffer = (T_U8 *)Fwl_Malloc(msgRect.width*msgRect.height*3);
    if (lcd_buffer != AK_NULL)
    {
        pLcdBuf = Fwl_GetDispMemory();
        for(x = msgRect.top; x < msgRect.top + msgRect.height; x++)
        {
            iIndex = (x * Fwl_GetLcdWidth() + msgRect.left) * 3;
            for(y = msgRect.left; y < msgRect.left + msgRect.width; y++)
            {
                lcd_buffer[k++] = pLcdBuf[iIndex++];
                lcd_buffer[k++] = pLcdBuf[iIndex++];
                lcd_buffer[k++] = pLcdBuf[iIndex++];
            }
        }
    }
#endif

	return lcd_buffer;
}

/**
 * @brief restore display region that call function Fwl_SaveDisplayRegion to save
 *
 * @author hoube
 * @date 2012-4-20
 * @param  lcd_buffer
 * @param  msgRect
 * @retval T_pDATA
 */
T_pDATA Fwl_RestoreDisplayRegion(T_pDATA lcd_buffer, T_RECT msgRect)
{
	T_pDATA pLcdBuf = AK_NULL;
	T_S16 x, y;
    int iIndex = 0;
    int k = 0;

#ifdef OS_ANYKA
#ifdef LCD_MODE_565
	/**if default display buffer is RGB565*/
	if (lcd_buffer != AK_NULL)
	{	///1.
		pLcdBuf = Fwl_GetDispMemory565();
		for(x = msgRect.top; x < msgRect.top + msgRect.height; x++)
		{
			iIndex = (x * Fwl_GetLcdWidth() + msgRect.left) * 2;
			for(y = msgRect.left; y < msgRect.left + msgRect.width; y++)
			{
				pLcdBuf[iIndex++] = lcd_buffer[k++];
				pLcdBuf[iIndex++] = lcd_buffer[k++];
			}
		}
	
		lcd_buffer = Fwl_Free(lcd_buffer);
		///2.
		//Fwl_RefreshDisplay();
	}
#elif LCD_MODE_888     
	/**if default display buffer is RGB888*/
	if (lcd_buffer != AK_NULL)
	{	///1.
		pLcdBuf = Fwl_GetDispMemory();
		for(x = msgRect.top; x < msgRect.top + msgRect.height; x++)
		{
			iIndex = (x * Fwl_GetLcdWidth() + msgRect.left) * 3;
			for(y = msgRect.left; y < msgRect.left + msgRect.width; y++)
			{
				pLcdBuf[iIndex++] = lcd_buffer[k++];
				pLcdBuf[iIndex++] = lcd_buffer[k++];
				pLcdBuf[iIndex++] = lcd_buffer[k++];
			}
		}
		lcd_buffer = Fwl_Free(lcd_buffer);
		///2.
		//Fwl_RefreshDisplay();
	}
#else
	error "not add current suspensory color space code!!!"
#endif        
#endif

#ifdef OS_WIN32
	if (lcd_buffer != AK_NULL)
	{
		pLcdBuf = Fwl_GetDispMemory();
		for(x = msgRect.top; x < msgRect.top + msgRect.height; x++)
		{
			iIndex = (x * Fwl_GetLcdWidth() + msgRect.left) * 3;
			for(y = msgRect.left; y < msgRect.left + msgRect.width; y++)
			{
				pLcdBuf[iIndex++] = lcd_buffer[k++];
				pLcdBuf[iIndex++] = lcd_buffer[k++];
				pLcdBuf[iIndex++] = lcd_buffer[k++];
			}
		}
		lcd_buffer = Fwl_Free(lcd_buffer);
	}
#endif

	return lcd_buffer;
}
 

#if 0
T_VOID Fwl_DrawLineOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight,
                                T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color)
{
    T_POS i;
    T_POS left, top;
    T_LEN width, height;

    if( x1 > x2 )
    {
        left = x2;
        width = x1 - x2 + 1;
    }
    else
    {
        left = x1;
        width = x2 - x1 + 1;
    }

    if( y1 > y2 )
    {
        top = y2;
        height = y1 - y2 + 1;
    }
    else
    {
        top = y1;
        height = y2 - y1 + 1;
    }

    AK_ASSERT_VAL_VOID( (left + width - 1) <= (T_LEN)imgwidth, "display buffer overflow: drawline\r\n" );
    AK_ASSERT_VAL_VOID( (top + height - 1) <= (T_LEN)imgheight, "display buffer overflow: drawline\r\n" );

    /*Alter by liu weijun, 2006-7-12*/
    if (width>height)
    {
        for(i=0;i<width;i++)
        {
            Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_U16)(left+i), (T_U16)(top+i*height/width), color);
        }
    }
    else
    {
        for(i=0;i<height;i++)
        {
            Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_U16)(left+i*width/height), (T_U16)(top+i), color);
        }
    }
    /*end of the alter*/

    return;
}


T_VOID Fwl_DrawRectOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight,
                         T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color)
{
    Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x1, y1, x2, y1, color);
    Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x1, y1, x1, y2, color);
    Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x2, y2, x2, y1, color);
    Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x2, y2, x1, y2, color); 
}

T_VOID Fwl_DrawCircleOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight,
                            T_POS x, T_POS y, T_LEN radius, T_COLOR color)
{
    T_S16   i, x0, y0, radius2, i2;
    T_S32   curdelta, nextdelta;

    radius2 = radius * radius;
    x0 = radius * 14142 / 20000;
    y0 = radius * 14142 / 20000;

    for (i = x0; i >= 0; i--)
    {
        i2 = i*i;
        curdelta = Fwl_Abs(i2+y0*y0-radius2);
        do
        {
            y0++;
            nextdelta = Fwl_Abs(i2+y0*y0-radius2);
        }
        while (nextdelta <= curdelta);
        y0--;
        Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_POS)(x - i), (T_POS)(y - y0), color);
        Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_POS)(x + i), (T_POS)(y - y0), color);
        Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_POS)(x - i), (T_POS)(y + y0), color);
        Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_POS)(x + i), (T_POS)(y + y0), color);

        Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_POS)(x - y0), (T_POS)(y - i), color);
        Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_POS)(x + y0), (T_POS)(y - i), color);
        Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_POS)(x - y0), (T_POS)(y + i), color);
        Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_POS)(x + y0), (T_POS)(y + i), color);
    }

    return;
}


T_VOID Fwl_DrawDiskOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, T_POS center_x, T_POS center_y, T_POS radii, T_COLOR color)
{
    T_POS i, j;
    T_POS left, top;

    AK_ASSERT_VAL_VOID((center_x > radii) && ((center_x + radii) < (T_LEN)imgwidth) , "display buffer overflow: draw_disk\r\n" );
    AK_ASSERT_VAL_VOID((center_y > radii) && ((center_y + radii) < (T_LEN)imgheight), "display buffer overflow: draw_disk\r\n" );

    left = center_x - radii;
    top = center_y - radii;

    for(i = 0; i < (radii*2); i++)
        for (j = 0; j < (radii*2); j++)
            if (PointInDisk(center_x, center_y, radii, (T_POS)(left + i), (T_POS)(top + j)) == AK_TRUE)
                Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_U16)(left + i), (T_U16)(top + j), color);

    return;
}

T_VOID Fwl_FillSolidTriaOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight,
                              T_POS left,T_POS top, T_LEN width, T_LEN height, T_TRIANGLE_DIRECTION dir, T_COLOR color)
{
    T_S16   mid;
    T_S16   thick = 2;
    T_LEN   i;

    if (dir == 0 || dir == 1)         // up down
    {
        mid = left + width / 2 - 1;
        if (width % 2 == 1)
        {
            mid = left + width / 2;
            thick = 1;
        }
        if (dir == 1)
            for (i = 0; i < height; i++)
                Fwl_FillSolidRectOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight,
                                       (T_POS)(mid - (width-thick)/2*(i+1)/height), (T_POS)(top + i),
                                       (T_LEN)((width-thick)/2*(i+1)/height*2+thick), 1, color);
        else if (dir == 0)
            for (i = 0; i < height; i++)
                Fwl_FillSolidRectOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight,
                                       (T_POS)(mid - (width-thick)/2*(i+1)/height), (T_POS)(top + height - i - 1),
                                       (T_LEN)((width-thick)/2*(i+1)/height*2+thick), 1, color);
    }
    else if (dir == 2 || dir == 3)
    {
        mid = top + height / 2 - 1;
        if (height % 2 == 1)
        {
            mid = top + height / 2;
            thick = 1;
        }
        if (dir == 3)
            for (i = 0; i < width; i++)
                Fwl_FillSolidRectOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight,
                                       (T_POS)(left + i), (T_POS)(mid - (height-thick)/2*(i+1)/width),
                                       1, (T_LEN)((height-thick)/2*(i+1)/width*2+thick), color);
        else if (dir == 2)
            for (i = 0; i < width; i++)
                Fwl_FillSolidRectOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight,
                                       (T_POS)(left + width - i - 1), (T_POS)(mid - (height-thick)/2*(i+1)/width),
                                       1, (T_LEN)((height-thick)/2*(i+1)/width*2+thick), color);
    }

    return;
}

/**
 * @brief Draw ellipse.
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_POS x  X coordination point
 * @param  T_POS y Y coordination point
 * @param  T_LEN radiusx X radius
 * @param  T_LEN radiusy Y radius.
 * @param  T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawEllipse(HLAYER hLayer, T_POS x, T_POS y, T_LEN radiusx, T_LEN radiusy, T_COLOR color)
{
    T_S16 dx,dy,f,fx,fy,xtemp,ytemp,j,mx,my,lx,ly,r;
	
    r=radiusx;
    if (radiusx<radiusy) r=radiusy;
    if ((radiusx==0)&&(radiusy==0)) return;
    lx=0; ly=0;
    j=8*r-1;
    dx=-1; dy=1;
    xtemp=r; ytemp=0;
    f=0; fx=2*xtemp*dx+1; fy=1;
    mx=0; my=0;
    lcd_set_pixel( (T_POS)(y-ytemp), (T_POS)(x+xtemp), color);
	
    while (j!=0)
      { if (f>=0)
          { lx=lx+radiusx;
            if (lx>=r)
              { if (mx!=0)
                  { lcd_set_pixel( (T_POS)(y-ytemp), (T_POS)(x+xtemp),color);
                    mx=0;
                  }
                xtemp=xtemp+dx; lx=lx-r;
                if (my!=0)
                  { my=0;
                    lcd_set_pixel( (T_POS)(y-ytemp),(T_POS)(x+xtemp),color);
                  }
                else mx=1;
              }
            if (fx>0) f=f-fx;
            else f=f+fx;
            fx=fx+2;
            if ((fx==0)||((fx<0)&&(fx-2>0))||
                         ((fx>0)&&(fx-2<0)))
              { dy=-dy; fy=-fy+2; f=-f;}
          }
        else
          { ly=ly+radiusy;
            if (ly>=r)
              { if (my!=0)
                  { lcd_set_pixel( (T_POS)(y-ytemp),(T_POS)(x+xtemp),color);
                    my=0;
                  }
                ytemp=ytemp+dy; ly=ly-r;
                if (mx!=0)
                  { mx=0;
                    lcd_set_pixel( (T_POS)(y-ytemp),(T_POS)(x+xtemp),color);
                  }
                else my=1;
              }
            if (fy>0) f=f+fy;
            else f=f-fy;
            fy=fy+2;
            if ((fy==0)||((fy<0)&&(fy-2>0))||
                         ((fy>0)&&(fy-2<0)))
              { dx=-dx; fx=-fx+2; f=-f;}
          }
        j=j-1;
      }
    if ((mx!=0)||(my!=0))
      lcd_set_pixel( (T_POS)(y-ytemp),(T_POS)(x+xtemp),color);

    return;
}

T_VOID Fwl_DrawDisk(HLAYER hLayer, T_POS center_x, T_POS center_y, T_POS radii, T_COLOR color)
{
    T_POS i, j;
    T_POS left, top;

    AK_ASSERT_VAL_VOID((center_x > radii) && ((center_x + radii) < Fwl_GetLcdWidth()) , "display buffer overflow: draw_disk\r\n" );
    AK_ASSERT_VAL_VOID((center_y > radii) && ((center_y + radii) < Fwl_GetLcdHeight()), "display buffer overflow: draw_disk\r\n" );

    left = center_x - radii;
    top = center_y - radii;

    for(i = 0; i < (radii*2); i++)
        for (j = 0; j < (radii*2); j++)
            if (PointInDisk(center_x, center_y, radii, (T_POS)(left + i), (T_POS)(top + j)) == AK_TRUE)
                lcd_set_pixel( (T_U16)(left + i), (T_U16)(top + j), color);

    return;
}


/**
 * @brief Draw a broken rect according to the 'chr' value.
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_POS left The left point of the rect.
 * @param  T_POS top The top point of the rect.
 * @param  T_LEN width Rect width.
 * @param  T_LEN height Rect height
 * @param  T_COLOR color Display color
 * @param  const T_U8 chr Char code.
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawRectByChar(HLAYER hLayer, T_POS left, T_POS top, T_LEN width, T_LEN height, T_COLOR color, T_U8 chr)
{
    Fwl_DrawLineByChar(hLayer, left, top, (T_POS)(left+width-1), top, color, chr);
    Fwl_DrawLineByChar(hLayer, left, (T_POS)(top+height-1), (T_POS)(left+width-1), (T_POS)(top+height-1), color, chr);
    Fwl_DrawLineByChar(hLayer, left, top, left, (T_POS)(top+height-1), color, chr);
    Fwl_DrawLineByChar(hLayer, (T_POS)(left+width-1), top, (T_POS)(left+width-1), (T_POS)(top+height-1), color, chr);
    return;
}


/**
 * @brief Draw a broken line according to the 'chr' value
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_POS x1  X coordination of start point.
 * @param  T_POS y1 Y coordination of start point.
 * @param  T_POS x2 X coordination of end point.
 * @param  T_POS y2 Y coordination of end point.
 * @param  T_COLOR color Display color
 * @param  const T_U8 chr char code.
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawLineByChar(HLAYER  hLayer, T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color, T_U8 chr)
{
    T_S16 i;

    if( x1 == x2 )
    {
        for( i=y1; i<=y2; i++ )
        {
            if (((chr >> (i % 8)) & 0x01) == 1)
                lcd_set_pixel(  x1, i, color);
        }
    }
    else if (y1 == y2)
    {
        for( i=x1; i<=x2; i++ )
        {
            if (((chr >> (i % 8)) & 0x01) == 1)
                lcd_set_pixel( i, y1, color);
        }
    }
    return;
}

/**
 * @brief Draw frame border rect
 *
 * @author LiaoJianhua
 * @date 2005-10-12
 * @param T_RECT position of the frame rect
 * @param T_U8 flag:user can use flag to identify the frame line: FRAME_LEFT -- FRAME_BOTTOM
 * @param T_LEN borderWidth:frame border width
 * @param T_COLOR color:frame border color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_FrameRect(HLAYER hLayer,
                     const T_RECT *rect,
                     T_U8 flag,
                     T_LEN borderWidth,
                     T_COLOR color)
{
    T_POS    i;

    for(i=0; i<borderWidth; ++i)
    {
        if (flag & FRAME_LEFT)    /* draw dialog frame: top line */
        {
            Fwl_DrawLine(hLayer, (T_POS)(rect->left+i), (T_POS)(rect->top+i), (T_POS)(rect->left+rect->width-1-i), (T_POS)(rect->top+i), color);
        }
        if (flag & FRAME_RIGHT)    /* draw dialog frame: left line */
        {
            Fwl_DrawLine(hLayer, (T_POS)(rect->left+i), (T_POS)(rect->top+i), (T_POS)(rect->left+i), (T_POS)(rect->top+rect->height-1-i), color);
        }
        if (flag & FRAME_TOP)    /* draw dialog frame: right line */
        {
            Fwl_DrawLine(hLayer, (T_POS)(rect->left+rect->width-1-i), (T_POS)(rect->top+i), (T_POS)(rect->left+rect->width-1-i), (T_POS)(rect->top+rect->height-1-i), color);
        }
        if (flag & FRAME_BOTTOM)    /* draw dialog frame: bottom line */
        {
            Fwl_DrawLine(hLayer, (T_POS)(rect->left+i), (T_POS)(rect->top+rect->height-1-i), (T_POS)(rect->left+rect->width-1-i), (T_POS)(rect->top+rect->height-1-i), color);
        }
    }
}


/**
 * @brief Draw a circle.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_POS x  X coordination point of the center point
 * @param  T_POS y Y coordination point of the center point
 * @param  T_LEN radius Circle radius.
 * @param  T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawCircle(HLAYER  hLayer, T_POS x, T_POS y, T_LEN radius, T_COLOR color)
{
    T_S16   i, x0, y0, radius2, i2;
    T_S32   curdelta, nextdelta;

    radius2 = radius * radius;
    x0 = radius * 14142 / 20000;
    y0 = radius * 14142 / 20000;

    for (i = x0; i >= 0; i--)
    {
        i2 = i*i;
        curdelta = Fwl_Abs(i2+y0*y0-radius2);
        do
        {
            y0++;
            nextdelta = Fwl_Abs(i2+y0*y0-radius2);
        }
        while (nextdelta <= curdelta);
        y0--;
        lcd_set_pixel( (T_POS)(x - i), (T_POS)(y - y0), color);
        lcd_set_pixel( (T_POS)(x + i), (T_POS)(y - y0), color);
        lcd_set_pixel( (T_POS)(x - i), (T_POS)(y + y0), color);
        lcd_set_pixel( (T_POS)(x + i), (T_POS)(y + y0), color);

        lcd_set_pixel( (T_POS)(x - y0), (T_POS)(y - i), color);
        lcd_set_pixel( (T_POS)(x + y0), (T_POS)(y - i), color);
        lcd_set_pixel( (T_POS)(x - y0), (T_POS)(y + i), color);
        lcd_set_pixel( (T_POS)(x + y0), (T_POS)(y + i), color);
    }

    return;
}

#endif

 

