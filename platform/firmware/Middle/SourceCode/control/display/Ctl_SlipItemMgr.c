/**
 * @file Ctl_SlipItemMgr.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */

#include "ctl_slipItemMgr.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "eng_graph.h"
#include "lib_res_port.h"
#include "fwl_graphic.h"

#define SHOW_BG_COLOR	COLOR_BLACK

extern T_pDATA p_menu_bckgrnd;

static T_BOOL SlipItemMgr_CreatItems(T_ITEM_MGR *pItemMgr);
static T_BOOL SlipItemMgr_CopyItemToShow(T_ITEM_MGR *pItemMgr, T_U32 index);
static T_U32 SlipItemMgr_GetStoreLen(T_ITEM_MGR *pItemMgr, T_U32 index, T_POS *pos);
static T_BOOL SlipItemMgr_SetOverLen(T_ITEM_MGR *pItemMgr);
static T_BOOL SlipItemMgr_SetRemainLen(T_ITEM_MGR *pItemMgr, T_S32 count);
static T_BOOL SlipItemMgr_GetMinPosAndMaxPos(T_ITEM_MGR *pItemMgr, T_POS *minPos, T_POS *maxPos);
static T_BOOL SlipItemMgr_CycItems(T_ITEM_MGR *pItemMgr, T_POS minPos, T_POS maxPos, T_U32 emptyNum, T_S32 *count, T_S32 offset);
static T_BOOL SlipItemMgr_SetCurItem(T_ITEM_MGR *pItemMgr);



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
T_ITEM_MGR *SlipItemMgr_Creat(E_ITEMTYPE itemtype, T_U32 width, T_U32 height, T_U32 itemW, T_U32 itemH, T_U32 totalItemNum, E_MOVETYPE movetype)
{
	T_ITEM_MGR *pItemMgr = AK_NULL;
	T_U32 itemNumShow = 0;
	T_U32 len = 0;
	T_S32 nextRemainLen = 0;

	if ((movetype > MOVETYPE_Y) || (itemtype >= ITEM_TYPE_NUM ))
	{
		return AK_NULL;
	}
	
	pItemMgr = (T_ITEM_MGR *)Fwl_Malloc(sizeof(T_ITEM_MGR));
	AK_ASSERT_PTR(pItemMgr, "SlipItemMgr_Creat(): pItemMgr malloc error", AK_NULL);

	memset(pItemMgr, 0, sizeof(T_ITEM_MGR));

	pItemMgr->itemType = itemtype;
	pItemMgr->width = width;
	pItemMgr->height = height;
	pItemMgr->itemW = itemW;
	pItemMgr->itemH = itemH;
	pItemMgr->totalItemNum = totalItemNum;
	pItemMgr->moveType = movetype;

	pItemMgr->pShow = Fwl_Malloc(pItemMgr->width * pItemMgr->height * COLOR_SIZE + 16);
	if (AK_NULL == pItemMgr->pShow)
	{
		AK_DEBUG_OUTPUT("SlipItemMgr_Creat pItemMgr->pShow malloc error\n");
		SlipItemMgr_Destroy(pItemMgr);
		return AK_NULL;
	}

	memset(pItemMgr->pShow, 0, pItemMgr->width * pItemMgr->height * COLOR_SIZE + 16);

	if (ITEM_TYPE_LIST == pItemMgr->itemType)
	{
		if (p_menu_bckgrnd != AK_NULL)
	    {
	        pItemMgr->pBgImg = p_menu_bckgrnd;
	    }
	    else
	    {
	        pItemMgr->pBgImg = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_MENU_BACKGROUND, &len);
	    }

		pItemMgr->itemNumPerRow = 1;
		pItemMgr->itemNumPerCol = 1;

		itemNumShow = pItemMgr->height / pItemMgr->itemH + 1;

		if (pItemMgr->totalItemNum <= itemNumShow + STORE_ITEMNUM)
		{
			pItemMgr->itemNum = pItemMgr->totalItemNum;
		}
		else
		{
			pItemMgr->itemNum = itemNumShow + STORE_ITEMNUM;
		}

		nextRemainLen = pItemMgr->itemH * pItemMgr->totalItemNum - pItemMgr->height;
	}
	else if (ITEM_TYPE_IMAGE == pItemMgr->itemType)
	{
		pItemMgr->bgColor = SHOW_BG_COLOR;

		if (MOVETYPE_Y == pItemMgr->moveType)
		{
			pItemMgr->itemNumPerRow = pItemMgr->width / pItemMgr->itemW;
			pItemMgr->itemNumPerCol = 1;

			itemNumShow = (pItemMgr->height / pItemMgr->itemH) * pItemMgr->itemNumPerRow;

			if (pItemMgr->totalItemNum <= itemNumShow + STORE_ITEMNUM * pItemMgr->itemNumPerRow)
			{
				pItemMgr->itemNum = pItemMgr->totalItemNum;
			}
			else
			{
				pItemMgr->itemNum = itemNumShow + STORE_ITEMNUM * pItemMgr->itemNumPerRow;
			}

			if (0 == pItemMgr->totalItemNum % pItemMgr->itemNumPerRow)
			{
				nextRemainLen = pItemMgr->itemH * (pItemMgr->totalItemNum / pItemMgr->itemNumPerRow) - pItemMgr->height;
			}
			else
			{
				nextRemainLen = pItemMgr->itemH * (pItemMgr->totalItemNum / pItemMgr->itemNumPerRow + 1) - pItemMgr->height;
			}
		}

		else if (MOVETYPE_X == pItemMgr->moveType)
		{
			pItemMgr->itemNumPerRow = 1;
			pItemMgr->itemNumPerCol = pItemMgr->height / pItemMgr->itemH;

			itemNumShow = (pItemMgr->width / pItemMgr->itemW) * pItemMgr->itemNumPerCol;

			if (pItemMgr->totalItemNum <= itemNumShow + STORE_ITEMNUM * pItemMgr->itemNumPerCol)
			{
				pItemMgr->itemNum = pItemMgr->totalItemNum;
			}
			else
			{
				pItemMgr->itemNum = itemNumShow + STORE_ITEMNUM * pItemMgr->itemNumPerCol;
			}

			if (0 == pItemMgr->totalItemNum % pItemMgr->itemNumPerCol)
			{
				nextRemainLen = pItemMgr->itemW * (pItemMgr->totalItemNum / pItemMgr->itemNumPerCol) - pItemMgr->width;
			}
			else
			{
				nextRemainLen = pItemMgr->itemW * (pItemMgr->totalItemNum / pItemMgr->itemNumPerCol + 1) - pItemMgr->width;
			}
		}
	}

	pItemMgr->nextRemainLen = nextRemainLen > 0 ? nextRemainLen : 0;

	if (pItemMgr->itemNum > 0)
	{
		if (!SlipItemMgr_CreatItems(pItemMgr))
		{
			SlipItemMgr_Destroy(pItemMgr);
			return AK_NULL;
		}
	}

	return pItemMgr;
}

