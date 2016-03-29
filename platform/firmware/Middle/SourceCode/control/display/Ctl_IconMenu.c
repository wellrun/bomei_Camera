
/**************************************************************
*
* Copyrights (C) 2005, ANYKA Software Inc.
* All rights reserced.
*
* File name: Ctl_IconMenu.c
* File flag: Icon Menu
* File description:
*
* Revision: 1.00
* Author: Guanghua Zhang
* Date: 2005-11-16
*
****************************************************************/

#include "Fwl_public.h"
#include "Ctl_IconMenu.h"
#include "Eng_DynamicFont.h"
#include "eng_string_uc.h"
#include "Eng_Font.h"
#include "fwl_keyhandler.h"
#include "Eng_AkBmp.h"
#include "fwl_oscom.h"
#include "fwl_pfdisplay.h"
#include "fwl_display.h"
#include "Fwl_tscrcom.h"

static T_BOOL IconMenu_ShowFastMode = AK_TRUE;

static T_BOOL IconMenu_GetItemSize(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_PlaceToRect(T_ICONMENU *pIconMenu, T_S32 place, T_U32 width, \
                                   T_U32 height, T_RECT *outRect);
static T_BOOL IconMenu_ShowIconAttach(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_ShowIcon(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_ShowOther(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_ShowIconAnimate(T_ICONMENU *pIconMenu);
//static T_BOOL IconMenu_AnimateTimerStart(T_ICONMENU *pIconMenu);
//static T_BOOL IconMenu_AnimateTimerStop(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_GetAkBmpSize(const T_U8 *pAkBmpData, T_RECT ShowRect, T_S16 *AkBmpWidth, \
                                    T_S16 *AkBmpHeight, T_U8 **pRgbData, T_RECT *AkBmpRect);
static T_BOOL IconMenu_ShowScBar(T_ICONMENU *pIconMenu);
static T_BOOL IconMenu_ShowAkBmp(const T_U8 *pAkBmpData, T_RECT ShowRect, T_COLOR *TransColor, \
                                 T_U8 Transparency);
static T_BOOL IconMenu_ShowAkBmpWithBack(const T_U8 *pAkBmpData, T_RECT ShowRect, \
                                         T_COLOR *TransColor, T_U8 Transparency, \
                                         const T_U8 *pBackAkBmpData, T_COLOR BackColor, \
                                         T_RECT BackRect);
static T_BOOL IconMenu_GetAkBmpSizePart(const T_U8 *pAkBmpData, T_RECT PartRect, \
                                        T_RECT ShowRect, T_S16 *AkBmpWidth, T_S16 *AkBmpHeight, \
                                        T_U8 **pRgbData, T_RECT *AkBmpRect);
static T_BOOL IconMenu_ShowAkBmpPart(const T_U8 *pAkBmpData, T_RECT PartRect, T_RECT ShowRect, \
                                     T_COLOR *TransColor, T_U8 Transparency);
static T_BOOL IconMenu_ShowAkBmpWithBackPart(const T_U8 *pAkBmpData, T_RECT ShowRect, \
                                             T_COLOR *TransColor, T_U8 Transparency, \
                                             const T_U8 *pBackAkBmpData, T_RECT PartRect, \
                                             T_COLOR BackColor, T_RECT BackRect);
static T_BOOL IconMenu_CheckPointInRect(T_U16 x, T_U16 y, T_RECT Rect);
static T_BOOL IconMenu_CheckRect(T_RECT *Rect);

T_BOOL IconMenu_Init(T_ICONMENU *pIconMenu, T_RECT IconAttachRect, \
                     T_RECT IconRect, T_RECT scbarRect)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->pItemHead = AK_NULL;
    pIconMenu->ItemQty = 0;
    pIconMenu->pItemFocus = AK_NULL;
#ifndef ICONMENU_VERTICAL_ICON
    pIconMenu->pOldItemFocus = AK_NULL;
#endif // ICONMENU_VERTICAL_ICON

    IconMenu_SetIconAttachStyle(pIconMenu, ICONMENU_ALIGN_HCENTER | ICONMENU_ALIGN_VCENTER, \
                                COLOR_BLACK, COLOR_BLUE, AK_NULL, AK_NULL);
    IconMenu_SetIconStyle(pIconMenu, ICONMENU_ICONINTERVAL_DEFAULT, ICONMENU_ICONINTERVAL_DEFAULT, \
                          COLOR_BLACK, COLOR_BLUE, ICONMENU_ICONTRANS_DEFAULT, AK_NULL, AK_NULL);
    IconMenu_SetIconImageNum(pIconMenu, ICONMENU_ICON_NUM, ICONMENU_ICON_DEFAULT);

    IconMenu_SetIconAttachRect(pIconMenu, IconAttachRect);
    IconMenu_SetIconRect(pIconMenu, IconRect);
    IconMenu_SetItemMatrix(pIconMenu, 1, 1);

    pIconMenu->IconAnimateCount = pIconMenu->IconImageDefault;
    pIconMenu->IconAnimateTimer = ERROR_TIMER;

    IconMenu_SetIconShowFlag(pIconMenu, AK_FALSE);
    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);

    pIconMenu->windowPlace = 0;
    pIconMenu->pageRow = 1;
    pIconMenu->ShowFastMode = IconMenu_ShowFastMode;
    pIconMenu->pOtherRect = AK_NULL;
    pIconMenu->OtherShowCallBack = AK_NULL;
    
#ifndef ICONMENU_VERTICAL_ICON
    ScBar_Init(&pIconMenu->scrollBar,scbarRect.left, scbarRect.top, \
               scbarRect.width, scbarRect.height,1,SCBAR_VERTICAL);
#endif // ICONMENU_VERTICAL_ICON
    return AK_TRUE;
}

T_BOOL IconMenu_Free(T_ICONMENU *pIconMenu)
{
    T_ICONMENU_RECT *m, *n;
    T_ICONMENU_ITEM *p, *q;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pIconMenu->pOtherRect != AK_NULL) {
        m = pIconMenu->pOtherRect;
        while (m != AK_NULL) {
            n = m->pNext;

            m = Fwl_Free(m);

            m = n;
        }

        pIconMenu->pOtherRect = AK_NULL;
        pIconMenu->OtherShowCallBack = AK_NULL;
    }

    if (pIconMenu->pItemHead != AK_NULL) {
        p = pIconMenu->pItemHead;
        while (p != AK_NULL) {
            q = p->pNext;

            p = Fwl_Free(p);
            pIconMenu->ItemQty--;

            p = q;
        }

        pIconMenu->pItemHead = AK_NULL;
    }

#ifdef ICONMENU_VERTICAL_ICON
    IconMenu_AnimateTimerStop(pIconMenu);
#endif // ICONMENU_VERTICAL_ICON

    return AK_TRUE;
}

T_BOOL IconMenu_Show(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    IconMenu_ShowIcon(pIconMenu);
    IconMenu_ShowIconAttach(pIconMenu);
    IconMenu_ShowOther(pIconMenu);

#ifndef ICONMENU_VERTICAL_ICON
    if (pIconMenu->ItemRow > pIconMenu->pageRow)
    {
        IconMenu_ShowScBar(pIconMenu);
    }
#endif // ICONMENU_VERTICAL_ICON

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_NONE);

    return AK_TRUE;
}

