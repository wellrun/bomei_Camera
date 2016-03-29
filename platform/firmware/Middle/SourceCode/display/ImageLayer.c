#include "anyka_types.h"
#include "Eng_Debug.h"
#include "Eng_String.h"
#include "Fwl_osMalloc.h"

#include "fwl_graphic.h"
#include "fwl_font.h"

#include "imageLayer.h"



static T_pImgLayList layerList = AK_NULL;


extern T_U8* Dev_GetFrameBuf (T_RECT *pstRectLay, T_U8 *format);

#if 0
static T_eImgLayRet ImgLay_RefreshData(T_ImgLay layer, T_RECT dstArea,
							const T_U8 *pBuf, T_U16 src_w, T_U16 src_h, T_U8 format);
static T_eImgLayRet ImgLay_RefreshParent(T_pImgLay layer, T_RECT dstArea,
							const T_U8 *pBuf, T_U16 src_w, T_U16 src_h, T_U8 format);
#endif
static T_BOOL ImgLay_ValidBufLayer(T_pImgLay searchLayer, T_RECT area, T_pImgLay retLayer);
static T_BOOL ImgLay_RangeValid(T_RECT dstArea, T_RECT srcArea);

static T_VOID ImgLay_RGB888to565Line(T_U8 *pBuffer565, T_U8 *pBuffer888, T_U32 width);
static T_VOID ImgLay_RGB565to888Line(T_U8 *pBuffer888, T_U8 *pBuffer565, T_U32 width);
static T_U8 ImgLay_CalcBitWidth(T_U8 format);


#if 0
static T_BOOL ImgLay_InitTvBuf(T_VOID)
{
	if (AK_NULL == tv)
	{
		tv = Fwl_Malloc(sizeof(T_ImgLay));
		AK_ASSERT_PTR(tv, "ImgLay_ValidBufLayer(): Malloc Failure!\n", AK_FALSE);

		tv->alpha = 0;
		tv->bDadaValid = AK_TRUE;
		tv->bUpdate = AK_FALSE;
		tv->colFilt = 0;
		tv->format = RGB565;
		tv->next = AK_NULL;
		tv->prev = AK_NULL;
		tv->tag = 0x0F;
	}
	return AK_TRUE;
}

#endif

/******************************************************************************/

#define ImgLay_CalcBufSize(layer) \
	( (layer)->area.width * (layer)->area.height * ImgLay_CalcBitWidth((layer)->format) >> 3 )

#define ImgLay_CalcYUVAddr(pBuf,width,height) \
	do{ \
		ybuf = (pBuf); \
		ubuf = (pBuf) + (width)*(height); \
		vbuf = (pBuf) + (width)*(height) + ((width)*(height) >> 2); \
	}while(AK_FALSE)

/******************************************************************************/


