/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: fwl_graphic.c
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

#include "fwl_graphic.h"
#include "eng_debug.h"
#include "eng_imgconvert.h"
#include "eng_math.h"
#include "arch_lcd.h"
#include "fwl_osmalloc.h"
#include "arch_gui.h"
#include "imagelayer.h"
#include "Eng_ScaleConvertSoft.h"
#include "Lib_state.h"
#include "Fwl_gui.h"
#include "Log_MediaPlayer.h"
#include "Eng_DynamicFont.h"
#include "arch_mmu.h"



#define GETABS(var) ((var)<0?(0-(var)):(var))
#define MAX_2D_DIM  	3

extern T_VOID lcd_set_pixel(T_U16 x, T_U16 y, T_COLOR color);

T_VOID Fwl_Reset2DGraphic(T_VOID)
{
#ifdef INSTALL_GAME_GBA 
	Reset_2DGraphic();	
#endif
}

/**
 * @brief Get a pixel on RGB
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x X coordination of the point
 * @param T_POS y Y coordination of the point
 * @param T_U8 colortype:RGB888 or RGB565
 * @return T_COLOR
 * @retval
 */
T_COLOR Fwl_GetPixelOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_POS x, T_POS y, T_U8 colortype )
{
	T_U8 *p = AK_NULL;
    T_COLOR color = 0;
	T_U8 r = 0;
	T_U8 g = 0;
	T_U8 b = 0;
	T_U16 temp = 0;

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_GetPixelOnRGB AK_NULL == buf!");
		return color;
	}

    if(x > (T_LEN)imgwidth || y > (T_LEN)imgheight)
    {
    	Fwl_Print(C3, M_DISPLAY, "get point out of range, error!"); 
    	return color;
    }

	if (RGB888 == colortype)
	{
	    p = buf + (y * imgwidth + x) * 3;

	    color = *p++;
	    color <<= 8;
	    color |= *p++;
	    color <<= 8;
	    color |= *p;
	}
	else if (RGB565 == colortype)
	{
		p = buf + (y * imgwidth + x) * 2;
   
    	temp = (*p) | (*(p+1)<<8);
		r = (T_U8)((temp>>11)<<3);
		g = (T_U8)((temp>>5)<<2);
		b = (T_U8)(temp<<3);
		color = (r<<16) | (g<<8) | b;
	}
    
	return color;
}

/**
 * @brief Draw a pixel on RGB
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x the point to be draw
 * @param T_POS y the point to be draw
 * @param T_COLOR color Display color
 * @param T_U8 colortype:RGB888 or RGB565
 * @return T_BOOL
 * @retval
 */
T_BOOL Fwl_SetPixelOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_POS x, T_POS y, 
								T_COLOR color, T_U8 colortype )
{
	T_U8 *p = AK_NULL;
	T_U8 r = 0;
	T_U8 g = 0;
	T_U8 b = 0;
	
#ifdef OS_ANYKA

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_SetPixelOnRGB AK_NULL == buf!");
		return AK_FALSE;
	}

    if(x > (T_LEN)imgwidth || y > (T_LEN)imgheight)
    {
    	Fwl_Print(C3, M_DISPLAY, "set point out of LCD, error!"); 
    	Fwl_Print(C3, M_DISPLAY, "x = %d, y = %d, widht = %d, heigh = %d",x, y, imgwidth, imgheight); 
    	return AK_FALSE;
    }

	if (RGB888 == colortype)
	{
		p = buf + (y * imgwidth + x) * 3;

	    *p++ = (T_U8)(color >> 16);		// R
	    *p++ = (T_U8)(color >> 8);      // G
	    *p++ = (T_U8)color;				// B
	}
	else if (RGB565 == colortype)
	{
		p = buf + (y * imgwidth + x) * 2;

	    r = (T_U8)(color >> 16);		// R
	    g = (T_U8)(color >> 8);			// G
	    b = (T_U8)color;				// B
	    
	    *p++ = ((b & 0xf8) >> 3) | ((g & 0x1c) << 3);	// b, g
	    *p++ = (r & 0xf8)  | ((g & 0xe0) >> 5);			// r, g  
	}
	else
	{
		return AK_FALSE;
	}
#else
	if (((T_pImgLay)Fwl_hRGBLayer)->pData == buf)
	{
		lcd_set_pixel(x, y, color);
	}
	else
	{
		if (AK_NULL == buf)
		{
			Fwl_Print(C3, M_DISPLAY, "Fwl_SetPixelOnRGB AK_NULL == buf!");
			return AK_FALSE;
		}

	    if(x > (T_LEN)imgwidth || y > (T_LEN)imgheight)
	    {
	    	Fwl_Print(C3, M_DISPLAY, "set point out of LCD, error!"); 
	    	Fwl_Print(C3, M_DISPLAY, "x = %d, y = %d, widht = %d, heigh = %d",x, y, imgwidth, imgheight); 
	    	return AK_FALSE;
	    }

		p = buf + (y * imgwidth + x) * 3;

	    *p++ = (T_U8)(color >> 16);		// R
	    *p++ = (T_U8)(color >> 8);      // G
	    *p++ = (T_U8)color;				// B

	}
	
#endif
	return AK_TRUE;
}


/**
 * @brief Draw a pixel on YUV
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width, must be even number
 * @param T_U32 imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param T_POS pos_x the point to be draw
 * @param T_POS pos_y the point to be draw
 * @param T_COLOR color Display color
 * @return T_VOID
 * @retval
 */
T_VOID Fwl_SetPixelOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
								T_U16 pos_x, T_U16 pos_y, T_COLOR color)
{
	T_U8 R = 0;
	T_U8 G = 0;
	T_U8 B = 0;
	T_U8 Y = 0;
	T_U8 U = 0;
	T_U8 V = 0;
    T_U16 x = pos_x;
	T_U16 y = pos_y;
    T_U32 uvpos;

    AK_ASSERT_VAL_VOID(((imgwidth%2 == 0))&&((imgheight%2 == 0)), \
                       "Fwl_SetPixelOnYUV: imgwidth or imgheight is not even number!");

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_SetPixelOnYUV AK_NULL == buf!");
		return;
	}

    if(x >= imgwidth|| y >= imgheight)
    {
        Fwl_Print(C3, M_DISPLAY, "Fwl_SetPixelOnYUV pos x or y too large");
        return;
    }

    B = (T_U8)color;         // B
    G = (T_U8)(color >> 8);  // G
    R = (T_U8)(color >> 16); // R

    Y = (77*R + 150*G + 29*B)>>8;
    U = ((-43*R -85*G + 128*B)>>8) + 128;
    V = ((128*R - 107*G - 21*B)>>8) + 128;


#ifdef OS_ANYKA
		//因为LCD 旋转了，所以x,y 的坐标要换一下
    x = pos_y;
    y = pos_x;
#endif

    
    ybuf[imgwidth * x + y] = Y;

    //four points that shared a U & V is a 2x2 matrix.
    uvpos = Eng_Get_UVBufPositon_YUV420((T_POS)x, (T_POS)y, (T_LEN)imgwidth);

    ubuf[uvpos] = U;
    vbuf[uvpos] = V;

}



/**
 * @brief Draw a line on RGB.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x1 X coordination of start point.
 * @param T_POS y1 Y coordination of start point.
 * @param T_POS x2 X coordination of end point.
 * @param T_POS y2 Y coordination of end point.
 * @param T_COLOR color Display color
 * @param T_U8 colortype:RGB888 or RGB565
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawLineOnRGB(T_U8* buf, T_U32 imgwidth, T_U32 imgheight ,T_POS x1, T_POS y1, 
								T_POS x2, T_POS y2, T_COLOR color, T_U8 colortype)
{
	T_POS x = 0;
    T_POS y = 0;
    T_POS p = 0;
    T_POS n = 0;
    T_POS tn = 0;

    const T_U16 lcdTop = 0;
    const T_U16 lcdLeft = 0;
	T_U16 lcdBottom = 0;
    T_U16 lcdRight = 0;

	lcdBottom = (T_U16)(imgheight - 1); 
    lcdRight = (T_U16)(imgwidth - 1);

    if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawLineOnRGB AK_NULL == buf!");
		return;
	}
	
    if(x1==x2 && y1==y2)
    {
        Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, x1, y1, color, colortype);
        return;
    }

    if(y1 == y2)
    {
        //The horizontal line exceed the bottom of lcdrect
        if(y1>lcdBottom || y1<lcdTop)
        { 
            return;
        }
        if(x1 > x2)
        {
            x=x2;x2=x1;x1=x;
        }
        if(x1 < lcdLeft)
        {
            x1 = lcdLeft;
        }
        if(x2 > lcdRight)
        {
            x2 = lcdRight;
        }
        if(x1 == x2)
        {
            Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, x1, y1, color, colortype);
        }
        else
        {
            for(x=x1; x<=x2; x++)
            {
                Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, x, y1, color, colortype);
            }
        }
        return;
    }

    if(x1 == x2)
    {
        if(x1>lcdRight || x2<lcdLeft)
        {
            return;
        }
        if(y1 > y2)
        {
            y=y2;y2=y1;y1=y;
        }
        if(y1 < lcdTop)
        {
            y1 = lcdTop;
        }
        if(y2 > lcdBottom)
        {
            y2 = lcdBottom;
        }
        if(y1 == y2)
        {
            Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, x1, y1, color, colortype);
        }
        else
        {
            for(y=y1; y<=y2; y++)
            {
                Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, x1, y, color, colortype);
            }
        }
        return;
    }

    if(GETABS(y2-y1) <= GETABS(x2-x1))
    {
        if((y2<y1&&x2<x1) || (y1<=y2&&x1>x2))
        {
            x=x2;y=y2;x2=x1;y2=y1;x1=x;y1=y;
        }
        if(y2>=y1 && x2>=x1)
        {
            x=x2-x1; y=y2-y1;
            p=2*y; n=2*x-2*y; tn=x;
            while(x1<=x2)
            {
                if(tn>=0)
                {
                    tn-=p;
                }
                else
                {
                    tn+=n;
                    y1++;
                }
                if(    x1>=lcdLeft 
                    && x1<=lcdRight
                    && y1>=lcdTop
                    && y1<=lcdBottom)
                {
                    Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, x1, y1, color, colortype);
                }
                ++x1;
            }
        }
        else
        {
            x=x2-x1;y=y2-y1;
            p=-2*y;n=2*x+2*y;tn=x;
            while(x1<=x2)
            {
                if(tn>=0)
                {
                    tn-=p;
                }
                else
                {
                    tn+=n;
                    y1--;
                }
                if(    x1>=lcdLeft 
                    && x1<=lcdRight
                    && y1>=lcdTop
                    && y1<=lcdBottom)
                {
                    Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, x1, y1, color, colortype);
                }
                ++x1;
            }
        }
    }
    else
    {
        x=x1;x1=y2;y2=x;y=y1;y1=x2;x2=y;
        if((y2<y1&&x2<x1) || (y1<=y2&&x1>x2))
        {
            x=x2;y=y2;x2=x1;y2=y1;x1=x;y1=y;
        }
        if(y2>=y1 && x2>=x1)
        {
            x=x2-x1;y=y2-y1;p=2*y;n=2*x-2*y;tn=x;
            while(x1 <= x2)
            {
                if(tn>=0)
                {
                    tn-=p;
                }
                else
                {
                    tn+=n;
                    y1++;
                }
                if(    y1>=lcdLeft 
                    && y1<=lcdRight
                    && x1>=lcdTop
                    && x1<=lcdBottom)
                {
                    Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, y1, x1, color, colortype);
                }
                ++x1;
            }
        }
        else
        {
            x=x2-x1;y=y2-y1;p=-2*y;n=2*x+2*y;tn=x;
            while(x1 <= x2)
            {
                if(tn>=0)
                {
                    tn-=p;
                }
                else
                {
                    tn+=n;
                    y1--;
                }
                if(    y1>=lcdLeft 
                    && y1<=lcdRight
                    && x1>=lcdTop
                    && x1<=lcdBottom)
                {
                    Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, y1, x1, color, colortype);
                }
                ++x1;
            }
        }
    }
}



/**
 * @brief Draw a line on YUV.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width, must be even number
 * @param T_U32 imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param T_POS x1 X coordination of start point.
 * @param T_POS y1 Y coordination of start point.
 * @param T_POS x2 X coordination of end point.
 * @param T_POS y2 Y coordination of end point.
 * @param T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawLineOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
								T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color)
{
	T_POS i 		= 0;
    T_POS left 		= 0;
	T_POS top 		= 0;
    T_LEN width 	= 0;
	T_LEN height 	= 0;

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawLineOnYUV AK_NULL == buf!");
		return;
	}

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
            Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_U16)(left+i), 
				(T_U16)(top+i*height/width), color);
        }
    }
    else
    {
        for(i=0;i<height;i++)
        {
            Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_U16)(left+i*width/height), 
				(T_U16)(top+i), color);
        }
    }
    /*end of the alter*/

    return;
}


