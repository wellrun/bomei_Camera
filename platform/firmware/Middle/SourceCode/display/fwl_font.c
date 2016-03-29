/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: fwl_font.c
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


#include "eng_dynamicfont.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "eng_dataconvert.h"
#include "eng_freetype.h"



/**
 * @brief Display unicode string in the appointed position on RGB.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x: the x coordiantion to draw the string
 * @param T_POS y: the y coordiantion to draw the string
 * @param T_U16* disp_string: unicode string to be displayed
 * @param T_U16 strLen:the length of the string
 * @param T_COLOR color:the color of the string to draw
 * @param T_U8 colortype:RGB888 or RGB565
 * @param T_FONT font:the font of the string to draw
 * @return T_VOID
 * @retval 
 */
T_VOID Fwl_UDispSpeciStringOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, 
	T_POS x, T_POS y, T_U16* disp_string, T_COLOR color, T_U8 colortype, T_FONT font, T_U16 strLen )
{
	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY,"Fwl_UDispSpeciStringOnRGB AK_NULL == buf!\n");
		return;
	}

	if (AK_NULL == disp_string)
	{
		Fwl_Print(C3, M_DISPLAY,"Fwl_UDispSpeciStringOnRGB AK_NULL == disp_string!\n");
		return;
	}
	

#ifdef SUPPORT_VFONT
	if(gs.VF_FontInstalled && gb.bIsUseVFont)
	{
		VF_DispStr(buf, imgwidth, imgheight, disp_string, x, y, color, gs.VF_FontSize);
	}
	else
	{
#endif

    	DynamicFont_UDispSpeciStringOnRGB(buf, imgwidth, imgheight, x, y, disp_string, 
    									color, colortype, font, strLen);
#ifdef SUPPORT_VFONT
	}
#endif
}



/**
 * @brief Display string in the appointed position on RGB.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x: the x coordiantion to draw the string
 * @param T_POS y: the y coordiantion to draw the string
 * @param T_pCSTR string: pointer to the string.
 * @param T_U16 strLen:the length of the string
 * @param T_COLOR color:the color of the string to draw
 * @param T_U8 colortype:RGB888 or RGB565
 * @param T_FONT font:the font of the string to draw
 * @return T_VOID
 * @retval 
 */
T_VOID Fwl_DispStringOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_POS x, T_POS y, 
	T_pCSTR string, T_U16 strLen, T_COLOR color, T_U8 colortype, T_FONT font)
{
    T_U16* wcs = AK_NULL;

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY,"Fwl_DispStringOnRGB AK_NULL == buf!\n");
		return;
	}
	
    AK_ASSERT_PTR_VOID(string, "Fwl_DispStringOnRGB(): string");

	wcs = Fwl_Malloc(sizeof(T_U16)*(strLen+1));

	if (AK_NULL == wcs)
	{
		Fwl_Print(C3, M_DISPLAY,"Fwl_DispStringOnRGB AK_NULL == wcs!\n");
		return;
	}
	
    Eng_StrMbcs2Ucs(string, wcs);
    Fwl_UDispSpeciStringOnRGB( buf, imgwidth, imgheight, x, y, wcs, color, colortype, font, strLen);
	
    Fwl_Free(wcs);
}



/**
 * @brief scroll-display unicode string in the appointed position on RGB.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *buf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x: the x coordiantion to draw the string
 * @param T_POS y: the y coordiantion to draw the string
 * @param T_U16* pUString: unicode string to be displayed
 * @param T_U16 UStrLen:the length of the string
 * @param T_U16 offset: the offset to begin displaying
 * @param T_U16 width_limit: the limit of displaying width
 * @param T_COLOR color:the color of the string to draw
 * @param T_U8 colortype:RGB888 or RGB565
 * @param T_FONT font:the font of the string to draw
 * @return T_BOOL
 * @retval AK_TRUE: success AK_FALSE: fail
 */
T_BOOL Fwl_UScrollDispStringOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_U16* pUString, 
	T_POS x, T_POS y, T_U16 UStrLen, T_U16 offset, T_U16 width_limit, T_COLOR color, T_U8 colortype, T_FONT font)
{
    T_U16* pUStr = AK_NULL;
    T_U16 strLen;

	if (AK_NULL == buf)
	{
		Fwl_Print(C3, M_DISPLAY,"Fwl_UScrollDispStringOnRGB AK_NULL == buf!\n");
		return AK_FALSE;
	}
	
    AK_ASSERT_VAL(width_limit>0, "Fwl_UScrollDispStringOnRGB():inputput parm is bad", AK_FALSE);

    if (offset >= UStrLen)
    {
        Fwl_Print(C3, M_DISPLAY,"Fwl_UScrollDispStringOnRGB():offset is too large or offset and string length both 0");
        return AK_FALSE;
    }
    
    pUStr = pUString + offset;
    strLen = Fwl_GetUStringDispNum(pUStr, (T_U16)(UStrLen-offset), width_limit, font);
    Fwl_UDispSpeciStringOnRGB(buf, imgwidth, imgheight, x, y, pUStr, color, colortype, font, strLen);

    return AK_TRUE;
}





