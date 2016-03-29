/**************************************************************************
 @FILE NAME:    Fwl_display.h
 @BRIEF:        Define all the structure and macro of the display module;
                Declare all the API of the display module;
 copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co.,LTD
 @AuTHOR:       
 @DATE:         2010-07-01
 @VERSION:       
***************************************************************************/

#ifndef __FWL_DISPLAY_H__
#define	__FWL_DISPLAY_H__

#include "Akdefine.h"

#ifdef LCD_MODE_565
#define BYTES_PER_PIXEL  2
#else
#define BYTES_PER_PIXEL  3
#endif

#define TV_OUT_WIDTH	720
#define TV_OUT_HEIGHT	576

#if (MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT > TV_OUT_WIDTH * TV_OUT_HEIGHT)
#define	DISPLAY_BUF_SIZE	(MAIN_LCD_WIDTH * MAIN_LCD_HEIGHT * BYTES_PER_PIXEL)
#else
#define	DISPLAY_BUF_SIZE	(TV_OUT_WIDTH * TV_OUT_HEIGHT * BYTES_PER_PIXEL)
#endif

//#define  DEMO_IMGBROWSE
//#define  DEMO_IMGBROWSE_BIG_PIC
#define  GIF_IMGBROWSE_NO_WAITING

typedef  T_VOID*    HLAYER;

typedef T_VOID (*T_ASYN_REFRESH_CALLBACK)(T_U8* ,T_U8 *);

typedef enum
{
	RGB888,
	BGR888,
	RGB565,
	YUV420,
	YUV422,
	YUV444,
	ERROR_LAYER_TYPE
}LAYER_TYPE;


typedef  struct 
{
	T_RECT			stLayerRect;			//图层的区域信息
	LAYER_TYPE		LayerType;				//图层的类型
	T_U8 * 			pBufLayer;				//图层的buffer指针
	T_U8 			AlphaBlend;				//透明度
}T_LAYER_INFO;


typedef struct 
{		
		T_S16		WidthLcd;
		T_S16		HeightLcd;	
		//根据需要再加	
}T_INIT_DISP_ST;



typedef enum 
{
	DISPLAY_LCD_0 = 0,
	DISPLAY_LCD_1,	
	DISPLAY_TVOUT_PAL,//此顺序不能更改代码中会利用这个判断
	DISPLAY_TVOUT_NTSC,
	DISPLAY_MAX_TYPE
} DISPLAY_TYPE_DEV;

typedef enum
{
    DISP_0_DEGREE = 0,
    DISP_90_DEGREE,
    DISP_180_DEGREE,
    DISP_270_DEGREE,
    DISP_MAX_DEGREE
}DISP_DEGREE;          //FOLLOW  T_eLCD_DEGREE



typedef enum
{
    CAPS_HORZRES,
    CAPS_VERTRES,
} CAPS_TYPE;


typedef enum{
	IMGLAY_OK			= 0x0,
	IMGLAY_FORMAT_ERR,
	IMGLAY_DATA_INVALID_ERR,
	IMGLAY_MEM_ERR,
	IMGLAY_FILE_OPEN_ERR,
	IMGLAY_NOT_FIND_FILE,
	IMGLAY_ERROR 		= 0x0F,
	IMGLAY_OUT_RANGE	= 0x10,
	IMGLAY_DATA_SAME,
}T_eImgLayRet;


typedef enum{
	ORIENT_TOP		= 0x01,
	ORIENT_RIGHT	= 0x02,
	ORIENT_BOTTM	= 0x04,
	ORIENT_LEFT		= 0x08,
	ORIENT_CENTRAL	= 0x0F,
	ORIENT_FULL		= 0x10,
	ORIENT_TILE		= 0x20,
	
}T_eORIENT;

#define	HAVE_BUFFER		1
#define	NO_HAVE_BUFFER	0

#define FLAG_SHOW_DIRECT    1
#define FLAG_SHOW_NO_DIRECT 0


#ifdef CI37XX_PLATFORM
#define		HRGB_LAYER	Fwl_hRGBLayer
#define		HYUV_LAYER	Fwl_hYUVLayer
#define		HYUV2_LAYER	HYUV_LAYER

extern HLAYER      Fwl_hRGBLayer;
extern HLAYER      Fwl_hYUVLayer;
/*
#define		HOSD_LAYER	Fwl_hOSDLayer
*/
#else
#error "Must define CI37XX_PLATFORM!"
#endif



/*export function*/

/**
  @BRIEF:       Initializes image layer and display device.
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       pstPlatformInfo 
                    Pointer to a T_INIT_DISP_ST structure that
                    contains LCD width and height.
  @RETVAL:      If the function succeeds, the return value is AK_TURE.
                If the function fails, the return value is AK_FALSE.
 */
T_BOOL	Fwl_InitDisplay(T_INIT_DISP_ST * pstPlatformInfo) ;

