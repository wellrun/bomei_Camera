/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  *
  * @File name: Ctl_ContextMenu.c
  * @Function: This file is the implement of the control Ctl_contextMenu.

  * @Author: WangWei
  * @Date: 2008-08-26
  * @Version: 1.0
  */

#include "Ctl_ContextMenu.h"
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
#include "fwl_pfdisplay.h"
#include "Fwl_display.h"
#include "Fwl_tscrcom.h"


#define CMENU_ROW_H             20
#define CMENU_ROW_W             120
#define CMENU_QTY_MAX           30
#define CMENU_QTY_PER_PAGE      5

//static function declare
static T_RECT ContextMenu_GetFocusOptionRect(T_OPTION_NODE *pListRoot);
static T_U32 ContextMenu_GetHitOptionId(T_OPTION_NODE *pListRoot, T_POS y);
static T_VOID ContextMenu_MovieItemFocusToPrevious(T_CONTEXTMENU *pCMenu);
static T_VOID ContextMenu_MovieItemFocusToNext(T_CONTEXTMENU *pCMenu);
static T_VOID ContextMenu_HitButton_Handle(T_CONTEXTMENU *pCMenu, T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y);
static T_eBACK_STATE ContextMenu_UserKey_Handle(T_CONTEXTMENU *pCMenu, T_MMI_KEYPAD *pPhyKey);
static T_VOID ContextMenu_OptionNodeInit(T_OPTION_NODE *pOptionNode);
static T_RECT ContextMenu_SetDispRect(T_POS x, T_POS y,T_LEN w,T_LEN h);
static T_VOID ContextMenu_ShowOptionList(T_OPTION_NODE *pListRoot, T_COLOR frmCl, T_COLOR bkCl, T_COLOR FcsCl, T_COLOR TxtCl);
static T_OPTION_NODE *ContextMenu_GetCurOption(T_CONTEXTMENU *pCMenu);
static T_OPTION_NODE *ContextMenu_FreeOptionTree(T_OPTION_NODE *pListHead);

/**
 * @brief   init T_CONTEXTMENU struct
 * @author  WangWei
 * @date    2008-08-26
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct
 * @param   T_POS posX: contextMenu rect x coordinate
 * @param   T_POS posY: contextMenu rect y coordinate
 * @param   T_COLOR frmCl: frame color
 * @param   T_COLOR bkCl: background color
 * @param   T_COLOR FcsCl: focus color
 * @param   T_COLOR TxtCl: test color
 * @return  T_VOID
 * @retval  
 */
T_VOID ContextMenu_Init(T_CONTEXTMENU *pCMenu, T_POS posX, T_POS posY, T_COLOR frmCl, T_COLOR bkCl, T_COLOR FcsCl, T_COLOR TxtCl)
{
    if (AK_NULL == pCMenu)
    {
        return;
    }

    ContextMenu_OptionNodeInit(&pCMenu->ListRoot);
    pCMenu->ListRoot.rect = ContextMenu_SetDispRect(posX, posY, CMENU_ROW_W, CMENU_ROW_H);

    pCMenu->CurLevel = 0;

    pCMenu->frameColor = frmCl;
    pCMenu->backColor = bkCl;
    pCMenu->textColor = TxtCl;
    pCMenu->focusColor = FcsCl;

    ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);
}

/**
 * @brief   add option to the List
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_OPTION_NODE *pListRoot: the father node of the option list
 * @param   const T_U16 *pOptionName: option neme
 * @return  T_BOOL
 * @retval  AK_TRUE:success, AK_FALSE:fail
 */
