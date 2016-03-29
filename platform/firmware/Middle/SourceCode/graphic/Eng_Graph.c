/**
 * @file
 * @brief ANYKA software
 * This file is for windows graphic operation
 *
 * @author Baoli.Miao ZouMai
 * @date 2001-4-20
 * @version 1.0
 */

#include "Eng_Math.h"
#include "Gbl_Global.h"
#include "Eng_Graph.h"
#include "Fwl_pfDisplay.h"
#include "Eng_DynamicFont.h"
#include "Eng_font.h"
#include "Eng_AkBmp.h"
#include "Eng_String.h"
#include "Lib_res_port.h"
#include "stdlib.h"
#include "fwl_display.h"


#ifndef SUPPORT_TITLE       /* Display the title */
    #define SUPPORT_TITLE
#endif



T_GRAPH g_Graph;
static T_U32 NumResInit = 0;


/**
 * @brief Convert RGB format to T_COLOR(T_U32)
 * 
 * @author ZouMai
 * @date    2002-10-10 
 * @param T_U8 r: red
 * @param T_U8 g: green
 * @param T_U8 b: blue
 * @param T_U8 deep: color deep
 * @return T_COLOR
 * @retval T_COLOR value
 */
T_COLOR RGB2AkColor(T_U8 r, T_U8 g ,T_U8 b, T_U8 deep)
{
    if (deep == 24)         
    {
        return (r<<16 | (g<<8) | (b));
    }
    else if (deep == 12 || deep == 1) //Anyka VME will do conversion
    {
        return ((b&0xF0) | ((g&0xF0)<<4) | ((r&0xF0)<<8));
    }
    else if(deep == 16)
    {
        return(((b&0xF8)>>3) | ((g&0xFC)<<3) | ((r&0xF8)<<8));
    }
    
    return 0;
}

/**
 * @brief Convert T_COLOR(T_U32) to RGB format
 * 
 * @author ZouMai
 * @date    2002-10-10 
 * @param T_COLOR: T_COLOR format color
 * @param T_U8 deep: color deep
 * @param T_pDATA r: red for return
 * @param T_pDATA g: green for return
 * @param T_pDATA b: blue for return
 * @return T_VOID
 * @retval
 */
T_VOID  AkColor2RGB(T_COLOR color, T_U8 deep, T_pDATA r, T_pDATA g ,T_pDATA b)
{
    AK_ASSERT_PTR_VOID(r, "AkColor2RGB(): r");
    AK_ASSERT_PTR_VOID(g, "AkColor2RGB(): g");
    AK_ASSERT_PTR_VOID(b, "AkColor2RGB(): b");

    if (deep == 24)
    {
        *b = (T_U8)(color & 0xff);
        *g = (T_U8)((color>>8) & 0xff);
        *r = (T_U8)((color>>16) & 0xff);
        return;
    }
    else if (deep == 12 || deep == 1)   /* color: 0b00000000 00000000 BBBBGGGG RRRR0000. It seems different from siemens spic */
    {
        *b = (T_U8)(color);
        *g = (T_U8)((color>>8) << 4);
        *r = (T_U8)((color>>12) << 4);
        return;
    }
    else if (deep == 16)
    {
        *b = (T_U8)(color<<3);
        *g = (T_U8)((color>>5) << 3);
        *r = (T_U8)((color>>10) << 3);
        return;
    }
    
    return;
}

T_VOID Fwl_GetNumberRes(T_VOID)
{
    NumResInit++;
}

T_VOID Fwl_FreeNumberRes(T_VOID)
{
    if (NumResInit > 0)
        NumResInit--;
}

/* graphics operation */
/**
 * @brief Draw a num, must call Fwl_GetNumberRes() function and end call Fwl_FreeNumberRes() function.
 * 
 * @author Liu Zhenwu
 * @date 2006-3-07
 * @param HLAYER hLayer    handle of layer
 * @param str display string pattern
 * @param seperator which char is the seperator
 * @return T_VOID
 * @retval void
 */