T_eImgLayRet ImgLay_SetTag(T_pImgLay item, T_S8 tag)
{	
	AK_ASSERT_PTR(item, "ImgLay_SetTag(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);

	item->tag = tag;
	
	return IMGLAY_OK;
}

T_S8 ImgLay_GetTag(T_pImgLay item)
{
	AK_ASSERT_PTR(item, "ImgLay_GetTag(): Input Parameter Is Error!\n", -1);

	return item->tag;
}


T_pImgLay ImgLay_LayerByTag(T_S8 tag)
{
	T_pImgLay p = AK_NULL;
		
	AK_ASSERT_PTR(layerList, "ImgLay_LayerByTag(): \"layerList\" IS NOT Initialization!\n", 0x0);
		
	p = layerList->head;

	if (p != AK_NULL)
	{
		while(p->tag != tag && p->next != layerList->head)
		{
			p = p->next;
		}
		
		// Find node
		if (p->tag == tag)
		{
			return p;
		}		
	}
	
	return AK_NULL;

}


//wgtbupt
T_eImgLayRet 	ImgLay_SetAlpha(T_pImgLay layer, T_S8 alpha)
{	
	AK_ASSERT_PTR(layer, "ImgLay_SetAlpha(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);

	if((alpha<0) || (alpha>100))
	{
		alpha = 100;
	}

	layer->alpha = alpha;
	
	return IMGLAY_OK;
}

T_S8 			ImgLay_GetAlpha(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_GetAlpha(): Input Parameter Is Error!\n", -1);

	return layer->alpha;
}


/******************************************************************************/

// This Have Implicit Variable of Module(T_pImgLayList layerList = AK_NULL;)
T_eImgLayRet ImgLay_ListInit(T_VOID)
{
	if (layerList == AK_NULL)
	{
		layerList = Fwl_Malloc(sizeof(T_ImgLayList));
		AK_ASSERT_PTR(layerList, "ImgLay_ListInit():	Malloc Failure!\n", IMGLAY_MEM_ERR);
		
		layerList->count = 0;
		layerList->head = AK_NULL;
	}
	
	return IMGLAY_OK;
}

T_eImgLayRet ImgLay_ListDestroy(T_VOID)
{
	T_pImgLay p, q;
	
	AK_ASSERT_PTR(layerList, "ImgLay_ListDestroy(): layerList IS NOT Initialization!\n", IMGLAY_MEM_ERR);
	
	p = layerList->head;
	
	if (p != AK_NULL)
	{
		while (p->next != p)
		{
			q = p->next;
			ImgLay_Free(ImgLay_DelLayer(p));
			p = q;

		}
		// Remove Last One
		ImgLay_Free(ImgLay_DelLayer(p));
	}

	return IMGLAY_OK;
}

T_eImgLayRet ImgLay_AddLayer(T_pImgLay layer)
{
	T_pImgLay p;
	
	AK_ASSERT_PTR(layerList, "ImgLay_ListAddItem(): layerList IS NOT Initialization!\n", IMGLAY_MEM_ERR);
	
	p = layerList->head;
	if (p != AK_NULL)
	{
		while(p->next != layerList->head)
		{
			p = p->next;
		}

		layer->next = p->next;
		p->next->prev = layer;
		p->next = layer;
		layer->prev = p;
	}
	else
	{
		layerList->head = layer;
		layer->next = layer->prev = layer;
	}
	
	layerList->count++;
	return IMGLAY_OK;
}

T_pImgLay ImgLay_DelLayer(T_pImgLay layer)
{
	T_pImgLay p;
		
	AK_ASSERT_PTR(layerList, "ImgLay_ListDelItem(): \"layerList\" IS NOT Initialization!\n", 0x0);
		
	p = layerList->head;

	if (p != AK_NULL)
	{
		while(p != layer && p->next != layerList->head)
		{
			p = p->next;
		}
		
		// Find node
		if (p == layer)
		{
			// Only One Node.
			if (p == p->next)
			{
				layerList->head = AK_NULL;
			}
			else
			{
				if (p == layerList->head)
				{
					layerList->head = p->next;
				}
				
				p->prev->next = p->next;
				p->next->prev = p->prev;
			}
			layerList->count--;
		}		
	}
	
	return p;
	
}

T_U8 ImgLay_Amount(T_VOID)
{
	AK_ASSERT_PTR(layerList, "ImgLay_Amount(): \"layerList\" IS NOT Initialization!\n", 0x0);

	return layerList->count;
}

T_pImgLay ImgLay_GetFirstLayer(T_VOID)
{
	AK_ASSERT_PTR(layerList, "ImgLay_GetFirstLayer(): \"layerList\" IS NOT Initialization!\n", 0x0);
	return layerList->head;
}

T_pImgLay ImgLay_GetNextLayer(T_pImgLay layer)
{
	AK_ASSERT_PTR(layerList, "ImgLay_GetFirstLayer(): \"layerList\" IS NOT Initialization!\n", 0x0);
	AK_ASSERT_PTR(layer, "ImgLay_GetNextLayer(): Input Parameter Is Error!\n", 0x0);
	
	if (layer->next != layerList->head)
	{
		return layer->next;
	}

	return AK_NULL;
}


/******************************************************************************/

T_eImgLayRet ImgLay_ResetBuf(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_SetBuf(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	
	if (layer->bDataValid)
	{
		layer->pData = Fwl_Malloc(ImgLay_CalcBufSize(layer));
		AK_ASSERT_PTR(layer->pData, "ImgLay_ResetBuf(): Malloc Buffer Failure!\n", IMGLAY_MEM_ERR);
		ImgLay_CleanBuf(layer);
	}
	
	return IMGLAY_OK;
}

T_pImgLay ImgLay_Duplicate(T_pImgLay *ppLayer, T_pImgLay srcLayer)
{
	T_pImgLay layer = AK_NULL;
	
	AK_ASSERT_PTR(srcLayer, "ImgLay_Duplicate(): Input Parameter Is Error!\n", 0x0);

	if (ppLayer == AK_NULL)
	{
		ppLayer = &layer;
	}	

	*ppLayer = Fwl_Malloc(sizeof(T_ImgLay));
	AK_ASSERT_PTR(*ppLayer, "ImgLay_Duplicate(): Malloc ERROR!\n", 0x0);
	
	(*ppLayer)->pData = Fwl_Malloc(ImgLay_CalcBufSize(*ppLayer));
	AK_ASSERT_PTR((*ppLayer)->pData, "ImgLay_Duplicate(): Malloc Buffer False!\n", 0x0);
	
	Utl_MemCpy((*ppLayer)->pData, srcLayer->pData, ImgLay_CalcBufSize(srcLayer));
	
	return *ppLayer;
}

T_pImgLay ImgLay_New(T_S8 tag, T_RECT area, T_S8 format, T_BOOL dataValid, T_S8 alpha, T_S16 filter)
{	
	T_pImgLay layer = Fwl_Malloc(sizeof(T_ImgLay));
	
	AK_ASSERT_PTR(layer, "ImgLay_New(): Malloc Failure!\n", 0x0);

	layer->tag 		= tag;
	layer->area 	= area;
	layer->format 	= format;
	layer->bDataValid = dataValid;
	layer->alpha 	= alpha;
	layer->colFilt 	= filter;
	layer->bUpdate 	= AK_FALSE;
	layer->prev 	= AK_NULL;
	layer->next 	= AK_NULL;
	
	if (layer->bDataValid)
	{
		// Allocate Memory
		ImgLay_ResetBuf(layer);		
	}
	else 
	{
		layer->pData = AK_NULL;
	}	

	return layer;
}

T_pVOID ImgLay_Free(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_Free(): Input Parameter Is Error!\n", 0x0);
	
	if (layer->bDataValid && layer->pData != AK_NULL)
	{
		layer->pData = Fwl_Free(layer->pData);
	}
	
	return Fwl_Free(layer);
}

T_S8 ImgLay_Format(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_Format(): Input Parameter Is Error!\n", -1);
	return layer->format;
}

T_eImgLayRet ImgLay_SetBuf(T_pImgLay layer, T_VOID* buffer)
{
	AK_ASSERT_PTR(layer, "ImgLay_SetBuf(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);

	if (layer->bDataValid)
	{
		layer->pData = buffer;
		return IMGLAY_OK;
	}
	else
	{
		return IMGLAY_DATA_INVALID_ERR;
	}	
}

T_pVOID ImgLay_GetBuf(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_GetBuf(): Input Parameter Is Error!\n", 0x0);

	if (layer->bDataValid)
	{
		return layer->pData;
	}
	else
	{
		return AK_NULL;
	}	
}

T_eImgLayRet ImgLay_FreeBuf(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_FreeBuf(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);

	if (layer->bDataValid)
	{
		layer->pData = Fwl_Free(layer->pData);
		return IMGLAY_OK;
	}
	else
	{
		return IMGLAY_DATA_INVALID_ERR;
	}
}

T_eImgLayRet ImgLay_CleanBuf(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_CleanBuf(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	
	if (layer->bDataValid)
	{
		Utl_MemSet(layer->pData, 0x0, ImgLay_CalcBufSize(layer));
		return IMGLAY_OK;		
	}
	else 
	{
		return IMGLAY_ERROR;
	}
}

T_eImgLayRet ImgLay_SetFilter(T_pImgLay layer, T_S16 filter)
{
	AK_ASSERT_PTR(layer, "ImgLay_SetFilter(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);

	layer->colFilt = filter;
	
	return IMGLAY_OK;
}

T_S16 ImgLay_Filter(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_Filter(): Input Parameter Is Error!\n", -1);

	return layer->colFilt;
}

T_BOOL ImgLay_Update(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_Update(): Input Parameter Is Error!\n", AK_FALSE);
	
	return layer->bUpdate;
}

T_eImgLayRet	ImgLay_Consumed(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_Consumed(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	
	layer->bUpdate = AK_FALSE;
	
	return IMGLAY_OK;
}

T_eImgLayRet ImgLay_SetArea(T_pImgLay layer, T_RECT area)
{
	AK_ASSERT_PTR(layer, "ImgLay_SetArea(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	
	layer->area = area;
	
	return IMGLAY_OK;
}

T_pRECT ImgLay_GetArea(T_pImgLay layer)
{
	AK_ASSERT_PTR(layer, "ImgLay_GetArea(): Input Parameter Is Error!\n", 0x0);
	
	return &layer->area;
}

T_BOOL ImgLay_DrawLine(T_pImgLay layer, T_POINT p1, T_POINT p2, T_COLOR color)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_DrawLine(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
	
	switch (layer->format)
	{
	case RGB888:
	case RGB565:		
		Fwl_DrawLineOnRGB(layer->pData, layer->area.width, layer->area.height,\
				p1.x, p1.y, p2.x, p2.y, color, layer->format);
		break;
		
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);
		
		Fwl_DrawLineOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				p1.x, p1.y, p2.x, p2.y, color);
		break;
		
	case YUV422:	
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;
}

T_BOOL ImgLay_DrawCycle(T_pImgLay layer, T_POINT center, T_U16 radius, T_COLOR color)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_DrawCycle(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		Fwl_DrawCircleOnRGB(layer->pData, layer->area.width, layer->area.height,\
				center.x, center.y, radius, color, layer->format);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);
		
		Fwl_DrawCircleOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				center.x, center.y, radius, color);
		break;
			
	case YUV422:	
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;
}

T_BOOL ImgLay_DrawRect(T_pImgLay layer, T_RECT area, T_COLOR color)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_DrawCycle(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		Fwl_DrawRectOnRGB(layer->pData, layer->area.width, layer->area.height,\
				&area, color, layer->format);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);
		
		Fwl_DrawRectOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				area.left, area.top, (T_POS)(area.left+area.width), (T_POS)(area.top+area.height), 
				color);
		break;
			
	case YUV422:	
	default:
		return AK_FALSE;
	}

	return AK_TRUE;
}

T_BOOL ImgLay_DrawArcThreePoint(T_pImgLay layer, T_POINT p1, T_POINT p2, T_POINT p3, T_COLOR color)
{
	return IMGLAY_ERROR;
}

T_BOOL ImgLay_DrawArcRadius(T_pImgLay layer, T_POINT p1, T_POINT p2, T_U16 radius, T_COLOR color)
{
	return IMGLAY_ERROR;
}

T_BOOL ImgLay_DrawArcCenter(T_pImgLay layer, T_POINT p1, T_POINT p2, T_POINT center, T_COLOR color)
{
	return IMGLAY_ERROR;
}

T_BOOL ImgLay_DrawRadio(T_pImgLay layer, T_POINT point, T_LEN radius, T_BOOL focus, T_COLOR color)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_DrawRadio(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		Fwl_DrawRadioOnRGB(layer->pData, layer->area.width, layer->area.height,\
				point.x, point.y, radius, focus, color, layer->format);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);
		
		Fwl_DrawRadioOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				point.x, point.y, radius, focus, color);
		break;
			
	case YUV422:	
	default:
		return AK_FALSE;
	}

	return AK_TRUE;
}

