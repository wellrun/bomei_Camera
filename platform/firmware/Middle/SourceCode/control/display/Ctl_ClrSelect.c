/**************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: Ctl_ClrSelect.c
* File flag: ClrSelect Control for menu
* File description: 
*
* Revision: 1.00
* Author: Junhua Zhao
* Modify date: 2005-04-14
*
****************************************************************/
#include "Fwl_public.h"

#ifdef SUPPORT_EBK


#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "fwl_keyhandler.h"
#include "Eng_KeyMapping.h"
#include "Ctl_ClrSelect.h"
#include "Fwl_pfAudio.h"
#include "Eng_AkBmp.h"
#include "Eng_DataConvert.h"
#include "Eng_String.h"
#include "lib_image_api.h"
#include "Fwl_pfdisplay.h"
#include "Fwl_tscrcom.h"
#include "Fwl_display.h"


#define COLOR_REFRESH_ALL           0xffff
#define COLOR_REFRESH_NONE          0x0000
#define COLOR_REFRESH_COLOR_FRAME   0x0001
#define COLOR_REFRESH_BUTTON        0X0002

#define STEP                        10

static T_VOID ClrSelect_GetRes(T_CLRSELECT *pClrSelect);
static T_VOID ClrSelect_InitResRect(T_CLRSELECT *pClrSelect);
static T_eBACK_STATE ClrSelect_UserKey_Handle(T_CLRSELECT *pClrSelect, T_MMI_KEYPAD *pPhyKey);
static T_VOID ClrSelect_HitButton_Handle(T_CLRSELECT *pClrSelect, T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pEventParm);


static T_U16 ClrSelect_GetRefresh(T_CLRSELECT *pClrSelect)
{
    return pClrSelect->RefreshFlag;
}

static T_VOID ClrSelect_SetRefresh(T_CLRSELECT *pClrSelect, T_U16 flag)
{
    if (flag != COLOR_REFRESH_NONE)
    {
        pClrSelect->RefreshFlag |= flag;
    }
    else
    {
        pClrSelect->RefreshFlag = COLOR_REFRESH_NONE;
    }
}

/**
 * @brief initialize the color select
 * 
 * @author: Junhua Zhao
 * 
 * @date 2005-07-04
 * @param T_CLRSELECT *pClrSelect
 * @return T_VOID
 * @retval 
 */
T_VOID  ClrSelect_Init(T_CLRSELECT *pClrSelect)
{
    T_RECT  rect;
    T_pRECT pRect = AK_NULL;
    T_U32   i = 0;
    
    AK_ASSERT_PTR_VOID(pClrSelect, "ClrSelect_Init(): pClrSelect");   

    ClrSelect_GetRes(pClrSelect);
    ClrSelect_InitResRect(pClrSelect);

    pRect = &pClrSelect->res.BkImgRct;

    rect.left = pRect->left;
    rect.top = pRect->top;
    rect.width = pRect->width; 
    rect.height= TOP_BAR_HEIGHT;
    Title_Init(&pClrSelect->Title, &rect);
    Title_SetData(&pClrSelect->Title,(T_pWSTR)GetCustomTitle(ctCLR_SELECT), AK_NULL);

    pClrSelect->curColor = CURRENT_RED;
    pClrSelect->TextColor = COLOR_BLACK;
    pClrSelect->color = COLOR_WHITE;
    pClrSelect->fcsBtn = BUTTON_OK;
    for (i = 0; i < 4; i++)
    {
        pClrSelect->bPrsIcn[i] = AK_FALSE;
    }

    Fwl_SetAudioVolumeStatus(AK_FALSE);

    ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_ALL);
}

/**
 * @brief free the color select
 * 
 * @author: Junhua Zhao
 * 
 * @date 2005-07-04
 * @param T_CLRSELECT *pClrSelect
 * @return T_VOID
 * @retval 
 */
T_VOID  ClrSelect_Free(T_CLRSELECT *pClrSelect)
{

    Fwl_SetAudioVolumeStatus(AK_TRUE);
}

/**
 * @brief set default color of the color select
 * 
 * @author: Junhua Zhao
 * 
 * @date 2005-07-04
 * @param T_CLRSELECT *pClrSelect
 * @param T_COLOR defcolor
 * @return T_VOID
 * @retval 
 */
T_VOID  ClrSelect_SetDefault(T_CLRSELECT *pClrSelect,T_COLOR defcolor)
{
    AK_ASSERT_PTR_VOID(pClrSelect, "ClrSelect_SetDefault(): pClrSelect");
    pClrSelect->color = defcolor;
}

