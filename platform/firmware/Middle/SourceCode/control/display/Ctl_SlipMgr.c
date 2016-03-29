/**
 * @file Ctl_SlipMgr.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */


#include "fwl_oscom.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "Eng_topbar.h"
#include "fwl_pfdisplay.h"
#include "Lib_event.h"
#include "Fwl_tscrcom.h"
#include "ctl_SlipMgr.h"
#include "Ctl_SlipCalc.h"
#include "Ctl_SlipScrb.h"
#include "Ctl_SlipItemMgr.h"
#include "Ctl_SlipMsg.h"

static T_BOOL SlipMgr_StartTimer(T_SLIPMGR *pSlipMgr);
static T_BOOL SlipMgr_StopTimer(T_SLIPMGR *pSlipMgr);
static T_BOOL SlipMgr_SlipComplete(T_SLIPMGR *pSlipMgr, T_U32 *focusId);
static T_BOOL SlipMgr_SlipStart(T_SLIPMGR *pSlipMgr);
static T_BOOL SlipMgr_StatusMgr(T_SLIPMGR *pSlipMgr, T_BOOL *bSlip, T_S32 overLen, T_S32 *count, T_U32 *loadItemNum);
static T_BOOL SlipMgr_TouchScrHandle(T_SLIPMGR *pSlipMgr, T_EVT_PARAM *pEventParm, T_U32 *focusId);
static T_BOOL SlipMgr_UserKeyHandle(T_SLIPMGR *pSlipMgr, T_EVT_PARAM *pEventParm, T_U32 *focusId, T_S32 overLen, T_S32 *count, T_U32 *loadItemNum, T_U32 emptyNum);
static T_BOOL SlipMgr_ShowItems(T_SLIPMGR *pSlipMgr);


/**
* @brief Creat a slip manager control
*
* @author Songmengxing
* @date 2011-8-23
* @param in E_ITEMTYPE itemtype:ITEM_TYPE_LIST or ITEM_TYPE_IMAGE
* @param in T_RECT rect: rect
* @param in T_U32 itemW: item width
* @param in T_U32 itemH: item height
* @param in T_U32 totalItemNum: total item num
* @param in E_MOVETYPE movetype:MOVETYPE_X or MOVETYPE_Y
* @return T_SLIPMGR * the SlipMgr handle
* @retval
*/
T_SLIPMGR *SlipMgr_Creat(E_ITEMTYPE itemtype, T_RECT rect, T_U32 itemW, T_U32 itemH, T_U32 totalItemNum, E_MOVETYPE movetype)
{
	T_SLIPMGR *pSlipMgr = AK_NULL;
	T_RECT scrb_rect;

	if ((movetype > MOVETYPE_Y) || (itemtype >= ITEM_TYPE_NUM ))
	{
		return AK_NULL;
	}

	pSlipMgr = (T_SLIPMGR *)Fwl_Malloc(sizeof(T_SLIPMGR));
	AK_ASSERT_PTR(pSlipMgr, "SlipMgr_Creat(): pSlipMgr malloc error", AK_NULL);

	memset(pSlipMgr, 0, sizeof(T_SLIPMGR));

	pSlipMgr->refreshTimer = ERROR_TIMER;
	pSlipMgr->rect = rect;
	pSlipMgr->itemType = itemtype;
	pSlipMgr->moveType = movetype;

	pSlipMgr->pItemMgr = SlipItemMgr_Creat(pSlipMgr->itemType, pSlipMgr->rect.width, pSlipMgr->rect.height, itemW, itemH, totalItemNum, pSlipMgr->moveType);
	if (AK_NULL == pSlipMgr->pItemMgr)
	{
		AK_DEBUG_OUTPUT("SlipMgr_Creat SlipItemMgr_Creat error\n");
		SlipMgr_Destroy(pSlipMgr);
		return AK_NULL;
	}

	pSlipMgr->pMsg = SlipMsg_Creat(pSlipMgr->moveType);
	if (AK_NULL == pSlipMgr->pMsg)
	{
		AK_DEBUG_OUTPUT("SlipMgr_Creat SlipMsg_Creat error\n");
		SlipMgr_Destroy(pSlipMgr);
		return AK_NULL;
	}

	pSlipMgr->pCalc = SlipCalc_Creat(pSlipMgr->moveType);
	if (AK_NULL == pSlipMgr->pCalc)
	{
		AK_DEBUG_OUTPUT("SlipMgr_Creat SlipCalc_Creat error\n");
		SlipMgr_Destroy(pSlipMgr);
		return AK_NULL;
	}

	if (MOVETYPE_Y == pSlipMgr->moveType)
	{
		RectInit(&scrb_rect, (T_POS)(pSlipMgr->rect.width - SCROLL_WIDTH - 1), pSlipMgr->rect.top, SCROLL_WIDTH, pSlipMgr->rect.height);
		pSlipMgr->pScrb = SlipScrb_Creat(scrb_rect, pSlipMgr->moveType);
		SlipScrb_SetMaxSize(pSlipMgr->pScrb, itemH * totalItemNum);
	}
	
	return pSlipMgr;
}

