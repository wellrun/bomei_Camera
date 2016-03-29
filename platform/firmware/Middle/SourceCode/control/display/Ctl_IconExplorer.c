
/**************************************************************
*
* Copyrights (C) 2006, ANYKA Software Inc.
* All rights reserced.
*
* File name: Ctl_IconExplorer.c
* File flag: Icon Explorer
* File description:
*
* Revision: 1.00
* Author: Guanghua Zhang
* Date: 2006-01-06
*
****************************************************************/

#include <string.h>
#include "Ctl_IconExplorer.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "fwl_keyhandler.h"
#include "fwl_pfkeypad.h"
#include "Eng_topbar.h"
#include "Fwl_osMalloc.h"
#include "Eng_String.h"
#include "Eng_AkBmp.h"
#include "Eng_DataConvert.h"
#include "Eng_String_UC.h"
#include "eng_debug.h"
#include "Fwl_Initialize.h"
#include "fwl_pfdisplay.h"
#include "Fwl_display.h"
#include "Ctl_IconList.h"
#include "Fwl_tscrcom.h"

#define IE_TITLE_TEXT_TAIL    6  //the number of space in the title text tail 
#define TEXT_LEN_MAX    		128


// icon explorer init
T_BOOL IconExplorer_Init(T_ICONEXPLORER *pIconExplorer, T_RECT TitleRect, T_RECT ItemRect, T_U32 ItemStyle)
{
    AK_ASSERT_PTR(pIconExplorer, "IconExplorer_Init(): pIconExplorer is NULL", AK_TRUE);

    memset(pIconExplorer, 0x00, sizeof(T_ICONEXPLORER));

    IconExplorer_SetItemQtyMax(pIconExplorer, ICONEXPLORER_ITEM_QTYMAX);
    pIconExplorer->ItemStyle = ItemStyle;
	
    if ((pIconExplorer->ItemStyle & ICONEXPLORER_ITEM_FRAME) == ICONEXPLORER_ITEM_FRAME)
        pIconExplorer->ItemFrameWidth = ICONEXPLORER_ITEM_FRAMEWIDTH;
 	
    IconExplorer_SetItemTransColor(pIconExplorer, ICONEXPLORER_ITEM_TRANSCOLOR);
    IconExplorer_SetItemIconStyle(pIconExplorer, ICONEXPLORER_SMALLICON);

    pIconExplorer->ScrollBarWidth = ICONEXPLORER_ITEM_SCROLLBAR_WIDTH;

    IconExplorer_SetTitleRect(pIconExplorer, TitleRect, ICONEXPLORER_TITLE_BACKCOLOR, AK_NULL);
    IconExplorer_SetTitleText(pIconExplorer, AK_NULL, ICONEXPLORER_TITLE_TEXTCOLOR);
   
    IconExplorer_SetItemRect(pIconExplorer, ItemRect, ICONEXPLORER_ITEM_BACKCOLOR, AK_NULL);
    IconExplorer_SetItemText(pIconExplorer, ICONEXPLORER_ITEM_TEXTCOLOR, ICONEXPLORER_ITEM_FOCUSBACKCOLOR, ICONEXPLORER_ITEM_FOCUSTEXTCOLOR);

    IconExplorer_SetScrollBarWidth(pIconExplorer, ICONEXPLORER_ITEM_SCROLLBAR_WIDTH);

    IconExplorer_SetSmallIcon(pIconExplorer, ICONEXPLORER_ITEM_SMALLICON_WIDTH, ICONEXPLORER_ITEM_SMALLICON_HEIGHT, \
            ICONEXPLORER_ITEM_SMALLINTERVAL, ICONEXPLORER_ITEM_SMALLINTERVAL, ICONEXPLORER_ITEM_SMALLINTERVAL);

    IconExplorer_SetLargeIcon(pIconExplorer, ICONEXPLORER_ITEM_LARGEICON_WIDTH, ICONEXPLORER_ITEM_LARGEICON_HEIGHT, \
            ICONEXPLORER_ITEM_LARGETEXT_HEIGHT, ICONEXPLORER_ITEM_LARGEINTERVAL, ICONEXPLORER_ITEM_LARGEINTERVAL, ICONEXPLORER_ITEM_LARGEINTERVAL);

    IconExplorer_SetNoneIcon(pIconExplorer, ICONEXPLORER_ITEM_NONETEXT_HEIGHT, ICONEXPLORER_ITEM_NONEINTERVAL, ICONEXPLORER_ITEM_NONEINTERVAL);

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ALL);

    IconExplorer_SetSortMode(pIconExplorer, ICONEXPLORER_SORT_ID);
    pIconExplorer->ItemListFlag = ICONEXPLORER_CHANGE_ADD;

    IconExplorer_SetSortIdCallBack(pIconExplorer, AK_NULL);
    IconExplorer_SetSortContentCallBack(pIconExplorer, AK_NULL);
    IconExplorer_SetListCallBack(pIconExplorer, AK_NULL);

    return AK_TRUE;
}

// icon explorer free
T_BOOL IconExplorer_Free(T_ICONEXPLORER *pIconExplorer)
{
    AK_ASSERT_PTR(pIconExplorer, "IconExplorer_Free(): pIconExplorer is NULL", AK_FALSE);

    // free title text
    if (pIconExplorer->pTitleText != AK_NULL)
    {
        pIconExplorer->pTitleText = Fwl_Free(pIconExplorer->pTitleText);
    }
    
    // free all item
    IconExplorer_DelAllItem(pIconExplorer);
	
	if (AK_NULL != pIconExplorer->pSlipMgr)
    {
        pIconExplorer->pSlipMgr = SlipMgr_Destroy(pIconExplorer->pSlipMgr);
    }
	
    return AK_TRUE;
}

// check item list
T_BOOL IconExplorer_CheckItemList(T_ICONEXPLORER *pIconExplorer)
{
	T_U32 focusId = 0;
	T_U32 index = 0;
	T_BOOL	bNeedSetFocus = AK_FALSE;
	
    if (AK_NULL == pIconExplorer)
    {
        return AK_FALSE;
    }

    if (ICONEXPLORER_CHANGE_NONE != pIconExplorer->ItemListFlag)
    {
        if (pIconExplorer->ListCallBack != AK_NULL) 
        {
        	if (ICONEXPLORER_CHANGE_DEL == (pIconExplorer->ItemListFlag & ICONEXPLORER_CHANGE_DEL))
        	{
	        	focusId = IconExplorer_GetItemFocusId(pIconExplorer);
				index = IconExplorer_GetItemIndexById(pIconExplorer, focusId);
				bNeedSetFocus = AK_TRUE;
        	}
			
            // delete all exist item
            IconExplorer_DelAllItem(pIconExplorer);

            // get item list again
            pIconExplorer->ListCallBack((T_pCVOID)pIconExplorer);

			if (bNeedSetFocus
				&& index < pIconExplorer->ItemQty)
			{
				IconExplorer_SetFocusByIndex(pIconExplorer, index);
			}

            // check scroll bar & show focus
            IconList_CheckScrollBar(pIconExplorer);
            IconList_CheckShowFocus(pIconExplorer);
        }
		
		pIconExplorer->ItemListFlag = ICONEXPLORER_CHANGE_NONE;

		IconList_CreatSlipMgr(pIconExplorer);
		IconList_CheckSlipFocus(pIconExplorer);
    }

    return AK_TRUE;
}

// icon explorer show
T_BOOL IconExplorer_Show(T_ICONEXPLORER *pIconExplorer)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    if (ICONEXPLORER_REFRESH_NONE == IconExplorer_GetRefresh(pIconExplorer))
    {
        return AK_TRUE;
    }

    // check item list
    /*
    if (pIconExplorer->ItemListFlag == AK_TRUE) {
        if (pIconExplorer->ListCallBack != AK_NULL) {
            // delete all exist item
            IconExplorer_DelAllItem(pIconExplorer);

            // get item list again
            pIconExplorer->ListCallBack((T_pCVOID)pIconExplorer);

            // check scroll bar & show focus
            IconList_CheckScrollBar(pIconExplorer);
            IconList_CheckShowFocus(pIconExplorer);
        }

        pIconExplorer->ItemListFlag = AK_FALSE;
    }*/
    IconExplorer_CheckItemList(pIconExplorer);


    // show content
    //IconExplorer_ShowTitle(pIconExplorer);
    /**Set topbar's title*/
    TopBar_SetTitle(pIconExplorer->pTitleText);

    /**Show topbar*/
    TopBar_EnableShow();
    TopBar_Show(TB_REFRESH_ALL);
	
    //IconExplorer_ShowItem(pIconExplorer);
    //IconExplorer_ShowFocus(pIconExplorer);
	
	SlipMgr_ShowItemById(pIconExplorer->pSlipMgr, IconExplorer_GetItemOldFocusId(pIconExplorer), AK_FALSE);
	SlipMgr_ShowItemById(pIconExplorer->pSlipMgr, IconExplorer_GetItemFocusId(pIconExplorer), AK_TRUE);
	
	SlipMgr_Refresh(pIconExplorer->pSlipMgr);
	
	// reset refresh flag
    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_NONE);

    return AK_TRUE;
}


// icon explorer handler
T_eBACK_STATE IconExplorer_Handler(T_ICONEXPLORER *pIconExplorer, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_eBACK_STATE	ret 		= eStay;
    T_MMI_KEYPAD	phyKey;
	T_S32			count 		= 0;
	T_U32			loadItemNum = 0;
	T_U32			focusId 	= 0;
    
    if (pIconExplorer == AK_NULL)
    {
        return eStay;
    }

    switch (Event) 
    {         
    case M_EVT_USER_KEY:
        phyKey.keyID = pParam->c.Param1;
        phyKey.pressType = pParam->c.Param2;

        ret = IconList_UserKey_Handler(pIconExplorer, &phyKey);
        break;
        
    case M_EVT_TOUCH_SCREEN:
       // Fwl_Print(C1, M_EDICT, "!!!! touch !!!!!");
       // Fwl_Print(C3, M_CTRL, "!!!! touch event !!!!");
        phyKey.keyID = kbNULL;
        phyKey.pressType = PRESS_SHORT;
        
        switch (pParam->s.Param1)
        {
        case eTOUCHSCR_UP:
            IconList_HitButton_Handler(pIconExplorer, &phyKey, pParam);
            ret = IconList_UserKey_Handler(pIconExplorer, &phyKey);
            break;
            
        case eTOUCHSCR_DOWN:
            break;
            
        case eTOUCHSCR_MOVE:
            break;
        default:
            break;
        }
        break;

    case M_EVT_PUB_TIMER:
#if 0
        if (IconExplorer_ScrollItemText(pIconExplorer) == AK_TRUE)
        {
            Fwl_RefreshDisplay();
        }
#endif
		if (SLIPMSG_STA_STOP == SlipMgr_GetCurStatus(pIconExplorer->pSlipMgr)
			&& SlipMgr_ScrollShowItemById(pIconExplorer->pSlipMgr, IconExplorer_GetItemFocusId(pIconExplorer), AK_TRUE))
		{
			SlipMgr_Refresh(pIconExplorer->pSlipMgr);
			Fwl_RefreshDisplay();			
		}
        break;
        
    default:
       //  Fwl_Print(C3, M_CTRL, "!!!! touch event id =%d !!!!", Event);
        break;
    }

	if (eStay != ret)
	{		
		return ret;
	}

	if ((M_EVT_EXIT == Event) || ((Event >= M_EVT_RETURN) && (Event <= M_EVT_RETURN9)))
	{
		SlipMgr_LoadItemBgImg(pIconExplorer->pSlipMgr);
		IconList_CheckSlipFocus(pIconExplorer);
	}

	if (VME_EVT_TIMER == Event
		&& pIconExplorer->pSlipMgr->refreshTimer == (T_TIMER)pParam->w.Param1)
	{
		IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ALL);
	}

	focusId = IconExplorer_GetItemFocusId(pIconExplorer);
		
	ret = SlipMgr_Handle(pIconExplorer->pSlipMgr, Event, pParam, &count, &loadItemNum, &focusId, 0);

	if (eNext == ret)
	{
		if (IconList_MoveItemOptionFocus(pIconExplorer) == AK_TRUE)
        {
            ret = eOption;
        }
        else 
        {
            IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ALL);
            ret = eNext;
        }		
	}

	if (IconExplorer_GetItemFocusId(pIconExplorer) != focusId)
	{
		IconExplorer_SetFocus(pIconExplorer, focusId);
	}

	if ((0 != count) && (0 != loadItemNum))
	{
		IconList_LoadSlipItem(pIconExplorer, count, loadItemNum);
	}

    return ret;
}

