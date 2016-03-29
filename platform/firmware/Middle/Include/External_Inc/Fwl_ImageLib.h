#ifndef _FWL_IMAGELIB_H_
#define _FWL_IMAGELIB_H_

#include "Anyka_Types.h"
#include "Lib_Image_Api.h"

T_U8 *Img_PNG2BMP_Mutex(const T_U8 *pngData, T_U32 *size);


T_BOOL Img_YUV2JPEG_Mutex(T_U8 *srcY, T_U8 *srcU, T_U8 *srcV, T_U8 *dstJPEG, 
									T_U32 *size, T_U32 width, T_U32 height, T_U8 quality);

T_U8 *Img_Jpeg2BMP_Mutex(const T_U8 *jpegData, T_U32 *size);

T_U8 *Img_Jpeg2BMPEx_Mutex(const T_U8 *jpegData, T_U16 dstWidth, T_U16 dstHeight, T_U32 *size);

T_BOOL Img_JPEG2YUV_Mutex(T_U8 *srcJPEG, T_U32 size, T_U8 *dstYUV, T_S32 *width, T_S32*height);

T_BOOL Img_VideoJPEG2YUV_Mutex(T_U8 *srcJPEG, T_U32 size, T_U8 *dstYUV, T_S32 *width, T_S32 *height);

T_BOOL Img_JPEG2YUV4x_Mutex(T_U8 *srcJPEG, T_U32 size, T_U8 *dstYUV, T_S32 *width, T_S32*height);

T_BOOL Img_JPEG2YUVEx_Mutex(T_U8 *srcJPEG, T_U32 size, T_U8 *dstYUV, T_U16 dstWidth, T_U16 dstHeight);

T_BOOL Img_Jpeg2RGB_Mutex(const T_U8 *srcJPEG, T_U8 *dstRGB, T_U32 *width, T_U32 *height, T_U32 size);

T_BOOL Img_YUV2JPEG4x_Mutex(T_U8 *srcYUV, T_U8 *dstJPEG, T_U32 *size, T_U32 width, T_U32 height, T_U8 quality);

T_BOOL Img_YUV2JPEGEx_Mutex(T_U8 *srcYUV, T_U8 *dstJPEG, T_U32 *size,
									T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth, T_U32 dstHeight, T_U8 quality);

T_BOOL Img_YUV2JPEGExs_Mutex(T_U8 *srcY, T_U8 *srcU, T_U8 *srcV, T_U8 *dstJPEG, T_U32 *size,
                           			T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth, T_U32 dstHeight, T_U8 quality);

T_BOOL Img_YUV2JPEGSave_Mutex(T_U8 *srcY, T_U8 *srcU, T_U8 *srcV, SaveBuffFunc saveFunc,
									T_U32 width, T_U32 height, T_U8 quality);

T_BOOL Img_YUV2JPEG_OSD_Mutex(T_U8 *srcY, T_U8 *srcU, T_U8 *srcV, T_U8 *dstJPEG, T_U32 *size,
								T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth, T_U32 dstHeight, T_U8 quality, 
								J_OSD_Info *osdinfo);

T_BOOL 	Img_YUV2JPEG_Stamp_Mutex(const T_U8 *srcY, const T_U8 *srcU,
					const T_U8 *srcV, T_U8 *dstJPEG, T_U32 *size,
					T_U32 width, T_U32 height, T_U8 quality,
					J_STAMP_Info *stampinfo);

#endif	// _FWL_IMAGELIB_H_

