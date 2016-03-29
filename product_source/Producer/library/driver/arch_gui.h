/*
 * @file arch_gui.h
 * @brief This file describe the interface of gui module
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author malei
 * @date 2010-06-18
 * @version 2.0
 */

#ifndef __ARCH_GUI_H__
#define __ARCH_GUI_H__


/* @defgroup Arch_gui gui function
 * @ingroup drvlib
 */
/*@{*/

/*
 * error value returned from function.
 */
#define  GUI_ERROR_MODULE_RUN   3
#define  GUI_ERROR_PARAMETER    2
#define  GUI_ERROR_TIMEOUT      1
#define  GUI_ERROR_OK           0

typedef enum {
    FORMAT_RGB565,        //for AK37
    FORMAT_RGB888,        //for AK98
    FORMAT_YUV420
} E_ImageFormat;

typedef enum {
    ROTATE_0,
    ROTATE_90,
    ROTATE_180,           //for AK37
    ROTATE_270            //for AK37
} E_RotateAngle;

/*
 * @BRIEF               open 2D hardware Module
 */
T_VOID Enable_2DModule(T_VOID);

/*
 * @BRIEF               colose 2D hardware Module for saving power
 */
T_VOID Close_2DModule(T_VOID);

/*
 * @BRIEF              reset 2D hardware Module
 */
T_VOID Reset_2DGraphic(T_VOID);

/*
 * @BRIEF               open YUV hardware Module
 *                      only for AK37XX
 */
T_VOID Enable_YUVROTModule(T_VOID);

/*
 * @BRIEF               colose YUV hardware Module for saving power
 *                      only for AK37XX
 */
T_VOID Close_YUVROTModule(T_VOID);

/*
 * @BRIEF               reset YUV hardware Module
 *                      only for AK37XX
 */
T_VOID Reset_YUVROTGraphic(T_VOID);

/*
 * @BRIEF                 image scale and format convert function
 * @BRIEF                 AK37XX support format:yuv420, rgb565; AK980X support format:rgb888 -- no block version
 * @PARAM ibuff           src buffer address arrays
 * @PARAM nibuff          num of src buffer address array.
 * @PARAM srcWidth        src image width
 * @PARAM srcRectX        src offset x
 * @PARAM srcRectY        src offset y
 * @PARAM srcRectW        src scale width
 * @PARAM srcRectH        src scale height
 * @PARAM format_in       src image format: FORMAT_YUV420, FORMAT_RGB565(AK37XX), FORMAT_RGB888(AK980X)
 * @PARAM obuff           dst buffer address arrays
 * @PARAM nobuff          num of dst buffer address array
 * @PARAM dstWidth        dst image width
 * @PARAM dstRectX        dst offset x
 * @PARAM dstRectY        dst offset y
 * @PARAM dstRectW        dst scaled width
 * @PARAM dstRectH        dst scaled height
 * @PARAM format_out      dst image format:FORMAT_YUV420(AK37XX), FORMAT_RGB565(AK37XX), FORMAT_RGB888(AK980X)
 * @PARAM luminance_enabled   if input image format is FORMAT_YUV420, this param can set AK_TRUE to transform luminance.(only used in AK37XX)
 * @PARAM luminance_table     a continuous memory buffer width length of 256 byte, used to transform yuv420 luminance.(only used in AK37XX)
 */
T_U8 Scale_ConvertNoBlock(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_U8 luminance_enabled, T_U8* luminance_table);

/*
 * @BRIEF                 image scale and format convert function
 * @BRIEF                 AK37XX support format:yuv420, rgb565; AK980X support format:rgb888 -- no block version
 * @BRIEF                 this function include alpha blending and color transparency function.
 * @BRIEF                 if alpha blending and color transparency used, output image format must be FORMAT_RGB565!
 * @PARAM ibuff           src buffer address arrays
 * @PARAM nibuff          num of src buffer address array.
 * @PARAM srcWidth        src image width
 * @PARAM srcRectX        src offset x
 * @PARAM srcRectY        src offset y
 * @PARAM srcRectW        src scale width
 * @PARAM srcRectH        src scale height
 * @PARAM format_in       src image format: FORMAT_YUV420, FORMAT_RGB565(AK37XX), FORMAT_RGB888(AK980X)
 * @PARAM obuff           dst buffer address arrays
 * @PARAM nobuff          num of dst buffer address array
 * @PARAM dstWidth        dst image width
 * @PARAM dstRectX        dst offset x
 * @PARAM dstRectY        dst offset y
 * @PARAM dstRectW        dst scaled width
 * @PARAM dstRectH        dst scaled height
 * @PARAM format_out      dst image format:FORMAT_YUV420(AK37XX), FORMAT_RGB565(AK37XX), FORMAT_RGB888(AK980X)
 * @PARAM alpha_enabled   alpha blending enabled. if dst image is FORMAT_RGB565(AK37XX), then alpha enabled is effective.
 * @PARAM alpha           alpha value for alpha transparence (0 ~ 0xf).
 * @PARAM color_trans_enabled   set color transparency effective. if dst image is FORMAT_RGB565(AK37XX), then color transparency is effective.
 * @PARAM color           color value is 24bits, must value & 0xf8fcf8 because of input format FORMAT_RGB565(AK37XX).
 */