/**
 * @brief Draw a rectange on RGB
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_RECT *rect
 * @param T_COLOR color Display color
 * @param T_U8 colortype:RGB888 or RGB565
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawRectOnRGB(T_U8* buf, T_U32 imgwidth, T_U32 imgheight ,T_RECT *rect, 
								T_COLOR color, T_U8 colortype)
{
	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawRectOnRGB AK_NULL == buf!");
		return;
	}

	if (AK_NULL == rect)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawRectOnRGB AK_NULL == rect!");
		return;
	}
	
	Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, rect->left, rect->top, (T_POS)(rect->left+rect->width-1), 
		rect->top, color, colortype);
	
    Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, rect->left, (T_POS)(rect->top+rect->height-1), 
		(T_POS)(rect->left+rect->width-1), (T_POS)(rect->top+rect->height-1), color, colortype);
	
    Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, rect->left, rect->top, rect->left, 
		(T_POS)(rect->top+rect->height-1), color, colortype);
	
    Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, (T_POS)(rect->left+rect->width-1), 
		rect->top, (T_POS)(rect->left+rect->width-1), (T_POS)(rect->top+rect->height-1), color, colortype);

    return;
}


/**
 * @brief Draw a rectange on YUV
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width, must be even number
 * @param T_U32 imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param T_POS left the left point of the rectangle
 * @param T_POS top the top point of the rectangle
 * @param T_LEN width rect width
 * @param T_LEN height Rect height.
 * @param T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawRectOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
								T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color)
{
	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawRectOnYUV AK_NULL == buf!");
		return;
	}
	
	Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x1, y1, x2, y1, color);
    Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x1, y1, x1, y2, color);
    Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x2, y2, x2, y1, color);
    Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x2, y2, x1, y2, color); 
}


/**
 * @brief Draw a circle on RGB.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x X coordination point of the center point
 * @param T_POS y Y coordination point of the center point
 * @param T_LEN radius Circle radius.
 * @param T_COLOR color Display color
 * @param T_U8 colortype:RGB888 or RGB565
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawCircleOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight ,T_POS x, T_POS y, 
									T_LEN radius, T_COLOR color, T_U8 colortype)
{
	T_S16 i 		= 0;
	T_S16 x0 		= 0;
	T_S16 y0 		= 0;
	T_S16 i2 		= 0;
	T_S16 radius2 	= 0;
    T_S32 curdelta 	= 0;
	T_S32 nextdelta = 0;

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawCircleOnRGB AK_NULL == buf!");
		return;
	}
	
    radius2 = radius * radius;
    x0 = radius * 14142 / 20000;
    y0 = radius * 14142 / 20000;

    for (i = x0; i >= 0; i--)
    {
        i2 = i * i;
        curdelta = Fwl_Abs(i2 + y0 * y0 - radius2);
		
        do
        {
            y0++;
            nextdelta = Fwl_Abs(i2 + y0 * y0 - radius2);
        }
        while (nextdelta <= curdelta);
		
        y0--;
		
        Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x - i), (T_POS)(y - y0), color, colortype);
        Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x + i), (T_POS)(y - y0), color, colortype);
        Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x - i), (T_POS)(y + y0), color, colortype);
        Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x + i), (T_POS)(y + y0), color, colortype);

        Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x - y0), (T_POS)(y - i), color, colortype);
        Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x + y0), (T_POS)(y - i), color, colortype);
        Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x - y0), (T_POS)(y + i), color, colortype);
        Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x + y0), (T_POS)(y + i), color, colortype);
    }

    return;
}


/**
 * @brief Draw a circle on YUV.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width, must be even number
 * @param T_U32 imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param T_POS x X coordination point of the center point
 * @param T_POS y Y coordination point of the center point
 * @param T_LEN radius Circle radius.
 * @param T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawCircleOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
									T_POS x, T_POS y, T_LEN radius, T_COLOR color)
{
	T_S16 i 		= 0;
	T_S16 x0 		= 0;
	T_S16 y0 		= 0;
	T_S16 i2 		= 0;
	T_S16 radius2 	= 0;
    T_S32 curdelta 	= 0;
	T_S32 nextdelta = 0;

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawCircleOnYUV AK_NULL == buf!");
		return;
	}
	
    radius2 = radius * radius;
    x0 = radius * 14142 / 20000;
    y0 = radius * 14142 / 20000;

    for (i = x0; i >= 0; i--)
    {
        i2 = i*i;
        curdelta = Fwl_Abs(i2 + y0 * y0 - radius2);
		
        do
        {
            y0++;
            nextdelta = Fwl_Abs(i2 + y0 * y0 - radius2);
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


/**
 * @brief Fill a disk on RGB.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS center_x X coordination point of the center point
 * @param T_POS center_y Y coordination point of the center point
 * @param T_LEN radii disk radius.
 * @param T_COLOR color Display color
 * @param T_U8 colortype:RGB888 or RGB565
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawDiskOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight ,T_POS center_x, T_POS center_y, 
								T_POS radii, T_COLOR color, T_U8 colortype)
{
	T_POS i 	= 0;
	T_POS j 	= 0;
    T_POS left 	= 0;
	T_POS top 	= 0;

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawDiskOnRGB AK_NULL == buf!");
		return;
	}

    AK_ASSERT_VAL_VOID((center_x > radii) && ((center_x + radii) < (T_LEN)imgwidth) , "display buffer overflow: draw_disk\r\n" );
    AK_ASSERT_VAL_VOID((center_y > radii) && ((center_y + radii) < (T_LEN)imgheight), "display buffer overflow: draw_disk\r\n" );

    left = center_x - radii;
    top = center_y - radii;

    for(i = 0; i < (radii*2); i++)
    {
        for (j = 0; j < (radii*2); j++)
        {
            if (PointInDisk(center_x, center_y, radii, (T_POS)(left + i), (T_POS)(top + j)) == AK_TRUE)
            {
                Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_U16)(left + i), (T_U16)(top + j), color, colortype);
            }
		}
    }
    return;
}


/**
 * @brief Fill a disk on YUV.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width, must be even number
 * @param T_U32 imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param T_POS center_x X coordination point of the center point
 * @param T_POS center_y Y coordination point of the center point
 * @param T_LEN radii disk radius.
 * @param T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DrawDiskOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
								T_POS center_x, T_POS center_y, T_POS radii, T_COLOR color)
{
	T_POS i 	= 0;
	T_POS j 	= 0;
    T_POS left 	= 0;
	T_POS top 	= 0;

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawDiskOnYUV AK_NULL == buf!");
		return;
	}

    AK_ASSERT_VAL_VOID((center_x > radii) && ((center_x + radii) < (T_LEN)imgwidth) , "display buffer overflow: draw_disk\r\n" );
    AK_ASSERT_VAL_VOID((center_y > radii) && ((center_y + radii) < (T_LEN)imgheight), "display buffer overflow: draw_disk\r\n" );

    left = center_x - radii;
    top = center_y - radii;

    for(i = 0; i < (radii*2); i++)
    {
        for (j = 0; j < (radii*2); j++)
        {
            if (PointInDisk(center_x, center_y, radii, (T_POS)(left + i), (T_POS)(top + j)) == AK_TRUE)
            {
                Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, 
					(T_U16)(left + i), (T_U16)(top + j), color);
            }
        }
    }
    return;
}

T_VOID Fwl_Clean(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_RECT *rect, T_COLOR color, T_U8 colortype)
{
	T_U16 x = 0;
	T_U16 y = 0;
	T_LEN width  = 0;
	T_LEN height = 0;
	T_U32 a = 0;
	T_U32 iIndex = 0;
	T_U8 r = 0;
	T_U8 g = 0;
	T_U8 b = 0;
    T_U8 *p = AK_NULL;

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_Clean AK_NULL == buf!");
		return;
	}

	if (AK_NULL == rect)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_Clean AK_NULL == rect!");
		return;
	}
	
    if(rect->left > (T_LEN)imgwidth || rect->top > (T_LEN)imgheight)
    {
        return;
    }

	width  = rect->left + rect->width > (T_LEN)imgwidth ? (T_LEN)imgwidth - rect->left : rect->width;
	height = rect->top + rect->height > (T_LEN)imgheight ? (T_LEN)imgheight - rect->top : rect->height;
	
	r = (T_U8)(color >> 16);
	g = (T_U8)(color >> 8);
	b = (T_U8)color;

	if (RGB888 == colortype)
	{
		iIndex = ( rect->top * imgwidth + rect->left ) * 3;
		a = imgwidth * 3;
		
	    for( x=0; x<height; x++, iIndex+=a )
		{
			p = buf + iIndex;
			
	        for( y=0; y<width; y++ )
	        {
	            *p++ = r;        // R
	            *p++ = g;        // G
	            *p++ = b;        // B
	        }
		}
	}
	else if (RGB565 == colortype)//RGB565
	{
		iIndex = ( rect->top * imgwidth + rect->left ) * 2;
		a = imgwidth * 2;
		
	    for( x=0; x<height; x++, iIndex+=a )
		{
			p = buf + iIndex;
			
	        for( y=0; y<width; y++ )
	        {   
			    *p++ = ((b & 0xf8) >> 3) | ((g & 0x1c) << 3);	// b, g
			    *p++ = (r & 0xf8)  | ((g & 0xe0) >> 5);			// r, g  
	        }
		}
	}
}


/**
 * @brief Fill  a rectangle with a specified color on RGB
 *
 * @author Baoli.Miao
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_RECT *rect
 * @param T_COLOR color Display color
 * @param T_U8 colortype:RGB888 or RGB565
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_FillSolidRectOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight ,T_RECT *rect, 
									T_COLOR color, T_U8 colortype)
{
#ifdef OS_WIN32

    T_POS right = 0;
    T_POS bottom = 0;

    T_POS i = 0;
	T_POS j = 0;

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_FillSolidRectOnRGB AK_NULL == buf!");
		return;
	}

	if (AK_NULL == rect)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_FillSolidRectOnRGB AK_NULL == rect!");
		return;
	}

	right = rect->left + rect->width;
    bottom = rect->top + rect->height;
	
    if (right == 0 || bottom== 0)
    {
		return;
    }
	
    AK_ASSERT_VAL_VOID(rect->left>=0, "Fwl_FillSolidRectOnRGB():left");
    AK_ASSERT_VAL_VOID(rect->top>=0, "Fwl_FillSolidRectOnRGB():top");
    AK_ASSERT_VAL_VOID(rect->width>=0, "Fwl_FillSolidRectOnRGB():width");
    AK_ASSERT_VAL_VOID(rect->height>=0, "Fwl_FillSolidRectOnRGB():height");

    AK_ASSERT_VAL_VOID( (rect->left + rect->width) <= (T_LEN)imgwidth, "display buffer Length > imgwidth\r\n" );
    AK_ASSERT_VAL_VOID( (rect->top + rect->height) <= (T_LEN)imgheight, "display buffer Height >imgheight\r\n" );

    for(j=rect->top; j<bottom; ++j)
    {
        for(i=rect->left; i<right; ++i)
        {
            Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, i, j, color, colortype);
        }
    }
	
    return;
#endif

#ifdef OS_ANYKA
    Fwl_Clean(buf, imgwidth, imgheight, rect,  color, colortype);
#endif
}


/**
 * @brief Fill  a rectangle with a specified color on YUV
 *
 * @author Baoli.Miao
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width, must be even number
 * @param T_U32 imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param T_RECT *rect
 * @param T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_FillSolidRectOnYUV( T_U8 *ybuf,  T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
									T_RECT *rect, T_COLOR color)
{
	T_U32 i = 0;
	T_U32 j = 0;
	T_U8 R = 0;
	T_U8 G = 0;
	T_U8 B = 0;
	T_U8 Y = 0;
	T_U8 U = 0;
	T_U8 V = 0;
    T_U32 uvpos = 0;
    T_U32 row = 0;
	T_U32 rowMax = 0;
    T_U32 col = 0;
	T_U32 colMax = 0;
    T_U32 tmpU = 0;
    T_U32 tmpV = 0;
    T_U32 tmpK = 0;
    T_U32 tmpH = imgwidth>>1;
    T_U32 tmpY = 0;
    T_U32 tmpW = imgwidth;

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_FillSolidRectOnYUV AK_NULL == buf!");
		return;
	}

	if (AK_NULL == rect)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_FillSolidRectOnYUV AK_NULL == rect!");
		return;
	}

    if ((T_U32)(rect->left + rect->width) > imgwidth || (T_U32)(rect->top + rect->height) > imgheight)
    {
    	Fwl_Print(C3, M_DISPLAY, "Fwl_FillSolidRectOnYUV(), pos is invalid!\
			(rect->left + rect->width): %d, imgwidth: %d.", (rect->left + rect->width), imgwidth);
        return;
    }

	row = rect->left;
	rowMax = rect->left + rect->width;
    col = rect->top;
	colMax = rect->top + rect->height;
    tmpU = rect->left;


    B = (T_U8)color;         // B
    G = (T_U8)(color >> 8);  // G
    R = (T_U8)(color >> 16); // R
    
    Y = (77*R + 150*G + 29*B)>>8;
    U = ((-43*R -85*G + 128*B)>>8) + 128;
    V = ((128*R - 107*G - 21*B)>>8) + 128;

#ifdef OS_ANYKA
    if (1)//(LCD_90_DEGREE == Fwl_GetLcdDegree()) || (LCD_270_DEGREE == Fwl_GetLcdDegree()))
    {
        row = rect->top;
        rowMax = rect->top + rect->height;
        col = rect->left;
        colMax = rect->left + rect->width;
        tmpU = rect->top;
    }
#endif
    tmpU = tmpH * (row>>1);
    tmpK = row & 0x00000001;
    tmpY = tmpW * row;
	
    for(i = row; i < rowMax; i++)
    {
        if (tmpK >= 2)
        {
            tmpK = 0;
            tmpU += tmpH;
        }
		
        tmpV = col;
		
        for(j = col; j < colMax; j++)
        {
            ybuf[tmpY + j] = Y;
			
            //four points that shared a U & V is a 2x2 matrix.
            if (tmpV > tmpW-1)
            {
                tmpV = tmpW-1;
            }
			
            uvpos = tmpU + (tmpV>>1);
            ubuf[uvpos] = U;
            vbuf[uvpos] = V;
            tmpV++;
            #if 0
            tmp = Eng_Get_UVBufPositon_YUV420(i, j, tmpW);
            if (tmp != uvpos)
            {
                ConsolePrint(0, "i = %d, j= %d, uvpos=%d, tmp=%d, tmpU=%d, tmpV=%d\n", i, j, uvpos, tmp, tmpU, tmpV);
                getch();
            }
            #endif
        }
		
        tmpK++;
        tmpY+=tmpW;
    }
}


/**
 * @brief Fill a solid triangle on RGB.
 * dir: 0: U->D, 1: D->U, 2: L->R, 3: R->L
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_RECT *rect
 * @param T_TRIANGLE_DIRECTION dir Draw direction
 * @param T_COLOR color Display color
 * @param T_U8 colortype:RGB888 or RGB565
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_FillSolidTriaOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight ,T_RECT *rect, 
									T_TRIANGLE_DIRECTION dir, T_COLOR color, T_U8 colortype)
{
	T_S16 mid = 0;
    T_S16 thick = 2;
    T_LEN i = 0;
	T_POS left = 0;
	T_POS top = 0;
	T_LEN width = 0;
	T_LEN height = 0;
	T_RECT rect_small;

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_FillSolidTriaOnRGB AK_NULL == buf!");
		return;
	}

	if (AK_NULL == rect)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_FillSolidTriaOnRGB AK_NULL == rect!");
		return;
	}

    if (dir == 0 || dir == 1)         // up down
    {
        mid = rect->left + rect->width / 2 - 1;
		
        if (rect->width % 2 == 1)
        {
            mid = rect->left + rect->width / 2;
            thick = 1;
        }
		
        if (dir == 1)
        {
            for (i = 0; i < rect->height; i++)
            {
            	left = (T_POS)(mid - (rect->width-thick)/2*(i+1)/rect->height);
				top = (T_POS)(rect->top + i);
				width = (T_LEN)((rect->width-thick)/2*(i+1)/rect->height*2+thick);
				height = 1;

				RectInit(&rect_small, left, top, width, height);
				
                Fwl_FillSolidRectOnRGB(buf, imgwidth, imgheight, &rect_small, color, colortype);
            }
        }
        else if (dir == 0)
        {
            for (i = 0; i < rect->height; i++)
            {
            	left = (T_POS)(mid - (rect->width-thick)/2*(i+1)/rect->height);
				top = (T_POS)(rect->top + rect->height - i - 1);
				width = (T_LEN)((rect->width-thick)/2*(i+1)/rect->height*2+thick);
				height = 1;

				RectInit(&rect_small, left, top, width, height);
                Fwl_FillSolidRectOnRGB(buf, imgwidth, imgheight, &rect_small, color, colortype);
            }
        }
    }
    else if (dir == 2 || dir == 3)
    {
        mid = rect->top + rect->height / 2 - 1;
		
        if (rect->height % 2 == 1)
        {
            mid = rect->top + rect->height / 2;
            thick = 1;
        }
		
        if (dir == 3)
        {
            for (i = 0; i < rect->width; i++)
            {
            	left = (T_POS)(rect->left + i);
				top = (T_POS)(mid - (rect->height-thick)/2*(i+1)/rect->width);
				width = 1;
				height = (T_LEN)((rect->height-thick)/2*(i+1)/rect->width*2+thick);

				RectInit(&rect_small, left, top, width, height);
                Fwl_FillSolidRectOnRGB(buf, imgwidth, imgheight, &rect_small, color, colortype);
            }
        }
        else if (dir == 2)
        {
            for (i = 0; i < rect->width; i++)
            {
            	left = (T_POS)(rect->left + rect->width - i - 1);
				top = (T_POS)(mid - (rect->height-thick)/2*(i+1)/rect->width);
				width = 1;
				height = (T_LEN)((rect->height-thick)/2*(i+1)/rect->width*2+thick);

				RectInit(&rect_small, left, top, width, height);
                Fwl_FillSolidRectOnRGB(buf, imgwidth, imgheight, &rect_small, color, colortype);
            }
        }
    }

    return;
}


/**
 * @brief Fill a solid triangle on YUV.
 * dir: 0: U->D, 1: D->U, 2: L->R, 3: R->L
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width, must be even number
 * @param T_U32 imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param T_RECT *rect
 * @param T_TRIANGLE_DIRECTION dir Draw direction
 * @param T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_FillSolidTriaOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
									T_RECT *rect, T_TRIANGLE_DIRECTION dir, T_COLOR color)
{
	T_S16 mid = 0;
    T_S16 thick = 2;
    T_LEN i = 0;
	T_POS left = 0;
	T_POS top = 0;
	T_LEN width = 0;
	T_LEN height = 0;
	T_RECT rect_small;

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_FillSolidTriaOnYUV AK_NULL == buf!");
		return;
	}

	if (AK_NULL == rect)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_FillSolidTriaOnYUV AK_NULL == rect!");
		return;
	}

    if (dir == 0 || dir == 1)         // up down
    {
        mid = rect->left + rect->width / 2 - 1;
		
        if (rect->width % 2 == 1)
        {
            mid = rect->left + rect->width / 2;
            thick = 1;
        }
		
        if (dir == 1)
        {
            for (i = 0; i < rect->height; i++)
            {
            	left = (T_POS)(mid - (rect->width-thick)/2*(i+1)/rect->height);
				top = (T_POS)(rect->top + i);
				width = (T_LEN)((rect->width-thick)/2*(i+1)/rect->height*2+thick);
				height = 1;

				RectInit(&rect_small, left, top, width, height);
                Fwl_FillSolidRectOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, &rect_small, color);
            }
        }
        else if (dir == 0)
        {
            for (i = 0; i < rect->height; i++)
            {
            	left = (T_POS)(mid - (rect->width-thick)/2*(i+1)/rect->height);
				top = (T_POS)(rect->top + rect->height - i - 1);
				width = (T_LEN)((rect->width-thick)/2*(i+1)/rect->height*2+thick);
				height = 1;

				RectInit(&rect_small, left, top, width, height);
                Fwl_FillSolidRectOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, &rect_small, color);
            }
        }
    }
    else if (dir == 2 || dir == 3)
    {
        mid = rect->top + rect->height / 2 - 1;
		
        if (rect->height % 2 == 1)
        {
            mid = rect->top + rect->height / 2;
            thick = 1;
        }
		
        if (dir == 3)
        {
            for (i = 0; i < rect->width; i++)
            {
            	left = (T_POS)(rect->left + i);
				top = (T_POS)(mid - (rect->height-thick)/2*(i+1)/rect->width);
				width = 1;
				height = (T_LEN)((rect->height-thick)/2*(i+1)/rect->width*2+thick);

				RectInit(&rect_small, left, top, width, height);
                Fwl_FillSolidRectOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, &rect_small, color);
            }
        }
        else if (dir == 2)
        {
            for (i = 0; i < rect->width; i++)
            {
            	left = (T_POS)(rect->left + rect->width - i - 1);
				top = (T_POS)(mid - (rect->height-thick)/2*(i+1)/rect->width);
				width = 1;
				height = (T_LEN)((rect->height-thick)/2*(i+1)/rect->width*2+thick);

				RectInit(&rect_small, left, top, width, height);
                Fwl_FillSolidRectOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, &rect_small, color);
            }
        }
    }

    return;
}


/**
 * @brief Draw a radio button on RGB.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x X coordination point of the center point
 * @param T_POS y Y coordination point of the center point
 * @param T_LEN radius Circle radius.
 * @param T_BOOL focus: the button is focused or not.
 * @param T_COLOR color Display color
 * @param T_U8 colortype:RGB888 or RGB565
 * @return T_VOID
 * @retval void
 */