T_VOID Fwl_DrawNumber(HLAYER hLayer, T_STR_INFO str, T_POS left, T_POS top)
{
    // must call Fwl_GetNumberRes() first, if not use the number, call Fwl_FreeNumberRes
    T_U32 tmpLen;
    T_U8 tmpDeep;
    T_LEN numWidth, seperatorWidth,tmpHeight;
    T_pDATA seperatorData;
    T_U16 i;

    // get number resource first, free the number resource after draw the number
    AK_ASSERT_VAL_VOID((NumResInit != 0), "Fwl_DrawNumber(): Number Res is empty");

    AKBmpGetInfo(Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_NUMBER_0, &tmpLen), &numWidth, &tmpHeight, &tmpDeep);
    for (i=0;i<Utl_StrLen(str);i++)
    {
        if ('0'<=str[i] && str[i]<='9')
        {
            Fwl_AkBmpDrawFromString(hLayer, left, top, Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_NUMBER_0 + (str[i] - '0'), &tmpLen), &g_Graph.TransColor, AK_FALSE);
            left+= numWidth + 1;
            continue;
        }

        switch (str[i])
        {
            case ':':
                seperatorData = (T_pDATA)Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_COLON, &tmpLen);
                break;
            case '-':
                seperatorData = (T_pDATA)Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_SUB, &tmpLen);
                break;
            case '/':
                seperatorData = (T_pDATA)Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_SLASH, &tmpLen);
                break;
            case '.':
                seperatorData = (T_pDATA)Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_DOT, &tmpLen);
                break;
            default:
                seperatorData = AK_NULL;
                break;
        }

        if (seperatorData != AK_NULL)
        {
            AKBmpGetInfo(seperatorData, &seperatorWidth, &tmpHeight, &tmpDeep);
            Fwl_AkBmpDrawFromString(hLayer, left, top, seperatorData, &g_Graph.TransColor, AK_FALSE);
            left+= seperatorWidth + 1;
        }
        else
            left += numWidth + 1;           
    }
}




/**
 * @brief Draw dialog frame
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_POS left Left to the rect.
 * @param  T_POS top Top to the rect.
 * @param  T_LEN width Rect width.
 * @param  T_LEN height Rect height.
 * @param  T_U8 flag: user can use flag to identify the frame line: 0x01 -- 0x0F
 * @return T_VOID
 * @retval void
 */
T_VOID Fwl_DialogFrame(HLAYER hLayer, T_POS left, T_POS top, T_LEN width, T_LEN height, T_U8 flag)
{
    if ((flag & 0x01) == 0x01)  /* draw dialog frame: top line */
    {
        Fwl_DrawLine(hLayer, left, top, (T_POS)(left+width-1), top, RGB2AkColor(121,121,121,LCD0_COLOR));
        Fwl_DrawLine(hLayer, left, (T_POS)(top+1), (T_POS)(left+width-1), (T_POS)(top+1), RGB2AkColor(204,204,204,LCD0_COLOR));
    }
    if ((flag & 0x04) == 0x04)  /* draw dialog frame: left line */
    {
        Fwl_DrawLine(hLayer, left, top, left, (T_POS)(top+height-1), RGB2AkColor(121,121,121,LCD0_COLOR));
        Fwl_DrawLine(hLayer, (T_POS)(left+1), top, (T_POS)(left+1), (T_POS)(top+height-1), RGB2AkColor(204,204,204,LCD0_COLOR));
    }
    if ((flag & 0x08) == 0x08)  /* draw dialog frame: right line */
    {
        Fwl_DrawLine(hLayer, (T_POS)(left+width-1), top, (T_POS)(left+width-1), (T_POS)(top+height-1), RGB2AkColor(121,121,121,LCD0_COLOR));
        Fwl_DrawLine(hLayer, (T_POS)(left+width-2), top, (T_POS)(left+width-2), (T_POS)(top+height-1), RGB2AkColor(204,204,204,LCD0_COLOR));
    }
    if ((flag & 0x02) == 0x02)  /* draw dialog frame: bottom line */
    {
        Fwl_DrawLine(hLayer, left, (T_POS)(top+height-1), (T_POS)(left+width-1), (T_POS)(top+height-1), RGB2AkColor(121,121,121,LCD0_COLOR));
        Fwl_DrawLine(hLayer, (T_POS)(left+1), (T_POS)(top+height-2), (T_POS)(left+width-3), (T_POS)(top+height-2), RGB2AkColor(204,204,204,LCD0_COLOR));
    }
}

/**
 * @brief Fill a appointed rectangle with a specified color.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param T_eLCD LCD
 * @return T_VOID
 * @retval 
 */
