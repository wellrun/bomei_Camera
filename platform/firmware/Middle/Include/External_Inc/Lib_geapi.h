/**
* @FILENAME lib_geapi.h
* @BRIEF Graphic Effects API Reference
* Copyright (C) 2007 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR Tommy Lau (Liu Huaiguang)
* @DATE 2007-10-12
* @UPDATE 2008-05-22
* @VERSION 4.1
* @REF No reference
*/

#ifndef __GEAPI_H__
#define __GEAPI_H__

#include "../../Library/ge/getypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GE_LIB_VERSION "GELib V0.8.3_SVN292"

typedef void* (*GE_CALLBACK_FUN_MALLOC)(unsigned int size);
typedef void (*GE_CALLBACK_FUN_FREE)(void *memblock);
typedef void* (*GE_CALLBACK_FUN_MEMCPY)(void *dest, const void *src, unsigned int count);
typedef void* (*GE_CALLBACK_FUN_MEMSET)(void *dest, int c, unsigned int count);
typedef int (*GE_CALLBACK_FUN_PRINTF)(const char *format, ...);

typedef GE_INT (*GE_CALLBACK_FUN_IMG_IMAGETYPE)(const GE_BYTE *imgBuf);

typedef GE_BYTE* (*GE_CALLBACK_FUN_IMG_IMAGETOBMP)
 (const GE_BYTE *imgBuf, GE_ULONG bufLen, GE_ULONG *outLen);

typedef GE_BYTE (*GE_CALLBACK_FUN_IMG_WBMPINFO)
 (const GE_BYTE *imgBuf, GE_WORD *width, GE_WORD *height);

typedef	GE_BYTE (*GE_CALLBACK_FUN_IMG_JPEGINFO)
 (const GE_BYTE *imgBuf, GE_ULONG size, GE_WORD *width, GE_WORD *height, GE_BYTE *y_h_samp, GE_BYTE *y_v_samp);

typedef GE_BYTE (*GE_CALLBACK_FUN_IMG_PNGINFO)
 (const GE_BYTE *imgBuf, GE_WORD *width, GE_WORD *height, GE_BYTE *bitCount);

typedef GE_BYTE (*GE_CALLBACK_FUN_IMG_GIFINFO)
 (const GE_BYTE *imgBuf, GE_WORD *width, GE_WORD *height, GE_BYTE *bitCount);

typedef struct _GE_CALLBACK_FUNCS
{
	//	For further standard file support
	/*
	FILE* (*fopen)( const char *filename, const char *mode );
	int (*fclose)(FILE* stream );
	int (*fwrite)( const void *buffer, int size, int count, FILE* stream );
	int (*fread)( void *buffer, int size, int count, FILE* stream );
	int (*fseek)( FILE *stream, long offset, int origin );
	long (*ftell)( FILE *stream );
	int (*fflush)( FILE *stream );
	*/

	//	For standard memory support
	GE_CALLBACK_FUN_MALLOC	malloc;
	GE_CALLBACK_FUN_FREE	free;
	GE_CALLBACK_FUN_MEMCPY	memcpy;
	GE_CALLBACK_FUN_MEMSET	memset;
	GE_CALLBACK_FUN_PRINTF	printf;

	// Image library api
	GE_CALLBACK_FUN_IMG_IMAGETYPE	Img_ImageType;
	GE_CALLBACK_FUN_IMG_IMAGETOBMP	Img_ImageToBmp;
	GE_CALLBACK_FUN_IMG_WBMPINFO	Img_WBMPInfo;
	GE_CALLBACK_FUN_IMG_JPEGINFO	Img_JpegInfo;
	GE_CALLBACK_FUN_IMG_PNGINFO		Img_PNGInfo;
	GE_CALLBACK_FUN_IMG_GIFINFO		Img_GIFInfo;

} GE_CALLBACK_FUNCS;

#if defined(GE_MSVC)				//	Microsoft Visual C++

#include <pshpack1.h>
typedef struct _GE_PIXEL_RGB
{
	//	Red component
	GE_BYTE		r;
	//	Green component
	GE_BYTE		g;
	//	Blue component
	GE_BYTE		b;
} GE_PIXEL_RGB;
#include <poppack.h>

#elif defined(GE_GNUC)				//	GNU C Compiler

typedef struct _GE_PIXEL_RGB
{
	//	Red component
	GE_BYTE		r;
	//	Green component
	GE_BYTE		g;
	//	Blue component
	GE_BYTE		b;
} __attribute__ ((packed)) GE_PIXEL_RGB;