/**
* @brief Destroy a slip manager control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipMgr_Destroy(T_SLIPMGR *pSlipMgr)
{
	if (AK_NULL == pSlipMgr)
	{
		return AK_NULL;
	}

	if (AK_NULL != pSlipMgr->pItemMgr)
	{
		pSlipMgr->pItemMgr = SlipItemMgr_Destroy(pSlipMgr->pItemMgr);
	}

	if (AK_NULL != pSlipMgr->pCalc)
	{
		pSlipMgr->pCalc = SlipCalc_Destroy(pSlipMgr->pCalc);
	}

	if (AK_NULL != pSlipMgr->pMsg)
	{
		pSlipMgr->pMsg = SlipMsg_Destroy(pSlipMgr->pMsg);
	}

	if (AK_NULL != pSlipMgr->pScrb)
	{
		pSlipMgr->pScrb = SlipScrb_Destroy(pSlipMgr->pScrb);
	}

	SlipMgr_StopTimer(pSlipMgr);

	pSlipMgr = Fwl_Free(pSlipMgr);

	return AK_NULL;
}

/**
* @brief get Item index by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_S32 id:the Item id
* @return T_S32
* @retval >=0 : index; <0 : error
*/
T_S32 SlipMgr_GetIndexById(T_SLIPMGR *pSlipMgr, T_S32 id)
{
	if(AK_NULL == pSlipMgr)
    {
        return -1;
    }

	return SlipItemMgr_GetIndexById(pSlipMgr->pItemMgr, id);
}

/**
* @brief get item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return T_U32
* @retval 
*/
T_U32 SlipMgr_GetItemNum(T_SLIPMGR *pSlipMgr)
{
	if(AK_NULL == pSlipMgr)
    {
        return 0;
    }

	return SlipItemMgr_GetItemNum(pSlipMgr->pItemMgr);
}