T_BOOL ImgLay_FillCycle(T_pImgLay layer, T_POINT p, T_U16 radius, T_COLOR color)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_FillCycle(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		Fwl_DrawDiskOnRGB(layer->pData, layer->area.width, layer->area.height,\
				p.x, p.y, radius, color, layer->format);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);
		
		Fwl_DrawDiskOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				p.x, p.y, radius, color);
		break;
			
	case YUV422:	
	default:
		return AK_FALSE;
	}

	return AK_TRUE;
}

T_BOOL ImgLay_FillRect(T_pImgLay layer, T_RECT area, T_COLOR color)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_FillRect(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		Fwl_FillSolidRectOnRGB(layer->pData, layer->area.width, layer->area.height,\
				&area, color, layer->format);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);
		
		Fwl_FillSolidRectOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				&area, color);
		break;
				
	case YUV422:	
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;
}

T_BOOL ImgLay_FillSector(T_pImgLay layer, T_POINT center, T_U16 radius, T_POINT p1, T_POINT p2, T_COLOR color)
{
/*
	T_U8 *ybuf, *ubuf, *vbuf;
	AK_ASSERT_PTR(layer, "ImgLay_SetPixel(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	
	if (!layer->bDataValid)
	{
		return IMGLAY_DATA_INVALID_ERR;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		Fwl_SetPixelOnRGB(layer->pData, layer->area.width, layer->area.height, point.x, point.y, color, layer->format);
		break;
			
	case YUV420:
		ybuf = layer->pData;
		ubuf = ybuf + layer->area.width*layer->area.height;
		vbuf = ubuf + (layer->area.width*layer->area.height)>>2;
		Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height, point.x, point.y, color);
		break;
				
	default:
		return IMGLAY_ERROR;
	}
	
	return IMGLAY_OK;
*/
	return AK_FALSE;
}

