/**
 * @file arch_lcd.h
 * @brief This file describe the interface of lcd module
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author lianGenhui
 * @date 2010-06-18
 * @version 2.0
 */

#ifndef __ARCH_LCD_H__
#define __ARCH_LCD_H__


/** @defgroup Arch_lcd Architecture of lcd
 *  @ingroup LCD
 */
/*@{*/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief LCD number define.
 * define the id of LCD
 */
typedef enum
{
    LCD_0 = 0,
    LCD_1,
    LCD_MAX_NUMBER
} T_eLCD;

/**
 * @brief LCD Degree define.
 * define the degree of LCD
 */
typedef enum
{
    LCD_0_DEGREE = 0,
    LCD_90_DEGREE,
    LCD_180_DEGREE,
    LCD_270_DEGREE,
    LCD_MAX_DEGREE
}T_eLCD_DEGREE;

/**
 * @brief LCD type define.
 * define the type of LCD
 */
typedef enum
{
    E_LCD_RESERVED = 0,
    E_LCD_TYPE_MPU,
    E_LCD_TYPE_RGB,
    E_LCD_TYPE_TVOUT,
    E_LCD_TYPE_STN
} E_LCD_TYPE;

/**
 * @brief TVOUT mode type define.
 * define the mode type of TVOUT
 */
typedef enum
{
    TV_OUT_TYPE_PAL = 0, ///< size: 720*576
    TV_OUT_TYPE_NTSC     ///< size: 720*480
} E_TV_OUT_TYPE;

/**
 * @brief TVOUT CLK type define.
 * define the CLK type of TVOUT
 */
typedef enum
{
    TVOUT_CLK_INTERNAL = 0,   ///Choose internal clk
    TVOUT_CLK_EXTERNAL        ///Choose external clk
} E_TV_OUT_CLK;


/**
 * @brief Initialize the LCD
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_VOID
 */
T_BOOL  lcd_initial (T_VOID);

/**
 * @brief Turn on the LCD
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @return T_VOID
 */
T_VOID lcd_turn_on (T_eLCD lcd);

 /**
 * @brief Turn off the LCD
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @return T_VOID
 */
T_VOID lcd_turn_off (T_eLCD lcd);

/**
 * @brief Set brightness value
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] brightness brightness value,use 0 to 7
 * @return T_U8 
 * @retval new brightness value after setting
 */
T_U8 lcd_set_brightness (T_eLCD lcd, T_U8 brightness);

/**
 * @brief Get brightness value
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @return T_U8 
 * @retval current brightness value
 */
T_U8 lcd_get_brightness (T_eLCD lcd);

/**
 * @brief rotate the lcd
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] degree rotate degree
 * @return T_VOID
 */
T_VOID lcd_rotate (T_eLCD lcd, T_eLCD_DEGREE degree);

/**
 * @brief get current lcd degree
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @return T_eLCD_DEGREE
 * @retval degree of LCD
 */
T_eLCD_DEGREE lcd_degree (T_eLCD lcd);

/**
 * @brief get current lcd type
 * @author WangGuotian
 * @date 2011-11-25
 * @param[in] T_VOID
 * @return E_LCD_TYPE
 * @retval type of LCD
 */
E_LCD_TYPE lcd_get_type(T_VOID);

/**
 * @brief get current lcd's hardware height
 * lcd hardware height is determined by the lcd factory, and won't be changed
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd select the lcd, LCD_0 or LCD_1
 * @return T_U32
 * @retval lcd hardware height
 */
T_U32 lcd_get_hardware_height (T_eLCD lcd);

/**
 * @brief get current lcd hardware width
 * lcd hardware width is determined by the lcd factory, and won't be changed
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd select the lcd, LCD_0 or LCD_1
 * @return T_U32
 * @retval lcd hardware width
 */
T_U32 lcd_get_hardware_width (T_eLCD lcd);

/**
 * @brief set lcd background color, default is black color.
 * @param BackgroundColor the value of background color.
 * @return T_VOID
 */
T_VOID lcd_set_BgColor (T_U32 BackgroundColor);