/**
 * @brief show color select
 * 
 * @author: Junhua Zhao
 * 
 * @date 2005-07-04
 * @param T_CLRSELECT *pClrSelect
 * @return T_VOID
 * @retval 
 */
T_VOID  ClrSelect_Show(T_CLRSELECT *pClrSelect)
{
    T_CLRSLCT_RES   *pRes = AK_NULL;
    T_U32           i = 0;
    T_U16           refresh = COLOR_REFRESH_NONE;
    
    AK_ASSERT_PTR_VOID(pClrSelect, "ClrSelect_Show(): pClrSelect");

    pRes = &pClrSelect->res;
    refresh = ClrSelect_GetRefresh(pClrSelect);

    if (COLOR_REFRESH_ALL == refresh)
    {
        if (AK_NULL != pRes->pBkImg)
        {
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->BkImgRct.left, pRes->BkImgRct.top, pRes->pBkImg, &g_Graph.TransColor, AK_FALSE);
        }
        else
        {
            Fwl_FillSolidRect(HRGB_LAYER, pRes->BkImgRct.left, pRes->BkImgRct.top,
                        pRes->BkImgRct.width, pRes->BkImgRct.height, g_Graph.MenuBkCL);
        }

        Title_Show(&pClrSelect->Title, AK_TRUE);

        for (i=0; i<3; i++)
        {
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->ClrImgRct[i].left, pRes->ClrImgRct[i].top, pRes->pClrImg[i], &g_Graph.TransColor, AK_FALSE);
        }
    }

    //edit area
    if ((refresh & COLOR_REFRESH_COLOR_FRAME) == COLOR_REFRESH_COLOR_FRAME)
    {
        ClrSelect_ShowEdit(pClrSelect);
    }

    //button ok and cancel
    if ((refresh & COLOR_REFRESH_BUTTON) == COLOR_REFRESH_BUTTON)
    {
        ClrSelect_ShowButton(pClrSelect);
    }

    ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_NONE);
}

/**
 * @brief show edit box of color select
 * 
 * @author: Junhua Zhao
 * 
 * @date 2005-07-04
 * @param T_CLRSELECT *pClrSelect
 * @return T_VOID
 * @retval 
 */
