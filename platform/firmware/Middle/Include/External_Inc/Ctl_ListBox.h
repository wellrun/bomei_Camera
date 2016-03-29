#ifndef __CTL_LISTBOX_H__
#define __CTL_LISTBOX_H__

#include "anyka_types.h"
#include "Ctl_ScrollBar.h"
#include "Ctl_ListItem.h"

#define     LISTBOX_REFRESH_NONE        (0)
#define     LISTBOX_REFRESH_TITLE       (1<<0)
#define     LISTBOX_REFRESH_ITEM        (1<<1)
#define     LISTBOX_REFRESH_SCROLL_TXT  (1<<2)
#define     LISTBOX_REFRESH_SCBAR       (1<<3)
#define     LISTBOX_REFRESH_BG          (1<<4)
#define     LISTBOX_REFRESH_LEFT        (1<<5)
#define     LISTBOX_REFRESH_BOTTOM      (1<<6)
#define     LISTBOX_REFRESH_EXIT        (1<<7)
#define     LISTBOX_REFRESH_ALL         (0xffff)


typedef enum
{
    LISTBOX_AREA_NONE = 0,
    LISTBOX_AREA_TITLE,
    LISTBOX_AREA_EXIT,
    LISTBOX_AREA_LEFT,
    LISTBOX_AREA_BOTTOM,
    LISTBOX_AREA_BAR,
    LISTBOX_AREA_BARUP,
    LISTBOX_AREA_BARDOWN,
    LISTBOX_AREA_PAGEUP,
    LISTBOX_AREA_PAGEDOWN,
    LISTBOX_AREA_ITEM
}T_LIXTBOX_AREA;


typedef enum
{
    HORIZONTAL = 0,
    VERTICAL
}T_LISTBOX_SCBAR_MODE;

typedef enum
{
    VISUABLE_NORMAL = 0,
    VISUABLE_LINE,
    VISUABLE_NONE
}T_VISUABLE_MODE;

typedef struct
{
    T_VISUABLE_MODE     VisuableMode;    
    T_pCDATA            pBgImg;
    T_RECT              Rect;
}T_LISTBOX_BORDER;

typedef struct
{
    T_VISUABLE_MODE         VisuableMode;
    T_pCDATA                pImg;      //up arrow
    T_RECT                  Rect;
}T_LISTBOX_EXIT;


typedef struct
{
    T_VISUABLE_MODE         VisuableMode;    
    
    T_pCDATA                pBarUpImg;      //up arrow
    T_RECT                  BarUpRect;

    T_pCDATA                pBarDownImg;    //down arrow
    T_RECT                  BarDownRect;

    T_pCDATA                pBarImg;
    T_RECT                  BarRect;        //上下可移动的范围
    T_RECT                  BarRunRect;     //实际滚动条显示区域
    
    T_pCDATA                pBarBgImg;
    T_RECT                  BarBgRect;
    T_LISTBOX_SCBAR_MODE    DispMode;
}T_LISTBOX_SCBAR;

typedef struct
{
    T_VISUABLE_MODE     VisuableMode;
    T_pCDATA            pBgImg;
    T_RECT              Rect;

    T_U16               *pText;
    T_FONT              Font;
    T_COLOR             FontColor;
    T_COLOR             BgColor;
}T_LISTBOX_TITLE;


typedef struct
{
    T_VISUABLE_MODE     VisuableMode;
    T_U16               Height;
    T_RECT              Rect;

    T_BOOL              bOutListItem;
    T_LIST_ITEM         ListItem;
    T_LIST_ITEM         *pListItem;
    T_FONT              Font;
    T_U16               FontOffset;
    T_COLOR             FontColor;
    T_COLOR             BgColor;
    T_COLOR             FocusFontColor;
    T_pCDATA            pBgImg;
    T_pCDATA            pFocusBgImg;
}T_LISTBOX_ITEM;


typedef struct
{
    T_RECT              Rect;
    T_LISTBOX_TITLE     Title;
    T_LISTBOX_BORDER    Left;
    T_LISTBOX_BORDER    Bottom;
    T_LISTBOX_ITEM      Item;
    T_LISTBOX_SCBAR     ScBar;
    T_LISTBOX_EXIT      Exit;

    T_U16               ScrollOffset;
    T_TIMER             ScrollTimer;
    T_U32               RefreshFlag;
}T_LISTBOX;


typedef struct
{
    T_VISUABLE_MODE         TitleVisuable;
    T_VISUABLE_MODE         ScBarVisuable;
    T_VISUABLE_MODE         ItemVisuable;
    T_VISUABLE_MODE         LeftVisuable;
    T_VISUABLE_MODE         BottomVisuable;
    T_VISUABLE_MODE         ExitVisuable;
    
    //T_TEXT_ALLIGN         TitleTextAllign;
    //T_TEXT_ALLIGN         ItemTextAllign;
    //T_TEXT_ALLIGN         ItemTextAllign;
    //T_TEXT_ALLIGN         ItemTextAllign;
    T_RECT                  Rect;
    
    T_LIST_ITEM             *pListItem;
    T_U32                   PageItemNum;
    
    T_LISTBOX_SCBAR_MODE    ScBarMode;
    T_BOOL                  ItemTextScroll;
    //T_BOOL                  TitleTextScroll;
}T_LISTBOX_CFG;



T_BOOL ListBox_Init(T_LISTBOX *pListBox, T_LISTBOX_CFG *pCfg);

T_BOOL ListBox_Show(T_LISTBOX *pListBox, T_BOOL bRefresh);

T_BOOL ListBox_Free(T_LISTBOX *pListBox);

T_BOOL ListBox_ScrollStart(T_LISTBOX *pListBox);

T_BOOL ListBox_ScrollStop(T_LISTBOX *pListBox);

T_eBACK_STATE ListBox_Handle(T_LISTBOX *pListBox, T_EVT_CODE Event, T_EVT_PARAM *pParam);

T_BOOL ListBox_SetItemText(T_LISTBOX *pListBox, T_U32 Index, T_U16 *pText);

T_BOOL ListBox_SetListItem(T_LISTBOX *pListBox, T_LIST_ITEM *pListItem);

T_BOOL ListBox_SetPageItemNum(T_LISTBOX *pListBox, T_U32 Num);

T_BOOL ListBox_SetTitleText(T_LISTBOX *pListBox, const T_U16 *pText);

T_BOOL ListBox_InsertItem(T_LISTBOX *pListBox, T_U32 Index, T_LIST_ELEMENT *pItem);

T_BOOL ListBox_AppendItem(T_LISTBOX *pListBox, T_LIST_ELEMENT *pItem);

T_BOOL ListBox_DeleteItem(T_LISTBOX *pListBox, T_U32 Index);

T_BOOL ListBox_DeleteAllItem(T_LISTBOX *pListBox);

T_BOOL ListBox_SetRefresh(T_LISTBOX *pListBox, T_U32 Flag);

T_pVOID ListBox_GetItemData(T_LISTBOX *pListBox, T_U32 Index);

T_U16 *ListBox_GetText(T_LISTBOX *pListBox, T_U32 Index);

T_U32 ListBox_GetFocusIndex(T_LISTBOX *pListBox);

T_S32 ListBox_GetRect(T_LISTBOX *pListBox, T_RECT *pRect);

const T_U16 *ListBox_GetItemText(T_LISTBOX *pListBox, T_U16 Index);

#endif

