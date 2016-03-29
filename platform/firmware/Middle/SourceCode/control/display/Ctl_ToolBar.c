/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  *
  * @File name: Ctl_ToolBar.c
  * @Function: This file is the implement of the control Ctl_ToolBar.

  * @Author: Liuweijun
  * @Date: 2008-04-03
  * @Version: 1.0
  */


#include "Ctl_ToolBar.h"
#ifdef CAMERA_SUPPORT

#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "fwl_keyhandler.h"
#include "Eng_topbar.h"
#include "Fwl_osMalloc.h"
#include "Eng_String.h"
#include "Eng_AkBmp.h"
#include "Eng_ImgConvert.h"
#include "Eng_DataConvert.h"
#include "Eng_String_UC.h"
#include "lib_image_api.h"
#include "Fwl_pfDisplay.h"
#include "Lib_res_port.h"
#include "fwl_pfdisplay.h"
#include "fwl_graphic.h"
#include "Fwl_tscrcom.h"
#include "fwl_display.h"
#include "Fwl_DispOsd.h"



///////////////////////////////////////////////////////////////////////////////////////

#define     TOOLBAR_MARGIN_H        (Fwl_GetLcdHeight() / 10)
#define     TOOLBAR_MARGIN_V        (Fwl_GetLcdWidth() / 10)

#define     BUTTON_MARGIN           4
#define     BUTTON_INTERVAL         2

#define     TOOLBAR_OPTION_WIDTH_V  160
#define     TOOLBAR_OPTION_HEIGHT   24      //Height of a option item.


//////////////////////////////////////////////////////////////////////////////////////


static T_VOID ToolBar_MoveFocusButton(T_pTOOLBAR pToolBar, T_TB_MOVE_DIRECTION Direction);

static T_VOID ToolBar_MoveFocusButtonOption(T_pBUTTON pButton, T_TB_MOVE_DIRECTION Direction);

static T_VOID ToolBar_UpdateBarRect(T_pTOOLBAR pToolBar);

static T_BOOL ToolBar_GetPrevScrollRect(T_pTOOLBAR pToolBar, T_RECT *PrevRect);

static T_BOOL ToolBar_GetBackScrollRect(T_pTOOLBAR pToolBar, T_RECT *BackRect);

static T_VOID ToolBar_GetFirstButtonRect(T_pTOOLBAR pToolBar, T_RECT *pFstBtnRect);

static T_BOOL ToolBar_UpdateFirstShownButton(T_pTOOLBAR pToolBar);

static T_VOID ToolBar_UpdateButtonNameRect(T_pBUTTON pButton, T_pTOOLBAR pToolBar);

static T_VOID ToolBar_UpdateButtonOptionRect(T_pBUTTON pButton, T_pTOOLBAR pToolBar);

static T_VOID ToolBar_CheckScroll(T_pTOOLBAR pToolBar);

static T_VOID ToolBar_DelAllOptionOfButton(T_pBUTTON pButton);

static T_VOID ToolBar_DelAllButton(T_pTOOLBAR pToolBar);

static T_BOOL ToolBar_SetFocusButton(T_pTOOLBAR pToolBar, T_pBUTTON pButton);

static T_VOID ToolBar_SetButtonState(T_pTOOLBAR pToolBar, T_pBUTTON pButton, T_BUTTON_STATE state);

static T_BOOL ToolBar_SetButtonStateByID(T_pTOOLBAR pToolBar, T_U32 ButtonId, T_BUTTON_STATE state);

static T_BUTTON_STATE ToolBar_GetButtonStateByID(T_pTOOLBAR pToolBar, T_U32 ButtonId);

static T_BOOL ToolBar_HotkeyMatching(T_pTOOLBAR pToolBar, T_eKEY_ID KeyId, T_U32 *ButtonId);

static T_VOID ToolBar_ShowButtonZone(T_pTOOLBAR pToolBar);

static T_VOID ToolBar_ShowNameZone(T_pTOOLBAR pToolBar);

static T_VOID ToolBar_ShowOptioItem(T_BUTTON_OPTION *pOptionItem, T_pBUTTON pButton, \
                                    T_pTOOLBAR pToolBar, T_RECT ItemRect);

static T_VOID ToolBar_ShowSubMenuZone(T_pTOOLBAR pToolBar);

static T_VOID ToolBar_ShowOptionZone(T_pTOOLBAR pToolBar);

static T_VOID ToolBar_EditTypeBtn_GetShowRect(T_pTOOLBAR pToolBar, T_RECT *pEditRect, 
                                       T_RECT *pPrevTriaRect, T_RECT *pBackTriaRect);

static T_VOID ToolBar_ShowEditZone(T_pTOOLBAR pToolBar);

static T_BOOL ToolBar_GetReturnIconRes(T_pTOOLBAR pToolBar);

static T_BOOL ToolBar_GetReturnIconRect(T_pTOOLBAR pToolBar, T_RECT* pRect);

static T_eBACK_STATE ToolBar_HandlerUserKey(T_pTOOLBAR pToolBar, T_MMI_KEYPAD phyKey);

static T_eBACK_STATE ToolBar_HandlerTscr(T_pTOOLBAR pToolBar, T_EVT_PARAM *pParam);

//////Internal function/////////////////////////////////////////////////////////////////////////////

//this function should be called when total button num is changed !
static T_VOID ToolBar_UpdateBarRect(T_pTOOLBAR pToolBar)
{
	T_LEN winW;	// The Window Width of The ToolBar Should Be Show
	T_LEN winH;	// The Window Height of The ToolBar Should Be Show
	
    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_UpdateBarRect()>> pToolBar is AK_NULL");

	if (TB_eMODE_SHOWN_ON_YUV == pToolBar->ShownMode)
	{
		winW = pToolBar->BackYUV.Width;
		winH = pToolBar->BackYUV.Height;
	}
	else
	{
		winW = Fwl_GetLcdWidth();
		winH = Fwl_GetLcdHeight();
	}

    if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
    {
        T_LEN   TotalWidth;
        T_LEN   ScrollIconWidth;

        //prev scroll icon with . it's a quarter of the button height.
        ScrollIconWidth = (pToolBar->ButtonHeight >> 2);

        //理论上所有有效buttons(disabled buttons除外)，以及前后滚动标志符以及所有间隔的总宽度。
        TotalWidth = (T_LEN)(pToolBar->ButtonTotalNum * (pToolBar->ButtonWidth + pToolBar->Interval) \
                     + ((ScrollIconWidth + pToolBar->Interval) << 1) + pToolBar->Interval);

        //理论上总宽度 小于 屏幕宽度，那么所有button都可以有空间一次显示。
        if (TotalWidth <= winW)
        {
            pToolBar->ButtonShownNum = pToolBar->ButtonTotalNum;

            //update barRect
            pToolBar->BarRect.width = TotalWidth;
            pToolBar->BarRect.left = ((winW - pToolBar->BarRect.width) >> 1);
            pToolBar->BarRect.height = (T_LEN)(pToolBar->ButtonHeight + (BUTTON_MARGIN << 1));

            if (TB_eTOP == pToolBar->Direction)
            {
                pToolBar->BarRect.top = TOOLBAR_MARGIN_H;
            }
            else
            {
                pToolBar->BarRect.top = winH - TOOLBAR_MARGIN_H - pToolBar->BarRect.height;
            }
        }
    }
    else if ((TB_eLEFT == pToolBar->Direction) || (TB_eRIGHT == pToolBar->Direction))
    {
        T_LEN   TotalHeight;
        T_LEN   ScrollIconHeight;

        //scroll icon height,it 's a quarter of the button width
        ScrollIconHeight = (pToolBar->ButtonWidth >> 2);

        TotalHeight = (T_LEN)(pToolBar->ButtonTotalNum *(pToolBar->ButtonHeight + pToolBar->Interval) \
                      + ((ScrollIconHeight + pToolBar->Interval) << 1) + pToolBar->Interval);

        if (TotalHeight <= winH)
        {
            pToolBar->ButtonShownNum = pToolBar->ButtonTotalNum;

            //update barRect
            pToolBar->BarRect.width = (T_LEN)(pToolBar->ButtonWidth + (BUTTON_MARGIN << 1));

            if (TB_eLEFT == pToolBar->Direction)
            {
                pToolBar->BarRect.left = TOOLBAR_MARGIN_V;
            }
            else
            {
                pToolBar->BarRect.left = winW - TOOLBAR_MARGIN_V - pToolBar->BarRect.width;
            }

            pToolBar->BarRect.height = TotalHeight;
            pToolBar->BarRect.top = ((winH - pToolBar->BarRect.height) >> 1);

        }
    }
}


//get the prev scroll icon's rect to show in.
static T_BOOL ToolBar_GetPrevScrollRect(T_pTOOLBAR pToolBar, T_RECT *PrevRect)
{
    AK_ASSERT_PTR(pToolBar, "ToolBar_GetPrevScrollRect()>> pToolBar is null", AK_FALSE);
    AK_ASSERT_PTR(PrevRect, "ToolBar_GetPrevScrollRect()>> PrevRect is null", AK_FALSE);

    if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
    {
        //get prev scroll icon rect
        PrevRect->height = (pToolBar->ButtonHeight >> 1);
        PrevRect->top = pToolBar->BarRect.top + ((pToolBar->BarRect.height - PrevRect->height) >> 1);
        PrevRect->width = (PrevRect->height >> 1);
        PrevRect->left = pToolBar->BarRect.left + pToolBar->Interval;
    }
    else
    {
        //get prev scroll icon rect
        PrevRect->width = (pToolBar->ButtonWidth >> 1);
        PrevRect->left = pToolBar->BarRect.left + ((pToolBar->BarRect.width - PrevRect->width) >> 1);
        PrevRect->height = (PrevRect->width >> 1);
        PrevRect->top = pToolBar->BarRect.top + pToolBar->Interval;
    }

    return AK_TRUE;
}


//get the back scroll icon's rect to show in.
static T_BOOL ToolBar_GetBackScrollRect(T_pTOOLBAR pToolBar, T_RECT *BackRect)
{
    AK_ASSERT_PTR(pToolBar, "ToolBar_GetBackScrollRect()>> pToolBar is null", AK_FALSE);
    AK_ASSERT_PTR(BackRect, "ToolBar_GetBackScrollRect()>> BackRect is null", AK_FALSE);

    if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
    {
        //get back scroll icon rect
        BackRect->height = (pToolBar->ButtonHeight >> 1);
        BackRect->top = pToolBar->BarRect.top + ((pToolBar->BarRect.height - BackRect->height) >> 1);
        BackRect->width = (BackRect->height >> 1);
        BackRect->left = pToolBar->BarRect.left + pToolBar->BarRect.width \
                        - (BackRect->width + pToolBar->Interval);
    }
    else
    {
        //get back scroll icon rect
        BackRect->width = (pToolBar->ButtonWidth >> 1);
        BackRect->left = pToolBar->BarRect.left + ((pToolBar->BarRect.width - BackRect->width) >> 1);
        BackRect->height = (BackRect->width >> 1);
        BackRect->top = pToolBar->BarRect.top + pToolBar->BarRect.height \
                        - (BackRect->height + pToolBar->Interval);
    }

    return AK_TRUE;
}


static T_VOID ToolBar_GetFirstButtonRect(T_pTOOLBAR pToolBar, T_RECT *pFstBtnRect)
{
    T_RECT      PrevIconRect = {0};

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_GetFirstButtonRect()>> pToolBar is null!");
    AK_ASSERT_PTR_VOID(pFstBtnRect, "ToolBar_GetFirstButtonRect()>> pFstBtnRect is null!");

    ToolBar_GetPrevScrollRect(pToolBar, &PrevIconRect);

    if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
    {
        pFstBtnRect->left = pToolBar->BarRect.left + (pToolBar->Interval << 1) + PrevIconRect.width;
        pFstBtnRect->top = pToolBar->BarRect.top + BUTTON_MARGIN;
    }
    else
    {
        pFstBtnRect->left = pToolBar->BarRect.left + BUTTON_MARGIN;
        pFstBtnRect->top = pToolBar->BarRect.top + (pToolBar->Interval << 1) + PrevIconRect.height;
    }

    pFstBtnRect->width = pToolBar->ButtonWidth;
    pFstBtnRect->height = pToolBar->ButtonHeight;
}


