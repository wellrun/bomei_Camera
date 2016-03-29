/**
 * @file Ctl_Progress.h
 * @brief This header file is for progress bar function prototype
 * @author: ZouMai
 */

#ifndef __UTL_PROGRESSBAR_H__
/**
 * @def __UTL_PROGRESSBAR_H__
 *
 */
#define __UTL_PROGRESSBAR_H__

/**
 * @include "ctl_global.h"
 *
 */
#include "ctl_global.h"
#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def PGBAR_PILE_OFF                  0x0000
 * Define pile mode, use bit 0 of a T_U16 integer number
 */
#define PGBAR_PILE_OFF                  0x0000
/**
 * @def PGBAR_PILE_ON                   0x0001
 *
 */
#define PGBAR_PILE_ON                   0x0001

/**
 * @def PGBAR_DIR_L2R                   0x0000
 * Define bar direction, use bits 4--7 of a T_U16 integer number
 */
#define PGBAR_DIR_L2R                   0x0000
/**
 * @def PGBAR_DIR_R2L                   0x0010
 *
 */
#define PGBAR_DIR_R2L                   0x0010
/**
 * @def PGBAR_DIR_U2D                   0x0020
 *
 */
#define PGBAR_DIR_U2D                   0x0020
/**
 * @def PGBAR_DIR_D2U                   0x0030
 *
 */
#define PGBAR_DIR_D2U                   0x0030

/**
 * @def PGBAR_STYLE_1                   0x0000
 * Define bar style, use bits 8--11 of a T_U16 integer number
 * Default: has rectangle frame
 */
#define PGBAR_STYLE_1                   0x0000
/**
 * @def PGBAR_STYLE_2                   0x0100
 * has background block
 */
#define PGBAR_STYLE_2                   0x0100

typedef struct {
    T_U16           pileMode;       /* Pile or not */
    T_U16           dirMode;        /* Direction, SCBAR_VERTICAL or SCBAR_HORIZONTAL */
    T_U16           styleMode;      /* progress bar style */

    T_U16           initFlag;       /* identify the control is initialized or not */
    T_POS           Left;           /*  location of progress bar */
    T_POS           Top;
    T_LEN           Width;
    T_LEN           Height;

    T_S16           Side;           /* Side height and width */
    T_S16           Intvl;          /* scroll bar unit height or width */
    T_S8            Drag;           /* in drag mode or not */
    T_S16           DragOffset;     /* identify mouse offset when draging */

    T_COLOR         frColor;        /* progress bar color */
    T_COLOR         bkColor;
    T_COLOR         fmColor;

    T_U16           CurValue;       /* current selected page */
    T_U16           TtlValue;       /* current total page number */
} T_PGBAR;

T_VOID  PgBar_Init(T_PGBAR *bar, T_POS left, T_POS top, T_LEN width, T_LEN height, T_S16 intvl, T_U16 mode);
T_VOID  PgBar_SetLoc(T_PGBAR *bar, T_POS left, T_POS top, T_LEN width, T_LEN height);
T_VOID  PgBar_SetValue(T_PGBAR *bar, T_U16 cur, T_U16 total);
T_VOID  PgBar_SetCurValue(T_PGBAR *bar, T_S16 cur);
T_VOID  PgBar_SetColor(T_PGBAR *bar, T_COLOR frcolor, T_COLOR bkcolor, T_COLOR fmcolor);

T_VOID  PgBar_Show(T_PGBAR *bar);

#ifdef __cplusplus
}
#endif

#endif
