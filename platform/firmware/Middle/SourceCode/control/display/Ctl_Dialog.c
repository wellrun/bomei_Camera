/**
 * @file Ctl_Dialog.c
 * @brief ANYKA software
 * This file is used to handle the setting dialog
 * @author: ZhuSiZhe
 * @date: 2005-02-03
 * @version: 1.0
 */

#include "gbl_macrodef.h"
#include "Ctl_Dialog.h"
#include "Ctl_Progress.h"
#include "Eng_GblString.h"
#include "Eng_AkBmp.h"
#include "Eng_GblString.h"
#include "Eng_KeyMapping.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Eng_DataConvert.h"
#include "eng_string_uc.h"
#include "Lib_res_port.h"
#include "Eng_TopBar.h"
#include "lib_image_api.h"
#include "Fwl_tscrcom.h"
#include "Fwl_display.h"


extern T_pDATA p_menu_bckgrnd;

static T_VOID Dialog_GetRes(T_DIALOG *pDialog);
static T_VOID Dialog_InitRect(T_DIALOG *pDialog);
//static T_VOID Dialog_showIcon(T_DIALOG *pDialog);
static T_VOID Dialog_ShowText(T_DIALOG *pDialog, T_COLOR color);
static T_VOID Dialog_ShowSelectButton(T_DIALOG *pDialog);
static T_VOID  Dialog_ShowAdjustButton(T_DIALOG *pDalog);
static T_eBACK_STATE Dialog_UserKey_Handler(T_DIALOG *pDialog, T_MMI_KEYPAD *pPhyKey);
static T_VOID Dialog_HitButton_Handler(T_DIALOG *pDialog, T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pParam);
static T_eBACK_STATE Dialog_HitProgressBar_Handler(T_DIALOG *pDialog, T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pParam);

/**
 * @brief Initialize the variables in T_DIALOG structure.
 * 
 * @author @b ZouMai
 *  * @date 2002-06-24
 * @author wangxuwen
 * @date 2008-10-29
 * @param T_DIALOG *dialog: T_DIALOG structure
 * @return T_BOOL
 * @retval 
 */
T_BOOL  Dialog_Init(T_DIALOG *pDialog, T_U16 CurValue, T_U16 MinVal, T_U16 MaxVal, T_U16 StepValue)
{
    T_U8  i;
  
    
    AK_FUNCTION_ENTER("Dialog_Init");
    AK_ASSERT_PTR(pDialog, "Dialog_Init(): pDialog", AK_FALSE);
 
    if (g_Font.CWIDTH <= 0 || g_Font.CWIDTH > 20)       /* maybe g_Font.CWIDTH not be assigned */
    {
        return AK_FALSE;
    }

    pDialog->initFlag  = INITIALIZED_FLAG;

    
    AK_ASSERT_VAL((MinVal < MaxVal), "Dialog_Init, MinVal > MaxVal", AK_FALSE);
    AK_ASSERT_VAL((MinVal <= CurValue), "Dialog_Init, MinVal > CurValue", AK_FALSE);
    AK_ASSERT_VAL((CurValue <= MaxVal), "Dialog_Init, CurValue > MaxVal", AK_FALSE);
    AK_ASSERT_VAL((StepValue < (MaxVal - MinVal)), "Dialog_Init, StepValue > (MaxVal - MinVal)", AK_FALSE);

    pDialog->CurValue = CurValue;
    pDialog->TtlValue = MaxVal;   
    pDialog->MinValue = MinVal;
    pDialog->StepValue = StepValue;

   //default value   
    pDialog->bFcsBtn  = DBTN_OK;
    pDialog->ClickBtn = DCLK_NONE;

    pDialog->res.pPrgBar = AK_NULL;
    pDialog->res.pPrgBkgd = AK_NULL;
    
    for (i = 0; i < 2; i++)
    {
        pDialog->res.pBtnMinus[i] = AK_NULL;
        pDialog->res.pBtnPlus[i] = AK_NULL;
        pDialog->res.pBtnOk[i] = AK_NULL;
        pDialog->res.pBtnCncl[i] = AK_NULL;
    }
    
    Dialog_GetRes(pDialog);
    Dialog_InitRect(pDialog);
    
    Dialog_SetRefresh(pDialog, DIALOG_REFRESH_ALL);

    AK_FUNCTION_LEAVE("Dialog_Init");

    return AK_TRUE;
}