//Get the Title Rect. The pToolBar is where the pButton in.
static T_VOID ToolBar_UpdateButtonNameRect(T_pBUTTON pButton, T_pTOOLBAR pToolBar)
{
    T_pBUTTON   p = AK_NULL;
    T_U32       count = 0;
    T_RECT      FstBtnRect = {0};

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_UpdateButtonNameRect()>> pToolBar is AK_NULL");

    if (pButton != pToolBar->pFocusBtn)
    {
        pButton->NameRect.width = 0;
        pButton->NameRect.left = 0;
        pButton->NameRect.height = 0;
        pButton->NameRect.top = 0;

        return;
    }

    if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
    {
        //the NameRect of the pFocusBtn is determinated.

        if (pToolBar->BarRect.width < TOOLBAR_OPTION_WIDTH_V)
            pButton->NameRect.width = TOOLBAR_OPTION_WIDTH_V;
        else
            pButton->NameRect.width = pToolBar->BarRect.width;

        pButton->NameRect.left = pToolBar->BarRect.left;
        pButton->NameRect.height = pToolBar->BarRect.height;

        if (TB_eTOP == pToolBar->Direction)
        {
            pButton->NameRect.top = pToolBar->BarRect.top + pToolBar->BarRect.height;
        }
        else
        {
            pButton->NameRect.top = pToolBar->BarRect.top - pButton->NameRect.height;
        }
    }
    else if ((TB_eLEFT == pToolBar->Direction) || (TB_eRIGHT == pToolBar->Direction))
    {
        //the top of the pFocusBtn's NameRect is the same as the top of the focus button.

        pButton->NameRect.width = TOOLBAR_OPTION_WIDTH_V;

        if (TB_eLEFT == pToolBar->Direction)
        {
            pButton->NameRect.left = pToolBar->BarRect.left + pToolBar->BarRect.width;
        }
        else
        {
            pButton->NameRect.left = pToolBar->BarRect.left - pButton->NameRect.width;
        }

        pButton->NameRect.height = pToolBar->ButtonHeight + pToolBar->Interval;

        p = pToolBar->pFirstBtn;

        //计算 pFocusBtn 相对 pFirstBtn 的位置，用于确定其 NameRect 位置。
        while ((AK_NULL != p) && (p != pToolBar->pFocusBtn))
        {
            //disabled button will not be shown.
            if (BTN_STATE_DISABLED != p->State)
                count++;

            p = p->pNext;
        }

        ToolBar_GetFirstButtonRect(pToolBar, &FstBtnRect);
        pButton->NameRect.top = (T_POS)(FstBtnRect.top + count * (pToolBar->ButtonHeight + pToolBar->Interval));
    }
}


/* if focus button changed, the first shown button maybe need to changed too.
   pFirstBtn is determined by pFocusBtn.
 */
static T_BOOL ToolBar_UpdateFirstShownButton(T_pTOOLBAR pToolBar)
{
    T_pBUTTON   pOldFstShown;
    T_pBUTTON   p;
    T_U32       count = 0;

    AK_ASSERT_PTR(pToolBar, "ToolBar_UpdateFirstShownButton()>> pToolBar is null!", AK_FALSE);

    if ((AK_NULL == pToolBar->pHeadBtn) || (AK_NULL == pToolBar->pFocusBtn))
    {
        pToolBar->pFirstBtn = AK_NULL;
        return AK_FALSE;
    }

    pOldFstShown = pToolBar->pFirstBtn;

    if (pOldFstShown == AK_NULL)
    {
        pOldFstShown = pToolBar->pHeadBtn;
    }

    p = pToolBar->pFocusBtn;
    count = 0;

    //从pFocusBtn往前找，找到了pOldFstShown，或者找的个数大于可显示button数量时，或者找到 pHeadBtn 时退出
    while (AK_NULL != p)
    {
        if (p == pOldFstShown)
            break;

        if (count >= pToolBar->ButtonShownNum)
            break;

        p = p->pPrev;
        count++;
    }


    if (AK_NULL == p)
    {
        //找到了头结点，但没找到之前的pOldFstShown， 说明pOldFstShown 在新的pFocusBtn之后。
        pToolBar->pFirstBtn = pToolBar->pFocusBtn;
    }
    else if (count >= pToolBar->ButtonShownNum)
    {
        //从pFocusBtn往前找了ButtonShownNum个button都没找到pOldFstShown，但也还没有到头结点。
        pToolBar->pFirstBtn = p->pNext;
		
		while ((BTN_STATE_DISABLED == pToolBar->pFirstBtn->State) && (AK_NULL != pToolBar->pFirstBtn))
		{
			pToolBar->pFirstBtn = pToolBar->pFirstBtn->pNext;
		}		
    }
    else
    {
        //不需要变动pFirstBtn，即可正确显示pFocusBtn及邻近button。
    }

    return AK_TRUE;
}


//pToolBar is the one in which pButton is .
static T_VOID ToolBar_UpdateButtonOptionRect(T_pBUTTON pButton, T_pTOOLBAR pToolBar)
{
    T_RECT      FstBtnRect = {0};

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_UpdateButtonOptionRect>> pToolBar is null!");
    AK_ASSERT_PTR_VOID(pButton, "ToolBar_UpdateButtonOptionRect>> pButton is null!");


    if ((pButton != pToolBar->pFocusBtn) || (BTN_TYPE_SWITCH == pButton->Type))
    {
        pButton->OptionRect.width = 0;
        pButton->OptionRect.left = 0;
        pButton->OptionRect.height = 0;
        pButton->OptionRect.top = 0;

        return;
    }

    ToolBar_GetFirstButtonRect(pToolBar, &FstBtnRect);

    if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
    {
        pButton->OptionRect.width = (T_LEN)((pToolBar->ButtonWidth + pToolBar->Interval)\
                                            * pToolBar->ButtonShownNum \
                                            - pToolBar->Interval);
        pButton->OptionRect.left = FstBtnRect.left;

        if (BTN_TYPE_EDIT != pButton->Type)
        {
            pButton->OptionRect.height = (T_LEN)(pButton->OptionNum \
                                                * (TOOLBAR_OPTION_HEIGHT + pToolBar->Interval) \
                                                + (pToolBar->Interval << 1));
        }
        else
        {
            pButton->OptionRect.height = (TOOLBAR_OPTION_HEIGHT << 1);
        }

        if (TB_eTOP == pToolBar->Direction)
        {
            pButton->OptionRect.top = pButton->NameRect.top;
        }
        else
        {
            pButton->OptionRect.top = pToolBar->BarRect.top - pButton->OptionRect.height;
        }
    }
    else if ((TB_eLEFT == pToolBar->Direction) || (TB_eRIGHT == pToolBar->Direction))
    {
        pButton->OptionRect.width = pButton->NameRect.width;
        pButton->OptionRect.left = pButton->NameRect.left;

        if (BTN_TYPE_EDIT != pButton->Type)
        {
            pButton->OptionRect.height = (T_LEN)(pButton->OptionNum \
                                                * (TOOLBAR_OPTION_HEIGHT + pToolBar->Interval) \
                                                + (pToolBar->Interval << 1));
        }
        else
        {
            pButton->OptionRect.height = (TOOLBAR_OPTION_HEIGHT << 1);
        }

        if ((pButton->NameRect.top + pButton->OptionRect.height) < Fwl_GetLcdHeight())
        {
            pButton->OptionRect.top = pButton->NameRect.top;
        }
        else if (pButton->NameRect.top + pButton->NameRect.height > pButton->OptionRect.height)
        {
            pButton->OptionRect.top = pButton->NameRect.top + pButton->NameRect.height \
                                      - pButton->OptionRect.height;
        }
        else
        {
            //Fwl_Print(C3, M_CTRL, "Too much options of the focus button! You'd better cut some ones!");
            pButton->OptionRect.top = FstBtnRect.top;
        }
    }
}


//check if need to show the prev scroll icon or the back scroll icon.
static T_VOID ToolBar_CheckScroll(T_pTOOLBAR pToolBar)
{
    T_pBUTTON   p = AK_NULL;
    T_U32       i = 0;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_CheckScroll()>> pToolBar is AK_NULL");

    //the focusbtn shoule always behind or the same as the headbtn and the firstbtn
    if (pToolBar->ButtonShownNum < pToolBar->ButtonTotalNum)
    {
        //check whether to show the prev-scroll icon.
        if (pToolBar->pFirstBtn != pToolBar->pHeadBtn)
        {
            pToolBar->ScrollPrev = AK_TRUE;
        }
        else
        {
            pToolBar->ScrollPrev = AK_FALSE;
        }

        //check whether to show the back-scroll icon.
        p = pToolBar->pFirstBtn;
        for (i = 0; i < pToolBar->ButtonShownNum; i++)
        {
            p = p->pNext;
            if (AK_NULL == p)
                break;
        }

        if (AK_NULL != p)
        {
            pToolBar->ScrollBack = AK_TRUE;
        }
        else
        {
            pToolBar->ScrollBack = AK_FALSE;
        }
    }
    else
    {
        pToolBar->ScrollPrev = AK_FALSE;
        pToolBar->ScrollBack = AK_FALSE;
        pToolBar->pFirstBtn = pToolBar->pHeadBtn;
    }
}



/* Move the foucs button to the prev one or the next one, by the direction. 
   Jump over the disabled button
 */
static T_VOID ToolBar_MoveFocusButton(T_pTOOLBAR pToolBar, T_TB_MOVE_DIRECTION Direction)
{
    T_pBUTTON   pOldFocusBtn = AK_NULL;
    T_pBUTTON   pTemp, pTemp2;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_MoveFocusButton()>> pToolBar is null!");
    AK_ASSERT_PTR_VOID(pToolBar->pFocusBtn, "ToolBar_MoveFocusButton()>> pToolBar->pFocusBtn is null!");
    AK_ASSERT_PTR_VOID(pToolBar->pHeadBtn, "ToolBar_MoveFocusButton()>> pToolBar->pHeadBtn is null!");
    AK_ASSERT_VAL_VOID(((Direction == TB_MOVE_PREV) || (Direction == TB_MOVE_NEXT)), \
                        "ToolBar_MoveFocusButton()>> Invalid move direction!");

    pOldFocusBtn = pToolBar->pFocusBtn;

    if (TB_MOVE_NEXT == Direction)
    {
        pTemp = pOldFocusBtn->pNext;

        while((AK_NULL != pTemp) && (BTN_STATE_DISABLED == pTemp->State))
        {
            pTemp = pTemp->pNext;
        }

        if (AK_NULL == pTemp)
        {
            //find the first button that is not disabled!
            pTemp = pToolBar->pHeadBtn;

            while ((AK_NULL != pTemp) && (BTN_STATE_DISABLED == pTemp->State))
            {
                pTemp = pTemp->pNext;
            }
        }

        pToolBar->pFocusBtn = pTemp;
    }
    else    //(TB_MOVE_PREV == Direction)
    {
        pTemp = pOldFocusBtn->pPrev;

        while ((AK_NULL != pTemp) && (BTN_STATE_DISABLED == pTemp->State))
        {
            pTemp = pTemp->pPrev;
        }

        if (AK_NULL == pTemp)
        {
            //find the last button that is not disabled
            pTemp = pOldFocusBtn;   //not use pOldFocusBtn->pNext, for the case the pNext is null

            while (AK_NULL != pTemp)
            {
                if (BTN_STATE_DISABLED != pTemp->State)
                    pTemp2 = pTemp;

                pTemp = pTemp->pNext;
            }
            pTemp = pTemp2;
        }

        pToolBar->pFocusBtn = pTemp;
    }

    ToolBar_SetButtonState(pToolBar, pToolBar->pFocusBtn, BTN_STATE_FOCUS);
    ToolBar_SetButtonState(pToolBar, pOldFocusBtn, BTN_STATE_NORMAL);

    ToolBar_UpdateFirstShownButton(pToolBar);
    ToolBar_UpdateButtonNameRect(pToolBar->pFocusBtn, pToolBar);

    //ToolBar_CheckScroll should be called after calling UpdateFirstShownButton().
    ToolBar_CheckScroll(pToolBar);
}


//Move the focus option of pointed button.
static T_VOID ToolBar_MoveFocusButtonOption(T_pBUTTON pButton, T_TB_MOVE_DIRECTION Direction)
{
    T_BUTTON_OPTION *pOldFocusOption, *pTemp;

    AK_ASSERT_PTR_VOID(pButton, "ToolBar_MoveFocusButtonOption>> pButton is null!");
    AK_ASSERT_PTR_VOID(pButton->pFocusOption, \
                       "ToolBar_MoveFocusButtonOption()>> pFocusOption is null!");
    AK_ASSERT_PTR_VOID(pButton->pHeadOption, \
                       "ToolBar_MoveFocusButtonOption()>> pHeadOption is null!");
    AK_ASSERT_VAL_VOID(((Direction == TB_MOVE_PREV) || (Direction == TB_MOVE_NEXT)), \
                       "ToolBar_MoveFocusButtonOption()>> Invalid move direction!");
    AK_ASSERT_VAL_VOID((BTN_TYPE_EDIT != pButton->Type), \
                       "ToolBar_MoveFocusButtonOption()>> BTN_TYPE_EDIT type button has no options!");

    pOldFocusOption = pButton->pFocusOption;

    if (TB_MOVE_NEXT == Direction)
    {
        if (AK_NULL == pOldFocusOption->pNext)
        {
            pButton->pFocusOption = pButton->pHeadOption;
        }
        else
        {
            pButton->pFocusOption = pOldFocusOption->pNext;
        }
    }
    else    //(TB_MOVE_PREV == Direction)
    {
        if (pOldFocusOption == pButton->pHeadOption)
        {
            //find the tail button of pToolBar, and set the tail button as the focus button.
            pTemp = pOldFocusOption;

            while (AK_NULL != pTemp)
            {
                if (AK_NULL == pTemp->pNext)
                {
                    //the tail button finded!
                    pButton->pFocusOption = pTemp;
                    break;
                }

                pTemp = pTemp->pNext;
            }
        }
        else
        {
            pButton->pFocusOption = pOldFocusOption->pPrev;
        }
    }

}