/**
 * @brief Display unicode string in the appointed position on YUV.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x: the x coordiantion to draw the string
 * @param T_POS y: the y coordiantion to draw the string
 * @param T_U16* disp_string: unicode string to be displayed
 * @param T_U16 strLen:the length of the string
 * @param T_COLOR color:the color of the string to draw
 * @param T_FONT font:the font of the string to draw
 * @return T_VOID
 * @retval 
 */
T_VOID Fwl_UDispSpeciStringOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
        T_POS x, T_POS y, T_U16* disp_string, T_COLOR color, T_FONT font, T_U16 strLen)
{
	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY,"Fwl_UDispSpeciStringOnYUV AK_NULL == buf!\n");
		return;
	}

	if (AK_NULL == disp_string)
	{
		Fwl_Print(C3, M_DISPLAY,"Fwl_UDispSpeciStringOnRGB AK_NULL == disp_string!\n");
		return;
	}
	
    DynamicFont_UDispSpeciStringOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x, y, disp_string, color, font, strLen);
}



/**
 * @brief Display string in the appointed position on YUV.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x: the x coordiantion to draw the string
 * @param T_POS y: the y coordiantion to draw the string
 * @param T_pCSTR string: pointer to the string
 * @param T_U16 strLen:the length of the string
 * @param T_COLOR color:the color of the string to draw
 * @param T_FONT font:the font of the string to draw
 * @return T_VOID
 * @retval 
 */
T_VOID Fwl_DispStringOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
		T_POS x, T_POS y, T_pCSTR string, T_U16 strLen, T_COLOR color, T_FONT font)
{
    T_U16* wcs = AK_NULL;

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY,"Fwl_DispStringOnYUV AK_NULL == buf!\n");
		return;
	}
	
    AK_ASSERT_PTR_VOID(string, "Fwl_DispStringOnYUV(): string");

	wcs = Fwl_Malloc(sizeof(T_U16)*(strLen+1));

	if (AK_NULL == wcs)
	{
		Fwl_Print(C3, M_DISPLAY,"Fwl_DispStringOnYUV AK_NULL == wcs!\n");
		return;
	}
	
    Eng_StrMbcs2Ucs(string, wcs);
    Fwl_UDispSpeciStringOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x, y, wcs, color, font, strLen);
	
    Fwl_Free(wcs);
}


/**
 * @brief scroll-display unicode string in the appointed position on YUV.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_U8 *ybuf,*ubuf,*vbuf:
 * @param T_U32 imgwidth:max width
 * @param T_U32 imgheight:max height
 * @param T_POS x: the x coordiantion to draw the string
 * @param T_POS y: the y coordiantion to draw the string
 * @param T_U16* pUString: unicode string to be displayed
 * @param T_U16 UStrLen:the length of the string
 * @param T_U16 offset: the offset to begin displaying
 * @param T_U16 width_limit: the limit of displaying width
 * @param T_COLOR color:the color of the string to draw
 * @param T_FONT font:the font of the string to draw
 * @return T_BOOL
 * @retval AK_TRUE: success AK_FALSE: fail
 */
T_BOOL Fwl_UScrollDispStringOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
	T_U16* pUString, T_POS x, T_POS y, T_U16 UStrLen, T_U16 offset, T_U16 width_limit, T_COLOR color, T_FONT font)
{
    T_U16* pUStr = AK_NULL;
    T_U16 strLen;

	if ((AK_NULL == ybuf) || (AK_NULL == ubuf) || (AK_NULL == vbuf))
	{
		Fwl_Print(C3, M_DISPLAY,"Fwl_UScrollDispStringOnYUV AK_NULL == buf!\n");
		return AK_FALSE;
	}

    AK_ASSERT_VAL(width_limit>0, "DynamicFont_UScrollDispString():inputput parm is bad", AK_FALSE);

    if (offset >= UStrLen)
    {
        Fwl_Print(C3, M_DISPLAY,"DynamicFont_UScrollDispString():offset is too large or offset and string length both 0");
        return AK_FALSE;
    }
    
    pUStr = pUString + offset;
    strLen = Fwl_GetUStringDispNum(pUStr, (T_U16)(UStrLen-offset), width_limit, font);
    Fwl_UDispSpeciStringOnYUV(ybuf, ubuf, vbuf, imgwidth, imgheight, x, y, pUStr, color, font, strLen);

    return AK_TRUE;
}


