/**
 * @file Ctl_ScrollBar.c
 * @brief ANYKA software
 * Scroll bar Control
 * @author ZouMai
 * @date 2002-05-25
 * @version 1.0 
 */

#include "Ctl_ScrollBar.h"
#include "Eng_AkBmp.h"
#include "Eng_graph.h"
#include "fwl_pfdisplay.h"
#include "eng_debug.h"
#include "Lib_res_port.h"
#include "fwl_display.h"

#define  LITTLE_BAR_UP_ICON_HEIGHT         9
#define  BIG_BAR_UP_ICON_HEIGHT            18

static T_pRECT ScBar_GetLoca(T_RECT *rect, const T_SCBAR *bar, T_U32 cur);
static T_VOID  ScBar_GetLoca2(T_RECT *rect, const T_SCBAR *bar, T_U32 cVal);
static T_VOID ScBar_InitResRect(T_SCBAR *bar);

/**
 * @brief Initialize T_SCBAR structure
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-05-25
 * @param T_SCBAR *bar: scroll bar sturcture
 * @param  T_POS left: scroll bar location
 * @param  T_POS top: scroll bar location
 * @param  T_LEN width: scroll bar location
 * @param  T_LEN height: scroll bar location
 * @param  T_S16 intvl: interval of the scroll bar
 * @param  T_U16 mode: be SCBAR_VERTICAL or SCBAR_HORIZONTAL
 * @return T_VOID
 * @retval 
 */
T_VOID ScBar_Init(T_SCBAR *bar, T_POS left, T_POS top, T_LEN width, T_LEN height, T_S16 intvl, T_U16 mode)
{
    T_U32   i;
    T_U32   tmpH;
    
    AK_ASSERT_PTR_VOID(bar, "ScBar_Init(): bar");
    AK_ASSERT_VAL_VOID(width <= Fwl_GetLcdWidth(), "ScBar_Init(): width");
    AK_ASSERT_VAL_VOID(height <= Fwl_GetLcdHeight(), "ScBar_Init(): height");

    bar->initFlag = INITIALIZED_FLAG;
    bar->dirMode    = (T_U16)(mode & 0x0001);

    for (i=0; i<4; i++)
    {
        bar->lScBarImg[i] = AK_NULL;
    }

    ScBar_LoadImageData(bar);
    ScBar_InitResRect(bar);

    bar->upIconRect.left = left;
    bar->upIconRect.top = top;
    
    bar->downIconRect.left = left;
    bar->downIconRect.top = top + height - bar->downIconRect.height;

    //ScBar_SetLoc(bar, left, top, width, height);
    tmpH = bar->upIconRect.height + bar->downIconRect.height;
    ScBar_SetLoc(bar, left, (T_POS)(top + bar->upIconRect.height), width, (T_LEN)(height - tmpH));

    //if (width > g_Graph.LScBarWidth)
        bar->useBigBar = AK_TRUE;
    /*else
        bar->useBigBar = AK_FALSE;*/
    bar->Intvl  = intvl;

    bar->Available = AK_TRUE;
   
    bar->Side = g_Graph.LScBarWidth;

    bar->bkColor = g_Graph.ScBar_BkCL;

    bar->CurValue = 0;
    bar->TtlValue = 0;
    bar->PgFactor = 0;

    return;
}

/**
 * @brief Set the location of the rectangle
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-05-25
 * @param T_SCBAR *bar
 * @param  T_POS left
 * @param  T_POS top
 * @param  T_LEN width
 * @param  T_LEN height
 * @return T_VOID
 * @retval 
 */
T_VOID ScBar_SetLoc(T_SCBAR *bar, T_POS left, T_POS top, T_LEN width, T_LEN height)
{
    AK_ASSERT_PTR_VOID(bar, "ScBar_SetLoc(): bar");
    AK_ASSERT_VAL_VOID(bar->initFlag == INITIALIZED_FLAG, "scBar not initialized"); /* check bar contorl has been initialized or not */
    AK_ASSERT_VAL_VOID(width <= Fwl_GetLcdWidth(), "ScBar_SetLoc(): width");

    bar->Left   = left;
    bar->Top    = top;
    bar->Width  = width;
    bar->Height = top + height > Fwl_GetLcdHeight() ? Fwl_GetLcdHeight() - top : height;
    if (bar->dirMode == SCBAR_VERTICAL && bar->Height - bar->Side * 2 <= 0 ||
        bar->dirMode == SCBAR_HORIZONTAL && bar->Width - bar->Side * 2 <= 0)
        bar->Available = AK_FALSE;

    return;
}

