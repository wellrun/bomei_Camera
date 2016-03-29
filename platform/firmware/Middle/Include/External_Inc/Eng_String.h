/**
 * @file Eng_String.h
 * @brief ANYKA software
 * this header file provide hal layer string function 
 */

#ifndef __ENG_STRING_H__
#define __ENG_STRING_H__

#include "anyka_types.h"
#include "Gbl_Global.h"
#include "eng_string_uc.h"

#define DMA_MEMTYPE_RAM         0x00    // SDRAM, SSRAM, SRAM
#define DMA_MEMTYPE_NORFLASH    0x01    // NOR FLASH
#define DMA_MEMTYPE_NANDFLASH   0x02    // NAND FLASH
#define DMA_MEMTYPE_SRAMLIKE    0x03    // SRAM-LIKE

#define DMA_MAX_LEN             65535   // 0xFFFF
#define DMA_MIN_LEN             64      // DMA最小长度为64?

#define MAX_STRING_LEN          1024000
#define MAX_STR_LINE            1000

typedef enum {    
    FILE_TYPE_BMP = 0,
    FILE_TYPE_JPG,
    FILE_TYPE_JPEG,
    FILE_TYPE_JPE,
    FILE_TYPE_PNG,
    FILE_TYPE_GIF,
    FILE_TYPE_MJ,
    FILE_TYPE_MJPG,
    FILE_TYPE_MJPEG,

    FILE_TYPE_AVI,	// 9
    FILE_TYPE_AKV,
    FILE_TYPE_3GP,
    FILE_TYPE_MP4,
    FILE_TYPE_FLV,
//    FILE_TYPE_RMVB,
//	FILE_TYPE_RM,
    FILE_TYPE_MP1,
    FILE_TYPE_MP2,
    FILE_TYPE_MP3,
    FILE_TYPE_MPG,
    //FILE_TYPE_MKV,	// 20

    FILE_TYPE_MID,
    FILE_TYPE_MIDI,
    FILE_TYPE_ADPCM,
    FILE_TYPE_WAV,
    FILE_TYPE_WAVE,
    
    FILE_TYPE_AMR,
    FILE_TYPE_ASF,
    FILE_TYPE_WMA,
    FILE_TYPE_MPEG,
    FILE_TYPE_AAC,
    FILE_TYPE_AC3,
    FILE_TYPE_ADIF,
    FILE_TYPE_ADTS,
    FILE_TYPE_M4A, 
    FILE_TYPE_FLAC_NATIVE,
    FILE_TYPE_FLAC_OGG,
    FILE_TYPE_FLAC_OGA,
    FILE_TYPE_APE,

    FILE_TYPE_LRC,
    FILE_TYPE_TXT,
    
    FILE_TYPE_DOC,
    FILE_TYPE_PDF,
    FILE_TYPE_XLS,
    FILE_TYPE_MAP,

    FILE_TYPE_NES,
    FILE_TYPE_SNES,
    FILE_TYPE_GBA,
    FILE_TYPE_MD,
    FILE_TYPE_ALT,
    FILE_TYPE_VLT,
    FILE_TYPE_SAV,
    FILE_TYPE_MFS,
    FILE_TYPE_TTF,
    FILE_TYPE_TTC,
    FILE_TYPE_OTF,
    FILE_TYPE_FLASH,
    FILE_TYPE_NONE,
    
    FILE_TYPE_NUM,
    FILE_TYPE_ALL     
}T_FILE_TYPE;



typedef struct {
    T_S8        *String[MAX_STR_LINE];
    T_U16       LineNum;
    T_U16       MaxLen;
} T_CARVED_STR;

