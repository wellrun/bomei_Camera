#include "Ctl_ListBox.h"
#include "fwl_keyhandler.h"
#include "eng_font.h"
#include "Eng_AkBmp.h"
#include "Eng_String_UC.h"
#include "Eng_DynamicFont.h"
#include "fwl_oscom.h"
#include "AKError.h"
#include "eng_graph.h"
#include "fwl_pfdisplay.h"
#include "Lib_res_port.h"
#include "Fwl_tscrcom.h"
#include "fwl_display.h"


static T_VOID ListBox_RefreshTitle(T_LISTBOX *pListBox);
static T_VOID ListBox_RefreshItems(T_LISTBOX *pListBox);
static T_VOID ListBox_RefreshScBar(T_LISTBOX *pListBox);
static T_VOID ListBox_RefreshBg(T_LISTBOX *pListBox);
static T_VOID ListBox_RefreshItemScrollText(T_LISTBOX *pListBox);
static T_VOID ListBox_RefreshLeft(T_LISTBOX *pListBox);
static T_VOID ListBox_RefreshBottom(T_LISTBOX *pListBox);
static T_U32 ListBox_GetItemPageIndexByPos(T_LISTBOX *pListBox, T_U16 Pos_x, T_U16 Pos_y);
static T_LIXTBOX_AREA ListBox_GetTouchArea(T_LISTBOX *pListBox, T_POS x, T_POS y);

#if 0
static ListBox_Printf(T_S8 *pstr, T_RECT rect)
{
    Fwl_Print(C3, M_CTRL, "%s rect.left: %d\n", pstr, rect.left);
    Fwl_Print(C3, M_CTRL, "%s rect.top: %d\n", pstr, rect.top);
    Fwl_Print(C3, M_CTRL, "%s rect.width: %d\n", pstr, rect.width);
    Fwl_Print(C3, M_CTRL, "%s rect.height: %d\n", pstr, rect.height);
}
#endif


static T_VOID ListBox_RefreshBg(T_LISTBOX *pListBox)
{    
    if (AK_NULL != pListBox)
    {
        T_RECT rect;
        T_U32  imgLen = 0;
        
        pListBox->Item.pBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_ITEM, &imgLen);

        if (AK_NULL != pListBox->Item.pBgImg)
        {
            T_U8 Deep = 0;
            T_LEN height = pListBox->Rect.height;
            T_LEN width = pListBox->Rect.width;
            
            AKBmpGetInfo(pListBox->Title.pBgImg, &rect.width, &rect.height, &Deep);

            rect.left   = 0;
            rect.top    = 0;
            rect.width  = (rect.width<width) ? rect.width : width;
            rect.height = (rect.height<height) ? rect.height : height;
            
            Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pListBox->Rect.left, pListBox->Rect.top, &rect, pListBox->Item.pBgImg, \
                                   &g_Graph.TransColor, AK_FALSE);  
        }
    }    
}

static T_VOID ListBox_RefreshTitle(T_LISTBOX *pListBox)
{
    T_RECT img_rect;
    T_FONT TitleFont;
    T_U16 *pTitleText = AK_NULL;
    T_U16  pos_x, pos_y;
    T_U16  disp_num;
	T_U32  txt_width;
	T_U16  txt_len;
    T_COLOR TitleFontColor, TitleBgColor;
    T_U16  TitleLeft, TitleTop, TitleWidth, TitleHeight, TitleFontHeight;

    if (pListBox)
    {
        TitleFont = pListBox->Title.Font;
        pTitleText = pListBox->Title.pText;
        TitleFontHeight = GetFontHeight(TitleFont);
        TitleLeft = pListBox->Title.Rect.left;
        TitleFontHeight = GetFontHeight(TitleFont);
        TitleTop = pListBox->Title.Rect.top;
        TitleWidth = pListBox->Title.Rect.width;
        TitleHeight = pListBox->Title.Rect.height;
        
        TitleFontColor = pListBox->Title.FontColor;
        TitleBgColor = pListBox->Title.BgColor;
        
        if (pListBox->Title.VisuableMode == VISUABLE_NORMAL)
        {
            T_U32 imgLen = 0;
            
            pListBox->Title.pBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_TITLE, &imgLen);
            if (pListBox->Title.pBgImg)
            {
                img_rect.left = 0;
                img_rect.top = 0;
                img_rect.width = pListBox->Title.Rect.width;
                img_rect.height = pListBox->Title.Rect.height;
                
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, TitleLeft, TitleTop, &img_rect, pListBox->Title.pBgImg, \
                                        &g_Graph.TransColor, AK_FALSE);  
            }

            if (pListBox->Title.pText)
            {
                txt_len = (T_U16)Utl_UStrLen(pTitleText);
                disp_num = Fwl_GetUStringDispNum(pTitleText, txt_len, TitleWidth, TitleFont);
                
                if (disp_num > txt_len)
                {
                    disp_num = txt_len;
                }
                
                txt_width = UGetSpeciStringWidth(pTitleText, TitleFont, disp_num);
                pos_x = (T_U16)(TitleLeft + (TitleWidth - txt_width)/2);
                pos_y = (T_U16)(TitleTop+(TitleHeight-TitleFontHeight)/2);
                Fwl_UDispSpeciString(HRGB_LAYER, pos_x, pos_y, pTitleText, TitleFontColor, TitleFont, disp_num);
            }
        }
        else if (pListBox->Title.VisuableMode == VISUABLE_LINE)
        {
            Fwl_DrawLine(HRGB_LAYER, TitleLeft, TitleTop, (T_POS)(TitleLeft+TitleWidth), TitleTop, COLOR_BLACK);
        }
    }
}

