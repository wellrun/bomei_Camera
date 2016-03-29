#include "Ctl_ImgBrowse.h"

#if (defined (SUPPORT_IMG_BROWSE) || defined (SUPPORT_EMAP))


#include "Eng_ImgConvert.h"
#include "Fwl_pfKeypad.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Eng_ScreenSave.h"
#include "Fwl_Image.h"
#include "Eng_Jpeg2Bmp.h"
#include "Eng_ImgDec.h"
#include "Eng_DataConvert.h"
#include "Eng_Math.h"
#include "fwl_oscom.h"
#include "Eng_AkBmp.h"
#include "lib_image_api.h"
#include "Fwl_pfdisplay.h"
#include "eng_imgconvert.h"

#include "Fwl_tscrcom.h"
#include "fwl_display.h"
#include "fwl_font.h"
#include "Eng_Graph.h"
#include "arch_gui.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_graphic.h"
#include "eng_autopoweroff.h"
#include "Ctl_fm.h"




#define IMAGE_SUPPORT_LARGE

#define IMAGE_BMP_STEPTIME      20
#define IMAGE_JPG_STEPTIME      20    //80
#define IMAGE_REFRESH_COUNT     40

#define IMAGE_BUTTON_INTERVAL   3



#ifndef ZOOM_MIN
#define ZOOM_MIN                10
#define OFFSET_MIN              10
#endif

#if (LCD_CONFIG_WIDTH == 800)        
#define  EAMP_SCALE_INFO_Y 4
#define  EAMP_SCALE_INFO_X 4
#else
#if (LCD_CONFIG_WIDTH == 480)   
#define  EAMP_SCALE_INFO_Y 0
#define  EAMP_SCALE_INFO_X 0
#else
#if (LCD_CONFIG_WIDTH == 320)   
#define  EAMP_SCALE_INFO_Y 6
#define  EAMP_SCALE_INFO_X 6

#else
#error "LCD no match!"
#endif
#endif
#endif

static T_VOID ImgBrowse_Date2Str(T_U32 datetime, T_S8 *string);

static T_VOID ImgBrowse_ZoomIn(T_IMGBROWSE * pImgBrowse, T_U32 changevalue);
static T_VOID ImgBrowse_ZoomOut(T_IMGBROWSE * pImgBrowse, T_U32 changevalue);
static T_VOID ImgBrowse_AddOffsetY(T_IMGBROWSE * pImgBrowse, T_U32 AddValue);
static T_VOID ImgBrowse_SubOffsetY(T_IMGBROWSE * pImgBrowse, T_U32 SubValue);
static T_VOID ImgBrowse_AddOffsetX(T_IMGBROWSE * pImgBrowse, T_U32 AddValue);
static T_VOID ImgBrowse_SubOffsetX(T_IMGBROWSE * pImgBrowse, T_U32 SubValue);
static T_VOID ImgBrowse_RollLeft(T_IMGBROWSE * pImgBrowse, T_U32 RollValue);
static T_VOID ImgBrowse_RollRight(T_IMGBROWSE * pImgBrowse, T_U32 RollValue);
static T_VOID ImgBrowse_RollUp(T_IMGBROWSE * pImgBrowse, T_U32 RollValue);
static T_VOID ImgBrowse_RollDown(T_IMGBROWSE * pImgBrowse, T_U32 RollValue);
static T_BOOL ImgBrowse_Roll(T_IMGBROWSE * pImgBrowse, T_U16 rollAct, T_U32 changeValue);
static T_VOID ImgBrowse_StopSlideTimer(T_IMGBROWSE * pImgBrowse);
static T_VOID ImgBrowse_StartSlideTimer(T_IMGBROWSE * pImgBrowse);
static T_BOOL ImgBrowse_SlideShow(T_IMGBROWSE * pImgBrowse);
static T_U16 ImgBrowse_MappingKey(T_IMGBROWSE * pImgBrowse, T_MMI_KEYPAD phyKey);
static T_BOOL ImgBrowse_OpenNextImg(T_IMGBROWSE * pImgBrowse, T_IMG_ACTION imgAction);
static T_VOID ImgBrowse_RotateImg(T_IMGBROWSE * pImgBrowse);
static T_BOOL ImgBrowse_ShowImgInfo(T_IMGBROWSE * pImgBrowse);
static T_BOOL ImgBrowse_ImgOpen(T_IMGBROWSE * pImgBrowse);

static T_VOID ImgBrowse_StartImgCtlTimer(T_IMGBROWSE * pImgBrowse);
static T_VOID ImgBrowse_StopImgCtlTimer(T_IMGBROWSE * pImgBrowse);
static T_VOID ImgBrowse_ShowEmapSmall (T_IMGBROWSE * pImgBrowse);

static T_VOID ImgBrowse_ImgClose(T_IMGBROWSE * pImgBrowse);
static T_BOOL ImgBrowse_ImgStep(T_IMGBROWSE * pImgBrowse);
//static T_pDATA ImgBrowse_GetOutBuf(T_IMGBROWSE * pImgBrowse);
static T_FILE_TYPE ImgBrowse_GetFileType(T_IMGBROWSE * pImgBrowse);
static T_VOID ImgBrowse_StartDecTimer(T_IMGBROWSE * pImgBrowse);
static T_VOID ImgBrowse_StopDecTimer(T_IMGBROWSE * pImgBrowse);
static T_U32 ImgBrowse_GetImgMaxZoom(T_IMGBROWSE * pImgBrowse);

static T_BOOL ImgBrowse_SetDisPostfixMode(T_IMGBROWSE * pImgBrowse, T_IMG_DIS_POSTFIX DisPostfix);
static T_IMG_DIS_POSTFIX ImgBrowse_GetDisPostfixMode(T_IMGBROWSE * pImgBrowse);
static T_BOOL ImgBrowse_SetDisPixelMode(T_IMGBROWSE * pImgBrowse, T_IMG_DIS_POSTFIX DisPixel);
static T_IMG_DIS_PIXEL ImgBrowse_GetDisPixelMode(T_IMGBROWSE * pImgBrowse);
static T_VOID ImgBrowse_ShowWait(T_IMGBROWSE * pImgBrowse);
static T_VOID ImgBrowse_GetRes(T_IMGBROWSE * pImgBrowse);
static T_VOID ImgBrowse_ShowToolButtons(T_IMGBROWSE * pImgBrowse);
static T_IMG_RET ImgBrowse_UserKey_Handle(T_IMGBROWSE * pImgBrowse, T_MMI_KEYPAD phyKey);
static T_MMI_KEYPAD ImgBrowse_MapTSCR_To_Key(T_IMGBROWSE * pImgBrowse, T_POS x, T_POS y);


T_BOOL                         Flag_ImgBrowse_ClrBuf;
static  T_ICONEXPLORER_ITEM    *pItemFocus_ImgBrowse = AK_NULL;  
static  T_U16  CON_DEFAULT_ZOOM_LEFT = 0; 
static  T_U16  CON_DEFAULT_ZOOM_TOP = 0; 




T_VOID ImgBrowse_ContolPhotoInit(T_IMGBROWSE * pImgBrowse);

T_BOOL ImgBrowse_Init(T_IMGBROWSE * pImgBrowse)
{
    AK_ASSERT_PTR(pImgBrowse, "ImgBrowse_Init(): pImgBrowse ptr error", AK_FALSE);

    memset(pImgBrowse, 0, sizeof(T_IMGBROWSE));
    pImgBrowse->pDisplayList = AK_NULL;

    pImgBrowse->offsetX = 0;
    pImgBrowse->offsetY = 0;
	pImgBrowse->ExOffsetX = 0;
    pImgBrowse->ExOffsetY = 0;
	pImgBrowse->DispPartX= 0;
    pImgBrowse->DispPartY = 0;
	pImgBrowse->IMG_ZOOM_FLAG=AK_FALSE;
	
    pImgBrowse->zoom = 100;

    pImgBrowse->slideTimerId = ERROR_TIMER;
    pImgBrowse->DecTimerId = ERROR_TIMER;
    pImgBrowse->imgCtlTimer= ERROR_TIMER;
    pImgBrowse->imgTimerShowFast= ERROR_TIMER;
    pImgBrowse->IMG_ROTATE_90_FLAG = AK_FALSE;
	pImgBrowse->DEFLAT_FLAG = AK_TRUE;
	pImgBrowse->IMG_GETDATA_FLAG = AK_FALSE;
	pImgBrowse->IMG_MOVE_FLAG = AK_FALSE;
	pImgBrowse->IMG_ZOOM_FLAG = AK_FALSE;
    pImgBrowse->largeFlag = AK_FALSE;


    ImgBrowse_SetDisMode(pImgBrowse, IMG_SLIDE);
    ImgBrowse_SetZoomChangValue(pImgBrowse, 10);
    ImgBrowse_SetOffsetChangValue(pImgBrowse, 10);
    ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_NONE);

    pImgBrowse->bShowBttn = AK_FALSE;

    ImgBrowse_GetRes(pImgBrowse);
		
	ImgBrowse_ContolPhotoInit(pImgBrowse);

    return AK_TRUE;
}

T_VOID ImgBrowse_ContolPhotoInit(T_IMGBROWSE * pImgBrowse)
{
    T_LEN bmp_width = 0;
	T_LEN bmp_height = 0;
	T_U8 deep = 0;

	pImgBrowse->ctrl_photo.ZoomBlock.pData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_PROGRESS_GRAY, &pImgBrowse->ctrl_photo.ZoomBlock.DataLen);
	AKBmpGetInfo(pImgBrowse->ctrl_photo.ZoomBlock.pData, &bmp_width, &bmp_height, &deep);
	CON_DEFAULT_ZOOM_LEFT = (Fwl_GetLcdWidth()-bmp_width)/2;
	CON_DEFAULT_ZOOM_TOP = 30;

	RectInit(&(pImgBrowse->ctrl_photo.ZoomBlock.rect), 
		CON_DEFAULT_ZOOM_LEFT, 
		(T_POS)(Fwl_GetLcdHeight()-bmp_height-CON_DEFAULT_ZOOM_TOP), 
		bmp_width, bmp_height);

	pImgBrowse->ctrl_photo.ZoomSchedule.pData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_PROGRESS, &pImgBrowse->ctrl_photo.ZoomSchedule.DataLen);
	AKBmpGetInfo(pImgBrowse->ctrl_photo.ZoomSchedule.pData, &bmp_width, &bmp_height, &deep);

	RectInit(&(pImgBrowse->ctrl_photo.ZoomSchedule.rect), 
		CON_DEFAULT_ZOOM_LEFT, 
		(T_POS)(Fwl_GetLcdHeight()-bmp_height-CON_DEFAULT_ZOOM_TOP), 
		bmp_width, bmp_height);

	pImgBrowse->ctrl_photo.ZoomPoint.pData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_VOLUME_BLOCK, &pImgBrowse->ctrl_photo.ZoomPoint.DataLen);
	AKBmpGetInfo(pImgBrowse->ctrl_photo.ZoomPoint.pData, &bmp_width, &bmp_height, &deep);

	RectInit(&(pImgBrowse->ctrl_photo.ZoomPoint.rect), 
		(T_POS)(Fwl_GetLcdWidth()/2), 
		(T_POS)(Fwl_GetLcdHeight()-pImgBrowse->ctrl_photo.ZoomSchedule.rect.height-CON_DEFAULT_ZOOM_TOP-(bmp_height-pImgBrowse->ctrl_photo.ZoomSchedule.rect.height)/2), 
		bmp_width, bmp_height);

	pImgBrowse->ctrl_photo.Rotate.pData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_IMAGE_BUTTON_ROTATE, &pImgBrowse->ctrl_photo.Rotate.DataLen);
	AKBmpGetInfo(pImgBrowse->ctrl_photo.Rotate.pData, &bmp_width, &bmp_height, &deep);

	RectInit(&(pImgBrowse->ctrl_photo.Rotate.rect), 
		CON_DEFAULT_LEFT, 
		CON_DEFAULT_TOP, 
		bmp_width, bmp_height);

	pImgBrowse->ctrl_photo.Return.pData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_IMAGE_BUTTON_RETURN, &pImgBrowse->ctrl_photo.Return.DataLen);
	AKBmpGetInfo(pImgBrowse->ctrl_photo.Return.pData, &bmp_width, &bmp_height, &deep);

	RectInit(&(pImgBrowse->ctrl_photo.Return.rect), 
		(T_POS)(Fwl_GetLcdWidth()-bmp_width-CON_DEFAULT_LEFT), 
		CON_DEFAULT_TOP, 
		bmp_width, bmp_height);

}
T_VOID ImgBrowse_ImgShow_Ex(T_IMGBROWSE * pImgBrowse)
{
	T_RECT range;
	T_AK_BMP AkBmp;
	T_U16 lcd_W;
	T_U16 lcd_H;
	T_POS OffsetX = 0;
	T_POS OffsetY = 0;
	T_POS disp_W = 0;
	T_POS disp_H = 0;

	lcd_W = Fwl_GetLcdWidth();
	lcd_H = Fwl_GetLcdHeight();

	if (pImgBrowse->srcW >= lcd_W)
	{
		if (pImgBrowse->virleft < 0)
		{
			OffsetX = 0-pImgBrowse->virleft;
			range.left = OffsetX;
			
			if (pImgBrowse->srcW - OffsetX >= lcd_W)
			{
				range.width = lcd_W;
			}
			else if ((pImgBrowse->srcW - OffsetX < lcd_W)
				&& (pImgBrowse->srcW - OffsetX > 0))
			{
				range.width = (T_LEN)(pImgBrowse->srcW - OffsetX);
			}
			else
			{
				range.width = 0;
			}
			
			disp_W = 0;
		}
		else if (pImgBrowse->virleft <= lcd_W)
		{
			OffsetX = (T_POS)(pImgBrowse->srcW + pImgBrowse->virleft - lcd_W);
			range.left = 0;
			range.width = (T_LEN)(pImgBrowse->srcW - OffsetX);
			disp_W = pImgBrowse->virleft;
		}
		else
		{
			range.left = 0 ;
			range.width = 0;
			disp_W = lcd_W;
		}
	}
	else
	{
		if (pImgBrowse->virleft < 0)
	   	{
			OffsetX = 0-pImgBrowse->virleft;
			range.left = OffsetX ;
			range.width = lcd_W-OffsetX;
			disp_W = 0;
		}
		else if ((pImgBrowse->virleft + pImgBrowse->srcW) >= lcd_W)
		{
			OffsetX = (T_POS)(pImgBrowse->virleft + pImgBrowse->srcW - lcd_W);
			range.left = 0;
			range.width = (T_LEN)(pImgBrowse->srcW - OffsetX);
			disp_W = pImgBrowse->virleft;
		}
		else
		{
			range.left = 0 ;
			range.width = lcd_W;
			disp_W = pImgBrowse->virleft;
		}
	}

	if (pImgBrowse->srcH >= lcd_H)
	{
		if (pImgBrowse->virtop < 0)
		{
			OffsetY = 0-pImgBrowse->virtop;
			range.top = OffsetY;
			
			if (pImgBrowse->srcH - OffsetY >= lcd_H)
			{
				range.height= lcd_H;
			}
			else if ((pImgBrowse->srcH - OffsetY < lcd_H) 
				&& (pImgBrowse->srcH - OffsetY > 0))
			{
				range.height= (T_LEN)(pImgBrowse->srcH - OffsetY);
			}
			else
			{
				range.height= 0;
			}
			
			disp_H = 0;
		}
		else if (pImgBrowse->virtop <= lcd_H)
		{
			OffsetY = (T_POS)(pImgBrowse->srcH + pImgBrowse->virtop - lcd_H);
			range.top= 0;
			range.height= (T_LEN)(pImgBrowse->srcH - OffsetY);
			disp_H = pImgBrowse->virtop;
		}
		else
		{
			range.top= 0 ;
			range.height= 0;
			disp_H = lcd_H;
		}
	}
	else
	{
		if (pImgBrowse->virtop< 0)
	   	{
			OffsetY = 0-pImgBrowse->virtop;
			range.top = OffsetY ;
			range.height = lcd_H - OffsetY;
			disp_H = 0;
		}
		else if ((pImgBrowse->virtop + pImgBrowse->srcH) >= lcd_H)
		{
			OffsetY = (T_POS)(pImgBrowse->virtop + pImgBrowse->srcH - lcd_H);
			range.top = 0;
			range.height = (T_LEN)(pImgBrowse->srcH - OffsetY);
			disp_H = pImgBrowse->virtop;
		}
		else
		{
			range.top = 0 ;
			range.height = lcd_H;
			disp_H = pImgBrowse->virtop;
		}
	}

    if (disp_W < 0)
   	{
		disp_W = 0 - disp_W;
	}
	
	if (disp_H < 0)
   	{
		disp_H = 0 - disp_H;
	}
	
	pImgBrowse->ExOffsetX = range.left;
	pImgBrowse->ExOffsetY = range.top;
	pImgBrowse->DispPartX = disp_W;
	pImgBrowse->DispPartY = disp_H;
	
	AkBmp.BmpData = pImgBrowse->pDatabuf;
	AkBmp.Deep = 16;//pImgBrowse->Deep;
	AkBmp.Frame = 1;
	AkBmp.Width=(T_LEN)pImgBrowse->srcW;
	AkBmp.Height= (T_LEN)pImgBrowse->srcH;

	Fwl_AkBmpDrawPartOnRGB(Fwl_GetDispMemory(),lcd_W, lcd_H, \
	   disp_W, disp_H, &range, &AkBmp, AK_NULL, AK_FALSE, RGB565);

}