//delete all options from a button that is not BTN_TYPE_EDIT type .
static T_VOID ToolBar_DelAllOptionOfButton(T_pBUTTON pButton)
{
    T_BUTTON_OPTION *p1, *p2;

    AK_ASSERT_PTR_VOID(pButton, "ToolBar_DelAllOptionOfButton()>> pButton is AK_NULL");
    AK_ASSERT_VAL_VOID((BTN_TYPE_EDIT != pButton->Type), \
                       "ToolBar_DelAllOptionOfButton()>> BTN_TYPE_EDIT type button has no options!");

    p1 = pButton->pHeadOption;

    while (AK_NULL != p1)
    {
        p2 = p1->pNext;

        p1 = Fwl_Free(p1);
        p1 = p2;
    }

    pButton->pHeadOption = AK_NULL;
    pButton->pFocusOption = AK_NULL;
    pButton->pOldFocusOption = AK_NULL;
}


//delete all buttons from a toolbar. just called when freeing the toolbar.
static T_VOID ToolBar_DelAllButton(T_pTOOLBAR pToolBar)
{
    T_pBUTTON p1, p2;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_DelAllButton()>> pToolBar is null!");

    p1 = pToolBar->pHeadBtn;

    while (AK_NULL != p1)
    {
        p2 = p1->pNext;

        if (TB_eMODE_SHOWN_ON_YUV == pToolBar->ShownMode)
        {
            //free all stete-icon YUV buf
            if (AK_NULL != p1->StateIcon[0])
                Fwl_Free((T_pVOID)p1->StateIcon[0]);
        }

        if (BTN_TYPE_EDIT != p1->Type)
        {
            ToolBar_DelAllOptionOfButton(p1);
        }

        p1 = Fwl_Free(p1);
        p1 = p2;
    }

    pToolBar->pHeadBtn = AK_NULL;
    pToolBar->pFocusBtn = AK_NULL;
    pToolBar->pFirstBtn = AK_NULL;
    pToolBar->ButtonTotalNum = 0;
    pToolBar->ButtonShownNum = 0;
}


//set foucs button by button ptr.
static T_BOOL ToolBar_SetFocusButton(T_pTOOLBAR pToolBar, T_pBUTTON pButton)
{
    T_pBUTTON   pOldfocusBtn;

    AK_ASSERT_PTR(pToolBar, "ToolBar_SetFocusButtonById()>> pToolBar is null!", AK_FALSE);
    AK_ASSERT_PTR(pButton, "ToolBar_SetFocusButtonById()>> pButton is null!", AK_FALSE);

    //a disabled button can't be set to focus.
    if (BTN_STATE_DISABLED == pButton->State)
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_SetFocusButtonById()>> the button is disabled!");
        return AK_FALSE;
    }

    if (pButton == pToolBar->pFocusBtn)
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_SetFocusButtonById()>> It's already the focus button!");
        return AK_TRUE;
    }
    else
    {
        pOldfocusBtn = pToolBar->pFocusBtn;

        pToolBar->pFocusBtn = pButton;

        //set the button to focus state, and set the oldfocus button to normal state.
        ToolBar_SetButtonState(pToolBar, pToolBar->pFocusBtn, BTN_STATE_FOCUS);
        ToolBar_SetButtonState(pToolBar, pOldfocusBtn, BTN_STATE_NORMAL);

        //update the values for displaying.
        ToolBar_UpdateFirstShownButton(pToolBar);
        ToolBar_UpdateButtonNameRect(pToolBar->pFocusBtn, pToolBar);
        ToolBar_CheckScroll(pToolBar);

        return AK_TRUE;
    }
}


//set button state by button ptr.
static T_VOID ToolBar_SetButtonState(T_pTOOLBAR pToolBar, T_pBUTTON pButton, T_BUTTON_STATE state)
{
    T_BUTTON_STATE  oldstate;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_SetButtonState()>> pToolBar is AK_NULL");
    AK_ASSERT_PTR_VOID(pButton, "ToolBar_SetButtonState()>> pButton is AK_NULL");

    oldstate = pButton->State;

    if (oldstate != state)
    {
        pButton->State = state;

        if (BTN_STATE_DISABLED == state)
        {
            //if a button is disabled, the total button number will decrease 1.
            pToolBar->ButtonTotalNum--;
        }
        else if (BTN_STATE_DISABLED == oldstate)
        {
            //if a disabled button is enabled, the total button number will increase 1.
            pToolBar->ButtonTotalNum++;
        }
        else
        {
            return;
        }

        //just when a button is disabled or enabled, it's need to do th following.
        ToolBar_UpdateBarRect(pToolBar);

        ToolBar_UpdateFirstShownButton(pToolBar);
        ToolBar_UpdateButtonNameRect(pToolBar->pFocusBtn, pToolBar);
        ToolBar_CheckScroll(pToolBar);
    }
}

//Set button state in a toolbar by button id.
static T_BOOL ToolBar_SetButtonStateByID(T_pTOOLBAR pToolBar, T_U32 ButtonId, T_BUTTON_STATE state)
{
    T_pBUTTON   p;

    AK_ASSERT_PTR(pToolBar, "ToolBar_SetButtonStateByID()>> pToolBar is AK_NULL", AK_FALSE);

    p = ToolBar_GetButtonById(pToolBar, ButtonId);

    if (AK_NULL == p)
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_SetButtonStateByID()>> not find button with %d", ButtonId);
        return AK_FALSE;
    }
    else
    {
        ToolBar_SetButtonState(pToolBar, p, state);
        return AK_TRUE;
    }
}

//get the state a button by button id.
static T_BUTTON_STATE ToolBar_GetButtonStateByID(T_pTOOLBAR pToolBar, T_U32 ButtonId)
{
    T_pBUTTON   p;

    AK_ASSERT_PTR(pToolBar, "ToolBar_GetButtonStateByID()>> pToolBar is AK_NULL", \
                  BTN_STATE_STATE_NONE);

    p = ToolBar_GetButtonById(pToolBar, ButtonId);

    if (AK_NULL == p)
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_GetButtonStateByID()>>Not find button with %d", ButtonId);
        return BTN_STATE_STATE_NONE;
    }
    else
    {
        return p->State;
    }
}

/* match a button by the keyId. if no button matched, return false,
   otherwise return true and set the ButtonId.
   disabled button will not be matched by any key.
 */
static T_BOOL ToolBar_HotkeyMatching(T_pTOOLBAR pToolBar, T_eKEY_ID KeyId, T_U32 *ButtonId)
{
    T_pBUTTON   p;

    AK_ASSERT_PTR(pToolBar, "ToolBar_HotkeyMatching()>> pToolBar is null!", AK_FALSE);

    p = pToolBar->pHeadBtn;

    while (AK_NULL != p)
    {
        //disabled button will not be matched by any key.
        if (BTN_STATE_DISABLED != p->State)
        {
            if (p->Hotkey == KeyId)
                break;
        }

        p = p->pNext;
    }

    if (AK_NULL == p)
    {
        return AK_FALSE;
    }
    else
    {
        *ButtonId = p->Id;
        return AK_TRUE;
    }
}

//show the buttons zone. include background, prev and back scroll icon and buttons.
static T_VOID ToolBar_ShowButtonZone(T_pTOOLBAR pToolBar)
{
    T_RECT      Rect;
    T_RECT      PrevScrllRect, BackScrllRect;
    T_pBUTTON   p;
    T_U16       i;
    T_TRIANGLE_DIRECTION  dir;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_ShowButtonZone()>> pToolBar is null !");

    ToolBar_GetPrevScrollRect(pToolBar, &PrevScrllRect);
    ToolBar_GetBackScrollRect(pToolBar, &BackScrllRect);

    if (TB_eMODE_SHOWN_NORMAL ==  pToolBar->ShownMode)
    {
//        Fwl_AKBmpAlphaShow(pToolBar->back, MAIN_LCD_WIDTH, pToolBar->BarRect,
//        			Fwl_GetDispMemory565(), MAIN_LCD_WIDTH, pToolBar->BarRect, pToolBar->Trans);

		Fwl_Osd_DrawRawBmpByGray(&pToolBar->BarRect, pToolBar->back);

        if (pToolBar->ScrollPrev)
        {
            //draw prev scrool icon
            if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
                dir = RIGHT2LEFT;
            else
                dir = DOWN2UP;

//            Fwl_FillSolidTria(HRGB_LAYER, PrevScrllRect.left, PrevScrllRect.top, PrevScrllRect.width, 
//                              PrevScrllRect.height, dir, pToolBar->FontColor);
			Fwl_Osd_FillSolidTriaByGray(&PrevScrllRect,dir,pToolBar->FontColor);                              
        }

        if (pToolBar->ScrollBack)
        {
            if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
                dir = LEFT2RIGHT;
            else
                dir = UP2DOWN;
            //draw back scroll icon

//            Fwl_FillSolidTria(HRGB_LAYER, BackScrllRect.left, BackScrllRect.top, BackScrllRect.width, 
//                              BackScrllRect.height, dir, pToolBar->FontColor);
			Fwl_Osd_FillSolidTriaByGray(&BackScrllRect,dir,pToolBar->FontColor);                              
        }

        //draw buttons
        p = pToolBar->pFirstBtn;

        for (i = 0; i< pToolBar->ButtonShownNum; i++)
        {
		    T_AK_BMP    AnykaBmp;
		    
        
            //not show the disabled button.
            if (BTN_STATE_DISABLED == p->State)
                p = p->pNext;

            //get the loacation to show every button.
            if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
            {
                Rect.left = PrevScrllRect.left + PrevScrllRect.width + pToolBar->Interval \
                            + i * (pToolBar->ButtonWidth + pToolBar->Interval);
                Rect.top = pToolBar->BarRect.top + BUTTON_MARGIN;
            }
            else
            {
                Rect.top = PrevScrllRect.top + PrevScrllRect.height + pToolBar->Interval \
                           + i * (pToolBar->ButtonHeight + pToolBar->Interval);
                Rect.left = pToolBar->BarRect.left + BUTTON_MARGIN;
            }

            //draw a button.
//            if (!Fwl_AkBmpDrawFromString(HRGB_LAYER, Rect.left, Rect.top, p->StateIcon[p->State], 
//                                &g_Graph.TransColor, AK_FALSE))
//				Fwl_Print(C3, M_CTRL, "ToolBar_ShowButtonZone():	Draw Button Failure.\n");

		    AkBmpGetFromString(p->StateIcon[p->State], &AnykaBmp);
		    Rect.width = AnykaBmp.Width;
		    Rect.height = AnykaBmp.Height;
			Fwl_Osd_DrawStreamBmpByGray(&Rect , p->StateIcon[p->State]);
			
            p = p->pNext;
        }
    }
    else
    {
        //fill back ground color.
        Fwl_FillSolidRectOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                                pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                                &pToolBar->BarRect, \
                                pToolBar->BkGrndColor);

        if (pToolBar->ScrollPrev)
        {
            //draw prev scrool icon.
            if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
                dir = RIGHT2LEFT;
            else
                dir = DOWN2UP;

            Fwl_FillSolidTriaOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                                   pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                                   &PrevScrllRect, dir, pToolBar->FontColor);
        }

        if (pToolBar->ScrollBack)
        {
            //draw back scroll icon.
            if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
                dir = LEFT2RIGHT;
            else
                dir = UP2DOWN;

            Fwl_FillSolidTriaOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                                   pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                                   &BackScrllRect, dir, pToolBar->FontColor);
        }

        //draw buttons
        p = pToolBar->pFirstBtn;

        Rect.height = pToolBar->ButtonHeight;
        Rect.width = pToolBar->ButtonWidth;

        for (i = 0; i< pToolBar->ButtonShownNum; i++)
        {
            //not show the disabled button.
            if (BTN_STATE_DISABLED == p->State)
                p = p->pNext;

            //get the loacation to show every button.
            if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
            {
                Rect.left = PrevScrllRect.left + PrevScrllRect.width + pToolBar->Interval\
                            + i * (pToolBar->ButtonWidth + pToolBar->Interval);
                Rect.top = pToolBar->BarRect.top + BUTTON_MARGIN;
            }
            else
            {
                Rect.top = PrevScrllRect.top + PrevScrllRect.height + pToolBar->Interval\
                           + i * (pToolBar->ButtonHeight + pToolBar->Interval);
                Rect.left = pToolBar->BarRect.left + BUTTON_MARGIN;
            }

            //draw a button.
            Fwl_InsertYUV2BckGrndYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                                     pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                                     (T_U8 *)p->StateIcon[p->State], Rect.left, Rect.top, \
                                     Rect.width, Rect.height, &g_Graph.TransColor, pToolBar->Trans);
		
			if (AK_NULL == (p = p->pNext))
				break;
        }
    }
}


