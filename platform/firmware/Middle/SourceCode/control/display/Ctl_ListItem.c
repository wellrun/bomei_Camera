/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File ListItem.c
 * description: this file used relized the list-item functions that 
 * not include the show functions
 * Author：guohui
 * Date：2008-8-27
 * Version：1.0		  
**************************************************************************/
#include "Ctl_ListItem.h"
#include "eng_font.h"
#include "Fwl_public.h"
#include "fwl_keyhandler.h"
#include "eng_string_uc.h"

static T_BOOL ListItem_FreeNode(T_LIST_ELEMENT *p);
//static T_BOOL ListItem_SetPageFirstItemIndex(T_LIST_ITEM *pListItem, T_U32 PageFirstItemIndex);
static T_BOOL ListItem_CalcPageIndex(T_LIST_ITEM *pListItem);

#if 0
static T_VOID ListItem_Print(T_LIST_ITEM *pListItem)
{
    T_LIST_ELEMENT *p = AK_NULL;
    
    if (pListItem)
    {
        p = pListItem->pHead;
        
        Fwl_Print(C3, M_CTRL, "\n\n*******START*********\n");
        while(p != AK_NULL)
        {
            Fwl_Print(C3, M_CTRL, "cur item id: %x\n", p->id);
            Fwl_Print(C3, M_CTRL, "cur item prev: %x\n", p->pPrev);
            Fwl_Print(C3, M_CTRL, "cur item next: %x\n", p->pNext);
            p = (T_LIST_ELEMENT *)p->pNext;
        }
        Fwl_Print(C3, M_CTRL, "*******END*********\n\n");
    }
    
}
#endif
static T_BOOL ListItem_FreeNode(T_LIST_ELEMENT *p)
{
    T_BOOL ret = AK_TRUE;
    
    if (p)
    {
        if (p->pImg)
        {
            //Fwl_Free(p->pImg);
            //p->pImg = AK_NULL;
        }

        if (p->pText)
        { 
            Fwl_Free(p->pText);
            p->pText = AK_NULL;
        }
        
        if (p->pData)
        {
            Fwl_Free(p->pData);
            p->pData = AK_NULL;
        }

        Fwl_Free(p);
    }
    else
    {
        ret = AK_FALSE;
    }
        
    return ret;
}

T_BOOL ListItem_MoveUp(T_LIST_ITEM *pListItem, T_BOOL bMoveFocus)
{
    T_BOOL ret = AK_FALSE;
    T_U32  page_item_real_num;
    T_U32  prev_focus_index, tmp_focus_index;
    
    if (pListItem)
    {
        if (pListItem->TotalItemNum)
        {
            if (bMoveFocus)
            {
                prev_focus_index = pListItem->FocusIndex;
                tmp_focus_index = pListItem->FocusIndex;
                
                if (pListItem->TotalItemNum <= pListItem->PageItemNum)
                {
                    page_item_real_num = pListItem->TotalItemNum;
                }
                else
                {
                    page_item_real_num = pListItem->PageItemNum;
                }
       
                if (tmp_focus_index)
                {
                    tmp_focus_index--;
                }
                else
                {
                    tmp_focus_index = pListItem->TotalItemNum - 1;
                }

                pListItem->FocusIndex = tmp_focus_index;

                if (pListItem->TotalItemNum <= pListItem->PageItemNum)
                {
                    if (pListItem->PageIndex)
                    {
                        pListItem->PageIndex--;
                    }
                    else
                    {
                        pListItem->PageIndex = page_item_real_num - 1;
                    }
                }
                else
                {
                    if (pListItem->PageIndex == 0)
                    {
                        if (prev_focus_index < pListItem->FocusIndex)
                        {
                            pListItem->PageIndex = page_item_real_num - 1;
                        }
                        
                        pListItem->PageFirstItemIndex = pListItem->FocusIndex - pListItem->PageIndex;
                    }
                    else
                    {
                        pListItem->PageIndex--;
                    }
                }
            }
            else
            {
                if (pListItem->TotalItemNum > pListItem->PageItemNum)
                {
                    if (pListItem->PageFirstItemIndex)
                    {
                        pListItem->PageFirstItemIndex--;
                    }

                    //当需要换页显示时，焦点始终要在当前页内
		            if(pListItem->FocusIndex > (pListItem->PageFirstItemIndex+pListItem->PageItemNum-1))
		            {
			           pListItem->FocusIndex = pListItem->PageFirstItemIndex + pListItem->PageItemNum -1;
	                }
                }
            }
            
            ret = AK_TRUE;
        }

        ListItem_CalcPageIndex(pListItem);
    }

    return ret;
}


