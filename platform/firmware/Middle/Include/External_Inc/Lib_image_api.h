#ifndef __IMAGE_API_H__
#define __IMAGE_API_H__

/**
* @FILENAME image_api.h
* @BRIEF image lib api  
* Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR xie_zhishan
* @DATE 2006-10-10
* @UPDATE 2010-05-28
* @VERSION 2.12.01
* @REF jpeg codec design spec.doc & AK3223/AK3224 programmer guide
*/
#include "Eng_callback.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IMAGE_LIB_VERSION "ImageLib V2.12.03"

/** 图像库使用说明：
 >> 图像库初始化
    在使用图像库的函数接口之前，必须要按照如下方法进行初始化：
    (1)使用SetImageChipID()函数设置所使用的芯片类型(参见本文件中的定义)
    (2)使用Img_SetCallbackFuns)()函数设置回调函数
    (3)使用Img_SetFlushCacheFunc()函数设置Cache刷新函数
    示例代码：
    CB_FUNS cb_funs;
    SetImageChipID(AK3224_JPEG_VER);
    memset(&cb_funs, 0, sizeof(cb_funs));
    cb_funs.malloc = NewMalloc;
    cb_funs.free   = NewFree;
    cb_funs.printf = DebugOutput;
    Img_SetCallbackFuns(&cb_funs);
    Img_SetFlushCacheFunc(Cache_FlushFunc);

 >> 获取图像库版本号
    使用Img_GetVersionInfo()函数可以获取图像库的版本号，返回值是一个字符串。
 
 >> 获取出错代码
    使用Img_GetLastError()函数可以获取最后一次出错的出错代码，出借代码的定义参见
    本文件中的T_IMAGE_ERROR枚举类型定义。

 >> 图像编解码
    参见本文件中的函数声明部分， 其中，<in>表示参数是输入参数，<out>表示参数是输
出参数，<in,out>表示参数是输入输出参数。更详细的信息请参阅图像库接口说明文档。

 >> JPEG图像编码时的EXIF信息设置
    (1)通过Img_EnableExif()函数使能/禁止JPEG编码时嵌入EXIF信息。
    (2)通过Img_GetExifInfo()函数获取JPEG编码时嵌入的EXIF信息。
    (3)通过Img_SetExifInfo()函数设置JPEG编码时嵌入的EXIF信息。
    示例代码：
    T_ExifInfo exifInfo;                        // 数据结构定义见下
    char MyDateTime[20] = "2008:10:22 16:18:54";

    Img_EnableExif(AK_TRUE);                    // 允许JPEG编码时嵌入EXIF信息
    Img_GetExifInfo(&exifInfo);                 // 获取JPEG编码时嵌入的EXIF信息

    exifInfo.ImageDescription[0] = '\0';        // 禁止嵌入 ImageDescription 信息项
    strcpy((char*)exifInfo.Make, "Anyka New");  // 注意字符串不要超长
    strcpy((char*)exifInfo.Model, "AK3224M");
    // 不修改原有的 Artist 和 Software 信息项
    strcpy((char*)exifInfo.DateTime, MyDateTime);
    exifInfo.bThumbnail = AK_TRUE;              // 允许大于800*600的图片嵌入缩略图
    Img_SetExifInfo(&exifInfo);                 // 设置JPEG编码时嵌入的EXIF信息
 */

typedef unsigned long        IMG_T_U32;
typedef signed long          IMG_T_S32;
typedef unsigned short       IMG_T_U16;
typedef unsigned char        IMG_T_U8;
typedef signed char          IMG_T_S8;
typedef unsigned char        IMG_T_BOOL;
typedef const signed char *  IMG_T_pCSTR;
typedef void                 IMG_T_VOID;


#if 0
typedef IMG_T_U32 (*CALLBACK_FUN_FREAD)
 (unsigned int hFile, void *pBuffer, unsigned int count); 

typedef IMG_T_U32 (*CALLBACK_FUN_FWRITE)
 (unsigned int hFile, void *pBuffer, unsigned int count); 

typedef IMG_T_U32 (*CALLBACK_FUN_FSEEK)
 (unsigned int hFile, int offset, unsigned char origin); 

typedef IMG_T_U32 (*CALLBACK_FUN_FGETLEN)(unsigned int hFile); 

typedef IMG_T_U32 (*CALLBACK_FUN_FTELL)(unsigned int hFile); 

typedef struct {
    IMG_T_U16    ResourceID;            
    IMG_T_U8    *Buff;      //The Pointer to save the resource.
                            // (Application should malloc the memory for it) 
    IMG_T_U32    Resource_len;    
} T_LOADRESOURCE_CB_PARA;
//if Buff is NULL, and Resource_len is 0, a failure have occurred

typedef void (*CALLBACK_FUN_LOADRESOURCE)(T_LOADRESOURCE_CB_PARA *pPara); 

typedef void (*CALLBACK_FUN_RELEASERESOURCE)(IMG_T_U8 *Buff); 

typedef void* (*CALLBACK_FUN_MALLOC)
 (IMG_T_U32 size, char *filename, IMG_T_U32 line); 

typedef void* (*CALLBACK_FUN_FREE)(void* mem); 

typedef void* (*CALLBACK_FUN_REMALLOC)(void* mem, IMG_T_U32 size); 

typedef void* (*CALLBACK_FUN_DMAMEMCOPY)
 (void* dst, void* src, IMG_T_U32 count); 