/* shown the name zone of the focus button.
   if the focus button is BTN_TYPE_SWITCH, show the focus option text, not the button name.
 */
static T_VOID ToolBar_ShowNameZone(T_pTOOLBAR pToolBar)
{
    T_RECT *pRect;
    T_RECT  FstBtnRect;
    T_USTR_INFO TextShown;
    T_POS   nm_left, nm_top;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_ShowNameZone()>> pToolBar is null !");

    ToolBar_UpdateButtonNameRect(pToolBar->pFocusBtn, pToolBar);

    pRect = &pToolBar->pFocusBtn->NameRect;
    ToolBar_GetFirstButtonRect(pToolBar, &FstBtnRect);

    if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
    {
        nm_left = FstBtnRect.left;
    }
    else
    {
        nm_left = pToolBar->pFocusBtn->NameRect.left + (pToolBar->Interval << 1);
    }

    nm_top = pToolBar->pFocusBtn->NameRect.top \
             + ((pToolBar->pFocusBtn->NameRect.height - g_Font.CHEIGHT) >> 1);

    if (BTN_TYPE_SWITCH == pToolBar->pFocusBtn->Type)
    {
        //BTN_TYPE_SWITCH, show the current option text of the focus button.
        Utl_UStrCpy(TextShown, pToolBar->pFocusBtn->pFocusOption->Text);
    }
    else
    {
        //other type button, show the button name.
        Utl_UStrCpy(TextShown, pToolBar->pFocusBtn->Name);
    }

    if (TB_eMODE_SHOWN_NORMAL ==  pToolBar->ShownMode)
    {        
//        Fwl_AKBmpAlphaShow(pToolBar->back, MAIN_LCD_WIDTH, *pRect,
//        			Fwl_GetDispMemory565(), MAIN_LCD_WIDTH, *pRect, pToolBar->Trans);
		Fwl_Osd_DrawRawBmpByGray(pRect, pToolBar->back);
        			
        			
        //dram name string
//        Fwl_UDispSpeciString(HRGB_LAYER, nm_left, nm_top, TextShown, pToolBar->FontColor, 
//                         CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen((T_U16 *)TextShown));
        Fwl_Osd_DrawUStringByGray(nm_left, nm_top, TextShown, (T_U16)Utl_UStrLen((T_U16 *)TextShown)
		        ,pToolBar->FontColor, CURRENT_FONT_SIZE );
    }
    else
    {
        //fill back color on background YUV
        Fwl_FillSolidRectOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                                pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                                pRect, \
                                pToolBar->BkGrndColor);

        //dram name string on background YUV.
        UDispSpeciStringOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                                pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                                nm_left, nm_top, TextShown, pToolBar->FontColor, \
                                CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen((T_U16 *)TextShown));
    }
}

static T_VOID ToolBar_ShowOptioItem(T_BUTTON_OPTION *pOptionItem, T_pBUTTON pButton,
                                    T_pTOOLBAR pToolBar, T_RECT ItemRect)
{
    T_COLOR Color;
    T_RECT  RadioRect, TextRect;
    T_BOOL  RadioFocus;

    AK_ASSERT_PTR_VOID(pOptionItem, "ToolBar_ShowOptioItem()>> pOptionItem is null!");
    AK_ASSERT_PTR_VOID(pButton, "ToolBar_ShowOptioItem()>> pButton is null!");

    Color = pToolBar->FontColor;

    RadioRect.height = ((g_Font.CHEIGHT * 3) >> 2);
    RadioRect.width = RadioRect.height;
    RadioRect.left = ItemRect.left + 4 + (RadioRect.width >> 1);
    RadioRect.top = ItemRect.top + (ItemRect.height >> 1);

    TextRect.left = RadioRect.left + RadioRect.width + 4;
    TextRect.height = g_Font.CHEIGHT;
    TextRect.width = 0;     //it will not be used, so it can be any value .
    TextRect.top = ItemRect.top + ((ItemRect.height - TextRect.height) >> 1);

    if (pOptionItem == pButton->pFocusOption)
    {
        //填focus颜色
        if(TB_eMODE_SHOWN_NORMAL == pToolBar->ShownMode)
        {
//            Fwl_FillSolidRect(HRGB_LAYER, ItemRect.left, ItemRect.top, 
//                              ItemRect.width, ItemRect.height, COLOR_ORANGE);
                              
			Fwl_Osd_FillSolidRectByGray(&ItemRect, 0x00808080);
        }
        else
        {
            Fwl_FillSolidRectOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                                   pToolBar->BackYUV.Width, pToolBar->BackYUV.Height,
                                   &ItemRect, \
                                   COLOR_ORANGE);
        }

        //设反显字体颜色
//        Color = COLOR_BLACK;
    }

    if(pOptionItem == pButton->pOldFocusOption)
    {
        RadioFocus = AK_TRUE;
    }
    else
    {
        RadioFocus = AK_FALSE;
    }

    if(TB_eMODE_SHOWN_NORMAL == pToolBar->ShownMode)
    {
        //draw radio icon
//        Fwl_DrawRadio(HRGB_LAYER, RadioRect.left, RadioRect.top, 
//                      (T_LEN)(RadioRect.height >> 1), RadioFocus, Color);
                      
        Fwl_Osd_DrawRadioByGray(&ItemRect,RadioRect.left -ItemRect.left,RadioRect.top - ItemRect.top
	        , (T_LEN)(RadioRect.height >> 1),RadioFocus,Color);
        //draw option text
//        Fwl_UDispSpeciString(HRGB_LAYER, TextRect.left, TextRect.top, pOptionItem->Text, Color, 
//                         CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen((T_U16 *)pOptionItem->Text));
        Fwl_Osd_DrawUStringByGray(TextRect.left, TextRect.top, pOptionItem->Text,Utl_UStrLen(pOptionItem->Text)
        		,Color,CURRENT_FONT_SIZE );
    }
    else
    {
        //draw radio icon
        Fwl_DrawRadioOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                      pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                      RadioRect.left, RadioRect.top, (T_LEN)(RadioRect.height >> 1), RadioFocus, Color);
        //draw option text
        UDispSpeciStringOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                              pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                              TextRect.left, TextRect.top, pOptionItem->Text, Color, \
                              CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen((T_U16 *)pOptionItem->Text));
    }

}

/* sub menu zone: if focus button is BTN_TYPE_EDIT, it means the edit zone;
  otherwise it means the option zone.
 */
static T_VOID ToolBar_ShowSubMenuZone(T_pTOOLBAR pToolBar)
{
    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_ShowSunMenuZone()>> pToolBar is null!");
    AK_ASSERT_VAL_VOID((BTN_TYPE_SWITCH != pToolBar->pFocusBtn->Type), \
                        "ToolBar_ShowSunMenuZone()>> focus button type is BTN_TYPE_SWITCH!");

    if (BTN_TYPE_SUBMENU == pToolBar->pFocusBtn->Type)
    {
        ToolBar_ShowOptionZone(pToolBar);
    }
    else //BTN_TYPE_EDIT
    {
        ToolBar_ShowEditZone(pToolBar);
    }
}

/* show the options area of the focus button..
   just when the BTN_TYPE_SUBMENU type button be pressed down will call this function.
 */
static T_VOID ToolBar_ShowOptionZone(T_pTOOLBAR pToolBar)
{
    T_BUTTON_OPTION *p;
    T_RECT          *pRect;
    T_RECT          Rect;
    T_U16           count = 0;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_ShowOptionZone()>> pToolBar is null!");
    AK_ASSERT_VAL_VOID((BTN_TYPE_SUBMENU == pToolBar->pFocusBtn->Type), \
                        "ToolBar_ShowOptionZone()>> focus button is not BTN_TYPE_SUBMENU one!");

    ToolBar_UpdateButtonOptionRect(pToolBar->pFocusBtn, pToolBar);

    pRect = &pToolBar->pFocusBtn->OptionRect;

    //fill back ground color
    if (TB_eMODE_SHOWN_NORMAL ==  pToolBar->ShownMode)
    {
//        Fwl_AKBmpAlphaShow(pToolBar->back, MAIN_LCD_WIDTH, *pRect,
//        			Fwl_GetDispMemory565(), MAIN_LCD_WIDTH, *pRect, pToolBar->Trans);
        			
		Fwl_Osd_DrawRawBmpByGray(pRect, pToolBar->back);
        			
    }
    else
    {
        Fwl_FillSolidRectOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                               pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                               pRect, \
                               pToolBar->BkGrndColor);
    }


    //the folling is drawing option items!
    p = pToolBar->pFocusBtn->pHeadOption;

    Rect.left = pToolBar->pFocusBtn->OptionRect.left + pToolBar->Interval;
    Rect.width = pToolBar->pFocusBtn->OptionRect.width - (pToolBar->Interval << 1);
    Rect.height = TOOLBAR_OPTION_HEIGHT + pToolBar->Interval;

    while (AK_NULL != p)
    {
        //calcute the option item location.
        Rect.top = pToolBar->pFocusBtn->OptionRect.top + pToolBar->Interval \
                 + count * Rect.height;

        //draw the item.
        ToolBar_ShowOptioItem(p, pToolBar->pFocusBtn, pToolBar, Rect);

        count++;
        p = p->pNext;
    }
}

static T_VOID ToolBar_ShowReturnIcon(T_pTOOLBAR pToolBar)
{
    T_RECT ReturnRect;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_ShowReturnIcon()>> pToolBar is null!");

    ToolBar_GetReturnIconRect(pToolBar, &ReturnRect);
    
    if (TB_eMODE_SHOWN_NORMAL ==  pToolBar->ShownMode)
    {
//        if (!Fwl_AkBmpDrawFromString(HRGB_LAYER, ReturnRect.left, ReturnRect.top, pToolBar->pReturnIcon, &g_Graph.TransColor, AK_FALSE))
//			Fwl_Print(C3, M_CTRL, "ToolBar_ShowReturnIcon():	draw Button Faliure.\n");
			
		Fwl_Osd_DrawStreamBmpByGray( &ReturnRect,pToolBar->pReturnIcon);
		
    }
    else
    {
        Fwl_InsertYUV2BckGrndYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, 
                        pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, pToolBar->pReturnIcon, 
                        ReturnRect.left, ReturnRect.top, ReturnRect.width, ReturnRect.height, &g_Graph.TransColor, pToolBar->Trans);
    }

}

static T_BOOL ToolBar_GetReturnIconRect(T_pTOOLBAR pToolBar, T_RECT* pRect)
{
    T_RECT FstBtnRect;

    AK_ASSERT_PTR(pToolBar, "ToolBar_GetReturnIconRect()>> pToolBar is null!", AK_FALSE);

    
    ToolBar_GetFirstButtonRect(pToolBar, &FstBtnRect);

    pRect->left = pToolBar->pFocusBtn->NameRect.left + pToolBar->pFocusBtn->NameRect.width 
                 - RETURN_ICON_WIDTH - (FstBtnRect.left - pToolBar->pFocusBtn->NameRect.left);

    pRect->top = pToolBar->pFocusBtn->NameRect.top +  RETURN_ICON_SPACE;

    pRect->width = RETURN_ICON_WIDTH;

    pRect->height = RETURN_ICON_HEIGHT;

	return AK_TRUE;
}

static T_VOID ToolBar_EditTypeBtn_GetShowRect(T_pTOOLBAR pToolBar, T_RECT *pEditRect, 
                                       T_RECT *pPrevTriaRect, T_RECT *pBackTriaRect)
{
    T_RECT  *pRect;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_EditTypeBtn_GetShowRect()>>pToolBar is null!");
    AK_ASSERT_VAL_VOID((BTN_TYPE_EDIT == pToolBar->pFocusBtn->Type), \
                       "ToolBar_EditTypeBtn_GetShowRect()>>focus button is not BTN_TYPE_EDIT type!");

    pRect = &pToolBar->pFocusBtn->OptionRect;

    pEditRect->width = pRect->width / 3;      // a third of the optionrect width
    pEditRect->height = g_Font.CHEIGHT + (pToolBar->Interval << 1);
    pEditRect->left = pRect->left + ((pRect->width - pEditRect->width) >> 1);
    pEditRect->top = pRect->top + (pRect->height >> 1)  + (((pRect->height >> 1) - pEditRect->height) >> 1);

    pPrevTriaRect->height = ((g_Font.CHEIGHT * 3) >> 2); 
    pPrevTriaRect->width = (pPrevTriaRect->height >> 1);
    pPrevTriaRect->left = pEditRect->left - pPrevTriaRect->width - (pToolBar->Interval << 2);
    pPrevTriaRect->top = pEditRect->top + ((pEditRect->height - pPrevTriaRect->height) >> 1);

    pBackTriaRect->height = pPrevTriaRect->height; 
    pBackTriaRect->width = pPrevTriaRect->width;
    pBackTriaRect->left = pEditRect->left + pEditRect->width + (pToolBar->Interval << 2);
    pBackTriaRect->top = pPrevTriaRect->top;

}


