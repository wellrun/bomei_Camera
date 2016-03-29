/********************************************************************
File:
Date:

Author:

Descrip:
  

*********************************************************************/

#include "akdefine.h"
#include "Arch_lcd.h"
#include "Dev_display.h"
#include "Display_func_common.h"
#include "arch_gui.h"
#include "Eng_ScaleConvertSoft.h"
#include "Lib_state.h"
#include "fwl_graphic.h"
#include "AKOS_Api.h"
#include "AKError.h"

typedef struct 
{
	T_U16  width;
	T_U16  height;
}T_LCD_SIZE;

static T_LCD_SIZE stLcdSize[DISPLAY_LCD_1] = {{0,0}};

static T_RECT stDevCap[DISPLAY_MAX_TYPE] = {
	{0,0,0,0},
	{0,0,0,0},				/*init in Dev_InitDisplay()*/
	{TV_LEFT_PAL, TV_TOP_PAL, TV_WIDTH_PAL, TV_HEIGHT_PAL},
	{TV_LEFT_NTSC, TV_TOP_NTSC, TV_WIDTH_NTSC, TV_HEIGHT_NTSC}
};

static T_hSemaphore dispMutex = AK_INVALID_SEMAPHORE;
static T_U8			    	*bDisplayBuf = AK_NULL;
static DISPLAY_TYPE_DEV		DisplayType = DISPLAY_LCD_0;	
static T_U8				    bBrightness[DISPLAY_TVOUT_PAL] = {0};	

#ifndef OS_ANYKA
extern  lcd_refresh(T_eLCD lcd, T_U16 left, T_U16 top, T_U16 width, T_U16 height);
#endif

static T_BOOL Dev_DisplayLock(T_VOID)
{
	if (AK_INVALID_SEMAPHORE == dispMutex)
	{
		dispMutex = AK_Create_Semaphore(1, AK_PRIORITY);
		if (AK_INVALID_SEMAPHORE == dispMutex)
		{
			Fwl_Print(C2, M_DISPLAY, "Create Display Lock Failure");
			return AK_FALSE;
		}
	}

	AK_Obtain_Semaphore(dispMutex, AK_SUSPEND);

	return AK_TRUE;
}

static T_BOOL Dev_DisplayUnlock(T_VOID)
{
	if (AK_INVALID_SEMAPHORE == dispMutex)
	{
		return AK_FALSE;
	}
	
	AK_Release_Semaphore(dispMutex);
	
	return AK_TRUE;
}

T_BOOL Dev_InitDisplay(T_VOID)
{
	bDisplayBuf = (T_U8*) MALLOC(DISPLAY_BUF_SIZE);
	if(AK_NULL == bDisplayBuf)
	{
		Fwl_Print(C1, M_DISPLAY, "Malloc for bDisplayBuf fail!\n");
		while(1);		/*loop!!!!!!*/
	}

	Fwl_Print(C4, M_DISPLAY, "Malloc for bDisplayBuf[%d] OK.\n", DISPLAY_BUF_SIZE);

#ifdef OS_ANYKA

	/*device init put here*/
	if(!lcd_initial())
	{
		Fwl_Print(C1, M_DISPLAY, "LCD Initial Failure.\n");
		return AK_FALSE;	
	}

#if 0   //no use LCD_1
	stDevCap[DISPLAY_LCD_1].height = lcd_get_hardware_height(DISPLAY_LCD_1);
	stDevCap[DISPLAY_LCD_1].width  = lcd_get_hardware_width(DISPLAY_LCD_1);	
#endif
	stLcdSize[DISPLAY_LCD_0].height = lcd_get_hardware_height(DISPLAY_LCD_0);
	stDevCap[DISPLAY_LCD_0].height 	= lcd_get_hardware_height(DISPLAY_LCD_0);
	
	stLcdSize[DISPLAY_LCD_0].width 	= lcd_get_hardware_width(DISPLAY_LCD_0);
	stDevCap[DISPLAY_LCD_0].width  	= lcd_get_hardware_width(DISPLAY_LCD_0);
#else
	stDevCap[DISPLAY_LCD_0].height 	= MAIN_LCD_HEIGHT;
	stDevCap[DISPLAY_LCD_0].width  	= MAIN_LCD_WIDTH;
#endif

	Fwl_Print(C3, M_DISPLAY, "lcd width = %d, lcd height = %d\n", stDevCap[DISPLAY_LCD_0].width, stDevCap[DISPLAY_LCD_0].height);
#ifdef OS_ANYKA    
    if (Fwl_GetDispalyType() < DISPLAY_TVOUT_PAL)
    {
//        lcd_tvout_clk_sel (TVOUT_CLK_EXTERNAL);
    }   
    else
    {
//        lcd_tvout_clk_sel (TVOUT_CLK_INTERNAL);
    }    
#endif
	Fwl_Print(C4, M_DISPLAY, "Exit Dev_InitDisplay\n");

	return	AK_TRUE;
}


