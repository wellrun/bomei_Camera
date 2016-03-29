/********************************************************************
File:
Date:

Author:

Descrip:



*********************************************************************/


#include "Fwl_display.h"
#include "Dev_display.h"
#include "arch_gui.h"
#include "Display_func_common.h"
#include "ImageLayer.h"
#include "dev_display.h"
#include "fwl_graphic.h"
#include "Fwl_DispOsd.h"
            
typedef struct 
{
	T_RECT			stLayerRect;			//图层的区域信息
	LAYER_TYPE		LayerType;				//图层的类型
	T_U8 * 			pBufLayer ;				//图层的buffer指针
	T_U8 			AlphaBlend ;			//透明度
	T_U8			bFlagRefresh ;			//是否要混合标志，1：混合；0：不需混合
}T_LAYER_INFO_MIX;

static T_LAYER_INFO_MIX    MixLayer(void);
static T_S32		       GetLayerInfoMix(HLAYER  hLayer, T_LAYER_INFO_MIX  *pstLayerInfoMix);


static	T_INIT_DISP_ST	stPlatInfo;	/*= {0,0};*/ /*init in Fwl_InitDisplay()*/
static  T_BOOL          bLcdTurnOn;

#ifdef	CHIP_AK98XX	/*CHIP_AK98XX*/
HLAYER	Fwl_hRGBLayer, Fwl_hYUVLayer, Fwl_hYUV2Layer, Fwl_hOSDLayer;
#else
#ifdef CI37XX_PLATFORM
HLAYER	Fwl_hRGBLayer, Fwl_hYUVLayer;
#else
#error "Must define CHIP_AK98XX or CI37XX_PLATFORM!"
#endif
#endif

T_BOOL	Fwl_InitDisplay(T_INIT_DISP_ST * pstPlatformInfo)
{
	Fwl_Print(C4, M_DISPLAY, "Enter Fwl_InitDisplay\n");
	
	stPlatInfo = *pstPlatformInfo;

	if(IMGLAY_OK != ImgLay_ListInit())
	{
		return AK_FALSE;
	}
	
#ifdef CI37XX_PLATFORM
#ifdef LCD_MODE_565
	Fwl_hRGBLayer = Fwl_CreateLayer(RGB565, 0, 0, stPlatInfo.WidthLcd, stPlatInfo.HeightLcd, HAVE_BUFFER, 100);
#else
	Fwl_hRGBLayer = Fwl_CreateLayer(RGB888, 0, 0, stPlatInfo.WidthLcd, stPlatInfo.HeightLcd, HAVE_BUFFER, 100);
#endif
	Fwl_hYUVLayer = Fwl_CreateLayer(YUV420, 0, 0, stPlatInfo.WidthLcd, stPlatInfo.HeightLcd, NO_HAVE_BUFFER, 100);
	
	/*Fwl_hOSDLayer = */
	if(!(Fwl_hRGBLayer && Fwl_hYUVLayer))
	{
		Fwl_Print(C2, M_DISPLAY, "Fwl_InitDisplay CreateLayer Fail.\n");
		while(1);
	}
#else
#error "Must define CI37XX_PLATFORM!"
#endif

	Fwl_Print(C4, M_DISPLAY, "Fwl_InitDisplay CreateLayer Succeed.\n");


	Dev_InitDisplay();
	Fwl_Osd_Init();

	return AK_TRUE;
}


T_VOID  Fwl_ReleaseDisplay( T_VOID )
{
	/*Put someting here if need*/	

	Fwl_Print(C4, M_DISPLAY, "Fwl_ReleaseDisplay do nothing.\n");

}