/**
 * @brief Refresh RGB picture to LCD
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] dsp_rect display rectangle,source picture should lower than 800*1022
 * @param[in] dsp_buf RGB buffer address
 * @return T_BOOL 
 * @retval AK_TRUE  refresh rgb channel successful
 * @retval AK_FALSE refresh rgb channel failed.
 * @note return failed:\n
 *     display size bigger than 800*1022\n
 *     display size smaller than 18*18
 */
T_BOOL lcd_refresh_RGB (T_eLCD lcd, T_RECT *dsp_rect, T_U8 *dsp_buf);

 /**
 * @brief Refresh RGB picture,use virtual page
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] dsp_rect display rectangle,display page should lower than 800*1022
 *            dsp_rect->left and top is the start postion of screen
 *            dsp_rect->width and height are display width and height
 * @param[in] vir_rect virtual page rectangle(source page),virture page should lower than 1022*1022
 *            vir_rect->left and top is the start postion of image
              vir_rect->width and height are image width and height
 * @param[in] dsp_buf RGB buffer
 * @return T_BOOL
 * @retval AK_TRUE  refresh rgb channel successful
 * @retval AK_FALSE refresh rgb channel failed
 * @note return failed:\n
 *    source picture smaller than display size\n
 *    source picture bigger than 1022*1022¡¢smaller than 18*18\n
 *    display size smaller than 18*18, bigger than 1022*1022  
 */
T_BOOL lcd_refresh_RGB_ex (T_eLCD lcd, T_RECT *dsp_rect, T_RECT *vir_rect, T_U8 *dsp_buf);

/**
 * @brief Refresh RGB picture to  LCD or TVOUT
 *        only support in Sundance3 !!
 * @author WangGuotian
 * @date 2011-9-14
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] dsp_rect display rectangle,source picture should lower than 1024*768
 * @param[in] dsp_buf RGB buffer address
 * @param[in] addr      user param
 * @return T_BOOL 
 * @retval AK_TRUE  refresh rgb channel successful
 * @retval AK_FALSE refresh rgb channel failed.
 * @note return failed:\n
 *     display size bigger than 1024*768\n
 *     display size smaller than 18*18
 */
T_BOOL lcd_asyn_refresh_RGB (T_eLCD lcd, T_RECT *dsp_rect, T_U8 *dsp_buf, T_U8 *addr);

/**
 * @brief Refresh YUV1 picture to LCD,use YUV 420 data
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] srcY source YUV picture addess -- Y
 * @param[in] srcU source YUV picture addess -- U
 * @param[in] srcV source YUV picture addess -- V
 * @param[in] src_width the size of source YUV picture -- width
 * @param[in] src_height the size of source YUV picture -- height
 * @param[in] dsp_rect rectangle
 * @return T_BOOL 
 * @retval AK_TRUE  refresh YUV1 channel successful
 * @retval AK_FALSE refresh YUV1 channel failed
 * @note return falsed:\n
 *    source picture bigger than 1022*1022\n
 *    display picture bigger than 800*1022\n
 *    source picture and display picture smaller than 18*18\n
 *    upscaled bigger than 16 times or downscaled smaller 1/16\n
 *    if use YUV1 and RBG channel,YUV1 picture not in RGB picture\n
 *    use upscaled,source Horizonial length bigger than 682 
**/ 
T_BOOL lcd_refresh_YUV1 (T_eLCD lcd, T_U8 *srcY, T_U8 *srcU,T_U8 *srcV,
                             T_U16 src_width, T_U16 src_height,
                             T_RECT *dsp_rect);

/**
 * @brief display YUV1 with virtual page and scaler
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd LCD_0 or LCD1
 * @param[in] srcY YUV source buffer address -- Y
 * @param[in] srcU YUV source buffer address -- U
 * @param[in] srcV YUV source buffer address -- V
 * @param[in] src_width the size of source page -- width
 * @param[in] src_height the size of source page -- height
 * @param[in] vir_rect page rectangle
 * @param[in] dsp_rect rectangle
 * @return T_BOOL
 * @retval AK_TRUE  refresh YUV1 channel successful
 * @retval AK_FALSE refresh YUV1 channel failed
 * @note return falsed: refer to lcd_refresh_YUV1
 **/ 
