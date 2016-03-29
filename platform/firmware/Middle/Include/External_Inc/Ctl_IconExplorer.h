
/**************************************************************
*
* Copyrights (C) 2006, ANYKA Software Inc.
* All rights reserced.
*
* File name: Ctl_IconExplorer.h
* File flag: Icon Explorer
* File description:
*
* Revision: 1.00
* Author: Guanghua Zhang
* Date: 2006-01-06
*
****************************************************************/

#ifndef __CTL_ICONEXPLORER_H__
#define __CTL_ICONEXPLORER_H__

#include "anyka_types.h"
#include "Ctl_ScrollBar.h"
#include "gbl_macrodef.h"
#include "Ctl_SlipMgr.h"

#define ICONEXPLORER_ITEM_QTYMAX            1003                    // 0 is no limit
#define ICONEXPLORER_ITEM_TRANSCOLOR        g_Graph.TransColor      // item transparent color default
#define ICONEXPLORER_TITLE_BACKCOLOR        COLOR_BLUE              // title back color default
#define ICONEXPLORER_TITLE_TEXTCOLOR        COLOR_BLACK             // title text color default
#define ICONEXPLORER_ITEM_FRAMEWIDTH        2                       // item frame width default
#define ICONEXPLORER_ITEM_BACKCOLOR         COLOR_WHITE             // item back color default
#define ICONEXPLORER_ITEM_TEXTCOLOR         COLOR_BLUE              // item text color default
#define ICONEXPLORER_ITEM_FOCUSBACKCOLOR    COLOR_BLUE              // item focus back color default
#define ICONEXPLORER_ITEM_FOCUSTEXTCOLOR    COLOR_WHITE             // item focus text color default
#define ICONEXPLORER_ITEM_SMALLICON_WMAX    (T_U32)(Fwl_GetLcdWidth()/3)      // item small icon width max
#define ICONEXPLORER_ITEM_SMALLICON_HMIN    (T_U32)FONT_HEIGHT_DEFAULT//(g_Font.CHEIGHT) // item small icon height min
#define ICONEXPLORER_ITEM_SMALLICON_WIDTH   12                      // item small icon width default
#define ICONEXPLORER_ITEM_SMALLICON_HEIGHT  (T_U32)FONT_HEIGHT_DEFAULT//(g_Font.CHEIGHT) // item small icon height default
#define ICONEXPLORER_ITEM_SMALLINTERVAL_MAX 16                      // item small style interval max
#define ICONEXPLORER_ITEM_SMALLINTERVAL     1                       // item small style interval default
#define ICONEXPLORER_ITEM_LARGEICON_WMAX    (T_U32)(Fwl_GetLcdWidth()/2)      // item large icon width max
#define ICONEXPLORER_ITEM_LARGEICON_HMAX    (T_U32)(Fwl_GetLcdHeight()/2)     // item large icon height max
#define ICONEXPLORER_ITEM_LARGETEXT_HMAX    (T_U32)(Fwl_GetLcdHeight()/4)     // item large text height max
#define ICONEXPLORER_ITEM_LARGETEXT_HMIN    (T_U32)FONT_HEIGHT_DEFAULT//(g_Font.CHEIGHT) // item large text height min
#define ICONEXPLORER_ITEM_LARGEICON_WIDTH   24                      // item large icon width default
#define ICONEXPLORER_ITEM_LARGEICON_HEIGHT  24                      // item large icon height default
#define ICONEXPLORER_ITEM_LARGETEXT_HEIGHT  (T_U32)FONT_HEIGHT_DEFAULT//(g_Font.CHEIGHT) // item large text height default
#define ICONEXPLORER_ITEM_LARGEINTERVAL_MAX 32                      // item large style interval max
#define ICONEXPLORER_ITEM_LARGEINTERVAL     2                       // item large style interval default
#define ICONEXPLORER_ITEM_NONETEXT_HMIN     (T_U32)FONT_HEIGHT_DEFAULT//(g_Font.CHEIGHT) // item none text height min
#define ICONEXPLORER_ITEM_NONETEXT_HEIGHT   (T_U32)FONT_HEIGHT_DEFAULT//(g_Font.CHEIGHT) // item none text height default
#define ICONEXPLORER_ITEM_NONEINTERVAL_MAX  16                      // item none style interval max
#define ICONEXPLORER_ITEM_NONEINTERVAL      1                       // item none style interval default
#define ICONEXPLORER_ITEM_ID_ERROR          0xffffffff              // item id error
#define ICONEXPLORER_ITEM_SCROLLBAR_WIDTH   8                       // item scroll bar width default
#define ICONEXPLORER_ITEMOPTION_STRLEN      16                      // item option text string length