HLAYER  Fwl_CreateLayer(LAYER_TYPE LayerType, T_U16 x,T_U16 y, T_U16 Width, 
						T_U16 Height, T_U8 FlagHaveBuf, T_U8 AlphaBlend)
{
	T_RECT	stRect;
	HLAYER	hLayer;

	stRect.left 	= x;
	stRect.top		= y;
	stRect.width 	= Width;
	stRect.height 	= Height;

	Fwl_Print(C4, M_DISPLAY, "Enter Fwl_CreateLayer\n");

	hLayer = ImgLay_New(0, stRect, (T_S8)LayerType, (T_BOOL)FlagHaveBuf, 
		(T_S8)AlphaBlend, 0);
	
	if(hLayer)
	{
		if(IMGLAY_OK != ImgLay_AddLayer(hLayer))
		{
			Fwl_Print(C3, M_DISPLAY, "Fwl_CreateLayer ImgLay_AddLayer error.\n");			
			ImgLay_DelLayer(hLayer);
			return AK_NULL;
		}
	}

	return	hLayer;
}



T_BOOL  Fwl_DeleteLayer(HLAYER  hLayer)
{    
	if (AK_NULL != ImgLay_DelLayer(hLayer))
	{
		ImgLay_Free(hLayer);
		return AK_TRUE;
	}
	
	return AK_FALSE;
}


T_BOOL	Fwl_GetLayerInfo(HLAYER  hLayer,T_LAYER_INFO  *pstLayerInfo)
{
	T_RECT   *pstRect;

	if(AK_NULL != (pstRect = ImgLay_GetArea(hLayer)))
	{
		pstLayerInfo->stLayerRect = *pstRect;
	}
	else
	{
		return AK_FALSE;
	}	  
	
	if(0x0FF == (pstLayerInfo->LayerType = ImgLay_Format(hLayer)))
	{
		return AK_FALSE;
	}
	
	pstLayerInfo->pBufLayer = ImgLay_GetBuf(hLayer);
	
	if(0x0FF == (pstLayerInfo->AlphaBlend	= ImgLay_GetAlpha(hLayer)))
	{
		return AK_FALSE;
	}
	
	return	AK_TRUE;
}


T_BOOL	Fwl_SetLayerInfo (HLAYER  hLayer, T_LAYER_INFO  *pstLayerInfo)
{

	if(IMGLAY_OK != ImgLay_SetArea(hLayer,pstLayerInfo->stLayerRect))
	{
		return AK_FALSE;
	}
	
	/*ignore format */
   	
	ImgLay_SetBuf(hLayer,pstLayerInfo->pBufLayer);

	if(IMGLAY_OK != ImgLay_SetAlpha(hLayer,pstLayerInfo->AlphaBlend))
	{
        return AK_FALSE;
	}

#ifdef CI37XX_PLATFORM
	if((0 == pstLayerInfo->AlphaBlend) && (hLayer == Fwl_hYUVLayer))
	{
		Dev_TurnOff_YUV();
	}
#endif  /*CHIP_AK98XX*/	
	return	AK_TRUE;
}



#if 0
T_BOOL  Fwl_RefreshRect(HLAYER hLayer, T_U8 *srcData, T_U16 src_width, 
					  T_U16 src_height, T_U16 x, T_U16 y,T_U16 dst_width,
					  T_U16 dst_height, T_U8 FlagShowDirect)
{
	T_RECT	stRect;	
	T_LAYER_INFO_MIX	lay_info_mix;

	stRect.left = x;
	stRect.top = y;
	stRect.width = dst_width;
	stRect.height = dst_height;

	Fwl_Print(C4, M_DISPLAY, "Enter Fwl_InitDisplay\n");

	GetLayerInfoMix( hLayer, &lay_info_mix );	//Will it always succeed?

#ifdef CI37XX_PLATFORM
	if(hLayer == Fwl_hRGBLayer)
	{
		ImgLay_RefreshRect(hLayer, stRect, srcData, src_width, src_height, ImgLay_Format(hLayer));

		if(1 == FlagShowDirect)
		{
			if(bLcdTurnOn)		
			{
				Dev_RefreshDisplay(srcData, lay_info_mix.stLayerRect.width,
					lay_info_mix.stLayerRect.height, lay_info_mix.LayerType);
			}
		}
	}
	
	else if(hLayer == Fwl_hYUVLayer)
	{
		
		if(bLcdTurnOn)		
		{
			Dev_Refresh_YUV1 (DISPLAY_LCD_0, srcData, srcData + src_width * src_height,
				srcData + (((src_width * src_height)*5)>>2), src_width,  src_height,
				x, y, dst_width, dst_height);
		}
	}
	else
	{
		/*should add something here for other case*/
		Fwl_Print(C3, M_DISPLAY, "Fwl_RefreshRect: wrong hLayer\n");
		return AK_FALSE;
	}

#endif	

	return  AK_TRUE;

}
#endif

