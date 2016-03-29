/**
 * @file eng_akbmp.h
 * @brief This header file is for BMP process function prototype
 * 
 */


#ifndef __ENG_AK_BMP_H__
/**
 * @def __ENG_AK_BMP_H__
 *
 */
#define __ENG_AK_BMP_H__

/**
 * @include "Gbl_Global.h"
 *
 */
#include "Gbl_Global.h"
#include "fwl_display.h"

///BMP structure
typedef struct {
	T_U8		Frame;	/**< Frame number of animation, picture frame is 1 */
	T_LEN		Width;  /**< bmp width*/
	T_LEN		Height; /**< bmp height*/
	T_U8		Deep;	/**< color deep, color number = Power(2, Deep) */
	T_U8		*BmpData;/**< bmp data */
} T_AK_BMP;

/** @defgroup BMP Bitmap drawing interface 
 * @ingroup ENG
 */
/*@{*/
/**
 * @brief Draw  AKBmp format bitmap 
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-29
 * @param[in] hLayer		handle of layer
 * @param[in] x			x position the AKBmp would be drawn 
 * @param[in] y			y position the AKBmp would be drawn,  the coordinate(x,y) relative to the left-top of LCD
 * @param[in] AkBmp		the source AKbmp would be drawn
 * @param[in] bkColor	The transparent color. If a dot with this color appears in bmp, it will not be drawn.
 * @param[in] Reverse	if AK_TRUE, reverse draw the source pixel
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @note if akcolor, reverse are AK_NULL, AK_FALSE, this function would run most quickly
 * @retval
 */
T_BOOL	AkBmpDraw(HLAYER hLayer, T_POS x, T_POS y, const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse);
/**
 * @brief Draw AKBmp format bitmap in normal mode, which is full part, non-transparent and non-reverse .
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-29
 * @param[in] hLayer		handle of layer
 * @param[in] x			x position the AKBmp would be drawn 
 * @param[in] y			y position the AKBmp would be drawn,  the coordinate(x,y) relative to the left-top of LCD
 * @param[in] AkBmp		the source AKbmp would be drawn
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @retval
 */
T_BOOL	AkBmpDrawBack(HLAYER hLayer, T_POS x, T_POS y, const T_AK_BMP *AkBmp);
/**
 * @brief Draw AKBmp format bitmap in part
 *
 * @author @b LiaoJianhua
 *
 * @author
 * @date 2005-12-29
 * @param[in] hLayer		handle of layer
 * @param[in] x			x position the AKBmp would be drawn 
 * @param[in] y			y position the AKBmp would be drawn,  the coordinate(x,y) relative to the left-top of LCD
 * @param[in] range		The part rect of AKBmp would be drawn, the range rect coordinate relative to the left-top of AKBmp image
 * @param[in] AkBmp		The source AKbmp would be drawn.
 * @param[in] bkColor	The transparent color.If a dot with this color appears in bmp, it will not be drawn.
 * @param[in] Reverse	if AK_TRUE, reverse draw the source pixel
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @note if range, color, reverse are AK_NULL, AK_NULL, AK_FALSE, this function would run most quickly
 * @retval
 */
T_BOOL	Fwl_AkBmpDrawPart(HLAYER layer, T_POS x, T_POS y, T_RECT *range,
				const T_AK_BMP *AkBmp, T_COLOR *bkColor, T_BOOL Reverse);


/**
 * @brief Draw partial BMP from ANYKA BMP string to the screen
 * 
 * @author ZouMai
 * @date 2002-09-03
 * @param T_POS x	 show rect left-top-point.x of LCD 
 * @param T_POS y	 show rect left-top-point.y of LCD 
 * @param T_RECT *rang: range of the partial BMP(Big BMP map left-top-point as (0,0) )
 * @param T_pCDATA bmpStream: ANYKA BMP string
 * @param T_COLOR bkColor: transparent color
 * @param T_BOOL Reverse: reverse or not
 * @return T_BOOL
 * @retval 
 */