T_BOOL lcd_refresh_YUV1_ex (T_eLCD lcd, T_U8 *srcY, T_U8 *srcU, T_U8 *srcV,
                                    T_U16 src_width, T_U16 src_height,
                                    T_RECT *vir_rect, T_RECT *dsp_rect);

/**
 * @brief Refresh YUV1 and YUV2 picture to LCD.
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] srcY1 source YUV1 picture addess -- Y
 * @param[in] srcU1 source YUV1 picture addess -- U
 * @param[in] srcV1 source YUV1 picture addess -- V
 * @param[in] src_width1 source YUV1 picture size -- width
 * @param[in] src_height1 source YUV1 picture size -- height
 * @param[in] dsp_rect1 rectangle of YUV1
 * @param[in] srcY2 source YUV2 picture addess -- Y
 * @param[in] srcU2 source YUV2 picture addess -- U
 * @param[in] srcV2 source YUV2 picture addess -- V
 * @param[in] src_width2 source YUV2 picture size -- width
 * @param[in] src_height2 source YUV2 picture size -- height
 * @param[in] dsp_rect2 rectangle of YUV2
 * @return T_BOOL
 * @retval AK_TRUE  refresh YUV1 and YUV2 channel successful
 * @retval AK_FALSE refresh YUV1 and YUV2 channel failed
 * @note return falsed:\n 
 *    YUV1 source picture bigger than 1022*1022\n
 *    YUV1 display picture bigger than 800*1022\n
 *    YUV1 source and display size smaller than 18*18\n
 *    YUV1 upscaled bigger than 16 times or downscaled smaller 1/16\n
 *    YUV1 picture not in RGB picture if used YUV1 and RBG channel both\n
 *    source Horizonial length bigger than 682 if YUV1 used upscaled\n
 *    YUV2 picture bigger than 320*240
**/ 
T_BOOL lcd_refresh_dual_YUV (T_eLCD lcd, T_U8 *srcY1,T_U8 *srcU1,T_U8 *srcV1,
                                    T_U16 src_width1, T_U16 src_height1,
                                    T_RECT *dsp_rect1,
                                    T_U8 *srcY2,T_U8 *srcU2,T_U8 *srcV2,
                                    T_U16 src_width2, T_U16 src_height2,
                                    T_RECT *dsp_rect2);

/**
 * @brief Refresh YUV1 and YUV2 picture to LCD,display YUV1 with virtual page and scaler
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] srcY1 source YUV1 picture addess  -- Y
 * @param[in] srcU1 source YUV1 picture addess  -- U
 * @param[in] srcV1 source YUV1 picture addess  -- V
 * @param[in] src_width1 source YUV1 picture size -- width
 * @param[in] src_height1 source YUV1 picture size -- height
 * @param[in] vir_rect1 page rectangle of YUV1
 * @param[in] dsp_rect1 rectangle of YUV1
 * @param[in] srcY2 source YUV2 picture addess  -- Y
 * @param[in] srcU2 source YUV2 picture addess  -- U
 * @param[in] srcV2 source YUV2 picture addess  -- V
 * @param[in] src_width2 source YUV2 picture size -- width
 * @param[in] src_height2 source YUV1 picture size -- height
 * @param[in] dsp_rect2 rectangle of YUV2
 * @return T_BOOL
 * @retval AK_TRUE  refresh YUV1 and YUV2 channel successful
 * @retval AK_FALSE refresh YUV1 and YUV2 channel failed
 * @note return falsed: refer to lcd_refresh_dual_YUV
 **/ 
T_BOOL lcd_refresh_dual_YUV_ex (T_eLCD lcd, T_U8 *srcY1,T_U8 *srcU1,T_U8 *srcV1,
                                        T_U16 src_width1, T_U16 src_height1,
                                        T_RECT *vir_rect1, T_RECT *dsp_rect1,
                                        T_U8 *srcY2,T_U8 *srcU2,T_U8 *srcV2,
                                        T_U16 src_width2, T_U16 src_height2,
                                        T_RECT *dsp_rect2);

/**
 * @brief open YUV channel
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_VOID
 */
T_VOID lcd_YUV_on (T_VOID);