/**
* @brief Set Item id, icon and text by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
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
T_BOOL SlipMgr_SetItem(T_SLIPMGR *pSlipMgr, T_U32 index, T_S32 id_of_item,
							T_pCDATA pIconLeft, T_pCDATA pIconRight, 
							const T_U16* pTextMain, const T_U16* pTextDown, const T_U16* pTextRight)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	return SlipItemMgr_SetItem(pSlipMgr->pItemMgr, index, id_of_item, pIconLeft, pIconRight, pTextMain, pTextDown, pTextRight);
}

/**
* @brief Set Item id by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 index:the Item index in mgr
* @param in T_S32 id_of_item:the Item id
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_SetItemId(T_SLIPMGR *pSlipMgr, T_U32 index, T_S32 id_of_item)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	return SlipItemMgr_SetItemId(pSlipMgr->pItemMgr, index, id_of_item);
}

/**
* @brief Change Item right text  by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_S32 id:the Item id
* @param in const T_U16* pTextRight:the new right text data
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_ChangeTextRightById(T_SLIPMGR *pSlipMgr, T_S32 id, const T_U16* pTextRight)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	return SlipItemMgr_ChangeTextRightById(pSlipMgr->pItemMgr, id, pTextRight);
}

/**
* @brief show Item by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_S32 id:the Item id
* @param in T_BOOL bFocus:it is focus or not
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_ShowItemById(T_SLIPMGR *pSlipMgr, T_S32 id, T_BOOL bFocus)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	return SlipItemMgr_ShowItemById(pSlipMgr->pItemMgr, id, bFocus);
}

/**
* @brief scroll show Item by id
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_S32 id:the Item id
* @param in T_BOOL bFocus:it is focus or not
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_ScrollShowItemById(T_SLIPMGR *pSlipMgr, T_S32 id, T_BOOL bFocus)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	return SlipItemMgr_ScrollShowItemById(pSlipMgr->pItemMgr, id, bFocus);
}



/**
* @brief clean Item focus
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr:the SlipMgr handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_CleanItemFocus(T_SLIPMGR *pSlipMgr)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	return SlipItemMgr_CleanItemFocus(pSlipMgr->pItemMgr);
}

/**
* @brief get item buf by index
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param inT_U32 index: item index
* @param out T_U8 **pbuf: buf
* @param out T_U32 *width: width of buf
* @param out T_U32 *height: height of buf
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_GetItemBufByIndex(T_SLIPMGR *pSlipMgr, T_U32 index, T_U8 **pbuf, T_U32 *width, T_U32 *height)
{
	if((AK_NULL == pSlipMgr) || (AK_NULL == pbuf) || (AK_NULL == width) || (AK_NULL == height))
    {
        return AK_FALSE;
    }

	return SlipItemMgr_GetItemBufByIndex(pSlipMgr->pItemMgr, index, pbuf, width, height);
}

/**
* @brief add loaded item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_S32 count: add count
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_AddLoadItemNum(T_SLIPMGR *pSlipMgr, T_S32 count)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }
	
	return SlipItemMgr_AddLoadItemNum(pSlipMgr->pItemMgr, count);
}

/**
* @brief set loaded item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 num: num
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_SetLoadItemNum(T_SLIPMGR *pSlipMgr, T_U32 num)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }
	
	return SlipItemMgr_SetLoadItemNum(pSlipMgr->pItemMgr, num);
}

/**
* @brief set total item num
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 num: total num
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_SetTotalItemNum(T_SLIPMGR *pSlipMgr, T_U32 totalnum)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }
	
	return SlipItemMgr_SetTotalItemNum(pSlipMgr->pItemMgr, totalnum);
}


/**
* @brief set background color
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_COLOR color: color
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_SetBgColor(T_SLIPMGR *pSlipMgr, T_COLOR color)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }
	
	return SlipItemMgr_SetBgColor(pSlipMgr->pItemMgr, color);
}



/**
* @brief refresh
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_Refresh(T_SLIPMGR *pSlipMgr)
{
	T_U8	*pbuf = AK_NULL;
	T_U8	*pBuffer = AK_NULL;
	T_U8    *pData = AK_NULL;
	T_U32	width = 0;
	T_U32	height = 0;
	T_U32	i = 0;
	E_SLIPMSG_STA cursta = SLIPMSG_STA_STOP;
	
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	SlipMgr_ShowItems(pSlipMgr);
	SlipItemMgr_GetDisBuf(pSlipMgr->pItemMgr, &pbuf, &width, &height);

	cursta = SlipMsg_GetCurStatus(pSlipMgr->pMsg);

	if ((SLIPMSG_STA_MOVE == cursta) || (SLIPMSG_STA_SLIP == cursta))
	{
		SlipScrb_Show(pSlipMgr->pScrb, pbuf, width, height);
	}

	pBuffer = Fwl_GetDispMemory() + (Fwl_GetLcdWidth()*pSlipMgr->rect.top + pSlipMgr->rect.left) * COLOR_SIZE; 
	pData = pbuf;

    for(i = 0; i < height; i++)
    {
        memcpy(pBuffer, pData, width * COLOR_SIZE);
        pData += width * COLOR_SIZE;
        pBuffer += Fwl_GetLcdWidth() * COLOR_SIZE;
    }

	return AK_TRUE;
}

/**
* @brief get display buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param out T_U8 **pbuf: buf
* @param out T_U32 *width: width of buf
* @param out T_U32 *height: height of buf
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_GetDisBuf(T_SLIPMGR *pSlipMgr, T_U8 **pbuf, T_U32 *width, T_U32 *height)
{
	T_U8	*pbuftmp = AK_NULL;
	T_U32	widthtmp = 0;
	T_U32	heighttmp = 0;
	
	if ((AK_NULL == pSlipMgr) || (AK_NULL == pbuf) || (AK_NULL == width) || (AK_NULL == height))
	{
		return AK_FALSE;
	}

	SlipMgr_ShowItems(pSlipMgr);
	SlipItemMgr_GetDisBuf(pSlipMgr->pItemMgr, &pbuftmp, &widthtmp, &heighttmp);

	*pbuf = pbuftmp;
	*width = widthtmp;
	*height = heighttmp;

	return AK_TRUE;
}


/**
* @brief prepare to show
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param out  T_S32 *count: count of item need to load
* @param out T_U32 *loadItemNum: loaded item num
* @param in T_S32 offset: offset
* @param in T_U32 emptyNum: empty item num
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_PrepareToShow(T_SLIPMGR *pSlipMgr, T_S32 *count, T_U32 *loadItemNum, T_S32 offset, T_U32 emptyNum)
{
	T_S32 curPos = 0;
	T_U32 storeLen = 0;
	T_U32 itemH = 0;
	T_U32 totalHeight = 0;
	T_S32 overLen = 0;
	T_U32 totalItemNum = 0;
	T_U32 itemNumPerRow = 0;
	T_U32 totalCol = 0;
	
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	if((AK_NULL == count) || (AK_NULL == loadItemNum))
    {
        return AK_FALSE;
    }

	totalItemNum = SlipItemMgr_GetTotalItemNum(pSlipMgr->pItemMgr);

	if (0 == totalItemNum)
	{
		return AK_TRUE;
	}
	
	if (0 != offset)
	{
		*count = SlipItemMgr_AddOffset(pSlipMgr->pItemMgr, offset, emptyNum);
	}
	else
	{
		*count = 0;
	}
	
	*loadItemNum = SlipItemMgr_GetLoadItemNum(pSlipMgr->pItemMgr);

	
	storeLen = SlipItemMgr_GetTotalStoreLen(pSlipMgr->pItemMgr);

	itemH = SlipItemMgr_GetItemHeight(pSlipMgr->pItemMgr);
	itemNumPerRow = SlipItemMgr_GetItemNumPerRow(pSlipMgr->pItemMgr);

	if (0 == (*loadItemNum + *count) % itemNumPerRow)
	{
		curPos = (T_S32)((*loadItemNum + *count) / itemNumPerRow * itemH - storeLen - pSlipMgr->rect.height);
	}
	else
	{
		curPos = (T_S32)(((*loadItemNum + *count) / itemNumPerRow + 1) * itemH - storeLen - pSlipMgr->rect.height);
	}
	
	overLen = SlipItemMgr_GetOverLen(pSlipMgr->pItemMgr);

	if (0 == totalItemNum % itemNumPerRow)
	{
		totalCol = totalItemNum / itemNumPerRow;
	}
	else
	{
		totalCol = totalItemNum / itemNumPerRow + 1;
	}
	
	if (overLen >= 0)
	{
		totalHeight = itemH * totalCol + overLen;
		curPos += overLen;
	}
	else
	{
		totalHeight = itemH * totalCol - overLen;
		curPos -= overLen;
	}

	if (curPos >= 0)
	{
		SlipScrb_SetCurPos(pSlipMgr->pScrb, (T_POS)(curPos * pSlipMgr->rect.height / totalHeight));
	}
	
	SlipScrb_SetMaxSize(pSlipMgr->pScrb, totalHeight);
	
	return AK_TRUE;
}

/**
* @brief check focus item is in show rect or not
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 focusId: focus item id
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_CheckFocusItem(T_SLIPMGR *pSlipMgr, T_U32 focusId)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	return SlipItemMgr_CheckFocusItem(pSlipMgr->pItemMgr, focusId);
}

/**
* @brief clean offset
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_U32 num: 对应实际item链表已经加载上的item数量
* @return T_BOOL
* @retval 
*/
T_BOOL SlipMgr_ClearOffset(T_SLIPMGR *pSlipMgr, T_U32 num)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	return SlipItemMgr_ClearOffset(pSlipMgr->pItemMgr, num);
}