T_VOID CleanMainScreen(T_VOID)
{
    Fwl_FillSolidRect(HRGB_LAYER, 0, g_Graph.LCDTBHEI[DISPLAY_LCD_0], Fwl_GetLcdWidth(), g_Graph.LCDMSHEI[DISPLAY_LCD_0], g_Graph.WinBkCL[DISPLAY_LCD_0]);
} /* end CleanMainScreen(T_VOID) */

/**
 * @brief init a rectangle size
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-07-06 
 * @param T_RECT *rect
 * @param  T_POS left
 * @param  T_POS top
 * @param  T_LEN width
 * @param  T_LEN height
 * @return T_RECT *
 * @retval 
 */
T_RECT *RectInit(T_RECT *rect, T_POS left, T_POS top, T_LEN width, T_LEN height)
{
    AK_ASSERT_PTR(rect, "RectInit(): rect", AK_NULL);

    rect->left = left;
    rect->top = top;
    rect->width = width;
    rect->height = height;
    return rect;
}

/**
 * @brief Deceide if a point is in the rectangle 
 * 
 * @author @b ZouMai
 * 
 * @author 
 * @date 2002-07-06
 * @param T_RECT *rect
 * @param  T_POS x
 * @param  T_POS y
 * @return T_BOOL
 * @retval 
 */
T_BOOL PointInRect(const T_RECT *rect, T_POS x, T_POS y)
{
    AK_ASSERT_PTR(rect, "PointInRect(): rect", AK_FALSE);

    if(x >= rect->left && x < rect->left + rect->width &&
       y >= rect->top  && y < rect->top + rect->height)
       return AK_TRUE;
    return AK_FALSE;
}

T_BOOL PointInDisk(T_POS center_x, T_POS center_y, T_POS radii,  T_POS x, T_POS y)
{
    T_POS left, right, top, bottom; 

    left = center_x - radii;
    top = center_y - radii;
    right = center_x + radii;
    bottom = center_y + radii;

    if ((x >= left) && (x <= right) && (y >= top) && (y <= bottom))
        if ( abs(x - center_x)*abs(x - center_x) + abs(y - center_y)*abs(y - center_y) <= radii*radii)
           return AK_TRUE;

    return AK_FALSE;
}