T_BOOL Dev_GetDisplayCaps (DISPLAY_TYPE_DEV disp_type, T_RECT *pstRect)
{
    if(DISPLAY_MAX_TYPE <= disp_type)
    {
        Fwl_Print(C3, M_DISPLAY,"Dev_GetDisplayCaps : DISPLAY_TYPE_DEV error.\n");
        return  AK_FALSE;
    }    

	*pstRect = stDevCap[disp_type];
		
	return	AK_TRUE;
}


T_VOID Dev_DisplayOn(T_VOID)
{
	Dev_DisplayLock();
	
	lcd_turn_on(DISPLAY_LCD_0);
	
	Dev_DisplayUnlock();
}


T_VOID Dev_DisplayOff(T_VOID)
{
	Dev_DisplayLock();
	
	lcd_turn_off(DISPLAY_LCD_0);
	
	Dev_DisplayUnlock();
}


T_VOID Dev_LcdRotate(T_eLCD_DEGREE rotate)
{
	switch(rotate)
	{
	case LCD_0_DEGREE:
	case LCD_180_DEGREE:
		stDevCap[DISPLAY_LCD_0].width 	= stLcdSize[DISPLAY_LCD_0].width;
		stDevCap[DISPLAY_LCD_0].height 	= stLcdSize[DISPLAY_LCD_0].height;
		break;
		
	case LCD_90_DEGREE:
	case LCD_270_DEGREE:
		stDevCap[DISPLAY_LCD_0].width 	= stLcdSize[DISPLAY_LCD_0].height ;
		stDevCap[DISPLAY_LCD_0].height 	= stLcdSize[DISPLAY_LCD_0].width;
		break;
		
	default:
		return ;			
	}
	
	lcd_rotate(DISPLAY_LCD_0, rotate);
}


T_eLCD_DEGREE Dev_GetLcdDegree(T_VOID)
{
    return lcd_degree(DISPLAY_LCD_0);
}

T_VOID Dev_RefreshTVOUT(const T_U8 *y, const T_U8 *u, const T_U8 *v, T_U16 srcW, T_U16 oriW, T_U16 oriH)
{
	T_RECT srcWin;
	T_RECT dstWin;
	
	Fwl_InitRect(&srcWin, 0, 0, oriW, oriH);
	Fwl_InitRect(&dstWin, stDevCap[DisplayType].left, (T_U16)(stDevCap[DisplayType].top << 1),
		stDevCap[DisplayType].width, (T_U16)(stDevCap[DisplayType].height << 1));
	
	//AVIPlayer_AspectRatio(&dstWin, oriW, oriH, TV_OUT_WIDTH, TV_OUT_HEIGHT);

	//dstWin.top >>= 2;
	//dstWin.height <<= 1;
	Dev_DisplayLock();
	Fwl_YUV420BitBlt(y, u, v, srcW, &srcWin, bDisplayBuf, TV_OUT_WIDTH, FORMAT_RGB565, &dstWin);	
	//lcd_refresh_RGB(DISPLAY_LCD_0, &dstWin, bDisplayBuf);
	Dev_DisplayUnlock();
}

//多通道同时刷新标识，对于
static T_U32 m_multiChannelFlag=AK_FALSE;
T_VOID Dev_SetMultiChannelDisp(T_BOOL flag)
{
	m_multiChannelFlag = flag;
}