#elif defined(GE_ADS)				//	ARM Develop Suite

typedef __packed struct _GE_PIXEL_RGB
{
	//	Red component
	GE_BYTE		r;
	//	Green component
	GE_BYTE		g;
	//	Blue component
	GE_BYTE		b;
} GE_PIXEL_RGB;

#endif	//	GE_ADS

typedef struct _GE_PIXEL_RGBA
{
	//	Red component
	GE_BYTE		r;
	//	Green component
	GE_BYTE		g;
	//	Blue component
	GE_BYTE		b;
	//	Alpha component
	GE_BYTE		a;
} GE_PIXEL_RGBA;

#if defined(GE_MSVC)				//	Microsoft Visual C++

#include <pshpack1.h>
typedef union _GE_PIXEL
{
	//	A struct contains spare r, g, b components
	GE_PIXEL_RGB	pixel;
	//	A single 32bit variable stores rgb components
	GE_BYTE			rgb[3];
} GE_PIXEL, *GE_PPIXEL;
#include <poppack.h>

#elif defined(GE_GNUC)				//	GNU C Compiler

typedef union _GE_PIXEL
{
	//	A struct contains spare r, g, b components
	GE_PIXEL_RGB	pixel;
	//	A single 32bit variable stores rgb components
	GE_BYTE			rgb[3];
} __attribute__ ((packed)) GE_PIXEL, *GE_PPIXEL;

#elif defined(GE_ADS)				//	ARM Develop Suite

typedef __packed union _GE_PIXEL
{
	//	A struct contains spare r, g, b components
	GE_PIXEL_RGB	pixel;
	//	A single 32bit variable stores rgb components
	GE_BYTE			rgb[3];
} GE_PIXEL, *GE_PPIXEL;

#endif

typedef union _GE_PIXELA
{
	//	A struct contains spare r, g, b, a components
	GE_PIXEL_RGBA	pixel;
	//	A single 32bit variable stores rgba components
	GE_DWORD		rgba;
} GE_PIXELA, *GE_PPIXELA;

typedef struct _GE_BITMAP
{
	//	The width of the bitmap in memory
	GE_DWORD	dwWidth;
	//	The height of the bitmap in memory
	GE_DWORD	dwHeight;
	//	The bitmap is a color key bitmap
	GE_BOOL		bColorKey;
	//	The color key
	GE_PIXEL	pixelColorKey;
	//	The bitmap has alpha channel
	GE_BOOL		bAlphaChannel;
	//	Bitmap data
	GE_PPIXEL	pData;
	//  Bitmap data size
	GE_ULONG	nRGBDataSize;
	//	Alpha data
	GE_BYTE		*pAData;
} GE_BITMAP, *GE_PBITMAP;

typedef struct _GE_RECT
{
	// The x coordinate of the rect's start point
	GE_INT	x;
	// The y coordinate of the rect's start point
	GE_INT	y;
	// The width of the rect
	GE_UINT	width;
	// The height of the rect
	GE_UINT	height;
} GE_RECT, *GE_PRECT;

// 浏览效果枚举值
typedef enum
{
	GE_EFFECT_MODE_ALPHA = 0,		// 淡入淡出 - alpha blend
	GE_EFFECT_MODE_SLIDE_COVER,		// 滑入 - slide show，后一幅图像滑进覆盖前一幅图像
	GE_EFFECT_MODE_SLIDE_PULL,		// 滑入滑出 - slide show，后一幅图滑进的同时前一幅图像滑出
	GE_EFFECT_MODE_WINDOWBLIND,		// 百叶窗
	GE_EFFECT_MODE_DIAMOND,			// 菱形扩展
	GE_EFFECT_MODE_POPUP_EXPAND,	// 弹出式－ 从中心向四周或从四周向中心的矩形扩展
	GE_EFFECT_MODE_POPUP_SCALE,		// 弹出式－ 带缩放的弹出
	GE_EFFECT_MODE_PLANE_TURN,		// 平面翻转
	GE_EFFECT_MODE_CUBIC_TURN,		// 立方体式翻转
	GE_EFFECT_MODE_OPENANDCLOSE_H,	// 水平方向打开或关闭
	GE_EFFECT_MODE_OPENANDCLOSE_V	// 垂直方向打开或关闭
} GE_EFFECT_MODE;