typedef void (*CALLBACK_FUN_SHOWFRAME)
 (void* srcImg, IMG_T_U32 src_width, IMG_T_U32 src_height); 

typedef void (*CALLBACK_FUN_CAMERASHOWFRAME)
 (void* srcImg, IMG_T_U32 src_width, IMG_T_U32 src_height); 

typedef void (*CALLBACK_FUN_CAPSTART)(void); 

typedef IMG_T_BOOL (*CALLBACK_FUN_CAPCOMPLETE)(void); 

typedef void* (*CALLBACK_FUN_CAPGETDATA)(void); 

typedef IMG_T_U32 (*CALLBACK_FUN_GETTICKCOUNT)(void); 

typedef IMG_T_S32 (*CALLBACK_FUN_PRINTF)(IMG_T_pCSTR format, ...); 

typedef IMG_T_BOOL (*CALLBACK_FUN_REGMODIFY)(IMG_T_U32 addr, IMG_T_U32 value, IMG_T_U32 mask); 

typedef struct{
    CALLBACK_FUN_FREAD               fread;
    CALLBACK_FUN_FWRITE              fwrite;
    CALLBACK_FUN_FSEEK               fseek;
    CALLBACK_FUN_FGETLEN             fgetlen;
    CALLBACK_FUN_FTELL               ftell;
    CALLBACK_FUN_LOADRESOURCE        LoadResource;
    CALLBACK_FUN_RELEASERESOURCE     ReleaseResource;
    CALLBACK_FUN_MALLOC              malloc;
    CALLBACK_FUN_FREE                free;
    CALLBACK_FUN_REMALLOC            remalloc;
    CALLBACK_FUN_DMAMEMCOPY          DMAMemcpy;
    CALLBACK_FUN_SHOWFRAME           ShowFrame;
    CALLBACK_FUN_CAMERASHOWFRAME     CameraShowFrame;
    CALLBACK_FUN_CAPSTART            CapStart;
    CALLBACK_FUN_CAPCOMPLETE         CapComplete;
    CALLBACK_FUN_CAPGETDATA          CapGetData;
    CALLBACK_FUN_GETTICKCOUNT        GetTickCount;
    CALLBACK_FUN_PRINTF              printf;
    CALLBACK_FUN_REGMODIFY		regModify;
} CB_FUNS;
#endif

typedef IMG_T_VOID (*FlushCacheFunc)(IMG_T_VOID);

typedef IMG_T_VOID (*SaveBuffFunc)(IMG_T_U8 *buff, IMG_T_U32 size);

typedef IMG_T_VOID (*JPEGTaskFunc)(IMG_T_VOID);

#define IMG_INVALID_HANDLE 0

typedef enum tagIMAGETYPE
{
    IMG_IMAGE_UNKNOWN,
    IMG_IMAGE_BMP,
    IMG_IMAGE_WBMP,
    IMG_IMAGE_JPG,
    IMG_IMAGE_GIF,
    IMG_IMAGE_PNG
}T_IMAGE_TYPE;


// gif decoder mode
typedef enum
{
    MIN_SPACE,
    MAX_SPEED
}GIF_DEC_MODE;

// error type
typedef enum
{
    IMG_NO_ERROR,
    IMG_INPUT_NULL_POINTER,
    IMG_PARAMETER_ERROR,
    IMG_STREAM_ERROR,
    IMG_NOT_ENOUGH_MOMORY,
    IMG_NOT_SUPPORT_FORMAT
}T_IMAGE_ERROR;

typedef enum
{
    JPEG_HW_VER1,
    JPEG_HW_VER2,
    JPEG_HW_VER3,
    JPEG_HW_VER4,
    JPEG_HW_VER5,
    JPEG_HW_VER6
} JPEG_HW_VERSION;

#define     AK3221_JPEG_VER                  JPEG_HW_VER1
#define     AK3223_JPEG_VER                  JPEG_HW_VER1
#define     AK3224_JPEG_VER                  JPEG_HW_VER1
#define     AK3225_JPEG_VER                  JPEG_HW_VER1
#define     AK3610_JPEG_VER                  JPEG_HW_VER1
#define     AK3620_JPEG_VER                  JPEG_HW_VER1
#define     AK322L_JPEG_VER                  JPEG_HW_VER2
#define     AK3225L_JPEG_VER                 JPEG_HW_VER2
#define     AK3631L_JPEG_VER                 JPEG_HW_VER2
#define     AK3671_JPEG_VER                  JPEG_HW_VER2
#define     AK3810_JPEG_VER                  JPEG_HW_VER3
#define     AK7801_JPEG_VER                  JPEG_HW_VER3
#define     AK7802_JPEG_VER                  JPEG_HW_VER3
#define     AK980x_JPEG_VER                  JPEG_HW_VER4
#define     AK37xx_JPEG_VER                  JPEG_HW_VER5
#define     AK37xC_JPEG_VER                  JPEG_HW_VER6


typedef struct
{
    IMG_T_U8    ImageDescription[80];
    IMG_T_U8    Make[30];
    IMG_T_U8    Model[20];
    IMG_T_U8    Artist[20];
    IMG_T_U8    Software[40];
    IMG_T_U8    DateTime[20];           // "YYYY:MM:DD hh:mm:ss"
    IMG_T_BOOL  bThumbnail;
} T_ExifInfo;