T_BOOL	Fwl_AkBmpDrawPartFromString(HLAYER layer, T_POS x, T_POS y, T_RECT *range,
                T_pCDATA BmpStream, T_COLOR *bkColor, T_BOOL Reverse);



/**
 * @brief Dram BMP from BMP data string 
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-09-03
 * @param T_POS x
 * @param  T_POS y
 * @param  T_pDATA BmpString
 * @return T_BOOL
 * @retval 
 */

T_BOOL	Fwl_AkBmpDrawFromString(HLAYER layer, T_POS x, T_POS y, T_pCDATA BmpStream,
                    T_COLOR *bkColor, T_BOOL Reverse);


/**
 * @brief Draw BMP from BMP data string in normal mode, which is full part, non-transparent and non-reverse .
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-09-03
 * @param[in] LCD		LCD ID
 * @param[in] x			x position the AKBmp would be drawn 
 * @param[in] y			y position the AKBmp would be drawn,  the coordinate(x,y) relative to the left-top of LCD
 * @param[in] BmpString	A string stores bmp data info.
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 * @retval 
 */
T_BOOL	AkBmpDrawBackFromString(HLAYER hLayer, T_POS x, T_POS y, T_pCDATA BmpString);

/**
 * @brief Load BMP data from string
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-09-03
 * @param[in]	BmpString	A string stores bmp data info.
 * @param[out]  AnykaBmp	A T_AK_BMP struct to be modified.
 * @return a T_AK_BMP struct which contains bmp info.
 * @retval 
 */
T_AK_BMP	*AkBmpGetFromString(T_pCDATA BmpString, T_AK_BMP *AnykaBmp);
/**
 * @brief Dram BMP with frame pattern. !!!The function can only support one-color deep bmp.
 * 
 * @author @b MiaoBaoli
 * 
 * @author 
 * @date 2002-09-03
 * @param[in] LCD		LCD ID
 * @param[in] x			x position the AKBmp would be drawn 
 * @param[in] y			y position the AKBmp would be drawn,  the coordinate(x,y) relative to the left-top of LCD
 * @param[in] AkBmp		The source AKbmp would be drawn.
 * @param[in] color		Dot with this color will have a frame.
 * @param[in] frameColor	The frame color.
 * @retval AK_TRUE	success
 * @retval AK_FALSE	fail
 */
T_BOOL AkBmpDrawWithFrame( HLAYER  hLayer, T_POS x, T_POS y, const T_AK_BMP *AkBmp, T_COLOR color, T_COLOR frameColor );
#if 0
T_BOOL AkBmpGetFromBmpFile(T_pSTR filename, T_pDATA buffer, T_LEN *Width, T_LEN *Height);
T_BOOL AkBmpGetFromOtaFile(T_pSTR otafilename, T_pDATA buffer, T_LEN *Width, T_LEN *Height);
#endif
/**
* @brief get the info of the bmp data string
* 
* @author  Zhuobin Li
* @date 2005-12-16
* @param[in]  BmpString		The bmp string.
* @param[out]  pWidth		The pointer for return the width of the bmp. 
* if the pointer is AK_NULL, don't return the value of the width.

* @param[out] pHeight		The pointer for return the Height of the bmp. 
* if the pointer is AK_NULL, don't return the value of the Height.

* @param[out] pDeep			The pointer for return the Deep of the bmp. 
* if the pointer is AK_NULL, don't return the value of the Deep.

* @return T_U8 
* @retval return the frame of the bmp(by the frame, width and height have 2 bytes or 1 byte).
* @retval 
*/
T_U8 AKBmpGetInfo(T_pCDATA BmpString, T_LEN *pWidth, T_LEN *pHeight, T_U8 *pDeep);
/*@}*/

/*
    get standard windows-bmp info.
*/

T_VOID StdBmpGetInfo(T_pCDATA StdBmpString, T_LEN *pWidth, T_LEN *pHeight, T_U8 *pDeep);


T_BOOL Fwl_AKBmpAlphaShow(T_U8 *srcBuf, T_U32 srcBufW, T_RECT srcRect,
						  T_U8 *dstBuf, T_U32 dstBufW, T_RECT dstRect,T_U8 alpha);


#endif