/**
* @brief Destroy a slip item manager control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipItemMgr_Destroy(T_ITEM_MGR *pItemMgr)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_SLIP_ITEM *q = AK_NULL;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_NULL;
	}

	if (AK_NULL != pItemMgr->ppItemhead)
	{
		p = *(pItemMgr->ppItemhead);

		while (AK_NULL != p)
		{
			q = p->pNext;
			p = SlipItem_Destroy(p);
			p = q;
		}

		pItemMgr->ppItemhead = Fwl_Free(pItemMgr->ppItemhead);
	}
	
	if (AK_NULL != pItemMgr->pShow)
	{
		pItemMgr->pShow = Fwl_Free(pItemMgr->pShow);
	}

	pItemMgr = Fwl_Free(pItemMgr);

	return AK_NULL;
}

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
T_BOOL SlipItemMgr_SetItem(T_ITEM_MGR *pItemMgr, T_U32 index, T_S32 id_of_item, T_pCDATA pIconLeft, T_pCDATA pIconRight, const T_U16* pTextMain, const T_U16* pTextDown, const T_U16* pTextRight)
{
	T_SLIP_ITEM *p = AK_NULL;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead + index % pItemMgr->itemNum);

	return SlipItem_SetItem(p, id_of_item, pIconLeft, pIconRight, pTextMain, pTextDown, pTextRight);
}

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
T_BOOL SlipItemMgr_SetItemId(T_ITEM_MGR *pItemMgr, T_U32 index, T_S32 id_of_item)
{
	T_SLIP_ITEM *p = AK_NULL;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead + index % pItemMgr->itemNum);

	return SlipItem_SetItemId(p, id_of_item);
}

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
T_BOOL SlipItemMgr_ChangeTextRightById(T_ITEM_MGR *pItemMgr, T_S32 id, const T_U16* pTextRight)
{
	T_SLIP_ITEM *p = AK_NULL;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead);

	while (AK_NULL != p)
	{		
		if (id == p->id_of_item)
		{
			return SlipItem_ChangeTextRight(p, pTextRight);
		}

		p = p->pNext;
	}

	return AK_FALSE;
}

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
T_BOOL SlipItemMgr_ShowItemById(T_ITEM_MGR *pItemMgr, T_S32 id, T_BOOL bFocus)
{
	T_SLIP_ITEM *p = AK_NULL;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead);

	while (AK_NULL != p)
	{		
		if (id == p->id_of_item)
		{
			return SlipItem_Show(p, bFocus);
		}

		p = p->pNext;
	}

	return AK_FALSE;
}



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
T_BOOL SlipItemMgr_ScrollShowItemById(T_ITEM_MGR *pItemMgr, T_S32 id, T_BOOL bFocus)
{
	T_SLIP_ITEM *p = AK_NULL;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead);

	while (AK_NULL != p)
	{		
		if (id == p->id_of_item)
		{
			return SlipItem_ScrollShow(p, bFocus);
		}

		p = p->pNext;
	}

	return AK_FALSE;
}


/**
* @brief clean Item focus
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipItemMgr_CleanItemFocus(T_ITEM_MGR *pItemMgr)
{
	T_SLIP_ITEM *p = AK_NULL;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead);

	while (AK_NULL != p)
	{		
		SlipItem_Show(p, AK_FALSE);
		p = p->pNext;
	}

	return AK_TRUE;
}

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
T_S32 SlipItemMgr_GetIndexById(T_ITEM_MGR *pItemMgr, T_S32 id)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_U32 i = 0;
	
	if (AK_NULL == pItemMgr)
	{
		return -1;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return -1;
	}

	p = *(pItemMgr->ppItemhead);

	for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
	{		
		if (id == p->id_of_item)
		{
			return i;
		}

		p = p->pNext;
	}

	return -1;
}

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
T_S32 SlipItemMgr_GetCurItemId(T_ITEM_MGR *pItemMgr)
{
	T_SLIP_ITEM *p = AK_NULL;
	
	if (AK_NULL == pItemMgr)
	{
		return -1;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return -1;
	}

	p = *(pItemMgr->ppItemhead + pItemMgr->curItem);

	if (AK_NULL == p)
	{
		return -1;
	}

	return p->id_of_item;
}

/**
* @brief get item height
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetItemHeight(T_ITEM_MGR *pItemMgr)
{
	if (AK_NULL == pItemMgr)
	{
		return 0;
	}

	return pItemMgr->itemH;
}

/**
* @brief get item width
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetItemWidth(T_ITEM_MGR *pItemMgr)
{
	if (AK_NULL == pItemMgr)
	{
		return 0;
	}

	return pItemMgr->itemW;
}

/**
* @brief get item num per row
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetItemNumPerRow(T_ITEM_MGR *pItemMgr)
{
	if (AK_NULL == pItemMgr)
	{
		return 0;
	}

	return pItemMgr->itemNumPerRow;
}

/**
* @brief get item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetItemNum(T_ITEM_MGR *pItemMgr)
{
	if (AK_NULL == pItemMgr)
	{
		return 0;
	}

	return pItemMgr->itemNum;
}

/**
* @brief get loaded item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetLoadItemNum(T_ITEM_MGR *pItemMgr)
{
	if (AK_NULL == pItemMgr)
	{
		return 0;
	}

	return pItemMgr->loadItemNum;
}

/**
* @brief get total item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetTotalItemNum(T_ITEM_MGR *pItemMgr)
{
	if (AK_NULL == pItemMgr)
	{
		return 0;
	}

	return pItemMgr->totalItemNum;
}

/**
* @brief set total item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_SetTotalItemNum(T_ITEM_MGR *pItemMgr, T_U32 totalnum)
{
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (MOVETYPE_Y == pItemMgr->moveType)
	{	
		pItemMgr->nextRemainLen += (totalnum - pItemMgr->totalItemNum) * pItemMgr->itemH;
	}
	else if (MOVETYPE_X == pItemMgr->moveType)
	{
		pItemMgr->nextRemainLen += (totalnum - pItemMgr->totalItemNum) * pItemMgr->itemW;
	}

	pItemMgr->totalItemNum = totalnum;

	return AK_TRUE;
}


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
T_BOOL SlipItemMgr_AddLoadItemNum(T_ITEM_MGR *pItemMgr, T_S32 count)
{
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	
	pItemMgr->loadItemNum += count;

	return AK_TRUE;
}

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
T_BOOL SlipItemMgr_SetLoadItemNum(T_ITEM_MGR *pItemMgr, T_U32 num)
{
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	
	pItemMgr->loadItemNum = num;

	return AK_TRUE;
}

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
T_BOOL SlipItemMgr_SetBgColor(T_ITEM_MGR *pItemMgr, T_COLOR color)
{
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	
	pItemMgr->bgColor = color;

	return AK_TRUE;
}

/**
* @brief get total store len
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipItemMgr_GetTotalStoreLen(T_ITEM_MGR *pItemMgr)
{
	T_U32 i = 0;
	T_U32 storeLenTemp = 0;
	T_U32 storeLenTotal = 0;
	T_POS pos_old = 0;
	T_POS pos_new = 0;

	if (AK_NULL == pItemMgr)
	{
		return 0;
	}
	
	for (i=0; i<pItemMgr->itemNum; i++)
	{
		storeLenTemp = SlipItemMgr_GetStoreLen(pItemMgr, i, &pos_new);

		if (pos_new != pos_old)
		{
			storeLenTotal += storeLenTemp;
			pos_old = pos_new;
		}
	}

	return storeLenTotal;
}

/**
* @brief fill the show buf with item buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_BOOL
* @retval 
*/
T_BOOL SlipItemMgr_FillBuf(T_ITEM_MGR *pItemMgr)
{
	T_U32 i = 0;
	T_RECT rect;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	RectInit(&rect, 0, 0, (T_LEN)pItemMgr->width, (T_LEN)pItemMgr->height);

	if (ITEM_TYPE_LIST == pItemMgr->itemType)
	{
		Fwl_AkBmpDrawPartFromStringOnRGB(pItemMgr->pShow, pItemMgr->width, pItemMgr->height, 0, 0,
									&rect, pItemMgr->pBgImg, AK_NULL, AK_FALSE, RGB565);
	}
	else if (ITEM_TYPE_IMAGE == pItemMgr->itemType)
	{
		Fwl_FillSolidRectOnRGB(pItemMgr->pShow, pItemMgr->width, pItemMgr->height, &rect, pItemMgr->bgColor, RGB565);
	}
	
	for (i=0; i<pItemMgr->itemNum; i++)
	{
		SlipItemMgr_CopyItemToShow(pItemMgr, i);
	}

	return AK_TRUE;
}