T_VOID  ClrSelect_ShowEdit(T_CLRSELECT *pClrSelect)
{
    T_CLRSLCT_RES   *pRes = AK_NULL;
    T_RECT  range;
    T_CHR   strTemp[8] = {0};
    T_U32   UstrWidth = 0;
    T_CURCOLOR   i = 0;
    T_S32   clr = 0;
    T_U16   Ustrtmp[3] = {0};
    T_U16   UstrLen = 0;
    T_POS   posX = 0;
    T_POS   posY = 0;
    T_U8    r,g,b;

    AK_ASSERT_PTR_VOID(pClrSelect, "ClrSelect_ShowEdit(): pClrSelect");

    pRes = &pClrSelect->res;


    //edit rect
    range.left = 0;
    range.top = pRes->EditRct.top - pRes->BkImgRct.top;
    range.width = pRes->EditRct.width;
    range.height = pRes->EditRct.height;

    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pRes->EditRct.left, pRes->EditRct.top, &range, pRes->pBkImg, AK_NULL, AK_FALSE);

    //color frame
    AkColor2RGB(pClrSelect->color, LCD0_COLOR, &r, &g, &b);
    for(i=CURRENT_RED; i<CURRENT_MAX; i++)
    {       
        switch (i)
        {
            case CURRENT_RED:
                clr = r;
                break;
            case CURRENT_GREEN:
                clr = g;
                break;
            case CURRENT_BLUE:
                clr = b;
                break;
        }

        if (pClrSelect->curColor == i)
        {
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->ClrFramRct[i].left, pRes->ClrFramRct[i].top, pRes->pClrFram[i], &g_Graph.TransColor, AK_FALSE);
        }
        else
        {
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->ClrFramRct[i].left, pRes->ClrFramRct[i].top, pRes->pClrFram[3], &g_Graph.TransColor, AK_FALSE);
        }

        Utl_Itoa(clr, strTemp, 10);
        Eng_StrMbcs2Ucs(strTemp, Ustrtmp);
        UstrLen = (T_U16)Utl_UStrLen(Ustrtmp);
        UstrWidth = UGetSpeciStringWidth(Ustrtmp, CURRENT_FONT_SIZE, UstrLen);
        posX = (T_POS)(pRes->ClrFramRct[i].left + (pRes->ClrFramRct[i].width - UstrWidth) / 2);
        posY = (T_POS)(pRes->ClrFramRct[i].top + (pRes->ClrFramRct[i].height - g_Font.CHEIGHT) / 2);
        
        Fwl_UDispSpeciString(HRGB_LAYER, posX, posY, Ustrtmp, pClrSelect->TextColor, CURRENT_FONT_SIZE, UstrLen);
    } 

    //left icon
    if (AK_TRUE == pClrSelect->bPrsIcn[2])
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->IcnBkRct_L.left, pRes->IcnBkRct_L.top, pRes->pClrIcnBckLR, &g_Graph.TransColor, AK_FALSE);
    }
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->IcnRct_L.left, pRes->IcnRct_L.top, pRes->pClrIcn[2], &g_Graph.TransColor, AK_FALSE);

    //right icon
    if (AK_TRUE == pClrSelect->bPrsIcn[3])
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->IcnBkRct_R.left, pRes->IcnBkRct_R.top, pRes->pClrIcnBckLR, &g_Graph.TransColor, AK_FALSE);
    }
    Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->IcnRct_R.left, pRes->IcnRct_R.top, pRes->pClrIcn[3], &g_Graph.TransColor, AK_FALSE);

    //up and down icon
    for(i=CURRENT_RED; i<CURRENT_MAX; i++)
    {
        if (pClrSelect->curColor == i) 
        {   
            if (AK_TRUE == pClrSelect->bPrsIcn[0])
            {
                Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->IcnBkRct_U[i].left, pRes->IcnBkRct_U[i].top, pRes->pClrIcnBckUD, &g_Graph.TransColor, AK_FALSE);
            }
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->IcnRct_U[i].left, pRes->IcnRct_U[i].top, pRes->pClrIcn[0], &g_Graph.TransColor, AK_FALSE);

            if (AK_TRUE == pClrSelect->bPrsIcn[1])
            {
                Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->IcnBkRct_D[i].left, pRes->IcnBkRct_D[i].top, pRes->pClrIcnBckUD, &g_Graph.TransColor, AK_FALSE);    
            }
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->IcnRct_D[i].left, pRes->IcnRct_D[i].top, pRes->pClrIcn[1], &g_Graph.TransColor, AK_FALSE);
        }
        else
        {
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->IcnRct_U[i].left, pRes->IcnRct_U[i].top, pRes->pClrIcn[0], &g_Graph.TransColor, AK_FALSE);
            Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->IcnRct_D[i].left, pRes->IcnRct_D[i].top, pRes->pClrIcn[1], &g_Graph.TransColor, AK_FALSE);
        }
    }



    //current color
    Fwl_FillSolidRect(HRGB_LAYER, pRes->curClrRct.left, pRes->curClrRct.top, pRes->curClrRct.width, pRes->curClrRct.height, pClrSelect->color);
    Fwl_DialogFrame(HRGB_LAYER, pRes->curClrRct.left, pRes->curClrRct.top, pRes->curClrRct.width, pRes->curClrRct.height, 0xF);
}

/**
 * @brief show the ok and cancel button for color select
 * 
 * @author: 
 * @date 
 * @param T_CLRSELECT *pClrSelect
 * @return T_VOID
 * @retval 
 */
T_VOID  ClrSelect_ShowButton(T_CLRSELECT *pClrSelect)
{
    T_CLRSLCT_RES *pRes = AK_NULL;
    
    if (AK_NULL == pClrSelect)
    {
        return;
    }

    pRes = &pClrSelect->res;

    if (BUTTON_OK == pClrSelect->fcsBtn)
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->ButnOkRct.left, pRes->ButnOkRct.top, pRes->pBtnOk[1], &g_Graph.TransColor, AK_FALSE);
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->ButnCnclRct.left, pRes->ButnCnclRct.top, pRes->pBtnCncl[0], &g_Graph.TransColor, AK_FALSE);
    }
    else
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->ButnOkRct.left, pRes->ButnOkRct.top, pRes->pBtnOk[0], &g_Graph.TransColor, AK_FALSE);
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pRes->ButnCnclRct.left, pRes->ButnCnclRct.top, pRes->pBtnCncl[1], &g_Graph.TransColor, AK_FALSE);
    }
}

/**
 * @brief color select handle
 * 
 * @author Junhua Zhao
 * @date 2005-07-04
 * @param T_CLRSELECT *pClrSelect
 * @param T_EVT_CODE Event
 * @param T_EVT_PARAM *pParam
 * @return T_eBACK_STATE
 */
