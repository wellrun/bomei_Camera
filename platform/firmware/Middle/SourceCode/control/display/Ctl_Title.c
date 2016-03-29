/**
 * @file Ctl_Title.c
 * @brief ANYKA software
 * Title Control
 * @author ZouMai
 * @date  2002-05-25
 * @version 1,0 
 */

#include "Ctl_Title.h"
#include "Eng_AkBmp.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "eng_string_uc.h"
#include "Lib_res_port.h"
#include "eng_graph.h"
#include "fwl_display.h"


/**
 * @brief Initialize T_TITLE structure
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-05-25 
 * @param T_TITLE *title : title sturcture
 * @param  T_POS left : title location
 * @param  T_POS top
 * @param  T_LEN width
 * @param  T_LEN height
 * @return T_VOID
 * @retval 
 */

T_VOID Title_Init(T_TITLE *pTitle, const T_pRECT pRect)
{
    AK_ASSERT_PTR_VOID(pTitle, "Title_Init(): pTitle");
    AK_ASSERT_PTR_VOID(pRect, "Title_Init(): rect");

    pTitle->initFlag = INITIALIZED_FLAG;
    Title_SetLoc(pTitle, pRect);

    pTitle->uTextLen = 0;
    pTitle->chrBegin = 0;
    pTitle->MaxCol = 0;
    pTitle->uText[0]  = 0;
    pTitle->bmpData  = AK_NULL;

    pTitle->TmCount = 0;

    return;
}

/**
 * @brief Set the location of the title
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-05-25 
 * @param T_TITLE *title
 * @param  T_POS left
 * @param  T_POS top
 * @param  T_LEN width
 * @param  T_LEN height
 * @return T_VOID
 * @retval 
 */
T_VOID Title_SetLoc(T_TITLE *pTitle, const T_pRECT pRect)
{
    AK_ASSERT_PTR_VOID(pTitle, "Title_SetLoc(): pTitle");
    AK_ASSERT_PTR_VOID(pRect, "Title_SetLoc(): rect");
    AK_ASSERT_VAL_VOID(pTitle->initFlag == INITIALIZED_FLAG, "Title not initialized");   /* check pTitle contorl has been initialized or not */

    pTitle->Rect = *pRect;
    pTitle->Rect.height = (T_LEN)(pRect->top + pRect->height > Fwl_GetLcdHeight() ? Fwl_GetLcdHeight() - pRect->top : pRect->height);

    return;
}
/**
 * @brief Get the rect of title control
 * 
 * @author @b LiaoJianhua
 * 
 * @date 2004-10-12
 * @param T_TITLE *title: The title control pointer
 * @return String *: return a String * pointer If the encode successful, 
                          otherwise, return AK_NULL(if memory is not enough)
 */ 

const T_pRECT Title_GetRect(const T_TITLE *pTitle)
{
    return (const T_pRECT)(&pTitle->Rect);
}
/**
 * @brief Set title text and bitmap
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-05-25 
 * @param T_TITLE *title : title sturcture
 * @param  T_pSTR text : title text
 * @param  T_pDATA bmpData :title bitmap data for shown before the text
 * @return T_VOID
 * @retval 
 */
T_VOID Title_SetData(T_TITLE *pTitle, T_pWSTR pText, T_pCDATA bmpData)
{
    AK_ASSERT_PTR_VOID(pTitle, "Title_SetData(): pTitle");
    AK_ASSERT_PTR_VOID(pText, "Title_SetData(): pText");
    AK_ASSERT_VAL_VOID(pTitle->initFlag == INITIALIZED_FLAG, "Title not initialized");   /* check pTitle contorl has been initialized or not */

    Utl_UStrCpyN(pTitle->uText, pText, TITLE_LEN_MAX);
    pTitle->bmpData  = (T_pDATA)bmpData;

    pTitle->uTextLen = (T_U16)Utl_UStrLen(pTitle->uText);
    if (pTitle->bmpData != AK_NULL)
    {
        pTitle->Width = 8 + 14;
    }
    else
    {
        pTitle->Width = 8;
    }
    pTitle->MaxCol = pTitle->Rect.width-pTitle->Width-2;
    return;
}

/**
 * @brief Get title text
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-05-25 
 * @param T_TITLE *title : title sturcture
 * @param  T_pSTR text : title text
 * @return T_VOID
 * @retval 
 */
T_pWSTR Title_GetText(T_pWSTR pText, const T_TITLE *pTitle)
{
    AK_ASSERT_PTR(pTitle, "TitleGetText(): pTitle", AK_NULL);
    AK_ASSERT_VAL(pTitle->initFlag == INITIALIZED_FLAG, "Title not initialized", AK_NULL);   /* check pTitle contorl has been initialized or not */

    if (pText == AK_NULL)        /* only return the pointer */
    {
        return (T_pWSTR)pTitle->uText;
    }
    else
    {
        Utl_UStrCpy(pText, (T_pWSTR)pTitle->uText);  /* return the content */
        return pText;
    }
}

