
/**************************************************************
*
* Copyrights (C) 2005, ANYKA Software Inc.
* All rights reserced.
*
* File name: Ctl_IconMenu.h
* File flag: Icon Menu
* File description:
*
* Revision: 1.00
* Author: Guanghua Zhang
* Date: 2005-11-16
*
****************************************************************/

#ifndef __CTL_ICONMENU_H__
#define __CTL_ICONMENU_H__

#include "anyka_types.h"
#include "Eng_Graph.h"
#include "Ctl_ScrollBar.h"

#define ICONMENU_TEXT_LEN               64

#ifdef ICONMENU_VERTICAL_ICON
#define ICONMENU_ICON_ON_FOCUS          1
#define ICONMENU_ICON_ATTACH_NUM        4
#define ICONMENU_ICON_LIST_NUM          2
#define ICONMENU_ICON_NUM               (ICONMENU_ICON_LIST_NUM+ICONMENU_ICON_ATTACH_NUM)
#define ICONMENU_ICONSIZE_MIN_WIDTH     60
#define ICONMENU_ICONSIZE_MIN_HEIGHT    30
#define ICONMENU_REFRESH_FOCUS          0x04
#else   // grid icon
#define ICONMENU_ICON_ADJUST_TOP_MARGIN 8
#define ICONMENU_ICONSIZE_MIN_WIDTH     60

#if (LCD_CONFIG_WIDTH==480)
#define ICONMENU_ICONSIZE_MIN_HEIGHT    70
#else
#define ICONMENU_ICONSIZE_MIN_HEIGHT    60
#endif
#define ICONMENU_ICON_NUM               1
#define ICONMENU_REFRESH_FOCUS          (0x04|0x10)
#endif

#define ICONMENU_ICON_FOCUS_BACK        COLOR_SKYBLUE
#define ICONMENU_ICON_DEFAULT           0
#define ICONMENU_ITEMQTY_MAX            256
#define ICONMENU_ICONINTERVAL_DEFAULT   0
#define ICONMENU_ICONINTERVAL_MAX       32
#define ICONMENU_ATTACH_FONT_INTERVAL   2
#define ICONMENU_ANIMATETIME            700
#define ICONMENU_ICONTRANS_DEFAULT      40
#define ICONMENU_ERROR_ID               0xff
#define ICONMENU_ERROR_PLACE            0xff
#define ICONMENU_ICON_TEXT_COLOR        COLOR_WHITE

#define ICONMENU_REFRESH_NONE           0x00
#define ICONMENU_REFRESH_ALL            0xff
#define ICONMENU_REFRESH_ICON_ATTACH    0x01
#define ICONMENU_REFRESH_ICON           0x02
#define ICONMENU_REFRESH_SCRBAR         0x10

#define ICONMENU_REFRESH_OTHER          0x08

#define ICONMENU_ALIGN_HORIZONTAL       0x0f
#define ICONMENU_ALIGN_LEFT             0x01
#define ICONMENU_ALIGN_RIGHT            0x02
#define ICONMENU_ALIGN_HCENTER          0x04
#define ICONMENU_ALIGN_VERTICAL         0xf0
#define ICONMENU_ALIGN_UP               0x10
#define ICONMENU_ALIGN_DOWN             0x20
#define ICONMENU_ALIGN_VCENTER          0x40

typedef T_VOID (*T_fICONMENU_CALLBACK)(T_VOID);

typedef enum {
    ICONMENU_DIRECTION_UP = 0,
    ICONMENU_DIRECTION_DOWN,
    ICONMENU_DIRECTION_LEFT,
    ICONMENU_DIRECTION_RIGHT,
    ICONMENU_DIRECTION_NUM
} T_ICONMENU_DIRECTION;

typedef struct _ICONMENU_RECT {
    T_RECT                  Rect;                           // rect
    struct _ICONMENU_RECT   *pNext;                         // next rect point
} T_ICONMENU_RECT;

typedef struct _ICONMENU_ITEM {
    T_U8                    Id;                             // icon item id
    T_S8                    Place;                          // icon item place
    T_U16                   ItemText[ICONMENU_TEXT_LEN+1];  // item text
    T_U8                    *pIconData[ICONMENU_ICON_NUM];  // icon graph data, AK BMP format string
    struct _ICONMENU_ITEM   *pPrevious;                     // previous item point
    struct _ICONMENU_ITEM   *pNext;                         // next item point
} T_ICONMENU_ITEM;                                          // icon item struct