typedef struct
{
	IMG_T_U16 osdWidth; 	// OSD宽度，必须是16的倍数
	IMG_T_U16 osdHeight;	// OSD高度，必须是16的倍数
	IMG_T_U16 osd_H_offset;	// OSD相对于目标背景图的水平方向偏移，必须是16的倍数
	IMG_T_U16 osd_V_offset;	// OSD相对于目标背景图的垂直方向偏移，必须是16的倍数
	IMG_T_U16 *osdRGB565;   // OSD的RGB565数据，存储顺序(R,G,B)；其中0值表示透明即背景色	
	IMG_T_U8  alpha;        // OSD和背景色混合度，取值范围[0,16]。0：透明(背景色)；16：不透明（前景色）

}J_OSD_Info;


// for VERRSION 6, AK37C
typedef struct
{
	IMG_T_U16 stampWidth; 		// stamp宽度，必须是16的倍数
	IMG_T_U16 stampHeight;		// stamp高度，必须是16的倍数
	IMG_T_U16 stamp_H_offset;	// stamp相对于目标背景图的水平方向偏移，必须是16的倍数
	IMG_T_U16 stamp_V_offset;	// stamp相对于目标背景图的垂直方向偏移，必须是16的倍数
	IMG_T_U8 *stampYptr;   	// stamp的Y数据指针YUV三个值全0即透明
	IMG_T_U8 *stampUptr;   	// stamp的U数据指针YUV三个值全0即透明
	IMG_T_U8 *stampVptr;   	// stamp的V数据指针YUV三个值全0即透明
	
}J_STAMP_Info;




/******************************************************************************
 * Miscellaneous Function
 ******************************************************************************/

/** 设置当前芯片的使用的JPEG硬件解码版本
* @PARAM ver <in>JPEG硬件解码版本(取值见本文件)
*/
IMG_T_VOID SetImageChipID(JPEG_HW_VERSION ver);

/** 设定回调函数
* @PARAM pCBFuns <in>函数指针结构体的指针
*/
IMG_T_VOID Img_SetCallbackFuns(const CB_FUNS *pCBFuns);

/** 设置Cache刷新函数
* @PARAM FlushCacheFunc <in>Cache刷新函数的指针
*/
IMG_T_VOID Img_SetFlushCacheFunc(const FlushCacheFunc func);

/** 设置JPEG并行工作任务函数
* @PARAM JPEGTaskFunc <in>与JPEG模块并行工作的函数的指针
*/
IMG_T_VOID Img_SetJPEGTaskFunc(const JPEGTaskFunc func);

/** 获取图像库版本信息
* @RETURN 图像库版本信息字符串
*/
IMG_T_S8 *Img_GetVersionInfo(IMG_T_VOID);

/** 获得最后一次出错的出错代码
* @RETURN 出错代码的枚举值(参见本文件中的T_IMAGE_ERROR枚举类型定义)
*/
T_IMAGE_ERROR Img_GetLastError(IMG_T_VOID);

/** 取得图片数据的图片格式类型
* @PARAM imgBuf <in>图片数据的缓冲区指针
* @RETURN 图片类型枚举值(参见本文件中的T_IMAGE_TYPE枚举类型定义)
*/
T_IMAGE_TYPE Img_ImageType(const IMG_T_U8 *imgBuf);

/** 将图片解码成BMP格式
* @PARAM imgBuf <in>图片数据的缓冲区指针
* @PARAM bufLen <in>图片数据的缓冲区长度
* @PARAM outLen <out>结果BMP数据的字节长度
* @RETURN 解码成功, 返回BMP格式图像数据；解码失败, 返回AK_NULL, outLen值无效
*/
IMG_T_U8 *Img_ImageToBmp(const IMG_T_U8 *imgBuf, IMG_T_U32 bufLen, IMG_T_U32 *outLen);

/** 将YUV数据转换为RGB数据
* @PARAM srcY <in>Y数据指针
* @PARAM srcU <in>U数据指针
* @PARAM srcV <in>V数据指针
* @PARAM RGB <out>RGB数据指针
* @PARAM srcWidth <in>源图像的宽度
* @PARAM srcHeight <in>源图像的高度
* @PARAM dstWidth <in>目的图像的宽度
* @PARAM dstHeight <in>目的图像的高度
* @PARAM timeout <in>设定的超时值（3210芯片有效）
* @RETURN 0：转换正确；负值：转换失败
* @COMMENT 输入YUV格式：H211（AK3810、AK78xx芯片则为YUV 420）
*          源图像尺寸必须大于目的图像尺寸
*/
IMG_T_S32 Img_YUV2RGB(IMG_T_U8 *srcY, IMG_T_U8 *srcU, IMG_T_U8 *srcV, IMG_T_U8 *RGB,
                      IMG_T_S32 srcWidth, IMG_T_S32 srcHeight , IMG_T_S32 dstWidth,
                      IMG_T_S32 dstHeight, IMG_T_S32 timeout);


/******************************************************************************
 * WBMP Decode Function
 ******************************************************************************/

/** 取得WBMP格式图片的宽度和高度信息
* @PARAM buffer <in>WBMP格式图片的缓冲区指针
* @PARAM width <out>WBMP格式图片宽度
* @PARAM height <out>WBMP格式图片高度
* @RETURN AK_TRUE：成功；AK_FALSE：失败，width、height中的值无效
*/
IMG_T_BOOL Img_WBMPInfo(const IMG_T_U8 *buffer, IMG_T_U16 *width, IMG_T_U16 *height);