T_eBACK_STATE IconMenu_Handler(T_ICONMENU *pIconMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_MMI_KEYPAD phyKey;
    T_U8 Id;

    if (pIconMenu == AK_NULL)
        return eStay;

    switch (Event) {
        case M_EVT_USER_KEY:
            phyKey.keyID = pParam->c.Param1;
            phyKey.pressType = pParam->c.Param2;
            switch (phyKey.keyID) {
                case kbOK:
                    if (phyKey.pressType == PRESS_SHORT) {
                        if (IconMenu_GetIconShowFlag(pIconMenu) == AK_FALSE) {
                            IconMenu_SetIconShowFlag(pIconMenu, AK_TRUE);
                        }
                        else {
                            IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
                            return eNext;
                        }
                    }
                    break;
                case kbCLEAR:
                    if (phyKey.pressType == PRESS_SHORT) {
                        ///if (IconMenu_GetIconShowFlag(pIconMenu) == AK_TRUE) {
                        ///    Fwl_ShadeEnable(REFRESHSHADE_NUM);
                        ///    IconMenu_SetIconShowFlag(pIconMenu, AK_FALSE);
                        ///}
                        ///else {
                            return eReturn;
                        ///}
                    }
                    else if (phyKey.pressType == PRESS_LONG) {
                        return eHome;
                    }
                    break;
                case kbMENU:
                    if (phyKey.pressType == PRESS_SHORT) {
                        if (IconMenu_GetIconShowFlag(pIconMenu) == AK_FALSE) {
                            IconMenu_SetIconShowFlag(pIconMenu, AK_TRUE);
                        }
                    }
                    break;
                case kbUP:
                    if (phyKey.pressType == PRESS_SHORT) {
                        if (IconMenu_GetIconShowFlag(pIconMenu) == AK_FALSE) {
                            IconMenu_SetIconShowFlag(pIconMenu, AK_TRUE);
                        }
                        else {
                            IconMenu_MoveFocus(pIconMenu, ICONMENU_DIRECTION_UP);
                            IconMenu_ShowIconAnimate(pIconMenu);
                        }
                    }
                    break;
                case kbDOWN:
                    if (phyKey.pressType == PRESS_SHORT) {
                        if (IconMenu_GetIconShowFlag(pIconMenu) == AK_FALSE) {
                            IconMenu_SetIconShowFlag(pIconMenu, AK_TRUE);
                        }
                        else {
                            IconMenu_MoveFocus(pIconMenu, ICONMENU_DIRECTION_DOWN);
                            IconMenu_ShowIconAnimate(pIconMenu);
                        }
                    }
                    break;
                case kbLEFT:
                    if (phyKey.pressType == PRESS_SHORT) {
                        if (IconMenu_GetIconShowFlag(pIconMenu) == AK_FALSE) {
                            IconMenu_SetIconShowFlag(pIconMenu, AK_TRUE);
                        }
                        else {
                            IconMenu_MoveFocus(pIconMenu, ICONMENU_DIRECTION_LEFT);
                            IconMenu_ShowIconAnimate(pIconMenu);
                        }
                    }
                    break;
                case kbRIGHT:
                    if (phyKey.pressType == PRESS_SHORT) {
                        if (IconMenu_GetIconShowFlag(pIconMenu) == AK_FALSE) {
                            IconMenu_SetIconShowFlag(pIconMenu, AK_TRUE);
                        }
                        else {
                            IconMenu_MoveFocus(pIconMenu, ICONMENU_DIRECTION_RIGHT);
                            IconMenu_ShowIconAnimate(pIconMenu);
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
        case VME_EVT_TIMER:
            if (pParam->w.Param1 == (T_U32)pIconMenu->IconAnimateTimer)
                IconMenu_ShowIconAnimate(pIconMenu);
            break;
        case M_EVT_TOUCH_SCREEN:
            switch (pParam->s.Param1) {
                case eTOUCHSCR_UP:
                    if (IconMenu_GetIconShowFlag(pIconMenu) == AK_FALSE) {
                        IconMenu_SetIconShowFlag(pIconMenu, AK_TRUE);
                    }
                    else {
                        Id = IconMenu_GetIdByPoint(pIconMenu, pParam->s.Param2, pParam->s.Param3);
                        if (Id != ICONMENU_ERROR_ID) 
                        {
                        	if (Id != pIconMenu->pItemFocus->Id)
                        	{
								IconMenu_SetItemFocus(pIconMenu, Id);
                        	}
                        	
                            IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
                            return eNext;
                        }
                    }
                    break;
                case eTOUCHSCR_DOWN:
                 		Id = IconMenu_GetIdByPoint(pIconMenu, pParam->s.Param2, pParam->s.Param3);
                        if (Id != ICONMENU_ERROR_ID) 
                        {
                        	if (Id != pIconMenu->pItemFocus->Id)
                        	{
								IconMenu_SetItemFocus(pIconMenu, Id);
								IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
                        	}
                        }
                    break;
                case eTOUCHSCR_MOVE:
                		Id = IconMenu_GetIdByPoint(pIconMenu, pParam->s.Param2, pParam->s.Param3);
                        if (Id != ICONMENU_ERROR_ID) 
                        {
                        	if (Id != pIconMenu->pItemFocus->Id)
                        	{
								IconMenu_SetItemFocus(pIconMenu, Id);
								IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
                        	}
                        }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return eStay;
}

T_BOOL IconMenu_AddItem(T_ICONMENU *pIconMenu, T_U8 Id, T_U8 Place, const T_U16 *pItemText)
{
    T_U8 i;
    T_ICONMENU_ITEM *p, *q;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pIconMenu->ItemQty >= ICONMENU_ITEMQTY_MAX)
        return AK_FALSE;

    // check ID or Place, and find tail item
    q = pIconMenu->pItemHead;
    p = AK_NULL;
    while (q != AK_NULL) 
    {
        if ((q->Id == Id) || (q->Place == Place))
            return AK_FALSE;

        p = q;
        q = q->pNext;
    }

    q = (T_ICONMENU_ITEM *)Fwl_Malloc(sizeof(T_ICONMENU_ITEM));
    if (q == AK_NULL)
        return AK_FALSE;

    q->Id = Id;
    q->Place = Place;
    
    if (pItemText != AK_NULL)
        Utl_UStrCpyN(q->ItemText, (T_U16 *)pItemText, ICONMENU_TEXT_LEN);

    for (i=0; i<pIconMenu->IconImageNum; i++)
        q->pIconData[i] = AK_NULL;

    if (p == AK_NULL) 
    {
        // item head is AK_NULL
        q->pPrevious = AK_NULL;
        q->pNext = AK_NULL;
        pIconMenu->pItemHead = q;
    }
    else 
    {
        // item head is not AK_NULL, and q is tail item
        q->pPrevious = p;
        q->pNext = AK_NULL;
        p->pNext = q;
    }

    if (pIconMenu->pItemFocus == AK_NULL)
        pIconMenu->pItemFocus = q;

    pIconMenu->ItemQty++;
    IconMenu_SetItemMatrixAuto(pIconMenu);

#ifndef ICONMENU_VERTICAL_ICON
    ScBar_SetValue(&pIconMenu->scrollBar, pIconMenu->windowPlace/pIconMenu->ItemCol, \
                   pIconMenu->ItemRow, (T_U16)pIconMenu->pageRow );
#endif // ICONMENU_VERTICAL_ICON

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
    return AK_TRUE;
}

T_BOOL IconMenu_ResetItemText(T_ICONMENU *pIconMenu, T_U8 Id, const T_U16 *pItemText)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pItemText == AK_NULL)
        return AK_FALSE;

    p = pIconMenu->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p ==  AK_NULL)
        return AK_FALSE;

    Utl_UStrCpyN(p->ItemText, (T_U16 *)pItemText, ICONMENU_TEXT_LEN);

    return AK_TRUE;
}

T_BOOL IconMenu_SetItemIcon(T_ICONMENU *pIconMenu, T_U8 Id, T_U8 index, const T_U8 *pIconData)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (index >= pIconMenu->IconImageNum)
        return AK_FALSE;

    if (pIconData == AK_NULL)
        return AK_FALSE;

    p = pIconMenu->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    p->pIconData[index] = (T_U8 *)pIconData;

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
    return AK_TRUE;
}

T_BOOL IconMenu_DelItemById(T_ICONMENU *pIconMenu, T_U8 Id)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    p = pIconMenu->pItemHead;
    while (p != AK_NULL) {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    if (p->pPrevious != AK_NULL) {
        p->pPrevious->pNext = p->pNext;
        if (pIconMenu->pItemFocus == p)
            pIconMenu->pItemFocus = p->pPrevious;
    }
    if (p->pNext != AK_NULL) {
        p->pNext->pPrevious = p->pPrevious;
        if (pIconMenu->pItemFocus == p)
            pIconMenu->pItemFocus = p->pNext;
    }
    if (pIconMenu->pItemFocus == p)
        pIconMenu->pItemFocus = AK_NULL;

    p = Fwl_Free(p);

    pIconMenu->ItemQty--;
    IconMenu_SetItemMatrixAuto(pIconMenu);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
    return AK_TRUE;
}

T_BOOL IconMenu_DelItemByPlace(T_ICONMENU *pIconMenu, T_U8 Place)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    p = pIconMenu->pItemHead;
    while (p != AK_NULL) {
        if (p->Place == Place)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    if (p->pPrevious != AK_NULL)
        p->pPrevious->pNext = p->pNext;
    if (p->pNext != AK_NULL)
        p->pNext->pPrevious = p->pPrevious;

    p = Fwl_Free(p);

    pIconMenu->ItemQty--;
    IconMenu_SetItemMatrixAuto(pIconMenu);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
    return AK_TRUE;
}

T_BOOL IconMenu_MoveFocus(T_ICONMENU *pIconMenu, T_ICONMENU_DIRECTION Dir)
{
    T_U8 FocusPlace;
    T_ICONMENU_ITEM *p;

//    T_S32  nFocusBak = 0;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pIconMenu->pItemHead == AK_NULL) 
    {
        pIconMenu->pItemFocus = AK_NULL;
        IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
        return AK_FALSE;
    }

    if (pIconMenu->pItemFocus == AK_NULL) 
    {

#ifndef ICONMENU_VERTICAL_ICON
        pIconMenu->pOldItemFocus = pIconMenu->pItemFocus;
#endif // ICONMENU_VERTICAL_ICON

        pIconMenu->pItemFocus = pIconMenu->pItemHead;
        pIconMenu->windowPlace = 0;
        IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_FOCUS | ICONMENU_REFRESH_ICON_ATTACH);
        return AK_TRUE;
    }

    FocusPlace = pIconMenu->pItemFocus->Place;
	
    Fwl_Print(C4, M_CTRL, "befor ,FocusPlace = %d", FocusPlace);
	
    switch (Dir) {
        case ICONMENU_DIRECTION_UP:
#ifdef TX8802_PLATFORM
        {
            if (FocusPlace < pIconMenu->ItemCol)
            {
                FocusPlace = \
                        (pIconMenu->ItemRow-1)*pIconMenu->ItemCol +  \
                        (pIconMenu->ItemCol + (FocusPlace % pIconMenu->ItemCol) - 1) % pIconMenu->ItemCol;
            }
            else
            {
                FocusPlace = \
                        (FocusPlace+pIconMenu->ItemRow*pIconMenu->ItemCol-pIconMenu->ItemCol)%\
                        (pIconMenu->ItemRow*pIconMenu->ItemCol);

            }
            break;
        }    
#else
        {
            FocusPlace = \
                    (FocusPlace+pIconMenu->ItemRow*pIconMenu->ItemCol-pIconMenu->ItemCol)%\
                    (pIconMenu->ItemRow*pIconMenu->ItemCol);
            break;
        }
#endif
        case ICONMENU_DIRECTION_DOWN:
#ifdef TX8802_PLATFORM
        {
            if (FocusPlace >= (pIconMenu->ItemRow-1)*pIconMenu->ItemCol)
            {
                FocusPlace = \
                        ((FocusPlace % pIconMenu->ItemCol) + 1) % pIconMenu->ItemCol;
            }
            else
            {
                FocusPlace = (FocusPlace+pIconMenu->ItemCol)%(pIconMenu->ItemRow*pIconMenu->ItemCol);
            }
            break;
        }
#else 
        {            
            FocusPlace = (FocusPlace+pIconMenu->ItemCol)%(pIconMenu->ItemRow*pIconMenu->ItemCol);
            break;
        }
#endif            
        case ICONMENU_DIRECTION_LEFT:
            FocusPlace = (FocusPlace+pIconMenu->ItemRow*pIconMenu->ItemCol-1) \
                          %(pIconMenu->ItemRow*pIconMenu->ItemCol);
            break;
        case ICONMENU_DIRECTION_RIGHT:
            FocusPlace = (FocusPlace+1)%(pIconMenu->ItemRow*pIconMenu->ItemCol);
            break;
        default:
            IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
            return AK_FALSE;
    }

    Fwl_Print(C4, M_CTRL, "after set ,FocusPlace = %d", FocusPlace);
	
    while (1) {
        p = IconMenu_FindItemByPlace(pIconMenu, FocusPlace);
        if (p != AK_NULL)
            break;

        switch (Dir) 
        {
            case ICONMENU_DIRECTION_UP:
                FocusPlace = \
                        (FocusPlace+pIconMenu->ItemRow*pIconMenu->ItemCol-pIconMenu->ItemCol)%\
                        (pIconMenu->ItemRow*pIconMenu->ItemCol);
                break;
            case ICONMENU_DIRECTION_DOWN:
                FocusPlace = (FocusPlace+pIconMenu->ItemCol)%(pIconMenu->ItemRow*pIconMenu->ItemCol);
                break;
            case ICONMENU_DIRECTION_LEFT:
                FocusPlace = (FocusPlace+pIconMenu->ItemRow*pIconMenu->ItemCol-1) \
                              %(pIconMenu->ItemRow*pIconMenu->ItemCol);
                break;
            case ICONMENU_DIRECTION_RIGHT:
                FocusPlace = (FocusPlace+1)%(pIconMenu->ItemRow*pIconMenu->ItemCol);
                break;
            default:
                IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
                return AK_FALSE;
        }
    }

    if (p == AK_NULL) 
    {
        IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
        return AK_FALSE;
    }

    IconMenu_SetItemFocus(pIconMenu, FocusPlace);
    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_FOCUS | ICONMENU_REFRESH_ICON_ATTACH);

    return AK_TRUE;
}