T_eBACK_STATE   ClrSelect_Handle(T_CLRSELECT *pClrSelect, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_MMI_KEYPAD    phyKey;
    T_eBACK_STATE   ret = eStay; 
    T_U32           i = 0;

    AK_ASSERT_PTR(pClrSelect, "ClrSelect_Handle(): pClrSelect",eStay);

    switch (Event) 
    {
        case M_EVT_USER_KEY:
            AK_ASSERT_PTR(pParam, "ClrSelect_Handle(): pParam", eStay);
            phyKey.keyID = (T_eKEY_ID)pParam->c.Param1;
            phyKey.pressType = (T_BOOL)pParam->c.Param2;

            ret = ClrSelect_UserKey_Handle(pClrSelect, &phyKey);
            break;
            
        case M_EVT_TOUCH_SCREEN:
            phyKey.keyID = kbNULL;
            phyKey.pressType = PRESS_SHORT;

            switch (pParam->s.Param1) 
            {
                case eTOUCHSCR_UP:
                    for (i = 0; i < 4; i++)
                    {
                        pClrSelect->bPrsIcn[i] = AK_FALSE;
                    }
                    ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_ALL);
                    break;

                case eTOUCHSCR_DOWN:
                    ClrSelect_HitButton_Handle(pClrSelect, &phyKey, pParam);
                    ret = ClrSelect_UserKey_Handle(pClrSelect, &phyKey);
                    break;

                case eTOUCHSCR_MOVE:
                     break;

                default:
                     break;
            }
            break;
            
        case M_EVT_PUB_TIMER:
            if (Title_ScrollText(&pClrSelect->Title, AK_TRUE))
                Fwl_RefreshDisplay();
            break;
        default:
            break;
    }

    if (ret!=eHome)
    {
        ClrSelect_ShowEdit(pClrSelect);
    }
    else 
    {
        ret = eStay;
    }
    
    return ret;
}

static T_eBACK_STATE ClrSelect_UserKey_Handle(T_CLRSELECT *pClrSelect, T_MMI_KEYPAD *pPhyKey)
{
    T_eBACK_STATE ret= eStay;
    T_U8    r = 0;
    T_U8    g = 0;
    T_U8    b = 0;

    switch (pPhyKey->keyID) 
    {
        case kbOK:
            if (pPhyKey->pressType==PRESS_SHORT)
            {
                if(BUTTON_OK == pClrSelect->fcsBtn)
                {
                    ret = eNext;
                }
                else if(BUTTON_CANCEL== pClrSelect->fcsBtn)
                {
                    ret = eReturn;  
                }                               
            }            
            break;
            
        case kbCLEAR:
            if (pPhyKey->pressType==PRESS_SHORT)
                ret = eReturn;  
            break;
            
        case kbLEFT:
            pClrSelect->curColor += CURRENT_MAX;
            pClrSelect->curColor--;         
            Fwl_Print(C3, M_EBOOK, "curcolor=%d",pClrSelect->curColor);
            pClrSelect->curColor = pClrSelect->curColor % (CURRENT_MAX);
            ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_COLOR_FRAME);
            break;
            
        case kbRIGHT:
            pClrSelect->curColor++;
            pClrSelect->curColor = pClrSelect->curColor % (CURRENT_MAX);
            ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_COLOR_FRAME);
            break;
            
        case kbVOICE_UP:
            AkColor2RGB(pClrSelect->color,LCD0_COLOR,&r,&g,&b);
            switch (pClrSelect->curColor)
            {
            case CURRENT_RED:
                r+=STEP;
                r%=(T_U8_MAX+1);
                break;
            case CURRENT_GREEN:
                g+=STEP;
                g%=(T_U8_MAX+1);
                break;
            case CURRENT_BLUE:
                b+=STEP;
                g%=(T_U8_MAX+1);
                break;
            }
            pClrSelect->color = RGB2AkColor(r,g,b,LCD0_COLOR);
            ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_COLOR_FRAME);
            break;
            
        case kbVOICE_DOWN:
            AkColor2RGB(pClrSelect->color,LCD0_COLOR,&r,&g,&b);
            switch (pClrSelect->curColor)
            {
            case CURRENT_RED:
                r-=STEP;
                r%=(T_U8_MAX+1);
                break;
            case CURRENT_GREEN:
                g-=STEP;
                g%=(T_U8_MAX+1);
                break;
            case CURRENT_BLUE:
                b-=STEP;
                g%=(T_U8_MAX+1);
                break;
            }
            pClrSelect->color = RGB2AkColor(r,g,b,LCD0_COLOR);
            ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_COLOR_FRAME);
            break;
            
        case kbDOWN:
            AkColor2RGB(pClrSelect->color,LCD0_COLOR,&r,&g,&b);
            switch (pClrSelect->curColor)
            {
            case CURRENT_RED:
                r--;
                r%=(T_U8_MAX+1);
                break;
            case CURRENT_GREEN:
                g--;
                g%=(T_U8_MAX+1);
                break;
            case CURRENT_BLUE:
                b--;
                g%=(T_U8_MAX+1);
                break;
            }
            pClrSelect->color = RGB2AkColor(r,g,b,LCD0_COLOR);
            ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_COLOR_FRAME);
            break;
            
        case kbUP:
            AkColor2RGB(pClrSelect->color,LCD0_COLOR,&r,&g,&b);
            switch (pClrSelect->curColor)
            {
            case CURRENT_RED:
                r++;
                r%=(T_U8_MAX+1);
                break;
            case CURRENT_GREEN:
                g++;
                g%=(T_U8_MAX+1);
                break;
            case CURRENT_BLUE:
                b++;
                g%=(T_U8_MAX+1);
                break;
            }
            pClrSelect->color = RGB2AkColor(r,g,b,LCD0_COLOR);
            ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_COLOR_FRAME);
            break;
            
        default:
            break;
    }
    
    return ret;
}

