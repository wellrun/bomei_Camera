#include "Eng_Topbar.h"
#include "Eng_DynamicFont.h"
#include "Eng_font.h"
#include "Eng_AkBmp.h"
#include "Eng_String_UC.h"
#include "Fwl_pfDisplay.h"
#include "Fwl_pfKeypad.h"
#include "Ctl_Iconlist.h"

static T_BOOL IconList_SetSlipItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItemStart, T_U32 startIndex, T_S32 count)
{
	T_ICONEXPLORER_ITEM *pItem = pItemStart;
	T_U32 i = 0;
	T_U16 uOptionText[ICONEXPLORER_ITEMOPTION_STRLEN + 1] = {0};
	T_U32 num = 0;
	T_U32 index = 0;

	if (AK_NULL == pIconExplorer
		|| AK_NULL == pIconExplorer->pSlipMgr)
	{
		return AK_FALSE;
	}

	num = count > 0 ? count : 0 - count;

	for (i=0; (i<num)&&(AK_NULL!=pItem); i++)
	{
		index = startIndex + i;
		
		switch (pItem->OptionType) 
		{
        case ICONEXPLORER_OPTION_LIST:
            if (pItem->pOptionHead != AK_NULL
				&& pItem->pOptionFocus != AK_NULL) 
			{
                SlipMgr_SetItem(pIconExplorer->pSlipMgr, index, pItem->Id, pItem->pSmallIcon, AK_NULL, pItem->pText, AK_NULL, pItem->pOptionFocus->Text);
            }
            break;
			
        case ICONEXPLORER_OPTION_NEXT:
            Eng_StrMbcs2Ucs(" >>", uOptionText);
			SlipMgr_SetItem(pIconExplorer->pSlipMgr, index, pItem->Id, pItem->pSmallIcon, AK_NULL, pItem->pText, AK_NULL, uOptionText);
            break;
			
        case ICONEXPLORER_OPTION_NONE:
        default:
			SlipMgr_SetItem(pIconExplorer->pSlipMgr, index, pItem->Id, pItem->pSmallIcon, AK_NULL, pItem->pText, AK_NULL, AK_NULL);
            break;
		}
		
		pItem = pItem->pNext;
	}

	SlipMgr_AddLoadItemNum(pIconExplorer->pSlipMgr, count);
	
	return AK_TRUE;
}

// get small item text pos
static T_BOOL IconList_GetSmallItemTextPos(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem, T_U32 Row, T_S16 *PosX, T_S16 *PosY, T_U32 *MaxLen)
{
    T_S16 OffsetX = 0, OffsetY = 0;

    if (pIconExplorer == AK_NULL
		|| pItem->pText == AK_NULL
		|| Row >= pIconExplorer->SmallItemRow)
        return AK_FALSE;

    *MaxLen = pIconExplorer->ItemRect.width - pIconExplorer->SmallIconWidth - pIconExplorer->ScrollBarShowWidth - \
            pIconExplorer->ItemFrameWidth*2 - pIconExplorer->SmallItemHInterval - pIconExplorer->SmallItemTInterval;

    *PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->SmallIconWidth + pIconExplorer->ItemFrameWidth + \
            pIconExplorer->SmallItemHInterval + pIconExplorer->SmallItemTInterval + OffsetX);

    if (pIconExplorer->SmallIconHeight > (T_U32)FONT_HEIGHT_DEFAULT)
        OffsetY = (T_S16)((pIconExplorer->SmallIconHeight-FONT_HEIGHT_DEFAULT)/2);
	
    *PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->SmallItemVInterval + \
            (pIconExplorer->SmallItemVInterval+pIconExplorer->SmallIconHeight)*Row + OffsetY);

    return AK_TRUE;
}

// get large item text pos
static T_BOOL IconList_GetLargeItemTextPos(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem, T_U32 Col, T_U32 Row, T_S16 *PosX, T_S16 *PosY, T_U32 *MaxLen)
{
    T_S16 OffsetX = 0, OffsetY = 0;
    T_U32   ustr_width = 0;

    if (pIconExplorer == AK_NULL
		|| pItem->pText == AK_NULL
		|| Col >= pIconExplorer->LargeItemCol
		|| Row >= pIconExplorer->LargeItemRow)
        return AK_FALSE;

    *MaxLen = pIconExplorer->LargeIconWidth;

    ustr_width = UGetSpeciStringWidth(pItem->pText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pItem->pText));

    if (ustr_width <= (*MaxLen))
        OffsetX = (T_S16)((pIconExplorer->LargeIconWidth - ustr_width) / 2);

    *PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth + pIconExplorer->LargeItemHInterval + \
            (pIconExplorer->LargeItemHInterval+pIconExplorer->LargeIconWidth)*Col + OffsetX);

    if (pIconExplorer->LargeTextHeight > (T_U32)FONT_HEIGHT_DEFAULT)
        OffsetY = (T_S16)((pIconExplorer->LargeTextHeight - FONT_HEIGHT_DEFAULT) / 2);

    *PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + \
            pIconExplorer->LargeIconHeight + pIconExplorer->LargeItemVInterval + pIconExplorer->LargeItemTInterval + \
            (pIconExplorer->LargeItemVInterval+pIconExplorer->LargeItemTInterval+pIconExplorer->LargeIconHeight+pIconExplorer->LargeTextHeight)*Row + \
            OffsetY);

    return AK_TRUE;
}

// get none item text pos
static T_BOOL IconList_GetNoneItemTextPos(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem, T_U32 Row, T_S16 *PosX, T_S16 *PosY, T_U32 *MaxLen)
{
    T_S16 OffsetX = 0, OffsetY = 0;

    if (pIconExplorer == AK_NULL
		|| pItem->pText == AK_NULL
		|| Row >= pIconExplorer->NoneItemRow)
        return AK_FALSE;

    *MaxLen = pIconExplorer->ItemRect.width - pIconExplorer->ScrollBarShowWidth - \
            pIconExplorer->ItemFrameWidth*2 - pIconExplorer->NoneItemHInterval;

    *PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth + \
            pIconExplorer->NoneItemHInterval + OffsetX);

    if (pIconExplorer->NoneTextHeight > (T_U32)FONT_HEIGHT_DEFAULT)
        OffsetY = (T_S16)((pIconExplorer->NoneTextHeight-FONT_HEIGHT_DEFAULT)/2);
	
    *PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->NoneItemVInterval + \
            (pIconExplorer->NoneItemVInterval+pIconExplorer->NoneTextHeight)*Row + OffsetY);

    return AK_TRUE;
}