/*LHS add  for game display fast .ColorType must be GRB888*/
T_BOOL Fwl_RefreshRect_Fast(void * imgBuf, int imgWidth, int imgHeight, T_BOOL FullScreen)
{
	if(!bLcdTurnOn)
		return AK_FALSE;
	
	return Dev_RefreshRect_Fast(imgBuf, imgWidth, imgHeight, FullScreen);
}
	
T_BOOL  Fwl_RefreshRectYUV(HLAYER hLayerYUV, T_U8 *srcY, T_U8 *srcU,T_U8 *srcV, 
						  T_U16 src_width, T_U16 src_height, T_U16 x, T_U16 y,
						  T_U16 dst_width,T_U16 dst_height, T_U8 FlagShowDirect)
{

#ifdef CI37XX_PLATFORM
	if(hLayerYUV == Fwl_hYUVLayer)
	{
		Dev_Refresh_YUV1 (DISPLAY_LCD_0, srcY, srcU, srcV, src_width, src_height,
			x, y, dst_width, dst_height);
	}
	else
	{
		/*should add something here for other case*/
		Fwl_Print(C3, M_DISPLAY, "Fwl_RefreshRectYUV: wrong hLayerYUV\n");
		return AK_FALSE;
	}
#endif	
    
    return  AK_TRUE;

}

T_VOID Fwl_SetMultiChannelDisp(T_BOOL flag)
{
	Dev_SetMultiChannelDisp(flag);
}

T_VOID Fwl_Refresh_Output(T_VOID)
{
	Dev_Refresh_Output();
}
T_BOOL  Fwl_RefreshYUV1( T_U8 *srcY, T_U8 *srcU,T_U8 *srcV, 
						  T_U16 src_width, T_U16 src_height, T_U16 x, T_U16 y,
						  T_U16 dst_width,T_U16 dst_height)

{
	Dev_Refresh_YUV1 (DISPLAY_LCD_0, srcY, srcU, srcV, src_width, src_height,
		x, y, dst_width, dst_height);
		
	return AK_TRUE;
}

T_VOID Fwl_TurnOff_YUV(T_VOID)
{
	Dev_TurnOff_YUV();
}

T_VOID  Fwl_RefreshDisplay(T_VOID)
{
	T_LAYER_INFO_MIX	stLayerInfoMixRGB;

	Fwl_Print(C4, M_DISPLAY,"Enter Fwl_RefreshDisplay\n");
	
	
	/*Obtain  SemaphoreLayer here*/
#ifdef OS_ANYKA
	if(!bLcdTurnOn)
		return;
#endif

	stLayerInfoMixRGB = MixLayer();

	Dev_RefreshDisplay(stLayerInfoMixRGB.pBufLayer,
		stLayerInfoMixRGB.stLayerRect.width,
		stLayerInfoMixRGB.stLayerRect.height,
		stLayerInfoMixRGB.LayerType);
}


/*有些界面(如tvout下的camera 预览界面)，软2D太慢，导致显示很卡，
若该界面的RGB层只需要刷某个颜色时，可以用此接口。
*/
T_VOID  Fwl_RefreshDisplayByColor(T_COLOR color)
{
	Dev_RefreshDisplayByColor(color);
}


