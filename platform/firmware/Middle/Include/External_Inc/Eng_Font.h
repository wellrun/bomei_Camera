/**
 * @file Eng_DispStr.h
 * @brief This header file is for display function prototype
 * 
 */
#ifndef __ENG_DISP_STR_H__
#define __ENG_DISP_STR_H__

#include "anyka_types.h"
#include "Eng_font.h"

#define FONT_BOLD           0x0010
#define FONT_ITALIC         0x0020
#define FONT_BOLDITALIC     0x0030
#define FONT_UNDERLINE      0x0100
#define FONT_STRIKETHR      0x0200


//#define FONT_HEIGHT_DEFAULT dynamic_font.FontManage[CURRENT_FONT_SIZE].FontLibHead.FontHeight
#define FONT_HEIGHT_DEFAULT (GetFontHeight(CURRENT_FONT_SIZE))

typedef enum {   
    FONT_16 = 0,
    FONT_12,
    FONTLIB_NUM
} T_eFONT;


typedef struct {
    T_U8    cWidth;
    T_U8    cHeight;        /* Chinese Char Width and Height */
    T_U8    eWidth;
    T_U8    eHeight;        /* English Char Width and Height */
} T_FONT_SIZE;

typedef struct
{
    T_LEN               CWIDTH;
    T_LEN               CHEIGHT;
    T_LEN               SCWIDTH;
    T_LEN               SCHEIGHT;
} T_FONT_DIMENSION;

extern T_FONT_DIMENSION g_Font;
extern T_U8 CURRENT_FONT_SIZE;

T_VOID FontInit(T_VOID);

T_VOID FontResize(T_U8 size);

#endif