/**
 * @brief Free the variables in T_DIALOG structure.
 * 
 * @author wangxuwen
 * @date 2008-10-29
 * @param T_DIALOG *dialog: T_DIALOG structure
 * @param T_U16 TtlValue: total value
 * @return T_VOID
 * @retval 
 */
T_VOID Dialog_Free(T_DIALOG *pDialog)
{

}

/**
 * @brief Add title to structure T_DIALOG.
 *        Before display the information, user must add relevant strings.
 * 
 * @author @b ZouMai
 *  * @date 2002-06-28
 * @author  wangxuwen
 * @date 2008-10-29
 * @param T_DIALOG *dialog: T_DIALOG structure
 * @param  T_pSTR title: information for display
 * @return T_VOID
 * @retval 
 */
T_VOID Dialog_SetTitle(T_DIALOG *pDialog, T_pCWSTR title)
{
    AK_ASSERT_PTR_VOID(pDialog, "Dialog_SetTitle(): pDialog");
    AK_ASSERT_PTR_VOID(title, "Dialog_SetTitle(): title");
    AK_ASSERT_VAL_VOID(pDialog->initFlag == INITIALIZED_FLAG, "pDialog not initialized"); 

    TopBar_SetTitle(title);
    return;
}/* end Dialog_SetTitle(T_DIALOG *pDialog, T_pSTR item) */

/**
 * @brief Set refresh flag
 * 
 * @author @b ZouMai
 * @date 2002-06-02
 * @author wangxuwen
 * @date 2008-10-29
 * @param T_DIALOG *pDialog
 * @param  T_S16 refresh
 * @return T_VOID
 * @retval 
 */
T_VOID Dialog_SetRefresh(T_DIALOG *pDialog, T_DIALOG_REFRESH refresh)
{
    AK_ASSERT_PTR_VOID(pDialog, "Dialog_SetRefresh(): pDialog");
    AK_ASSERT_VAL_VOID(pDialog->initFlag == INITIALIZED_FLAG, "pDialog not initialized"); /* check pDialog contorl has been initialized or not */

    if ((DIALOG_REFRESH_NONE == refresh) ||(DIALOG_REFRESH_ALL == refresh))
    {
        pDialog->RefreshFlag |= refresh;
    }
    else 
    {
        if (CTL_REFRESH_ALL != pDialog->RefreshFlag)
        {
            pDialog->RefreshFlag |= refresh;
        }
    }

    return;
}


/**
 * @brief Show the message box
 * 
 * @author @b ZhuSiZhe
 * @date 2005-02-03
 * @author wangxuwen
 * @date 2008-10-29
 * @param T_DIALOG *dialog
 * @return T_VOID
 * @retval 
 */
T_VOID Dialog_ShowGraph(T_DIALOG *pDialog, T_COLOR color)
{
    T_DIALOG_RES *pRes = AK_NULL;
    T_RECT  rect;

    AK_ASSERT_PTR_VOID(pDialog, "Dialog_ShowGraph(): pDialog");
    AK_ASSERT_VAL_VOID(pDialog->initFlag == INITIALIZED_FLAG, "pDialog not initialized"); 

    pRes = &pDialog->res;

    pDialog->CurGraNum = (pDialog->CurValue - pDialog->MinValue) * pRes->PrgBarRct.width / (pDialog->TtlValue - pDialog->MinValue);

    //refresh the background image
    rect.left = pRes->PrgBkgdRct.left - pRes->BkImgRct.left;
    rect.top = pRes->PrgBkgdRct.top - pRes->BkImgRct.top;
    rect.width = pRes->PrgBkgdRct.width;
    rect.height = pRes->PrgBkgdRct.height;

    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pRes->PrgBkgdRct.left, pRes->PrgBkgdRct.top, \
               &rect, pRes->pBkImg, AK_NULL, AK_FALSE);

    //progress bar background
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->PrgBkgdRct.left, pRes->PrgBkgdRct.top, \
                    pRes->pPrgBkgd, &g_Graph.TransColor, AK_FALSE);

    rect.left = 0;
    rect.top = 0;
    rect.width = pDialog->CurGraNum;
    rect.height = pRes->PrgBarRct.height;

    //progress bar
    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pRes->PrgBarRct.left, pRes->PrgBarRct.top, \
               &rect, pRes->pPrgBar, AK_NULL, AK_FALSE);

}

