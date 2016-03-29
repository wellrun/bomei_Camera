/**
 * @file Ctl_SlipItem.h
 * @brief This header file is for title definition and function prototype
 * @author: songmengxing
 */

#ifndef __CTL_SLIPITEM_H__
#define __CTL_SLIPITEM_H__

#include "ctl_text.h"
#include "ctl_icon.h"


#ifdef __cplusplus
extern "C" {
#endif

#define X_INTERVAL          	(3)
#define Y_INTERVAL          	(2)

#define ITEM_BG_TRANS_COLOR     (g_Graph.TransColor)


/**
* @brief Creat a Item control
*
* @author Songmengxing
* @date 2011-8-23
* @param in E_ITEMTYPE itemtype:ITEM_TYPE_LIST or ITEM_TYPE_IMAGE
* @param in T_RECT rect:
* @return T_SLIP_ITEM * the Item handle
* @retval
*/
T_SLIP_ITEM *SlipItem_Creat(E_ITEMTYPE itemtype, T_RECT rect);

/**
* @brief Destroy a Item control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipItem_Destroy(T_SLIP_ITEM *pItem);

/**
* @brief Set Item id, icon and text
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in T_S32 id_of_item:the Item id point
* @param in const T_pDATA pIconLeft:the left icon data
* @param in const T_pDATA pIconRight:the right icon data
* @param in const T_U16* pTextMain:the main text data
* @param in const T_U16* pTextDown:the down line text data
* @param in const T_U16* pTextRight:the right text data
* @return T_BOOL
* @retval
*/
T_BOOL SlipItem_SetItem(T_SLIP_ITEM *pItem, T_S32 id_of_item, T_pCDATA pIconLeft, T_pCDATA pIconRight, const T_U16* pTextMain, const T_U16* pTextDown, const T_U16* pTextRight);

/**
* @brief Show item to its buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in T_BOOL bFocus:the item is focus or not
* @return T_BOOL
* @retval
*/
T_BOOL SlipItem_Show(T_SLIP_ITEM *pItem, T_BOOL bFocus);


/**
* @brief scroll Show item to its buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in T_BOOL bFocus:the item is focus or not
* @return T_BOOL
* @retval
*/
T_BOOL SlipItem_ScrollShow(T_SLIP_ITEM *pItem, T_BOOL bFocus);


/**
* @brief change the right text of the item
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in const T_U16* pTextRight:the new right text data 
* @return T_BOOL
* @retval
*/
T_BOOL SlipItem_ChangeTextRight(T_SLIP_ITEM *pItem, const T_U16* pTextRight);

/**
* @brief get the buf of the item
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param out T_U8 **pbuf:the buf  
* @param out T_U32 *width:the width of buf  
* @param out T_U32 *height:the height of buf  
* @return T_BOOL
* @retval
*/
T_BOOL SlipItem_GetItemBuf(T_SLIP_ITEM *pItem, T_U8 **pbuf, T_U32 *width, T_U32 *height);

/**
* @brief Set Item id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in T_S32 id_of_item:the Item id
* @return T_BOOL
* @retval
*/
T_BOOL SlipItem_SetItemId(T_SLIP_ITEM *pItem, T_S32 id_of_item);


/**
* @brief Get Item backgroud img
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipItem_LoadItemBgImg(T_SLIP_ITEM *pItem);


#ifdef __cplusplus
}
#endif

#endif
