/**
 * @file Ctl_MsgBox.h
 * @brief This header file is for message box definition and function prototype
 * @author: ZouMai
 */

#ifndef __UTL_MSG_BOX_H__
#define __UTL_MSG_BOX_H__

#include "ctl_global.h"
#include "Ctl_Title.h"
#include "Ctl_ScrollBar.h"
#include "eng_string_uc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Define icon mode, use bits 0--3 of a T_U16 integer number */
#define MSGBOX_NOICON               0x0000
#define MSGBOX_EXCLAMATION          0x0001
#define MSGBOX_INFORMATION          0x0002
#define MSGBOX_QUESTION             0x0003
#define MSGBOX_ALLICON_MODE         0x0003

/*Define button mode, use bit 8--10 of a T_U16 integer number */
#define MSGBOX_MB_NOBUTTON          0x0000      /* Message box no button */
#define MSGBOX_YESNO                0x0100      /* Message box "yesno" button */
#define MSGBOX_OK                   0x0200      /* Message box "ok" button */
#define MSGBOX_OKCANCEL             0x0300      /* Message box "retrycancel" button */
#define MSGBOX_RETRYCANCEL          0x0400      /* Message box "abortretryignore" button */
#define MSGBOX_EXIT                 0x0500      /* Message box "exit" button */
#define MSGBOX_ALLBUTTON_MODE       0x0700

/* Define horizontal typeset mode, use bits 4--5 of a T_U16 integer number */
#define MSGBOX_HMIDDLE              0x0000
#define MSGBOX_HLEFT                0x0010
#define MSGBOX_HRIGHT               0x0020
#define MSGBOX_ALLHOR_MODE          0x0030

/* Define vertical typeset mode, use bits 6--7 of a T_U16 integer number */
#define MSGBOX_VMIDDLE              0x0000
#define MSGBOX_VTOP                 0x0040
#define MSGBOX_VBOTTOM              0x0050
#define MSGBOX_ALLVER_MODE          0x0050

#define MSGBOX_WIDTH_DEF            220
#define MSGBOX_HEIGHT_DEF           120

#define MSGBOX_MIN_HEIGHT           100

#define MSGBOX_BUTTON_BOTTOM        6
#define MSGBOX_BUTTON_HEIGTH        22
#define MSGBOX_CONTEN_TOP           10
#define MSGBOX_INTVL_TEXT_IMG       4   /* interval between text and scroll bar or icon*/

#define MSGBOX_DELAY_0              2   /* Message box delay(second) */
#define MSGBOX_DELAY_1              3   /* Message box delay(second) */
#define MSGBOX_DELAY_2              6   /* Message box long delay(second) */

#define MSGBOX_FRAME_WIDTH          14


#define MSGBOX_LEN_MAX              1024

typedef T_S8    T_STR_MSGINFO[MSGBOX_LEN_MAX+1];
typedef T_U16   T_USTR_MSGINFO[MSGBOX_LEN_MAX+1];

typedef struct {
    T_pCDATA    pMsgBkImg;
    T_pCDATA    pBtnImg[2];
    T_pCDATA    pIconImg;
    T_RECT      MsgBkImgRct;
    T_RECT      BtnImgRct1;
    T_RECT      BtnImgRct2;
    T_RECT      IconImgRct;
    T_RECT      contentRct;
}T_MSGBOX_RES;

