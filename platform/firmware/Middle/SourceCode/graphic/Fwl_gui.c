#include "anyka_Types.h"
#include "Akdefine.h"
#include "AKOS_api.h"
#include "Eng_Debug.h"
#include "Arch_gui.h"
#include "Eng_ScaleConvertSoft.h"
#include "Fwl_gui.h"

#define MIN_2D_WIDTH	18
#define MIN_2D_HEIGHT	18
#define MAX_2D_WIDTH	1280
#define MAX_2D_HEIGHT	1024

// 2D Harder Decoder ONLY Size > 18 Pixel And W < 1280, H < 1024
#define IS_OUT_RANGE_2D(width,height)\
	((width) < MIN_2D_WIDTH || (width) > MAX_2D_WIDTH || (height) < MIN_2D_HEIGHT || (height) > MAX_2D_HEIGHT)? AK_TRUE : AK_FALSE

static T_hSemaphore guiSem = AK_INVALID_SEMAPHORE;

static T_S32 Gui_Lock(T_VOID)
{
	if (AK_INVALID_SEMAPHORE == guiSem)
		guiSem = AK_Create_Semaphore(1, AK_PRIORITY);
	
	return AK_Obtain_Semaphore(guiSem, AK_SUSPEND);
}

static T_S32 Gui_Unlock(T_VOID)
{
	if (AK_INVALID_SEMAPHORE == guiSem)
		return -1;

	return AK_Release_Semaphore(guiSem);
}

T_S32 Gui_DeleteLock(T_VOID)
{
	if (AK_INVALID_SEMAPHORE == guiSem)
		return AK_SUCCESS;

	AK_Delete_Semaphore(guiSem);
	guiSem = AK_INVALID_SEMAPHORE;

	return AK_SUCCESS;
}

T_U8 Fwl_ScaleConvert(T_U32 *iBuf, T_U32 niBuf, T_U16 srcW, T_pRECT srcWin, E_ImageFormat formatIn, 
                     T_U32* oBuf, T_U8 noBuf, T_U16 dstW, T_pRECT dstWin, E_ImageFormat formatOut,
                     T_U8 luminanceEn, T_U8* luminanceTab)
{
	T_U8 ret = 0;
	
//	if (IS_OUT_RANGE_2D(srcWin->width, srcWin->height)
//		|| IS_OUT_RANGE_2D(dstWin->width, dstWin->height))
	if (1)
	{
		return Scale_ConvertSoft(iBuf, niBuf, srcW, srcWin->left, srcWin->top, srcWin->width, srcWin->height, formatIn, 
                     oBuf, noBuf, dstW, dstWin->left, dstWin->top, dstWin->width, dstWin->height, formatOut,
                     luminanceEn, luminanceTab);
	}
	
#ifdef OS_ANYKA	
	Gui_Lock();
	Reset_2DGraphic();
	ret = Scale_Convert(iBuf, niBuf, srcW, srcWin->left, srcWin->top, srcWin->width, srcWin->height, formatIn, 
                     oBuf, noBuf, dstW, dstWin->left, dstWin->top, dstWin->width, dstWin->height, formatOut,
                     luminanceEn, luminanceTab);
	Gui_Unlock();

	return ret;
#else
	return Scale_ConvertSoft(iBuf, niBuf, srcW, srcWin->left, srcWin->top, srcWin->width, srcWin->height, formatIn, 
                     oBuf, noBuf, dstW, dstWin->left, dstWin->top, dstWin->width, dstWin->height, formatOut,
                     luminanceEn, luminanceTab);
#endif	
}

T_U8 Fwl_ScaleConvertNoBlock(T_U32 *iBuf, T_U32 niBuf, T_U16 srcW, T_pRECT srcWin, E_ImageFormat formatIn, 
                     T_U32* oBuf, T_U8 noBuf, T_U16 dstW, T_pRECT dstWin, E_ImageFormat formatOut,
                     T_U8 luminanceEn, T_U8* luminanceTab)
{
	T_U8 ret = 0;
	
	if (IS_OUT_RANGE_2D(srcWin->width, srcWin->height)
		|| IS_OUT_RANGE_2D(dstWin->width, dstWin->height))
	{
		Fwl_Print(C3, M_2D, "Out Of Range");
		return 2;
	}
	
#ifdef OS_ANYKA		
	Gui_Lock();
	Reset_2DGraphic();
	ret = Scale_ConvertNoBlock(iBuf, niBuf, srcW, srcWin->left, srcWin->top, srcWin->width, srcWin->height, formatIn, 
                     oBuf, noBuf, dstW, dstWin->left, dstWin->top, dstWin->width, dstWin->height, formatOut,
                     luminanceEn, luminanceTab);
	Gui_Unlock();
#endif		
	return ret;
}