// modify title text
T_BOOL IconExplorer_ModifyTitleText(T_ICONEXPLORER *pIconExplorer, const T_U16 *pTitleText)
{
    T_U32 uTextLen = 0;
    
    AK_ASSERT_PTR(pIconExplorer, "IconExplorer_SetTitleText():pIconExplorer is NULL", AK_FALSE);
    AK_ASSERT_PTR(pTitleText, "IconExplorer_SetTitleText(): pTitleText is NULL", AK_FALSE);

    uTextLen = Utl_UStrLen(pTitleText);

    /* free old title text space*/
    if (pIconExplorer->pTitleText != AK_NULL)
    {
        pIconExplorer->pTitleText = Fwl_Free(pIconExplorer->pTitleText);
    }

    /* malloc title text space again*/
    pIconExplorer->pTitleText = (T_U16 *)Fwl_Malloc((uTextLen + 1) << 1);
	if (AK_NULL == pIconExplorer->pTitleText)
    {
        Fwl_Print(C3, M_CTRL, "IconExplorer_ModifyTitleText(): pIconExplorer->pTitleText malloc fail");
        return AK_FALSE;
    }

    Utl_UStrCpy(pIconExplorer->pTitleText, pTitleText);

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_TITLE);

    return AK_TRUE;
}

// add an item
T_BOOL IconExplorer_AddItem(T_ICONEXPLORER *pIconExplorer, T_U32 Id, T_VOID *pContent, T_U32 ContentLen, const T_U16 *pText, const T_U8 *pSmallIcon, const T_U8 *pLargeIcon)
{
    T_ICONEXPLORER_ITEM *p, *q, *f;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    // check item quantity max limit
    if ((pIconExplorer->ItemQtyMax > 0) && (pIconExplorer->ItemQty >= pIconExplorer->ItemQtyMax))
        return AK_FALSE;

    // malloc item space
    p = (T_ICONEXPLORER_ITEM *)Fwl_Malloc(sizeof(T_ICONEXPLORER_ITEM));

    if (p == AK_NULL)
        return AK_FALSE;

    // assign item parameter
    p->Id = Id;
    if (pContent != AK_NULL) 
	{
        // malloc item content space
        p->pContent = Fwl_Malloc(ContentLen + 1);
        if (p->pContent == AK_NULL) 
		{
            p = Fwl_Free(p);
            return AK_FALSE;
        }

        memcpy(p->pContent, pContent, ContentLen);
        p->contentLen=ContentLen;
    }
    else
    {
        p->pContent = AK_NULL;
        p->contentLen=0;
    }

    if (pText != AK_NULL)
    {
    	T_U32	len = 0;

    	len = Utl_UStrLen(pText) << 1;

    	if (len > TEXT_LEN_MAX)
    	{
			len = TEXT_LEN_MAX;
    	}
    	
        // malloc item text space
        p->pText = (T_U16 *)Fwl_Malloc(len + 2);

        if (AK_NULL == p->pText)
        {
            Fwl_Print(C3, M_CTRL, "IconExplorer_AddItem() : Fwl_Malloc error !! "  );
			
            if (p->pContent != AK_NULL)
                p->pContent = Fwl_Free(p->pContent);
			
            p = Fwl_Free(p);
			
            return AK_FALSE;
        }

        Utl_UStrCpyN(p->pText,  (T_U16 *)pText, len/2);
    }
    else
    {
        p->pText = AK_NULL;
    }

    p->pSmallIcon 	= (T_U8 *)pSmallIcon;
    p->pLargeIcon 	= (T_U8 *)pLargeIcon;
    p->OptionType 	= ICONEXPLORER_OPTION_NONE;
    p->pOptionValue = AK_NULL;
    p->pOptionHead 	= AK_NULL;
    p->pOptionFocus = AK_NULL;

    // sort & check chain table point
    switch (pIconExplorer->ItemSortMode)
	{
    case ICONEXPLORER_SORT_CONTENT:
        if (pIconExplorer->SortContentCallBack == AK_NULL)
		{
            // not sort rule, so add tail
            f = pIconExplorer->pItemHead;
            q = AK_NULL;
			
            while (f != AK_NULL)
			{
                if (f->Id == Id) 
				{
                    if (p->pText != AK_NULL)
                        p->pText = Fwl_Free(p->pText);
					
                    if (p->pContent != AK_NULL)
                        p->pContent = Fwl_Free(p->pContent);
					
                    p = Fwl_Free(p);
					
                    return AK_FALSE;
                }

                q = f;
                f = f->pNext;
            }

            p->pPrevious = q;
            p->pNext = AK_NULL;

            if (q == AK_NULL)
                pIconExplorer->pItemHead = p;
            else
                q->pNext = p;
        }
        else 
		{
            // base on rule to insert item
            q = pIconExplorer->pItemHead;
            if (q == AK_NULL)
			{
                // item head is null
                p->pPrevious = AK_NULL;
                p->pNext = AK_NULL;

                pIconExplorer->pItemHead = p;
            }
            else 
			{
                while (q != AK_NULL)
				{
                    if (q->Id == Id)
					{
                        if (p->pText != AK_NULL)
                            p->pText = Fwl_Free(p->pText);
						
                        if (p->pContent != AK_NULL)
                            p->pContent = Fwl_Free(p->pContent);
						
                        p = Fwl_Free(p);
						
                        return AK_FALSE;
                    }

                    if (pIconExplorer->SortContentCallBack((T_pCVOID)pIconExplorer, q->pContent, pContent))
					{
                        p->pPrevious = q->pPrevious;
                        p->pNext = q;
						
                        if (q->pPrevious != AK_NULL)
                            q->pPrevious->pNext = p;
						
                        q->pPrevious = p;

                        if (pIconExplorer->pItemHead == q)
                            pIconExplorer->pItemHead = p;

                        break;
                    }
                    else if (q->pNext == AK_NULL)
					{
                        p->pPrevious = q;
                        p->pNext = AK_NULL;
                        q->pNext = p;

                        break;
                    }

                    q = q->pNext;
                }
            }
        }
        break;
		
    case ICONEXPLORER_SORT_ID:
    default:
        if (pIconExplorer->SortIdCallBack == AK_NULL)
		{
            // not sort rule, so add tail
            f = pIconExplorer->pItemHead;
            q = AK_NULL;
			
            while (f != AK_NULL) 
			{
                if (f->Id == Id)
				{
                    if (p->pText != AK_NULL)
                        p->pText = Fwl_Free(p->pText);
					
                    if (p->pContent != AK_NULL)
                        p->pContent = Fwl_Free(p->pContent);
					
                    p = Fwl_Free(p);
					
                    return AK_FALSE;
                }

                q = f;
                f = f->pNext;
            }

            p->pPrevious = q;
            p->pNext = AK_NULL;

            if (q == AK_NULL)
                pIconExplorer->pItemHead = p;
            else
                q->pNext = p;
        }
        else
		{
            q = pIconExplorer->pItemHead;
            if (q == AK_NULL)
			{
                // item head is null
                p->pPrevious = AK_NULL;
                p->pNext = AK_NULL;

                pIconExplorer->pItemHead = p;
            }
            else 
			{
                while (q != AK_NULL) 
				{
                    if (q->Id == Id)
					{
                        if (p->pText != AK_NULL)
                            p->pText = Fwl_Free(p->pText);
						
                        if (p->pContent != AK_NULL)
                            p->pContent = Fwl_Free(p->pContent);
						
                        p = Fwl_Free(p);
						
                        return AK_FALSE;
                    }

                    if (pIconExplorer->SortIdCallBack(q->Id, Id) == AK_TRUE)
					{
                        p->pPrevious = q->pPrevious;
                        p->pNext = q;
						
                        if (q->pPrevious != AK_NULL)
                            q->pPrevious->pNext = p;
						
                        q->pPrevious = p;

                        if (pIconExplorer->pItemHead == q)
                            pIconExplorer->pItemHead = p;

                        break;
                    }
                    else if (q->pNext == AK_NULL)
					{
                        p->pPrevious = q;
                        p->pNext = AK_NULL;
                        q->pNext = p;

                        break;
                    }

                    q = q->pNext;
                }
            }
        }
        break;
    }

    // modify old focus, focus, item quantity
    if (pIconExplorer->pItemFocus == AK_NULL)
	{
        pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
        pIconExplorer->pItemFocus = pIconExplorer->pItemHead;
        IconList_CheckShowFocus(pIconExplorer);
    }
	
    pIconExplorer->ItemQty++;

	if (SlipMgr_GetItemNum(pIconExplorer->pSlipMgr) == pIconExplorer->ItemQty - 1)
	{
		pIconExplorer->ItemListFlag = ICONEXPLORER_CHANGE_ADD;
	}
	else
	{
		SlipMgr_SetTotalItemNum(pIconExplorer->pSlipMgr, pIconExplorer->ItemQty);
	}

    // check scroll bar, modify refresh flag
    IconList_CheckScrollBar(pIconExplorer);
    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// modify item small icon by id
T_BOOL IconExplorer_ModifyItemSmallIcon(T_ICONEXPLORER *pIconExplorer, T_U32 Id, const T_U8 *pSmallIcon)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    // find item by id
    p = pIconExplorer->pItemHead;
    while (p != AK_NULL)
	{
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    p->pSmallIcon = (T_U8 *)pSmallIcon;

    // show item again
    if (pIconExplorer->ItemIconStyle == ICONEXPLORER_SMALLICON)
        IconList_ReShowItem(pIconExplorer, p);

    return AK_TRUE;
}

// delete an item by id
T_BOOL IconExplorer_DelItem(T_ICONEXPLORER *pIconExplorer, T_U32 Id)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    // find item by id
    p = pIconExplorer->pItemHead;
    while (p != AK_NULL)
	{
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
    {
        Fwl_Print(C3, M_CTRL, "Not find item by id");
        return AK_FALSE;

    }

    // modify after delete point
    if (p->pPrevious != AK_NULL)
        p->pPrevious->pNext = p->pNext;
    else
        pIconExplorer->pItemHead = p->pNext;
    if (p->pNext != AK_NULL)
        p->pNext->pPrevious = p->pPrevious;

	if (p == pIconExplorer->pItemShow)
	{		
        if (p->pNext != AK_NULL)
            pIconExplorer->pItemShow = p->pNext;
        else if (p->pPrevious != AK_NULL)
            pIconExplorer->pItemShow = p->pPrevious;
        else
            pIconExplorer->pItemShow = AK_NULL;
    }

    // check item focus point
    if (p == pIconExplorer->pItemFocus)
	{
        pIconExplorer->pItemOldFocus = AK_NULL;
		
        if (p->pNext != AK_NULL)
            pIconExplorer->pItemFocus = p->pNext;
        else if (p->pPrevious != AK_NULL)
            pIconExplorer->pItemFocus = p->pPrevious;
        else
            pIconExplorer->pItemFocus = AK_NULL;

        IconList_CheckShowFocus(pIconExplorer);
    }

    // free item space
    if (p->pText != AK_NULL)
        p->pText = Fwl_Free(p->pText);
	
    if (p->pContent != AK_NULL)
        p->pContent = Fwl_Free(p->pContent);
	
    IconList_Item_DelAllOption(p);
    p = Fwl_Free(p);

    // modify item quantity
    pIconExplorer->ItemQty--;

	pIconExplorer->ItemListFlag = ICONEXPLORER_CHANGE_DEL;

    // check scroll bar, modify refresh flag
    IconList_CheckScrollBar(pIconExplorer);
    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// delete item focus
T_BOOL IconExplorer_DelItemFocus(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    // find item focus
    p = pIconExplorer->pItemHead;
    while (p != AK_NULL)
	{
        if (p == pIconExplorer->pItemFocus)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    // modify after delete point
    if (p->pPrevious != AK_NULL)
        p->pPrevious->pNext = p->pNext;
    else
        pIconExplorer->pItemHead = p->pNext;
	
    if (p->pNext != AK_NULL)
        p->pNext->pPrevious = p->pPrevious;

    // check item focus point
    if (p == pIconExplorer->pItemFocus)
	{
        pIconExplorer->pItemOldFocus = AK_NULL;
		
        if (p->pNext != AK_NULL)
            pIconExplorer->pItemFocus = p->pNext;
        else if (p->pPrevious != AK_NULL)
            pIconExplorer->pItemFocus = p->pPrevious;
        else
            pIconExplorer->pItemFocus = AK_NULL;

        IconList_CheckShowFocus(pIconExplorer);
    }

    // free item space
    if (p->pText != AK_NULL)
        p->pText = Fwl_Free(p->pText);
	
    if (p->pContent != AK_NULL)
        p->pContent = Fwl_Free(p->pContent);
	
    IconList_Item_DelAllOption(p);
    p = Fwl_Free(p);

    // modify item quantity
    pIconExplorer->ItemQty--;

	pIconExplorer->ItemListFlag = ICONEXPLORER_CHANGE_DEL;

    // check scroll bar, modify refresh flag
    IconList_CheckScrollBar(pIconExplorer);
    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// delete all item
T_BOOL IconExplorer_DelAllItem(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p, *q;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    // free all item space
    p = pIconExplorer->pItemHead;
    while (p != AK_NULL)
	{
        q = p->pNext;

        if (p->pText != AK_NULL)
            p->pText = Fwl_Free(p->pText);
		
        if (p->pContent != AK_NULL)
            p->pContent = Fwl_Free(p->pContent);
		
        IconList_Item_DelAllOption(p);
		
        p = Fwl_Free(p);
        p = q;
    }

    // modify item parameter
    pIconExplorer->pItemHead = AK_NULL;
    pIconExplorer->pItemShow = AK_NULL;
    pIconExplorer->pItemFocus = AK_NULL;
    pIconExplorer->pItemOldFocus = AK_NULL;
    pIconExplorer->ItemQty = 0;

	pIconExplorer->ItemListFlag = ICONEXPLORER_CHANGE_DEL;

    // check scroll bar, modify refresh flag
    IconList_CheckScrollBar(pIconExplorer);
    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// move focus
T_BOOL IconExplorer_MoveFocus(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_DIRECTION Direction)
{
    T_ICONEXPLORER_ITEM *pOldItemFocus;

    if (pIconExplorer == AK_NULL
		|| pIconExplorer->pItemHead == AK_NULL
		|| pIconExplorer->pItemFocus == AK_NULL)
        return AK_FALSE;

    // save current focus
    pOldItemFocus = pIconExplorer->pItemFocus;

    // move focus by direction
    switch (Direction)
	{
    case ICONEXPLORER_DIRECTION_LEFT:
        switch (pIconExplorer->ItemIconStyle)
		{
        case ICONEXPLORER_LARGEICON:
            IconList_LargeMoveLeft(pIconExplorer);
            break;
			
        case ICONEXPLORER_NONEICON:
        case ICONEXPLORER_SMALLICON:
        default:
            IconList_SmallMoveUp(pIconExplorer);
            break;
        }
        break;
		
    case ICONEXPLORER_DIRECTION_RIGHT:
        switch (pIconExplorer->ItemIconStyle)
		{
        case ICONEXPLORER_LARGEICON:
            IconList_LargeMoveRight(pIconExplorer);
            break;
			
        case ICONEXPLORER_NONEICON:
        case ICONEXPLORER_SMALLICON:
        default:
            IconList_SmallMoveDown(pIconExplorer);
            break;
        }
        break;
		
    case ICONEXPLORER_DIRECTION_UP:
        switch (pIconExplorer->ItemIconStyle)
		{
        case ICONEXPLORER_LARGEICON:
            IconList_LargeMoveUp(pIconExplorer);
            break;
			
        case ICONEXPLORER_NONEICON:
        case ICONEXPLORER_SMALLICON:
        default:
            IconList_SmallMoveUp(pIconExplorer);
            break;
        }
        break;
		
    case ICONEXPLORER_DIRECTION_DOWN:
        switch (pIconExplorer->ItemIconStyle)
		{
        case ICONEXPLORER_LARGEICON:
            IconList_LargeMoveDown(pIconExplorer);
            break;
			
        case ICONEXPLORER_NONEICON:
        case ICONEXPLORER_SMALLICON:
        default:
            IconList_SmallMoveDown(pIconExplorer);
            break;
        }
        break;
		
    default:
        return AK_FALSE;
    }

    // check refresh flag
    if (pIconExplorer->pItemFocus != pOldItemFocus)
        IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_FOCUS);

    return AK_TRUE;
}

// set focus by id
T_BOOL IconExplorer_SetFocus(T_ICONEXPLORER *pIconExplorer, T_U32 Id)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL
		|| pIconExplorer->pItemHead == AK_NULL)
        return AK_FALSE;

    // find item by id
    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
    pIconExplorer->pItemFocus = p;

	IconList_CheckShowFocus(pIconExplorer);

    // check refresh flag
    if (pIconExplorer->pItemFocus != pIconExplorer->pItemOldFocus)
        IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_FOCUS);

    return AK_TRUE;
}

// set focus by index
T_BOOL IconExplorer_SetFocusByIndex(T_ICONEXPLORER *pIconExplorer, T_U32 Index)
{
    T_ICONEXPLORER_ITEM *p;
    T_U32 i;

    if (pIconExplorer == AK_NULL
		|| pIconExplorer->pItemHead == AK_NULL)
        return AK_FALSE;

    // find item by index
    p = pIconExplorer->pItemHead;
    for (i=0; (p!=AK_NULL)&&(i<Index); i++)
        p = p->pNext;

    if (p == AK_NULL)
        return AK_FALSE;

    pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
    pIconExplorer->pItemFocus = p;

	IconList_CheckShowFocus(pIconExplorer);

    // check refresh flag
    if (pIconExplorer->pItemFocus != pIconExplorer->pItemOldFocus)
        IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_FOCUS);

    return AK_TRUE;
}