static T_VOID ListBox_RefreshItems(T_LISTBOX *pListBox)
{
    T_U32  i;
    T_RECT img_rect;
    T_FONT ItemFont;
    T_LIST_ELEMENT *pItem = AK_NULL;
    T_U16  txt_len, disp_num;
    T_U32 page_item_real_num, index_start;
    T_U16  ItemLeft, ItemTopStart, ItemWidth, ItemHeight, ItemFontHeight;
    T_COLOR ItemFontColor, ItemBgColor, ItemFocusColor, tmp_color;

    if (pListBox)
    {
        ItemTopStart = pListBox->Item.Rect.top;
        ItemLeft = pListBox->Item.Rect.left;
        ItemWidth = pListBox->Item.Rect.width;

        if (pListBox->Title.VisuableMode == VISUABLE_NORMAL)
        {
            T_U32 imgLen = 0;
        
            ItemFont = pListBox->Item.Font;
            ItemFontHeight = GetFontHeight(ItemFont);
            ItemHeight = pListBox->Item.Height;
            
            pListBox->Item.pBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_ITEM, &imgLen);
            if (pListBox->Item.pBgImg)
            {
                ItemFontColor = pListBox->Item.FontColor;
                ItemBgColor = pListBox->Item.BgColor;
                ItemFocusColor = pListBox->Item.FocusFontColor;
                
                img_rect.left = 0;
                img_rect.top = 0;
                img_rect.width = ItemWidth;
                img_rect.height = pListBox->Item.Rect.height;

                {
                    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, ItemLeft, ItemTopStart, \
                            &img_rect, pListBox->Item.pBgImg, &g_Graph.TransColor, AK_FALSE);  
                }
            }

            if (pListBox->Item.pListItem->TotalItemNum > pListBox->Item.pListItem->PageItemNum)
            {
                page_item_real_num = pListBox->Item.pListItem->PageItemNum;
            }
            else
            {
                page_item_real_num = pListBox->Item.pListItem->TotalItemNum;
                pListBox->Item.pListItem->PageFirstItemIndex = 0;
            }
            
    
            index_start = pListBox->Item.pListItem->PageFirstItemIndex;
            pItem = ListItem_GetItemByIndex(pListBox->Item.pListItem, index_start);
       
            for (i=0; i<page_item_real_num; i++)
            {
                
                if ((index_start + i) == pListBox->Item.pListItem->FocusIndex)
                {
                    T_U32 imgLen = 0;
                    
                    tmp_color = pListBox->Item.FocusFontColor;

                    pListBox->Item.pFocusBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_ITEMBG, &imgLen);;
                    
                    if (pListBox->Item.pFocusBgImg)
                    {
                        img_rect.left = 0;
                        img_rect.top = 0;
                        img_rect.width = ItemWidth;
                        img_rect.height = ItemHeight;
                        Fwl_AkBmpDrawPartFromString(HRGB_LAYER, ItemLeft, (T_POS)(ItemTopStart+i*ItemHeight),
                                &img_rect, pListBox->Item.pFocusBgImg, &g_Graph.TransColor, AK_FALSE);
                    }
                }
                else
                {
                    tmp_color = pListBox->Item.FontColor;
                }            
    
                if (pItem)
                {
                    T_pCWSTR pItemText = pItem->pText;
                    txt_len = (T_U16)Utl_UStrLen(pItemText);
                    disp_num = Fwl_GetUStringDispNum((T_pWSTR)pItemText, txt_len, (T_U16)(ItemWidth - pListBox->Item.FontOffset), ItemFont);
                    Fwl_UDispSpeciString(HRGB_LAYER, (T_POS)(ItemLeft+pListBox->Item.FontOffset), \
                             (T_POS)(ItemTopStart+i*ItemHeight+(ItemHeight-ItemFontHeight)/2), \
                             (T_pWSTR)pItemText, tmp_color, ItemFont, disp_num);
                    
                    pItem = (T_LIST_ELEMENT *)pItem->pNext;
                }
            }
        }
        else if (pListBox->Item.VisuableMode == VISUABLE_LINE)
        {
            Fwl_DrawLine(HRGB_LAYER, ItemLeft, ItemTopStart, (T_POS)(ItemLeft+ItemWidth), ItemTopStart, COLOR_BLACK);
        }
    }
}


static T_VOID ListBox_RefreshItemScrollText(T_LISTBOX *pListBox)
{
    T_FONT ItemFont;
    T_RECT img_rect;
    T_U16  dstTop;
    T_LIST_ELEMENT *pItem = AK_NULL;  
    T_U32  txt_width; 
    T_U16  txt_len, focus_index;
    T_U16  ItemLeft, ItemTopStart, ItemWidth, ItemHeight, ItemFontHeight;
    T_COLOR ItemFontColor, ItemBgColor, ItemFocusColor;
     
    if (pListBox)
    {
        if (pListBox->Item.VisuableMode == VISUABLE_NORMAL)
        {
            if (pListBox->Item.pListItem->FocusIndex >= pListBox->Item.pListItem->PageFirstItemIndex \
                && pListBox->Item.pListItem->FocusIndex <= (pListBox->Item.pListItem->PageFirstItemIndex \
                + pListBox->Item.pListItem->PageItemNum))
            {
                ItemFont = pListBox->Item.Font;
                ItemFontHeight = GetFontHeight(ItemFont);
                ItemLeft = pListBox->Item.Rect.left;
                ItemTopStart = pListBox->Item.Rect.top;
                ItemWidth = pListBox->Item.Rect.width;
                ItemHeight = pListBox->Item.Height;
                
                ItemFontColor = pListBox->Item.FontColor;
                ItemBgColor = pListBox->Item.BgColor;
                ItemFocusColor = pListBox->Item.FocusFontColor;       
        
                focus_index = (T_U16)(pListBox->Item.pListItem->FocusIndex - pListBox->Item.pListItem->PageFirstItemIndex);
                pItem = ListItem_GetFocusItem(pListBox->Item.pListItem);
                if (pItem)
                {
                    T_pCWSTR pItemText = pItem->pText;
                    if (pItemText)
                    {
                        txt_len = (T_U16)Utl_UStrLen(pItem->pText);
                        txt_width = UGetSpeciStringWidth((T_pWSTR)pItemText, ItemFont, txt_len);
                        if (txt_width > ItemWidth)
                        {
                            T_U32 imgLen = 0;
                            
                            img_rect.left = 0;
                            img_rect.width = ItemWidth;
                            img_rect.height = ItemHeight;
                            
                            dstTop = ItemTopStart+focus_index*ItemHeight;
/*
                            if (pListBox->Item.pBgImg)
                            {
                                img_rect.top = focus_index*ItemHeight;
                                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, ItemLeft, dstTop, &img_rect, pListBox->Item.pBgImg, \
                                                &g_Graph.TransColor, AK_FALSE);
                            }
                            
*/
                            pListBox->Item.pFocusBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_ITEMBG, &imgLen);;
                            if (pListBox->Item.pFocusBgImg)
                            {
                                img_rect.top = 0;
                                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, ItemLeft, dstTop, &img_rect, pListBox->Item.pFocusBgImg, \
                                                        &g_Graph.TransColor, AK_FALSE);
                            }

                            Fwl_UScrollDispString(HRGB_LAYER, (T_pWSTR)pItemText, (T_POS)(ItemLeft+pListBox->Item.FontOffset), \
                                              (T_POS)(ItemTopStart+focus_index*ItemHeight+(ItemHeight-ItemFontHeight)/2), \
                                              txt_len, pListBox->ScrollOffset, (T_U16)(ItemWidth - pListBox->Item.FontOffset), \
                                              ItemFocusColor, ItemFont);
        
                            pListBox->ScrollOffset = (pListBox->ScrollOffset+1)%txt_len;
                        }
                    }
                }
            }
        }
    }
}


