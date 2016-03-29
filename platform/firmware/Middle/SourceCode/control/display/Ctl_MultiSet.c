
/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  * @File name: Ctl_MultiSet.c
  * @Function: This file is the implement of the control Ctl_MultiSet.
  * @Author: WangWei
  * @Date: 2008-05-04   
  * @Version: 1.0
  */

#include "Ctl_MultiSet.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_osMalloc.h"
#include "Ctl_MsgBox.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Eng_KeyMapping.h"
#include "Eng_Time.h"
#include "Eng_AkBmp.h"
#include "Eng_GblString.h"
#include "Eng_String_UC.h"
#include "Eng_DataConvert.h"
#include "Eng_TopBar.h"
#include "Lib_res_port.h"
#include "Fwl_pfdisplay.h"
#include "Fwl_tscrcom.h"
#include "fwl_display.h"

#define	MULTISET_TITLE_WIDTH            320
#define MULTISET_ITEMMENU_WIDTH         320
#define MULTISET_ITEMMENU_HEIGHT        240
#define MULTISET_ITEM_WIDTH             272
#define MULTISET_ITEM_HEIGHT            60
#define MULTISET_EDITAREA_WIDTH         320
#define MULTISET_EDITAREA_HEIGHT        216
#define MULTISET_EDITTEXT_WIDTH         228
#define MULTISET_EDITTEXT_HEIGHT        24
#define MULTISET_OPTION_WIDTH           192
#define MULTISET_OPTION_HEIGHT          18
#define MULTISET_OPTION_ICON            12
#define	MULTISET_ITEM_ROW_RINTVL        9
#define	MULTISET_OPTION_ROW_RINTVL      1
#define	MULTISET_ITEM_QTY_MAX		    10
#define	MULTISET_ITEM_QTY_PER_PAGE	    3
#define	MULTISET_OPTION_QTY_MAX		    10
#define	MULTISET_OPTION_QTY_PER_PAGE    7

#define MULTISET_TITLE_ON           0x00000001       // show title
#define MULTISET_TEXT_HCENTER       0x00000002       // title text horizontal align center
#define MULTISET_TEXT_LEFT          0x00000004       // title text horizontal align left
#define MULTISET_TEXT_RIGHT         0x00000008       // title text horizontal align right
#define MULTISET_TEXT_HALIGN        0x0000000e       // title text horizontal align mask
#define MULTISET_TEXT_VCENTER       0x00000010       // title text vertical align center
#define MULTISET_TEXT_UP            0x00000020       // title text vertical align up
#define MULTISET_TEXT_DOWN          0x00000040       // title text vertical align down
#define MULTISET_TEXT_VALIGN        0x00000070       // title text vertical align mask

static T_LEN scrBarWidth = 0;

static T_BOOL MultiSet_TitleInit(T_MULTISET_TITLE *pTitle);
static T_BOOL MultiSet_BackGroundInit(T_DISP_BLOCK *pDispBlock);
static T_BOOL MultiSet_EditAreaInit(T_EDIT_AREA *pEditArea);
//static T_BOOL MultiSet_ScrBarInit(T_SCROLL_BAR *pScrBar);
static T_BOOL MultiSet_ItemAreaInit(T_ITEM *pItem, T_ITEM_IMAGE *pItemImage);
static T_BOOL MultiSeT_OptionAreaInit(T_ITEM_OPTION_NODE *pOptioNode, T_ITEM_TYPE itemType, T_OPTION_IMAGE *pOptionImage);
static T_BOOL MultiSet_SetOptionTop(T_MULTISET_OPTION *pOption, T_S16 option_top);
static T_BOOL MultiSet_ShowAll(T_MULTISET *pMultiSet);
static T_BOOL MultiSet_ShowItem(T_MULTISET *pMultiSet);
static T_BOOL MultiSet_ShowEditArea(T_MULTISET *pMultiSet);
static T_BOOL MultiSet_ShowBackGround(T_RECT *pRect, T_COLOR backColor, T_pDATA pBackData);    
static T_BOOL MultiSet_ShowSubTitle(T_MULTISET_TITLE *pTitle, T_BOOL flag);
static T_BOOL MultiSet_ShowItemText(T_TEXT *pText, T_BOOL flag); 
static T_BOOL MultiSet_ShowScrBar(T_MULTISET *pMultiSet);
static T_BOOL MultiSet_ShowOptionArea(T_MULTISET *pMultiSet);

static T_eBACK_STATE MultiSet_UserKey_Handle(T_MULTISET *pMultiSet, T_MMI_KEYPAD *pPhyKey);
static T_VOID MultiSet_HitButton_Handle(T_MULTISET *pMultiSet, T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y);

/**
 * @brief   init T_MULTISET struct
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_BOOL
 * @retval  AK_TRUE  init success
 * @retval  AK_FALSE init fail 
 */
T_BOOL  MultiSet_Init(T_MULTISET *pMultiSet)
{
	T_LEN	width = 0;
	T_LEN	height = 0;
    
    AK_ASSERT_PTR(pMultiSet, "MultiSet_Init(): pMultiSet", AK_FALSE);

    pMultiSet->ctlMode = MULTISET_MODE_NORMAL;

    pMultiSet->pItemHead = AK_NULL;
    pMultiSet->pItemFocus = AK_NULL;
    pMultiSet->pFirstItem = AK_NULL;
    
    pMultiSet->itemQtyMax = MULTISET_ITEM_QTY_MAX;
    pMultiSet->itemQty = 0;
    pMultiSet->itemQtyPerPage = MULTISET_ITEM_QTY_PER_PAGE;

    pMultiSet->itmInterval = MULTISET_ITEM_ROW_RINTVL;
    pMultiSet->optnInterval = MULTISET_OPTION_ROW_RINTVL;

    //load image data
    if (AK_TRUE != MultiSet_LoadImageData(pMultiSet))
    {
        return AK_FALSE;
    }

    if (AK_NULL != pMultiSet->itmImgDt.itmBkDt.pImgDt)
    {
        AKBmpGetInfo((T_pCDATA)pMultiSet->itmImgDt.itmBkDt.pImgDt, &width, &height, AK_NULL);
        pMultiSet->itmInterval = (Fwl_GetLcdHeight() - TOP_BAR_HEIGHT - MULTISET_ITEM_QTY_PER_PAGE * height) / (MULTISET_ITEM_QTY_PER_PAGE + 1);
    }

    if (AK_NULL != pMultiSet->optnImgDt.optnBkDt.pImgDt)
    {
        AKBmpGetInfo((T_pCDATA)pMultiSet->optnImgDt.optnBkDt.pImgDt, &width, &height, AK_NULL);
        pMultiSet->itmInterval = (Fwl_GetLcdHeight() - TOP_BAR_HEIGHT - MULTISET_OPTION_QTY_PER_PAGE * height) / (MULTISET_OPTION_QTY_PER_PAGE + 1);
    }

    if ((AK_TRUE != MultiSet_TitleInit(&pMultiSet->mainTitle))
        || (AK_TRUE != MultiSet_BackGroundInit(&pMultiSet->backGround))
        || (AK_TRUE != MultiSet_EditAreaInit(&pMultiSet->editArea))
        /*|| (AK_TRUE != MultiSet_ScrBarInit(&pMultiSet->scrollBar))*/)
    {
        return AK_FALSE;
    }

    //init scroll bar 
    if (AK_NULL != pMultiSet->scrBar.bScBarImg[0])
    {
        AKBmpGetInfo(pMultiSet->scrBar.bScBarImg[0], &width, &height, AK_NULL);
    }
    ScBar_Init(&pMultiSet->scrBar, (T_POS)(Fwl_GetLcdWidth()-width), TOP_BAR_HEIGHT, \
           width, (T_LEN)(Fwl_GetLcdHeight()-TOP_BAR_HEIGHT), -10, SCBAR_VERTICAL);
    pMultiSet->ScrBarIsValid = AK_FALSE;
    scrBarWidth = width;
        
    pMultiSet->refreshFlag = MULTISET_REFRESH_NONE;
    
    return AK_TRUE;
}

/**
 * @brief   set control mode
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_MULTISET_MODE mode: control mode
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetCtlMode(T_MULTISET *pMultiSet, T_MULTISET_MODE mode)
{
    AK_ASSERT_PTR(pMultiSet, "MultiSet_Init(): pMultiSet", AK_FALSE);
    pMultiSet->ctlMode = mode;
    return AK_TRUE;
}

/**
 * @brief   get current control mode
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_MULTISET_MODE
 * @retval  MULTISET_MODE_ERROR: pMultiSet is AK_NULL, 
 * @retval  MULTISET_MODE_NORMAL: menu option mode, 
 * @retval  MULTISET_MODE_SETTING: edit mode, 
 */
T_MULTISET_MODE MultiSet_GetCtlMode(T_MULTISET *pMultiSet)
{
    if (AK_NULL == pMultiSet)
    {
        return MULTISET_MODE_NORMAL;
    }
    
    return pMultiSet->ctlMode;
}

