/**
 * @file Fwl_DispOsd.c
 * @brief   draw all kinds of graphics on osd buffer and show osd buffer
 *
 * Copyright (C) 2014 Anyka (GuangZhou) Micro-electronics Technology Co., Ltd.
 * @author  liu guodong
 * @MODIFY  
 * @DATE    2014-7
 * @version 
 * @
 */
#include "Fwl_Public.h"
#include "Fwl_display.h"
#include "Fwl_osMalloc.h"
#include "Eng_AkBmp.h"
#include "Fwl_DispOsd.h"
#include "Arch_lcd.h"
#include "Fwl_graphic.h"
#include "Fwl_font.h"
#include "Dev_display.h"

//存储15个颜色的调色板
static T_U16   maPanel[15];
//使用两个缓冲区避免闪烁
static T_U8   * mpDispBuf=AK_NULL , * mpDispBuf2=AK_NULL;

static T_U32  mDispBufWidth=0;
static T_U32  mDispBufHeight=0;

//初始化osd相关的缓冲区
T_BOOL Fwl_Osd_Init()
{
	mDispBufWidth =  MAIN_LCD_WIDTH;
	mDispBufHeight = 	MAIN_LCD_HEIGHT;
	if ( AK_NULL == mpDispBuf)
	{
		mpDispBuf = Fwl_Malloc(mDispBufWidth * mDispBufHeight /2);
		memset(mpDispBuf, 0 , mDispBufWidth * mDispBufHeight /2);
	}
	if ( AK_NULL == mpDispBuf2)
	{
		mpDispBuf2 = Fwl_Malloc(mDispBufWidth * mDispBufHeight /2);
		memset(mpDispBuf2, 0 , mDispBufWidth * mDispBufHeight /2);
	}
	Fwl_Osd_SetColorPanelByGray();
	return AK_TRUE;
}

//按照16位色渐进的灰度设置调色板
T_BOOL Fwl_Osd_SetColorPanelByGray()
{
	T_U32  i;
	T_U16 r,g,b;

//根据r,g,b 分别从小到大给调色板赋值	
 	for(i =0 ;i < 15 ;i++)
	{
		r = (3 + 2 * i ) << 11; 
		g = (3+ 2 * i) << 6;
		b = 3 + 2* i;
		
		maPanel[i] = r+ g +b ; 
	}
	lcd_osd_set_color_palette(maPanel);

	return AK_TRUE;		
}


//按照颜色rgb的平均灰度， 获取颜色在调色板的索引值
T_U8  Fwl_Osd_GetColorPanelIndexByGray(T_U16 color )
{
	T_U32 i;
	T_U16 iGrayColor, iAverage;

	//取r,g,b 值的平均值
	iAverage = ((color >>11) + ((color & 0x7E0) >>6)+ (color & 0x1F) )/3; 
	//按照r,g,b平均值获得颜色值
	iGrayColor = (iAverage <<11) + (iAverage<<6)+iAverage;

	//通过颜色比对获得最接近颜色的索引 
	for(i =0 ;i < 14 ;i++)
	{
		if (iGrayColor <=maPanel[i])
			break;
		if (iGrayColor >  maPanel[i]  && iGrayColor <= maPanel[i+1])
		 break;
	}
	return i+1;
	
}

T_BOOL Fwl_Osd_DrawStreamBmpByGray(T_RECT * pRangeRect ,T_pCDATA pBmpStream)
{
	T_AK_BMP  anykaBmp;

	
    AkBmpGetFromString(pBmpStream, &anykaBmp);
    return Fwl_Osd_DrawBmpByGray(pRangeRect, &anykaBmp);
	
}

T_BOOL Fwl_Osd_DrawRawBmpByGray(T_RECT * pRangeRect ,T_U8 * pRawBmp)
{
	T_AK_BMP  anykaBmp;

    anykaBmp.BmpData = pRawBmp;
    anykaBmp.Deep = 16;
    anykaBmp.Frame =1;
    anykaBmp.Width =pRangeRect->width;
    anykaBmp.Height =pRangeRect->height;
    return Fwl_Osd_DrawBmpByGray(pRangeRect, &anykaBmp);
	
}