T_VOID Dev_Refresh_Output(T_VOID)
{
	lcd_refresh_output(DISPLAY_LCD_0);

}
T_VOID Dev_RefreshDisplay(T_U8 *bBuf, T_U16 Width, T_U16 Height, LAYER_TYPE type)
{ 
	T_RECT  stRect;
	DISPLAY_TYPE_DEV dispType;
	
	Fwl_Print(C4, M_DISPLAY, "Enter Dev_RefreshDisplay.\n");	
	
	AK_ASSERT_PTR_VOID(bBuf, "bBuf Is Invalid");

	Dev_DisplayLock();
	
	dispType = Dev_GetDisplayType();
	/*Width & Hieght need to check ? maybe ,bu don't now*/	
	stRect.left = 0;
	stRect.top  = 0;
	stRect.width = stDevCap[dispType].left*2 + stDevCap[dispType].width;
	stRect.height = stDevCap[dispType].top*2 + stDevCap[dispType].height;
	
#ifdef OS_ANYKA
	if(dispType < DISPLAY_TVOUT_PAL)
	{
		if (E_LCD_TYPE_RGB == lcd_get_type())
		{
			memcpy(bDisplayBuf, bBuf, (MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * BYTES_PER_PIXEL));

			if (m_multiChannelFlag)
			{
				if (!lcd_refreshdata_RGB(DISPLAY_LCD_0, &stRect, bDisplayBuf))
				{
					Fwl_Print(C2, M_DISPLAY, "Dev_RefreshDisplay: lcd_refreshdata_RGB error.\n");	
				}
				
			}else
			{
				if (!lcd_refresh_RGB(DISPLAY_LCD_0, &stRect, bDisplayBuf))
				{
					Fwl_Print(C2, M_DISPLAY, "Dev_RefreshDisplay: lcd_refresh_RGB error.\n");	
				}
			}
		}
		else
		{
			if (m_multiChannelFlag)
			{
				if (!lcd_refreshdata_RGB(DISPLAY_LCD_0, &stRect, bBuf))
				{
					Fwl_Print(C2, M_DISPLAY, "Dev_RefreshDisplay: lcd_refreshdata_RGB error.\n");	
				}
				
			}else
			{
				if (!lcd_refresh_RGB(DISPLAY_LCD_0, &stRect, bBuf))
				{
					Fwl_Print(C2, M_DISPLAY, "Dev_RefreshDisplay: lcd_refresh_RGB error.\n");	
				}
			}
			
		}
	}
	else
	{
		if(AK_NULL == bDisplayBuf)
		{
			Fwl_Print(C1, M_DISPLAY, "Dev_RefreshDisplay() bDisplayBuf==NULL!\n");
			while(1);
		}
		
		if(0 > Fwl_Scale_Convert(bDisplayBuf, stDevCap[dispType].width, 
			stDevCap[dispType].height, stDevCap[dispType].left, 
			stDevCap[dispType].top, stRect.width,
			bBuf, Width, Height, type))
		{
			Fwl_Print(C3, M_DISPLAY,"Dev_RefreshDisplay Fwl_Scale_Convert 2 error.\n");		
		}				

		if (m_multiChannelFlag)
		{
			if(!lcd_refreshdata_RGB(DISPLAY_LCD_0, &stRect, bDisplayBuf))
			{
				Fwl_Print(C2, M_DISPLAY, "Dev_RefreshDisplay: lcd_refreshdata_RGB error.\n");	
			}
			
		}else
		{
			if(!lcd_refresh_RGB(DISPLAY_LCD_0, &stRect, bDisplayBuf))
			{
				Fwl_Print(C2, M_DISPLAY, "Dev_RefreshDisplay: lcd_refresh_RGB error.\n");	
			}
		}
		
	}

#else
	lcd_refresh(DISPLAY_LCD_0, stRect.left, stRect.top, stRect.width, stRect.height);
#endif

	Dev_DisplayUnlock();

	/*show to where ?*/	
}


/*有些界面(如tvout下的camera 预览界面)，软2D太慢，导致显示很卡，
若该界面的RGB层只需要刷某个颜色时，可以用此接口。
*/
T_VOID  Dev_RefreshDisplayByColor(T_COLOR color)
{
	T_RECT	stRect;
	DISPLAY_TYPE_DEV dispType;
	T_RECT rect;
	
	Fwl_Print(C4, M_DISPLAY,"Enter Dev_RefreshDisplayByColor\n");
	
	Dev_DisplayLock();

	dispType = Dev_GetDisplayType();
	
	stRect.left = 0;
	stRect.top	= 0;
	stRect.width = stDevCap[dispType].left*2 + stDevCap[dispType].width;
	stRect.height = stDevCap[dispType].top*2 + stDevCap[dispType].height;

	if(dispType < DISPLAY_TVOUT_PAL)
	{
		stRect.width = MAIN_LCD_WIDTH;
		stRect.height = MAIN_LCD_HEIGHT;
	}
	
#ifdef OS_ANYKA

	if(AK_NULL == bDisplayBuf)
	{
		Fwl_Print(C1, M_DISPLAY, "Dev_RefreshDisplayByColor() bDisplayBuf==NULL!\n");
		while(1);
	}

	RectInit(&rect, 0, 0, stRect.width, stRect.height);
	Fwl_Clean(bDisplayBuf, stRect.width, stRect.height, &rect, color, RGB565);
				
	if (m_multiChannelFlag)
	{
		if(!lcd_refreshdata_RGB(DISPLAY_LCD_0, &stRect, bDisplayBuf))
		{
			Fwl_Print(C2, M_DISPLAY, "Dev_RefreshDisplayByColor: lcd_refreshdata_RGB error.\n");	
		}
		
	}else
	{
		if(!lcd_refresh_RGB(DISPLAY_LCD_0, &stRect, bDisplayBuf))
		{
			Fwl_Print(C2, M_DISPLAY, "Dev_RefreshDisplayByColor: lcd_refresh_RGB error.\n");	
		}
	}
#else
	lcd_refresh(DISPLAY_LCD_0, stRect.left, stRect.top, stRect.width, stRect.height);
#endif

	Dev_DisplayUnlock();
}