/**
 * @brief   set main title text
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   const T_U16 *pTitleText: main title text
 * @param   T_COLOR titleColor: main title text color
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetTitleText(T_MULTISET *pMultiSet, const T_U16 *pTitleText,  T_COLOR titleColor)
{
    
    AK_ASSERT_PTR(pMultiSet, "MultiSet_SetTitleText():pMultiSet", AK_FALSE);
    
    pMultiSet->mainTitle.textColor = titleColor;
    
    if (pTitleText != AK_NULL) 
    {
        Utl_UStrCpy(pMultiSet->mainTitle.content, pTitleText);
    }

    MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_TITLE);

    return AK_TRUE;
}

/**
 * @brief   add item to multiSet struct 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: item id
 * @param   const T_U16 *pItemTitle: item title text
 * @param   const T_U16 *pItemText: item attribute text
 * @param   T_ITEM_TYPE itemType: item Type
 * @param   T_ITEM_KEYPROCESS_CALLFUNC keyProcessCallFunc: item key process call back function pointer
 * @param   T_ITEM_ShowCALLFUNC editShowCallFunc: item show call back function pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_AddItemWithOption(  T_MULTISET *pMultiSet, 
                                    T_U32 itemId, 
                                    const T_U16 *pItemTitle,
                                    const T_U16 *pItemText, 
                                    T_ITEM_TYPE itemType)
{
	T_ITEM_NODE	*pItemNode, *q, *r;

	AK_ASSERT_PTR(pMultiSet, "TimeSet_AddItemWithOption(): pMultiSet", AK_FALSE);

    if ((pMultiSet->itemQtyMax > 0) && (pMultiSet->itemQty >= pMultiSet->itemQtyMax))
    {
        return AK_FALSE;
    }

    pItemNode = (T_ITEM_NODE *)Fwl_Malloc(sizeof(T_ITEM_NODE));
    if (AK_NULL == pItemNode)
    {
        Fwl_Print(C3, M_CTRL, "MultiSet_AddItemWithOption() pitemnode malloc fail");
        return AK_FALSE;        
    }
    
    pItemNode->id = itemId;
    pItemNode->keyProcessCallFunc   = AK_NULL;
    pItemNode->editShowCallFunc     = AK_NULL;

    //init item
    if (AK_FALSE == MultiSet_ItemAreaInit(&pItemNode->item, &pMultiSet->itmImgDt))
    {
        pItemNode = Fwl_Free(pItemNode);
        return AK_FALSE;   
    }

    if (AK_NULL == pItemTitle)
    {
        pItemNode = Fwl_Free(pItemNode);
        return AK_FALSE;   
    }
    else
    {
        Utl_UStrCpy(pItemNode->item.title.content, pItemTitle);
        pItemNode->item.text.pText = (T_pWSTR)pItemText;
        pItemNode->itemType     = itemType;
        pItemNode->pOptionHead  = AK_NULL;
        pItemNode->pOptionFocus = AK_NULL;
        pItemNode->pPrevious    = AK_NULL;
        pItemNode->optionQty    = 0;
        pItemNode->pNext        = AK_NULL;

        if (AK_NULL == pMultiSet->pItemHead)
        {
            pMultiSet->pItemHead    = pItemNode;
            pMultiSet->pItemFocus   = pItemNode;
            pMultiSet->pFirstItem  = pItemNode;            
        }
        else
        {
            q = pMultiSet->pItemHead;
            r = q;
            while(AK_NULL != q)
            {
                r = q;
                q = q->pNext;
            }
            pItemNode->pPrevious = r;
            r->pNext = pItemNode;
        }

        pMultiSet->itemQty++;
    }

    return AK_TRUE;
}

/**
 * @brief   add option to item
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: item id
 * @param   T_U8 OptionId: option id
 * @param   const T_U16 *pOptionText: option conten
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_AddItemOption(T_MULTISET *pMultiSet, T_U32 ItemId, T_U8 OptionId, const T_U16 *pOptionText)
{
    T_ITEM_NODE         *pItemNode;
    T_ITEM_OPTION_NODE  *pOptionNode, *q;
    
    AK_ASSERT_PTR(pMultiSet, "MULTISET_AddItemOption(): pMultiSet", AK_FALSE);

    if (AK_NULL == pOptionText)
    {
        return AK_FALSE;
    }

    pItemNode = pMultiSet->pItemHead;
    while (AK_NULL != pItemNode)
    {
        if (pItemNode->id == ItemId)
        {
            break;
        }
        pItemNode = pItemNode->pNext;
    }

    if (AK_NULL == pItemNode)
    {
        return AK_FALSE;
    }
    else
    {
        if (ITEM_TYPE_EDIT == pItemNode->itemType)   
        {
            return AK_FALSE;
        }
        else
        {
            pOptionNode = pItemNode->pOptionHead;
            while (AK_NULL != pOptionNode)
            {
                if (pOptionNode->Id == OptionId)
                {
                    return AK_FALSE;
                }

                q = pOptionNode;                
                pOptionNode = pOptionNode->pNext;
            }

            pOptionNode = (T_ITEM_OPTION_NODE *)Fwl_Malloc(sizeof(T_ITEM_OPTION_NODE));
            if (AK_NULL == pOptionNode)
            {
                Fwl_Print(C3, M_CTRL, "MultiSet_AddItemOption() malloc fail");
                return AK_FALSE;
            }

            pOptionNode->Id = OptionId;

            ////init option
            MultiSeT_OptionAreaInit(pOptionNode, pItemNode->itemType, &pMultiSet->optnImgDt);

            Utl_UStrCpy(pOptionNode->option.title.content, pOptionText);
            pOptionNode->pPrevious = AK_NULL;
            pOptionNode->pNext = AK_NULL;

            if (AK_NULL == pItemNode->pOptionHead)
            {
                pItemNode->pOptionHead = pOptionNode;
				pItemNode->pOptionFocus = pItemNode->pOptionHead;
            }
            else
            {
                q->pNext = pOptionNode;
                pOptionNode->pPrevious = q;
            }
            pItemNode->optionQty++;
        }
    }

    return AK_TRUE;
}

/**
 * @brief   set hit button precess call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_MULTISET_HITBUTTON_CALLFUNC hitButtonCallFunc: hit button process call back function pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetHitButtonCallBack(T_MULTISET *pMultiSet, T_U32 ItemId, T_MULTISET_HITBUTTON_CALLFUNC fHitButtonCallFunc)
{
    T_ITEM_NODE *pItemNode = AK_NULL;

    if (AK_NULL == pMultiSet)
    {
        return AK_FALSE;
    }

    pItemNode = MultiSet_GetItemById(pMultiSet, ItemId);
    if (AK_NULL == pItemNode)
    {
        return AK_FALSE;
    }
    else
    {
        pItemNode->fHitButtonCallFunc = fHitButtonCallFunc;
    }

    return AK_TRUE;
}


/**
 * @brief   set item key precess call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: item id
 * @param   T_ITEM_KEYPROCESS_CALLFUNC keyProcessCallFunc: item key process call back function pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetItemKeyProcessCallBack(T_MULTISET *pMultiSet, T_U32 ItemId, T_ITEM_KEYPROCESS_CALLFUNC keyProcessCallFunc)
{
    T_ITEM_NODE         *pItemNode;

    if (AK_NULL == pMultiSet)
    {
        return AK_FALSE;
    }

    pItemNode = MultiSet_GetItemById(pMultiSet, ItemId);
    if (AK_NULL == pItemNode)
    {
        return AK_FALSE;
    }
    else
    {
        pItemNode->keyProcessCallFunc = keyProcessCallFunc;
    }

    return AK_TRUE;
}

/**
 * @brief   set show edit call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: item id
 * @param   T_ITEM_KEYPROCESS_CALLFUNC keyProcessCallFunc: item key process call back function pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetEditShowCallBack(T_MULTISET *pMultiSet, T_U32 ItemId, T_ITEM_ShowCALLFUNC showEditCallBackFunc)
{
    T_ITEM_NODE         *pItemNode;

    if (AK_NULL == pMultiSet)
    {
        return AK_FALSE;
    }

    pItemNode = MultiSet_GetItemById(pMultiSet, ItemId);
    if (AK_NULL == pItemNode)
    {
        return AK_FALSE;
    }
    else
    {
        pItemNode->editShowCallFunc = showEditCallBackFunc;
    }

    return AK_TRUE;
}

/**
 * @brief   set item focus by id
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemFocusId: the item id
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetItemFocus(T_MULTISET *pMultiSet, T_U32 itemFocusId)
{
    T_ITEM_NODE         *pItemNode;

    AK_ASSERT_PTR(pMultiSet, "MultiSet_SetOptionFocus(): pMultiSet", AK_FALSE);

    pItemNode = MultiSet_GetItemById(pMultiSet, itemFocusId);
    if (AK_NULL == pItemNode)
    {
        return AK_FALSE;
    }
    
    pMultiSet->pItemFocus = pItemNode;

    return AK_TRUE;
}

/**
 * @brief   get item focus id
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemFocus: item focus node pointer
 * @return  T_U32
 * @retval  item id
 */
T_U32 MultiSet_GetItemFocusId(T_ITEM_NODE *pItemFocus)
{
    T_U32   id;

    id = pItemFocus ? pItemFocus->id : 0;

    return id;
}

/**
 * @brief   get the focus item pointer
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_ITEM_NODE *
 * @retval  focus item pointer
 * @retval  AK_NULL
 */
T_ITEM_NODE *MultiSet_GetItemFocus(T_MULTISET *pMultiSet)
{
    AK_ASSERT_PTR(pMultiSet, "MultiSet_SetOptionFocus(): pMultiSet", AK_NULL);

    return pMultiSet->pItemFocus;
}

/**
 * @brief   根据item focus设置显示的第一个item指针
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetFirstItem(T_MULTISET *pMultiSet)
{
    T_ITEM_NODE *pItemNode = AK_NULL;
    T_U32       count = 0;

    AK_ASSERT_PTR(pMultiSet, "MultiSet_SetOptionFocus(): pMultiSet", AK_FALSE);

    if ((AK_NULL == pMultiSet->pItemHead) || (AK_NULL == pMultiSet->pItemFocus))
    {
        pMultiSet->pFirstItem = AK_NULL;
        return AK_TRUE;
    }

    if (AK_NULL == pMultiSet->pFirstItem)
    {
        pMultiSet->pFirstItem = pMultiSet->pItemHead;
    }

    pItemNode = pMultiSet->pItemFocus;

    //从pItemFocus往前找
    while (AK_NULL != pItemNode)
    {
        //找到了pFirstItem 退出查找
        if (pItemNode == pMultiSet->pFirstItem)
        {
            break;
        }

        //对比次数已经是itemQtyPerPage次，退出查找
        if (count == pMultiSet->itemQtyPerPage - 1)
        {
            break;
        }
        
        pItemNode = pItemNode->pPrevious;
        count++;
    }

    //pItemNode为空了也没找到，说明pFirstItem在pItemNode之后
    if (AK_NULL == pItemNode)
    {
        pMultiSet->pFirstItem = pMultiSet->pItemFocus;
        return AK_TRUE;
    }
    else 
    {   
        //对比了itemQtyPerPage次，正好找到了或没找到
        if (count == pMultiSet->itemQtyPerPage - 1)
        {
            pMultiSet->pFirstItem = pItemNode;
        }
        else
        {
        //对比itemQtyPerPage次之内找到，不改变pFirstItem
        }
    }

    return AK_TRUE;
}

/**
 * @brief   get item pointer by id
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: the special item id
 * @return  T_ITEM_NODE
 * @retval  item pionter
 * @retval  AK_NULL  fail
 */
T_ITEM_NODE *MultiSet_GetItemById(T_MULTISET *pMultiSet, T_U32 itemId)
{
    T_ITEM_NODE *pItemNode = AK_NULL;

    if (AK_NULL == pMultiSet)
    {
        return AK_NULL;
    }

    pItemNode = pMultiSet->pItemHead;
    while (AK_NULL != pItemNode)
    {
        if (pItemNode->id == itemId)
        {
            break;
        }
        pItemNode = pItemNode->pNext;
    }

    return pItemNode;
}

/**
 * @brief   get option pointer by id
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemNode: item node struct
 * @param   T_U32 itemId: the special option id
 * @return  T_ITEM_OPTION_NODE *
 * @retval  option node pionter
 * @retval  AK_NULL  fail
 */
T_ITEM_OPTION_NODE *MultiSet_GetOptionById(T_ITEM_NODE *pItemNode, T_U32 optionId)
{
    T_ITEM_OPTION_NODE  *pOptionNode = AK_NULL;

    if (AK_NULL == pItemNode)
    {
        return AK_NULL;
    }

    pOptionNode = pItemNode->pOptionHead;
    while (AK_NULL != pOptionNode)
    {
        if (pOptionNode->Id == optionId)
        {
            break;
        }

        pOptionNode = pOptionNode->pNext;
    }
    
    return pOptionNode;
}