/**
 * @brief Get current value
 * 
 * @author @b ZhuSiZhe
 * 
 * @author 
 * @date 2005-02-16
 * @param T_DIALOG *dialog
 * @return T_VOID
 * @retval 
 */
T_S16 Dialog_GetCurValue(T_DIALOG *pDialog)
{
    AK_ASSERT_PTR(pDialog, "Dialog_GetCurValue(): pDialog", 0);

    if (pDialog->CurValue > pDialog->TtlValue)
        return 0;
    else
        return pDialog->CurValue;
}

/**
 * @brief Show the message box
 * 
 * @author @b ZhuSiZhe
 *  * @date 2005-02-03
 * @author wangxuwen
 * @date 2008-10-29
 * @param T_DIALOG *dialog
 * @return T_VOID
 * @retval 
 */
T_VOID Dialog_Show(T_DIALOG *pDialog, T_COLOR color)
{
    T_DIALOG_RES    *pRes = AK_NULL;
    
    AK_ASSERT_PTR_VOID(pDialog, "Dialog_Show(): pDialog");
    AK_ASSERT_VAL_VOID(pDialog->initFlag == INITIALIZED_FLAG, "pDialog not initialized"); /* check pDialog contorl has been initialized or not */

    if (DIALOG_REFRESH_NONE == pDialog->RefreshFlag)
    {
        return;
    }

    pRes = &pDialog->res;

    if (DIALOG_REFRESH_BKGD & pDialog->RefreshFlag)
    {

        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->BkImgRct.left, pRes->BkImgRct.top, \
                    pRes->pBkImg, &g_Graph.TransColor, AK_FALSE);
    }
    
    if (DIALOG_REFRESH_SET & pDialog->RefreshFlag)
    {
        Dialog_ShowGraph(pDialog, color);
        Dialog_ShowText(pDialog, color);
    }

    if (DIALOG_REFRESH_SELECT_BUTTON & pDialog->RefreshFlag)
    {
        Dialog_ShowSelectButton(pDialog);
    }

    if (DIALOG_REFRESH_ADJUST_BUTTON & pDialog->RefreshFlag)
    {
        Dialog_ShowAdjustButton(pDialog);
    }

    TopBar_Show(TB_REFRESH_ALL);

//  	TopBar_Refresh();

    pDialog->RefreshFlag = DIALOG_REFRESH_NONE;

    return;
}

static T_VOID Dialog_ShowText(T_DIALOG *pDialog, T_COLOR color)
{
    T_DIALOG_RES    *pRes = AK_NULL;
    T_U16           UStrBuf[6] = {0};
    T_U32           UStrWidth = 0;
    T_U16           UStrlen = 0;
    T_POS           posX = 0;
    T_POS           posY = 0;
    T_RECT          FreshRect;

    if (AK_NULL == pDialog)
    {
        return;
    }
    
    pRes = &pDialog->res;

    if (pDialog->TtlValue > 0)
    {
        //refresh background
        FreshRect.left = pRes->textRct.left - pRes->BkImgRct.left;
        FreshRect.top = pRes->textRct.top - pRes->BkImgRct.top;
        FreshRect.width = pRes->textRct.width;
        FreshRect.height = pRes->textRct.height;

        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pRes->textRct.left, pRes->textRct.top, \
                      &FreshRect, pRes->pBkImg, AK_NULL, AK_FALSE);

        //refresh text string
        Utl_UItoa(pDialog->CurValue, UStrBuf, 10);
        UStrlen = (T_U16)Utl_UStrLen(UStrBuf);

        UStrWidth = UGetSpeciStringWidth(UStrBuf, CURRENT_FONT_SIZE, UStrlen);
        posX = (T_POS)(pRes->textRct.left + (pRes->textRct.width - UStrWidth) / 2);
        posY = (T_POS)(pRes->textRct.top + (pRes->textRct.height - g_Font.CHEIGHT) / 2);


        Fwl_UDispSpeciString(HRGB_LAYER, posX, posY, UStrBuf, color, CURRENT_FONT_SIZE, UStrlen);
    }
}