static T_BOOL IconList_ShowItemText(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem, T_BOOL Focus, T_S16 PosX, T_S16 PosY, T_U32 MaxLen)
{
    T_U16 StringLen, ItemLen, uOptionLen = 0;
    T_U16 uOptionText[ICONEXPLORER_ITEMOPTION_STRLEN + 1] = {0};
    T_USTR_INFO uItemText = {0};
    T_USTR_INFO uTmpItemText = {0};
    T_U16 vch = 0x003e;    // vch is '>'
    T_U32 ItemTextOffset = 0;

    if (pIconExplorer == AK_NULL
		|| pItem == AK_NULL)
        return AK_FALSE;

    switch (pItem->OptionType) 
	{
    case ICONEXPLORER_OPTION_LIST:
        if ((pItem->pOptionHead != AK_NULL) && (pItem->pOptionFocus != AK_NULL)) {
            Eng_StrMbcs2Ucs(" ", uOptionText);
            Utl_UStrCat(uOptionText, pItem->pOptionFocus->Text);
        }
        break;
		
    case ICONEXPLORER_OPTION_NEXT:
        Eng_StrMbcs2Ucs(" >>", uOptionText);
        break;
		
    case ICONEXPLORER_OPTION_NONE:
    default:
        break;
    }


    StringLen = (T_U16)Utl_UStrLen(uOptionText);
    if (StringLen > 0)
	{
        uOptionLen = (T_U16)UGetSpeciStringWidth(uOptionText, CURRENT_FONT_SIZE, StringLen);
    }

    ItemLen = (T_U16)(MaxLen - uOptionLen);

    Utl_UStrCpy(uItemText, pItem->pText);
    StringLen = Fwl_GetUStringDispNum( uItemText, (T_U16)Utl_UStrLen(uItemText), ItemLen, CURRENT_FONT_SIZE);

    if ((Focus == AK_TRUE) && (StringLen < Utl_UStrLen(uItemText)))
	{
        ItemTextOffset = pIconExplorer->ItemTextOffset;
        pIconExplorer->ItemTextOffset++;
    }

    StringLen = Fwl_GetUStringDispNum( uItemText+ItemTextOffset, (T_U16)Utl_UStrLen(uItemText+ItemTextOffset), ItemLen, CURRENT_FONT_SIZE);
    if (StringLen < Utl_UStrLen(uItemText+ItemTextOffset))
	{
        if (Focus) 
		{
            Utl_UStrMid(uTmpItemText, uItemText, (T_S16)ItemTextOffset, (T_S16)(ItemTextOffset+StringLen-1));
        }
        else 
		{
            Utl_UStrMid(uTmpItemText, uItemText, (T_S16)ItemTextOffset, (T_S16)(ItemTextOffset+StringLen-1-1));
            Utl_UStrCatChr(uTmpItemText, vch, 1);
        }
    }
    else 
	{
        Utl_UStrCpy(uTmpItemText, uItemText+ItemTextOffset);
        if (Focus)
            pIconExplorer->ItemTextOffset = 0;
    }
    
    if (Focus)
	{
        Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, uTmpItemText, pIconExplorer->ItemFocusTextColor, CURRENT_FONT_SIZE, StringLen);
        Fwl_UDispSpeciString(HRGB_LAYER, (T_U16)(PosX+ItemLen), PosY, uOptionText, pIconExplorer->ItemFocusTextColor, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uOptionText));
    }
    else 
	{
        Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, uTmpItemText, pIconExplorer->ItemTextColor, CURRENT_FONT_SIZE, StringLen);
        Fwl_UDispSpeciString(HRGB_LAYER, (T_U16)(PosX+ItemLen), PosY, uOptionText, pIconExplorer->ItemTextColor, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(uOptionText));
    }
	
    return AK_TRUE;
}

// show a small item
static T_BOOL IconList_ShowSmallItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem, T_U32 Row, T_BOOL Focus, T_BOOL ShowBack)
{
    T_RECT Rect;
    T_S16 AKBmpW, AKBmpH;
    T_U8 AKBmpD;
    T_S16 PosX, PosY;
    T_U32 OffsetX, OffsetY;
    T_U32 MaxLen;

    if (pIconExplorer == AK_NULL
		|| pItem == AK_NULL
		|| Row >= pIconExplorer->SmallItemRow)
        return AK_FALSE;

    // show item back
    if (ShowBack)
    {
        PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth);
        PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->SmallItemVInterval + \
                (pIconExplorer->SmallItemVInterval+pIconExplorer->SmallIconHeight)*Row);
        Rect.width = (T_S16)(pIconExplorer->ItemRect.width - pIconExplorer->ItemFrameWidth*2 - pIconExplorer->ScrollBarShowWidth);
        Rect.height = (T_S16)(pIconExplorer->SmallIconHeight);
		
        if (Focus)
        {
            Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemFocusBackColor);
        }
        else 
        {
            if (pIconExplorer->pItemBackData != AK_NULL)
            {
                Rect.left = PosX - pIconExplorer->ItemRect.left;
                Rect.top = PosY - pIconExplorer->ItemRect.top;
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, PosX, PosY, &Rect, \
                        pIconExplorer->pItemBackData, AK_NULL, AK_FALSE);
            }
            else
            {
                Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemBackColor);
            }
        }
    }
    else
    {
        if (Focus)
        {
            PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth);
            PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->SmallItemVInterval + \
                    (pIconExplorer->SmallItemVInterval+pIconExplorer->SmallIconHeight)*Row);
            Rect.width = (T_S16)(pIconExplorer->ItemRect.width - pIconExplorer->ItemFrameWidth*2 - pIconExplorer->ScrollBarShowWidth);
            Rect.height = (T_S16)(pIconExplorer->SmallIconHeight);
            Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemFocusBackColor);
        }
    }

    // show item small icon
    if (pItem->pSmallIcon != AK_NULL)
	{
        AKBmpGetInfo(pItem->pSmallIcon, &AKBmpW, &AKBmpH, &AKBmpD);
		
        // show icon align center
        if (pIconExplorer->SmallIconWidth > (T_U32)AKBmpW)
            OffsetX = (pIconExplorer->SmallIconWidth-AKBmpW)/2;
        else
            OffsetX = 0;
		
        if (pIconExplorer->SmallIconHeight > (T_U32)AKBmpH)
            OffsetY = (pIconExplorer->SmallIconHeight-AKBmpH)/2;
        else
            OffsetY = 0;
		
        PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth + pIconExplorer->SmallItemHInterval + OffsetX);
        PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->SmallItemVInterval + \
                (pIconExplorer->SmallItemVInterval+pIconExplorer->SmallIconHeight)*Row + OffsetY);
        Rect.left = 0;
        Rect.top = 0;
        Rect.width = (T_S16)pIconExplorer->SmallIconWidth;
        Rect.height = (T_S16)pIconExplorer->SmallIconHeight;
		
        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, PosX, PosY, &Rect, pItem->pSmallIcon, &pIconExplorer->ItemTransColor, AK_FALSE);
    }

    // show item text
    if (pItem->pText != AK_NULL)
	{
        // get show item text pos
        if (IconList_GetSmallItemTextPos(pIconExplorer, pItem, Row, &PosX, &PosY, &MaxLen))
            IconList_ShowItemText(pIconExplorer, pItem, Focus, PosX, PosY, MaxLen);
    }

    return AK_TRUE;
}

