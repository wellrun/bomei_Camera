/**
 * @file Ctl_Dialog.h
 * @brief This header file is for setting dialog function prototype
 * @author: ZhuSiZhe
 * @date: 2005-02-03
 * @version: 1.0
 */

#ifndef __UTL_DIALOG_H__
#define __UTL_DIALOG_H__

#include "ctl_global.h"
#include "Ctl_Title.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DIALOG_REFRESH_NONE = 0,
    DIALOG_REFRESH_BKGD = (1<<1),
    DIALOG_REFRESH_SET = (1<<2),
    DIALOG_REFRESH_SELECT_BUTTON = (1<<3),
    DIALOG_REFRESH_ADJUST_BUTTON = (1<<4),
    DIALOG_REFRESH_ALL = (0xff)
}T_DIALOG_REFRESH;

typedef enum
{
    DBTN_OK = 0,
    DBTN_CANCEL
}T_DIALOG_BTN;

typedef enum
{
    DCLK_MINUS = 0,
    DCLK_PLUS,
    DCLK_NONE
}T_DIALOG_CLK;

typedef struct{
    T_pCDATA        pBkImg;            

    T_pCDATA        pBtnMinus[2];         
    T_pCDATA        pBtnPlus[2];         
    T_pCDATA        pBtnOk[2];         
    T_pCDATA        pBtnCncl[2];       
    T_pCDATA        pPrgBar;       
    T_pCDATA        pPrgBkgd;

    T_RECT          BkImgRct;
    T_RECT          PrgBarRct;            /*progress bar rect */
    T_RECT          PrgBkgdRct;           /* progress background rect */

    T_RECT          textRct;
    T_RECT          OkBtRct;           
    T_RECT          CnclBtRct;         
    T_RECT          MinusBtRct;           
    T_RECT          PlusBtRct;  
}T_DIALOG_RES;

typedef struct{
    T_LEN   gBsH;
    T_LEN   gBsW;
    T_LEN   gIntvl;
    T_LEN   gAddH;
}T_DIALOG_GRAPH;

typedef struct {
    T_DIALOG_RES        res;
    T_U16               initFlag;               /* identify the control is initialized or not */
    T_DIALOG_BTN        bFcsBtn;
    T_U16               CurValue;               /* graph current value */
    T_U16               TtlValue;               /* graph total value */
    T_U16               MinValue;               /* graph min value*/
    T_U16               StepValue;               /* graph step value */

    T_DIALOG_REFRESH    RefreshFlag;  
    T_U16               CurGraNum;              /* current graph number */
    T_DIALOG_CLK        ClickBtn;             /* click plus/minus buttons or not*/
} T_DIALOG;


T_BOOL  Dialog_Init(T_DIALOG *pDialog, T_U16 CurValue, T_U16 MinVal, T_U16 MaxVal, T_U16 StepValue);
T_VOID Dialog_Free(T_DIALOG *pDialog);
T_VOID Dialog_SetTitle(T_DIALOG *pDialog, T_pCWSTR title);
T_VOID Dialog_SetRefresh(T_DIALOG *pDialog, T_DIALOG_REFRESH refresh);
T_S16 Dialog_GetCurValue(T_DIALOG *pDialog);

T_VOID  Dialog_ShowGraph(T_DIALOG *pDialog, T_COLOR color);
T_VOID  Dialog_Show(T_DIALOG *pDialog, T_COLOR color);
T_eBACK_STATE   Dialog_Handler(T_DIALOG *pDialog, T_EVT_CODE Event, T_EVT_PARAM *pParam);
T_VOID Dialog_ModifyRectTop(T_DIALOG *pDialog, T_POS prgBk_top, T_POS OkBtn_top);

#ifdef __cplusplus
}
#endif
#endif  /* end of __UTL_DIALOG_H__ */