T_VOID  Fwl_DrawRadioOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight ,T_POS x, T_POS y, 
									T_LEN radius, T_BOOL focus, T_COLOR color, T_U8 colortype)
{
	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawRadioOnRGB AK_NULL == buf!");
		return;
	}
	
	Fwl_DrawCircleOnRGB(buf, imgwidth, imgheight, x, y, radius, color, colortype);

    if (AK_TRUE == focus)
    {
        Fwl_DrawDiskOnRGB(buf, imgwidth, imgheight, x, y, (T_POS)(radius * 2 / 3), color, colortype);
    }
}


/**
 * @brief Draw a radio button on YUV.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width, must be even number
 * @param T_U32 imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param T_POS x X coordination point of the center point
 * @param T_POS y Y coordination point of the center point
 * @param T_LEN radius Circle radius.
 * @param T_BOOL focus: the button is focused or not.
 * @param T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID  Fwl_DrawRadioOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
									T_POS x, T_POS y, T_LEN radius, T_BOOL focus, T_COLOR color)
{
	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_DrawRadioOnYUV AK_NULL == buf!");
		return;
	}
	
	Fwl_DrawCircleOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x, y, radius, color);

    if (AK_TRUE == focus)
    {
        Fwl_DrawDiskOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x, y, (T_POS)(radius * 2 / 3), color);
    }
}


/**
 * @brief Draw 3D rectangle on RGB
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_RECT *rect
 * @param T_S8 flag Display mode.
 * @param T_U8 colortype:RGB888 or RGB565
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_Draw3DRectOnRGB(T_U8* buf, T_U32 imgwidth, T_U32 imgheight ,T_RECT *rect, 
									T_S8 flag, T_U8 colortype)
{
	T_COLOR dark = RGB2AkColor(0, 0, 0, (T_U8)g_Graph.LCDCOLOR[DISPLAY_LCD_0]);
    T_COLOR bright = RGB2AkColor(255,255,255, (T_U8)g_Graph.LCDCOLOR[DISPLAY_LCD_0]);

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_Draw3DRectOnRGB AK_NULL == buf!");
		return;
	}

	if (AK_NULL == rect)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_Draw3DRectOnRGB AK_NULL == rect!");
		return;
	}
	
    if (flag)        /* out */
    {
        Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, rect->left, rect->top, 
			(T_POS)(rect->left+rect->width-1), rect->top, bright, colortype);
		
        Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, rect->left, (T_POS)(rect->top+rect->height-1), 
			(T_POS)(rect->left+rect->width-1), (T_POS)(rect->top+rect->height-1), dark, colortype);
		
        Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, rect->left, rect->top, 
			rect->left, (T_POS)(rect->top+rect->height-1), bright, colortype);
		
        Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, (T_POS)(rect->left+rect->width-1), rect->top, 
			(T_POS)(rect->left+rect->width-1), (T_POS)(rect->top+rect->height-1), dark, colortype);
    }
    else
    {
        Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, rect->left, rect->top, 
			(T_POS)(rect->left+rect->width-1), rect->top, dark, colortype);
		
        Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, rect->left, (T_POS)(rect->top+rect->height-1), 
			(T_POS)(rect->left+rect->width-1), (T_POS)(rect->top+rect->height-1), bright, colortype);
		
        Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, rect->left, rect->top, 
			rect->left, (T_POS)(rect->top+rect->height-1), dark, colortype);
		
        Fwl_DrawLineOnRGB(buf, imgwidth, imgheight, (T_POS)(rect->left+rect->width-1), rect->top, 
			(T_POS)(rect->left+rect->width-1), (T_POS)(rect->top+rect->height-1), bright, colortype);
    }

    return;
}



/**
 * @brief Draw 3D rectangle on YUV
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width, must be even number
 * @param T_U32 imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param T_RECT *rect
 * @param T_S8 flag Display mode.
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_Draw3DRectOnYUV(T_U8 *ybuf,  T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
									T_RECT *rect, T_S8 flag)
{
	T_COLOR dark = RGB2AkColor(0, 0, 0, (T_U8)g_Graph.LCDCOLOR[DISPLAY_LCD_0]);
    T_COLOR bright = RGB2AkColor(255,255,255, (T_U8)g_Graph.LCDCOLOR[DISPLAY_LCD_0]);

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_Draw3DRectOnYUV AK_NULL == buf!");
		return;
	}

	if (AK_NULL == rect)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_Draw3DRectOnYUV AK_NULL == rect!");
		return;
	}
	
    if (flag)        /* out */
    {
        Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, rect->left, 
			rect->top, (T_POS)(rect->left+rect->width-1), rect->top, bright);
		
        Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, rect->left, 
			(T_POS)(rect->top+rect->height-1), (T_POS)(rect->left+rect->width-1), (T_POS)(rect->top+rect->height-1), dark);
		
        Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, rect->left, 
			rect->top, rect->left, (T_POS)(rect->top+rect->height-1), bright);
		
        Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_POS)(rect->left+rect->width-1), 
			rect->top, (T_POS)(rect->left+rect->width-1), (T_POS)(rect->top+rect->height-1), dark);
    }
    else
    {
        Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, rect->left, 
			rect->top, (T_POS)(rect->left+rect->width-1), rect->top, dark);
		
        Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, rect->left, 
			(T_POS)(rect->top+rect->height-1), (T_POS)(rect->left+rect->width-1), (T_POS)(rect->top+rect->height-1), bright);
		
        Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, rect->left, 
			rect->top, rect->left, (T_POS)(rect->top+rect->height-1), dark);
		
        Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, (T_POS)(rect->left+rect->width-1), 
			rect->top, (T_POS)(rect->left+rect->width-1), (T_POS)(rect->top+rect->height-1), bright);
    }

    return;
}




/**
* @BRIEF copy source image to destination image with format convert & scale
* @PARAM dstBuf:    destination image buffer (AK98:RGB888, other:RGB565)
* @PARAM dstFormat: destination image format (AK98:RGB888, other:RGB565)
* @PARAM scaleWidth:  image width after scale
* @PARAM scaleHeight: image height after scale
* @PARAM dstPosX:   destination offset X
* @PARAM dstPosY:   destination offset Y
* @PARAM dstWidth:  destination image width
* @PARAM srcBuf:    source image buffer (RGB888/BGR888/RGB565)
* @PARAM srcWidth:  source image width
* @PARAM srcHeight: source image height
* @PARAM srcFormat: source image format(0 = RGB888, 1 = BGR888, 2 = RGB565)
* @retval 0  success
* @retval <0 fail
* @AUTHOR zhang_chengyan
* @DATE 2008-07-01
* @UPDATE 2008-10-08
*/
T_S32 Fwl_Img_BitBltRGB(T_VOID *dstBuf, T_U8 dstFormat, T_U16 scaleWidth, T_U16 scaleHeight,
							T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth, 
							T_VOID *srcBuf, T_U16 srcWidth, T_U16 srcHeight, T_U8 srcFormat)
{
	T_S32 ret = -1;
	//T_S8 *pDestData888 = AK_NULL;
	T_VOID *sBuf = AK_NULL;
	T_U8 sFormat = srcFormat;

	sBuf = srcBuf;

	if ((AK_NULL == dstBuf) || (AK_NULL == srcBuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_Img_BitBltRGB AK_NULL == buf!");
		return ret;
	}

#ifdef OS_ANYKA
#ifdef CHIP_AK98XX
	if (RGB888 == dstFormat)
	{
	/*
	由于98系列Img_BitBlt的源数据格式只支持0 = RGB888, 1 = BGR888；
	因此当传入RGB565格式的源数据时，先做RGB565到RGB888的转换，
	再把转换好的RGB888格式的数据作为源数据传入Img_BitBlt。
	*/
		if (RGB565 == srcFormat)
		{
#if 0		
			pDestData888 = Fwl_Malloc(srcWidth * srcHeight * 3);

			if (AK_NULL != pDestData888)
			{
				Fwl_RGB565toRGB888(pDestData888, srcBuf, srcWidth, srcHeight);
				sBuf = pDestData888;
				sFormat = RGB888;
			}
#endif	
          sBuf = srcBuf;
          sFormat = RGB565;
		}

		ret = Fwl_Scale_Convert(dstBuf, scaleWidth, scaleHeight, dstPosX, dstPosY, dstWidth, 
			sBuf, srcWidth, srcHeight, sFormat);

		if (AK_NULL != pDestData888)
		{
			pDestData888 = Fwl_Free(pDestData888);
		}
	}

#else
	if (RGB565 == dstFormat)
	{		

		if (BGR888 == srcFormat)
		{
			return ret;
		}

		ret = Fwl_Scale_Convert(dstBuf, scaleWidth, scaleHeight, dstPosX, dstPosY, dstWidth, 
			sBuf, srcWidth, srcHeight, sFormat);
	}
#endif
#endif
	return ret;
}


/**
* @BRIEF copy source YUV to destination image with format convert & scale
* @PARAM dstBuf:    destination image buffer (AK98:RGB888, other:RGB565)
* @PARAM dstFormat: destination image format (AK98:RGB888, other:RGB565)
* @PARAM scaleWidth:  image width after scale
* @PARAM scaleHeight: image height after scale
* @PARAM dstPosX:   destination offset X
* @PARAM dstPosY:   destination offset Y
* @PARAM dstWidth:  destination image width
* @PARAM srcY:    source image Y buffer (YUV420)
* @PARAM srcU:    source image U buffer (YUV420)
* @PARAM srcV:    source image V buffer (YUV420)
* @PARAM srcWidth:  source image width
* @PARAM srcHeight: source image height
* @retval 0  success
* @retval <0  fail
* @AUTHOR liu_zhenwu
* @DATE 2009-01-15
*/
T_S32 Fwl_Img_BitBltYUV(T_VOID *dstBuf, T_U8 dstFormat, T_U16 scaleWidth, T_U16 scaleHeight,
                 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
                 T_VOID *srcY, T_VOID *srcU, T_VOID *srcV, T_U16 srcWidth, T_U16 srcHeight)
{
	T_S32 ret = -1;

	if ((AK_NULL == dstBuf) || (AK_NULL == srcY) || (AK_NULL == srcU) || (AK_NULL == srcV))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_Img_BitBltYUV AK_NULL == buf!");
		return ret;
	}
	
#ifdef OS_ANYKA
#ifdef CHIP_AK98XX
	if (RGB888 == dstFormat)
#else
	if (RGB565 == dstFormat)
#endif
	{
		ret = Img_BitBltYUV(dstBuf, scaleWidth, scaleHeight, dstPosX, dstPosY, dstWidth, 
			srcY, srcU, srcV, srcWidth, srcHeight);
	}
#endif
	return ret;
}



/**
 * @brief: convert RGB565 to RGB888
 * 
 * @author 
 * @modify 
 * @param T_U8 *pDestData888: destation buffer, format is RGB888
 * @param T_U8 *pSrcData565: source buffer, format is RGB565
 * @param T_S32 width, T_S32 height: 
 * @return T_VOID
 */
#if 1
T_VOID Fwl_RGB565toRGB888(T_S8 * pDestData888, T_S8 * pSrcData565, T_S32 nWidth, T_S32 nHeight)
{
	T_S32 i = 0;
	T_S32 j = 0;
	T_U8 *pSrcData = AK_NULL;
	T_U8 *pDestData = AK_NULL;
	
	if((pDestData888 != AK_NULL) && (pSrcData565 != AK_NULL) && (nWidth > 0) && (nHeight > 0)) 
	{		 
		for(i = 0; i < nHeight; i++)
		{						 
			for(j = 0; j < nWidth; j++)
			{
				pSrcData =	pSrcData565  + (i * nWidth + j ) * 2;
				pDestData = pDestData888 + (i * nWidth + j ) * 3;
				
				pDestData[0] = pSrcData[1] & 0xF8;
				pDestData[1] = ((pSrcData[1] & 0x07) << 5) | ((pSrcData[0] & 0xE0) >> 3);	
				pDestData[2] = (pSrcData[0] & 0x1F) << 3;					
			}			
		}
	}
	else
	{
		Fwl_Print(C3, M_DISPLAY, "data format error");
	}	 
}
#else

T_VOID Fwl_RGB565toRGB888(T_S8 * pDestData888, T_S8 * pSrcData565, T_S32 nWidth, T_S32 nHeight)
{
	T_S32 i = 0;
	T_S32 j = 0;                   

	if((pDestData888 != AK_NULL) && (pSrcData565 != AK_NULL) && (nWidth > 0) && (nHeight > 0)) 
	{                 
		if(nWidth & 3)
		{
            T_U16 *pSrcData = AK_NULL;
            T_U8 *pDestData = AK_NULL;
            
            for(i = 0; i < nHeight; i++)
            {
				pSrcData = (T_U16*)pSrcData565 + i * nWidth;
				pDestData = pDestData888 + i*nWidth*3;

				for(j = 0; j < nWidth; j++)                       
				{
					pDestData[0] = (T_U8)(pSrcData[0]>>8);                              
					pDestData[1] = (T_U8)(pSrcData[0]>>3);                              
					pDestData[2] = (T_U8)(pSrcData[0]<<3);
					pSrcData++;
					pDestData += 3;
				}
            }
		}
		else
		{
			T_U32 *pSrcData = AK_NULL;
            T_U32 *pDestData = AK_NULL;

            for(i = 0; i < nHeight; i++)
            {
				pSrcData = (T_U32 *)((T_U16*)pSrcData565 + i*nWidth);
				pDestData = (T_U32 *)(pDestData888 + i*nWidth*3);              

				for(j = 0; j < nWidth; j+=4)
				{
					*pDestData++ = ((pSrcData[0]&0xf800)>>8)|((pSrcData[0]<<5)&0xfc00)
						|((pSrcData[0]&0x1f)<<19)|((pSrcData[0]&0xf8000000));
					
					*pDestData++ = ((pSrcData[0]>>19)&0xfc)|((pSrcData[0]>>5)&0xf800)
						|((pSrcData[1]&0xf800)<<8)|((pSrcData[1]<<21)&0xfc000000);
					
					*pDestData++ = ((pSrcData[1]&0x1f)<<3)|((pSrcData[1]>>16)&0xf800)
						|((pSrcData[1]>>3)&0xfc0000)|((pSrcData[1]&0x1f0000)<<11);
					pSrcData+=2;
				}
            }
       }
	 }
	else
	{
		Fwl_Print(C3, M_DISPLAY, "data format error");
	}
}
#endif