static T_VOID ListBox_RefreshScBar(T_LISTBOX *pListBox)
{
    if (pListBox)
    {
        T_RECT dst_rect;
        T_RECT img_rect;
        T_U16  tmp_height;
        T_U32  img_len;
        
        img_rect.left = 0;
        img_rect.top = 0;

        if (pListBox->ScBar.VisuableMode == VISUABLE_NORMAL)
        {        
            pListBox->ScBar.pBarBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_BARBG, &img_len);
            if (pListBox->ScBar.pBarBgImg)
            {
                img_rect.width = pListBox->ScBar.BarBgRect.width;
                img_rect.height = pListBox->ScBar.BarBgRect.height;
    
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pListBox->ScBar.BarBgRect.left, pListBox->ScBar.BarBgRect.top, \
                                    &img_rect, pListBox->ScBar.pBarBgImg, &g_Graph.TransColor, AK_FALSE);
            }

            pListBox->ScBar.pBarUpImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_BARUP, &img_len);
            if (pListBox->ScBar.pBarUpImg)
            {
                img_rect.width = pListBox->ScBar.BarUpRect.width;
                img_rect.height = pListBox->ScBar.BarUpRect.height;
    
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pListBox->ScBar.BarUpRect.left, pListBox->ScBar.BarUpRect.top, \
                                    &img_rect, pListBox->ScBar.pBarUpImg, &g_Graph.TransColor, AK_FALSE);
            }
            
            pListBox->ScBar.pBarDownImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_BARDOWN, &img_len);
            if (pListBox->ScBar.pBarDownImg)
            {
                img_rect.width = pListBox->ScBar.BarDownRect.width;
                img_rect.height = pListBox->ScBar.BarDownRect.height;
                
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pListBox->ScBar.BarDownRect.left, pListBox->ScBar.BarDownRect.top, \
                                    &img_rect, pListBox->ScBar.pBarDownImg, &g_Graph.TransColor, AK_FALSE);
            }        

            pListBox->ScBar.pBarImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_BAR, &img_len);
            if (pListBox->ScBar.pBarImg)
            {
                if (pListBox->Item.pListItem->TotalItemNum <= pListBox->Item.pListItem->PageItemNum)
                {
                    img_rect.width = pListBox->ScBar.BarRect.width;
                    img_rect.height = pListBox->ScBar.BarRect.height;

                    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pListBox->ScBar.BarRect.left, pListBox->ScBar.BarRect.top, \
                                        &img_rect, pListBox->ScBar.pBarImg, &g_Graph.TransColor, AK_FALSE);
                }
                else
                {                   
                    img_rect.width = pListBox->ScBar.BarRect.width;
                    img_rect.height = (T_U16)(pListBox->ScBar.BarRect.height*pListBox->Item.pListItem->PageItemNum
                                  /pListBox->Item.pListItem->TotalItemNum);
                    
                    dst_rect.left = pListBox->ScBar.BarRect.left;
                    tmp_height = (T_U16)((pListBox->Item.pListItem->PageFirstItemIndex) * (pListBox->ScBar.BarRect.height - img_rect.height) \
                                 /(pListBox->Item.pListItem->TotalItemNum - pListBox->Item.pListItem->PageItemNum));
                    if (tmp_height > pListBox->ScBar.BarRect.height - img_rect.height)
                    {
                        tmp_height = pListBox->ScBar.BarRect.height - img_rect.height;
                    }
                    
                    dst_rect.top = pListBox->ScBar.BarRect.top + tmp_height;

                    Fwl_AkBmpDrawPartFromString(HRGB_LAYER, dst_rect.left, dst_rect.top, &img_rect, pListBox->ScBar.pBarImg, \
                                            &g_Graph.TransColor, AK_FALSE);
                    
                    pListBox->ScBar.BarRunRect.width = img_rect.width;
                    pListBox->ScBar.BarRunRect.height = img_rect.height;
                    pListBox->ScBar.BarRunRect.left = dst_rect.left;
                    pListBox->ScBar.BarRunRect.top = dst_rect.top;
                }
            }
        }
        else if (pListBox->ScBar.VisuableMode == VISUABLE_LINE)
        {
            Fwl_DrawLine(HRGB_LAYER, pListBox->ScBar.BarRect.left, pListBox->ScBar.BarRect.top, \
                         pListBox->ScBar.BarRect.left, \
                         (T_POS)(pListBox->ScBar.BarRect.top+pListBox->ScBar.BarRect.height), COLOR_BLACK);
        }
    }
}


static T_VOID ListBox_RefreshLeft(T_LISTBOX *pListBox)
{
    if (pListBox)
    {
        if (pListBox->Left.VisuableMode == VISUABLE_NORMAL)
        {
            T_U32 imgLen = 0;
        
            pListBox->Left.pBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_LEFT, &imgLen);
            if (pListBox->Left.pBgImg)
            {
                T_RECT img_rect;
                
                img_rect.left = 0;
                img_rect.top = 0;
                img_rect.width = pListBox->Left.Rect.width;
                img_rect.height = pListBox->Left.Rect.height;
                
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pListBox->Left.Rect.left, pListBox->Left.Rect.top, \
                                    &img_rect, pListBox->Left.pBgImg, &g_Graph.TransColor, AK_FALSE);
            }
        }
        else if (pListBox->Left.VisuableMode == VISUABLE_LINE)
        {
            Fwl_DrawLine(HRGB_LAYER, pListBox->Left.Rect.left, pListBox->Left.Rect.top, \
                         pListBox->Left.Rect.left, \
                         (T_POS)(pListBox->Left.Rect.top+pListBox->Left.Rect.height), COLOR_BLACK);
        }
    }
    
}