// show a large item
static T_BOOL IconList_ShowLargeItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem, T_U32 Col, T_U32 Row, T_BOOL Focus, T_BOOL ShowBack)
{
    T_RECT Rect;
    T_S16 AKBmpW, AKBmpH;
    T_U8 AKBmpD;
    T_S16 PosX, PosY;
    T_U32 OffsetX, OffsetY;
    T_U32 MaxLen;

    if (pIconExplorer == AK_NULL
		|| pItem == AK_NULL
		|| Col >= pIconExplorer->LargeItemCol
		|| Row >= pIconExplorer->LargeItemRow)
        return AK_FALSE;

    // show item back
    if (ShowBack) 
	{
        PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth + pIconExplorer->LargeItemHInterval + \
                (pIconExplorer->LargeItemHInterval+pIconExplorer->LargeIconWidth)*Col);
        PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->LargeItemVInterval + \
                (pIconExplorer->LargeItemVInterval+pIconExplorer->LargeItemTInterval+pIconExplorer->LargeIconHeight+pIconExplorer->LargeTextHeight)*Row);
        Rect.width = (T_S16)(pIconExplorer->LargeIconWidth);
        Rect.height = (T_S16)(pIconExplorer->LargeIconHeight + pIconExplorer->LargeTextHeight + pIconExplorer->LargeItemTInterval);
        if (Focus)
		{
            Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemFocusBackColor);
        }
        else
		{
            if (pIconExplorer->pItemBackData != AK_NULL)
			{
                Rect.left = PosX - pIconExplorer->ItemRect.left;
                Rect.top = PosY - pIconExplorer->ItemRect.top;
//                Rect.top = PosY ;
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, PosX, PosY, &Rect, \
                        pIconExplorer->pItemBackData, AK_NULL, AK_FALSE);
            }
            else 
			{
                Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemBackColor);
            }
        }
    }
    else if (Focus)
	{
        PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth + pIconExplorer->LargeItemHInterval + \
                (pIconExplorer->LargeItemHInterval+pIconExplorer->LargeIconWidth)*Col);
        PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->LargeItemVInterval + \
                (pIconExplorer->LargeItemVInterval+pIconExplorer->LargeItemTInterval+pIconExplorer->LargeIconHeight+pIconExplorer->LargeTextHeight)*Row);
        Rect.width = (T_S16)(pIconExplorer->LargeIconWidth);
        Rect.height = (T_S16)(pIconExplorer->LargeIconHeight + pIconExplorer->LargeTextHeight + pIconExplorer->LargeItemTInterval);
        Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemFocusBackColor);
    }

    // show item large icon
    if (pItem->pLargeIcon != AK_NULL)
	{
        AKBmpGetInfo(pItem->pLargeIcon, &AKBmpW, &AKBmpH, &AKBmpD);
        // show icon align center
        if (pIconExplorer->LargeIconWidth > (T_U32)AKBmpW)
            OffsetX = (pIconExplorer->LargeIconWidth-AKBmpW)/2;
        else
            OffsetX = 0;
		
        if (pIconExplorer->LargeIconHeight > (T_U32)AKBmpH)
            OffsetY = (pIconExplorer->LargeIconHeight-AKBmpH)/2;
        else
            OffsetY = 0;
		
        PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth + pIconExplorer->LargeItemHInterval + \
                (pIconExplorer->LargeItemHInterval+pIconExplorer->LargeIconWidth)*Col + OffsetX);
        PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->LargeItemVInterval + \
                (pIconExplorer->LargeItemVInterval+pIconExplorer->LargeItemTInterval+pIconExplorer->LargeIconHeight+pIconExplorer->LargeTextHeight)*Row + \
                OffsetY);
        Rect.left = 0;
        Rect.top = 0;
        Rect.width = (T_S16)pIconExplorer->LargeIconWidth;
        Rect.height = (T_S16)pIconExplorer->LargeIconHeight;
		
        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, PosX, PosY, &Rect, pItem->pLargeIcon, &pIconExplorer->ItemTransColor, AK_FALSE);
    }

    // show item text
    if (pItem->pText != AK_NULL)
	{
        // get show item text pos
        if (IconList_GetLargeItemTextPos(pIconExplorer, pItem, Col, Row, &PosX, &PosY, &MaxLen) == AK_TRUE)
            IconList_ShowItemText(pIconExplorer, pItem, Focus, PosX, PosY, MaxLen);
    }

    return AK_TRUE;
}

// show a none item
static T_BOOL IconList_ShowNoneItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem, T_U32 Row, T_BOOL Focus, T_BOOL ShowBack)
{
    T_RECT Rect;
    T_S16 PosX, PosY;
    T_U32 MaxLen;

    if (pIconExplorer == AK_NULL
		|| pItem == AK_NULL
		|| Row >= pIconExplorer->NoneItemRow)
        return AK_FALSE;

    // show item back
    if (ShowBack) 
	{
        PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth);
        PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->NoneItemVInterval + \
                (pIconExplorer->NoneItemVInterval+pIconExplorer->NoneTextHeight)*Row);
        Rect.width = (T_S16)(pIconExplorer->ItemRect.width - pIconExplorer->ItemFrameWidth*2 - pIconExplorer->ScrollBarShowWidth);
        Rect.height = (T_S16)(pIconExplorer->NoneTextHeight);
		
        if (Focus)
		{
            Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemFocusBackColor);
        }
        else 
		{
            if (pIconExplorer->pItemBackData != AK_NULL)
			{
                Rect.left = PosX - pIconExplorer->ItemRect.left;
                Rect.top = PosY - pIconExplorer->ItemRect.top;
//                Rect.top = PosY;

                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, PosX, PosY, &Rect, \
                        pIconExplorer->pItemBackData, AK_NULL, AK_FALSE);
            }
            else
			{
                Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemBackColor);
            }
        }
    }
    else if (Focus) 
	{
        PosX = (T_S16)(pIconExplorer->ItemRect.left + pIconExplorer->ItemFrameWidth);
        PosY = (T_S16)(pIconExplorer->ItemRect.top + pIconExplorer->ItemFrameWidth + pIconExplorer->NoneItemVInterval + \
                (pIconExplorer->NoneItemVInterval+pIconExplorer->NoneTextHeight)*Row);
        Rect.width = (T_S16)(pIconExplorer->ItemRect.width - pIconExplorer->ItemFrameWidth*2 - pIconExplorer->ScrollBarShowWidth);
        Rect.height = (T_S16)(pIconExplorer->NoneTextHeight);
        Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pIconExplorer->ItemFocusBackColor);
    }

    // show item text
    if (pItem->pText != AK_NULL)
	{
        // get show item text pos
        if (IconList_GetNoneItemTextPos(pIconExplorer, pItem, Row, &PosX, &PosY, &MaxLen) == AK_TRUE)
            IconList_ShowItemText(pIconExplorer, pItem, Focus, PosX, PosY, MaxLen);
    }

    return AK_TRUE;
}