T_VOID Fwl_Set_Ref_FinishCbf(T_ASYN_REFRESH_CALLBACK  pFuncCB)
{
	Fwl_Print(C3, M_DISPLAY,"Fwl_SetRefreshFinishCbf %x begin\n", pFuncCB);
#ifdef OS_ANYKA
	Dev_Set_Asyn_RefreshCbf((T_REFRESH_CALLBACK)pFuncCB);
#endif
    Fwl_Print(C3, M_DISPLAY,"Fwl_SetRefreshFinishCbf %x end\n", pFuncCB);
}

T_BOOL Fwl_Asyn_RefDisplay(T_RECT *dsp_rect, T_U8 *dsp_buf, T_U8 *addr, T_U32 origin_width, T_U32 origin_height)
{
#ifdef OS_ANYKA
	Dev_Asyn_RefreshDisplay(dsp_rect, dsp_buf, addr, origin_width, origin_height);
#endif
	return AK_TRUE;
}


#if 0

T_VOID	Fwl_RefreshDisplayTVOUT_Fast(T_VOID)
{
	if(!bLcdTurnOn)
		return;
	
	Dev_RefreshDisplayTVOUT_Fast();
}

#endif

static T_LAYER_INFO_MIX MixLayer(void)
{
	T_LAYER_INFO_MIX	stLayerInfoMixRGB;
	HLAYER				hLayerRGB;
	T_LAYER_INFO_MIX	stLayerInfoMix;
	HLAYER				hLayer;

//	Fwl_Print(C3, M_DISPLAY,"Enter Fwl_InitDisplay\n");


	hLayerRGB = ImgLay_GetFirstLayer();
	
	if(!hLayerRGB)
	{
		Fwl_Print(C3, M_DISPLAY,"MixLayer First layer is NULL.\n");
		while(1);
	}


	GetLayerInfoMix(hLayerRGB,&stLayerInfoMixRGB);

	hLayer = hLayerRGB;

	while(AK_NULL != (hLayer = ImgLay_GetNextLayer(hLayer)))
	{
		GetLayerInfoMix(hLayer,&stLayerInfoMix);

		if((0 == stLayerInfoMix.bFlagRefresh) || 
			(AK_NULL == stLayerInfoMix.pBufLayer))
		{
			continue;
		}
		

		/*should add some code to deal AlphaBlend.Do nothing now*/

#ifdef OS_ANYKA	
		if(YUV420 == stLayerInfoMix.LayerType)
		{
			Img_BitBltYUV(stLayerInfoMixRGB.pBufLayer,
				stLayerInfoMix.stLayerRect.width,stLayerInfoMix.stLayerRect.height,
				stLayerInfoMix.stLayerRect.left,stLayerInfoMix.stLayerRect.top,
				stLayerInfoMixRGB.stLayerRect.width,
				stLayerInfoMix.pBufLayer,
				stLayerInfoMix.pBufLayer + 
				stLayerInfoMix.stLayerRect.width*stLayerInfoMix.stLayerRect.height,
				stLayerInfoMix.pBufLayer + 
				(((stLayerInfoMix.stLayerRect.width*stLayerInfoMix.stLayerRect.height)*5)>>2),
				stLayerInfoMix.stLayerRect.width,stLayerInfoMix.stLayerRect.height);
		}
#if 0
		else if(RGB888 == stLayerInfoMix.LayerType) 
		{
			Fwl_Scale_Convert(stLayerInfoMixRGB.pBufLayer,stLayerInfoMix.stLayerRect.width,
				stLayerInfoMix.stLayerRect.height,
				stLayerInfoMix.stLayerRect.left,stLayerInfoMix.stLayerRect.top,
				stLayerInfoMixRGB.stLayerRect.width,stLayerInfoMix.pBufLayer,
				stLayerInfoMix.stLayerRect.width,stLayerInfoMix.stLayerRect.height,
				stLayerInfoMix.LayerType);
		}
#else
		else if ((RGB565 == stLayerInfoMix.LayerType) || (RGB888 == stLayerInfoMix.LayerType)) 
		{
			Fwl_Scale_ConvertWithAlpha(stLayerInfoMixRGB.pBufLayer,stLayerInfoMix.stLayerRect.width,
					stLayerInfoMix.stLayerRect.height,
					stLayerInfoMix.stLayerRect.left,stLayerInfoMix.stLayerRect.top,
					stLayerInfoMixRGB.stLayerRect.width,stLayerInfoMix.pBufLayer,
					stLayerInfoMix.stLayerRect.width,stLayerInfoMix.stLayerRect.height,
					stLayerInfoMix.LayerType,stLayerInfoMix.AlphaBlend);
		}
#endif
		else
		{
			Fwl_Print(C3, M_DISPLAY,"MixLayer Not support this type.\n");
		}
#endif
	}

	return	stLayerInfoMixRGB;
}



