
/*************************************************************************
 
    Copyright (C) Anyka Co., LTD
    AUTHOR: he_ying
    DATE: 2007-12-4
 
*************************************************************************/

#ifndef __UTL_EBOOK_H__
#define __UTL_EBOOK_H__

#include "ctl_ebook.h"
//#include "ebookcore.h"
#include "Lib_event.h"
#include "Ctl_Title.h"
#include "Eng_TopBar.h"
#include "Ctl_ScrollBar.h"

#ifdef __cplusplus
extern "C"
{
#endif


    T_BOOL EBCtl_New(const T_U16 * path, const T_pRECT pEbRect);
    T_VOID EBCtl_Close(T_VOID);

    T_BOOL EBCtl_IsSupportFileType(T_pCWSTR pFileName);
    T_S32 EBCtl_Handler(T_EVT_CODE event, T_EVT_PARAM * pEventParm);

    T_VOID EBCtl_Refresh(T_VOID);
    T_VOID EBCtl_Show(T_VOID);

    T_VOID EBCtl_SetColor(T_COLOR frontColor, T_COLOR backColor,T_COLOR focusFrontColor, T_COLOR focusBackColor);

    T_VOID EBCtl_SetIsShowFocus(T_BOOL isShow);

    T_VOID EBCtl_StopAutoScroll(T_VOID);
    T_VOID EBCtl_AutoScroll(T_U8 perLines, T_U16 interval);

    T_U32 EBCtl_GetBookMarkSize(T_VOID);
    T_VOID EBCtl_CleanBookmark(T_pVOID bookmark);
    T_VOID EBCtl_SetColor(T_COLOR frontColor, T_COLOR backColor,T_COLOR focusFrontColor, T_COLOR focusBackColor);

    /**
     * @brief   load scroll bar image data
     * @author  WangWei
     * @date    2008-05-04 
     * @param   T_VOID
     * @return  T_VOID
     * @retval  
     */
    T_VOID EBCtl_LoadScrBarImageData(T_VOID);
#ifdef __cplusplus
}
#endif

#endif