/**
 * @brief Set scroll bar current value and total value
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-05-25
 * @param T_SCBAR *bar: scroll bar sturcture
 * @param  T_U16 cur: current value of scroll bar
 * @param  T_U16 total: total value of scroll bar
 * @param  T_U16 factor: factor number will scroll page
 * @return T_BOOL
 * @retval 
 */
T_BOOL ScBar_SetValue(T_SCBAR *bar, T_U32 cur, T_U32 total, T_U16 factor)
{
    AK_ASSERT_PTR(bar, "ScBar_SetValue(): bar", AK_FALSE);
    AK_ASSERT_VAL(bar->initFlag == INITIALIZED_FLAG, "scBar not initialized", AK_FALSE);    /* check bar contorl has been initialized or not */

    if (!bar->Available)
        return AK_FALSE;

    bar->TtlValue = total;
    bar->PgFactor = factor;
    ScBar_SetCurValue(bar, cur);
    return AK_TRUE;
}

/**
 * @brief get scroll bar display location
 * 
 * @author @b he_ying_gz
 * 
 * @author 
 * @date 2008-04-02
 * @param T_SCBAR *bar: scroll bar structure
 * @return T_RECT
 * @retval location in rect
 */
T_RECT ScBar_GetRect(T_SCBAR *bar)
{
    T_RECT rect;
    rect.left = bar->Left;
    rect.width = bar->Width;
    rect.top = bar->Top;
    rect.height = bar->Height;
    return rect;
}
/**
 * @brief Set the current value of the scroll bar 
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-05-25
 * @param T_SCBAR *bar
 * @param  T_S16 cur
 * @return T_BOOL
 * @retval 
 */
T_BOOL ScBar_SetCurValue(T_SCBAR *bar, T_U32 cur)
{
    AK_ASSERT_PTR(bar, "ScBar_SetCurValue(): bar", AK_FALSE);
    AK_ASSERT_VAL(bar->initFlag == INITIALIZED_FLAG, "scBar not initialized", AK_FALSE);    /* check bar contorl has been initialized or not */

    if (!bar->Available)
        return AK_FALSE;
    if(bar->TtlValue == 0)
        return AK_FALSE;
#if 0
    if (cur < 0)
        bar->CurValue = 0;
    else if (cur >= bar->TtlValue)
        bar->CurValue = (T_U16)(bar->TtlValue - 1);
    else
        bar->CurValue = cur;
#endif
    if (cur >= bar->TtlValue)
        bar->CurValue = bar->TtlValue - 1;
    else
        bar->CurValue = cur;

    return AK_TRUE;
}

/**
 * @brief Show the Scroll bar
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-05-25
 * @param T_SCBAR *bar
 * @return T_VOID
 * @retval 
 */