T_VOID Dev_Set_Asyn_RefreshCbf(T_REFRESH_CALLBACK  pFuncCB)
{
#ifdef OS_ANYKA
//	lcd_set_refresh_finish_callback(pFuncCB);
#endif
}

T_VOID Dev_Asyn_RefreshDisplay(T_RECT *dsp_rect, T_U8 *dsp_buf, T_U8 *addr, T_U32 origin_width, T_U32 origin_height)
{
#ifdef OS_ANYKA
	T_RECT Ref_Rect;
	DISPLAY_TYPE_DEV dispType;

	dispType =   Dev_GetDisplayType();		
	Ref_Rect.left = 0;
	Ref_Rect.top  = 0;
	Ref_Rect.width = stDevCap[dispType].left*2 + stDevCap[dispType].width;
	Ref_Rect.height = stDevCap[dispType].top*2 + stDevCap[dispType].height;
		
	if((origin_width == Ref_Rect.width) && (origin_height == Ref_Rect.height)
		&& (dispType < DISPLAY_TVOUT_PAL))
	{
		T_U32 i = 0;
		T_U16* dstPtr;
		T_U16* srcPtr;

		dstPtr = (T_U16*)bDisplayBuf + 
				 	(dsp_rect->top * Ref_Rect.width + dsp_rect->left);
		srcPtr = (T_U16*)dsp_buf + 
					(dsp_rect->top * Ref_Rect.width + dsp_rect->left);
		
		/*Sometimes only update part area of buf*/
		for(i=0; i<dsp_rect->height; i++)
		{
			memcpy(dstPtr, srcPtr, dsp_rect->width*sizeof(T_U16));
			dstPtr+=Ref_Rect.width;
			srcPtr+=Ref_Rect.width;
		}
	}
	else
	{
		T_RECT srcRect;
		T_RECT dstRect;
		
		srcRect.height = origin_height;
		srcRect.width  = origin_width;
		srcRect.left   = 0;
		srcRect.top	   = 0;

		if(dispType < DISPLAY_TVOUT_PAL)
		{
			dstRect.height = Ref_Rect.height;
			dstRect.width  = Ref_Rect.width;
			dstRect.left   = Ref_Rect.left;
			dstRect.top    = Ref_Rect.top;
		}
		else
		{
			dstRect.left = stDevCap[dispType].left;
			dstRect.top  = stDevCap[dispType].top<<1;
			dstRect.width = stDevCap[dispType].width;
			dstRect.height = stDevCap[dispType].height<<1;
		}
		
		/*if origin buffer size no equal to DisplayBuf, do 2D scale */
		Fwl_RGB565BitBlt(dsp_buf, origin_width, &srcRect,
							bDisplayBuf, Ref_Rect.width, FORMAT_RGB565, &dstRect);
	}

	//lcd_asyn_refresh_RGB(DISPLAY_LCD_0, &Ref_Rect, bDisplayBuf, addr);	
#endif
}


#if 0
T_VOID Dev_RefreshDisplayTVOUT_Fast(T_VOID)
{
	
	T_RECT  stRect;	
	T_RECT	stVirRect;

	Dev_DisplayLock();
	
	stRect.left = 0;
	stRect.top  = 0;
	stRect.width = stDevCap[DisplayType].left*2 + stDevCap[DisplayType].width;
	stRect.height = stDevCap[DisplayType].top*2 + stDevCap[DisplayType].height;

	stVirRect.left	 = 0;
	stVirRect.top	 = 0;
	stVirRect.width  = stRect.width << 1;
	stVirRect.height = stRect.height;

#ifdef OS_AMYKA
	if(!lcd_refresh_RGB_ex(DISPLAY_LCD_0, &stRect, 
			&stVirRect, bDisplayBuf))
	{			
		Fwl_Print(C2, M_DISPLAY, "Dev_RefreshDisplayTVOUT_Fast: lcd_refresh_RGB_ex error.\n");	
	}
#endif

	Dev_DisplayUnlock();
}

#endif