/**
 * @brief   设置复选框的设置状态
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: the special item id
 * @param   T_U32 optionId: the special option id
 * @param   T_BOOL chooseFlag: AK_TRUE: be chosen; AK_FALSE: not be chosen   
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetCheckBoxState(T_MULTISET *pMultiSet, T_U32 itemId, T_U32 optionId, T_BOOL setFlag)
{
    T_ITEM_NODE         *pItemNode;
    T_ITEM_OPTION_NODE  *pOptionNode;

    AK_ASSERT_PTR(pMultiSet, "MultiSet_SetOptionFocus(): pMultiSet", AK_FALSE);

    pItemNode = MultiSet_GetItemById(pMultiSet, itemId);
    if (AK_NULL == pItemNode)
    {
        return AK_FALSE;
    }
    else
    {
        if (ITEM_TYPE_CHECK != pItemNode->itemType)   
        {
            return AK_FALSE;
        }
        else
        {
            pOptionNode = MultiSet_GetOptionById(pItemNode, optionId);
            if (AK_NULL == pOptionNode)
            {
                return AK_FALSE;
            }
            
            pOptionNode->option.choosed = setFlag;
        }
    }
    return AK_TRUE;
}

/**
 * @brief   set the focus option 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @param   T_U32 itemId: the special item id
 * @param   T_U8 optionId: the special option id
 * @return  T_BOOL
 * @retval  AK_TRUE  success, the special id option be setup
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetOptionFocus(T_MULTISET *pMultiSet, T_U32 itemId, T_U8 optionFocusId)
{
    T_ITEM_NODE         *pItemNode;
    T_ITEM_OPTION_NODE  *pOptionNode;

    AK_ASSERT_PTR(pMultiSet, "MultiSet_SetOptionFocus(): pMultiSet", AK_FALSE);

    pItemNode = MultiSet_GetItemById(pMultiSet, itemId);
    if (AK_NULL == pItemNode)
    {
        return AK_FALSE;
    }
    else
    {
        if (ITEM_TYPE_RADIO == pItemNode->itemType)   
        {
            pOptionNode = pItemNode->pOptionHead;
            while (AK_NULL != pOptionNode)
            {
                if (pOptionNode->Id == optionFocusId)
                {
                    pOptionNode->option.choosed = AK_TRUE;
                    pItemNode->pOptionFocus = pOptionNode;

                    break;
                }

                pOptionNode = pOptionNode->pNext;
            }

            if (AK_NULL == pOptionNode)
            {
                return AK_FALSE;
            }
        }
        else if (ITEM_TYPE_CHECK == pItemNode->itemType)   
        {
            pItemNode->pOptionFocus = pItemNode->pOptionHead;
        }
        else
        {
            return AK_FALSE;
        }
    }
    return AK_TRUE;
}

/**
 * @brief   get the focus item's index 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_U32
 * @retval  0:error; other: success
 */
T_U32 MultiSet_GetFocusItemIndex(T_MULTISET *pMultiSet)
{
    T_ITEM_NODE *pItemNode = AK_NULL;
    T_U32       count = 0;
        
    if (AK_NULL == pMultiSet)
    {
        return 0;
    }

    pItemNode = pMultiSet->pItemHead;
    while (AK_NULL != pItemNode)
    {
        if (pItemNode == pMultiSet->pItemFocus)
        {
            break;
        }
        count++;
        pItemNode = pItemNode->pNext;
    }

    if (AK_NULL == pItemNode)
    {
        return 0;
    }
    
    return count;
}

/**
 * @brief   get the focus item's option type 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemNode: the special item node pointer
 * @return  T_ITEM_OPTION_TYPE: 
 * @retval  ITEM_TYPE_EDIT: edit;
 * @retval  ITEM_TYPE_RADIO: option button
 * @retval  ITEM_TYPE_CHECK: check box
 * @retval  error: pItemNode is AK_NULL
 */
T_ITEM_TYPE MultiSet_GetFocusItemType(T_ITEM_NODE *pItemNode)
{
    if (AK_NULL == pItemNode)
    {
        return ITEM_TYPE_ERROR;
    }

    return pItemNode->itemType;
}

/**
 * @brief   get the focus option id 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemNode: the special item node pointer
 * @return  T_U8: 
 * @retval  option id
 */
T_U32 MultiSet_GetOptionFocusId(T_ITEM_NODE *pItemFocus)
{
    T_U32   id;

    id = pItemFocus->pOptionFocus ? pItemFocus->pOptionFocus->Id : 0;

    return id;
}

/**
 * @brief   get the focus option choose state 
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemFocus: the focus item node pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE : be chosen
 * @retval  AK_FALSE : not be chosen
 */
T_BOOL MultiSet_GetOptionFocusChooseState(T_ITEM_NODE *pItemFocus)
{
    AK_ASSERT_PTR(pItemFocus, "MULTISET_Handler(): pOptionFocus", AK_FALSE);
    AK_ASSERT_PTR(pItemFocus->pOptionFocus, "MULTISET_Handler(): pItemFocus->pOptionFocus", AK_FALSE);

    return pItemFocus->pOptionFocus->option.choosed;
}

/**
 * @brief   multiSet handler
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @param   T_EVT_CODE Event: event
 * @param   T_EVT_PARAM *pPara: event parameter 
 * @return  T_eBACK_STATE: 
 * @retval  
 */
T_eBACK_STATE MultiSet_Handler(T_MULTISET *pMultiSet, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_MMI_KEYPAD        phyKey;
    T_eBACK_STATE       ret = eStay;
    T_POS               posX;
    T_POS               posY;

    if ((AK_NULL == pMultiSet) || (AK_NULL == pParam))
    {
        return eStay;    
    }

    switch (Event)
    {
        case M_EVT_USER_KEY:
            phyKey.keyID = pParam->c.Param1;
            phyKey.pressType = pParam->c.Param2;

            ret = MultiSet_UserKey_Handle(pMultiSet, &phyKey);
            break;
            
        case M_EVT_TOUCH_SCREEN:
            posX = (T_POS)pParam->s.Param2;
            posY = (T_POS)pParam->s.Param3;

            phyKey.keyID = kbNULL;
            phyKey.pressType = PRESS_SHORT;

            switch (pParam->s.Param1) 
            {
                case eTOUCHSCR_UP:
                    MultiSet_HitButton_Handle(pMultiSet, &phyKey, posX, posY);
                    ret = MultiSet_UserKey_Handle(pMultiSet, &phyKey);
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
            break;
        default:
            break;
    }

    return ret;
}

static T_VOID MultiSet_HitButton_Handle(T_MULTISET *pMultiSet, T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y)
{
    T_ITEM_NODE         *pItem = AK_NULL;
    T_ITEM_OPTION_NODE  *pOption = AK_NULL;
    T_RECT              rect, rect1;
    T_pRECT             pRect = AK_NULL;
    T_ITEM_TYPE         itemType = ITEM_TYPE_NONE;
    T_U32               i = 0;

    if ((AK_NULL == pMultiSet)||(AK_NULL == pPhyKey))
    {
        return;
    }
    
    //get the rect of cancel button
    rect = TopBar_GetRectofCancelButton();
    if (PointInRect(&rect, x, y))
    {
        pPhyKey->keyID = kbCLEAR;
        pPhyKey->pressType = PRESS_SHORT;
    }

    //item
    if (MULTISET_MODE_NORMAL == MultiSet_GetCtlMode(pMultiSet))
    {
        pItem = pMultiSet->pFirstItem;

        for (i=0; i<pMultiSet->itemQtyPerPage; i++)
        {
            pRect = &pItem->item.rect;

            if (pMultiSet->pItemFocus == pItem)
            {
                if (PointInRect(pRect, x, y))
                {
                    pPhyKey->keyID = kbOK;
                    pPhyKey->pressType = PRESS_SHORT;
                    return;
                }
            }
            else
            {
                if (PointInRect(pRect, x, y))
                {
                    pMultiSet->pItemFocus = pItem;
                    MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_ALL);
                    return;
                }
            }

            pItem = pItem->pNext;
        }

        //scroll bar
        rect = ScBar_GetRect(&pMultiSet->scrBar);
        if (PointInRect(&rect, x, y))
        {
            ScBar_GetLocaRect2(&rect1, &pMultiSet->scrBar);
            if (!PointInRect(&rect1, x, y))
            {
                i = (y - rect.top)/(rect.height/pMultiSet->itemQty);
                if (i > pMultiSet->pItemFocus->id)
                {
                    MultiSet_SetItemFocus(pMultiSet, pMultiSet->pItemFocus->id + 1);
                }
                else
                {
                    MultiSet_SetItemFocus(pMultiSet, pMultiSet->pItemFocus->id - 1);
                }
                
                MultiSet_SetFirstItem(pMultiSet);
            
                MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_ALL);
            }
            return;
        }
        else
        {
            ScBar_GetUpIconRect(&rect1, &pMultiSet->scrBar);
            if (PointInRect(&rect1, x, y))
            {
                MultiSet_SetItemFocus(pMultiSet, pMultiSet->pItemFocus->id - 1);
                MultiSet_SetFirstItem(pMultiSet);
                MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_ALL);
                return;
            }

            ScBar_GetDownIconRect(&rect1, &pMultiSet->scrBar);
            if (PointInRect(&rect1, x, y))
            {
                MultiSet_SetItemFocus(pMultiSet, pMultiSet->pItemFocus->id + 1);
                MultiSet_SetFirstItem(pMultiSet);
                MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_ALL);
                return;
            }
        }
    }
    else //edit / radio button / check box
    {
        itemType = MultiSet_GetFocusItemType(pMultiSet->pItemFocus);
        switch(itemType)
        {
            case ITEM_TYPE_EDIT:
            	if (AK_NULL != pMultiSet->pItemFocus->fHitButtonCallFunc)
                {
                    pMultiSet->pItemFocus->fHitButtonCallFunc(pPhyKey, x, y);
                }
                break;

            case ITEM_TYPE_RADIO:
            case ITEM_TYPE_CHECK:
                pOption = pMultiSet->pItemFocus->pOptionHead;
                for (i = 0; i < pMultiSet->pItemFocus->optionQty; i++)
                {
                    if (pMultiSet->pItemFocus->pOptionFocus == pOption)
                    {
                        pRect = &pOption->option.icon.rect;
                        if (PointInRect(pRect, x, y))
                        {
                            pPhyKey->keyID = kbOK;
                            pPhyKey->pressType = PRESS_SHORT;
                            return;
                        }
                    }
                    else
                    {
                        pRect = &pOption->option.rect;
                        if (PointInRect(pRect, x, y))
                        {
                            pMultiSet->pItemFocus->pOptionFocus = pOption;
                            MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_OPTION);
                            return;
                        }
                    }

                    pOption = pOption->pNext;
                }
                break;
                
            default:
                break;
        }
    }
}