T_VOID ImgBrowse_ContolPhotoShow(T_IMGBROWSE * pImgBrowse)
{
	T_U32  lcd_W;
	T_U32  lcd_H;
	T_RECT rect_src;
#ifdef OS_ANYKA
	T_RECT rect_dst;
#endif

	lcd_W = Fwl_GetLcdWidth();
	lcd_H = Fwl_GetLcdHeight();
	
	pImgBrowse->ctrl_photo.ZoomPoint.rect.left = (T_POS)(CON_DEFAULT_ZOOM_LEFT + (MAIN_LCD_WIDTH-CON_DEFAULT_ZOOM_LEFT*2)*(400-pImgBrowse->zoom)/300 - pImgBrowse->ctrl_photo.ZoomPoint.rect.width / 2);

	if (pImgBrowse->ctrl_photo.ZoomPoint.rect.left > (MAIN_LCD_WIDTH - CON_DEFAULT_ZOOM_LEFT - pImgBrowse->ctrl_photo.ZoomPoint.rect.width / 2))
	{
		pImgBrowse->ctrl_photo.ZoomPoint.rect.left = MAIN_LCD_WIDTH - CON_DEFAULT_ZOOM_LEFT - pImgBrowse->ctrl_photo.ZoomPoint.rect.width / 2;
	}
	
	if (pImgBrowse->ctrl_photo.ZoomPoint.rect.left < CON_DEFAULT_ZOOM_LEFT - pImgBrowse->ctrl_photo.ZoomPoint.rect.width / 2)
	{
		pImgBrowse->ctrl_photo.ZoomPoint.rect.left = CON_DEFAULT_ZOOM_LEFT - pImgBrowse->ctrl_photo.ZoomPoint.rect.width / 2;
	}
	
#ifdef OS_ANYKA
	RectInit(&rect_src, 0, 0, pImgBrowse->ctrl_photo.Rotate.rect.width, pImgBrowse->ctrl_photo.Rotate.rect.height);
	RectInit(&rect_dst, pImgBrowse->ctrl_photo.Rotate.rect.left, pImgBrowse->ctrl_photo.Rotate.rect.top, pImgBrowse->ctrl_photo.Rotate.rect.width, pImgBrowse->ctrl_photo.Rotate.rect.height);

	// Rotate ICON
	Fwl_AKBmpAlphaShow(pImgBrowse->ctrl_photo.Rotate.pData, \
		pImgBrowse->ctrl_photo.Rotate.rect.width, rect_src, \
		Fwl_GetDispMemory(),lcd_W, rect_dst, 8);


	RectInit(&rect_src, 0, 0, pImgBrowse->ctrl_photo.Return.rect.width, pImgBrowse->ctrl_photo.Return.rect.height);
	RectInit(&rect_dst, pImgBrowse->ctrl_photo.Return.rect.left, pImgBrowse->ctrl_photo.Return.rect.top, pImgBrowse->ctrl_photo.Return.rect.width, pImgBrowse->ctrl_photo.Return.rect.height);

	// Return ICON
	Fwl_AKBmpAlphaShow(pImgBrowse->ctrl_photo.Return.pData, \
		pImgBrowse->ctrl_photo.Return.rect.width, rect_src, \
		Fwl_GetDispMemory(),lcd_W, rect_dst, 8);

#else	// OS_WIN32
	// Rotate ICON
	Fwl_AkBmpDrawFromStringOnRGB(Fwl_GetDispMemory(),lcd_W, lcd_H,\
		pImgBrowse->ctrl_photo.Rotate.rect.left,\
		pImgBrowse->ctrl_photo.Rotate.rect.top, \
		pImgBrowse->ctrl_photo.Rotate.pData, \
		&g_Graph.TransColor, AK_FALSE, RGB565);

	// Return ICON
	Fwl_AkBmpDrawFromStringOnRGB(Fwl_GetDispMemory(),lcd_W, lcd_H,\
		pImgBrowse->ctrl_photo.Return.rect.left,\
		pImgBrowse->ctrl_photo.Return.rect.top, \
		pImgBrowse->ctrl_photo.Return.pData, \
		&g_Graph.TransColor,AK_FALSE, RGB565);
#endif
	// Zoom Background
	Fwl_AkBmpDrawAlphaFromStringOnRGB(Fwl_GetDispMemory(),lcd_W, lcd_H,\
		pImgBrowse->ctrl_photo.ZoomBlock.rect.left,\
		pImgBrowse->ctrl_photo.ZoomBlock.rect.top, \
		pImgBrowse->ctrl_photo.ZoomBlock.pData, \
		&g_Graph.TransColor,AK_FALSE, RGB565);

	RectInit(&rect_src, 0, 0, 
			(T_POS)(pImgBrowse->ctrl_photo.ZoomPoint.rect.left - CON_DEFAULT_ZOOM_LEFT + pImgBrowse->ctrl_photo.ZoomPoint.rect.width / 2), 
			pImgBrowse->ctrl_photo.ZoomSchedule.rect.height);
	Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pImgBrowse->ctrl_photo.ZoomSchedule.rect.left,\
		pImgBrowse->ctrl_photo.ZoomSchedule.rect.top, &rect_src, pImgBrowse->ctrl_photo.ZoomSchedule.pData, \
		&g_Graph.TransColor, AK_FALSE);

	// Zoom Point	
	Fwl_AkBmpDrawFromStringOnRGB(Fwl_GetDispMemory(),lcd_W, lcd_H,\
		pImgBrowse->ctrl_photo.ZoomPoint.rect.left,\
		pImgBrowse->ctrl_photo.ZoomPoint.rect.top, \
		pImgBrowse->ctrl_photo.ZoomPoint.pData, \
		&g_Graph.TransColor,AK_FALSE, RGB565);
}

static T_VOID ImgBrowse_ShowEmapSmall (T_IMGBROWSE * pImgBrowse)
{
	T_U8  *pData;
	T_U32 width;
	T_U32 height;
	T_U32 uCount;
	T_U32 w,h;
	T_U16 deep;
	
	
#ifdef OS_ANYKA
	T_U32 offset_x,offset_y;
	T_U16 w_lcd ;
	T_U16 h_lcd ;
	T_U16 w_lcd_tmp ;
	T_U16 h_lcd_tmp ;
#endif

	T_pDATA pDataBuf = AK_NULL;
	
	pData = (T_U8*)ImgBrowse_GetOutBuf(pImgBrowse);

	if (AK_NULL == pData)
	{
		Fwl_Print(C3, M_IMAGE, "AK_NULL == pData ");
		return;
	}

	memcpy((void *)&width, (void *)&pData[0x12], 4);
	memcpy((void *)&height, (void *)&pData[0x16], 4);
	memcpy((void *)&deep, (void *)&pData[0x1c], 2);
	
	if (24 != deep)
	{
		Fwl_Print(C3, M_IMAGE, "EMAP NOT 24 BIT ");
		return;
	}
	
	pDataBuf = Fwl_Malloc(width*height*2+64);
	if (AK_NULL == pDataBuf)
	{
		Fwl_Print(C3, M_IMAGE, "EMAP Small Show Malloc is Fail!");
		return;
	}

	Fwl_Print(C3, M_IMAGE, "width = %d, height = %d.",
		width, height);

	pData += 54;

	pData = pData + ((width*3+3)&(~3)) * (height-1);

	uCount = 0;

	for(h=0; h<height; ++h)
	{
		for(w=0; w < width*3; )
		{
			pDataBuf[uCount++] = ((pData[w+1] << 3) & 0x0E0) | (pData[w] >> 3);
			pDataBuf[uCount++] = (pData[w+2] & 0x0F8) | (pData[w+1] >> 5);
	
			w += 3;
		}	

		pData -= (width*3+3)&(~3) ;
	}
	
#ifdef OS_ANYKA	
	if (width*100/Fwl_GetLcdWidth() > height*100/Fwl_GetLcdHeight())
	{
		w_lcd = Fwl_GetLcdWidth();
		h_lcd = height*Fwl_GetLcdWidth()/width;
		offset_x = 0;
		offset_y = (Fwl_GetLcdHeight()-h_lcd)/2;
	}
	else
	{
		w_lcd = Fwl_GetLcdHeight()*width/height;
		h_lcd = Fwl_GetLcdHeight(); 
		offset_x = (Fwl_GetLcdWidth()-w_lcd)/2;
		offset_y = 0;
	}
	
	w_lcd_tmp = w_lcd >> 2;
	h_lcd_tmp = h_lcd >> 2;
	
	Fwl_CleanScreen(0);
	
	if ((width < w_lcd_tmp) || (height < h_lcd_tmp))
	{
		T_pDATA pDataBuf_S = AK_NULL;
		pDataBuf_S = Fwl_Malloc(w_lcd_tmp*h_lcd_tmp*2+64);
		
		if (AK_NULL == pDataBuf_S)
		{
			Fwl_Print(C3, M_IMAGE, "ERR:LHS:EMAP Small Show Malloc is Fail!");
			return;
		}
		
		memset(pDataBuf_S,0,w_lcd_tmp*h_lcd_tmp*2+64);
		Reset_2DGraphic();
		Fwl_Scale_Convert(pDataBuf_S,w_lcd_tmp, h_lcd_tmp,0,0,\
					  w_lcd_tmp ,pDataBuf,width, height, RGB565);

		Reset_2DGraphic();
		Fwl_Scale_Convert(Fwl_GetDispMemory(),w_lcd,h_lcd,offset_x,offset_y,\
					  w_lcd,pDataBuf_S,w_lcd_tmp,h_lcd_tmp, RGB565);
		
		if (AK_NULL != pDataBuf_S)
		{
			Fwl_Free (pDataBuf_S);
			pDataBuf_S = AK_NULL;
		}
	}
	else
	{
		Reset_2DGraphic();
		Fwl_Scale_Convert(Fwl_GetDispMemory(),w_lcd,h_lcd,offset_x,offset_y,\
					  w_lcd,pDataBuf,width, height, RGB565);
	}

#endif
	if (AK_NULL != pDataBuf)
	{
		Fwl_Free (pDataBuf);
		pDataBuf = AK_NULL;
	}
}

T_BOOL ImgBrowse_Show(T_IMGBROWSE * pImgBrowse)
{
    T_U32 flag;
	T_BOOL ret = AK_FALSE;

    flag = ImgBrowse_GetRefresh(pImgBrowse);

    if (IMG_BROWSE_REFRESH_NONE != flag)
    {
        if (flag & IMG_BROWSE_REFRESH_IMG)
        {
        	if ((ImgBrowse_GetDisMode(pImgBrowse) == IMG_MAP) && \
				((ImgBrowse_GetOutImgW(pImgBrowse) < (T_U32)Fwl_GetLcdWidth()) || \
			     (ImgBrowse_GetOutImgH(pImgBrowse) < (T_U32)Fwl_GetLcdHeight())))
            {
				ImgBrowse_ShowEmapSmall(pImgBrowse);		
		    }
			else
			{
				ret = ImgBrowse_ShowImg(pImgBrowse);
			}
        }
        if (flag & IMG_BROWSE_REFRESH_INFO)
        {
#ifndef DEMO_IMGBROWSE  //define in fwl_display.h
			if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_MAP)
			{
				ImgBrowse_ShowImgInfo(pImgBrowse);
			}
#endif
        }
        
        ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_NONE);
    }

	if(Fwl_TvoutIsOpen() && 
		((ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
		|| (ImgBrowse_GetDisMode(pImgBrowse) == IMG_PREVIEW)
		|| (ImgBrowse_GetDisMode(pImgBrowse) == IMG_SLIDE)))  
		return ret;
	
    if (AK_TRUE == pImgBrowse->bShowBttn)
    {
        ImgBrowse_ShowToolButtons(pImgBrowse);
    }

	return ret;
}


T_BOOL ImgBrowse_SetDownPoint(T_IMGBROWSE * pImgBrowse, T_POS x, T_POS y)
{
	if (AK_NULL == pImgBrowse)
	{
		return AK_FALSE;
	}
	else
	{
		pImgBrowse->down_x = x;
		pImgBrowse->down_y = y;

		return AK_TRUE;
	}
}

T_BOOL ImgBrowse_SetMovePoint(T_IMGBROWSE * pImgBrowse, T_POS x, T_POS y)
{
	if (AK_NULL == pImgBrowse)
	{
		return AK_FALSE;
	}
	
	pImgBrowse->move_x = x;
	pImgBrowse->move_y = y;

	return AK_TRUE;
}


T_BOOL ImgBrowse_SetVirPoint(T_IMGBROWSE * pImgBrowse, T_POS x, T_POS y)
{
	if (AK_NULL == pImgBrowse)
	{
		return AK_FALSE;
	}
	else
	{
		if (x <= pImgBrowse->down_x)
		{
			if (pImgBrowse->srcW <= MAIN_LCD_WIDTH)
			{
				if (((pImgBrowse->virleft - (pImgBrowse->down_x - x)) >= 0)
					&& (pImgBrowse->virleft + (T_S32)pImgBrowse->srcW - (pImgBrowse->down_x - x) > pImgBrowse->ctrl_photo.Rotate.rect.left + pImgBrowse->ctrl_photo.Rotate.rect.width + 10))
				{
					pImgBrowse->virleft -= pImgBrowse->down_x - x;
				}
			}
			else
			{
				if (pImgBrowse->virleft + pImgBrowse->srcW - (pImgBrowse->down_x - x) >= MAIN_LCD_WIDTH)
				{
					pImgBrowse->virleft -= pImgBrowse->down_x - x;
				}
				else
				{
					pImgBrowse->virleft = (T_POS)(MAIN_LCD_WIDTH - pImgBrowse->srcW);
				}
			}
		}
		else
		{
			if (pImgBrowse->srcW <= MAIN_LCD_WIDTH)
			{
				if (((pImgBrowse->virleft + pImgBrowse->srcW + (x - pImgBrowse->down_x)) <= MAIN_LCD_WIDTH)
					&& (pImgBrowse->virleft + (x - pImgBrowse->down_x) < pImgBrowse->ctrl_photo.Return.rect.left - 10))
				{
					pImgBrowse->virleft += x - pImgBrowse->down_x;
				}
			}
			else
			{
				if (pImgBrowse->virleft + (x - pImgBrowse->down_x) <= 0)
				{
					pImgBrowse->virleft += x - pImgBrowse->down_x;
				}
				else
				{
					pImgBrowse->virleft = 0;
				}
			}
		}
		
		if (y <= pImgBrowse->down_y)
		{
			if (pImgBrowse->srcH <= MAIN_LCD_HEIGHT)
			{
				if (((pImgBrowse->virtop - (pImgBrowse->down_y - y)) >0)
					&& (pImgBrowse->virtop + (T_S32)pImgBrowse->srcH - (pImgBrowse->down_y - y) > pImgBrowse->ctrl_photo.Rotate.rect.top + pImgBrowse->ctrl_photo.Rotate.rect.height + 10))
				{
					pImgBrowse->virtop -= pImgBrowse->down_y - y;
				}
			}
			else
			{
				if (pImgBrowse->virtop + pImgBrowse->srcH - (pImgBrowse->down_y - y) >= MAIN_LCD_HEIGHT)
				{
					pImgBrowse->virtop -= pImgBrowse->down_y - y;
				}
				else
				{
					pImgBrowse->virtop = (T_POS)(MAIN_LCD_HEIGHT - pImgBrowse->srcH);
				}
			}
		}
		else
		{
			if (pImgBrowse->srcH <= MAIN_LCD_HEIGHT)
			{
				if ((pImgBrowse->virtop + pImgBrowse->srcH + (y - pImgBrowse->down_y)) < MAIN_LCD_HEIGHT)
				{
					pImgBrowse->virtop += y - pImgBrowse->down_y;
				}
			}
			else
			{
				if (pImgBrowse->virtop + (y - pImgBrowse->down_y) <= 0)
				{
					pImgBrowse->virtop += y - pImgBrowse->down_y;
				}
				else
				{
					pImgBrowse->virtop= 0;
				}
			}
		}

		pImgBrowse->down_x = x;
		pImgBrowse->down_y = y;
		
		return AK_TRUE;
	}
}


static T_VOID ImgBrowse_ShowFastTimer(T_IMGBROWSE * pImgBrowse)
{
	if (AK_NULL == pImgBrowse)
	{
		return;
	}
	
	if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
	{
		if (ERROR_TIMER == pImgBrowse->imgTimerShowFast)
		{
			return;
		}
		
		ImgBrowse_StopImgShowFastTimer(pImgBrowse);
		ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_IMG);

		if (pImgBrowse->IMG_ZOOM_FLAG)
		{
			if (pImgBrowse->move_x > pImgBrowse->down_x)
			{
				pImgBrowse->T_ZoomChangeV = 300*(pImgBrowse->move_x-pImgBrowse->down_x)/(pImgBrowse->ctrl_photo.ZoomBlock.rect.width);
				pImgBrowse->zoom -= pImgBrowse->T_ZoomChangeV;
				if (pImgBrowse->zoom < 100)
				{
					pImgBrowse->zoom = 100;
				}
			}
			else
			{
				pImgBrowse->T_ZoomChangeV = 300*(pImgBrowse->down_x-pImgBrowse->move_x)/(pImgBrowse->ctrl_photo.ZoomBlock.rect.width);							
				pImgBrowse->zoom += pImgBrowse->T_ZoomChangeV;
				
				if (pImgBrowse->zoom > 400)
				{
					pImgBrowse->zoom = 400;
				}
			}

			pImgBrowse->T_ZoomChangeV = 0;
			pImgBrowse->down_x = pImgBrowse->move_x;
		}						
		
		if (pImgBrowse->IMG_MOVE_FLAG)
		{
			ImgBrowse_SetVirPoint(pImgBrowse, pImgBrowse->move_x, pImgBrowse->move_y);
		}
	
		if (pImgBrowse->DEFLAT_FLAG || pImgBrowse->IMG_GETDATA_FLAG || pImgBrowse->bGifStepFlag)
		{
			ImgBrowse_Show(pImgBrowse);
		}  
		
		memset(Fwl_GetDispMemory(),0,Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*2);

		ImgBrowse_ImgShow_Ex(pImgBrowse);
		ImgBrowse_ContolPhotoShow(pImgBrowse);
		
		GE_StartShade();
		
		Fwl_RefreshDisplay();

		pImgBrowse->IMG_ROTATE_90_FLAG = AK_FALSE;
		pImgBrowse->IMG_GETDATA_FLAG = AK_FALSE;
		pImgBrowse->DEFLAT_FLAG = AK_FALSE;
		pImgBrowse->bGifStepFlag = AK_FALSE;

		ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_NONE);
	}

}