// 浏览效果扩展枚举值
// 指示滑入、翻转等效果的方向
// 弹出的方式 - 从中心向四周或从四周向中心
typedef enum
{
	GE_EFFECT_MODE_EXTRA_NONE = 0,
	GE_EFFECT_MODE_EXTRA_DIREC_LEFT,
	GE_EFFECT_MODE_EXTRA_DIREC_RIGHT,
	GE_EFFECT_MODE_EXTRA_DIREC_UP,
	GE_EFFECT_MODE_EXTRA_DIREC_DOWN,
	GE_EFFECT_MODE_EXTRA_DIREC_VERTICAL,
	GE_EFFECT_MODE_EXTRA_DIREC_HORIZONTAL,
	GE_EFFECT_MODE_EXTRA_CENTER_TO_EDGE,
	GE_EFFECT_MODE_EXTRA_EDGE_TO_CENTER
} GE_EFFECT_MODE_EXTRA;

/**
* @BRIEF	Initialize the Graphic Effects Engine
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-12
* @PARAM	GE_VOID
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Initialize OK.
*/
GE_RESULT GE_API GE_Initialize(GE_VOID);

/**
* @BRIEF	Setup the callback functions
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-20
* @PARAM	pcbf - The struct pointing to the callback functions
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Setup OK.
*/
GE_RESULT GE_API GE_SetCallback(const GE_CALLBACK_FUNCS *pcbf);

/**
* @BRIEF	Setup a color key of a specific bitmap
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-12
* @PARAM	pBitmap - The bitmap in memory to setup color key
*			pixel - The color key
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Setup OK.
*/
GE_RESULT GE_API GE_SetColorKey(GE_PBITMAP pBitmap, GE_PIXEL pixel);

/**
* @BRIEF	Create a new bitmap buffer from exist memory
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-12
* @PARAM	pBuffer - The buffer which stores the image data
*			ulLength - The buffer's length
*			pBitmapDest	- The pointer of the result bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_CreateBitmapFromMemory(GE_BYTE *pBuffer, GE_ULONG ulLength, GE_PBITMAP pBitmapDest);

/**
* @BRIEF	Create a new bitmap buffer from exist memory with scaling
*			scaling method -- bilinear interpolation method
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-10-18
* @PARAM	pBuffer - The buffer which stores the image data
*			ulLength - The buffer's length
*			dwWidth - The width of the window
*			dwHeight - The height of the window
*			pBitmapDest	- The pointer of the result bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_CreateBitmapFromMemoryEx(GE_BYTE *pBuffer, GE_ULONG ulLength, GE_DWORD dwWidth, GE_DWORD dwHeight, GE_PBITMAP pBitmapDest);

/**
* @BRIEF	Create a new bitmap buffer from exist memory with scaling
*			scaling method -- nearest neighbor interpolation method
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-11-16
* @PARAM	pBuffer - The buffer which stores the image data
*			ulLength - The buffer's length
*			dwWidth - The width of the window
*			dwHeight - The height of the window
*			pBitmapDest	- The pointer of the result bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_CreateBitmapFromMemoryEx2(GE_BYTE *pBuffer, GE_ULONG ulLength, GE_DWORD dwWidth, GE_DWORD dwHeight, GE_PBITMAP pBitmapDest);

/**
* @BRIEF	Create a new bitmap from an exist 32bit buffer
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-10-15
* @PARAM	dwWidth - The width of the exist image
*			dwHeight - The height of the exist image
*			pBuffer - The buffer of the exist image
*			pBitmapDest - The pointer of the result bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Setup OK.
*/
GE_RESULT GE_API GE_CreateBitmapFromBuffer32(GE_DWORD dwWidth, GE_DWORD dwHeight, GE_BYTE *pBuffer, GE_PBITMAP pBitmapDest);

/**
* @BRIEF	Create a new bitmap from an exist 24bit buffer
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-10-15
* @PARAM	dwWidth - The width of the exist image
*			dwHeight - The height of the exist image
*			pBuffer - The buffer of the exist image
*			pBitmapDest - The pointer of the result bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Setup OK.
*/
GE_RESULT GE_API GE_CreateBitmapFromBuffer24(GE_DWORD dwWidth, GE_DWORD dwHeight, GE_BYTE *pBuffer, GE_PBITMAP pBitmapDest);

/**
* @BRIEF	Free the memory that a bitmap hold
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-13
* @PARAM	pBitmap - The pointer to the bitmap needs to be free
* @RETURN	GE_RESULT
* @RETVAL	Greater or equal 0 - Success
*			Less than 0 - Fail
*/
GE_RESULT GE_API GE_DestroyBitmap(GE_PBITMAP *pBitmap);