T_COLOR ImgLay_GetPixel(T_pImgLay layer, T_POINT point)
{
	//T_U8 *ybuf, *ubuf, *vbuf;

	AK_ASSERT_PTR(layer, "ImgLay_GetPixel(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	
	if (!layer->bDataValid)
	{
		return 0x0;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		return Fwl_GetPixelOnRGB(layer->pData, layer->area.width, layer->area.height,\
					point.x, point.y, layer->format);
		break;
			
	case YUV420:
/*		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);
		
		return Fwl_GetPixelOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
					point.x, point.y);
		break;
*/		
	case YUV422:	
	default:
		return 0x0;
	}
	
	return 0x0;
}

T_BOOL ImgLay_SetPixel(T_pImgLay layer, T_POINT point, T_COLOR color)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_SetPixel(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		return Fwl_SetPixelOnRGB(layer->pData, layer->area.width, layer->area.height,\
				point.x, point.y, color, layer->format);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);	
		
		Fwl_SetPixelOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				point.x, point.y, color);
		break;
				
	case YUV422:	
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;
}

T_eImgLayRet ImgLay_RefreshRect(T_pImgLay layer, T_RECT dstArea,
							const T_U8 *pBuf, T_U16 src_w, T_U16 src_h, T_U8 format)
{	
	T_ImgLay imgLay;
	T_U8 *ybuf, *ubuf, *vbuf;

	AK_ASSERT_PTR(layer, "ImgLay_RefreshRect(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);

	if (layer->area.width < dstArea.left+dstArea.width
		|| layer->area.height < dstArea.top+dstArea.height)
	{
		Fwl_Print(C3, M_DISPLAY,"IMG_LAY:	Destination Area Is OUT RANGE!\n");
		return IMGLAY_OUT_RANGE | IMGLAY_ERROR;
	}
	
	// Copy Data to Current Layer
	if (layer->bDataValid)
	{
		imgLay = *layer;
	}
	// Find Valid Layer of Buffer
	else if (!ImgLay_ValidBufLayer(layer->prev, layer->area, &imgLay))
	{
		Fwl_Print(C3, M_DISPLAY,"IMG_LAY:	Can NOT Retrieve Valid Layer!\n");

		return IMGLAY_ERROR;
	}

	// Calculate Origin Offset
	dstArea.left += imgLay.area.left - layer->area.left;
	dstArea.top += imgLay.area.top - layer->area.top;
	
	if (format == RGB888 || format == RGB565)
	{
		Fwl_Img_BitBltRGB(imgLay.pData, imgLay.format, dstArea.width, dstArea.height,
			dstArea.left, dstArea.top, imgLay.area.width, (T_U8*)pBuf, src_w, src_h, format);		
	}
	else if (format == YUV420 )
	{
		ImgLay_CalcYUVAddr((T_U8*)pBuf, src_w, src_h);
		Fwl_Img_BitBltYUV(imgLay.pData, imgLay.format, dstArea.width, dstArea.height,
			dstArea.left, dstArea.top, imgLay.area.width, ybuf, ubuf, vbuf, src_w, src_h);		
	}
	else
	{
		Fwl_Print(C3, M_DISPLAY,"IMG_LAY:	Source Data Format %d is Nonsupport!\n", format);
		return IMGLAY_FORMAT_ERR;
	}
	
	layer->bUpdate = AK_TRUE; // Maybe It is Invalid to Get Data to Displayer Memory
	return IMGLAY_OK;
}

T_eImgLayRet ImgLay_RefreshFromImg(T_pImgLay layer, T_eORIENT orient, 
						const T_U8 *pBuf, T_U16 src_w, T_U16 src_h, T_U8 format)
{
	T_RECT dstArea;
	
	AK_ASSERT_PTR(layer, "ImgLay_RefreshFromImg(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);

	if (orient & ORIENT_FULL)
	{
		dstArea.left = 0;
		dstArea.top = 0;
		dstArea.width = layer->area.width;
		dstArea.height = layer->area.height;
	}
	else if (orient & ORIENT_CENTRAL)
	{
		// Image size < Layer size
		if (src_w <= layer->area.width && src_h <= layer->area.height)
		{
			dstArea.left = (layer->area.width-src_w)>>0x1;
			dstArea.top = (layer->area.height-src_h)>>0x1;
			dstArea.width = src_w;
			dstArea.height = src_h;			
		}
		// src_w > src_h, Keep aspect ratio
		else if ( src_w*layer->area.height >= src_h*layer->area.width )
		{
			dstArea.left = 0;
			dstArea.width = layer->area.width;
			dstArea.height = layer->area.width * src_h / src_w;
			dstArea.top = (src_h - dstArea.height)>>0x1;
		}
		// src_w < src_h, Keep aspect ratio
		else 
		{
			dstArea.top = 0;
			dstArea.height = layer->area.height;
			dstArea.width = layer->area.height * src_w / src_h;
			dstArea.left = (src_w - dstArea.width)>>0x1;
		}
		
	}
	else
	{
		if (orient & ORIENT_LEFT)
		{
			dstArea.left = 0;
		}
		else if (orient & ORIENT_RIGHT)
		{
			dstArea.left = src_w>layer->area.width ? 0 : layer->area.width-src_w;
		}
		else
		{
			dstArea.left = src_w>layer->area.width ? 0 : (layer->area.width-src_w)>>1;
		}
		
		dstArea.width = src_w>layer->area.width ? layer->area.width : src_w;
		
		if (orient & ORIENT_TOP)
		{
			dstArea.top = 0;
		}
		else if (orient & ORIENT_BOTTM)
		{
			dstArea.top = src_h>layer->area.height ? 0 : layer->area.height-src_h;
		}
		else
		{
			dstArea.top = src_h>layer->area.height ? 0 : (layer->area.height-src_h)>>1;
		}
		
		dstArea.height= src_h>layer->area.height ? layer->area.height : src_h;
		
	}
	
	return ImgLay_RefreshRect(layer, dstArea, pBuf, src_w, src_h, format);
}

T_pVOID ImgLay_GetRect(T_pImgLay layer, T_RECT area, T_U8 **ppBuf, T_U8 format)
{
	T_U8 *pBuf = AK_NULL;
	T_U8 *p = AK_NULL;
	T_U8 *q = AK_NULL;
	T_U16 srcStep = 0;
	T_U16 dstStep = 0;
	T_U16 i;
	T_U8 *tmpBuf = AK_NULL;
	
	AK_ASSERT_PTR(layer, "ImgLay_GetRGBRect(): Input Parameter Is Error!\n", AK_NULL);

	if (ppBuf == AK_NULL)
	{
		ppBuf = &pBuf;
	}

	if (layer->format == YUV422 || format == YUV422)
	{
		Fwl_Print(C3, M_DISPLAY,"IMG_LAY:	The Format YUV422 is Unavailable.\n");
		return AK_NULL;
	}
	
	*ppBuf = Fwl_Malloc(area.left*area.height*ImgLay_CalcBitWidth(format));
	AK_ASSERT_PTR(*ppBuf, "ImgLay_GetRGBRect(): Malloc Failure!\n", AK_NULL);
	pBuf = *ppBuf;
	
	srcStep = layer->area.width * (ImgLay_CalcBitWidth(layer->format) >> 3);
	dstStep = area.width * (ImgLay_CalcBitWidth(format) >> 3);

	q = pBuf;
	p = layer->pData 
		+ (layer->area.width*area.top + area.left)*(ImgLay_CalcBitWidth(layer->format) >> 3);	

	// The Format is Same
	if (layer->format == format)
	{	
		// RGB
		if (format==RGB888 || format==RGB565)
		{
			for (i = 0; i < area.height; ++i)
			{
				Utl_MemCpy(q, p, dstStep);
				p += srcStep;
				q += dstStep;
			}
		}
		// YUV
		else // if (format==YUV420 || format==YUV422)
		{
			pBuf = Fwl_Free(pBuf);
			Fwl_Print(C3, M_DISPLAY,"IMG_LAY:	The Format %d is Unavailable.\n", format);
			return AK_NULL;
		}
	}
	// From RGB888 to RGB565
	else if (layer->format == RGB888 && format == RGB565)
	{
		for (i = 0; i < area.height; ++i)
		{
			ImgLay_RGB888to565Line(q, p, area.width);
			p += srcStep;
			q += dstStep;
		}
	}
	// From RGB565 to RGB888
	else if (layer->format == RGB565 && format == RGB888)
	{
		for (i = 0; i < area.height; ++i)
		{
			ImgLay_RGB565to888Line(q, p, area.width);
			p += srcStep;
			q += dstStep;
		}
	}
	// From RGB888/565 to YUV420
	else if ((layer->format == RGB888 || layer->format == RGB565) && format == YUV420)
	{
		ImgLay_GetRect(layer, area, &tmpBuf, RGB565);
		Fwl_RGB565toYUV420(tmpBuf, pBuf, area.width, area.height);
		tmpBuf = Fwl_Free(tmpBuf);
	}
	else
	{
		pBuf = Fwl_Free(pBuf);
		Fwl_Print(C3, M_DISPLAY,"IMG_LAY:	The Source Format %d is Unavailable.\n", layer->format);
		return AK_NULL;
	}
	
	return pBuf;
}

T_eImgLayRet ImgLay_LoadImg(T_pImgLay layer, T_eORIENT orient, T_USTR_FILE path)
{
	AK_ASSERT_PTR(layer, "ImgLay_LoadImg(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	return IMGLAY_ERROR;
}

T_eImgLayRet ImgLay_ZoomInternal(T_pImgLay layer, T_RECT dstArea, T_RECT srcArea, T_BOOL bResvOld)
{
	T_U8 *srcBuf = AK_NULL;
	T_eImgLayRet retVal = IMGLAY_ERROR;
	
	AK_ASSERT_PTR(layer, "ImgLay_ZoomInternal(): Input Parameter Is Error!\n", IMGLAY_MEM_ERR);

	if (!layer->bDataValid)
	{
		return IMGLAY_DATA_INVALID_ERR;
	}

	if (AK_NULL != ImgLay_GetRect(layer, srcArea, &srcBuf, layer->format))
	{
		if (!bResvOld)
		{
			ImgLay_CleanBuf(layer);
		}
		
		retVal = ImgLay_RefreshRect(layer, dstArea, srcBuf, srcArea.width, srcArea.height, layer->format);
		srcBuf = Fwl_Free(srcBuf);
	}

	return retVal;
}

T_eImgLayRet ImgLay_Zoom(T_pImgLay dstLayer, T_RECT dstArea, T_pImgLay srcLayer, T_RECT srcArea)
{
	T_U8 *srcBuf = AK_NULL;
	T_eImgLayRet retVal = IMGLAY_ERROR;
	
	AK_ASSERT_PTR(dstLayer, "ImgLay_Zoom():	Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	AK_ASSERT_PTR(srcLayer, "ImgLay_Zoom():	Input Parameter Is Error!\n", IMGLAY_MEM_ERR);

	if (dstLayer == srcLayer)
	{
		Fwl_Print(C3, M_DISPLAY,"IMG_LAY:	Source Pointer EQUAL Destination Pointer!\n");
		return IMGLAY_DATA_SAME;
	}
	
	if (!srcLayer->bDataValid)
	{
		return IMGLAY_DATA_INVALID_ERR;
	}
	
	if (srcArea.left == 0 
		&& srcArea.top == 0
		&& srcArea.width == srcLayer->area.width)
	{
		srcBuf = srcLayer->pData;
		retVal = ImgLay_RefreshRect(dstLayer, dstArea, srcBuf, srcArea.width, srcArea.height, srcLayer->format);
	}
	else if (AK_NULL != ImgLay_GetRect(srcLayer, srcArea, &srcBuf, srcLayer->format))
	{
		retVal = ImgLay_RefreshRect(dstLayer, dstArea, srcBuf, srcArea.width, srcArea.height, srcLayer->format);
		srcBuf = Fwl_Free(srcBuf);
	}
	
	return retVal;
}

T_eImgLayRet ImgLay_CopyAll(T_pImgLay dstLayer, T_pImgLay srcLayer)
{
	T_RECT dstArea;
	
	AK_ASSERT_PTR(dstLayer, "ImgLay_CopyAll():	Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	AK_ASSERT_PTR(srcLayer, "ImgLay_CopyAll():	Input Parameter Is Error!\n", IMGLAY_MEM_ERR);

	if (dstLayer == srcLayer)
	{
		Fwl_Print(C3, M_DISPLAY,"IMG_LAY:	Source Pointer Is Equal Destination Pointer!\n");
		return IMGLAY_DATA_SAME;
	}
	
	if (!srcLayer->bDataValid)
	{
		return IMGLAY_DATA_INVALID_ERR;
	}
	
	dstArea = srcLayer->area;
	dstArea.left = dstLayer->area.left - srcLayer->area.left;
	dstArea.top = dstLayer->area.top - srcLayer->area.top;

	if (dstArea.left < 0 || dstArea.top < 0)
	{
		return (IMGLAY_OUT_RANGE | IMGLAY_ERROR);
	}

	return ImgLay_RefreshRect(dstLayer, dstArea, \
		srcLayer->pData, srcLayer->area.width, srcLayer->area.height, srcLayer->format);
		
	
}

T_eImgLayRet ImgLay_MoveAll(T_pImgLay dstLayer, T_pImgLay srcLayer)
{
	T_eImgLayRet retVal = IMGLAY_ERROR;
	
	AK_ASSERT_PTR(dstLayer, "ImgLay_MoveAll():	Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	AK_ASSERT_PTR(srcLayer, "ImgLay_MoveAll():	Input Parameter Is Error!\n", IMGLAY_MEM_ERR);
	
	if (dstLayer == srcLayer)
	{
		return IMGLAY_OK;
	}
	
	if ( IMGLAY_OK == (retVal = ImgLay_CopyAll(dstLayer, srcLayer)) )
	{	
		ImgLay_Free(srcLayer);
	}
	
	return retVal;
}

T_BOOL ImgLay_AkBmpDrawPart(T_pImgLay layer, T_POINT point, T_RECT *range,
				const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_AkBmpDrawPart(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		return Fwl_AkBmpDrawPartOnRGB(layer->pData, layer->area.width, layer->area.height,\
				point.x, point.y, range, AkBmp, bkColor, Reverse, layer->format);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);	
		
		return Fwl_AkBmpDrawPartOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				point.x, point.y, range, AkBmp, bkColor, Reverse);
		break;
		
	case YUV422:			
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;
		

}

T_BOOL ImgLay_AkBmpDrawPartFromString(T_pImgLay layer, T_POINT point, T_RECT *range,
				T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_AkBmpDrawPartFromString(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		return Fwl_AkBmpDrawPartFromStringOnRGB(layer->pData, layer->area.width, layer->area.height,\
				point.x, point.y, range, BmpStream, bkColor, Reverse, layer->format);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);	
		
		return Fwl_AkBmpDrawPartFromStringOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				point.x, point.y, range, BmpStream, bkColor, Reverse);
		break;
		
	case YUV422:			
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;


}

T_BOOL ImgLay_AkBmpDrawFromString(T_pImgLay layer, T_POINT point, T_pCDATA BmpStream,
				T_COLOR *bkColor, T_BOOL Reverse)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_AkBmpDrawFromString(): layer Parameter Is Error!\n", AK_FALSE);
	AK_ASSERT_PTR(BmpStream, "ImgLay_AkBmpDrawFromString(): BmpStream Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		return Fwl_AkBmpDrawFromStringOnRGB(layer->pData, layer->area.width, layer->area.height,\
				point.x, point.y, BmpStream, bkColor, Reverse, layer->format);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);	
		
		return Fwl_AkBmpDrawFromStringOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				point.x, point.y, BmpStream, bkColor, Reverse);
		break;

	case YUV422:	
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;
}

T_BOOL ImgLay_AkBmpDrawAlphaFromString(T_pImgLay layer, T_POINT point, T_pCDATA BmpStream,
				T_COLOR *bkColor, T_BOOL Reverse)
{
	AK_ASSERT_PTR(layer, "ImgLay_AkBmpDrawFromString(): layer Parameter Is Error!\n", AK_FALSE);
	AK_ASSERT_PTR(BmpStream, "ImgLay_AkBmpDrawFromString(): BmpStream Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		return Fwl_AkBmpDrawAlphaFromStringOnRGB(layer->pData, layer->area.width, layer->area.height,\
				point.x, point.y, BmpStream, bkColor, Reverse, layer->format);
		break;
	
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;
}



T_BOOL ImgLay_UDispSpeciString(T_pImgLay layer, T_POINT point, T_U16* disp_string,
				T_COLOR color, T_FONT font, T_U16 strLen )
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_UDispSpeciString(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		Fwl_UDispSpeciStringOnRGB(layer->pData, layer->area.width, layer->area.height,\
				point.x, point.y, disp_string, color, layer->format, font, strLen);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);		
		
		Fwl_UDispSpeciStringOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				point.x, point.y, disp_string, color, font, strLen);
		break;

	case YUV422:	
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;
}

T_BOOL ImgLay_DispString(T_pImgLay layer, T_POINT point, T_pCSTR string, T_U16 strLen,
				T_COLOR color, T_FONT font)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_DispString(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		Fwl_DispStringOnRGB(layer->pData, layer->area.width, layer->area.height,\
				point.x, point.y, string, strLen, color, layer->format, font);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);	
		
		Fwl_DispStringOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				point.x, point.y, string, strLen, color, font);
		break;

	case YUV422:	
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;
}