T_IMG_RET ImgBrowse_Handle(T_IMGBROWSE * pImgBrowse, T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_MMI_KEYPAD    phyKey;
    T_IMG_RET       ret = IMG_STAY;

    switch (event)
    {
        case M_EVT_USER_KEY:
            phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
            phyKey.pressType = (T_BOOL)pEventParm->c.Param2;

			if (IMG_SLIDE == ImgBrowse_GetDisMode(pImgBrowse))
			{
				//up event don't exit
				if (PRESS_UP == phyKey.pressType)
				{
					ret = IMG_STAY;
				}
				else
				{
					ret = IMG_RETURN;
				}
				break;
			}

            ret = ImgBrowse_UserKey_Handle(pImgBrowse, phyKey);

            if (pImgBrowse->bShowBttn)
            {
                pImgBrowse->bShowBttn = AK_FALSE;
                ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);
            }
            
            ImgBrowse_StopImgCtlTimer(pImgBrowse);
			if ((ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE) && (IMG_STAY == ret))
			{
				if (pImgBrowse->largeFlag)
				{
					if (pImgBrowse->DecTimerId == ERROR_TIMER)
					{
						ImgBrowse_StartImgShowFastTimer(pImgBrowse);
					}
				}
				else
				{
					ImgBrowse_StartImgShowFastTimer(pImgBrowse);
				}
			}
            break;
			
        case VME_EVT_TIMER:
			if (pEventParm->w.Param1 == (T_U32)pImgBrowse->DecTimerId)
            {
                if (AK_FALSE == ImgBrowse_ImgStep(pImgBrowse))
                {
                    ret = IMG_OPEN_ERROR;
                }
				else
				{
					pImgBrowse->bGifStepFlag = AK_TRUE;
				}
            }
            else if(pEventParm->w.Param1 == (T_U32)pImgBrowse->imgCtlTimer)
            {
            	if (IMG_MAP != ImgBrowse_GetDisMode(pImgBrowse))
				{
					break;
				}
				
                pImgBrowse->bShowBttn = AK_FALSE;
                ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);   
                ImgBrowse_StopImgCtlTimer(pImgBrowse);
            }
            else if (pEventParm->w.Param1 == (T_U32)pImgBrowse->slideTimerId)
            {
            	if (IMG_BROWSE == ImgBrowse_GetDisMode(pImgBrowse))
				{
					break;
				}

				if (pImgBrowse->largeFlag && (ERROR_TIMER != pImgBrowse->DecTimerId))
				{				    
					break;
				}
				
                // Init  GE_Shade only when the toolbar is not showed 
                if (!pImgBrowse->bShowBttn)
                {
                    GE_ShadeInit();
                }
                
                if (AK_FALSE == ImgBrowse_SlideShow(pImgBrowse))
                {
                    ret = IMG_OPEN_ERROR;
                }
            }
			else if ((T_U32)pImgBrowse->imgTimerShowFast == pEventParm->w.Param1)
			{
				ImgBrowse_ShowFastTimer(pImgBrowse);
			}
            break;

        case M_EVT_TOUCH_SCREEN:
        {
            T_POS x = (T_POS)pEventParm->s.Param2;
            T_POS y = (T_POS)pEventParm->s.Param3;
                            
            if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
			{
				if (pImgBrowse->largeFlag)
				{
					if (pImgBrowse->DecTimerId == ERROR_TIMER)
					{
						ImgBrowse_StartImgShowFastTimer(pImgBrowse);
					}
				}
				else
				{
					ImgBrowse_StartImgShowFastTimer(pImgBrowse);
				}
			} 
			
            phyKey.keyID = kbNULL;
            phyKey.pressType = PRESS_SHORT;

            switch (pEventParm->s.Param1)
            {
            case eTOUCHSCR_UP:
				if (IMG_SLIDE == ImgBrowse_GetDisMode(pImgBrowse))
				{
					ret = IMG_RETURN;
					break;
				}
				
				if (IMG_MAP == ImgBrowse_GetDisMode(pImgBrowse))
				{
	                if (pImgBrowse->bShowBttn == AK_FALSE)
	                {
	                    pImgBrowse->bShowBttn = AK_TRUE;
	                    ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);
	                    //start the timer to show the image ctl in IMG_CTL_SHOW_INTERVAL second
	                    ImgBrowse_StartImgCtlTimer(pImgBrowse);
	                }
	                else
	                {
	                    if (PointInRect(&pImgBrowse->BkgdRect.ButtnRect, x, y))
	                    {
	                        /* if the point(x,y) hit in the control buttons rect,  
	                                                transform it to the corresponding key */
	                        phyKey = ImgBrowse_MapTSCR_To_Key(pImgBrowse, x, y);
	                        ImgBrowse_StopImgCtlTimer(pImgBrowse);
	                        ImgBrowse_StartImgCtlTimer(pImgBrowse);
	                        
	                        ret = ImgBrowse_UserKey_Handle(pImgBrowse, phyKey);
	                    }
	                    else
	                    {
	                        pImgBrowse->bShowBttn = AK_FALSE;
	                        ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);

	                        //no button in the image cotrol is pressed ,then hide the image control
	                        ImgBrowse_StopImgCtlTimer(pImgBrowse);
	                    }
	                }
				}
				else if (IMG_BROWSE == ImgBrowse_GetDisMode(pImgBrowse))
				{
					if (PointInRect(&(pImgBrowse->ctrl_photo.Return.rect), x, y)
						&& pImgBrowse->IMG_RETURN_FLAG)
					{
					    pImgBrowse->IMG_RETURN_FLAG = AK_FALSE;
					    return IMG_RETURN;
					}					
					
					pImgBrowse->IMG_MOVE_FLAG = AK_FALSE;
					pImgBrowse->IMG_ZOOM_FLAG = AK_FALSE;
					pImgBrowse->IMG_RETURN_FLAG = AK_FALSE;
				}
                break;
                
            case eTOUCHSCR_DOWN:
			{
				T_IMG_TOUCHSCR_DOWN TOUCHSCR_DOWN;

				if (IMG_SLIDE == ImgBrowse_GetDisMode(pImgBrowse))
				{
					break;
				}
				
				TOUCHSCR_DOWN = IMG_SATY;
				
				ImgBrowse_SetDownPoint(pImgBrowse, x, y);				
				ImgBrowse_SetMovePoint(pImgBrowse, x, y);
				
				if (PointInRect(&(pImgBrowse->ctrl_photo.Return.rect), x, y))
				{
			        TOUCHSCR_DOWN = IMG_EXIT;
				}
				else if (PointInRect(&(pImgBrowse->ctrl_photo.Rotate.rect), x, y))
				{
                    TOUCHSCR_DOWN  = IMG_ROTATE;
				}
				else if ((y > pImgBrowse->ctrl_photo.ZoomPoint.rect.top-14) /*14 不知道为什么屏的点触点会有这么大的偏离*/
						&&(y < pImgBrowse->ctrl_photo.ZoomPoint.rect.top+pImgBrowse->ctrl_photo.ZoomPoint.rect.height)
						&& (x > pImgBrowse->ctrl_photo.ZoomPoint.rect.left-8)
						&& (x< pImgBrowse->ctrl_photo.ZoomPoint.rect.left+pImgBrowse->ctrl_photo.ZoomPoint.rect.width+4))
				{
					TOUCHSCR_DOWN = IMG_ZOOM;
				}
				else if ((pImgBrowse->down_x > pImgBrowse->DispPartX)
						&& (pImgBrowse->down_x < pImgBrowse->DispPartX+(T_S32)pImgBrowse->srcW-pImgBrowse->ExOffsetX)
						&& (pImgBrowse->down_y > pImgBrowse->DispPartY)
						&& (pImgBrowse->down_y < pImgBrowse->DispPartY+(T_S32)pImgBrowse->srcH-pImgBrowse->ExOffsetY))
				{
					TOUCHSCR_DOWN = IMG_MOVE;
				}
				
				switch(TOUCHSCR_DOWN)
				{
				case IMG_ROTATE:
					ImgBrowse_RotateImg(pImgBrowse);
					pImgBrowse->IMG_ROTATE_90_FLAG = AK_TRUE;
					pImgBrowse->IMG_GETDATA_FLAG = AK_TRUE;
					break;
					
				case IMG_ZOOM:
					pImgBrowse->ctrl_photo.ZoomPoint.rect.left = x;
					pImgBrowse->IMG_ZOOM_FLAG = AK_TRUE;
					break;
					
				case IMG_MOVE:
					pImgBrowse->IMG_MOVE_FLAG = AK_TRUE;
					break;
					
				case IMG_EXIT:
					pImgBrowse->IMG_RETURN_FLAG = AK_TRUE;
					break;
					
				default:
					break;
				}
				break;
            }
            case eTOUCHSCR_MOVE:
				if (IMG_SLIDE == ImgBrowse_GetDisMode(pImgBrowse))
				{
					break;
				}
				
				ImgBrowse_SetMovePoint(pImgBrowse, x, y);

				if (pImgBrowse->IMG_ZOOM_FLAG)
				{
					pImgBrowse->IMG_GETDATA_FLAG = AK_TRUE;
				}            
				break;
				
            default:
                break;

            }
			break;
        }            

        default:
            break;
    }

    return ret;
}

static T_IMG_RET ImgBrowse_UserKey_Handle(T_IMGBROWSE * pImgBrowse, T_MMI_KEYPAD phyKey)
{
    T_U16       ImgBrowseAct;
    T_IMG_RET   ret = IMG_STAY;

    ImgBrowseAct = ImgBrowse_MappingKey(pImgBrowse, phyKey);
    
    switch (ImgBrowseAct)
    {
    case IMG_UP_PIC:
    case IMG_DOWN_PIC:
    case IMG_LEFT_PIC:
    case IMG_RIGHT_PIC:
        if (!ImgBrowse_OpenNextImg(pImgBrowse, ImgBrowseAct))
        {
            ret = IMG_OPEN_ERROR;
        }
        else
        {
            // Init  GE_Shade only when the toolbar is not showed 
            if ((ImgBrowse_GetDisMode(pImgBrowse) == IMG_MAP) && (!pImgBrowse->bShowBttn))
            {
                GE_ShadeInit();
            }
			else if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
			{
				GE_ShadeInit();
			}
            
            switch (ImgBrowseAct)
            {
            case IMG_UP_PIC:
                break;
				
            case IMG_DOWN_PIC:
                break;
				
            case IMG_LEFT_PIC:
                break;
				
            case IMG_RIGHT_PIC:
            default:
                break;
            }

			pImgBrowse->DEFLAT_FLAG = AK_TRUE;
        }
        break;
		
    case IMG_ZOOM_OUT:
    case IMG_ZOOM_IN:
        ImgBrowse_ChangeZoom(pImgBrowse, ImgBrowseAct, pImgBrowse->ZoomChangeV);
		pImgBrowse->IMG_GETDATA_FLAG = AK_TRUE;
        break;
		
    case IMG_ROLL_LEFT:
    case IMG_ROLL_RIGHT:
    case IMG_ROLL_UP:
    case IMG_ROLL_DOWN:
        ImgBrowse_Roll(pImgBrowse, ImgBrowseAct, pImgBrowse->OffsetChangeV);
        break;
		
    case IMG_ROTATE_90:
        ImgBrowse_RotateImg(pImgBrowse);
		pImgBrowse->IMG_ROTATE_90_FLAG = AK_TRUE;
		pImgBrowse->IMG_GETDATA_FLAG = AK_TRUE;
        break;
		
    case IMG_GO_MENU:
        // if the large photo is open and decoding
        // will not go to menu
        if (pImgBrowse->largeFlag && ImgDec_GetLargeDecStatus(&pImgBrowse->LargeImgAttrib) == DEC_CONTINUE)
        {
        }
        else
        {
            // before go to menu, stop the slide show timer
            if (gs.ImgSlideInterval)
            {
                ImgBrowse_StopSlideShow(pImgBrowse);
            }
            ret = IMG_MENU;
        }
        break;
		
    case IMG_GO_RETURN:
        ret = IMG_RETURN;
        break;
		
    case IMG_GO_RETURN_HOME:
        ret = IMG_RETURN_HOME;
        break;
		
    default:
        break;
    }

    return ret;

}

static T_MMI_KEYPAD ImgBrowse_MapTSCR_To_Key(T_IMGBROWSE * pImgBrowse, T_POS x, T_POS y)
{
    T_MMI_KEYPAD    phyKey;
        
    phyKey.keyID = kbNULL;
    phyKey.pressType = PRESS_SHORT;

	if (IMG_MAP != ImgBrowse_GetDisMode(pImgBrowse))
    {
		return phyKey;
	}

    //hit up button
    if (PointInRect(&pImgBrowse->UpBttn.ButtnRect, x, y))
    {
        phyKey.keyID = kbUP;
    }

	//hit next button
    if (PointInRect(&pImgBrowse->NextBttn.ButtnRect, x, y))
    {
        phyKey.keyID = kbRIGHT;
    }
    
    //hit previous button
    if (PointInRect(&pImgBrowse->PrvBttn.ButtnRect, x, y))
    {
        phyKey.keyID = kbLEFT;
    }
    
    //hit zoomin button
    if (PointInRect(&pImgBrowse->ZoomInBttn.ButtnRect, x, y))
    {
        phyKey.keyID = kbVOICE_UP;
    }

    //hit zoomout button
    if (PointInRect(&pImgBrowse->ZoomOutBttn.ButtnRect, x, y))
    {
        phyKey.keyID = kbVOICE_DOWN;
    }

	 //hit down button
    if (PointInRect(&pImgBrowse->DownBttn.ButtnRect, x, y))
    {
        phyKey.keyID = kbDOWN;
    }
    
    //hit return button
    if (PointInRect(&pImgBrowse->ReturnBttn.ButtnRect, x, y))
    {
        phyKey.keyID = kbCLEAR;
    }

    return phyKey;
}

static T_VOID ImgBrowse_ZoomOut(T_IMGBROWSE * pImgBrowse, T_U32 changeValue)
{
    T_U32 zoom_max;
    T_U32 srcW,srcH;
    T_U32 srcWZoomMax, srcHZoomMax;

    if (AK_NULL == pImgBrowse)
        return;

    if (pImgBrowse->rotate == 0 || pImgBrowse->rotate == 180)
    {
        srcW = ImgBrowse_GetOutImgW(pImgBrowse);
        srcH = ImgBrowse_GetOutImgH(pImgBrowse);
    }
    else
    {
        srcW = ImgBrowse_GetOutImgH(pImgBrowse);
        srcH = ImgBrowse_GetOutImgW(pImgBrowse);
    }
	
	if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_MAP)
	{
		srcWZoomMax = 100*srcW/Fwl_GetLcdWidth();
    	srcHZoomMax = 100*srcH/Fwl_GetLcdHeight();
	}
	else
	{
		srcWZoomMax = 400;//100*srcW/Fwl_GetLcdWidth();
		srcHZoomMax = 400;//100*srcH/Fwl_GetLcdHeight();
	}
	
    zoom_max = (srcWZoomMax >= srcHZoomMax) ? srcWZoomMax : srcHZoomMax;
    if (zoom_max < 100)
        zoom_max = 100;

    pImgBrowse->zoom = ((pImgBrowse->zoom + changeValue) <= zoom_max) ? (pImgBrowse->zoom + changeValue) : zoom_max;
}

static T_VOID ImgBrowse_ZoomIn(T_IMGBROWSE * pImgBrowse, T_U32 changeValue)
{
    if (AK_NULL == pImgBrowse)
        return;

    pImgBrowse->zoom = (pImgBrowse->zoom >= (T_U32)(100+changeValue)) ? (pImgBrowse->zoom - changeValue) : 100;
}