/** 将WBMP格式图片解码成BMP格式图片
* @PARAM buffer <in>WBMP格式的源数据缓冲区指针
* @PARAM length <in>WBMP格式的源数据字节长度
* @PARAM outLen <out>结果BMP数据的字节长度
* @RETURN 返回结果BMP数据缓冲区指针，解码失败则返回AK_NULL
*/
IMG_T_U8 *Img_WBMP2BMP(const IMG_T_U8 *buffer, IMG_T_U32 length, IMG_T_U32 *outLen);


/******************************************************************************
 * JPEG Decode Function
 ******************************************************************************/

/** 取得对应Jpeg图片的宽度和高度信息及亮度的垂直和水平采样率
* @PARAM JpegData <in>Jpeg的源数据指针
* @PARAM size <in>Jpeg数据长度
* @PARAM width <out>返回Jpeg源数据对应图片的宽度
* @PARAM height <out>返回Jpeg源数据对应图片的高度
* @PARAM y_h_sam <out>返回亮度的水平采样率
* @PARAM y_v_samp <out>返回亮度的垂直采样率
* @RETURN AK_TRUE：成功；AK_FALSE：失败，width、height、y_h_sam、y_v_sam中的值无效
* @COMMENT 如果不需要获取采样率的信息，可将传入参数y_h_samp和y_v_samp设为AK_NULL
*/
IMG_T_BOOL Img_JpegInfo(const IMG_T_U8 *jpegData, IMG_T_U32 size, IMG_T_U16 *width,
                        IMG_T_U16 *height, IMG_T_U8 *y_h_samp, IMG_T_U8 *y_v_samp);

/** 将Jpeg图片解码成BMP格式（硬件解码）
* @PARAM jpegData <in>Jpeg的源数据指针
* @PARAM size <in,out>输入：Jpeg数据长度；输出：返回解码后的BMP数据长度
* @RETURN 返回结果BMP数据缓冲区指针，解码失败则返回AK_NULL
* @COMMENT 返回的BMP数据指针在不用时必须将其释放，否则会有内存泄漏
*/
IMG_T_U8 *Img_Jpeg2BMP(const IMG_T_U8 *jpegData, IMG_T_U32 *size);

/** 将Jpeg图片解码成BMP格式（软件解码）
* @PARAM jpegData <in>Jpeg的源数据指针
* @PARAM size <in,out>输入：Jpeg数据长度；输出：返回解码后的BMP数据长度
* @RETURN 返回结果BMP数据缓冲区指针，解码失败则返回AK_NULL
* @COMMENT 返回的BMP数据指针在不用时必须将其释放，否则会有内存泄漏
*/
IMG_T_U8 *Img_Jpeg2BMPSoft(const IMG_T_U8 *jpegData, IMG_T_U32 *size);

/** 将JpegJPEG图片解码成BMP格式，任意比例缩小（硬件解码）
* @PARAM jpegData <in>Jpeg的源数据指针
* @PARAM dstWidth <in>指定解码后的BMP图片宽度
* @PARAM dstHeight <in>指定解码后的BMP图片高度
* @PARAM size <in,out>输入：Jpeg数据长度；输出：返回解码后的BMP数据长度
* @RETURN 返回结果BMP数据缓冲区指针，解码失败则返回AK_NULL
* @COMMENT 返回的BMP数据指针在不用时必须将其释放，否则会有内存泄漏
*/
IMG_T_U8 *Img_Jpeg2BMPEx(const IMG_T_U8 *jpegData, IMG_T_U16 dstWidth,
                         IMG_T_U16 dstHeight, IMG_T_U32 *size);

/** 将Jpeg图片解码成BMP格式，任意比例缩小（软件解码）
* @PARAM jpegData <in>Jpeg的源数据指针
* @PARAM dstWidth <in>指定解码后的BMP图片宽度
* @PARAM dstHeight <in>指定解码后的BMP图片高度
* @PARAM size <in,out>输入：Jpeg数据长度；输出：返回解码后的BMP数据长度
* @RETURN 返回结果BMP数据缓冲区指针，解码失败则返回AK_NULL
* @COMMENT 返回的BMP数据指针在不用时必须将其释放，否则会有内存泄漏
*/
IMG_T_U8 *Img_Jpeg2BMPExSoft(const IMG_T_U8 *jpegData, IMG_T_U16 dstWidth,
                             IMG_T_U16 dstHeight, IMG_T_U32 *size);


/** 将Jpeg图片数据解码成YUV数据，并获得图片宽高（硬件解码）,解码后的YUV格式与JPEG文件中定义的格式一致
* @PARAM srcJPEG <in>Jpeg的源数据指针
* @PARAM size <in>Jpeg数据长度
* @PARAM dstYUV <out>解码后的YUV数据地址
* @PARAM width <out>Jpeg图片宽度
* @PARAM height <out>Jpeg图片高度
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT dstYUV指向的YUV数据缓冲区必须在外部预先分配好
*/
IMG_T_BOOL Img_JPEG2YUV(IMG_T_U8 *srcJPEG, IMG_T_U32 size, IMG_T_U8 *dstYUV,
                        IMG_T_S32 *width, IMG_T_S32*height);

/** 将来源于MJPEG视频中的Jpeg数据解码成YUV数据，并获得图片宽高（硬件解码），解码后的YUV格式与平台定义的格式一致
* @PARAM srcJPEG <in>Jpeg的源数据指针
* @PARAM size <in>Jpeg数据长度
* @PARAM dstYUV <out>解码后的YUV数据地址
* @PARAM width <out>Jpeg图片宽度
* @PARAM height <out>Jpeg图片高度
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT dstYUV指向的YUV数据缓冲区必须在外部预先分配好
*/
IMG_T_BOOL Img_VideoJPEG2YUV(IMG_T_U8 *srcJPEG, IMG_T_U32 size, IMG_T_U8 *dstYUV,
                        IMG_T_S32 *width, IMG_T_S32*height);