T_BOOL IconMenu_SetItemFocus(T_ICONMENU *pIconMenu, T_U8 Id)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    p = IconMenu_FindItemById(pIconMenu, Id);
    if (p == AK_NULL)
        return AK_FALSE;

#ifndef ICONMENU_VERTICAL_ICON
    pIconMenu->pOldItemFocus = pIconMenu->pItemFocus;
#endif // ICONMENU_VERTICAL_ICON

    pIconMenu->pItemFocus = p;

#ifdef ICONMENU_VERTICAL_ICON
    pIconMenu->pIconAttachBkImg = p->pIconData[pIconMenu->IconAnimateCount];
    pIconMenu->windowPlace = pIconMenu->pItemFocus->Place - pIconMenu->pageRow/2;
#else
    if (pIconMenu->ItemQty > (pIconMenu->pageRow * pIconMenu->ItemCol))
    {
        if ((pIconMenu->pItemFocus->Place - pIconMenu->windowPlace ) < 0)
        {
            pIconMenu->windowPlace -= pIconMenu->pageRow * pIconMenu->ItemCol;
            if (pIconMenu->windowPlace < 0)
            {
                pIconMenu->windowPlace = 0;
            }
        }
        else if (pIconMenu->pItemFocus->Place >= pIconMenu->windowPlace \
                 + pIconMenu->pageRow * pIconMenu->ItemCol)
        {
            if (pIconMenu->windowPlace + 2*pIconMenu->pageRow * pIconMenu->ItemCol > pIconMenu->ItemRow * pIconMenu->ItemCol)
            {
                pIconMenu->windowPlace = (pIconMenu->ItemRow - pIconMenu->pageRow)*pIconMenu->ItemCol;
            } 
            else
            {
                pIconMenu->windowPlace += pIconMenu->pageRow * pIconMenu->ItemCol;
            }
        }
        
        ScBar_SetValue(&pIconMenu->scrollBar, pIconMenu->windowPlace/pIconMenu->ItemCol, \
                       pIconMenu->ItemRow, (T_U16)pIconMenu->pageRow);
    }
#endif // ICONMENU_VERTICAL_ICON

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_FOCUS | ICONMENU_REFRESH_ICON \
                        | ICONMENU_REFRESH_ICON_ATTACH);

    return AK_TRUE;
}

T_BOOL IconMenu_SetIconAttachRect(T_ICONMENU *pIconMenu, T_RECT IconAttachRect)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->IconAttachtRect = IconAttachRect;
    IconMenu_CheckRect(&pIconMenu->IconAttachtRect);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON_ATTACH);
    return AK_TRUE;
}

T_BOOL IconMenu_SetIconRect(T_ICONMENU *pIconMenu, T_RECT IconRect)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->IconRect = IconRect;
    IconMenu_CheckRect(&pIconMenu->IconRect);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON);
    return AK_TRUE;
}

T_BOOL IconMenu_SetItemMatrix(T_ICONMENU *pIconMenu, T_U8 ItemRow, T_U8 ItemCol)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->ItemRow = ItemRow;
    pIconMenu->ItemCol = ItemCol;

    if ((IconMenu_GetItemSize(pIconMenu) == AK_FALSE) || \
            ((pIconMenu->ItemRow*pIconMenu->ItemCol) < pIconMenu->ItemQty) || \
            ((pIconMenu->ItemRow*pIconMenu->ItemCol) > ICONMENU_ITEMQTY_MAX))
        IconMenu_SetItemMatrixAuto(pIconMenu);
    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON | ICONMENU_REFRESH_SCRBAR);
    return AK_TRUE;
}

T_BOOL IconMenu_SetItemMatrixAuto(T_ICONMENU *pIconMenu)
{
#if 0
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    for (pIconMenu->ItemRow=1, pIconMenu->ItemCol=1; \
         (pIconMenu->ItemRow*pIconMenu->ItemCol)<pIconMenu->ItemQty;) 
    {
        if (pIconMenu->ItemRow > pIconMenu->ItemCol)
            pIconMenu->ItemCol++;
        else
            pIconMenu->ItemRow++;
    }
#endif

    IconMenu_GetItemSize(pIconMenu);

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON);
    return AK_TRUE;
}

T_BOOL IconMenu_SetIconAttachStyle(T_ICONMENU *pIconMenu, T_U8 TextAlign, T_COLOR TextColor, \
                                   T_COLOR TextBackColor, const T_U8 *pAttachBkImg, \
                                   T_RECT *pIconAttachPartRect)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->IconAttachTextAlign = TextAlign;
    pIconMenu->IconAttachTextColor = TextColor;
    pIconMenu->IconAttachTextBackColor = TextBackColor;
    pIconMenu->pIconAttachBkImg = (T_U8 *)pAttachBkImg;
    if (pIconAttachPartRect != AK_NULL) 
    {
        pIconMenu->IconAttachPartRect = *pIconAttachPartRect;
        IconMenu_CheckRect(&pIconMenu->IconAttachPartRect);
        pIconMenu->IconAttachPartRectFlag = AK_TRUE;
    }
    else 
    {
        pIconMenu->IconAttachPartRectFlag = AK_FALSE;
    }

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON_ATTACH);
    return AK_TRUE;
}

T_BOOL IconMenu_SetIconStyle(T_ICONMENU *pIconMenu, T_U32 IconHInterval, T_U32 IconVInterval, \
                             T_COLOR IconTransColor, T_COLOR IconBackColor, T_U8 IconTransparency, \
                             const T_U8 *pIconBackData, T_RECT *pIconPartRect)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->IconHInterval = IconHInterval;
    if (pIconMenu->IconHInterval > ICONMENU_ICONINTERVAL_MAX)
        pIconMenu->IconHInterval = ICONMENU_ICONINTERVAL_MAX;
    pIconMenu->IconVInterval = IconVInterval;
    if (pIconMenu->IconVInterval > ICONMENU_ICONINTERVAL_MAX)
        pIconMenu->IconVInterval = ICONMENU_ICONINTERVAL_MAX;
    pIconMenu->IconTransColor = IconTransColor;
    pIconMenu->IconBackColor = IconBackColor;
    pIconMenu->IconTransparency = IconTransparency;
    if (pIconMenu->IconTransparency > 100)
        pIconMenu->IconTransparency = 100;
    pIconMenu->pIconBackData = (T_U8 *)pIconBackData;
    if (pIconPartRect != AK_NULL) {
        pIconMenu->IconPartRect = *pIconPartRect;
        IconMenu_CheckRect(&pIconMenu->IconPartRect);
        pIconMenu->IconPartRectFlag = AK_TRUE;
    }
    else {
        pIconMenu->IconPartRectFlag = AK_FALSE;
    }

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ICON);
    return AK_TRUE;
}

T_BOOL IconMenu_SetIconImageNum(T_ICONMENU *pIconMenu, T_U8 IconImageNum, T_U8 IconImageDefault)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (IconImageDefault >= IconImageNum)
        return AK_FALSE;

    pIconMenu->IconImageNum = IconImageNum;
    pIconMenu->IconImageDefault = IconImageDefault;

    return AK_TRUE;
}

T_BOOL IconMenu_SetIconShowFlag(T_ICONMENU *pIconMenu, T_BOOL IconShowFlag)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->IconShowFlag = IconShowFlag;

#ifdef ICONMENU_VERTICAL_ICON
    if (pIconMenu->IconShowFlag == AK_TRUE)
        IconMenu_AnimateTimerStart(pIconMenu);
    else
        IconMenu_AnimateTimerStop(pIconMenu);