static T_eBACK_STATE MultiSet_UserKey_Handle(T_MULTISET *pMultiSet, T_MMI_KEYPAD *pPhyKey)
{
    T_ITEM_TYPE         itemType;
    T_ITEM_NODE         *pItemFocus = AK_NULL;
    T_eBACK_STATE       ret = eStay;

    pItemFocus = pMultiSet->pItemFocus;
    
    switch (pMultiSet->ctlMode)
    {
        case MULTISET_MODE_NORMAL:
            switch (pPhyKey->keyID)
            {
                case kbOK:
                    itemType = MultiSet_GetFocusItemType(pItemFocus);
                    
                    if (ITEM_TYPE_NONE == itemType)
                	{
                		ret = eNext;
                	}
                    else 
                    {
                		pMultiSet->ctlMode = MULTISET_MODE_SETTING;
                		if (ITEM_TYPE_EDIT == itemType)
                		{
                			MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_EDITAREA);
                		}
                		else
                		{
                			MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_OPTION);
                		}
                    }
                	break;                

                case kbCLEAR:
                    if (PRESS_LONG == pPhyKey->pressType)
                    {
                        ret = eHome;
                    }
                    else
                    {
                        ret = eReturn;
                    }
                    break;

                case kbLEFT:
                case kbUP:
                    MultiSet_MovieItemFocusToPrevious(pMultiSet);
                	MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_ALL);
                    break;                

                case kbDOWN:
                case kbRIGHT:
                    MultiSet_MovieItemFocusToNext(pMultiSet);
                	MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_ALL);
                    break;                

                default:
                    break;
            } 
            break;

        case MULTISET_MODE_SETTING:  
            switch (pItemFocus->itemType)
            {
                case ITEM_TYPE_EDIT:
                	if (AK_NULL != pItemFocus->keyProcessCallFunc)
                    {
                        pItemFocus->keyProcessCallFunc(pPhyKey);
                    }

                    if (kbCLEAR == pPhyKey->keyID)
                    {
                        if (PRESS_LONG == pPhyKey->pressType)
                        {
                            ret = eHome;
                        }                        
                    }
                    break;

                case ITEM_TYPE_RADIO:
                case ITEM_TYPE_CHECK:                        
                    switch (pPhyKey->keyID)
                    {
                        case kbOK:
                        	if (AK_NULL != pItemFocus->keyProcessCallFunc)
                            {
                                pItemFocus->keyProcessCallFunc(pPhyKey);
                            }

                            MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_OPTION);
                            break;

                        case kbUP:
                        case kbLEFT:
                            MultiSet_MovieOptionFocusToPrevious(pItemFocus);
                            MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_OPTION);
                            break;

                        case kbDOWN:
                        case kbRIGHT:
                            MultiSet_MovieOptionFocusToNext(pItemFocus);
                            MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_OPTION);
                            break;                
                            
                        case kbCLEAR:
                            if (PRESS_LONG == pPhyKey->pressType)
                            {
                                ret = eHome;
                            }
                            else
                            {
                                pMultiSet->ctlMode = MULTISET_MODE_NORMAL;
                                MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_ALL);
                            }
                            break;                
                            
                        default:
                            break;
                    }
                    break;
                
                default:
                    break;
            }       
            break;                    

        default:
            break;
    } 
    
    return ret;
}


/**
 * @brief   set refresh mode
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @param   T_MULTISET_REFRESH_FLAG refreshFlag: refresh flag
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_SetRefresh(T_MULTISET *pMultiSet, T_MULTISET_REFRESH_FLAG refreshFlag)
{
    AK_ASSERT_PTR(pMultiSet, "MultiSet_Show(): pMultiSet", AK_FALSE);

    pMultiSet->refreshFlag = refreshFlag;
    
    return AK_TRUE;
}

/**
 * @brief   show function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_VOID
 */
T_VOID MultiSet_Show(T_MULTISET *pMultiSet)
{
    T_U32   itemIndex = 0;
    
    AK_ASSERT_PTR_VOID(pMultiSet, "MultiSet_Show(): pMultiSet");

    itemIndex = MultiSet_GetFocusItemIndex(pMultiSet);
    ScBar_SetValue(&pMultiSet->scrBar, itemIndex, pMultiSet->itemQty, (T_U16)pMultiSet->itemQtyPerPage);

    switch(pMultiSet->refreshFlag)
    {
        case MULTISET_REFRESH_NONE:
            break;
            
        case MULTISET_REFRESH_ALL:
            MultiSet_ShowAll(pMultiSet);
            break;
            
        case MULTISET_REFRESH_ITEM:
            MultiSet_ShowItem(pMultiSet);
            break;            

        case MULTISET_REFRESH_EDITAREA:
            MultiSet_ShowEditArea(pMultiSet);
            break;
          
        case MULTISET_REFRESH_OPTION:
            MultiSet_ShowOptionArea(pMultiSet);
            break;
            
        default:
            break;
    }

    TopBar_SetTitle(pMultiSet->mainTitle.content);
    TopBar_EnableShow();
    TopBar_Show(TB_REFRESH_ALL);

    MultiSet_SetRefresh(pMultiSet, MULTISET_REFRESH_NONE);
}

/**
 * @brief   movie item focus to previous item
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_MovieItemFocusToPrevious(T_MULTISET *pMultiSet)
{
    T_ITEM_NODE     *pItemNode = AK_NULL;
    
    AK_ASSERT_PTR(pMultiSet, "MultiSet_MovieFocusToPrevious():pMultiSet", AK_FALSE);

	if (AK_NULL == pMultiSet->pItemHead)
	{
		pMultiSet->pItemFocus = AK_NULL;
	}
    else if (AK_NULL == pMultiSet->pItemFocus)
	{
		pMultiSet->pItemFocus = pMultiSet->pItemHead;
	}
	else if (AK_NULL == pMultiSet->pItemFocus->pPrevious)
    {
        pItemNode = pMultiSet->pItemHead;
        while (AK_NULL != pItemNode->pNext)
        {
            pItemNode = pItemNode->pNext;
        }
        pMultiSet->pItemFocus = pItemNode;
    }
	else 
	{
		pMultiSet->pItemFocus = pMultiSet->pItemFocus->pPrevious;
	}

    MultiSet_SetFirstItem(pMultiSet);

	return AK_TRUE;
}

/**
 * @brief   movie item focus to next item
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_MovieItemFocusToNext(T_MULTISET *pMultiSet)
{
    AK_ASSERT_PTR(pMultiSet, "TimeSet_MovieFocusToNext():pMultiSet", AK_FALSE);

	if (AK_NULL == pMultiSet->pItemHead)
	{
		pMultiSet->pItemFocus = AK_NULL;
	}
    else if (AK_NULL == pMultiSet->pItemFocus)
	{
		pMultiSet->pItemFocus = pMultiSet->pItemHead;
	}
	else if (AK_NULL == pMultiSet->pItemFocus->pNext)
    {
		pMultiSet->pItemFocus = pMultiSet->pItemHead;
    }
	else
	{
		pMultiSet->pItemFocus = pMultiSet->pItemFocus->pNext;
	}
    
    MultiSet_SetFirstItem(pMultiSet);
    
	return AK_TRUE;
}

/**
 * @brief   movie option focus to previous option
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemFocus: the item focus node pionter
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_MovieOptionFocusToPrevious(T_ITEM_NODE *pItemFocus)
{
    T_ITEM_OPTION_NODE *pOptionNode = AK_NULL;
    
    AK_ASSERT_PTR(pItemFocus, "MultiSet_MovieFocusToPrevious():pItemFocus", AK_FALSE);

    if (ITEM_TYPE_EDIT == pItemFocus->itemType)
    {
        return AK_FALSE;
    }

	if (AK_NULL == pItemFocus->pOptionFocus)
	{
		pItemFocus->pOptionFocus = pItemFocus->pOptionHead;
	}
	else if (AK_NULL == pItemFocus->pOptionFocus->pPrevious)
    {
        pOptionNode = pItemFocus->pOptionHead;
        while (AK_NULL != pOptionNode->pNext)
        {
            pOptionNode = pOptionNode->pNext;
        }

        pItemFocus->pOptionFocus = pOptionNode;
    }
	else 
	{
	    pItemFocus->pOptionFocus = pItemFocus->pOptionFocus->pPrevious;
	}

	return AK_TRUE;
}

/**
 * @brief   movie option focus to next option
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemFocus: the item focus node pionter
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_MovieOptionFocusToNext(T_ITEM_NODE *pItemFocus)
{
    AK_ASSERT_PTR(pItemFocus, "MultiSet_MovieFocusToPrevious():pItemFocus", AK_FALSE);

    if (ITEM_TYPE_EDIT == pItemFocus->itemType)
    {
        return AK_FALSE;
    }
    
	if ((AK_NULL == pItemFocus->pOptionFocus)
        || (AK_NULL == pItemFocus->pOptionFocus->pNext))
    {
		pItemFocus->pOptionFocus = pItemFocus->pOptionHead;
    }
	else 
	{
	    pItemFocus->pOptionFocus = pItemFocus->pOptionFocus->pNext;
	}

	return AK_TRUE;
}

/**
 * @brief   chang the option's choose state
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_NODE *pItemFocus: the item focus node pionter
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_ChangOptionChooseState(T_ITEM_NODE *pItemFocus)
{
    T_ITEM_OPTION_NODE  *pOptionNode;
    T_BOOL              flag = AK_FALSE;
    
    AK_ASSERT_PTR(pItemFocus, "MultiSet_ChangOptionChooseState():pItemFocus", AK_FALSE);

    if ((AK_NULL == pItemFocus->pOptionFocus)
        || (AK_NULL == pItemFocus->pOptionHead))
    {
        return AK_FALSE;
    }

    switch (pItemFocus->itemType)
    {
        case ITEM_TYPE_CHECK:
        	if (AK_TRUE == pItemFocus->pOptionFocus->option.choosed)
            {
        		pItemFocus->pOptionFocus->option.choosed = AK_FALSE;
            }
        	else 
        	{
        	    pItemFocus->pOptionFocus->option.choosed = AK_TRUE;
        	}
            break;

        case ITEM_TYPE_RADIO:
            pOptionNode = pItemFocus->pOptionHead;
            while (AK_NULL != pOptionNode)
            {
                if (AK_TRUE == pOptionNode->option.choosed)
                {
                    flag = AK_TRUE;
                    break;
                }
                
                pOptionNode = pOptionNode->pNext;
            }

            if (AK_FALSE == flag)
            {
                pItemFocus->pOptionFocus->option.choosed = AK_TRUE;
            }
            else
            {
                if (pOptionNode != pItemFocus->pOptionFocus)
                {
                    pOptionNode->option.choosed = AK_FALSE;
                    pItemFocus->pOptionFocus->option.choosed = AK_TRUE;
                }
            }
            break;

        default:
            break;
    }

	return AK_TRUE;
}

/**
 * @brief   check scroll bar is valid or not
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE:  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_CheckScrollBar(T_MULTISET *pMultiSet)
{
    AK_ASSERT_PTR(pMultiSet, "TimeSet_CheckScrollBar(): pMultiSet", AK_FALSE);

    if (pMultiSet->itemQty > pMultiSet->itemQtyPerPage)
    {
        pMultiSet->ScrBarIsValid = AK_TRUE;
    }
    else
    {
        pMultiSet->ScrBarIsValid = AK_FALSE;
    }

    return AK_TRUE;
}

/**
 * @brief   free T_MULTISET struct
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct
 * @return  T_VOID
 */
