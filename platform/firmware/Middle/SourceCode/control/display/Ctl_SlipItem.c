/**
 * @file Ctl_SlipItem.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */


#include "ctl_slipItem.h"
#include "akdefine.h"
#include "lib_res_port.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"
#include "eng_graph.h"
#include "fwl_graphic.h"
#include "Ctl_SlipMgr.h"

#define ITEM_FOCUS_BG_COLOR		(0x005096ff)
#define ITEM_FOCUS_TEXT_COLOR	(COLOR_WHITE)
#define ITEM_TEXT_COLOR			(COLOR_WHITE)


/****以下内部接口不要改为外部接口，若调用顺序错误会导致运行失败******/
static T_BOOL SlipItem_SetIconMode(T_SLIP_ITEM *pItem, T_pCDATA pIconLeft, T_pCDATA pIconRight);
static T_BOOL SlipItem_SetTextMode(T_SLIP_ITEM *pItem, const T_U16* pTextMain, const T_U16* pTextDown, const T_U16* pTextRight);
static T_BOOL SlipItem_CreatIcon(T_SLIP_ITEM *pItem, T_U32 *iconLwidth, T_U32 *iconRwidth);
static T_BOOL SlipItem_CreatText(T_SLIP_ITEM *pItem, T_U32 iconLwidth, T_U32 iconRwidth);
static T_BOOL SlipItem_CreatIconAndText(T_SLIP_ITEM *pItem);
static T_BOOL SlipItem_SetIcon(T_SLIP_ITEM *pItem, T_pCDATA pIconLeft, T_pCDATA pIconRight);
static T_BOOL SlipItem_SetText(T_SLIP_ITEM *pItem, const T_U16* pTextMain, const T_U16* pTextDown, const T_U16* pTextRight);
static T_BOOL SlipItem_ShowBg(T_SLIP_ITEM *pItem, T_BOOL bFocus);
static T_BOOL SlipItem_ShowIcon(T_SLIP_ITEM *pItem);
static T_BOOL SlipItem_ShowText(T_SLIP_ITEM *pItem, T_BOOL bFocus);
static T_BOOL SlipItem_ShowScrlText(T_SLIP_ITEM *pItem);



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
T_SLIP_ITEM *SlipItem_Creat(E_ITEMTYPE type, T_RECT rect)
{
	T_SLIP_ITEM *pItem = AK_NULL;
	//T_U32 len = 0;

	if (type >= ITEM_TYPE_NUM )
	{
		return AK_NULL;
	}
	
	pItem = (T_SLIP_ITEM *)Fwl_Malloc(sizeof(T_SLIP_ITEM));
	AK_ASSERT_PTR(pItem, "SlipItem_Creat(): pItem malloc error", AK_NULL);

	memset(pItem, 0, sizeof(T_SLIP_ITEM));

	pItem->type = type;
	pItem->rect = rect;

	pItem->bgColor = ITEM_BG_TRANS_COLOR;
	pItem->focusBgColor = ITEM_FOCUS_BG_COLOR;

	//pItem->pBgImg = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_SLIDEITEM_BG, &len);

	pItem->pbuf = Fwl_Malloc(pItem->rect.width * pItem->rect.height * COLOR_SIZE + 16);

	if (AK_NULL == pItem->pbuf)
	{
		AK_DEBUG_OUTPUT("SlipItem_Creat pItem->pbuf malloc error\n");
		SlipItem_Destroy(pItem);
		return AK_NULL;
	}

	memset(pItem->pbuf, 0, pItem->rect.width * pItem->rect.height * COLOR_SIZE + 16);

	return pItem;
}

/**
* @brief Destroy a Item control
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @return AK_NULL
* @retval
*/
T_VOID *SlipItem_Destroy(T_SLIP_ITEM *pItem)
{
	if (AK_NULL == pItem)
	{
		return AK_NULL;
	}


    pItem->pTextMain = Text_Destroy(pItem->pTextMain);
    pItem->pTextDownLine = Text_Destroy(pItem->pTextDownLine);
    pItem->pTextRight = Text_Destroy(pItem->pTextRight);

	pItem->pIconLeft = Icon_Destroy(pItem->pIconLeft);
	pItem->pIconRight = Icon_Destroy(pItem->pIconRight);


	if (AK_NULL != pItem->pbuf)
	{
		pItem->pbuf = Fwl_Free(pItem->pbuf);
	}

	pItem = Fwl_Free(pItem);
	
	return AK_NULL;
}