T_BOOL ListItem_MoveDown(T_LIST_ITEM *pListItem, T_BOOL bMoveFocus)
{
    T_BOOL ret = AK_FALSE;
    T_U32  page_item_real_num;
    T_U32  prev_focus_index, tmp_focus_index;

    if (pListItem)
    {
        if (pListItem->TotalItemNum)
        {
            if (bMoveFocus)
            {
                prev_focus_index = pListItem->FocusIndex;
                tmp_focus_index = pListItem->FocusIndex;
                
                if (pListItem->TotalItemNum <= pListItem->PageItemNum)
                {
                    page_item_real_num = pListItem->TotalItemNum;
                }
                else
                {
                    page_item_real_num = pListItem->PageItemNum;
                }
        
                if (tmp_focus_index < pListItem->TotalItemNum - 1)
                {
                    tmp_focus_index++;
                }
                else
                {
                    tmp_focus_index = 0;
                }
                
                pListItem->FocusIndex = tmp_focus_index;

                if (pListItem->TotalItemNum <= pListItem->PageItemNum)
                {
                    if (pListItem->PageIndex < page_item_real_num - 1)
                    {
                        pListItem->PageIndex++;
                    }
                    else
                    {
                        pListItem->PageIndex = 0;
                    }
                }
                else
                {
                    if ((pListItem->PageIndex==page_item_real_num-1)
						|| (prev_focus_index==pListItem->TotalItemNum-1))
                    {
                        if (prev_focus_index > pListItem->FocusIndex)
                        {
                            pListItem->PageIndex = 0;
                        }
                        
                        pListItem->PageFirstItemIndex = pListItem->FocusIndex - pListItem->PageIndex;
                    }
                    else
                    {
                        pListItem->PageIndex++;
                    }
                }
            }
            else
            {
                if (pListItem->TotalItemNum > pListItem->PageItemNum)
                {
                    if (pListItem->PageFirstItemIndex + pListItem->PageItemNum < pListItem->TotalItemNum)
                    {
                        pListItem->PageFirstItemIndex++;
                    }

					//当需要换页显示时，焦点始终要在当前页内
		            if(pListItem->FocusIndex < pListItem->PageFirstItemIndex)
			        {
			            pListItem->FocusIndex = pListItem->PageFirstItemIndex;
			        }
                }
            }
            
            ret = AK_TRUE;
        }

        ListItem_CalcPageIndex(pListItem);
    }
    
    return ret;
}


T_BOOL ListItem_PageUp(T_LIST_ITEM *pListItem)
{
    T_BOOL ret = AK_FALSE;

    if (pListItem)
    {
		T_U32 prePageFirstItemIndex = pListItem->PageFirstItemIndex;
    
		if (pListItem->PageItemNum < pListItem->TotalItemNum)
        {
            if (pListItem->PageFirstItemIndex < pListItem->PageItemNum)
            {
                pListItem->PageFirstItemIndex = 0;
            }
            else
            {
                pListItem->PageFirstItemIndex -= pListItem->PageItemNum;
            }
			
            //当需要换页显示时，焦点始终要在当前页内
			if (prePageFirstItemIndex != pListItem->PageFirstItemIndex)
				pListItem->FocusIndex = pListItem->PageFirstItemIndex;
            
			ret = AK_TRUE;
        }

        ListItem_CalcPageIndex(pListItem);
    }

    return ret;
}