T_U8 Scale_Convert2NoBlock(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_BOOL alpha_enabled, T_U8 alpha, T_BOOL color_trans_enabled, T_U32 color);

/*
 * @BRIEF               RGB565(AK37XX) or RGB888(AK980X) format rectangle fill and rotate function,  -- no block version
 * @PARAM ibuff         source image buffer address
 * @PARAM srcWidth      source image width
 * @PARAM srcRectX      source image rectangle start X
 * @PARAM srcRectY      source image rectangle start Y
 * @PARAM srcRectW      source image rectangle width
 * @PARAM srcRectH      source image rectangle height
 * @PARAM obuff         destination image buffer address
 * @PARAM dstWidth      destination image width
 * @PARAM dstRectX      destination image rectangle startX
 * @PARAM dstRectY      destination image rectangle startY
 * @PARAM rotate90      rotate rectangle clockwise 90 degree, value: 1, true; 0,false.
 */
T_U8 Rotate_FillRGBNoBlock(T_U8* ibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                    T_U16 srcRectH, T_U8* obuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY,
                    T_U8  rotate90);

/*
 * @BRIEF            yuv420 rotate function, angle:0, 90, 180, 270.counter clockwise. -- no block version
 * @PARAM ibuff      source image buffer address arrays.
 * @PARAM nibuff     number of source image buffer
 * @PARAM srcWidth   source image width
 * @PARAM srcHeight  source image height
 * @PARAM obuff      destination image buffer address arrays.
 * @PARAM rotate     rotate anlge, value: ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270
 * @PARAM uv_interleaved  whether uv is interleaved, if true, nibuff and nobuff is 2, else nibuff and nobuff is 3.
 *                   only for AK37XX
 */
T_U8 Rotate_FillYUVNoBlock(T_U32* ibuff, T_U8 nibuff, T_U16 srcWidth, T_U16 srcHeight, T_U32* obuff, T_U8 nobuff,
                    E_RotateAngle rotate, T_U8 uv_interleaved);

/*
 * @BRIEF            luminance transform function only for yuv420.
 * @PARAM ibuff      source image Y address
 * @PARAM srcWidth   source image width
 * @PARAM srcHeight  source image height
 * @PARAM format_in  source image format:FORMAT_YUV420
 * @PARAM obuff      destination image Y address
 *                   only for AK37XX
 */
T_U8 Luminance_TransformNoBlock(T_U8*ibuff, T_U16 srcWidth, T_U16 srcHeight, E_ImageFormat format_in, T_U8* obuff,
        T_U8* luminance_table);

/*
 * @BRIEF            Filter source image function, support for YUV420 and RGB565.    -- no block version
 * @PARAM ibuff      source image address, if format_in is FORMAT_YUV420, ibuff is source image Y address
 * @PARAM srcWidth   source image width.
 * @PARAM srcHeight  source image height.
 * @PARAM format_in  source image format:FORMAT_YUV420 or FORMAT_RGB565.
 * @PARAM obuff      destination image address, if format_in is FORMAT_YUV420, obuff is destinantion image Y address.
 *                   only for AK37XX
 */
T_U8 Filter3x3NoBlock(T_U8* ibuff, T_U16 srcWidth, T_U16 srcHeight, E_ImageFormat format_in, T_U8* obuff);

/*
 * @BRIEF                 image scale and format convert function
 * @BRIEF                 AK37XX support output format:yuv420, rgb565
 * @BRIEF                 AK980X support output format:rgb888
 * @PARAM ibuff           src buffer address arrays
 * @PARAM nibuff          num of src buffer address array.
 * @PARAM srcWidth        src image width
 * @PARAM srcRectX        src offset x
 * @PARAM srcRectY        src offset y
 * @PARAM srcRectW        src scale width
 * @PARAM srcRectH        src scale height
 * @PARAM format_in       src image format: FORMAT_YUV420, FORMAT_RGB565(AK37XX), FORMAT_RGB888(AK980X)
 * @PARAM obuff           dst buffer address arrays
 * @PARAM nobuff          num of dst buffer address array
 * @PARAM dstWidth        dst image width
 * @PARAM dstRectX        dst offset x
 * @PARAM dstRectY        dst offset y
 * @PARAM dstRectW        dst scaled width
 * @PARAM dstRectH        dst scaled height
 * @PARAM format_out      dst image format:FORMAT_YUV420(AK37XX), FORMAT_RGB565(AK37XX), FORMAT_RGB888(AK980X)
 * @PARAM luminance_enabled   if input image format is FORMAT_YUV420, this param can set AK_TRUE to transform luminance(only used in AK37XX).
 * @PARAM luminance_table     a continuous memory buffer width length of 256 byte, used to transform yuv420 luminance(only used in AK37XX).
*/
T_U8 Scale_Convert(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_U8 luminance_enabled, T_U8* luminance_table);