typedef struct {
    T_MSGBOX_RES    res;
    T_U16           initFlag;       /* identify the control is initialized or not */
    T_U16           *pInfo;         /* message box content */
    T_RES_BINARY    iconBmpId;
    T_U16           iconMode;       /* message box icon type */
    T_U16           horMode;        /* horizontal typeset: MSGBOX_HMIDDLE, MSGBOX_HLEFT or MSGBOX_HRIGHT */
    T_U16           verMode;        /* vertical typeset: MSGBOX_VMIDDLE, MSGBOX_VTOP or MSGBOX_VBOTTOM */
    T_U16           buttonMode;     /* message box button type */
    T_S16           delayTime;      /* delay time of message box */
    T_U16           rIntvl;         /* interval between two lines divided from one sentence */
    T_U16           scrbarWidth;    /* scroll bar width */
    T_LEN           frameWidth;     /* frame width */
    T_U16           MaxPgRow;       /* Maximum row number in one page */
    T_U16           MaxRowQty;      /* Maximum row number */
    T_U16           CurRowQty;      /* current row number */
    T_U16           CurRowID;       /* current row ID of the beginning for display */
    T_U16           CurChrQty;      /* current character number */
    T_S16           RefreshFlag;    /* if RefreshFlag == CTL_REFRESH_NONE: refresh nothing
                                   if RefreshFlag == CTL_REFRESH_CONTENT: refresh all the menu items
                                   if RefreshFlag == CTL_REFRESH_ALL: refresh the overall menu control */
    T_U16           ReturnLevel;    /* return level after show the message */

    T_U8            buttonNum;      /* buttom number */
    T_U8            buttonFocus;    /* focused buttom ID */
    T_USTR_MSGINFO  Info;           /* message box content */
    T_CARVED_USTR   UCarvedStr;
    T_SCBAR         scBar;          /* message box scroll bar */
} T_MSGBOX, *T_hMSGBOX;

T_BOOL  MsgBox_Init(T_MSGBOX *mbox, T_LEN width, T_LEN height, T_U16 mode);
T_VOID  MsgBox_InitAfx(T_MSGBOX *mbox, T_U16 retLevel, T_S16 tID, T_S16 sID, T_U16 mode);
T_VOID  MsgBox_InitStr(T_MSGBOX *mbox, T_U16 retLevel, T_pCWSTR title, T_pCWSTR content, T_U16 mode);
T_VOID  MsgBox_Reset(T_MSGBOX *mbox);
T_VOID  MsgBox_SetDelay(T_MSGBOX *mbox, T_S16 delayTime);
T_VOID  MsgBox_SetData(T_MSGBOX *mbox, T_U16 *pInfo);
T_VOID  MsgBox_SetMaxRow(T_MSGBOX *mbox, T_U16 maxRow);
T_VOID  MsgBox_SetReturnLevel(T_MSGBOX *mbox, T_U16 retLevel);
T_VOID  MsgBox_SetRefresh(T_MSGBOX *mbox, T_S16 refresh);
T_BOOL MsgBox_AddLine(T_MSGBOX *mbox, T_pCWSTR uText);
T_BOOL MsgBox_CatText(T_MSGBOX *mbox, T_pCWSTR text);

T_U16   MsgBox_GetReturnLevel(const T_MSGBOX *mbox);
T_pWSTR  MsgBox_GetText(const T_MSGBOX *mbox);
T_U8    MsgBox_GetFocusButtonID(const T_MSGBOX *mbox);
T_BOOL  MsgBox_ScrollUpLine(T_MSGBOX *mbox);
T_BOOL  MsgBox_ScrollDnLine(T_MSGBOX *mbox);
T_BOOL  MsgBox_ScrollUpPage(T_MSGBOX *mbox);
T_BOOL  MsgBox_ScrollDnPage(T_MSGBOX *mbox);

T_VOID  MsgBox_Show(T_MSGBOX *mbox);
T_VOID  MsgBox_ShowButton(T_MSGBOX *mbox);
T_eBACK_STATE   MsgBox_Handler(T_MSGBOX *mbox, T_EVT_CODE Event, T_EVT_PARAM *pParam);
T_BOOL MsgBox_GetRect(T_MSGBOX *mbox, T_pRECT pMsgRect);

T_VOID  MsgBox_Reset(T_MSGBOX *mbox);
T_VOID  MsgBox_ClearContent_EX(T_MSGBOX *mbox);


#ifdef __cplusplus
}
#endif

#endif