/**
 * @brief: convert RGB888 to RGB565
 * 
 * @author 
 * @modify 
 * @param T_U8 *pDestData565: destation buffer, format is RGB565
 * @param T_U8 *pScrData888: source buffer, format is RGB888
 * @param T_S32 width, T_S32 height: 
 * @return T_VOID
 */
T_VOID Fwl_RGB888toRGB565(T_S8 * pDestData565, T_S8 * pScrData888, T_S32 nWidth, T_S32 nHeight)
{
	T_S32 i = 0;
	T_S32 j = 0;
	
    T_S32 length = 0;

	if((pDestData565 != AK_NULL) && (pScrData888 != AK_NULL) && (nWidth > 0) && (nHeight > 0)) 
	{		 
		length = nWidth * nHeight * 3;
		
		for (i=0; i<length; i+=3)
	    {    
	        pDestData565[j++] = ((pScrData888[i+1] & 0x1C) << 3) | ((pScrData888[i+2] & 0xF8) >> 3);    // low 8 bit, G and B
	        pDestData565[j++] = (pScrData888[i] & 0xF8) | ((pScrData888[i+1] & 0xE0) >> 5);             // high 8 bit, R and G
	    }
	}
	else
	{
		Fwl_Print(C3, M_DISPLAY, "data format error");
	}
    
}



/**
 * @brief: convert YUV420 to RGB888
 * 
 * @author 
 * @modify 
 * @param T_U8 *dstRGB: destation buffer, format is RGB888
 * @param T_U8 *y, *u, *v: source buffer, format is YUV420
 * @param T_U32 width, T_U32 uvLine: 
 * @return T_VOID
 */
T_BOOL Fwl_YUV420toRGB888(T_U8 *y, T_U8 *u, T_U8 *v, T_U8 *dstRGB, T_U32 width, T_U32 uvLine)
{
	T_S32 rgb = 0;
	T_U32 h = 0;
	T_U32 w = 0;
	T_U32 uv_width = width >> 1;
	T_S32 curY = 0;
	T_S32 curU = 0;
	T_S32 curV = 0;
	T_U8 *_y = AK_NULL;
	T_U8 *_u = AK_NULL;
	T_U8 *_v = AK_NULL;

	if ((AK_NULL == y) || (AK_NULL == u) || (AK_NULL == v) || (AK_NULL == dstRGB))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_YUV420toRGB888 AK_NULL == buf!");
		return AK_FALSE;
	}

	_y = y;
	_u = u;
	_v = v;
	
	for (h = 0; h < uvLine; h++)
	{
		for (w = 0; w < uv_width; w++)
		{
			//// point odd
			curY = *_y++;
			curU = (*_u++)-128;
			curV = (*_v++)-128;

			//R = curY + ((359*curV)>>8);	// [-179.5, 433.097]
			rgb = curY + ((180*curV)>>7);	// optimize, reduce a ldr_imm8 instruction
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);
			
			rgb = curY - ((88*curU + 183*curV)>>8);	// [-134.44, 390.5]
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);

			//B = curY + ((454*curU)>>8);	// [-227, 480.226]
			rgb = curY + ((227*curU)>>7);	// optimize, reduce a ldr_imm8 instruction
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);
			
			//// point even
			curY = *_y++;

			//R = curY + ((359*curV)>>8);	// [-179.5, 433.097]
			rgb = curY + ((180*curV)>>7);	// optimize, reduce a ldr_imm8 instruction
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);
			
			rgb = curY - ((88*curU + 183*curV)>>8);	// [-134.44, 390.5]
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);

			//B = curY + ((454*curU)>>8);	// [-227, 480.226]
			rgb = curY + ((227*curU)>>7);	// optimize, reduce a ldr_imm8 instruction
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);
		}

		//// line even
		_u -= uv_width;
		_v -= uv_width;

		for (w = 0; w < uv_width; w++)
		{
			//// point odd
			curY = *_y++;
			curU = (*_u++)-128;
			curV = (*_v++)-128;

			//R = curY + ((359*curV)>>8);	// [-179.5, 433.097]
			rgb = curY + ((180*curV)>>7);	// optimize, reduce a ldr_imm8 instruction
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);
			
			rgb = curY - ((88*curU + 183*curV)>>8);	// [-134.44, 390.5]
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);

			//B = curY + ((454*curU)>>8);	// [-227, 480.226]
			rgb = curY + ((227*curU)>>7);	// optimize, reduce a ldr_imm8 instruction
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);

			//// point even
			curY = *_y++;

			//R = curY + ((359*curV)>>8);	// [-179.5, 433.097]
			rgb = curY + ((180*curV)>>7);	// optimize, reduce a ldr_imm8 instruction
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);

			rgb = curY - ((88*curU + 183*curV)>>8);	// [-134.44, 390.5]
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);

			//B = curY + ((454*curU)>>8);	// [-227, 480.226]
			rgb = curY + ((227*curU)>>7);	// optimize, reduce a ldr_imm8 instruction
			*dstRGB++ = rgb>255 ? 255 : (rgb<0 ? 0 : (T_U8)rgb);
		}
	}

	return AK_TRUE;
}


/**
 * @brief: convert YUV420 to RGB565
 * 
 * @author 
 * @modify 
 * @param T_U8 *dstRGB: destation buffer, format is RGB565
 * @param T_U8 *y, *u, *v: source buffer, format is YUV420
 * @param T_U32 width, T_U32 uvLine: 
 * @return T_VOID
 */
T_BOOL Fwl_YUV420toRGB565(T_U8 *y, T_U8 *u, T_U8 *v, T_U32 *dstRGB, T_U32 width, T_U32 uvLine)
{
	T_S32 rgb = 0;
	T_U32 color = 0;
	T_U32 h = 0;
	T_U32 w = 0;
	T_U32 uv_width = width >> 1;
	T_S32 curY = 0;
	T_S32 curU = 0;
	T_S32 curV = 0;
	T_U8 *_y = AK_NULL;
	T_U8 *_u = AK_NULL;
	T_U8 *_v = AK_NULL;

	if ((AK_NULL == y) || (AK_NULL == u) || (AK_NULL == v) || (AK_NULL == dstRGB))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_YUV420toRGB565 AK_NULL == buf!");
		return AK_FALSE;
	}

	_y = y;
	_u = u;
	_v = v;
	
	for (h = 0; h < uvLine; h++)
	{
		for (w = 0; w < uv_width; w++)
		{
			//// point odd
			curY = (*_y++)<<7;
			curU = (*_u++)-128;
			curV = (*_v++)-128;

			//R = curY + ((359*curV)>>8);	// [-179.5, 433.097]
			rgb = (curY + 180*curV)>>10;	// optimize, reduce a ldr_imm8 instruction
			color = (rgb>0x1f ? 0x1f : (rgb<0 ? 0 : rgb))<<11;
			
			rgb =  (curY - 44*curU - 92*curV)>>9;	// [-134.44, 390.5]
			color |= (rgb>0x3f ? 0x3f : (rgb<0 ? 0 : rgb))<<5;

			//B = curY + ((454*curU)>>8);	// [-227, 480.226]
			rgb = (curY + 227*curU)>>10;	// optimize, reduce a ldr_imm8 instruction
			color |= (rgb>0x1f ? 0x1f : (rgb<0 ? 0 : rgb));
			
			//// point even
			curY = (*_y++)<<7;

			//R = curY + ((359*curV)>>8);	// [-179.5, 433.097]
			rgb = (curY + 180*curV)>>10;	// optimize, reduce a ldr_imm8 instruction
			color |= (rgb>0x1f ? 0x1f : (rgb<0 ? 0 : rgb))<<27;
			
			rgb =  (curY - 44*curU - 92*curV)>>9;	// [-134.44, 390.5]
			color |= (rgb>0x3f ? 0x3f : (rgb<0 ? 0 : rgb))<<21;

			//B = curY + ((454*curU)>>8);	// [-227, 480.226]
			rgb = (curY + 227*curU)>>10;	// optimize, reduce a ldr_imm8 instruction
			color |= (rgb>0x1f ? 0x1f : (rgb<0 ? 0 : rgb))<<16;
			*dstRGB++ = color;
		}

		//// line even
		_u -= uv_width;
		_v -= uv_width;

		for (w = 0; w < uv_width; w++)
		{
			//// point odd
			curY = (*_y++)<<7;
			curU = (*_u++)-128;
			curV = (*_v++)-128;

			//R = curY + ((359*curV)>>8);	// [-179.5, 433.097]
			rgb = (curY + 180*curV)>>10;	// optimize, reduce a ldr_imm8 instruction
			color = (rgb>0x1f ? 0x1f : (rgb<0 ? 0 : rgb))<<11;
			
			rgb =  (curY - 44*curU - 92*curV)>>9;	// [-134.44, 390.5]
			color |= (rgb>0x3f ? 0x3f : (rgb<0 ? 0 : rgb))<<5;

			//B = curY + ((454*curU)>>8);	// [-227, 480.226]
			rgb = (curY + 227*curU)>>10;	// optimize, reduce a ldr_imm8 instruction
			color |= (rgb>0x1f ? 0x1f : (rgb<0 ? 0 : rgb));
			
			//// point even
			curY = (*_y++)<<7;

			//R = curY + ((359*curV)>>8);	// [-179.5, 433.097]
			rgb = (curY + 180*curV)>>10;	// optimize, reduce a ldr_imm8 instruction
			color |= (rgb>0x1f ? 0x1f : (rgb<0 ? 0 : rgb))<<27;
			
			rgb =  (curY - 44*curU - 92*curV)>>9;	// [-134.44, 390.5]
			color |= (rgb>0x3f ? 0x3f : (rgb<0 ? 0 : rgb))<<21;

			//B = curY + ((454*curU)>>8);	// [-227, 480.226]
			rgb = (curY + 227*curU)>>10;	// optimize, reduce a ldr_imm8 instruction
			color |= (rgb>0x1f ? 0x1f : (rgb<0 ? 0 : rgb))<<16;
			*dstRGB++ = color;
		}
	}

	return AK_TRUE;
}


/**
 * @brief: convert RGB565 to YUV420
 * 
 * @author 
 * @modify 
 * @param T_U8 *yuv: destation buffer, format is YUV420
 * @param T_U8 *pRgb565: source buffer, format is RGB565
 * @param T_U32 rectwidth, T_U32 rectheigth: 
 * @return T_VOID
 */
T_VOID  Fwl_RGB565toYUV420(T_U8* pRgb565, T_U8* yuv, T_U32 rectwidth, T_U32 rectheigth)
{
	T_U8 *Ybuf = AK_NULL;
	T_U8 *Ubuf = AK_NULL;
	T_U8 *Vbuf = AK_NULL;
    T_U32 width  = rectwidth;
    T_U32 height = rectheigth;
    T_U32 lineWidth = 0;
    T_U32 i = 0;
	T_U32 j = 0;
    T_U8  *rawData = AK_NULL;
    T_U32 xOffset = 0;
    T_U32 yOffset = 0;
    T_U32 yuvWidth = rectwidth; 
    T_U8 *dstY = AK_NULL;
	T_U8 *dstU = AK_NULL;
	T_U8 *dstV = AK_NULL;
    T_S16 Y = 0;
	T_S16 U = 0;
	T_S16 V = 0;
    T_U16 R = 0;
	T_U16 G = 0;
	T_U16 B = 0;
    T_U32 biWidth = rectwidth;
    T_U32 biHeight = rectheigth;

	if ((AK_NULL == pRgb565) || (AK_NULL == yuv))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_YUV420toRGB565 AK_NULL == buf!");
		return;
	}

    Ybuf = yuv;
    Ubuf = yuv + width * height;
    Vbuf = yuv + width * height * 5 / 4;
 
    lineWidth = (biWidth*2+3)/4*4;

    for(i=0; i<biHeight; i++)
    {   
		rawData = pRgb565 + i*lineWidth;
		dstY = Ybuf + (yOffset + i)*yuvWidth + xOffset;
		dstU = Ubuf + ((yOffset + i)/2)*(yuvWidth/2) + xOffset/2;
		dstV = Vbuf + ((yOffset + i)/2)*(yuvWidth/2) + xOffset/2;

		for(j=0; j<biWidth; j++)
		{
		//if(j+xOffset >= clipRectLeft && 
	    //j+xOffset <= clipRectRight)  //current pixel not in the clip rect, continue to next pixel
		{
		    R = rawData[1]&0xf8;
		    G = ((*(T_U16*)rawData)>>3)&0xfc;
		    B = (rawData[0]<<3)&0xf8;
		    Y = ( 306 * R + 601 * G + 117 * B) >> 10; //caculate Y data of BMP pixel
		             dstY[0] = (Y > 255) ? 255 : Y;         //overflow check:[0, 255]
		             
		    if (((i&0x1) == 0)&& ((j&0x1) == 0))
		    {
			     U = ((-173 * R - 339 * G + 512 * B) >> 10) + 128; //caculate U data of BMP pixel
			     dstU[0] = (U > 255) ? 255 : ((U < 0) ? 0 : U);

			     V = (( 512 * R - 429 * G -  83 * B) >> 10) + 128; //caculate V data of BMP pixel
			     dstV[0] = (V > 255) ? 255 : ((V < 0) ? 0 : V);
		    }
		 }
	   
		   rawData += 2;
		   dstY++;

		   if((j&0x01)==1)
		   {
			    dstU++;
			    dstV++;
		   }
	  	}
 	} 
}

/**
 * @brief: convert RGB888 to YUV420
 * 
 * @author 
 * @modify 
 * @param T_U8 *yuv: destation buffer, format is YUV420
 * @param T_U8 *pRgb888: source buffer, format is RGB888
 * @param T_U32 rectwidth, T_U32 rectheigth: 
 * @return T_VOID
 */
T_VOID  Fwl_RGB888toYUV420(T_U8* pRgb888, T_U8* yuv, T_U32 rectwidth, T_U32 rectheigth)
{

}




/**
 * @brief Draw AKBmp format bitmap in part on RGB
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-29
 * @param[in]*buf:
 * @param[in] imgwidth:max width
 * @param[in] imgheight:max height
 * @param[in] x		x position the AKBmp would be drawn, Destination Area Left 
 * @param[in] y		y position the AKBmp would be drawn, Destination area Top
 * @param[in] range	The part rect of AKBmp would be drawn, the range rect coordinate relative to the left-top of AKBmp image
 * @param[in] AkBmp	The source AKbmp would be drawn.
 * @param[in] bkColor	The transparent color.If a dot with this color appears in bmp, it will not be drawn.
 * @param[in] Reverse	if AK_TRUE, reverse draw the source pixel
 * @param[in] colortype:RGB888 or RGB565
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @note if range, color, reverse are AK_NULL, AK_NULL, AK_FALSE, this function would run most quickly
 * @retval
 */