/**
* @brief handle function
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_EVT_CODE event: event
* @param in T_EVT_PARAM *pEventParm: pEventParm
* @param out  T_S32 *count: count of item need to load
* @param out T_U32 *loadItemNum: loaded item num
* @param in/out T_U32 *focusId: focus item id
* @param in T_U32 emptyNum: empty item num
* @return T_BOOL
* @retval 
*/
T_eBACK_STATE SlipMgr_Handle(T_SLIPMGR *pSlipMgr, T_EVT_CODE event, T_EVT_PARAM *pEventParm,
									T_S32 *count, T_U32 *loadItemNum, T_U32 *focusId, T_U32 emptyNum)
{
	T_eBACK_STATE ret = eStay;
	T_S32 offset = 0;
	T_BOOL bSlip = AK_FALSE;
	T_U8 SlipOK = 0;
	T_S32 overLen = 0;
	
	
	if(AK_NULL == pSlipMgr)
    {
        return ret;
    }

	if((AK_NULL == count) || (AK_NULL == loadItemNum) || (AK_NULL == pEventParm) || (AK_NULL == focusId))
    {
        return ret;
    }

	if ((M_EVT_TOUCH_SCREEN == event) && (eTOUCHSCR_DOWN == pEventParm->s.Param1)
		&& (!PointInRect(&(pSlipMgr->rect), pEventParm->s.Param2, pEventParm->s.Param3)))
	{
		return ret;
	}

	if (M_EVT_TOUCH_SCREEN == event)
	{
		SlipMgr_TouchScrHandle(pSlipMgr, pEventParm, focusId);
	}

	ret = SlipMsg_Handle(pSlipMgr->pMsg, event, pEventParm, pSlipMgr->refreshTimer);

	if (eStay != ret)
	{
		return ret;
	}

	overLen = SlipItemMgr_GetOverLen(pSlipMgr->pItemMgr);

	SlipMgr_StatusMgr(pSlipMgr, &bSlip, overLen, count, loadItemNum);

	if (M_EVT_USER_KEY == event)
	{
		SlipMgr_UserKeyHandle(pSlipMgr, pEventParm, focusId, overLen, count, loadItemNum, emptyNum);
	}
	else
	{
		offset = SlipCalc_Handle(pSlipMgr->pCalc, event, pEventParm, pSlipMgr->refreshTimer, bSlip, &SlipOK, overLen, pSlipMgr->rect);
		SlipMgr_PrepareToShow(pSlipMgr, count, loadItemNum, offset, emptyNum);

		if (1 == SlipOK)
		{
			SlipMgr_SlipComplete(pSlipMgr, focusId);
		}
	}
	
	return ret;
}