/** 将Jpeg图片数据解码成YUV数据，并获得图片宽高（软件解码）
* @PARAM srcJPEG <in>Jpeg的源数据指针
* @PARAM size <in>Jpeg数据长度
* @PARAM dstYUV <out>解码后的YUV数据地址
* @PARAM width <out>Jpeg图片宽度
* @PARAM height <out>Jpeg图片高度
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT dstYUV指向的YUV数据缓冲区必须在外部预先分配好
*/
IMG_T_BOOL Img_JPEG2YUVSoft(IMG_T_U8 *srcJPEG, IMG_T_U32 size, IMG_T_U8 *dstYUV,
                            IMG_T_S32 *width,IMG_T_S32*height);

/** 将Jpeg图片数据解码成YUV数据，宽高各缩小1/2，并获得缩小后的图片宽高（硬件解码）
* @PARAM srcJPEG <in>Jpeg的源数据指针
* @PARAM size <in>Jpeg数据长度
* @PARAM dstYUV <out>解码后的YUV数据地址
* @PARAM width <out>Jpeg图片宽度
* @PARAM height <out>Jpeg图片高度
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT dstYUV指向的YUV数据缓冲区必须在外部预先分配好
*/
IMG_T_BOOL Img_JPEG2YUV4x(IMG_T_U8 *srcJPEG, IMG_T_U32 size, IMG_T_U8 *dstYUV,
                          IMG_T_S32 *width, IMG_T_S32*height);

/** 将Jpeg图片数据解码成YUV数据，任意比例缩小（硬件解码）
* @PARAM srcJPEG <in>Jpeg的源数据指针
* @PARAM size <in>Jpeg数据长度
* @PARAM dstYUV <out>解码后的YUV数据地址
* @PARAM dstWidth <in>指定解码后的Jpeg图片宽度
* @PARAM dstHeight <in>指定解码后的Jpeg图片高度
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT dstYUV指向的YUV数据缓冲区必须在外部预先分配好
*/
IMG_T_BOOL Img_JPEG2YUVEx(IMG_T_U8 *srcJPEG, IMG_T_U32 size, IMG_T_U8 *dstYUV,
                          IMG_T_U16 dstWidth, IMG_T_U16 dstHeight);

/** 将Jpeg图片解码成不带文件头的BMP格式（BGR24，带4字节填充，Bottom-Up）（硬件解码）
* @PARAM jpegData <in>Jpeg的源数据指针
* @PARAM width <out>Jpeg图片宽度
* @PARAM height <out>Jpeg图片高度
* @PARAM size <in,out>输入：Jpeg数据长度；输出：返回解码后的BMP数据长度
* @RETURN 返回结果BMP数据缓冲区指针，解码失败则返回AK_NULL
* @COMMENT dstRGB指向的RGB数据缓冲区必须在外部预先分配好
*/
IMG_T_BOOL Img_Jpeg2RGB(const IMG_T_U8 *srcJPEG, IMG_T_U8 *dstRGB, IMG_T_U32 *width,
                        IMG_T_U32 *height, IMG_T_U32 size);

/** 解码JPEG文件中的缩略图（硬件编码）
* @PARAM jpegData <in>Jpeg的源数据指针
* @PARAM size <in,out>输入：Jpeg数据长度；输出：返回解码后的BMP缩略图的数据长度
* @PARAM width <out>缩略图的宽度
* @PARAM height <out>缩略图的高度
* @RETURN 返回结果BMP数据缓冲区指针，解码失败(或缩略图不存在)则返回AK_NULL
* @COMMENT 返回的BMP数据指针在不用时必须将其释放，否则会有内存泄漏
*/
IMG_T_U8 *Img_JpegThumbnail(const IMG_T_U8 *jpegData, IMG_T_U32 *size,
                            IMG_T_U16 *width, IMG_T_U16 *height);

/** 往JPEG文件中嵌入缩略图（硬件编解码）
* @PARAM jpegData <in>Jpeg的源数据指针
* @PARAM size <in,out>输入：Jpeg数据长度；输出：返回解码后的BMP缩略图的数据长度
* @RETURN 返回结果JPEG数据缓冲区指针，失败则返回AK_NULL
* @COMMENT 返回的Jpeg数据指针在不用时必须将其释放，否则会有内存泄漏
*/
IMG_T_U8 *Img_EmbedThumbnail(const IMG_T_U8 *jpegData, IMG_T_U32 *size);

/** 使能 / 禁止 JPEG 编码时嵌入EXIF信息
* @PRAM enable <in> JPEG编码时是否加入EXIF信息
* @RETURN 无
*/
IMG_T_VOID Img_EnableExif(IMG_T_BOOL enable);

/** 获取JPEG编码时嵌入的EXIF信息
* @PARAM exifInfo: 指向T_ExifInfo类型的结构体的指针
* @RETURN none
*/
IMG_T_VOID Img_GetExifInfo(T_ExifInfo* exifInfo);

/** 设置JPEG编码时嵌入的EXIF信息
* @PARAM exifInfo: 指向T_ExifInfo类型的结构体的指针
* @RETURN none
*/
IMG_T_VOID Img_SetExifInfo(T_ExifInfo* exifInfo);