T_BOOL ListItem_PageDown(T_LIST_ITEM *pListItem)
{
    T_BOOL ret = AK_FALSE;
    
    if (pListItem)
    {
		T_U32 prePageFirstItemIndex = pListItem->PageFirstItemIndex;

        if (pListItem->PageItemNum < pListItem->TotalItemNum)
        {           
			pListItem->PageFirstItemIndex += pListItem->PageItemNum;

            if (pListItem->PageFirstItemIndex + pListItem->PageItemNum > pListItem->TotalItemNum)
            {
                pListItem->PageFirstItemIndex = pListItem->TotalItemNum - pListItem->PageItemNum;
            }

            //当需要换页显示时，焦点始终要在当前页内
			if (prePageFirstItemIndex != pListItem->PageFirstItemIndex)
				pListItem->FocusIndex = pListItem->PageFirstItemIndex;
            
			ret = AK_TRUE;
        }

        ListItem_CalcPageIndex(pListItem);
    }

    return ret;
}


T_BOOL ListItem_Enter(T_LIST_ITEM *pListItem)
{
    T_BOOL ret = AK_FALSE;
    
    if (pListItem)
    {
        pListItem->WorkingIndex = pListItem->FocusIndex;
    }

    return ret;
}

static T_BOOL ListItem_CalcPageIndex(T_LIST_ITEM *pListItem)
{
    if (AK_NULL == pListItem)
        return AK_FALSE;
    
    if (pListItem->TotalItemNum <= pListItem->PageItemNum)
    {
        pListItem->PageFirstItemIndex = 0;
        pListItem->PageIndex = pListItem->FocusIndex;
    }
    else
    {
        if ((pListItem->FocusIndex<pListItem->PageFirstItemIndex+pListItem->PageItemNum)
            && (pListItem->FocusIndex>=pListItem->PageFirstItemIndex))
        {
            if (pListItem->TotalItemNum<pListItem->PageFirstItemIndex+pListItem->PageItemNum)
            {
                pListItem->PageFirstItemIndex = pListItem->TotalItemNum - pListItem->PageItemNum;
            }
            
            pListItem->PageIndex = pListItem->FocusIndex - pListItem->PageFirstItemIndex;
        }
        else
        {
            if (pListItem->FocusIndex + pListItem->PageItemNum > pListItem->TotalItemNum)
            {
                pListItem->PageFirstItemIndex = pListItem->TotalItemNum - pListItem->PageItemNum;
                pListItem->PageIndex = pListItem->FocusIndex - pListItem->PageFirstItemIndex;
            }
            else
            {
                pListItem->PageFirstItemIndex = pListItem->FocusIndex;
                pListItem->PageIndex = 0;
            }
        }
    }

    return AK_TRUE;
}

T_BOOL ListItem_ResetFoucsIndex(T_LIST_ITEM *pListItem)
{
    T_BOOL ret = AK_FALSE;
    
    if (pListItem != AK_NULL)
    {
        pListItem->FocusIndex = pListItem->WorkingIndex;
		ListItem_CalcPageIndex(pListItem);
        ret = AK_TRUE;
    }

    return ret;
}


/**
 * @brief   Init a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   PageItemNum :to indicate the maxnum of one page in the list item
 * @return  AK_TRUE:Init ok
 * @return  AK_FALSE:init failed
 */
T_BOOL ListItem_Init(T_LIST_ITEM *pListItem, T_U32 PageItemNum)
{
    T_BOOL ret = AK_TRUE;
    
    if (pListItem)
    {
        pListItem->pHead = AK_NULL;
    
        pListItem->FocusIndex = 0;
        pListItem->WorkingIndex = 0;

        pListItem->PageIndex = 0;
        pListItem->PageFirstItemIndex = 0;
        pListItem->PageItemNum = PageItemNum;
        pListItem->TotalItemNum = 0;
    }
    else
    {
        ret = AK_FALSE;
    }
    
    return ret;
}


/**
 * @brief   append a item to a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   pItem :the item's addr that to be appended
 * @return  AK_TRUE:append a item ok
 * @return  AK_FALSE:append a item failed
 */