static T_VOID  Dialog_ShowSelectButton(T_DIALOG *pDialog)
{
    T_DIALOG_RES *pRes = AK_NULL;
    T_RECT  FreshRect;
    
    if (AK_NULL == pDialog)
    {
        return;
    }

    pRes = &pDialog->res;

    //refresh the background of ok and cancel button 
    FreshRect.left = pRes->OkBtRct.left - pRes->BkImgRct.left;
    FreshRect.top = pRes->OkBtRct.top - pRes->BkImgRct.top;
    FreshRect.width = pRes->CnclBtRct.left + pRes->CnclBtRct.width - pRes->OkBtRct.left;
    FreshRect.height = pRes->OkBtRct.height;
    
    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pRes->OkBtRct.left, pRes->OkBtRct.top, \
               &FreshRect, pRes->pBkImg, AK_NULL, AK_FALSE);

    if (DBTN_OK == pDialog->bFcsBtn)
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->OkBtRct.left, pRes->OkBtRct.top, pRes->pBtnOk[1], &g_Graph.TransColor, AK_FALSE);
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->CnclBtRct.left, pRes->CnclBtRct.top, pRes->pBtnCncl[0], &g_Graph.TransColor, AK_FALSE);
    }
    else 
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->OkBtRct.left, pRes->OkBtRct.top, pRes->pBtnOk[0], &g_Graph.TransColor, AK_FALSE);
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->CnclBtRct.left, pRes->CnclBtRct.top, pRes->pBtnCncl[1], &g_Graph.TransColor, AK_FALSE);
    }
}

static T_VOID  Dialog_ShowAdjustButton(T_DIALOG *pDialog)
{
    T_DIALOG_RES *pRes = AK_NULL;
    T_RECT  FreshRect;
    
    if (AK_NULL == pDialog)
    {
        return;
    }

    pRes = &pDialog->res;

    //refresh the background of minus button 
    FreshRect.left = pRes->MinusBtRct.left - pRes->BkImgRct.left;
    FreshRect.top = pRes->MinusBtRct.top - pRes->BkImgRct.top;
    FreshRect.width = pRes->MinusBtRct.width;
    FreshRect.height = pRes->MinusBtRct.height;
    
    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pRes->MinusBtRct.left, pRes->MinusBtRct.top, \
               &FreshRect, pRes->pBkImg, AK_NULL, AK_FALSE);

    //refresh the background of plus button 
    FreshRect.left = pRes->PlusBtRct.left - pRes->BkImgRct.left;
    FreshRect.top = pRes->PlusBtRct.top - pRes->BkImgRct.top;
    FreshRect.width = pRes->PlusBtRct.width;
    FreshRect.height = pRes->PlusBtRct.height;

    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pRes->PlusBtRct.left, pRes->PlusBtRct.top, \
               &FreshRect, pRes->pBkImg, AK_NULL, AK_FALSE);

    if (DCLK_NONE == pDialog->ClickBtn)
    {
    
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->MinusBtRct.left, pRes->MinusBtRct.top, \
                pRes->pBtnMinus[0], &g_Graph.TransColor, AK_FALSE);

        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->PlusBtRct.left, pRes->PlusBtRct.top, \
                pRes->pBtnPlus[0], &g_Graph.TransColor, AK_FALSE);
    }
    else if (DCLK_MINUS == pDialog->ClickBtn)
    {


        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->MinusBtRct.left, pRes->MinusBtRct.top, \
            pRes->pBtnMinus[1], &g_Graph.TransColor, AK_FALSE);
        
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->PlusBtRct.left, pRes->PlusBtRct.top, \
            pRes->pBtnPlus[0], &g_Graph.TransColor, AK_FALSE);
    }
    else if (DCLK_PLUS == pDialog->ClickBtn)
    {

        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->MinusBtRct.left, pRes->MinusBtRct.top, \
            pRes->pBtnMinus[0], &g_Graph.TransColor, AK_FALSE);

        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->PlusBtRct.left, pRes->PlusBtRct.top, \
            pRes->pBtnPlus[1], &g_Graph.TransColor, AK_FALSE);
    }


}