static T_VOID ClrSelect_HitButton_Handle(T_CLRSELECT *pClrSelect, T_MMI_KEYPAD *pPhyKey, T_EVT_PARAM *pEventParm)
{
    T_CLRSLCT_RES *pRes = AK_NULL;
    T_POS   posX = 0;
    T_POS   posY = 0;
    T_CURCOLOR   i = 0;
    T_S32   nIndex = 0;
    
    if ((AK_NULL == pPhyKey) || (AK_NULL == pEventParm))
    {
        return;
    }

    pRes = &pClrSelect->res;

    posX = (T_POS)pEventParm->s.Param2;
    posY = (T_POS)pEventParm->s.Param3;
    
    if (!PointInRect(&pRes->BkImgRct, posX, posY))
    {
        return;    
    }

    for(nIndex = 0; nIndex < CURRENT_MAX; nIndex++)
    {
        if (PointInRect(&pRes->ClrImgRct[nIndex], posX, posY) || PointInRect(&pRes->ClrFramRct[nIndex], posX, posY)  )
        {            
            pClrSelect->curColor = nIndex % CURRENT_MAX;
            ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_COLOR_FRAME);
            break;                
        }    
    }

    //button ok
    if (PointInRect(&pRes->ButnOkRct, posX, posY))
    {
        if (BUTTON_OK != pClrSelect->fcsBtn)
        {
            pClrSelect->fcsBtn = BUTTON_OK;
            ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_BUTTON);
        }
        else
        {
            pPhyKey->keyID = kbOK;
            pPhyKey->pressType = PRESS_SHORT;
        }
        return;
    }

    //button cancel
    if (PointInRect(&pRes->ButnCnclRct, posX, posY))
    {
        if (BUTTON_CANCEL != pClrSelect->fcsBtn)
        {
            pClrSelect->fcsBtn = BUTTON_CANCEL;
            ClrSelect_SetRefresh(pClrSelect, COLOR_REFRESH_BUTTON);
        }
        else
        {
            pPhyKey->keyID = kbCLEAR;
            pPhyKey->pressType = PRESS_SHORT;
        }
        return;
    }

    //left icon
    if (PointInRect(&pRes->IcnRct_L, posX, posY))
    {
        //show icon background image
        pPhyKey->keyID = kbLEFT;
        pPhyKey->pressType = PRESS_SHORT;
        pClrSelect->bPrsIcn[2] = AK_TRUE;
        return;
    }

    //right icon    
    if (PointInRect(&pRes->IcnRct_R, posX, posY))
    {
        pPhyKey->keyID = kbRIGHT;
        pPhyKey->pressType = PRESS_SHORT;
        pClrSelect->bPrsIcn[3] = AK_TRUE;
        return;
    }

    //up and down icon
    //for (i = 0; i < 3; i++)
    for(i=CURRENT_RED; i<CURRENT_MAX; i++)
    {
        if (pClrSelect->curColor == i)
        {
            if (PointInRect(&pRes->IcnRct_U[i], posX, posY))
            {
                pPhyKey->keyID = kbUP;
                pPhyKey->pressType = PRESS_SHORT;
                pClrSelect->bPrsIcn[0] = AK_TRUE;
                return;
            }

            if (PointInRect(&pRes->IcnRct_D[i], posX, posY))
            {
                pPhyKey->keyID = kbDOWN;
                pPhyKey->pressType = PRESS_SHORT;
                pClrSelect->bPrsIcn[1] = AK_TRUE;
                return;
            }
        }
    }
}

