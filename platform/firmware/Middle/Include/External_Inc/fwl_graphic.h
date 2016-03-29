/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: fwl_graphic.h
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

#ifndef __FWL_GRAPHIC_H__
#define __FWL_GRAPHIC_H__


#include "anyka_types.h"
#include "fwl_pfdisplay.h"
#include "eng_akbmp.h"
#include <Fwl_display.h>



/***************************************   绘制图形**********************************************/


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
T_COLOR Fwl_GetPixelOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_POS x, T_POS y, T_U8 colortype );



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
								T_COLOR color, T_U8 colortype );


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
								T_U16 pos_x, T_U16 pos_y, T_COLOR color);


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
								T_POS x2, T_POS y2, T_COLOR color, T_U8 colortype);


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
								T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color);


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
								T_COLOR color, T_U8 colortype);


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
								T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color);


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
								T_LEN radius, T_COLOR color, T_U8 colortype);


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
									T_POS x, T_POS y, T_LEN radius, T_COLOR color);


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
								T_POS radii, T_COLOR color, T_U8 colortype);


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
								T_POS center_x, T_POS center_y, T_POS radii, T_COLOR color);


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
									T_COLOR color, T_U8 colortype);


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
									T_RECT *rect, T_COLOR color);


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
									T_TRIANGLE_DIRECTION dir, T_COLOR color, T_U8 colortype);


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
									T_RECT *rect, T_TRIANGLE_DIRECTION dir, T_COLOR color);



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
									T_LEN radius, T_BOOL focus, T_COLOR color, T_U8 colortype);


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
									T_POS x, T_POS y, T_LEN radius, T_BOOL focus, T_COLOR color);



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
									T_S8 flag, T_U8 colortype);


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
									T_RECT *rect, T_S8 flag);





/***************************************   缩放**********************************************/

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
							T_VOID *srcBuf, T_U16 srcWidth, T_U16 srcHeight, T_U8 srcFormat);




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
                 T_VOID *srcY, T_VOID *srcU, T_VOID *srcV, T_U16 srcWidth, T_U16 srcHeight);


/***************************************  YUV - RGB  转换**********************************************/


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
T_VOID Fwl_RGB565toRGB888(T_S8 * pDestData888, T_S8 * pSrcData565, T_S32 nWidth, T_S32 nHeight);


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
T_VOID Fwl_RGB888toRGB565(T_S8 * pDestData565, T_S8 * pScrData888, T_S32 nWidth, T_S32 nHeight);


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
T_BOOL Fwl_YUV420toRGB888(T_U8 *y, T_U8 *u, T_U8 *v, T_U8 *dstRGB, T_U32 width, T_U32 uvLine);



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
T_BOOL Fwl_YUV420toRGB565(T_U8 *y, T_U8 *u, T_U8 *v, T_U32 *dstRGB, T_U32 width, T_U32 uvLine);


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
T_VOID  Fwl_RGB565toYUV420(T_U8* pRgb565, T_U8* yuv, T_U32 rectwidth, T_U32 rectheigth);


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
T_VOID  Fwl_RGB888toYUV420(T_U8* pRgb888, T_U8* yuv, T_U32 rectwidth, T_U32 rectheigth);






/***************************************   绘制图像**********************************************/


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
T_BOOL Fwl_AkBmpDrawPartOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_POS x, T_POS y, 
		T_RECT *range, const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse, T_U8 colortype);


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
	T_POS x, T_POS y, T_RECT *range, T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse, T_U8 colortype);


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
	T_POS x, T_POS y, T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse, T_U8 colortype);



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
	T_POS x, T_POS y, T_RECT *range, const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse);



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
    T_POS x, T_POS y, T_RECT *range, T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse);



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
	T_POS x, T_POS y, T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse);


T_S32 Fwl_Scale_Convert(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
                 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
                 T_VOID *srcBuf, T_U16 srcWidth, T_U16 srcHeight, T_U8 srcFormat);

T_BOOL Fwl_YUV420BitBlt(const T_U8 *y, const T_U8 *u, const T_U8 *v, T_LEN srcBufW, T_RECT* srcRect,
					 		 T_U8 *dstBuf, T_LEN dstBufW, T_U8 dstFormat, T_RECT* dstRect);

T_BOOL Fwl_RGB565BitBlt(const T_U8* rgbBuf, T_LEN srcBufW, T_RECT* srcRect,
					 		 T_U8 *dstBuf, T_LEN dstBufW, T_U8 dstFormat, T_RECT* dstRect);


/**
 * @brief YUV 2 YUV/RGB Zoom, (NOT Limited 4X or 1/4X) Aspect Ratio < 4X
 *
 * @date 	July 5, 2011
 * @author 	Xie_Wenzhong
 * @param	srcY		[in] YUV Source Addr Y
 * @param	srcU		[in] YUV Source Addr U
 * @param	srcU		[in] YUV Source Addr V
 * @param	srcW	[in] Source Frame Width
 * @param	srcH		[in] Source Frame Height 
 * @param	dstBuf	[out] Destination YUV Size Data
 * @param	dstW	[in] Destination Frame Width
 * @param	dstH	[in] Destination Frame Height
 * @param	dstForamt	[in] Destination Frame Format FORMAT_YUV420/FORMAT_RGB565
 * @return 	T_BOOL
 * @retval	AK_FALSE	Zoom Failure
 * @retval	AK_TRUE	Zoom Success
 */
T_BOOL Fwl_YuvZoom(const T_U8 *srcY, const T_U8 *srcU, const T_U8 *srcV, T_LEN srcBufW, T_pRECT srcWin,
						T_U8 *dstBuf, T_LEN dstBufW,  T_U8 dstForamt, T_pRECT dstWin);



T_VOID Fwl_Clean(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_RECT *rect, T_COLOR color, T_U8 colortype);

T_VOID Fwl_Clean(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_RECT *rect, T_COLOR color, T_U8 colortype);

T_BOOL Fwl_AkBmpDrawAlphaFromStringOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, 
	T_POS x, T_POS y, T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse, T_U8 colortype);
T_VOID Fwl_Reset2DGraphic(T_VOID);
T_S32 Fwl_Scale_ConvertWithAlpha(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
                 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
                 T_VOID *srcBuf, T_U16 srcWidth, T_U16 srcHeight, T_U8 srcFormat,T_U8 alpha);

T_VOID Fwl_ColorToRGB565(T_COLOR color, T_U8 *low, T_U8 *high);

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
T_BOOL Fwl_InitRect(T_RECT *rect, T_POS x, T_POS y, T_LEN width, T_LEN height);

#endif