/*****************************************************************************/


T_BOOL IconList_InsertTailItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *p)
{
	T_ICONEXPLORER_ITEM *q, *f;
	
	f = pIconExplorer->pItemHead;
    q = AK_NULL;
	
    while (f) 
	{
        if (f->Id == p->Id) 
		{
            if (p->pText)
                p->pText = Fwl_Free(p->pText);
			
            if (p->pContent)
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

	return AK_TRUE;
}

T_BOOL IconList_LoadSlipItem(T_ICONEXPLORER *pIconExplorer, T_S32 count, T_U32 loadItemNum)
{
	T_ICONEXPLORER_ITEM *p = AK_NULL;
	T_U32 i = 0;
	T_S32 startIndex = 0;
	T_U32 itemNum = 0;

	if (AK_NULL == pIconExplorer)
	{
		return AK_FALSE;
	}

	p = pIconExplorer->pItemHead;

	for (i=0; (p!=AK_NULL)&&(i<loadItemNum-1); i++)
	{
    	p = p->pNext;
	}

	itemNum = SlipMgr_GetItemNum(pIconExplorer->pSlipMgr);

	if (AK_NULL != p)
	{
		if (count > 0)
		{
			startIndex = SlipMgr_GetIndexById(pIconExplorer->pSlipMgr, p->Id) + 1;
			
			if (startIndex >= (T_S32)itemNum)
			{
				startIndex -= itemNum;
			}
			
			IconList_SetSlipItem(pIconExplorer, p->pNext, startIndex, count);
		}
		else
		{
			startIndex = SlipMgr_GetIndexById(pIconExplorer->pSlipMgr, p->Id) + count + 1;

			if (startIndex < 0)
			{
				startIndex += itemNum;
			}
			
			for (i=0; (p!=AK_NULL)&&(i<itemNum-count-1); i++)
			{
	        	p = p->pPrevious;
			}

			IconList_SetSlipItem(pIconExplorer, p, startIndex, count);
		}
	}

	return AK_TRUE;
}

T_BOOL IconList_CreatSlipMgr(T_ICONEXPLORER *pIconExplorer)
{
	T_U32 slipItemNum = 0;
	T_RECT rect;

	if (AK_NULL == pIconExplorer)
    {
        return AK_FALSE;
    }

	if (AK_NULL != pIconExplorer->pSlipMgr)
	{
		pIconExplorer->pSlipMgr = SlipMgr_Destroy(pIconExplorer->pSlipMgr);
	}

	RectInit(&rect, 0, pIconExplorer->TitleRect.height, Fwl_GetLcdWidth(), (T_LEN)(Fwl_GetLcdHeight() - pIconExplorer->TitleRect.height));
	
	pIconExplorer->pSlipMgr = SlipMgr_Creat(ITEM_TYPE_LIST, rect, Fwl_GetLcdWidth(), 
				SLIP_ITEM_HEIGHT, pIconExplorer->ItemQty, MOVETYPE_Y);

	if (AK_NULL == pIconExplorer->pSlipMgr)
    {
        return AK_FALSE;
    }

	slipItemNum = SlipMgr_GetItemNum(pIconExplorer->pSlipMgr);
	IconList_SetSlipItem(pIconExplorer, pIconExplorer->pItemHead, 0, slipItemNum);
	
	return AK_TRUE;
}


T_BOOL IconList_CheckSlipFocus(T_ICONEXPLORER *pIconExplorer)
{
	T_U32 focusId = 0;
	T_U32 index = 0;
	T_U32 slipItemNum = 0;
	T_S32 i = 0;
	T_ICONEXPLORER_ITEM *p = AK_NULL;
	T_U32 remainNum = 0;
	T_S32 count = 0;
	T_U32 loadItemNum = 0;
	
    if (AK_NULL == pIconExplorer)
    {
        return AK_FALSE;
    }

	if (ICONEXPLORER_CHANGE_NONE != pIconExplorer->ItemListFlag)
	{
		IconList_CreatSlipMgr(pIconExplorer);
		pIconExplorer->ItemListFlag = ICONEXPLORER_CHANGE_NONE;
	}


	focusId = IconExplorer_GetItemFocusId(pIconExplorer);
	
	if (SlipMgr_CheckFocusItem(pIconExplorer->pSlipMgr, focusId))
	{
		SlipMgr_CleanItemFocus(pIconExplorer->pSlipMgr);
		IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ALL);
		return AK_TRUE;
	}
	
	index = IconExplorer_GetItemIndexById(pIconExplorer, focusId);
	slipItemNum = SlipMgr_GetItemNum(pIconExplorer->pSlipMgr);

	if (index + slipItemNum <= pIconExplorer->ItemQty)
	{
		SlipMgr_ClearOffset(pIconExplorer->pSlipMgr, index);
		IconList_SetSlipItem(pIconExplorer, pIconExplorer->pItemFocus, 0, slipItemNum);
		SlipMgr_SetLoadItemNum(pIconExplorer->pSlipMgr, index + slipItemNum);
		SlipMgr_PrepareToShow(pIconExplorer->pSlipMgr, &count, &loadItemNum, 0, 0);
	}
	else
	{
		remainNum = index + slipItemNum - pIconExplorer->ItemQty;
		p = pIconExplorer->pItemFocus;
		
		for (i=remainNum;((i > 0) && (AK_NULL != p)); i--)
		{
			p = p->pPrevious;
		}

		SlipMgr_ClearOffset(pIconExplorer->pSlipMgr, index - remainNum);
		IconList_SetSlipItem(pIconExplorer, p, 0, slipItemNum);
		SlipMgr_SetLoadItemNum(pIconExplorer->pSlipMgr, pIconExplorer->ItemQty);

		if (!SlipMgr_CheckFocusItem(pIconExplorer->pSlipMgr, focusId))
		{
			SlipMgr_PrepareToShow(pIconExplorer->pSlipMgr, &count, &loadItemNum, Fwl_GetLcdHeight() - pIconExplorer->TitleRect.height - slipItemNum * SLIP_ITEM_HEIGHT, 0);
		}
	}

	IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ALL);
	
    return AK_TRUE;
}

