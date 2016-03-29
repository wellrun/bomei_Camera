/**
 * @file Eng_pfDisplay.h
 * @brief This header file is for display function prototype
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @version 1.0
 */

#ifndef __FWL_PF_DISPLAY_H__
#define __FWL_PF_DISPLAY_H__

#include "anyka_types.h"
#include "eng_graph.h"
#include "fwl_display.h"


#define LCD0_WIDTH				MAIN_LCD_WIDTH
#define LCD0_HEIGHT				MAIN_LCD_HEIGHT

#ifndef LCD_BKL_BRIGHTNESS_MAX
#define LCD_BKL_BRIGHTNESS_MAX 7
#endif


typedef enum
{
    UP2DOWN = 0,
    DOWN2UP,
    LEFT2RIGHT,
    RIGHT2LEFT
}
T_TRIANGLE_DIRECTION;



/** macro definition for brightness value*/

#define LCD_BRIGHTNESS_MIN	1
#define LCD_BRIGHTNESS_MAX	7

typedef enum {
    REFRESHSHADE_T2B = 0,
    REFRESHSHADE_T2B2,
    REFRESHSHADE_B2T,
    REFRESHSHADE_B2T2,
    REFRESHSHADE_C2H,
    REFRESHSHADE_H2C,
    REFRESHSHADE_L2R,
    REFRESHSHADE_L2R2,
    REFRESHSHADE_R2L,
    REFRESHSHADE_R2L2,
    REFRESHSHADE_C2W,
    REFRESHSHADE_W2C,
    REFRESHSHADE_C2S,
    REFRESHSHADE_S2C,
    REFRESHSHADE_T2B_EX,
    REFRESHSHADE_B2T_EX,
    REFRESHSHADE_L2R_EX,
    REFRESHSHADE_R2L_EX,
    REFRESHSHADE_TL2BR,
    REFRESHSHADE_TL2BR2,
    REFRESHSHADE_TR2BL,
    REFRESHSHADE_TR2BL2,
    REFRESHSHADE_BL2TR,
    REFRESHSHADE_BL2TR2,
    REFRESHSHADE_BR2TL,
    REFRESHSHADE_BR2TL2,
    REFRESHSHADE_WINBLIND_H,
    REFRESHSHADE_WINBLIND_W,
    REFRESHSHADE_ALPHA,
    REFRESHSHADE_NUM
} T_eREFRESHSHADE;



/** @defgroup FWL_DISP Display interface
    @ingroup FWL
 */
/*@{*/


/**
 * @brief Invalidate part of screen(synchronous)
 *
 * @author LiaoJianhua
 * @date 2005-12-29
 * @param[in] left		left of rect.
 * @param[in] top		top of rect.
 * @param[in] width		width of rect.
 * @param[in] height	height of rect.
 * @return void
 */
T_VOID	Fwl_InvalidateRect(T_POS left, T_POS top, T_LEN width, T_LEN height);

/**
 * @brief Draw a pixel
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param[in] hLayer    handle of layer
 * @param[in] x   the point to be draw
 * @param[in] y	  the point to be draw
 * @param[in] color   Display color
 * @return success or not
 */
T_BOOL	Fwl_SetPixel(HLAYER hLayer, T_POS x, T_POS y, T_COLOR color);

//T_VOID  Fwl_SetPixelOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, T_U16 pos_x, T_U16 pos_y, T_COLOR color);

/**
 * @brief Get the color of the point
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param[in] hLayer    handle of layer
 * @param[in] x		X coordination of the point
 * @param[in] y		Y coordination of the point
 * @return The color of this point
 */
T_COLOR	Fwl_GetPixel(HLAYER hLayer, T_POS x, T_POS y);

/**
 * @brief Fill  whole layer with a specified color
 *
 * @author Baoli.Miao
 * @date 2001-4-20
 * @param  T_COLOR color Display color
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_FillSolid(HLAYER layer, T_COLOR color);


/**
 * @brief Fill  a rectangle with a specified color
 *
 * @author Baoli.Miao
 * @date 2001-4-20
 * @param[in] hLayer    handle of layer
 * @param[in] left		the left pointer of the rect.
 * @param[in] top		the top pointer of the rect.
 * @param[in] width		rect width.
 * @param[in] height	rect height.
 * @param[in] color		Display color
 * @return void
 */