T_BOOL Fwl_AkBmpDrawPartOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight,
	T_POS x, T_POS y, T_RECT *range, const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse, T_U8 colortype)
{
    T_LEN	i = 0;
	T_LEN	j = 0;
	T_LEN	k = 0;
	T_U8	r = 0;
	T_U8	g = 0;
	T_U8	b = 0;
    T_LEN	MyWidth = 0;
	T_LEN	MyHeight = 0;
    T_COLOR	color = 0;
    T_U8	cMask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
    T_U32	Linebytes = 0;
	T_U32	bufferWidth = 0;
    T_U32	iIndex = 0;
	T_U32	dataIndex = 0;
    T_U16	bkColor16 = 0;
	T_U32	bkColor32 = 0;
    T_U16	temp = 0;
    T_BOOL	transMode = 0;
    T_U32	dataWidth = 0;
	T_U8	*bmpData = AK_NULL;
    T_U8	*curBmpData = AK_NULL;
	T_U8	*displayBuffer = AK_NULL;
	T_U8	*p = AK_NULL;
    T_U8	*endP = AK_NULL;
    T_U8	*dataP = AK_NULL;
    T_U8	*endDataP = AK_NULL;
    T_U8	*focus = AK_NULL;
    T_U32	nextLineDataWidth = 0;
    T_U32	nextLineBufferWidth = 0;
    T_RECT	rangeWhenNull;

	AK_ASSERT_PTR(AkBmp, "Fwl_AkBmpDrawPartOnRGB():	AkBmp = NULL", AK_FALSE);

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_AkBmpDrawPartOnRGB():	buf = AK_NULL!");
		return AK_FALSE;
	}
	
	

    if (x >= (T_LEN)imgwidth || y >= (T_LEN)imgheight)
    { 
        Fwl_Print(C3, M_DISPLAY, "x or y out of range, x = %d, y = %d", x, y);
        return AK_FALSE;
    }

	
    if(AK_NULL == range)
    {
		RectInit(&rangeWhenNull, 0, 0, AkBmp->Width, AkBmp->Height);
        range = &rangeWhenNull;
    }
    if (range->left >= AkBmp->Width || range->top >= AkBmp->Height)
    {
		Fwl_Print(C3, M_DISPLAY, "ICON Out of Range.");
        return AK_FALSE;
    }

    bmpData = (T_U8 *)AkBmp->BmpData;
	
	if (AK_NULL == bmpData)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_AkBmpDrawPartOnRGB, bmpData = AK_NULL !");
		return AK_FALSE;
	}
	
#ifdef OS_ANYKA
    displayBuffer = buf;
#else
	#ifdef LCD_MODE_565
		if (((T_pImgLay)Fwl_hRGBLayer)->pData == buf)
		{
			displayBuffer = Fwl_Malloc(MAIN_LCD_WIDTH*MAIN_LCD_HEIGHT*2);
			if (AK_NULL == displayBuffer)
			{
				return AK_FALSE;
			}
			Fwl_RGB888toRGB565(displayBuffer, Fwl_GetDispMemory(), MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT);
		}
		else
		{
			displayBuffer = Fwl_Malloc(imgwidth*imgheight*2);
			if (AK_NULL == displayBuffer)
			{
				return AK_FALSE;
			}
			Fwl_RGB888toRGB565(displayBuffer, buf, imgwidth, imgheight);
		}
	#else
		if (((T_pImgLay)Fwl_hRGBLayer)->pData == buf)
		{
			displayBuffer = Fwl_GetDispMemory();
		}
		else
		{
			displayBuffer = buf;
		}
	#endif
#endif
	if (RGB888 == colortype)
	{
    	bufferWidth = imgwidth * 3;
	}
	else if (RGB565 == colortype)
	{
		bufferWidth = imgwidth * 2;
	}
	else
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_AkBmpDrawPartOnRGB(): Data Format Is NOT RGB888/565.");
		return AK_FALSE;
	}

    if (range->left + range->width > AkBmp->Width)
    {
        range->width = AkBmp->Width - range->left;
    }
	
    if (range->top + range->height > AkBmp->Height)
    {
        range->height = AkBmp->Height - range->top;
    }

    if (x + AkBmp->Width > (T_LEN)imgwidth)
    {
        MyWidth = (T_LEN)(imgwidth - x);
    }
    else
    {
        MyWidth = AkBmp->Width;
    }

    if (MyWidth > range->width)
    {
        MyWidth = range->width;
    }

    if (y + AkBmp->Height > (T_LEN)imgheight)
    {
        MyHeight = (T_LEN)(imgheight - y);
    }
    else
    {
        MyHeight = AkBmp->Height;
    }

    if (MyHeight > range->height)
    {
        MyHeight = range->height;
    }

    switch (AkBmp->Deep)
	{
    case 32:   /* 32 bits deep */ 
		if(RGB888 == colortype)
		{
	        Linebytes = AkBmp->Width * 4; 
	        iIndex = ( y * imgwidth + x ) * 3; 
	        dataIndex = Linebytes * range->top + range->left * 4; 
	        dataWidth = (AkBmp->Width * 4); 
	        nextLineDataWidth = (AkBmp->Width - MyWidth) * 4; 
	        p = displayBuffer + iIndex; 
	        endP = p + bufferWidth * MyHeight; 
	        dataP = bmpData + dataIndex + dataWidth*(AkBmp->Height-1); 
	        endDataP = dataP + (MyWidth * 4); 
	        nextLineBufferWidth = (imgwidth - MyWidth) * 3; 

	        for(; p<endP; p+=nextLineBufferWidth, dataP-=2*dataWidth) 
	        { 
	            endDataP = dataP+ (MyWidth * 4); 
	            for(; dataP<endDataP; ) 
	            { 
	                 *p = *(dataP+2) + ((T_U16)(*p)*(255-*(dataP+3))/255); //R 
	                 *(p+1) = *(dataP+1) + ((T_U16)(*(p+1))*(255-*(dataP+3))/255); //G 
	                 *(p+2) = *(dataP) + ((T_U16)(*(p+2))*(255-*(dataP+3))/255); //B 
	                 p+= 3; 
	                 dataP += 4; 
	            }
	            dataP += nextLineDataWidth;
	        } 

	        break; 
		}
		else if (RGB565 == colortype)//RGB565
		{
			
		//	Linebytes = AkBmp->Width * 4; 
			iIndex = ( y * imgwidth + x ) * 2; 
		//	dataIndex = Linebytes * range->top + range->left * 4; 			
			dataWidth = (AkBmp->Width * 4); 
		//	nextLineDataWidth = (AkBmp->Width - MyWidth) * 4; 
			p = displayBuffer + iIndex; 
			endP = p + bufferWidth * MyHeight; 
			//dataP = bmpData + dataIndex + dataWidth*(AkBmp->Height-1);
			dataP = bmpData + (AkBmp->Height-1-range->top)*dataWidth + range->left * 4;
		//	endDataP = dataP + (MyWidth * 4); 
			nextLineBufferWidth = (imgwidth - MyWidth) * 2; 
			
		//	for(; p<endP; p+=nextLineBufferWidth, dataP-=2*dataWidth) 
			for(; p<endP; p+=nextLineBufferWidth, dataP-=(dataWidth+MyWidth * 4))  
			{ 
				//endDataP = dataP+dataWidth ; 
				endDataP = dataP+(MyWidth * 4);
				for(; dataP<endDataP; ) 
				{
					T_U8  rr, gg, bb, rrr, ggg, bbb;
					T_U16 temp = *((T_U16*)p);
					
					// 565 to 888
     				rrr = (T_U8)((temp>>11)<<3);
    				ggg = (T_U8)((temp>>5)<<2);
   					bbb = (T_U8)(temp<<3);
					
					// 混合
					rr = *(dataP+2) + ((T_U16)(rrr)*(255-*(dataP+3))/255);
					gg = *(dataP+1) + ((T_U16)(ggg)*(255-*(dataP+3))/255);
					bb = *(dataP) + ((T_U16)(bbb)*(255-*(dataP+3))/255);

					// 888 to 565
					*p = ((bb & 0xF8) >> 3) | ((gg & 0x1C) << 3);
					*(p+1) = (rr & 0xF8) | ((gg & 0xE0) >> 5);

					p+= 2; 
					dataP += 4; 	 
				} 
			}
			
			break;
		}
		break;
		
    case 24:            /* 24 bits */
        if( bkColor != AK_NULL )
        {
            transMode = AK_TRUE;
            AkColor2RGB( *bkColor, 24, &r, &g, &b );
			bkColor32 = RGB2AkColor( r, g, b, 16 );
        }
        else
        {
            transMode = AK_FALSE;
        }

		if(RGB888 != colortype)
		{
		    break;
        }

        Linebytes = AkBmp->Width*3;
        iIndex = ( y * imgwidth + x )*3;
        dataIndex = Linebytes*range->top + range->left*3;
        dataWidth = (AkBmp->Width*3);
        nextLineDataWidth = (AkBmp->Width-MyWidth)*3;
        p = displayBuffer + iIndex;
        endP = p + bufferWidth*MyHeight;
        dataP = bmpData + dataIndex;
        endDataP = dataP + (MyWidth*3);
        nextLineBufferWidth = (imgwidth-MyWidth)*3;

        if (Reverse)
        {
            if( transMode )
            {
                for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
                {
                    for(; dataP<endDataP; )
                    {
                        if( r==*(dataP) && g==*(dataP+1) && b==*(dataP+2))
                        {
                            p += 3;
                            dataP += 3;
                        }
                        else
                        {                        
                            *p++ = ~(*dataP++);
                            *p++ = ~(*dataP++);
                            *p++ = ~(*dataP++);
                        }
                    }
                }
            }
            else
            {
                for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
                {
                    for(; dataP<endDataP; )
                    {
                        *p++ = ~(*dataP++);
                        *p++ = ~(*dataP++);
                        *p++ = ~(*dataP++);
                    }
                }
            }
        }
        else
        {
            if( transMode )
            {
                for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
                {
                    for(; dataP<endDataP; )
                    {
                        if( r==*(dataP) && g==*(dataP+1) && b==*(dataP+2))
                        {
                            p += 3;
                            dataP += 3;
                        }
                        else
                        {                        
                            *p++ = (*dataP++);
                            *p++ = (*dataP++);
                            *p++ = (*dataP++);
                        }
                    }
                }
            }
            else
            {
                for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
                {
                    for(; dataP<endDataP; )
                    {
                        *p++ = (*dataP++);
                        *p++ = (*dataP++);
                        *p++ = (*dataP++);
                    }
                }
            }
        }
        break;
		
    case 16:
        if( bkColor != AK_NULL )
        {
            transMode = AK_TRUE;
            AkColor2RGB( *bkColor, 24, &r, &g, &b );
            bkColor16 = (T_U16)RGB2AkColor( r, g, b, 16 );
        }
        else
        {
            transMode = AK_FALSE;
        }

		if (RGB888 == colortype)
		{
	        Linebytes = AkBmp->Width << 1;
	        iIndex = ( y * imgwidth + x )*3;
	        dataIndex = Linebytes*range->top + (range->left<<1);
	        dataWidth = (AkBmp->Width<<1);
	        nextLineDataWidth = (AkBmp->Width-MyWidth)<<1;
	        p = displayBuffer + iIndex;
	        endP = p + bufferWidth*MyHeight;
	        dataP = bmpData + dataIndex;
	        endDataP = dataP + (MyWidth<<1);    //MyWidth*2
	        nextLineBufferWidth = (imgwidth-MyWidth)*3;

	        if (Reverse)
	        {
	            if( transMode )
	            {
	                bkColor16 = ~bkColor16;
	            
	                if(0 == (((T_U32)dataP)%(sizeof(T_U16))))
	                {
	                    //read source data 16bit per one loop, (more quickly)
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = ~(*((T_U16*)dataP));
	                            if(bkColor16 == temp)
	                            {
	                                p += 3;
	                            }
	                            else
	                            {                        
	                                *p++ = (temp<<3);
	                                *p++ = ((temp>>5)<<3);
	                                *p++ = ((temp>>10)<<3);
	                            }
	                        }
	                    }
	                }
	                else
	                {
	                    //read source data 8bit per one loop
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = ~(*dataP | (*(dataP+1)<<8));
	                            if(bkColor16 == temp)
	                            {
	                                p += 3;
	                            }
	                            else
	                            {                        
	                                *p++ = (temp<<3);
	                                *p++ = ((temp>>5)<<3);
	                                *p++ = ((temp>>10)<<3);
	                            }
	                        }
	                    }
	                }
	            }
	            else
	            {
	                
	                if(0 == (((T_U32)dataP)%(sizeof(T_U16))))
	                {
	                    //read source data 16bit per one loop, (more quickly)
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = ~(*((T_U16*)dataP));
	                            *p++ = (temp<<3);
	                            *p++ = ((temp>>5)<<3);
	                            *p++ = ((temp>>10)<<3);
	                        }
	                    }
	                }
	                else
	                {
	                    //read source data 8bit per one loop

	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = ~(*dataP | (*(dataP+1)<<8));
	                            *p++ = (temp<<3);
	                            *p++ = ((temp>>5)<<3);
	                            *p++ = ((temp>>10)<<3);
	                        }
	                    }
	            
	                }
	            }
	        }
	        else
	        {
	            if( transMode )
	            {
	                if(0 == (((T_U32)dataP)%(sizeof(T_U16))))
	                {
	                    //read source data 16bit per one loop, (more quickly)
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = *((T_U16*)dataP);
	                            if(bkColor16 == temp)
	                            {
	                                p += 3;
	                            }
	                            else
	                            {                        
	                                *p++ = (temp<<3);
	                                *p++ = ((temp>>5)<<3);
	                                *p++ = ((temp>>10)<<3);
	                            }
	                        }
	                    }
	                }
	                else
	                {
	                    //read source data 8bit per one loop
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = *dataP | (*(dataP+1)<<8);
	                            if(bkColor16 == temp)
	                            {
	                                p += 3;
	                            }
	                            else
	                            {                        
	                                *p++ = (temp<<3);
	                                *p++ = ((temp>>5)<<3);
	                                *p++ = ((temp>>10)<<3);
	                            }
	                        }
	                    }
	                }
	            }
	            else
	            {
	                if(0 == (((T_U32)dataP)%(sizeof(T_U16))))
	                {
	                    //read source data 16bit per one loop, (more quickly)
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = *((T_U16*)dataP);
	                            *p++ = (temp<<3);
	                            *p++ = ((temp>>5)<<3);
	                            *p++ = ((temp>>10)<<3);
	                        }
	                    }
	                }
	                else
	                {
	                    //read source data 8bit per one loop
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = *dataP | (*(dataP+1)<<8);
	                            *p++ = (temp<<3);
	                            *p++ = ((temp>>5)<<3);
	                            *p++ = ((temp>>10)<<3);
	                        }
	                    }

	                }
	            }
	        }
	        break;
    	}
		else if (RGB565 == colortype)
		{
			T_U16 *p16 = AK_NULL;
			T_U8  red, green, blue;

			Linebytes = AkBmp->Width*2;
			iIndex = ( y * imgwidth + x )*2;
			dataIndex = Linebytes*range->top + range->left*2;
			dataWidth = (AkBmp->Width*2);
			nextLineDataWidth = (AkBmp->Width-MyWidth)*2;
			p = displayBuffer + iIndex;
			endP = p + bufferWidth*MyHeight;
			dataP = bmpData + dataIndex;
			endDataP = dataP + (MyWidth*2);
			nextLineBufferWidth = (imgwidth-MyWidth)*2;

			if(Reverse)
			{
				if(transMode)
				{						
					for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
					{
						for(; dataP<endDataP; )
						{
							p16 = (T_U16 *)dataP;
							
							if(*p16 == bkColor16)
							{
								p += 2;
								dataP += 2;
							}
							else
							{
								red = ~(*p16<<3);
								green = ~((*p16>>5)<<2);
								blue = ~((*p16>>11)<<3);

								*p++ = ((green & 0x1C) << 3) | ((blue & 0xF8) >> 3);
								*p++ = (red & 0xF8) | ((green & 0xE0) >> 5);

								dataP += 2;
							}
						}
					}
				}
				else
				{
					for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
					{
						for(; dataP<endDataP; )
						{
							p16 = (T_U16 *)dataP;
							
							red = ~(*p16<<3);
							green = ~((*p16>>5)<<2);
							blue = ~((*p16>>11)<<3);
							
							*p++ = ((green & 0x1C) << 3) | ((blue & 0xF8) >> 3);
							*p++ = (red & 0xF8) | ((green & 0xE0) >> 5);
							
							dataP += 2;
						}
					}
				}
			}
			else
			{
				if(transMode)
				{
					for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
					{
						for(; dataP<endDataP; )
						{
							p16 = (T_U16 *)dataP;
							
							if(*p16 == bkColor16)
							{
								p += 2;
								dataP += 2;
							}
							else
							{	
								*p++ = (*dataP++);
								*p++ = (*dataP++);
							}
						}
					}
				}
				else
				{					
					for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
					{
						for(; dataP<endDataP; )
						{
							*p++ = (*dataP++);
							*p++ = (*dataP++);
						}
					}
				}
			}
			
			break;
		}
		break;
		
    case 12:        /* 12 bits, 4096 colors */
        if (bkColor != AK_NULL)
        {
            //bkColor24 = ...;
        }
        if (g_Graph.LCDCOLOR[DISPLAY_LCD_0] == 12)
        {
            for (j = 0; j < MyHeight; ++j)
            {
                curBmpData = bmpData + (AkBmp->Width*2) * (j+range->top) + (range->left*2);
                for (i = 0; i < MyWidth; ++i, curBmpData += 2)
                {
                    color = *(curBmpData) * 0x100 + *(curBmpData+1);
                    if (color != *bkColor)
                    {
                        if (Reverse)
                        {
                            Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x + i), (T_POS)(y + j), 
								(T_COLOR)~color, colortype);
                        }
                        else
                        {
                            Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x + i), (T_POS)(y + j), 
								color, colortype);
                        }
                    }
                }
            }
        }
        else
        {
            T_U8        rgbR, rgbG, rgbB;
            for (j = 0; j < MyHeight; ++j)
            {
                curBmpData = bmpData + (AkBmp->Width*2) * (j+range->top) + (range->left*2);
                for (i = 0; i < MyWidth; ++i, curBmpData += 2)
                {
                    AkColor2RGB((T_COLOR)(*(curBmpData) * 0x100 + *(curBmpData+1)), 12, &rgbR, &rgbG, &rgbB);
                    color = RGB2AkColor(rgbR, rgbG, rgbB, (T_U8)g_Graph.LCDCOLOR[DISPLAY_LCD_0]);
                    if (color != *bkColor)
                    {
                        if (Reverse)
                        {
                            Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x + i), (T_POS)(y + j), 
								(T_COLOR)~color, colortype);
                        }
                        else
                        {
                            Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x + i), (T_POS)(y + j), 
								color, colortype);
                        }
                    }
                }
            }
        }
        break;
		
    case 1:        /* 1 bit, black and white */
        {
            T_U16    BytesPerLine = ((AkBmp->Width+7)>>3);
            T_U16    PointX, MaxX;

            MaxX = x + MyWidth;
            curBmpData = bmpData;

            if( Reverse )
            {
                for ( j=0; j<AkBmp->Height; j++, curBmpData += BytesPerLine )
                {
                    focus = curBmpData;
                    PointX = x;
                    for ( i=0; i<BytesPerLine; i++, focus++ )
                    {
                        for ( k=0; k<8; k++, PointX++ )
                        {
                            if( PointX >= MaxX )
                            {
                                /* do not draw the left pixels of the current line,
                                   and continue to draw the next line */
                                i = 0x6FFF;
                                break;
                            }
                            if (*focus & cMask[k])
                            {
                                Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, PointX, (T_POS)(y + j), 
									g_Graph.WinBkCL[DISPLAY_LCD_0], colortype);
                            }
                        }
                    }
                }
            }
            else
            {
                for ( j=0; j<AkBmp->Height; j++, curBmpData += BytesPerLine )
                {
                    focus = curBmpData;
                    PointX = x;
                    for ( i=0; i<BytesPerLine; i++, focus++ )
                    {
                        for ( k=0; k<8; k++, PointX++ )
                        {
                            if( PointX >= MaxX )
                            {
                                /* do not draw the left pixels of the current line,
                                   and continue to draw the next line */
                                i = 0x6FFF;
                                break;
                            }
                            if (*focus & cMask[k])
                            {
                                Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, PointX, (T_POS)(y + j), 
									g_Graph.WinFrCL[DISPLAY_LCD_0], colortype);
                            }
                        }
                    }
                }
            }
        }
        break;
		
    default:
        Fwl_Print(C3, M_DISPLAY, "Fwl_AkBmpDrawPartOnRGB error! AkBmp->Deep = %d.", AkBmp->Deep);
        return AK_FALSE;
    }