/**
* @brief Get Item backgroud img
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr:the ItemMgr handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipItemMgr_LoadItemBgImg(T_ITEM_MGR *pItemMgr)
{
	//T_U32 i = 0;
	T_U32 len = 0;
	//T_SLIP_ITEM *p = AK_NULL;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (p_menu_bckgrnd != AK_NULL)
    {
        pItemMgr->pBgImg = p_menu_bckgrnd;
    }
    else
    {
        pItemMgr->pBgImg = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_MENU_BACKGROUND, &len);
    }
/*
	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead);

	for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
	{
		SlipItem_LoadItemBgImg(p);
		p = p->pNext;
	}
*/
	return AK_TRUE;
}


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
T_BOOL SlipItemMgr_ClearOffset(T_ITEM_MGR *pItemMgr, T_U32 num)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_U32 i = 0;
	T_U32 j = 0;
	T_S32 nextRemainLen = 0;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return 0;
	}

	p = *(pItemMgr->ppItemhead);

	if (ITEM_TYPE_LIST == pItemMgr->itemType)
	{
		for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
		{
			p->rect.top = (T_POS)i * p->rect.height;
			p = p->pNext;
		}

		nextRemainLen = pItemMgr->itemH * (pItemMgr->totalItemNum - num) - pItemMgr->height;
		pItemMgr->preRemainLen = pItemMgr->itemH * num;
	}
	else if (ITEM_TYPE_IMAGE == pItemMgr->itemType)
	{
		if (MOVETYPE_Y == pItemMgr->moveType)
		{
			for (i=0; i<pItemMgr->itemNum/pItemMgr->itemNumPerRow+1; i++)
			{
				for (j=0; (j<pItemMgr->itemNumPerRow)&&(i*pItemMgr->itemNumPerRow+j<pItemMgr->itemNum); j++)
				{
					p->rect.top = (T_POS)i * p->rect.height;
					p = p->pNext;
				}
			}

			if (0 == (pItemMgr->totalItemNum - num) % pItemMgr->itemNumPerRow)
			{
				nextRemainLen = pItemMgr->itemH * ((pItemMgr->totalItemNum - num) / pItemMgr->itemNumPerRow) - pItemMgr->height;
			}
			else
			{
				nextRemainLen = pItemMgr->itemH * ((pItemMgr->totalItemNum - num) / pItemMgr->itemNumPerRow + 1) - pItemMgr->height;
			}

			pItemMgr->preRemainLen = pItemMgr->itemH * num / pItemMgr->itemNumPerRow;
		}

		else if (MOVETYPE_X == pItemMgr->moveType)
		{
			for (i=0; i<pItemMgr->itemNum/pItemMgr->itemNumPerCol+1; i++)
			{
				for (j=0; (j<pItemMgr->itemNumPerCol)&&(i*pItemMgr->itemNumPerCol+j<pItemMgr->itemNum); j++)
				{
					p->rect.left = (T_POS)i * p->rect.width;
					p = p->pNext;
				}
			}

			if (0 == (pItemMgr->totalItemNum - num) % pItemMgr->itemNumPerCol)
			{
				nextRemainLen = pItemMgr->itemW * ((pItemMgr->totalItemNum - num) / pItemMgr->itemNumPerCol) - pItemMgr->width;
			}
			else
			{
				nextRemainLen = pItemMgr->itemW * ((pItemMgr->totalItemNum - num) / pItemMgr->itemNumPerCol + 1) - pItemMgr->width;
			}
			
			pItemMgr->preRemainLen = pItemMgr->itemW * num / pItemMgr->itemNumPerCol;
		}
	}

	pItemMgr->nextRemainLen = nextRemainLen > 0 ? nextRemainLen : 0;

	pItemMgr->curItem = 0;
	pItemMgr->overLen = 0;

	return AK_TRUE;
}


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
T_S32 SlipItemMgr_AddOffset(T_ITEM_MGR *pItemMgr, T_S32 offset, T_U32 emptyNum)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_S32 ret = 0;
	T_U32 i = 0;
	T_POS minPos = (T_POS)0x7fff;
	T_POS maxPos = (T_POS)0x8000;
	T_U32 cnt = 0;
	T_U32 length = 0;
	T_U32 emptyItemNum = emptyNum;
	
	if (AK_NULL == pItemMgr)
	{
		return 0;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return 0;
	}

	p = *(pItemMgr->ppItemhead);

	for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
	{
		if (MOVETYPE_Y == pItemMgr->moveType)
		{
			p->rect.top += (T_POS)offset;
		}
		else if (MOVETYPE_X == pItemMgr->moveType)
		{
			p->rect.left += (T_POS)offset;
		}
			
		p = p->pNext;
	}

	length = offset >= 0 ? offset : 0 - offset;

	if (MOVETYPE_Y == pItemMgr->moveType)
	{
		cnt = length / pItemMgr->itemH + 1;
	}
	else if (MOVETYPE_X == pItemMgr->moveType)
	{
		cnt = length / pItemMgr->itemW + 1;
	}

	for (i=0; i<cnt; i++)
	{
		SlipItemMgr_GetMinPosAndMaxPos(pItemMgr, &minPos, &maxPos);
		SlipItemMgr_CycItems(pItemMgr, minPos, maxPos, emptyItemNum, &ret, offset);
		emptyItemNum = 0;
	}
	
	SlipItemMgr_SetCurItem(pItemMgr);
	SlipItemMgr_SetOverLen(pItemMgr);
	SlipItemMgr_SetRemainLen(pItemMgr, ret);

	return ret;
}

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
T_BOOL SlipItemMgr_GetDisBuf(T_ITEM_MGR *pItemMgr, T_U8 **pbuf, T_U32 *width, T_U32 *height)
{
	if ((AK_NULL == pItemMgr) || (AK_NULL == pbuf) || (AK_NULL == width) || (AK_NULL == height))
	{
		return AK_FALSE;
	}

	*pbuf = pItemMgr->pShow;
	*width = pItemMgr->width;
	*height = pItemMgr->height;

	return AK_TRUE;
}

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
T_BOOL SlipItemMgr_GetItemBufByIndex(T_ITEM_MGR *pItemMgr, T_U32 index, T_U8 **pbuf, T_U32 *width, T_U32 *height)
{
	T_SLIP_ITEM *p = AK_NULL;
	
	if ((AK_NULL == pItemMgr) || (AK_NULL == pbuf) || (AK_NULL == width) || (AK_NULL == height) || (AK_NULL == pItemMgr->ppItemhead))
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead + index % pItemMgr->itemNum);
	

	return SlipItem_GetItemBuf(p, pbuf, width, height);

	return AK_TRUE;
}

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
T_S32 SlipItemMgr_GetItemIdByPoint(T_ITEM_MGR *pItemMgr, T_POS x, T_POS y)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_U32 i = 0;
	
	if (AK_NULL == pItemMgr)
	{
		return -1;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return -1;
	}

	p = *(pItemMgr->ppItemhead);

	for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
	{
		if ((x >= p->rect.left) && (x <= p->rect.left + p->rect.width)
			&& (y >= p->rect.top) && (y <= p->rect.top + p->rect.height))
		{
			return p->id_of_item;
		}

		p = p->pNext;
	}

	return -1;
}

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
T_BOOL SlipItemMgr_CheckFocusItem(T_ITEM_MGR *pItemMgr, T_U32 focusId)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_U32 i = 0;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead);

	for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
	{		
		if ((T_S32)focusId == p->id_of_item)
		{
			if ((p->rect.top >= 0) && (p->rect.top + p->rect.height <= (T_S16)pItemMgr->height)
				&& (p->rect.left >= 0) && (p->rect.left + p->rect.width <= (T_S16)pItemMgr->width))
			{
				return AK_TRUE;
			}
		}

		p = p->pNext;
	}
	
	return AK_FALSE;
}