#define ICONEXPLORER_ITEMOPTION_ID_ERROR    0xff                    // item option id error
// style
// title style: [0, 15]
#define ICONEXPLORER_TITLE_ON               0x00000001              // show title
#define ICONEXPLORER_TITLE_TEXT_HCENTER     0x00000002              // title text horizontal align center
#define ICONEXPLORER_TITLE_TEXT_LEFT        0x00000004              // title text horizontal align left
#define ICONEXPLORER_TITLE_TEXT_RIGHT       0x00000008              // title text horizontal align right
#define ICONEXPLORER_TITLE_TEXT_HALIGN      0x0000000e              // title text horizontal align mask
#define ICONEXPLORER_TITLE_TEXT_VCENTER     0x00000010              // title text vertical align center
#define ICONEXPLORER_TITLE_TEXT_UP          0x00000020              // title text vertical align up
#define ICONEXPLORER_TITLE_TEXT_DOWN        0x00000040              // title text vertical align down
#define ICONEXPLORER_TITLE_TEXT_VALIGN      0x00000070              // title text vertical align mask
// item style: [16, 31]
#define ICONEXPLORER_ITEM_FRAME             0x00010000              // show item frame

// refresh flag
#define ICONEXPLORER_REFRESH_ALL            0xffffffff              // refresh all
#define ICONEXPLORER_REFRESH_NONE           0x00000000              // no refresh
#define ICONEXPLORER_REFRESH_TITLE          0x00000001              // refresh title
#define ICONEXPLORER_REFRESH_ITEM           0x00000002              // refresh item
#define ICONEXPLORER_REFRESH_FOCUS          0x00000004              // refresh focus

// sort callback function type by id
typedef T_BOOL (*T_fICONEXPLORER_SORT_ID_CALLBACK)(T_U32 Id1, T_U32 Id2);
// sort callback function type by content
typedef T_BOOL (*T_fICONEXPLORER_SORT_CONTENT_CALLBACK)(T_pCVOID pObject, T_VOID *pContent1, T_VOID *pContent2);
// get item list callback function type
typedef T_BOOL (*T_fICONEXPLORER_LIST_CALLBACK)(T_pCVOID pObject);

// define icon style
typedef enum {
    ICONEXPLORER_SMALLICON = 0,
    ICONEXPLORER_LARGEICON,
    ICONEXPLORER_NONEICON,
    ICONEXPLORER_ICONSTYLE_NUM
} T_ICONEXPLORER_ICONSTYLE;

// define sort mode
typedef enum {
    ICONEXPLORER_SORT_ID = 0,
    ICONEXPLORER_SORT_CONTENT,
    ICONEXPLORER_SORT_NUM
} T_ICONEXPLORER_SORTMODE;

// define focus move direction
typedef enum {
    ICONEXPLORER_DIRECTION_LEFT = 0,
    ICONEXPLORER_DIRECTION_RIGHT,
    ICONEXPLORER_DIRECTION_UP,
    ICONEXPLORER_DIRECTION_DOWN,
    ICONEXPLORER_DIRECTION_NUM
} T_ICONEXPLORER_DIRECTION;

