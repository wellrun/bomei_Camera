/**@file ImageLayer.h
 * @brief Define the various common data types.
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Microelectronics Technology Co., Ltd.
 * @author xie wenzhong
 * @date 2010-06-29
 * @version 0.1
 */
 
#ifndef _IMAGE_LAYER_H_
#define _IMAGE_LAYER_H_

#include "Anyka_types.h"
#include "Gbl_macrodef.h"
#include "eng_AkBmp.h"
#include "Fwl_graphic.h"
#include "Fwl_font.h"
#include "Fwl_display.h"


typedef struct _ImageLayer{
	T_U8 		*pData;
	T_RECT 		area;		// The left and top Member of area is Relative to The Top Left Corner of Monitor.
	T_S8 		alpha;		// Transparency, From 0 to 100, "0" is Opaque, "100" is FULL Transparent.
	T_S8 		format;		// Image Layer Format, RGB888/RGB565/YUV420 etc.
	T_S16		colFilt;	// Color Filter, "0" is NOT Filtered
	T_BOOL		bUpdate;	
	T_BOOL		bDataValid;

	struct _ImageLayer 	*next;
	struct _ImageLayer 	*prev;
	T_S8				tag;
	
} T_ImgLay, *T_pImgLay;




/******************************************************************************/

typedef struct _ImgLayerList{
	T_pImgLay 	head;
	T_U8 		count;
	
}T_ImgLayList, *T_pImgLayList;



/******************    Image Layer List Access   *****************************/

// This Have Implicit Variable of Module(T_pImgLayList layerList = AK_NULL;)
T_eImgLayRet 	ImgLay_ListInit(T_VOID);

T_eImgLayRet 	ImgLay_ListDestroy(T_VOID);

T_eImgLayRet 	ImgLay_AddLayer(T_pImgLay layer);

T_pImgLay		ImgLay_DelLayer(T_pImgLay layer);

T_U8			ImgLay_Amount(T_VOID);

T_pImgLay 		ImgLay_GetFirstLayer(T_VOID);
T_pImgLay 		ImgLay_GetNextLayer(T_pImgLay layer);

/******************* Image Layer Member Access *****************************/

T_pImgLay 		ImgLay_New(T_S8 tag, T_RECT area, T_S8 format, T_BOOL dataValid,
							T_S8 alpha, T_S16 filter);
T_pVOID 		ImgLay_Free(T_pImgLay layer);

T_S8			ImgLay_Format(T_pImgLay layer);

T_eImgLayRet 	ImgLay_SetBuf(T_pImgLay layer, T_VOID* buffer);
T_pVOID 	 	ImgLay_GetBuf(T_pImgLay layer);
T_eImgLayRet 	ImgLay_FreeBuf(T_pImgLay layer);
T_eImgLayRet 	ImgLay_CleanBuf(T_pImgLay layer);
T_eImgLayRet 	ImgLay_ResetBuf(T_pImgLay layer);

T_eImgLayRet 	ImgLay_SetFilter(T_pImgLay layer, T_S16 filter);
T_S16			ImgLay_Filter(T_pImgLay layer);
T_BOOL			ImgLay_Update(T_pImgLay layer);
T_eImgLayRet	ImgLay_Consumed(T_pImgLay layer);


T_eImgLayRet 	ImgLay_SetArea(T_pImgLay layer, T_RECT area);
T_pRECT 		ImgLay_GetArea(T_pImgLay layer);


T_eImgLayRet 	ImgLay_SetTag(T_pImgLay layer, T_S8 tag);
T_S8 			ImgLay_GetTag(T_pImgLay layer);
T_pImgLay 		ImgLay_LayerByTag(T_S8 tag);


T_eImgLayRet 	ImgLay_SetAlpha(T_pImgLay layer, T_S8 alpha);
T_S8 			ImgLay_GetAlpha(T_pImgLay layer);




T_pImgLay 		ImgLay_Duplicate(T_pImgLay *ppLayer, T_pImgLay srcLayer);

/******************************************************************************/

T_BOOL ImgLay_DrawLine(T_pImgLay layer, T_POINT p1, T_POINT p2, 
				T_COLOR color);
				
T_BOOL ImgLay_DrawCycle(T_pImgLay layer, T_POINT center, T_U16 radius,
				T_COLOR color);

T_BOOL ImgLay_DrawRect(T_pImgLay layer, T_RECT area, T_COLOR color);