T_VOID  MultiSet_Free(T_MULTISET *pMultiSet)
{
    T_ITEM_NODE         *qItemNode = AK_NULL;
    T_ITEM_NODE         *pItemNode = AK_NULL;
    T_ITEM_OPTION_NODE  *qOptionNode = AK_NULL;
    T_ITEM_OPTION_NODE  *pOptionNode = AK_NULL;

    if (AK_NULL != pMultiSet)
    {
        pItemNode = pMultiSet->pItemHead;
        while (AK_NULL != pItemNode)
        {
            pOptionNode = pItemNode->pOptionHead;
            while (AK_NULL != pOptionNode)
            {
                qOptionNode = pOptionNode;
                pOptionNode = pOptionNode->pNext;
                qOptionNode = Fwl_Free(qOptionNode);
            }
            qItemNode = pItemNode;
            pItemNode = pItemNode->pNext;
            qItemNode->pOptionFocus = AK_NULL;        
            
            qItemNode = Fwl_Free(qItemNode);
        }

        pMultiSet->pItemFocus = AK_NULL;
        pMultiSet->pFirstItem = AK_NULL;
        pMultiSet->editArea.text.pText = AK_NULL;
    }
}

/**
 * @brief   get the text rect width
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_LEN: 
 * @retval  the text rect width
 */
T_LEN  MultiSet_GetTextRectWidth(T_MULTISET *pMultiSet)
{
    AK_ASSERT_PTR(pMultiSet, "MultiSet_GetTextRectWidth(): pMultiSet", AK_FALSE);

    if (AK_NULL != pMultiSet->pItemHead) 
    {
        return pMultiSet->pItemHead->item.text.rect.width;
    }
    else
    {
        return MULTISET_EDITTEXT_WIDTH;
    }
}