/**
* @brief get over len
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_S32
* @retval 
*/
T_S32 SlipItemMgr_GetOverLen(T_ITEM_MGR *pItemMgr)
{
	if (AK_NULL == pItemMgr)
	{
		return 0;
	}

	return pItemMgr->overLen;
}


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
T_BOOL SlipItemMgr_GetRemainLen(T_ITEM_MGR *pItemMgr, T_S32 *nextRemainLen, T_S32 *preRemainLen)
{
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL != nextRemainLen)
	{
		*nextRemainLen = pItemMgr->nextRemainLen;
	}

	if (AK_NULL != preRemainLen)
	{
		*preRemainLen = pItemMgr->preRemainLen;
	}

	return AK_TRUE;
}

/**
* @brief set remain len
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_S32 count: count need to load
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipItemMgr_SetRemainLen(T_ITEM_MGR *pItemMgr, T_S32 count)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_U32 i = 0;
	T_S32 bottomItem = -1;
	T_S32 nextRemainLen = 0;
	T_S32 preRemainLen = 0;
	T_POS top_max = 0;
	T_POS left_max = 0;
	T_U32 storeLen = 0;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	storeLen = SlipItemMgr_GetTotalStoreLen(pItemMgr);

	p = *(pItemMgr->ppItemhead);

	if (MOVETYPE_Y == pItemMgr->moveType)
	{
		for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
		{
			if ((p->rect.top < (T_S16)pItemMgr->height) && (p->rect.top + p->rect.height >= (T_S16)pItemMgr->height))
			{
				bottomItem = i;
				break;
			}

			p = p->pNext;
		}

		if (-1 == bottomItem)
		{
			pItemMgr->nextRemainLen = 0;

			p = *(pItemMgr->ppItemhead);

			while (AK_NULL != p)
			{
				if (p->rect.top > top_max)
				{
					top_max = p->rect.top;
				}

				p = p->pNext;
			}

			if (0 == pItemMgr->loadItemNum % pItemMgr->itemNumPerRow)
			{
				pItemMgr->preRemainLen = pItemMgr->loadItemNum / pItemMgr->itemNumPerRow * pItemMgr->itemH - top_max - pItemMgr->itemH;
			}
			else
			{
				pItemMgr->preRemainLen = (pItemMgr->loadItemNum / pItemMgr->itemNumPerRow + 1) * pItemMgr->itemH - top_max - pItemMgr->itemH;
			}
			
		}
		else
		{		
			if (0 == (pItemMgr->totalItemNum - pItemMgr->loadItemNum - count) % pItemMgr->itemNumPerRow)
			{
				nextRemainLen = (pItemMgr->totalItemNum - pItemMgr->loadItemNum - count) / pItemMgr->itemNumPerRow * pItemMgr->itemH + storeLen;
			}
			else
			{
				nextRemainLen = ((pItemMgr->totalItemNum - pItemMgr->loadItemNum - count) / pItemMgr->itemNumPerRow + 1) * pItemMgr->itemH + storeLen;
			}
			
			pItemMgr->nextRemainLen = nextRemainLen > 0 ? nextRemainLen : 0;


			if (0 == pItemMgr->totalItemNum % pItemMgr->itemNumPerRow)
			{
				preRemainLen = pItemMgr->totalItemNum / pItemMgr->itemNumPerRow * pItemMgr->itemH - pItemMgr->nextRemainLen - pItemMgr->height;
			}
			else
			{
				preRemainLen = (pItemMgr->totalItemNum / pItemMgr->itemNumPerRow + 1) * pItemMgr->itemH - pItemMgr->nextRemainLen - pItemMgr->height;
			}
			
			pItemMgr->preRemainLen = preRemainLen > 0 ? preRemainLen : 0;
		}
	}
	else if (MOVETYPE_X == pItemMgr->moveType)
	{
		for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
		{
			if ((p->rect.left < (T_S16)pItemMgr->width) && (p->rect.left + p->rect.width >= (T_S16)pItemMgr->width))
			{
				bottomItem = i;
				break;
			}

			p = p->pNext;
		}

		if (-1 == bottomItem)
		{
			pItemMgr->nextRemainLen = 0;

			p = *(pItemMgr->ppItemhead);

			while (AK_NULL != p)
			{
				if (p->rect.left > left_max)
				{
					left_max = p->rect.left;
				}

				p = p->pNext;
			}

			if (0 == pItemMgr->loadItemNum % pItemMgr->itemNumPerCol)
			{
				pItemMgr->preRemainLen = pItemMgr->loadItemNum / pItemMgr->itemNumPerCol * pItemMgr->itemW - left_max - pItemMgr->itemW;
			}
			else
			{
				pItemMgr->preRemainLen = (pItemMgr->loadItemNum / pItemMgr->itemNumPerCol + 1) * pItemMgr->itemW - left_max - pItemMgr->itemW;
			}
			
		}
		else
		{		
			if (0 == (pItemMgr->totalItemNum - pItemMgr->loadItemNum - count) % pItemMgr->itemNumPerCol)
			{
				nextRemainLen = (pItemMgr->totalItemNum - pItemMgr->loadItemNum - count) / pItemMgr->itemNumPerCol * pItemMgr->itemW + storeLen;
			}
			else
			{
				nextRemainLen = ((pItemMgr->totalItemNum - pItemMgr->loadItemNum - count) / pItemMgr->itemNumPerCol + 1) * pItemMgr->itemW + storeLen;
			}
			
			pItemMgr->nextRemainLen = nextRemainLen > 0 ? nextRemainLen : 0;


			if (0 == pItemMgr->totalItemNum % pItemMgr->itemNumPerCol)
			{
				preRemainLen = pItemMgr->totalItemNum / pItemMgr->itemNumPerCol * pItemMgr->itemW - pItemMgr->nextRemainLen - pItemMgr->width;
			}
			else
			{
				preRemainLen = (pItemMgr->totalItemNum / pItemMgr->itemNumPerCol + 1) * pItemMgr->itemW - pItemMgr->nextRemainLen - pItemMgr->width;
			}
			
			pItemMgr->preRemainLen = preRemainLen > 0 ? preRemainLen : 0;
		}
	}

	return AK_TRUE;
}

/**
* @brief set over len
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipItemMgr_SetOverLen(T_ITEM_MGR *pItemMgr)
{
	T_POS top_min = (T_POS)0x7fff;
	T_POS top_max = (T_POS)0x8000;
	T_POS left_min = (T_POS)0x7fff;
	T_POS left_max = (T_POS)0x8000;
	T_SLIP_ITEM *p = AK_NULL;
	T_S32 length = 0;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead);

	if (MOVETYPE_Y == pItemMgr->moveType)
	{
		while (AK_NULL != p)
		{
			if (p->rect.top < top_min)
			{
				top_min = p->rect.top;
			}

			if (p->rect.top > top_max)
			{
				top_max = p->rect.top;
			}

			p = p->pNext;
		}

		if (0 == pItemMgr->itemNum % pItemMgr->itemNumPerRow)
		{
			length = pItemMgr->itemH * pItemMgr->itemNum / pItemMgr->itemNumPerRow >= pItemMgr->height ? pItemMgr->height : pItemMgr->itemH * pItemMgr->itemNum / pItemMgr->itemNumPerRow;
		}
		else
		{
			length = pItemMgr->itemH * (pItemMgr->itemNum / pItemMgr->itemNumPerRow + 1) >= pItemMgr->height ? pItemMgr->height : pItemMgr->itemH * (pItemMgr->itemNum / pItemMgr->itemNumPerRow + 1);
		}
		

		if (top_min > 0)
		{
			pItemMgr->overLen = top_min;
		}
		else if ((T_S32)(top_max + pItemMgr->itemH) < length)
		{
			pItemMgr->overLen = top_max + pItemMgr->itemH - length;
		}
		else
		{
			pItemMgr->overLen = 0;
		}
	}
	else if (MOVETYPE_X == pItemMgr->moveType)
	{
		while (AK_NULL != p)
		{
			if (p->rect.left < left_min)
			{
				left_min = p->rect.left;
			}

			if (p->rect.left > left_max)
			{
				left_max = p->rect.left;
			}

			p = p->pNext;
		}

		if (0 == pItemMgr->itemNum % pItemMgr->itemNumPerCol)
		{
			length = pItemMgr->itemW * pItemMgr->itemNum / pItemMgr->itemNumPerCol >= pItemMgr->width ? pItemMgr->width : pItemMgr->itemW * pItemMgr->itemNum / pItemMgr->itemNumPerCol;
		}
		else
		{
			length = pItemMgr->itemW * (pItemMgr->itemNum / pItemMgr->itemNumPerCol+ 1) >= pItemMgr->width ? pItemMgr->width : pItemMgr->itemW * (pItemMgr->itemNum / pItemMgr->itemNumPerCol + 1);
		}
		
		if (left_min > 0)
		{
			pItemMgr->overLen = left_min;
		}
		else if ((T_S32)(left_max + pItemMgr->itemW) < length)
		{
			pItemMgr->overLen = left_max + pItemMgr->itemW - length;
		}
		else
		{
			pItemMgr->overLen = 0;
		}
	}

	return AK_TRUE;
}


/**
* @brief creat items
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipItemMgr_CreatItems(T_ITEM_MGR *pItemMgr)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_SLIP_ITEM *q = AK_NULL;
	T_U32 i = 0;
	T_U32 j = 0;
	T_RECT rect;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	pItemMgr->ppItemhead = (T_SLIP_ITEM **)Fwl_Malloc(pItemMgr->itemNum * sizeof(T_U32));

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		AK_DEBUG_OUTPUT("SlipItemMgr_Creat pItemMgr->ppItemhead malloc error\n");
		return AK_FALSE;
	}

	memset(pItemMgr->ppItemhead, 0, pItemMgr->itemNum * sizeof(T_U32));
	
	q = *(pItemMgr->ppItemhead);

	if (MOVETYPE_Y == pItemMgr->moveType)
	{
		for (i=0; i<pItemMgr->itemNum/pItemMgr->itemNumPerRow+1; i++)
		{
			for (j=0; (j<pItemMgr->itemNumPerRow)&&(i*pItemMgr->itemNumPerRow+j<pItemMgr->itemNum); j++)
			{
				RectInit(&rect, (T_POS)(j*pItemMgr->itemW), (T_POS)(i*pItemMgr->itemH), (T_LEN)pItemMgr->itemW, (T_LEN)pItemMgr->itemH);
				p = SlipItem_Creat(pItemMgr->itemType, rect);

				if (AK_NULL != p)
				{
					p->pPrevious = q;
		            p->pNext = AK_NULL;

					if (q == AK_NULL)
					{
		                *(pItemMgr->ppItemhead) = p;
					}
		            else
		            {
		                q->pNext = p;
						*(pItemMgr->ppItemhead + i * pItemMgr->itemNumPerRow + j) = p;
		            }

					q = p;
				}
				else
				{
					return AK_FALSE;
				}
			}
		}
	}
	else if (MOVETYPE_X == pItemMgr->moveType)
	{
		for (i=0; i<pItemMgr->itemNum/pItemMgr->itemNumPerCol+1; i++)
		{
			for (j=0; (j<pItemMgr->itemNumPerCol)&&(i*pItemMgr->itemNumPerCol+j<pItemMgr->itemNum); j++)
			{
				RectInit(&rect, (T_POS)(i*pItemMgr->itemW), (T_POS)(j*pItemMgr->itemH), (T_LEN)pItemMgr->itemW, (T_LEN)pItemMgr->itemH);
				p = SlipItem_Creat(pItemMgr->itemType, rect);

				if (AK_NULL != p)
				{
					p->pPrevious = q;
		            p->pNext = AK_NULL;

					if (q == AK_NULL)
					{
		                *(pItemMgr->ppItemhead) = p;
					}
		            else
		            {
		                q->pNext = p;
						*(pItemMgr->ppItemhead + i * pItemMgr->itemNumPerCol + j) = p;
		            }

					q = p;
				}
				else
				{
					return AK_FALSE;
				}
			}
		}
	}

	return AK_TRUE;
}

/**
* @brief copy item buf to show buf by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_U32 index: the Item index
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipItemMgr_CopyItemToShow(T_ITEM_MGR *pItemMgr, T_U32 index)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_U8    *pBuffer = AK_NULL;
	T_U8    *pData = AK_NULL;
	T_U32 src_offset = 0;
	T_U32 dst_offset = 0;
	T_U32 length = 0;
	T_U32 i = 0;
	T_S16 j = 0;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead + index % pItemMgr->itemNum);

	if (MOVETYPE_Y == pItemMgr->moveType)
	{
		if (p->rect.top >= 0)
		{
			if ((T_U32)p->rect.top < pItemMgr->height)
			{
				src_offset = 0;
				dst_offset = p->rect.top;
				length = (T_U32)(p->rect.top + p->rect.height) > pItemMgr->height ? pItemMgr->height - p->rect.top : p->rect.height;
			}
		}
		else if ((p->rect.top < 0) && (p->rect.top + p->rect.height > 0))
		{
			src_offset = 0 - p->rect.top;
			dst_offset = 0;
			length = p->rect.height + p->rect.top;
		}
		else
		{
			return AK_FALSE;
		}

		pBuffer = pItemMgr->pShow + (dst_offset * pItemMgr->width + p->rect.left) * COLOR_SIZE; 
		pData = p->pbuf + src_offset * p->rect.width * COLOR_SIZE;

		if (ITEM_TYPE_IMAGE == pItemMgr->itemType)
		{
		    for(i = 0; i < length; i++)
		    {
		        memcpy(pBuffer, pData, p->rect.width * COLOR_SIZE);
		        pData += p->rect.width * COLOR_SIZE;
		        pBuffer += pItemMgr->width * COLOR_SIZE;
		    }
		}
		else if (ITEM_TYPE_LIST == pItemMgr->itemType)
		{
#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
			T_U8 low = 0;
			T_U8 high = 0;

			Fwl_ColorToRGB565(ITEM_BG_TRANS_COLOR, &low, &high);
#endif			
			for(i=0; i<length; i++)
		    {
		    	for (j=0; j<p->rect.width; j++)
		    	{
#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
					if (low != (T_U8)(*pData)
						|| high != (T_U8)(*(pData + 1)))
		    		{
				        memcpy(pBuffer, pData, COLOR_SIZE);
		    		}
#else
		    		if (ITEM_BG_TRANS_COLOR != (T_COLOR)((*pData << 16) | (*(pData + 1) << 8) | (*(pData + 2)))
						&& (ITEM_BG_TRANS_COLOR & 0xF8FCF8) != (T_COLOR)((*pData << 16) | (*(pData + 1) << 8) | (*(pData + 2))))
		    		{
				        memcpy(pBuffer, pData, COLOR_SIZE);
		    		}
#endif
					pData += COLOR_SIZE;
				    pBuffer += COLOR_SIZE;
		    	}

				pBuffer += (pItemMgr->width - p->rect.width) * COLOR_SIZE;
		    }
		}
	}
	else if (MOVETYPE_X == pItemMgr->moveType)
	{
		if (p->rect.left >= 0)
		{
			if ((T_U32)p->rect.left < pItemMgr->width)
			{
				src_offset = 0;
				dst_offset = p->rect.left;
				length = (T_U32)(p->rect.left + p->rect.width) > pItemMgr->width ? pItemMgr->width - p->rect.left : p->rect.width;
			}
		}
		else if ((p->rect.left < 0) && (p->rect.left + p->rect.width > 0))
		{
			src_offset = 0 - p->rect.left;
			dst_offset = 0;
			length = p->rect.width + p->rect.left;
		}
		else
		{
			return AK_FALSE;
		}

		pBuffer = pItemMgr->pShow + (p->rect.top * pItemMgr->width + dst_offset) * COLOR_SIZE; 
		pData = p->pbuf + src_offset * COLOR_SIZE;

	    for(i = 0; i < (T_U32)p->rect.height; i++)
	    {
	        memcpy(pBuffer, pData, length * COLOR_SIZE);
	        pData += p->rect.width * COLOR_SIZE;
	        pBuffer += pItemMgr->width * COLOR_SIZE;
	    }
	}

	return AK_TRUE;
}

/**
* @brief get store len by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_U32 index: the Item index
* @param out T_POS *pos: the Item top or left
* @return T_U32
* @retval 
*/
static T_U32 SlipItemMgr_GetStoreLen(T_ITEM_MGR *pItemMgr, T_U32 index, T_POS *pos)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_U32 length = 0;
	T_U32 storeLen = 0;
	
	if (AK_NULL == pItemMgr)
	{
		return 0;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return 0;
	}

	p = *(pItemMgr->ppItemhead + index % pItemMgr->itemNum);

	if (MOVETYPE_Y == pItemMgr->moveType)
	{
		if (p->rect.top >= 0)
		{
			if ((T_U32)p->rect.top < pItemMgr->height)
			{
				length = (T_U32)(p->rect.top + p->rect.height) > pItemMgr->height ? pItemMgr->height - p->rect.top : p->rect.height;
				storeLen = p->rect.height - length;
			}
			else
			{
				storeLen = p->rect.height;
			}

			if (storeLen > 0)
			{
				*pos = p->rect.top;
			}
		}
		else
		{
			return 0;
		}
	}
	else if (MOVETYPE_X == pItemMgr->moveType)
	{
		if (p->rect.left >= 0)
		{
			if ((T_U32)p->rect.left < pItemMgr->width)
			{
				length = (T_U32)(p->rect.left + p->rect.width) > pItemMgr->width ? pItemMgr->width - p->rect.left : p->rect.width;
				storeLen = p->rect.width - length;
			}
			else
			{
				storeLen = p->rect.width;
			}

			if (storeLen > 0)
			{
				*pos = p->rect.left;
			}
		}
		else
		{
			return 0;
		}
	}

	return storeLen;
}


