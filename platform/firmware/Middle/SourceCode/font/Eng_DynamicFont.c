
#include "Eng_DynamicFont.h"
#include "Gbl_Global.h"
#include "Eng_font.h"
#include "eng_debug.h"
#include "Eng_String_UC.h"


#include "fwl_display.h"
#include "ImageLayer.h"
#include "eng_freetype.h"

/// Fontlib
#ifdef SPIBOOT
T_pCSTR pFontLib_Path[] =  {
    "FONT", 
    "FONT",
};
#else //nand boot/ sd boot
T_pCSTR pFontLib_Path[] = {
    DRI_A"DynamicFont4_16.bin", //default    font    
    DRI_A"DynamicFont4_12.bin",
};
T_pCSTR pFontLib_Path_Bak[] =  {
    DRI_C"DynamicFont4_16.bin", //bakeup  font
    DRI_C"DynamicFont4_12.bin",
};
#endif


/// codepage
#ifdef SPIBOOT
const char DefLangCodepageFile[] = {"LANG"};
#else
const char DefLangCodepageFile[] = {DRI_A"LangCodepage.bin"};
const char DefLangCodepageFileBak[] = {DRI_C"LangCodepage.bin"};
#endif

T_DYNAMIC_FONT dynamic_font;

T_BOOL Eng_FontLib_ChkOpenBackFile(T_VOID)
{
#ifndef SPIBOOT

    T_BOOL ret = AK_FALSE;

	if (FONT_12 == gs.FontSize)
	{
	    ret = Fwl_ChkOpenBackFile((T_U8*)pFontLib_Path[1], (T_U8*)pFontLib_Path_Bak[1]);
	    if (!ret)
	    {
	        Fwl_Print(C2, M_FONT, "Eng_FontLib_Init():Fwl_ChkOpenBackFile font12 fail!");
	    }
    }

    if (FONT_16 == gs.FontSize)
    {
	    ret = Fwl_ChkOpenBackFile((T_U8*)pFontLib_Path[0], (T_U8*)pFontLib_Path_Bak[0]);
	    if (!ret)
	    {
	        Fwl_Print(C2, M_FONT, "Eng_FontLib_Init():Fwl_ChkOpenBackFile font16 fail!");
	    }
    }

    return ret;
#endif

	return AK_FALSE;
}

T_BOOL Eng_Codepage_ChkOpenBackFile(T_VOID)
{
#ifndef SPIBOOT

    T_BOOL ret = AK_FALSE;

    ret = Fwl_ChkOpenBackFile((T_U8*)DefLangCodepageFile, (T_U8*)DefLangCodepageFileBak);
    if (!ret)
    {
        Fwl_Print(C2, M_FONT, "Eng_Codepage_Init:Fwl_ChkOpenBackFile:LangCodepage,fail");
    }    

	return ret;
#endif

	return AK_FALSE;
}

T_VOID Eng_FontManager_Init(T_VOID)
{
    memset((T_pVOID)(&dynamic_font), 0, sizeof(T_DYNAMIC_FONT));
    dynamic_font.InitFlag = AK_FALSE;
}

T_BOOL Eng_FontLib_Init(T_VOID)
{
    T_BOOL ret = AK_FALSE;
    
    ret = DynamicFont_FontLib_Init(pFontLib_Path, 1, gs.FontSize);

    if (AK_FALSE == ret)
    {
        Fwl_Print(C1, M_FONT, "DynamicFont_FontLib_Init() fail!");
    }
    
    return ret;
}