#ifdef OS_WIN32
#ifdef LCD_MODE_565
	if (((T_pImgLay)Fwl_hRGBLayer)->pData == buf)
	{	
		Fwl_RGB565toRGB888(Fwl_GetDispMemory(), displayBuffer, MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT);
		Fwl_Free(displayBuffer);
	}
	else
	{
		Fwl_RGB565toRGB888(buf, displayBuffer, imgwidth, imgheight);
		Fwl_Free(displayBuffer);
	}
#endif
#endif
    return AK_TRUE;
}

/**
 * @brief Draw AKBmp format bitmap in part on RGB
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-29
 * @param[in]*buf:
 * @param[in] imgwidth:max width
 * @param[in] imgheight:max height
 * @param[in] x		x position the AKBmp would be drawn 
 * @param[in] y		y position the AKBmp would be drawn
 * @param[in] range	The part rect of AKBmp would be drawn, the range rect coordinate relative to the left-top of AKBmp image
 * @param[in] AkBmp	The source AKbmp would be drawn.
 * @param[in] bkColor	The transparent color.If a dot with this color appears in bmp, it will not be drawn.
 * @param[in] Reverse	if AK_TRUE, reverse draw the source pixel
 * @param[in] colortype:RGB888 or RGB565
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @note if range, color, reverse are AK_NULL, AK_NULL, AK_FALSE, this function would run most quickly
 * @retval
 */
T_BOOL Fwl_AkBmpDrawAlphaPartOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight,
	T_POS x, T_POS y, T_RECT *range, const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse, T_U8 colortype)
{
    T_LEN	i = 0;
	T_LEN	j = 0;
	T_LEN	k = 0;
	T_U8	r = 0;
	T_U8	g = 0;
	T_U8	b = 0;
    T_LEN	MyWidth = 0;
	T_LEN	MyHeight = 0;
    T_COLOR	color = 0;
    T_U8	cMask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
    T_U32	Linebytes = 0;
	T_U32	bufferWidth = 0;
    T_U32	iIndex = 0;
	T_U32	dataIndex = 0;
    T_U16	bkColor16 = 0;
	T_U32	bkColor32 = 0;
    T_U16	temp = 0;
    T_BOOL	transMode = 0;
    T_U32	dataWidth = 0;
	T_U8	*bmpData = AK_NULL;
    T_U8	*curBmpData = AK_NULL;
	T_U8	*displayBuffer = AK_NULL;
	T_U8	*p = AK_NULL;
    T_U8	*endP = AK_NULL;
    T_U8	*dataP = AK_NULL;
    T_U8	*endDataP = AK_NULL;
    T_U8	*focus = AK_NULL;
    T_U32	nextLineDataWidth = 0;
    T_U32	nextLineBufferWidth = 0;
    T_RECT	rangeWhenNull;

    Reverse = Reverse; // avoid warning

    AK_ASSERT_PTR(AkBmp, "Fwl_AkBmpDrawPartOnRGB():AkBmp==NULL", AK_FALSE);

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_AkBmpDrawPartOnRGB AK_NULL == buf!");
		return AK_FALSE;
	}
	
	

    if (x >= (T_LEN)imgwidth || y >= (T_LEN)imgheight)
    { 
        Fwl_Print(C3, M_DISPLAY, "x or y out of range, x = %d, y = %d", x, y);
        return AK_FALSE;
    }

	
    if(AK_NULL == range)
    {
        RectInit(&rangeWhenNull, 0, 0, AkBmp->Width, AkBmp->Height);
        range = &rangeWhenNull;
    }
    if (range->left >= AkBmp->Width || range->top >= AkBmp->Height)
    {
        return AK_FALSE;
    }

    bmpData = (T_U8 *)AkBmp->BmpData;
	
#ifdef OS_ANYKA
    displayBuffer = buf;
#else
	displayBuffer = Fwl_GetDispMemory();
#endif
	if (RGB888 == colortype)
	{
    	bufferWidth = imgwidth * 3;
	}
	else if (RGB565 == colortype)
	{
		bufferWidth = imgwidth * 2;
	}
	else
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_AkBmpDrawPartOnRGB(): Data Format Is NOT RGB888/565.");
		return AK_FALSE;
	}

    if (range->left + range->width > AkBmp->Width)
    {
        range->width = AkBmp->Width - range->left;
    }
	
    if (range->top + range->height > AkBmp->Height)
    {
        range->height = AkBmp->Height - range->top;
    }

    if (x + AkBmp->Width > (T_LEN)imgwidth)
    {
        MyWidth = (T_LEN)(imgwidth - x);
    }
    else
    {
        MyWidth = AkBmp->Width;
    }

    if (MyWidth > range->width)
    {
        MyWidth = range->width;
    }

    if (y + AkBmp->Height > (T_LEN)imgheight)
    {
        MyHeight = (T_LEN)(imgheight - y);
    }
    else
    {
        MyHeight = AkBmp->Height;
    }

    if (MyHeight > range->height)
    {
        MyHeight = range->height;
    }
    


    switch (AkBmp->Deep) {

    case 32:   /* 32 bits deep */ 
		if(RGB888 == colortype)
		{
	        Linebytes = AkBmp->Width * 4; 
	        iIndex = ( y * imgwidth + x ) * 3; 
	        dataIndex = Linebytes * range->top + range->left * 4; 
	        dataWidth = (AkBmp->Width * 4); 
	        nextLineDataWidth = (AkBmp->Width - MyWidth) * 4; 
	        p = displayBuffer + iIndex; 
	        endP = p + bufferWidth * MyHeight; 
	        dataP = bmpData + dataIndex + dataWidth*(AkBmp->Height-1); 
	        endDataP = dataP + (MyWidth * 4); 
	        nextLineBufferWidth = (imgwidth - MyWidth) * 3; 

	        for(; p<endP; p+=nextLineBufferWidth, dataP-=2*dataWidth) 
	        { 
	            endDataP = dataP+ (MyWidth * 4); 
	            for(; dataP<endDataP; ) 
	            { 
	                 *p = *(dataP+2) + ((T_U16)(*p)*(255-*(dataP+3))/255); //R 
	                 *(p+1) = *(dataP+1) + ((T_U16)(*(p+1))*(255-*(dataP+3))/255); //G 
	                 *(p+2) = *(dataP) + ((T_U16)(*(p+2))*(255-*(dataP+3))/255); //B 
	                 p+= 3; 
	                 dataP += 4; 
	            }
	            dataP += nextLineDataWidth;
	        } 

	        break; 
		}
		else if (RGB565 == colortype)//RGB565
		{			
		//	Linebytes = AkBmp->Width * 4; 
			iIndex = ( y * imgwidth + x ) * 2; 
		//	dataIndex = Linebytes * range->top + range->left * 4; 			
			dataWidth = (AkBmp->Width * 4); 
		//	nextLineDataWidth = (AkBmp->Width - MyWidth) * 4; 
			p = displayBuffer + iIndex; 
			endP = p + bufferWidth * MyHeight; 
			//dataP = bmpData + dataIndex + dataWidth*(AkBmp->Height-1);
			dataP = bmpData + (AkBmp->Height-1-range->top)*dataWidth + range->left * 4;
		//	endDataP = dataP + (MyWidth * 4); 
			nextLineBufferWidth = (imgwidth - MyWidth) * 2; 
			
		//	for(; p<endP; p+=nextLineBufferWidth, dataP-=2*dataWidth) 
			for(; p<endP; p+=nextLineBufferWidth, dataP-=(dataWidth+MyWidth * 4))  
			{ 
				//endDataP = dataP+dataWidth ; 
				endDataP = dataP+(MyWidth * 4);
				for(; dataP<endDataP; ) 
				{
					T_U8  rr, gg, bb, rrr, ggg, bbb;
					T_U16 temp = *((T_U16*)p);
					
					// 565 to 888
     				rrr = (T_U8)((temp>>11)<<3);
    				ggg = (T_U8)((temp>>5)<<2);
   					bbb = (T_U8)(temp<<3);
					
					// 混合
					rr = *(dataP+2) + ((T_U16)(rrr)*(255-*(dataP+3))/255);
					gg = *(dataP+1) + ((T_U16)(ggg)*(255-*(dataP+3))/255);
					bb = *(dataP) + ((T_U16)(bbb)*(255-*(dataP+3))/255);

					// 888 to 565
					*p = ((bb & 0xF8) >> 3) | ((gg & 0x1C) << 3);
					*(p+1) = (rr & 0xF8) | ((gg & 0xE0) >> 5);

					p+= 2; 
					dataP += 4; 	 
				} 
			}
			
			break;
		}
		break;
    case 24:            /* 24 bits */
        if( bkColor != AK_NULL )
        {
            transMode = AK_TRUE;
            AkColor2RGB( *bkColor, 24, &r, &g, &b );
			bkColor32 = RGB2AkColor( r, g, b, 16 );
        }
        else
        {
            transMode = AK_FALSE;
        }

		if(RGB888 != colortype)
		{
		    break;
        }

        Linebytes = AkBmp->Width*3;
        iIndex = ( y * imgwidth + x )*3;
        dataIndex = Linebytes*range->top + range->left*3;
        dataWidth = (AkBmp->Width*3);
        nextLineDataWidth = (AkBmp->Width-MyWidth)*3;
        p = displayBuffer + iIndex;
        endP = p + bufferWidth*MyHeight;
        dataP = bmpData + dataIndex;
        endDataP = dataP + (MyWidth*3);
        nextLineBufferWidth = (imgwidth-MyWidth)*3;

        {
            if( transMode )
            {
                for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
                {
                    for(; dataP<endDataP; )
                    {
                        if( r==*(dataP) && g==*(dataP+1) && b==*(dataP+2))
                        {
                            p += 3;
                            dataP += 3;
                        }
                        else
                        {                        
                            *p = (*p/2) + ((*dataP++)/2); p++;
                            *p = (*p/2) + ((*dataP++)/2); p++;
                            *p = (*p/2) + ((*dataP++)/2); p++;
                        }
                    }
                }
            }
            else
            {
                for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
                {
                    for(; dataP<endDataP; )
                    {
                        *p = (*p/2) + ((*dataP++)/2); p++;
                        *p = (*p/2) + ((*dataP++)/2); p++;
                        *p = (*p/2) + ((*dataP++)/2); p++;
                    }
                }
            }
        }
        break;
    case 16:
        if( bkColor != AK_NULL )
        {
            transMode = AK_TRUE;
            AkColor2RGB( *bkColor, 24, &r, &g, &b );
            bkColor16 = (T_U16)RGB2AkColor( r, g, b, 16 );
        }
        else
        {
            transMode = AK_FALSE;
        }

		if (RGB888 == colortype)
		{
	        Linebytes = AkBmp->Width << 1;
	        iIndex = ( y * imgwidth + x )*3;
	        dataIndex = Linebytes*range->top + (range->left<<1);
	        dataWidth = (AkBmp->Width<<1);
	        nextLineDataWidth = (AkBmp->Width-MyWidth)<<1;
	        p = displayBuffer + iIndex;
	        endP = p + bufferWidth*MyHeight;
	        dataP = bmpData + dataIndex;
	        endDataP = dataP + (MyWidth<<1);    //MyWidth*2
	        nextLineBufferWidth = (imgwidth-MyWidth)*3;

	        {
	            if( transMode )
	            {
	                if(0 == (((T_U32)dataP)%(sizeof(T_U16))))
	                {
	                    //read source data 16bit per one loop, (more quickly)
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = *((T_U16*)dataP);
	                            if(bkColor16 == temp)
	                            {
	                                p += 3;
	                            }
	                            else
	                            {                        
	                                *p = *p/2+(temp<<3)/2; p++;
	                                *p = *p/2+((temp>>5)<<3)/2;p++;
	                                *p = *p/2+((temp>>10)<<3)/2;p++;
	                            }
	                        }
	                    }
	                }
	                else
	                {
	                    //read source data 8bit per one loop
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = *dataP | (*(dataP+1)<<8);
	                            if(bkColor16 == temp)
	                            {
	                                p += 3;
	                            }
	                            else
	                            {                        
	                                *p = *p/2+(temp<<3)/2; p++;
	                                *p = *p/2+((temp>>5)<<3)/2;p++;
	                                *p = *p/2+((temp>>10)<<3)/2;p++;
	                            }
	                        }
	                    }
	                }
	            }
	            else
	            {
	                if(0 == (((T_U32)dataP)%(sizeof(T_U16))))
	                {
	                    //read source data 16bit per one loop, (more quickly)
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = *((T_U16*)dataP);
	                            *p = *p/2+(temp<<3)/2; p++;
	                            *p = *p/2+((temp>>5)<<3)/2;p++;
	                            *p = *p/2+((temp>>10)<<3)/2;p++;
	                        }
	                    }
	                }
	                else
	                {
	                    //read source data 8bit per one loop
	                    for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
	                    {
	                        for(; dataP<endDataP; dataP+=2)
	                        {
	                            temp = *dataP | (*(dataP+1)<<8);
	                            *p = *p/2+(temp<<3)/2; p++;
	                            *p = *p/2+((temp>>5)<<3)/2;p++;
	                            *p = *p/2+((temp>>10)<<3)/2;p++;
	                        }
	                    }

	                }
	            }
	        }
	        break;
    	}
		else if (RGB565 == colortype)
		{
			T_U16 *p16 = AK_NULL;
			T_U16 *m16 = AK_NULL;
			T_U8  red, green, blue;		// 新数据
			T_U8  red1, green1, blue1;	// 旧数据

			Linebytes = AkBmp->Width*2;
			iIndex = ( y * imgwidth + x )*2;
			dataIndex = Linebytes*range->top + range->left*2;
			dataWidth = (AkBmp->Width*2);
			nextLineDataWidth = (AkBmp->Width-MyWidth)*2;
			p = displayBuffer + iIndex;
			endP = p + bufferWidth*MyHeight;
			dataP = bmpData + dataIndex;
			endDataP = dataP + (MyWidth*2);
			nextLineBufferWidth = (imgwidth-MyWidth)*2;

			{
				if(transMode)
				{
					for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
					{
						for(; dataP<endDataP; )
						{
							p16 = (T_U16 *)dataP;
							
							if(*p16 == bkColor16)
							{
								p += 2;
								dataP += 2;
							}
							else
							{	

								m16 = (T_U16 *)p;
							
								red   = (*p16 & 0x001f);
								green = (*p16>> 5) & 0x003f;
								blue  = (*p16>>11) & 0x001f;
								
								red1   = (*m16 & 0x001f);
								green1 = (*m16>> 5) & 0x003f;
								blue1  = (*m16>>11) & 0x001f;


								green = (green>>1) + (green1>>1);
								blue = (blue>>1) + (blue1>>1);
								red = (red>>1) + (red1>>1);
								
								*p++ = (green<<5) | (blue);
								*p++ = (red<<3) | (green>>3);

								dataP += 2;
							}
						}
					}
				}
				else
				{					
					for(; p<endP; p+=nextLineBufferWidth, dataP+=nextLineDataWidth, endDataP+=dataWidth)
					{
						for(; dataP<endDataP; )
						{
							p16 = (T_U16 *)dataP;
							m16 = (T_U16 *)p;
							
							red   = (*p16 & 0x001f);
							green = (*p16>> 5) & 0x003f;
							blue  = (*p16>>11) & 0x001f;
							
							red1   = (*m16 & 0x001f);
							green1 = (*m16>> 5) & 0x003f;
							blue1  = (*m16>>11) & 0x001f;


							green = (green>>1) + (green1>>1);
							blue = (blue>>1) + (blue1>>1);
							red = (red>>1) + (red1>>1);
							
							*p++ = (green<<5) | (blue);
							*p++ = (red<<3) | (green>>3);

							dataP += 2;

						}
					}
				}
			}
			
			break;
		}
		break;
    case 12:        /* 12 bits, 4096 colors */
        if (bkColor != AK_NULL)
        {
            //bkColor24 = ...;
        }
        if (g_Graph.LCDCOLOR[LCD_0] == 12)
        {
            for (j = 0; j < MyHeight; ++j)
            {
                curBmpData = bmpData + (AkBmp->Width*2) * (j+range->top) + (range->left*2);
                for (i = 0; i < MyWidth; ++i, curBmpData += 2)
                {
                    color = *(curBmpData) * 0x100 + *(curBmpData+1);
                    if (color != *bkColor)
                    {
                        {
                            Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x + i), (T_POS)(y + j), 
								color, colortype);
                        }
                    }
                }
            }
        }
        else
        {
            T_U8        rgbR, rgbG, rgbB;
            for (j = 0; j < MyHeight; ++j)
            {
                curBmpData = bmpData + (AkBmp->Width*2) * (j+range->top) + (range->left*2);
                for (i = 0; i < MyWidth; ++i, curBmpData += 2)
                {
                    AkColor2RGB((T_COLOR)(*(curBmpData) * 0x100 + *(curBmpData+1)), 12, &rgbR, &rgbG, &rgbB);
                    color = RGB2AkColor(rgbR, rgbG, rgbB, (T_U8)g_Graph.LCDCOLOR[LCD_0]);
                    if (color != *bkColor)
                    {
                        {
                            Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, (T_POS)(x + i), (T_POS)(y + j), 
								color, colortype);
                        }
                    }
                }
            }
        }
        break;
    case 1:        /* 1 bit, black and white */
        {
            T_U16    BytesPerLine = ((AkBmp->Width+7)>>3);
            T_U16    PointX, MaxX;

            MaxX = x + MyWidth;
            curBmpData = bmpData;

            {
                for ( j=0; j<AkBmp->Height; j++, curBmpData += BytesPerLine )
                {
                    focus = curBmpData;
                    PointX = x;
                    for ( i=0; i<BytesPerLine; i++, focus++ )
                    {
                        for ( k=0; k<8; k++, PointX++ )
                        {
                            if( PointX >= MaxX )
                            {
                                /* do not draw the left pixels of the current line,
                                   and continue to draw the next line */
                                i = 0x6FFF;
                                break;
                            }
                            if (*focus & cMask[k])
                            {
                                Fwl_SetPixelOnRGB(buf, imgwidth, imgheight, PointX, (T_POS)(y + j), 
									g_Graph.WinFrCL[LCD_0], colortype);
                            }
                        }
                    }
                }
            }
        }
        break;
    default:
        Fwl_Print(C3, M_DISPLAY, "Fwl_AkBmpDrawPartOnRGB error! AkBmp->Deep = %d", AkBmp->Deep);
        return AK_FALSE;
    }

    return AK_TRUE;
}