// set item quantity max limit
T_BOOL IconExplorer_SetItemQtyMax(T_ICONEXPLORER *pIconExplorer, T_U32 ItemQtyMax)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->ItemQtyMax = ItemQtyMax;

    return AK_TRUE;
}

// set icon transparent color
T_BOOL IconExplorer_SetItemTransColor(T_ICONEXPLORER *pIconExplorer, T_COLOR ItemTransColor)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->ItemTransColor = ItemTransColor;

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ALL);

    return AK_TRUE;
}

// set item icon style
T_BOOL IconExplorer_SetItemIconStyle(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ICONSTYLE ItemIconStyle)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->ItemIconStyle = ItemIconStyle;
    // if error style, then small icon
    if (pIconExplorer->ItemIconStyle >= ICONEXPLORER_ICONSTYLE_NUM)
        pIconExplorer->ItemIconStyle = ICONEXPLORER_SMALLICON;

    // check scroll bar & show focus, modify refresh flag
    IconList_CheckScrollBar(pIconExplorer);
    IconList_CheckShowFocus(pIconExplorer);
    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// set sort item mode
T_BOOL IconExplorer_SetSortMode(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_SORTMODE ItemSortMode)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->ItemSortMode = ItemSortMode;
    if (pIconExplorer->ItemSortMode >= ICONEXPLORER_SORT_NUM)
        pIconExplorer->ItemSortMode = ICONEXPLORER_SORT_ID;

    // sort item again
    IconExplorer_SortItem(pIconExplorer);

    return AK_TRUE;
}

// set title rect, back color, back graph data
T_BOOL IconExplorer_SetTitleRect(T_ICONEXPLORER *pIconExplorer, T_RECT TitleRect, T_COLOR TitleBackColor, const T_U8 *pTitleBackData)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->TitleRect = TitleRect;
    IconList_CheckRect(&pIconExplorer->TitleRect);
    pIconExplorer->TitleBackColor = TitleBackColor;
    pIconExplorer->pTitleBackData = (T_U8 *)pTitleBackData;

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_TITLE);

    return AK_TRUE;
}

// set title text
T_BOOL IconExplorer_SetTitleText(T_ICONEXPLORER *pIconExplorer, const T_U16 *pTitleText, T_COLOR TitleTextColor)
{
    T_U32 uTextLen = 0;
    
    AK_ASSERT_PTR(pIconExplorer, "IconExplorer_SetTitleText():pIconExplorer is NULL", AK_FALSE);
    
    pIconExplorer->TitleTextColor = TitleTextColor;
    
    if (pTitleText != AK_NULL) 
    {
        uTextLen = Utl_UStrLen(pTitleText);

        /* free old title text space*/
        if (pIconExplorer->pTitleText != AK_NULL)
        {
            pIconExplorer->pTitleText = Fwl_Free(pIconExplorer->pTitleText);
        }

        /*malloc new title text*/
        pIconExplorer->pTitleText = (T_U16*)Fwl_Malloc((uTextLen + 1) << 1);
        if (AK_NULL == pIconExplorer->pTitleText)
        {
            Fwl_Print(C3, M_CTRL, "IconExplorer_SetTitleText(): pIconExplorer->pTitleText malloc fail");
            return AK_FALSE;
        }

        Utl_UStrCpy(pIconExplorer->pTitleText, pTitleText);
    }

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_TITLE);

    return AK_TRUE;
}