//show the edit submenu zone
static T_VOID ToolBar_ShowEditZone(T_pTOOLBAR pToolBar)
{
    T_pBUTTON   pEditBtn;
    T_RECT      *pRect;
    T_RECT      NameRect;
    T_RECT      PrevRect;
    T_RECT      BackRect;
    T_RECT      EditRect;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_ShowEditZone()>> pToolBar is null! ");
    AK_ASSERT_VAL_VOID((BTN_TYPE_EDIT == pToolBar->pFocusBtn->Type), \
                        "ToolBar_ShowEditZone()>> button is not BTN_TYPE_EDIT!")

    pEditBtn = pToolBar->pFocusBtn;

    ToolBar_UpdateButtonOptionRect(pToolBar->pFocusBtn, pToolBar);
    pRect = &pToolBar->pFocusBtn->OptionRect;

    NameRect.left = pRect->left + (pToolBar->Interval << 1);
    NameRect.top = pRect->top + (((pRect->height >> 1) - g_Font.CHEIGHT) >> 1);

    ToolBar_EditTypeBtn_GetShowRect(pToolBar, &EditRect, &PrevRect, &BackRect);

    if (TB_eMODE_SHOWN_NORMAL ==  pToolBar->ShownMode)
    {                          
//        Fwl_AKBmpAlphaShow(pToolBar->back, MAIN_LCD_WIDTH, *pRect,
//        			Fwl_GetDispMemory565(), MAIN_LCD_WIDTH, *pRect, pToolBar->Trans);

		Fwl_Osd_DrawRawBmpByGray(pRect,pToolBar->back );
        //draw name in the edit zone
//        Fwl_UDispSpeciString(HRGB_LAYER, NameRect.left, NameRect.top, pEditBtn->Name, 
//                        pToolBar->FontColor, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen((T_U16 *)pEditBtn->Name));
                        
        Fwl_Osd_DrawUStringByGray(NameRect.left, NameRect.top, pEditBtn->Name, (T_U16)Utl_UStrLen((T_U16 *)pEditBtn->Name)\
                        ,pToolBar->FontColor, CURRENT_FONT_SIZE);

//        Fwl_FillSolidTria(HRGB_LAYER, PrevRect.left, PrevRect.top, PrevRect.width, 
//                          PrevRect.height, RIGHT2LEFT, pToolBar->FontColor);
		Fwl_Osd_FillSolidTriaByGray(&PrevRect, RIGHT2LEFT, pToolBar->FontColor);
		
//        Fwl_FillSolidTria(HRGB_LAYER, BackRect.left, BackRect.top, BackRect.width, 
//                          BackRect.height, LEFT2RIGHT, pToolBar->FontColor);
		Fwl_Osd_FillSolidTriaByGray(&BackRect, LEFT2RIGHT, pToolBar->FontColor);

//        Fwl_FillSolidRect(HRGB_LAYER, EditRect.left, EditRect.top, EditRect.width, 
//                          EditRect.height, COLOR_ORANGE);
        Fwl_Osd_FillSolidRectByGray(&EditRect , 0x00808080);
		
    }
    
    else
    {
        //draw background color
        Fwl_FillSolidRectOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                               pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                               pRect, \
                               pToolBar->BkGrndColor);
        //draw name in the edit zone
        UDispSpeciStringOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                               pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                               NameRect.left, NameRect.top, pEditBtn->Name, \
                               pToolBar->FontColor, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen((T_U16 *)pEditBtn->Name));

        Fwl_FillSolidTriaOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                               pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                               &PrevRect, RIGHT2LEFT, pToolBar->FontColor);
        Fwl_FillSolidTriaOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                               pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                               &BackRect, LEFT2RIGHT, pToolBar->FontColor);

        Fwl_FillSolidRectOnYUV(pToolBar->BackYUV.pY, pToolBar->BackYUV.pU, pToolBar->BackYUV.pV, \
                               pToolBar->BackYUV.Width, pToolBar->BackYUV.Height, \
                               &EditRect, COLOR_ORANGE);
    }

//show edit number
    if (AK_NULL != pEditBtn->EditShowCallback)
        pEditBtn->EditShowCallback(EditRect);
}

static T_VOID ToolBar_ButtonClickDown(T_pTOOLBAR pToolBar)
{
    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_ButtonPressDown()>> pToolBar is null |\n");

    if (BTN_TYPE_NORMAL == pToolBar->pFocusBtn->Type)
    {
        pToolBar->pFocusBtn->NormalClickCallback(pToolBar->pFocusBtn->pFocusOption->Id);
    }
    else if (BTN_TYPE_SWITCH != pToolBar->pFocusBtn->Type)
    {
        //open a sub menu: a option list, or a edit bar.
        ToolBar_SetButtonState(pToolBar, pToolBar->pFocusBtn, BTN_STATE_DOWN);
        pToolBar->SubMenuFlag = AK_TRUE;
    }
    else //(BTN_TYPE_SWITCH)
    {
        //switch the focus option to the next one.
        ToolBar_MoveFocusButtonOption(pToolBar->pFocusBtn, TB_MOVE_NEXT);

        if (AK_NULL != pToolBar->pFocusBtn->NormalClickCallback)
        {
            pToolBar->pFocusBtn->NormalClickCallback(pToolBar->pFocusBtn->pFocusOption->Id);
        }
    }
}

static T_VOID ToolBar_ButtonSubMenuClickDown(T_pBUTTON pButton, T_pTOOLBAR pToolBar)
{
    AK_ASSERT_PTR_VOID(pButton, "ToolBar_ButtonOptionClickDown>> pButton is null!");
    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_ButtonOptionClickDown>> pToolBar is null!");

    pToolBar->SubMenuFlag = AK_FALSE;

    if (AK_NULL != pButton->NormalClickCallback)
    {
        pButton->NormalClickCallback(pButton->pFocusOption->Id);
    }

    //resume the button state from down to focus.
    ToolBar_SetButtonState(pToolBar, pButton, BTN_STATE_FOCUS);

    pButton->pOldFocusOption = pButton->pFocusOption;
}

//for touch screen
static T_BOOL ToolBar_CheckPointInRect(T_U16 x, T_U16 y, T_RECT Rect)
{
    if (((x >= Rect.left) && (x < Rect.left+Rect.width)) && \
            ((y >= Rect.top) && (y <= Rect.top+Rect.height)))
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

static T_pBUTTON ToolBar_GetButtonByPoint(T_pTOOLBAR pToolBar, T_U16 PointX, T_U16 PointY)
{
    T_pBUTTON   p;
    T_RECT      RectTmp, FstBtnRect;
    T_U32       count = 0;
    T_U32       i = 0;

    AK_ASSERT_PTR(pToolBar, "ToolBar_GetButtonByPoint()>> pToolBar is null!", AK_NULL);
    AK_ASSERT_PTR(pToolBar->pHeadBtn, \
                  "ToolBar_GetButtonByPoint()>> pToolBar->pHeadBtn is null!", AK_NULL);
    AK_ASSERT_PTR(pToolBar->pFirstBtn, \
                  "ToolBar_GetButtonByPoint()>> pToolBar->pFirstBtn is null!", AK_NULL);

    ToolBar_GetFirstButtonRect(pToolBar, &FstBtnRect);

    if ((TB_eTOP == pToolBar->Direction) || (TB_eBOTTOM == pToolBar->Direction))
    {
        RectTmp.left = FstBtnRect.left;
        RectTmp.top = pToolBar->BarRect.top + BUTTON_MARGIN;
        RectTmp.width = (T_LEN)(pToolBar->ButtonShownNum * (pToolBar->ButtonWidth + pToolBar->Interval));
        RectTmp.height = pToolBar->ButtonHeight;
		
		count = (PointX - RectTmp.left)/(pToolBar->ButtonWidth + pToolBar->Interval);       
    }
    else
    {
        RectTmp.left = pToolBar->BarRect.left + BUTTON_MARGIN;
        RectTmp.top = FstBtnRect.top;
        RectTmp.width = pToolBar->ButtonWidth;
        RectTmp.height = (T_LEN)pToolBar->ButtonShownNum \
                         * (pToolBar->ButtonHeight + pToolBar->Interval);

        count = (PointY - RectTmp.top)/(pToolBar->ButtonHeight + pToolBar->Interval);
    }

	if (!ToolBar_CheckPointInRect(PointX, PointY, RectTmp))
    {
        return AK_NULL;
    }
    
    p = pToolBar->pFirstBtn;

    for (i = count; i > 0; i--)
    {
        p = p->pNext;

        //jump over the Disabled Button.
        if (BTN_STATE_DISABLED == p->State)
        {
            p = p->pNext;
        }
    }

    return p;
}

static T_BUTTON_OPTION * ToolBar_GetButtonOptionByPoint(T_pBUTTON pButton, T_pTOOLBAR pToolBar,
                                                        T_U16 PointX, T_U16 PointY)
{
    T_BUTTON_OPTION *p;
    T_U32           count = 0;
    T_U32           i = 0;

    AK_ASSERT_PTR(pButton, "ToolBar_GetButtonOptionByPoint()>> pButton is null!", AK_NULL);
    AK_ASSERT_PTR(pToolBar, "ToolBar_GetButtonOptionByPoint()>> pToolBar is null!", AK_NULL);
    AK_ASSERT_PTR(pToolBar->pFocusBtn, \
                  "ToolBar_GetButtonOptionByPoint()>> pToolBar->pFocusBtn is null!", AK_NULL);

    if (pButton != pToolBar->pFocusBtn)
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_GetButtonOptionByPoint()>> pButton is not the focus button!");
        return AK_NULL;
    }

    if (!ToolBar_CheckPointInRect(PointX, PointY, pButton->OptionRect))
    {
        return AK_NULL;
    }

    count = (PointY - pButton->OptionRect.top)/(TOOLBAR_OPTION_HEIGHT + pToolBar->Interval);

    p = pButton->pHeadOption;

    for (i = count; i > 0; i--)
    {
        p = p->pNext;
    }

    return p;
}

static T_eBACK_STATE ToolBar_SubMenuTypeBtnMappingUserKey(T_pTOOLBAR pToolBar, T_MMI_KEYPAD key)
{
    AK_ASSERT_PTR(pToolBar, "ToolBar_SubMenuTypeBtnMappingUserKey()>> ptoolbar is null!", eStay);
    AK_ASSERT_VAL((BTN_TYPE_SUBMENU == pToolBar->pFocusBtn->Type), \
                  "ToolBar_SubMenuTypeBtnMappingUserKey()>> buton type is not correct!", eStay);

    switch (key.keyID)
    {
    case kbUP:
        ToolBar_MoveFocusButtonOption(pToolBar->pFocusBtn, TB_MOVE_PREV);
        return eStay;

    case kbDOWN:
        ToolBar_MoveFocusButtonOption(pToolBar->pFocusBtn, TB_MOVE_NEXT);
        return eStay;

    case kbOK:
        ToolBar_ButtonSubMenuClickDown(pToolBar->pFocusBtn, pToolBar);
		Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);		
        return eStay;

    case kbCLEAR:
        pToolBar->SubMenuFlag = AK_FALSE;

        pToolBar->pFocusBtn->pFocusOption = pToolBar->pFocusBtn->pOldFocusOption;
        ToolBar_SetButtonState(pToolBar, pToolBar->pFocusBtn, BTN_STATE_FOCUS);
		Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
        return eStay;

    default:
        Fwl_Print(C3, M_CTRL, "Invalid key to BTN_TYPE_SUBMENU type button!");
        return eStay;
    }
}

static T_eBACK_STATE ToolBar_EditTypeBtnMappingUserKey(T_pTOOLBAR pToolBar, T_MMI_KEYPAD key)
{
    T_pBUTTON   pBtn;

    AK_ASSERT_PTR(pToolBar, "ToolBar_EditTypeBtnMappingUserKey()>> ptoolbar is null!", eStay);
    AK_ASSERT_VAL((BTN_TYPE_EDIT == pToolBar->pFocusBtn->Type), \
                  "ToolBar_EditTypeBtnMappingUserKey()>> buton type is not correct!", eStay);

    pBtn = pToolBar->pFocusBtn;

    switch (key.keyID)
    {
    case kbLEFT:
    case kbRIGHT:
        if (AK_NULL != pBtn->EditClickCallback)
        {
            pBtn->EditClickCallback(key.keyID);
        }

        return eStay;

    case kbCLEAR:
    case kbOK:
        pToolBar->SubMenuFlag = AK_FALSE;
        ToolBar_SetButtonState(pToolBar, pToolBar->pFocusBtn, BTN_STATE_FOCUS);
		Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
		return eStay;

    default:
        return eStay;
    }
}