T_VOID GraphInit(T_VOID)
{  
    T_pCDATA pImg;
    T_U32    len;
    
    /*init lcd*/
    g_Graph.LCD_NUM = MAX_LCD_NUM;
    //Fwl_GetLcdWidth() = MAIN_LCD_WIDTH;
    //Fwl_GetLcdHeight() = MAIN_LCD_HEIGHT;

	

    if (g_Graph.LCDCOLOR[DISPLAY_LCD_0] == 1)
    {
        g_Graph.WinFrCL[DISPLAY_LCD_0] = RGB2AkColor(0, 0, 0, LCD0_COLOR);
        g_Graph.WinBkCL[DISPLAY_LCD_0]  = RGB2AkColor(255, 255, 255, LCD0_COLOR);
        g_Graph.FocusFrCL = RGB2AkColor(0, 0, 0, LCD0_COLOR);
        g_Graph.FocusBkCL = RGB2AkColor(0, 0, 0, LCD0_COLOR);
        //g_Graph.MenuBkCL = RGB2AkColor(211, 229, 250, LCD0_COLOR);
        g_Graph.MenuBkCL = RGB2AkColor(255, 255, 255, LCD0_COLOR);
        g_Graph.TtlFrCL = RGB2AkColor(255, 255, 255, LCD0_COLOR);
        g_Graph.TtlBkCL = RGB2AkColor(249, 249, 249, LCD0_COLOR);
        g_Graph.ScBar_FrCL = RGB2AkColor(0, 0, 0, LCD0_COLOR);
        g_Graph.ScBar_BkCL = RGB2AkColor(255, 255, 255, LCD0_COLOR);
        g_Graph.DisableCL = RGB2AkColor(255, 255, 255, LCD0_COLOR);
    }
    else
    {
        g_Graph.WinFrCL[DISPLAY_LCD_0] = RGB2AkColor(0, 0, 0, LCD0_COLOR);
        g_Graph.WinBkCL[DISPLAY_LCD_0]  = RGB2AkColor(241, 241, 241, LCD0_COLOR);
        g_Graph.FocusFrCL = RGB2AkColor(255, 255, 255, LCD0_COLOR);
        g_Graph.FocusBkCL = RGB2AkColor(48, 96, 197, LCD0_COLOR);
        //g_Graph.MenuBkCL = RGB2AkColor(211, 229, 250, LCD0_COLOR);
        g_Graph.MenuBkCL = RGB2AkColor(255, 255, 255, LCD0_COLOR);
        g_Graph.TtlFrCL = RGB2AkColor(255, 255, 255, LCD0_COLOR);
        g_Graph.TtlBkCL = RGB2AkColor(160, 183, 241, LCD0_COLOR);
        g_Graph.ScBar_FrCL = RGB2AkColor(75, 76, 158, LCD0_COLOR);
        g_Graph.ScBar_BkCL = RGB2AkColor(211, 229, 250, LCD0_COLOR);
        g_Graph.DisableCL = RGB2AkColor(190, 160, 200, LCD0_COLOR);
    }
/*
    if (g_Graph.LCD_NUM >= 2)
    {
        if (g_Graph.LCDCOLOR[LCD_1] == 1)
        {
            g_Graph.WinFrCL[LCD_1] = RGB2AkColor( 0, 0, 0, LCD1_COLOR );
            g_Graph.WinBkCL[LCD_1]  = RGB2AkColor(255, 255, 255, LCD1_COLOR);
        }
        else
        {
            g_Graph.WinFrCL[LCD_1] = RGB2AkColor(0, 0, 0, LCD1_COLOR);
            g_Graph.WinBkCL[LCD_1]  = RGB2AkColor(255, 255, 255, LCD1_COLOR);
        }
    }
*/
    g_Graph.TransColor = RGB2AkColor(255,0,255,LCD0_COLOR);

    //Fwl_GetLcdWidth() = MAIN_LCD_WIDTH;
    //Fwl_GetLcdHeight() = MAIN_LCD_HEIGHT;
    g_Graph.LCDCOLOR[DISPLAY_LCD_0] = LCD0_COLOR;           /* color deep */

#ifdef SUPPORT_STATESBAR
    g_Graph.LCDTBHEI[DISPLAY_LCD_0] = 0;    /* top icon bar height */
#else
    g_Graph.LCDTBHEI[DISPLAY_LCD_0] = 0;
#endif
    //g_Graph.LCDTBHEIINSTANDBY[DISPLAY_LCD_0] = LCD0_TOPBAR_HEIGHT;

    g_Graph.TBWIDTH = 16;       /* tab bar width */

    g_Graph.STATEBARHEIGHT = g_Font.CHEIGHT;
#ifdef SUPPORT_TITLE
    pImg = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_PUB_TITLE, &len);
    if (pImg != AK_NULL)
    {
        AKBmpGetInfo(pImg, AK_NULL, &(g_Graph.TTLHEIGHT), AK_NULL);
    }
    else
    {
        g_Graph.TTLHEIGHT = 22;
    }
#else
    g_Graph.TTLHEIGHT = 0;
#endif
    /*pImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_LSCBAR_BACKGROUND, &len);
    if (pImg != AK_NULL)
    {
        AKBmpGetInfo(pImg, &g_Graph.LScBarWidth, AK_NULL, AK_NULL);
    }*/
    // little scroll bar width is 8
    g_Graph.LScBarWidth = 12;
    pImg = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_BSCBAR_BACKGROUND, &len);
    if (pImg != AK_NULL)
    {
        AKBmpGetInfo(pImg, &g_Graph.BScBarWidth, AK_NULL, AK_NULL);
    }

#ifdef SUPPORT_SOFTKEY
    g_Graph.SKHEIGHT = g_Font.CHEIGHT + 8;
#else
    g_Graph.SKHEIGHT = 0;
#endif

    g_Graph.LCDMSHEI[DISPLAY_LCD_0] = Fwl_GetLcdHeight()-g_Graph.LCDTBHEI[DISPLAY_LCD_0];
    g_Graph.LCDCOL[DISPLAY_LCD_0]   = (T_U16)(Fwl_GetLcdWidth() / g_Font.CWIDTH);

    g_Graph.LCWIDTH = 12;
    g_Graph.LCHEIGHT = 24;
}