/**
 * @brief   get text pos
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U16 *pText: display string pointer
 * @param   T_RECT *pRect: text display rect
 * @param   T_U32 textStyle: display style
 * @param   T_S16 *PosX:  save PosX
 * @param   T_S16 *PosY: save PosY
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_GetTextPos(T_U16 *pText, T_RECT *pRect, T_U32 textStyle, 
                            T_S16 *PosX, T_S16 *PosY)
{
    T_S16   OffsetX = 0;
    T_S16   OffsetY = 0;
    T_U32   width = 0;

    AK_ASSERT_PTR(pText, "MultiSet_GetTextPos():pText", AK_FALSE);

    if (AK_NULL == pText)
	{
		return AK_FALSE;
	}
	else
	{
		width = UGetSpeciStringWidth(pText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pText));
	}
	
    if (width <= (T_U32)(pRect->width - 8)) 
    {
        switch (textStyle & ICONEXPLORER_TITLE_TEXT_HALIGN) 
        {
            case ICONEXPLORER_TITLE_TEXT_LEFT:
                OffsetX = 0;
                break;
            case ICONEXPLORER_TITLE_TEXT_RIGHT:
                OffsetX = (T_S16)(pRect->width - width);
                break;
            case ICONEXPLORER_TITLE_TEXT_HCENTER:
            default:
                OffsetX = (T_S16)(pRect->width - width)/2;
                break;
        }
    }
    *PosX = pRect->left + OffsetX;

    if (pRect->height > FONT_HEIGHT_DEFAULT) 
    {
        switch (textStyle & ICONEXPLORER_TITLE_TEXT_VALIGN) 
        {
            case ICONEXPLORER_TITLE_TEXT_UP:
                OffsetY = 0;
                break;
            case ICONEXPLORER_TITLE_TEXT_DOWN:
                OffsetY = pRect->height - FONT_HEIGHT_DEFAULT;
                break;
            case ICONEXPLORER_TITLE_TEXT_VCENTER:
            default:
                OffsetY = (pRect->height-FONT_HEIGHT_DEFAULT)/2;
                break;
        }
    }
    *PosY = pRect->top + OffsetY;

    return AK_TRUE;
}

/**
 * @brief   load image data
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_IMAGE *pItemImage: T_ITEM_IMAGE struct pointer, save image data pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL MultiSet_LoadImageData(T_MULTISET *pMultiSet)
{   
    T_MULTISET_TITLE    *pMainTitle     = AK_NULL;
    T_DISP_BLOCK    *pBackGround    = AK_NULL;
//    T_DISP_BLOCK    *pStaticBar     = AK_NULL;
//    T_DISP_BLOCK    *pActiveBar     = AK_NULL;
    T_EDIT_AREA     *pEditArea      = AK_NULL;
    T_ITEM_IMAGE    *pItemImage     = AK_NULL;
    T_OPTION_IMAGE  *pOptionImage   = AK_NULL;
    T_U32           i = 0;

    if (AK_NULL == pMultiSet)
    {
        return AK_FALSE;
    }

    pMainTitle      = &pMultiSet->mainTitle;
    pBackGround     = &pMultiSet->backGround;
    pEditArea       = &pMultiSet->editArea;
    pItemImage      = &pMultiSet->itmImgDt;
    pOptionImage    = &pMultiSet->optnImgDt;

    //main title
    pMainTitle->pBackData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_TITLE, AK_NULL);

    //normal mode background 
    pBackGround->pBackData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MAIN_BACKGROUND, AK_NULL);

    //setting mode background 
    pEditArea->pBackData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_EDITFRAME, AK_NULL);
    pEditArea->txtBckDt.pImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_EDIT_TEXT_BCKGRND, AK_NULL);
    pEditArea->txtBckDt.pFcsImgDt = AK_NULL;
    pEditArea->pButton = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_EDIT_OK, AK_NULL);
    for (i = 0; i<4; i++)
    {
        pEditArea->pIcon[ICON_LEFT+i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_EDIT_ICON_LEFT+i, AK_NULL);
    }

    //item back image data
    pItemImage->itmBkDt.pImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_ITEM, AK_NULL);
    pItemImage->itmBkDt.pFcsImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_ITEM_FOCUS, AK_NULL);
    
    //item attribute image data
    pItemImage->attrbBkDt.pImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_ATTRIBUTE, AK_NULL);
    pItemImage->attrbBkDt.pFcsImgDt= pItemImage->attrbBkDt.pImgDt;

    //option back image data
    pOptionImage->optnBkDt.pImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_OPTION, AK_NULL);
    pOptionImage->optnBkDt.pFcsImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_OPTION_FOCUS, AK_NULL);

    //option button
    pOptionImage->radioBtDt.pImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_RADIOBT_NOTSETUP, AK_NULL);
    pOptionImage->radioBtDt.pFcsImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_RADIOBT_NOTSETUP_FOCUS, AK_NULL);
    pOptionImage->radioBtDt_Setup.pImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_RADIOBT_SETUP, AK_NULL);
    pOptionImage->radioBtDt_Setup.pFcsImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_RADIOBT_SETUP_FOCUS, AK_NULL);        

    //check box
    pOptionImage->chkBxDt.pImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_CHKBX_NOTSETUP, AK_NULL);
    pOptionImage->chkBxDt.pFcsImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_CHKBX_NOTSETUP_FOCUS, AK_NULL);
    pOptionImage->chkBxDt_Setup.pImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_CHKBX_SETUP, AK_NULL);
    pOptionImage->chkBxDt_Setup.pFcsImgDt = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_CHKBX_SETUP_FOCUS, AK_NULL);        

    //scroll bar
    //pStaticBar->pBackData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_SCRBAR_STATIC, AK_NULL);
    //pActiveBar->pBackData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_SCRBAT_ACTIVE, AK_NULL);

    ScBar_LoadImageData(&pMultiSet->scrBar);

    return AK_TRUE; 
}
/*
/////////////////////////////////////////////////////////////////////////////////////////////
//          static function area
//////////////////////////////////////////////////////////////////////////////////////////////
*/
/**
 * @brief   init main title
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET_TITLE *pTitle: T_MULTISET_TITLE struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_TitleInit(T_MULTISET_TITLE *pTitle)
{
    //T_IMAGE_DATA    *pImageData = AK_NULL;

    AK_ASSERT_PTR(pTitle, "MultiSet_TitleInit(): pTitle", AK_FALSE);

    pTitle->rect.left   = 0;
    pTitle->rect.top    = 0;
    pTitle->rect.width  = MULTISET_TITLE_WIDTH;
    pTitle->rect.height = TOP_BAR_HEIGHT;
    pTitle->backColor = COLOR_BLUE;

    if (AK_NULL != pTitle->pBackData)
    {
        AKBmpGetInfo((T_pCDATA)pTitle->pBackData, &pTitle->rect.width, &pTitle->rect.height, AK_NULL);
    }

    pTitle->content[0] = UNICODE_END;
    pTitle->textColor = COLOR_BLACK;
    pTitle->textStyle = MULTISET_TEXT_HCENTER | MULTISET_TEXT_VCENTER;
    
    return AK_TRUE;
}

/**
 * @brief   init item menu background
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_DISP_BLOCK *pDispBlock: T_DISP_BLOCK struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_BackGroundInit(T_DISP_BLOCK *pDispBlock)
{
    //T_pDATA     pImgData = AK_NULL;

    AK_ASSERT_PTR(pDispBlock, "MultiSet_BackGroundInit(): pDispBlock", AK_FALSE);

    pDispBlock->rect.left  = 0;
    pDispBlock->rect.top   = 0;
    pDispBlock->rect.width  = MULTISET_ITEMMENU_WIDTH;
    pDispBlock->rect.height = MULTISET_ITEMMENU_HEIGHT;    
    pDispBlock->backColor   = COLOR_WHITE;
    
    if (AK_NULL != pDispBlock->pBackData)
    {
        AKBmpGetInfo((T_pCDATA)pDispBlock->pBackData, &pDispBlock->rect.width, &pDispBlock->rect.height, AK_NULL);
    }

    pDispBlock->rect.width = (pDispBlock->rect.width <= Fwl_GetLcdWidth()) ? pDispBlock->rect.width : Fwl_GetLcdWidth(); 
    pDispBlock->rect.height= (pDispBlock->rect.height <= Fwl_GetLcdHeight()) ? pDispBlock->rect.height: Fwl_GetLcdHeight(); 
    pDispBlock->rect.left  = (Fwl_GetLcdWidth() - pDispBlock->rect.width)/2;
    pDispBlock->rect.top   = (Fwl_GetLcdHeight() - pDispBlock->rect.height)/2;

    return AK_TRUE;
}

/**
 * @brief   init edit area
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_EDIT_AREA *pEditArea: T_EDIT_AREA struct pointer
 * @param   T_IMAGE_DATA *pAttribImgDt: T_IMAGE_DATA struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_EditAreaInit(T_EDIT_AREA *pEditArea)
{
    T_LEN   temp = 0;
    
    AK_ASSERT_PTR(pEditArea, "MultiSet_EditAreaInit(): pEditArea", AK_FALSE);
    
    pEditArea->rect.left    = 0;
    pEditArea->rect.top     = TOP_BAR_HEIGHT;
    pEditArea->rect.width   = MULTISET_EDITAREA_WIDTH;
    pEditArea->rect.height  = MULTISET_EDITAREA_HEIGHT;
    pEditArea->backColor    = COLOR_SKYBLUE;

    if (AK_NULL != pEditArea->pBackData)
    {
        AKBmpGetInfo((T_pCDATA)pEditArea->pBackData, &pEditArea->rect.width, &pEditArea->rect.height, AK_NULL);
        pEditArea->rect.left  = (Fwl_GetLcdWidth() - pEditArea->rect.width)/2;
    }

    //init title.
    pEditArea->title.rect.left      = pEditArea->rect.left + 20;
    pEditArea->title.rect.top       = pEditArea->rect.top + 9;
    pEditArea->title.rect.width     = MULTISET_TITLE_WIDTH;
    pEditArea->title.rect.height    = TOP_BAR_HEIGHT;
    pEditArea->title.textColor      = COLOR_WHITE;
    pEditArea->title.textStyle      = MULTISET_TEXT_LEFT | MULTISET_TEXT_UP;
    pEditArea->title.content[0]     = UNICODE_END;
    pEditArea->title.backColor      = COLOR_WHITE;
    pEditArea->title.pBackData      = AK_NULL;

    //init text rect.
    pEditArea->text.pText          = AK_NULL;
    pEditArea->text.textColor      = COLOR_WHITE;
    pEditArea->text.textStyle      = MULTISET_TEXT_HCENTER | MULTISET_TEXT_VCENTER;
    pEditArea->text.backColor      = COLOR_WHITE;

    if (AK_NULL != pEditArea->txtBckDt.pImgDt)
    {
        pEditArea->text.pBackData = &pEditArea->txtBckDt;
        AKBmpGetInfo((T_pCDATA)pEditArea->text.pBackData->pImgDt,\
            &pEditArea->text.rect.width, &pEditArea->text.rect.height, AK_NULL);
    }
    else
    {
        pEditArea->text.rect.width  = MULTISET_EDITTEXT_WIDTH;
        pEditArea->text.rect.height = MULTISET_EDITTEXT_HEIGHT;
        pEditArea->text.pBackData   = AK_NULL;
    }
    pEditArea->text.rect.left = pEditArea->rect.left + (pEditArea->rect.width  - pEditArea->text.rect.width) / 2;
    pEditArea->text.rect.top  = pEditArea->rect.top + (pEditArea->rect.height - pEditArea->text.rect.height) / 2;

    //icon left
    if (AK_NULL != pEditArea->pIcon[ICON_LEFT])
    {
        AKBmpGetInfo((T_pCDATA)pEditArea->pIcon[ICON_LEFT],\
            &pEditArea->IconRct[ICON_LEFT].width, &pEditArea->IconRct[ICON_LEFT].height, AK_NULL);
    }
    else
    {
        pEditArea->IconRct[ICON_LEFT].width = 0;
        pEditArea->IconRct[ICON_LEFT].height = 0;
    }
    pEditArea->IconRct[ICON_LEFT].left = pEditArea->text.rect.left - pEditArea->IconRct[ICON_LEFT].width - 6;
    pEditArea->IconRct[ICON_LEFT].top = pEditArea->text.rect.top \
        + (pEditArea->text.rect.height - pEditArea->IconRct[ICON_LEFT].height) / 2;

    //icon right
    if (AK_NULL != pEditArea->pIcon[ICON_RIGHT])
    {
        AKBmpGetInfo((T_pCDATA)pEditArea->pIcon[ICON_RIGHT],\
            &pEditArea->IconRct[ICON_RIGHT].width, &pEditArea->IconRct[ICON_RIGHT].height, AK_NULL);
    }
    else
    {
        pEditArea->IconRct[ICON_RIGHT].width = 0;
        pEditArea->IconRct[ICON_RIGHT].height = 0;
    }
    pEditArea->IconRct[ICON_RIGHT].left = pEditArea->text.rect.left + pEditArea->text.rect.width + 6;
    pEditArea->IconRct[ICON_RIGHT].top = pEditArea->IconRct[ICON_LEFT].top;

    //icon up
    if (AK_NULL != pEditArea->pIcon[ICON_UP])
    {
        AKBmpGetInfo((T_pCDATA)pEditArea->pIcon[ICON_UP],\
            &pEditArea->IconRct[ICON_UP].width, &pEditArea->IconRct[ICON_UP].height, AK_NULL);
    }
    else
    {
        pEditArea->IconRct[ICON_UP].width = 0;
        pEditArea->IconRct[ICON_UP].height = 0;
    }
    pEditArea->IconRct[ICON_UP].left = pEditArea->text.rect.left \
                + (pEditArea->text.rect.width - pEditArea->IconRct[ICON_UP].width)/2;
    pEditArea->IconRct[ICON_UP].top = pEditArea->text.rect.top - pEditArea->IconRct[ICON_UP].height - 6;

    //icon down
    if (AK_NULL != pEditArea->pIcon[ICON_DOWN])
    {
        AKBmpGetInfo((T_pCDATA)pEditArea->pIcon[ICON_DOWN],\
            &pEditArea->IconRct[ICON_DOWN].width, &pEditArea->IconRct[ICON_DOWN].height, AK_NULL);
    }
    else
    {
        pEditArea->IconRct[ICON_DOWN].width = 0;
        pEditArea->IconRct[ICON_DOWN].height = 0;
    }
    pEditArea->IconRct[ICON_DOWN].left = pEditArea->text.rect.left \
                + (pEditArea->text.rect.width - pEditArea->IconRct[ICON_DOWN].width)/2;
    pEditArea->IconRct[ICON_DOWN].top = pEditArea->text.rect.top + pEditArea->text.rect.height + 6;

    //button
    if (AK_NULL != pEditArea->pButton)
    {
        AKBmpGetInfo((T_pCDATA)pEditArea->pButton, &pEditArea->buttonRct.width, &pEditArea->buttonRct.height, AK_NULL);
    }
    else
    {
        pEditArea->buttonRct.width = 0;
        pEditArea->buttonRct.height = 0;
    }
    pEditArea->buttonRct.left = pEditArea->text.rect.left \
                + (pEditArea->text.rect.width - pEditArea->buttonRct.width)/2;
    temp = pEditArea->IconRct[ICON_DOWN].top + pEditArea->IconRct[ICON_DOWN].height;
    pEditArea->buttonRct.top = temp + (Fwl_GetLcdHeight() - temp - pEditArea->buttonRct.height)/2;

    //cursor block
    pEditArea->text.bCursor         = AK_TRUE;
    pEditArea->text.cursor.left     = 0;
    pEditArea->text.cursor.top      = 0;
    pEditArea->text.cursor.width    = g_Font.SCWIDTH;
    pEditArea->text.cursor.height   = g_Font.SCHEIGHT;
    pEditArea->text.cursorColor     = COLOR_BLUE;

    return AK_TRUE;  
}

/**
 * @brief   init scroll bar
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_SCROLL_BAR *pScrBar: T_SCROLL_BAR struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 *//*
static T_BOOL MultiSet_ScrBarInit(T_SCROLL_BAR *pScrBar)
{
    T_pDATA     pImgData = AK_NULL;

    AK_ASSERT_PTR(pScrBar, "MultiSet_ScrBarInit(): pScrBar", AK_FALSE);

    //static scroll bar
    pScrBar->staticRect.rect.left   = Fwl_GetLcdWidth() - MULTISET_STATIC_SCRBAR_WIDTH;
    pScrBar->staticRect.rect.top    = TOP_BAR_HEIGHT;
    pScrBar->staticRect.rect.width  = MULTISET_STATIC_SCRBAR_WIDTH;
    pScrBar->staticRect.rect.height = MULTISET_STATIC_SCRBAR_HEIGHT;
    pScrBar->staticRect.backColor = COLOR_BLUE;
    if (AK_NULL != pScrBar->staticRect.pBackData)
    {
        AKBmpGetInfo((T_pCDATA)pScrBar->staticRect.pBackData, &pScrBar->staticRect.rect.width, &pScrBar->staticRect.rect.height, AK_NULL);
    }

    //active scroll bar
    pScrBar->activeRect.rect.left   = pScrBar->staticRect.rect.left + 1;
    pScrBar->activeRect.rect.top    = TOP_BAR_HEIGHT;
    pScrBar->activeRect.rect.width  = MULTISET_ACTIVE_SCRBAR_WIDTH;
    pScrBar->activeRect.rect.height = MULTISET_ACTIVE_SCRBAR_HEIGHT;
    pScrBar->activeRect.backColor = COLOR_CYAN;
    if (AK_NULL != pScrBar->activeRect.pBackData)
    {
        AKBmpGetInfo((T_pCDATA)pScrBar->activeRect.pBackData, &pScrBar->activeRect.rect.width, &pScrBar->activeRect.rect.height, AK_NULL);
    }

    pScrBar->ScrBarIsValid = AK_FALSE;

    return AK_TRUE;  
}
*/
/**
 * @brief   init item area
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM *pItem: T_ITEM struct pointer
 * @param   T_ITEM_IMAGE *pItemImage: T_ITEM_IMAGE struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_ItemAreaInit(T_ITEM *pItem, T_ITEM_IMAGE *pItemImage)
{
    AK_ASSERT_PTR(pItem, "MultiSet_ItemAreaInit(): pItem", AK_FALSE);
    AK_ASSERT_PTR(pItemImage, "MultiSet_ItemAreaInit(): pItemImage", AK_FALSE);
    
    pItem->rect.width   = MULTISET_ITEM_WIDTH;
    pItem->rect.height  = MULTISET_ITEM_HEIGHT;
    pItem->backColor    = COLOR_SKYBLUE;
    pItem->pBackData    = &pItemImage->itmBkDt;

    if (AK_NULL != pItem->pBackData->pImgDt)
    {
        AKBmpGetInfo((T_pCDATA)pItem->pBackData->pImgDt, &pItem->rect.width, &pItem->rect.height, AK_NULL);
    }
    pItem->rect.left = (Fwl_GetLcdWidth() - pItem->rect.width - scrBarWidth) / 2;
    pItem->rect.top  = TOP_BAR_HEIGHT + MULTISET_ITEM_ROW_RINTVL;

    //init title.
    pItem->title.rect.left   = pItem->rect.left + 3;
    pItem->title.rect.top    = pItem->rect.top + 2;
    pItem->title.rect.width  = MULTISET_TITLE_WIDTH;
    pItem->title.rect.height = TOP_BAR_HEIGHT;
    pItem->title.content[0]  = UNICODE_END;
    pItem->title.textColor   = COLOR_WHITE;
    pItem->title.textStyle   = MULTISET_TEXT_LEFT | MULTISET_TEXT_UP;
    pItem->title.backColor   = COLOR_WHITE;
    pItem->title.pBackData   = AK_NULL;

    //init text.
    pItem->text.rect.width  = MULTISET_EDITTEXT_WIDTH;
    pItem->text.rect.height = MULTISET_EDITTEXT_HEIGHT;
    
    if (AK_NULL != pItemImage->attrbBkDt.pImgDt)
    {
        pItem->text.pBackData = &pItemImage->attrbBkDt;
        AKBmpGetInfo((T_pCDATA)pItemImage->attrbBkDt.pImgDt, &pItem->text.rect.width, &pItem->text.rect.height, AK_NULL);
    }
    else
    {
        pItem->text.pBackData = AK_NULL;
    }
    pItem->text.rect.left = pItem->rect.left + 3 * (pItem->rect.width - pItem->text.rect.width) / 4;
    pItem->text.rect.top  = pItem->rect.top + 29;         

    pItem->text.pText     = AK_NULL;
    pItem->text.textColor = COLOR_WHITE;
    pItem->text.textStyle = MULTISET_TEXT_HCENTER | MULTISET_TEXT_VCENTER;
    pItem->text.backColor = COLOR_WHITE;

    return AK_TRUE; 
}

/**
 * @brief   init option area
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ITEM_OPTION_NODE *pOptioNode: T_ITEM_OPTION_NODE struct pointer
 * @param   T_ITEM_TYPE itemType: item type
 * @param   T_OPTION_IMAGE *pOptionImage: option image data struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSeT_OptionAreaInit(T_ITEM_OPTION_NODE *pOptioNode, T_ITEM_TYPE itemType, T_OPTION_IMAGE *pOptionImage)
{
    T_MULTISET_OPTION    *pOption = AK_NULL;    
    T_S16               base_top = 0;

    AK_ASSERT_PTR(pOptioNode, "MultiSeT_OptionAreaInit(): pOption", AK_FALSE);
    AK_ASSERT_PTR(pOptionImage, "MultiSeT_OptionAreaInit(): pOption", AK_FALSE);

    base_top = 2 * TOP_BAR_HEIGHT + 3;
    
	pOption = &pOptioNode->option;

    pOption->rect.width = MULTISET_OPTION_WIDTH;
    pOption->rect.height= MULTISET_OPTION_HEIGHT;
    pOption->backColor  = COLOR_BLUE;
    pOption->pBkImgDt   = &pOptionImage->optnBkDt;
    pOption->choosed    = AK_FALSE;

    //back ground of radio button and check box 
    if (AK_NULL != pOption->pBkImgDt->pImgDt)
    {
        AKBmpGetInfo((T_pCDATA)pOption->pBkImgDt->pImgDt, &pOption->rect.width, &pOption->rect.height, AK_NULL);
    }
    pOption->rect.left = (Fwl_GetLcdWidth() - pOption->rect.width - scrBarWidth) / 2;
    pOption->rect.top   = (T_POS)(base_top + pOptioNode->Id * pOption->rect.height);

    //icon of radio button or check box 
    pOption->icon.rect.width    = MULTISET_OPTION_ICON;
    pOption->icon.rect.height   = MULTISET_OPTION_ICON;
    pOption->icon.backColor     = COLOR_WHITE;
    switch (itemType)
    {
        case ITEM_TYPE_RADIO:
            pOption->icon.pBackData[ICONSTATE_SETUP_NOT] = &pOptionImage->radioBtDt;
            pOption->icon.pBackData[ICONSTATE_SETUP] = &pOptionImage->radioBtDt_Setup;
            if (AK_NULL != pOptionImage->radioBtDt.pImgDt)
            {
                AKBmpGetInfo((T_pCDATA)pOptionImage->radioBtDt.pImgDt, 
                            &pOption->icon.rect.width, &pOption->icon.rect.height, AK_NULL);
            }
            break;
            
        case ITEM_TYPE_CHECK:
            pOption->icon.pBackData[ICONSTATE_SETUP_NOT] = &pOptionImage->chkBxDt;
            pOption->icon.pBackData[ICONSTATE_SETUP] = &pOptionImage->chkBxDt_Setup;
            if (AK_NULL != pOption->icon.pBackData[ICONSTATE_SETUP_NOT])
            {
                AKBmpGetInfo((T_pCDATA)pOptionImage->chkBxDt.pImgDt, 
                            &pOption->icon.rect.width, &pOption->icon.rect.height, AK_NULL);
            }
            break;
            
        default:
            pOption->icon.pBackData[ICONSTATE_SETUP_NOT] = AK_NULL;
            pOption->icon.pBackData[ICONSTATE_SETUP] = AK_NULL;
            break;
    }

    //if (pOption->rect.width / 4 > pOption->icon.rect.width)
    //{
     //   pOption->icon.rect.left = pOption->rect.left + 3 * pOption->rect.width / 8 - pOption->icon.rect.width / 2;
    //}
    //else
    //{
        pOption->icon.rect.left = pOption->rect.left + (pOption->rect.width / 4) - (pOption->icon.rect.width / 2);
    //}
	pOption->icon.rect.top = pOption->rect.top + (pOption->rect.height - pOption->icon.rect.height) / 2; 

    //title of option 
    pOption->title.rect.left    = pOption->rect.left + pOption->rect.width / 2;
    pOption->title.rect.top     = pOption->rect.top;
    pOption->title.rect.width   = pOption->rect.width / 2 ;
    pOption->title.rect.height  = pOption->rect.height;
    pOption->title.content[0]   = UNICODE_END;
    pOption->title.textColor    = COLOR_WHITE;
    pOption->title.textStyle    = MULTISET_TEXT_LEFT | MULTISET_TEXT_VCENTER;
    pOption->title.backColor    = COLOR_WHITE;
    pOption->title.pBackData    = AK_NULL;
    return AK_TRUE; 
}

/**
 * @brief   set option rect top value
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET_OPTION *pOption: T_MULTISET_OPTION struct pointer
 * @param   T_S16 option_top: option rect top value
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_SetOptionTop(T_MULTISET_OPTION *pOption, T_S16 option_top)
{
    AK_ASSERT_PTR(pOption, "MultiSeT_OptionAreaInit(): pOption", AK_FALSE);
    
    pOption->rect.top   = option_top;
    pOption->icon.rect.top = pOption->rect.top + ((pOption->rect.height - pOption->icon.rect.height) / 2); 
    pOption->title.rect.top     = pOption->rect.top;

    return AK_TRUE; 
}

/**
 * @brief   show all
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_ShowAll(T_MULTISET *pMultiSet)
{
    T_DISP_BLOCK *pBackGround = AK_NULL;

    AK_ASSERT_PTR(pMultiSet, "TimeSet_SetTitleText(): pMultiSet", AK_FALSE);

    //show item menu background image or back color
    pBackGround = &pMultiSet->backGround;
    if (AK_TRUE != MultiSet_ShowBackGround(&pBackGround->rect, \
                                pBackGround->backColor, pBackGround->pBackData))
    {
        return AK_FALSE;
    }

    //show item list
    if (AK_TRUE != MultiSet_ShowItem(pMultiSet)) 
    {
        return AK_FALSE;
    }

    //show scroll bar
    if (AK_TRUE != MultiSet_ShowScrBar(pMultiSet))
    {
        return AK_FALSE;
    }    

    return AK_TRUE;
}

/**
 * @brief   show item list
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_ShowItem(T_MULTISET *pMultiSet)
{
    T_ITEM_NODE *pItemNode  = AK_NULL;
    T_ITEM      *pItem      = AK_NULL;
    T_S16       baseTop = 0;
    T_S16       tItmHght = 0;
	T_U32		count = 0;
 //   T_BOOL      flag = AK_FALSE;
 //   T_pDATA     pImgData = AK_NULL;
    T_ITEM_OPTION_NODE  *pOptionNode = AK_NULL;
    
    AK_ASSERT_PTR(pMultiSet, "TimeSet_SetTitleText(): pMultiSet", AK_FALSE);

    pItemNode = pMultiSet->pFirstItem;
    pItem = &pItemNode->item;
    tItmHght = (T_S16)(pMultiSet->itemQtyPerPage * (pItem->rect.height + pMultiSet->itmInterval) - pMultiSet->itmInterval);
    baseTop = TOP_BAR_HEIGHT + (Fwl_GetLcdHeight()- TOP_BAR_HEIGHT - tItmHght) / 2;
    
    for (count = 0; ((count < pMultiSet->itemQtyPerPage) && (AK_NULL != pItemNode)); count++)
    {
        pItem = &pItemNode->item;
        pItem->rect.top = (T_POS)(baseTop + count * (pItem->rect.height + pMultiSet->itmInterval));
        pItem->title.rect.left= pItem->rect.left + 3;
        pItem->title.rect.top = pItem->rect.top + 2;
        pItem->text.rect.top  = pItem->rect.top + 29;

        //option button
        if (ITEM_TYPE_RADIO == pItemNode->itemType)
        {
            //attribute is the focus option text
            pOptionNode = pItemNode->pOptionHead;
            while (AK_NULL != pOptionNode)
            {
                if (AK_TRUE == pOptionNode->option.choosed)
                {
                    pItem->text.pText = pOptionNode->option.title.content;
                    break;
                }
                
                pOptionNode = pOptionNode->pNext;
            }

            if (AK_NULL == pOptionNode)
            {
                pItem->text.pText = AK_NULL;
            }
        }

        //the item is the focus item 
		if (pItemNode == pMultiSet->pItemFocus)
		{
		    //show item background image or back color
            if (AK_TRUE != MultiSet_ShowBackGround(&pItem->rect, \
                                pItem->backColor, pItem->pBackData->pFcsImgDt))
	        {
		        return AK_FALSE;
	        }

            //show item title
            if (AK_TRUE != MultiSet_ShowSubTitle(&pItem->title, AK_TRUE))
        	{
        		return AK_FALSE;
        	}

            //show item attribute
            MultiSet_ShowItemText(&pItem->text, AK_TRUE);
		}
        else
        {
            //show item background image or back color
            if (AK_TRUE != MultiSet_ShowBackGround(&pItem->rect, \
                                 pItem->backColor, pItem->pBackData->pImgDt))
	        {
		        return AK_FALSE;
	        }

            //show item title
            if (AK_TRUE != MultiSet_ShowSubTitle(&pItem->title, AK_FALSE))
        	{
        		return AK_FALSE;
        	}

            //show item attribute
            MultiSet_ShowItemText(&pItem->text, AK_FALSE);
        }

        pItemNode = pItemNode->pNext;
    }
    return AK_TRUE;
}

/**
 * @brief   show background image or back color
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_RECT *pRect: background fill rect
 * @param   T_COLOR backColor: back color
 * @param   T_pDATA pBackData: background image data pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_ShowBackGround(T_RECT *pRect, T_COLOR backColor, T_pDATA pBackData)    
{
    AK_ASSERT_PTR(pRect, "MultiSet_ShowBackGround(): pRect", AK_FALSE);

    //background image data not exist
    if (AK_NULL == pBackData)
    {
        Fwl_FillSolidRect(HRGB_LAYER, (T_U16)(pRect->left), (T_U16)(pRect->top), (T_U16)(pRect->width), (T_U16)(pRect->height), backColor);
    }
    else//background image data exist
    {
        if (AK_TRUE != Fwl_AkBmpDrawFromString(HRGB_LAYER, pRect->left, pRect->top, (T_pCDATA)pBackData, &g_Graph.TransColor, AK_FALSE))
		{
			Fwl_Print(C3, M_CTRL, "MultiSet_ShowBackGround(): false !!! ");
            return AK_FALSE;
		}
    }

    return AK_TRUE;
}

/**
 * @brief   show scroll bar
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: T_MULTISET struct pointer
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_ShowScrBar(T_MULTISET *pMultiSet)
{
    //T_SCROLL_BAR    *pScrBar = AK_NULL;
    
    AK_ASSERT_PTR(pMultiSet, "MultiSet_ShowScrBar(): pMultiSet", AK_FALSE);

    //pScrBar = &pMultiSet->scrollBar;
    
    if (AK_TRUE != pMultiSet->ScrBarIsValid)
    {
        return AK_TRUE;
    }

    ScBar_Show2(&pMultiSet->scrBar);
   /*
    if (AK_TRUE != MultiSet_ShowBackGround(&pScrBar->staticRect.rect, pScrBar->staticRect.backColor, pScrBar->staticRect.pBackData))
    {
        return AK_FALSE;
    }

    pScrBar->activeRect.rect.top = (T_POS)(TOP_BAR_HEIGHT + pMultiSet->pItemFocus->id * pScrBar->staticRect.rect.height / pMultiSet->itemQty);
    if ((pMultiSet->pItemFocus->id == pMultiSet->itemQty - 1)
        ||((pScrBar->activeRect.rect.top + pScrBar->activeRect.rect.height) > Fwl_GetLcdHeight()))
    {
        pScrBar->activeRect.rect.top = Fwl_GetLcdHeight() - pScrBar->activeRect.rect.height;
    }
    
    if (AK_TRUE != MultiSet_ShowBackGround(&pScrBar->activeRect.rect, pScrBar->activeRect.backColor, pScrBar->activeRect.pBackData))
    {
        return AK_FALSE;
    }
*/
    return AK_TRUE;
}  

