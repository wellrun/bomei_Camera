
#ifndef __ENG_DYNAMICFONT_H__
#define __ENG_DYNAMICFONT_H__

#include "anyka_types.h"
#include "Fwl_osMalloc.h"
#include "Eng_convert_unicode.h"
#include "Fwl_osFS.h"
#include "Eng_Debug.h"
#include "string.h"
#include "akdefine.h"
#include "fwl_display.h"

#ifndef _SUBAREA_INDEX
	#define _SUBAREA_INDEX
#endif

typedef struct cp_info T_CP_INFO;

typedef struct {
    unsigned char   id;
    unsigned char   bold;
    unsigned char   height;
    unsigned char   italic;
} T_FONT_TYPE;

//file head structure of LangFontLib.bin
typedef struct{
    unsigned char Version;          /**< font library version */
    unsigned char FontHeight;       /**< font height */
    unsigned short FontByteCount;    /**< char matrix impropriate FontByteCount bytes */
    //unsigned char rev1;
    T_FONT_TYPE   FontType;         /**< font type*/
    unsigned int  FontLibOffset;    /**< font library offset, font library file have two convert table and one font library */
    unsigned int  FontLibSize;      /**< font library size (the number of chars in lib)*/
    unsigned char rev2;
    unsigned char rev3;
    unsigned char rev4;
    unsigned char rev5;
} T_DYNAMIC_FONT_LIB_HEAD;

#ifdef _SUBAREA_INDEX
typedef struct {
	unsigned int  begin;
    unsigned int  end;
} T_FONT_AREA_INDEX;
#endif

/** font library manage struct */
typedef struct {
    T_pFILE             hFontFile;      /**< font lib file handle */
    unsigned int        CurItemId;      /**< Del Item id, if the dynamic font buffer use up, replace the item with a new item */
    unsigned int        TailItemId;     /**< last item id, if the dynamic font buffer don't use up, TailItemId record the last item */
    unsigned int        MaxItemNum;     /**< max quantity of char to be saved */
    unsigned short      *pUni2Font;     /**< unicode to font matrix table */
    unsigned short      *pFontId2Uni;   /**< font id to unicode table */
    unsigned char       *pFontBuf;      /**< font matrix */
    T_DYNAMIC_FONT_LIB_HEAD FontLibHead;
	
#ifdef _SUBAREA_INDEX
    T_FONT_AREA_INDEX   *pAreaIndex;
#endif
    unsigned char       rev1;
    unsigned char       rev2;
    unsigned char       rev3;
    unsigned char       rev4;
} T_DYNAMIC_FONT_MANAGE;

/** Dynamic Font struct */
typedef struct {
    union cptable                       Cptable;            /**< codepage table*/
    unsigned int                        CpInfoOffset;       /**< codepage table offset */
    T_DYNAMIC_FONT_MANAGE               *FontManage;         /**< font library manager */
    unsigned char                       LangNum;            /**<the num of supported languages*/
    T_BOOL                              InitFlag;           /** whether dynamic_font lib is inited */
    T_U8                                fontNum; 
} T_DYNAMIC_FONT;

extern T_DYNAMIC_FONT dynamic_font;

extern T_S32 Eng_StrMbcs2Ucs(const T_S8 *src, T_U16 *ucBuf);

/**DynamicFont lib interface****/
T_U16 DynamicFont_GetFontHeight(T_FONT font);
T_BOOL DynamicFont_FontLib_Init(T_pCSTR* FontPaths,T_U8 FontNum, T_U8 startFontSize);
T_BOOL DynamicFont_Codepage_Init(T_pCSTR CpPath, T_CP_INFO CpInfo, T_U8* LeadByte);
T_BOOL DynamicFont_Codepage_Free(T_VOID);
T_BOOL DynamicFont_FontLib_Free(T_VOID);
T_U16 DynamicFont_GetCharWidth(const T_U16 chr, T_FONT font);