T_BOOL ContextMenu_AddOption(T_OPTION_NODE *pListRoot, const T_U16 *pOptionName)
{
    T_OPTION_NODE *pOptnNode = AK_NULL;
    T_OPTION_NODE *p = AK_NULL;

    if ((AK_NULL == pListRoot)||(AK_NULL == pOptionName))
    {
        return AK_FALSE;
    }

    if (pListRoot->MaxChQty == pListRoot->CurChQty)
    {
        return AK_FALSE;
    }

    pOptnNode = (T_OPTION_NODE *)Fwl_Malloc(sizeof(T_OPTION_NODE));
    if (AK_NULL == pOptnNode)
    {
        return AK_FALSE;
    }

    ContextMenu_OptionNodeInit(pOptnNode);
    Utl_UStrCpy(pOptnNode->neme, pOptionName);

    if (AK_NULL == pListRoot->pChild)
    {
        pListRoot->pChild = pOptnNode;
        pOptnNode->pFather = pListRoot;

        pOptnNode->rect.left = pListRoot->rect.left + pListRoot->rect.width;
        pOptnNode->rect.top = pListRoot->rect.top + 2;
        pOptnNode->rect.width = CMENU_ROW_W;
        pOptnNode->rect.height = CMENU_ROW_H;

        if ((pListRoot->rect.left + pListRoot->rect.width + pOptnNode->rect.width) > Fwl_GetLcdWidth())
        {
            pOptnNode->rect.left = pListRoot->rect.left - pOptnNode->rect.width;
        }
        
        pListRoot->pFrstCh = pListRoot->pChild;
        pListRoot->pFcsCh = pListRoot->pChild;
        pListRoot->CurChQty = 1;
    }
    else
    {
        p = pListRoot->pChild;
        while (AK_NULL != p->pNext)
        {
            p = p->pNext;
        }

        p->pNext = pOptnNode;
        pOptnNode->pPrevious = p;
        pOptnNode->id = p->id + 1;

        pOptnNode->rect.left = (T_POS)(pListRoot->rect.left + pListRoot->rect.width);
        pOptnNode->rect.top  = (T_POS)(pListRoot->rect.top + 2 + pOptnNode->id * CMENU_ROW_H);
        pOptnNode->rect.width = CMENU_ROW_W;
        pOptnNode->rect.height = CMENU_ROW_H;

        if ((pListRoot->rect.left + pListRoot->rect.width + pOptnNode->rect.width) > Fwl_GetLcdWidth())
        {
            pOptnNode->rect.left = pListRoot->rect.left - pOptnNode->rect.width;
        }


        pOptnNode->pFather = pListRoot;
        p->level = pListRoot->level + 1;
        pListRoot->CurChQty++;
    }
    
    return AK_TRUE;
}

/**
 * @brief   Get option pointer by the id:(0~n)
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_OPTION_NODE *pListRoot: T_OPTION_NODE struct pointer
 * @param   T_U32 id: the option id
 * @return  T_OPTION_NODE *
 * @retval  T_OPTION_NODE struct pointer
 */
T_OPTION_NODE *ContextMenu_GetOptionById(T_OPTION_NODE *pListRoot, T_U32 id)
{
    T_OPTION_NODE *p = AK_NULL;

    if (AK_NULL == pListRoot)
    {
        return AK_NULL;
    }

    if (pListRoot->MaxChQty <= id)
    {
        return AK_NULL;
    }

    p = pListRoot->pChild;
    while (AK_NULL != p)
    {
        if (p->id == id)
        {
            break;
        }
        
        p = p->pNext;
    }

    return p;
}

/**
 * @brief   set contextMenu rect
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_OPTION_NODE *pListRoot: T_OPTION_NODE struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE:success, AK_FALSE:fail
 */
T_BOOL ContextMenu_CheckLocaRect(T_OPTION_NODE *pListRoot)
{
    if (AK_NULL == pListRoot)
    {
        return AK_FALSE;
    }
    
    pListRoot->rect.width = CMENU_ROW_W;

    if (pListRoot->CurChQty > pListRoot->QtyPerPg)
    {
        pListRoot->rect.height = (T_POS)(CMENU_ROW_H * pListRoot->QtyPerPg + 4);
    }
    else
    {
        pListRoot->rect.height = (T_POS)(CMENU_ROW_H * pListRoot->CurChQty + 4);
    }
    
    if (pListRoot->rect.left + CMENU_ROW_W > Fwl_GetLcdWidth())
    {
        pListRoot->rect.left = Fwl_GetLcdWidth() - CMENU_ROW_W;
    }

    if (pListRoot->rect.top + pListRoot->rect.height > Fwl_GetLcdHeight())
    {
        pListRoot->rect.top = Fwl_GetLcdHeight() - pListRoot->rect.height;
    }
    
    return AK_TRUE;
}