T_BOOL ImgLay_DrawRadio(T_pImgLay layer, T_POINT point, T_LEN radius, 
	T_BOOL focus, T_COLOR color);
				
T_BOOL ImgLay_DrawRectWithRect(T_pImgLay layer, T_RECT area,
				T_COLOR color);
				
T_BOOL ImgLay_DrawRectWithPoint(T_pImgLay layer, T_POINT p1, T_POINT p2,
				T_COLOR color);
				
T_BOOL ImgLay_DrawArcThreePoint(T_pImgLay layer, T_POINT p1, T_POINT p2, T_POINT p3,
				T_COLOR color);
				
T_BOOL ImgLay_DrawArcRadius(T_pImgLay layer, T_POINT p1, T_POINT p2, T_U16 radius,
				T_COLOR color);
				
T_BOOL ImgLay_DrawArcCenter(T_pImgLay layer, T_POINT p1, T_POINT p2, T_POINT center,
				T_COLOR color);

/******************************************************************************/

T_BOOL ImgLay_FillCycle(T_pImgLay layer, T_POINT p, T_U16 radius, T_COLOR color);
T_BOOL ImgLay_FillRect(T_pImgLay layer, T_RECT area, T_COLOR color);
T_BOOL ImgLay_FillSector(T_pImgLay layer, T_POINT center, T_U16 radius, T_POINT p1, T_POINT p2,
				T_COLOR color);
/******************************************************************************/

T_COLOR 	 ImgLay_GetPixel(T_pImgLay layer, T_POINT point);
T_BOOL		 ImgLay_SetPixel(T_pImgLay layer, T_POINT point, T_COLOR color);

T_eImgLayRet ImgLay_LoadImg(T_pImgLay layer, T_eORIENT orient, T_USTR_FILE path);

T_eImgLayRet ImgLay_RefreshFromImg(T_pImgLay layer, T_eORIENT orient, 
							const T_U8 *pBuf, T_U16 src_w, T_U16 src_h, T_U8 format);

T_eImgLayRet ImgLay_RefreshRect(T_pImgLay layer, T_RECT dstArea,
							const T_U8 *pBuf, T_U16 src_w, T_U16 src_h, T_U8 format);

T_pVOID 	 ImgLay_GetRect(T_pImgLay layer, T_RECT area, T_U8 **ppBuf, T_U8 foramt);

/******************************************************************************/

T_eImgLayRet ImgLay_ZoomInternal(T_pImgLay layer, T_RECT dstArea, T_RECT srcArea, T_BOOL bResvOld);
T_eImgLayRet ImgLay_Zoom(T_pImgLay dstLayer, T_RECT dstArea, T_pImgLay srcLayer, T_RECT srcArea);

T_eImgLayRet ImgLay_CopyAll(T_pImgLay dstLayer, T_pImgLay srcLayer);
T_eImgLayRet ImgLay_MoveAll(T_pImgLay dstLayer, T_pImgLay srcLayer);


/********************************  Refresh Image  *********************************/
T_BOOL ImgLay_AkBmpDrawPart(T_pImgLay layer, T_POINT point, T_RECT *range,
				const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse);
				
T_BOOL ImgLay_AkBmpDrawPartFromString(T_pImgLay layer, T_POINT point, T_RECT *range,
				T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse);
				
T_BOOL ImgLay_AkBmpDrawFromString(T_pImgLay layer, T_POINT point, T_pCDATA BmpStream,
				T_COLOR *bkColor, T_BOOL Reverse);

T_BOOL ImgLay_AkBmpDrawAlphaFromString(T_pImgLay layer, T_POINT point, T_pCDATA BmpStream,
				T_COLOR *bkColor, T_BOOL Reverse);



/********************************  Font Access  ***********************************/
T_BOOL ImgLay_UDispSpeciString(T_pImgLay layer, T_POINT point, T_U16* disp_string,
				T_COLOR color, T_FONT font, T_U16 strLen );
				
T_BOOL ImgLay_DispString(T_pImgLay layer, T_POINT point, T_pCSTR string, T_U16 strLen,
				T_COLOR color, T_FONT font);
				
T_BOOL ImgLay_UScrollDispString(T_pImgLay layer, T_POINT point, T_U16* pUString, T_U16 UStrLen,
				T_U16 offset, T_U16 width_limit, T_COLOR color, T_FONT font);

#endif