/**
 * @brief Show title.
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-05-25 
 * @param T_TITLE *title: title sturcture
 * @return T_VOID
 * @retval 
 */
T_VOID Title_Show(T_TITLE *pTitle, T_BOOL showImg)
{
    AK_ASSERT_PTR_VOID(pTitle, "Title_Show(): pTitle");
    AK_ASSERT_VAL_VOID(pTitle->initFlag == INITIALIZED_FLAG, "Title not initialized");   /* check pTitle contorl has been initialized or not */

    if ((pTitle->MaxCol == 0) || (pTitle->Rect.height == 0))
    {
        Fwl_Print(C3, M_CTRL, "Title_Show(): show fail");
        return;
    }

    pTitle->chrBegin = pTitle->uTextLen;
    pTitle->TmCount = -1;
    Title_ScrollText(pTitle, showImg);

    return;
}

/**
 * @brief Show title.
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date  2002-05-25 
 * @param T_TITLE *title: title sturcture
 * @return T_BOOL
 * @retval 
 */
T_BOOL Title_ScrollText(T_TITLE *pTitle, T_BOOL showImg)
{
    T_BOOL      lastPage = AK_FALSE;
    T_U8        yOffset;
    T_RECT      rect;
    T_pCDATA    defTitleImg;
    T_U32       titleImgLen;
    T_LEN       defTitleWidth;
    T_LEN       defTitleHght;
    //T_U16   strWidth;

    AK_ASSERT_PTR(pTitle, "Title_ScrollText(): pTitle", 0);
    AK_ASSERT_VAL(pTitle->initFlag == INITIALIZED_FLAG, "Title not initialized", 0); /* check pTitle contorl has been initialized or not */

    pTitle->TmCount++;       /* flash pTitle */
    if ((pTitle->TmCount != 0) && (pTitle->TmCount <= TEXT_SCROLL_DELAY))
        return AK_FALSE;

    if (pTitle->MaxCol == 0)
        return AK_FALSE;

    if ((pTitle->uTextLen <= pTitle->MaxCol) && (pTitle->chrBegin != pTitle->uTextLen))       /* needn't scroll */
        return AK_FALSE;

    pTitle->chrBegin++;
    if (pTitle->chrBegin + pTitle->MaxCol >= pTitle->uTextLen)
    {
        if ((pTitle->chrBegin + pTitle->MaxCol) >= (pTitle->uTextLen + 1))
            pTitle->chrBegin = 0;
        else
            lastPage = AK_TRUE;
    }

    if (AK_TRUE == showImg)
    {
        defTitleImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_TITLE, &titleImgLen);
        if (AK_NULL == defTitleImg)
        {
            return AK_FALSE;
        }
        AKBmpGetInfo(defTitleImg, &defTitleWidth, &defTitleHght, AK_NULL);
        
        rect.width = pTitle->Rect.width/2;
        rect.height = defTitleHght;
        rect.left = 0;
        rect.top = 0;
            
        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, (T_POS)(pTitle->Rect.left), (T_POS)(pTitle->Rect.top), &rect, defTitleImg, &g_Graph.TransColor, AK_FALSE);
        rect.width = pTitle->Rect.width-rect.width;
        rect.height = defTitleHght;
        rect.left = defTitleWidth-rect.width;
        rect.top = 0;
        
        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, (T_POS)(pTitle->Rect.left+pTitle->Rect.width/2), (T_POS)(pTitle->Rect.top), &rect, defTitleImg, &g_Graph.TransColor, AK_FALSE);

        if (pTitle->bmpData != AK_NULL)
            Fwl_AkBmpDrawFromString(HRGB_LAYER, (T_POS)(pTitle->Rect.left+1), (T_POS)(pTitle->Rect.top), pTitle->bmpData, AK_NULL, AK_FALSE);
    }

    yOffset = (pTitle->Rect.height - g_Font.CHEIGHT) >> 1;


    Fwl_UScrollDispString(HRGB_LAYER, pTitle->uText, (T_POS)(pTitle->Rect.left + pTitle->Width), (T_POS)(pTitle->Rect.top+yOffset), 
                    pTitle->uTextLen, pTitle->chrBegin, pTitle->MaxCol, COLOR_BLACK, CURRENT_FONT_SIZE);


    if (lastPage)       /* the last page */
        pTitle->TmCount = -1 * TEXT_SCROLL_DELAY;

    return AK_TRUE;
}