T_BOOL ListItem_Append(T_LIST_ITEM *pListItem, T_LIST_ELEMENT *pItem)
{
    T_BOOL ret = AK_TRUE;
    T_U32 txt_len = 0;
    T_LIST_ELEMENT *p = AK_NULL, *q = AK_NULL;
    T_LIST_ELEMENT *tmp = AK_NULL;
    
    if (pListItem && pItem)
    {
        p = pListItem->pHead;   
        while(p != AK_NULL)
        {
            q = p;
            p = (T_LIST_ELEMENT *)p->pNext;
        }  
        
        tmp = Fwl_Malloc(sizeof(T_LIST_ELEMENT));
        if (tmp)
        {
            pListItem->TotalItemNum++;
            
            if (pItem->pText)
            {
                txt_len = (Utl_UStrLen(pItem->pText)+1)<<1;
                tmp->pText = Fwl_Malloc(txt_len);
                if (tmp->pText)
                {
                    Utl_UStrCpy(tmp->pText, pItem->pText);
                }               
            }

            if (pItem->pData && pItem->DataLen)
            {
                tmp->pData = Fwl_Malloc(pItem->DataLen);
                if (tmp->pData)
                {
                    memcpy(tmp->pData, pItem->pData, pItem->DataLen);
                    tmp->DataLen = pItem->DataLen;
                }
                else
                {
                    tmp->DataLen = 0;
                } 
            }
            else
            {
                tmp->pData = AK_NULL;
                tmp->DataLen = 0;
            }
            
            tmp->id = pItem->id;
            tmp->index = pListItem->TotalItemNum - 1; //it means append
            tmp->pImg = AK_NULL;//pItem->pImg;
            if (q == AK_NULL)
            {
                tmp->pPrev = AK_NULL;
                tmp->pNext = AK_NULL;

                pListItem->pHead = tmp;
            }
            else
            {
                tmp->pPrev = (T_U8 *)q;
                tmp->pNext = AK_NULL;

                q->pNext = (T_U8 *)tmp;
            }
        }
        else
        {
            ret = AK_FALSE;
        }
        
        //ListItem_Print(pListItem);
    }
    else
    {
        ret = AK_FALSE;
    }
    
    return ret;
    
}


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
T_BOOL ListItem_Insert(T_LIST_ITEM *pListItem, T_U32 Index, T_LIST_ELEMENT *pItem)
{
    T_BOOL ret = AK_TRUE;
    T_U32 txt_len = 0;
    T_LIST_ELEMENT *p = AK_NULL;
    T_LIST_ELEMENT *pNode = AK_NULL, *tmp = AK_NULL;
   
    if (pListItem && pItem)
    {
        pNode = ListItem_GetItemByIndex(pListItem, Index);
        if (pNode)
        {
            tmp = Fwl_Malloc(sizeof(T_LIST_ELEMENT));
            if (tmp)
            {
                if (pItem->pText)
                {
                    txt_len = (Utl_UStrLen(pItem->pText)+1)<<1;
                    tmp->pText = Fwl_Malloc(txt_len);
                    if (tmp->pText)
                    {
                        Utl_UStrCpy(tmp->pText, pItem->pText);
                    }               
                }
    
                if (pItem->pData && pItem->DataLen)
                {
                    tmp->pData = Fwl_Malloc(pItem->DataLen);
                    if (tmp->pData)
                    {
                        memcpy(tmp->pData, pItem->pData, pItem->DataLen);
                        tmp->DataLen = pItem->DataLen;
                    }
                    else
                    {
                        tmp->DataLen = 0;
                    } 
                }
                else
                {
                    tmp->pData = AK_NULL;
                    tmp->DataLen = 0;
                }
                
                tmp->id = pItem->id;
                tmp->index = pNode->index+1; //it means insert
                tmp->pImg = AK_NULL;//pItem->pImg;
                
                tmp->pNext = pNode->pNext;
                tmp->pPrev = (T_U8 *)pNode;
                pNode->pNext = (T_U8 *)tmp;
                
                pListItem->TotalItemNum++;

                p = (T_LIST_ELEMENT *)tmp->pNext;
                while(p != AK_NULL)
                {
                    p->index++;
                    p = (T_LIST_ELEMENT *)p->pNext;
                }
                
            }
        }
    }
    else
    {
        ret = AK_FALSE;
    }

    return ret;
    
}



