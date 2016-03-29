/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: Eng_freetype.h
* Function: 
*
* Author: songmengxing
* Date: 2011-09-28
* Version: 1.0
*
* Revision: 
* Author: 
* Date: 
***************************************************************************/
#ifdef SUPPORT_VFONT
#ifndef _ENG_FREETYPE_H
#define _ENG_FREETYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "anyka_types.h"

#define VFONT_DEFAULT_SIZE		16


/**
* @brief VFont init
*
* @author Songmengxing
* @date 2012-04-01
* @return T_S32
* @retval 0 is success;
*/
T_S32 VF_Init(T_VOID);

/**
* @brief VFont destroy
*
* @author Songmengxing
* @date 2012-04-01
* @return T_S32
* @retval 0 is success;
*/
T_S32 VF_Destroy(T_VOID);

/**
* @brief load a font
*
* @author Songmengxing
* @date 2012-04-01
* @param in T_pCSTR path : font file path
* @return T_S32
* @retval 0 is success;
*/
T_S32 VF_New_Face(T_pCSTR path);

/**
* @brief free the font
*
* @author Songmengxing
* @date 2012-04-01
* @return T_S32
* @retval 0 is success;
*/
T_S32 VF_Done_Face(T_VOID);

/**
* @brief set font size
*
* @author Songmengxing
* @date 2012-04-01
* @param in T_U32 size : font size
* @return T_S32
* @retval 0 is success;
*/
T_S32 VF_Set_Size(T_U32 size);

/**
* @brief show string to a buffer
*
* @author Songmengxing
* @date 2012-04-01
* @param in T_U8* pbuf : buffer to show
* @param in T_U32 imgW : buf width
* @param in T_U32 imgH : buf height
* @param in T_U16* string : string to show
* @param in T_POS x :
* @param in T_POS y :
* @param in T_U32 color : color
* @param in T_U32 size : font size
* @return T_BOOL
* @retval 
*/
T_BOOL VF_DispStr(T_U8* pbuf, T_U32 imgW, T_U32 imgH, T_U16* string, T_POS x, T_POS y, T_COLOR color, T_U32 size);
/**
* @brief get char width
*
* @author Songmengxing
* @date 2012-04-01
* @param in const T_U16 chr : unicode id
* @return T_U16
* @retval char width
*/
T_U16 VF_GetCharWidth(const T_U16 chr);



#ifdef __cplusplus
}
#endif

#endif
#endif