/**
  @BRIEF:       Do nothing.
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       
  @RETVAL:      This function does not return a value. 
 */
T_VOID  Fwl_ReleaseDisplay( T_VOID );

/**
  @BRIEF:       Create a new image layer.
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       LayerType 
                    Specifies the type of the image layer being created.
                x
                    Specifies the initial horizontal position of the layer.
                    x is the x-coordinate of the upper-left corner of the 
                    layer relative to the upper-left corner of the LCD.
                y
                    Specifies the initial vertical position of the layer.
                    y is the y-coordinate of the upper-left corner of the 
                    layer relative to the upper-left corner of the LCD.
                Width
                    Specifies the width of the layer.
                Height
                    Specifies the height of the layer.
                FlagHaveBuf
                    Specifies having buffer flag of the layer .
                    HAVE_BUFFER : have buffer
                    NO_HAVE_BUFFER : no buffer
                AlphaBlend
                    Specifies the transparence of the layer.
                    From 0 to 100.                    
  @RETVAL:      If the function succeeds, the return value is a handle to
                the new image layer.
                If the function fails, the return value is AK_NULL.
 */
HLAYER  Fwl_CreateLayer(LAYER_TYPE LayerType, T_U16 x,T_U16 y, T_U16 Width,
						T_U16 Height, T_U8 FlagHaveBuf, T_U8 AlphaBlend);
						
/**
  @BRIEF:       Destroys the specified image layer.
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       hLayer 
                    Handle to the layer to be destroyed.
  @RETVAL:      If the function succeeds, the return value is nonzero.
                If the function fails, the return value is zero.
 */						
T_BOOL  Fwl_DeleteLayer(HLAYER  hLayer);	

/**
  @BRIEF:       Retrieves information for the specified layer. 
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       hLayer 
                    Handle to the layer whose information is to be retrieved.
                pstLayerInfo
                    Pointer to a T_LAYER_INFO structure to receive the information. 
  @RETVAL:      If the function succeeds, the return value is AK_TURE.
                If the function fails, the return value is AK_FALSE.
 */	
T_BOOL	Fwl_GetLayerInfo(HLAYER  hLayer,T_LAYER_INFO  *pstLayerInfo);

/**
  @BRIEF:       Changes the size, position, type and transparence of a layer.
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       hLayer 
                    Handle to the layer whose information is to be changed.
                pstLayerInfo
                    Pointer to a T_LAYER_INFO structure to set the information. 
  @RETVAL:      If the function succeeds, the return value is AK_TURE.
                If the function fails, the return value is AK_FALSE.
 */	
T_BOOL	Fwl_SetLayerInfo (HLAYER  hLayer, T_LAYER_INFO  *pstLayerInfo);

#if 0
T_S32  Fwl_RefreshImg(HLAYER hLayer, T_U8 *srcImg, T_U16 src_width, 
					  T_U16 src_height,T_eORIENT Orient , T_U8 FlagShowDirect);
#endif

/**
  @BRIEF:       Refresh image data to a layer.
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       hLayer 
                    Handle to the layer to be refreshed.
                srcData
                    source image data.                
                src_width
                    Specifies the width of the source image.
                src_height
                    Specifies the height of the source image.
                x
                    x-coord of destination upper-left corner.
                y
                    y-coord of destination upper-left corner.                   
                dst_width
                    width of destination rectangle.
                dst_height
                    height of destination rectangle.
                FlagShowDirect
                    Specifies the flag to refresh the layer to display device.
                    FLAG_SHOW_DIRECT    : refresh the layer to display device
                                          at once.  
                    FLAG_SHOW_NO_DIRECT : do not refresh the layer to display 
                                          device.
  @RETVAL:      If the function succeeds, the return value AK_TRUE.
                If the function fails, the return value is AK_FALSE.
 */
T_BOOL  Fwl_RefreshRect(HLAYER hLayer, T_U8 *srcData, T_U16 src_width, 
					  T_U16 src_height, T_U16 x, T_U16 y,T_U16 dst_width,
					  T_U16 dst_height, T_U8 FlagShowDirect);

T_BOOL Fwl_RefreshRect_Fast(void * imgBuf, int imgWidth, int imgHeight, 
				      T_BOOL FullScreen);


/**
  @BRIEF:       Refresh image data to a layer.
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       hLayer 
                    Handle to the layer to be refreshed.
                    The type of the layer must be YUV420.
                srcY                   
                srcU                    
                srcV
                    source image data.                
                src_width
                    Specifies the width of the source image.
                src_height
                    Specifies the height of the source image.
                x
                    x-coord of destination upper-left corner.
                y
                    y-coord of destination upper-left corner.                   
                dst_width
                    width of destination rectangle.
                dst_height
                    height of destination rectangle.
                FlagShowDirect
                    Specifies the flag to refresh the layer to display device.
                    FLAG_SHOW_DIRECT    : refresh the layer to display device
                                          at once.  
                    FLAG_SHOW_NO_DIRECT : do not refresh the layer to display 
                                          device.
  @RETVAL:      If the function succeeds, the return value is AK_TRUE.
                If the function fails, the return value is AK_FALSE.
 */
