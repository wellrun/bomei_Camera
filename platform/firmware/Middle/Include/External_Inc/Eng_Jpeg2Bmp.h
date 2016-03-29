/**
 * @file jpeg2bmp.h
 * @brief Convert jpeg image to BMP image or RGB data.
 * reorganized by ZouMai in 2004
 * Copyright (C) 2001 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author WangYanFei
 * @date 2001-08-01
 * @version 1.0
 * @ref
 */

/*********************************************************
    The following is an example to use JPEG 2 BMP APIs

    jpgRet = JpegInit(JpgData, JPEG_TO_BMP, &Width, &Height);
    if (jpgRet <= 0)
    {
        return;
    }
    jpgRet = JpegSetDstSize((T_U16)(Width/2), (T_U16)(Height/2));
    if (jpgRet <= 0)
    {
        return;
    }
    BmpBuff = malloc(jpgRet);
    if (BmpBuff == AK_NULL)
    {
        return;
    }
    if (JpegDecode(BmpBuff) <= 0)
    {
        free(BmpBuff);
        return;
    }
    ... ...

*********************************************************/

#ifndef  __JPEG_2_BMP_H__
#define  __JPEG_2_BMP_H__

#include "anyka_types.h"

/* --------------------------- Define JPEG API error code --------------------------- */
#define JPEG_OK             0           /* success */
#define JPEG_PARM_ERROR     -1          /* parameter error */
#define JPEG_DATA_ERROR     -2          /* not jpeg data */
#define JPEG_INIT_ERROR     -3          /* init jpeg tag error */
#define JPEG_FORMAT_ERROR   -4          /* format error */
#define JPEG_SAMPLE_ERROR   -5          /* sample size error */
#define JPEG_DST_TYPE_ERROR -6          /* error destination type, must be JPEG_TO_BMP, JPEG_TO_RGB or JPEG_TO_BGR */
#define JPEG_NOT_INIT       -7          /* JPEG has not been initialized */
#define JPEG_MALLOC_ERROR   -8          /* malloc error */

/* --------------------------- Define destination type --------------------------- */
#define JPEG_TO_BMP         0           /* convert JPEG to Bitmap */
#define JPEG_TO_RGB         1           /* convert JPEG to RGB format data */
#define JPEG_TO_BGR         2           /* convert JPEG to BRG format data */
#define JPEG_TO_RGBLINE     3           /* convert JPEG to RGB line data */
/* --------------------------- Bitmap header type definition --------------------------- */
#ifndef __arm
#pragma pack(1)
#endif
typedef struct tagBITMAPFILEHEADER
{
    T_U16   bfType;
    T_U32   bfSize;
    T_U16   bfReserved1;
    T_U16   bfReserved2;
    T_U32   bfOffBits;
} BITMAPFILEHEADER;

typedef  struct tagBITMAPINFOHEADER
{
    T_U32   biSize;
    T_S32   biWidth;
    T_S32   biHeight;
    T_U16   biPlanes;
    T_U16   biBitCount;
    T_U32   biCompression;
    T_U32   biSizeImage;
    T_S32   biXPelsPerMeter;
    T_S32   biYPelsPerMeter;
    T_U32   biClrUsed;
    T_U32   biClrImportant;
} BITMAPINFOHEADER;

#ifndef __arm
#pragma pack()
#endif
/**
 * @brief initialize jpeg for decoding
 * User must call this function before call function JpegSetDstSize() or JpegDecode()
 * @author WangYanFei
 * @date 2001-08-01
 * @param T_U8 *JpgData: the jpeg data
 * @param T_U8 dstType: destination type, must be JPEG_TO_BMP, JPEG_TO_RGB or JPEG_TO_BGR
 * @param T_U16 *Width: image width for return
 * @param T_U16 *Height: image height for return
 * @return T_S32
 * @retval > 0: means success, and the returned value is the buffer size user should allocate
 * @retval < 0: error code
 */
T_S32 JpegInit(T_U8 *JpgData, T_U8 dstType, T_U16 *Width, T_U16 *Height);

/**
 * @brief free the handle
 * @author songmengxing
 * @date 2011-10-25
 * @param void
 * @return void
 */
T_VOID JpegFree(T_VOID);

/**
 * @brief reset destination BMP or RGB image size
 * If user want not to chage image size, user needn't to call this function
 * Function JpegInit() must be called before call this fucntion
 * @author WangYanFei
 * @date 2001-08-01
 * @param T_U16 dstWidth: new destination image width
 * @param T_U16 dstHeight: new destination image height
 * @return T_S32
 * @retval > 0: means success, and the returned value is the buffer size user should allocate
 * @retval < 0: error code
 */
T_S32 JpegSetDstSize(T_U16 dstWidth, T_U16 dstHeight);

/**
 * @brief convert JPEG data to BMP or RGB data
 * User must allocate memory for dstBuf before call this function
 * Function JpegInit() must be called before call this fucntion
 * @author WangYanFei
 * @date 2001-08-01
 * @param T_U8 *dstBuf: buffer for storage destination image(BMP or RGB)
 * @return T_S32
 * @retval > 0: means success, and the returned value is the image size
 * @retval < 0: error code
 */
T_S32 JpegDecode(T_U8 *dstBuf);

/**
 * @brief decode a block line from JPEG data
 * User must allocate memory for dstBuf before call this function
 * Function JpegInit() must be called before call this fucntion
 * @author WangYanFei
 * @date 2001-08-01
 * @param T_U8 *dstBuf: buffer for storage destination image(BMP or RGB)
 * @return T_S32
 * @retval > 0: means success, and the returned number of decoded lines 
 * @retval < 0: error code
 */
T_S32 JpegDecodeLine(T_U8 *dstBuf);

#endif // __JPEG_2_BMP_H__