static T_VOID ListBox_RefreshBottom(T_LISTBOX *pListBox)
{
    if (pListBox)
    {
        if (pListBox->Bottom.VisuableMode == VISUABLE_NORMAL)
        {    
            T_U32 imgLen = 0;
        
            pListBox->Bottom.pBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_BOTTOM, &imgLen);
            if (pListBox->Bottom.pBgImg)
            {
                T_RECT img_rect;
                
                img_rect.left = 0;
                img_rect.top = 0;
                img_rect.width = pListBox->Bottom.Rect.width;
                img_rect.height = pListBox->Bottom.Rect.height;
                
                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pListBox->Bottom.Rect.left, pListBox->Bottom.Rect.top, \
                                    &img_rect, pListBox->Bottom.pBgImg, &g_Graph.TransColor, AK_FALSE);
            }
        }
        else if (pListBox->Bottom.VisuableMode == VISUABLE_LINE)
        {
            Fwl_DrawLine(HRGB_LAYER, pListBox->Bottom.Rect.left, pListBox->Bottom.Rect.top, \
                         (T_POS)(pListBox->Bottom.Rect.left+pListBox->Bottom.Rect.width), \
                         pListBox->Bottom.Rect.top, COLOR_BLACK);
        }
    }
}

static T_VOID ListBox_RefreshExit(T_LISTBOX *pListBox)
{
    if (pListBox)
    {
        if (pListBox->Exit.VisuableMode == VISUABLE_NORMAL)
        {
            T_U32 imgLen = 0;
            
            pListBox->Exit.pImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_EXIT, &imgLen);
            if (pListBox->Exit.pImg)
            {
                T_RECT img_rect;
                
                img_rect.left = 0;
                img_rect.top = 0;
                img_rect.width = pListBox->Exit.Rect.width;
                img_rect.height = pListBox->Exit.Rect.height;

                Fwl_AkBmpDrawPartFromString(HRGB_LAYER, pListBox->Exit.Rect.left, pListBox->Exit.Rect.top, \
                                    &img_rect, pListBox->Exit.pImg, &g_Graph.TransColor, AK_FALSE);
            }
        }
    }
}




static T_U32 ListBox_GetItemPageIndexByPos(T_LISTBOX *pListBox, T_U16 Pos_x, T_U16 Pos_y)
{
    T_U32 index = INVALID_INDEX;
    
    if (pListBox)
    {
        if ((Pos_y > pListBox->Item.Rect.top) \
            && Pos_y < (pListBox->Item.Rect.top+pListBox->Item.Height*pListBox->Item.pListItem->PageItemNum) \
            &&(Pos_x > pListBox->Item.Rect.left) \
            &&(Pos_x < pListBox->Item.Rect.left+pListBox->Item.Rect.width))
        {
            index = (Pos_y - pListBox->Item.Rect.top)/pListBox->Item.Height;
        }
    }
    
    return index;
}


static T_LIXTBOX_AREA ListBox_GetTouchArea(T_LISTBOX *pListBox, T_POS x, T_POS y)
{
    T_LIXTBOX_AREA ret = LISTBOX_AREA_NONE;
    
    if (pListBox)
    {
        if (PointInRect(&pListBox->Rect, x, y)) //is in listbox area
        {
            if (pListBox->Item.VisuableMode == VISUABLE_NORMAL)
            {
                if (PointInRect(&pListBox->Item.Rect, x, y))        //in item area
                {
                    ret = LISTBOX_AREA_ITEM;
                }
            }
            
            if (pListBox->Exit.VisuableMode == VISUABLE_NORMAL)
            {
                if (PointInRect(&pListBox->Exit.Rect, x, y))        //in item area
                {
                    ret = LISTBOX_AREA_EXIT;
                }
            }
            
            if (pListBox->ScBar.VisuableMode == VISUABLE_NORMAL)    //in scbar area
            {
                if (PointInRect(&pListBox->ScBar.BarBgRect, x, y))  //in scbar bg area
                {
                    if (PointInRect(&pListBox->ScBar.BarUpRect, x, y))  //in bar up area
                    {
                        ret = LISTBOX_AREA_BARUP;
                    }
                    else if (PointInRect(&pListBox->ScBar.BarDownRect, x, y))  //in bar down area
                    {
                        ret = LISTBOX_AREA_BARDOWN;
                    }
                    else if (PointInRect(&pListBox->ScBar.BarRunRect, x, y))  //in scbar bg
                    {
                        ret = LISTBOX_AREA_BAR;
                    }
                    else
                    {
                        if (y < pListBox->ScBar.BarRunRect.top)
                        {
                            ret = LISTBOX_AREA_PAGEUP;
                        }
                        else if ((y > pListBox->ScBar.BarRunRect.top + pListBox->ScBar.BarRunRect.height))
                        {
                            ret = LISTBOX_AREA_PAGEDOWN;
                        }
                    }
                }
            }
        }
    }
    
    return ret;
}