#endif

    IconMenu_SetRefresh(pIconMenu, ICONMENU_REFRESH_ALL);
    return AK_TRUE;
}

T_BOOL IconMenu_SetRefresh(T_ICONMENU *pIconMenu, T_U8 RefreshFlag)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (RefreshFlag == ICONMENU_REFRESH_NONE)
        pIconMenu->RefreshFlag = ICONMENU_REFRESH_NONE;
    else
        pIconMenu->RefreshFlag |= RefreshFlag;

#ifndef ICONMENU_VERTICAL_ICON
    if (pIconMenu->RefreshFlag == ICONMENU_REFRESH_ALL)
        pIconMenu->pOldItemFocus = AK_NULL;
#endif // ICONMENU_VERTICAL_ICON

    return AK_TRUE;
}

T_BOOL IconMenu_SetShowFastMode(T_ICONMENU *pIconMenu, T_BOOL ShowFastMode)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    IconMenu_ShowFastMode = ShowFastMode;
    pIconMenu->ShowFastMode = IconMenu_ShowFastMode;

    return AK_TRUE;
}

T_BOOL IconMenu_SetOtherShowCallBack(T_ICONMENU *pIconMenu, T_fICONMENU_CALLBACK OtherShowCallBack)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->OtherShowCallBack = OtherShowCallBack;

    return AK_TRUE;
}

T_BOOL IconMenu_AddOtherRect(T_ICONMENU *pIconMenu, T_RECT OtherRect)
{
    T_ICONMENU_RECT *p, *q;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    q = pIconMenu->pOtherRect;
    p = AK_NULL;
    while (q != AK_NULL) {
        p = q;
        q = q->pNext;
    }

    q = (T_ICONMENU_RECT *)Fwl_Malloc(sizeof(T_ICONMENU_RECT));
    if (q == AK_NULL)
        return AK_FALSE;

    q->Rect = OtherRect;
    IconMenu_CheckRect(&q->Rect);
    q->pNext = AK_NULL;

    if (p == AK_NULL)
        pIconMenu->pOtherRect = q;
    else
        p->pNext = q;

    return AK_TRUE;
}

T_U8 IconMenu_GetRefreshFlag(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return ICONMENU_REFRESH_NONE;

    return pIconMenu->RefreshFlag;
}

T_BOOL IconMenu_GetIconShowFlag(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    return pIconMenu->IconShowFlag;
}

T_ICONMENU_ITEM *IconMenu_GetFocusItem(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return AK_NULL;

    if (pIconMenu->pItemHead == AK_NULL)
        pIconMenu->pItemFocus = AK_NULL;

    if (pIconMenu->pItemFocus == AK_NULL)
        pIconMenu->pItemFocus = pIconMenu->pItemHead;

    return pIconMenu->pItemFocus;
}

T_U8 IconMenu_GetFocusId(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return ICONMENU_ERROR_ID;

    if (pIconMenu->pItemHead == AK_NULL)
        pIconMenu->pItemFocus = AK_NULL;

    if (pIconMenu->pItemFocus == AK_NULL)
        pIconMenu->pItemFocus = pIconMenu->pItemHead;

    if (pIconMenu->pItemFocus == AK_NULL)
        return ICONMENU_ERROR_ID;

    return pIconMenu->pItemFocus->Id;
}

T_U8 IconMenu_GetFocusPlace(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return ICONMENU_ERROR_PLACE;

    if (pIconMenu->pItemHead == AK_NULL)
        pIconMenu->pItemFocus = AK_NULL;

    if (pIconMenu->pItemFocus == AK_NULL)
        pIconMenu->pItemFocus = pIconMenu->pItemHead;

    if (pIconMenu->pItemFocus == AK_NULL)
        return ICONMENU_ERROR_PLACE;

    return pIconMenu->pItemFocus->Place;
}

T_U8 IconMenu_GetIdByPoint(T_ICONMENU *pIconMenu, T_U16 x, T_U16 y)
{
    T_ICONMENU_ITEM *p;
    T_RECT ShowRect;
    T_S16 AkBmpWidth;
    T_S16 AkBmpHeight;

    if (pIconMenu == AK_NULL)
        return ICONMENU_ERROR_ID;

    p = pIconMenu->pItemHead;
    while (p != AK_NULL) 
    {
        if ((p->Place < pIconMenu->ItemRow*pIconMenu->ItemCol) && \
                (p->pIconData[pIconMenu->IconImageDefault] != AK_NULL)) 
        {
        	AKBmpGetInfo(p->pIconData[pIconMenu->IconImageDefault], &AkBmpWidth,
                         &AkBmpHeight, AK_NULL);
            IconMenu_PlaceToRect(pIconMenu, p->Place, AkBmpWidth, AkBmpHeight, &ShowRect);
            if (IconMenu_CheckPointInRect(x, y, ShowRect) == AK_TRUE)
                return p->Id;
        }

        p = p->pNext;
    }

    return ICONMENU_ERROR_ID;
}

T_ICONMENU_ITEM *IconMenu_FindItemById(T_ICONMENU *pIconMenu, T_U8 Id)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_NULL;

    p = pIconMenu->pItemHead;
    while (p != AK_NULL) 
    {
        if (p->Id == Id)
            break;

        p = p->pNext;
    }

    return p;
}

T_ICONMENU_ITEM *IconMenu_FindItemByPlace(T_ICONMENU *pIconMenu, T_U32 Place)
{
    T_ICONMENU_ITEM *p;

    if (pIconMenu == AK_NULL)
        return AK_NULL;

    p = pIconMenu->pItemHead;
    while (p != AK_NULL) 
    {
        if ((T_U32)(p->Place) == Place)
            break;

        p = p->pNext;
    }

    return p;
}

static T_BOOL IconMenu_GetItemSize(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    pIconMenu->ItemWidth = (pIconMenu->IconRect.width-pIconMenu->IconHInterval* \
                            (pIconMenu->ItemCol+1))/pIconMenu->ItemCol;
    pIconMenu->ItemHeight = (pIconMenu->IconRect.height-pIconMenu->IconVInterval* \
                             (pIconMenu->ItemRow+1))/pIconMenu->ItemRow;
    
    if (pIconMenu->ItemWidth < ICONMENU_ICONSIZE_MIN_WIDTH) 
    {
        pIconMenu->ItemWidth = ICONMENU_ICONSIZE_MIN_WIDTH;
    }
    
    if (pIconMenu->ItemHeight < ICONMENU_ICONSIZE_MIN_HEIGHT)
    {
        pIconMenu->ItemHeight = ICONMENU_ICONSIZE_MIN_HEIGHT;
    }

    pIconMenu->pageRow = (pIconMenu->IconRect.height-pIconMenu->IconVInterval)\
                          /(pIconMenu->IconVInterval + pIconMenu->ItemHeight);
    
    return AK_TRUE;
}

static T_BOOL IconMenu_ShowIconAttach(T_ICONMENU *pIconMenu)
{
#ifndef ICONMENU_VERTICAL_ICON
    T_S16 PosX, PosY;
#endif // ICONMENU_VERTICAL_ICON

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if ((pIconMenu->RefreshFlag & ICONMENU_REFRESH_ICON_ATTACH) != ICONMENU_REFRESH_ICON_ATTACH)
        return AK_FALSE;
    
    if (pIconMenu->IconShowFlag == AK_TRUE) 
    {
        if (pIconMenu->pIconAttachBkImg != AK_NULL) 
        {
            if (pIconMenu->IconAttachPartRectFlag == AK_TRUE)
                IconMenu_ShowAkBmpPart(pIconMenu->pIconAttachBkImg, pIconMenu->IconAttachPartRect, \
                                       pIconMenu->IconAttachtRect, AK_NULL, 0);
            else
                IconMenu_ShowAkBmp(pIconMenu->pIconAttachBkImg, pIconMenu->IconAttachtRect, \
                                   &pIconMenu->IconTransColor, pIconMenu->IconTransparency);
        }
    }
    else 
    {
        if (pIconMenu->pIconBackData != AK_NULL)
                IconMenu_ShowAkBmpPart(pIconMenu->pIconBackData, pIconMenu->IconAttachtRect, \
                                       pIconMenu->IconAttachtRect, AK_NULL, 0);

        return AK_TRUE;
    }

#ifndef ICONMENU_VERTICAL_ICON
    switch (pIconMenu->IconAttachTextAlign & ICONMENU_ALIGN_HORIZONTAL) 
    {
        case ICONMENU_ALIGN_LEFT:
            PosX = pIconMenu->IconAttachtRect.left;
            break;
        case ICONMENU_ALIGN_RIGHT:
            PosX = (T_S16)(pIconMenu->IconAttachtRect.left + pIconMenu->IconAttachtRect.width \
                           - UGetSpeciStringWidth(pIconMenu->pItemFocus->ItemText, \
                           CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pIconMenu->pItemFocus->ItemText)));
            break;
        case ICONMENU_ALIGN_HCENTER:
        default:
            PosX = (T_S16)(pIconMenu->IconAttachtRect.left + (pIconMenu->IconAttachtRect.width \
                           - UGetSpeciStringWidth(pIconMenu->pItemFocus->ItemText, \
                           CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pIconMenu->pItemFocus->ItemText)))/2);
            break;
    }

    switch (pIconMenu->IconAttachTextAlign & ICONMENU_ALIGN_VERTICAL) 
    {
        case ICONMENU_ALIGN_UP:
            PosY = pIconMenu->IconAttachtRect.top;
            break;
        case ICONMENU_ALIGN_DOWN:
            PosY = pIconMenu->IconAttachtRect.top+pIconMenu->IconAttachtRect.height-g_Font.CHEIGHT;
            break;
        case ICONMENU_ALIGN_VCENTER:
        default:
            PosY = pIconMenu->IconAttachtRect.top+(pIconMenu->IconAttachtRect.height-g_Font.CHEIGHT)/2;
            break;
    }