/**
* @brief Get Cur Status
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return E_SLIPMSG_STA
* @retval
*/
E_SLIPMSG_STA SlipMgr_GetCurStatus(T_SLIPMGR *pSlipMgr)
{
	if (AK_NULL == pSlipMgr)
	{
		return SLIPMSG_STA_STOP;
	}

	return SlipMsg_GetCurStatus(pSlipMgr->pMsg);
}

/**
* @brief Get Item backgroud img
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr:the SlipMgr handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipMgr_LoadItemBgImg(T_SLIPMGR *pSlipMgr)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	return SlipItemMgr_LoadItemBgImg(pSlipMgr->pItemMgr);
}


/**
* @brief start timer
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return T_BOOL
* @retval
*/
static T_BOOL SlipMgr_StartTimer(T_SLIPMGR *pSlipMgr)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }
	
	if (ERROR_TIMER == pSlipMgr->refreshTimer)
	{
		pSlipMgr->refreshTimer = Fwl_SetTimerMilliSecond(SLIP_REFRESH_TIME, AK_TRUE);
	}
	
	return AK_TRUE;
}

/**
* @brief stop timer
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return T_BOOL
* @retval
*/
static T_BOOL SlipMgr_StopTimer(T_SLIPMGR *pSlipMgr)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	if (ERROR_TIMER != pSlipMgr->refreshTimer)
	{
		Fwl_StopTimer(pSlipMgr->refreshTimer);
		pSlipMgr->refreshTimer = ERROR_TIMER;
	}

	return AK_TRUE;
}