/**
 * @brief Draw BMP from BMP data string partly on RGB.
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-09-03
 * @param[in]*buf:
 * @param[in] imgwidth:max width
 * @param[in] imgheight:max height
 * @param[in] x		x position the AKBmp would be drawn 
 * @param[in] y		y position the AKBmp would be drawn
 * @param[in] range	The part rect of AKBmp would be drawn, the range rect coordinate relative to the left-top of AKBmp image
 * @param[in] bmpStream	A string stores bmp data info.
 * @param[in] bkColor	The transparent color.If a dot with this color appears in bmp, it will not be drawn.
 * @param[in] Reverse	if AK_TRUE, reverse draw the source pixel
 * @param[in] colortype:RGB888 or RGB565
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @retval 
 */
T_BOOL Fwl_AkBmpDrawPartFromStringOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, 
	T_POS x, T_POS y, T_RECT *range, T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse, T_U8 colortype)
{
    T_AK_BMP    AnykaBmp;

    AK_ASSERT_PTR(BmpStream, "Fwl_Fwl_AkBmpDrawPartFromStringOnRGB(): BmpStream", AK_FALSE);

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_Fwl_AkBmpDrawPartFromStringOnRGB AK_NULL == buf!");
		return AK_FALSE;
	}
    
    AkBmpGetFromString(BmpStream, &AnykaBmp);
    return Fwl_AkBmpDrawPartOnRGB(buf, imgwidth, imgheight, x, y, range, &AnykaBmp, bkColor, Reverse, colortype);
}


/**
 * @brief Draw BMP from BMP data string on RGB.
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-09-03
 * @param[in]*buf:
 * @param[in] imgwidth:max width
 * @param[in] imgheight:max height
 * @param[in] x		x position the AKBmp would be drawn 
 * @param[in] y		y position the AKBmp would be drawn
 * @param[in] bmpStream	A string stores bmp data info.
 * @param[in] bkColor	The transparent color.If a dot with this color appears in bmp, it will not be drawn.
 * @param[in] Reverse	if AK_TRUE, reverse draw the source pixel
 * @param[in] colortype:RGB888 or RGB565
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @retval 
 */
T_BOOL Fwl_AkBmpDrawFromStringOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, 
	T_POS x, T_POS y, T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse, T_U8 colortype)
{
    T_AK_BMP    AnykaBmp;

    AK_ASSERT_PTR(BmpStream, "Fwl_AkBmpDrawFromStringOnRGB(): BmpStream", AK_FALSE);

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_AkBmpDrawFromStringOnRGB AK_NULL == buf!");
		return AK_FALSE;
	}
    
    AkBmpGetFromString(BmpStream, &AnykaBmp);
    return Fwl_AkBmpDrawPartOnRGB(buf, imgwidth, imgheight, x, y, AK_NULL, &AnykaBmp, bkColor, Reverse, colortype);
    
}


T_BOOL Fwl_AkBmpDrawAlphaFromStringOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, 
	T_POS x, T_POS y, T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse, T_U8 colortype)
{
    T_AK_BMP    AnykaBmp;

    AK_ASSERT_PTR(BmpStream, "Fwl_AkBmpDrawFromStringOnRGB(): BmpStream", AK_FALSE);

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_AkBmpDrawFromStringOnRGB AK_NULL == buf!");
		return AK_FALSE;
	}
    
    AkBmpGetFromString(BmpStream, &AnykaBmp);
    return Fwl_AkBmpDrawAlphaPartOnRGB(buf, imgwidth, imgheight, x, y, AK_NULL, &AnykaBmp, bkColor, Reverse, colortype);
    
}



/**
 * @brief Draw AKBmp format bitmap in part on YUV
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-29
 * @param[in]*ybuf,*ubuf,*vbuf:
 * @param[in] imgwidth:max width, must be even number
 * @param[in] imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param[in] x		x position the AKBmp would be drawn 
 * @param[in] y		y position the AKBmp would be drawn
 * @param[in] range	The part rect of AKBmp would be drawn, the range rect coordinate relative to the left-top of AKBmp image
 * @param[in] AkBmp	The source AKbmp would be drawn.
 * @param[in] bkColor	The transparent color.If a dot with this color appears in bmp, it will not be drawn.
 * @param[in] Reverse	if AK_TRUE, reverse draw the source pixel
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @note if range, color, reverse are AK_NULL, AK_NULL, AK_FALSE, this function would run most quickly
 * @retval
 */
T_BOOL Fwl_AkBmpDrawPartOnYUV (T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
	T_POS x, T_POS y, T_RECT *range, const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse)
{
	return AK_FALSE;
}


/**
 * @brief Draw BMP from BMP data string partly on YUV.
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-09-03
 * @param[in]*ybuf,*ubuf,*vbuf:
 * @param[in] imgwidth:max width, must be even number
 * @param[in] imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param[in] x		x position the AKBmp would be drawn 
 * @param[in] y		y position the AKBmp would be drawn
 * @param[in] range	The part rect of AKBmp would be drawn, the range rect coordinate relative to the left-top of AKBmp image
 * @param[in] bmpStream	A string stores bmp data info.
 * @param[in] bkColor	The transparent color.If a dot with this color appears in bmp, it will not be drawn.
 * @param[in] Reverse	if AK_TRUE, reverse draw the source pixel
 * @param[in] colortype:RGB888 or RGB565
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @retval 
 */
T_BOOL Fwl_AkBmpDrawPartFromStringOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
    T_POS x, T_POS y, T_RECT *range, T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse)
{
	T_AK_BMP    AnykaBmp;

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_Fwl_AkBmpDrawPartFromStringOnYUV AK_NULL == buf!");
		return AK_FALSE;
	}

    AK_ASSERT_PTR(BmpStream, "Fwl_AkBmpDrawPartFromStringOnYUV(): BmpStream", AK_FALSE);
    
    AkBmpGetFromString(BmpStream, &AnykaBmp);
    return Fwl_AkBmpDrawPartOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x, y, range, &AnykaBmp, bkColor, Reverse);
}


/**
 * @brief Draw BMP from BMP data string on YUV.
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-09-03
 * @param[in]*ybuf,*ubuf,*vbuf:
 * @param[in] imgwidth:max width, must be even number
 * @param[in] imgheight:max height, if (yuv 4:2:0),must be even number too
 * @param[in] x		x position the AKBmp would be drawn 
 * @param[in] y		y position the AKBmp would be drawn
 * @param[in] bmpStream	A string stores bmp data info.
 * @param[in] bkColor	The transparent color.If a dot with this color appears in bmp, it will not be drawn.
 * @param[in] Reverse	if AK_TRUE, reverse draw the source pixel
 * @param[in] colortype:RGB888 or RGB565
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @retval 
 */
T_BOOL Fwl_AkBmpDrawFromStringOnYUV (T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
	T_POS x, T_POS y, T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse)
{
	T_AK_BMP	AnykaBmp;

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY, "Fwl_AkBmpDrawFromStringOnYUV AK_NULL == buf!");
		return AK_FALSE;
	}
	
	AK_ASSERT_PTR(BmpStream, "Fwl_AkBmpDrawFromStringOnYUV(): BmpStream", AK_FALSE);
	
	AkBmpGetFromString(BmpStream, &AnykaBmp);
	return Fwl_AkBmpDrawPartOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x, y, AK_NULL, &AnykaBmp, bkColor, Reverse);
}


T_S32 Fwl_Scale_ConvertWithAlpha(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
                 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
                 T_VOID *srcBuf, T_U16 srcWidth, T_U16 srcHeight, T_U8 srcFormat,T_U8 alpha)
{
	T_U8 ret = AK_FALSE;
	E_ImageFormat format_in;
    T_VOID *pBuf565 = AK_NULL; 

	T_RECT srcWin;
#ifdef OS_ANYKA
	T_RECT dstWin;
#endif

	srcWin.left 	= 0;
	srcWin.top 		= 0;
	srcWin.width 	= srcWidth;
	srcWin.height 	= srcHeight;

	srcWin.left 	= dstPosX;
	srcWin.top 		= dstPosY;
	srcWin.width 	= scaleWidth;
	srcWin.height 	= scaleHeight;
	
	if ((scaleWidth>1280) || (srcWidth>1280) || (dstWidth>1280) || (scaleHeight>1024) || (srcHeight>1024)
		|| (scaleWidth<18) || (srcWidth<18) || (dstWidth<18) || (scaleHeight<18) || (srcHeight<18))
    {
    	Fwl_Print(C3, M_DISPLAY, "Fwl_Scale_Convert size not support!");		
		return AK_FALSE;
	}
	
	switch(srcFormat)
	{
	case RGB565:
		format_in = FORMAT_RGB565;
		pBuf565 = srcBuf;
		break;
		
	case RGB888:
	    {
            if (AK_NULL == pBuf565)
           	{
				pBuf565 = Fwl_Malloc(srcWidth*srcHeight*2);
				if (AK_NULL == pBuf565)
				{
					Fwl_Print(C3, M_DISPLAY, "Fwl_Scale_Convert:malloc is fail");
					return AK_FALSE;
				}
			}
			Fwl_RGB888toRGB565(pBuf565,srcBuf,srcWidth,srcHeight);
			format_in = FORMAT_RGB565;
			Fwl_Print(C3, M_DISPLAY, "Fwl_Scale_Convert:RGB888 to RGB565");
		}
		break;
		
	default:
		Fwl_Print(C3, M_DISPLAY, "Fwl_Scale_Convert srcFormat error");
		return AK_FALSE;			
	}

#ifdef OS_ANYKA
    //Reset_2DGraphic();//这里需不需要reset,还有待以后跟进.
	MMU_Clean_Invalidate_Dcache();
		//Fwl_Print(C3, M_DISPLAY, "noUse2D");
		ret = Fwl_ScaleConvertEx(&pBuf565, 1, srcWidth, &srcWin, format_in, 
		                     &dstBuf, 1, dstWidth, &dstWin, format_in,
		                     AK_TRUE,alpha,AK_TRUE,g_Graph.TransColor);
	
	if (0 != ret)
	{
		Fwl_Print(C3, M_DISPLAY, "Scale_Convert return fail and do it again");	
			ret = Fwl_ScaleConvertEx(&pBuf565, 1, srcWidth, &srcWin, format_in, 
	                     &dstBuf, 1, dstWidth, &dstWin, format_in,
	                     AK_TRUE, alpha, AK_TRUE, g_Graph.TransColor);
		}

#endif
	if ((AK_NULL != pBuf565) && (pBuf565 != srcBuf))
	{
		pBuf565 = Fwl_Free(pBuf565);
	}
	if(0 != ret)
	{
		Fwl_Print(C3, M_DISPLAY, "Img_BitBlt Scale_Convert return error code %d", ret);		
		return AK_FALSE;
	}

	return AK_TRUE;
	
}