T_VOID ScBar_Show(T_SCBAR *bar)
{
    T_RECT  rect;
    T_S16   i;
    T_COLOR mycolor1, mycolor2;
    T_RECT   range;
    T_LEN    width;
    T_LEN    height;
    T_pCDATA *barImg;

    mycolor1 = RGB2AkColor(13, 142, 249, LCD0_COLOR);
    mycolor2 = RGB2AkColor(106, 186, 251, LCD0_COLOR);

    AK_ASSERT_PTR_VOID(bar, "ScBar_Show(): bar");
    AK_ASSERT_VAL_VOID(bar->initFlag == INITIALIZED_FLAG, "scBar not initialized"); /* check bar contorl has been initialized or not */

    if (bar->Width == 0 || bar->Height == 0 || bar->TtlValue < 1)
        return;

    /**load resource*/
    ScBar_LoadImageData(bar);

    if (bar->dirMode == SCBAR_VERTICAL)
    {
        if (bar->Intvl > 0 && (T_S16)((bar->Height + bar->Intvl) / bar->TtlValue) <= bar->Intvl)
        {
            bar->Intvl = 0;
        }

        /* show scroll bar */
        ScBar_GetLoca(&rect, bar, bar->CurValue);
        if (bar->useBigBar)
        {
            barImg = bar->bScBarImg;
        }
        else
        {
            barImg = bar->lScBarImg;
        }

        /* show up icon*/
        //ScBar_GetLocaRect(&rect1, bar);
        if (rect.top > bar->Top)
        {
            //blue
            Fwl_AkBmpDrawFromString(HRGB_LAYER, bar->upIconRect.left, bar->upIconRect.top, \
                    bar->bUpIcon[1], &g_Graph.TransColor, AK_FALSE);
        }
        else
        {   //gray
            Fwl_AkBmpDrawFromString(HRGB_LAYER, bar->upIconRect.left, bar->upIconRect.top, \
                    bar->bUpIcon[0], &g_Graph.TransColor, AK_FALSE);
        }
        
        if (rect.top < bar->Top + bar->Height - rect.height)
        {
            //blue
            Fwl_AkBmpDrawFromString(HRGB_LAYER, bar->downIconRect.left, bar->downIconRect.top, \
                    bar->bDownIcon[1], &g_Graph.TransColor, AK_FALSE);
        }
        else
        {
            //gray
            Fwl_AkBmpDrawFromString(HRGB_LAYER, bar->downIconRect.left, bar->downIconRect.top, \
                    bar->bDownIcon[0], &g_Graph.TransColor, AK_FALSE);
        }
        
        /* show background */
        if (bar->useBigBar)
        {
            AKBmpGetInfo(barImg[0], &width, &height, AK_NULL);
            range.width = width;
            range.height = bar->Height ;
            range.left = 0;
            range.top = 0;
            Fwl_AkBmpDrawPartFromString(HRGB_LAYER, bar->Left, bar->Top, &range, barImg[0], AK_NULL, AK_FALSE);

            range.top = 0;
            range.height = rect.height;
            Fwl_AkBmpDrawPartFromString(HRGB_LAYER, rect.left, rect.top, &range, barImg[1], AK_NULL, AK_FALSE);
        }
        else
        {
                AKBmpGetInfo(barImg[0], &width, &height, AK_NULL);
                range.width = width;
                range.height = bar->Height >> 1;
                range.left = 0;
                range.top = 0;
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, bar->Left, bar->Top, &range, barImg[0], AK_NULL, AK_FALSE);
                range.width = width;
                range.height = bar->Height - range.height;
                range.left = 0;
                range.top = height - range.height;
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, bar->Left, (T_POS)(bar->Top + (bar->Height >> 1)), &range, barImg[0], AK_NULL, AK_FALSE);

                
                
                Fwl_AkBmpDrawFromString(HRGB_LAYER, rect.left, rect.top, barImg[1], &g_Graph.TransColor, AK_FALSE);
                for (i = rect.top+2; i < rect.top+rect.height-2; i++)
                    Fwl_AkBmpDrawFromString(HRGB_LAYER, rect.left, i, barImg[2], &g_Graph.TransColor, AK_FALSE);
                Fwl_AkBmpDrawFromString(HRGB_LAYER, rect.left, (T_POS)(rect.top+rect.height-2), barImg[3], &g_Graph.TransColor, AK_FALSE);
            
        }
     }
    else
    {
        if (bar->Intvl > 0 && (T_S16)((bar->Width + bar->Intvl) / bar->TtlValue) <= bar->Intvl)
        {
            bar->Intvl = 0;
        }
        if (bar->Width - bar->Side * 2 <= 0)
        {
            Fwl_FillSolidRect(HRGB_LAYER, bar->Left, bar->Top, bar->Width, bar->Height, g_Graph.WinBkCL[DISPLAY_LCD_0]);
            return;
        }
    }

    return;
}

/**
 * @brief Show the Scroll bar
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-05-25
 * @param T_SCBAR *bar
 * @return T_VOID
 * @retval 
 */