T_VOID IconList_HitButton_Handler(T_ICONEXPLORER *pIconExplorer, T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pParam)
{
    T_RECT  rect;
#if 0
	T_RECT rect1;
    T_U32   Id = 0;
#endif
    T_POS   posX = 0;
    T_POS   posY = 0;

    if (AK_NULL == pIconExplorer
		|| AK_NULL == pPhyKey
		|| AK_NULL == pParam)
    {
        return;
    }

    posX = pParam->s.Param2;
    posY = pParam->s.Param3;

    //cancel button
    rect = TopBar_GetRectofCancelButton();
    if (PointInRect(&rect, posX, posY))
    {
        pPhyKey->keyID = kbCLEAR;
        pPhyKey->pressType = PRESS_SHORT;
        return;
    }

    //hit menu button icon
    if (TopBar_GetMenuButtonState()
		&& TopBar_GetMenuButtonShowState())
    {
        TopBar_GetRectofMenuButton(&rect);
        if (PointInRect(&rect, posX, posY))
        {
            pPhyKey->keyID = kbMENU;
            pPhyKey->pressType = PRESS_SHORT;
            return;
        }
    }

#if 0
    //hit up icon
    ScBar_GetUpIconRect(&rect, &pIconExplorer->ScrollBar);
    if (PointInRect(&rect, posX, posY))
    {
        pPhyKey->keyID = kbUP;
        pPhyKey->pressType = PRESS_SHORT;
        return;
    }

    //hit up icon
    ScBar_GetDownIconRect(&rect, &pIconExplorer->ScrollBar);
    if (PointInRect(&rect, posX, posY))
    {
        pPhyKey->keyID = kbDOWN;
        pPhyKey->pressType = PRESS_SHORT;
        return;
    }

    if(pIconExplorer->ScrollBarFlag){
    //scroll bar 
    rect = ScBar_GetRect(&pIconExplorer->ScrollBar);
    if (PointInRect(&rect, posX, posY))
    {
        ScBar_GetLocaRect(&rect1, &pIconExplorer->ScrollBar);

        if (posY < rect1.top)
        {
            pPhyKey->keyID = kbUP;
            pPhyKey->pressType = PRESS_SHORT;
        }
        else if (posY > rect1.top + rect1.height)
        {
            pPhyKey->keyID = kbDOWN;
            pPhyKey->pressType = PRESS_SHORT;
        }
        return;    
        }
    }

    //other
    Id = IconExplorer_GetIdByPoint(pIconExplorer, posX, posY);
    if ((AK_NULL != pIconExplorer->pItemFocus) && (Id == pIconExplorer->pItemFocus->Id)) 
    {
        pPhyKey->keyID = kbOK;
        pPhyKey->pressType = PRESS_SHORT;
        return;
    }
    else if (Id != ICONEXPLORER_ITEM_ID_ERROR)
    {
        IconExplorer_SetFocus(pIconExplorer, Id);
    }

#endif
}

T_eBACK_STATE IconList_UserKey_Handler(T_ICONEXPLORER *pIconExplorer, T_MMI_KEYPAD *pPhyKey)
{
    T_eBACK_STATE ret = eStay;

    if (AK_NULL == pIconExplorer
		|| AK_NULL == pPhyKey)
    {
        return ret;
    }

	if ((SLIPMSG_STA_STOP != SlipMgr_GetCurStatus(pIconExplorer->pSlipMgr))
		&& (kbCLEAR != pPhyKey->keyID))
	{
		IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ALL);
		return ret;
	}
    
    switch (pPhyKey->keyID) 
    {
    case kbOK:
        if (pPhyKey->pressType == PRESS_SHORT)
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
        else if (pPhyKey->pressType == PRESS_LONG)
        {
            Fwl_KeyStop();
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
        break;
        
    case kbCLEAR:
        if (pPhyKey->pressType == PRESS_SHORT) 
        {
            ret = eReturn;
        }
        else if (pPhyKey->pressType == PRESS_LONG)
        {
            Fwl_KeyStop();
            ret = eHome;
        }
        break;

    case kbMENU:
        if (pPhyKey->pressType == PRESS_SHORT) 
        {
            ret = eMenu;
        }
        break;
        
    case kbLEFT:
        if (pPhyKey->pressType == PRESS_SHORT) 
        {
            IconExplorer_MoveFocus(pIconExplorer, ICONEXPLORER_DIRECTION_LEFT);
        }
        break;
        
    case kbRIGHT:
        if (pPhyKey->pressType == PRESS_SHORT) 
        {
            IconExplorer_MoveFocus(pIconExplorer, ICONEXPLORER_DIRECTION_RIGHT);
        }
        break;
        
    case kbUP:
        if (pPhyKey->pressType == PRESS_SHORT)
        {
            IconExplorer_MoveFocus(pIconExplorer, ICONEXPLORER_DIRECTION_UP);
        }
        break;
        
    case kbDOWN:
        if (pPhyKey->pressType == PRESS_SHORT)
        {
            IconExplorer_MoveFocus(pIconExplorer, ICONEXPLORER_DIRECTION_DOWN);
        }
        break;
        
    default:
        break;
    }

    return ret;
}


// move focus item option
T_BOOL IconList_MoveItemOptionFocus(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *pItem;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    // get focus item
    pItem = IconExplorer_GetItemFocus(pIconExplorer);
    if (pItem == AK_NULL)
        return AK_FALSE;

    switch (pItem->OptionType)
	{
    case ICONEXPLORER_OPTION_LIST:
        if (pItem->pOptionHead == AK_NULL)
            return AK_FALSE;

        if (pItem->pOptionFocus == AK_NULL)
            pItem->pOptionFocus = pItem->pOptionHead;

        if (pItem->pOptionFocus->pNext == AK_NULL)
            pItem->pOptionFocus = pItem->pOptionHead;
        else
            pItem->pOptionFocus = pItem->pOptionFocus->pNext;

        if (pItem->pOptionValue != AK_NULL)
            *pItem->pOptionValue = pItem->pOptionFocus->Id;

		SlipMgr_ChangeTextRightById(pIconExplorer->pSlipMgr, pItem->Id, pItem->pOptionFocus->Text);

        IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_FOCUS);
        return  AK_TRUE;
		
    case ICONEXPLORER_OPTION_NEXT:
    case ICONEXPLORER_OPTION_NONE:
    default:
        return AK_FALSE;
    }
}