/**
 * @brief   delete a item from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   Index :the item's index that to be deleted
 * @return  AK_TRUE:delete a item ok
 * @return  AK_FALSE:delete a item failed
 */
T_BOOL ListItem_Delete(T_LIST_ITEM *pListItem, T_U32 Index)
{
    T_BOOL ret = AK_TRUE;
    T_LIST_ELEMENT *p = AK_NULL, *q = AK_NULL;
    T_LIST_ELEMENT *pNode = AK_NULL;

    if (pListItem)
    {
        pNode = ListItem_GetItemByIndex(pListItem, Index);
        if (pNode)
        {
            if (pNode->pPrev == AK_NULL)    //the head node
            {
                p = (T_LIST_ELEMENT *)pNode->pNext;
                if (p)
                {
                    p->pPrev = AK_NULL;
                    pListItem->pHead = p;
                }
                else
                {
					pListItem->pHead = AK_NULL;//return AK_FALSE;
                }
            }
            else if (pNode->pNext == AK_NULL)   //the tail node
            {
                p = (T_LIST_ELEMENT *)pNode->pPrev;
                if (p)
                {
                    p->pNext = AK_NULL;
                }
                else
                {
                    return AK_FALSE;
                }
            }
            else
            {
                p = (T_LIST_ELEMENT *)pNode->pPrev;
                q = (T_LIST_ELEMENT *)pNode->pNext;
                
                p->pNext = (T_U8 *)q;
                q->pPrev = (T_U8 *)p;
            }
            
            
            p = (T_LIST_ELEMENT *)pNode->pNext;
            while (p != AK_NULL)
            {
                p->index--;;
                p = (T_LIST_ELEMENT *)p->pNext;                
            }
            
            pListItem->TotalItemNum--;
            //free the Node
            ListItem_FreeNode(pNode);
        }
    }
    else
    {
        ret = AK_FALSE;
    }
    
    return ret;
    
}



/**
 * @brief   delete all items from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  AK_TRUE:delete a item ok
 * @return  AK_FALSE:delete a item failed
 */
T_BOOL ListItem_DeleteAll(T_LIST_ITEM *pListItem)
{
    T_BOOL ret = AK_TRUE;
    T_LIST_ELEMENT *p = AK_NULL, *q = AK_NULL;

    if (pListItem)
    {
        pListItem->FocusIndex = 0;
        pListItem->WorkingIndex = 0;
        pListItem->PageIndex = 0;
        pListItem->PageFirstItemIndex = 0;
        pListItem->TotalItemNum = 0;
      
        p = pListItem->pHead;

        while(p != AK_NULL)
        {
            q = (T_LIST_ELEMENT *)p->pNext;
            ListItem_FreeNode(p);
            p = q;
        }
        
        pListItem->pHead = AK_NULL;  
    }
    else
    {
        ret = AK_FALSE;
    }
    
    return ret;
}



/**
 * @brief   get a item's focus index from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  T_U32:the list item cur focus item's index
 */
T_U32 ListItem_GetFocusIndex(T_LIST_ITEM *pListItem)
{
    T_U32 ret = INVALID_INDEX;
    
    if (pListItem)
    {
        ret = pListItem->FocusIndex;
    }
    
    return ret;
}


/**
 * @brief   get a item's working index from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  T_U32:the list item cur working item's index
 */
T_U32 ListItem_GetWorkingIndex(T_LIST_ITEM *pListItem)
{
    T_U32 ret = INVALID_INDEX;
    
    if (pListItem)
    {
        ret = pListItem->WorkingIndex;
    }
    
    return ret;
}


/**
 * @brief   get the focus item's id from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  T_S32:the list item focus item's id
 */
