/**
 * @file Ctl_SlipItemMgr.h
 * @brief This header file is for title definition and function prototype
 * @author: songmengxing
 */

#ifndef __CTL_SLIPITEMMGR_H__
#define __CTL_SLIPITEMMGR_H__

#include "ctl_slipItem.h"



#ifdef __cplusplus
extern "C" {
#endif


#define STORE_ITEMNUM	(2)



/**
* @brief Creat a slip item manager control
*
* @author Songmengxing
* @date 2011-8-23
* @param in E_ITEMTYPE itemtype:ITEM_TYPE_LIST or ITEM_TYPE_IMAGE
* @param in T_U32 width: rect width
* @param in T_U32 height: rect height
* @param in T_U32 itemW: item width
* @param in T_U32 itemH: item height
* @param in T_U32 totalItemNum: total item num
* @param in E_MOVETYPE movetype:MOVETYPE_X or MOVETYPE_Y
* @return T_ITEM_MGR * the ItemMgr handle
* @retval
*/
T_ITEM_MGR *SlipItemMgr_Creat(E_ITEMTYPE itemtype, T_U32 width, T_U32 height, T_U32 itemW, T_U32 itemH, T_U32 totalItemNum, E_MOVETYPE movetype);

/**
* @brief Destroy a slip item manager control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipItemMgr_Destroy(T_ITEM_MGR *pItemMgr);

/**
* @brief Set Item id, icon and text by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_U32 index:the Item index in mgr
* @param in T_S32 id_of_item:the Item id
* @param in const T_pDATA pIconLeft:the left icon data
* @param in const T_pDATA pIconRight:the right icon data
* @param in const T_U16* pTextMain:the main text data
* @param in const T_U16* pTextDown:the down line text data
* @param in const T_U16* pTextRight:the right text data
* @return T_BOOL
* @retval
*/
T_BOOL SlipItemMgr_SetItem(T_ITEM_MGR *pItemMgr, T_U32 index, T_S32 id_of_item, T_pCDATA pIconLeft, T_pCDATA pIconRight, const T_U16* pTextMain, const T_U16* pTextDown, const T_U16* pTextRight);

/**
* @brief Change Item right text  by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_S32 id:the Item id
* @param in const T_U16* pTextRight:the new right text data
* @return T_BOOL
* @retval
*/
T_BOOL SlipItemMgr_ChangeTextRightById(T_ITEM_MGR *pItemMgr, T_S32 id, const T_U16* pTextRight);

/**
* @brief show Item by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_S32 id:the Item id
* @param in T_BOOL bFocus:it is focus or not
* @return T_BOOL
* @retval
*/
T_BOOL SlipItemMgr_ShowItemById(T_ITEM_MGR *pItemMgr, T_S32 id, T_BOOL bFocus);


/**
* @brief scroll show Item by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_S32 id:the Item id
* @param in T_BOOL bFocus:it is focus or not
* @return T_BOOL
* @retval
*/
T_BOOL SlipItemMgr_ScrollShowItemById(T_ITEM_MGR *pItemMgr, T_S32 id, T_BOOL bFocus);


/**
* @brief get Item index by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_S32 id:the Item id
* @return T_S32
* @retval >=0 : index; <0 : error
*/
T_S32 SlipItemMgr_GetIndexById(T_ITEM_MGR *pItemMgr, T_S32 id);

/**
* @brief get first whole Item id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_S32 id:the Item id
* @return T_S32
* @retval >=0 : id; <0 : error
*/
T_S32 SlipItemMgr_GetCurItemId(T_ITEM_MGR *pItemMgr);

/**
* @brief get item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetItemNum(T_ITEM_MGR *pItemMgr);

/**
* @brief get loaded item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetLoadItemNum(T_ITEM_MGR *pItemMgr);

/**
* @brief get total item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetTotalItemNum(T_ITEM_MGR *pItemMgr);


/**
* @brief set total item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_SetTotalItemNum(T_ITEM_MGR *pItemMgr, T_U32 totalnum);

/**
* @brief add loaded item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_S32 count: add count
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_AddLoadItemNum(T_ITEM_MGR *pItemMgr, T_S32 count);

/**
* @brief fill the show buf with item buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_FillBuf(T_ITEM_MGR *pItemMgr);

/**
* @brief add offset
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_S32 offset: offset
* @param in T_U32 emptyNum: empty item num(image thumbnail use)
* @return T_S32 count need to load
* @retval 
*/
T_S32 SlipItemMgr_AddOffset(T_ITEM_MGR *pItemMgr, T_S32 offset, T_U32 emptyNum);

/**
* @brief get display buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param out T_U8 **pbuf: buf
* @param out T_U32 *width: width of buf
* @param out T_U32 *height: height of buf
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_GetDisBuf(T_ITEM_MGR *pItemMgr, T_U8 **pbuf, T_U32 *width, T_U32 *height);

/**
* @brief get item id by point
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_POS x: x
* @param in T_POS y: y
* @return T_S32
* @retval 
*/
T_S32 SlipItemMgr_GetItemIdByPoint(T_ITEM_MGR *pItemMgr, T_POS x, T_POS y);

/**
* @brief check focus item is in show rect or not
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_U32 focusId: focus item id
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_CheckFocusItem(T_ITEM_MGR *pItemMgr, T_U32 focusId);

/**
* @brief get over len
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_S32
* @retval 
*/
T_S32 SlipItemMgr_GetOverLen(T_ITEM_MGR *pItemMgr);

/**
* @brief get remain len
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param out T_S32 *nextRemainLen: 后方剩余长度
* @param out T_S32 *preRemainLen: 前方剩余长度
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_GetRemainLen(T_ITEM_MGR *pItemMgr, T_S32 *nextRemainLen, T_S32 *preRemainLen);


/**
* @brief get item height
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetItemHeight(T_ITEM_MGR *pItemMgr);

/**
* @brief get item width
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetItemWidth(T_ITEM_MGR *pItemMgr);

/**
* @brief get total store len
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetTotalStoreLen(T_ITEM_MGR *pItemMgr);

/**
* @brief clean offset
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_U32 num: 对应实际item链表已经加载上的item数量
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_ClearOffset(T_ITEM_MGR *pItemMgr, T_U32 num);

/**
* @brief set loaded item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_U32 num: num
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_SetLoadItemNum(T_ITEM_MGR *pItemMgr, T_U32 num);

/**
* @brief Set Item id by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_U32 index:the Item index in mgr
* @param in T_S32 id_of_item:the Item id
* @return T_BOOL
* @retval
*/
T_BOOL SlipItemMgr_SetItemId(T_ITEM_MGR *pItemMgr, T_U32 index, T_S32 id_of_item);

/**
* @brief get item num per row
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetItemNumPerRow(T_ITEM_MGR *pItemMgr);

/**
* @brief get item buf by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param inT_U32 index: item index
* @param out T_U8 **pbuf: buf
* @param out T_U32 *width: width of buf
* @param out T_U32 *height: height of buf
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_GetItemBufByIndex(T_ITEM_MGR *pItemMgr, T_U32 index, T_U8 **pbuf, T_U32 *width, T_U32 *height);

/**
* @brief set background color
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_COLOR color: color
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_SetBgColor(T_ITEM_MGR *pItemMgr, T_COLOR color);


/**
* @brief Get Item backgroud img
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr:the ItemMgr handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipItemMgr_LoadItemBgImg(T_ITEM_MGR *pItemMgr);

/**
* @brief clean Item focus
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipItemMgr_CleanItemFocus(T_ITEM_MGR *pItemMgr);



#ifdef __cplusplus
}
#endif

#endif