// show item again
T_BOOL IconList_ReShowItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem)
{
    T_ICONEXPLORER_ITEM *p;
    T_U32 Col, Row;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    if (pItem == AK_NULL)
        return AK_FALSE;

    switch (pIconExplorer->ItemIconStyle)
	{
    case ICONEXPLORER_LARGEICON:
        // get item row, show it
        p = pIconExplorer->pItemShow;
        for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->LargeItemRow); Row++)
		{
            for (Col=0; (p!=AK_NULL)&&(Col<pIconExplorer->LargeItemCol); Col++)
			{
                if (p == pItem)
				{
                    if (p == pIconExplorer->pItemFocus)
                        IconList_ShowLargeItem(pIconExplorer, p, Col, Row, AK_TRUE, AK_TRUE);
                    else
                        IconList_ShowLargeItem(pIconExplorer, p, Col, Row, AK_FALSE, AK_TRUE);

                    break;
                }

                p = p->pNext;
            }

            if (Col < pIconExplorer->LargeItemCol)
                break;
        }
        break;
		
    case ICONEXPLORER_NONEICON:
        // get item row, show it
        p = pIconExplorer->pItemShow;
        for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->NoneItemRow); Row++)
		{
            if (p == pItem)
			{
                if (p == pIconExplorer->pItemFocus)
                    IconList_ShowNoneItem(pIconExplorer, p, Row, AK_TRUE, AK_TRUE);
                else
                    IconList_ShowNoneItem(pIconExplorer, p, Row, AK_FALSE, AK_TRUE);

                break;
            }

            p = p->pNext;
        }
        break;
		
    case ICONEXPLORER_SMALLICON:
    default:
        // get item row, show it
        p = pIconExplorer->pItemShow;
        for (Row=0; (p!=AK_NULL)&&(Row<pIconExplorer->SmallItemRow); Row++)
		{
            if (p == pItem)
			{
                if (p == pIconExplorer->pItemFocus)
                    IconList_ShowSmallItem(pIconExplorer, p, Row, AK_TRUE, AK_TRUE);
                else
                    IconList_ShowSmallItem(pIconExplorer, p, Row, AK_FALSE, AK_TRUE);

                break;
            }

            p = p->pNext;
        }
        break;
		
    }

    if (pItem == pIconExplorer->pItemFocus)
        pIconExplorer->ItemTextOffset = 0;

    return AK_TRUE;
}


// check show item & focus item
T_BOOL IconList_CheckShowFocus(T_ICONEXPLORER *pIconExplorer)
{
    T_U32 i;
    T_ICONEXPLORER_ITEM *p, *pOldItemShow;
    T_U32 Qty;
    T_BOOL Show = AK_FALSE;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    if (pIconExplorer->pItemHead == AK_NULL
		|| pIconExplorer->pItemFocus == AK_NULL) 
	{
        pIconExplorer->pItemShow = AK_NULL;
        return AK_FALSE;
    }

    pOldItemShow = pIconExplorer->pItemShow;

    if (pIconExplorer->pItemShow == AK_NULL)
        pIconExplorer->pItemShow = pIconExplorer->pItemHead;

    switch (pIconExplorer->ItemIconStyle)
	{
    case ICONEXPLORER_LARGEICON:
        Qty = 0;
        p = pIconExplorer->pItemHead;
        while (p != AK_NULL) {
            if (p == pIconExplorer->pItemShow)
                break;

            Qty++;
            p = p->pNext;
        }

        if (Qty%pIconExplorer->LargeItemCol != 0)
            pIconExplorer->pItemShow = pIconExplorer->pItemHead;

        for (i=0; i<pIconExplorer->ItemQty; i++)
		{
            Qty = 0;
            p = pIconExplorer->pItemFocus;
            while (p != AK_NULL) {
                Qty++;

                if (p == pIconExplorer->pItemShow)
				{
                    Show = AK_TRUE;
                    break;
                }

                if (Qty >= pIconExplorer->LargeItemCol*pIconExplorer->LargeItemRow)
                    break;

                p = p->pPrevious;
            }

            if (Show == AK_FALSE)
			{
                for (Qty=0; Qty<pIconExplorer->LargeItemCol; Qty++)
				{
                    if (pIconExplorer->pItemShow->pNext == AK_NULL)
					{
                        pIconExplorer->pItemShow = pIconExplorer->pItemHead;
                        break;
                    }

                    pIconExplorer->pItemShow = pIconExplorer->pItemShow->pNext;
                }
            }
            else
			{
                break;
            }
        }
        break;
		
    case ICONEXPLORER_NONEICON:
        for (i=0; i<pIconExplorer->ItemQty; i++)
		{
            Qty = 0;
            p = pIconExplorer->pItemFocus;
            while (p != AK_NULL)
			{
                Qty++;

                if (p == pIconExplorer->pItemShow)
				{
                    Show = AK_TRUE;
                    break;
                }

                if (Qty >= pIconExplorer->NoneItemRow)
                    break;

                p = p->pPrevious;
            }

            if (Show == AK_FALSE)
			{
                if (pIconExplorer->pItemShow->pNext == AK_NULL)
                    pIconExplorer->pItemShow = pIconExplorer->pItemFocus;
                else
                    pIconExplorer->pItemShow = pIconExplorer->pItemShow->pNext;
            }
            else 
			{
                Qty = 0;
                p = pIconExplorer->pItemShow;
                while (p != AK_NULL)
				{
                    Qty++;

                    if (Qty >= pIconExplorer->NoneItemRow)
                        break;

                    p = p->pNext;
                }

                for (; Qty<pIconExplorer->NoneItemRow; Qty++)
				{
                    if (pIconExplorer->pItemShow->pPrevious == AK_NULL)
                        break;

                    pIconExplorer->pItemShow = pIconExplorer->pItemShow->pPrevious;
                }
                break;
            }
        }
        break;
		
    case ICONEXPLORER_SMALLICON:
    default:
        for (i=0; i<pIconExplorer->ItemQty; i++) 
		{
            Qty = 0;
            p = pIconExplorer->pItemFocus;
            while (p != AK_NULL) 
			{
                Qty++;

                if (p == pIconExplorer->pItemShow)
				{
                    Show = AK_TRUE;
                    break;
                }

                if (Qty >= pIconExplorer->SmallItemRow)
                    break;

                p = p->pPrevious;
            }

            if (Show == AK_FALSE)
			{
                if (pIconExplorer->pItemShow->pNext == AK_NULL)
                    pIconExplorer->pItemShow = pIconExplorer->pItemFocus;
                else
                    pIconExplorer->pItemShow = pIconExplorer->pItemShow->pNext;
            }
            else 
			{
                Qty = 0;
                p = pIconExplorer->pItemShow;
                while (p != AK_NULL) 
				{
                    Qty++;

                    if (Qty >= pIconExplorer->SmallItemRow)
                        break;

                    p = p->pNext;
                }

                for (; Qty<pIconExplorer->SmallItemRow; Qty++) 
				{
                    if (pIconExplorer->pItemShow->pPrevious == AK_NULL)
                        break;

                    pIconExplorer->pItemShow = pIconExplorer->pItemShow->pPrevious;
                }
                break;
            }
        }
        break;
    }


    if (pIconExplorer->pItemShow != pOldItemShow)
        IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}

