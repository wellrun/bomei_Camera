/**
 * @file Ctl_ScrollBar.h
 * @brief This header file is for scroll bar definition and function prototype
 * @author: ZouMai
 */

#ifndef __UTL_SCROLLBAR_H__
#define __UTL_SCROLLBAR_H__

#include "anyka_types.h"
#include "ctl_global.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def SCBAR_VERTICAL                  0x0000      
 * Define direction mode, use bit 0 of a T_U16 integer number
 */
#define SCBAR_VERTICAL                  0x0000      
/**
 * @def SCBAR_HORIZONTAL                0x0001
 *
 */
#define SCBAR_HORIZONTAL                0x0001

typedef struct {
    T_U16           dirMode;        /* Direction, SCBAR_VERTICAL or SCBAR_HORIZONTAL */

    T_U16           initFlag;       /* identify the control is initialized or not */
    T_POS           Left;           /*  location of progress bar */
    T_POS           Top;
    T_LEN           Width;
    T_LEN           Height;

    T_BOOL          Available;      /* current scroll bar is available or not */
    T_S16           Side;           /* Side height or width */
    T_S16           Intvl;          /* scroll bar unit height or width */

    T_COLOR         frColor;        /* scroll bar color */
    T_COLOR         bkColor;
    T_COLOR         fmColor;

    T_U32           CurValue;       /* current selected page */
    T_U32           TtlValue;       /* current total page number */
    T_U16           PgFactor;       /* how many items skip when scroll one page */
    T_pCDATA        lScBarImg[4];   /* little scroll bar images */
    T_pCDATA        bScBarImg[4];   /* big scroll bar imgages */
    T_pCDATA        bUpIcon[2];     /* big scroll bar imgages */
    T_pCDATA        bDownIcon[2];   /* big scroll bar imgages */
    T_RECT          scbarRect;
    T_RECT          upIconRect;
    T_RECT          downIconRect;    
    T_BOOL          useBigBar;      /* if it is true,use big bar; or use little bar */
} T_SCBAR;

T_VOID  ScBar_Init(T_SCBAR *bar, T_POS left, T_POS top, T_LEN width, T_LEN height, T_S16 intvl, T_U16 mode);
T_VOID  ScBar_SetLoc(T_SCBAR *bar, T_POS left, T_POS top, T_LEN width, T_LEN height);
T_BOOL  ScBar_SetValue(T_SCBAR *bar, T_U32 cur, T_U32 total, T_U16 factor);
T_BOOL  ScBar_SetCurValue(T_SCBAR *bar, T_U32 cur);

T_VOID  ScBar_Show(T_SCBAR *bar);
T_VOID  ScBar_Show2(T_SCBAR *bar);

/**
 * @brief load scroll bar image data
 * @author wangwei
 * @date 2002-05-25
 * @param T_SCBAR *bar
 * @return T_VOID
 * @retval 
 */
T_VOID ScBar_LoadImageData(T_SCBAR *bar);

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
T_RECT ScBar_GetRect(T_SCBAR *bar);

/**
 * @brief get scroll bar display location
 * @author wangwei
 * @date 2008-08-07
 * @param T_RECT *rect: scroll bar display location rect
 * @param T_SCBAR *bar: scroll bar struct pionter
 * @return T_VOID
 * @retval 
 */
T_VOID ScBar_GetLocaRect(T_RECT *rect, const T_SCBAR *bar);

/**
 * @brief get scroll bar display location
 * @author wangwei
 * @date 2008-05-25
 * @param T_RECT *rect: scroll bar display location rect
 * @param T_SCBAR *bar: scroll bar struct pionter
 * @return T_VOID
 * @retval 
 */
T_VOID ScBar_GetLocaRect2(T_RECT *rect, const T_SCBAR *bar);

T_VOID ScBar_GetUpIconRect(T_pRECT pRect, const T_SCBAR *pBar);
T_VOID ScBar_GetDownIconRect(T_pRECT pRect, const T_SCBAR *pBar);

#ifdef __cplusplus
}
#endif

#endif