T_BOOL  Fwl_RefreshRectYUV(HLAYER hLayerYUV, T_U8 *srcY, T_U8 *srcU,T_U8 *srcV, 
						  T_U16 src_width, T_U16 src_height, T_U16 x, T_U16 y,
						  T_U16 dst_width,T_U16 dst_height, T_U8 FlagShowDirect);


T_VOID Fwl_TurnOff_YUV(T_VOID);

T_VOID Fwl_SetMultiChannelDisp(T_BOOL flag);
T_VOID Fwl_Refresh_Output(T_VOID);
				  
T_BOOL  Fwl_RefreshYUV1( T_U8 *srcY, T_U8 *srcU,T_U8 *srcV, 
						  T_U16 src_width, T_U16 src_height, T_U16 x, T_U16 y,
						  T_U16 dst_width,T_U16 dst_height);

/**
  @BRIEF:       Mix all the layer and refresh to display device.
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       
  @RETVAL:      This function does not return a value. 
 */
T_VOID  Fwl_RefreshDisplay(T_VOID);


/*有些界面(如tvout下的camera 预览界面)，软2D太慢，导致显示很卡，
若该界面的RGB层只需要刷某个颜色时，可以用此接口。
*/
T_VOID  Fwl_RefreshDisplayByColor(T_COLOR color);


/**
 * @brief  Set the callback function what will be called 
 *            when a frame having sended to the lcd
 * @author 
 * @date 2012-3-31
 */
T_VOID Fwl_Set_Ref_FinishCbf(T_ASYN_REFRESH_CALLBACK  pFuncCB);

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
T_BOOL Fwl_Asyn_RefDisplay(T_RECT *dsp_rect, T_U8 *dsp_buf, T_U8 *addr, T_U32 origin_width, T_U32 origin_height);

/**
  @BRIEF:       Retrieves the display type.
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       
  @RETVAL:      This function will return the current display type. 
 */
DISPLAY_TYPE_DEV  Fwl_GetDispalyType(T_VOID);

/**
  @BRIEF:       Changes the display type.
  @AUTHOR:      
  @DATE:        2010-07-01
  @PARAM:       
  @RETVAL:      This function does not return a value. 
 */


T_VOID	Fwl_RefreshDisplayTVOUT_Fast(T_VOID);


T_VOID		Fwl_SetDisplayType(DISPLAY_TYPE_DEV type);
T_BOOL      Fwl_TvoutIsOpen(T_VOID);
T_U8*		Fwl_GetFrameBuf ( T_RECT  *pstRectLay, T_U8  * ColorSpace );
T_U8*		Fwl_GetFrameBufInfo( T_RECT  *pstRectLay, T_U8  * ColorSpace );
T_VOID      Fwl_CleanFrameBuf(T_VOID);
T_BOOL      Fwl_GetDispFrameRect(DISPLAY_TYPE_DEV dispMode, T_U32 *pWidth, T_U32 *pHeight,T_RECT  *clipRect);

T_BOOL		Fwl_GetDisplayCaps ( DISPLAY_TYPE_DEV disp_type,T_RECT  *pstRect);


T_U16       Fwl_GetLcdWidth(void);
T_U16       Fwl_GetLcdHeight(void);

T_U8		Fwl_SetBrightness(DISPLAY_TYPE_DEV lcd_type, T_U8 brightness);
T_U8		Fwl_GetBrightness(DISPLAY_TYPE_DEV lcd_type);
T_VOID      Fwl_DisplayOn(T_VOID);
T_VOID      Fwl_DisplayOff(T_VOID);
T_VOID      Fwl_LcdRotate(DISP_DEGREE rotate);
DISP_DEGREE Fwl_GetLcdDegree(T_VOID);

T_BOOL          Fwl_FillRect(HLAYER layer, T_U16 left, T_U16 top, 
							 T_U16 width, T_U16 height, T_COLOR color);
T_BOOL          Fwl_DrawLine(HLAYER layer, T_POS x1, T_POS y1, T_POS x2, 
							  T_POS y2, T_COLOR color);
T_BOOL          Fwl_DrawRect(HLAYER layer,  T_POS left, T_POS top, T_LEN width, 
							  T_LEN height, T_COLOR color);

T_VOID 		Fwl_RefreshTVOUT(const T_U8 *y, const T_U8 *u, const T_U8 *v, T_U16 srcW, T_U16 oriW, T_U16 oriH);

T_VOID Fwl_FlashRefLcd(T_VOID);

/*static function*/ /*should move to the C file*/



#endif	/*__FWL_DISPLAY_H__*/