static T_S32  GetLayerInfoMix(HLAYER  hLayer,
							  T_LAYER_INFO_MIX  *pstLayerInfoMix)
{
    //add exception 
	
	pstLayerInfoMix->stLayerRect	= (*ImgLay_GetArea(hLayer));
	pstLayerInfoMix->LayerType		= ImgLay_Format(hLayer);
	pstLayerInfoMix->pBufLayer		= ImgLay_GetBuf(hLayer);
	pstLayerInfoMix->AlphaBlend		= ImgLay_GetAlpha(hLayer);
	pstLayerInfoMix->bFlagRefresh	= ImgLay_Update(hLayer) ;
	
	return	0;
}

T_VOID Fwl_RefreshTVOUT(const T_U8 *y, const T_U8 *u, const T_U8 *v, T_U16 srcW, T_U16 oriW, T_U16 oriH)
{
	Dev_RefreshTVOUT(y, u, v, srcW, oriW, oriH);
}

DISPLAY_TYPE_DEV  Fwl_GetDispalyType(T_VOID)
{
    return	Dev_GetDisplayType();
}

T_BOOL Fwl_GetDisplayCaps (DISPLAY_TYPE_DEV disp_type,T_RECT  *pstRect)
{
	return Dev_GetDisplayCaps(disp_type, pstRect);
}


T_VOID		Fwl_SetDisplayType(DISPLAY_TYPE_DEV type)
{
	Dev_SetDisplayType(type);
	Fwl_Osd_Init();
}

T_BOOL      Fwl_TvoutIsOpen(T_VOID)
{
	if(Dev_GetDisplayType() >= DISPLAY_TVOUT_PAL)
		return AK_TRUE;
	else
		return AK_FALSE;
}


T_U8*		Fwl_GetFrameBuf ( T_RECT  *pstRectLay, T_U8  * ColorSpace )
{
	return Dev_GetFrameBuf(pstRectLay, ColorSpace);
}



    
T_BOOL  Fwl_GetDispFrameRect(DISPLAY_TYPE_DEV dispMode, T_U32 *pWidth, T_U32 *pHeight,T_RECT  *clipRect)
{
    if (dispMode >= DISPLAY_MAX_TYPE)
    {
        dispMode = DISPLAY_LCD_0;
        Fwl_Print(C3, M_DISPLAY, "Fwl_GetDispFrameRect:parm invalid,adjust to lcd parm");
    }
    
    return  Dev_GetFrameRect(dispMode, pWidth, pHeight, clipRect);
}

T_U8*		Fwl_GetFrameBufInfo( T_RECT  *pstRectLay, T_U8  * ColorSpace )
{
	return Dev_GetFrameBuf(pstRectLay, ColorSpace);
}

T_VOID      Fwl_CleanFrameBuf(T_VOID)
{
	Dev_CleanFrameBuf();
}


T_U16       Fwl_GetLcdWidth(void)
{
    T_S32   ret;
	T_RECT  stRect;
    
    ret = Dev_GetDisplayCaps(DISPLAY_LCD_0,&stRect);
    
    if(AK_FALSE == ret)
        return  0;
    
    return  (T_U16)stRect.width;
}