// check scroll bar show flag
T_BOOL IconList_CheckScrollBar(T_ICONEXPLORER *pIconExplorer)
{
    T_U32 OldScrollBarWidth;
    T_U32 OldLargeItemCol;

    if (pIconExplorer == AK_NULL)
        return AK_FALSE;

    OldScrollBarWidth = pIconExplorer->ScrollBarShowWidth;

    switch (pIconExplorer->ItemIconStyle)
	{
    case ICONEXPLORER_LARGEICON:
        if (pIconExplorer->ItemQty > (pIconExplorer->LargeItemRow*pIconExplorer->LargeItemCol))
		{
            pIconExplorer->ScrollBarShowWidth = pIconExplorer->ScrollBarWidth;
            pIconExplorer->ScrollBarFlag = AK_TRUE;
        }
        else 
		{
            pIconExplorer->ScrollBarShowWidth = 0;
            pIconExplorer->ScrollBarFlag = AK_FALSE;
        }
        break;
		
    case ICONEXPLORER_NONEICON:
        if (pIconExplorer->ItemQty > pIconExplorer->NoneItemRow)
		{
            pIconExplorer->ScrollBarShowWidth = pIconExplorer->ScrollBarWidth;
            pIconExplorer->ScrollBarFlag = AK_TRUE;
        }
        else 
		{
            pIconExplorer->ScrollBarShowWidth = 0;
            pIconExplorer->ScrollBarFlag = AK_FALSE;
        }
        break;
		
    case ICONEXPLORER_SMALLICON:
    default:
        if (pIconExplorer->ItemQty > pIconExplorer->SmallItemRow)
		{
            pIconExplorer->ScrollBarShowWidth = pIconExplorer->ScrollBarWidth;
            pIconExplorer->ScrollBarFlag = AK_TRUE;
        }
        else 
		{
            pIconExplorer->ScrollBarShowWidth = 0;
            pIconExplorer->ScrollBarFlag = AK_FALSE;
        }
        break;
    }

    if (pIconExplorer->ScrollBarShowWidth != OldScrollBarWidth) {
        OldLargeItemCol = pIconExplorer->LargeItemCol;

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

        if (pIconExplorer->LargeItemCol != OldLargeItemCol)
            IconList_CheckShowFocus(pIconExplorer);
    }

    return AK_TRUE;
}

// small icon move up or left focus
T_BOOL IconList_SmallMoveUp(T_ICONEXPLORER *pIconExplorer)
{
    if (pIconExplorer == AK_NULL
		|| pIconExplorer->pItemHead == AK_NULL
		|| pIconExplorer->pItemFocus == AK_NULL)
        return AK_FALSE;

    pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
    if (pIconExplorer->pItemFocus->pPrevious == AK_NULL) 
	{		
        while (pIconExplorer->pItemFocus->pNext != AK_NULL)
            pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pNext;

		IconList_CheckSlipFocus(pIconExplorer);       
    }
    else
	{
        pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pPrevious;
    }

    IconList_CheckShowFocus(pIconExplorer);

    return AK_TRUE;
}

// small icon move down or right focus
T_BOOL IconList_SmallMoveDown(T_ICONEXPLORER *pIconExplorer)
{
    if (pIconExplorer == AK_NULL
		|| pIconExplorer->pItemHead == AK_NULL
		|| pIconExplorer->pItemFocus == AK_NULL)
        return AK_FALSE;

    pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
    if (pIconExplorer->pItemFocus->pNext == AK_NULL)
    {
        pIconExplorer->pItemFocus = pIconExplorer->pItemHead;
        
		IconList_CheckSlipFocus(pIconExplorer);
    }
    else
        pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pNext;

    IconList_CheckShowFocus(pIconExplorer);

    return AK_TRUE;
}

// large icon move left focus
T_BOOL IconList_LargeMoveLeft(T_ICONEXPLORER *pIconExplorer)
{
    if (pIconExplorer == AK_NULL
		|| pIconExplorer->pItemHead == AK_NULL
		|| pIconExplorer->pItemFocus == AK_NULL)
        return AK_FALSE;

    pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
    if (pIconExplorer->pItemFocus->pPrevious == AK_NULL)
	{
        while (pIconExplorer->pItemFocus->pNext != AK_NULL)
            pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pNext;
    }
    else 
	{
        pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pPrevious;
    }

    IconList_CheckShowFocus(pIconExplorer);

    return AK_TRUE;
}

// large icon move right focus
T_BOOL IconList_LargeMoveRight(T_ICONEXPLORER *pIconExplorer)
{
    if (pIconExplorer == AK_NULL
		|| pIconExplorer->pItemHead == AK_NULL
		|| pIconExplorer->pItemFocus == AK_NULL)
        return AK_FALSE;

    pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
    if (pIconExplorer->pItemFocus->pNext == AK_NULL)
        pIconExplorer->pItemFocus = pIconExplorer->pItemHead;
    else
        pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pNext;

    IconList_CheckShowFocus(pIconExplorer);

    return AK_TRUE;
}