/**
 * @brief   set scroll bar of contextMenu
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct
 * @return  T_BOOL
 * @retval  AK_TRUE:success, AK_FALSE:fail
 */
T_BOOL ContextMenu_SetScrBar(T_CONTEXTMENU *pCMenu)
{
 //   T_POS   left = 0;
//    T_LEN   width = 0;
    
    if (AK_NULL == pCMenu)
    {
        return AK_FALSE;
    }
/*
    left = pCMenu->rect.left + pCMenu->rect.width - g_Graph.BScBarWidth;
    if (pCMenu->CurQty > pCMenu->QtyPerPage)
    {
        width = g_Graph.BScBarWidth;
    }
    else
    {
        width = 0;
    }
        
    ScBar_Init(&pCMenu->scrBar, left, pCMenu->rect.top, \
            width, pCMenu->rect.height, -10, SCBAR_VERTICAL);
*/    
    return AK_TRUE;
}

/**
 * @brief   show contextMenu
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct
 * @return  T_VOID
 * @retval  
 */
T_VOID ContextMenu_Show(T_CONTEXTMENU *pCMenu)
{
    T_OPTION_NODE   *pListRoot = AK_NULL;
    T_U32           level = 0;
    
    if (AK_NULL == pCMenu)
    {
        return;
    }

    if (CMENU_REFRESH_NONE == pCMenu->refreshFlag)
    {
        return;
    }

    pListRoot = &pCMenu->ListRoot;
    while ((pCMenu->CurLevel != pListRoot->level) && (AK_NULL != pListRoot->pFcsCh))
    {
        pListRoot = pListRoot->pFcsCh;
    }

    if (pCMenu->CurLevel == pListRoot->level)
    {
        pListRoot = &pCMenu->ListRoot;
        level = pCMenu->ListRoot.level;
        while (level <= pCMenu->CurLevel)
        {
            ContextMenu_ShowOptionList(pListRoot, pCMenu->frameColor, pCMenu->backColor, \
                                        pCMenu->focusColor, pCMenu->textColor);

            pListRoot = pListRoot->pFcsCh;
            if (AK_NULL == pListRoot)
            {
                break;
            }
            level = pListRoot->level;
        }
    }
    
    pCMenu->refreshFlag = CMENU_REFRESH_NONE;
}

/**
 * @brief   free contextMenu 
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct
 * @return  T_VOID
 * @retval  
 */
T_VOID ContextMenu_Free(T_CONTEXTMENU *pCMenu)
{
    if (AK_NULL == pCMenu)
    {
        return;
    }

    if (AK_NULL != pCMenu->ListRoot.pChild)
    {
        pCMenu->ListRoot.pChild = ContextMenu_FreeOptionTree(pCMenu->ListRoot.pChild);
    }

    ContextMenu_OptionNodeInit(&pCMenu->ListRoot);
    pCMenu->CurLevel = 0;
}

/**
 * @brief   set list focus by the option id
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_OPTION_NODE *pListRoot: T_OPTION_NODE struct pionter
 * @param   T_U32 id: option id
 * @return  T_VOID
 * @retval  
 */
T_VOID ContextMenu_SetFocusById(T_OPTION_NODE *pListRoot, T_U32 id)
{
    T_OPTION_NODE   *p = AK_NULL;
    
    if (AK_NULL == pListRoot)
    {
        return;
    }

    p = pListRoot->pChild;
    while (AK_NULL != p)
    {
        if (p->id == id)
        {
            break;
        }
        p = p->pNext;
    }

    if (id == p->id)
    {
        pListRoot->pFcsCh = p;
        if (pListRoot->pFcsCh->id < pListRoot->pFrstCh->id)
        {
            pListRoot->pFrstCh = pListRoot->pFcsCh;
        }
        else 
        {
            while ((pListRoot->pFcsCh->id - pListRoot->pFrstCh->id) > (pListRoot->QtyPerPg-1))
            {
                pListRoot->pFrstCh = pListRoot->pFrstCh->pNext;
            }
        }
    }
}