T_S32 Fwl_Scale_Convert(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
                 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
                 T_VOID *srcBuf, T_U16 srcWidth, T_U16 srcHeight, T_U8 srcFormat)
{
	T_U8 ret = AK_FALSE;
	E_ImageFormat format_in;
    T_VOID *pBuf565 = AK_NULL; 

	T_RECT srcWin;
	T_RECT dstWin;
	
	Fwl_InitRect(&srcWin, 0, 0, srcWidth, srcHeight);
	Fwl_InitRect(&dstWin, dstPosX, dstPosY, scaleWidth, scaleHeight);
	
	switch(srcFormat)
	{
	case RGB565:
		format_in = FORMAT_RGB565;
		pBuf565 = srcBuf;
		break;
		
	case RGB888:
	    {
            if (AK_NULL == pBuf565)
           	{
				pBuf565 = Fwl_Malloc(srcWidth*srcHeight*2);
				if (AK_NULL == pBuf565)
				{
					Fwl_Print(C3, M_DISPLAY, "Fwl_Scale_Convert:malloc is fail");
					return AK_FALSE;
				}
			}
			Fwl_RGB888toRGB565(pBuf565,srcBuf,srcWidth,srcHeight);
			format_in = FORMAT_RGB565;
			Fwl_Print(C3, M_DISPLAY, "Fwl_Scale_Convert:RGB888 to RGB565");
		}
		break;
	default:
		Fwl_Print(C3, M_DISPLAY, "Fwl_Scale_Convert srcFormat error");
		return AK_FALSE;			
	}

#ifdef OS_ANYKA
    //Reset_2DGraphic();//这里需不需要reset,还有待以后跟进.
	MMU_Clean_Invalidate_Dcache();
		//Fwl_Print(C3, M_DISPLAY, "noUse2D");
		ret = Fwl_ScaleConvert(&pBuf565, 1, srcWidth, &srcWin, format_in, 
		                     &dstBuf, 1, dstWidth, &dstWin, format_in, AK_FALSE, AK_NULL);
	
	if (0 != ret)
	{
		Fwl_Print(C3, M_DISPLAY, "Scale_Convert return fail and do it again");	
			ret = Fwl_ScaleConvert(&pBuf565, 1, srcWidth, &srcWin, format_in, 
	                     &dstBuf, 1, dstWidth, &dstWin, format_in,AK_FALSE,AK_NULL);
		}

#endif
	if ((AK_NULL != pBuf565) && (pBuf565 != srcBuf))
	{
		pBuf565 = Fwl_Free(pBuf565);
	}
	if(0 != ret)
	{
		Fwl_Print(C3, M_DISPLAY, "Img_BitBlt Scale_Convert return error code %d", ret);		
		return AK_FALSE;
	}

	return AK_TRUE;
	
}

T_BOOL Fwl_YUV420BitBlt(const T_U8 *y, const T_U8 *u, const T_U8 *v, T_LEN srcBufW, T_RECT* srcRect,
					 		 T_U8 *dstBuf, T_LEN dstBufW, T_U8 dstFormat, T_RECT* dstRect)
{
	
	T_U8 	ret = 0;
    T_U32 	inBuf[MAX_2D_DIM];	// = {y, u, v};
    T_U32 	outBuf[MAX_2D_DIM];
	T_U8 	outDim = 1;

	AK_ASSERT_PTR(y, "Fwl_YUV420BitBlt() y Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(u, "Fwl_YUV420BitBlt() u Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(v, "Fwl_YUV420BitBlt() v Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(dstBuf, "Fwl_YUV420BitBlt() dstBuf Is Invalid", AK_FALSE);
	
	inBuf[0] = (T_U32)y;
	inBuf[1] = (T_U32)u;
	inBuf[2] = (T_U32)v;
	
	outBuf[0] = (T_U32)dstBuf;
	
	if (FORMAT_YUV420 == dstFormat)
	{
		outBuf[1] = (T_U32)(dstBuf + dstRect->width*dstRect->height);
		outBuf[2] = (T_U32)(dstBuf + dstRect->width*dstRect->height + (dstRect->width*dstRect->height>>2));
		
		outDim = MAX_2D_DIM;
	} 	

	ret = Fwl_ScaleConvert(inBuf, MAX_2D_DIM, srcBufW, srcRect, FORMAT_YUV420, 
							outBuf, outDim, dstBufW, dstRect, dstFormat, AK_FALSE, AK_NULL);
    if (ret)
    {
          Fwl_Print(C3, M_DISPLAY, "Fwl_YUV420BitBlt() Failure %d", ret); 
          return AK_FALSE;
    }
	
    return AK_TRUE;
}


T_BOOL Fwl_RGB565BitBlt(const T_U8* rgbBuf, T_LEN srcBufW, T_RECT* srcRect,
					 		 T_U8 *dstBuf, T_LEN dstBufW, T_U8 dstFormat, T_RECT* dstRect)
{
	T_U8 	ret = 0;
    T_U32 	inBuf[1];
    T_U32 	outBuf[1];

	AK_ASSERT_PTR(rgbBuf, "Fwl_RGB565BitBlt() rgbBuf Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(dstBuf, "Fwl_RGB565BitBlt() dstBuf Is Invalid", AK_FALSE);
		
	inBuf[0] 	= (T_U32)rgbBuf;		
	outBuf[0] 	= (T_U32)dstBuf;	 	
	
	ret = Fwl_ScaleConvert(inBuf, 1, srcBufW, srcRect, FORMAT_RGB565,
		outBuf, 1, dstBufW, dstRect, dstFormat, AK_FALSE, AK_NULL);
	
    if (ret != 0)
    {
          Fwl_Print(C3, M_DISPLAY, "Fwl_RGB565BitBlt() Failure %d", ret); 
          return AK_FALSE;
    }
	
    return AK_TRUE;
}


/**
 * @brief YUV 2 YUV/RGB Zoom, (NOT Limited 4X or 1/4X) Aspect Ratio < 4X
 *
 * @date 	July 5, 2011
 * @author 	Xie_Wenzhong
 * @param	srcY		[in] YUV Source Addr Y
 * @param	srcU		[in] YUV Source Addr U
 * @param	srcU		[in] YUV Source Addr V
 * @param	srcBufW	[in] Source Frame Width
 * @param	srcWin	[in] Source Frame Valid Window 
 * @param	dstBuf	[out] Destination YUV Size Data
 * @param	dstBufW	[in] Destination Frame Width
 * @param	dstWin	[in] Destination Frame Valid Window
 * @param	dstForamt	[in] Destination Frame Format FORMAT_YUV420/FORMAT_RGB565
 * @return 	T_BOOL
 * @retval	AK_FALSE	Zoom Failure
 * @retval	AK_TRUE	Zoom Success
 */
T_BOOL Fwl_YuvZoom(const T_U8 *srcY, const T_U8 *srcU, const T_U8 *srcV, T_LEN srcBufW, T_pRECT srcWin,
						T_U8 *dstBuf, T_LEN dstBufW, T_U8 dstForamt, T_pRECT dstWin)
{
	T_BOOL 	ret = AK_FALSE;
	T_U8 	*tmpBuf, *y, *u, *v;
	T_U8	format;
	T_RECT 	srcRECT;
	T_RECT 	dstRECT;
	T_LEN	srcFrameW;
	T_LEN	dstFrameW;

	AK_ASSERT_PTR(srcY, "Fwl_YuvZoom() srcY Is Invalid", AK_FALSE);
	AK_ASSERT_PTR(dstBuf, "Fwl_YuvZoom() dstBuf Is Invalid", AK_FALSE);

	if (srcWin->width <<2 < srcWin->height || srcWin->height<<2 < srcWin->width)
	{
		Fwl_Print(C2, M_IMAGE, "Unsupport Source Aspect Ratio Width %d, Height %d", srcWin->width, srcWin->height);
		return ret;
	}

	if (dstWin->width <<2 < dstWin->height || dstWin->height<<2 < dstWin->width)
	{
		Fwl_Print(C2, M_IMAGE, "Unsupport Destination Aspect Ratio Width %d, Height %d", dstWin->width, dstWin->height);
		return ret;
	}

	srcFrameW = srcBufW;
	memcpy(&srcRECT, srcWin, sizeof(T_RECT));
	memset(&dstRECT, 0, sizeof(T_RECT));

	y = (T_U8*)srcY;
	u = (T_U8*)srcU;
	v = (T_U8*)srcV;

	format = FORMAT_YUV420;

	do
	{
		if ((srcRECT.width > dstWin->width && srcRECT.height > dstWin->height && (srcRECT.width>>2 > dstWin->width || srcRECT.height>>2 > dstWin->height))
			|| (srcRECT.width < dstWin->width && srcRECT.height < dstWin->height && (srcRECT.width<<2 < dstWin->width || srcRECT.height<<2 < dstWin->height)))
		{
			if (srcRECT.width > dstWin->width)
			{
				dstRECT.width 	= srcRECT.width >> 2;
				dstRECT.height 	= srcRECT.height >> 2;

				// Width and Height Must is Even.
                if (0 != dstRECT.width % 2)
                {
                    dstRECT.width += 1;
                }

                if (0 != dstRECT.height % 2)
                {
                    dstRECT.height += 1;
                }
			}
			else
			{
				dstRECT.width 	= srcRECT.width << 2;
				dstRECT.height 	= srcRECT.height << 2;
			}

			dstFrameW = dstRECT.width;
			
			tmpBuf = Fwl_Malloc(dstRECT.width*dstRECT.height + (dstRECT.width*dstRECT.height>>1));
			AK_ASSERT_PTR(tmpBuf, "Fwl_YuvZoom() tmpBuf Malloc Failure", AK_FALSE);
		}
		else
		{
			memcpy(&dstRECT, dstWin, sizeof(T_RECT));
			
			tmpBuf 		= dstBuf;
			dstFrameW 	= dstBufW;
			
			if (RGB565 == dstForamt)
				format = FORMAT_RGB565;
			else if (YUV420 == dstForamt)
				format = FORMAT_YUV420;
			else
			{
				Fwl_Print(C2, M_IMAGE, "Unsupport Destination Format");
				return AK_FALSE;
			}
		}		
		
		ret = Fwl_YUV420BitBlt(y, u, v, srcFrameW, &srcRECT, tmpBuf, dstFrameW, format, &dstRECT);

        if (!ret)
		{
			if (y != tmpBuf && dstBuf != tmpBuf)
			{
				tmpBuf = Fwl_Free(tmpBuf);		
			}
            
		    Fwl_Print(C2, M_IMAGE, "2D Fail!\n");
			return ret;
		}
        
		// Release Temp Memory
		if (y != srcY)
		{
			y = Fwl_Free(y);
			u = AK_NULL;
			v = AK_NULL;
		}		

		// Zoom Finish
		if (dstRECT.width == dstWin->width && dstRECT.height == dstWin->height)
		{
			break;
		}
		// Do Next 2D
		else
		{
			y = tmpBuf;
			u = tmpBuf + dstRECT.width * dstRECT.height;
			v = u + (dstRECT.width * dstRECT.height >> 2);

			srcFrameW = dstRECT.width;
			srcRECT.width 	= dstRECT.width;
			srcRECT.height 	= dstRECT.height;			
            srcRECT.left    = 0;
            srcRECT.top     = 0;
		}		
		
	}while (AK_TRUE);

	return ret;
}



//========================================================================
//dowith ZoomInMultiple 2011-5-6
//========================================================================
#define TRUNCATE_RGB(x)  ((x)>255?255:((x)<0?0:(x))) // truncate R/G/B to [0, 255]
#define AKRGB565(r,g,b)  ((((r)&0xf8)<<8)|(((g)&0xfc)<<3)|((b)>>3))
#define YUV2R888(y,u,v)  ((y) + ((359 * (v))>>8))
#define YUV2G888(y,u,v)  ((y) - ((88 * (u) + 183 * (v))>>8))
#define YUV2B888(y,u,v)  ((y) + ((454 * (u))>>8))

#define GETSRC(rgb,x)\
	Y = *(yAddr + x);\
	U = (*(uAddr + (x>>1)))-128; \
	V = (*(vAddr + (x>>1)))-128; \
	r = TRUNCATE_RGB(YUV2R888(Y,U,V));\
	g = TRUNCATE_RGB(YUV2G888(Y,U,V));\
	b = TRUNCATE_RGB(YUV2B888(Y,U,V));\
rgb = AKRGB565(r, g, b);

T_BOOL SwScaleFormat(T_VOID *RGBbuf, T_U8 *Ybuf, T_U8 *Ubuf, T_U8 *Vbuf, T_U16 Ywidth, T_U16 Yheight,
					  T_U16 offsetX, T_U16 offsetY, T_U16 rectW, T_U16 rectH, T_U16 rgbW, T_U16 rgbH)
{
	T_U32 Xstep,Ystep,stepInc; // fixed _16_16
	T_U16 srcPx1, srcPx2, Qwidth, i, j, k, xpos, ypos;
	T_U8 r, g, b;
	T_S16 Y, U, V;
	T_U32 *buff = (T_U32*)RGBbuf;
	T_U8 *yAddr,*uAddr,*vAddr;
	
	Xstep = (rectW << 16) / rgbW;
	Ystep = (rectH << 16) / rgbH;	
	Qwidth = rgbW >> 2; // rgbW/4;   
	
	for(j = 0; j < rgbH; j++)
	{
		ypos = (T_U16)(offsetY+((j*Ystep)>>16));
		yAddr = Ybuf+(Ywidth*ypos)+offsetX;
		uAddr = Ubuf+(Ywidth>>1)*(ypos>>1)+(offsetX>>1);
		vAddr = Vbuf+(Ywidth>>1)*(ypos>>1)+(offsetX>>1);
		stepInc = 0;
		xpos = 0;
		for (i = 0; i < Qwidth; i++)
		{
			stepInc += Xstep;
			xpos = (T_U16)(stepInc>>16);
			GETSRC(srcPx1,xpos);
			stepInc += Xstep;
			xpos = (T_U16)(stepInc>>16);
			GETSRC(srcPx2,xpos);
			*buff++ = (srcPx2 << 16) | srcPx1;
			
			stepInc += Xstep;
			xpos = (T_U16)(stepInc>>16);
			GETSRC(srcPx1,xpos);
			stepInc += Xstep;
			xpos = (T_U16)(stepInc>>16);
			GETSRC(srcPx2,xpos);
			*buff++ = (srcPx2 << 16) | srcPx1;
		}
		
		k = i<<2;
		while (k < rgbW)
		{	
			stepInc += Xstep;
			xpos = (T_U16)(stepInc>>16);
			GETSRC(srcPx1,xpos);
			*((T_U16*)buff)++ = srcPx1;
			k++;
		}
	}
	return AK_TRUE;
}

//========================================================================

T_VOID Fwl_ColorToRGB565(T_COLOR color, T_U8 *low, T_U8 *high)
{
	T_U8 r = 0;
	T_U8 g = 0;
	T_U8 b = 0;
	
	if (AK_NULL != low && AK_NULL != high)
	{
		r = (T_U8)(color >> 16);
		g = (T_U8)(color >> 8);
		b = (T_U8)(color);

		*low = (T_U8)(((b & 0xf8) >> 3) | ((g & 0x1c) << 3));
		*high = (T_U8)((r & 0xf8) | ((g & 0xe0) >> 5));
	}
}

/**
 * @brief Init a Rect
 * 
 * @author ljh 
 * @date  2005-1-14
 * @param T_RECT *rect Points to the rect that 
 * store the intersection rectangle
 * @param  x the horizontal position of rectangle
 * @param  y the vertical position of rectangle
 * @param width the width of rectangle
 * @param height the height of rectangle
 * @return T_VOID
 */
T_BOOL Fwl_InitRect(T_RECT *rect, T_POS x, T_POS y, T_LEN width, T_LEN height)
{
	AK_ASSERT_PTR(rect, "rect Is Invalid", AK_FALSE);
	
	rect->left 		= x;
	rect->top 		= y;
	rect->height 	= height;
	rect->width 	= width;
	
	return AK_TRUE;
}