/**
 * @brief get selected color of the color select
 * 
 * @author: Junhua Zhao
 * 
 * @date 2005-04-15
 * @param T_CLRSELECT *pClrSelect
 * @return T_VOID
 * @retval 
 */
T_COLOR ClrSelect_GetColor(T_CLRSELECT *pClrSelect)
{
    AK_ASSERT_PTR(pClrSelect, "ClrSelect_GetColor(): pClrSelect",COLOR_BLACK);

    return pClrSelect->color;
}

static T_VOID ClrSelect_GetRes(T_CLRSELECT *pClrSelect)
{
    T_U32 len;
    T_U32 i;
    T_CLRSLCT_RES   *pRes = AK_NULL;

    if (AK_NULL == pClrSelect)
    {
        return;
    }

    pRes = &pClrSelect->res;

    //background image
    pRes->pBkImg = Res_GetBinResByID(&pRes->pBkImg, AK_TRUE, eRES_BMP_COLOR_BACKGROUND, &len);

    //color image
    for (i=0; i<3; i++)
    {
        pRes->pClrImg[i] = Res_GetBinResByID(&pRes->pClrImg[i], AK_TRUE, eRES_BMP_COLOR_SELECT_RED + i, &len);
    }

    for (i=0; i<4; i++)
    {
        //color select frame image
        pRes->pClrFram[i] = Res_GetBinResByID(&pRes->pClrFram[i], AK_TRUE, eRES_BMP_COLOR_SELECT_RED_FRAME + i, &len);
    }

    //up icon

    pRes->pClrIcn[0] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_COLOR_SELECT_ICON_UP, AK_NULL);
	AKBmpGetInfo(pRes->pClrIcn[0], &pRes->IcnRct_U[0].width, &pRes->IcnRct_U[0].height, AK_NULL);
    pRes->IcnRct_U[1].width = pRes->IcnRct_U[0].width;
    pRes->IcnRct_U[1].height = pRes->IcnRct_U[0].height;
    pRes->IcnRct_U[2].width = pRes->IcnRct_U[0].width;
    pRes->IcnRct_U[2].height =pRes->IcnRct_U[0].height;

    //down icon
    pRes->pClrIcn[1] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_COLOR_SELECT_ICON_DOWN, AK_NULL);
	AKBmpGetInfo(pRes->pClrIcn[1], &pRes->IcnRct_D[0].width, &pRes->IcnRct_D[0].height, AK_NULL);
    pRes->IcnRct_D[1].width = pRes->IcnRct_D[0].width;
    pRes->IcnRct_D[1].height = pRes->IcnRct_D[0].height;
    pRes->IcnRct_D[2].width = pRes->IcnRct_D[0].width;
    pRes->IcnRct_D[2].height =pRes->IcnRct_D[0].height;

    //left icon
    pRes->pClrIcn[2] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_COLOR_SELECT_ICON_LEFT, AK_NULL);
	AKBmpGetInfo(pRes->pClrIcn[2], &pRes->IcnRct_L.width, &pRes->IcnRct_L.height, AK_NULL);

    //right icon
    pRes->pClrIcn[3] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_COLOR_SELECT_ICON_RIGHT, AK_NULL);
	AKBmpGetInfo(pRes->pClrIcn[3], &pRes->IcnRct_R.width, &pRes->IcnRct_R.height, AK_NULL);

    //up/down icon back image
	pRes->pClrIcnBckUD = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_COLOR_SELECT_ICON_BACK_UD, AK_NULL);
	AKBmpGetInfo(pRes->pClrIcnBckUD, &pRes->IcnBkRct_U[0].width, &pRes->IcnBkRct_U[0].height, AK_NULL);
    pRes->IcnBkRct_U[1].width = pRes->IcnBkRct_U[0].width;
    pRes->IcnBkRct_U[1].height = pRes->IcnBkRct_U[0].height;
    pRes->IcnBkRct_U[2].width = pRes->IcnBkRct_U[0].width;
    pRes->IcnBkRct_U[2].height = pRes->IcnBkRct_U[0].height;
    pRes->IcnBkRct_D[0].width = pRes->IcnBkRct_U[0].width;
    pRes->IcnBkRct_D[0].height = pRes->IcnBkRct_U[0].height;
    pRes->IcnBkRct_D[1].width = pRes->IcnBkRct_U[0].width;
    pRes->IcnBkRct_D[1].height = pRes->IcnBkRct_U[0].height;
    pRes->IcnBkRct_D[2].width = pRes->IcnBkRct_U[0].width;
    pRes->IcnBkRct_D[2].height = pRes->IcnBkRct_U[0].height;
   
    //left/right icon back image
    pRes->pClrIcnBckLR = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_COLOR_SELECT_ICON_BACK_LR, AK_NULL);
	AKBmpGetInfo(pRes->pClrIcnBckLR, &pRes->IcnBkRct_L.width, &pRes->IcnBkRct_L.height, AK_NULL);
    pRes->IcnBkRct_R.width = pRes->IcnBkRct_L.width;
    pRes->IcnBkRct_R.height = pRes->IcnBkRct_L.height;

    //button 
    for (i = 0; i < 2; i++)
    {
        pRes->pBtnOk[i]   = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_COLOR_SELECT_OK + i, AK_NULL);
        AKBmpGetInfo(pRes->pBtnOk[i], &pRes->ButnOkRct.width, &pRes->ButnOkRct.height, AK_NULL);
        pRes->pBtnCncl[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_PNG_COLOR_SELECT_CANCEL + i, AK_NULL);
		AKBmpGetInfo(pRes->pBtnCncl[i], &pRes->ButnCnclRct.width, &pRes->ButnCnclRct.height, AK_NULL);
    }
}