typedef struct {
    T_ICONMENU_ITEM         *pItemHead;                     // first (head) item point
    T_U16                   ItemQty;                        // items quantity
    T_ICONMENU_ITEM         *pItemFocus;                    // focus item point
#ifndef ICONMENU_VERTICAL_ICON
    T_ICONMENU_ITEM         *pOldItemFocus;                 // old focus item point
    T_SCBAR                 scrollBar;
#endif // ICONMENU_VERTICAL_ICON
    T_RECT                  IconAttachtRect;                       // item text show rect
    T_RECT                  IconRect;                       // icon show rect
    T_ICONMENU_RECT         *pOtherRect;                    // other show rect point
    T_U8                    ItemRow;                        // items row
    T_U8                    ItemCol;                        // items column

    T_U8                    IconImageNum;                   // icon image number
    T_U8                    IconImageDefault;               // icon image static number
    T_U32                   IconHInterval;                  // icon horizontal interval
    T_U32                   IconVInterval;                  // icon vertical interval
    T_U32                   ItemWidth;                      // item show width
    T_U32                   ItemHeight;                     // item show height

    T_U8                    IconAttachTextAlign;                      // item text align
    T_COLOR                 IconAttachTextColor;                      // item text color
    T_COLOR                 IconAttachTextBackColor;                  // item text back color
    T_U8                    *pIconAttachBkImg;                 // item text back graph data, AK BMP format string, if AK_NULL, show item text back color
    T_BOOL                  IconAttachPartRectFlag;               // item text back graph part flag
    T_RECT                  IconAttachPartRect;                   // item text back graph part rect
    T_COLOR                 IconTransColor;                 // icon transparent color
    T_COLOR                 IconBackColor;                  // icon back color
    T_U8                    *pIconBackData;                 // icon back graph data, AK BMP format string, if AK_NULL, show icon back color
    T_BOOL                  IconPartRectFlag;               // icon back graph part flag
    T_RECT                  IconPartRect;                   // icon bakc graph part rect
    T_U8                    IconTransparency;               // icon transparency

    T_BOOL                  IconShowFlag;                   // icon show flag
    T_U8                    RefreshFlag;                    // refresh flag

    T_U8                    IconAnimateCount;               // icon animate refresh count
    T_TIMER                 IconAnimateTimer;               // icon animate refresh timer id

    T_S32                   windowPlace;
    T_S32                   pageRow;                        // number of row in one page
    T_BOOL                  ShowFastMode;                   // show graph fast mode
    T_fICONMENU_CALLBACK    OtherShowCallBack;              // other show call back funtion
} T_ICONMENU;

T_BOOL IconMenu_Init(T_ICONMENU *pIconMenu, T_RECT IconAttachRect, T_RECT IconRect, T_RECT scbarRect);
T_BOOL IconMenu_Free(T_ICONMENU *pIconMenu);
T_BOOL IconMenu_Show(T_ICONMENU *pIconMenu);
T_eBACK_STATE IconMenu_Handler(T_ICONMENU *pIconMenu, T_EVT_CODE Event, T_EVT_PARAM *pParam);

T_BOOL IconMenu_AddItem(T_ICONMENU *pIconMenu, T_U8 Id, T_U8 Place, const T_U16 *pItemText);
T_BOOL IconMenu_ResetItemText(T_ICONMENU *pIconMenu, T_U8 Id, const T_U16 *pItemText);
T_BOOL IconMenu_SetItemIcon(T_ICONMENU *pIconMenu, T_U8 Id, T_U8 index, const T_U8 *pIconData);
T_BOOL IconMenu_DelItemById(T_ICONMENU *pIconMenu, T_U8 Id);
T_BOOL IconMenu_DelItemByPlace(T_ICONMENU *pIconMenu, T_U8 Place);
T_BOOL IconMenu_MoveFocus(T_ICONMENU *pIconMenu, T_ICONMENU_DIRECTION Dir);
T_BOOL IconMenu_SetItemFocus(T_ICONMENU *pIconMenu, T_U8 Id);

T_BOOL IconMenu_SetIconAttachRect(T_ICONMENU *pIconMenu, T_RECT IconAttachRect);
T_BOOL IconMenu_SetIconRect(T_ICONMENU *pIconMenu, T_RECT IconRect);
T_BOOL IconMenu_SetItemMatrix(T_ICONMENU *pIconMenu, T_U8 ItemRow, T_U8 ItemCol);
T_BOOL IconMenu_SetItemMatrixAuto(T_ICONMENU *pIconMenu);
T_BOOL IconMenu_SetIconAttachStyle(T_ICONMENU *pIconMenu, T_U8 TextAlign, T_COLOR TextColor, \
                                   T_COLOR TextBackColor, const T_U8 *pTextBackData, \
                                   T_RECT *pTextPartRect);
T_BOOL IconMenu_SetIconStyle(T_ICONMENU *pIconMenu, T_U32 IconHInterval, T_U32 IconVInterval, \
                             T_COLOR IconTransColor, T_COLOR IconBackColor, T_U8 IconTransparency, \
                             const T_U8 *pIconBackData, T_RECT *pIconPartRect);
T_BOOL IconMenu_SetIconImageNum(T_ICONMENU *pIconMenu, T_U8 IconImageNum, T_U8 IconImageDefault);
T_BOOL IconMenu_SetIconShowFlag(T_ICONMENU *pIconMenu, T_BOOL IconShowFlag);
T_BOOL IconMenu_SetRefresh(T_ICONMENU *pIconMenu, T_U8 RefreshFlag);
T_BOOL IconMenu_SetShowFastMode(T_ICONMENU *pIconMenu, T_BOOL ShowFastMode);
T_BOOL IconMenu_SetOtherShowCallBack(T_ICONMENU *pIconMenu, T_fICONMENU_CALLBACK OtherShowCallBack);
T_BOOL IconMenu_AddOtherRect(T_ICONMENU *pIconMenu, T_RECT OtherRect);

T_U8 IconMenu_GetRefreshFlag(T_ICONMENU *pIconMenu);
T_BOOL IconMenu_GetIconShowFlag(T_ICONMENU *pIconMenu);
T_ICONMENU_ITEM *IconMenu_GetFocusItem(T_ICONMENU *pIconMenu);
T_U8 IconMenu_GetFocusId(T_ICONMENU *pIconMenu);
T_U8 IconMenu_GetFocusPlace(T_ICONMENU *pIconMenu);
T_U8 IconMenu_GetIdByPoint(T_ICONMENU *pIconMenu, T_U16 x, T_U16 y);
T_ICONMENU_ITEM *IconMenu_FindItemById(T_ICONMENU *pIconMenu, T_U8 Id);
T_ICONMENU_ITEM *IconMenu_FindItemByPlace(T_ICONMENU *pIconMenu, T_U32 Place);

#endif