T_VOID Dev_CleanFrameBuf(void)
{
	T_U32 i;
	T_U32 *pu32;
	T_U16 *pu16;
	T_U8  *pu8;

	if(AK_NULL == bDisplayBuf)
	{
		Fwl_Print(C1, M_DISPLAY, "Dev_CleanFrameBuf() bDisplayBuf==NULL!\n");		
	}
	
	if(!((T_U32)bDisplayBuf & 0x03))
	{
		pu32 = (T_U32*)bDisplayBuf;
		
		for(i=0; i<DISPLAY_BUF_SIZE >> 2; ++i)
		{
			*pu32 = 0;
			++pu32;
		}
		
		Fwl_Print(C4, M_DISPLAY, "CleanDspBuf T_U32.\n");
	}
	else if(!((T_U32)bDisplayBuf & 0x01))
	{
		pu16 = (T_U16*)bDisplayBuf;
		
		for( i= 0; i < DISPLAY_BUF_SIZE>>1; ++i)
		{
			*pu16 = 0;
			++pu16;
		}
		
		Fwl_Print(C4, M_DISPLAY, "CleanDspBuf T_U16.\n");
	}
	else
	{
		pu8= bDisplayBuf;
		
		for(i = 0; i < DISPLAY_BUF_SIZE; ++i)
		{
			*pu8 = 0;
			++pu8;
		}
		
		Fwl_Print(C4, M_DISPLAY, "CleanDspBuf T_U8.\n");
	}
	
}


T_VOID Dev_SetDisplayType(DISPLAY_TYPE_DEV type)
{
	DISPLAY_TYPE_DEV  dispType;
	
    Fwl_Print(C4, M_DISPLAY, "Dev_SetDisplayType type = %d",type);
	
	if(type >= DISPLAY_MAX_TYPE)
	{
		Fwl_Print(C3, M_DISPLAY, "Dev_SetDisplayType : DISPLAY_TYPE_DEV error!\n");
		return;
	}

	if(Dev_GetDisplayType() ==  type) /* !(DisplayType ^ type) may be good */
		return;
	
	Dev_DisplayLock();

	dispType = Dev_GetDisplayType();
	
	/*put code here to enter display type*/
#ifdef OS_ANYKA
	switch(type)
	{
	case DISPLAY_LCD_0:		
	case DISPLAY_LCD_1:
//        lcd_tvout_clk_sel (TVOUT_CLK_EXTERNAL);
        
		if(dispType > DISPLAY_LCD_1)
		{
			lcd_tv_out_close();
		}
		else
		{			
			lcd_set_brightness(dispType, 0);
			lcd_turn_off(dispType);
		}
		//lcd_turn_on(type);
		//lcd_set_brightness(type,bBrightness[type]);
#if (defined(CHIP_AK3750) || defined(CHIP_AK3753))
		Fwl_LcdRotate(LCD_270_DEGREE);
#else //3760
		Fwl_LcdRotate(LCD_90_DEGREE);
#endif
		break;
		
	case DISPLAY_TVOUT_PAL:
	case DISPLAY_TVOUT_NTSC:
//        lcd_tvout_clk_sel (TVOUT_CLK_INTERNAL);      
    
		Dev_CleanFrameBuf(); 

		if(dispType < DISPLAY_TVOUT_PAL)
		{    
			lcd_set_brightness(dispType, 0);

			//resolve: show remnants inner frame when MPU panel is opened
			if (E_LCD_TYPE_MPU == lcd_get_type())
			{
				T_RECT 	stRect;
				T_U8 	colorSpace;
				T_U8 *	bBuf;

				bBuf = Dev_GetFrameBuf(&stRect, &colorSpace);
				lcd_refresh_RGB(DISPLAY_LCD_0, &stRect, bBuf);
			}
		}
		else
		{
			//解决TVOUT时N制式切换P制式杂乱无章画面
			lcd_turn_off(LCD_0);
        	//delay for TVOUT -PAL<->NTSC    
        	AK_Sleep(250);
		}
		
		lcd_tv_out_open(type - DISPLAY_TVOUT_PAL);		
		break;
		
	default:
		Fwl_Print(C3, M_DISPLAY, "Dev_SetDisplayType : no this type\n");
		return ;
	}
#endif

	DisplayType = type;

	Dev_DisplayUnlock();
}


DISPLAY_TYPE_DEV  Dev_GetDisplayType(T_VOID)
{
	return DisplayType;
}

