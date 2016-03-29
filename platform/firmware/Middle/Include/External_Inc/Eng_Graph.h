
/**
 * @file Eng_Graph.h
 * @brief This header file is for display function prototype
 * 
 */

#ifndef __ENG_GRAPH_H__
#define __ENG_GRAPH_H__

#include "anyka_types.h"
#include "akdefine.h"
#include "gbl_global.h"
#include "config.h"
#include "fwl_display.h"

/**Color table*/
#define COLOR_SKYBLUE     0x0087CEEB        /*skyblue:RGB(135, 206, 235)*/       
#define COLOR_ORANGE      0x00FF4500        /*orange:RGB(255, 69, 0)*/
#define COLOR_DARKGREY    0x00A9A9A9        /*DarkGrey:RGB(169, 169, 169)*/
#define COLOR_ROYALBLUE1  0x004876FF        /*royalblue(72 118 255)*/

typedef enum {
    COLOR_MODE_RGB565 = 0,
    COLOR_MODE_RGB888
}T_COLOR_MODE;


T_VOID Fwl_GetNumberRes(T_VOID);


T_COLOR RGB2AkColor(T_U8 r, T_U8 g, T_U8 b, T_U8 deep);
T_VOID  AkColor2RGB(T_COLOR color, T_U8 deep, T_pDATA r, T_pDATA g ,T_pDATA b);

T_VOID Fwl_FreeNumberRes(T_VOID);

T_VOID   Fwl_DrawNumber(HLAYER hLayer, T_STR_INFO str, T_POS left, T_POS top);
T_VOID  Fwl_DialogFrame(HLAYER  hLayer, T_POS left, T_POS top, T_LEN width, T_LEN height, T_U8 flag);
T_VOID  CleanMainScreen(T_VOID);

T_RECT  *RectInit(T_RECT *rect, T_POS left, T_POS top, T_LEN width, T_LEN height);
T_BOOL PointInRect(const T_RECT *rect, T_POS x, T_POS y);
T_BOOL PointInDisk(T_POS center_x, T_POS center_y, T_POS radii,  T_POS x, T_POS y);

typedef struct
{
    T_COLOR WinFrCL[MAX_LCD_NUM];       /* windows fore color */
    T_COLOR WinBkCL[MAX_LCD_NUM];       /* windows background color */
    T_COLOR FocusFrCL;      /* focus fore color */
    T_COLOR FocusBkCL;      /* foucs background color */
    T_COLOR MenuBkCL;       /* menu background color */
    T_COLOR TtlFrCL;        /* title fore color */
    T_COLOR TtlBkCL;        /* title background color */
    T_COLOR ScBar_FrCL;     /* scroll bar fore color */
    T_COLOR ScBar_BkCL;     /* scroll bar background color */
    T_COLOR SKFrCL;         /* soft key fore color */
    T_COLOR SKBkCL;         /* soft key background color */
    T_LEN   LScBarWidth;    /* little scroll bar width */
    T_LEN   BScBarWidth;    /* big scroll bar width */
    T_COLOR TopbarBkCL[MAX_LCD_NUM];    /* Top bar background color */
    T_COLOR DisableCL;      /* disable color */
    T_COLOR TransColor;     /* Transparent Color */

    T_U8                LCD_NUM;
    T_LEN               LCDWIDTH[MAX_LCD_NUM];
    T_LEN               LCDHEIGHT[MAX_LCD_NUM];
    T_U8                LCDCOLOR[MAX_LCD_NUM];

    T_LEN               LCDMSHEI[MAX_LCD_NUM];  /* main screen height */
    T_LEN               LCDTBHEI[MAX_LCD_NUM];  /* top icon bar height */
    //T_LEN             LCDTBHEIINSTANDBY[MAX_LCD_NUM]; /* top icon bar height */
    T_U16               LCDCOL[MAX_LCD_NUM];

    T_LEN               LCWIDTH;
    T_LEN               LCHEIGHT;

    T_LEN               TBWIDTH;        /* Tab bar width */
    T_LEN               TTLHEIGHT;      /* Title height */
    T_LEN               SKHEIGHT;       /* softkey height */
    T_LEN               STATEBARHEIGHT; /* state bar height */
} T_GRAPH;

extern T_GRAPH g_Graph;

T_VOID GraphInit(T_VOID);

#endif