/**
 * @brief   set list refresh flag
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct pionter
 * @param   T_S8 refresh: refresh flag
 * @return  T_VOID
 * @retval  
 */
T_VOID ContextMenu_SetRefresh(T_CONTEXTMENU *pCMenu, T_U8 refresh)
{
    if (AK_NULL == pCMenu)
    {
        return;
    }

    pCMenu->refreshFlag = refresh;
}

/**
 * @brief   contextMenu handler
 * @author  WangWei
 * @date    2008-08-26 
 * @param   T_CONTEXTMENU *pCMenu: T_CONTEXTMENU struct pointer
 * @param   T_EVT_CODE Event: event
 * @param   T_EVT_PARAM *pParam: event parameter 
 * @return  T_eBACK_STATE: 
 * @retval  
 */
T_eBACK_STATE ContextMenu_Handler(T_CONTEXTMENU *pCMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_MMI_KEYPAD    phyKey;
    T_eBACK_STATE   ret = eStay;

    if ((AK_NULL == pCMenu) || (AK_NULL == pParam))
    {
        return eStay;    
    }

    switch (Event)
    {
        case M_EVT_USER_KEY:
            phyKey.keyID = pParam->c.Param1;
            phyKey.pressType = pParam->c.Param2;

            ret = ContextMenu_UserKey_Handle(pCMenu, &phyKey);
            break;
            
        case M_EVT_TOUCH_SCREEN:
            phyKey.keyID = kbNULL;
            phyKey.pressType = PRESS_SHORT;

            switch (pParam->s.Param1) 
            {
                case eTOUCHSCR_UP:
                    ContextMenu_HitButton_Handle(pCMenu, &phyKey, \
                        (T_POS)pParam->s.Param2, (T_POS)pParam->s.Param3);

                    ret = ContextMenu_UserKey_Handle(pCMenu, &phyKey);
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

/**********************************
        static function
***********************************/
static T_VOID ContextMenu_HitButton_Handle(T_CONTEXTMENU *pCMenu, T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y)
{
    T_OPTION_NODE   *pListRoot = AK_NULL;
    T_OPTION_NODE   *p = AK_NULL;
    T_RECT          rect;
//    T_U32           i = 0;
    T_U32           id = CMENU_QTY_MAX;
 //   T_U32           level = 0;

    if ((AK_NULL == pCMenu)||(AK_NULL == pPhyKey))
    {
        return;
    }

    pListRoot = ContextMenu_GetCurOption(pCMenu);
    if (AK_NULL == pListRoot)
    {
        pPhyKey->keyID = kbCLEAR;
        pPhyKey->pressType = PRESS_SHORT;
        return;
    }

    //if the point is not in the context menu, exit 
    if (!PointInRect(&pListRoot->rect, x, y))
    {
        p = pListRoot->pFather;

        while (AK_NULL != p)
        {
            if (!PointInRect(&p->rect, x, y))
            {
               p = p->pFather;
            }
            else
            {
                pCMenu->CurLevel = p->level;
                id = ContextMenu_GetHitOptionId(p, y);
                ContextMenu_SetFocusById(p, id);
                ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);

                return;
            }
        }
        
        pPhyKey->keyID = kbCLEAR;
        pPhyKey->pressType = PRESS_SHORT;
        return;
    }

    /*
    //scroll bar
    rect = ScBar_GetRect(&pCMenu->scrBar);
    if (PointInRect(&rect, x, y))
    {
        ScBar_GetLocaRect2(&rect1, &pCMenu->scrBar);
        if (!PointInRect(&rect1, x, y))
        {
            i = (y - rect.top)/(rect.height/pCMenu->CurQty);
            if (i > pCMenu->pFocus->id)
            {
                ContextMenu_SetFocusById(pCMenu, pCMenu->pFocus->id + 1);
            }
            else
            {
                ContextMenu_SetFocusById(pCMenu, pCMenu->pFocus->id - 1);
            }
            
            ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);
            return;
        }
        
        ScBar_GetUpIconRect(&rect1, &pCMenu->scrBar);
        if (PointInRect(&rect1, x, y))
        {
            ContextMenu_SetFocusById(pCMenu, pCMenu->pFocus->id - 1);
            ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);
            return;
        }

        ScBar_GetDownIconRect(&rect1, &pCMenu->scrBar);
        if (PointInRect(&rect1, x, y))
        {
            ContextMenu_SetFocusById(pCMenu, pCMenu->pFocus->id + 1);
            ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);
            return;
        }
    }
    */
    
    //hit the focus option
    rect = ContextMenu_GetFocusOptionRect(pListRoot);
    if (PointInRect(&rect, x, y))
    {
        pPhyKey->keyID = kbOK;
        pPhyKey->pressType = PRESS_SHORT;
        return;
    }

    //others
    id = ContextMenu_GetHitOptionId(pListRoot, y);
    if (id >= CMENU_QTY_MAX)
    {
        pPhyKey->keyID = kbCLEAR;
        pPhyKey->pressType = PRESS_SHORT;
        return;
    }
    else
    {
        ContextMenu_SetFocusById(pListRoot, id);
        ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);
    }
}

static T_eBACK_STATE ContextMenu_UserKey_Handle(T_CONTEXTMENU *pCMenu, T_MMI_KEYPAD *pPhyKey)
{
    T_eBACK_STATE   ret = eStay;
    T_OPTION_NODE   *pListRoot = AK_NULL;

    if ((AK_NULL == pCMenu) || (AK_NULL == pPhyKey))
    {
        return ret;
    }

    switch (pPhyKey->keyID)
    {
        case kbOK:
            pListRoot = ContextMenu_GetCurOption(pCMenu);
            if (AK_NULL == pListRoot)
            {
                //结构体中的数据已经出错
                ret = eReturn;
            }
            else if (AK_NULL == pListRoot->pFcsCh->pChild) //没有子列表
            {
                ret = eNext;
            }
            else
            {
                pCMenu->CurLevel += 1;
                ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);
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
            //退到上一层列表
            pListRoot = ContextMenu_GetCurOption(pCMenu);
            if (AK_NULL == pListRoot)
            {
                //结构体中的数据已经出错
                ret = eReturn;
            }
            else if (AK_NULL == pListRoot->pFather) //已经是最顶层
            {
                ret = eReturn;
            }
            else
            {
                pCMenu->CurLevel -= 1;
                ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);
            }
        	break;                
            
        case kbUP:
            ContextMenu_MovieItemFocusToPrevious(pCMenu);
        	ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);
            break;                

        case kbDOWN:
            ContextMenu_MovieItemFocusToNext(pCMenu);
        	ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);
            break;
            
        case kbRIGHT:
            //进入下一层列表
            pListRoot = ContextMenu_GetCurOption(pCMenu);
            if (AK_NULL == pListRoot)
            {
                //结构体中的数据已经出错
                ret = eReturn;
            }
            else if (AK_NULL != pListRoot->pFcsCh->pChild) //有子列表
            {
                pCMenu->CurLevel += 1;
                ContextMenu_SetRefresh(pCMenu, CMENU_REFRESH_ALL);
            }
            break;                

        default:
            break;
    } 
    
    return ret;
}