/**
 * @brief   show sub title
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET_TITLE *pTitle: T_MULTISET_TITLE struct pointer
 * @param   T_BOOL flag: focus flag 
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_ShowSubTitle(T_MULTISET_TITLE *pTitle, T_BOOL flag)    
{
    T_S16 PosX, PosY;
    
    AK_ASSERT_PTR(pTitle, "MultiSet_ShowSubTitle(): pTitle", AK_FALSE);

	if (AK_TRUE != MultiSet_GetTextPos(pTitle->content, &pTitle->rect, pTitle->textStyle, &PosX, &PosY))
    {
        return AK_FALSE;
    }

    if (AK_TRUE == flag)
    {
        Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pTitle->content, ~(pTitle->textColor), CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pTitle->content));
    }
    else
    {
        Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pTitle->content, pTitle->textColor, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pTitle->content));
    }
    
    return AK_TRUE;
}

/**
 * @brief   show text
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TEXT *pText: T_TEXT struct pointer
 * @param   T_BOOL flag: focus flag 
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_ShowItemText(T_TEXT *pText, T_BOOL flag)    
{
    T_S16       PosX, PosY;

    AK_ASSERT_PTR(pText, "TimeSet_ShowText():pText", AK_FALSE);

    if (AK_TRUE == flag)
    {
        if (AK_TRUE != MultiSet_ShowBackGround(&pText->rect, pText->backColor, pText->pBackData->pFcsImgDt))    
        {
            return AK_FALSE;
        }
        
        if (AK_TRUE != MultiSet_GetTextPos(pText->pText, &pText->rect, pText->textStyle, &PosX, &PosY))
        {
            return AK_FALSE;
        }

        Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pText->pText, ~(pText->textColor), CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pText->pText));
    }
    else
    {
        if (AK_TRUE != MultiSet_ShowBackGround(&pText->rect, pText->backColor, pText->pBackData->pImgDt))    
        {
            return AK_FALSE;
        }
        
    	if (AK_TRUE != MultiSet_GetTextPos(pText->pText, &pText->rect, pText->textStyle,&PosX, &PosY))
    	{
            return AK_FALSE;
        }

        Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pText->pText, pText->textColor, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pText->pText));
    }

    return AK_TRUE;
}

/**
 * @brief   show edit area
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: multiset struct pionter
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_ShowEditArea(T_MULTISET *pMultiSet)
{
    T_EDIT_AREA *pEditArea  = AK_NULL;
    T_U32       i = 0;
    
    AK_ASSERT_PTR(pMultiSet, "TimeSet_SetTitleText(): pMultiSet", AK_FALSE);

    pEditArea = &pMultiSet->editArea;

    //display edit area back color or background image 
    if (AK_TRUE != MultiSet_ShowBackGround(&pEditArea->rect, pEditArea->backColor, pEditArea->pBackData))
    {
        return AK_FALSE;
    }

    //set edit area subtitle content and edit text content
    Utl_UStrCpy(pEditArea->title.content, pMultiSet->pItemFocus->item.title.content);
    pEditArea->text.pText = pMultiSet->pItemFocus->item.text.pText;

    if (AK_TRUE != MultiSet_ShowSubTitle(&pEditArea->title, AK_FALSE))
	{
		return AK_FALSE;
	}

    if (AK_FALSE == pEditArea->text.bCursor) 
    {
        MultiSet_ShowItemText(&pEditArea->text, AK_FALSE);
    }
    else    
    {
        if (AK_NULL != pEditArea->pBackData)
        {
/*            Fwl_AkBmpDrawFromString(HRGB_LAYER, pEditArea->rect.left, pEditArea->rect.top, \
                (T_pCDATA)pEditArea->pBackData, &g_Graph.TransColor, AK_FALSE);
*/
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pEditArea->text.rect.left, pEditArea->text.rect.top, \
                (T_pCDATA)pEditArea->text.pBackData->pImgDt, &g_Graph.TransColor, AK_FALSE);

        }

        if (AK_NULL != pEditArea->pButton)
        {
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pEditArea->buttonRct.left, pEditArea->buttonRct.top,
                (T_pCDATA)pEditArea->pButton, &g_Graph.TransColor, AK_FALSE);
        }
        
        for (i=0; i < 4; i++)
        {
             Fwl_AkBmpDrawFromString(HRGB_LAYER, 
                 pEditArea->IconRct[ICON_LEFT+i].left, pEditArea->IconRct[ICON_LEFT+i].top, 
                (T_pCDATA)pEditArea->pIcon[ICON_LEFT+i], &g_Graph.TransColor, AK_FALSE);
        }
        
        //call back
        if (AK_NULL != pMultiSet->pItemFocus->editShowCallFunc)
        {
            pMultiSet->pItemFocus->editShowCallFunc(&pEditArea->text);
        }
    }

    return AK_TRUE;
}