T_U8* Dev_GetFrameBuf (T_RECT* pstRectLay, T_U8* ColorSpace)
{
	DISPLAY_TYPE_DEV dispType = Dev_GetDisplayType();
	/*如何检测是否全屏？*/
	pstRectLay->left 	= 0;
	pstRectLay->top 	= 0;
	pstRectLay->width 	= stDevCap[dispType].left*2 + stDevCap[dispType].width;	
	pstRectLay->height 	= stDevCap[dispType].top*2 + stDevCap[dispType].height;	

#ifdef CHIP_AK98XX	
	*ColorSpace = RGB888;
#else
	*ColorSpace = RGB565;
#endif

	if(AK_NULL == bDisplayBuf)
	{
		Fwl_Print(C1, M_DISPLAY, "Dev_GetFrameBuf() bDisplayBuf==NULL!\n");	
		while(1);
	}

	return	bDisplayBuf;
}

T_BOOL Dev_GetFrameRect(DISPLAY_TYPE_DEV dispMode, T_U32 *pWidth, T_U32 *pHeight,T_RECT *clipRect)
{
    if (AK_NULL != pWidth)
    {
        (*pWidth)       = stDevCap[dispMode].left*2 + stDevCap[dispMode].width; 
    }

    if (AK_NULL != pHeight)
    {
        if (dispMode >= DISPLAY_TVOUT_PAL)
        {
            (*pHeight)	= stDevCap[dispMode].top*4 + (stDevCap[dispMode].height  << 1); 
        }
        else
        {
            (*pHeight)	= stDevCap[dispMode].top*2 + (stDevCap[dispMode].height); 
        }
    }

    if (AK_NULL != clipRect)
    {
        clipRect->left   = stDevCap[dispMode].left;
        clipRect->top    = stDevCap[dispMode].top;
        clipRect->width  = stDevCap[dispMode].width;
		
        if (dispMode >= DISPLAY_TVOUT_PAL)
        {
            clipRect->height = (stDevCap[dispMode].height << 1);
        }
        else
        {
            clipRect->height = stDevCap[dispMode].height;
        }
    }

	return	AK_TRUE;
}

T_U8 Dev_SetBrightness(DISPLAY_TYPE_DEV lcd_type, T_U8 brightness)
{
	if(lcd_type > DISPLAY_LCD_1)
	{
		Fwl_Print(C3, M_DISPLAY, "Dev_SetBrightness : DISPLAY_TYPE_DEV error!\n");
		return	0;
	}

	Dev_DisplayLock();
	
	lcd_set_brightness(lcd_type,brightness);
	bBrightness[lcd_type] = brightness;
	
	Dev_DisplayUnlock();
	
	return	brightness;
}


T_U8 Dev_GetBrightness(DISPLAY_TYPE_DEV lcd_type)
{
	if(lcd_type > DISPLAY_LCD_1)
	{
		Fwl_Print(C3, M_DISPLAY, "Dev_GetBrightness : DISPLAY_TYPE_DEV error!\n");
		return	0;
	}

	return	bBrightness[lcd_type] ;
}

/*just for lcd driver,don't use in other place*/
E_TV_OUT_TYPE TvOut_GetType(T_VOID)	
{
	DISPLAY_TYPE_DEV dispType = Dev_GetDisplayType();
	
	if(dispType >= DISPLAY_TVOUT_PAL)
	{
		return dispType - DISPLAY_TVOUT_PAL;
	}
	else
	{
		return dispType;
	}
}