T_S32 ListItem_GetFocusId(T_LIST_ITEM *pListItem)
{
    T_LIST_ELEMENT *pItem = AK_NULL;
    T_S32 ret = INVALID_ID;
    
    if (pListItem)
    {
        pItem = ListItem_GetItemByIndex(pListItem, pListItem->FocusIndex);
        if (pItem)
        {
            ret = pItem->id;
        }
    }
    
    return ret;
}

/**
 * @brief   get the working item's id from a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  T_S32:the list item working item's id
 */
T_U32 ListItem_GetWorkingId(T_LIST_ITEM *pListItem)
{
    T_LIST_ELEMENT *pItem = AK_NULL;
    T_U32 ret = INVALID_ID;
    
    if (pListItem)
    {
        pItem = pListItem->pHead+pListItem->WorkingIndex;
        if (pItem)
        {
            ret = pItem->id;
        }
    }
    
    return ret;
}

/**
 * @brief   set the focus item's index to a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   FocusIndex :the index to be set
 * @return  AK_TRUE:set success
 * @return  AK_FALSE:set failed
 */
T_BOOL ListItem_SetFocusIndex(T_LIST_ITEM *pListItem, T_U32 FocusIndex)
{
    T_BOOL ret = AK_FALSE;
    
    if (pListItem)
    {
        if (FocusIndex < pListItem->TotalItemNum)
        {
            pListItem->FocusIndex = FocusIndex;

            if ((pListItem->FocusIndex>=pListItem->PageFirstItemIndex)
                && (pListItem->FocusIndex<=pListItem->PageFirstItemIndex+pListItem->PageItemNum-1))
	        {
	            ret = AK_TRUE;
	        }
			else if (pListItem->FocusIndex>=pListItem->TotalItemNum-pListItem->PageItemNum)
			{
				pListItem->PageFirstItemIndex = pListItem->TotalItemNum-pListItem->PageItemNum;
                ret = AK_TRUE;
			}
            else
            {
                pListItem->PageFirstItemIndex = pListItem->FocusIndex;
                ret = AK_TRUE;
            }
            
            pListItem->PageIndex = pListItem->FocusIndex - pListItem->PageFirstItemIndex;
        }
    }

    return ret;
}

#if 0
T_BOOL ListItem_SetPageFirstItemIndex(T_LIST_ITEM *pListItem, T_U32 PageFirstItemIndex)
{
    T_BOOL ret = AK_FALSE;
    
    if (pListItem)
    {
        if (pListItem->TotalItemNum > pListItem->PageItemNum)
        {
            if (PageFirstItemIndex < pListItem->TotalItemNum)
            {
                pListItem->PageFirstItemIndex = PageFirstItemIndex;
                ret = AK_TRUE;
            }
        }
    }

    return ret;
}
#endif

T_BOOL ListItem_SetPageIndex(T_LIST_ITEM *pListItem, T_U32 PageIndex)
{
    T_BOOL ret = AK_FALSE;
    T_U32  page_real_item_num;
    
    if (pListItem)
    {
        if (pListItem->TotalItemNum <= pListItem->PageItemNum)
        {
            page_real_item_num = pListItem->TotalItemNum;
        }
        else
        {
            page_real_item_num = pListItem->PageItemNum;
        }
        
        if (PageIndex < page_real_item_num)
        {
            pListItem->PageIndex = PageIndex;
        }
        
        ret = AK_TRUE;
    }

    return ret;
}


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
T_BOOL ListItem_SetText(T_LIST_ITEM *pListItem, T_U32 Index, T_U16 *pText)
{
    T_BOOL ret = AK_TRUE;
    T_U32  txt_len1, txt_len2;
    T_LIST_ELEMENT *pItem = AK_NULL;

    if (pListItem && pText)
    {
        if (pListItem->pHead && Index<pListItem->TotalItemNum)
        {
            pItem = ListItem_GetItemByIndex(pListItem, Index);
        }

        if (pItem->pText && pItem)
        {
            txt_len1 = (Utl_UStrLen(pItem->pText)+1)<<1;
            txt_len2 = (Utl_UStrLen(pText)+1)<<1;
            
            if (txt_len1 < txt_len2)
            {
                Fwl_Free(pItem->pText);
                pItem->pText = Fwl_Malloc(txt_len2);
            }
            
            if (pItem->pText)
            {
                Utl_UStrCpy(pItem->pText, pText);
            }
            else
            {
                ret = AK_FALSE;
            }
        }
        else
        {
            txt_len1 = (Utl_UStrLen(pText)+1)<<1;
            pItem->pText = Fwl_Malloc(txt_len1);
            if (pItem->pText)
            {
                Utl_UStrCpy(pItem->pText, pText);
            }
            else
            {
                ret = AK_FALSE;
            }
        }
    }
    else
    {
        ret = AK_FALSE;
    }
    
    return ret;
}