/**
* @BRIEF	Draw bitmap
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-12
* @PARAM	x - The x offset
*			y - The y offset
*			pBitmapSrc - The source bitmap
*			pBitmapDest - The destination bitmap
*			bIsColorKey - To use color key or not
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_DrawBitmap(GE_INT x, GE_INT y, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest, GE_BOOL bIsColorKey);

/**
* @BRIEF	Draw bitmap rect
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-10-29
* @PARAM	x - The x offset
*			y - The y offset
*			rect - The selected rect area in source bitmap
*			pBitmapSrc - The source bitmap
*			pBitmapDest - The destination bitmap
*			bIsColorKey - To use color key or not
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_DrawBitmapRect(GE_INT x, GE_INT y, GE_RECT rect, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest, GE_BOOL bIsColorKey);

/**
* @BRIEF	Draw bitmap with alpha
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-12
* @PARAM	x - The x offset
*			y - The y offset
*			pBitmapSrc - The source bitmap
*			pBitmapDest - The destination bitmap
*			nAlpha - Alpha value
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_DrawBitmapAlpha(GE_INT x, GE_INT y, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest, GE_INT nAlpha);

/**
* @BRIEF	Draw bitmap with alpha channel
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-12
* @PARAM	x - The x offset
*			y - The y offset
*			pBitmapSrc - The source bitmap
*			pBitmapDest - The destination bitmap
*			nAlpha - Alpha value
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_DrawBitmapAlphaChannel(GE_INT x, GE_INT y, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest, GE_INT nAlpha);

/**
* @BRIEF	Draw bitmap with additive
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-12
* @PARAM	x - The x offset
*			y - The y offset
*			pBitmapSrc - The source bitmap
*			pBitmapDest - The destination bitmap
*			nAlpha - Alpha value
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_DrawBitmapAdditive(GE_INT x, GE_INT y, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest, GE_INT nAlpha);

/**
* @BRIEF	Draw bitmap with attenuation
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-12
* @PARAM	x - The x offset
*			y - The y offset
*			pBitmapSrc - The source bitmap
*			pBitmapDest - The destination bitmap
*			nAlpha - Alpha value
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_DrawBitmapAttenuation(GE_INT x, GE_INT y, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest, GE_INT nAlpha);

/* For further use
GE_RESULT GE_API GE_DrawBitmapZoom(GE_INT x, GE_INT y, GE_INT nScaleX, GE_INT nScaleY, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest);

GE_RESULT GE_API GE_DrawBitmapRotate(GE_INT x, GE_INT y, GE_INT nAngle, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest);

GE_RESULT GE_API GE_DrawBitmapTile(GE_INT x, GE_INT y, GE_INT nTileX, GE_INT nTileY, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest);
*/