/**
 * @brief Message box control
 * 
 * @author ZouMai
 * @date 2001-9-16
  * @author wangxuwen
 * @date 2008-10-29
 * @param T_DIALOG *dialog: T_DIALOG structure
 * @param  T_S16 DelayTime: Delay time
 * @return T_eBACK_STATE
 * @retval BACK_OFF_1: return to the previous menu
 */
T_eBACK_STATE   Dialog_Handler(T_DIALOG *pDialog, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_MMI_KEYPAD    phyKey;
    T_eBACK_STATE   ret = eStay;

    AK_ASSERT_PTR(pDialog, "Dialog_Handler(): pDialog", eStay);
    AK_ASSERT_VAL(pDialog->initFlag == INITIALIZED_FLAG, "pDialog not initialized", eStay);   /* check pDialog contorl has been initialized or not */

    switch (Event) 
    {
        case M_EVT_USER_KEY:
            AK_ASSERT_PTR(pParam, "Dialog_Handler(): pParam", eStay);
            phyKey.keyID = (T_eKEY_ID)pParam->c.Param1;
            phyKey.pressType = (T_BOOL)pParam->c.Param2;

            ret = Dialog_UserKey_Handler(pDialog, &phyKey);
            break;

        case M_EVT_TOUCH_SCREEN:
            AK_ASSERT_PTR(pParam, "Dialog_Handler(): pParam", eStay);

            phyKey.keyID = kbNULL;
            phyKey.pressType = PRESS_SHORT;

            switch (pParam->s.Param1) 
            {
                case eTOUCHSCR_DOWN:          
                    Dialog_HitButton_Handler(pDialog, &phyKey, pParam);
                    break;

                case eTOUCHSCR_UP:                   
                    Dialog_HitButton_Handler(pDialog, &phyKey, pParam);
										
					ret = Dialog_HitProgressBar_Handler(pDialog, &phyKey, pParam);
					if (eStay == ret)
					{
	                    ret = Dialog_UserKey_Handler(pDialog, &phyKey);
					}
					
                    //touch screen up, minus and plus buttons must lose focus
                    pDialog->ClickBtn = DCLK_NONE;
                    Dialog_SetRefresh(pDialog, DIALOG_REFRESH_ADJUST_BUTTON);
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

static T_eBACK_STATE Dialog_UserKey_Handler(T_DIALOG *pDialog, T_MMI_KEYPAD *pPhyKey)
{
    T_eBACK_STATE ret = eStay;
    
    if (AK_NULL == pDialog || AK_NULL == pPhyKey)
    {
        return ret;
    }

    switch (pPhyKey->keyID)
    {
        case kbOK:
            if(DBTN_OK == pDialog->bFcsBtn)
            {                
                ret = eNext;
            }
            else
            {
                ret = eReturn;
            }
            break;
            
        case kbCLEAR:
            if (pPhyKey->pressType == PRESS_SHORT)
            {
                ret = eReturn;
            }
            else if (pPhyKey->pressType == PRESS_LONG)
            {
                ret = eHome;
            }
            break;
            
        case kbLEFT:
        case kbDOWN:
            if (pDialog->CurValue > (pDialog->StepValue + pDialog->MinValue))
            {
                pDialog->CurValue -= pDialog->StepValue;
            }
            else
            {
                pDialog->CurValue = pDialog->MinValue;
            }

            ret = eOption;
            Dialog_SetRefresh(pDialog, DIALOG_REFRESH_SET);
            break;
            
        case kbRIGHT:
        case kbUP:
            if (pDialog->CurValue < (pDialog->TtlValue - pDialog->StepValue))
            {
                pDialog->CurValue += pDialog->StepValue;
            }
            else
            {
                pDialog->CurValue = pDialog->TtlValue;
            }
            
            ret = eOption;
            Dialog_SetRefresh(pDialog, DIALOG_REFRESH_SET);
            break;
            
        default:
            break;
    }

    return ret;
}

static T_VOID Dialog_HitButton_Handler(T_DIALOG *pDialog, T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pParam)
{
    T_DIALOG_RES *pRes = AK_NULL;
    T_POS   posX = 0;
    T_POS   posY = 0;
    T_RECT  rect;
    
    if ((AK_NULL == pDialog) || (AK_NULL == pPhyKey) || (AK_NULL == pParam))
    {
        return;
    }

    pRes = &pDialog->res;

    posX = (T_POS)pParam->s.Param2;
    posY = (T_POS)pParam->s.Param3;
    
    //get the rect of cancel button
    rect = TopBar_GetRectofCancelButton();

    //hit cancel button
    if (PointInRect(&rect, posX, posY))
    {
        pPhyKey->keyID = kbCLEAR;
        pPhyKey->pressType = PRESS_SHORT;
		return;
    }


    if (PointInRect(&pRes->MinusBtRct, posX, posY))
    {
        pPhyKey->keyID = kbLEFT;
        pPhyKey->pressType = PRESS_SHORT;
        pDialog->ClickBtn = DCLK_MINUS;
        
        Dialog_SetRefresh(pDialog, DIALOG_REFRESH_ADJUST_BUTTON);
        return;    
    }

    if (PointInRect(&pRes->PlusBtRct, posX, posY))
    {
        pPhyKey->keyID = kbRIGHT;
        pPhyKey->pressType = PRESS_SHORT;
        pDialog->ClickBtn = DCLK_PLUS;

        Dialog_SetRefresh(pDialog, DIALOG_REFRESH_ADJUST_BUTTON);
        return;    
    }

    if (PointInRect(&pRes->OkBtRct, posX, posY))
    {
        if (DBTN_OK == pDialog->bFcsBtn)
        {
            pPhyKey->keyID = kbOK;
            pPhyKey->pressType = PRESS_SHORT;
        }
        else
        {
            pDialog->bFcsBtn = DBTN_OK;
        }

        Dialog_SetRefresh(pDialog, DIALOG_REFRESH_SELECT_BUTTON);
        return;    
    }

    if (PointInRect(&pRes->CnclBtRct, posX, posY))
    {
        if (DBTN_CANCEL == pDialog->bFcsBtn)
        {
            pPhyKey->keyID = kbCLEAR;
            pPhyKey->pressType = PRESS_SHORT;
        }
        else
        {
            pDialog->bFcsBtn = DBTN_CANCEL;
        }

        Dialog_SetRefresh(pDialog, DIALOG_REFRESH_SELECT_BUTTON);
        return;    
    }
}

/**
 *@author Bennis Zhang
 */
static T_eBACK_STATE Dialog_HitProgressBar_Handler(T_DIALOG *pDialog, T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pParam)
{
	T_POS posX, posY;
	T_DIALOG_RES *pRes;
	T_RECT rectProg;
	T_U16 oldValue;

	posX = (T_POS)pParam->s.Param2;
	posY = (T_POS)pParam->s.Param3;

	pRes = &pDialog->res;

	memcpy((T_U8 *)&rectProg, (T_U8 *)&pRes->PrgBarRct, sizeof(T_RECT));

	if (AK_FALSE == PointInRect(&rectProg, posX, posY))
	{
		return eStay;
	}

	oldValue = pDialog->CurValue;

	pDialog->CurValue = (posX - rectProg.left) * (pDialog->TtlValue - pDialog->MinValue) / rectProg.width + pDialog->MinValue;

	if (++pDialog->CurValue > pDialog->TtlValue)
	{
		pDialog->CurValue = pDialog->TtlValue;
	}

	if (oldValue != pDialog->CurValue)
	{
		Dialog_SetRefresh(pDialog, DIALOG_REFRESH_SET);

		return eOption;
	}

	return eStay;
}

static T_VOID Dialog_GetRes(T_DIALOG *pDialog)
{
    T_DIALOG_RES    *pRes = AK_NULL;
    T_U32           i = 0;
    
    if (AK_NULL == pDialog)
    {
        return;
    }

    pRes = &pDialog->res;

    //background image
    pRes->pBkImg = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_MAIN_BACKGROUND, AK_NULL);

    //progress bar background
    if (pRes->pPrgBkgd == AK_NULL)
    {
        pRes->pPrgBkgd = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CTL_DIALOG_PROGRESS_BACKGROUND, AK_NULL);
		AKBmpGetInfo(pRes->pPrgBkgd,  &pRes->PrgBkgdRct.width, &pRes->PrgBkgdRct.height, AK_NULL);
    }

    //progress bar
    if (pRes->pPrgBar == AK_NULL)
    {
        pRes->pPrgBar = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CTL_DIALOG_PROGRESS, AK_NULL);
		AKBmpGetInfo(pRes->pPrgBar, &pRes->PrgBarRct.width, &pRes->PrgBarRct.height, AK_NULL);
    }

    //buttons
    for (i = 0; i < 2; i++)
    {
        //minus button
        if (pRes->pBtnMinus[i] == AK_NULL)
        {
            pRes->pBtnMinus[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CTL_DIALOG_MINUS + i, AK_NULL);
			AKBmpGetInfo(pRes->pBtnMinus[i], &pRes->MinusBtRct.width, &pRes->MinusBtRct.height, AK_NULL);
        }

        //plus button
        if (pRes->pBtnPlus[i] == AK_NULL)
        {
            pRes->pBtnPlus[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CTL_DIALOG_PLUS + i, AK_NULL);
			AKBmpGetInfo(pRes->pBtnPlus[i], &pRes->PlusBtRct.width, &pRes->PlusBtRct.height, AK_NULL);	
        }

        //ok button
        if (pRes->pBtnOk[i] == AK_NULL)
        {
            pRes->pBtnOk[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CTL_DIALOG_OK + i, AK_NULL);
			AKBmpGetInfo(pRes->pBtnOk[i], &pRes->OkBtRct.width, &pRes->OkBtRct.height, AK_NULL);
        }

        //cancel button
        if (pRes->pBtnCncl[i] == AK_NULL)
        {
            pRes->pBtnCncl[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_CTL_DIALOG_CANCEL + i, AK_NULL);		
			AKBmpGetInfo(pRes->pBtnCncl[i], &pRes->CnclBtRct.width, &pRes->CnclBtRct.height, AK_NULL);
        }
    }
    
}