//T_BOOL DynamicFont_Codepage_Init(T_pSTR codePagePath, struct cp_info  info, T_U8* leadByte);
T_BOOL Eng_Codepage_Init(T_VOID)
{

    T_BOOL ret = AK_FALSE;
    T_CODE_PAGE     code_page;

    /*Get language ID*/
    switch (gs.Lang)
    {
        case eRES_LANG_CHINESE_SIMPLE:
        case eRES_LANG_CHINESE_TRADITION:
        case eRES_LANG_ENGLISH:
            code_page = CP_936;
            break;
        case eRES_LANG_CHINESE_BIG5:
            code_page = CP_950;
            break;
        case eRES_LANG_THAI:
            code_page = CP_874;
            break;
        case eRES_LANG_SPAIN:
            code_page = CP_1252;
            break;
        default:
            code_page = CP_936;
            break;
    }

    switch(code_page)
    {
        case CP_950:
            {
                struct cp_info    info = { 950, 2, 0x003f, 0x003f, "ANSI/OEM Traditional Chinese Big5" };
                T_U8 lead_bytes[12] = { 0x81, 0xfe, 0x00, 0x00 };     /* lead bytes ranges */
                ret=DynamicFont_Codepage_Init(DefLangCodepageFile,info,lead_bytes);
            }
            break;
        case CP_874: 
            {
                struct cp_info    info = { 874, 1, 0x003f, 0x003f, "ANSI/OEM Thai" };
                ret=DynamicFont_Codepage_Init(DefLangCodepageFile,info,NULL);
            }
            break;
        case CP_1252:
            {
                struct cp_info    info = { 1252, 1, 0x003f, 0x003f, "ANSI Latin 1" };
                ret=DynamicFont_Codepage_Init(DefLangCodepageFile,info,NULL);
            }
            break;
        case CP_936:
        default:
            {
                struct cp_info    info = { 936, 2, 0x003f, 0x003f, "ANSI/OEM Simplified Chinese GBK" };
                T_U8 lead_bytes[12] = { 0x81, 0xfe, 0x00, 0x00 };     /* lead bytes ranges */
                ret=DynamicFont_Codepage_Init(DefLangCodepageFile,info,lead_bytes);
            }
            break;      
    }
    
    if (AK_FALSE == ret)
    {
        Fwl_Print(C2, M_FONT, "DynamicFont_Codepage_Init() fail! codepage=%d",code_page);
    }
    return ret;
}


/*****for sword3 platform interface****/

T_U16 GetFontHeight(T_FONT font)
{
#ifdef SUPPORT_VFONT
    if(gs.VF_FontInstalled && gb.bIsUseVFont)
    {
        return gs.VF_FontSize;
    }
    else
    {
#endif
        return DynamicFont_GetFontHeight(font);
#ifdef SUPPORT_VFONT
    }
#endif
}

T_U16 GetCharFontWidth(const T_U16 chr, T_FONT font)
{
#ifdef SUPPORT_VFONT
        if(gs.VF_FontInstalled && gb.bIsUseVFont)
        {
            return VF_GetCharWidth(chr);
        }
        else
        {
#endif
            return DynamicFont_GetCharWidth(chr, font);
#ifdef SUPPORT_VFONT
        }
#endif

    
}
/**
 * @brief Display string in the appointed position.
 * 
 * @author ZouMai
 * @date 2001-4-20
 * @param HLAYER layer: handle of layer
 * @param T_POS x: the x coordiantion to draw the string
 * @param T_POS y: the y coordiantion to draw the string
 * @param T_pCSTR string: pointer to the string.
 * @return T_VOID
 * @retval 
 */
T_BOOL    Fwl_DispString(HLAYER layer, T_POS x, T_POS y, T_pCSTR string, T_U16 strLen,
                T_COLOR color, T_FONT font)
{
    T_POINT point;

    point.x = x;
    point.y = y;

    return ImgLay_DispString( layer,  point,  string,  strLen,
                 color,  font);
}
    

/**
 * @brief Display unicode string in the appointed position.
 * 
 * @author Junhua Zhao
 * @date 2005-08-08
 * @param T_eLCD LCD:
 * @param T_POS x: the x coordiantion to draw the string
 * @param T_POS y: the y coordiantion to draw the string
 * @param T_U16* string: pointer to the string.
 * @return T_VOID
 * @retval 
 */