/**
 * @brief   show option area
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MULTISET *pMultiSet: multiset struct pionter
 * @return  T_BOOL: 
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL MultiSet_ShowOptionArea(T_MULTISET *pMultiSet)
{
    T_EDIT_AREA         *pEditArea  = AK_NULL;
    T_ITEM_OPTION_NODE  *pOptionNode = AK_NULL;
    T_U32               count = 0;
    T_S16               base_top = 0;
    T_S16               base_option_top = 0;    
    T_S16               option_top = 0;
	T_S16               totleHeight = 0;
    T_ICON              *pIcon = AK_NULL;
    T_MULTISET_OPTION            *pOption = AK_NULL;
    
    AK_ASSERT_PTR(pMultiSet, "TimeSet_SetTitleText(): pMultiSet", AK_FALSE);

    pEditArea = &pMultiSet->editArea;

    //display background
    if (AK_TRUE != MultiSet_ShowBackGround(&pEditArea->rect, pEditArea->backColor, pEditArea->pBackData))
    {
        return AK_FALSE;
    }

    //display sub title
    Utl_UStrCpy(pEditArea->title.content, pMultiSet->pItemFocus->item.title.content);
    if (AK_TRUE != MultiSet_ShowSubTitle(&pEditArea->title, AK_FALSE))
	{
		return AK_FALSE;
	}

    pOptionNode = pMultiSet->pItemFocus->pOptionHead;
    
    if (pMultiSet->pItemFocus->optionQty <= MULTISET_OPTION_QTY_PER_PAGE)
    {
        totleHeight = (T_POS)(pMultiSet->pItemFocus->optionQty * pOptionNode->option.rect.height 
                        + (pMultiSet->pItemFocus->optionQty - 1) * pMultiSet->optnInterval);
        base_top = 2 * TOP_BAR_HEIGHT + 4;
        base_option_top = base_top + ((Fwl_GetLcdHeight() - base_top - totleHeight - 4) >> 1); 
    }
    else
    {
        totleHeight = (T_POS)(MULTISET_OPTION_QTY_PER_PAGE * pOptionNode->option.rect.height 
                        + (MULTISET_OPTION_QTY_PER_PAGE - 1) * pMultiSet->optnInterval);
        base_top = 2 * TOP_BAR_HEIGHT + 4;
        base_option_top = base_top;
    }

    //display option
    count = 0;
    while (AK_NULL != pOptionNode)
    {
        option_top =  (T_POS)(base_option_top + count * (pOptionNode->option.rect.height + pMultiSet->optnInterval));
        MultiSet_SetOptionTop(&pOptionNode->option, option_top);

        //the option is focus option
        if (pOptionNode == pMultiSet->pItemFocus->pOptionFocus)
        {
            pOption = &pOptionNode->option;
            pIcon = &pOption->icon;

            // display option back color or background image
        	if (AK_TRUE != MultiSet_ShowBackGround(&pOption->rect, pOption->backColor, pOption->pBkImgDt->pFcsImgDt))
        	{
        		return AK_FALSE;
        	}

            // the option is setup
            if (AK_TRUE == pOption->choosed) 
            {
            	if (AK_TRUE != MultiSet_ShowBackGround(&pIcon->rect, pIcon->backColor, pIcon->pBackData[ICONSTATE_SETUP]->pFcsImgDt))
            	{
            		return AK_FALSE;
            	}
            }
            else
            {
            	if (AK_TRUE != MultiSet_ShowBackGround(&pIcon->rect, pIcon->backColor, pIcon->pBackData[ICONSTATE_SETUP_NOT]->pFcsImgDt))
            	{
            		return AK_FALSE;
            	}
            }
            
            if (AK_TRUE != MultiSet_ShowSubTitle(&pOption->title, AK_TRUE))
        	{
        		return AK_FALSE;
        	}
		}
        else 
        {
            pOption = &pOptionNode->option;
            pIcon = &pOption->icon;
            
        	if (AK_TRUE != MultiSet_ShowBackGround(&pOption->rect, pOption->backColor, pOption->pBkImgDt->pImgDt))
        	{
        		return AK_FALSE;
        	}
            
            if (AK_TRUE == pOptionNode->option.choosed)
            {
                if (AK_TRUE != MultiSet_ShowBackGround(&pIcon->rect, pIcon->backColor, pIcon->pBackData[ICONSTATE_SETUP]->pImgDt))
                {
                    return AK_FALSE;
                }
            }
            else
            {
                if (AK_TRUE != MultiSet_ShowBackGround(&pIcon->rect, pIcon->backColor, pIcon->pBackData[ICONSTATE_SETUP_NOT]->pImgDt))
                {
                    return AK_FALSE;
                }
            }

            if (AK_TRUE != MultiSet_ShowSubTitle(&pOptionNode->option.title, AK_FALSE))
        	{
        		return AK_FALSE;
        	}
        }
        count++;
        pOptionNode = pOptionNode->pNext;
    }

    return AK_TRUE;
}
#endif