T_BOOL ListBox_Init(T_LISTBOX *pListBox, T_LISTBOX_CFG *pCfg)
{
    T_U32 imgLen;
    T_U8 Deep = 0;
    T_U16 Width = 0, Height = 0;
    T_BOOL ret = AK_TRUE;
    T_U16 TitleHeitht = 0, BorderLeftWidth = 0;
    T_U16 BorderBottomHeight = 0, ScbarWidth = 0, ScbarBgWidth = 0;
    
    if (pListBox && pCfg)
    {
        T_LISTBOX_CFG tmpCfg;
        
        memset(pListBox, 0, sizeof(T_LISTBOX));
        memcpy(&tmpCfg, pCfg, sizeof(T_LISTBOX_CFG));
        memcpy(&pListBox->Rect, &pCfg->Rect, sizeof(T_RECT));

        /************************the visuable set********************************/
        if (tmpCfg.TitleVisuable == VISUABLE_NORMAL)
        {
            pListBox->Title.pBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_TITLE, &imgLen);
            if (pListBox->Title.pBgImg)
            {
                AKBmpGetInfo(pListBox->Title.pBgImg, &Width, &Height, &Deep);
                TitleHeitht = Height;
            }
            else
            {
                TitleHeitht = 0;
                tmpCfg.TitleVisuable = VISUABLE_NONE;
            }
        }
        else if (tmpCfg.TitleVisuable == VISUABLE_LINE)
        {
            TitleHeitht = 1;
        }
        else
        {
            TitleHeitht = 0;
        }
        
        if (tmpCfg.ItemVisuable == VISUABLE_NORMAL)
        {
            pListBox->Item.pBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_ITEM, &imgLen);
            if (pListBox->Item.pBgImg == AK_NULL)
            {
                //tmpCfg.ItemVisuable = VISUABLE_NONE;
            }
        }
        else if (tmpCfg.ItemVisuable == VISUABLE_LINE)
        {
            TitleHeitht = 1;
            tmpCfg.LeftVisuable = VISUABLE_LINE;
            tmpCfg.BottomVisuable = VISUABLE_LINE;
            tmpCfg.ScBarVisuable = VISUABLE_LINE;
        }
        else
        {
            tmpCfg.LeftVisuable = VISUABLE_NONE;
            tmpCfg.BottomVisuable = VISUABLE_NONE;
            tmpCfg.ScBarVisuable = VISUABLE_NONE;
        }

        if (tmpCfg.BottomVisuable == VISUABLE_NORMAL)
        {
            pListBox->Bottom.pBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_BOTTOM, &imgLen);
            if (pListBox->Bottom.pBgImg)
            {
                AKBmpGetInfo(pListBox->Bottom.pBgImg, &Width, &Height, &Deep);
                BorderBottomHeight = Height;
            }
            else
            {
                BorderBottomHeight = 0;
                tmpCfg.BottomVisuable = VISUABLE_NONE;
            }
        }
        else if (tmpCfg.BottomVisuable == VISUABLE_LINE)
        {
            BorderBottomHeight = 1;
        }
        else
        {
            BorderBottomHeight = 0;
        }

        if (tmpCfg.LeftVisuable == VISUABLE_NORMAL)
        {
            pListBox->Left.pBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_LEFT, &imgLen);
            if (pListBox->Left.pBgImg)
            {
                AKBmpGetInfo(pListBox->Left.pBgImg, &Width, &Height, &Deep);
                BorderLeftWidth = Width;
            }
            else
            {
                BorderLeftWidth = 0;
                tmpCfg.LeftVisuable = VISUABLE_NONE;
            }
        }
        else if (tmpCfg.LeftVisuable == VISUABLE_LINE)
        {
            BorderLeftWidth = 1;
        }
        else
        {
            BorderLeftWidth = 0;
        }
        
        if (tmpCfg.ScBarVisuable == VISUABLE_NORMAL)
        {
            pListBox->ScBar.pBarImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_BAR, &imgLen);
            pListBox->ScBar.pBarUpImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_BARUP, &imgLen);
            pListBox->ScBar.pBarDownImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_BARDOWN, &imgLen);
            pListBox->ScBar.pBarBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_BARBG, &imgLen);
            if (pListBox->ScBar.pBarImg)
            {
                AKBmpGetInfo(pListBox->ScBar.pBarImg, &Width, &Height, &Deep);
				ScbarWidth = Width;
            }
            else
            {
                ScbarWidth = 0;
                tmpCfg.ScBarVisuable = VISUABLE_NONE;
            }
            
            if (pListBox->ScBar.pBarBgImg)
            {
                AKBmpGetInfo(pListBox->ScBar.pBarBgImg, &Width, &Height, &Deep);
                ScbarBgWidth = Width;
            }
            else
            {
                ScbarBgWidth = 0;
                tmpCfg.ScBarVisuable = VISUABLE_NONE;
            }
            
        }
        else if (tmpCfg.ScBarVisuable == VISUABLE_LINE)
        {
            ScbarWidth = 1;
            ScbarBgWidth = 1;
        }
        else
        {
            ScbarWidth = 0;
            ScbarBgWidth = 0;
        }
        
        if (tmpCfg.ExitVisuable == VISUABLE_NORMAL)
        {
            pListBox->Exit.pImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_EXIT, &imgLen);
            if (pListBox->Exit.pImg)
            {
                AKBmpGetInfo(pListBox->Exit.pImg, &Width, &Height, &Deep);
            }
            else
            {
                tmpCfg.LeftVisuable = VISUABLE_NONE;
            }
        }
        
        
        
        /************************the following is variable set********************************/
        //title set
        pListBox->Title.VisuableMode = tmpCfg.TitleVisuable;
        pListBox->Title.pText = AK_NULL;
        pListBox->Title.Font = CURRENT_FONT_SIZE;
        pListBox->Title.Rect.left = pListBox->Rect.left;
        pListBox->Title.Rect.top = pListBox->Rect.top;
        pListBox->Title.Rect.width = pListBox->Rect.width;
        pListBox->Title.Rect.height = TitleHeitht;
        pListBox->Title.FontColor = COLOR_BLACK;
        pListBox->Title.BgColor = g_Graph.TransColor;
        //ListBox_Printf("TITLE___", pListBox->Title.Rect);
        
        //set border
        pListBox->Left.VisuableMode = tmpCfg.LeftVisuable;
        pListBox->Left.Rect.left = pListBox->Rect.left;
        pListBox->Left.Rect.top = pListBox->Rect.top + pListBox->Title.Rect.height;
        pListBox->Left.Rect.width = BorderLeftWidth;
        pListBox->Left.Rect.height = pListBox->Rect.height - TitleHeitht - BorderBottomHeight;
        //ListBox_Printf("LEFT___", pListBox->Left.Rect);
        
        pListBox->Bottom.VisuableMode = tmpCfg.BottomVisuable;
        pListBox->Bottom.Rect.left = pListBox->Rect.left;
        pListBox->Bottom.Rect.top = pListBox->Rect.top + pListBox->Rect.height - BorderBottomHeight;
        pListBox->Bottom.Rect.width = pListBox->Rect.width;
        pListBox->Bottom.Rect.height = BorderBottomHeight;
        //ListBox_Printf("BOTTOM___", pListBox->Bottom.Rect);
        
        //item set
        pListBox->Item.VisuableMode = tmpCfg.ItemVisuable;
#if (LCD_CONFIG_WIDTH == 800)		
        pListBox->Item.Height = 35;