T_BOOL    Fwl_UDispSpeciString(HLAYER layer, T_POS x, T_POS y, T_U16* disp_string,
                T_COLOR color, T_FONT font, T_U16 strLen )
{
    T_POINT point;

    point.x = x;
    point.y = y;

    return ImgLay_UDispSpeciString( layer,  point,  disp_string,
                 color,  font,  strLen );

}

T_VOID Fwl_UDispString(HLAYER hLayer, T_POS x, T_POS y, T_U16* disp_string, T_U16 strLen, T_COLOR color, T_FONT font)
{
    Fwl_UDispSpeciString(hLayer, x, y, disp_string, color, font, strLen);    
}



//for ebookcore.c
T_VOID UDispString(DISPLAY_TYPE_DEV LCD, T_POS x, T_POS y, T_U16* disp_string, T_U16 strLen, T_COLOR color, T_FONT font)
{
    Fwl_UDispSpeciString(HRGB_LAYER, x, y, disp_string, color, font, strLen);    
}


/**
 * @brief Get the total width of unicode string.
 * 
 * @author zhengwenbo
 * @date 2006-12-30
 * @param pUniStr: unicode string
 * @param  font: font info
 * @param  strLen: the length of unicode string
 * @return T_U32
 * @retval the length of unicode string
 */
T_U32 UGetSpeciStringWidth(T_U16 *pUniStr, T_FONT font, T_U16 strLen)
{
    T_U32 textWidth = 0;

    AK_ASSERT_PTR(pUniStr, "Eng_GetTextWidthGB gbText is null", 0);

#ifdef SUPPORT_VFONT
    if(gs.VF_FontInstalled && gb.bIsUseVFont)
    {
        T_U16 i = 0;

#ifdef OS_ANYKA
        T_USTR_FILE     path = {0};

        Eng_StrMbcs2Ucs(gs.VF_InstalledPath, path);
        
        if (!Fwl_CheckDriverIsValid(path))
        {
            return 0;
        }
#endif


        for (i=0; i<strLen; i++)
        {
            textWidth += GetCharFontWidth(pUniStr[i], font);
        }
    }
    else
    {
#endif
        textWidth = DynamicFont_UGetSpeciStringWidth(pUniStr, font, strLen);
#ifdef SUPPORT_VFONT
    }
#endif

    return textWidth;
}

/**
 * @brief scroll-display unicode string.
 * 
 * @author zhengwenbo
 * @date 2006-01-10
 * @param pUString: unicode string to be displayed
 * @param  T_POS x
 * @param  T_POS y
 * @param  UStrLen: the length of unicode string
 * @param  T_U16 offset: the offset to begin displaying
 * @param  T_U16 width_limit: the limit of displaying width
 * @return T_BOOL
 * @retval AK_TRUE: success AK_FALSE: fail
 */
T_BOOL    Fwl_UScrollDispString(HLAYER layer, T_U16* pUString, T_POS x, T_POS y, T_U16 UStrLen,
                T_U16 offset, T_U16 width_limit, T_COLOR color, T_FONT font)
{
    T_POINT point;

    point.x = x;
    point.y = y;

    return ImgLay_UScrollDispString( layer,  point,  pUString,    UStrLen,
                 offset,  width_limit,    color,    font);

}


/**
 * @brief Get the number of special string according displaying width
 * 
 * @author zhengwenbo
 * @date 2006-12-30
 * @param uStr: unicode string that is got length
 * @param font: font info
 * @param strLen: the length fo unicode string
 * @param disp_width: the displaying range
 * @return T_U32 
 * @retval  the num of unicode string to display
*/
T_U16 Fwl_GetUStringDispNum( T_U16* uStr, T_U16 strLen, T_U16 disp_width, T_FONT font)
{
    return DynamicFont_GetUStringDispNum(uStr, strLen, disp_width, font);
}