// set item rect, back color, back graph data
T_BOOL IconExplorer_SetItemRect(T_ICONEXPLORER *pIconExplorer, T_RECT ItemRect, T_COLOR ItemBackColor, const T_U8 *pItemBackData)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->ItemRect = ItemRect;
    IconList_CheckRect(&pIconExplorer->ItemRect);
    pIconExplorer->ItemBackColor = ItemBackColor;
    pIconExplorer->pItemBackData = (T_U8 *)pItemBackData;

    // set scroll bar parameter again
    ScBar_Init(&pIconExplorer->ScrollBar, \
            (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemRect.width - pIconExplorer->ItemFrameWidth - pIconExplorer->ScrollBarWidth), \
            (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth), \
            (T_S16)(pIconExplorer->ScrollBarWidth), \
            (T_S16)(pIconExplorer->ItemRect.height - pIconExplorer->ItemFrameWidth*2), \
            -10, SCBAR_VERTICAL);

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// set itemm text style
T_BOOL IconExplorer_SetItemText(T_ICONEXPLORER *pIconExplorer, T_COLOR ItemTextColor, T_COLOR ItemFocusBackColor, T_COLOR ItemFocusTextColor)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->ItemTextColor = ItemTextColor;
    pIconExplorer->ItemFocusBackColor = ItemFocusBackColor;
    pIconExplorer->ItemFocusTextColor = ItemFocusTextColor;

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// set scroll bar width
T_BOOL IconExplorer_SetScrollBarWidth(T_ICONEXPLORER *pIconExplorer, T_U32 ScrollBarWidth)
{
    T_U32 OldScrollBarShowWidth;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    // save old scroll bar width
    OldScrollBarShowWidth = pIconExplorer->ScrollBarShowWidth;

    // set scroll bar width, check scroll bar show width
    pIconExplorer->ScrollBarWidth = ScrollBarWidth;
    if (pIconExplorer->ScrollBarFlag == AK_TRUE)
        pIconExplorer->ScrollBarShowWidth = pIconExplorer->ScrollBarWidth;

    // set scroll bar parameter again
    ScBar_Init(&pIconExplorer->ScrollBar, \
            (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemRect.width - pIconExplorer->ItemFrameWidth - pIconExplorer->ScrollBarWidth), \
            (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth), \
            (T_S16)(pIconExplorer->ScrollBarWidth), \
            (T_S16)(pIconExplorer->ItemRect.height - pIconExplorer->ItemFrameWidth*2), \
            -10, SCBAR_VERTICAL);

    // check other parameter by scroll bar change
    if (pIconExplorer->ScrollBarShowWidth != OldScrollBarShowWidth)
    {
        if (pIconExplorer->LargeIconWidth+pIconExplorer->LargeItemHInterval != 0)
        {
            pIconExplorer->LargeItemCol = (pIconExplorer->ItemRect.width-pIconExplorer->ItemFrameWidth*2-\
                    pIconExplorer->ScrollBarShowWidth-pIconExplorer->LargeItemHInterval) / \
                    (pIconExplorer->LargeIconWidth+pIconExplorer->LargeItemHInterval);
        }
        else
        {
            pIconExplorer->LargeItemCol = 1;
        }
        if (pIconExplorer->LargeItemCol == 0)
            pIconExplorer->LargeItemCol = 1;

        IconList_CheckScrollBar(pIconExplorer);
        IconList_CheckShowFocus(pIconExplorer);
    }

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// set small icon style
T_BOOL IconExplorer_SetSmallIcon(T_ICONEXPLORER *pIconExplorer, T_U32 SmallIconWidth, T_U32 SmallIconHeight, T_U32 SmallItemTInterval, T_U32 SmallItemHInterval, T_U32 SmallItemVInterval)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->SmallIconWidth = SmallIconWidth;
    if (pIconExplorer->SmallIconWidth > ICONEXPLORER_ITEM_SMALLICON_WMAX)
        pIconExplorer->SmallIconWidth = ICONEXPLORER_ITEM_SMALLICON_WMAX;
	
    pIconExplorer->SmallIconHeight = SmallIconHeight;
    if (pIconExplorer->SmallIconHeight < ICONEXPLORER_ITEM_SMALLICON_HMIN)
        pIconExplorer->SmallIconHeight = ICONEXPLORER_ITEM_SMALLICON_HMIN;
	
    pIconExplorer->SmallItemTInterval = SmallItemTInterval;
    if (pIconExplorer->SmallItemTInterval > ICONEXPLORER_ITEM_SMALLINTERVAL_MAX)
        pIconExplorer->SmallItemTInterval = ICONEXPLORER_ITEM_SMALLINTERVAL_MAX;
	
    pIconExplorer->SmallItemHInterval = SmallItemHInterval;
    if (pIconExplorer->SmallItemHInterval > ICONEXPLORER_ITEM_SMALLINTERVAL_MAX)
        pIconExplorer->SmallItemHInterval = ICONEXPLORER_ITEM_SMALLINTERVAL_MAX;
	
    pIconExplorer->SmallItemVInterval = SmallItemVInterval;
    if (pIconExplorer->SmallItemVInterval > ICONEXPLORER_ITEM_SMALLINTERVAL_MAX)
        pIconExplorer->SmallItemVInterval = ICONEXPLORER_ITEM_SMALLINTERVAL_MAX;

    // check small item row
    pIconExplorer->SmallItemRow = (pIconExplorer->ItemRect.height-pIconExplorer->ItemFrameWidth*2-pIconExplorer->SmallItemVInterval) / \
            (pIconExplorer->SmallIconHeight+pIconExplorer->SmallItemVInterval);

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// set large icon style
T_BOOL IconExplorer_SetLargeIcon(T_ICONEXPLORER *pIconExplorer, T_U32 LargeIconWidth, T_U32 LargeIconHeight, T_U32 LargeTextHeight, T_U32 LargeItemTInterval, T_U32 LargeItemHInterval, T_U32 LargeItemVInterval)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->LargeIconWidth = LargeIconWidth;
    if (pIconExplorer->LargeIconWidth > ICONEXPLORER_ITEM_LARGEICON_WMAX)
        pIconExplorer->LargeIconWidth = ICONEXPLORER_ITEM_LARGEICON_WMAX;
	
    pIconExplorer->LargeIconHeight = LargeIconHeight;
    if (pIconExplorer->LargeIconHeight > ICONEXPLORER_ITEM_LARGEICON_HMAX)
        pIconExplorer->LargeIconHeight = ICONEXPLORER_ITEM_LARGEICON_HMAX;
	
    pIconExplorer->LargeTextHeight = LargeTextHeight;
    if (pIconExplorer->LargeTextHeight > ICONEXPLORER_ITEM_LARGETEXT_HMAX)
        pIconExplorer->LargeTextHeight = ICONEXPLORER_ITEM_LARGETEXT_HMAX;
	
    if (pIconExplorer->LargeTextHeight < ICONEXPLORER_ITEM_LARGETEXT_HMIN)
        pIconExplorer->LargeTextHeight = ICONEXPLORER_ITEM_LARGETEXT_HMIN;
	
    pIconExplorer->LargeItemTInterval = LargeItemTInterval;
    if (pIconExplorer->LargeItemTInterval > ICONEXPLORER_ITEM_LARGEINTERVAL_MAX)
        pIconExplorer->LargeItemTInterval = ICONEXPLORER_ITEM_LARGEINTERVAL_MAX;
	
    pIconExplorer->LargeItemHInterval = LargeItemHInterval;
    if (pIconExplorer->LargeItemHInterval > ICONEXPLORER_ITEM_LARGEINTERVAL_MAX)
        pIconExplorer->LargeItemHInterval = ICONEXPLORER_ITEM_LARGEINTERVAL_MAX;
	
    pIconExplorer->LargeItemVInterval = LargeItemVInterval;
    if (pIconExplorer->LargeItemVInterval > ICONEXPLORER_ITEM_LARGEINTERVAL_MAX)
        pIconExplorer->LargeItemVInterval = ICONEXPLORER_ITEM_LARGEINTERVAL_MAX;

    // check large item col & row
    if (pIconExplorer->LargeIconWidth+pIconExplorer->LargeItemHInterval != 0)
    {
        pIconExplorer->LargeItemCol = (pIconExplorer->ItemRect.width-pIconExplorer->ItemFrameWidth*2-\
                pIconExplorer->ScrollBarShowWidth-pIconExplorer->LargeItemHInterval) / \
                (pIconExplorer->LargeIconWidth+pIconExplorer->LargeItemHInterval);
    }
    else
    {
        pIconExplorer->LargeItemCol = 1;
    }
    if (pIconExplorer->LargeItemCol == 0)
        pIconExplorer->LargeItemCol = 1;
	
    pIconExplorer->LargeItemRow = (pIconExplorer->ItemRect.height-pIconExplorer->ItemFrameWidth*2-pIconExplorer->LargeItemVInterval) / \
            (pIconExplorer->LargeIconHeight+pIconExplorer->LargeTextHeight+pIconExplorer->LargeItemVInterval+pIconExplorer->LargeItemTInterval);

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// set none icon style
T_BOOL IconExplorer_SetNoneIcon(T_ICONEXPLORER *pIconExplorer, T_U32 NoneTextHeight, T_U32 NoneItemHInterval, T_U32 NoneItemVInterval)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->NoneTextHeight = NoneTextHeight;
    if (pIconExplorer->NoneTextHeight < ICONEXPLORER_ITEM_NONETEXT_HMIN)
        pIconExplorer->NoneTextHeight = ICONEXPLORER_ITEM_NONETEXT_HMIN;
	
    pIconExplorer->NoneItemHInterval = NoneItemHInterval;
    if (pIconExplorer->NoneItemHInterval > ICONEXPLORER_ITEM_NONEINTERVAL_MAX)
        pIconExplorer->NoneItemHInterval = ICONEXPLORER_ITEM_NONEINTERVAL_MAX;
	
    pIconExplorer->NoneItemVInterval = NoneItemVInterval;
    if (pIconExplorer->NoneItemVInterval > ICONEXPLORER_ITEM_NONEINTERVAL_MAX)
        pIconExplorer->NoneItemVInterval = ICONEXPLORER_ITEM_NONEINTERVAL_MAX;

    // check none item row
    pIconExplorer->NoneItemRow = (pIconExplorer->ItemRect.height-pIconExplorer->ItemFrameWidth*2-pIconExplorer->NoneItemVInterval) / \
            (pIconExplorer->NoneTextHeight+pIconExplorer->NoneItemVInterval);

    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// set refresh flag
T_BOOL IconExplorer_SetRefresh(T_ICONEXPLORER *pIconExplorer, T_U32 RefreshFlag)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    if (RefreshFlag == ICONEXPLORER_REFRESH_NONE)
        pIconExplorer->RefreshFlag = ICONEXPLORER_REFRESH_NONE;
    else
        pIconExplorer->RefreshFlag |= RefreshFlag;

    return AK_TRUE;
}

T_U32   IconExplorer_GetRefresh(T_ICONEXPLORER *pIconExplorer)
{
    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_REFRESH_NONE;

    return pIconExplorer->RefreshFlag;
}


// set list item again flag
T_BOOL IconExplorer_SetListFlag(T_ICONEXPLORER *pIconExplorer)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->ItemListFlag |= ICONEXPLORER_CHANGE_ADD;

    return AK_TRUE;
}

// set sort callback function point by id
T_BOOL IconExplorer_SetSortIdCallBack(T_ICONEXPLORER *pIconExplorer, T_fICONEXPLORER_SORT_ID_CALLBACK SortIdCallBack)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->SortIdCallBack = SortIdCallBack;

    return AK_TRUE;
}

// set sort callback function point by ContentLen
T_BOOL IconExplorer_SetSortContentCallBack(T_ICONEXPLORER *pIconExplorer, T_fICONEXPLORER_SORT_CONTENT_CALLBACK SortContentCallBack)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->SortContentCallBack = SortContentCallBack;

    return AK_TRUE;
}

// set list item callback function point
T_BOOL IconExplorer_SetListCallBack(T_ICONEXPLORER *pIconExplorer, T_fICONEXPLORER_LIST_CALLBACK ListCallBack)
{
    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pIconExplorer->ListCallBack = ListCallBack;

    return AK_TRUE;
}

// get item quantity
T_U32 IconExplorer_GetItemQty(T_ICONEXPLORER *pIconExplorer)
{
    if (pIconExplorer == AK_NULL)
        return 0;
    
    return pIconExplorer->ItemQty;
}

// get current item icon style
T_ICONEXPLORER_ICONSTYLE IconExplorer_GetItemIconStyle(T_ICONEXPLORER *pIconExplorer)
{
    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_ICONSTYLE_NUM;

    if (pIconExplorer->ItemIconStyle >= ICONEXPLORER_ICONSTYLE_NUM)
        return ICONEXPLORER_ICONSTYLE_NUM;

    return pIconExplorer->ItemIconStyle;
}


// get item index by id
T_U32 IconExplorer_GetItemIndexById(T_ICONEXPLORER *pIconExplorer, T_U32 id)
{
    T_ICONEXPLORER_ITEM *p;
    T_U32 index = 0;

    if (pIconExplorer == AK_NULL)
    {
        return ICONEXPLORER_ITEM_ID_ERROR;
    }
	
    p = pIconExplorer->pItemHead;

    while(AK_NULL != p)
    {
        if (p->Id == id)
        {
            break;
        }
		
        p = p->pNext;
        index++;    
    }
	
    if (AK_NULL == p)
    {
        return ICONEXPLORER_ITEM_ID_ERROR;
    }

    return index;
}

// get current focus item id
T_U32 IconExplorer_GetItemFocusId(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) 
	{
        if (p == pIconExplorer->pItemFocus)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    return p->Id;
}

T_U32 IconExplorer_GetItemOldFocusId(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL)
	{
        if (p == pIconExplorer->pItemOldFocus)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    return p->Id;
}


// get current show first item id
T_U32 IconExplorer_GetItemShowId(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) 
	{
        if (p == pIconExplorer->pItemShow)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    return p->Id;
}



// get item point by id
T_ICONEXPLORER_ITEM *IconExplorer_GetItem(T_ICONEXPLORER *pIconExplorer, T_U32 Id)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL)
	{
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    return p;
}

// get focus item point
T_ICONEXPLORER_ITEM *IconExplorer_GetItemFocus(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
	
    while (p != AK_NULL)
	{
        if (p == pIconExplorer->pItemFocus)
            break;

        p = p->pNext;
    }

    return p;
}


