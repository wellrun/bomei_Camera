/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: fwl_font.h
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
	
#ifndef __FWL_FONT_H__
#define __FWL_FONT_H__


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
T_VOID Fwl_UDispSpeciStringOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_POS x, T_POS y, 
	T_U16* disp_string, T_COLOR color, T_U8 colortype, T_FONT font, T_U16 strLen );



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
	T_pCSTR string, T_U16 strLen, T_COLOR color, T_U8 colortype, T_FONT font);


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
	T_POS x, T_POS y, T_U16 UStrLen, T_U16 offset, T_U16 width_limit, T_COLOR color, T_U8 colortype, T_FONT font);


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
        T_POS x, T_POS y, T_U16* disp_string, T_COLOR color, T_FONT font, T_U16 strLen);


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
		T_POS x, T_POS y, T_pCSTR string, T_U16 strLen, T_COLOR color, T_FONT font);


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
		T_U16* pUString, T_POS x, T_POS y, T_U16 UStrLen, T_U16 offset, T_U16 width_limit, T_COLOR color, T_FONT font);


#endif