T_BOOL ImgLay_UScrollDispString(T_pImgLay layer, T_POINT point, T_U16* pUString, T_U16 UStrLen,
				T_U16 offset, T_U16 width_limit, T_COLOR color, T_FONT font)
{
	T_U8 *ybuf, *ubuf, *vbuf;
	
	AK_ASSERT_PTR(layer, "ImgLay_DispString(): Input Parameter Is Error!\n", AK_FALSE);
	
	if (!layer->bDataValid)
	{
		return AK_FALSE;
	}
		
	switch(layer->format)
	{
	case RGB888:
	case RGB565:
		return Fwl_UScrollDispStringOnRGB(layer->pData, layer->area.width, layer->area.height,\
				pUString, point.x, point.y, UStrLen, offset, width_limit, color, layer->format, font);
		break;
			
	case YUV420:
		ImgLay_CalcYUVAddr(layer->pData, layer->area.width, layer->area.height);
		
		return Fwl_UScrollDispStringOnYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height,\
				pUString, point.x, point.y, UStrLen, offset, width_limit, color, font);
		break;

	case YUV422:	
	default:
		return AK_FALSE;
	}
	
	return AK_TRUE;
}

/******************************************************************************/
static T_VOID ImgLay_RGB888to565Line(T_U8 *pBuffer565, T_U8 *pBuffer888, T_U32 width)
{
    T_U32 i,j=0;
    T_U32 length = width * 3;
    
    for (i=0; i<length; i+=3)
    {    
        pBuffer565[j++] = ((pBuffer888[i+1] & 0x1C) << 3) | ((pBuffer888[i+2] & 0xF8) >> 3);    // low 8 bit, G and B
        pBuffer565[j++] = (pBuffer888[i] & 0xF8) | ((pBuffer888[i+1] & 0xE0) >> 5);             // high 8 bit, R and G
    }
}