/**
 * @brief close YUV channel
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_VOID
 */
T_VOID lcd_YUV_off (T_VOID);

/**
 * @brief check frame finish,for muti-yuv buffer refresh
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] pbuf buffer address
 * @return T_BOOL
 */ 
T_BOOL lcd_check_frame_finish(T_U8 *pbuf);

/**
 * @brief set yuv2 alpha level
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] Level the level of alpha, must 0 to 8, 0 means transparent
 * @return T_BOOL
 * @retval AK_TRUE frame refresh finished
 * @retval AK_FALSE frame refresh unfinished
 */
T_BOOL lcd_set_YUV2AlhpaLvl (T_U32 Level);

/**
 * @brief get current YUV2 alpha level
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_U32
 * @retval current alpha level
 */
T_U32 lcd_get_YUV2AlhpaCurLvl (T_VOID);

/**
 * @brief Set OSD color palette
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] color color palette,15 kind of colors in this palette,
 *            one color size on two bytes 
 * @return T_VOID
 */
T_VOID lcd_osd_set_color_palette (T_U16 *color);

/**
 * @brief display OSD channel to LCD
 * @author LianGenhui
 * @date 2010-06-18
 * @param lcd LCD_0 or LCD1
 * @param[in] dsp_rect display area 
 * @param[in] alpha value of alpha blending opration. value between 0 to 8
 * @param[in] dsp_buf display buffer, the index value(start from 1) of color palette.
 *            low 4bit of one byte stand for first pixel, high 4bit stand for next pixel.
 *            0 means transparent.
 * @return T_BOOL
 * @retval AK_TRUE  refresh OSD channel successful
 * @retval AK_FALSE refresh OSD channel failed
 * @note return failed:\n
 *    the pixels of line more than 256\n
 *    OSD picture covered the YUV2 picture\n 
 *    OSD picture and YUV2 picture are display in the same line 
 */
T_BOOL lcd_osd_display_on (T_eLCD lcd, T_RECT *dsp_rect, T_U8 alpha, T_U8 *dsp_buf);

/**
 * @brief turn off OSD
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd LCD_0 or LCD1
 * @return T_VOID
 */
T_VOID lcd_osd_display_off (T_eLCD lcd);

/**
 * @brief open TV-out function
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in]  type TV_OUT_TYPE_PAL or TV_OUT_TYPE_NTSC
 * @return T_BOOL
 * @retval AK_TRUE  open TV out successful
 * @retval AK_FALSE open TV out failed
 * @note reutrn failed:type not TV_OUT_TYPE_PAL or TV_OUT_TYPE_NTSC
 */
T_BOOL lcd_tv_out_open (E_TV_OUT_TYPE type);

/**
 * @brief close TV-out function
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_VOID
 */
T_VOID lcd_tv_out_close (T_VOID);

/**
 * @brief choose TV-out clk function
 * @author LiuHuadong
 * @param[in] type type = 1 choose external clk or  type = 0  choose internal clk
 * @date 2011-04-12
 * @return T_VOID
 */
T_VOID lcd_tvout_clk_sel (E_TV_OUT_CLK type);

/**
 * @brief set rgb channel input format
 *           only support in aspen3s
 * @author Liaozhijun
 * @param[in] bRgb565 AK_TRUE: rgb565, AK_FALSE: rgb888
 * @param[in] bRgbSeq AK_TRUE: rgb, AK_FALSE: bgr
 * @date 2010-12-20
 * @return T_VOID
 */
 T_VOID lcd_set_rgb_data_format(T_BOOL bRgb565, T_BOOL bRgbSeq);

/**
 * @brief Set the callback function what will be called 
 *            when a frame having sended to the lcd.
 *            only support in Sundance3 !!
 * @author WangGuotian
 * @date 2011-9-14
 * @return the old callback function pointer. 
 */
typedef T_VOID (*T_REFRESH_CALLBACK)(T_U8* ,T_U8 *);
T_REFRESH_CALLBACK lcd_set_refresh_finish_callback(T_REFRESH_CALLBACK  pFuncCB);

#ifdef __cplusplus
}
#endif

/*@}*/
#endif