T_ICONEXPLORER *IconExplorer_CopyItems(T_ICONEXPLORER *pIconExplorerDest, T_ICONEXPLORER *pIconExplorerSrc)
{
    T_ICONEXPLORER_ITEM *p;
    T_S32 focusid;
    
    if (pIconExplorerSrc == AK_NULL)
    {
        return pIconExplorerDest;
    }
    
    p = pIconExplorerSrc->pItemHead;

    while (p != AK_NULL)
	{
        IconExplorer_AddItemWithOption(pIconExplorerDest,p->Id,p->pContent,p->contentLen,p->pText,p->pSmallIcon,p->pLargeIcon,p->OptionType,p->pOptionValue);
        p = p->pNext;
    }

    if ((focusid=IconExplorer_GetItemFocusId(pIconExplorerSrc))!=ICONEXPLORER_ITEM_ID_ERROR)
    {
        IconExplorer_SetFocus(pIconExplorerDest,focusid);
    }

    return pIconExplorerDest;
}



// get focus item content
T_VOID *IconExplorer_GetItemContentFocus(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
	
    while (p != AK_NULL) 
	{
        if (p == pIconExplorer->pItemFocus)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_NULL;

    return p->pContent;
}



// get next item content by id
T_VOID *IconExplorer_GetItemContentNextById(T_ICONEXPLORER *pIconExplorer, T_U32 Id)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL)
	{
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_NULL;

    if (p->pNext == AK_NULL)
        return AK_NULL;

    return p->pNext->pContent;
}

// sort all item
T_BOOL IconExplorer_SortItem(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p, *q;
    T_BOOL done;

    if (pIconExplorer == AK_NULL
		|| pIconExplorer->pItemHead == AK_NULL)
        return AK_FALSE;

    // base on sort mode
    switch (pIconExplorer->ItemSortMode)
	{
    case ICONEXPLORER_SORT_CONTENT:
        if (pIconExplorer->SortContentCallBack == AK_NULL)
            return AK_FALSE;

        p = pIconExplorer->pItemHead;
        while (p->pNext != AK_NULL)
            p = p->pNext;

        // sort all item
        done = AK_FALSE;
        while ((!done) && (p != AK_NULL))
		{
            done = AK_TRUE;
            q = pIconExplorer->pItemHead;
			
            while (q != p)
			{
                if (pIconExplorer->SortContentCallBack((T_pCVOID)pIconExplorer, q->pContent, q->pNext->pContent)) 
				{
                    done = AK_FALSE;
                    IconList_SwapItemContent(q, q->pNext);
                }

                q = q->pNext;
            }

            p = p->pPrevious;
        }
        break;
		
    case ICONEXPLORER_SORT_ID:
    default:
        if (pIconExplorer->SortIdCallBack == AK_NULL)
            return AK_FALSE;

        p = pIconExplorer->pItemHead;
        while (p->pNext != AK_NULL)
            p = p->pNext;

        // sort all item
        done = AK_FALSE;
        while ((!done) && (p != AK_NULL))
		{
            done = AK_TRUE;
            q = pIconExplorer->pItemHead;
            while (q != p)
			{
                if (pIconExplorer->SortIdCallBack(q->Id, q->pNext->Id))
				{
                    done = AK_FALSE;
                    IconList_SwapItemContent(q, q->pNext);
                }

                q = q->pNext;
            }

            p = p->pPrevious;
        }
        break;
    }

    // modify old focus & focus point
    pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
    pIconExplorer->pItemFocus = pIconExplorer->pItemHead;

    IconList_CheckShowFocus(pIconExplorer);
    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// add an item with option
T_BOOL IconExplorer_AddItemWithOption(T_ICONEXPLORER *pIconExplorer, T_U32 Id, T_VOID *pContent, T_U32 ContentLen, 
													const T_U16 *pText, const T_U8 *pSmallIcon, const T_U8 *pLargeIcon, 
													T_ICONEXPLORER_OPTION_TYPE OptionType, T_U8 *pOptionValue)
{
    T_ICONEXPLORER_ITEM *p, *q;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    // check item quantity max limit
    if ((pIconExplorer->ItemQtyMax > 0) && (pIconExplorer->ItemQty >= pIconExplorer->ItemQtyMax))
        return AK_FALSE;

    // malloc item space
    p = (T_ICONEXPLORER_ITEM *)Fwl_Malloc(sizeof(T_ICONEXPLORER_ITEM));
    if (p == AK_NULL)
        return AK_FALSE;

    // assign item parameter
    p->Id = Id;
    if (pContent != AK_NULL)
	{
        // malloc item content space
        p->pContent = Fwl_Malloc(ContentLen + 1);
        if (p->pContent == AK_NULL) 
		{
            p = Fwl_Free(p);
            return AK_FALSE;
        }

        memcpy(p->pContent, pContent, ContentLen);
        p->contentLen=ContentLen;
    }
    else
    {
        p->pContent = AK_NULL;
        p->contentLen=ContentLen;
    }
	
    if (pText != AK_NULL)
    {
        T_U32	len = 0;

    	len = Utl_UStrLen(pText) << 1;

    	if (len > TEXT_LEN_MAX)
    	{
			len = TEXT_LEN_MAX;
    	}
    	
        // malloc item text space
        p->pText = (T_U16 *)Fwl_Malloc(len + 2);

        if (AK_NULL == p->pText)
        {
            Fwl_Print(C3, M_CTRL, "IconExplorer_AddItemWithOption() : Fwl_Malloc error !! "  );
            if (p->pContent != AK_NULL)
                p->pContent = Fwl_Free(p->pContent);
            p = Fwl_Free(p);
            return AK_FALSE;
        }

        Utl_UStrCpyN(p->pText,  (T_U16 *)pText, len/2);
    }
    else
	{
        p->pText = AK_NULL;
    }
	
    p->pSmallIcon = (T_U8 *)pSmallIcon;
    p->pLargeIcon = (T_U8 *)pLargeIcon;
    p->OptionType = OptionType;
    p->pOptionValue = pOptionValue;
    p->pOptionHead = AK_NULL;
    p->pOptionFocus = AK_NULL;

    // sort & check chain table point
    switch (pIconExplorer->ItemSortMode)
	{
    case ICONEXPLORER_SORT_CONTENT:
        if (pIconExplorer->SortContentCallBack == AK_NULL) 
		{
            // not sort rule, so add tail
            if (!IconList_InsertTailItem(pIconExplorer, p))
				return AK_FALSE;
        }
        else 
		{
            // base on rule to insert item
            q = pIconExplorer->pItemHead;
            if (q == AK_NULL) 
			{
                // item head is null
                p->pPrevious = AK_NULL;
                p->pNext = AK_NULL;

                pIconExplorer->pItemHead = p;
            }
            else 
			{
                while (q != AK_NULL) 
				{
                    if (q->Id == Id) 
					{
                        if (p->pText != AK_NULL)
                            p->pText = Fwl_Free(p->pText);
						
                        if (p->pContent != AK_NULL)
                            p->pContent = Fwl_Free(p->pContent);
						
                        p = Fwl_Free(p);
                        return AK_FALSE;
                    }

                    if (pIconExplorer->SortContentCallBack((T_pCVOID)pIconExplorer, q->pContent, pContent)) 
					{
                        p->pPrevious = q->pPrevious;
                        p->pNext = q;
                        if (q->pPrevious != AK_NULL)
                            q->pPrevious->pNext = p;
						
                        q->pPrevious = p;

                        if (pIconExplorer->pItemHead == q)
                            pIconExplorer->pItemHead = p;

                        break;
                    }
                    else if (q->pNext == AK_NULL) 
					{
                        p->pPrevious = q;
                        p->pNext = AK_NULL;
                        q->pNext = p;

                        break;
                    }

                    q = q->pNext;
                }
            }
        }
        break;
		
    case ICONEXPLORER_SORT_ID:
    default:
        if (pIconExplorer->SortIdCallBack == AK_NULL) 
		{
            // not sort rule, so add tail
            if (!IconList_InsertTailItem(pIconExplorer, p))
				return AK_FALSE;
        }
        else 
		{
            q = pIconExplorer->pItemHead;
            if (q == AK_NULL)
			{
                // item head is null
                p->pPrevious = AK_NULL;
                p->pNext = AK_NULL;

                pIconExplorer->pItemHead = p;
            }
            else 
			{
                while (q != AK_NULL) 
				{
                    if (q->Id == Id) 
					{
                        if (p->pText != AK_NULL)
                            p->pText = Fwl_Free(p->pText);
						
                        if (p->pContent != AK_NULL)
                            p->pContent = Fwl_Free(p->pContent);
						
                        p = Fwl_Free(p);
                        return AK_FALSE;
                    }

                    if (pIconExplorer->SortIdCallBack(q->Id, Id)) 
					{
                        p->pPrevious = q->pPrevious;
                        p->pNext = q;
                        if (q->pPrevious != AK_NULL)
                            q->pPrevious->pNext = p;
						
                        q->pPrevious = p;

                        if (pIconExplorer->pItemHead == q)
                            pIconExplorer->pItemHead = p;

                        break;
                    }
                    else if (q->pNext == AK_NULL) 
					{
                        p->pPrevious = q;
                        p->pNext = AK_NULL;
                        q->pNext = p;

                        break;
                    }

                    q = q->pNext;
                }
            }
        }
        break;
    }

    // modify old focus, focus, item quantity
    if (pIconExplorer->pItemFocus == AK_NULL) 
	{
        pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
        pIconExplorer->pItemFocus = pIconExplorer->pItemHead;
        IconList_CheckShowFocus(pIconExplorer);
    }
    pIconExplorer->ItemQty++;

	if (SlipMgr_GetItemNum(pIconExplorer->pSlipMgr) == pIconExplorer->ItemQty - 1)
	{
		pIconExplorer->ItemListFlag = ICONEXPLORER_CHANGE_ADD;
	}
	else
	{
		SlipMgr_SetTotalItemNum(pIconExplorer->pSlipMgr, pIconExplorer->ItemQty);
	}

    // check scroll bar, modify refresh flag
    IconList_CheckScrollBar(pIconExplorer);
    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// add an option to item
T_BOOL IconExplorer_AddItemOption(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId, T_U8 OptionId, const T_U16 *pText)
{
    T_ICONEXPLORER_ITEM *pItem;
    T_ICONEXPLORER_OPTION *p, *q;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pItem = IconExplorer_GetItem(pIconExplorer, ItemId);
    if (pItem == AK_NULL)
        return AK_FALSE;

    switch (pItem->OptionType)
	{
    case ICONEXPLORER_OPTION_LIST:
        // check option id
        p = pItem->pOptionHead;
		
        while (p != AK_NULL)
		{
            if (p->Id == OptionId)
                break;

            p = p->pNext;
        }

        if (p != AK_NULL)
            return AK_FALSE;

        // malloc option space
        p = (T_ICONEXPLORER_OPTION *)Fwl_Malloc(sizeof(T_ICONEXPLORER_OPTION));
        if (p == AK_NULL)
            return AK_FALSE;

        p->Id = OptionId;
		
        if (pText != AK_NULL)
            Utl_UStrMid(p->Text, pText, 0, (T_S16)(ICONEXPLORER_ITEMOPTION_STRLEN-1));
        else
            memset((void *)p->Text, 0x00, ICONEXPLORER_ITEMOPTION_STRLEN);
		
        p->pNext = AK_NULL;

        if (pItem->pOptionHead == AK_NULL)
		{
            pItem->pOptionHead = p;
        }
        else
		{
            q = pItem->pOptionHead;
            while (q->pNext != AK_NULL)
                q = q->pNext;

            q->pNext = p;
        }

        if ((pItem->pOptionValue != AK_NULL) && (*pItem->pOptionValue == p->Id))
            pItem->pOptionFocus = p;

        if (pItem->pOptionFocus == AK_NULL)
            pItem->pOptionFocus = pItem->pOptionHead;

        IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);
		
        return AK_TRUE;
		
    case ICONEXPLORER_OPTION_NEXT:
    case ICONEXPLORER_OPTION_NONE:
    default:
        return AK_FALSE;
    }
}

T_BOOL IconExplorer_SortIdCallback(T_U32 Id1, T_U32 Id2)
{
    if (Id1 > Id2)
        return AK_TRUE;
    else
        return AK_FALSE;
}


/********************************************************************/
#if 0
static T_BOOL IconExplorer_ShowTitle(T_ICONEXPLORER *pIconExplorer);
static T_BOOL IconExplorer_ShowItem(T_ICONEXPLORER *pIconExplorer);
static T_BOOL IconExplorer_ShowFocus(T_ICONEXPLORER *pIconExplorer);
static T_BOOL IconExplorer_ScrollTitleText(T_ICONEXPLORER *pIconExplorer);
static T_BOOL IconExplorer_ScrollItemText(T_ICONEXPLORER *pIconExplorer);
static T_BOOL IconExplorer_ShowScrollBar(T_ICONEXPLORER *pIconExplorer);
static T_BOOL IconExplorer_GetTitleTextPos(T_ICONEXPLORER *pIconExplorer, T_S16 *PosX, T_S16 *PosY, T_U32 *MaxLen);

static T_BOOL IconExplorer_CheckPointInRect(T_U16 x, T_U16 y, T_RECT Rect);


// check point in rect
static T_BOOL IconExplorer_CheckPointInRect(T_U16 x, T_U16 y, T_RECT Rect)
{
    if (((x >= Rect.left) && (x < Rect.left+Rect.width)) && \
            ((y >= Rect.top) && (y <= Rect.top+Rect.height)))
        return AK_TRUE;
    else
        return AK_FALSE;
}


// modify item content by id
T_BOOL IconExplorer_ModifyItemContent(T_ICONEXPLORER *pIconExplorer, T_U32 Id, T_VOID *pContent, T_U32 ContentLen)
{
    T_ICONEXPLORER_ITEM *p;
    T_VOID *q;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    if (pContent == AK_NULL)
        return AK_FALSE;

    // find item by id
    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    // malloc item content space again
    q = Fwl_Malloc(ContentLen);
    if (q == AK_NULL)
        return AK_FALSE;

    // free old item content space
    if (p->pContent != AK_NULL)
        p->pContent = Fwl_Free(p->pContent);

    memcpy(q, pContent, ContentLen);
    p->pContent = q;

    return AK_TRUE;
}

// modify item text by id
T_BOOL IconExplorer_ModifyItemText(T_ICONEXPLORER *pIconExplorer, T_U32 Id, const T_U16 *pText)
{
    T_ICONEXPLORER_ITEM *p;
    T_U16 *q;
    T_U32	len = 0;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    if (pText == AK_NULL)
        return AK_FALSE;

    // find item by id
    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    len = Utl_UStrLen(pText) << 1;

	if (len > TEXT_LEN_MAX)
	{
		len = TEXT_LEN_MAX;
	}
	
    // malloc item text space again
    q = (T_U16 *)Fwl_Malloc(len + 2);

    if (AK_NULL == q)
    {
        return AK_FALSE;
    }

    // free old item text space
    if (p->pText != AK_NULL)
        p->pText = Fwl_Free(p->pText);

    Utl_UStrCpyN(q, (T_U16 *)pText, len/2);
    p->pText = q;

    // show item again
    IconList_ReShowItem(pIconExplorer, p);

    return AK_TRUE;
}


// modify item large icon by Id
T_BOOL IconExplorer_ModifyItemLargeIcon(T_ICONEXPLORER *pIconExplorer, T_U32 Id, const T_U8 *pLargeIcon)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    // find item by id
    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    p->pLargeIcon = (T_U8 *)pLargeIcon;

    // show item again
    if (pIconExplorer->ItemIconStyle == ICONEXPLORER_LARGEICON)
        IconList_ReShowItem(pIconExplorer, p);

    return AK_TRUE;
}