static T_VOID Dialog_InitRect(T_DIALOG *pDialog)
{
    T_DIALOG_RES    *pRes = AK_NULL;
    T_LEN           tmpW = 0;
    T_U16           TtlValue = 0;
    T_U8            bitcount = 1;
 
    if (AK_NULL == pDialog)
    {
        AK_ASSERT_PTR_VOID(pDialog, "Dialog_InitRect(): pDialog is null pointer");
    }

    pRes = &pDialog->res;
    TtlValue = pDialog->TtlValue;

    //background image
    pRes->BkImgRct.width = Fwl_GetLcdWidth();
    pRes->BkImgRct.height = Fwl_GetLcdHeight() - TOP_BAR_HEIGHT;
    pRes->BkImgRct.left = 0;
    pRes->BkImgRct.top = TOP_BAR_HEIGHT;
    
    //progress bar background
    pRes->PrgBkgdRct.left = (pRes->BkImgRct.width - pRes->PrgBkgdRct.width) / 2;
    pRes->PrgBkgdRct.top = (pRes->BkImgRct.height - pRes->PrgBkgdRct.height) / 2;

    //progress bar image
    pRes->PrgBarRct.left = pRes->PrgBkgdRct.left + 2;
    pRes->PrgBarRct.top = pRes->PrgBkgdRct.top + 2;

    //minus button
    pRes->MinusBtRct.left = (pRes->PrgBkgdRct.left - pRes->MinusBtRct.width) / 2;
    pRes->MinusBtRct.top = pRes->PrgBkgdRct.top;

    //plus button
    pRes->PlusBtRct.left = pRes->BkImgRct.width - pRes->MinusBtRct.left - pRes->PlusBtRct.width;
    pRes->PlusBtRct.top = pRes->PrgBkgdRct.top;

    //ok button
    tmpW = pRes->PrgBkgdRct.width / 2;
    pRes->OkBtRct.left = pRes->PrgBkgdRct.left + (tmpW - pRes->OkBtRct.width) / 2;
    pRes->OkBtRct.top = (pRes->BkImgRct.height * 2 / 3) + pRes->BkImgRct.top;

    //cancel button
    pRes->CnclBtRct.left = pRes->PrgBkgdRct.left + tmpW + (tmpW - pRes->CnclBtRct.width) / 2;
    pRes->CnclBtRct.top = pRes->OkBtRct.top;

    //算出TtlValue 是一个几位的数字
    while ((TtlValue = TtlValue / 10) != 0)
    {
        bitcount++;
    }

    //text rect
    pRes->textRct.width = bitcount * g_Font.CWIDTH * 2;
    pRes->textRct.height = g_Font.CHEIGHT;
    pRes->textRct.left = pRes->PrgBkgdRct.left + (pRes->PrgBkgdRct.width - pRes->textRct.width) / 2;
    pRes->textRct.top =  pRes->PrgBkgdRct.top - pRes->textRct.height - 5;
 
}