static T_RECT ContextMenu_GetFocusOptionRect(T_OPTION_NODE *pListRoot)
{
    T_RECT rect;
    
    rect.left = pListRoot->rect.left;
    rect.top = (T_POS)(pListRoot->rect.top + 2 + (pListRoot->pFcsCh->id - pListRoot->pFrstCh->id) * CMENU_ROW_H);
    rect.width = pListRoot->rect.width;
    rect.height = CMENU_ROW_H;

    return rect;
}

static T_U32 ContextMenu_GetHitOptionId(T_OPTION_NODE *pListRoot, T_POS y)
{
    T_U32 id;
    
    if (AK_NULL == pListRoot)
    {
        return CMENU_QTY_MAX;
    }

    if (y < pListRoot->rect.top || y > (pListRoot->rect.top + pListRoot->rect.height))
    {
        return CMENU_QTY_MAX;
    }

    id = (y - pListRoot->rect.top - 2)/CMENU_ROW_H;

    return id;
}

static T_VOID ContextMenu_MovieItemFocusToPrevious(T_CONTEXTMENU *pCMenu)
{
    T_OPTION_NODE   *pListRoot = AK_NULL;
    T_OPTION_NODE   *p = AK_NULL;
    T_U32           level = 0;
    T_U32           i = 0;

    if (AK_NULL == pCMenu)
    {
        return;
    }

    pListRoot = &pCMenu->ListRoot;
    level = pListRoot->level;
    while ((level != pCMenu->CurLevel) && (AK_NULL != pListRoot->pFcsCh))
    {
        pListRoot = pListRoot->pFcsCh;
        level = pListRoot->level;
    }

    if (level != pCMenu->CurLevel)
    {
        return;
    }
    else
    {
        if (AK_NULL == pListRoot->pFcsCh->pPrevious)
        {
            p = pListRoot->pFcsCh;
            while (AK_NULL != p->pNext)
            {
                p = p->pNext;
            }

            pListRoot->pFcsCh = p;

            for (i = 0; (i < pListRoot->QtyPerPg-1) && (AK_NULL != p->pPrevious); i++)
            {
                p = p->pPrevious;
            }
            pListRoot->pFrstCh= p;
        }
        else
        {
            pListRoot->pFcsCh = pListRoot->pFcsCh->pPrevious;
            if (pListRoot->pFcsCh->id < pListRoot->pFrstCh->id)
            {
                pListRoot->pFrstCh = pListRoot->pFcsCh;
            }
        }
    }
}

