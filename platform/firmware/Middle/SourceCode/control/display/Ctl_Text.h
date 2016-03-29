/**
 * @file Ctl_Text.h
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */

#ifndef __CTL_TEXT_H__
#define __CTL_TEXT_H__
 
#include "akdefine.h"
#include "Ctl_SlipMgr.h"
 
#ifdef __cplusplus
 extern "C" {
#endif


 /**
 * @brief Creat a Text control
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_RECT rect:
 * @param in T_U32 alignmode:
 * @return T_SLIP_TEXT * the Text handle
 * @retval
 */
 T_SLIP_TEXT *Text_Creat(T_RECT rect, T_U32 alignmode);

 /**
 * @brief Destroy a Text control
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_TEXT *pSlipText:the Text handle
 * @return AK_NULL
 * @retval
 */
 T_VOID *Text_Destroy(T_SLIP_TEXT *pSlipText);

 /**
 * @brief Set Text data
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_TEXT *pSlipText:the Text handle
 * @param in const T_U16* pText:the text data
 * @return T_BOOL
 * @retval
 */
 T_BOOL Text_SetText(T_SLIP_TEXT *pSlipText, const T_U16* pText);

/**
 * @brief Show Text data to a buf
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_TEXT *pSlipText:the Text handle
 * @param in T_U8 *pbuf:the buf to show
 * @param in T_U32 imgwidth:the width of the buf
 * @param in T_U32 imgheight:the height of the buf
 * @param in  T_COLOR color:color
 * @return T_BOOL
 * @retval
 */
T_BOOL Text_Show(T_SLIP_TEXT *pSlipText, T_U8 *pbuf, T_U32 imgwidth, T_U32 imgheight, T_COLOR color);

/**
 * @brief scroll Show Text data to a buf
 *
 * @author Songmengxing
 * @date 2011-8-23
 * @param in T_SLIP_TEXT *pSlipText:the Text handle
 * @param in T_U8 *pbuf:the buf to show
 * @param in T_U32 imgwidth:the width of the buf
 * @param in T_U32 imgheight:the height of the buf
 * @param in  T_COLOR color:color
 * @return T_BOOL
 * @retval
 */
T_BOOL Text_ScrollShow(T_SLIP_TEXT *pSlipText, T_U8 *pbuf, T_U32 imgwidth, T_U32 imgheight, T_COLOR color);

 
#ifdef __cplusplus
 }
#endif
 
#endif