T_VOID DynamicFont_DispString(DISPLAY_TYPE_DEV LCD, T_POS x, T_POS y, T_pCSTR string, T_U16 strLen, T_COLOR color, T_FONT font);
//display function of UNICODE
T_VOID DynamicFont_UDispSpeciString(DISPLAY_TYPE_DEV LCD, T_POS x, T_POS y, T_U16* disp_string, T_COLOR color, T_FONT font, T_U16 strLen );
T_VOID DynamicFont_UDispSpeciChar(DISPLAY_TYPE_DEV LCD, T_POS x, T_POS y, T_U16 chr, T_COLOR color, T_FONT font);
T_U32 DynamicFont_UGetSpeciStringWidth(T_U16 *pUniStr, T_FONT font, T_U16 strLen);
T_BOOL DynamicFont_UScrollDispString(T_U16* pUString, T_POS x, T_POS y, T_U16 UStrLen, T_U16 offset, T_U16 width_limit, T_COLOR color, T_FONT font);
T_U16 DynamicFont_GetUStringDispNum( T_U16* uStr, T_U16 strLen, T_U16 disp_width, T_FONT font);

T_VOID Eng_FontManager_Init(T_VOID);
T_BOOL Eng_FontLib_Init(T_VOID);
T_BOOL Eng_FontLib_ChkOpenBackFile(T_VOID);
T_BOOL Eng_Codepage_Init(T_VOID);
T_BOOL Eng_Codepage_ChkOpenBackFile(T_VOID);

T_VOID DynamicFont_UDispSpeciStringOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight, 
        T_POS x, T_POS y, T_U16* disp_string, T_COLOR color, T_FONT font, T_U16 strLen );
T_VOID DynamicFont_DispSpeciStringOnYUV(T_U8 *ybuf, T_U8 *ubuf, T_U8 *vbuf, T_U32 imgwidth, T_U32 imgheight,
        T_POS x, T_POS y, T_pCSTR disp_string, T_COLOR color, T_U16 font, T_U16 strLen);


T_VOID DynamicFont_UDispSpeciStringOnRGB(T_U8 *buf, T_U32 imgwidth, T_U32 imgheight, T_POS x, T_POS y, T_U16* disp_string, T_COLOR color, T_U8 colortype, T_FONT font, T_U16 strLen );


/******adapt for sword3 platform interface*/

T_U16 GetFontHeight(T_FONT font);
T_U16 GetFontWidth(T_FONT font);
T_U16 GetCharFontWidth(const T_U16 chr, T_FONT font);

T_BOOL	Fwl_DispString(HLAYER layer, T_POS x, T_POS y, T_pCSTR string, T_U16 strLen,
            T_COLOR color, T_FONT font);

//display function of UNICODE
T_VOID  Fwl_UDispString(HLAYER hLayer, T_POS x, T_POS y, T_U16* string, T_U16 strLen, T_COLOR color, T_FONT font);
T_BOOL	Fwl_UDispSpeciString(HLAYER layer, T_POS x, T_POS y, T_U16* disp_string,
            T_COLOR color, T_FONT font, T_U16 strLen );
T_BOOL	Fwl_UScrollDispString(HLAYER layer, T_U16* pUString, T_POS x, T_POS y, T_U16 UStrLen,
			T_U16 offset, T_U16 width_limit, T_COLOR color, T_FONT font);
T_U16   Fwl_GetUStringDispNum( T_U16* uStr, T_U16 strLen, T_U16 disp_width, T_FONT font);

T_VOID  UDispString(DISPLAY_TYPE_DEV LCD, T_POS x, T_POS y, T_U16* string, T_U16 strLen, T_COLOR color, T_FONT font);
T_U32   UGetSpeciStringWidth(T_U16 *pUniStr, T_FONT font, T_U16 strLen);



#define UDispSpeciStringOnYUV	DynamicFont_UDispSpeciStringOnYUV
#define DispSpeciStringOnYUV 	DynamicFont_DispSpeciStringOnYUV


#endif