#elif (LCD_CONFIG_WIDTH == 480)
		pListBox->Item.Height = 20;
#elif (LCD_CONFIG_WIDTH == 320)
		pListBox->Item.Height = 20;
#else
#error "LCD no match!"
#endif
        pListBox->Item.Rect.left = pListBox->Rect.left + BorderLeftWidth;
        pListBox->Item.Rect.top = pListBox->Rect.top + pListBox->Title.Rect.height;
        pListBox->Item.Rect.width = pListBox->Rect.width - BorderLeftWidth - ScbarBgWidth;
        pListBox->Item.Rect.height = pListBox->Rect.height - pListBox->Title.Rect.height - pListBox->Bottom.Rect.height;

        //the item number in a page should be calculated at here 2008-12-9
        pCfg->PageItemNum = (pCfg->Rect.height \
                            - pListBox->Title.Rect.height \
                            - pListBox->Bottom.Rect.height) \
                            / pListBox->Item.Height;

        if (tmpCfg.pListItem)
        {
            pListBox->Item.pListItem = tmpCfg.pListItem;
            pListBox->Item.bOutListItem = AK_TRUE;
        }
        else
        {
            pListBox->Item.bOutListItem = AK_FALSE;
            pListBox->Item.pListItem = &(pListBox->Item.ListItem);
            ListItem_Init(pListBox->Item.pListItem, pCfg->PageItemNum);
        }
        
        pListBox->Item.Font = CURRENT_FONT_SIZE;
        pListBox->Item.FontOffset = 5;
        pListBox->Item.FontColor = COLOR_BLACK;
        pListBox->Item.BgColor = g_Graph.TransColor;
        pListBox->Item.FocusFontColor = COLOR_BLUE;
        pListBox->Item.pFocusBgImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LISTBOX_ITEMBG, &imgLen);;
        //ListBox_Printf("ITEM___", pListBox->Item.Rect);
        
        //pListBox->ScBar.Rect;
        pListBox->ScBar.VisuableMode = tmpCfg.ScBarVisuable;
        pListBox->ScBar.DispMode = tmpCfg.ScBarMode;
        pListBox->ScBar.BarUpRect.left = pListBox->Rect.left + pListBox->Rect.width - ScbarBgWidth;
        pListBox->ScBar.BarUpRect.top = pListBox->Item.Rect.top;
        if (pListBox->ScBar.VisuableMode == VISUABLE_NORMAL)
        {
            AKBmpGetInfo(pListBox->ScBar.pBarUpImg, &Width, &Height, &Deep);
            pListBox->ScBar.BarUpRect.width = Width;
            pListBox->ScBar.BarUpRect.height = Height;
        }
        //ListBox_Printf("SCBARUP___", pListBox->ScBar.BarUpRect);

        if (pListBox->ScBar.VisuableMode == VISUABLE_NORMAL)
        {
            AKBmpGetInfo(pListBox->ScBar.pBarDownImg, &Width, &Height, &Deep);
            pListBox->ScBar.BarDownRect.width = Width;
            pListBox->ScBar.BarDownRect.height = Height;
        }

        pListBox->ScBar.BarDownRect.left = pListBox->Rect.left + pListBox->Rect.width - ScbarBgWidth;
        pListBox->ScBar.BarDownRect.top = pListBox->Rect.top + pListBox->Rect.height - BorderBottomHeight - pListBox->ScBar.BarDownRect.height;
        //ListBox_Printf("SCBARDOWN___", pListBox->ScBar.BarDownRect);
        
        pListBox->ScBar.BarRect.left = pListBox->Rect.left + pListBox->Rect.width - ScbarBgWidth + (ScbarBgWidth - ScbarWidth)/2;
        pListBox->ScBar.BarRect.top = pListBox->Rect.top + pListBox->Title.Rect.height + pListBox->ScBar.BarUpRect.height;
        pListBox->ScBar.BarRect.width = ScbarWidth;
        pListBox->ScBar.BarRect.height = pListBox->Item.Rect.height - pListBox->ScBar.BarUpRect.height \
                                               - pListBox->ScBar.BarDownRect.height;
        
        pListBox->ScBar.BarRunRect = pListBox->ScBar.BarRect;        
        //ListBox_Printf("SCBAR___", pListBox->ScBar.BarRect);                                       
            
        pListBox->ScBar.BarBgRect.left = pListBox->Rect.left + pListBox->Rect.width - ScbarBgWidth;
        pListBox->ScBar.BarBgRect.top = pListBox->Item.Rect.top;
        pListBox->ScBar.BarBgRect.width = ScbarBgWidth;
        pListBox->ScBar.BarBgRect.height = pListBox->Item.Rect.height;
        //ListBox_Printf("SCBARBG___", pListBox->ScBar.BarBgRect);
        
        
        pListBox->Exit.VisuableMode = tmpCfg.ExitVisuable;
        if (pListBox->Exit.VisuableMode == VISUABLE_NORMAL)
        {
            AKBmpGetInfo(pListBox->Exit.pImg, &Width, &Height, &Deep);
            pListBox->Exit.Rect.left = pListBox->Title.Rect.left + pListBox->Title.Rect.width - Width;
            pListBox->Exit.Rect.top = pListBox->Title.Rect.top;
            pListBox->Exit.Rect.width = Width;
            pListBox->Exit.Rect.height = Height;
        }
        
        
        pListBox->ScrollTimer = ERROR_TIMER;
        if (tmpCfg.ItemTextScroll)
        {
            ListBox_ScrollStart(pListBox);
        }
        
        pListBox->RefreshFlag = LISTBOX_REFRESH_ALL;
    }
    else
    {
        ret = AK_FALSE;
    }
	
    return ret;
}