typedef enum {
    ICONEXPLORER_OPTION_NONE = 0,
    ICONEXPLORER_OPTION_NEXT,
    ICONEXPLORER_OPTION_LIST,
    ICONEXPLORER_OPTION_NUM
} T_ICONEXPLORER_OPTION_TYPE;

typedef enum {
    ICONEXPLORER_CHANGE_NONE = 0,
    ICONEXPLORER_CHANGE_ADD = 0x1,	//bit 0
    ICONEXPLORER_CHANGE_DEL = 0x2,	//bit 1
} T_ICONEXPLORER_CHANGE_FLAG;


// icon explorer item option struct
typedef struct _ICONEXPLORER_OPTION {
    T_U8                        Id;                     // option id
    T_U16                        Text[ICONEXPLORER_ITEMOPTION_STRLEN+1];   // option text
    struct _ICONEXPLORER_OPTION *pNext;                 // next option point
} T_ICONEXPLORER_OPTION;

// icon explorer item struct
typedef struct _ICONEXPLORER_ITEM {
    T_U32                       Id;                     // item id
    T_VOID                      *pContent;              // item content point, malloc it
    T_S32                       contentLen;
    T_U16                       *pText;                 // item text point, malloc it
    T_U8                        *pSmallIcon;            // item small icon point, AK bmp data
    T_U8                        *pLargeIcon;            // item large icon point, AK bmp data
    T_ICONEXPLORER_OPTION_TYPE  OptionType;             // item option type
    T_U8                        *pOptionValue;          // item option value point
    T_ICONEXPLORER_OPTION       *pOptionHead;           // item option head point
    T_ICONEXPLORER_OPTION       *pOptionFocus;          // item option focuss point
    struct _ICONEXPLORER_ITEM   *pPrevious;             // previous item point
    struct _ICONEXPLORER_ITEM   *pNext;                 // next item point
} T_ICONEXPLORER_ITEM;

// icon explorer struct
typedef struct {
	T_SLIPMGR 					*pSlipMgr;				//SlipMgr handle

    T_ICONEXPLORER_ITEM         *pItemHead;             // item head point
    T_ICONEXPLORER_ITEM         *pItemShow;             // item show first point
    T_ICONEXPLORER_ITEM         *pItemFocus;            // item focus point
    T_ICONEXPLORER_ITEM         *pItemOldFocus;         // item old focus point

    T_U32                       ItemQty;                // items quantity
    T_U32                       ItemQtyMax;             // items quantity max, 0 is no limit
    T_U32                       ItemStyle;              // item style
    T_U32                       ItemFrameWidth;         // item frame width
    T_COLOR                     ItemTransColor;         // item icon transparent color
    T_ICONEXPLORER_ICONSTYLE    ItemIconStyle;          // item icon style
    T_ICONEXPLORER_SORTMODE     ItemSortMode;           // item sort mode

    T_RECT                      TitleRect;              // title rect
    T_COLOR                     TitleBackColor;         // title back color
    T_U8                        *pTitleBackData;        // title back graph data point, AK bmp data
    T_U16                       *pTitleText;            // title text point, malloc it
    T_COLOR                     TitleTextColor;         // title text color
    T_U32                       TitleTextOffset;        // title text scroll offset

    T_SCBAR                     ScrollBar;              // scroll bar
    T_U32                       ScrollBarShowWidth;     // scroll bar show width
    T_U32                       ScrollBarWidth;         // scroll bar width
    T_BOOL                      ScrollBarFlag;          // scroll bar flag

    T_RECT                      ItemRect;               // item rect
    T_COLOR                     ItemBackColor;          // item back color
    T_U8                        *pItemBackData;         // item back graph data point, AK bmp data
    T_COLOR                     ItemTextColor;          // item text color
    T_COLOR                     ItemFocusBackColor;     // item focus back color
    T_COLOR                     ItemFocusTextColor;     // item focus text color
    T_U32                       ItemTextOffset;         // item text scroll offset

    T_U32                       SmallIconWidth;         // small icon width
    T_U32                       SmallIconHeight;        // small icon height
    T_U32                       SmallItemTInterval;     // small item icon-text interval
    T_U32                       SmallItemHInterval;     // small item horizontal interval
    T_U32                       SmallItemVInterval;     // small item vertical interval
    T_U32                       SmallItemRow;           // small item show row

    T_U32                       LargeIconWidth;         // large icon width
    T_U32                       LargeIconHeight;        // large icon height
    T_U32                       LargeTextHeight;        // large text height
    T_U32                       LargeItemTInterval;     // large item icon-text interval
    T_U32                       LargeItemHInterval;     // large item horizontal interval
    T_U32                       LargeItemVInterval;     // large item vertical interval
    T_U32                       LargeItemCol;           // large item show col
    T_U32                       LargeItemRow;           // large item show row

    T_U32                       NoneTextHeight;         // none text height
    T_U32                       NoneItemHInterval;      // none item horizontal interval
    T_U32                       NoneItemVInterval;      // none item vertical interval
    T_U32                       NoneItemRow;            // none item show row

    T_U32                       RefreshFlag;            // refresh flag
    T_ICONEXPLORER_CHANGE_FLAG  ItemListFlag;           // item list flag

    T_fICONEXPLORER_SORT_ID_CALLBACK        SortIdCallBack;         // sort id call back function
    T_fICONEXPLORER_SORT_CONTENT_CALLBACK   SortContentCallBack;    // sort content call back function
    T_fICONEXPLORER_LIST_CALLBACK           ListCallBack;           // list call back function
} T_ICONEXPLORER;

