#ifndef __FWL_DISPOSD_H__
#define __FWL_DISPOSD_H__
#include "Fwl_Public.h"
#include "Eng_AkBmp.h"
/**
 * @brief Query :init osd buffer and set color panel
 *
 * @author 	Liuguodong
 * @param	no
 * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_Init(T_VOID);


/**
 * @brief Query : set osd color panel by gradual gray degree
 *
 * @author 	Liuguodong
 * @param	no
 * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_SetColorPanelByGray(T_VOID);


/**
 * @brief Query : get index in color panel by color gray degree
 *
 * @author 	Liuguodong
 * @param	color [in] : color 
 * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_U8  Fwl_Osd_GetColorPanelIndexByGray(T_U16 color );

/**
 * @brief Query : draw bmp on osd buffer by bmp gray degree 
 *
 * @author 	Liuguodong
 * @param	pRangeRect[in]: pointer to rect of bmp for draw
 * @param	pAnykaBmp[in]:  pointer to structural bmp data 
  * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_DrawBmpByGray(T_RECT * pRangeRect , T_AK_BMP    * pAnykaBmp);

/**
 * @brief Query : draw bmp on osd buffer by bmp gray degree 
 *
 * @author 	Liuguodong
 * @param	pRangeRect[in]: pointer to rect of bmp for draw
 * @param	pBmpStream[in]:  pointer to bmp data stream including bmp information
  * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_DrawStreamBmpByGray(T_RECT * pRangeRect ,T_pCDATA pBmpStream);

/**
 * @brief Query : draw bmp on osd buffer by bmp gray degree 
 *
 * @author 	Liuguodong
 * @param	pRangeRect[in]: pointer to rect of bmp for draw
 * @param	pRawBmp[in]:  pointer to bmp data only including rgb data
  * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_DrawRawBmpByGray(T_RECT * pRangeRect ,T_U8 * pRawBmp);

/**
 * @brief Query : draw radio control  on osd buffer by color gray degree
 *
 * @author 	Liuguodong
 * @param	pRangeRect[in]:  pointer to rect surrounding radio 
 * @param	              x[in]:  radio center horizontal pos
 * @param	              y[in]:  radio center vertical pos
 * @param	        radius[in]:  radio  radius
 * @param	         focus[in]:  whether radios is selected
 * @param	          color[in]:  radio color 
    * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_DrawRadioByGray(T_RECT * pRangeRect , T_POS x, T_POS y,T_LEN radius,T_BOOL focus,    T_COLOR color);

/**
 * @brief Query : draw solid rect  on osd buffer by color gray degree
 *
 * @author 	Liuguodong
 * @param	pRangeRect[in]:  pointer to rect  to fill 
 * @param	               color[in]:  rect color 
    * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_FillSolidRectByGray(T_RECT * pRangeRect ,   T_COLOR color);

/**
 * @brief Query : draw string  on osd buffer by color gray degree
 *
 * @author 	Liuguodong
 * @param	pRangeRect[in]:  pointer to rect  to fill 
 * @param	              x[in]:  string start horizontal pos
 * @param	              y[in]:  string start  vertical pos
 * @param	      pString[in]:  pointer to string  
 * @param            strLen[in]:  string len 
 * @param	         color[in]:  string color 
 * @param	          font[in]:  string color 
    * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_DrawStringByGray(T_POS x, T_POS y ,T_pCSTR  pString,T_U16 strLen 	,T_COLOR color, T_FONT font);

/**
 * @brief Query : draw unicode string  on osd buffer by color gray degree
 *
 * @author 	Liuguodong
 * @param	              x[in]:  string start horizontal pos
 * @param	              y[in]:  string start  vertical pos
 * @param	      pString[in]:  pointer to string  
 * @param            strLen[in]:  string len 
 * @param	         color[in]:  string color 
 * @param	          font[in]:  string color 
    * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_DrawUStringByGray(T_POS x, T_POS y ,T_U16* pString,T_U16 strLen 
	,T_COLOR color, T_FONT font);

/**
 * @brief Query : draw solid triangle  on osd buffer by color gray degree
 *
 * @author 	Liuguodong
 * @param	pRect[in]:  pointer to rect  to surround triangle 
 * @param	         direction[in]:  direction of triangle
 * @param	         color[in]:  string color 
    * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL  Fwl_Osd_FillSolidTriaByGray(T_RECT *pRect, T_TRIANGLE_DIRECTION direction, T_COLOR color);

/**
 * @brief Query : display osd buffer on lcd or tvout
 *
 * @author 	Liuguodong
 * @param	no
    * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_RefreshDisplay(T_VOID);


/**
 * @brief Query : display osd buffer on lcd or tvout
 *
 * @author 	songmengxing
 * @param	pRect[in]: pRect  to refresh, width must be lcd width
    * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_RefreshDisplayRect(T_RECT *pRect);


/**
 * @brief Query : close osd show
 *
 * @author 	Liuguodong
 * @param	no
    * @return 	AK_TRUE=success, AK_FALSE=fail
 */
T_BOOL Fwl_Osd_DisplayOff(T_VOID);

/**
 * @brief Query : clear osd display buffer to zero
 *
 * @author 	Liuguodong
 * @param	no
    * @return  no
 */
T_VOID Fwl_Osd_ClearDispBuf(T_VOID);

#endif