/**
* @brief Set Item id, icon and text
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in T_S32 id_of_item:the Item id
* @param in const T_pDATA pIconLeft:the left icon data
* @param in const T_pDATA pIconRight:the right icon data
* @param in const T_U16* pTextMain:the main text data
* @param in const T_U16* pTextDown:the down line text data
* @param in const T_U16* pTextRight:the right text data
* @return T_BOOL
* @retval
*/
T_BOOL SlipItem_SetItem(T_SLIP_ITEM *pItem, T_S32 id_of_item, T_pCDATA pIconLeft, T_pCDATA pIconRight, const T_U16* pTextMain, const T_U16* pTextDown, const T_U16* pTextRight)
{
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	pItem->id_of_item = id_of_item;

	//free old icon and old text first
	pItem->pIconLeft = Icon_Destroy(pItem->pIconLeft);
	pItem->pIconRight = Icon_Destroy(pItem->pIconRight);

	pItem->pTextMain = Text_Destroy(pItem->pTextMain);
    pItem->pTextDownLine = Text_Destroy(pItem->pTextDownLine);
    pItem->pTextRight = Text_Destroy(pItem->pTextRight);

	//设置顺序为:先设置icon和text模式，再创建icon，再创建text，最后再设置icon和text的数据
	//set icon and text mode
	SlipItem_SetIconMode(pItem, pIconLeft, pIconRight);

	//text的参数组合若有误，会导致模式设置失败，不要再往下走了
	if (!SlipItem_SetTextMode(pItem, pTextMain, pTextDown, pTextRight))
	{
		return AK_FALSE;
	}

	//create icon and text
	SlipItem_CreatIconAndText(pItem);

	//set icon data and text data
	SlipItem_SetIcon(pItem, pIconLeft, pIconRight);
	SlipItem_SetText(pItem, pTextMain, pTextDown, pTextRight);

	SlipItem_Show(pItem, AK_FALSE);
	
	return AK_TRUE;
}

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
T_BOOL SlipItem_Show(T_SLIP_ITEM *pItem, T_BOOL bFocus)
{
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	SlipItem_ShowBg(pItem, bFocus);

	SlipItem_ShowIcon(pItem);

	SlipItem_ShowText(pItem, bFocus);

	return AK_TRUE;
}


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
T_BOOL SlipItem_ScrollShow(T_SLIP_ITEM *pItem, T_BOOL bFocus)
{
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	SlipItem_ShowBg(pItem, bFocus);

	SlipItem_ShowIcon(pItem);

	return SlipItem_ShowScrlText(pItem);
}


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
T_BOOL SlipItem_ChangeTextRight(T_SLIP_ITEM *pItem, const T_U16* pTextRight)
{
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}
	
	Text_SetText(pItem->pTextRight, pTextRight);

	return AK_TRUE;    
}

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
T_BOOL SlipItem_GetItemBuf(T_SLIP_ITEM *pItem, T_U8 **pbuf, T_U32 *width, T_U32 *height)
{
	if ((AK_NULL == pItem) || (AK_NULL == pbuf) || (AK_NULL == width) || (AK_NULL == height))
	{
		return AK_FALSE;
	}

	*pbuf = pItem->pbuf;
	*width = pItem->rect.width;
	*height = pItem->rect.height;

	return AK_TRUE;
}

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
T_BOOL SlipItem_SetItemId(T_SLIP_ITEM *pItem, T_S32 id_of_item)
{
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	pItem->id_of_item = id_of_item;
	return AK_TRUE;
}



/**
* @brief Get Item backgroud img
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @return T_BOOL
* @retval
*/
T_BOOL SlipItem_LoadItemBgImg(T_SLIP_ITEM *pItem)
{
	T_U32 len = 0;
	
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	pItem->pBgImg = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_SLIDEITEM_BG, &len);
	return AK_TRUE;
}