T_BOOL ListBox_Show(T_LISTBOX *pListBox, T_BOOL bRefresh)
{
    T_BOOL ret = AK_TRUE;
    
    if (pListBox)
    {
        if (pListBox->RefreshFlag != LISTBOX_REFRESH_NONE)
        {
            if (pListBox->RefreshFlag & LISTBOX_REFRESH_BG)
            {
                ListBox_RefreshBg(pListBox);
            }
    
            if (pListBox->RefreshFlag & LISTBOX_REFRESH_TITLE)
            {
                ListBox_RefreshTitle(pListBox);
                ListBox_RefreshExit(pListBox);
            }
    
            if (pListBox->RefreshFlag & LISTBOX_REFRESH_ITEM)
            {
                ListBox_RefreshItems(pListBox);
            }
    
            if (pListBox->RefreshFlag & LISTBOX_REFRESH_SCBAR)
            {
                ListBox_RefreshScBar(pListBox);
            }
    
            if (pListBox->RefreshFlag & LISTBOX_REFRESH_SCROLL_TXT)
            {
                ListBox_RefreshItemScrollText(pListBox);
            }
    
            if (pListBox->RefreshFlag & LISTBOX_REFRESH_LEFT)
            {
                ListBox_RefreshLeft(pListBox);
            }
    
            if (pListBox->RefreshFlag & LISTBOX_REFRESH_BOTTOM)
            {
                ListBox_RefreshBottom(pListBox);
            }
            
            if (pListBox->RefreshFlag & LISTBOX_REFRESH_EXIT)
            {
                ListBox_RefreshExit(pListBox);
            }

            if (bRefresh)
            {
                Fwl_InvalidateRect( pListBox->Rect.left, pListBox->Rect.top, pListBox->Rect.width, pListBox->Rect.height);
            }
            
            pListBox->RefreshFlag = LISTBOX_REFRESH_NONE;
        }
    }
    else
    {
        ret = AK_FALSE;
    }

    return ret;
}


T_BOOL ListBox_Free(T_LISTBOX *pListBox)
{
    T_BOOL ret = AK_TRUE;

    if (pListBox)
    {  
        if (pListBox->Title.pText)
        {
            Fwl_Free(pListBox->Title.pText);
            pListBox->Title.pText = AK_NULL;
        }
        
        ListBox_ScrollStop(pListBox);
        
        if (!pListBox->Item.bOutListItem)
        {
            ListItem_Free(&pListBox->Item.ListItem);
        }
    }
    else
    {
        ret = AK_FALSE;
    }
  
    return ret;
}

T_BOOL ListBox_ScrollStart(T_LISTBOX *pListBox)
{
    T_BOOL ret = AK_TRUE;

    if (pListBox)
    {
        if (pListBox->ScrollTimer != ERROR_TIMER)
        {
            Fwl_StopTimer(pListBox->ScrollTimer);
            pListBox->ScrollTimer = ERROR_TIMER;
        }
        
        pListBox->ScrollOffset = 0;
        pListBox->ScrollTimer = Fwl_SetTimerMilliSecond(400, AK_TRUE);
    }
    else
    {
        ret = AK_FALSE;
    }
  
    return ret;
}


T_BOOL ListBox_ScrollStop(T_LISTBOX *pListBox)
{
    T_BOOL ret = AK_TRUE;

    if (pListBox)
    {
        if (pListBox->ScrollTimer != ERROR_TIMER)
        {
            Fwl_StopTimer(pListBox->ScrollTimer);
            pListBox->ScrollTimer = ERROR_TIMER;
        }
        
        pListBox->ScrollOffset = 0;
    }
    else
    {
        ret = AK_FALSE;
    }
  
    return ret;
}




T_eBACK_STATE ListBox_Handle(T_LISTBOX *pListBox, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_MMI_KEYPAD        phyKey;
    T_eBACK_STATE       ret = eStay;
 
    if (pListBox)
    {
    
        if ((Event==M_EVT_USER_KEY) && (PRESS_SHORT==(T_PRESS_TYPE)pParam->c.Param2))
        {
            phyKey.keyID = (T_eKEY_ID)pParam->c.Param1;
            phyKey.pressType = (T_U8)pParam->c.Param2;

            switch (phyKey.keyID)
            {
                case kbUP:
                    ListItem_MoveUp(pListBox->Item.pListItem, AK_TRUE);
                    pListBox->ScrollOffset = 0;
                    ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_ITEM);
                    ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_SCBAR);
                    break;
                    
                case kbDOWN:
                    ListItem_MoveDown(pListBox->Item.pListItem, AK_TRUE);
                    pListBox->ScrollOffset = 0;
                    ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_ITEM);
                    ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_SCBAR);
                    break;
                
                case kbMENU:
                    ret = eMenu;
                    break;
                    
                case kbOK:
                    ListItem_Enter(pListBox->Item.pListItem);
                    ret = eNext;
                    break;
                                
                case kbLEFT:
                    break;
                
                case kbRIGHT:
                    break;
                
                case kbCLEAR:
                    ret = eReturn;
                    break;
                
                default:
                    break;
            }            
        }
        else if (Event == VME_EVT_TIMER)
        {
            if (pParam->w.Param1 == (T_U32)pListBox->ScrollTimer)
            {
                ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_SCROLL_TXT);
            }
        }
        else if (Event == M_EVT_TOUCH_SCREEN)
        {
            T_POS x = 0;
            T_POS y = 0;
            T_LIXTBOX_AREA Area;
            T_U16 touch_event;
            T_U32 tmp_index, page_index, focus_index;
            
            touch_event = pParam->s.Param1;
            
            if (touch_event == eTOUCHSCR_UP)
            {
                x = (T_POS)pParam->s.Param2;
                y = (T_POS)pParam->s.Param3;
                Area = ListBox_GetTouchArea(pListBox, x, y);
                switch (Area)
                {
                    case LISTBOX_AREA_EXIT:
                        ret = eReturn;
                        break;
                    
                    case LISTBOX_AREA_BARUP:
                        ListItem_MoveUp(pListBox->Item.pListItem, AK_FALSE);
                        ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_ITEM);
                        ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_SCBAR);
                        break;
                        
                    case LISTBOX_AREA_BARDOWN:
                        ListItem_MoveDown(pListBox->Item.pListItem, AK_FALSE);
                        ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_ITEM);
                        ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_SCBAR);
                        break;
                        
                    case LISTBOX_AREA_PAGEUP:
                        ListItem_PageUp(pListBox->Item.pListItem);
                        ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_ITEM);
                        ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_SCBAR);
                        break;
                    
                    case LISTBOX_AREA_PAGEDOWN:
                        ListItem_PageDown(pListBox->Item.pListItem);
                        ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_ITEM);
                        ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_SCBAR);
                        break;
                    
                    case LISTBOX_AREA_ITEM:
                        page_index = ListBox_GetItemPageIndexByPos(pListBox, x, y);
                        
                        focus_index = ListItem_GetFocusIndex(pListBox->Item.pListItem);
                        if (page_index != INVALID_INDEX)
                        {
                            ListItem_SetPageIndex(pListBox->Item.pListItem, page_index);
                            tmp_index = pListBox->Item.pListItem->PageFirstItemIndex + page_index;
                            if (tmp_index != focus_index)
                            {
                                ListItem_SetFocusIndex(pListBox->Item.pListItem, tmp_index);
                                
                                ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_ITEM);
                            }
                            else
                            {
                                ListItem_Enter(pListBox->Item.pListItem);
                                ret = eNext;
                            }
                        }
                        break;
                    
                    default:
                        break; 
                }
            }
        }
    }

    return ret;
}