/**
* @brief fill the show buf with item buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipMgr_ShowItems(T_SLIPMGR *pSlipMgr)
{
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	return SlipItemMgr_FillBuf(pSlipMgr->pItemMgr);
}

/**
* @brief  user key handle function
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_EVT_PARAM *pEventParm: pEventParm
* @param out  T_S32 *count: count of item need to load
* @param out T_U32 *loadItemNum: loaded item num
* @param in/out T_U32 *focusId: focus item id
* @param in T_S32 overLen: over len
* @param in T_U32 emptyNum: empty item num
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipMgr_UserKeyHandle(T_SLIPMGR *pSlipMgr, T_EVT_PARAM *pEventParm, T_U32 *focusId, T_S32 overLen, T_S32 *count, T_U32 *loadItemNum, T_U32 emptyNum)
{
	T_S32 nextRemainLen = 0;
	T_S32 preRemainLen = 0;
	T_U32 length = 0;
	T_U32 maxLen = 0;
	
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }
	
	if((AK_NULL == pEventParm) || (AK_NULL == focusId))
    {
        return AK_FALSE;
    }

	if (SLIPMSG_STA_SLIP == SlipMsg_GetCurStatus(pSlipMgr->pMsg))
	{
		if (0 != overLen)
		{
			SlipMgr_PrepareToShow(pSlipMgr, count, loadItemNum, 0 - overLen, 0);
		}

		SlipMgr_SlipComplete(pSlipMgr, focusId);
	}
	
	if ((!SlipItemMgr_CheckFocusItem(pSlipMgr->pItemMgr, *focusId))
			&& (SLIPMSG_STA_STOP == SlipMsg_GetCurStatus(pSlipMgr->pMsg)))
	{
		SlipItemMgr_GetRemainLen(pSlipMgr->pItemMgr, &nextRemainLen, &preRemainLen);
		SlipCalc_SetMoveDirectionByKey(pSlipMgr->pCalc, (T_eKEY_ID) pEventParm->c.Param1);
		SlipCalc_SetRemainLen(pSlipMgr->pCalc, nextRemainLen, preRemainLen);
		maxLen = SlipCalc_GetRemainLen(pSlipMgr->pCalc);


		if (MOVETYPE_Y == pSlipMgr->moveType)
		{
			length = SlipItemMgr_GetItemHeight(pSlipMgr->pItemMgr);
		}
		else if (MOVETYPE_X == pSlipMgr->moveType)
		{
			length = SlipItemMgr_GetItemWidth(pSlipMgr->pItemMgr);
		}

		if (length > maxLen)
		{
			length = maxLen;
		}

		if ((kbLEFT == pEventParm->c.Param1) || (kbUP == pEventParm->c.Param1))
		{
			SlipMgr_PrepareToShow(pSlipMgr, count, loadItemNum, length, emptyNum);
		}
		else
		{
			SlipMgr_PrepareToShow(pSlipMgr, count, loadItemNum, 0 - length, emptyNum);
		}
	}

	return AK_TRUE;
}

/**
* @brief touch screen handle function
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in T_EVT_PARAM *pEventParm: pEventParm
* @param in/out T_U32 *focusId: focus item id
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipMgr_TouchScrHandle(T_SLIPMGR *pSlipMgr, T_EVT_PARAM *pEventParm, T_U32 *focusId)
{
	T_S32 temp = -1;
	
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	if((AK_NULL == pEventParm) || (AK_NULL == focusId))
    {
        return AK_FALSE;
    }
	
	if (eTOUCHSCR_DOWN == pEventParm->s.Param1)
	{
		temp = SlipItemMgr_GetItemIdByPoint(pSlipMgr->pItemMgr, (T_POS)(pEventParm->s.Param2-pSlipMgr->rect.left), (T_POS)(pEventParm->s.Param3-pSlipMgr->rect.top));

		if (-1 != temp)
		{
			if (*focusId == (T_U32)temp)
			{
				SlipMsg_SetFocusFlag(pSlipMgr->pMsg, SLIPMSG_FOCUS_ITEM);
			}
			else
			{
				SlipMsg_SetFocusFlag(pSlipMgr->pMsg, SLIPMSG_UNFOCUS_ITEM);
				*focusId = temp;
			}
		}
		else
		{
			SlipMsg_SetFocusFlag(pSlipMgr->pMsg, SLIPMSG_NOT_ITEM);
		}
	}

	return AK_TRUE;
}

/**
* @brief slip complete
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param in/out T_U32 *focusId: focus item id
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipMgr_SlipComplete(T_SLIPMGR *pSlipMgr, T_U32 *focusId)
{
	T_SLIPMSG_STRC msg;
	T_S32 focusIdtmp = -1;
	
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	if(AK_NULL == focusId)
    {
        return AK_FALSE;
    }
	
	SlipMgr_StopTimer(pSlipMgr);
		
	if (!SlipItemMgr_CheckFocusItem(pSlipMgr->pItemMgr, *focusId))
	{
		focusIdtmp = SlipItemMgr_GetCurItemId(pSlipMgr->pItemMgr);

		if (-1 != focusIdtmp)
		{
			*focusId = focusIdtmp;
		}
	}
	
	msg.type = SLIPMSG_SLIPOK;
	msg.param1 = 0;
	msg.param2 = 0;
	msg.param3 = 0;
	msg.param4 = 0;
	
	SlipMsg_Send(pSlipMgr->pMsg, msg);

	return AK_TRUE;
}

/**
* @brief slip start
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipMgr_SlipStart(T_SLIPMGR *pSlipMgr)
{
	T_S32 overLen = 0;
	T_S32 nextRemainLen = 0;
	T_S32 preRemainLen = 0;
	E_MOVE_DIRECTION moveDirection;
	
	
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }
	
	overLen = SlipItemMgr_GetOverLen(pSlipMgr->pItemMgr);
			
	if (0 != overLen)
	{
		SlipCalc_SetV0AndAByLen(pSlipMgr->pCalc, overLen);
		moveDirection = SlipCalc_GetMoveDirection(pSlipMgr->pCalc);
		
		if (overLen > 0)
		{
			if ((SLIPCALC_MOVE_DOWN == moveDirection) || (SLIPCALC_MOVE_RIGHT == moveDirection))
			{
				SlipCalc_Rebound(pSlipMgr->pCalc);
			}
			
			SlipCalc_SetRemainLen(pSlipMgr->pCalc, overLen, 0);
		}
		else
		{
			if ((SLIPCALC_MOVE_UP == moveDirection) || (SLIPCALC_MOVE_LEFT == moveDirection))
			{
				SlipCalc_Rebound(pSlipMgr->pCalc);
			}
			
			SlipCalc_SetRemainLen(pSlipMgr->pCalc, 0, 0 - overLen);
		}
	}
	else
	{
		SlipItemMgr_GetRemainLen(pSlipMgr->pItemMgr, &nextRemainLen, &preRemainLen);
		SlipCalc_SetRemainLen(pSlipMgr->pCalc, nextRemainLen, preRemainLen);
	}

	return AK_TRUE;
}

/**
* @brief handle function
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIPMGR *pSlipMgr: the SlipMgr handle
* @param out T_BOOL *bSlip: is slip or not
* @param in T_S32 overLen: over len
* @param out  T_S32 *count: count of item need to load
* @param out T_U32 *loadItemNum: loaded item num
* @return T_BOOL
* @retval 
*/
static T_BOOL SlipMgr_StatusMgr(T_SLIPMGR *pSlipMgr, T_BOOL *bSlip, T_S32 overLen, T_S32 *count, T_U32 *loadItemNum)
{
	E_SLIPMSG_STA cursta = SLIPMSG_STA_STOP;
	E_SLIPMSG_STA laststa = SLIPMSG_STA_STOP;
	
	if(AK_NULL == pSlipMgr)
    {
        return AK_FALSE;
    }

	if(AK_NULL == bSlip)
    {
        return AK_FALSE;
    }
	
	cursta = SlipMsg_GetCurStatus(pSlipMgr->pMsg);
	laststa = SlipMsg_GetLastStatus(pSlipMgr->pMsg);

	if (SLIPMSG_STA_MOVE == cursta)
	{
		if (SLIPMSG_STA_MOVE != laststa)
		{
			SlipMgr_StartTimer(pSlipMgr);			
		}

		if (SLIPMSG_STA_SLIP == laststa)
		{
			if (0 != overLen)
			{
				SlipMgr_PrepareToShow(pSlipMgr, count, loadItemNum, 0 - overLen, 0);
			}
		}

		*bSlip = AK_FALSE;	
	}
	else if (SLIPMSG_STA_SLIP == cursta)
	{
		if (SLIPMSG_STA_SLIP != laststa)
		{
			SlipMgr_SlipStart(pSlipMgr);
		}

		*bSlip = AK_TRUE;	
	}

	return AK_TRUE;
}