#endif // ICONMENU_VERTICAL_ICON

    return AK_TRUE;
}


static T_BOOL IconMenu_PlaceToRect(T_ICONMENU *pIconMenu, T_S32 place, T_U32 width, \
                                   T_U32 height, T_RECT *outRect)
{
    AK_ASSERT_PTR(outRect,"IconMenu_PlaceToRect outRect==null",AK_FALSE);

    outRect->left = (T_S16)(pIconMenu->IconRect.left + \
        pIconMenu->IconHInterval*(place%(T_S32)pIconMenu->ItemCol+1) + \
        pIconMenu->ItemWidth*(place%(T_S32)pIconMenu->ItemCol));
    outRect->top = (T_S16)(pIconMenu->IconRect.top + \
        pIconMenu->IconVInterval*((place - pIconMenu->windowPlace)/pIconMenu->ItemCol+1) + \
        pIconMenu->ItemHeight*((place - pIconMenu->windowPlace)/pIconMenu->ItemCol));

#ifndef ICONMENU_VERTICAL_ICON
    outRect->top -= ICONMENU_ICON_ADJUST_TOP_MARGIN;
#endif // ICONMENU_VERTICAL_ICON

    if (width != 0)
    {
        outRect->left -= (T_POS)(width - pIconMenu->ItemWidth)/2;
        outRect->width = (T_POS)width;
    } 
    else 
    {
        outRect->width = (T_S16)(pIconMenu->ItemWidth);
    }

    if (height != 0)
    {
        outRect->top -= (T_POS)(height - pIconMenu->ItemHeight)/2;
        outRect->height = (T_POS)height;
    } 
    else
    {
        outRect->height = (T_S16)(pIconMenu->ItemHeight);
    }
    if (outRect->top < pIconMenu->IconRect.top
        || outRect->left < pIconMenu->IconRect.left
        ||(outRect->top + outRect->height) > (pIconMenu->IconRect.top + pIconMenu->IconRect.height)
        || (outRect->left + outRect->width) > (pIconMenu->IconRect.left + pIconMenu->IconRect.width))
    {
        return AK_FALSE;
    }
    return AK_TRUE;
}

static T_BOOL IconMenu_ShowIcon(T_ICONMENU *pIconMenu)
{
    T_ICONMENU_ITEM *p;
    T_RECT ShowRect, IconPartRect;
    T_S16 AkBmpWidth, AkBmpHeight;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (((pIconMenu->RefreshFlag & ICONMENU_REFRESH_ICON) != ICONMENU_REFRESH_ICON) && \
            ((pIconMenu->RefreshFlag & ICONMENU_REFRESH_FOCUS) != ICONMENU_REFRESH_FOCUS))
        return AK_FALSE;

    if ((pIconMenu->RefreshFlag & ICONMENU_REFRESH_ICON) == ICONMENU_REFRESH_ICON) 
    {
       IconPartRect = pIconMenu->IconPartRect;
      
       if (pIconMenu->pIconBackData != AK_NULL) 
       {
            if (pIconMenu->IconPartRectFlag == AK_TRUE)
            {    
                IconMenu_ShowAkBmpPart(pIconMenu->pIconBackData, IconPartRect, \
                                    pIconMenu->IconRect, AK_NULL, 0);                                    
            }
            else
            {
                IconMenu_ShowAkBmp(pIconMenu->pIconBackData, pIconMenu->IconRect, AK_NULL, 0);
            }
       }

        if (pIconMenu->IconShowFlag == AK_FALSE)
            return AK_TRUE;

        p = pIconMenu->pItemHead;
        p = IconMenu_FindItemByPlace(pIconMenu,pIconMenu->windowPlace);

#ifdef ICONMENU_VERTICAL_ICON
        if (pIconMenu->windowPlace < 0)
        {
            p = IconMenu_FindItemByPlace(pIconMenu, 0);
        }
#endif // ICONMENU_VERTICAL_ICON

        while (p != AK_NULL) 
        {
#ifdef ICONMENU_VERTICAL_ICON
            if ((p == pIconMenu->pItemFocus) && (p->pIconData[ICONMENU_ICON_ON_FOCUS] != AK_NULL))
            {
                pIconMenu->IconImageDefault = ICONMENU_ICON_ON_FOCUS;
            }
            else
            {
                pIconMenu->IconImageDefault = ICONMENU_ICON_DEFAULT;
            }
#endif // ICONMENU_VERTICAL_ICON

            if ((p->Place < pIconMenu->ItemRow*pIconMenu->ItemCol) && \
                    (p->pIconData[pIconMenu->IconImageDefault] != AK_NULL)) 
            {
                AKBmpGetInfo(p->pIconData[pIconMenu->IconImageDefault], &AkBmpWidth, \
                             &AkBmpHeight, AK_NULL);
                if (IconMenu_PlaceToRect(pIconMenu,p->Place,AkBmpWidth,AkBmpHeight,&ShowRect))
                {
#ifndef ICONMENU_VERTICAL_ICON
                    if (p->Place == pIconMenu->pItemFocus->Place)
                    {
                        Fwl_FillSolidRect(HRGB_LAYER, (T_U16)(ShowRect.left-pIconMenu->IconHInterval/2),\
                                          (T_U16)(ShowRect.top-pIconMenu->IconVInterval/2), \
                                          (T_U16)(ShowRect.width+pIconMenu->IconHInterval), \
                                          (T_U16)(ShowRect.height+pIconMenu->IconVInterval), \
                                          ICONMENU_ICON_FOCUS_BACK);
                    }
#else
                    ShowRect.left = pIconMenu->IconRect.left + 6;
#endif                
                    IconMenu_ShowAkBmp(p->pIconData[pIconMenu->IconImageDefault], ShowRect, \
                                       &pIconMenu->IconTransColor, pIconMenu->IconTransparency);
#ifndef ICONMENU_VERTICAL_ICON
                    Fwl_UDispString(HRGB_LAYER, (T_U16)(ShowRect.left + (ShowRect.width - \
                                UGetSpeciStringWidth(p->ItemText, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(p->ItemText)))/2), \
                                (T_U16)(ShowRect.top+ShowRect.height+ICONMENU_ATTACH_FONT_INTERVAL), \
                                p->ItemText, (T_U16)Utl_UStrLen(p->ItemText), ICONMENU_ICON_TEXT_COLOR, CURRENT_FONT_SIZE);
#else
{
                    T_COLOR dispColor = ICONMENU_ICON_TEXT_COLOR;

                    if (p->Place == pIconMenu->pItemFocus->Place)
                        dispColor = COLOR_ORANGE;

                    Fwl_UDispString(HRGB_LAYER, (T_POS)(ShowRect.left + AkBmpWidth + 4), \
                                (T_POS)(ShowRect.top + (ShowRect.height - 16)/2 + ICONMENU_ATTACH_FONT_INTERVAL), \
                                p->ItemText, (T_U16)Utl_UStrLen(p->ItemText), dispColor, CURRENT_FONT_SIZE);
}
#endif // ICONMENU_VERTICAL_ICON
                }
            }
            p = p->pNext;
        }
    }

#ifndef ICONMENU_VERTICAL_ICON
    if (((pIconMenu->RefreshFlag & ICONMENU_REFRESH_FOCUS) == ICONMENU_REFRESH_FOCUS) 
            && (pIconMenu->pOldItemFocus != AK_NULL)) 
    {
        p = pIconMenu->pOldItemFocus;
        pIconMenu->pOldItemFocus = AK_NULL;

        if ((p->Place < pIconMenu->ItemRow*pIconMenu->ItemCol) && 
            (p->pIconData[pIconMenu->IconImageDefault] != AK_NULL)) 
        {
            AKBmpGetInfo(p->pIconData[pIconMenu->IconImageDefault], &AkBmpWidth, &AkBmpHeight, AK_NULL);
            if (IconMenu_PlaceToRect(pIconMenu,p->Place,AkBmpWidth,AkBmpHeight,&ShowRect) != AK_FALSE)
            {
                IconMenu_ShowAkBmp(p->pIconData[pIconMenu->IconImageDefault], ShowRect, \
                                   &pIconMenu->IconTransColor, pIconMenu->IconTransparency);
                Fwl_UDispString(HRGB_LAYER, (T_U16)(ShowRect.left+(ShowRect.width-UGetSpeciStringWidth(p->ItemText, \
                            CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(p->ItemText)))/2), \
                            (T_U16)(ShowRect.top+ShowRect.height+ICONMENU_ATTACH_FONT_INTERVAL), \
                            p->ItemText, (T_U16)Utl_UStrLen(p->ItemText), ICONMENU_ICON_TEXT_COLOR, CURRENT_FONT_SIZE);
            }
        }
    }
#else
    {
        T_RECT BackRect;
        AKBmpGetInfo(pIconMenu->pItemFocus->pIconData[ICONMENU_ICON_ON_FOCUS], \
                     &AkBmpWidth, &AkBmpHeight, AK_NULL);
        if (IconMenu_PlaceToRect(pIconMenu,pIconMenu->pItemFocus->Place,AkBmpWidth,\
            AkBmpHeight,&BackRect) != AK_FALSE)
        {
            IconMenu_ShowAkBmpPart(pIconMenu->pItemFocus->pIconData[ICONMENU_ICON_ON_FOCUS], \
                                   BackRect, BackRect, AK_NULL, 0);
        }
    }

    if ((pIconMenu->RefreshFlag == ICONMENU_REFRESH_ALL) \
        || ((pIconMenu->RefreshFlag & ICONMENU_REFRESH_FOCUS) == ICONMENU_REFRESH_FOCUS))
    {
        IconMenu_ShowIconAnimate(pIconMenu);
    }

#endif

    return AK_TRUE;
}