T_BOOL Fwl_Osd_DrawBmpByGray(T_RECT * pRangeRect , T_AK_BMP    * pAnykaBmp)
{
	T_U32  i,j;
	T_U16 * pBmpBuf ;
	T_U32 iPosOff;
	
	if ( AK_NULL == mpDispBuf)
		return AK_FALSE;
		
	pBmpBuf = Fwl_Malloc(pRangeRect->width* pRangeRect->height*2);
	if ( AK_NULL == pBmpBuf)
		return AK_FALSE;

//绘制到缓冲区
	Fwl_AkBmpDrawPartOnRGB((T_U8*)pBmpBuf,pRangeRect->width,pRangeRect->height
		, 0,0,AK_NULL,pAnykaBmp,AK_NULL,AK_FALSE,RGB565);

//把缓冲区每个颜色值转换为索引值
	iPosOff = pRangeRect->top * (mDispBufWidth/2) + pRangeRect->left/2;
	for(i =0 ;i < pRangeRect->width/2 ;i ++)
	{
		for(j =0 ; j < pRangeRect->height ;j++)
		{
			mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] = Fwl_Osd_GetColorPanelIndexByGray(pBmpBuf[j * pRangeRect->width + 2*i])   + (Fwl_Osd_GetColorPanelIndexByGray(pBmpBuf[j * pRangeRect->width + 2*i+1])<<4);
		}
		
	}
	if (pBmpBuf !=AK_NULL)
		Fwl_Free(pBmpBuf);
		
	return AK_TRUE;
	
}

T_BOOL Fwl_Osd_DrawRadioByGray(T_RECT * pRangeRect , T_POS x, T_POS y,T_LEN radius,T_BOOL focus,    T_COLOR color)
{
	T_U32  i,j;
	T_U16 * pBmpBuf ;
	T_U32 iPosOff;
	T_U8  idx1,idx2;
		
	if ( AK_NULL == mpDispBuf)
		return AK_FALSE;
		
	pBmpBuf = Fwl_Malloc(pRangeRect->width* pRangeRect->height*2);
	if ( AK_NULL == pBmpBuf)
		return AK_FALSE;

	memset(pBmpBuf, 0 ,pRangeRect->width* pRangeRect->height*2);

	Fwl_DrawRadioOnRGB((T_U8*)pBmpBuf,pRangeRect->width,pRangeRect->height
		, x,y,radius,focus,color,RGB565);


	iPosOff = (pRangeRect->top )* (mDispBufWidth/2) + (pRangeRect->left)/2  ;
		
	for(i =0 ;i < pRangeRect->width /2 ; i ++)
	{
		for(j =0 ; j < pRangeRect->height ;j++)
		{
			if (pBmpBuf[j * pRangeRect->width+2*i] == 0)//保留原值
				idx1 = mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] & 0x0f;
			else
				idx1 = 	Fwl_Osd_GetColorPanelIndexByGray(pBmpBuf[j * pRangeRect->width + 2*i]);			
			if (pBmpBuf[j * pRangeRect->width+2*i+1] == 0) //保留原值
				idx2 = mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] & 0xf0 ;
			else
				idx2 = (Fwl_Osd_GetColorPanelIndexByGray(pBmpBuf[j * pRangeRect->width + 2*i+1])<<4);
			
			mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] = idx1 + idx2;
			
		}
		
	}
	if (pBmpBuf !=AK_NULL)
		Fwl_Free(pBmpBuf);
		
	return AK_TRUE;

}

T_BOOL Fwl_Osd_FillSolidRectByGray(T_RECT * pRangeRect ,   T_COLOR color)
{

		T_U32  i,j;
		T_U16 * pBmpBuf ;
		T_U32 iPosOff;
		T_RECT  rect;
		
		if ( AK_NULL == mpDispBuf)
			return AK_FALSE;
			
		pBmpBuf = Fwl_Malloc(pRangeRect->width* pRangeRect->height*2);
		if ( AK_NULL == pBmpBuf)
			return AK_FALSE;

		rect.left = rect.top =0 ;
		rect.width = pRangeRect->width;
		rect.height = pRangeRect->height;
		
		Fwl_FillSolidRectOnRGB((T_U8*)pBmpBuf,pRangeRect->width,pRangeRect->height
			, &rect,color,RGB565);

		iPosOff = pRangeRect->top * (mDispBufWidth/2) + pRangeRect->left/2;
		if (pRangeRect->width ==1)
		{
			for(j =0 ; j < pRangeRect->height ;j++)
			{
				if (pRangeRect->left % 2 ==0)
				{
					mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] = Fwl_Osd_GetColorPanelIndexByGray(pBmpBuf[j * pRangeRect->width + 2*i])   + (mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] & 0xf0);
				}else
					mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] = mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] & 0x0f   + (Fwl_Osd_GetColorPanelIndexByGray(pBmpBuf[j * pRangeRect->width + 2*i+1])<<4);
			}
		}else
		for(i =0 ;i < pRangeRect->width/2 ;i ++)
		{
			for(j =0 ; j < pRangeRect->height ;j++)
			{
				mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] = Fwl_Osd_GetColorPanelIndexByGray(pBmpBuf[j * pRangeRect->width + 2*i])   + (Fwl_Osd_GetColorPanelIndexByGray(pBmpBuf[j * pRangeRect->width + 2*i+1])<<4);
			}
			
		}
		if (pBmpBuf !=AK_NULL)
			Fwl_Free(pBmpBuf);
			
		return AK_TRUE;
				
}