T_VOID Dialog_ModifyRectTop(T_DIALOG *pDialog, T_POS prgBk_top, T_POS OkBtn_top)
{
    T_DIALOG_RES    *pRes = AK_NULL;
    T_LEN           tmpW = 0;
    T_U16           TtlValue = 0;
    T_U8            bitcount = 1;
 
    if (AK_NULL == pDialog)
    {
        AK_ASSERT_PTR_VOID(pDialog, "Dialog_InitRect(): pDialog is null pointer");
    }

    pRes = &pDialog->res;
    TtlValue = pDialog->TtlValue;

    //background image
    pRes->BkImgRct.width = Fwl_GetLcdWidth();
    pRes->BkImgRct.height = Fwl_GetLcdHeight() - TOP_BAR_HEIGHT;
    pRes->BkImgRct.left = 0;
    pRes->BkImgRct.top = TOP_BAR_HEIGHT;
    
    //progress bar background
    pRes->PrgBkgdRct.left = (pRes->BkImgRct.width - pRes->PrgBkgdRct.width) / 2;
    pRes->PrgBkgdRct.top = prgBk_top;

    //progress bar image
    pRes->PrgBarRct.left = pRes->PrgBkgdRct.left + 2;
    pRes->PrgBarRct.top = pRes->PrgBkgdRct.top + 2;

    //minus button
    pRes->MinusBtRct.left = (pRes->PrgBkgdRct.left - pRes->MinusBtRct.width) / 2;
    pRes->MinusBtRct.top = pRes->PrgBkgdRct.top;

    //plus button
    pRes->PlusBtRct.left = pRes->BkImgRct.width - pRes->MinusBtRct.left - pRes->PlusBtRct.width;
    pRes->PlusBtRct.top = pRes->PrgBkgdRct.top;

    //ok button
    tmpW = pRes->PrgBkgdRct.width / 2;
    pRes->OkBtRct.left = pRes->PrgBkgdRct.left + (tmpW - pRes->OkBtRct.width) / 2;
    pRes->OkBtRct.top = OkBtn_top;

    //cancel button
    pRes->CnclBtRct.left = pRes->PrgBkgdRct.left + tmpW + (tmpW - pRes->CnclBtRct.width) / 2;
    pRes->CnclBtRct.top = pRes->OkBtRct.top;

    //算出TtlValue 是一个几位的数字
    while ((TtlValue = TtlValue / 10) != 0)
    {
        bitcount++;
    }

    //text rect
    pRes->textRct.width = bitcount * g_Font.CWIDTH * 2;
    pRes->textRct.height = GetFontHeight(CURRENT_FONT_SIZE);
    pRes->textRct.left = pRes->PrgBkgdRct.left + (pRes->PrgBkgdRct.width - pRes->textRct.width) / 2;
    pRes->textRct.top =  pRes->PrgBkgdRct.top - pRes->textRct.height - 5;
 
}