// get current sort item mode
T_ICONEXPLORER_SORTMODE IconExplorer_GetItemSortMode(T_ICONEXPLORER *pIconExplorer)
{
    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_SORT_NUM;

    if (pIconExplorer->ItemSortMode >= ICONEXPLORER_SORT_NUM)
        return ICONEXPLORER_SORT_NUM;

    return pIconExplorer->ItemSortMode;
}

// get item id by index
T_U32 IconExplorer_GetItemIdByIndex(T_ICONEXPLORER *pIconExplorer, T_U32 Index)
{
    T_ICONEXPLORER_ITEM *p;
    T_U32 i;

    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    // find item by index
    p = pIconExplorer->pItemHead;
    for (i=0; (p!=AK_NULL)&&(i<Index); i++)
        p = p->pNext;

    if (p == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    return p->Id;
}

// get previous item id by id
T_U32 IconExplorer_GetItemPreviousId(T_ICONEXPLORER *pIconExplorer, T_U32 Id)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    if (p->pPrevious == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    return p->pPrevious->Id;
}

// get next item id by id
T_U32 IconExplorer_GetItemNextId(T_ICONEXPLORER *pIconExplorer, T_U32 Id)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    if (p->pNext == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    return p->pNext->Id;
}

// get item id by point
T_U32 IconExplorer_GetIdByPoint(T_ICONEXPLORER *pIconExplorer, T_U16 x, T_U16 y)
{
    T_ICONEXPLORER_ITEM *p;
    T_RECT ShowRect;
    T_U32 Row, Col;

    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    switch (pIconExplorer->ItemIconStyle) {
        case ICONEXPLORER_LARGEICON:
            p = pIconExplorer->pItemShow;
            for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->LargeItemRow); Row++) {
                for (Col=0; (p!=AK_NULL)&&(Col<pIconExplorer->LargeItemCol); Col++) {
                    ShowRect.left = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth + pIconExplorer->LargeItemHInterval + \
                            (pIconExplorer->LargeItemHInterval+pIconExplorer->LargeIconWidth)*Col);
                    ShowRect.top = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->LargeItemVInterval + \
                            (pIconExplorer->LargeItemVInterval+pIconExplorer->LargeItemTInterval+pIconExplorer->LargeIconHeight+pIconExplorer->LargeTextHeight)*Row);
                    ShowRect.width = (T_S16)(pIconExplorer->LargeIconWidth);
                    ShowRect.height = (T_S16)(pIconExplorer->LargeIconHeight + pIconExplorer->LargeTextHeight + pIconExplorer->LargeItemTInterval);

                    if (IconExplorer_CheckPointInRect(x, y, ShowRect) == AK_TRUE)
                        return p->Id;

                    p = p->pNext;
                }
            }
            break;
        case ICONEXPLORER_NONEICON:
            p = pIconExplorer->pItemShow;
            for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->NoneItemRow); Row++) {
                ShowRect.left = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth);
                ShowRect.top = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->NoneItemVInterval + \
                        (pIconExplorer->NoneItemVInterval+pIconExplorer->NoneTextHeight)*Row);
                ShowRect.width = (T_S16)(pIconExplorer->ItemRect.width - pIconExplorer->ItemFrameWidth*2 - pIconExplorer->ScrollBarShowWidth);
                ShowRect.height = (T_S16)(pIconExplorer->NoneTextHeight);

                if (IconExplorer_CheckPointInRect(x, y, ShowRect) == AK_TRUE)
                    return p->Id;

                p = p->pNext;
            }
            break;
        case ICONEXPLORER_SMALLICON:
        default:
            p = pIconExplorer->pItemShow;
            for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->SmallItemRow); Row++) {
                ShowRect.left = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth);
                ShowRect.top = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->SmallItemVInterval + \
                        (pIconExplorer->SmallItemVInterval+pIconExplorer->SmallIconHeight)*Row);
                ShowRect.width = (T_S16)(pIconExplorer->ItemRect.width - pIconExplorer->ItemFrameWidth*2 - pIconExplorer->ScrollBarShowWidth);
                ShowRect.height = (T_S16)(pIconExplorer->SmallIconHeight);

                if (IconExplorer_CheckPointInRect(x, y, ShowRect) == AK_TRUE)
                    return p->Id;

                p = p->pNext;
            }
            break;
    }

    return ICONEXPLORER_ITEM_ID_ERROR;
}


// get show first item point
T_ICONEXPLORER_ITEM *IconExplorer_GetItemShow(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p == pIconExplorer->pItemShow)
            break;

        p = p->pNext;
    }

    return p;
}

// get previous item point by item
T_ICONEXPLORER_ITEM *IconExplorer_GetItemPrevious(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p == pItem)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_NULL;

    return p->pPrevious;
}

// get next item point by item
T_ICONEXPLORER_ITEM *IconExplorer_GetItemNext(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p == pItem)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_NULL;

    return p->pNext;
}


// get item content by id
T_VOID *IconExplorer_GetItemContent(T_ICONEXPLORER *pIconExplorer, T_U32 Id)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_NULL;

    return p->pContent;

}

// get show first item content
T_VOID *IconExplorer_GetItemContentShow(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p == pIconExplorer->pItemShow)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_NULL;

    return p->pContent;
}

// get previous item content by id
T_VOID *IconExplorer_GetItemContentPreviousById(T_ICONEXPLORER *pIconExplorer, T_U32 Id)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_NULL;

    if (p->pPrevious == AK_NULL)
        return AK_NULL;

    return p->pPrevious->pContent;
}


// get previous item content by item
T_VOID *IconExplorer_GetItemContentPreviousByItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p == pItem)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_NULL;

    if (p->pPrevious == AK_NULL)
        return AK_NULL;

    return p->pPrevious->pContent;
}

//get next item content by item
T_VOID *IconExplorer_GetItemContentNextByItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem)
{
    T_ICONEXPLORER_ITEM *p;

    if (pIconExplorer == AK_NULL)
        return AK_NULL;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p == pItem)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_NULL;

    if (p->pNext == AK_NULL)
        return AK_NULL;

    return p->pNext->pContent;
}

// swap two item
T_BOOL IconExplorer_SwapItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem1, T_ICONEXPLORER_ITEM *pItem2)
{
    T_ICONEXPLORER_ITEM *p;
    T_BOOL Flag1 = AK_FALSE, Flag2 = AK_FALSE;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    if ((pItem1 == AK_NULL) || (pItem2 == AK_NULL) || (pItem1 == pItem2))
        return AK_FALSE;

    // find two item
    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p == pItem1)
            Flag1 = AK_TRUE;
        if (p == pItem2)
            Flag2 = AK_TRUE;

        if ((Flag1 == AK_TRUE) && (Flag2 == AK_TRUE))
            break;

        p = p->pNext;
    }

    if ((Flag1 == AK_FALSE) || (Flag2 == AK_FALSE))
        return AK_FALSE;

    // swap two item content
    IconList_SwapItemContent(pItem1, pItem2);

    IconList_CheckShowFocus(pIconExplorer);
    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// swap tow item by id
T_BOOL IconExplorer_SwapItemById(T_ICONEXPLORER *pIconExplorer, T_U32 Id1, T_U32 Id2)
{
    T_ICONEXPLORER_ITEM *p, *pItem1 = AK_NULL, *pItem2 = AK_NULL;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    if (Id1 == Id2)
        return AK_FALSE;

    // find two item
    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id1)
            pItem1 = p;
        if (p->Id == Id2)
            pItem2 = p;

        if ((pItem1 != AK_NULL) && (pItem2 != AK_NULL))
            break;

        p = p->pNext;
    }

    if ((pItem1 == AK_NULL) || (pItem2 == AK_NULL))
        return AK_FALSE;

    // swap two item content
    IconList_SwapItemContent(pItem1, pItem2);

    IconList_CheckShowFocus(pIconExplorer);
    IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}


// modify an option at item
T_BOOL IconExplorer_ModifyItemOption(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId, T_U32 OptionId, const T_U16 *pText)
{
    T_ICONEXPLORER_ITEM *pItem;
    T_ICONEXPLORER_OPTION *p;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pItem = IconExplorer_GetItem(pIconExplorer, ItemId);
    if (pItem == AK_NULL)
        return AK_FALSE;

    switch (pItem->OptionType) {
        case ICONEXPLORER_OPTION_LIST:
            // find option by id
            p = pItem->pOptionHead;
            while (p != AK_NULL) {
                if (p->Id == OptionId)
                    break;

                p = p->pNext;
            }

            if (p == AK_NULL)
                return AK_FALSE;

            Utl_UStrMid(p->Text, pText, 0, (T_S16)(ICONEXPLORER_ITEMOPTION_STRLEN-1));

            IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);
            return AK_TRUE;
        case ICONEXPLORER_OPTION_NEXT:
        case ICONEXPLORER_OPTION_NONE:
        default:
            return AK_FALSE;
    }
}