// regulate offset after zoom, focus at the middle
static T_BOOL ImgBrowse_RegulateOffset(T_IMGBROWSE * pImgBrowse, T_U32 OldZoom, T_U32 NewZoom)
{
    T_U32 imgW, imgH;
    T_U32 oldShowW, oldShowH;
    T_U32 newShowW, newShowH;
    T_U32 ChangeOffsetX, ChangeOffsetY;
    T_U32 tmp;

    if (AK_NULL == pImgBrowse)
        return AK_FALSE;

    if (OldZoom != NewZoom)
    {
        if (pImgBrowse->rotate == 0 || pImgBrowse->rotate == 180)
        {
            imgW = ImgBrowse_GetOutImgW(pImgBrowse);
            imgH = ImgBrowse_GetOutImgH(pImgBrowse);
        }
        else
        {
            imgW = ImgBrowse_GetOutImgH(pImgBrowse);
            imgH = ImgBrowse_GetOutImgW(pImgBrowse);
        }

        oldShowW = imgW * 100 / OldZoom;
        oldShowH = imgH * 100 / OldZoom;
        newShowW = imgW * 100 / NewZoom;
        newShowH = imgH * 100 / NewZoom;
		
        if (OldZoom < NewZoom)
        {
            if (oldShowW > (T_U32)Fwl_GetLcdWidth())
            {
                ChangeOffsetX = (newShowW >= (T_U32)Fwl_GetLcdWidth()) ? (oldShowW - newShowW) >> 1 : (oldShowW - Fwl_GetLcdWidth()) >> 1;
            }
            else
            {
                ChangeOffsetX = 0;
            }
			
            if (oldShowH > (T_U32)Fwl_GetLcdHeight())
            {
                ChangeOffsetY = (newShowH >= (T_U32)Fwl_GetLcdHeight()) ? (oldShowH - newShowH) >> 1 : (oldShowH - Fwl_GetLcdHeight()) >> 1;
            }
            else
            {
                ChangeOffsetY = 0;
            }
            ImgBrowse_AddOffsetX(pImgBrowse, ChangeOffsetX);
            ImgBrowse_AddOffsetY(pImgBrowse, ChangeOffsetY);
        }
        else
        {
            if (newShowW > (T_U32)Fwl_GetLcdWidth())
            {
                ChangeOffsetX = (oldShowW >= (T_U32)Fwl_GetLcdWidth()) ? (newShowW - oldShowW) >> 1 : (newShowW - Fwl_GetLcdWidth()) >> 1;
                if (pImgBrowse->offsetX >= ChangeOffsetX)
                {
                    // the most right of the photo
                    tmp = pImgBrowse->offsetX - ChangeOffsetX;
                    ChangeOffsetX = (newShowW + tmp >= imgW) ? (ChangeOffsetX + (newShowW + tmp - imgW)) : ChangeOffsetX;
                }
                else
                {
                    // the most left of the photo
                    ChangeOffsetX = pImgBrowse->offsetX;
                }
            }
            else
            {
                ChangeOffsetX = 0;
            }
			
            if (newShowH > (T_U32)Fwl_GetLcdHeight())
            {
                ChangeOffsetY = (oldShowH >= (T_U32)Fwl_GetLcdHeight()) ? (newShowH - oldShowH) >> 1 : (newShowH - Fwl_GetLcdHeight()) >> 1;
                if (pImgBrowse->offsetY >= ChangeOffsetY)
                {
                    tmp = pImgBrowse->offsetY - ChangeOffsetY;
                    ChangeOffsetY = (newShowH + tmp >= imgH) ? (ChangeOffsetY + (newShowH + tmp - imgH)) : ChangeOffsetY;
                }
                else
                {
                    ChangeOffsetY = pImgBrowse->offsetY;
                }
            }
            else
            {
                ChangeOffsetY = 0;
            }
			
            ImgBrowse_SubOffsetX(pImgBrowse, ChangeOffsetX);
            ImgBrowse_SubOffsetY(pImgBrowse, ChangeOffsetY);
        }
    }

    return AK_TRUE;
}

T_BOOL ImgBrowse_ChangeZoom(T_IMGBROWSE * pImgBrowse, T_U16 zoomAct, T_U32 changValue)
{
    T_BOOL zoomRet = AK_FALSE;
    T_U32 oldZoom;

    if (AK_NULL == pImgBrowse || (IMG_ZOOM_IN != zoomAct && IMG_ZOOM_OUT != zoomAct))
        return zoomRet;

    oldZoom = pImgBrowse->zoom;
    // change zoom
    if (IMG_ZOOM_IN == zoomAct)
    {
        ImgBrowse_ZoomIn(pImgBrowse, changValue);
    }
    else
    {
        ImgBrowse_ZoomOut(pImgBrowse, changValue);
    }

    // regulate the offset x and y, when change zoom
    if (oldZoom != pImgBrowse->zoom)
    {
        ImgBrowse_RegulateOffset(pImgBrowse, oldZoom, pImgBrowse->zoom);
    }

/*
    GetBMP2RGBBuf(ImgBrowse_GetOutBuf(pImgBrowse), \
                    Fwl_GetDispMemory(), Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), \
                    pImgBrowse->offsetX, pImgBrowse->offsetY, \
                    pImgBrowse->zoom, pImgBrowse->rotate, &pImgBrowse->scale);
*/
    ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);

    return AK_TRUE;
}

static T_VOID ImgBrowse_AddOffsetY(T_IMGBROWSE * pImgBrowse, T_U32 AddValue)
{
    T_U32 maxOffsetY;

    maxOffsetY = (ImgBrowse_GetOutImgH(pImgBrowse))*(pImgBrowse->zoom-100)/pImgBrowse->zoom;
    pImgBrowse->offsetY = ((pImgBrowse->offsetY + AddValue) > maxOffsetY) ?  maxOffsetY : (pImgBrowse->offsetY + AddValue);
}

static T_VOID ImgBrowse_SubOffsetY(T_IMGBROWSE * pImgBrowse, T_U32 SubValue)
{
    pImgBrowse->offsetY = (pImgBrowse->offsetY >= SubValue) ? pImgBrowse->offsetY -= SubValue : 0;
}

static T_VOID ImgBrowse_AddOffsetX(T_IMGBROWSE * pImgBrowse, T_U32 AddValue)
{
    T_U32 maxOffsetX;

    maxOffsetX = (ImgBrowse_GetOutImgW(pImgBrowse))*(pImgBrowse->zoom-100)/pImgBrowse->zoom;
    pImgBrowse->offsetX = ((pImgBrowse->offsetX + AddValue) > maxOffsetX) ? maxOffsetX : (pImgBrowse->offsetX + AddValue);
}

static T_VOID ImgBrowse_SubOffsetX(T_IMGBROWSE * pImgBrowse, T_U32 SubValue)
{
    pImgBrowse->offsetX = (pImgBrowse->offsetX >= SubValue) ? (pImgBrowse->offsetX - SubValue) : 0;
}

static T_VOID ImgBrowse_RollLeft(T_IMGBROWSE * pImgBrowse, T_U32 RollValue)
{
    if (pImgBrowse->rotate != 90 && pImgBrowse->rotate != 0)
    {
        ImgBrowse_AddOffsetX(pImgBrowse, RollValue);
    }
    else
    {
        ImgBrowse_SubOffsetX(pImgBrowse, RollValue);
    }
}

static T_VOID ImgBrowse_RollRight(T_IMGBROWSE * pImgBrowse, T_U32 RollValue)
{
    if (pImgBrowse->rotate != 90 && pImgBrowse->rotate != 0)
    {
        ImgBrowse_SubOffsetX(pImgBrowse, RollValue);
    }
    else
    {
        ImgBrowse_AddOffsetX(pImgBrowse, RollValue);
    }
}

static T_VOID ImgBrowse_RollUp(T_IMGBROWSE * pImgBrowse, T_U32 RollValue)
{
    if (pImgBrowse->rotate != 270 && pImgBrowse->rotate != 0)
    {
        ImgBrowse_SubOffsetY(pImgBrowse, RollValue);
    }
    else
    {
        ImgBrowse_AddOffsetY(pImgBrowse, RollValue);
    }
}

static T_VOID ImgBrowse_RollDown(T_IMGBROWSE * pImgBrowse, T_U32 RollValue)
{
    if (pImgBrowse->rotate != 270 && pImgBrowse->rotate != 0)
    {
        ImgBrowse_AddOffsetY(pImgBrowse, RollValue);
    }
    else
    {
        ImgBrowse_SubOffsetY(pImgBrowse, RollValue);
    }
}



static T_BOOL ImgBrowse_Roll(T_IMGBROWSE * pImgBrowse, T_U16 rollAct, T_U32 changeValue)
{
    if (AK_NULL == pImgBrowse || 100 == pImgBrowse->zoom)
    {
        return AK_FALSE;
    }

    switch (rollAct)
    {
    case IMG_ROLL_LEFT:
        ImgBrowse_RollLeft(pImgBrowse, changeValue);
        break;
		
    case IMG_ROLL_RIGHT:
        ImgBrowse_RollRight(pImgBrowse, changeValue);
        break;
		
    case IMG_ROLL_UP:
        ImgBrowse_RollUp(pImgBrowse, changeValue);
        break;
		
    case IMG_ROLL_DOWN:
        ImgBrowse_RollDown(pImgBrowse, changeValue);
        break;
		
    default:
        break;
    }
/*
    GetBMP2RGBBuf(ImgBrowse_GetOutBuf(pImgBrowse), \
            Fwl_GetDispMemory(), Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), \
            pImgBrowse->offsetX, pImgBrowse->offsetY, \
            pImgBrowse->zoom, pImgBrowse->rotate, &pImgBrowse->scale);
*/
    ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);

    return AK_TRUE;
}

static T_U32 ImgBrowse_GetImgMaxZoom(T_IMGBROWSE * pImgBrowse)
{
    T_U32 imgW, imgH;
    T_U32 srcWZoomMax, srcHZoomMax;
    T_U32 zoom_max = 100;

    if (AK_NULL == pImgBrowse)
        return zoom_max;

    if (pImgBrowse->rotate == 0 || pImgBrowse->rotate == 180)
    {
        imgW = ImgBrowse_GetOutImgW(pImgBrowse);
        imgH = ImgBrowse_GetOutImgH(pImgBrowse);
    }
    else
    {
        imgW = ImgBrowse_GetOutImgH(pImgBrowse);
        imgH = ImgBrowse_GetOutImgW(pImgBrowse);
    }
	
    srcWZoomMax = 100*imgW/Fwl_GetLcdWidth();
    srcHZoomMax = 100*imgH/Fwl_GetLcdHeight();
    zoom_max = (srcWZoomMax >= srcHZoomMax) ? srcWZoomMax : srcHZoomMax;
	
    if (zoom_max < 100)
        zoom_max = 100;

    return zoom_max;
}

static T_VOID ImgBrowse_RotateImg(T_IMGBROWSE * pImgBrowse)
{
    T_U32 zoom_max;
    T_U32 oldZoom;
    T_U32 tmpOffsetX;

    pImgBrowse->rotate = (pImgBrowse->rotate + 90) % 360;

    tmpOffsetX = pImgBrowse->offsetX;
    pImgBrowse->offsetX = pImgBrowse->offsetY;
    pImgBrowse->offsetY = tmpOffsetX;

    zoom_max = ImgBrowse_GetImgMaxZoom(pImgBrowse);
    if (pImgBrowse->zoom > zoom_max)
    {
        oldZoom = pImgBrowse->zoom;
        pImgBrowse->zoom = zoom_max;
        pImgBrowse->offsetX = pImgBrowse->offsetX * zoom_max / oldZoom;
        pImgBrowse->offsetY = pImgBrowse->offsetY * zoom_max / oldZoom;
    }

    /*
    GetBMP2RGBBuf(ImgBrowse_GetOutBuf(pImgBrowse), \
                Fwl_GetDispMemory(), Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), \
                pImgBrowse->offsetX, pImgBrowse->offsetY, \
                pImgBrowse->zoom, pImgBrowse->rotate, &pImgBrowse->scale);
                */
    ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);
}

static T_BOOL ImgBrowse_OpenNextImg(T_IMGBROWSE * pImgBrowse, T_IMG_ACTION imgAction)
{
    T_BOOL ret = AK_FALSE;
    T_DISPLAYLIST_DIRECTION disListRollDir;
    T_FILE_INFO  *pFileInfo = AK_NULL;
    T_DISPLAYLIST *pDisplayList = AK_NULL;

    AK_ASSERT_PTR(pImgBrowse, "ImgBrowse_OpenNextImg(): pImgBrowse is invalid ptr\n", AK_FALSE);

    pDisplayList = pImgBrowse->pDisplayList;
    //ImgBrowse_ImgClose(pImgBrowse);
	
    if (IMG_UP_PIC == imgAction || IMG_DOWN_PIC == imgAction || IMG_LEFT_PIC == imgAction || IMG_RIGHT_PIC == imgAction)
    {
        disListRollDir = ((IMG_UP_PIC == imgAction) || (IMG_LEFT_PIC == imgAction)) ? DISPLAYLIST_DIRECTION_UP : DISPLAYLIST_DIRECTION_DOWN;

        if ((ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
			|| (ImgBrowse_GetDisMode(pImgBrowse) == IMG_PREVIEW)
			|| (ImgBrowse_GetDisMode(pImgBrowse) == IMG_SLIDE))
        {
            do
            {
                DisplayList_MoveFocus(pDisplayList, disListRollDir);
                pFileInfo = DisplayList_GetItemContentFocus(pDisplayList);
            } while (pFileInfo && ((pFileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER || (!ImgBrowse_IsBrowseSupportFileType(pFileInfo->name))));
        }

        if (pFileInfo)
        {
            if (FILE_TYPE_GIF == Utl_GetFileType(pFileInfo->name))
            {
#ifndef GIF_IMGBROWSE_NO_WAITING   //define in fwl_display.h
                ImgBrowse_ShowWait(pImgBrowse);
#endif
            }
			
            ImgBrowse_ImgClose(pImgBrowse);
			pImgBrowse->pDatabuf = Fwl_Free(pImgBrowse->pDatabuf);
			pImgBrowse->virleft = 0;
			pImgBrowse->virtop = 0;
			
            ret = ImgBrowse_ImgOpen(pImgBrowse);;
        }
    }

    if (AK_TRUE == ret)
    {
        ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);		
    }

    return ret;
}

T_VOID ImgBrowse_SetRefresh(T_IMGBROWSE * pImgBrowse, T_U16 refreshFlag)
{
    if (IMG_BROWSE_REFRESH_NONE != refreshFlag)
    {
        pImgBrowse->refreshFlag |= refreshFlag;
    }
    else
    {
        pImgBrowse->refreshFlag = refreshFlag;
    }
}

T_U16 ImgBrowse_GetRefresh(T_IMGBROWSE * pImgBrowse)
{
    return pImgBrowse->refreshFlag;
}

static T_VOID ImgBrowse_StopSlideTimer(T_IMGBROWSE * pImgBrowse)
{
    if (pImgBrowse->slideTimerId != ERROR_TIMER)
    {
        Fwl_StopTimer(pImgBrowse->slideTimerId);
        pImgBrowse->slideTimerId = ERROR_TIMER;
    }
}


T_VOID ImgBrowse_StartSlideTimer(T_IMGBROWSE * pImgBrowse)
{
    if (pImgBrowse->slideTimerId != ERROR_TIMER)
    {
        Fwl_StopTimer(pImgBrowse->slideTimerId);
        pImgBrowse->slideTimerId = ERROR_TIMER;
    }
	
    pImgBrowse->slideTimerId = Fwl_SetTimerSecond(gs.ImgSlideInterval, AK_FALSE);
}

T_VOID ImgBrowse_StartSlideShow(T_IMGBROWSE * pImgBrowse)
{
    ImgBrowse_StartSlideTimer(pImgBrowse);
    ScreenSaverDisable();
	AutoPowerOffDisable(FLAG_IMG);
}

static T_BOOL ImgBrowse_SlideShow(T_IMGBROWSE * pImgBrowse)
{
    T_BOOL ret = AK_FALSE;
    T_FILE_INFO  *pFileInfo;

    if (pImgBrowse->bShowBttn)
    {
        pImgBrowse->bShowBttn = AK_FALSE;
        ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);   
        ImgBrowse_StopImgCtlTimer(pImgBrowse);
    }
    
    ImgBrowse_StopSlideTimer(pImgBrowse);
	
    do
    {
        DisplayList_MoveFocus(pImgBrowse->pDisplayList, DISPLAYLIST_DIRECTION_DOWN);
        pFileInfo = DisplayList_GetItemContentFocus(pImgBrowse->pDisplayList);
    }while (pFileInfo != AK_NULL 
    	&& ((pFileInfo->attrib & EXPLORER_ISFOLDER) || (!ImgBrowse_IsBrowseSupportFileType(pFileInfo->name))));

    if (pFileInfo != AK_NULL)
    {
        if (FILE_TYPE_GIF == Utl_GetFileType(pFileInfo->name))
            ImgBrowse_ShowWait(pImgBrowse);

        ImgBrowse_ImgClose(pImgBrowse);

        //ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);
        ret = ImgBrowse_ImgOpen(pImgBrowse);

        //if (AK_TRUE == ret)
        {
            ImgBrowse_StartSlideTimer(pImgBrowse);
        }
    }

    if (AK_TRUE == ret)
        ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);

    return ret;
}

T_VOID ImgBrowse_StopSlideShow(T_IMGBROWSE * pImgBrowse)
{
    ImgBrowse_StopSlideTimer(pImgBrowse);
    ScreenSaverEnable();

	AutoPowerOffEnable(FLAG_IMG);
}