/*
 * @BRIEF                 image scale and format convert function
 * @BRIEF                 AK37XX support format:yuv420, rgb565; AK980X support format:rgb888 -- no block version
 * @BRIEF                 this function include alpha blending and color transparency function.
 * @BRIEF                 if alpha blending and color transparency used, output image format must be FORMAT_RGB565!
 * @PARAM ibuff           src buffer address arrays
 * @PARAM nibuff          num of src buffer address array.
 * @PARAM srcWidth        src image width
 * @PARAM srcRectX        src offset x
 * @PARAM srcRectY        src offset y
 * @PARAM srcRectW        src scale width
 * @PARAM srcRectH        src scale height
 * @PARAM format_in       src image format: FORMAT_YUV420, FORMAT_RGB565(AK37XX), FORMAT_RGB888(AK980X)
 * @PARAM obuff           dst buffer address arrays
 * @PARAM nobuff          num of dst buffer address array
 * @PARAM dstWidth        dst image width
 * @PARAM dstRectX        dst offset x
 * @PARAM dstRectY        dst offset y
 * @PARAM dstRectW        dst scaled width
 * @PARAM dstRectH        dst scaled height
 * @PARAM format_out      dst image format:FORMAT_YUV420(AK37XX), FORMAT_RGB565(AK37XX), FORMAT_RGB888(AK980X)
 * @PARAM alpha_enabled   set alpha enabled.if dst image is FORMAT_RGB565(AK37XX), then alpha enabled is effective.
 * @PARAM alpha           alpha value for alpha transparence (0 ~ 0xf).
 * @PARAM color_trans_enabled   set color transparency effective.if dst image is FORMAT_RGB565(AK37XX), then color transparency is effective.
 * @PARAM color           color value is 24bits, must value & 0xf8fcf8 because of input format FORMAT_RGB565(AK37XX).
 */
T_U8 Scale_Convert2(T_U32 *ibuff, T_U32 nibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                     T_U16 srcRectH, E_ImageFormat format_in, 
                     T_U32* obuff, T_U8 nobuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY, T_U16 dstRectW, 
                     T_U16 dstRectH, E_ImageFormat format_out,
                     T_BOOL alpha_enabled, T_U8 alpha, T_BOOL color_trans_enabled, T_U32 color);

/*
 * @BRIEF               RGB565(AK37XX) or RGB888(AK980X) format rectangle fill and rotate function.       
 * @PARAM ibuff         source image buffer address
 * @PARAM srcWidth      source image width
 * @PARAM srcRectX      source image rectangle start X
 * @PARAM srcRectY      source image rectangle start Y
 * @PARAM srcRectW      source image rectangle width
 * @PARAM srcRectH      source image rectangle height
 * @PARAM obuff         destination image buffer address
 * @PARAM dstWidth      destination image width
 * @PARAM dstRectX      destination image rectangle startX
 * @PARAM dstRectY      destination image rectangle startY
 * @PARAM rotate90      rotate rectangle clockwise 90 degree, value: 1, true; 0,false.
 */
T_U8 Rotate_FillRGB(T_U8* ibuff, T_U16 srcWidth, T_U16 srcRectX, T_U16 srcRectY, T_U16 srcRectW,
                    T_U16 srcRectH, T_U8* obuff, T_U16 dstWidth, T_U16 dstRectX, T_U16 dstRectY,
                    T_U8  rotate90);

/*
 * @BRIEF            yuv420 rotate function, angle:0, 90, 180, 270.counter clockwise.
 * @PARAM ibuff      source image buffer address arrays.
 * @PARAM nibuff     number of source image buffer
 * @PARAM srcWidth   source image width
 * @PARAM srcHeight  source image height
 * @PARAM obuff      destination image buffer address arrays.
 * @PARAM rotate     rotate anlge, value: ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270
 * @PARAM uv_interleaved  whether uv is interleaved, if true, nibuff and nobuff is 2, else nibuff and nobuff is 3.
 *                   only for AK37XX
 */