static T_BOOL ToolBar_GetReturnIconRes(T_pTOOLBAR pToolBar)
{
    T_pDATA bmp;
    T_U32   pnglen;

    AK_ASSERT_PTR(pToolBar, "ToolBar_GetReturnIcon()>> ptoolbar is null!", AK_FALSE);
	
#ifdef CAMERA_SUPPORT

    bmp = (T_pDATA)Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_CAMERA_TOOLBAR_RETURN, &pnglen);
#endif

    if (TB_eMODE_SHOWN_ON_YUV ==  pToolBar->ShownMode)
    {
        pToolBar->pReturnIcon = Fwl_Malloc(RETURN_ICON_WIDTH * RETURN_ICON_HEIGHT * 3 / 2);

        //Change bmp to YUV
        Eng_Bmp2Yuv(bmp , pToolBar->pReturnIcon);
    }
	else
	{
		pToolBar->pReturnIcon = bmp;
	}
	
    if (pToolBar->pReturnIcon == AK_NULL)
    {
        return AK_FALSE;
    }
    else
    {
        return AK_TRUE;
    }

}

static T_eBACK_STATE ToolBar_HandlerUserKey(T_pTOOLBAR pToolBar, T_MMI_KEYPAD phyKey)
{
	T_TB_DIRECTION  MoveDirec;
	T_U32           MatchBtnId;
	T_eBACK_STATE   RetVal = eStay;
	
	if (!pToolBar->SubMenuFlag)   //no button is pressed down.
    {
        //hotkey machting
        if (ToolBar_HotkeyMatching(pToolBar, phyKey.keyID, &MatchBtnId))
        {
            if (PRESS_LONG == phyKey.pressType)
                Fwl_KeyStop();

            if (PRESS_UP != phyKey.pressType)
            {
                ToolBar_SetFocusButtonById(pToolBar, MatchBtnId);
                phyKey.keyID = kbOK;
            }
        }

        switch (phyKey.keyID)
        {
        case kbLEFT:
        case kbRIGHT:
            if ((TB_eTOP != pToolBar->Direction) && (TB_eBOTTOM != pToolBar->Direction))
            {
                Fwl_Print(C3, M_CTRL, "Invalid key to this toolbar!");
                return eStay;
            }

            if (kbLEFT == phyKey.keyID)
            {
                MoveDirec = TB_MOVE_PREV;
            }
            else
            {
                MoveDirec = TB_MOVE_NEXT;
            }
            break;

        case kbUP:
        case kbDOWN:
            if ((TB_eLEFT != pToolBar->Direction) && (TB_eRIGHT != pToolBar->Direction))
            {
                Fwl_Print(C3, M_CTRL, "Invalid key to this toolbar!");
                return eStay;
            }

            if (kbUP == phyKey.keyID)
            {
                MoveDirec = TB_MOVE_PREV;
            }
            else
            {
                MoveDirec = TB_MOVE_NEXT;
            }            
            break;

        case kbOK:
            ToolBar_ButtonClickDown(pToolBar);

            if (BTN_TYPE_SWITCH == pToolBar->pFocusBtn->Type)
            {
                return eNext;
            }
            break;

        case kbCLEAR:
            return eReturn;
            break;

        default:
			return eStay;
            break;
        }
		
		ToolBar_MoveFocusButton(pToolBar, MoveDirec);
    }
	//SubMenuFlag flag is true. any BTN_TYPE_SUBMENU or BTN_TYPE_EDIT type button is pressed down.
    else if (BTN_TYPE_SUBMENU == pToolBar->pFocusBtn->Type)
    {
        RetVal = ToolBar_SubMenuTypeBtnMappingUserKey(pToolBar, phyKey);
    }
    else if (BTN_TYPE_EDIT == pToolBar->pFocusBtn->Type)
    {
        RetVal = ToolBar_EditTypeBtnMappingUserKey(pToolBar, phyKey);
    }    

	return RetVal;
}

static T_eBACK_STATE ToolBar_HandlerTscr(T_pTOOLBAR pToolBar, T_EVT_PARAM *pParam)
{
	T_pBUTTON       pBtn = AK_NULL;
	T_BUTTON_OPTION *pOptn = AK_NULL;
	T_eBACK_STATE   RetVal = eStay;
	T_RECT          RectTmp = {0};
	
	switch (pParam->s.Param1)
    {
    case eTOUCHSCR_UP:
		pBtn = ToolBar_GetButtonByPoint(pToolBar, pParam->s.Param2, pParam->s.Param3);
		
        if (!pToolBar->SubMenuFlag)
        { 
            if (pBtn == pToolBar->pFocusBtn)
            {
                ToolBar_ButtonClickDown(pToolBar);

                if (BTN_TYPE_SWITCH == pToolBar->pFocusBtn->Type)
                {
                    RetVal = eNext;
                }
            }
            else if (AK_NULL != pBtn)
            {
                ToolBar_SetFocusButton(pToolBar, pBtn);
            }            
            else
            {
            	//check if is in the rect [RETURN]
            	ToolBar_GetReturnIconRect(pToolBar, &RectTmp);
                if (ToolBar_CheckPointInRect(pParam->s.Param2, pParam->s.Param3, RectTmp))
                {
                    return eReturn;
                }
				
                //check the PrevScroll rect
                ToolBar_GetPrevScrollRect(pToolBar, &RectTmp);
                if (ToolBar_CheckPointInRect(pParam->s.Param2, pParam->s.Param3, RectTmp))
                {
                    ToolBar_MoveFocusButton(pToolBar, TB_MOVE_PREV);
					break;
                }
				
				//check the BackScroll rect
				ToolBar_GetBackScrollRect(pToolBar, &RectTmp);
                if (ToolBar_CheckPointInRect(pParam->s.Param2, pParam->s.Param3, RectTmp))
                {
                    ToolBar_MoveFocusButton(pToolBar, TB_MOVE_NEXT);
					break;
                }
            }
        }
        else    //SubMenuFlag == AK_TRUE
        {
            //cancle from the submenu
            if ((pBtn != AK_NULL) && (pBtn == pToolBar->pFocusBtn))
            {
                pToolBar->SubMenuFlag = AK_FALSE;

                if (BTN_TYPE_SUBMENU == pToolBar->pFocusBtn->Type)
                {
                    pToolBar->pFocusBtn->pFocusOption = pToolBar->pFocusBtn->pOldFocusOption;
                }

                ToolBar_SetButtonState(pToolBar, pToolBar->pFocusBtn, BTN_STATE_FOCUS);
				Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
				break;
            }

            if (BTN_TYPE_SUBMENU == pToolBar->pFocusBtn->Type)
            {
                pOptn = ToolBar_GetButtonOptionByPoint(pToolBar->pFocusBtn, pToolBar, \
                                               pParam->s.Param2, pParam->s.Param3);
				
                if (pOptn == pToolBar->pFocusBtn->pFocusOption)
                {
                    ToolBar_ButtonSubMenuClickDown(pToolBar->pFocusBtn, pToolBar);
                }
                else if (AK_NULL != pOptn)
                {
                    pToolBar->pFocusBtn->pFocusOption = pOptn;
                }

            }
            else if (BTN_TYPE_EDIT == pToolBar->pFocusBtn->Type
				&& pToolBar->pFocusBtn->EditClickCallback)
            {
                //do it later !
                T_RECT EditRect, PrevRect, BackRect;

                ToolBar_EditTypeBtn_GetShowRect(pToolBar, &EditRect, &PrevRect, &BackRect);

                if (ToolBar_CheckPointInRect(pParam->s.Param2, pParam->s.Param3, PrevRect))
                {
                    pToolBar->pFocusBtn->EditClickCallback(kbLEFT);
                }
                else if (ToolBar_CheckPointInRect(pParam->s.Param2, pParam->s.Param3, BackRect))
                {
                    pToolBar->pFocusBtn->EditClickCallback(kbRIGHT);
                }
             }
            //check if is in the rect [RETURN]
            Fwl_FillSolid(HRGB_LAYER, COLOR_BLACK);
        }
        break;

    case eTOUCHSCR_DOWN:
    case eTOUCHSCR_MOVE:
    default:
        break;
    }

	return RetVal;
}

//////Interface function////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief      Init the ToolBar control
 *             After initialized, the toolbar has no button. You should add some buttons to the toolbar.
 *
 * @param[in]  pToolbar  The ToolBar to be initialized.User should malloc for the toolbar before call this function.
 * @param[in]  Rect      Toolbar rect,  coordinates relative to the upper-left corner of LCD
 * @param[in]  Direction
 * @param[in]  ButtonInterval   the interval of two buttons.
 * @param[in]  BtnWidth    Button width. All buttons added later should be the same width as this value.
 * @param[in]  BtnHeitght  Button height.All buttons added later should be the same height as this value.
 * @param[in]  ShownMode To be shown on YUV data or RGB data.
 * @param[in]  BkGrndColor ToolBar back ground color.
 * @param[in]  display transparence. the valid trans is 0~255 . If it's 0, it's not transparent and the background can't be shown.
 * @return T_BOOL
 * @retval    AK_FALSE   Create failed
 * @retval    AK_TRUE    Create success
 */
T_BOOL ToolBar_Init(T_pTOOLBAR pToolBar, T_TB_DIRECTION Direction, T_LEN winW, T_LEN winH, T_U16 ButtonInterval,
                    T_U16 ButtonWidth, T_U16 ButtonHight, T_TB_SHOWN_MODE ShownMode,
                    T_COLOR FontColor, T_COLOR BkGrndColor, T_U8 Trans)
{
	T_RECT rect;
	
    AK_ASSERT_PTR(pToolBar, "ToolBar_Init()>> pToolBar is AK_NULL", AK_FALSE);
    AK_ASSERT_VAL(((TB_eMODE_SHOWN_NORMAL == ShownMode) || (TB_eMODE_SHOWN_ON_YUV == ShownMode)), \
                    "ToolBar_Init()>> ShownMode is invalid", AK_FALSE);

    pToolBar->ButtonWidth = ButtonWidth;
    pToolBar->ButtonHeight = ButtonHight;
    pToolBar->Interval = ButtonInterval;;
    pToolBar->Direction = Direction;

    pToolBar->pHeadBtn = AK_NULL;
    pToolBar->pFirstBtn = AK_NULL;
    pToolBar->pFocusBtn = AK_NULL;

    pToolBar->ButtonTotalNum = 0;
    pToolBar->ButtonShownNum = 0;
    pToolBar->showTimeCnt    = 0;

    pToolBar->ShownMode = ShownMode;
    pToolBar->Trans = Trans;

    pToolBar->SubMenuFlag = AK_FALSE;
    pToolBar->FontColor = FontColor;
    pToolBar->BkGrndColor = BkGrndColor;

    pToolBar->ScrollPrev = AK_FALSE;
    pToolBar->ScrollBack = AK_FALSE;

    pToolBar->BackYUV.pY = AK_NULL;
    pToolBar->BackYUV.pU = AK_NULL;
    pToolBar->BackYUV.pV = AK_NULL;
    pToolBar->BackYUV.Width = winW;
    pToolBar->BackYUV.Height = winH;

    if (AK_NULL == (pToolBar->back= (T_U8 *)Fwl_Malloc(MAIN_LCD_HEIGHT*MAIN_LCD_WIDTH*2)))
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_Init(): back malloc fail\n");
        return AK_FALSE;
    }
    

	rect.left = 0;
	rect.top  = 0;
	rect.width = MAIN_LCD_WIDTH;
	rect.height= MAIN_LCD_HEIGHT;
    Fwl_Clean(pToolBar->back, MAIN_LCD_WIDTH, MAIN_LCD_HEIGHT, &rect, BkGrndColor, RGB565);
	
    ToolBar_UpdateBarRect(pToolBar);
    ToolBar_GetReturnIconRes(pToolBar);
    return AK_TRUE;
}

/**
 * @brief      Add a button to a created ToolBar.
 *             You should add some buttons to a initiallized toolbar.
 *             All buttons should be the same size.
 *             if the new added button is not BTN_TYPE_EDIT, you shoule add some options to it.
 *
 * @param[in]  pToolbar    The ToolBar to add button to.
 * @param[in]  ButtonId    Id of the button to be added.
 * @param[in]  Type        Switch type or option type.
 * @param[in]  Name        Button name.
 * @param[in]  pButtonIcon Icons for all button states to show but BTN_STATE_DISABLED.
 * @return     T_pBUTTON
 * @retval     AK_NULL     Add failed
 * @retval     Button ptr  Add success
 */