T_VOID ScBar_Show2(T_SCBAR *bar)
{
    T_RECT  rect;
    T_S16   i;
    T_COLOR mycolor1, mycolor2;
    T_RECT   range;
    T_LEN    width;
    T_LEN    height;
    T_pCDATA *barImg;

    mycolor1 = RGB2AkColor(13, 142, 249, LCD0_COLOR);
    mycolor2 = RGB2AkColor(106, 186, 251, LCD0_COLOR);

    AK_ASSERT_PTR_VOID(bar, "ScBar_Show(): bar");
    AK_ASSERT_VAL_VOID(bar->initFlag == INITIALIZED_FLAG, "scBar not initialized"); /* check bar contorl has been initialized or not */

    if (bar->Width == 0 || bar->Height == 0 || bar->TtlValue < 1)
        return;

    /**load resource*/
    ScBar_LoadImageData(bar);

    if (bar->dirMode == SCBAR_VERTICAL)
    {
        if (bar->Intvl > 0 && (T_S16)((bar->Height + bar->Intvl) / bar->TtlValue) <= bar->Intvl)
        {
            bar->Intvl = 0;
        }

        /* show scroll bar */
        ScBar_GetLoca2(&rect, bar, bar->CurValue);
        if (bar->useBigBar)
        {
            barImg = bar->bScBarImg;
        }
        else
        {
            barImg = bar->lScBarImg;
        }

        /* show up icon*/
        //ScBar_GetLocaRect(&rect1, bar);
        if (rect.top > bar->Top)
        {
            //blue
            Fwl_AkBmpDrawFromString(HRGB_LAYER, bar->upIconRect.left, bar->upIconRect.top, \
                    bar->bUpIcon[1], &g_Graph.TransColor, AK_FALSE);
        }
        else
        {   //gray
            Fwl_AkBmpDrawFromString(HRGB_LAYER, bar->upIconRect.left, bar->upIconRect.top, \
                    bar->bUpIcon[0], &g_Graph.TransColor, AK_FALSE);
        }
        
        if (rect.top < bar->Top + bar->Height - rect.height)
        {
            //blue
            Fwl_AkBmpDrawFromString(HRGB_LAYER, bar->downIconRect.left, bar->downIconRect.top, \
                    bar->bDownIcon[1], &g_Graph.TransColor, AK_FALSE);
        }
        else
        {
            //gray
            Fwl_AkBmpDrawFromString(HRGB_LAYER, bar->downIconRect.left, bar->downIconRect.top, \
                    bar->bDownIcon[0], &g_Graph.TransColor, AK_FALSE);
        }
        
        /* show background */
        if (bar->useBigBar)
        {
            AKBmpGetInfo(barImg[0], &width, &height, AK_NULL);
            range.width = width;
            range.height = bar->Height ;
            range.left = 0;
            range.top = 0;
            Fwl_AkBmpDrawPartFromString(HRGB_LAYER, bar->Left, bar->Top, &range, barImg[0], AK_NULL, AK_FALSE);

            range.top = 0;
            range.height = rect.height;
            Fwl_AkBmpDrawPartFromString(HRGB_LAYER, rect.left, rect.top, &range, barImg[1], AK_NULL, AK_FALSE);
        }
        else
        {
                AKBmpGetInfo(barImg[0], &width, &height, AK_NULL);
                range.width = width;
                range.height = bar->Height >> 1;
                range.left = 0;
                range.top = 0;
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, bar->Left, bar->Top, &range, barImg[0], AK_NULL, AK_FALSE);
                range.width = width;
                range.height = bar->Height - range.height;
                range.left = 0;
                range.top = height - range.height;
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, bar->Left, (T_POS)(bar->Top + (bar->Height >> 1)), &range, barImg[0], AK_NULL, AK_FALSE);

                
                
                Fwl_AkBmpDrawFromString(HRGB_LAYER, rect.left, rect.top, barImg[1], &g_Graph.TransColor, AK_FALSE);
                for (i = rect.top+2; i < rect.top+rect.height-2; i++)
                    Fwl_AkBmpDrawFromString(HRGB_LAYER, rect.left, i, barImg[2], &g_Graph.TransColor, AK_FALSE);
                Fwl_AkBmpDrawFromString(HRGB_LAYER, rect.left, (T_POS)(rect.top+rect.height-2), barImg[3], &g_Graph.TransColor, AK_FALSE);
            
        }
     }
    else
    {
        if (bar->Intvl > 0 && (T_S16)((bar->Width + bar->Intvl) / bar->TtlValue) <= bar->Intvl)
        {
            bar->Intvl = 0;
        }
        if (bar->Width - bar->Side * 2 <= 0)
        {
            Fwl_FillSolidRect(HRGB_LAYER, bar->Left, bar->Top, bar->Width, bar->Height, g_Graph.WinBkCL[DISPLAY_LCD_0]);
            return;
        }
    }

    return;
}