/* string functions*/
T_U16   Utl_StrLen(T_pCSTR strMain);
T_S16   Utl_StrFnd(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset);
T_S16   Utl_StrFndN(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 count);
T_S16   Utl_StrFndNL(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 count, T_U16 length);
T_S16   Utl_StrFndL(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 length);
T_S16   Utl_StrFndChr(T_pCSTR strMain, T_S8 chr, T_S16 offset);
T_S16   Utl_StrFndChn(T_pCSTR strMain);
T_S16   Utl_StrRevFnd(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset);
T_S16   Utl_StrRevFndChr(T_pCSTR strMain, T_S8 chr, T_S16 offset);
T_S8    Utl_StrCmp(T_pCSTR str1, T_pCSTR str2);
T_S8    Utl_StrRevCmp(T_pCSTR str1, T_pCSTR str2);
T_S8    Utl_StrRevNCmp(T_pCSTR str1, T_pCSTR str2, T_U8 length);
T_S8    Utl_StrCmpN(T_pCSTR str1, T_pCSTR str2, T_U16 length);
T_S8    Utl_StrCmpC(T_pCSTR str1, T_pCSTR str2);
T_S8    Utl_StrCmpNC(T_pCSTR str1, T_pCSTR str2, T_U16 length);
T_pSTR  Utl_StrCpy(T_pSTR strDest, T_pCSTR strSrc);
T_pSTR  Utl_StrCpyN(T_pSTR strDest, T_pCSTR strSour, T_U32 length);
T_pSTR  Utl_StrCpyChr(T_pSTR strDest, T_S8 chr, T_U16 count);
T_pSTR  Utl_StrCat(T_pSTR strDest, T_pCSTR strSrc);
T_pSTR  Utl_StrCatChr(T_pSTR strDest, T_S8 chr, T_S16 count);
T_pSTR  Utl_StrIns(T_pSTR strDest, T_pCSTR strSub, T_S16 offset);
T_pSTR  Utl_StrInsChr(T_pSTR strDest, T_S8 chr, T_S16 offset);
T_pSTR  Utl_StrRep(T_pSTR strDest, T_pCSTR strSub, T_S16 offset);
T_pSTR  Utl_StrEmpty(T_pSTR strDest);
T_pSTR  Utl_StrDel(T_pSTR strDest, T_S16 offset, T_U16 count);
T_pSTR  Utl_StrDelChr(T_pSTR strDest, T_S8 chr, T_S16 offset, T_S16 count);
T_pSTR  Utl_StrMid(T_pSTR strDest, T_pCSTR strMain, T_S16 offset, T_S16 end);
T_pSTR Utl_StrMidL(T_pSTR strDest, T_pCSTR strSour, T_S16 offset, T_S16 end, T_U16 strlength);
T_pSTR  Utl_StrRight(T_pSTR strDest, T_pCSTR strMain, T_U16 count);
T_CHR   Utl_StrRightChr(T_pCSTR strMain);
T_pSTR  Utl_StrUpper(T_pSTR strMain);
T_pSTR  Utl_StrLower(T_pSTR strMain);
T_pSTR  Utl_StrLTrim(T_pSTR strMain);
T_pSTR  Utl_StrRTrim(T_pSTR strMain);
T_pSTR  Utl_StrTrim(T_pSTR strMain);
T_pSTR  Utl_Itoa(T_S32 intNum, T_pSTR strDest, T_U8 flag);
T_S32   Utl_Atoi(T_pCSTR strMain);
T_BOOL  Utl_StrCarve(T_pCSTR strSour, T_U16 limit, T_CARVED_STR *CarvedStr);
T_BOOL  Utl_StrCarveInit(T_CARVED_STR *CarvedStr);
T_BOOL  Utl_StrCarveFree(T_CARVED_STR *CarvedStr);
T_BOOL  Utl_StrIsEmpty(T_pCSTR strMain);
T_BOOL Utl_IsDWordChar(T_pCSTR strMain, T_RES_LANGUAGE Lang);
T_BOOL Utl_StrDigital(T_pCSTR strMain);
T_S16   Utl_IsAlpha(T_S8 chr);
T_BOOL IToA2(T_S16 number, T_pSTR string);
T_S16   AToI2(T_pCSTR string);
T_S32   AToI16(T_pCSTR string);
T_S16   Utl_Unchange2(T_pCSTR result1point);
T_S16   Utl_Unchange4(T_pCSTR result1point);
T_BOOL Utl_Change4(T_S16 length, T_pSTR resultpoint);
T_BOOL Utl_Change2(T_S16 length, T_pSTR resultpoint);
T_BOOL ConvertIntToString(T_pCSTR strPDU, T_pSTR String2);
T_VOID  Utl_ChangPlace(T_pSTR string);

T_BOOL Utl_MemCpy(T_pVOID strDest, T_pCVOID strSour, T_U32 count);
T_BOOL Utl_MemSet(T_pVOID strDest, T_U8 chr, T_U32 count);
T_S8    Utl_MemCmp(T_pCVOID data1, T_pCVOID data2, T_U32 count);

T_VOID SplitFilePath(T_STR_FILE file_path, T_pSTR path, T_pSTR name);
T_VOID SplitFileName(T_STR_FILE file_name, T_pSTR name, T_pSTR ext);

T_FILE_TYPE Utl_GetFileType(T_pCWSTR file_path);
T_BOOL Utl_StrCarve_Ansi2Unic(T_pSTR pStrSour, T_U16 WidthLimit, T_CARVED_USTR *pUCarvedStr, T_U16 WordWith);
T_BOOL Utl_IsLegalFname(T_U16* Fname);
T_BOOL UTL_isBlankStr(T_pCWSTR wcs);

//add by Jassduke
/*
* @brief  convert a  int64 number to string
*/

T_pSTR Utl_I64toa(T_S64 int64Num , T_pSTR strDest, T_U8 flag);

#endif // __UTLSTIRNG_H