T_U16       Fwl_GetLcdHeight(void)
{
	T_S32   ret;	
	T_RECT  stRect;
    
    ret = Dev_GetDisplayCaps(DISPLAY_LCD_0,&stRect);
    
    if(AK_FALSE == ret)
        return  0;
    
    return  (T_U16)stRect.height;
    
}


T_U8		Fwl_SetBrightness(DISPLAY_TYPE_DEV lcd_type, T_U8 brightness)
{
	if (Fwl_TvoutIsOpen())
	{
		T_S8 tvout_brightness = 0;
		
		switch (brightness)
		{
		case 0:
			tvout_brightness = -127;
			break;
		case 1:
			tvout_brightness = -20;
			break;
		case 2:
			tvout_brightness = -10;
			break;
		case 3:
			tvout_brightness = 0;
			break;
		case 4:
			tvout_brightness = 10;
			break;
		case 5:
			tvout_brightness = 20;
			break;
		case 6:
			tvout_brightness = 30;
			break;
		case 7:
			tvout_brightness = 40;
			break;
		default:
			tvout_brightness = 0;
			break;
		}

		return	Dev_SetBrightness(lcd_type, tvout_brightness);
	}
	else
	{
		return	Dev_SetBrightness(lcd_type,brightness);
	}
}


T_U8		Fwl_GetBrightness(DISPLAY_TYPE_DEV lcd_type)
{
	if (Fwl_TvoutIsOpen())
	{
		T_U8 brightness = 0;
		
		//return	Dev_GetBrightness(lcd_type);
		switch ((T_S8)Dev_GetBrightness(lcd_type))
		{
		case -127:
			brightness = 0;
			break;
		case -20:
			brightness = 1;
			break;
		case -10:
			brightness = 2;
			break;
		case 0:
			brightness = 3;
			break;
		case 10:
			brightness = 4;
			break;
		case 20:
			brightness = 5;
			break;
		case 30:
			brightness = 6;
			break;
		case 40:
			brightness = 7;
			break;
		default:
			brightness = 3;
			break;
		}
		
		return brightness;
	}
	else
	{
		return	Dev_GetBrightness(lcd_type);
	}
}


T_VOID     Fwl_DisplayOn(T_VOID)
{
#ifdef OS_ANYKA    
    Dev_DisplayOn();
	bLcdTurnOn = AK_TRUE;
#endif	
	
}


T_VOID Fwl_DisplayOff(T_VOID)
{
#ifdef OS_ANYKA
    Dev_DisplayOff();  
	bLcdTurnOn = AK_FALSE;
#endif
}


T_VOID Fwl_LcdRotate(DISP_DEGREE rotate)
{
#ifndef	USE_LCD_RGB_OTA5180A
	Dev_LcdRotate(rotate);
#endif
}

DISP_DEGREE Fwl_GetLcdDegree(T_VOID)
{
	return Dev_GetLcdDegree();
}


T_BOOL Fwl_FillRect(HLAYER layer, T_U16 left, T_U16 top, T_U16 width, T_U16 height, T_COLOR color)
{
    T_RECT       stRect ;

    stRect.left = left;
    stRect.top = top;
    stRect.width = width;
    stRect.height = height;
 
    return  ImgLay_FillRect( layer,  stRect,  color);
}



T_BOOL Fwl_DrawLine(HLAYER layer, T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color)
{
	T_POINT p1;
	T_POINT p2;

	p1.x = x1;
	p1.y = y1;

	p2.x = x2;
	p2.y = y2;


	return  ImgLay_DrawLine( layer,  p1,  p2,  color);
}


T_BOOL Fwl_DrawRect(HLAYER layer,  T_POS left, T_POS top, T_LEN width, 
					 T_LEN height, T_COLOR color)
{
	T_RECT  stRect;

	stRect.left = left;
	stRect.top = top;
	stRect.width = width;
	stRect.height = height;
	
	return ImgLay_DrawRect( layer,  stRect,  color);
}