static T_BOOL IconMenu_ShowScBar(T_ICONMENU *pIconMenu)
{
#ifndef ICONMENU_VERTICAL_ICON
    T_RECT rect;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if ((pIconMenu->RefreshFlag & ICONMENU_REFRESH_SCRBAR) != ICONMENU_REFRESH_SCRBAR)
        return AK_FALSE;

    if (pIconMenu->IconShowFlag == AK_FALSE) 
    {
        rect = ScBar_GetRect(&pIconMenu->scrollBar);
        if (pIconMenu->pIconBackData != AK_NULL)
        {
             IconMenu_ShowAkBmpPart(pIconMenu->pIconBackData, rect, rect, AK_NULL, 0);
        }
    }
    else
    {
        ScBar_Show(&pIconMenu->scrollBar);
    }
    
    return AK_TRUE;
#else
    return AK_FALSE;
#endif
}

static T_BOOL IconMenu_ShowOther(T_ICONMENU *pIconMenu)
{
    T_ICONMENU_RECT *p;

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if ((pIconMenu->RefreshFlag & ICONMENU_REFRESH_OTHER) != ICONMENU_REFRESH_OTHER)
        return AK_FALSE;

    if (((pIconMenu->IconShowFlag == AK_FALSE) || (pIconMenu->RefreshFlag == ICONMENU_REFRESH_ALL)) \
          &&(pIconMenu->pOtherRect != AK_NULL)) 
    {
        p = pIconMenu->pOtherRect;
        while (p != AK_NULL) 
        {
            //Fwl_FillSolidRect(HRGB_LAYER, p->Rect.left, p->Rect.top, p->Rect.width, p->Rect.height, pIconMenu->IconBackColor);
            if (pIconMenu->pIconBackData != AK_NULL)
                    IconMenu_ShowAkBmpPart(pIconMenu->pIconBackData, p->Rect, p->Rect, AK_NULL, 0);

            p = p->pNext;
        }
    }

    if ((pIconMenu->IconShowFlag == AK_TRUE) && (pIconMenu->pOtherRect != AK_NULL) \
        && (pIconMenu->OtherShowCallBack != AK_NULL))
    {
        pIconMenu->OtherShowCallBack();
    }

    return AK_TRUE;
}

static T_BOOL IconMenu_ShowIconAnimate(T_ICONMENU *pIconMenu)
{
#ifndef ICONMENU_VERTICAL_ICON
    T_RECT ShowRect;
    T_S16 AkBmpWidth, AkBmpHeight;
#endif // ICONMENU_VERTICAL_ICON

    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pIconMenu->IconShowFlag == AK_FALSE)
        return AK_TRUE;

    if (pIconMenu->pItemFocus != AK_NULL) 
    {
#ifdef ICONMENU_VERTICAL_ICON
        if (pIconMenu->IconAnimateCount < ICONMENU_ICON_LIST_NUM)
        {
            pIconMenu->IconAnimateCount = ICONMENU_ICON_LIST_NUM;
        }
        pIconMenu->pIconAttachBkImg = pIconMenu->pItemFocus->pIconData[pIconMenu->IconAnimateCount];
        if (pIconMenu->IconAttachPartRectFlag == AK_TRUE)
        {
            IconMenu_ShowAkBmpWithBackPart(pIconMenu->pIconAttachBkImg, pIconMenu->IconAttachPartRect, \
                                           &pIconMenu->IconTransColor, \
                                           (T_U8)(pIconMenu->IconTransparency/4), \
                                           pIconMenu->pIconBackData, pIconMenu->IconPartRect, \
                                           pIconMenu->IconBackColor, pIconMenu->IconAttachtRect);
                                           
          if (pIconMenu->IconAttachPartRectFlag == AK_TRUE)
            IconMenu_ShowAkBmpWithBackPart(pIconMenu->pIconAttachBkImg, \
            pIconMenu->IconAttachPartRect, &pIconMenu->IconTransColor, (T_U8)(pIconMenu->IconTransparency/4), \
            pIconMenu->pIconBackData, pIconMenu->IconPartRect, pIconMenu->IconBackColor, pIconMenu->IconAttachtRect);                                 
                                           
        }
        else
        {
            IconMenu_ShowAkBmpWithBack(pIconMenu->pIconAttachBkImg, pIconMenu->IconAttachtRect, \
                                       &pIconMenu->IconTransColor, \
                                       (T_U8)(pIconMenu->IconTransparency/4), pIconMenu->pIconBackData, \
                                       pIconMenu->IconBackColor, pIconMenu->IconAttachtRect);
        }
 #else
        if (pIconMenu->IconAnimateCount == pIconMenu->IconImageDefault)
            pIconMenu->IconAnimateCount = (pIconMenu->IconAnimateCount+1)%pIconMenu->IconImageNum;
        
        if ((pIconMenu->pItemFocus->Place < pIconMenu->ItemRow*pIconMenu->ItemCol) && \
            (pIconMenu->pItemFocus->pIconData[pIconMenu->IconAnimateCount] != AK_NULL)) 
        {
            
            AKBmpGetInfo(pIconMenu->pItemFocus->pIconData[pIconMenu->IconAnimateCount], \
                         &AkBmpWidth, &AkBmpHeight, AK_NULL);
            IconMenu_PlaceToRect(pIconMenu,pIconMenu->pItemFocus->Place, AkBmpWidth, AkBmpHeight,\
                                &ShowRect);
            if (pIconMenu->IconPartRectFlag == AK_TRUE)
            {
                IconMenu_ShowAkBmpWithBackPart(pIconMenu->pItemFocus->pIconData[pIconMenu->IconAnimateCount], \
                                               ShowRect, &pIconMenu->IconTransColor, \
                                               (T_U8)(pIconMenu->IconTransparency/4), \
                                                pIconMenu->pIconBackData, pIconMenu->IconPartRect, \
                                                pIconMenu->IconBackColor, pIconMenu->IconRect);
            }
            else
            {
                IconMenu_ShowAkBmpWithBack(pIconMenu->pItemFocus->pIconData[pIconMenu->IconAnimateCount], \
                                           ShowRect, &pIconMenu->IconTransColor, \
                                           (T_U8)(pIconMenu->IconTransparency/4), \
                                           pIconMenu->pIconBackData, pIconMenu->IconBackColor, \
                                           pIconMenu->IconRect);
            }

        }
#endif // ICONMENU_VERTICAL_ICON

        pIconMenu->IconAnimateCount = (pIconMenu->IconAnimateCount+1)%pIconMenu->IconImageNum;
    }

    return AK_TRUE;
}

#if  0
static T_BOOL IconMenu_AnimateTimerStart(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pIconMenu->IconAnimateTimer != ERROR_TIMER) {
        Fwl_StopTimer(pIconMenu->IconAnimateTimer);
        pIconMenu->IconAnimateTimer = ERROR_TIMER;
    }

    pIconMenu->IconAnimateTimer = Fwl_SetTimerMilliSecond(ICONMENU_ANIMATETIME, AK_TRUE);
    if (pIconMenu->IconAnimateTimer == ERROR_TIMER)
        return AK_FALSE;

    return AK_TRUE;
}

static T_BOOL IconMenu_AnimateTimerStop(T_ICONMENU *pIconMenu)
{
    if (pIconMenu == AK_NULL)
        return AK_FALSE;

    if (pIconMenu->IconAnimateTimer != ERROR_TIMER) {
        Fwl_StopTimer(pIconMenu->IconAnimateTimer);
        pIconMenu->IconAnimateTimer = ERROR_TIMER;
    }

    return AK_TRUE;
}
#endif

static T_BOOL IconMenu_GetAkBmpSize(const T_U8 *pAkBmpData, T_RECT ShowRect, T_S16 *AkBmpWidth, \
                                    T_S16 *AkBmpHeight, T_U8 **pRgbData, T_RECT *AkBmpRect)
{
    T_U8 AkBmpFrame, AkBmpDeep;

    if (pAkBmpData == AK_NULL) {
        (*AkBmpWidth) = ShowRect.width;
        (*AkBmpHeight) = ShowRect.height;
        (*pRgbData) = AK_NULL;
        (*AkBmpRect) = ShowRect;

        return AK_TRUE;
    }

    // check AK BMP format, only deep=24
    AkBmpFrame = AKBmpGetInfo(pAkBmpData, AkBmpWidth, AkBmpHeight, &AkBmpDeep);
    if (AkBmpDeep != 24)
        return AK_FALSE;
    if ((AkBmpFrame == 0) || (AkBmpFrame == 2))
        (*pRgbData) = (T_U8 *)(pAkBmpData+6);
    else
        (*pRgbData) = (T_U8 *)(pAkBmpData+4);

    if (((*AkBmpWidth) > ShowRect.width) || ((*AkBmpHeight) > ShowRect.height)) {
        if (((*AkBmpWidth)*ShowRect.height) >= ((*AkBmpHeight)*ShowRect.width)) {
            AkBmpRect->width = ShowRect.width;
            AkBmpRect->height = (*AkBmpHeight)*ShowRect.width/(*AkBmpWidth);
        }
        else {
            AkBmpRect->width = (*AkBmpWidth)*ShowRect.height/(*AkBmpHeight);
            AkBmpRect->height = ShowRect.height;
        }
    }
    else {
        AkBmpRect->width = (*AkBmpWidth);
        AkBmpRect->height = (*AkBmpHeight);
    }
    AkBmpRect->left = ShowRect.left + (ShowRect.width-AkBmpRect->width)/2;
    AkBmpRect->top = ShowRect.top + (ShowRect.height-AkBmpRect->height)/2;

    return AK_TRUE;
}

