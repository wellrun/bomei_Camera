
/**
 * @file convert.h
 * @brief image convert APIs heaser file.
 * This file provides image convert APIs: image zoom in or out,
 * cut image, convert image mirror.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Guanghua Zhang
 * @date 2004-10-29
 * @version 1.0
 * @ref AK3210 technical manual.
 */

#ifndef __IMAGE_CONVERT_H__
#define __IMAGE_CONVERT_H__

#include "anyka_types.h"
#include "akdefine.h"

/**
 * @brief RGB image convert function
 * Convert the image's width & height to height & width.
 * @author Guanghua Zhang
 * @date 2004-09-20
 * @param T_U8 *RGB: input & output source RGB image data
 * @param T_U32 srcW: source RGB image's width
 * @param T_U32 srcH: source RGB image's height
 * @param T_BOOL Xmirror: image X mirror(AK_TRUE) or not(AK_FALSE)
 * @param T_BOOL Ymirror: image Y mirror(AK_TRUE) or not(AK_FALSE)
 * @param T_BOOL reverse: image force reverse(AK_TRUE) or auto reverse(AK_FALSE)
 * @param T_BOOL XCenter: X center align
 * @param T_BOOL YCenter: Y center align
 * @param T_BOOL clean: clean LCD display
 * @param T_BOOL swap: swap red & blue color
 * @return T_VOID
 * @retval
 */
T_VOID ConvertRGB2LCDBuf(T_U8 *RGB, T_U32 width, T_U32 height, \
                   T_BOOL Xmirror, T_BOOL Ymirror, T_BOOL reverse, \
                   T_BOOL XCenter, T_BOOL YCenter, \
                   T_BOOL clean, T_BOOL swap);

T_VOID GetRGB2RGBBuf(T_U8 *srcRGB, T_U32 srcW, T_U32 srcH, \
                     T_U8 *dstRGB, T_U32 dstW, T_U32 dstH, \
                     T_U32 offsetX, T_U32 offsetY, \
                     T_U32 zoom, T_U32 *scale);

T_U32 PaletteNum(T_U16 deep);



T_BOOL GetBMP2RGBBuf565(T_U8 *BMPBuf, T_U8 *RGBBuf, \
                     T_U32 RGBWidth, T_U32 RGBHeight, \
                     T_U32 offsetX, T_U32 offsetY, \
                     T_U32 zoom, T_U16 rotate, T_U32 *scale, T_COLOR bColor);

T_BOOL GetBMP2RGBBuf888(T_U8 *BMPBuf, T_U8 *RGBBuf, \
                     T_U32 RGBWidth, T_U32 RGBHeight, \
                     T_U32 offsetX, T_U32 offsetY, \
                     T_U32 zoom, T_U16 rotate, T_U32 *scale, T_COLOR bColor);
#ifdef OS_ANYKA
#ifdef LCD_MODE_565
#define GetBMP2RGBBuf	GetBMP2RGBBuf565
#else
#define GetBMP2RGBBuf	GetBMP2RGBBuf888
#endif
#else
#define GetBMP2RGBBuf	GetBMP2RGBBuf888
#endif

T_VOID ConvertYUVByZoomOut(T_U8 *iY, T_U8 *iU, T_U8 *iV, T_U32 width, T_U32 height, T_U8 zoom);

T_VOID CutYUVByZoom(T_U8 *iY, T_U8 *iU, T_U8 *iV, T_U32 width, T_U32 height, T_U8 zoom);

T_VOID CutYUVByOffsetZoom(T_U8 *iY, T_U8 *iU, T_U8 *iV, T_U32 width, T_U32 height, T_U32 offsetX, T_U32 offsetY, T_U8 zoom);

T_VOID CutYUVBySize(T_U8 *iY, T_U8 *iU, T_U8 *iV, T_U32 srcW, T_U32 srcH, T_U32 dstW, T_U32 dstH);

T_VOID ZoomOutYUVBySize(T_U8 *iY, T_U8 *iU, T_U8 *iV, T_U32 srcW, T_U32 srcH, T_U32 dstW, T_U32 dstH);

T_VOID ZoomInYUVBySize(T_U8 *iY, T_U8 *iU, T_U8 *iV, T_U32 srcW, T_U32 srcH, T_U32 dstW, T_U32 dstH);

T_VOID ConvertYUVByZoomIn(T_U8 *iY, T_U8 *iU, T_U8 *iV, T_U32 width, T_U32 height, T_U16 zoom);

T_VOID ConvertRGB2BGR(T_U8 *buffer, T_U32 width, T_U32 height);

T_U32 FillAkBmpHead(T_U8 *img_buff, T_U16 width, T_U16 height);

// T_U16   Eng_Get_UVBufPositon_YUV420(T_POS row, T_POS col, T_LEN imgwidth);
#define Eng_Get_UVBufPositon_YUV420(row,col,imgwidth) (((imgwidth)>>1)*((row)>>1)+((col)>>1))

T_VOID  Eng_Bmp2Yuv(T_U8 * bmp, T_U8 * yuv);

/**
 * @brief Convert BMP Image to BMP 16Bit Deep Image(RGB565)
 *
 * @author 
 * @date	March 9, 2012
 * @param	bmp_src		[in]	Source BMP Image
 * @param	size			[in/out]	Source/Destination BMP Image Size
 * @param	dst_w		[in]	Destination BMP Image Width
 * @param	dst_w		[in]	Destination BMP Image Height
 * @return 	T_pDATA
 * @retval	AK_NULL	Failure
 * @retval	Others		Success
 */
T_pDATA Eng_BMPSubSample(T_U8* bmp_src, T_U32 *size, T_U16 dst_w, T_U16 dst_h);


#endif  /* end of __CONVERT_H__ */