/**
 * @brief   get a indicated index item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   Index :the index that be indicated
 * @return  T_LIST_ELEMENT* :the indicated item's addr
 */
T_LIST_ELEMENT *ListItem_GetItemByIndex(T_LIST_ITEM *pListItem, T_U32 Index)
{
    T_LIST_ELEMENT *pItem = AK_NULL;
    
    if (pListItem)
    {
        pItem = pListItem->pHead;
        while (pItem != AK_NULL)
        {
            if (pItem->index == Index)
            {
                return pItem;
            }
            else
            {
                pItem = (T_LIST_ELEMENT *)pItem->pNext;
            }
        }
    }
    
    return pItem;
}


/**
 * @brief   get a indicated index item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  T_LIST_ELEMENT* :the Focus item's ptr
 */
T_LIST_ELEMENT *ListItem_GetFocusItem(T_LIST_ITEM *pListItem)
{
    T_LIST_ELEMENT *pItem = AK_NULL;

    if (pListItem)
    {
        pItem = ListItem_GetItemByIndex(pListItem, pListItem->FocusIndex);
    }
    
    return pItem;
}


/**
 * @brief   free a list item
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @return  AK_TRUE :free success
 * @return  AK_FALSE :free failed
 */
T_BOOL ListItem_Free(T_LIST_ITEM *pListItem)
{
    T_BOOL ret = AK_TRUE;
    
    if (pListItem)
    {
        pListItem->PageItemNum = 0;
        ret = ListItem_DeleteAll(pListItem);
    }
    else
    {
        ret = AK_FALSE;
    }
    
    return ret;
}


/**
 * @brief   the list item handle function that respond the event
 * @author  guohui
 * @date    2008-08-27 
 * @param   pListItem :the listitem handle
 * @param   Event :the event id
 * @param   pParam :the event param
 * @return  T_eBACK_STATE: refer to its define
 */
T_eBACK_STATE ListItem_Handle(T_LIST_ITEM *pListItem, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_MMI_KEYPAD        phyKey;
    T_eBACK_STATE       ret = eStay;
    
    if (pListItem)
    {
        if (Event == M_EVT_USER_KEY)
        {
            phyKey.keyID = (T_eKEY_ID)pParam->c.Param1;
            phyKey.pressType = (T_U8)pParam->c.Param2;

            switch (phyKey.keyID)
            {
                case kbUP:
                    //ListItem_MoveUp(pListItem, AK_TRUE);
                    break;
                    
                case kbDOWN:
                    //ListItem_MoveDown(pListItem, AK_TRUE);
                    break;
                    
                case kbOK:
                    ret = eNext;
                    break;
                                
                case kbLEFT:
                    break;
                
                case kbRIGHT:
                    break;
                
                case kbCLEAR:
                    ret = eReturn;
                    break;
                
                default:
                    break; 
            }            
        }
    }
    
    return ret;
}


T_BOOL ListItem_SetPageItemNum(T_LIST_ITEM *pListItem, T_U32 Num)
{
    T_BOOL ret = AK_FALSE;
    
    if (pListItem)
    {
        pListItem->PageItemNum = Num;

        //reset page first element
        if (pListItem->FocusIndex-pListItem->PageFirstItemIndex >= Num)
        {
            pListItem->PageFirstItemIndex = pListItem->FocusIndex-Num+1;
        }
        
        ret = AK_TRUE;
    }

    return ret;
}