T_BOOL  Fwl_Osd_FillSolidTriaByGray(T_RECT *pRect, T_TRIANGLE_DIRECTION direction, T_COLOR color)
{
    T_S16   mid;
    T_S16   thick = 2;
    T_LEN   i;
	T_POS left,top,width,height;
	T_TRIANGLE_DIRECTION dir;
	T_RECT  subRect;

	left = pRect->left;
	top  = pRect->top;
	width = pRect->width;
	height = pRect->height;
	dir = direction ;
	
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
            {
            	subRect.left = (T_POS)(mid - (width-thick)/2*(i+1)/height);
            	subRect.top  = (T_POS)(top + i);
            	subRect.width = (T_LEN)((width-thick)/2*(i+1)/height*2+thick);
            	subRect.height = 1;
            	
                Fwl_Osd_FillSolidRectByGray(&subRect, color);
             }
        }
        else if (dir == 0)
        {
            for (i = 0; i < height; i++)
            {
            	subRect.left = (T_POS)(mid - (width-thick)/2*(i+1)/height);
            	subRect.top  = (T_POS)(top + height - i - 1);
            	subRect.width = (T_LEN)((width-thick)/2*(i+1)/height*2+thick);
            	subRect.height = 1;
                Fwl_Osd_FillSolidRectByGray(&subRect, color);
             }
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
            {
            	subRect.left = (T_POS)(left + i);
            	subRect.top  = (T_POS)(mid - (height-thick)/2*(i+1)/width);
            	subRect.width = 1;
            	subRect.height = (T_LEN)((height-thick)/2*(i+1)/width*2+thick);
            	
                Fwl_Osd_FillSolidRectByGray(&subRect, color);
             }
        }
        else if (dir == 2)
        {
            for (i = 0; i < width; i++)
            {
            	subRect.left = (T_POS)(left + width - i - 1);
            	subRect.top  = (T_POS)(mid - (height-thick)/2*(i+1)/width);
            	subRect.width = 1;
            	subRect.height = (T_LEN)((height-thick)/2*(i+1)/width*2+thick);
            	
                Fwl_Osd_FillSolidRectByGray(&subRect, color);
             }
        }
    }

	return AK_TRUE;
}

T_BOOL Fwl_Osd_DrawStringByGray(T_POS x, T_POS y ,T_pCSTR  pString,T_U16 strLen 
	,T_COLOR color, T_FONT font)
{
    T_U16* wcs = AK_NULL;

	wcs = Fwl_Malloc(sizeof(T_U16)*(strLen+1));

	if (AK_NULL == wcs)
	{
		return AK_FALSE;
	}
	
    Eng_StrMbcs2Ucs(pString, wcs);
    Fwl_Osd_DrawUStringByGray( x,y,wcs,strLen,color,font);
	
    Fwl_Free(wcs);

	return AK_TRUE ;
}