#ifdef CI37XX_PLATFORM
T_VOID Dev_Refresh_YUV1(T_eLCD lcd, T_U8 *srcY, T_U8 *srcU,T_U8 *srcV,
						T_U16 src_width, T_U16 src_height,T_U16 left, 
						T_U16 top, T_U16 dst_width, T_U16 dst_height)
{
	T_U32   height_temp;
	T_U32   width_temp;
	DISPLAY_TYPE_DEV dispType;	
	T_RECT   stRectYUV1;
	
	if(src_width * src_height < 18*18)
	{
		return ;
	}	

	Dev_DisplayLock();
	
	dispType = Dev_GetDisplayType();
	
	if(dispType > DISPLAY_LCD_1)
	{
		if((0 == left) && (0 == top) 
			/*&&  (dst_height == stDevCap[DISPLAY_LCD_0].height)*/)
		{
			dst_width = dst_width * stDevCap[dispType].width / stDevCap[DISPLAY_LCD_0].width;
			dst_height = dst_height * stDevCap[dispType].height	/ stDevCap[DISPLAY_LCD_0].height;
		}
		else
		{
			if((0 == left) || (0 == top))
			{
				height_temp = (stDevCap[dispType].height) * 
					(top * 2 + dst_height) / stDevCap[DISPLAY_LCD_0].height;
				width_temp = (stDevCap[dispType].width) * 
					(left * 2 + dst_width ) / stDevCap[DISPLAY_LCD_0].width;

				if(dst_width*height_temp > dst_height*width_temp)
				{
					left = 0;
					dst_height = (T_U16)(width_temp*dst_height/dst_width) ;				
					dst_width  = (T_U16)width_temp;
					top = (T_U16)((height_temp-dst_height) /2) ;
					dst_height = dst_height;

				}
				else
				{
					top = 0;
					dst_width = (T_U16)(height_temp*dst_width/dst_height);				
					left = (T_U16)((width_temp-dst_width)/2);
					dst_height = (T_U16)(height_temp);
				}
			}
			else	/*(0 != left) && (0 != top)*/
			{
				top = (stDevCap[dispType].height ) * 
					top  / stDevCap[DISPLAY_LCD_0].height  ;
				left = (stDevCap[dispType].width) * 
					left  / stDevCap[DISPLAY_LCD_0].width;
		
				dst_width = dst_width*stDevCap[dispType].width/
					stDevCap[DISPLAY_LCD_0].width;

				dst_height = dst_height*(stDevCap[dispType].height)/
					stDevCap[DISPLAY_LCD_0].height ;			
				
			}


		}
		
		left += stDevCap[dispType].left;
		top  += stDevCap[dispType].top;
	}
	
	stRectYUV1.top 		= top;
	stRectYUV1.left 	= left;
	stRectYUV1.width 	= dst_width;
	stRectYUV1.height 	= dst_height;

	if(m_multiChannelFlag)
		lcd_refreshdata_YUV1(lcd, srcY, srcU, srcV, src_width, src_height, &stRectYUV1);
	else
		lcd_refresh_YUV1(lcd, srcY, srcU, srcV, src_width, src_height, &stRectYUV1);
	
	Dev_DisplayUnlock();
}


#if 0
T_VOID Dev_Refresh_YUV2(T_eLCD lcd, T_U8 *srcY,T_U8 *srcU,T_U8 *srcV,T_U16 src_width, 
						T_U16 src_height,T_U16 left, T_U16 top,T_U16 dst_width, 
						T_U16 dst_height)
{
    T_RECT  stRect;    

	if(0 == srcY_YUV1)
	{		
		return ;
	}

	Dev_DisplayLock();
	
    if(DisplayType > DISPLAY_LCD_1)
    {
        left = (stDevCap[DisplayType].width) * 
					left  / stDevCap[DISPLAY_LCD_0].width;
        top = (stDevCap[DisplayType].height ) * 
					top  / stDevCap[DISPLAY_LCD_0].height  ;
				
		dst_width = dst_width*stDevCap[DisplayType].width/			
                        stDevCap[DISPLAY_LCD_0].width;
        
        dst_height = dst_height*(stDevCap[DisplayType].height  )/  
                        stDevCap[DISPLAY_LCD_0].height ;

		left += stDevCap[DisplayType].left;
		top  += stDevCap[DisplayType].top;
    }   
    
    stRect.left = left;
    stRect.top = top;
    stRect.width = dst_width;
    stRect.height = dst_height;   

#ifdef OS_ANYKA
    lcd_refresh_dual_YUV(lcd,srcY_YUV1,srcU_YUV1,srcV_YUV1,
                         src_width_YUV1, src_height_YUV1,
                         &stRectYUV1,
                         srcY,srcU,srcV,
                         src_width, src_height,
                         &stRect);
#endif
	Dev_DisplayUnlock();
}
#endif

T_VOID Dev_TurnOff_YUV(T_VOID)
{	
#ifdef OS_ANYKA
	Dev_DisplayLock();

	lcd_YUV_off();
	
	Dev_DisplayUnlock();
#endif
}

T_BOOL Dev_RefreshRect_Fast(void * imgBuf, int imgWidth, int imgHeight, T_BOOL FullScreen)