static T_VOID ImgLay_RGB565to888Line(T_U8 *pBuffer888, T_U8 *pBuffer565, T_U32 width)
{
    T_U32 k, dstPos = 0;
	T_U8  *p = pBuffer565;
    T_U16 temp;

    for(k = 0; k < width; k++)
    {
		temp = (*p) | (*(p+1)<<8);
		p+=2;

    	pBuffer888[dstPos++] = (T_U8)((temp>>11)<<3);
    	pBuffer888[dstPos++] = (T_U8)((temp>>5)<<2);
    	pBuffer888[dstPos++] = (T_U8)(temp<<3);
    }
}

static T_U8 ImgLay_CalcBitWidth(T_U8 format)
{
	switch(format)
	{
		case YUV420:
			return 12;
			
		case YUV422:
		case RGB565:
			return 16;
			
		case RGB888:
			return 24;
		
		default:
			return 0;
	}	
}

static T_BOOL ImgLay_RangeValid(T_RECT dstArea, T_RECT srcArea)
{
	if (dstArea.left <= srcArea.left 
		&& dstArea.top <= srcArea.top 
		&& dstArea.width >= srcArea.width 
		&& dstArea.height >= srcArea.height
		&& dstArea.width >= srcArea.left-dstArea.left+srcArea.width
		&& dstArea.height >= srcArea.top-dstArea.top+srcArea.height)
	{
		return AK_TRUE;
	}

	return AK_FALSE;
}