// delete an option at item
T_BOOL IconExplorer_DelItemOption(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId, T_U32 OptionId)
{
    T_ICONEXPLORER_ITEM *pItem;
    T_ICONEXPLORER_OPTION *p, *q;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pItem = IconExplorer_GetItem(pIconExplorer, ItemId);
    if (pItem == AK_NULL)
        return AK_FALSE;

    switch (pItem->OptionType) {
        case ICONEXPLORER_OPTION_LIST:
            // find option by id
            p = pItem->pOptionHead;
            q = AK_NULL;
            while (p != AK_NULL) {
                if (p->Id == OptionId)
                    break;

                q = p;
                p = p->pNext;
            }

            if (p == AK_NULL)
                return AK_FALSE;

            // modify after delete point
            if (q != AK_NULL)
                q->pNext = p->pNext;
            else
                pItem->pOptionHead = p->pNext;

            // free option space
            p = Fwl_Free(p);

            IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);
            return AK_TRUE;
        case ICONEXPLORER_OPTION_NEXT:
        case ICONEXPLORER_OPTION_NONE:
        default:
            return AK_FALSE;
    }
}

// delete all option at item
T_BOOL IconExplorer_DelAllItemOption(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId)
{
    T_ICONEXPLORER_ITEM *pItem;
    T_ICONEXPLORER_OPTION *p, *q;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pItem = IconExplorer_GetItem(pIconExplorer, ItemId);
    if (pItem == AK_NULL)
        return AK_FALSE;

    switch (pItem->OptionType) {
        case ICONEXPLORER_OPTION_LIST:
            if (pItem->pOptionHead == AK_NULL)
                return AK_FALSE;

            // free all option space
            p = pItem->pOptionHead;
            while (p != AK_NULL) {
                q = p->pNext;

                p = Fwl_Free(p);

                p = q;
            }

            // modify option point
            pItem->pOptionHead = AK_NULL;

            IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);
            return AK_TRUE;
        case ICONEXPLORER_OPTION_NEXT:
        case ICONEXPLORER_OPTION_NONE:
        default:
            return AK_FALSE;
    }
}

// set item option focus by option id
T_BOOL IconExplorer_SetItemOptionFocusId(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId, T_U32 OptionId)
{
    T_ICONEXPLORER_ITEM *pItem;
    T_ICONEXPLORER_OPTION *p;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    pItem = IconExplorer_GetItem(pIconExplorer, ItemId);
    if (pItem == AK_NULL)
        return AK_FALSE;

    switch (pItem->OptionType) {
        case ICONEXPLORER_OPTION_LIST:
            // find option by id
            p = pItem->pOptionHead;
            while (p != AK_NULL) {
                if (p->Id == OptionId)
                    break;

                p = p->pNext;
            }

            if (p == AK_NULL)
                return AK_FALSE;

            pItem->pOptionFocus = p;

            if (pItem->pOptionValue != AK_NULL)
                *pItem->pOptionValue = pItem->pOptionFocus->Id;

            IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_FOCUS);
            return AK_TRUE;
        case ICONEXPLORER_OPTION_NEXT:
        case ICONEXPLORER_OPTION_NONE:
        default:
            return AK_FALSE;
    }
}

// get item option focus id
T_U32 IconExplorer_GetItemOptionFocusId(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *pItem;

    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    // get focus item
    pItem = IconExplorer_GetItemFocus(pIconExplorer);
    if (pItem == AK_NULL)
        return ICONEXPLORER_ITEM_ID_ERROR;

    switch (pItem->OptionType) {
        case ICONEXPLORER_OPTION_LIST:
            if (pItem->pOptionHead == AK_NULL)
                return ICONEXPLORER_ITEM_ID_ERROR;

            if (pItem->pOptionFocus == AK_NULL) {
                pItem->pOptionFocus = pItem->pOptionHead;

                if (pItem->pOptionValue != AK_NULL)
                    *pItem->pOptionValue = pItem->pOptionFocus->Id;
            }

            return pItem->pOptionFocus->Id;
        case ICONEXPLORER_OPTION_NEXT:
        case ICONEXPLORER_OPTION_NONE:
        default:
            return ICONEXPLORER_ITEM_ID_ERROR;
    }
}

T_U8 IconExplorer_GetItemOptionFocusIdByItemId(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId)
{
    T_ICONEXPLORER_ITEM *pItem;

    if (pIconExplorer == AK_NULL)
        return ICONEXPLORER_ITEMOPTION_ID_ERROR;

    // get focus item
    pItem = IconExplorer_GetItem(pIconExplorer, ItemId);
    if (pItem == AK_NULL)
        return ICONEXPLORER_ITEMOPTION_ID_ERROR;

    switch (pItem->OptionType) {
        case ICONEXPLORER_OPTION_LIST:
            if (pItem->pOptionHead == AK_NULL)
                return ICONEXPLORER_ITEMOPTION_ID_ERROR;

            if (pItem->pOptionFocus == AK_NULL) {
                pItem->pOptionFocus = pItem->pOptionHead;

                if (pItem->pOptionValue != AK_NULL)
                    *pItem->pOptionValue = pItem->pOptionFocus->Id;
            }

            return pItem->pOptionFocus->Id;
        case ICONEXPLORER_OPTION_NEXT:
        case ICONEXPLORER_OPTION_NONE:
        default:
            return ICONEXPLORER_ITEMOPTION_ID_ERROR;
    }
}

#endif

#if 0
// show title
static T_BOOL IconExplorer_ShowTitle(T_ICONEXPLORER *pIconExplorer)
{
    T_S16 PosX, PosY;
    T_U32 MaxLen;
    T_RECT Rect;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    // if not title, skip show
    if ((pIconExplorer->ItemStyle & ICONEXPLORER_TITLE_ON) != ICONEXPLORER_TITLE_ON)
        return AK_FALSE;

    if ((pIconExplorer->RefreshFlag & ICONEXPLORER_REFRESH_TITLE) == ICONEXPLORER_REFRESH_TITLE) {
        // show title back
        if (pIconExplorer->pTitleBackData != AK_NULL) {
            // show title back graph
            Rect.left = 0;
            Rect.top = 0;
            Rect.width = pIconExplorer->TitleRect.width;
            Rect.height = pIconExplorer->TitleRect.height;
            Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pIconExplorer->TitleRect.left, pIconExplorer->TitleRect.top, \
                    &Rect, pIconExplorer->pTitleBackData, &pIconExplorer->ItemTransColor, AK_FALSE);
        }
        else {
            // not title back graph, so show title back color
            Fwl_FillSolidRect(HRGB_LAYER, \
                    pIconExplorer->TitleRect.left, pIconExplorer->TitleRect.top, \
                    pIconExplorer->TitleRect.width, pIconExplorer->TitleRect.height, \
                    pIconExplorer->TitleBackColor);
        }

        // show title text

        if (pIconExplorer->pTitleText != AK_NULL)
		{
            if (IconExplorer_GetTitleTextPos(pIconExplorer, &PosX, &PosY, &MaxLen) == AK_TRUE)
                UDispSpeciString(DISPLAY_LCD_0, PosX, PosY, pIconExplorer->pTitleText, \
                        pIconExplorer->TitleTextColor, CURRENT_FONT_SIZE, Utl_UStrLen(pIconExplorer->pTitleText));

            pIconExplorer->TitleTextOffset = 0;
        }
    }

    return AK_TRUE;
}
#endif

#if 0
// show all item
static T_BOOL IconExplorer_ShowItem(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;
    T_U32 Col, Row;
    T_RECT Rect;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;


    if ((pIconExplorer->RefreshFlag & ICONEXPLORER_REFRESH_ITEM) == ICONEXPLORER_REFRESH_ITEM) {
        // show item back
        if (pIconExplorer->pItemBackData != AK_NULL) {
            // show item back graph
            Rect.left = 0;
            Rect.top = 0;
//            Rect.top = pIconExplorer->ItemRect.top;
            Rect.width = pIconExplorer->ItemRect.width;
            Rect.height = pIconExplorer->ItemRect.height;
            Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pIconExplorer->ItemRect.left, pIconExplorer->ItemRect.top, \
                    &Rect, pIconExplorer->pItemBackData, AK_NULL, AK_FALSE);
        }
        else {
            // not item back graph, fo show item back color
            Fwl_FillSolidRect(HRGB_LAYER, \
                    pIconExplorer->ItemRect.left, pIconExplorer->ItemRect.top, \
                    pIconExplorer->ItemRect.width, pIconExplorer->ItemRect.height, \
                    pIconExplorer->ItemBackColor);
        }

        // show item frame
        if ((pIconExplorer->ItemStyle & ICONEXPLORER_ITEM_FRAME) == ICONEXPLORER_ITEM_FRAME)
            Fwl_DialogFrame(HRGB_LAYER, pIconExplorer->ItemRect.left, pIconExplorer->ItemRect.top, \
                    pIconExplorer->ItemRect.width, pIconExplorer->ItemRect.height, 0x0f);

        // show item by style
        switch (pIconExplorer->ItemIconStyle) {
            case ICONEXPLORER_LARGEICON:
                p = pIconExplorer->pItemShow;
                for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->LargeItemRow); Row++) {
                    for (Col=0; (p!=AK_NULL)&&(Col<pIconExplorer->LargeItemCol); Col++) {
                        if (p == pIconExplorer->pItemFocus)
                            IconList_ShowLargeItem(pIconExplorer, p, Col, Row, AK_TRUE, AK_FALSE);
                        else
                            IconList_ShowLargeItem(pIconExplorer, p, Col, Row, AK_FALSE, AK_FALSE);

                        p = p->pNext;
                    }
                }
                break;
            case ICONEXPLORER_NONEICON:
                p = pIconExplorer->pItemShow;
                for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->NoneItemRow); Row++) {
                    if (p == pIconExplorer->pItemFocus)
                        IconList_ShowNoneItem(pIconExplorer, p, Row, AK_TRUE, AK_FALSE);
                    else
                        IconList_ShowNoneItem(pIconExplorer, p, Row, AK_FALSE, AK_FALSE);

                    p = p->pNext;
                }
                break;
            case ICONEXPLORER_SMALLICON:
            default:
                p = pIconExplorer->pItemShow;
                for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->SmallItemRow); Row++) {
                    if (p == pIconExplorer->pItemFocus)
                        IconList_ShowSmallItem(pIconExplorer, p, Row, AK_TRUE, AK_FALSE);
                    else
                        IconList_ShowSmallItem(pIconExplorer, p, Row, AK_FALSE, AK_FALSE);

                    p = p->pNext;
                }
                break;
        }

        pIconExplorer->ItemTextOffset = 0;

        // check scroll bar & show it again
        if (pIconExplorer->ScrollBarFlag == AK_TRUE)
            IconExplorer_ShowScrollBar(pIconExplorer);
    }

	

    return AK_TRUE;
}