T_U8 Fwl_ScaleConvertEx(T_U32 *iBuf, T_U32 niBuf, T_U16 srcW, T_pRECT srcWin, E_ImageFormat formatIn, 
                     T_U32* oBuf, T_U8 noBuf, T_U16 dstW, T_pRECT dstWin, E_ImageFormat formatOut,
                     T_BOOL alphaEn, T_U8 alpha, T_BOOL colorTransEn, T_U32 color)
{
	T_U8 ret = 0;
	
	//if (IS_OUT_RANGE_2D(srcWin->width, srcWin->height)
		//|| IS_OUT_RANGE_2D(dstWin->width, dstWin->height))
	if (1)
	{
		return Scale_Convert2Soft(iBuf, niBuf, srcW, srcWin->left, srcWin->top, srcWin->width, srcWin->height, formatIn, 
                     oBuf, noBuf, dstW, dstWin->left, dstWin->top, dstWin->width, dstWin->height, formatOut,
                     alphaEn, (T_U8)(alpha << 4), AK_FALSE, color);
	}
	
#ifdef OS_ANYKA		
	Gui_Lock();
	Reset_2DGraphic();
	ret = Scale_Convert2(iBuf, niBuf, srcW, srcWin->left, srcWin->top, srcWin->width, srcWin->height, formatIn, 
                     oBuf, noBuf, dstW, dstWin->left, dstWin->top, dstWin->width, dstWin->height, formatOut,
                     alphaEn, alpha, colorTransEn, color);
	Gui_Unlock();
	return ret;
#else
	return Scale_Convert2Soft(iBuf, niBuf, srcW, srcWin->left, srcWin->top, srcWin->width, srcWin->height, formatIn, 
                     oBuf, noBuf, dstW, dstWin->left, dstWin->top, dstWin->width, dstWin->height, formatOut,
                     alphaEn, (T_U8)(alpha << 4), AK_FALSE, color);
#endif	
}

T_U8 Fwl_ScaleConvertNoBlockEx(T_U32 *iBuf, T_U32 niBuf, T_U16 srcW, T_pRECT srcWin, E_ImageFormat formatIn, 
                     T_U32* oBuf, T_U8 noBuf, T_U16 dstW, T_pRECT dstWin, E_ImageFormat formatOut,
                     T_BOOL alphaEn, T_U8 alpha, T_BOOL colorTransEn, T_U32 color)
{
	T_U8 ret = 0;
	
	if (IS_OUT_RANGE_2D(srcWin->width, srcWin->height)
		|| IS_OUT_RANGE_2D(dstWin->width, dstWin->height))
	{
		Fwl_Print(C3, M_2D, "EX Out Of Range");
		return 2;
	}
	
#ifdef OS_ANYKA		
	Gui_Lock();
	Reset_2DGraphic();
	ret = Scale_Convert2NoBlock(iBuf, niBuf, srcW, srcWin->left, srcWin->top, srcWin->width, srcWin->height, formatIn, 
                     oBuf, noBuf, dstW, dstWin->left, dstWin->top, dstWin->width, dstWin->height, formatOut,
                     alphaEn, alpha, colorTransEn, color);
	Gui_Unlock();
#endif		
	return ret;
}



T_U8 Fwl_ScaleConvert_Sw(T_U32 *iBuf, T_U32 niBuf, T_U16 srcW, T_pRECT srcWin, E_ImageFormat formatIn, 
                     T_U32* oBuf, T_U8 noBuf, T_U16 dstW, T_pRECT dstWin, E_ImageFormat formatOut,
                     T_U8 luminanceEn, T_U8* luminanceTab)
{
	
	return Scale_ConvertSoft(iBuf, niBuf, srcW, srcWin->left, srcWin->top, srcWin->width, srcWin->height, formatIn, 
                     oBuf, noBuf, dstW, dstWin->left, dstWin->top, dstWin->width, dstWin->height, formatOut,
                     luminanceEn, luminanceTab);

}


T_BOOL Fwl_ScaleParamCheck(T_RECT *src, T_RECT *dest, T_U32 srcW, T_U32 srcH)
{
    T_LEN tmp = 0;
    
    if ((AK_NULL == src) || (AK_NULL == dest))
    {
        return AK_FALSE;
    }

    if (src->width > dest->width)
    {
        tmp = (dest->width << 2);
        if (src->width > tmp)
        {
            src->width = tmp;
        }
    }

    else if (src->width < dest->width)
    {
        tmp = (dest->width >> 2);
        if ((src->width < tmp) && (tmp <= (T_S32)srcW))
        {
            src->width = tmp;
        }
    }
        
    if (src->height > dest->height)
    {
        tmp = (dest->height << 2);
        if (src->height > tmp)
        {
            src->height = tmp;
        }
    }
    
    else if (src->height < dest->height)
    {
        tmp = (dest->height >> 2);
        if ((src->height < tmp) && (tmp <= (T_S32)srcH))
        {
            src->height = tmp;
        }
    }

    src->left = (T_POS)((srcW - src->width) >> 1);
    src->top  = (T_POS)((srcH - src->height)>> 1);

	src->width  -= src->width  % 2;
	src->height -= src->height % 2;
	src->left   -= src->left   % 2;
	src->top    -= src->top    % 2;

    return AK_TRUE;
}