static T_U16 ImgBrowse_MappingKey(T_IMGBROWSE * pImgBrowse, T_MMI_KEYPAD phyKey)
{
    T_U16 ret = 0;
    T_U32 zoom;

    zoom = pImgBrowse->zoom;

    if (phyKey.pressType == PRESS_SHORT)
    {
        switch (phyKey.keyID)
        {
        case kbLEFT:
			if ((ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
				||(ImgBrowse_GetDisMode(pImgBrowse) == IMG_PREVIEW))
			{
				ret = (phyKey.keyID == kbUP) ? IMG_UP_PIC : IMG_LEFT_PIC;
			}
			else if ((ImgBrowse_GetDisMode(pImgBrowse) == IMG_MAP) && (100 != zoom))
			{
                ret = (phyKey.keyID == kbUP) ? IMG_ROLL_UP : IMG_ROLL_LEFT;
            }
            break;
        case kbRIGHT:
			if ((ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
				||(ImgBrowse_GetDisMode(pImgBrowse) == IMG_PREVIEW))
			{
				ret = (phyKey.keyID == kbDOWN) ? IMG_DOWN_PIC : IMG_RIGHT_PIC;
			}
			else if ((ImgBrowse_GetDisMode(pImgBrowse) == IMG_MAP) && (100 != zoom))
			{
                ret = (phyKey.keyID == kbDOWN) ? IMG_ROLL_DOWN : IMG_ROLL_RIGHT;
            }
            break;
			
        case kbOK:
            if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
            {
                ret = IMG_ROTATE_90;
            }
            break;
			
        case kbCLEAR:
            ret = IMG_GO_RETURN;
            break;
			
        case kbMENU:
            ret = IMG_GO_MENU;
            break;
        case kbUP:
			if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_MAP)
			{
				 ret = IMG_ZOOM_OUT;
			}
			else
			{
				 ret = IMG_ZOOM_IN;
			}
            break;
        case kbDOWN:
			if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_MAP)
			{
				 ret = IMG_ZOOM_IN;
			}
			else
			{
				 ret = IMG_ZOOM_OUT;
			}
            break;
			
        default:
            break;
        }
    }
    else if (phyKey.pressType == PRESS_LONG)    /* hold */
    {
        switch (phyKey.keyID) {
        case kbUP:
        case kbLEFT:
            if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
            {
                Fwl_KeyStop();
                ret = IMG_UP_PIC;
            }
            else
            {
                ret = (phyKey.keyID == kbUP) ? IMG_ROLL_UP : IMG_ROLL_LEFT;
            }
            break;
			
        case kbDOWN:
        case kbRIGHT:
            if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
            {
                Fwl_KeyStop();
                ret = IMG_DOWN_PIC;
            }
            else
            {
                ret = (phyKey.keyID == kbDOWN) ? IMG_ROLL_DOWN : IMG_ROLL_RIGHT;
            }
            /*
            if (zoom == 100)
            {
                if(ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
                {
                    ret = IMG_DOWN_PIC;
                }
            }
            else
            {
                ret = (phyKey.keyID == kbDOWN) ? IMG_ROLL_DOWN : IMG_ROLL_RIGHT;
            }
            */
            break;
			
        case kbCLEAR:
            Fwl_KeyStop();
            ret = IMG_GO_RETURN_HOME;
            break;
			
        case kbOK:
            Fwl_KeyStop();
            if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
            {
                ret = IMG_ROTATE_90;
            }
            break;
			
        default:
            break;
        }
    }

    return ret;
} /* end ImgBrowse_MappingKey(T_MMI_KEYPAD phyKey) */

static T_BOOL ImgBrowse_ShowImgInfo(T_IMGBROWSE * pImgBrowse)
{
    T_STR_INFO tmpstr;
    T_USTR_FILE file_name, file_ext;
    T_FILE_INFO *pFileInfo;
    T_USTR_INFO pUniStr1;
    T_USTR_INFO pUniStr2;

    T_STR_INFO  timestr;
    T_USTR_FILE Utmpstr, Utmpstr2;

	T_RECT      stRectTVOUT;
	T_U8        *pBufDisp;
	T_U8        ColorSpace;
	T_RECT      stRectCap;
	T_U8        bFlagTVOUT;
	

    T_S32       nStrWidth = 0;
    T_U16       fontHeight=GetFontHeight(CURRENT_FONT_SIZE);

    Eng_StrMbcs2Ucs("~1.", pUniStr1);
    Eng_StrMbcs2Ucs("~1", pUniStr2);

    if (AK_NULL == pImgBrowse)
        return AK_FALSE;

    pFileInfo = DisplayList_GetItemContentFocus(pImgBrowse->pDisplayList);
    if (pFileInfo == AK_NULL || (pFileInfo->attrib & EXPLORER_ISFOLDER) == EXPLORER_ISFOLDER)
        return AK_FALSE;

	if (Fwl_TvoutIsOpen() == AK_TRUE)
	{
		bFlagTVOUT = AK_TRUE;
		pBufDisp = Fwl_GetFrameBufInfo(&stRectTVOUT,&ColorSpace);
		Fwl_GetDisplayCaps(Fwl_GetDispalyType(), &stRectCap);
	}
	else
	{
		bFlagTVOUT = AK_FALSE;
	}

	
    Utl_USplitFileName(pFileInfo->name, file_name, file_ext);

    if (UGetSpeciStringWidth(pFileInfo->name, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pFileInfo->name)) > (T_U32)(Fwl_GetLcdWidth() -10))
    {
        T_U16 n;
        T_U32 ExtWidth, TmpWidth;

        if (ImgBrowse_GetDisPostfixMode(pImgBrowse) == IMG_DISPOSTFIX)
        {
            ExtWidth = UGetSpeciStringWidth(file_ext, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(file_ext));
            TmpWidth = UGetSpeciStringWidth(pUniStr1, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pUniStr1));
        }
        else
        {
            ExtWidth = 0;
            TmpWidth = UGetSpeciStringWidth(pUniStr2, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pUniStr2));
        }

        for (n = 10; n < Utl_UStrLen(file_name);)
        {
            if (UGetSpeciStringWidth(file_name, CURRENT_FONT_SIZE, (T_U16)(n + 1)) > (Fwl_GetLcdWidth() - 10 - ExtWidth - TmpWidth))
                break;
            n++;
        }
        file_name[n] = 0;

        if (ImgBrowse_GetDisPostfixMode(pImgBrowse) == IMG_DISPOSTFIX)
        {
            Utl_UStrCpy(Utmpstr, file_name);
            Utl_UStrCat(Utmpstr, pUniStr1);
            Utl_UStrCat(Utmpstr, file_ext);
        }
        else
        {
            Utl_UStrCpy(Utmpstr, file_name);
            Utl_UStrCat(Utmpstr, pUniStr2);
        }
    }
    else
    {
        if (ImgBrowse_GetDisPostfixMode(pImgBrowse) == IMG_DISPOSTFIX)
        {
            //sprintf(tmpstr, "%s.%s", file_name, file_ext);
            Utl_UStrCpy(Utmpstr, file_name);
            Utl_UStrCat(Utmpstr, _T("."));
            Utl_UStrCat(Utmpstr, file_ext);
        }
        else
        {
            //sprintf(tmpstr, "%s", file_name);
            Utl_UStrCpy(Utmpstr, file_name);
        }
    }
	
    if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
    {
		if (bFlagTVOUT)
		{ 
			Fwl_UDispSpeciStringOnRGB(pBufDisp,(stRectTVOUT.width), (stRectTVOUT.height << 1),
				(T_POS)(stRectCap.left), (T_POS)(stRectCap.top+6), Utmpstr, COLOR_RED, RGB888, 
				CURRENT_FONT_SIZE,
				(T_U16)Utl_UStrLen(Utmpstr));
		}
		else
		{			
	       Fwl_UDispSpeciString(HRGB_LAYER, 0, 0, Utmpstr, COLOR_RED, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
		}
    }
    else
    {
        Fwl_UDispSpeciString(HRGB_LAYER, 6+EAMP_SCALE_INFO_X, 6, Utmpstr, COLOR_RED, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
    }
	
    if (ImgBrowse_GetDisPixelMode(pImgBrowse) == IMG_DISPIXEL)
    {
        T_U32   scale,scale_tmp1,scale_tmp2;
        T_U32   wid_act,hei_act;

        wid_act = Fwl_GetLcdWidth() *pImgBrowse->zoom/100;
        hei_act = Fwl_GetLcdHeight() *pImgBrowse->zoom/100;
        scale_tmp1 = (Fwl_GetLcdWidth()*100/(wid_act))*(Fwl_GetLcdHeight()*100/(hei_act));

        scale =scale_tmp1 /100;
        scale_tmp2 = scale_tmp1%100;
		
        if (scale_tmp2>50)
            scale += 1;
		
        if (scale>100)
            scale = 100;
		
        sprintf(tmpstr, "%ld X %ld  %ld%%", \
                ImgBrowse_GetInImgW(pImgBrowse), ImgBrowse_GetInImgH(pImgBrowse),scale);
        Eng_StrMbcs2Ucs(tmpstr, Utmpstr);

                //show accessing time 
                
        ImgBrowse_Date2Str(pFileInfo->time_write, timestr);  //pImgBrowse->fsFileInfo.time_write
//        Utl_UStrCpy(Utmpstr2, Res_GetStringByID(eRES_STR_DATE));
        Eng_StrMbcs2Ucs(timestr, Utmpstr2);
//        Utl_UStrCat(Utmpstr2, UtmpStr3);
    }
    else
    {
        T_U32   scale,scale_tmp1,scale_tmp2;
        T_U32   wid_act,hei_act;

        wid_act = Fwl_GetLcdWidth() *pImgBrowse->zoom/100;
        hei_act = Fwl_GetLcdHeight() *pImgBrowse->zoom/100;
        scale_tmp1 = (Fwl_GetLcdWidth()*100/(wid_act))*(Fwl_GetLcdHeight()*100/(hei_act));

        scale =scale_tmp1 /100;
        scale_tmp2 = scale_tmp1%100;
		
        if (scale_tmp2>50)
            scale += 1;
		
        if (scale>100)
            scale = 100;

        if ((Fwl_GetLcdWidth() > ImgBrowse_GetOutImgW(pImgBrowse)) || \
			(Fwl_GetLcdHeight() > ImgBrowse_GetOutImgH(pImgBrowse)))
       	{
			scale = 100;
		}
		
        sprintf(tmpstr, "SCALE: %ld%%", scale);
        Eng_StrMbcs2Ucs(tmpstr, Utmpstr);
    }

	
	
    if (ImgBrowse_GetDisMode(pImgBrowse) == IMG_BROWSE)
    {
		if (bFlagTVOUT)
		{
			Fwl_UDispSpeciStringOnRGB(pBufDisp,(T_U32)stRectTVOUT.width, (T_U32)(stRectTVOUT.height << 1),
				stRectCap.left, (T_POS)((stRectTVOUT.height << 1) - stRectCap.top - fontHeight-6), Utmpstr,
				COLOR_RED, RGB888, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
			nStrWidth = UGetSpeciStringWidth(Utmpstr2, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr2));
			Fwl_UDispSpeciStringOnRGB(pBufDisp,(stRectTVOUT.width), (stRectTVOUT.height << 1),
				(T_POS)(stRectCap.left + stRectCap.width - nStrWidth -6), 
				(T_POS)((stRectTVOUT.height << 1) - stRectCap.top - fontHeight-6), Utmpstr2, 
				COLOR_RED, RGB888, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr2));
		}
		else
		{
	        Fwl_UDispSpeciString(HRGB_LAYER, 0, (T_POS)(Fwl_GetLcdHeight() - fontHeight), Utmpstr, COLOR_RED, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
	        nStrWidth = UGetSpeciStringWidth(Utmpstr2, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr2));
	        Fwl_UDispSpeciString(HRGB_LAYER, (T_POS)(Fwl_GetLcdWidth() -  nStrWidth - 2), (T_POS)(Fwl_GetLcdHeight() - fontHeight), Utmpstr2, COLOR_RED, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr2));
		}
	}
    else
    {
        Fwl_UDispSpeciString(HRGB_LAYER, 6+EAMP_SCALE_INFO_X, (T_POS)(Fwl_GetLcdHeight()-g_Font.CHEIGHT-EAMP_SCALE_INFO_Y), Utmpstr, COLOR_RED, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
    }

    return AK_TRUE;
}

static T_VOID ImgBrowse_Date2Str(T_U32 datetime, T_S8 *string)
{
    T_U16 date,time,tmp;

    date = (T_U16)(datetime >> 16);
    time = (T_U16)(datetime & 0xFFFF);
    //year
    tmp = (date>>9)&0x7f; //get year
    tmp += 1980;
    tmp = tmp%100;
    string[0] = (tmp/10)+'0';
    string[1] = (tmp%10)+'0';
    string[2] = '/';
    //month
    tmp = (date>>5)&0x0f;
    tmp = tmp%13;
    if(tmp==0)
    tmp = 1;
    string[3] = (tmp/10)+'0';
    string[4] = (tmp%10)+'0';
    string[5] = '/';
    //days
    tmp = (date)&0x1f;
    tmp = tmp%32;
    if(tmp==0)
    tmp = 1;
    string[6] = (tmp/10)+'0';
    string[7] = (tmp%10)+'0';
    string[8] = ' ';
    //hour
    tmp = (time>>11)&0x1f;
    tmp = tmp%24;
    string[9] = (tmp/10)+'0';
    string[10] = (tmp%10)+'0';
    string[11] = ':';

    //miniute
    tmp = (time>>5)&0x3f;
    tmp = tmp%60;
    string[12] = (tmp/10)+'0';
    string[13] = (tmp%10)+'0';
    string[14] = ':';

    //hour
    tmp = (time)&0x1f;
    tmp = tmp%30;
    tmp = tmp*2;
    string[15] = (tmp/10)+'0';
    string[16] = (tmp%10)+'0';
    string[17] = 0;
}


T_BOOL ImgBrowse_Open(T_IMGBROWSE * pImgBrowse, T_DISPLAYLIST *pDisplayList)
{
    T_BOOL ret = AK_FALSE;

    if (AK_NULL == pImgBrowse || AK_NULL == pDisplayList)
        return AK_FALSE;

    pImgBrowse->pDisplayList = pDisplayList;

    ImgBrowse_ShowWait(pImgBrowse);
    ret = ImgBrowse_ImgOpen(pImgBrowse);
	
    return ret;
}


T_VOID ImgBrowse_Free(T_IMGBROWSE * pImgBrowse)
{
    ImgBrowse_ImgClose(pImgBrowse);

	pImgBrowse->pDatabuf = Fwl_Free(pImgBrowse->pDatabuf);
	
	if (pImgBrowse->imgTimerShowFast != ERROR_TIMER)
    {
        Fwl_StopTimer(pImgBrowse->imgTimerShowFast);
        pImgBrowse->imgTimerShowFast = ERROR_TIMER;
    }
}

static T_BOOL ImgBrowse_ImgOpen(T_IMGBROWSE * pImgBrowse)
{
    T_USTR_FILE FilePath;
    T_FILE_INFO *pFileInfo;
    T_BOOL ret = AK_FALSE;

    if (AK_NULL == pImgBrowse)
        return ret;

    pFileInfo = DisplayList_GetItemContentFocus(pImgBrowse->pDisplayList);
    if (pFileInfo && ((pFileInfo->attrib & EXPLORER_ISFOLDER) != EXPLORER_ISFOLDER))
    {
        pImgBrowse->rotate = 0;
        pImgBrowse->offsetX = 0;
        pImgBrowse->offsetY = 0;
        pImgBrowse->zoom = 100;
        pImgBrowse->largeFlag = AK_FALSE;

        Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(pImgBrowse->pDisplayList));
        Utl_UStrCat(FilePath, pFileInfo->name);

		switch (pImgBrowse->DisMode)
		{
		case IMG_SLIDE:
		case IMG_BROWSE:
		case IMG_PREVIEW:		
            // image browse open the image, if the image is large than 640*480
            // auto shorten to smaller than 640*480
            if (ImgDec_ImgOpen(&pImgBrowse->ImgAttrib, FilePath, AK_TRUE, &pImgBrowse->largeFlag))
            {
                if (FILE_TYPE_GIF == ImgBrowse_GetFileType(pImgBrowse))
                {
                    ImgBrowse_StartDecTimer(pImgBrowse);
                }
				
                ret = AK_TRUE;
				pImgBrowse->bSupportPic = AK_TRUE;
            }
            else if (pImgBrowse->largeFlag)
            {
                if (ImgDec_LargeImgOpen(&pImgBrowse->LargeImgAttrib, FilePath))
                {
                    ImgBrowse_StartDecTimer(pImgBrowse);
                    ret = AK_TRUE;
					pImgBrowse->bSupportPic = AK_TRUE;
					ScreenSaverDisable();
                }
				else
				{				
					if (ImgDec_ImgOpen(&pImgBrowse->ImgAttrib, NO_SUPPORT_PIC, AK_TRUE, &pImgBrowse->largeFlag))
		            {
		                ret = AK_TRUE;
		            }
					pImgBrowse->bSupportPic = AK_FALSE;
				}
            }
			else
			{
				pImgBrowse->srcImgW = pImgBrowse->ImgAttrib.InImgW;
				pImgBrowse->srcImgH = pImgBrowse->ImgAttrib.InImgH;
				
				if (ImgDec_ImgOpen(&pImgBrowse->ImgAttrib, NO_SUPPORT_PIC, AK_TRUE, &pImgBrowse->largeFlag))
	            {
	                ret = AK_TRUE;
	            }
				pImgBrowse->bSupportPic = AK_FALSE;
			}
			break;
			
		case IMG_MAP:
#if (SDRAM_MODE == 8)
            // electronic map, because the memory is limited,so auto shorten to smaller than the size 800*600..
            if (ImgDec_ImgOpen(&pImgBrowse->ImgAttrib, FilePath, AK_TRUE, &pImgBrowse->largeFlag))
#else
            // electronic map, the map is jpg and is not large, not auto shorten at decompress
            if (ImgDec_ImgOpen(&pImgBrowse->ImgAttrib, FilePath, AK_FALSE, &pImgBrowse->largeFlag))
#endif
            {
                ret = AK_TRUE;
            }
            else
            {
                pImgBrowse->largeFlag = AK_FALSE;
            }
			break;

		default:
			break;
		}

    }

    if (AK_TRUE == ret)
        ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);

    return ret;
}