T_BOOL Fwl_Osd_DrawUStringByGray(T_POS x, T_POS y ,T_U16* pString,T_U16 strLen 
	,T_COLOR color, T_FONT font)
{
	T_RECT  rect;
	T_U16 * pBmpBuf ;
	T_U32 iPosOff;
	T_U32 i, j;
	T_U8  idx1,idx2;
	T_U32  fontHeight;

	
	if ( AK_NULL == mpDispBuf)
		return AK_FALSE;
		

	fontHeight = DynamicFont_GetFontHeight(font);		
	rect.width = fontHeight * strLen;
	rect.height = fontHeight;
		
	pBmpBuf = Fwl_Malloc(rect.width * rect.height* 2);
	if ( AK_NULL == pBmpBuf)
		return AK_FALSE;

	memset(pBmpBuf, 0 , rect.width * rect.height *2 ) ;
	Fwl_UDispSpeciStringOnRGB(pBmpBuf,rect.width,rect.height,0,0,pString,color,RGB565,font,strLen);

	iPosOff = y * (mDispBufWidth/2) + x/2;
	for(i =0 ;i < rect.width/2  ;i++)
		for(j =0 ; j < rect.height ; j++)
		{
			if (pBmpBuf[j * rect.width+2*i] == 0)//保留原值
				idx1 = mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] & 0x0f;
			else
				idx1 = 	Fwl_Osd_GetColorPanelIndexByGray(pBmpBuf[j * rect.width + 2*i]);			
			if (pBmpBuf[j * rect.width+2*i+1] == 0) //保留原值
				idx2 = mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] & 0xf0 ;
			else
				idx2 = (Fwl_Osd_GetColorPanelIndexByGray(pBmpBuf[j * rect.width + 2*i+1])<<4);
			
			mpDispBuf[j * (mDispBufWidth/2)+ i+iPosOff] = idx1 + idx2;
			
		}
		
	if (pBmpBuf !=AK_NULL)
		Fwl_Free(pBmpBuf);
		
	return AK_TRUE;
	
	
	

}


T_BOOL Fwl_Osd_RefreshDisplay(T_VOID)
{
	T_RECT  rect, rectDev;
	T_U8 * pTmp;
	DISPLAY_TYPE_DEV dispDev;
	
	rect.left = rect.top =0 ;
	rect.width = mDispBufWidth;
	rect.height =mDispBufHeight;

	dispDev = Fwl_GetDispalyType();
	Dev_GetDisplayCaps(dispDev, &rectDev);
	if (dispDev >= DISPLAY_TVOUT_PAL)
	{
		T_S32 offX ,offY;
		
		//将osd移动至tvout的右下角显示
		offX = rectDev.left + rectDev.width - rect.width;
		//tvout 实际的缓冲区只有720*288(pal制),所以坐标要除以2
		offY = rectDev.top/2  + rectDev.height/2 - rect.height;

		if (offY < 0)
		{
			offY = 0;
		}

		rect.left += offX ;
		rect.top += offY;
	}

	pTmp = mpDispBuf;
	mpDispBuf = mpDispBuf2;
	mpDispBuf2 = pTmp;


	return lcd_osd_display_on(DISPLAY_LCD_0, &rect, 8, mpDispBuf2);
		
}


/**
 * @brief Query : display osd buffer on lcd or tvout
 *
 * @author 	songmengxing
 * @param	pRect[in]: pRect  to refresh, width must be lcd width
    * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_RefreshDisplayRect(T_RECT *pRect)
{
	T_RECT  rect, rectDev;
	T_U8 * pTmp;
	DISPLAY_TYPE_DEV dispDev;
	
	rect.left	= pRect->left;
	rect.top	= pRect->top;
	rect.width	= pRect->width;
	rect.height = pRect->height;

	dispDev = Fwl_GetDispalyType();
	Dev_GetDisplayCaps(dispDev, &rectDev);
	if (dispDev >= DISPLAY_TVOUT_PAL)
	{
		T_S32 offX ,offY;

		//tvout边缘显示修正
		offX = rectDev.left;
		//tvout 实际的缓冲区只有720*288(pal制),所以坐标要除以2
		offY = rectDev.top/2;

		if (offY < 0)
		{
			offY = 0;
		}

		rect.left += offX ;
		rect.top += offY;
	}

	pTmp = mpDispBuf;
	mpDispBuf = mpDispBuf2;
	mpDispBuf2 = pTmp;


	return lcd_osd_display_on(DISPLAY_LCD_0, &rect, 8, mpDispBuf2);
		
}

T_VOID Fwl_Osd_ClearDispBuf(T_VOID)
{
	if ( AK_NULL != mpDispBuf)
	{
		memset(mpDispBuf, 0 , mDispBufWidth * mDispBufHeight /2);
	}
	
}
T_BOOL Fwl_Osd_DisplayOff(T_VOID)
{
	lcd_osd_display_off(DISPLAY_LCD_0);
	return AK_TRUE;
}