/******************************************************************************
 * JPEG Encode Function
 ******************************************************************************/

/** 将YUV数据编码成Jpeg图片数据（硬件编码）
* @PARAM srcY <in>Y数据指针
* @PARAM srcU <in>U数据指针
* @PARAM srcV <in>V数据指针
* @PARAM dstJPEG <out>Jpeg数据指针
* @PARAM size <in,out>输入：dstJPEG缓冲大小；输出：Jpeg长度
* @PARAM width <in>图像宽度
* @PARAM height <in>图像高度
* @PARAM quality <in>编码质量，取值范围0-200
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT 输入YUV格式：H211（AK3810、AK78xx芯片则为YUV 420）
*/
IMG_T_BOOL Img_YUV2JPEG(IMG_T_U8 *srcY, IMG_T_U8 *srcU, IMG_T_U8 *srcV,
                        IMG_T_U8 *dstJPEG, IMG_T_U32 *size, IMG_T_U32 width,
                        IMG_T_U32 height, IMG_T_U8 quality);


/** 将YUV数据编码成Jpeg图片数据（软件编码）
* @PARAM srcY <in>Y数据指针
* @PARAM srcU <in>U数据指针
* @PARAM srcV <in>V数据指针
* @PARAM dstJPEG <out>Jpeg数据指针
* @PARAM size <in,out>输入：dstJPEG缓冲大小；输出：Jpeg长度
* @PARAM width <in>图像宽度
* @PARAM height <in>图像高度
* @PARAM quality <in>编码质量，取值范围0-200
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT 输入YUV格式：H211（AK3810、AK78xx芯片则为YUV 420）
*/
IMG_T_BOOL Img_YUV2JPEGSoft(IMG_T_U8 *srcY, IMG_T_U8 *srcU, IMG_T_U8 *srcV,
                            IMG_T_U8 *dstJPEG, IMG_T_U32 *size, IMG_T_U32 width,
                            IMG_T_U32 height, IMG_T_U8 quality);

IMG_T_BOOL Img_YUV2JPEGExSoft(IMG_T_U8 *srcY, IMG_T_U8 *srcU, IMG_T_U8 *srcV,
                           IMG_T_U8 *dstJPEG, IMG_T_U32 *size,
                           IMG_T_U32 srcWidth, IMG_T_U32 srcHeight, IMG_T_U32 dstWidth,
                           IMG_T_U32 dstHeight, IMG_T_U8 quality);

/** 将YUV数据编码成Jpeg图片数据，水平垂直方向各放大2倍（硬件编码）
* @PARAM srcY <in>YUV数据指针
* @PARAM dstJPEG <out>Jpeg数据指针
* @PARAM size <in,out>输入：dstJPEG缓冲大小；输出：Jpeg长度
* @PARAM width <in>源图像宽度
* @PARAM height <in>源图像高度
* @PARAM quality <in>编码质量，取值范围0-200
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT 输入YUV格式：H211（AK3810、AK78xx芯片则为YUV 420）
*/
IMG_T_BOOL Img_YUV2JPEG4x(IMG_T_U8 *srcYUV, IMG_T_U8 *dstJPEG,
                    IMG_T_U32 *size, IMG_T_U32 width, IMG_T_U32 height,
                    IMG_T_U8 quality);

/** 将YUV数据编码成Jpeg图片数据，任意比例缩放（硬件编码）
* @PARAM srcY <in>YUV数据指针
* @PARAM dstJPEG <out>Jpeg数据指针
* @PARAM size <in,out>输入：dstJPEG缓冲大小；输出：Jpeg长度
* @PARAM srcWidth <in>源图像宽度
* @PARAM srcHeight <in>源图像高度
* @PARAM dstWidth <in>目标图像宽度
* @PARAM dstHeight <in>目标图像高度
* @PARAM quality <in>编码质量，取值范围0-200
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT 输入YUV格式：H211（AK3810、AK78xx芯片则为YUV 420）
*/
IMG_T_BOOL Img_YUV2JPEGEx(IMG_T_U8 *srcYUV, IMG_T_U8 *dstJPEG, IMG_T_U32 *size,
                          IMG_T_U32 srcWidth, IMG_T_U32 srcHeight, IMG_T_U32 dstWidth,
                          IMG_T_U32 dstHeight, IMG_T_U8 quality);

/** 将YUV数据编码成Jpeg图片数据，任意比例缩放（硬件编码）
* @PARAM srcY <in>Y数据指针
* @PARAM srcU <in>U数据指针
* @PARAM srcV <in>V数据指针
* @PARAM dstJPEG <out>Jpeg数据指针
* @PARAM size <in,out>输入：dstJPEG缓冲大小；输出：Jpeg长度
* @PARAM srcWidth <in>源图像宽度
* @PARAM srcHeight <in>源图像高度
* @PARAM dstWidth <in>目标图像宽度
* @PARAM dstHeight <in>目标图像高度
* @PARAM quality <in>编码质量，取值范围0-200
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT 输入YUV格式：H211（AK3810、AK78xx芯片则为YUV 420）
*/
IMG_T_BOOL Img_YUV2JPEGExs(IMG_T_U8 *srcY, IMG_T_U8 *srcU, IMG_T_U8 *srcV,
                           IMG_T_U8 *dstJPEG, IMG_T_U32 *size,
                           IMG_T_U32 srcWidth, IMG_T_U32 srcHeight, IMG_T_U32 dstWidth,
                           IMG_T_U32 dstHeight, IMG_T_U8 quality);