static T_VOID ClrSelect_InitResRect(T_CLRSELECT *pClrSelect)
{
    T_CLRSLCT_RES   *pRes = AK_NULL;
    T_U32           i = 0;
    T_LEN           tmpW = 0;
    T_POS           tmpY = 0;

    if (AK_NULL == pClrSelect)
    {
        return;
    }

    pRes = &pClrSelect->res;

    //background image rect
    AKBmpGetInfo(pRes->pBkImg, &pRes->BkImgRct.width, &pRes->BkImgRct.height, AK_NULL);
    pRes->BkImgRct.left = (Fwl_GetLcdWidth() - pRes->BkImgRct.width) / 2;
    pRes->BkImgRct.top = (Fwl_GetLcdHeight() - pRes->BkImgRct.height) / 2;

    //color image
    for (i=0; i<3; i++)
    {
        AKBmpGetInfo(pRes->pClrImg[i], &pRes->ClrImgRct[i].width, &pRes->ClrImgRct[i].height, AK_NULL);
    }
    
    //text rect
    for (i=0; i<4; i++)
    {
        AKBmpGetInfo(pRes->pClrFram[i], &pRes->ClrFramRct[i].width, &pRes->ClrFramRct[i].height, AK_NULL);
    }

    tmpW = pRes->BkImgRct.width / 4;
    tmpY = pRes->BkImgRct.top + TOP_BAR_HEIGHT + 6;

    //red/green/blue color icon 
    pRes->ClrImgRct[0].left = pRes->BkImgRct.left + tmpW - pRes->ClrImgRct[0].width / 2;
    pRes->ClrImgRct[1].left = pRes->ClrImgRct[0].left + tmpW;
    pRes->ClrImgRct[2].left = pRes->ClrImgRct[1].left + tmpW;
    pRes->ClrImgRct[0].top = tmpY;
    pRes->ClrImgRct[1].top = pRes->ClrImgRct[0].top;
    pRes->ClrImgRct[2].top = pRes->ClrImgRct[0].top;

    pRes->IcnBkRct_U[0].left = pRes->BkImgRct.left + tmpW - pRes->IcnBkRct_U[0].width / 2;
    pRes->IcnBkRct_U[1].left = pRes->IcnBkRct_U[0].left + tmpW;
    pRes->IcnBkRct_U[2].left = pRes->IcnBkRct_U[1].left + tmpW;
    pRes->IcnBkRct_U[0].top = pRes->ClrImgRct[0].top + pRes->ClrImgRct[0].height + 6;
    pRes->IcnBkRct_U[1].top = pRes->IcnBkRct_U[0].top;
    pRes->IcnBkRct_U[2].top = pRes->IcnBkRct_U[0].top;

    pRes->IcnRct_U[0].left = pRes->BkImgRct.left + tmpW - pRes->IcnRct_U[0].width / 2;
    pRes->IcnRct_U[1].left = pRes->IcnRct_U[0].left + tmpW;
    pRes->IcnRct_U[2].left = pRes->IcnRct_U[1].left + tmpW;
    pRes->IcnRct_U[0].top = pRes->IcnBkRct_U[0].top + (pRes->IcnBkRct_U[0].height - pRes->IcnRct_U[0].height) / 2;
    pRes->IcnRct_U[1].top = pRes->IcnRct_U[0].top;
    pRes->IcnRct_U[2].top = pRes->IcnRct_U[0].top;

    pRes->ClrFramRct[0].left = pRes->BkImgRct.left + tmpW - pRes->ClrFramRct[0].width / 2;
    pRes->ClrFramRct[1].left = pRes->ClrFramRct[0].left + tmpW;
    pRes->ClrFramRct[2].left = pRes->ClrFramRct[1].left + tmpW;
    pRes->ClrFramRct[3].left = pRes->ClrFramRct[0].left + tmpW;
    pRes->ClrFramRct[0].top = pRes->IcnBkRct_U[0].top + pRes->IcnBkRct_U[0].height + 2;
    pRes->ClrFramRct[1].top = pRes->ClrFramRct[0].top;
    pRes->ClrFramRct[2].top = pRes->ClrFramRct[0].top;
    pRes->ClrFramRct[3].top = pRes->ClrFramRct[0].top;

    pRes->IcnBkRct_D[0].left = pRes->BkImgRct.left + tmpW - pRes->IcnBkRct_D[0].width / 2;
    pRes->IcnBkRct_D[1].left = pRes->IcnBkRct_D[0].left + tmpW;
    pRes->IcnBkRct_D[2].left = pRes->IcnBkRct_D[1].left + tmpW;
    pRes->IcnBkRct_D[0].top = pRes->ClrFramRct[0].top + pRes->ClrFramRct[0].height + 2;
    pRes->IcnBkRct_D[1].top = pRes->IcnBkRct_D[0].top;
    pRes->IcnBkRct_D[2].top = pRes->IcnBkRct_D[0].top;

    pRes->IcnRct_D[0].left = pRes->BkImgRct.left + tmpW - pRes->IcnRct_D[0].width / 2;
    pRes->IcnRct_D[1].left = pRes->IcnRct_D[0].left + tmpW;
    pRes->IcnRct_D[2].left = pRes->IcnRct_D[1].left + tmpW;
    pRes->IcnRct_D[0].top = pRes->IcnBkRct_D[0].top + (pRes->IcnBkRct_D[0].height - pRes->IcnRct_D[0].height) / 2;
    pRes->IcnRct_D[1].top = pRes->IcnRct_D[0].top;
    pRes->IcnRct_D[2].top = pRes->IcnRct_D[0].top;

    //icon:left/right
    pRes->IcnBkRct_L.left =  pRes->ClrFramRct[0].left - 2 - pRes->IcnBkRct_L.width;
    pRes->IcnBkRct_L.top = pRes->ClrFramRct[0].top + (pRes->ClrFramRct[0].height - pRes->IcnBkRct_L.height) / 2;
    pRes->IcnBkRct_R.left =  pRes->ClrFramRct[2].left + pRes->ClrFramRct[2].width + 2;
    pRes->IcnBkRct_R.top = pRes->IcnBkRct_L.top;

    pRes->IcnRct_L.left =  pRes->IcnBkRct_L.left + (pRes->IcnBkRct_L.width - pRes->IcnBkRct_L.width) / 2;
    pRes->IcnRct_L.top =  pRes->IcnBkRct_L.top + (pRes->IcnBkRct_L.height - pRes->IcnBkRct_L.height) / 2;
    pRes->IcnRct_R.left =  pRes->IcnBkRct_R.left + (pRes->IcnBkRct_R.width - pRes->IcnRct_R.width) / 2;
    pRes->IcnRct_R.top =  pRes->IcnRct_L.top;

    //current color rect
    pRes->curClrRct.width = 60;
    pRes->curClrRct.height= 12;
    pRes->curClrRct.left = pRes->BkImgRct.left + (pRes->BkImgRct.width - pRes->curClrRct.width) / 2;
    pRes->curClrRct.top = pRes->IcnBkRct_D[0].top + pRes->IcnBkRct_D[0].height + 6;

    //button
    pRes->ButnOkRct.left = pRes->BkImgRct.left + tmpW - pRes->ButnOkRct.width / 2;
    pRes->ButnCnclRct.left = pRes->BkImgRct.left + pRes->BkImgRct.width \
                            - tmpW - pRes->ButnOkRct.width / 2;
    pRes->ButnOkRct.top = pRes->curClrRct.top + pRes->curClrRct.height + 6;
    pRes->ButnCnclRct.top = pRes->ButnOkRct.top;

    //edit rect
    pRes->EditRct.left = pRes->BkImgRct.left;
    pRes->EditRct.top = pRes->IcnBkRct_U[0].top;
    pRes->EditRct.width = pRes->BkImgRct.width;
    pRes->EditRct.height = pRes->ButnOkRct.top - pRes->EditRct.top - 3;
}


#endif
/* end of file */