static T_VOID ContextMenu_MovieItemFocusToNext(T_CONTEXTMENU *pCMenu)
{
    T_OPTION_NODE   *pListRoot = AK_NULL;
    T_U32           level = 0;
    
    if (AK_NULL == pCMenu)
    {
        return;
    }

    pListRoot = &pCMenu->ListRoot;
    level = pListRoot->level;
    while ((level != pCMenu->CurLevel) && (AK_NULL != pListRoot->pFcsCh))
    {
        pListRoot = pListRoot->pFcsCh;
        level = pListRoot->level;
    }

    if (level != pCMenu->CurLevel)
    {
        return;
    }
    else
    {
        if (AK_NULL == pListRoot->pFcsCh->pNext)
        {
            pListRoot->pFcsCh = pListRoot->pChild;
            pListRoot->pFrstCh= pListRoot->pFcsCh;
        }
        else
        {
            pListRoot->pFcsCh = pListRoot->pFcsCh->pNext;
            if ((pListRoot->pFcsCh->id - pListRoot->pFrstCh->id) > (pListRoot->QtyPerPg-1))
            {
                pListRoot->pFrstCh = pListRoot->pFrstCh->pNext;
            }
        }
    }
}

static T_VOID ContextMenu_OptionNodeInit(T_OPTION_NODE *pOptionNode)
{
    if (AK_NULL == pOptionNode)
    {
        return;
    }

    pOptionNode->id = 0;
    Utl_UStrCpy(pOptionNode->neme, _T("\0"));
    pOptionNode->pPrevious = AK_NULL;
    pOptionNode->pNext = AK_NULL;

    pOptionNode->level = 0;
    
    pOptionNode->rect.left = 0;
    pOptionNode->rect.top = 0;
    pOptionNode->rect.width = 0;
    pOptionNode->rect.height= 0;
    
    pOptionNode->pFather = AK_NULL;
    pOptionNode->pChild = AK_NULL;
    pOptionNode->pFcsCh = AK_NULL;
    pOptionNode->pFrstCh = AK_NULL;
    
    pOptionNode->MaxChQty = CMENU_QTY_MAX;
    pOptionNode->CurChQty = 0;
    pOptionNode->QtyPerPg = CMENU_QTY_PER_PAGE;
}

static T_RECT ContextMenu_SetDispRect(T_POS x, T_POS y, T_LEN w, T_LEN h)
{
    T_RECT rect;

    rect.left = x;
    rect.top = y;
    rect.width = w;
    rect.height = h;

    if ((x + w) > Fwl_GetLcdWidth())
    {
        rect.left = Fwl_GetLcdWidth() - w;
    }

    if ((y + h) > Fwl_GetLcdHeight())
    {
        rect.top = Fwl_GetLcdHeight() - h;
    }

    return rect;
 }