/**
 * @brief load scroll bar image data
 * @author wangwei
 * @date 2002-05-25
 * @param T_SCBAR *bar
 * @return T_VOID
 * @retval 
 */
T_VOID ScBar_LoadImageData(T_SCBAR *bar)
{
    T_U32 i;
   
    AK_ASSERT_PTR_VOID(bar, "ScBar_Init(): bar");

    for (i=0; i<2; i++)
    {
        bar->bScBarImg[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_BSCBAR_BACKGROUND + i, AK_NULL);
        bar->bUpIcon[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_BSCBAR_UP_ICON0 + i, AK_NULL);
        bar->bDownIcon[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_BSCBAR_DOWN_ICON0 + i, AK_NULL);
    }
}

/*---------------------------------------------------------------------------
                        STATIC function definition

  All the following functions can only be used in current source file.
---------------------------------------------------------------------------*/
static T_VOID ScBar_InitResRect(T_SCBAR *bar)
{
    if (AK_NULL == bar)
    {
        bar = AK_NULL;
    }

    if (AK_NULL != bar->bScBarImg[0])

    {
        AKBmpGetInfo(bar->bScBarImg[0], &bar->scbarRect.width, &bar->scbarRect.height, AK_NULL);
    }
    if (AK_NULL != bar->bUpIcon[0])
    {
        AKBmpGetInfo(bar->bUpIcon[0], &bar->upIconRect.width, &bar->upIconRect.height, AK_NULL);
    }
    if (AK_NULL != bar->bDownIcon[0])
    {
        AKBmpGetInfo(bar->bDownIcon[0], &bar->downIconRect.width, &bar->downIconRect.height, AK_NULL);
    }
}

/**
 * @brief Get the position of the scroll bar
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-05-25
 * @param T_SCBAR *bar
 * @param  T_RECT *rect
 * @param  T_U16 cVal: current value
 * @return T_RECT
 * @retval 
 */
 static T_pRECT ScBar_GetLoca(T_RECT *rect, const T_SCBAR *bar, T_U32 cVal)
 {
     T_U32   total;
     T_POS   x0;
     T_POS   y0;
     T_LEN   width;
     T_LEN   height;
     T_LEN   upIconHght;
     
     AK_ASSERT_PTR(bar, "ScBar_GetLoca(): bar", AK_NULL);
     AK_ASSERT_PTR(rect, "ScBar_GetLoca(): rect", AK_NULL);
     AK_ASSERT_VAL(bar->initFlag == INITIALIZED_FLAG, "scBar not initialized", AK_NULL); /* check bar contorl has been initialized or not */
     
     total = bar->TtlValue;
     //x0 = bar->Left;
     
     //x0 = bar->Left + (bar->Width - bar->Side) / 2;
     x0 = bar->Left;
     y0 = bar->Top;
     width = bar->Width;
     height = bar->Height;
     
     if (bar->useBigBar)
     {
         upIconHght = BIG_BAR_UP_ICON_HEIGHT;
     }
     else
     {
         upIconHght = LITTLE_BAR_UP_ICON_HEIGHT;
     }
     
     if (bar->dirMode == SCBAR_VERTICAL)
     {
         if (!bar->useBigBar)
         {
             y0 += upIconHght;
             height -= (upIconHght << 1);
         }
         if ((height > 0) && (total > bar->PgFactor) && total)
         {
             rect->width = width;
             rect->height = (T_LEN)((height * bar->PgFactor) / total);
             if (rect->height < 4)
                 rect->height = 4;
             rect->left = x0;
             rect->top = (T_S16)(y0 + (((height - rect->height) * cVal) / (total - bar->PgFactor)));
         }
         else if ((height > 0) && total)
         {
             rect->left = x0;
             rect->top = bar->Top;
             rect->width = width;
             rect->height = bar->Height;
         }
     }
     else
     {
         x0 += upIconHght;
         width -= (upIconHght << 1);
         if (width>0){
             rect->left = (T_S16)(x0 + (width + bar->Intvl) * cVal / total);
             rect->top = y0;
             rect->width = (T_S16)((width + bar->Intvl) * (cVal+1) / total) - (T_S16)((width + bar->Intvl) * cVal / total) - bar->Intvl;
             rect->height = height;
         }
         else{
             rect->left = bar->Left + bar->Width / 2;
             rect->top = y0;
             rect->width = 0;
             rect->height = height;
         }
     }
     return rect;
 }

