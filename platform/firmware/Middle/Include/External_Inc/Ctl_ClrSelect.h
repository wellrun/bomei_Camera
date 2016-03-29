/*
 * Copyright (c) 2002, Anyka Inc.
 * All rights reserved.
 * 
 * @file Ctl_ClrSelect.h
 * @brief This header file is for ClrSelect definition and function prototype
 * 
 * @version 1.00
 * @author Junhua Zhao
 * @date 2005-04-14
 */

#ifndef __UTL_CLRSELECT_H__
#define __UTL_CLRSELECT_H__

#include "anyka_types.h"
#include "Ctl_Title.h"

#define CLRSELECT_WIDTH_DEF         200
#define CLRSELECT_HEIGHT_DEF            100

typedef enum {
    CURRENT_RED = 0,
    CURRENT_GREEN,
    CURRENT_BLUE,       
    CURRENT_MAX
} T_CURCOLOR;

typedef enum {
    BUTTON_OK       = 0,
    BUTTON_CANCEL
} T_BUTTON_FCS;

typedef struct {
    T_pCDATA        pBkImg;               /* color select background image */
    T_pCDATA        pClrImg[3];           /* red, green, blue color image */
    T_pCDATA        pClrFram[4];          /* red, green, blue color frame image */
    T_pDATA         pClrIcnBckLR;         /* left and right icon background image */
    T_pDATA         pClrIcnBckUD;         /* up and down icon background image */
    T_pDATA         pClrIcn[4];           /* up,down,left,right icon */
    T_pDATA         pBtnOk[2];              /* ok button: focus or not */
    T_pDATA         pBtnCncl[2];            /* cancel button:bocus or not */

    T_RECT          BkImgRct;           //±³¾°Í¼Æ¬
    T_RECT          ClrImgRct[3];       //ÑÕÉ«Í¼Æ¬
    T_RECT          ClrFramRct[4];      //ÑÕÉ«ÉèÖÃ¿éÍ¼Æ¬
    T_RECT          IcnBkRct_L;         //×ó±ßÈý½ÇÍ¼±ê±³¾°
    T_RECT          IcnBkRct_R;         //ÓÒ±ßÈý½ÇÍ¼±ê±³¾°
    T_RECT          IcnBkRct_U[3];      //ÉÏ±ß
    T_RECT          IcnBkRct_D[3];      //ÏÂ±ß
    T_RECT          IcnRct_L;           /* Ïò×óÈý½ÇÍ¼±ê */
    T_RECT          IcnRct_R;           /* ÏòÓÒ */
    T_RECT          IcnRct_U[3];        /* ÏòÉÏ */
    T_RECT          IcnRct_D[3];        /* ÏòÏÂ */
    
    T_RECT          EditRct;            /* ±à¼­¿ò */
    T_RECT          curClrRct;           /* current color rect */
    T_RECT          ButnOkRct;
    T_RECT          ButnCnclRct;
} T_CLRSLCT_RES;

typedef struct {
    T_CLRSLCT_RES   res;
    T_TITLE         Title;                  /* color select title */
    T_CURCOLOR      curColor;               /* current select color*/
    T_COLOR         TextColor;              /* the color of text */
    T_COLOR         color;                  /* current color */
    T_BUTTON_FCS    fcsBtn;
    T_BOOL          bPrsIcn[4];             //press up/down/left/dow icon flag
    T_U16           RefreshFlag;            /* refresh flag */
} T_CLRSELECT;

T_VOID  ClrSelect_Init(T_CLRSELECT *pClrSelect);                    // initiailize color select
T_VOID  ClrSelect_Free(T_CLRSELECT *pClrSelect);                    // free color select

T_VOID  ClrSelect_SetDefault(T_CLRSELECT *pClrSelect,T_COLOR defcolor);     //set default color

T_VOID  ClrSelect_Show(T_CLRSELECT *pClrSelect);        //show color select
T_VOID  ClrSelect_ShowEdit(T_CLRSELECT *pClrSelect);                    //show edit box
T_VOID  ClrSelect_ShowButton(T_CLRSELECT *pClrSelect);

T_eBACK_STATE   ClrSelect_Handle(T_CLRSELECT *pClrSelect, T_EVT_CODE Event, T_EVT_PARAM *pParam);       //process key event
T_COLOR ClrSelect_GetColor(T_CLRSELECT *pClrSelect);                    //get color of select

#endif

/* end of file */