{
	T_U8	*video_buffer = AK_NULL;
	T_RECT	stRect;
	T_RECT  nstRect;
	T_U16	DstHeight;
	T_U16	DstWidth;
	T_U16	dstX;
	T_U16	dstY;
	T_U8	ColorSpace;
	
	if(imgBuf == AK_NULL)
	{
		 Fwl_Print(C3, M_DISPLAY, "Fwl_RefreshRect_Fast(): imgBuf is NULL\n");
		 return AK_FALSE;
	}
	
	Dev_DisplayLock();
	
	video_buffer = Dev_GetFrameBuf(&stRect, &ColorSpace);
	if(video_buffer == AK_NULL)
	{
		Dev_DisplayUnlock();
		Fwl_Print(C3, M_DISPLAY, "Fwl_RefreshRect_Fast(): video_buffer is NULL\n");
		return AK_FALSE;
	}

	Dev_GetDisplayCaps(Dev_GetDisplayType(),&nstRect);

	if (FullScreen)  //full screen
	{
		DstHeight = nstRect.height;
		DstWidth  = nstRect.width;
		dstX = nstRect.left; 
		dstY = nstRect.top;
	}
	else
	{
		DstHeight = imgHeight;
		DstWidth  = imgWidth;
		if(Dev_GetDisplayType() < DISPLAY_TVOUT_PAL)  //LCD
		{
			dstX = (stRect.width- imgWidth) / 2;
			dstY = (stRect.height- imgHeight) / 2;
		}
		else  //TVOUT
		{			
			dstX = (stRect.width- imgWidth) / 2;
			dstY = (stRect.height*2- imgHeight) / 2;
			DstHeight = DstHeight >> 1;
			dstY = dstY >> 1;
		}
	}
	
	if(Dev_GetDisplayType() < DISPLAY_TVOUT_PAL)  //LCD
	{
#ifdef OS_ANYKA
		if(0 > Fwl_Scale_Convert(video_buffer, DstWidth, DstHeight, dstX, dstY, 
					stRect.width, imgBuf, imgWidth, imgHeight, RGB565))
		{
			Dev_DisplayUnlock();
			
			Fwl_Print(C3, M_DISPLAY, "Fwl_RefreshRect_Fast(): Fwl_Scale_Convert() LCD error.\n");	
			return AK_FALSE;
		}
		
		//当从全屏切为窗口显示，则对窗口区之外的区域做一次清屏
		if (AK_FALSE == FullScreen)
		{
			T_S32 width, height;
			
			for(height = 0; height < stRect.height; height++)
				for(width = 0; width < stRect.width; width++)
				{
					if (width < dstX || width >= dstX + DstWidth 
						|| height < dstY || height >= dstY + DstHeight)
					{
						memset(video_buffer + (height*stRect.width + width)*BYTES_PER_PIXEL, 
							0, BYTES_PER_PIXEL);
					}
				}
		}

		if(!lcd_refresh_RGB(DISPLAY_LCD_0, &stRect, video_buffer))
		{
			Dev_DisplayUnlock();
			
			Fwl_Print(C3, M_DISPLAY, "Fwl_RefreshRect_Fast(): lcd_refresh_RGB() error.\n");
			return AK_FALSE;
		}
#endif
		
	}
	else  //TV_OUT
	{   
#ifdef OS_ANYKA
		T_RECT	stVirRect;
		
		if(0 > Fwl_Scale_Convert(video_buffer,DstWidth, DstHeight<<1, dstX, dstY<<1, 
					stRect.width, imgBuf, imgWidth, imgHeight, RGB565))
		{
			Dev_DisplayUnlock();
			
			Fwl_Print(C3, M_DISPLAY, "Fwl_RefreshRect_Fast(): Fwl_Scale_Convert() TVOUT error.\n");
			return AK_FALSE;
		}	

		stVirRect.left	 = 0;
		stVirRect.top	 = 0;
		stVirRect.width  = stRect.width<<1;
		stVirRect.height = stRect.height;
		
		if(!lcd_refresh_RGB_ex(DISPLAY_LCD_0, &stRect, &stVirRect, video_buffer))
		{
			Dev_DisplayUnlock();
			
			Fwl_Print(C3, M_DISPLAY, "Fwl_RefreshRect_Fast(): lcd_refresh_RGB_ex() error.\n");
			return AK_FALSE;
		}
#endif
	}

	Dev_DisplayUnlock();
	
	return AK_TRUE;
}

#if (SDRAM_MODE == 8)
T_VOID Dev_FreeBuf(T_VOID)
{	
	if (AK_NULL != bDisplayBuf)
    {
        bDisplayBuf = FREE(bDisplayBuf);
        bDisplayBuf = AK_NULL;
		Fwl_Print(C3, M_DISPLAY, "FREE for bDisplayBuf[%d] OK.\n\n\n", DISPLAY_BUF_SIZE);
    }	
}

T_VOID Dev_ReMallocBuf(T_VOID)
{
	if(AK_NULL == bDisplayBuf)
	{
		bDisplayBuf = (T_U8*) MALLOC(DISPLAY_BUF_SIZE);
		if(AK_NULL == bDisplayBuf)
		{
			Fwl_Print(C1, M_DISPLAY, "ReMalloc for bDisplayBuf fail!\n");
			while(1);
		}
	}
	
	Fwl_Print(C3, M_DISPLAY, "ReMalloc for bDisplayBuf[%d] OK.\n\n\n", DISPLAY_BUF_SIZE);
}
#endif

#endif /*CHIP_AK98XX*/