/**
* @brief Set Item icon mode
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in const T_pDATA pIconLeft:the left icon data
* @param in const T_pDATA pIconRight:the right icon data
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_SetIconMode(T_SLIP_ITEM *pItem, T_pCDATA pIconLeft, T_pCDATA pIconRight)
{
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	if ((AK_NULL != pIconLeft) && (AK_NULL != pIconRight))
	{
		pItem->iconMode = ICON_ALL;
	}
	else if (AK_NULL != pIconLeft)
	{
		pItem->iconMode = ICON_LEFT_ONLY;
	}
	else if (AK_NULL != pIconRight)
	{
		pItem->iconMode = ICON_RIGHT_ONLY;
	}
	else
	{
		pItem->iconMode = ICON_NONE;
	}

	return AK_TRUE;
}

/**
* @brief Set Item text mode
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in const T_U16* pTextMain:the main text data
* @param in const T_U16* pTextDown:the down line text data
* @param in const T_U16* pTextRight:the right text data
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_SetTextMode(T_SLIP_ITEM *pItem, const T_U16* pTextMain, const T_U16* pTextDown, const T_U16* pTextRight)
{
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	if ((AK_NULL != pTextMain) && (AK_NULL != pTextDown) && (AK_NULL != pTextRight))
	{
		pItem->textMode = TEXT_ALL;
	}
	else if ((AK_NULL != pTextMain) && (AK_NULL != pTextDown))
	{
		pItem->textMode = TEXT_MAIN_AND_DOWNLINE;
	}
	else if ((AK_NULL != pTextMain) && (AK_NULL != pTextRight))
	{
		pItem->textMode = TEXT_MAIN_AND_RIGHT;
	}
	else if (AK_NULL != pTextMain)
	{
		pItem->textMode = TEXT_MAIN_ONLY;
	}
	else if ((AK_NULL == pTextMain) && (AK_NULL == pTextDown) && (AK_NULL == pTextRight))
	{
		pItem->textMode = TEXT_NONE;
	}
	else
	{
		AK_DEBUG_OUTPUT("SlipItem_SetTextMode error\n");
		return AK_FALSE;
	}

	return AK_TRUE;
}

/**
* @brief Creat Item icon
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param out T_U32 *iconLwidth:the width with interval of left icon
* @param out T_U32 *iconRwidth:the width with interval of right icon
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_CreatIcon(T_SLIP_ITEM *pItem, T_U32 *iconLwidth, T_U32 *iconRwidth)
{
	T_RECT rect;
	
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	switch (pItem->iconMode)
	{
		case ICON_LEFT_ONLY:		
			RectInit(&rect, 
				(T_POS)(X_INTERVAL), 
				(T_POS)(Y_INTERVAL), 
				SLIP_ITEM_ICON_WIDTH, SLIP_ITEM_ICON_HEIGHT);
			pItem->pIconLeft = Icon_Creat(rect, ALIGN_LEFT | ALIGN_VCENTER);
			
			if (AK_NULL != iconLwidth)
			{
				*iconLwidth = X_INTERVAL + SLIP_ITEM_ICON_WIDTH;
			}
			break;
			
		case ICON_RIGHT_ONLY:
			RectInit(&rect, 
				(T_POS)(pItem->rect.width-X_INTERVAL-SLIP_ITEM_ICON_WIDTH), 
				(T_POS)(Y_INTERVAL), 
				SLIP_ITEM_ICON_WIDTH, SLIP_ITEM_ICON_HEIGHT);
			pItem->pIconRight = Icon_Creat(rect, ALIGN_RIGHT | ALIGN_VCENTER);
			
			if (AK_NULL != iconRwidth)
			{
				*iconRwidth = X_INTERVAL + SLIP_ITEM_ICON_WIDTH;
			}
			break;
			
		case ICON_ALL:
			RectInit(&rect, 
				(T_POS)(X_INTERVAL), 
				(T_POS)(Y_INTERVAL), 
				SLIP_ITEM_ICON_WIDTH, SLIP_ITEM_ICON_HEIGHT);
			pItem->pIconLeft = Icon_Creat(rect, ALIGN_LEFT | ALIGN_VCENTER);
			
			RectInit(&rect, 
				(T_POS)(pItem->rect.width-X_INTERVAL-SLIP_ITEM_ICON_WIDTH), 
				(T_POS)(Y_INTERVAL), 
				SLIP_ITEM_ICON_WIDTH, SLIP_ITEM_ICON_HEIGHT);
			pItem->pIconRight = Icon_Creat(rect, ALIGN_RIGHT | ALIGN_VCENTER);
			
			if (AK_NULL != iconLwidth)
			{
				*iconLwidth = X_INTERVAL + SLIP_ITEM_ICON_WIDTH;
			}
			
			if (AK_NULL != iconRwidth)
			{
				*iconRwidth = X_INTERVAL + SLIP_ITEM_ICON_WIDTH;
			}
			break;
			
		default:
			break;
	}

	return AK_TRUE;
}

/**
* @brief Creat Item text
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in T_U32 iconLwidth:the width with interval of left icon
* @param in T_U32 iconRwidth:the width with interval of right icon
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_CreatText(T_SLIP_ITEM *pItem, T_U32 iconLwidth, T_U32 iconRwidth)
{
	T_RECT rect;
	
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}
	
	switch (pItem->textMode)
	{
		case TEXT_MAIN_ONLY:
			RectInit(&rect, 
					(T_POS)(iconLwidth + X_INTERVAL), 
					(T_POS)(+ Y_INTERVAL), 
					(T_LEN)(pItem->rect.width - (iconLwidth + 2*X_INTERVAL + iconRwidth)), 
					(T_LEN)(pItem->rect.height-2*Y_INTERVAL));
			pItem->pTextMain = Text_Creat(rect, ALIGN_LEFT | ALIGN_VCENTER);
			break;
			
		case TEXT_MAIN_AND_DOWNLINE:		
			RectInit(&rect, 
				(T_POS)(iconLwidth + X_INTERVAL), 
				(T_POS)(Y_INTERVAL), 
				(T_LEN)(pItem->rect.width - (iconLwidth + 2*X_INTERVAL + iconRwidth)), 
				(T_LEN)((pItem->rect.height-2*Y_INTERVAL) / 2));
			pItem->pTextMain = Text_Creat(rect, ALIGN_LEFT | ALIGN_VCENTER);
			
			RectInit(&rect, 
				(T_POS)(iconLwidth + X_INTERVAL), 
				(T_POS)(pItem->rect.height/ 2), 
				(T_LEN)(pItem->rect.width - (iconLwidth + 2*X_INTERVAL + iconRwidth)), 
				(T_LEN)((pItem->rect.height-2*Y_INTERVAL) / 2));
			pItem->pTextDownLine = Text_Creat(rect, ALIGN_LEFT | ALIGN_VCENTER);
			break;
			
		case TEXT_MAIN_AND_RIGHT:
			RectInit(&rect, 
				(T_POS)(iconLwidth + X_INTERVAL), 
				(T_POS)(Y_INTERVAL), 
				(T_LEN)((pItem->rect.width - (iconLwidth + 3*X_INTERVAL + iconRwidth))*3/4), 
				(T_LEN)(pItem->rect.height-2*Y_INTERVAL));
			pItem->pTextMain = Text_Creat(rect, ALIGN_LEFT | ALIGN_VCENTER);
			
			RectInit(&rect, 
				(T_POS)(iconLwidth + 2*X_INTERVAL + pItem->pTextMain->rect.width), 
				(T_POS)(Y_INTERVAL), 
				(T_LEN)((pItem->rect.width - (iconLwidth + 3*X_INTERVAL + iconRwidth))/4), 
				(T_LEN)(pItem->rect.height-2*Y_INTERVAL));
			pItem->pTextRight = Text_Creat(rect, ALIGN_RIGHT | ALIGN_VCENTER);
			break;
			
		case TEXT_ALL:
			RectInit(&rect, 
				(T_POS)(iconLwidth + X_INTERVAL), 
				(T_POS)(Y_INTERVAL), 
				(T_LEN)((pItem->rect.width - (iconLwidth + 3*X_INTERVAL + iconRwidth))*3/4), 
				(T_LEN)((pItem->rect.height-2*Y_INTERVAL) / 2));
			pItem->pTextMain = Text_Creat(rect, ALIGN_LEFT | ALIGN_VCENTER);
			
			RectInit(&rect, 
				(T_POS)(iconLwidth + X_INTERVAL), 
				(T_POS)(pItem->rect.height/ 2), 
				(T_LEN)((pItem->rect.width - (iconLwidth + 3*X_INTERVAL + iconRwidth))*3/4), 
				(T_LEN)((pItem->rect.height-2*Y_INTERVAL) / 2));
			pItem->pTextDownLine = Text_Creat(rect, ALIGN_LEFT | ALIGN_VCENTER);
			
			RectInit(&rect, 
				(T_POS)(iconLwidth + 2*X_INTERVAL + pItem->pTextMain->rect.width), 
				(T_POS)(Y_INTERVAL), 
				(T_LEN)((pItem->rect.width - (iconLwidth + 3*X_INTERVAL + iconRwidth))/4), 
				(T_LEN)(pItem->rect.height-2*Y_INTERVAL));
			pItem->pTextRight = Text_Creat(rect, ALIGN_RIGHT | ALIGN_VCENTER);
			break;
			
		default:
			break;
	}
	

	return AK_TRUE;
}

/**
* @brief Creat Item icon and text
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_CreatIconAndText(T_SLIP_ITEM *pItem)
{
	T_U32 iconLwidth = 0;
	T_U32 iconRwidth = 0;
	
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	//注意，先创建icon，再创建text
	SlipItem_CreatIcon(pItem, &iconLwidth, &iconRwidth);
	SlipItem_CreatText(pItem, iconLwidth, iconRwidth);

	return AK_TRUE;
}

/**
* @brief Set Item icon
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in const T_pDATA pIconLeft:the left icon data
* @param in const T_pDATA pIconRight:the right icon data
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_SetIcon(T_SLIP_ITEM *pItem, T_pCDATA pIconLeft, T_pCDATA pIconRight)
{
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	switch (pItem->iconMode)
	{
		case ICON_ALL:
			Icon_SetIcon(pItem->pIconLeft, pIconLeft);
			Icon_SetIcon(pItem->pIconRight, pIconRight);
			break;

		case ICON_LEFT_ONLY:
			Icon_SetIcon(pItem->pIconLeft, pIconLeft);
			break;

		case ICON_RIGHT_ONLY:
			Icon_SetIcon(pItem->pIconRight, pIconRight);
			break;

		default:
			break;
	}

	return AK_TRUE;
}

/**
* @brief Set Item text
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in const T_U16* pTextMain:the main text data
* @param in const T_U16* pTextDown:the down line text data
* @param in const T_U16* pTextRight:the right text data
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_SetText(T_SLIP_ITEM *pItem, const T_U16* pTextMain, const T_U16* pTextDown, const T_U16* pTextRight)
{
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	switch (pItem->textMode)
	{
		case TEXT_ALL:
			Text_SetText(pItem->pTextMain, pTextMain);
			Text_SetText(pItem->pTextDownLine, pTextDown);
			Text_SetText(pItem->pTextRight, pTextRight);
			break;

		case TEXT_MAIN_ONLY:
			Text_SetText(pItem->pTextMain, pTextMain);
			break;

		case TEXT_MAIN_AND_DOWNLINE:
			Text_SetText(pItem->pTextMain, pTextMain);
			Text_SetText(pItem->pTextDownLine, pTextDown);
			break;

		case TEXT_MAIN_AND_RIGHT:
			Text_SetText(pItem->pTextMain, pTextMain);
			Text_SetText(pItem->pTextRight, pTextRight);
			break;

		default:
			break;
	}
	
	return AK_TRUE;
}


/**
* @brief Show item backgroud to its buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in T_BOOL bFocus:the item is focus or not
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_ShowBg(T_SLIP_ITEM *pItem, T_BOOL bFocus)
{
	T_RECT rect;
	
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	RectInit(&rect, 0, 0, pItem->rect.width, pItem->rect.height);
	
	if (bFocus)
	{
		if (AK_NULL == pItem->pFocusBgImg)
		{
			Fwl_FillSolidRectOnRGB(pItem->pbuf, pItem->rect.width, pItem->rect.height, &rect, pItem->focusBgColor, RGB565);
		}
		else
		{
			Fwl_AkBmpDrawPartFromStringOnRGB(pItem->pbuf, pItem->rect.width, pItem->rect.height, 0, 0,
									&rect, pItem->pFocusBgImg, &g_Graph.TransColor, AK_FALSE, RGB565);
		}
		
	}
	else
	{
		if (AK_NULL == pItem->pBgImg)
		{
			Fwl_FillSolidRectOnRGB(pItem->pbuf, pItem->rect.width, pItem->rect.height, &rect, pItem->bgColor, RGB565);
		}
		else
		{
			RectInit(&rect, 0, 0, pItem->rect.width, pItem->rect.height);
			Fwl_AkBmpDrawPartFromStringOnRGB(pItem->pbuf, pItem->rect.width, pItem->rect.height, 0, 0,
									&rect, pItem->pBgImg, &g_Graph.TransColor, AK_FALSE, RGB565);
		}
		
	}

	return AK_TRUE;
}

/**
* @brief Show item icon to its buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_ShowIcon(T_SLIP_ITEM *pItem)
{
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	if (AK_NULL != pItem->pIconLeft)
	{
		Icon_Show(pItem->pIconLeft, pItem->pbuf, pItem->rect.width, pItem->rect.height);
	}

	if (AK_NULL != pItem->pIconRight)
	{
		Icon_Show(pItem->pIconRight, pItem->pbuf, pItem->rect.width, pItem->rect.height);
	}

	return AK_TRUE;
}

/**
* @brief Show item text to its buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @param in T_BOOL bFocus:the item is focus or not
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_ShowText(T_SLIP_ITEM *pItem, T_BOOL bFocus)
{
	T_COLOR	color = 0;
	
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	if (bFocus)
	{
		color = ITEM_FOCUS_TEXT_COLOR;
	}
	else
	{
		color = ITEM_TEXT_COLOR;
	}

	if (AK_NULL != pItem->pTextMain)
	{
		Text_Show(pItem->pTextMain, pItem->pbuf, pItem->rect.width, pItem->rect.height, color);
	}

	if (AK_NULL != pItem->pTextDownLine)
	{
		Text_Show(pItem->pTextDownLine, pItem->pbuf, pItem->rect.width, pItem->rect.height, color);
	}

	if (AK_NULL != pItem->pTextRight)
	{
		Text_Show(pItem->pTextRight, pItem->pbuf, pItem->rect.width, pItem->rect.height, color);
	}

	return AK_TRUE;
}


/**
* @brief Show item text to its buf
*
* @author Songmengxing
* @date 2011-8-23
* @param in T_SLIP_ITEM *pItem:the Item handle
* @return T_BOOL
* @retval
*/
static T_BOOL SlipItem_ShowScrlText(T_SLIP_ITEM *pItem)
{
	T_BOOL ret = AK_FALSE;
	T_COLOR	color = ITEM_FOCUS_TEXT_COLOR;
	
	if (AK_NULL == pItem)
	{
		return AK_FALSE;
	}

	if (AK_NULL != pItem->pTextMain)
	{
		ret = Text_ScrollShow(pItem->pTextMain, pItem->pbuf, pItem->rect.width, pItem->rect.height, color);
	}

	if (AK_NULL != pItem->pTextDownLine)
	{
		ret |= Text_ScrollShow(pItem->pTextDownLine, pItem->pbuf, pItem->rect.width, pItem->rect.height, color);
	}

	if (AK_NULL != pItem->pTextRight)
	{
		ret |= Text_ScrollShow(pItem->pTextRight, pItem->pbuf, pItem->rect.width, pItem->rect.height, color);
	}

	return ret;
}