T_U8 Rotate_FillYUV(T_U32* ibuff, T_U8 nibuff, T_U16 srcWidth, T_U16 srcHeight, T_U32* obuff, T_U8 nobuff,
                    E_RotateAngle rotate, T_U8 uv_interleaved);

/*
 * @BRIEF            luminance transform function only for yuv420.
 * @PARAM ibuff      source image Y address
 * @PARAM srcWidth   source image width
 * @PARAM srcHeight  source image height
 * @PARAM format_in  source image format:FORMAT_YUV420
 * @PARAM obuff      destination image Y address
 *                   only for AK37XX
 */
T_U8 Luminance_Transform(T_U8*ibuff, T_U16 srcWidth, T_U16 srcHeight, E_ImageFormat format_in, T_U8* obuff,
        T_U8* luminance_table);

/*
 * @BRIEF            Filter source image function, support for YUV420 and RGB565.
 * @PARAM ibuff      source image address, if format_in is FORMAT_YUV420, ibuff is source image Y address
 * @PARAM srcWidth   source image width.
 * @PARAM srcHeight  source image height.
 * @PARAM format_in  source image format:FORMAT_YUV420 or FORMAT_RGB565.
 * @PARAM obuff      destination image address, if format_in is FORMAT_YUV420, obuff is destinantion image Y address.
 *                   only for AK37XX
 */
T_U8 Filter3x3(T_U8* ibuff, T_U16 srcWidth, T_U16 srcHeight, E_ImageFormat format_in, T_U8* obuff);

/*
 * @BRIEF this function check if gui module is busy. if return AK_TRUE, the gui module is free,
 *        return AK_FALSE, gui module is busy now.
 */
T_BOOL  is2DGUIFinish(T_VOID);

/*
 * @BRIEF this function check if yuv rotate module is busy. if return AK_TRUE, the yuv rotate module is free,
 *        else return AK_FALSE, yuv rotate module is busy now.
 *        only for AK37XX
 */
T_BOOL isYUVROTFinish(T_VOID);

/*
 * @brief copy source image to destination image with format convert & scale
 * @param dstBuf:    destination image buffer (FORMAT_RGB888)
 * @param scaleWidth:  image width after scale
 * @param scaleHeight: image height after scale
 * @param dstPosX:   destination offset X
 * @param dstPosY:   destination offset Y
 * @param dstWidth:  destination image width
 * @param srcBuf:    source image buffer (FORMAT_RGB888/FORMAT_YUV420)
 * @param srcWidth:  source image width
 * @param srcHeight: source image height
 * @param srcFormat: source image format 
 * @author malei
 * @date 2010-08-24
 */
T_S32 Img_BitBlt(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
                 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
                 T_VOID *srcBuf, T_U16 srcWidth, T_U16 srcHeight, T_U8 srcFormat);

/*
 * @brief copy source YUV to destination image with format convert & scale
 * @param dstBuf:    destination image buffer (FORMAT_RGB888)
 * @param scaleWidth:  image width after scale
 * @param scaleHeight: image height after scale
 * @param dstPosX:   destination offset X
 * @param dstPosY:   destination offset Y
 * @param dstWidth:  destination image width
 * @param srcY:    source image Y buffer (FORAMT_YUV420)
 * @param srcU:    source image U buffer (FORAMT_YUV420)
 * @param srcV:    source image V buffer (FORAMT_YUV420)
 * @param srcWidth:  source image width
 * @param srcHeight: source image height
 * @author malei
 * @date 2010-08-24
 */
T_S32 Img_BitBltYUV(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
                 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
                 T_VOID *srcY, T_VOID *srcU, T_VOID *srcV, T_U16 srcWidth, T_U16 srcHeight);

/*
 * @brief copy source image to destination image with format convert & scale & alpha blend
 * @param dstBuf:    destination image buffer (FORAMT_RGB888)
 * @param scaleWidth:  image width after scale
 * @param scaleHeight: image height after scale
 * @param dstPosX:   destination offset X
 * @param dstPosY:   destination offset Y
 * @param dstWidth:  destination image width
 * @param srcBuf:    source image buffer (FORAMT_RGB888)
 * @param srcWidth:  source image width
 * @param srcHeight: source image height
 * @param srcFormat: source image format (FORAMT_RGB888)
 * @param alpha: alpha blend value
 * @author malei
 * @date 2010-08-24
 */         
T_S32 Img_BitBlt_alpha(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
                 T_U16  dstPosX, T_U16 dstPosY,    T_U16 dstWidth,
                 T_VOID *srcBuf, T_U16 srcWidth,   T_U16 srcHeight, T_U8 srcFormat, T_U8 alpha);

/*@}*/
#endif