static T_BOOL IconMenu_ShowAkBmp(const T_U8 *pAkBmpData, T_RECT ShowRect, T_COLOR *TransColor, \
                                 T_U8 Transparency)
{
    T_S16 AkBmpWidth, AkBmpHeight, x, y;
    T_U8 AkBmpDeep;
    T_RECT AkBmpRect;
    T_U8 *pRgbData, *pLcdBuf;
    T_U32 AkBmpPos, LcdBufPos;
    T_COLOR PosColor;

    if (pAkBmpData == AK_NULL)
        return AK_FALSE;

    if (IconMenu_ShowFastMode == AK_TRUE) {
        AKBmpGetInfo(pAkBmpData, &AkBmpWidth, &AkBmpHeight, &AkBmpDeep);

#ifdef ICONMENU_VERTICAL_ICON
        AkBmpRect.left = (T_S16)(ShowRect.left + ((ShowRect.width>AkBmpWidth)?((ShowRect.width-AkBmpWidth)):0));
#else
        AkBmpRect.left = (T_S16)(ShowRect.left + ((ShowRect.width>AkBmpWidth)?((ShowRect.width-AkBmpWidth)/2):0));
#endif
        AkBmpRect.top = (T_S16)(ShowRect.top + ((ShowRect.height>AkBmpHeight)?((ShowRect.height-AkBmpHeight)/2):0));

        return Fwl_AkBmpDrawFromString(HRGB_LAYER, \
                AkBmpRect.left, \
                AkBmpRect.top, \
                pAkBmpData, TransColor, AK_FALSE);
    }

    if (IconMenu_GetAkBmpSize(pAkBmpData, ShowRect, &AkBmpWidth, &AkBmpHeight, &pRgbData, &AkBmpRect) \
        == AK_FALSE)
        return AK_FALSE;

    if (Transparency > 100)
        Transparency = 100;

    pLcdBuf = Fwl_GetDispMemory();

    for (y=0; y<AkBmpRect.height; y++) {
        for (x=0; x<AkBmpRect.width; x++) {
            AkBmpPos = (y*AkBmpHeight/AkBmpRect.height)*AkBmpWidth + (x*AkBmpWidth/AkBmpRect.width);
            LcdBufPos = (AkBmpRect.top+y)*Fwl_GetLcdWidth() + (AkBmpRect.left+x);
            PosColor = (*(pRgbData+AkBmpPos*3)) | ((*(pRgbData+AkBmpPos*3+1))<<8) | ((*(pRgbData+AkBmpPos*3+2))<<16);
            if ((TransColor == AK_NULL) || (PosColor != *TransColor)) {
                *(pLcdBuf+LcdBufPos*3) = (*(pRgbData+AkBmpPos*3))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3))*Transparency/100;
                *(pLcdBuf+LcdBufPos*3+1) = (*(pRgbData+AkBmpPos*3+1))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3+1))*Transparency/100;
                *(pLcdBuf+LcdBufPos*3+2) = (*(pRgbData+AkBmpPos*3+2))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3+2))*Transparency/100;
            }
        }
    }

    return AK_TRUE;
}

static T_BOOL IconMenu_ShowAkBmpWithBack(const T_U8 *pAkBmpData, T_RECT ShowRect, \
                                         T_COLOR *TransColor, T_U8 Transparency, const \
                                         T_U8 *pBackAkBmpData, T_COLOR BackColor, T_RECT BackRect)
{
    T_S16 AkBmpWidth, AkBmpHeight, BackAkBmpWidth, BackAkBmpHeight, x, y;
    T_U8 AkBmpDeep;
    T_RECT AkBmpRect, BackAkBmpRect;
    T_U8 *pRgbData, *pBackRgbData, *pLcdBuf;
    T_U32 AkBmpPos, LcdBufPos;
    T_COLOR PosColor;

    if (pAkBmpData == AK_NULL)
        return AK_FALSE;

    if (IconMenu_ShowFastMode == AK_TRUE) {
        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, ShowRect.left, ShowRect.top, &ShowRect, pBackAkBmpData, \
                                AK_NULL, AK_FALSE);
        AKBmpGetInfo(pAkBmpData, &AkBmpWidth, &AkBmpHeight, &AkBmpDeep);

#ifdef ICONMENU_VERTICAL_ICON
        AkBmpRect.left = (T_S16)(ShowRect.left + ((ShowRect.width>AkBmpWidth)?((ShowRect.width-AkBmpWidth)):0));
#else
        AkBmpRect.left = (T_S16)(ShowRect.left + ((ShowRect.width>AkBmpWidth)?((ShowRect.width-AkBmpWidth)/2):0));
#endif
        AkBmpRect.top = (T_S16)(ShowRect.top + ((ShowRect.height>AkBmpHeight)?((ShowRect.height-AkBmpHeight)/2):0));

        return Fwl_AkBmpDrawFromString(HRGB_LAYER, \
                AkBmpRect.left, \
                AkBmpRect.top, \
                pAkBmpData, TransColor, AK_FALSE);
    }

    if (IconMenu_GetAkBmpSize(pAkBmpData, ShowRect, &AkBmpWidth, &AkBmpHeight, &pRgbData, &AkBmpRect) \
        == AK_FALSE)
        return AK_FALSE;

    if (IconMenu_GetAkBmpSize(pBackAkBmpData, BackRect, &BackAkBmpWidth, &BackAkBmpHeight, \
        &pBackRgbData, &BackAkBmpRect) == AK_FALSE)
        return AK_FALSE;

    pLcdBuf = Fwl_GetDispMemory();

    for (y=0; y<ShowRect.height; y++) 
    {
        for (x=0; x<ShowRect.width; x++) 
        {
            if ((pBackAkBmpData != AK_NULL) && (((ShowRect.left+x) >= (BackAkBmpRect.left)) 
                && ((ShowRect.left+x) < (BackAkBmpRect.left+BackAkBmpRect.width))) \
                && (((ShowRect.top+y) >= (BackAkBmpRect.top)) 
                && ((ShowRect.top+y) < (BackAkBmpRect.top+BackAkBmpRect.height)))) 
            {
                AkBmpPos = ((ShowRect.top+y-BackAkBmpRect.top)*BackAkBmpHeight/BackAkBmpRect.height) * \
                             BackAkBmpWidth + ((ShowRect.left+x-BackAkBmpRect.left) \
                             * BackAkBmpWidth/BackAkBmpRect.width);
                PosColor = (*(pBackRgbData+AkBmpPos*3)) | ((*(pBackRgbData+AkBmpPos*3+1))<<8) \
                           | ((*(pBackRgbData+AkBmpPos*3+2))<<16);
            }
            else 
            {
                PosColor = BackColor;
            }
            
            LcdBufPos = (ShowRect.top+y)*Fwl_GetLcdWidth() + (ShowRect.left+x);
            memcpy((void *)(pLcdBuf+LcdBufPos*3), (void *)(&PosColor), 3);
        }
    }

    if (Transparency > 100)
        Transparency = 100;

    for (y=0; y<AkBmpRect.height; y++) 
    {
        for (x=0; x<AkBmpRect.width; x++) 
        {
            AkBmpPos = (y*AkBmpHeight/AkBmpRect.height)*AkBmpWidth + (x*AkBmpWidth/AkBmpRect.width);
            LcdBufPos = (AkBmpRect.top+y)*Fwl_GetLcdWidth() + (AkBmpRect.left+x);
            PosColor = (*(pRgbData+AkBmpPos*3)) | ((*(pRgbData+AkBmpPos*3+1))<<8) \
                        | ((*(pRgbData+AkBmpPos*3+2))<<16);
            if ((TransColor == AK_NULL) || (PosColor != *TransColor)) 
            {
                *(pLcdBuf+LcdBufPos*3) = (*(pRgbData+AkBmpPos*3))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3))*Transparency/100;
                *(pLcdBuf+LcdBufPos*3+1) = (*(pRgbData+AkBmpPos*3+1))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3+1))*Transparency/100;
                *(pLcdBuf+LcdBufPos*3+2) = (*(pRgbData+AkBmpPos*3+2))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3+2))*Transparency/100;
            }
        }
    }

    return AK_TRUE;
}

static T_BOOL IconMenu_GetAkBmpSizePart(const T_U8 *pAkBmpData, T_RECT PartRect, T_RECT ShowRect, \
                                        T_S16 *AkBmpWidth, T_S16 *AkBmpHeight, T_U8 **pRgbData, \
                                        T_RECT *AkBmpRect)
{
    T_U8 AkBmpFrame, AkBmpDeep;

    if (pAkBmpData == AK_NULL) 
    {
        (*AkBmpWidth) = ShowRect.width;
        (*AkBmpHeight) = ShowRect.height;
        (*pRgbData) = AK_NULL;
        (*AkBmpRect) = ShowRect;

        return AK_TRUE;
    }

    // check AK BMP format, only deep=24
    AkBmpFrame = AKBmpGetInfo(pAkBmpData, AkBmpWidth, AkBmpHeight, &AkBmpDeep);
    if (AkBmpDeep != 24)
        return AK_FALSE;
    if ((AkBmpFrame == 0) || (AkBmpFrame == 2))
        (*pRgbData) = (T_U8 *)(pAkBmpData+6);
    else
        (*pRgbData) = (T_U8 *)(pAkBmpData+4);

    if ((PartRect.width > (*AkBmpWidth)) || (PartRect.height > (*AkBmpHeight)))
        return AK_FALSE;

    if (((PartRect.left+PartRect.width) > (*AkBmpWidth)) || ((PartRect.top+PartRect.height) > (*AkBmpHeight)))
        return AK_FALSE;

    if ((PartRect.width > ShowRect.width) || (PartRect.height > ShowRect.height)) 
    {
        if ((PartRect.width*ShowRect.height) >= (PartRect.height*ShowRect.width)) 
        {
            AkBmpRect->width = ShowRect.width;
            AkBmpRect->height = PartRect.height*ShowRect.width/PartRect.width;
        }
        else 
        {
            AkBmpRect->width = PartRect.width*ShowRect.height/PartRect.height;
            AkBmpRect->height = ShowRect.height;
        }
    }
    else 
    {
        AkBmpRect->width = PartRect.width;
        AkBmpRect->height = PartRect.height;
    }
    
    AkBmpRect->left = ShowRect.left + (ShowRect.width-AkBmpRect->width)/2;
    AkBmpRect->top = ShowRect.top + (ShowRect.height-AkBmpRect->height)/2;

    return AK_TRUE;
}