// state machine function, these function must by execute
T_BOOL IconExplorer_Init(T_ICONEXPLORER *pIconExplorer, T_RECT TitleRect, T_RECT ItemRect, T_U32 ItemStyle);
T_BOOL IconExplorer_Free(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconExplorer_Show(T_ICONEXPLORER *pIconExplorer);
T_eBACK_STATE IconExplorer_Handler(T_ICONEXPLORER *pIconExplorer, T_EVT_CODE Event, T_EVT_PARAM *pParam);

// item operate: add, modify, delete, move, etc.
T_BOOL IconExplorer_ModifyTitleText(T_ICONEXPLORER *pIconExplorer, const T_U16 *pTitleText);
T_BOOL IconExplorer_AddItem(T_ICONEXPLORER *pIconExplorer, T_U32 Id, T_VOID *pContent, T_U32 ContentLen, const T_U16 *pText, const T_U8 *pSmallIcon, const T_U8 *pLargeIcon);
T_BOOL IconExplorer_ModifyItemSmallIcon(T_ICONEXPLORER *pIconExplorer, T_U32 Id, const T_U8 *pSmallIcon);
T_BOOL IconExplorer_DelItem(T_ICONEXPLORER *pIconExplorer, T_U32 Id);
T_BOOL IconExplorer_DelItemFocus(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconExplorer_DelAllItem(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconExplorer_MoveFocus(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_DIRECTION Direction);
T_BOOL IconExplorer_SetFocus(T_ICONEXPLORER *pIconExplorer, T_U32 Id);
T_BOOL IconExplorer_SetFocusByIndex(T_ICONEXPLORER *pIconExplorer, T_U32 Index);

// icon explorer setup
T_BOOL IconExplorer_SetItemQtyMax(T_ICONEXPLORER *pIconExplorer, T_U32 ItemQtyMax);
T_BOOL IconExplorer_SetItemTransColor(T_ICONEXPLORER *pIconExplorer, T_COLOR ItemTransColor);
T_BOOL IconExplorer_SetItemIconStyle(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ICONSTYLE ItemIconStyle);
T_BOOL IconExplorer_SetSortMode(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_SORTMODE ItemSortMode);
T_BOOL IconExplorer_SetTitleRect(T_ICONEXPLORER *pIconExplorer, T_RECT TitleRect, T_COLOR TitleBackColor, const T_U8 *pTitleBackData);
T_BOOL IconExplorer_SetTitleText(T_ICONEXPLORER *pIconExplorer, const T_U16 *pTitleText, T_COLOR TitleTextColor);
T_BOOL IconExplorer_SetItemRect(T_ICONEXPLORER *pIconExplorer, T_RECT ItemRect, T_COLOR ItemBackColor, const T_U8 *pItemBackData);
T_BOOL IconExplorer_SetItemText(T_ICONEXPLORER *pIconExplorer, T_COLOR ItemTextColor, T_COLOR ItemFocusBackColor, T_COLOR ItemFocusTextColor);
T_BOOL IconExplorer_SetScrollBarWidth(T_ICONEXPLORER *pIconExplorer, T_U32 ScrollBarWidth);
T_BOOL IconExplorer_SetSmallIcon(T_ICONEXPLORER *pIconExplorer, T_U32 SmallIconWidth, T_U32 SmallIconHeight, T_U32 SmallItemTInterval, T_U32 SmallItemHInterval, T_U32 SmallItemVInterval);
T_BOOL IconExplorer_SetLargeIcon(T_ICONEXPLORER *pIconExplorer, T_U32 LargeIconWidth, T_U32 LargeIconHeight, T_U32 LargeTextHeight, T_U32 LargeItemTInterval, T_U32 LargeItemHInterval, T_U32 LargeItemVInterval);
T_BOOL IconExplorer_SetNoneIcon(T_ICONEXPLORER *pIconExplorer, T_U32 NoneTextHeight, T_U32 NoneItemHInterval, T_U32 NoneItemVInterval);
T_BOOL IconExplorer_SetRefresh(T_ICONEXPLORER *pIconExplorer, T_U32 RefreshFlag);
T_BOOL IconExplorer_SetListFlag(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconExplorer_SetSortIdCallBack(T_ICONEXPLORER *pIconExplorer, T_fICONEXPLORER_SORT_ID_CALLBACK SortIdCallBack);
T_BOOL IconExplorer_SetSortContentCallBack(T_ICONEXPLORER *pIconExplorer, T_fICONEXPLORER_SORT_CONTENT_CALLBACK SortContentCallBack);
T_BOOL IconExplorer_SetListCallBack(T_ICONEXPLORER *pIconExplorer, T_fICONEXPLORER_LIST_CALLBACK ListCallBack);

// get icon explorer infomation
T_U32 IconExplorer_GetItemQty(T_ICONEXPLORER *pIconExplorer);
T_ICONEXPLORER_ICONSTYLE IconExplorer_GetItemIconStyle(T_ICONEXPLORER *pIconExplorer);

T_U32 IconExplorer_GetItemIndexById(T_ICONEXPLORER *pIconExplorer, T_U32 id);
T_U32 IconExplorer_GetItemFocusId(T_ICONEXPLORER *pIconExplorer);
T_U32 IconExplorer_GetItemShowId(T_ICONEXPLORER *pIconExplorer);

T_ICONEXPLORER_ITEM *IconExplorer_GetItem(T_ICONEXPLORER *pIconExplorer, T_U32 Id);
T_ICONEXPLORER_ITEM *IconExplorer_GetItemFocus(T_ICONEXPLORER *pIconExplorer);
T_ICONEXPLORER *IconExplorer_CopyItems(T_ICONEXPLORER *pIconExplorerDest, T_ICONEXPLORER *pIconExplorerSrc);
T_VOID *IconExplorer_GetItemContentFocus(T_ICONEXPLORER *pIconExplorer);
T_VOID *IconExplorer_GetItemContentNextById(T_ICONEXPLORER *pIconExplorer, T_U32 Id);
T_U32   IconExplorer_GetRefresh(T_ICONEXPLORER *pIconExplorer);

// icon explorer other operate: swap, sort, etc.
T_BOOL IconExplorer_SortItem(T_ICONEXPLORER *pIconExplorer);

// item option operate: add, modify, delete, etc.
T_BOOL IconExplorer_AddItemWithOption(T_ICONEXPLORER *pIconExplorer, T_U32 Id, T_VOID *pContent, T_U32 ContentLen, const T_U16 *pText, const T_U8 *pSmallIcon, const T_U8 *pLargeIcon, T_ICONEXPLORER_OPTION_TYPE OptionType, T_U8 *pOptionValue);
T_BOOL IconExplorer_AddItemOption(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId, T_U8 OptionId, const T_U16 *pText);

// check item list
T_BOOL IconExplorer_CheckItemList(T_ICONEXPLORER *pIconExplorer);
T_BOOL IconExplorer_SortIdCallback(T_U32 Id1, T_U32 Id2);

#if 0

T_BOOL IconExplorer_ModifyItemContent(T_ICONEXPLORER *pIconExplorer, T_U32 Id, T_VOID *pContent, T_U32 ContentLen);
T_BOOL IconExplorer_ModifyItemText(T_ICONEXPLORER *pIconExplorer, T_U32 Id, const T_U16 *pText);
T_BOOL IconExplorer_ModifyItemLargeIcon(T_ICONEXPLORER *pIconExplorer, T_U32 Id, const T_U8 *pLargeIcon);

T_ICONEXPLORER_SORTMODE IconExplorer_GetItemSortMode(T_ICONEXPLORER *pIconExplorer);
T_U32 IconExplorer_GetItemIdByIndex(T_ICONEXPLORER *pIconExplorer, T_U32 Index);

T_U32 IconExplorer_GetItemPreviousId(T_ICONEXPLORER *pIconExplorer, T_U32 Id);
T_U32 IconExplorer_GetItemNextId(T_ICONEXPLORER *pIconExplorer, T_U32 Id);
T_U32 IconExplorer_GetIdByPoint(T_ICONEXPLORER *pIconExplorer, T_U16 x, T_U16 y);

T_ICONEXPLORER_ITEM *IconExplorer_GetItemShow(T_ICONEXPLORER *pIconExplorer);
T_ICONEXPLORER_ITEM *IconExplorer_GetItemPrevious(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem);
T_ICONEXPLORER_ITEM *IconExplorer_GetItemNext(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem);

T_VOID *IconExplorer_GetItemContent(T_ICONEXPLORER *pIconExplorer, T_U32 Id);

T_VOID *IconExplorer_GetItemContentShow(T_ICONEXPLORER *pIconExplorer);
T_VOID *IconExplorer_GetItemContentPreviousById(T_ICONEXPLORER *pIconExplorer, T_U32 Id);

T_VOID *IconExplorer_GetItemContentPreviousByItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem);
T_VOID *IconExplorer_GetItemContentNextByItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem);

T_BOOL IconExplorer_SwapItem(T_ICONEXPLORER *pIconExplorer, T_ICONEXPLORER_ITEM *pItem1, T_ICONEXPLORER_ITEM *pItem2);
T_BOOL IconExplorer_SwapItemById(T_ICONEXPLORER *pIconExplorer, T_U32 Id1, T_U32 Id2);

T_BOOL IconExplorer_ModifyItemOption(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId, T_U32 OptionId, const T_U16 *pText);
T_BOOL IconExplorer_DelItemOption(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId, T_U32 OptionId);
T_BOOL IconExplorer_DelAllItemOption(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId);

// item option focus
T_BOOL IconExplorer_SetItemOptionFocusId(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId, T_U32 OptionId);
T_U32 IconExplorer_GetItemOptionFocusId(T_ICONEXPLORER *pIconExplorer);

T_U8 IconExplorer_GetItemOptionFocusIdByItemId(T_ICONEXPLORER *pIconExplorer, T_U32 ItemId);

#endif
#endif  /* end of __CTL_ICONEXPLORER_H__ */
