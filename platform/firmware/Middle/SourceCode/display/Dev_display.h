/**************************************************************************
 @FILE NAME:    Dev_display.h
 @BRIEF:        Define all the structure and macro of the display module;
                Declare all the API of the display module;
 copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co.,LTD
 @AuTHOR:       
 @DATE:         2010-07-01
 @VERSION:       
***************************************************************************/

#ifndef	__DEV_DISPLAY_H__
#define	__DEV_DISPLAY_H__   1

#include "Akdefine.h"
#include "Fwl_display.h"
#include "arch_lcd.h"


#define TV_HEIGHT_PAL 540
#define TV_WIDTH_PAL 650
#define TV_LEFT_PAL 35
#define TV_TOP_PAL 18

#define TV_HEIGHT_NTSC 440
#define TV_WIDTH_NTSC 636
#define TV_LEFT_NTSC 42
#define TV_TOP_NTSC 20


#undef  GPIO_CHRONTEL_ONOFF
#define GPIO_CHRONTEL_ONOFF 27


/*export function*/
T_BOOL		Dev_InitDisplay(T_VOID);		
T_BOOL		Dev_GetDisplayCaps (DISPLAY_TYPE_DEV disp_type, T_RECT *pstRect);
T_VOID      Dev_DisplayOn(T_VOID);
T_VOID      Dev_DisplayOff(T_VOID);
T_VOID      Dev_LcdRotate(T_eLCD_DEGREE rotate);
T_eLCD_DEGREE Dev_GetLcdDegree(T_VOID);

T_VOID      Dev_SetMultiChannelDisp(T_BOOL flag);

//only for multichannel display is true
T_VOID Dev_Refresh_Output(T_VOID);

T_VOID		Dev_RefreshDisplay(T_U8  *bBuf,T_U16 Width, T_U16 Height,
							   LAYER_TYPE type);


/*有些界面(如tvout下的camera 预览界面)，软2D太慢，导致显示很卡，
若该界面的RGB层只需要刷某个颜色时，可以用此接口。
*/
T_VOID  Dev_RefreshDisplayByColor(T_COLOR color);


/**
 * @brief  Set the callback function what will be called 
 *            when a frame having sended to the lcd
 * @author 
 * @date 2012-3-31
 */
T_VOID 		Dev_Set_Asyn_RefreshCbf(T_REFRESH_CALLBACK  pFuncCB);

/**
 * @brief Refresh RGB picture to  LCD or TVOUT
 * @author 
 * @date 2012-3-31
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] dsp_rect display rectangle,source picture should lower than 1024*768
 * @param[in] dsp_buf RGB buffer address
 * @param[in] addr      user param
 * @param[in] origin_width      dsp_buf original width
 * @param[in] origin_height     dsp_buf original height
 * @return T_BOOL 
 * @retval  AK_TRUE  refresh rgb channel successful
 * @retval  AK_FALSE refresh rgb channel failed.
 * @note  return failed:\n
 *     display size bigger than 1024*768\n
 *     display size smaller than 18*18
 */
T_VOID		Dev_Asyn_RefreshDisplay(T_RECT *dsp_rect, T_U8 *dsp_buf, T_U8 *addr, T_U32 origin_width, T_U32 origin_height);
T_VOID 		Dev_FlashRefBuf(T_VOID);

T_BOOL      Dev_RefreshRect_Fast(void * imgBuf, int imgWidth, int imgHeight, 
								T_BOOL FullScreen);

T_VOID		Dev_RefreshDisplayTVOUT_Fast(T_VOID);

DISPLAY_TYPE_DEV  Dev_GetDisplayType(T_VOID);
T_VOID		Dev_SetDisplayType(DISPLAY_TYPE_DEV type);

T_U8		Dev_SetBrightness(DISPLAY_TYPE_DEV lcd_type, T_U8 brightness);
T_U8		Dev_GetBrightness(DISPLAY_TYPE_DEV lcd_type);

T_U8*		Dev_GetFrameBuf ( T_RECT  *pstRectLayer, T_U8  * ColorSpace );
T_U8*		Dev_GetFrameBufInfo( T_RECT  *pstRectLay, T_U8  * ColorSpace );
T_BOOL	    Dev_GetFrameRect(DISPLAY_TYPE_DEV dispMode, T_U32 *pWidth, T_U32 *pHeight,T_RECT  *clipRect);

T_VOID 		Dev_RefreshTVOUT(const T_U8 *y, const T_U8 *u, const T_U8 *v, T_U16 srcW, T_U16 oriW, T_U16 oriH);

T_VOID		Dev_Refresh_YUV1(T_eLCD lcd, T_U8 *srcY, T_U8 *srcU,T_U8 *srcV,
						T_U16 src_width, T_U16 src_height,T_U16 left, 
						T_U16 top, T_U16 dst_width, T_U16 dst_height);
T_VOID		Dev_Refresh_YUV2(T_eLCD lcd, T_U8 *srcY,T_U8 *srcU,T_U8 *srcV,
							 T_U16 src_width, T_U16 src_height,T_U16 left, 
							 T_U16 top,T_U16 dst_width, T_U16 dst_height);
T_VOID      Dev_TurnOff_YUV(T_VOID);


T_VOID      Dev_CleanFrameBuf(void);

#if (SDRAM_MODE == 8)
T_VOID 		Dev_FreeBuf(T_VOID);
T_VOID 		Dev_ReMallocBuf(T_VOID);
#endif

/*static function*/ /*should move to the C file*/





#endif //__DEV_DISPLAY_H__
