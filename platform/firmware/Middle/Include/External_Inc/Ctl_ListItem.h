/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  * @File name: ctl_yuvmenu.h
  * @Function:  This file is a part of the state machine s_mobile_tv_player.c
  * @Author:    WangWei
  * @Date:      2008-7-14
  * @Version:   1.0
  */

#ifndef __CTL_LISTITEM_H__
#define __CTL_LISTITEM_H__

#include "gbl_global.h"
#include "anyka_types.h"
#include "fwl_vme.h"

#define INVALID_ID        -1
#define INVALID_INDEX     0xffffffff



typedef struct
{
    T_S32               id;
    T_U32               index;

    T_U8                *pImg;
    T_pWSTR             pText;
    T_U8                *pData;
    T_U32               DataLen;
    T_BOOL              bVisuable;

    T_U8                *pPrev;
    T_U8                *pNext;
}T_LIST_ELEMENT;

typedef struct
{
    T_LIST_ELEMENT      *pHead;
    
    T_U32               FocusIndex;
    T_U32               WorkingIndex;
    T_U32               PageIndex;
    T_U32               PageFirstItemIndex;     //used for page up/down
    T_U32               PageItemNum;
    T_U32               TotalItemNum;
}T_LIST_ITEM;

/**
 * @brief   Init a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   PageItemNum :to indicate the maxnum of one page in the list item
 * @return  AK_TRUE:Init ok
 * @return  AK_FALSE:init failed
 */
T_BOOL ListItem_Init(T_LIST_ITEM *pListItem, T_U32 PageItemNum);

/**
 * @brief   append a item to a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   pItem :the item's addr that to be appended
 * @return  AK_TRUE:append a item ok
 * @return  AK_FALSE:append a item failed
 */
T_BOOL ListItem_Append(T_LIST_ITEM *pListItem, T_LIST_ELEMENT *pItem);


/**
 * @brief   insert a item into a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   Index:the index below which the item will be inserted
 * @param   pItem :the item's addr that to be appended
 * @return  AK_TRUE:insert a item ok
 * @return  AK_FALSE:insert a item failed
 */
T_BOOL ListItem_Insert(T_LIST_ITEM *pListItem, T_U32 Index, T_LIST_ELEMENT *pItem);

/**
 * @brief   delete a item from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   Index :the item's index that to be deleted
 * @return  AK_TRUE:delete a item ok
 * @return  AK_FALSE:delete a item failed
 */
T_BOOL ListItem_Delete(T_LIST_ITEM *pListItem, T_U32 Index);


/**
 * @brief   delete all items from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  AK_TRUE:delete a item ok
 * @return  AK_FALSE:delete a item failed
 */
T_BOOL ListItem_DeleteAll(T_LIST_ITEM *pListItem);


/**
 * @brief   get a item's focus index from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  T_U32:the list item cur focus item's index
 */
T_U32 ListItem_GetFocusIndex(T_LIST_ITEM *pListItem);

/**
 * @brief   get a item's working index from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  T_U32:the list item cur working item's index
 */
T_U32 ListItem_GetWorkingIndex(T_LIST_ITEM *pListItem);

/**
 * @brief   get the focus item's id from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  T_S32:the list item focus item's id
 */
T_S32 ListItem_GetFocusId(T_LIST_ITEM *pListItem);

/**
 * @brief   get the working item's id from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  T_S32:the list item working item's id
 */
T_U32 ListItem_GetWorkingId(T_LIST_ITEM *pListItem);

/**
 * @brief   set the focus item's index to a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   FocusIndex :the index to be set
 * @return  AK_TRUE:set success
 * @return  AK_FALSE:set failed
 */
T_BOOL ListItem_SetFocusIndex(T_LIST_ITEM *pListItem, T_U32 FocusIndex);

T_BOOL ListItem_SetPageIndex(T_LIST_ITEM *pListItem, T_U32 PageIndex);

/**
 * @brief   set/change a indicated index item's text
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   Index :the index to be set
 * @param   pText :the text to be set
 * @return  AK_TRUE :set text success
 * @return  AK_FALSE :set text failed
 */
T_BOOL ListItem_SetText(T_LIST_ITEM *pListItem, T_U32 Index, T_U16 *pText);

/**
 * @brief   get a indicated index item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   Index :the index that be indicated
 * @return  T_LIST_ELEMENT* :the indicated item's addr
 */
T_LIST_ELEMENT *ListItem_GetItemByIndex(T_LIST_ITEM *pListItem, T_U32 Index);


/**
 * @brief   get a indicated index item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  T_LIST_ELEMENT* :the Focus item's ptr
 */
T_LIST_ELEMENT *ListItem_GetFocusItem(T_LIST_ITEM *pListItem);


/**
 * @brief   free a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  AK_TRUE :free success
 * @return  AK_FALSE :free failed
 */
T_BOOL ListItem_Free(T_LIST_ITEM *pListItem);

/**
 * @brief   the list item handle function that respond the event
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   Event :the event id
 * @param   pParam :the event param
 * @return  T_eBACK_STATE: refer to its define
 */
T_eBACK_STATE ListItem_Handle(T_LIST_ITEM *pListItem, T_EVT_CODE Event, T_EVT_PARAM *pParam);

T_BOOL ListItem_MoveUp(T_LIST_ITEM *pListItem, T_BOOL bMoveFocus);
T_BOOL ListItem_MoveDown(T_LIST_ITEM *pListItem, T_BOOL bMoveFocus);

T_BOOL ListItem_PageUp(T_LIST_ITEM *pListItem);
T_BOOL ListItem_PageDown(T_LIST_ITEM *pListItem);

T_BOOL ListItem_Enter(T_LIST_ITEM *pListItem);

T_BOOL ListItem_SetPageItemNum(T_LIST_ITEM *pListItem, T_U32 Num);

T_BOOL ListItem_ResetFoucsIndex(T_LIST_ITEM *pListItem);

//T_BOOL ListItem_Show(T_LIST_ITEM *pListItem, T_EVT_CODE Event, T_EVT_PARAM *pParam);
#endif