T_pBUTTON ToolBar_AddButton(T_pTOOLBAR pToolBar, T_U32 ButtonId, T_BUTTON_TYPE Type,
                         T_pCWSTR Name, T_pSTATEICON_DATA pButtonIcon)
{
    T_pBUTTON   pButton = AK_NULL;
    T_pBUTTON   p1;
    T_U32       i = 0;
    T_U32       ButtonSize;

    AK_ASSERT_PTR(pToolBar, "ToolBar_AddButton()>> pToolBar is AK_NULL", AK_NULL);
    AK_ASSERT_PTR(pButtonIcon, "ToolBar_AddButton()>> pButtonIcon is AK_NULL", AK_NULL);

    //Check if the ButtonID has been used!
    if (AK_NULL != ToolBar_GetButtonById(pToolBar, ButtonId))
    {
        Fwl_Print(C3, M_CTRL, "The ButtonId [%d] has been used in this toolbar !!"
                         "You must change another button Id to use!", ButtonId);
        return AK_NULL;
    }

    pButton = (T_pBUTTON)Fwl_Malloc(sizeof(T_BUTTON));
    AK_ASSERT_PTR(pButton, "ToolBar_AddButton()>> malloc for pButton failed!", AK_NULL);

    ButtonSize = pToolBar->ButtonWidth * pToolBar->ButtonHeight;

    pButton->Id = ButtonId;
    Utl_UStrCpy(pButton->Name, Name);

    pButton->Type = Type;
    pButton->Hotkey = kbNULL;

    pButton->State = BTN_STATE_NORMAL;

    /* the folling is setting state icon data
       if the toolbar 's show mode is TB_eMODE_SHOWN_ON_YUV, malloc for the state icons,
       then change the bmp icons to yuv bufs, and store them in the state icons.
       if it is TB_eMODE_SHOWN_NORMAL, point the stateicons to the bmp resource directly!
     */
    if (TB_eMODE_SHOWN_ON_YUV == pToolBar->ShownMode)
    {
        pButton->StateIcon[0] = AK_NULL;

        //malloc yuvbuf for all state icon except the disabled state.

#if (defined(CHIP_AK3631) || defined(CHIP_AK322L) || defined(CHIP_AK3224))
        //YUV4:2:2
        pButton->StateIcon[BTN_STATE_NORMAL] = Fwl_Malloc(ButtonSize * 2 * (BTN_STATE_STATE_MAX - 1));
#else   //ak78xx YUV4:2:0
        pButton->StateIcon[BTN_STATE_NORMAL] = Fwl_Malloc(ButtonSize * 3 / 2 * (BTN_STATE_STATE_MAX - 1));
#endif

        if(AK_NULL == pButton->StateIcon[BTN_STATE_NORMAL])
        {
            Fwl_Print(C3, M_CTRL, "ToolBar_AddButton()>>malloc StateIcon YUV failed!");
            Fwl_Free(pButton);

            return AK_NULL;
        }

        for(i = 0; i < (BTN_STATE_STATE_MAX - 1); i++)
        {
            if (i > 0)
            {
#if (defined(CHIP_AK3631) || defined(CHIP_AK322L) || defined(CHIP_AK3224))
                //YUV format is 4:2:2
                pButton->StateIcon[i] = pButton->StateIcon[i - 1] + (ButtonSize * 2);
#else           //ak78xx YUV4:2:0
                pButton->StateIcon[i] = pButton->StateIcon[i - 1] + (ButtonSize * 3/2);
#endif
            }

            //Change bmp data to YUV data and store in the stateicon .
            Eng_Bmp2Yuv((T_U8 *)pButtonIcon[i], (T_U8 *)pButton->StateIcon[i]);
        }
    }
    else    //TB_eMODE_SHOWN_NORMAL, using bmp data.
    {
        for (i = 0; i < (BTN_STATE_STATE_MAX - 1); i++)
        {
            pButton->StateIcon[i] = (T_pDATA)pButtonIcon[i];
        }
    }

    pButton->NormalClickCallback = AK_NULL;
    pButton->EditClickCallback = AK_NULL;
    pButton->EditShowCallback = AK_NULL;

    pButton->OptionNum = 0;

    pButton->OptionRect.left = 0;
    pButton->OptionRect.top = 0;
    pButton->OptionRect.width = 0;
    pButton->OptionRect.height = 0;

    pButton->pHeadOption = AK_NULL;
    pButton->pFocusOption = AK_NULL;
    pButton->pOldFocusOption = AK_NULL;

    pButton->pNext = AK_NULL;
    pButton->pPrev = AK_NULL;

    if (AK_NULL == pToolBar->pHeadBtn)
    {
        //the first added button of the toolbar.
        pToolBar->pHeadBtn = pButton;
        pToolBar->pFirstBtn = pButton;
        pToolBar->pFocusBtn = pButton;
        ToolBar_SetButtonState(pToolBar, pToolBar->pFocusBtn, BTN_STATE_FOCUS);
    }
    else
    {
        p1 = pToolBar->pHeadBtn;

        while (AK_NULL != p1->pNext)
        {
            p1 = p1->pNext;
        }

        pButton->pPrev = p1;
        p1->pNext = pButton;
    }

    //the new added button's state is NORMAL, so the ButtonTotalNum add one .
    pToolBar->ButtonTotalNum++;

    //update the BarRect, and the value of pToolBar->ButtonShownNum.
    ToolBar_UpdateBarRect(pToolBar);

    //update the NameRect of the new added button.
    //It should be called after ToolBar_UpdateBarRect() here.
    ToolBar_UpdateButtonNameRect(pButton, pToolBar);

    //if the ButtonTotalNum is larger than the ButtonShownNum,
    //the prev scroll or the back scroll flag is true.
    ToolBar_CheckScroll(pToolBar);

    return pButton;
}


/**
 * @brief      Set Click callback function to a button whose type is not BTN_TYPE_EDIT.
 *             the function can't be called by other type button.
 *
 * @param[in]  pToolbar    The ToolBar where the button in..
 * @param[in]  ButtonId    Id of the button to be set callback function.
 * @param[in]  ClickCallBack  the callback funtion. When the button be clicked, this function will be called.
 * @return     T_BOOL
 * @retval     AK_FALSE  failed.May be the toolbar or the button is not exist, or the button type is BTN_TYPE_EDIT.
 * @retval     AK_TRUE   success
 */
T_BOOL ToolBar_SetNormalButtonClickCB(T_pTOOLBAR pToolBar, T_U32 ButtonId, \
                                            T_fBUTTON_CLICK_CALLBACK_NORMAL ClickCallBack)
{
    T_pBUTTON pButton;

    AK_ASSERT_PTR(pToolBar, "ToolBar_SetNormalButtonClickCallBack()>> pToolBar is null! ", AK_FALSE);

    pButton = ToolBar_GetButtonById(pToolBar, ButtonId);

    AK_ASSERT_PTR(pButton, "ToolBar_SetNormalButtonClickCallBack()>> the button is not exist!", AK_FALSE);
    AK_ASSERT_VAL((BTN_TYPE_EDIT != pButton->Type), \
                 "ToolBar_SetNormalButtonClickCallBack()>>EDIT type button can't call this func!", AK_FALSE);

    pButton->NormalClickCallback = ClickCallBack;

    return AK_TRUE;
}


/**
 * @brief      Set Click callback function to a button whose type is just BTN_TYPE_EDIT.
 *             the function can't be called by other type button.
 *
 * @param[in]  pToolbar    The ToolBar where the BTN_TYPE_EDIT button in..
 * @param[in]  ButtonId    Id of the BTN_TYPE_EDIT type button to be set callback function.
 * @param[in]  ClickCallBack  the callback funtion. When the button be clicked, this function will be called.
 * @return     T_BOOL
 * @retval     AK_FALSE  failed.May be the toolbar or the button is not exist, or the button type is not BTN_TYPE_EDIT.
 * @retval     AK_TRUE   success
 */
T_BOOL ToolBar_SetEditButtonClickCB(T_pTOOLBAR pToolBar, T_U32 ButtonId,
                                          T_fBUTTON_CLICK_CALLBACK_EDIT ClickCallBack)
{
    T_pBUTTON pButton;

    AK_ASSERT_PTR(pToolBar, "ToolBar_SetEditButtonClickCallBack()>> pToolBar is null! ", AK_FALSE);

    pButton = ToolBar_GetButtonById(pToolBar, ButtonId);

    AK_ASSERT_PTR(pButton, "ToolBar_SetEditButtonClickCallBack()>> the button is not exist!", AK_FALSE);
    AK_ASSERT_VAL((BTN_TYPE_EDIT == pButton->Type), \
                  "ToolBar_SetEditButtonClickCallBack()>>Just EDIT type button can call this func!", AK_FALSE);

    pButton->EditClickCallback = ClickCallBack;

    return AK_TRUE;
}


/**
 * @brief      Set shown-callback function to a button whose type is BTN_TYPE_EDIT.
 *             the function can't be called by other type button.
 *             when need to show the content of the button, it will be called.
 *
 * @param[in]  pToolbar    The ToolBar where the BTN_TYPE_EDIT button in..
 * @param[in]  ButtonId    Id of the BTN_TYPE_EDIT type button to be set callback function.
 * @param[in]  ClickCallBack  the callback funtion. When the button need to show, this function will be called.
 * @return     T_BOOL
 * @retval     AK_FALSE  failed.May be the toolbar or the button is not exist, or the button type is not BTN_TYPE_EDIT.
 * @retval     AK_TRUE   success
 */
T_BOOL ToolBar_SetEditButtonShowCB(T_pTOOLBAR pToolBar, T_U32 ButtonId,
                                         T_fBUTTON_SHOW_CALLBACK_EDIT ShowCallBack)
{
    T_pBUTTON pButton;

    AK_ASSERT_PTR(pToolBar, "ToolBar_SetEditButtonShowCallBack()>> pToolBar is null! ", AK_FALSE);

    pButton = ToolBar_GetButtonById(pToolBar, ButtonId);

    AK_ASSERT_PTR(pButton, "ToolBar_SetEditButtonShowCallBack()>> the button is not exist!", AK_FALSE);
    AK_ASSERT_VAL((BTN_TYPE_EDIT == pButton->Type), \
                  "ToolBar_SetEditButtonShowCallBack()>>Just EDIT type button can call this func!", AK_FALSE);

    pButton->EditShowCallback = ShowCallBack;

    return AK_TRUE;
}


/**
 * @brief      Get a button from a toolbar bu ButtonId.
 *
 * @param[in]  pToolbar  The ToolBar from which to get a button.
 * @param[in]  ButtonId  Id of the button to get.
 * @return     T_pBUTTON
 * @retval     AK_NULL   no such a button
 * @retval     pointer to the button
 */
T_pBUTTON ToolBar_GetButtonById(T_pTOOLBAR pToolBar, T_U32 ButtonId)
{
    T_pBUTTON   p;

    AK_ASSERT_PTR(pToolBar, "ToolBar_GetButtonById()>> pToolBar is AK_NULL", AK_NULL);

    p = pToolBar->pHeadBtn;

    while (AK_NULL != p)
    {
        if (p->Id == ButtonId)
        {
            break;
        }

        p = p->pNext;
    }

    if (AK_NULL == p)
    {
        //Fwl_Print(C3, M_CTRL, "ToolBar_GetButtonById()>> not find button with Id %d", ButtonId);
    }

    return p;
}


/**
 * @brief      set the focus button by ButtonId.
 *
 * @param[in]  pToolbar  The ToolBar to set focus button.
 * @param[in]  ButtonId  Id of the button to be set as focus button.
 * @return     T_BOOL
 * @retval     AK_TRUE   Success!
 * @retval     AK_FALSE  Failed! the focus button not changed!
 */
T_BOOL ToolBar_SetFocusButtonById(T_pTOOLBAR pToolBar, T_U32 ButtonId)
{
    T_pBUTTON   p;

    AK_ASSERT_PTR(pToolBar, "ToolBar_SetFocusButtonById()>> pToolBar is null!", AK_FALSE);

    p = ToolBar_GetButtonById(pToolBar, ButtonId);

    if (AK_NULL == p)
    {
        return AK_FALSE;
    }
    else
    {
        return ToolBar_SetFocusButton(pToolBar, p);
    }
}


/**
 * @brief      Set a button to diabled state.
 *             Then the button is existent but will not shown and can't be dealed in the toolbar..
 *             if a button is disabled, the total button number will decrease one.
 *
 * @param[in]  pToolbar  The ToolBar from which to delete a button.
 * @param[in]  ButtonId  Id of the button to be deleted.
 * @return     T_BOOL
 * @retval     AK_TRUE: Disabled the button successful. If the button has been the disabled state, return true too.
 * @retval     AK_FALSE:FAILED. Maybe the ptoolbar or the button is not exist.
 */
T_BOOL ToolBar_DisableButton(T_pTOOLBAR pToolBar, T_U32 ButtonId)
{
    AK_ASSERT_PTR(pToolBar, "ToolBar_DisableButton()>> pToolBar is AK_NULL", AK_FALSE);

    return ToolBar_SetButtonStateByID(pToolBar, ButtonId, BTN_STATE_DISABLED);
}