/**
* @BRIEF	Bitmap rotate
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-14
* @PARAM	nAngle - anticlockwise if (nAngle > 0)
*					 clockwise if (nAngle < 0)
*			pBitmapSrc - The source bitmap
*			pBitmapDest - The destination bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_CopyBitmapRotate(GE_INT nAngle, GE_PBITMAP pBitmapSrc, GE_PBITMAP *pBitmapDest);

/**
* @BRIEF	Bitmap rotate
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-10-29
* @PARAM	nAngle - 0、90、180、270, clockwise
*			pBitmapSrc - The source bitmap
*			pBitmapDest - The destination bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_CopyBitmapRotateEx(GE_UINT nAngle, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest);

/**
* @BRIEF	Bitmap zoom in and zoom out
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-14
* @PARAM	nDstW - Width of the result bitmap, in pixel
*			nDstH - Height of the result bitmap, in pixel
*			pBitmapSrc - The source bitmap
*			pBitmapDest - The destination bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_CopyBitmapZoom(GE_DWORD nDstW, GE_DWORD nDstH, GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest);

/**
* @BRIEF	Copy bitmap of horizontal mirror
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-14
* @PARAM	pBitmapSrc - The source bitmap
*			pBitmapDest - Pointer of the destination bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_CopyBitmapHMirror(GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest);

/**
* @BRIEF	Copy bitmap of vertical mirror
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-14
* @PARAM	pBitmapSrc - The source bitmap
*			pBitmapDest - Pointer of the destination bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_CopyBitmapVMirror(GE_PBITMAP pBitmapSrc, GE_PBITMAP pBitmapDest);

/**
* @BRIEF	Get 16 bit 5-6-5 format rgb buffer from bitmap
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-28
* @PARAM	pBitmap - The source bitmap
*			nBufSize - Size of the memory that pBuffer pointer holded
*			pBuffer - Pointer of the 16 bit rgb data
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_GetRGB16Buffer(GE_PBITMAP pBitmap, GE_ULONG nBufSize, GE_BYTE *pBuffer);

/**
* @BRIEF	Get 24 bit rgb buffer from bitmap
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-28
* @PARAM	pBitmap - The source bitmap
*			nBufSize - Size of the memory that pBuffer pointer holded
*			pBuffer - Pointer of the 24 bit rgb data
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_GetRGB24Buffer(GE_PBITMAP pBitmap, GE_ULONG nBufSize, GE_BYTE *pBuffer);

/**
* @BRIEF	Get 32 bit rgb buffer from bitmap
* @AUTHOR	Tommy Lau (Liu Huaiguang)
* @AUTHOR	Zhang Hong
* @DATE		2007-09-28
* @PARAM	pBitmap - The source bitmap
*			pBuffer - Pointer of the 32 bit rgb data
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
//GE_RESULT GE_API GE_GetRGB32Buffer(GE_PBITMAP pBitmap, GE_BYTE **pBuffer);

/**
* @BRIEF	Create a bitmap struct, return the pointer alloced in the function
*			Call GE_DestroyBitmap to free the memory after usage of the pBitmap pointer
* @AUTHOR	Zhang Hong
* @DATE		2008-01-08
* @PARAM	width - Width of the bitmap
*			height - Height of the bitmap
*			bAlphaChannel - TRUE or FALSE
*			pBitmapt - Pointer to the bitmap stream
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_CreateBitmap(GE_ULONG width, GE_ULONG height, GE_BOOL bAlphaChannel, GE_PBITMAP *pBitmap);

/**
* @BRIEF	Generate temporary bitmap of multiple browse mode
* @AUTHOR	Zhang Hong
* @DATE		2008-01-08
* @PARAM	pBitmapPre - Pointer to the previous bitmap
*			pBitmapNext - Pointer to the next bitmap
*			pBitmapTmp - Pointer to the temporary bitmap that created in the function
*			nStep		- Step value
*			mode		- Mode of the effect
*			extra		- Extra info of the effect mode
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_BrowseEffect(GE_PBITMAP pBitmapPre, GE_PBITMAP pBitmapNext, GE_PBITMAP pBitmapTmp, GE_ULONG nStep, GE_EFFECT_MODE mode, GE_EFFECT_MODE_EXTRA extra);

/**
* @BRIEF	Get width、height and alpha channel info of a image file
* @AUTHOR	Zhang Hong
* @DATE		2008-01-30
* @PARAM	pBuffer - Pointer to the image data (bmp/wbmp/jpg/png/gif)
*			nSize - Size of the image data buffer
*			bitmapW - width of the bitmap
*			bitmapH - height of the bitmap
*			bAlphaChannel - TRUE of FALSE
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_GetBitmapInfo(GE_BYTE *pBuffer, GE_ULONG nSize, GE_WORD *bitmapW, GE_WORD *bitmapH, GE_BOOL *bALphaChannel);

/**
* @BRIEF	Create a new bitmap buffer from exist memory with scaling
*			scaling method -- bilinear interpolation method
* @AUTHOR	Zhang Hong
* @DATE		2008-02-13
* @PARAM	pBuffer - The buffer which stores the image data
*			ulLength - The buffer's length
*			dwWidth - The width of the result bitmap
*			dwHeight - The height of the result bitmap
*			pBitmapDest	- The pointer of the result bitmap
* @RETURN	GE_RESULT
* @RETVAL	GE_SUCCESS - Success
*/
GE_RESULT GE_API GE_CreateBitmapWithSpecificWH(GE_BYTE *pBuffer, GE_ULONG ulLength, GE_DWORD dwWidth, GE_DWORD dwHeight, GE_PBITMAP pBitmapDest);

/**
* @BRIEF	Get the current lib version info
* @AUTHOR	Zhang Hong
* @DATE		2008-03-06
* @PARAM	GE_VOID
* @RETURN	GE_CHAR*
* @RETVAL	Pointer to the version info string
*/
GE_CHAR* GE_GetVersionInfo(GE_VOID);

#ifdef __cplusplus
}
#endif

#endif	//	__GEAPI_H__