static T_BOOL IconMenu_ShowAkBmpPart(const T_U8 *pAkBmpData, T_RECT PartRect, T_RECT ShowRect, \
                                     T_COLOR *TransColor, T_U8 Transparency)
{
    T_S16 AkBmpWidth, AkBmpHeight, x, y;
    T_U8 AkBmpDeep;
    T_RECT AkBmpRect;
    T_U8 *pRgbData, *pLcdBuf;
    T_U32 AkBmpPos, LcdBufPos;
    T_COLOR PosColor;

    if (pAkBmpData == AK_NULL)
        return AK_FALSE;

    if (IconMenu_ShowFastMode == AK_TRUE) 
    {
        AKBmpGetInfo(pAkBmpData, &AkBmpWidth, &AkBmpHeight, &AkBmpDeep);
        return Fwl_AkBmpDrawPartFromString(HRGB_LAYER, \
                (T_S16)(ShowRect.left + ((ShowRect.width>AkBmpWidth)?((ShowRect.width-AkBmpWidth)/2):0)), \
                (T_S16)(ShowRect.top + ((ShowRect.height>AkBmpHeight)?((ShowRect.height-AkBmpHeight)/2):0)), \
                &PartRect, pAkBmpData, TransColor, AK_FALSE);
    }

    if (IconMenu_GetAkBmpSizePart(pAkBmpData, PartRect, ShowRect, &AkBmpWidth, &AkBmpHeight, \
                                  &pRgbData, &AkBmpRect) == AK_FALSE)
        return AK_FALSE;

    if (Transparency > 100)
        Transparency = 100;

    pLcdBuf = Fwl_GetDispMemory();

    for (y=0; y<AkBmpRect.height; y++) 
    {
        for (x=0; x<AkBmpRect.width; x++) 
        {
            AkBmpPos = ((y+PartRect.top)*PartRect.height/AkBmpRect.height)*AkBmpWidth + ((x+PartRect.left)*PartRect.width/AkBmpRect.width);
            LcdBufPos = (AkBmpRect.top+y)*Fwl_GetLcdWidth() + (AkBmpRect.left+x);
            PosColor = (*(pRgbData+AkBmpPos*3)) | ((*(pRgbData+AkBmpPos*3+1))<<8) | ((*(pRgbData+AkBmpPos*3+2))<<16);
            if ((TransColor == AK_NULL) || (PosColor != *TransColor)) 
            {
                *(pLcdBuf+LcdBufPos*3) = (*(pRgbData+AkBmpPos*3))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3))*Transparency/100;
                *(pLcdBuf+LcdBufPos*3+1) = (*(pRgbData+AkBmpPos*3+1))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3+1))*Transparency/100;
                *(pLcdBuf+LcdBufPos*3+2) = (*(pRgbData+AkBmpPos*3+2))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3+2))*Transparency/100;
            }
        }
    }

    return AK_TRUE;
}

static T_BOOL IconMenu_ShowAkBmpWithBackPart(const T_U8 *pAkBmpData, T_RECT ShowRect, \
                                             T_COLOR *TransColor, T_U8 Transparency, \
                                             const T_U8 *pBackAkBmpData, T_RECT PartRect, \
                                             T_COLOR BackColor, T_RECT BackRect)
{
    T_S16 AkBmpWidth, AkBmpHeight, BackAkBmpWidth, BackAkBmpHeight, x, y;
    T_U8 AkBmpDeep;
    T_RECT AkBmpRect, BackAkBmpRect;
    T_U8 *pRgbData, *pBackRgbData, *pLcdBuf;
    T_U32 AkBmpPos, LcdBufPos;
    T_COLOR PosColor;

    if (pAkBmpData == AK_NULL)
        return AK_FALSE;

    if (IconMenu_ShowFastMode == AK_TRUE) {
        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, ShowRect.left, ShowRect.top, &ShowRect, pBackAkBmpData, \
                                AK_NULL, AK_FALSE);
        AKBmpGetInfo(pAkBmpData, &AkBmpWidth, &AkBmpHeight, &AkBmpDeep);
        return Fwl_AkBmpDrawFromString(HRGB_LAYER, \
                (T_S16)(ShowRect.left + ((ShowRect.width>AkBmpWidth)?((ShowRect.width-AkBmpWidth)/2):0)), \
                (T_S16)(ShowRect.top + ((ShowRect.height>AkBmpHeight)?((ShowRect.height-AkBmpHeight)/2):0)), \
                pAkBmpData, TransColor, AK_FALSE);
    }

    if (IconMenu_GetAkBmpSize(pAkBmpData, ShowRect, &AkBmpWidth, &AkBmpHeight, &pRgbData, \
                              &AkBmpRect) == AK_FALSE)
        return AK_FALSE;

    if (IconMenu_GetAkBmpSizePart(pBackAkBmpData, PartRect, BackRect, &BackAkBmpWidth, \
                                  &BackAkBmpHeight, &pBackRgbData, &BackAkBmpRect) == AK_FALSE)
        return AK_FALSE;

    pLcdBuf = Fwl_GetDispMemory();

    for (y=0; y<ShowRect.height; y++) {
        for (x=0; x<ShowRect.width; x++) {
            if ((pBackAkBmpData != AK_NULL) && \
                    (((ShowRect.left+x) >= (BackAkBmpRect.left)) && \
                    ((ShowRect.left+x) < (BackAkBmpRect.left+BackAkBmpRect.width))) && \
                    (((ShowRect.top+y) >= (BackAkBmpRect.top)) && \
                    ((ShowRect.top+y) < (BackAkBmpRect.top+BackAkBmpRect.height)))) {
                AkBmpPos = ((ShowRect.top+y+PartRect.top-BackAkBmpRect.top)*PartRect.height/BackAkBmpRect.height)*BackAkBmpWidth + \
                        ((ShowRect.left+x+PartRect.left-BackAkBmpRect.left)*PartRect.width/BackAkBmpRect.width);
                PosColor = (*(pBackRgbData+AkBmpPos*3)) | \
                        ((*(pBackRgbData+AkBmpPos*3+1))<<8) | \
                        ((*(pBackRgbData+AkBmpPos*3+2))<<16);
            }
            else {
                PosColor = BackColor;
            }
            LcdBufPos = (ShowRect.top+y)*Fwl_GetLcdWidth() + (ShowRect.left+x);
            memcpy((void *)(pLcdBuf+LcdBufPos*3), (void *)(&PosColor), 3);
        }
    }

    if (Transparency > 100)
        Transparency = 100;

    for (y=0; y<AkBmpRect.height; y++) {
        for (x=0; x<AkBmpRect.width; x++) {
            AkBmpPos = (y*AkBmpHeight/AkBmpRect.height)*AkBmpWidth + (x*AkBmpWidth/AkBmpRect.width);
            LcdBufPos = (AkBmpRect.top+y)*Fwl_GetLcdWidth() + (AkBmpRect.left+x);
            PosColor = (*(pRgbData+AkBmpPos*3)) | ((*(pRgbData+AkBmpPos*3+1))<<8) \
                         | ((*(pRgbData+AkBmpPos*3+2))<<16);
            if ((TransColor == AK_NULL) || (PosColor != *TransColor)) {
                *(pLcdBuf+LcdBufPos*3) = (*(pRgbData+AkBmpPos*3))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3))*Transparency/100;
                *(pLcdBuf+LcdBufPos*3+1) = (*(pRgbData+AkBmpPos*3+1))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3+1))*Transparency/100;
                *(pLcdBuf+LcdBufPos*3+2) = (*(pRgbData+AkBmpPos*3+2))*(100-Transparency)/100 + \
                        (*(pLcdBuf+LcdBufPos*3+2))*Transparency/100;
            }
        }
    }

    return AK_TRUE;
}

static T_BOOL IconMenu_CheckPointInRect(T_U16 x, T_U16 y, T_RECT Rect)
{
    if (((x >= Rect.left) && (x < Rect.left+Rect.width)) && \
            ((y >= Rect.top) && (y <= Rect.top+Rect.height)))
        return AK_TRUE;
    else
        return AK_FALSE;
}

static T_BOOL IconMenu_CheckRect(T_RECT *Rect)
{
    if (Rect == AK_NULL)
        return AK_FALSE;

    if ((Rect->width < 0) || (Rect->width > Fwl_GetLcdWidth()))
        Rect->width = Fwl_GetLcdWidth();
    if ((Rect->height < 0) || (Rect->height > Fwl_GetLcdHeight()))
        Rect->height = Fwl_GetLcdWidth();
    if (((Rect->left+Rect->width) < 0) || \
            ((Rect->left+Rect->width) > Fwl_GetLcdWidth()))
        Rect->left = Fwl_GetLcdWidth() - Rect->width;
    if (((Rect->top+Rect->height) < 0) || \
            ((Rect->top+Rect->height) > Fwl_GetLcdHeight()))
        Rect->top = Fwl_GetLcdHeight() - Rect->height;

    return AK_TRUE;
}