// show focus item
static T_BOOL IconExplorer_ShowFocus(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;
    T_U32 Col, Row;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;


    if (((pIconExplorer->RefreshFlag & ICONEXPLORER_REFRESH_FOCUS) == ICONEXPLORER_REFRESH_FOCUS) && \
            ((pIconExplorer->RefreshFlag & ICONEXPLORER_REFRESH_ITEM) != ICONEXPLORER_REFRESH_ITEM)) {
        // refresh old focus & focus item
        switch (pIconExplorer->ItemIconStyle) {
            case ICONEXPLORER_LARGEICON:
                p = pIconExplorer->pItemShow;
                for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->LargeItemRow); Row++) {
                    for (Col=0; (p!=AK_NULL)&&(Col<pIconExplorer->LargeItemCol); Col++) {
                        if (p == pIconExplorer->pItemOldFocus)
                            IconList_ShowLargeItem(pIconExplorer, p, Col, Row, AK_FALSE, AK_TRUE);
                        if (p == pIconExplorer->pItemFocus)
                            IconList_ShowLargeItem(pIconExplorer, p, Col, Row, AK_TRUE, AK_TRUE);

                        p = p->pNext;
                    }
                }
                break;
            case ICONEXPLORER_NONEICON:
                p = pIconExplorer->pItemShow;
                for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->NoneItemRow); Row++) {
                    if (p == pIconExplorer->pItemOldFocus)
                        IconList_ShowNoneItem(pIconExplorer, p, Row, AK_FALSE, AK_TRUE);
                    if (p == pIconExplorer->pItemFocus)
                        IconList_ShowNoneItem(pIconExplorer, p, Row, AK_TRUE, AK_TRUE);

                    p = p->pNext;
                }
                break;
            case ICONEXPLORER_SMALLICON:
            default:
                p = pIconExplorer->pItemShow;
                for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->SmallItemRow); Row++) {
                    if (p == pIconExplorer->pItemOldFocus)
                        IconList_ShowSmallItem(pIconExplorer, p, Row, AK_FALSE, AK_TRUE);
                    if (p == pIconExplorer->pItemFocus)
                        IconList_ShowSmallItem(pIconExplorer, p, Row, AK_TRUE, AK_TRUE);

                    p = p->pNext;
                }
                break;
        }

        pIconExplorer->ItemTextOffset = 0;
    }


    return AK_TRUE;
}
#endif
#if 0
// scroll long title text
static T_BOOL IconExplorer_ScrollTitleText(T_ICONEXPLORER *pIconExplorer)
{
    T_S16 PosX, PosY;
    T_U32 MaxLen;
    T_U16 uTextLen;
    T_RECT Rect;
    T_U32 uTextWidth = 0;
    
    AK_ASSERT_PTR(pIconExplorer, "IconExplorer_ScrollTitleText(): pIconExplorer is NULL", AK_FALSE);

    if (AK_NULL == pIconExplorer->pTitleText)
    {
        Fwl_Print(C3, M_CTRL, "IconExplorer_ScrollTitleText(): pIconExplorer->pTitleText is NULL");
        return AK_FALSE;
    }
    
    uTextWidth = UGetSpeciStringWidth(pIconExplorer->pTitleText, CURRENT_FONT_SIZE, Utl_UStrLen(pIconExplorer->pTitleText));

    if (AK_FALSE == IconExplorer_GetTitleTextPos(pIconExplorer, &PosX, &PosY, &MaxLen))
    {
        Fwl_Print(C3, M_CTRL, "IconExplorer_ScrollTitleText(): IconExplorer_GetTitleTextPos fail");
        return AK_FALSE;
    }

    /*Judge if need scroll*/
    if (uTextWidth <= MaxLen)
    {
        return AK_FALSE;
    }

    /*----------follow process scroll--------------*/
    // show title back again
    if (pIconExplorer->pTitleBackData != AK_NULL) {
        Rect.left = 0;
        Rect.top = 0;
        Rect.width = pIconExplorer->TitleRect.width;
        Rect.height = pIconExplorer->TitleRect.height;
        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pIconExplorer->TitleRect.left, pIconExplorer->TitleRect.top, \
                &Rect, pIconExplorer->pTitleBackData, AK_NULL, AK_FALSE);
    }
    else {
        Fwl_FillSolidRect(HRGB_LAYER, \
                pIconExplorer->TitleRect.left, pIconExplorer->TitleRect.top, \
                pIconExplorer->TitleRect.width, pIconExplorer->TitleRect.height, \
                pIconExplorer->TitleBackColor);
    }

    uTextLen = Utl_UStrLen(pIconExplorer->pTitleText);

    pIconExplorer->TitleTextOffset++;
    if (pIconExplorer->TitleTextOffset + IE_TITLE_TEXT_TAIL > MaxLen)
    {
       pIconExplorer->TitleTextOffset = 0; 
    }
        
    Fwl_UScrollDispString(HRGB_LAYER, pIconExplorer->pTitleText, PosX, PosY, uTextLen, (T_U16)pIconExplorer->TitleTextOffset, 
        (T_U16)MaxLen, COLOR_BLACK, CURRENT_FONT_SIZE);

    return AK_TRUE;
}
#endif
#if 0
// scroll long focus item text
static T_BOOL IconExplorer_ScrollItemText(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;
    T_U32 Col, Row;
    T_RECT Rect;
    T_S16 PosX, PosY;
    T_U32 MaxLen;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    if ((pIconExplorer->pItemFocus == AK_NULL) || (pIconExplorer->pItemFocus->pText == AK_NULL))
        return AK_FALSE;


    switch (pIconExplorer->ItemIconStyle) {
        case ICONEXPLORER_LARGEICON:
            // get focus item col & row
            p = pIconExplorer->pItemShow;
            for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->LargeItemRow); Row++) {
                for (Col=0; (p!=AK_NULL)&&(Col<pIconExplorer->LargeItemCol); Col++) {
                    if (p == pIconExplorer->pItemFocus)
                        break;

                    p = p->pNext;
                }

                if ((Col < pIconExplorer->LargeItemCol) && (p == pIconExplorer->pItemFocus))
                    break;
            }
            if (p == AK_NULL)
                return AK_FALSE;

            // show focus item text back
            PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth + pIconExplorer->LargeItemHInterval + \
                    (pIconExplorer->LargeItemHInterval+pIconExplorer->LargeIconWidth)*Col);
            PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + \
                    pIconExplorer->LargeIconHeight + pIconExplorer->LargeItemVInterval + pIconExplorer->LargeItemTInterval + \
                    (pIconExplorer->LargeItemVInterval+pIconExplorer->LargeItemTInterval+pIconExplorer->LargeIconHeight+pIconExplorer->LargeTextHeight)*Row);
            Rect.width = (T_S16)(pIconExplorer->LargeIconWidth);
            Rect.height = (T_S16)(pIconExplorer->LargeTextHeight);
            Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemFocusBackColor);

            // get show focus item text pos
            if (IconList_GetLargeItemTextPos(pIconExplorer, pIconExplorer->pItemFocus, Col, Row, &PosX, &PosY, &MaxLen) == AK_TRUE)
                IconList_ShowItemText(pIconExplorer, pIconExplorer->pItemFocus, AK_TRUE, PosX, PosY, MaxLen);
            break;
        case ICONEXPLORER_NONEICON:
            // get focus item row
            p = pIconExplorer->pItemShow;
            for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->NoneItemRow); Row++) {
                if (p == pIconExplorer->pItemFocus)
                    break;

                p = p->pNext;
            }
            if (p == AK_NULL)
                return AK_FALSE;

            // get show focus item text pos
            if (IconList_GetNoneItemTextPos(pIconExplorer, pIconExplorer->pItemFocus, Row, &PosX, &PosY, &MaxLen) == AK_TRUE) {
                // show focus item text back
                PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth + pIconExplorer->NoneItemHInterval);
                PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->NoneItemVInterval + \
                        (pIconExplorer->NoneItemVInterval+pIconExplorer->NoneTextHeight)*Row);
                Rect.width = (T_S16)(pIconExplorer->ItemRect.width - pIconExplorer->ScrollBarShowWidth - \
                        pIconExplorer->ItemFrameWidth*2 - pIconExplorer->NoneItemHInterval);
                Rect.height = (T_S16)(pIconExplorer->NoneTextHeight);
                Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemFocusBackColor);

                IconList_ShowItemText(pIconExplorer, pIconExplorer->pItemFocus, AK_TRUE, PosX, PosY, MaxLen);
            }
            break;
        case ICONEXPLORER_SMALLICON:
        default:
            // get focus item row
            p = pIconExplorer->pItemShow;
            for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->SmallItemRow); Row++) {
                if (p == pIconExplorer->pItemFocus)
                    break;

                p = p->pNext;
            }
            if (p == AK_NULL)
                return AK_FALSE;

                // show focus item text back
                PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->SmallIconWidth + \
                        pIconExplorer->ItemFrameWidth + pIconExplorer->SmallItemHInterval + pIconExplorer->SmallItemTInterval);
                PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->SmallItemVInterval + \
                        (pIconExplorer->SmallItemVInterval+pIconExplorer->SmallIconHeight)*Row);
                Rect.width = (T_S16)(pIconExplorer->ItemRect.width - pIconExplorer->SmallIconWidth - pIconExplorer->ScrollBarShowWidth - \
                        pIconExplorer->ItemFrameWidth*2 - pIconExplorer->SmallItemHInterval - pIconExplorer->SmallItemTInterval);
                Rect.height = (T_S16)(pIconExplorer->SmallIconHeight);
                Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemFocusBackColor);
				
				// get show focus item text pos
				if (IconList_GetSmallItemTextPos(pIconExplorer, pIconExplorer->pItemFocus, Row, &PosX, &PosY, &MaxLen) == AK_TRUE) 
				{
	                IconList_ShowItemText(pIconExplorer, pIconExplorer->pItemFocus, AK_TRUE, PosX, PosY, MaxLen);
	            }
            break;
    }

    return AK_TRUE;
}
#endif

#if 0
// show scroll bar
static T_BOOL IconExplorer_ShowScrollBar(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;
    T_U16 Num = 0;
    T_U32 MaxQty;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    if ((pIconExplorer->pItemHead == AK_NULL) || (pIconExplorer->pItemShow == AK_NULL))
        return AK_FALSE;

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL) {
        if (p == pIconExplorer->pItemShow)
            break;

        Num++;
        p = p->pNext;
    }

    // check scroll bar max quantity
    switch (pIconExplorer->ItemIconStyle) {
        case ICONEXPLORER_LARGEICON:
            MaxQty = pIconExplorer->LargeItemRow*pIconExplorer->LargeItemCol;
            break;
        case ICONEXPLORER_NONEICON:
            MaxQty = pIconExplorer->NoneItemRow;
            break;
        case ICONEXPLORER_SMALLICON:
        default:
            MaxQty = pIconExplorer->SmallItemRow;
            break;
    }

    // set scroll bar value, show it
    ScBar_SetValue(&pIconExplorer->ScrollBar, Num, \
            (T_U16)(pIconExplorer->ItemQty), \
            (T_U16)(MaxQty));
    ScBar_Show(&pIconExplorer->ScrollBar);

    return AK_TRUE;
}
#endif
#if 0
// get title text pos
static T_BOOL IconExplorer_GetTitleTextPos(T_ICONEXPLORER *pIconExplorer, T_S16 *PosX, T_S16 *PosY, T_U32 *MaxLen)
{
    T_S16 OffsetX = 0, OffsetY = 0;
    T_U32  width = 0;

    AK_ASSERT_PTR(pIconExplorer, "IconExplorer_GetTitleTextPos", AK_FALSE);

    if (AK_NULL == pIconExplorer->pTitleText)
    {
        Fwl_Print(C3, M_CTRL, "IconExplorer_GetTitleTextPos(): pIconExplorer->pTitleText is NULL");
        return AK_FALSE;
    }

    *MaxLen = pIconExplorer->TitleRect.width;    
    width = UGetSpeciStringWidth(pIconExplorer->pTitleText, CURRENT_FONT_SIZE, Utl_UStrLen(pIconExplorer->pTitleText));

    if (width <= (*MaxLen)) {
        switch (pIconExplorer->ItemStyle & ICONEXPLORER_TITLE_TEXT_HALIGN) {
            case ICONEXPLORER_TITLE_TEXT_LEFT:
                OffsetX = 0;
                break;
            case ICONEXPLORER_TITLE_TEXT_RIGHT:
                OffsetX = (T_S16)(pIconExplorer->TitleRect.width - width);
                break;
            case ICONEXPLORER_TITLE_TEXT_HCENTER:
            default:
                OffsetX = (T_S16)(pIconExplorer->TitleRect.width - width)/2;
                break;
        }
    }
    *PosX = pIconExplorer->TitleRect.left + OffsetX;

    if (pIconExplorer->TitleRect.height > FONT_HEIGHT_DEFAULT) {
        switch (pIconExplorer->ItemStyle & ICONEXPLORER_TITLE_TEXT_VALIGN) {
            case ICONEXPLORER_TITLE_TEXT_UP:
                OffsetY = 0;
                break;
            case ICONEXPLORER_TITLE_TEXT_DOWN:
                OffsetY = pIconExplorer->TitleRect.height - FONT_HEIGHT_DEFAULT;
                break;
            case ICONEXPLORER_TITLE_TEXT_VCENTER:
            default:
                OffsetY = (pIconExplorer->TitleRect.height-FONT_HEIGHT_DEFAULT)/2;
                break;
        }
    }
    *PosY = pIconExplorer->TitleRect.top + OffsetY;

    return AK_TRUE;
}
#endif