/*
 * @BRIEF Search Valid Layer Buffer for Refresh Dada.
 * @PARAM searchLayer[IN]: Current Processing Layer
 * @PARAM area[IN]:  		First Caller Layer Range
 * @PARAM retLayer[OUT]:	Valid Layer to Refresh Data
 * @PARAM Return Value: AK_TURE: Success, Find Valid Layer Buffer
 * @PARAM Return Value: AK_FALSE: Failure, Can NOT Find Valid Layer Buffer
 * @AUTHOR xie_wenzhong
 * @DATE 2010-07-16
 */

static T_BOOL ImgLay_ValidBufLayer(T_pImgLay searchLayer, T_RECT area, T_pImgLay retLayer)
{
	T_U8 *buffer = AK_NULL;
	T_U8 format;
	
	AK_ASSERT_PTR(layerList, "ImgLay_ValidBufLayer(): \"layerList\" IS NOT Initialization!\n", 0x0);
	AK_ASSERT_PTR(searchLayer, "ImgLay_ValidBufLayer(): Input Parameter Is Error!\n", 0x0);

	// TV Open, YUV Stream Media & Full Monitor
	buffer = Dev_GetFrameBuf(&area, &format);
	if ((searchLayer == layerList->head) && (buffer))
	{
		retLayer->area = area;
		retLayer->pData = buffer;
		retLayer->format = format;
		return AK_TRUE;
	}

	// Current Layer is Valid Layer
	else if (searchLayer->bDataValid && ImgLay_RangeValid(searchLayer->area, area))
	{
		retLayer->area = searchLayer->area;
		retLayer->pData = searchLayer->pData;
		retLayer->format = searchLayer->format;
		return AK_TRUE;
	}

	// Set A Protected Flag
	else if (searchLayer == layerList->head)
	{
		return AK_FALSE;
	}

	// Continue to Search Valid Buffer Layer in Parent Layer
	else
	{
		return ImgLay_ValidBufLayer(searchLayer->prev, area, retLayer);
	}
}