/**
* @brief get min pos and max pos
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param out T_POS *minPos: the min pos
* @param out T_POS *maxPos: the max pos
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipItemMgr_GetMinPosAndMaxPos(T_ITEM_MGR *pItemMgr, T_POS *minPos, T_POS *maxPos)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_U32 i = 0;
	T_POS top_min = (T_POS)0x7fff;
	T_POS top_max = (T_POS)0x8000;
	T_POS left_min = (T_POS)0x7fff;
	T_POS left_max = (T_POS)0x8000;
	
	if ((AK_NULL == pItemMgr) || (AK_NULL == minPos) || (AK_NULL == maxPos))
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead);

	for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
	{
		if (MOVETYPE_Y == pItemMgr->moveType)
		{
			if (p->rect.top < top_min)
			{
				top_min = p->rect.top;
			}

			if (p->rect.top > top_max)
			{
				top_max = p->rect.top;
			}
		}
		else if (MOVETYPE_X == pItemMgr->moveType)
		{
			if (p->rect.left < left_min)
			{
				left_min = p->rect.left;
			}

			if (p->rect.left > left_max)
			{
				left_max = p->rect.left;
			}
		}
			
		p = p->pNext;
	}

	if (MOVETYPE_Y == pItemMgr->moveType)
	{
		*minPos = top_min;
		*maxPos = top_max;
	}
	else if (MOVETYPE_X == pItemMgr->moveType)
	{
		*minPos = left_min;
		*maxPos = left_max;
	}

	return AK_TRUE;
}

/**
* @brief Cyc  items and calc the count need to load
* 
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @param in T_POS minPos: the min pos
* @param in T_POS maxPos: the max pos
* @param in T_U32 emptyNum: the empty item num
* @param in/out T_S32 *count: the count need to load
* @param in T_S32 offset:offset
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipItemMgr_CycItems(T_ITEM_MGR *pItemMgr, T_POS minPos, T_POS maxPos, T_U32 emptyNum, T_S32 *count, T_S32 offset)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_U32 i = 0;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead);

	for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
	{
		if (MOVETYPE_Y == pItemMgr->moveType)
		{
			if (offset >= 0)
			{
				if ((p->rect.top > (T_S16)((pItemMgr->itemNum / pItemMgr->itemNumPerRow - STORE_ITEMNUM) * pItemMgr->itemH))
					&& (maxPos == p->rect.top)
					&& (pItemMgr->loadItemNum + *count + emptyNum > pItemMgr->itemNum))
				{
					p->rect.top -= (T_S16)(pItemMgr->itemNum / pItemMgr->itemNumPerRow * pItemMgr->itemH);
					*count = *count - 1;
				}
			}
			else
			{
				if ((p->rect.top < (T_S16)(0 - (STORE_ITEMNUM / 2 * pItemMgr->itemH)))
					&& (minPos == p->rect.top)
					&& (pItemMgr->loadItemNum + *count < pItemMgr->totalItemNum))
				{
					p->rect.top += (T_S16)(pItemMgr->itemNum / pItemMgr->itemNumPerRow * pItemMgr->itemH);
					*count = *count + 1;
				}
			}
		
		}
		else if (MOVETYPE_X == pItemMgr->moveType)
		{
			if (offset >= 0)
			{
				if ((p->rect.left > (T_S16)((pItemMgr->itemNum / pItemMgr->itemNumPerCol - STORE_ITEMNUM) * pItemMgr->itemW))
					&& (maxPos == p->rect.left)
					&& (pItemMgr->loadItemNum + *count > pItemMgr->itemNum))
				{
					p->rect.left -= (T_S16)(pItemMgr->itemNum / pItemMgr->itemNumPerCol * pItemMgr->itemW);
					*count = *count - 1;
				}
			}
			else
			{
				if ((p->rect.left < (T_S16)(0 - (STORE_ITEMNUM / 2 * pItemMgr->itemW)))
					&& (minPos == p->rect.left)
					&& (pItemMgr->loadItemNum + *count < pItemMgr->totalItemNum))
				{
					p->rect.left += (T_S16)(pItemMgr->itemNum / pItemMgr->itemNumPerCol * pItemMgr->itemW);
					*count = *count + 1;
				}
			}
		}
		
		p = p->pNext;
	}

	return AK_TRUE;
}

/**
* @brief set cur item
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_ITEM_MGR *pItemMgr: the ItemMgr handle
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipItemMgr_SetCurItem(T_ITEM_MGR *pItemMgr)
{
	T_SLIP_ITEM *p = AK_NULL;
	T_U32 i = 0;
	
	if (AK_NULL == pItemMgr)
	{
		return AK_FALSE;
	}

	if (AK_NULL == pItemMgr->ppItemhead)
	{
		return AK_FALSE;
	}

	p = *(pItemMgr->ppItemhead);

	for (i=0; (i<pItemMgr->itemNum)&&(AK_NULL!=p); i++)
	{
		if (MOVETYPE_Y == pItemMgr->moveType)
		{
			if ((((p->rect.top <= 0) && (p->rect.top + p->rect.height > 0))
				|| ((p->rect.top > 0) && ((*(pItemMgr->ppItemhead + (i + pItemMgr->itemNum - pItemMgr->itemNumPerRow) % pItemMgr->itemNum))->rect.top > p->rect.top)))
				&& (0 == p->rect.left))
			{
				if (p->rect.top <= -2)
				{
					pItemMgr->curItem = (i + pItemMgr->itemNumPerRow) % pItemMgr->itemNum;
				}
				else
				{
					pItemMgr->curItem = i;
				}
			}
		}
		else if (MOVETYPE_X == pItemMgr->moveType)
		{
			if ((((p->rect.left <= 0) && (p->rect.left + p->rect.width > 0))
				|| ((p->rect.left > 0) && ((*(pItemMgr->ppItemhead + (i + pItemMgr->itemNum - pItemMgr->itemNumPerCol) % pItemMgr->itemNum))->rect.left > p->rect.left)))
				&& (0 == p->rect.top))
			{
				if (p->rect.left <= -2)
				{
					pItemMgr->curItem = (i + pItemMgr->itemNumPerCol) % pItemMgr->itemNum;
				}
				else
				{
					pItemMgr->curItem = i;
				}
			}
		}
		
		p = p->pNext;
	}

	return AK_TRUE;
}