static T_VOID ContextMenu_ShowOptionList(T_OPTION_NODE *pListRoot, 
                            T_COLOR frmCl, T_COLOR bkCl, T_COLOR FcsCl, T_COLOR TxtCl)
{
    T_OPTION_NODE *p = AK_NULL;
    T_pRECT  pRect = AK_NULL;
    T_POS   posX = 0;
    T_POS   posY = 0;
    T_U16   UStrLen = 0;
    T_U32   i = 0;

    if (AK_NULL == pListRoot)
    {
        return;
    }

    if (AK_NULL == pListRoot->pChild)
    {
        return;
    }

    pRect = &pListRoot->rect;
    
    //show frame
    Fwl_DrawRect(HRGB_LAYER, pRect->left, pRect->top, pRect->width, pRect->height, frmCl);

    //show background color
    Fwl_FillSolidRect(HRGB_LAYER, (T_POS)(pRect->left+1), (T_POS)(pRect->top+1), 
    							  (T_LEN)(pRect->width-2),(T_LEN)(pRect->height-2), bkCl);

    //show option
    p = pListRoot->pFrstCh;
    for (i = 0; (i < pListRoot->QtyPerPg) && (AK_NULL != p); i++)
    {
        posX = pRect->left + 4;
        posY = (T_POS)(pRect->top + 2 + i * CMENU_ROW_H);
        UStrLen = (T_U16)Utl_UStrLen(p->neme);

        if (p == pListRoot->pFcsCh)
        {
            Fwl_FillSolidRect(HRGB_LAYER, posX, posY, (T_LEN)(pRect->width - 8), CMENU_ROW_H, FcsCl);

            Fwl_UDispSpeciString(HRGB_LAYER, posX, posY, p->neme, ~TxtCl, CURRENT_FONT_SIZE, UStrLen);

            if (AK_NULL != p->pChild)
            {   
                Fwl_UDispSpeciString(HRGB_LAYER, (T_POS)(pRect->left + pRect->width -24), posY, Eng_StrMbcs2Ucs_3(">>"), ~TxtCl, CURRENT_FONT_SIZE, 2);
            }
        }
        else
        {
            Fwl_UDispSpeciString(HRGB_LAYER, posX, posY, p->neme, TxtCl, CURRENT_FONT_SIZE, UStrLen);

            if (AK_NULL != p->pChild)
            {
                Fwl_UDispSpeciString(HRGB_LAYER, (T_POS)(pRect->left + pRect->width -24), posY, Eng_StrMbcs2Ucs_3(">>"), TxtCl, CURRENT_FONT_SIZE, 2);
            }

        }
        p = p->pNext;
    }
}

static T_OPTION_NODE *ContextMenu_GetCurOption(T_CONTEXTMENU *pCMenu)
{
    T_OPTION_NODE   *p = AK_NULL;
     
    if (AK_NULL == pCMenu)
    {
        return AK_NULL;
    }

    p = &pCMenu->ListRoot;
    while ((p->level != pCMenu->CurLevel) && (AK_NULL != p->pFcsCh))
    {
        p = p->pFcsCh;
    }

    if (p->level != pCMenu->CurLevel)
    {
        return AK_NULL;
    }

    return p;
}

static T_OPTION_NODE *ContextMenu_FreeOptionTree(T_OPTION_NODE *pListHead)
{
    if (AK_NULL == pListHead)
    {
        return AK_NULL;
    }

    while (AK_NULL != pListHead->pChild)
    {
        pListHead->pChild = ContextMenu_FreeOptionTree(pListHead->pChild);
    }

    while (AK_NULL != pListHead->pNext)
    {
        pListHead->pNext = ContextMenu_FreeOptionTree(pListHead->pNext);
    }
    
    if ((AK_NULL == pListHead->pNext) && (AK_NULL == pListHead->pChild))
    {
        pListHead = Fwl_Free(pListHead);
    }

    return pListHead;
}