T_BOOL ListBox_SetItemText(T_LISTBOX *pListBox, T_U32 Index, T_U16 *pText)
{
    T_BOOL ret = AK_TRUE;
    
    if (pListBox)
    {
        ret = ListItem_SetText(pListBox->Item.pListItem, Index, pText);
    }
    else
    {
        ret = AK_FALSE;
    }
    
    return ret;
}


T_BOOL ListBox_SetListItem(T_LISTBOX *pListBox, T_LIST_ITEM *pListItem)
{
    T_BOOL ret = AK_TRUE;
    
    if (pListBox)
    {
        pListBox->Item.pListItem = pListItem;
    }
    else
    {
        ret = AK_FALSE;
    }
    
    return ret;
}


T_BOOL ListBox_SetPageItemNum(T_LISTBOX *pListBox, T_U32 Num)
{
    T_BOOL ret = AK_TRUE;
    
    if (pListBox)
    {
        ret = ListItem_SetPageItemNum(pListBox->Item.pListItem, Num);
    }
    else
    {
        ret = AK_FALSE;
    }
    
    return ret;
}


T_BOOL ListBox_SetTitleText(T_LISTBOX *pListBox, const T_U16 *pText)
{
    T_BOOL ret = AK_TRUE;
    T_U32  txt_len1, txt_len2;

    if (pListBox)
    {
        if (pListBox->Title.pText)
        {
            txt_len1 = (Utl_UStrLen(pListBox->Title.pText)+1)<<1;
            txt_len2 = (Utl_UStrLen(pText)+1)<<1;

            if (txt_len1 < txt_len2)
            {
                Fwl_Free(pListBox->Title.pText);
                pListBox->Title.pText = Fwl_Malloc(txt_len2);
            }
            
            if (pListBox->Title.pText)
            {
                Utl_UStrCpy(pListBox->Title.pText, pText);
            }
        }
        else
        {
            txt_len1 = (Utl_UStrLen(pText)+1)<<1;
            pListBox->Title.pText = Fwl_Malloc(txt_len1);
            if (pListBox->Title.pText)
            {
                Utl_UStrCpy(pListBox->Title.pText, pText);
            }
        }
        
        ListBox_SetRefresh(pListBox, LISTBOX_REFRESH_TITLE);
    }
    else
    {
        ret = AK_FALSE;
    }

    return ret;
}

T_BOOL ListBox_AppendItem(T_LISTBOX *pListBox, T_LIST_ELEMENT *pItem)
{
    T_BOOL ret = AK_TRUE;
   
    if (pListBox && pItem)
    {
        ret = ListItem_Append(pListBox->Item.pListItem, pItem);
    }
    else
    {
        ret = AK_FALSE;
    }

    return ret;
}


T_BOOL ListBox_InsertItem(T_LISTBOX *pListBox, T_U32 Index, T_LIST_ELEMENT *pItem)
{
    T_BOOL ret = AK_TRUE;

    if (pListBox)
    {
        ret = ListItem_Insert(pListBox->Item.pListItem, Index, pItem);
    }
    else
    {
        ret = AK_FALSE;
    }

    return ret;
}


T_BOOL ListBox_DeleteItem(T_LISTBOX *pListBox, T_U32 Index)
{
    T_BOOL ret = AK_TRUE;

    if (pListBox)
    {
        ret = ListItem_Delete(pListBox->Item.pListItem, Index);
    }
    else
    {
        ret = AK_FALSE;
    }
 
    return ret;
}



T_BOOL ListBox_DeleteAllItem(T_LISTBOX *pListBox)
{
    T_BOOL ret = AK_TRUE;

    if (pListBox)
    {
        ret = ListItem_DeleteAll(pListBox->Item.pListItem);
    }
    else
    {
        ret = AK_FALSE;
    }
 
    return ret;
}



T_BOOL ListBox_SetRefresh(T_LISTBOX *pListBox, T_U32 Flag)
{
    T_BOOL ret = AK_TRUE;
    
    if (pListBox)
    {
        if (Flag)
        {
            pListBox->RefreshFlag |= Flag;
        }
        else
        {
            pListBox->RefreshFlag = Flag;
        }
    }
    else
    {
        ret = AK_FALSE;
    }
    
    return ret;
}



T_pVOID ListBox_GetItemData(T_LISTBOX *pListBox, T_U32 Index)
{
    T_LIST_ELEMENT *pItem = AK_NULL;

    if (pListBox)
    {   
        pItem = ListItem_GetItemByIndex(pListBox->Item.pListItem, Index);
        if(pItem)
        {
            return (T_pVOID)pItem->pData;
        }
    }
    
    return (T_pVOID)AK_NULL;
}


const T_U16 *ListBox_GetItemText(T_LISTBOX *pListBox, T_U16 Index)
{
    T_LIST_ELEMENT *pItem = AK_NULL;

    if (pListBox)
    {
        pItem = ListItem_GetItemByIndex(pListBox->Item.pListItem, Index);
        if(pItem)
        {
            return pItem->pText;
        }
    }
    
    return AK_NULL;
}


T_U32 ListBox_GetFocusIndex(T_LISTBOX *pListBox)
{
    T_U32 Index = INVALID_INDEX;

    if (pListBox)
    {
        Index = ListItem_GetFocusIndex(pListBox->Item.pListItem);
    }

    return Index;
}

T_S32 ListBox_GetRect(T_LISTBOX *pListBox, T_RECT *pRect)
{
    if ((AK_NULL!=pListBox) && (AK_NULL!=pRect))
    {
        memcpy (pRect, &pListBox->Rect, sizeof(T_RECT));
        return AK_SUCCESS;
    }
    else
    {
        return AK_EFAILED;
    }
}