#if 0
static T_eImgLayRet ImgLay_RefreshData(T_ImgLay layer, T_RECT dstArea, const T_U8 *pBuf, T_U16 src_w, T_U16 src_h, T_U8 format)
{
	T_U8 *srcBuf = AK_NULL;
	T_U8 *dstBuf = AK_NULL;
	T_U16 srcStep = 0;
	T_U16 dstStep = 0;
	T_U8 *rgb565Buf = AK_NULL;
	T_U8 *yuv420Buf = AK_NULL;
	T_eImgLayRet retVal = IMGLAY_ERROR;
	T_U16 i;	
	T_U8 *ybuf, *ubuf, *vbuf;	
	
//	if (layer->format == RGB565)
	{
		Fwl_Img_BitBlt(layer.pData, dstformat, dstArea.width, dstArea.height, dstArea.left, dstArea.top, 
				layer->area.width, pBuf, src_w, src_h, srcformat);
						
		return IMGLAY_OK;
	}
	// The Zone Size Is Same
	else if (dstArea.width == src_w && dstArea.height == src_h)
	{		
		srcStep = src_w * (ImgLay_CalcBitWidth(format) >> 3);
		dstStep = layer->area.width * (ImgLay_CalcBitWidth(layer->format) >> 3);				
				
		srcBuf = pBuf;
		dstBuf = layer->pData
				+ (layer->area.width*dstArea.top + dstArea.left)*(ImgLay_CalcBitWidth(layer->format) >> 3); 
							
		if (layer->format == format)					
		{
			if (format == YUV420)
			{
				ImgLay_CalcYUVAddr(pBuf, src_w, src_h);
				
				Fwl_InsertYUV2BckGrndYUV(ybuf, ubuf, vbuf, layer->area.width, layer->area.height, layer->pData,\
						dstArea.left, dstArea.top, src_w, src_h, AK_NULL, 0x0);
			}
			else if (format == RGB888 || format == RGB565)
			{					
				for (i = 0; i < dstArea.height; ++i)
				{
					Utl_MemCpy(dstBuf, srcBuf, srcStep);
					srcBuf += srcStep;
					dstBuf += dstStep;
				}
			}
			return IMGLAY_OK;
		}
		// RGB565 to  RGB888
		else if (format == RGB565 && layer->format == RGB888)
		{
			for (i = 0; i < dstArea.height; ++i)
			{
				ImgLay_RGB565to888Line(dstBuf, srcBuf, dstArea.width)
				srcBuf += srcStep;
				dstBuf += dstStep;
			}
			return IMGLAY_OK;
		}
		// RGB565 to  YUV420
		else if (format == RGB565 && layer->format == YUV420)
		{
			yuv420Buf = Fwl_Malloc(dstArea.width * dstArea.height * (ImgLay_CalcBitWidth(RGB565) >> 3));
			AK_ASSERT_PTR(yuv420Buf, "ImgLay_RefreshData(): Malloc ERROR!\n", IMGLAY_MEM_ERR);
	
			Fwl_RGB565toYUV420(pBuf, yuv420Buf, src_w, src_h);
			
			retVal = ImgLay_RefreshData(layer, dstArea,
						yuv420Buf, dstArea.width, dstArea.height, YUV420);
						
			yuv420Buf = Fwl_Free(yuv420Buf);	
			return retVal;
		}				
	}

	rgb565Buf = Fwl_Malloc(dstArea.width * dstArea.height * (ImgLay_CalcBitWidth(RGB565) >> 3));
	AK_ASSERT_PTR(rgb565Buf, "ImgLay_RefreshData(): Malloc ERROR!\n", IMGLAY_MEM_ERR);
		
	Fwl_Img_BitBlt(rgb565Buf, dstArea.width, dstArea.height, 0, 0, 
				dstArea.width, pBuf, src_w, src_h, format);
				
	retVal = ImgLay_RefreshRect(layer, dstArea,
				rgb565Buf, dstArea.width, dstArea.height, RGB565);
					
	rgb565Buf = Fwl_Free(rgb565Buf);
	return retVal;

}

static T_eImgLayRet ImgLay_RefreshParent(T_pImgLay layer, T_RECT dstArea,
							const T_U8 *pBuf, T_U16 src_w, T_U16 src_h, T_U8 format)
{
	T_pVOID pParentBuf = AK_NULL;
	T_RECT parentArea = AK_NULL;
	T_ImgLay parentLayer;
	
	if (layer->buffer.fParent != AK_NULL)
	{
		pParentBuf = layer->buffer.fParent(layer, &parentArea);
		if (pParentBuf != AK_NULL)
		{
			parentLayer.area = parentArea;
			parentLayer.bDataValid = AK_TRUE;
			parentLayer.format = RGB565;		// Special Application for Display Buffer
			parentLayer.pData = pParentBuf;
					
			dstArea.left += parentArea.left - layer->area.left;
			dstArea.top  += parentArea.top - layer->area.top;
					
			return ImgLay_RefreshRect(&parentLayer, dstArea, pBuf, src_w, src_h, format);
		}
		else
		{
			Fwl_Print(C3, M_DISPLAY,"IMG_LAY:	Can NOT Retrieve Valid Buffer!\n");
			return IMGLAY_ERROR;
		}
	}
	else
	{
		Fwl_Print(C3, M_DISPLAY,"IMG_LAY:	The Function Entrance of Parent Layer is NOT Available.\n");
		return IMGLAY_ERROR;
	}
}
#endif