T_VOID ScBar_GetLocaRect(T_RECT *rect, const T_SCBAR *bar)
{
    //ScBar_GetLoca2(rect, bar, bar->CurValue);
    ScBar_GetLoca(rect, bar, bar->CurValue);
}

static T_VOID ScBar_GetLoca2(T_RECT *rect, const T_SCBAR *bar, T_U32 cVal)
 {
     T_U32   total;
     T_POS   x0;
     T_POS   y0;
     T_LEN   width;
     T_LEN   height;
     T_LEN   upIconHght;
     
     AK_ASSERT_PTR_VOID(bar, "ScBar_GetLoca(): bar");
     AK_ASSERT_PTR_VOID(rect, "ScBar_GetLoca(): rect");
     AK_ASSERT_VAL_VOID(bar->initFlag == INITIALIZED_FLAG, "scBar not initialized"); /* check bar contorl has been initialized or not */
     
     total = bar->TtlValue;
     x0 = bar->Left;
     y0 = bar->Top;
     width = bar->Width;
     height = bar->Height;
     
     if (bar->useBigBar)
     {
         upIconHght = BIG_BAR_UP_ICON_HEIGHT;
     }
     else
     {
         upIconHght = LITTLE_BAR_UP_ICON_HEIGHT;
     }
     
     if (bar->dirMode == SCBAR_VERTICAL)
     {
         if (!bar->useBigBar)
         {
             y0 += upIconHght;
             height -= (upIconHght << 1);
         }
         if ((height > 0) && (total > bar->PgFactor) && total)
         {
             rect->width = width;
             rect->height = (T_LEN)((height * bar->PgFactor) / total);
             if (rect->height < 4)
                 rect->height = 4;
             rect->left = x0;
             rect->top = (T_S16)(y0 + (((height - rect->height) * cVal) / (total-1)));
         }
         else if ((height > 0) && total)
         {
             rect->left = x0;
             rect->top = bar->Top;
             rect->width = width;
             rect->height = bar->Height;
         }
     }
     else
     {
         x0 += upIconHght;
         width -= (upIconHght << 1);
         if (width>0){
             rect->left = (T_S16)(x0 + (width + bar->Intvl) * cVal / total);
             rect->top = y0;
             rect->width = (T_S16)((width + bar->Intvl) * (cVal+1) / total) - (T_S16)((width + bar->Intvl) * cVal / total) - bar->Intvl;
             rect->height = height;
         }
         else{
             rect->left = bar->Left + bar->Width / 2;
             rect->top = y0;
             rect->width = 0;
             rect->height = height;
         }
     }
 }

T_VOID ScBar_GetLocaRect2(T_RECT *rect, const T_SCBAR *bar)
{
    ScBar_GetLoca2(rect, bar, bar->CurValue);
}

T_VOID ScBar_GetUpIconRect(T_pRECT pRect, const T_SCBAR *pBar)
{
    if ((AK_NULL == pRect) || (AK_NULL == pBar))
    {
        return;
    }
    
    pRect->left = pBar->upIconRect.left;
    pRect->top  = pBar->upIconRect.top;
    pRect->width = pBar->upIconRect.width;
    pRect->height= pBar->upIconRect.height;    
}

T_VOID ScBar_GetDownIconRect(T_pRECT pRect, const T_SCBAR *pBar)
{
    if ((AK_NULL == pRect) || (AK_NULL == pBar))
    {
        return;
    }
    
    pRect->left = pBar->downIconRect.left;
    pRect->top  = pBar->downIconRect.top;
    pRect->width = pBar->downIconRect.width;
    pRect->height= pBar->downIconRect.height;    
}