/**
 * @brief      Enable a diabled button to normal state.
 *             if a disabled button is enabled, the total button number will increase one.
 *             just disabled button could be enabled.
 *
 * @param[in]  pToolbar  The ToolBar from which to delete a button.
 * @param[in]  ButtonId  Id of the button to be deleted.
 * @return     T_BOOL
 * @retval     AK_TRUE: enable the button successful.
 * @retval     AK_FALSE:FAILED. Maybe the ptoolbar or the button is not exist.
 */
T_VOID ToolBar_EnableButton(T_pTOOLBAR pToolBar, T_U32 ButtonId)
{
    T_BUTTON_STATE oldstate;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_DisableButton()>> pToolBar is AK_NULL");

    oldstate = ToolBar_GetButtonStateByID(pToolBar, ButtonId);

    if (BTN_STATE_DISABLED != oldstate)
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_EnableButton>>Not disabled! It doesn't need to enable!");
        return;
    }

    ToolBar_SetButtonStateByID(pToolBar, ButtonId, BTN_STATE_NORMAL);
}

T_VOID ToolBar_DelButtonOption(T_pTOOLBAR pToolBar, T_U32 ButtonId)
{
	T_pBUTTON p;
	AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_DelButtonOption()>> pToolBar is AK_NULL");

	p = ToolBar_GetButtonById(pToolBar, ButtonId);

	if (BTN_TYPE_EDIT != p->Type)
	{
		ToolBar_DelAllOptionOfButton(p);
	}

	p->OptionNum = 0;
}


/**
 * @brief      Add a option to a existent button whose type is not BTN_TYPE_EDIT.
 *             You shoule add several options for this kind a button.
 *
 * @param[in]  pToolBar   the toolbar where the button is.
 * @param[in]  ButtonId   The button to which to add a option.
 * @param[in]  OptionId   option ID.
 * @param[in]  OptionText option text to show.
 * @return T_BOOL
 * @retval     AK_FALSE  Add failed
 * @retval     AK_TRUE   Add success
 */
T_BOOL ToolBar_AddOptionToButton(T_pTOOLBAR pToolBar, T_U32 ButtonId,
                                 T_U32 OptionId, T_pCWSTR OptionText)
{
    T_pBUTTON       pButton;
    T_BUTTON_OPTION *pOption;
    T_BUTTON_OPTION *p1;

    AK_ASSERT_PTR(pToolBar, "ToolBar_AddOptionToButton()>> pToolBar is AK_NULL", AK_FALSE)

    //get the button by id.
    pButton = ToolBar_GetButtonById(pToolBar, ButtonId);
    AK_ASSERT_PTR(pButton, "ToolBar_AddOptionToButton()>> pButton is AK_NULL", AK_FALSE);
    AK_ASSERT_VAL((BTN_TYPE_EDIT != pButton->Type), \
            "ToolBar_AddOptionToButton()>>EDIT type button can't add options!", AK_FALSE);

    //malloc for the new added option.
    pOption = (T_BUTTON_OPTION *)Fwl_Malloc(sizeof(T_BUTTON_OPTION));
    AK_ASSERT_PTR(pOption, "ToolBar_AddOptionToButton()>> malloc for the option failed! ", AK_FALSE);

    pOption->Id = OptionId;
    Utl_UStrCpy(pOption->Text, OptionText);
    pOption->pNext = AK_NULL;
    pOption->pPrev = AK_NULL;

    if (AK_NULL == pButton->pHeadOption)
    {
        pButton->pHeadOption = pOption;
        pButton->pFocusOption = pOption;
        pButton->pOldFocusOption = pOption;
    }
    else
    {
        p1 = pButton->pHeadOption;

        while (AK_NULL != p1->pNext)
        {
            p1 = p1->pNext;
        }

        pOption->pPrev = p1;
        p1->pNext = pOption;
    }

    pButton->OptionNum++;
    ToolBar_UpdateButtonOptionRect(pButton, pToolBar);

    return AK_TRUE;
}


/**
 * @brief      Set the focus option of a button whose type is not BTN_TYPE_EDIT.
 *             after add some options to a buton, you should set a focus option.
 *             Otherwise the focus option is the first added one.
 *
 * @param[in]  pToolBar   the toolbar where the button is.
 * @param[in]  ButtonId   The button to which to add a option.
 * @param[in]  OptionId   the Option ID to be set as focus option.
 * @return T_BOOL
 * @retval     AK_FALSE  Add failed
 * @retval     AK_TRUE   Add success
 */
T_BOOL ToolBar_SetFocusOption(T_pTOOLBAR pToolBar, T_U32 ButtonId, T_U32 OptionId)
{
    T_pBUTTON pButton;
    T_BUTTON_OPTION *p;

    pButton = ToolBar_GetButtonById(pToolBar, ButtonId);

    AK_ASSERT_PTR(pButton, "ToolBar_SetFocusOption()>> pButton is null!", AK_FALSE);
    AK_ASSERT_VAL((BTN_TYPE_EDIT != pButton->Type), \
                  "ToolBar_SetFocusOption()>> BTN_TYPE_EDIT type button has no options!", \
                  AK_FALSE);

    p = pButton->pHeadOption;

    while (AK_NULL != p)
    {
        if (OptionId == p->Id)
        {
            break;
        }

        p= p->pNext;
    }

    if (AK_NULL == p)
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_SetFocusOption()>>not find option with Id %d", OptionId);
        return AK_FALSE;
    }
    else
    {
        pButton->pFocusOption = p;
        pButton->pOldFocusOption = p;

        return AK_TRUE;
    }
}


/**
 * @brief      Set button hot key in a toolbar by button id.
 *             If you don't set any hot key, set kbNULL to it.
 *
 * @param[in]  pToolbar  The ToolBar.
 * @param[in]  ButtonId  button id of the button to be set hotkey.
 * @param[in]  key       the hot key value.
 * @return     T_VOID
 */
T_VOID ToolBar_SetButtonHotKeyByID(T_pTOOLBAR pToolBar, T_U32 ButtonId, T_eKEY_ID key)
{
    T_pBUTTON   p;

    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_SetButtonHotKeyByID()>> pToolBar is AK_NULL");

    p = ToolBar_GetButtonById(pToolBar, ButtonId);

    if (AK_NULL == p)
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_SetButtonHotKeyByID()>> not find button with Id %d", ButtonId);
        return;
    }
    else
    {
        p->Hotkey = key;
    }
}


/**
 * @brief      get the hotkey value of a button by button id.
 *
 * @param[in]  pToolbar  The ToolBar.
 * @param[in]  ButtonId  button id of the button to get hotkey.
 * @return T_eKEY_ID
 * @retval     kbNULL    this button hasn't a hotkey
 */
T_eKEY_ID ToolBar_GetButtonHotKeyByID(T_pTOOLBAR pToolBar, T_U32 ButtonId)
{
    T_pBUTTON   p;

    AK_ASSERT_PTR(pToolBar, "ToolBar_GetButtonHotKeyByID()>> pToolBar is AK_NULL", kbNULL);

    p = ToolBar_GetButtonById(pToolBar, ButtonId);

    if (AK_NULL == p)
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_GetButtonHotKeyByID()>> not find button with Id %d", ButtonId);
        return kbNULL;
    }
    else
    {
        return p->Hotkey;
    }
}


/**
 * @brief      the entry of show-module.
 *             the toolbar will be shown in different ways due to the T_TB_SHOWN_MODE.
 *
 * @param[in]  pToolbar  The ToolBar to be shown.
 * @retval     kbNULL    this button hasn't a hotkey
 */
T_VOID ToolBar_Show(T_pTOOLBAR pToolBar)
{
    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_Show()>> pToolBar is null!");

    //show the buttons, the prev-scroll icon, the back-scroll icon and the back ground of this zone.
    ToolBar_ShowButtonZone(pToolBar);

    if (!pToolBar->SubMenuFlag)
    {
        //show the name and the background.
        ToolBar_ShowNameZone(pToolBar);

        //show the return icon
        ToolBar_ShowReturnIcon(pToolBar);
    }
    else
    {
        //show the sub menu: sub menu or the edit item.
        ToolBar_ShowSubMenuZone(pToolBar);
    }
}


/**
 * @brief      The toolbar handle function.
 *             the toolbar will be shown in different ways due to the T_TB_SHOWN_MODE.
 *
 * @param[in]  pToolbar   The ToolBar to be handled.
 * @param[in]  event      event.
 * @param[in]  pParam     parameter imported.
 * @retval     T_eBACK_STATE
 */
T_eBACK_STATE ToolBar_Handler(T_pTOOLBAR pToolBar, T_EVT_CODE event, T_EVT_PARAM *pParam)
{
    T_MMI_KEYPAD    phyKey;    
    
    AK_ASSERT_PTR(pToolBar, "ToolBar_Handle()>> pToolBar is null |\n", eStay);

    switch (event)
    {
    case M_EVT_USER_KEY:
        phyKey.keyID = pParam->c.Param1;
        phyKey.pressType = pParam->c.Param2;

		pToolBar->showTimeCnt = 0;
		return ToolBar_HandlerUserKey(pToolBar, phyKey);      

        break;

    case M_EVT_TOUCH_SCREEN:
    	pToolBar->showTimeCnt = 0;
        return ToolBar_HandlerTscr(pToolBar, pParam);
        
        break;

    case M_EVT_PUB_TIMER:
        if(++pToolBar->showTimeCnt > MAX_SHOW_TIME)
        {
        	return eReturn;
        }
        break;

    case VME_EVT_TIMER:
        //do nothing
        break;
    }

    return eStay;
}


/**
 * @brief      to free all the resources allocted to the toolbar.
 *
 * @param[in]  pToolbar   The ToolBar to free.
 * @retval     T_VOID
 */
T_VOID ToolBar_Free(T_pTOOLBAR pToolBar)
{
    if (AK_NULL == pToolBar)
    {
        return;
    }

    if (AK_NULL != pToolBar->back)
    {
        pToolBar->back= Fwl_Free(pToolBar->back);
    }

	if (TB_eMODE_SHOWN_ON_YUV ==  pToolBar->ShownMode)
	{
    	if (pToolBar->pReturnIcon != AK_NULL)
    	{
			//Fwl_Free(pToolBar->pReturnIcon);
    	}
	}
	
    ToolBar_DelAllButton(pToolBar);
}


/**
 * @brief      set background color for the toolbar.
 *             the option area will use the same background color.
 *
 * @param[in]  pToolbar  The ToolBar to free.
 * @param[in]  color     the background to be set.
 * @retval     T_VOID
 */
T_VOID ToolBar_SetBackColor(T_pTOOLBAR pToolBar, T_COLOR Color)
{
    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_SetBackColor()>> pToolBar is AK_NULL");

    pToolBar->BkGrndColor = Color;
}


/**
 * @brief      To set the background YUV buf for the whole toolbar.
 *             This function is need to call just when the show mode is TB_eMODE_SHOWN_ON_YUV.
 *             For example, in cam preview module, the cam yuv buf should be set.
 *
 * @param[in]  pToolbar  The ToolBar .
 * @param[in]  pBackY    the background ybuf.
 * @param[in]  pBackU    the background ubuf.
 * @param[in]  pBackV    the background vbuf.
 * @param[in]  BackWidth the width of the background yuv buf.
 * @param[in]  BackHeight the height of the background yuv buf.
 * @retval     T_VOID
 */
T_VOID ToolBar_SetBackYUV(T_pTOOLBAR pToolBar, T_pCDATA pBackY, T_pCDATA pBackU, T_pCDATA pBackV,
                          T_LEN BackWidth, T_LEN BackHeight)
{
    AK_ASSERT_PTR_VOID(pToolBar, "ToolBar_SetBackYUV()>> pToolBar is AK_NULL");
    AK_ASSERT_PTR_VOID(pBackY, "ToolBar_SetBackYUV()>> pBackY is AK_NULL");
    AK_ASSERT_PTR_VOID(pBackU, "ToolBar_SetBackYUV()>> pBackU is AK_NULL");
    AK_ASSERT_PTR_VOID(pBackV, "ToolBar_SetBackYUV()>> pBackV is AK_NULL");

    if (TB_eMODE_SHOWN_ON_YUV != pToolBar->ShownMode)
    {
        Fwl_Print(C3, M_CTRL, "ToolBar_SetBackYUV()>> Not shown on YUV, \
                         the toolbar doesn't need back YUV buf!");
        return;
    }

    pToolBar->BackYUV.pY = (T_U8 *)pBackY;
    pToolBar->BackYUV.pU = (T_U8 *)pBackU;
    pToolBar->BackYUV.pV = (T_U8 *)pBackV;
    pToolBar->BackYUV.Width = BackWidth;
    pToolBar->BackYUV.Height = BackHeight;
}
#endif
//end of the file Ctl_ToolBar.c