// large icon move up focus
T_BOOL IconList_LargeMoveUp(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;
    T_U32 i;
    T_S32 Num, OldNum, NewNum;

    if (pIconExplorer == AK_NULL
		|| pIconExplorer->pItemHead == AK_NULL
		|| pIconExplorer->pItemFocus == AK_NULL)
        return AK_FALSE;

    pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
    for (i=0; i<pIconExplorer->LargeItemCol; i++)
	{
        if (pIconExplorer->pItemFocus->pPrevious == AK_NULL) 
		{
            while (pIconExplorer->pItemFocus->pNext != AK_NULL)
                pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pNext;

            Num = 0;
            OldNum = 0;
            NewNum = 0;
            p = pIconExplorer->pItemHead;
			
            while (p != AK_NULL)
			{
                if (p == pIconExplorer->pItemOldFocus)
                    OldNum = Num;
				
                if (p == pIconExplorer->pItemFocus)
                    NewNum = Num;

                Num++;
                p = p->pNext;
            }

            for (; (OldNum%pIconExplorer->LargeItemCol)!=(NewNum%pIconExplorer->LargeItemCol); NewNum--)
			{
                if (pIconExplorer->pItemFocus->pPrevious == AK_NULL)
                    break;

                pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pPrevious;
            }

            break;
        }
        else 
		{
            pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pPrevious;
        }
    }

    IconList_CheckShowFocus(pIconExplorer);

    return AK_TRUE;
}

// large icon move down focus
T_BOOL IconList_LargeMoveDown(T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p;
    T_U32 i;
    T_S32 Num, OldNum, NewNum;

    if (pIconExplorer == AK_NULL
		|| pIconExplorer->pItemHead == AK_NULL
		|| pIconExplorer->pItemFocus == AK_NULL)
        return AK_FALSE;

    pIconExplorer->pItemOldFocus = pIconExplorer->pItemFocus;
    for (i=0; i<pIconExplorer->LargeItemCol; i++) 
	{
        if (pIconExplorer->pItemFocus->pNext == AK_NULL)
		{
            pIconExplorer->pItemFocus = pIconExplorer->pItemHead;

            Num = 0;
            OldNum = 0;
            NewNum = 0;
            p = pIconExplorer->pItemHead;
			
            while (p != AK_NULL) {
                if (p == pIconExplorer->pItemOldFocus)
                    OldNum = Num;
				
                if (p == pIconExplorer->pItemFocus)
                    NewNum = Num;

                Num++;
                p = p->pNext;
            }

            for (; (OldNum%pIconExplorer->LargeItemCol)!=(NewNum%pIconExplorer->LargeItemCol); NewNum++) 
			{
                if (pIconExplorer->pItemFocus->pNext == AK_NULL)
                    break;

                pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pNext;
            }

            break;
        }
        else
            pIconExplorer->pItemFocus = pIconExplorer->pItemFocus->pNext;
    }

    IconList_CheckShowFocus(pIconExplorer);

    return AK_TRUE;
}


// check rect
T_BOOL IconList_CheckRect(T_RECT *Rect)
{
    if (Rect == AK_NULL)
        return AK_FALSE;

    if ((Rect->width < 0) || (Rect->width > Fwl_GetLcdWidth()))
        Rect->width = Fwl_GetLcdWidth();
	
    if ((Rect->height < 0) || (Rect->height > Fwl_GetLcdHeight()))
        Rect->height = Fwl_GetLcdWidth();
	
    if (((Rect->left+Rect->width) < 0) || ((Rect->left+Rect->width) > Fwl_GetLcdWidth()))
        Rect->left = Fwl_GetLcdWidth() - Rect->width;
	
    if (((Rect->top+Rect->height) < 0) || ((Rect->top+Rect->height) > Fwl_GetLcdHeight()))
        Rect->top = Fwl_GetLcdHeight() - Rect->height;

    return AK_TRUE;
}

// swap two item content
T_BOOL IconList_SwapItemContent(T_ICONEXPLORER_ITEM *pItem1, T_ICONEXPLORER_ITEM *pItem2)
{
    T_U32 Id;
    T_VOID *pContent;
    T_U16 *pText;
    T_U8 *pSmallIcon, *pLargeIcon;
    T_ICONEXPLORER_OPTION_TYPE OptionType;
    T_U8 *pOptionValue;
    T_ICONEXPLORER_OPTION *pOptionHead, *pOptionFocus;

    if ((pItem1 == AK_NULL) || (pItem2 == AK_NULL))
        return AK_FALSE;

    Id = pItem1->Id;
    pContent = pItem1->pContent;
    pText = pItem1->pText;
    pSmallIcon = pItem1->pSmallIcon;
    pLargeIcon = pItem1->pLargeIcon;
    OptionType = pItem1->OptionType;
    pOptionValue = pItem1->pOptionValue;
    pOptionHead = pItem1->pOptionHead;
    pOptionFocus = pItem1->pOptionFocus;

    pItem1->Id = pItem2->Id;
    pItem1->pContent = pItem2->pContent;
    pItem1->pText = pItem2->pText;
    pItem1->pSmallIcon = pItem2->pSmallIcon;
    pItem1->pLargeIcon = pItem2->pLargeIcon;
    pItem1->OptionType = pItem2->OptionType;
    pItem1->pOptionValue = pItem2->pOptionValue;
    pItem1->pOptionHead = pItem2->pOptionHead;
    pItem1->pOptionFocus = pItem2->pOptionFocus;

    pItem2->Id = Id;
    pItem2->pContent = pContent;
    pItem2->pText = pText;
    pItem2->pSmallIcon = pSmallIcon;
    pItem2->pLargeIcon = pLargeIcon;
    pItem2->OptionType = OptionType;
    pItem2->pOptionValue = pOptionValue;
    pItem2->pOptionHead = pOptionHead;
    pItem2->pOptionFocus = pOptionFocus;

    return AK_TRUE;
}

// delete all option at item
T_BOOL IconList_Item_DelAllOption(T_ICONEXPLORER_ITEM *pItem)
{
    T_ICONEXPLORER_OPTION *p, *q;

    if (pItem == AK_NULL)
        return AK_FALSE;

    switch (pItem->OptionType)
	{
    case ICONEXPLORER_OPTION_LIST:
        if (pItem->pOptionHead == AK_NULL)
            return AK_FALSE;

        // free all option space
        p = pItem->pOptionHead;
        while (p != AK_NULL)
		{
            q = p->pNext;
            p = Fwl_Free(p);
            p = q;
        }

        // modify option point
        pItem->pOptionHead = AK_NULL;

        return AK_TRUE;
		
    case ICONEXPLORER_OPTION_NEXT:
    case ICONEXPLORER_OPTION_NONE:
    default:
        return AK_FALSE;
    }
}