static T_VOID ImgBrowse_ImgClose(T_IMGBROWSE * pImgBrowse)
{
    ImgBrowse_StopDecTimer(pImgBrowse);

    if (!pImgBrowse->largeFlag)
    {
        ImgDec_ImgClose(&pImgBrowse->ImgAttrib);
    }
    else
    {
        ImgDec_LargeImgClose(&pImgBrowse->LargeImgAttrib);
    }
}


T_BOOL ImgBrowse_Change(T_IMGBROWSE * pImgBrowse)
{
	if (AK_NULL == pImgBrowse)
	{
		return AK_FALSE;
	}
	
	ImgBrowse_ImgClose(pImgBrowse);
	
	return ImgBrowse_ImgOpen(pImgBrowse);
}

static T_BOOL ImgBrowse_ImgStep(T_IMGBROWSE * pImgBrowse)
{
    T_BOOL ret = AK_FALSE;
    T_LARGE_IMG_STATUS LargeDecStatus;

    if (AK_NULL == pImgBrowse)
        return ret;

    if (!pImgBrowse->largeFlag)
    {
        if (FILE_TYPE_GIF == ImgBrowse_GetFileType(pImgBrowse)
			&& ImgDec_GifDecGetNextFrame(&pImgBrowse->ImgAttrib))
        {
            if (pImgBrowse->ImgAttrib.bIntervalChange)
            {
	        	ImgBrowse_StartDecTimer(pImgBrowse);
				pImgBrowse->ImgAttrib.bIntervalChange = AK_FALSE;
            }
			
            ret = AK_TRUE;
        }
    }
    else
    {
        LargeDecStatus = ImgDec_LargeImgStep(&pImgBrowse->LargeImgAttrib);
        if (DEC_CONTINUE == LargeDecStatus)
        {
            ret = AK_TRUE;
        }
        else if (DEC_COMPLETE == LargeDecStatus)
        {
            ImgBrowse_StopDecTimer(pImgBrowse);
			ScreenSaverEnable();

			if (gs.ImgSlideInterval && (IMG_SLIDE == ImgBrowse_GetDisMode(pImgBrowse)))
		    {
		        ImgBrowse_StartSlideTimer(pImgBrowse);	
		    }

            ret = AK_TRUE;
        }
    }
	
    if (ret)
    {
        /*
        GetBMP2RGBBuf(ImgBrowse_GetOutBuf(pImgBrowse), \
                    Fwl_GetDispMemory(), Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), \
                    pImgBrowse->offsetX, pImgBrowse->offsetY, \
                    pImgBrowse->zoom, pImgBrowse->rotate, &pImgBrowse->scale);
                    */
        ImgBrowse_SetRefresh(pImgBrowse, IMG_BROWSE_REFRESH_ALL);
    }

    return ret;
}

T_pDATA ImgBrowse_GetOutBuf(T_IMGBROWSE * pImgBrowse)
{
    T_pDATA pOutBuf = AK_NULL;

    if (AK_NULL != pImgBrowse)
    {
        if (pImgBrowse->largeFlag)
        {
            pOutBuf = ImgDec_GetOutBuf(&pImgBrowse->LargeImgAttrib.ImgAttrib);
        }
        else
        {
            pOutBuf = ImgDec_GetOutBuf(&pImgBrowse->ImgAttrib);
        }
    }

    return pOutBuf;
}
T_BOOL ImgBrowse_GetRGBData(T_IMGBROWSE * pImgBrowse)
{
	T_U8  *pData = AK_NULL;
	T_U32 width;
	T_U32 height;
	T_U8 deep;
    T_U32 compression;
    T_U32 bioffset,colorused;

#ifdef OS_ANYKA
	T_U8  *pTemp = AK_NULL;
	T_U8 DeepBitNum;
	T_U32 uCount;
	T_U32 tmp;
	T_U32 w,h;
	T_U32 color, maskR, maskG, maskB;
	T_U8 palette[256][4];
	T_U8  moveR, moveG, moveB;
	T_U16 lcd_W;
	T_U16 lcd_H;
	T_U16 Zwidth;
	T_U16 Zheight;	
	T_pDATA pDataBuf = AK_NULL;
#endif	
	pData = (T_U8*)ImgBrowse_GetOutBuf(pImgBrowse); 

	if (AK_NULL == pData)
	{
		Fwl_Print(C3, M_IMAGE, "AK_NULL == pData!");
		pImgBrowse->FailReason = IMG_NOT_SUPPORT_PIC;
		return AK_FALSE;
	}

    memcpy((void *)&width, (void *)&pData[0x12], 4);
    memcpy((void *)&height, (void *)&pData[0x16], 4);
	memcpy((void *)&deep, (void *)&pData[0x1c], 2);

	if ((1 != deep) && (4 != deep) && (8 != deep) && (16 != deep) && (24 != deep) &&(32 != deep))
	{
		pImgBrowse->FailReason = IMG_NOT_SUPPORT_PIC;
		return AK_FALSE;
	}

	pImgBrowse->Deep = deep;
	
    memcpy((void *)&compression, (void *)&pData[0x1e], 4);
	memcpy((void *)&bioffset, (void *)&pData[0x0a], 4);
	colorused = (bioffset - 54) >> 2; //(bioffset-54)/4;
	
#ifdef OS_ANYKA
	pDataBuf = Fwl_Malloc(width*height*2+16);
	if (AK_NULL == pDataBuf)
	{
		Fwl_Print(C3, M_IMAGE, "ERR: IMG Get Data Malloc is Fail!");
		pImgBrowse->FailReason = IMG_NO_ENOUGH_MOMORY;
		return AK_FALSE;
	}
	
	if (deep < 24)
	{
		if (deep == 16)
		{
			if (compression == 0)
			{
				maskR = 0x7c00; moveR = 7;
				maskG = 0x03e0; moveG = 2;
				maskB = 0x001f; moveB = 3;
				pData += BMP_HEAD_SIZE;
			}
			else if (compression == 3)
			{
				memcpy((void *)&maskR, (void *)(pData+BMP_HEAD_SIZE+4*0), 4);
				memcpy((void *)&maskG, (void *)(pData+BMP_HEAD_SIZE+4*1), 4);
				memcpy((void *)&maskB, (void *)(pData+BMP_HEAD_SIZE+4*2), 4);
				if (maskR == 0x7c00)
				{
					moveR = 7;
				}
				else if (maskR == 0xF800)
				{
					moveR = 8;
				}
				else
				{
					pDataBuf = Fwl_Free (pDataBuf);
					return AK_FALSE;
				}
				if (maskG == 0x03e0)
				{
					moveG = 2;
				}
				else if (maskG == 0x07e0)
				{
					moveG = 3;
				}
				else
				{
					pDataBuf = Fwl_Free (pDataBuf);
					return AK_FALSE;
				}
				if (maskB == 0x001f)
				{
					moveB = 3;
				}
				else
				{
					pDataBuf = Fwl_Free (pDataBuf);
					return AK_FALSE;
				}
				pData = pData + BMP_HEAD_SIZE + 3*4;
			}
			else
			{
				pDataBuf = Fwl_Free (pDataBuf);
				return AK_FALSE;
			}
			DeepBitNum = 2;
		}
		else  //dedp < 16
		{	
			memcpy((void *)palette, (void *)(pData+BMP_HEAD_SIZE), colorused*4);
			pData = pData + bioffset;
			DeepBitNum = 1;
		}
	}
	else
	{
		pData += BMP_HEAD_SIZE;
		if (24 == deep)
		{
			DeepBitNum = 3;
		}
		else
		{
			DeepBitNum = 4;
		}
		
	}

    switch (pImgBrowse->rotate)
   	{
	case 0:
		uCount = 0;
		
		if (1 == deep)
        {
			pData = pData + ((width*DeepBitNum/8+3)&(~3)) * (height-1);
			
			for(h=0; h<height; ++h)
			{
				for(w=0; w < width*DeepBitNum;)
				{
					if (0 == (w%8))
					{
						color = 0;	
						memcpy((void *)&color, (void *)&pData[w/8], DeepBitNum);
					}
					
					pDataBuf[uCount++] = (palette[(color>>(7-w%8))&0x01][0]>>3) |
						((palette[(color>>(7-w%8))&0x01][1] << 3) & 0x0C0 );
					pDataBuf[uCount++] = (palette[(color>>(7-w%8))&0x01][2] & 0x0F8) |
			    		(palette[(color>>(7-w%8))&0x01][1] >> 5);
					
					w += DeepBitNum;
				}
				
				pData -= (width*DeepBitNum/8+3)&(~3);
			}
         }
         else if (4 == deep)
         {     
         	pData = pData + ((width*DeepBitNum/2+3)&(~3)) * (height-1);
			
			for (h=0; h<height; ++h)
			{
				for (w=0; w < width*DeepBitNum; )
				{
					if (0 == (w%2))
					{
						color = 0;	
				   		memcpy((void *)&color, (void *)&pData[w/2], DeepBitNum);
					}
					
					pDataBuf[uCount++] = (palette[(color>>(4*(1-w%2)))&0x0f][0]>>3) |
						((palette[(color>>(4*(1-w%2)))&0x0f][1] << 3) & 0x0E0);
					pDataBuf[uCount++] = (palette[(color>>(4*(1-w%2)))&0x0f][2] & 0x0F8) |
						(palette[(color>>(4*(1-w%2)))&0x0f][1] >> 5); 
					
					w += DeepBitNum;
				}
				
				pData -= (width*DeepBitNum/2+3)&(~3);
			}	
         }
         else if (8 == deep)
         {
			pData = pData + ((width*DeepBitNum+3)&(~3)) * (height-1);
			
			for(h=0; h<height; ++h)
			{
				for(w=0; w < width*DeepBitNum; )
				{
				    color = 0;	
				    memcpy((void *)&color, (void *)&pData[w], DeepBitNum);
					pDataBuf[uCount++] = (palette[color][0] >> 3) |
						((palette[color][1] << 3) & 0x0C0);
					pDataBuf[uCount++] = (palette[color][2] & 0x0F8) |
						(palette[color][1] >> 5);  
	
					w += DeepBitNum;
				}	
				
				pData -= (width*DeepBitNum+3)&(~3);
			}				
        }
		else if (16 == deep)
		{
			pData = pData + ((width*DeepBitNum+3)&(~3)) * (height-1);
			
			for (h=0; h<height; ++h)
			{
				for (w=0; w < width*DeepBitNum; )
				{
					color = 0;	
				    memcpy((void *)&color, (void *)&pData[w], DeepBitNum);
					pDataBuf[uCount++] = ((T_U8)((color&maskB)<<moveB) >> 3) |
						(((T_U8)((color&maskG)>>moveG) << 3) & 0x0C0);
					
					pDataBuf[uCount++] = ((T_U8)((color&maskR)>>moveR) & 0x0F8) |
						(((T_U8)((color&maskG)>>moveG) >> 5));  
	
					w += DeepBitNum;
				}	
	
				pData -= (width*DeepBitNum+3)&(~3) ;
			}	
		}
		else if (24 <= deep)
		{
			pData = pData + ((width*DeepBitNum+3)&(~3)) * (height-1);
			
			for(h=0; h<height; ++h)
			{
				for(w=0; w < width*DeepBitNum; )
				{
					pDataBuf[uCount++] = ((pData[w+1] << 3) & 0x0E0) | (pData[w] >> 3);
					pDataBuf[uCount++] = (pData[w+2] & 0x0F8) | (pData[w+1] >> 5);
	
					w += DeepBitNum;
				}	
	
				pData -= (width*DeepBitNum+3)&(~3) ;
			}	
		}
		break;
		
	case 90:
		pTemp = pData;
		tmp = height;
		height = width;
		width = tmp;
		uCount = 0;
		
		if (1 == deep)
		{
			for(w=0; w<height*DeepBitNum;)
			{
				for(h=0; h < width; h++)
				{
					color = 0;	
					memcpy((void *)&color, (void *)&pData[w/8], DeepBitNum);
				
					pDataBuf[uCount++] = (palette[(color>>(7-w%8))&0x01][0]>>3) |
						((palette[(color>>(7-w%8))&0x01][1] << 3) & 0x0C0 );
					pDataBuf[uCount++] = (palette[(color>>(7-w%8))&0x01][2] & 0x0F8) |
			    		(palette[(color>>(7-w%8))&0x01][1] >> 5);

					pData += (height*DeepBitNum/8+3)&(~3);

				}
				
				w += DeepBitNum;

				pData=pTemp;
	
			}
		}
		else if (4 == deep)
		{
			for (w=0; w<height*DeepBitNum;)
			{
				for(h=0; h < width; h++)
				{
					
					color = 0;	
					memcpy((void *)&color, (void *)&pData[w/2], DeepBitNum);
				
					pDataBuf[uCount++] = (palette[(color>>(4*(1-w%2)))&0x0f][0]>>3) |
						((palette[(color>>(4*(1-w%2)))&0x0f][1] << 3) & 0x0E0);
					pDataBuf[uCount++] = (palette[(color>>(4*(1-w%2)))&0x0f][2] & 0x0F8) |
						(palette[(color>>(4*(1-w%2)))&0x0f][1] >> 5); 

					pData += (height*DeepBitNum/2+3)&(~3);

				}
				
				w += DeepBitNum;

				pData=pTemp;
				
			}
		}
		else if (8 == deep)
		{
			for (w=0; w<height*DeepBitNum; )
			{
				for(h=0; h < width; h++)
				{
				    color = 0;	
				    memcpy((void *)&color, (void *)&pData[w], DeepBitNum);
					pDataBuf[uCount++] = (palette[color][0] >> 3) |
				((palette[color][1] << 3) & 0x0C0);
					pDataBuf[uCount++] = (palette[color][2] & 0x0F8) |
				(palette[color][1] >> 5);  
			
					pData += (height*DeepBitNum+3)&(~3);
				}	
				w += DeepBitNum;
				pData=pTemp;
			}
		}
		else if (16 == deep)
		{
			for (w=0; w<height*DeepBitNum; )
			{
				for(h=0; h < width; h++)
				{
					color = 0;	
					memcpy((void *)&color, (void *)&pData[w], DeepBitNum);
					pDataBuf[uCount++] = ((T_U8)((color&maskB)<<moveB) >> 3) |
						(((T_U8)((color&maskG)>>moveG) << 3) & 0x0C0);
					
					pDataBuf[uCount++] = ((T_U8)((color&maskR)>>moveR) & 0x0F8) |
						(((T_U8)((color&maskG)>>moveG) >> 5));  
			
					pData += (height*DeepBitNum+3)&(~3);
				}	
				
				w += DeepBitNum;
				pData=pTemp;
			}
		}
		else if (24 <= deep)
		{
			for(w=0; w<height*DeepBitNum; )
			{
				for(h=0; h < width; h++)
				{
					pDataBuf[uCount++] = ((pData[w+1] << 3) & 0x0E0) | (pData[w] >> 3);
					pDataBuf[uCount++] = (pData[w+2] & 0x0F8) | (pData[w+1] >> 5);
			
					pData += (height*DeepBitNum+3)&(~3);
				}
				
				w += DeepBitNum;
				pData=pTemp;
			}
		}
		break;
		
	case 180:
		uCount = 0;
		
		if (1 == deep)
		{
			for(h=0; h<height; ++h)
			{
				for(w=width*DeepBitNum; w > 0;)
				{
					if (w%8)
					{
						color = 0;	
				   		memcpy((void *)&color, (void *)&pData[w/8], DeepBitNum);
					}
					
					pDataBuf[uCount++] = (palette[(color>>(7-w%8))&0x01][0]>>3) |
						((palette[(color>>(7-w%8))&0x01][1] << 3) & 0x0C0 );
					pDataBuf[uCount++] = (palette[(color>>(7-w%8))&0x01][2] & 0x0F8) |
			    		(palette[(color>>(7-w%8))&0x01][1] >> 5);
					
					w -= DeepBitNum;

				}	
				pData += (width*DeepBitNum/8+3)&(~3) ;
			}
		}
		else if (4 == deep)
		{
			for(h=0; h<height; ++h)
			{
				for(w=width*DeepBitNum; w > 0;)
				{
					if (w%2)
					{
						color = 0;	
				   		memcpy((void *)&color, (void *)&pData[w/2], DeepBitNum);
					}
					pDataBuf[uCount++] = (palette[(color>>(4*(1-w%2)))&0x0f][0]>>3) |
						((palette[(color>>(4*(1-w%2)))&0x0f][1] << 3) & 0x0E0);
					pDataBuf[uCount++] = (palette[(color>>(4*(1-w%2)))&0x0f][2] & 0x0F8) |
						(palette[(color>>(4*(1-w%2)))&0x0f][1] >> 5); 
					
					w -= DeepBitNum;
				}	
				
				pData += (width*DeepBitNum/2+3)&(~3) ;
			}
		}
		else if (8 == deep)
		{
			for(h=0; h<height; ++h)
			{
				for(w=width*DeepBitNum; w > 0; )
				{
					color = 0;	
				    memcpy((void *)&color, (void *)&pData[w], DeepBitNum);
					pDataBuf[uCount++] = (palette[color][0] >> 3) |
						((palette[color][1] << 3) & 0x0C0);
					pDataBuf[uCount++] = (palette[color][2] & 0x0F8) |
						(palette[color][1] >> 5);  
					w -= DeepBitNum;
				}
				
				pData += (width*DeepBitNum+3)&(~3) ;
			}
		}
		else if (16 == deep)
		{
			for(h=0; h<height; ++h)
			{
				for(w=width*DeepBitNum; w > 0 ; )
				{
					color = 0;	
					memcpy((void *)&color, (void *)&pData[w], DeepBitNum);
					pDataBuf[uCount++] = ((T_U8)((color&maskB)<<moveB) >> 3) |
						(((T_U8)((color&maskG)>>moveG) << 3) & 0x0C0);
					
					pDataBuf[uCount++] = ((T_U8)((color&maskR)>>moveR) & 0x0F8) |
						(((T_U8)((color&maskG)>>moveG) >> 5));  
					w -= DeepBitNum;
				}	
				
				pData += (width*DeepBitNum+3)&(~3) ;
			}
		}
		else if (24 <= deep)
		{
			for(h=0; h<height; ++h)
			{
				for(w=width*DeepBitNum; w >0 ; )
				{
					w -= DeepBitNum;
					pDataBuf[uCount++] = ((pData[w+1] << 3) & 0x0E0) | (pData[w] >> 3);
					pDataBuf[uCount++] = (pData[w+2] & 0x0F8) | (pData[w+1] >> 5);

				}	
				
				pData += (width*DeepBitNum+3)&(~3) ;
			}
		}	
		break;
		
	case 270:
		uCount = 0;
		
		if (1 == deep)
		{
			pData = pData + ((width*DeepBitNum/8+3)&(~3)) * (height-1);
			pTemp = pData;
			tmp = height;
			height = width;
			width = tmp;
			
			for(w=height*DeepBitNum; w>0;)
			{
				
				w -= DeepBitNum;

				for(h=0; h < width; h++)
				{
					color = 0;	
					memcpy((void *)&color, (void *)&pData[w/8], DeepBitNum);
				
					pDataBuf[uCount++] = (palette[(color>>(7-w%8))&0x01][0]>>3) |
						((palette[(color>>(7-w%8))&0x01][1] << 3) & 0x0C0 );
					pDataBuf[uCount++] = (palette[(color>>(7-w%8))&0x01][2] & 0x0F8) |
			    		(palette[(color>>(7-w%8))&0x01][1] >> 5);

					pData -= (height*DeepBitNum/8+3)&(~3);
				}
				
				pData=pTemp;
			}
		}
		else if (4 == deep)
		{
			pData = pData + ((width*DeepBitNum/2+3)&(~3)) * (height-1);
			pTemp = pData;
			tmp = height;
			height = width;
			width = tmp;
			
			for(w=height*DeepBitNum; w>0;)
			{
				
				w -= DeepBitNum;

				for(h=0; h < width; h++)
				{
					color = 0;	
					memcpy((void *)&color, (void *)&pData[w/2], DeepBitNum);
				
					pDataBuf[uCount++] = (palette[(color>>(4*(1-w%2)))&0x0f][0]>>3) |
						((palette[(color>>(4*(1-w%2)))&0x0f][1] << 3) & 0x0E0);
					pDataBuf[uCount++] = (palette[(color>>(4*(1-w%2)))&0x0f][2] & 0x0F8) |
						(palette[(color>>(4*(1-w%2)))&0x0f][1] >> 5); 

				    pData -= (height*DeepBitNum/2+3)&(~3);
				}
				
				pData=pTemp;
			}
		}
		else if (8 == deep)
		{
			pData = pData + ((width*DeepBitNum+3)&(~3)) * (height-1);
			pTemp = pData;
			tmp = height;
			height = width;
			width = tmp;
			
			for(w=height*DeepBitNum; w > 0; )
			{
				w -= DeepBitNum;
				
				for(h=0; h < width; h++)
				{
					color = 0;	
				    memcpy((void *)&color, (void *)&pData[w], DeepBitNum);
					pDataBuf[uCount++] = (palette[color][0] >> 3) |
						((palette[color][1] << 3) & 0x0C0);
					pDataBuf[uCount++] = (palette[color][2] & 0x0F8) |
						(palette[color][1] >> 5);  

				    pData -= (height*DeepBitNum+3)&(~3);
				}
				
				pData=pTemp;
			}
		}
		else if (16 == deep)
		{
		    pData = pData + ((width*DeepBitNum+3)&(~3)) * (height-1);
			pTemp = pData;
			tmp = height;
			height = width;
			width = tmp;
			
			for(w=height*DeepBitNum; w>0; )
			{
				w -= DeepBitNum;
				for(h=0; h < width; h++)
				{
					color = 0;	
					memcpy((void *)&color, (void *)&pData[w], DeepBitNum);
					pDataBuf[uCount++] = ((T_U8)((color&maskB)<<moveB) >> 3) |
						(((T_U8)((color&maskG)>>moveG) << 3) & 0x0C0);
					
					pDataBuf[uCount++] = ((T_U8)((color&maskR)>>moveR) & 0x0F8) |
						(((T_U8)((color&maskG)>>moveG) >> 5));	

				    pData -= (height*DeepBitNum+3)&(~3);
				}	
				
				pData=pTemp;
			}
		}
		else if (24 <= deep)
		{
			pData = pData + ((width*DeepBitNum+3)&(~3)) * (height-1);
			pTemp = pData;
			tmp = height;
			height = width;
			width = tmp;
			
			for(w=height*DeepBitNum; w>0; )
			{
				w -= DeepBitNum;
				
				for(h=0; h < width; h++)
				{
					pDataBuf[uCount++] = ((pData[w+1] << 3) & 0x0E0) | (pData[w] >> 3);
					pDataBuf[uCount++] = (pData[w+2] & 0x0F8) | (pData[w+1] >> 5);

				    pData -= (height*DeepBitNum+3)&(~3);
				}	
				
				pData=pTemp;
			}
		}	
		break;
		
	default:
		break;
	}

    
	lcd_W = Fwl_GetLcdWidth();
	lcd_H = Fwl_GetLcdHeight();
    if (pImgBrowse->IMG_ROTATE_90_FLAG)
   	{
		if (width >= lcd_W || height >= lcd_H)
		{
            T_U32 srcWZoomDef;
			T_U32 srcHZoomDef;
			
			if (width*100/lcd_W > height*100/lcd_H)
			{
				Zwidth = lcd_W;
				Zheight = height*lcd_W/width;
			}
			else
			{
				Zwidth = lcd_H*width/height;
				Zheight = lcd_H; 
			}
			
			srcWZoomDef = 100*width/lcd_W;
			srcHZoomDef = 100*height/lcd_H;
			pImgBrowse->zoom = (srcWZoomDef >= srcHZoomDef) ? srcWZoomDef : srcHZoomDef;
			
			if (pImgBrowse->zoom > 400)
			{
				pImgBrowse->zoom = 400;
				Zwidth = width*100/pImgBrowse->zoom;
				Zheight= height*100/pImgBrowse->zoom;
			}
		}
		else
		{
			Zwidth = width;
			Zheight = height;
		}
	}
	else
	{
		Zwidth = width*100/pImgBrowse->zoom;
		Zheight= height*100/pImgBrowse->zoom;
	}

	pImgBrowse->srcW = Zwidth;
	pImgBrowse->srcH = Zheight;

	if ((pImgBrowse->DEFLAT_FLAG == AK_TRUE) || (pImgBrowse->IMG_GETDATA_FLAG == AK_TRUE))
	{
		pImgBrowse->virleft = (lcd_W-Zwidth)/2;
		pImgBrowse->virtop = (lcd_H-Zheight)/2;

		Fwl_Print(C3, M_IMAGE, "Zwidth:%d,Zheight:%d,width:%d,height:%d,zoom:%d",\
		          Zwidth,Zheight,width,height,pImgBrowse->zoom);
	}
	
    if ((Zwidth>1280) || (width>1280) || (Zheight>1024) || (height>1024)
		|| (Zwidth<18) || (width<18) || (Zheight<18) || (height<18))
    {
    	Fwl_Print(C3, M_IMAGE, "img size not support!");

		pImgBrowse->FailReason = IMG_NOT_SUPPORT_SIZE;
		
		if (AK_NULL != pDataBuf)
		{
			Fwl_Free (pDataBuf);
			pDataBuf = AK_NULL;
		}

		return AK_FALSE;
	}
	
	Reset_2DGraphic();
	Fwl_Scale_Convert(pImgBrowse->pDatabuf,Zwidth,Zheight,0,0,\
		Zwidth,pDataBuf,width, height, RGB565);

	if (AK_NULL != pDataBuf)
	{
		Fwl_Free(pDataBuf);
		pDataBuf = AK_NULL;
	}
#endif

	return AK_TRUE;
}