IMG_T_BOOL Img_YUV2JPEGSave(IMG_T_U8 *srcY, IMG_T_U8 *srcU, IMG_T_U8 *srcV,
                            SaveBuffFunc saveFunc, IMG_T_U32 width, IMG_T_U32 height,
                            IMG_T_U8 quality);

 /** 将YUV数据编码成Jpeg图片数据，支持10倍内任意放大；
  **  支持2倍内任意缩小，支持OSD(硬件缩放编码)
* @PARAM srcY <in>Y数据指针
* @PARAM srcU <in>U数据指针
* @PARAM srcV <in>V数据指针
* @PARAM dstJPEG <out>Jpeg数据指针
* @PARAM size <in,out>输入：dstJPEG缓冲大小；输出：Jpeg长度
* @PARAM srcWidth <in>源图像宽度取值范围[16, 2046]
* @PARAM srcHeight <in>源图像高度取值范围[16, 2046]
* @PARAM dstWidth <in>目标图像宽度取值范围[96, 2046]
* @PARAM dstHeight <in>目标图像高度取值范围[96, 2046]
* @PARAM quality <in>编码质量，取值范围0-200
* @PARAM osdinfo<in>OSD相关信息
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT 输入YUV格式：Sundance3E芯片，必须YUV411
*/                           
IMG_T_BOOL 	Img_YUV2JPEG_OSD(const IMG_T_U8 *srcY, const IMG_T_U8 *srcU,
					const IMG_T_U8 *srcV, IMG_T_U8 *dstJPEG, IMG_T_U32 *size,IMG_T_U32 srcWidth, IMG_T_U32 srcHeight, 
					IMG_T_U32 dstWidth,IMG_T_U32 dstHeight,IMG_T_U8 quality,
					J_OSD_Info *osdinfo);



 

 /** 将YUV数据编码成Jpeg图片数据
* @PARAM srcY <in>Y数据指针
* @PARAM srcU <in>U数据指针
* @PARAM srcV <in>V数据指针
* @PARAM dstJPEG <out>Jpeg数据指针
* @PARAM size <in,out>输入：dstJPEG缓冲大小；输出：Jpeg长度
* @PARAM width <in>源图像宽度取值范围[16, 4096]
* @PARAM height <in>源图像高度取值范围[16, 4096]
* @PARAM quality <in>编码质量，取值范围0-200
* @PARAM stampinfo<in>stamp相关信息
* @RETURN AK_TRUE：成功；AK_FALSE：失败
* @COMMENT 输入YUV格式：AK37C芯片，Stamp数据是YUV420
*/                           
IMG_T_BOOL 	Img_YUV2JPEG_Stamp(const IMG_T_U8 *srcY, const IMG_T_U8 *srcU,
					const IMG_T_U8 *srcV, IMG_T_U8 *dstJPEG, IMG_T_U32 *size,
					IMG_T_U32 width, IMG_T_U32 height, IMG_T_U8 quality,
					J_STAMP_Info *stampinfo);





/******************************************************************************
 * GIF Decode Function
 ******************************************************************************/
typedef IMG_T_S32 T_hGIFENUM;


/** 设置GIF解码模式：MAX_SPEED（默认）或MIN_SPACE
* @PARAM mode <in>设置的GIF解码模式
*/
IMG_T_VOID GIFEnum_SetDecMode(GIF_DEC_MODE mode);

/** 获取GIF信息
* @PARAM GIFbuf <in>GIF图像数据
* @PARAM width <out>GIF图像宽度
* @PARAM height <out>GIF图像高度
* @PARAM bitCount <out>GIF图像颜色深度
* @RETURN AK_TRUE：成功；AK_FALSE：失败，width、height、bitCount中的值无效
*/
IMG_T_BOOL Img_GIFInfo(const IMG_T_U8 *GIFbuf, IMG_T_U16 *width, IMG_T_U16 *height,
                       IMG_T_U8 *bitCount);

/** 创建GIF图片解码HANDLE
* @PARAM GIFbuf <in>GIF格式图片的源缓冲区指针
* @PARAM buflength <in>GIF格式图片的源缓冲区长度
* @RETURN 成功则返回有效的HANDLE，失败则返回IMG_INVALID_HANDLE
* @COMMENT 创建HANDLE成功, GIFbuf所指向内存会一直被HANDLE引用直到HANDLE被Close，
*          因此在HANDLE被Close之前GIFbuf所指向的内存不能被释放
*/
T_hGIFENUM GIFEnum_New(const IMG_T_U8 *GIFbuf, IMG_T_S32 buflength);

/** 关闭GIF解码HANDLE
* @PARAM gifEnum <in>用GIFEnum_New创建的HANDLE
*/
IMG_T_VOID GIFEnum_Close(T_hGIFENUM gifEnum);

/** 取得HANDLE对应GIF图片的总帧数
* @PARAM gifEnum <in>用GIFEnum_New创建的HANDLE
* @RETURN GIF图片的总帧数
*/
IMG_T_U16  GIFEnum_GetFrameCount(T_hGIFENUM gifEnum);

/** 解码GIF第一帧图片
* @PARAM gifEnum <in>用GIFEnum_New创建的HANDLE
* @RETURN AK_TRUE：成功；AK_FALSE：失败
*/
IMG_T_BOOL GIFEnum_FirstFrame(T_hGIFENUM gifEnum);

