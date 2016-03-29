/**
 * @file Ctl_Title.h
 * @brief This header file is for title definition and function prototype
 * @author: ZouMai
 */

#ifndef __UTL_TITLE_H__
#define __UTL_TITLE_H__

#include "anyka_types.h"
#include "ctl_global.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @def TITLE_MENU_OPENED           -20
 *
 */
#define TITLE_MENU_OPENED           -20
/**
 * @def TITLE_DO_NOTHING            -30
 *
 */
#define TITLE_DO_NOTHING            -30
/**
 * @def TITLE_LEN_MAX               260
 *
 */
#define TITLE_LEN_MAX               260

typedef T_U16    T_USTR_TITLE[TITLE_LEN_MAX+1];

typedef struct {
    T_U16           initFlag;       /* identify the control is initialized or not */
    T_USTR_TITLE    uText;           /* unicode text, display 3 lines at most */
    T_pDATA         bmpData;        /* bmp data in anyka format */

    T_RECT          Rect;           /* location of pTitle */
    T_POS           Width;         /* prefix icon width */
    T_U16           chrBegin;       /* The first shown character's ID, this variable for scroll pTitle */
    T_U16           uTextLen;        /* text length */
    T_U16           MaxCol;         /* maximum text column */
    T_S16           TmCount;        /* time count for scroll the text automatically */
} T_TITLE;

T_VOID  Title_Init(T_TITLE *pTitle, const T_pRECT pRect);
T_VOID  Title_SetLoc(T_TITLE *pTitle, const T_pRECT rect);
//add by ljh
const T_pRECT Title_GetRect(const T_TITLE *pTitle);
//add end
T_VOID  Title_SetData(T_TITLE *pTitle, T_pWSTR pText, T_pCDATA bmpData);

T_pWSTR  Title_GetText(T_pWSTR pText, const T_TITLE *pTitle);

// T_BOOL showImg:  «∑Òœ‘ æpub_title Õº∆¨
// Alter by liuweijun
T_VOID  Title_Show(T_TITLE *pTitle, T_BOOL showImg);
T_BOOL  Title_ScrollText(T_TITLE *pTitle ,T_BOOL showImg);

#ifdef __cplusplus
}
#endif

#endif