#ifdef DEMO_IMGBROWSE_BIG_PIC   //define in fwl_display.h
static T_U8 Data_Array[720*576*3+64];
#endif

T_pDATA ImgBrowse_GetShowBuf_ex(T_IMGBROWSE * pImgBrowse)
{
	T_U8 *srcBMP = AK_NULL;
	T_U32 imgW=0;
	T_U32 imgH=0;
	
	if (AK_NULL == pImgBrowse->pDatabuf)
	{
		srcBMP = ImgBrowse_GetOutBuf(pImgBrowse);

		if (AK_NULL == srcBMP)
		{
			return AK_NULL;
		}
		
		memcpy((void *)&imgW, (void *)&srcBMP[0x12], 4);
		memcpy((void *)&imgH, (void *)&srcBMP[0x16], 4);
				
		pImgBrowse->pDatabuf = Fwl_Malloc(imgW*imgH*2+64);		
		
        if (pImgBrowse->pDatabuf != AK_NULL)
       	{
       		memset(pImgBrowse->pDatabuf, 0, imgW*imgH*2+64);
			return pImgBrowse->pDatabuf;
		}
	}
	
	return pImgBrowse->pDatabuf;
}
T_VOID ImgBrowse_SetDispInitZoom(T_IMGBROWSE * pImgBrowse)
{
	T_U32 width = 0;
	T_U32 height = 0;	
	T_U32 lcd_W = 0;
	T_U32 lcd_H = 0;
	T_U32 srcWZoomDef = 0;
	T_U32 srcHZoomDef = 0;
	
	width = ImgBrowse_GetOutImgW(pImgBrowse);
	height = ImgBrowse_GetOutImgH(pImgBrowse);
#ifdef  OS_ANYKA	
	lcd_W = Fwl_GetLcdWidth();
	lcd_H = Fwl_GetLcdHeight();
#else
	lcd_W = MAIN_LCD_WIDTH;
    lcd_H = MAIN_LCD_HEIGHT;
#endif  
	if (width >= lcd_W || height >= lcd_H)
	{
		srcWZoomDef = 100*width/lcd_W;
		srcHZoomDef = 100*height/lcd_H;
		pImgBrowse->zoom = (srcWZoomDef >= srcHZoomDef) ? srcWZoomDef : srcHZoomDef;
		
		if (pImgBrowse->zoom > 400)
		{
			pImgBrowse->zoom = 400;
		}
		pImgBrowse->virleft = 0;
	    pImgBrowse->virtop = 0; 
	}
	else
	{
		pImgBrowse->zoom = 100;
		pImgBrowse->virleft = (T_POS)((lcd_W-width)/2);
	    pImgBrowse->virtop = (T_POS)((lcd_H-height)/2); 	
	}
	
	Fwl_Print(C3, M_IMAGE, "InitZoom:%d",pImgBrowse->zoom);
}


#ifdef DEMO_IMGBROWSE_BIG_PIC   //define in fwl_display.h
static T_U8 Data_Array[720*576*3+64];
#endif


T_BOOL ImgBrowse_ShowImg(T_IMGBROWSE * pImgBrowse)
{
    if (AK_NULL != pImgBrowse)
    {				
        if (ImgBrowse_GetOutBuf(pImgBrowse) != AK_NULL)
        {			
			if (FILE_TYPE_GIF == ImgBrowse_GetFileType(pImgBrowse) &&
				( pImgBrowse->pDisplayList->IconExplorer.pItemFocus == 
				pItemFocus_ImgBrowse))
			{
				Flag_ImgBrowse_ClrBuf = AK_FALSE;				
			}
			else
			{
				Flag_ImgBrowse_ClrBuf = AK_TRUE;				
			}

			pItemFocus_ImgBrowse = 
				pImgBrowse->pDisplayList->IconExplorer.pItemFocus;
			
			if ((ImgBrowse_GetDisMode(pImgBrowse) == IMG_MAP)
				|| (ImgBrowse_GetDisMode(pImgBrowse) == IMG_PREVIEW)
				|| (ImgBrowse_GetDisMode(pImgBrowse) == IMG_SLIDE))
			{   
		    	return GetBMP2RGBBuf(ImgBrowse_GetOutBuf(pImgBrowse), \
			            Fwl_GetDispMemory(), Fwl_GetLcdWidth(), Fwl_GetLcdHeight(), \
			            pImgBrowse->offsetX, pImgBrowse->offsetY, \
			            pImgBrowse->zoom, pImgBrowse->rotate, &pImgBrowse->scale,COLOR_BLACK);
						
			}
			else
			{				
	            ImgBrowse_GetShowBuf_ex(pImgBrowse);

				if (pImgBrowse->DEFLAT_FLAG)
				{
					ImgBrowse_SetDispInitZoom(pImgBrowse);
				}
			
				if ((pImgBrowse->DEFLAT_FLAG == AK_TRUE)
					|| (pImgBrowse->IMG_GETDATA_FLAG == AK_TRUE)
					|| pImgBrowse->bGifStepFlag)
				{
					ImgBrowse_GetRGBData(pImgBrowse);
				}
			}
        }
        else
        {
            return AK_FALSE;
        }

        return AK_TRUE;
    }

    return AK_FALSE;
}

static T_FILE_TYPE ImgBrowse_GetFileType(T_IMGBROWSE * pImgBrowse)
{
    T_FILE_TYPE FileType = 0;

    if (AK_NULL != pImgBrowse)
    {
        if (pImgBrowse->largeFlag)
        {
            FileType = ImgDec_GetFileType(&pImgBrowse->LargeImgAttrib.ImgAttrib);
        }
        else
        {
            FileType = ImgDec_GetFileType(&pImgBrowse->ImgAttrib);
        }
    }

    return FileType;
}

static T_VOID ImgBrowse_StartImgCtlTimer(T_IMGBROWSE * pImgBrowse)
{
    if (pImgBrowse->imgCtlTimer!= ERROR_TIMER)
    {
        Fwl_StopTimer(pImgBrowse->imgCtlTimer);
        pImgBrowse->imgCtlTimer = ERROR_TIMER;
    }
	
    pImgBrowse->imgCtlTimer = Fwl_SetTimerSecond(IMG_CTL_SHOW_INTERVAL, AK_TRUE);

}
static T_VOID ImgBrowse_StopImgCtlTimer(T_IMGBROWSE * pImgBrowse)
{
    if (pImgBrowse->imgCtlTimer!= ERROR_TIMER)
    {
        Fwl_StopTimer(pImgBrowse->imgCtlTimer);
        pImgBrowse->imgCtlTimer = ERROR_TIMER;
    }
}

static T_VOID ImgBrowse_StartDecTimer(T_IMGBROWSE * pImgBrowse)
{
    T_U32 interval;

    if (pImgBrowse->DecTimerId != ERROR_TIMER)
    {
        Fwl_StopTimer(pImgBrowse->DecTimerId);
        pImgBrowse->DecTimerId = ERROR_TIMER;
    }
	
    if (!pImgBrowse->largeFlag)
    {
        interval = pImgBrowse->ImgAttrib.GifFrameInterval;
    }
    else
    {
        if (ImgBrowse_GetFileType(pImgBrowse) == FILE_TYPE_BMP)
        {
            //interval = IMAGE_BMP_STEPTIME*pImgBrowse->ImgAttrib.InImgW/LARGE_DEC_BMP_DST_HEIGHT_MAX;
            interval = IMAGE_BMP_STEPTIME;
        }
        else
        {
            interval = IMAGE_JPG_STEPTIME;//IMAGE_JPG_STEPTIME*pImgBrowse->ImgAttrib.InImgW/LARGE_DEC_JPG_DST_WIDTH_MAX;
        }
    }

    pImgBrowse->DecTimerId = Fwl_SetTimerMilliSecond(interval, AK_TRUE);
}

static T_VOID ImgBrowse_StopDecTimer(T_IMGBROWSE * pImgBrowse)
{
    if (pImgBrowse->DecTimerId != ERROR_TIMER)
    {
        Fwl_StopTimer(pImgBrowse->DecTimerId);
        pImgBrowse->DecTimerId = ERROR_TIMER;
    }
}


T_VOID ImgBrowse_StartImgShowFastTimer(T_IMGBROWSE * pImgBrowse)
{
    if (pImgBrowse->imgTimerShowFast!= ERROR_TIMER)
    {
        Fwl_StopTimer(pImgBrowse->imgTimerShowFast);
        pImgBrowse->imgTimerShowFast = ERROR_TIMER;
    }
	
    pImgBrowse->imgTimerShowFast = Fwl_SetTimerMilliSecond(IMG_SHOW_FAST_TIMER, AK_TRUE);//IMG_CTL_SHOW_INTERVAL
}
T_VOID ImgBrowse_StopImgShowFastTimer(T_IMGBROWSE * pImgBrowse)
{
    if (pImgBrowse->imgTimerShowFast!= ERROR_TIMER)
    {
        Fwl_StopTimer(pImgBrowse->imgTimerShowFast);
        pImgBrowse->imgTimerShowFast = ERROR_TIMER;
    }
}
T_U32 ImgBrowse_GetInImgW(T_IMGBROWSE * pImgBrowse)
{
    if (!pImgBrowse->largeFlag)
    {
    	if (pImgBrowse->bSupportPic)
    	{
        	return pImgBrowse->ImgAttrib.InImgW;
    	}
		else
		{
			return pImgBrowse->srcImgW;
		}
    }
    else
    {
        return pImgBrowse->LargeImgAttrib.ImgAttrib.InImgW;
    }
}