/** 解码GIF下一帧图片
* @PARAM gifEnum <in>用GIFEnum_New创建的HANDLE
* @RETURN AK_TRUE：成功；AK_FALSE：失败
*/
IMG_T_BOOL GIFEnum_NextFrame(T_hGIFENUM gifEnum);

/** 取得GIF解码HANDLE当前帧的BMP
* @PARAM gifEnum <in>用GIFEnum_New创建的HANDLE
* @PARAM dataLen <out>当前帧的BMP数据长度
* @PARAM bitsPerPix <out>当前帧的BMP色深值
* @RETURN 返回当前帧的BMP数据指针，解码失败则返回AK_NULL
*/
const IMG_T_U8 *GIFEnum_GetCurBMP(T_hGIFENUM gifEnum, IMG_T_U32 *dataLen,
                                  IMG_T_U8 *bitsPerPix);

/** 解码HANDLE对应GIF数据指定帧的BMP并返回
* @PARAM gifEnum <in>用GIFEnum_New创建的HANDLE
* @PARAM packetIdx <in>指定的帧号
* @PARAM outLen <out>指定帧的BMP数据长度
* @PARAM bitsPerPix <out>指定帧的BMP色深值
* @RETURN 返回当指定帧的BMP数据指针，解码失败则返回AK_NULL
*/
const IMG_T_U8 *GIFEnum_GetFrameBMP(T_hGIFENUM gifEnum, IMG_T_U16 packetIdx,
                                    IMG_T_U32 *outLen, IMG_T_U8 *bitsPerPix);

/** 取得GIF解码HANDLE当前帧的帧序号
* @PARAM gifEnum <in>用GIFEnum_New创建的HANDLE
* @RETURN 当前帧的帧序号
*/
IMG_T_U16  GIFEnum_GetCurPacket(T_hGIFENUM gifEnum);

/** 取得GIF解码HANDLE当前帧的延时时间(ms)
* @PARAM gifEnum <in>用GIFEnum_New创建的HANDLE
* @RETURN 当前帧的动画延时
*/
IMG_T_S32  GIFEnum_GetCurDelay(T_hGIFENUM gifEnum);

/** 更改当前帧的延时时间(ms)
* @PARAM gifEnum <in>用GIFEnum_New创建的HANDLE
* @PARAM curDelay <in>延时时间, 单位为ms
* @COMMENT 不推荐使用该接口来更改GIF帧的动画延时
*/
IMG_T_VOID GIFEnum_SetCurDelay(T_hGIFENUM gifEnum, IMG_T_S32 curDelay);

/** 计算GIF所有帧的BMP总大小
* @PARAM gifEnum <in>用GIFEnum_New创建的HANDLE
* @RETURN GIF所有帧的BMP总大小
*/
IMG_T_U32  GIFEnum_GetTotalSize(T_hGIFENUM gifEnum);


/******************************************************************************
 * PNG Function
 ******************************************************************************/

/** 获取PNG信息
* @PARAM pngbuf <in>PNG格式的源数据缓冲区指针
* @PARAM width <out>PNG图片宽度
* @PARAM height <out>PNG图片高度
* @PARAM bitCount <out>PNG图片颜色深度
* @RETURN AK_TRUE：成功；AK_FALSE：失败，width、height、bitCount中的值无效
*/
IMG_T_BOOL Img_PNGInfo(const IMG_T_U8 *pngbuf, IMG_T_U16 *width, IMG_T_U16 *height,
                       IMG_T_U8 *bitCount);

/** 将PNG格式图片解码成BMP格式图片
* @PARAM pngbuf	<in>PNG格式的源数据缓冲区指针
* @PARAM outLen	<out>解码成功返回结果BMP数据字节长度
* @RETURN 返回结果BMP数据缓冲区指针，解码失败则返回AK_NULL
* @COMMENT 返回的BMP数据指针在不用时必须将其释放，否则会有内存泄漏
*/
IMG_T_U8 *Img_PNG2BMP(const IMG_T_U8 *pngbuf, IMG_T_U32 *outLen);


/** 将PNG图片解码成BMP格式，任意比例缩小(软件解码)
* @PARAM pngbuf	<in>PNG格式的源数据缓冲区指针
* @PARAM outLen	<out>解码成功返回结果BMP数据字节长度
* @PARAM dstWidth <in>指定解码后的BMP图片宽度
* @PARAM dstHeight <in>指定解码后的BMP图片高度
* @RETURN 返回结果BMP数据缓冲区指针，解码失败则返回AK_NULL
* @COMMENT 返回的BMP数据指针在不用时必须将其释放，否则会有内存泄漏
*/
IMG_T_U8 *Img_PNG2BMPEx(const IMG_T_U8 *pngbuf, IMG_T_U32 *outLen,IMG_T_U16 dstWidth, IMG_T_U16 dstHeight);



/** 将BMP格式图片解码成PNG格式图片
* @PARAM png_buff	<out>PNG格式的数据缓冲区指针
* @PARAM png_len <in,out>输入：PNG缓冲大小；输出：编码后的PNG数据长度
* @PARAM bmp_buff <in>BMP格式的源数据缓冲指针
* @RETURN AK_TRUE：成功；AK_FALSE：失败
*/
IMG_T_BOOL Img_BMP2PNG(IMG_T_U8 *png_buff, IMG_T_U32 *png_len, const IMG_T_U8 *bmp_buff);


#ifdef __cplusplus
}
#endif

#endif    // #ifndef __IMAGE_API_H__