T_VOID	Fwl_FillSolidRect(HLAYER hLayer, T_POS left,T_POS top, T_LEN width, T_LEN height, T_COLOR color);


/**
 * @brief Draw a broken line according to the 'chr' value
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param[in] hLayer    handle of layer
 * @param[in] x1		X coordination of start point.
 * @param[in] y1		Y coordination of start point.
 * @param[in] x2		X coordination of end point.
 * @param[in] y2		Y coordination of end point.
 * @param[in] color		Display color
 * @param[in] chr		char code.
 * @return void
 */
T_VOID	Fwl_DrawLineByChar(HLAYER  hLayer, T_POS x1, T_POS y1, T_POS x2, T_POS y2, T_COLOR color, T_U8 chr);


T_VOID Fwl_DrawDiskOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
                            T_POS center_x, T_POS center_y, T_POS radii, T_COLOR color);


/**
 * @brief Draw 3D rectangle
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param[in] hLayer    handle of layer
 * @param[in] left		Left to the rect.
 * @param[in] top		Top to the rect.
 * @param[in] width		Rect width.
 * @param[in] height	Rect height.
 * @param[in] flag		Display mode.
 * @return void
 */
T_VOID	Fwl_Draw3DRect(HLAYER  hLayer, T_POS left, T_POS top, T_LEN width, T_LEN height, T_S8 flag);

/**
 * @brief Fill a solid triangle.
 * dire: 0: U->D, 1: D->U, 2: L->R, 3: R->L
 *
 * @author ZouMai
 * @date 2001-4-20
 * @param[in] hLayer    handle of layer
 * @param[in] left		Left to the rect.
 * @param[in] top		Top to the rect.
 * @param[in] width		Rect width.
 * @param[in] height	Rect height.
 * @param[in] dire		Draw direction
 * @param[in] color		Display color
 * @return void
 */
T_VOID  Fwl_FillSolidTria(HLAYER  hLayer, T_POS left,T_POS top, T_LEN width, T_LEN height, T_TRIANGLE_DIRECTION dire, T_COLOR color);


T_BOOL  Fwl_DrawRadio(HLAYER  hLayer, T_POS x, T_POS y, T_LEN radius, T_BOOL focus, T_COLOR color);


/**
 * @brief Turn on the LCD backlight
 * @param[in] RadioTxCtrl	backlight brightness value
 * @return brightness
 */
T_S16	Fwl_DisplayBacklightOn(T_BOOL RadioTxCtrl);

/**
 * @brief Turn off the LCD backlight
 * @author qxj
 * @date 2005-07-18
 * @return always be zero
 */
T_S16	Fwl_DisplayBacklightOff(T_VOID);


/**
 * @brief Clean the whole LCD
 * Clean the LCD with specified color
 * @author qxj
 * @date 2005-07-18
 * @param[in] color			specified color
 * @return void
 */
T_VOID Fwl_CleanScreen(T_COLOR color);


/**
 * @brief Get the display memory buffer address
 *      rgb format is 888
 * @author Jianhua.Liao
 * @date 2005-9-6
 * @return the display memory buffer address
 */
T_U8 *Fwl_GetDispMemory(T_VOID);

/**
 * @brief Get the display memory buffer address
 *      rgb format is 565
 * @author Jianhua.Liao
 * @date 2005-9-6
 * @return the display memory buffer address
 */
T_U8 *Fwl_GetDispMemory565(T_VOID);

T_VOID Fwl_Invalidate(T_VOID);
T_VOID Fwl_InvalidateLine( T_U16 top, T_U16 height);

//Insert YUV buf into background YUV buf.
T_VOID Fwl_InsertYUV2BckGrndYUV(const T_U8 *ybuf, const T_U8 *ubuf,const T_U8 *vbuf, T_S16 BckGrndWidth, T_S16 BckGrndHeight,
	T_U8 *YuvBuf, T_S16 left, T_S16 top, T_S16 Width, T_S16 Height, T_COLOR *bkColor, T_U8 trans);

T_pDATA Fwl_SaveDisplayRegion(T_RECT msgRect);
T_pDATA Fwl_RestoreDisplayRegion(T_pDATA lcd_buffer, T_RECT msgRect);

#endif




