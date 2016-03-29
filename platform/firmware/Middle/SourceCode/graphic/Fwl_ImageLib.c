#include "anyka_types.h"
#include "AKdefine.h"
#include "Akos_api.h"
#include "Lib_Image_api.h"
#include "Fwl_ImageLib.h"

static T_hSemaphore img_semaphore = 0;

static T_S32 Img_Obtain_Semaphore(T_VOID)
{
	if (0 == img_semaphore)
	{
		img_semaphore = AK_Create_Semaphore(1, AK_PRIORITY);
	}
	
	return AK_Obtain_Semaphore(img_semaphore, AK_SUSPEND);
}

static T_S32 Img_Release_Semaphore(T_VOID)
{
	if (0 == img_semaphore)
	{
		return -1;
	}
	
	return AK_Release_Semaphore(img_semaphore);
}

T_U8 *Img_PNG2BMP_Mutex(const T_U8 *pngData, T_U32 *size)
{
	T_U8* pData = AK_NULL;

	Img_Obtain_Semaphore();
	pData = Img_PNG2BMP(pngData, size);
	Img_Release_Semaphore();

	return pData;
}

T_BOOL Img_YUV2JPEG_Mutex(T_U8 *srcY, T_U8 *srcU, T_U8 *srcV, T_U8 *dstJPEG,
							T_U32 *size, T_U32 width, T_U32 height, T_U8 quality)
{
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
 	ret = Img_YUV2JPEG(srcY, srcU, srcV, dstJPEG, size, width, height, quality);
	Img_Release_Semaphore();

	return ret;
}

T_U8 *Img_Jpeg2BMP_Mutex(const T_U8 *jpegData, T_U32 *size)
{
	T_U8* pData = AK_NULL;

	Img_Obtain_Semaphore();
	pData = Img_Jpeg2BMPSoft(jpegData, size);
	Img_Release_Semaphore();

	return pData;
}

T_U8 *Img_Jpeg2BMPEx_Mutex(const T_U8 *jpegData, T_U16 dstWidth, T_U16 dstHeight, T_U32 *size)
{
	
	T_U8* pData = AK_NULL;
	
	Img_Obtain_Semaphore();
	pData = Img_Jpeg2BMPExSoft(jpegData, dstWidth, dstHeight, size);
	Img_Release_Semaphore();
	
	return pData;
}

T_BOOL Img_JPEG2YUV_Mutex(T_U8 *srcJPEG, T_U32 size, T_U8 *dstYUV, T_S32 *width, T_S32 *height)
{
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_JPEG2YUV(srcJPEG, size, dstYUV, width, height);
	Img_Release_Semaphore();

	return ret;
}

T_BOOL Img_VideoJPEG2YUV_Mutex(T_U8 *srcJPEG, T_U32 size, T_U8 *dstYUV, T_S32 *width, T_S32*height)
{	
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_VideoJPEG2YUV(srcJPEG, size, dstYUV, width, height);
	Img_Release_Semaphore();

	return ret;
}

T_BOOL Img_JPEG2YUV4x_Mutex(T_U8 *srcJPEG, T_U32 size, T_U8 *dstYUV, T_S32 *width, T_S32*height)
{	
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_JPEG2YUV4x(srcJPEG, size, dstYUV, width, height);
	Img_Release_Semaphore();

	return ret;
}                          

T_BOOL Img_JPEG2YUVEx_Mutex(T_U8 *srcJPEG, T_U32 size, T_U8 *dstYUV, T_U16 dstWidth, T_U16 dstHeight)
{	
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_JPEG2YUVEx(srcJPEG, size, dstYUV, dstWidth, dstHeight);
	Img_Release_Semaphore();

	return ret;
}                          

T_BOOL Img_Jpeg2RGB_Mutex(const T_U8 *srcJPEG, T_U8 *dstRGB, T_U32 *width, T_U32 *height, T_U32 size)
{	
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_Jpeg2RGB(srcJPEG, dstRGB, width, height, size);
	Img_Release_Semaphore();

	return ret;
}                        

T_BOOL Img_YUV2JPEG4x_Mutex(T_U8 *srcYUV, T_U8 *dstJPEG, T_U32 *size, T_U32 width, T_U32 height, T_U8 quality)
{	
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_YUV2JPEG4x(srcYUV, dstJPEG, size, width, height, quality);
	Img_Release_Semaphore();

	return ret;
}

T_BOOL Img_YUV2JPEGEx_Mutex(T_U8 *srcYUV, T_U8 *dstJPEG, T_U32 *size,
                          T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth, T_U32 dstHeight, T_U8 quality)
{	
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_YUV2JPEGEx(srcYUV, dstJPEG, size, srcWidth, srcHeight, dstWidth, dstHeight, quality);
	Img_Release_Semaphore();

	return ret;
}

T_BOOL Img_YUV2JPEGExs_Mutex(T_U8 *srcY, T_U8 *srcU, T_U8 *srcV, T_U8 *dstJPEG, T_U32 *size,
								T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth, T_U32 dstHeight, T_U8 quality)
{	
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_YUV2JPEGExs(srcY, srcU, srcV, dstJPEG, size, srcWidth, srcHeight, dstWidth, dstHeight, quality);
	Img_Release_Semaphore();

	return ret;
}

T_BOOL Img_YUV2JPEG_OSD_Mutex(T_U8 *srcY, T_U8 *srcU, T_U8 *srcV, T_U8 *dstJPEG, T_U32 *size,
								T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth, T_U32 dstHeight, T_U8 quality, J_OSD_Info *osdinfo)
{	
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_YUV2JPEG_OSD(srcY, srcU, srcV, dstJPEG, size, srcWidth, srcHeight, dstWidth, dstHeight, quality, osdinfo);
	Img_Release_Semaphore();

	return ret;
}


T_BOOL Img_YUV2JPEGSave_Mutex(T_U8 *srcY, T_U8 *srcU, T_U8 *srcV, SaveBuffFunc saveFunc, 
								T_U32 width, T_U32 height, T_U8 quality)
{	
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_YUV2JPEGSave(srcY, srcU, srcV, saveFunc, width, height, quality);
	Img_Release_Semaphore();

	return ret;
}

T_BOOL 	Img_YUV2JPEG_Stamp_Mutex(const T_U8 *srcY, const T_U8 *srcU,
					const T_U8 *srcV, T_U8 *dstJPEG, T_U32 *size,
					T_U32 width, T_U32 height, T_U8 quality,
					J_STAMP_Info *stampinfo)
{
	T_BOOL ret = AK_FALSE;

	Img_Obtain_Semaphore();
	ret = Img_YUV2JPEG_Stamp(srcY, srcU, srcV, dstJPEG, size, width, height,quality,stampinfo);
	Img_Release_Semaphore();

	return ret;

}

// End of File