T_U32 ImgBrowse_GetInImgH(T_IMGBROWSE * pImgBrowse)
{
    if (!pImgBrowse->largeFlag)
    {
        if (pImgBrowse->bSupportPic)
    	{
        	return pImgBrowse->ImgAttrib.InImgH;
    	}
		else
		{
			return pImgBrowse->srcImgH;
		}
    }
    else
    {
        return pImgBrowse->LargeImgAttrib.ImgAttrib.InImgH;
    }
}

T_U32 ImgBrowse_GetOutImgW(T_IMGBROWSE * pImgBrowse)
{
    if (!pImgBrowse->largeFlag)
    {
        return pImgBrowse->ImgAttrib.OutImgW;
    }
    else
    {
        return pImgBrowse->LargeImgAttrib.ImgAttrib.OutImgW;
    }
}

T_U32 ImgBrowse_GetOutImgH(T_IMGBROWSE * pImgBrowse)
{
    if (!pImgBrowse->largeFlag)
    {
        return pImgBrowse->ImgAttrib.OutImgH;
    }
    else
    {
        return pImgBrowse->LargeImgAttrib.ImgAttrib.OutImgH;
    }
}

T_BOOL ImgBrowse_IsBrowseSupportFileType(T_USTR_FILE pFileName)
{
    T_U8 FileType;
    T_BOOL ret = AK_FALSE;

    FileType = Utl_GetFileType(pFileName);

    switch (FileType)
    {
    case FILE_TYPE_BMP:
    case FILE_TYPE_JPG:
    case FILE_TYPE_JPEG:
    case FILE_TYPE_JPE:
    case FILE_TYPE_PNG:
    case FILE_TYPE_GIF:
        ret = AK_TRUE;
        break;
		
    default:
        break;
    }

    return ret;
}

T_BOOL ImgBrowse_IsEmapSupportFileType(T_USTR_FILE pFileName)
{
    T_U8 FileType;
    T_BOOL ret = AK_FALSE;

    FileType = Utl_GetFileType(pFileName);
	
    if (FILE_TYPE_MAP == FileType)
    {
        ret = AK_TRUE;
    }

    return ret;
}


T_BOOL ImgBrowse_SetDisMode(T_IMGBROWSE * pImgBrowse, T_IMG_DIS_MODE DisMode)
{
    if (AK_NULL == pImgBrowse)
    {
        return AK_FALSE;
    }

    if (IMG_MAP == DisMode)
    {
        ImgBrowse_SetDisPostfixMode(pImgBrowse, IMG_NOT_DISPOSTFIX);
        ImgBrowse_SetDisPixelMode(pImgBrowse, IMG_NOT_DISPIXEL);
    }
    else
    {
        ImgBrowse_SetDisPostfixMode(pImgBrowse, IMG_DISPOSTFIX);
        ImgBrowse_SetDisPixelMode(pImgBrowse, IMG_DISPIXEL);
    }
	
    pImgBrowse->DisMode = DisMode;

    return AK_TRUE;
}

T_IMG_DIS_MODE ImgBrowse_GetDisMode(T_IMGBROWSE * pImgBrowse)
{
    if (AK_NULL != pImgBrowse)

    {
        return pImgBrowse->DisMode;
    }
	
    return IMG_BROWSE;
}


static T_BOOL ImgBrowse_SetDisPostfixMode(T_IMGBROWSE * pImgBrowse, T_IMG_DIS_POSTFIX DisPostfix)
{
    if (AK_NULL == pImgBrowse)
    {
        return AK_FALSE;
    }

    if (DisPostfix < IMG_DISPOSTFIX_NUM)
    {
        pImgBrowse->DisPostfix = DisPostfix;
        return AK_TRUE;
    }

    return AK_FALSE;
}

static T_IMG_DIS_POSTFIX ImgBrowse_GetDisPostfixMode(T_IMGBROWSE * pImgBrowse)
{
    if (AK_NULL != pImgBrowse)

    {
        return pImgBrowse->DisPostfix;
    }
	
    return IMG_NOT_DISPIXEL;
}

static T_BOOL ImgBrowse_SetDisPixelMode(T_IMGBROWSE * pImgBrowse, T_IMG_DIS_POSTFIX DisPixel)
{
    if (AK_NULL == pImgBrowse)
    {
        return AK_FALSE;
    }

    if (DisPixel < IMG_DISPIXEL_NUM)
    {
        pImgBrowse->DisPixel = DisPixel;
        return AK_TRUE;
    }

    return AK_FALSE;
}

static T_IMG_DIS_PIXEL ImgBrowse_GetDisPixelMode(T_IMGBROWSE * pImgBrowse)
{
    if (AK_NULL != pImgBrowse)

    {
        return pImgBrowse->DisPixel;
    }
	
    return IMG_NOT_DISPIXEL;
}

static T_VOID ImgBrowse_ShowWait(T_IMGBROWSE * pImgBrowse)
{
    /**start waitbox*/
	if (IMG_MAP == ImgBrowse_GetDisMode(pImgBrowse))
    {
    	WaitBox_Start(WAITBOX_RAINBOW, (T_pWSTR)GetCustomString(csWAITING));
    }
}

static T_VOID ImgBrowse_GetRes(T_IMGBROWSE * pImgBrowse)
{
 //#ifdef TOUCH_SCR

    T_U32   len;

    //background rectangle
    pImgBrowse->BkgdRect.pRes = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_IMAGE_TOOLBAR_BACKGROUND, AK_NULL);
	AKBmpGetInfo(pImgBrowse->BkgdRect.pRes, &pImgBrowse->BkgdRect.ButtnRect.width, &pImgBrowse->BkgdRect.ButtnRect.height, AK_NULL);
    pImgBrowse->BkgdRect.ButtnRect.left = Fwl_GetLcdWidth() - pImgBrowse->BkgdRect.ButtnRect.width;
    pImgBrowse->BkgdRect.ButtnRect.top = 0;

    //rotate button
    pImgBrowse->RotateBttn.pRes = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_ROTATE, &len);
    AKBmpGetInfo(pImgBrowse->RotateBttn.pRes, &pImgBrowse->RotateBttn.ButtnRect.width, &pImgBrowse->RotateBttn.ButtnRect.height, AK_NULL);
    pImgBrowse->RotateBttn.ButtnRect.left = pImgBrowse->BkgdRect.ButtnRect.left
                                            + (pImgBrowse->BkgdRect.ButtnRect.width - pImgBrowse->RotateBttn.ButtnRect.width) / 2;
    pImgBrowse->RotateBttn.ButtnRect.top = pImgBrowse->BkgdRect.ButtnRect.top 
                                           + (pImgBrowse->BkgdRect.ButtnRect.height - pImgBrowse->RotateBttn.ButtnRect.height*2) / 2;

    /*down button and rotate button are in the same position, 
            if in the emap mode, show down button, else show rotate button 
        */
    pImgBrowse->DownBttn.pRes = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_DOWN, &len);
    AKBmpGetInfo(pImgBrowse->DownBttn.pRes, &pImgBrowse->DownBttn.ButtnRect.width, &pImgBrowse->DownBttn.ButtnRect.height, AK_NULL);
    pImgBrowse->DownBttn.ButtnRect.left = pImgBrowse->BkgdRect.ButtnRect.left
                                            + (pImgBrowse->BkgdRect.ButtnRect.width - pImgBrowse->DownBttn.ButtnRect.width) / 2;
    pImgBrowse->DownBttn.ButtnRect.top = pImgBrowse->BkgdRect.ButtnRect.top 
                                           + (pImgBrowse->BkgdRect.ButtnRect.height - pImgBrowse->DownBttn.ButtnRect.height*2) / 2;

    
    //next button
    pImgBrowse->NextBttn.pRes = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_NEXT, &len);
    AKBmpGetInfo(pImgBrowse->NextBttn.pRes, &pImgBrowse->NextBttn.ButtnRect.width, &pImgBrowse->NextBttn.ButtnRect.height, AK_NULL);
    pImgBrowse->NextBttn.ButtnRect.left = pImgBrowse->RotateBttn.ButtnRect.left;
    pImgBrowse->NextBttn.ButtnRect.top = pImgBrowse->RotateBttn.ButtnRect.top - pImgBrowse->NextBttn.ButtnRect.height
                                         - IMAGE_BUTTON_INTERVAL; 
    //previous button
    pImgBrowse->PrvBttn.pRes = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_PREVIOUS, &len);
    AKBmpGetInfo(pImgBrowse->PrvBttn.pRes, &pImgBrowse->PrvBttn.ButtnRect.width, &pImgBrowse->PrvBttn.ButtnRect.height, AK_NULL);
    pImgBrowse->PrvBttn.ButtnRect.left = pImgBrowse->RotateBttn.ButtnRect.left;
    pImgBrowse->PrvBttn.ButtnRect.top = pImgBrowse->NextBttn.ButtnRect.top - pImgBrowse->PrvBttn.ButtnRect.height
                                        - IMAGE_BUTTON_INTERVAL; 
    //menu button
    pImgBrowse->MenuBttn.pRes = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_MENU, &len);
    AKBmpGetInfo(pImgBrowse->MenuBttn.pRes, &pImgBrowse->MenuBttn.ButtnRect.width, &pImgBrowse->MenuBttn.ButtnRect.height, AK_NULL);
    pImgBrowse->MenuBttn.ButtnRect.left = pImgBrowse->RotateBttn.ButtnRect.left;
    pImgBrowse->MenuBttn.ButtnRect.top = pImgBrowse->PrvBttn.ButtnRect.top - pImgBrowse->MenuBttn.ButtnRect.height
                                        - IMAGE_BUTTON_INTERVAL;

    /*up button and menu button are in the same position, 
            if in the emap mode, show up button, else show menu button 
        */
    pImgBrowse->UpBttn.pRes = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_UP, &len);
    AKBmpGetInfo(pImgBrowse->UpBttn.pRes, &pImgBrowse->UpBttn.ButtnRect.width, &pImgBrowse->UpBttn.ButtnRect.height, AK_NULL);
    pImgBrowse->UpBttn.ButtnRect.left = pImgBrowse->RotateBttn.ButtnRect.left;
    pImgBrowse->UpBttn.ButtnRect.top = pImgBrowse->PrvBttn.ButtnRect.top - pImgBrowse->UpBttn.ButtnRect.height
                                        - IMAGE_BUTTON_INTERVAL;

    
    //zoom in button
    pImgBrowse->ZoomInBttn.pRes = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_ZOOMIN, &len);
    AKBmpGetInfo(pImgBrowse->ZoomInBttn.pRes, &pImgBrowse->ZoomInBttn.ButtnRect.width, &pImgBrowse->ZoomInBttn.ButtnRect.height, AK_NULL);
    pImgBrowse->ZoomInBttn.ButtnRect.left = pImgBrowse->RotateBttn.ButtnRect.left;
    pImgBrowse->ZoomInBttn.ButtnRect.top = pImgBrowse->RotateBttn.ButtnRect.top + pImgBrowse->RotateBttn.ButtnRect.height
                                           + IMAGE_BUTTON_INTERVAL;
    //zoom out button
    pImgBrowse->ZoomOutBttn.pRes = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_ZOOMOUT, &len);
    AKBmpGetInfo(pImgBrowse->ZoomOutBttn.pRes, &pImgBrowse->ZoomOutBttn.ButtnRect.width, &pImgBrowse->ZoomOutBttn.ButtnRect.height, AK_NULL);
    pImgBrowse->ZoomOutBttn.ButtnRect.left = pImgBrowse->RotateBttn.ButtnRect.left;
    pImgBrowse->ZoomOutBttn.ButtnRect.top = pImgBrowse->ZoomInBttn.ButtnRect.top + pImgBrowse->ZoomInBttn.ButtnRect.height
                                           + IMAGE_BUTTON_INTERVAL;
    //return button
    pImgBrowse->ReturnBttn.pRes = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_IMAGE_BUTTON_RETURN, &len);
    AKBmpGetInfo(pImgBrowse->ReturnBttn.pRes, &pImgBrowse->ReturnBttn.ButtnRect.width, &pImgBrowse->ReturnBttn.ButtnRect.height, AK_NULL);
    pImgBrowse->ReturnBttn.ButtnRect.left = pImgBrowse->RotateBttn.ButtnRect.left;
    pImgBrowse->ReturnBttn.ButtnRect.top = pImgBrowse->ZoomOutBttn.ButtnRect.top + pImgBrowse->ZoomOutBttn.ButtnRect.height
                                           + IMAGE_BUTTON_INTERVAL;
//#endif
}

static T_VOID ImgBrowse_ShowToolButtons(T_IMGBROWSE * pImgBrowse)
{
	if (IMG_MAP != ImgBrowse_GetDisMode(pImgBrowse))
    {
		return;
	}
	
	//toolbar background rectangle 
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pImgBrowse->BkgdRect.ButtnRect.left,
                        pImgBrowse->BkgdRect.ButtnRect.top, pImgBrowse->BkgdRect.pRes,
                        AK_NULL, AK_FALSE);

    //up button icon
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pImgBrowse->UpBttn.ButtnRect.left,
                        pImgBrowse->UpBttn.ButtnRect.top, pImgBrowse->UpBttn.pRes,
                        AK_NULL, AK_FALSE);
	//next button icon
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pImgBrowse->NextBttn.ButtnRect.left,
                        pImgBrowse->NextBttn.ButtnRect.top, pImgBrowse->NextBttn.pRes, 
                        AK_NULL, AK_FALSE);

    //previous button
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pImgBrowse->PrvBttn.ButtnRect.left,
                        pImgBrowse->PrvBttn.ButtnRect.top, pImgBrowse->PrvBttn.pRes, 
                        AK_NULL, AK_FALSE);
	//zoom in button
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pImgBrowse->ZoomInBttn.ButtnRect.left, 
                        pImgBrowse->ZoomInBttn.ButtnRect.top, pImgBrowse->ZoomInBttn.pRes, 
                        AK_NULL, AK_FALSE);

    //zoom out button
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pImgBrowse->ZoomOutBttn.ButtnRect.left, 
                        pImgBrowse->ZoomOutBttn.ButtnRect.top, pImgBrowse->ZoomOutBttn.pRes, 
                        AK_NULL, AK_FALSE);
    //down button
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pImgBrowse->DownBttn.ButtnRect.left, 
                        pImgBrowse->DownBttn.ButtnRect.top, pImgBrowse->DownBttn.pRes,
                        AK_NULL, AK_FALSE);

    //return button
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pImgBrowse->ReturnBttn.ButtnRect.left,
                        pImgBrowse->ReturnBttn.ButtnRect.top, pImgBrowse->ReturnBttn.pRes, 
                        AK_NULL, AK_FALSE);


}

T_U32  ImgBrowse_GetZoom(T_IMGBROWSE * pImgBrowse)
{
    if (AK_NULL == pImgBrowse)
    {
        return 0;
    }

    return pImgBrowse->zoom;
}

T_U32  ImgBrowse_GetScale(T_IMGBROWSE * pImgBrowse)
{
    if (AK_NULL == pImgBrowse)
    {
        return 0;
    }

    return pImgBrowse->scale;
}

T_U32  ImgBrowse_GetOffsetX(T_IMGBROWSE * pImgBrowse)
{
    if (AK_NULL == pImgBrowse)
    {
        return 0;
    }

    return pImgBrowse->offsetX;
}

T_U32  ImgBrowse_GetOffsetY(T_IMGBROWSE * pImgBrowse)
{
    if (AK_NULL == pImgBrowse)
    {
        return 0;
    }

    return pImgBrowse->offsetY;
}

T_BOOL ImgBrowse_SetZoomChangValue(T_IMGBROWSE * pImgBrowse, T_U32 changeValue)
{
    AK_ASSERT_PTR(pImgBrowse, "ImgBrowse_SetZoomChangValue(): pImgBrowse == AK_NULL", AK_FALSE);

    pImgBrowse->ZoomChangeV = changeValue;

    return AK_TRUE;
}

T_U32 ImgBrowse_GetZoomChangValue(T_IMGBROWSE * pImgBrowse)
{
    AK_ASSERT_PTR(pImgBrowse, "ImgBrowse_GetZoomChangValue(): pImgBrowse == AK_NULL", 0);

    return pImgBrowse->ZoomChangeV;
}

T_BOOL ImgBrowse_SetOffsetChangValue(T_IMGBROWSE * pImgBrowse, T_U32 changeValue)
{
    AK_ASSERT_PTR(pImgBrowse, "ImgBrowse_SetOffsetChangValue(): pImgBrowse == AK_NULL", AK_FALSE);

    pImgBrowse->OffsetChangeV = changeValue;

    return AK_TRUE;
}

T_U32 ImgBrowse_GetOffsetChangValue(T_IMGBROWSE * pImgBrowse, T_U32 changeValue)
{
    AK_ASSERT_PTR(pImgBrowse, "ImgBrowse_GetOffsetChangValue(): pImgBrowse == AK_NULL", 0);

    return pImgBrowse->OffsetChangeV;
}



#endif
/*end of file*/
