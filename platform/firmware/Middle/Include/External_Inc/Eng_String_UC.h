/**
 * @file Eng_String.h
 * @author Junhua Zhao
 * @date 2005-08-10
 * @brief ANYKA software
 * this header file provide hal layer string function 
 */

#ifndef __UTL_UNICODE_STRING_H
#define __UTL_UNICODE_STRING_H

#include "anyka_types.h"
#include "Gbl_Global.h"
#include "eng_string_uc.h"

//#define MAX_USTRING_LEN       1024000
#define MAX_USTRING_LEN     0x7fff
#define MAX_USTR_LINE       1000

#define UNICODE_R           0x000d      // '\R'
#define UNICODE_N           0x000a      // '\N'
#define UNICODE_SPACE       0x0020      // ' '

#define UNICODE_A           0x0041      // 'A'
#define UNICODE_B           0x0042      // 'B'
#define UNICODE_C           0x0043      // 'C'
#define UNICODE_D           0x0044      // 'D'
#define UNICODE_E			0x0045
#define UNICODE_Z           0x005a      // 'Z'

#define UNICODE_a           0x0061      // 'a'  
#define UNICODE_b           0x0062      // 'b'
#define UNICODE_c           0x0063      // 'c'
#define UNICODE_d           0x0064      // 'd'
#define UNICODE_e           0x0065      // 'e'
#define UNICODE_f           0x0066      // 'f'
#define UNICODE_i           0x0069      // 'i'
#define UNICODE_k           0x006b      // 'k'
#define UNICODE_l           0x006c      // 'l'
#define UNICODE_o           0x006f      // 'o'
#define UNICODE_r           0x0072      // 'r'
#define UNICODE_s           0x0073      // 's'
#define UNICODE_t           0x0074      // 't'
#define UNICODE_y           0x0079      // 'y'

#define UNICODE_0           0x0030      // '0'
#define UNICODE_1           0x0031      // '1'
#define UNICODE_2           0x0032      // '2'
#define UNICODE_3           0x0033      // '3'
#define UNICODE_4           0x0034      // '4'
#define UNICODE_5           0x0035      // '5'
#define UNICODE_6           0x0036      // '6'
#define UNICODE_7           0x0037      // '7'
#define UNICODE_8           0x0038      // '8'
#define UNICODE_9           0x0039      // '9'

#define UNICODE_END         0x0000      // '\0'
#define UNICODE_COLON       0x003A      // ':'
#define UNICODE_SEPARATOR   0x003B      //';'
#define UNICODE_DOT         0x002E      // '.'
#define UNICODE_SOLIDUS     0x002F      // '/'
#define UNICODE_RES_SOLIDUS 0x005C      // '\'
#define UNICODE_STAR        0x002A      // '*'
#define UNICODE_QUESTION    0x003F      // '?'
#define UNICODE_LBRACKET    0x005b      // '['
#define UNICODE_RBRACKET    0x005d      // ']'
#define UNICODE_COMMA       0x002c      //','
#define UNICODE_QUOTATION   0x0722      //'"'
#define UNICODE_BRACKET_L   0x0728      //'('
#define UNICODE_BRACKET_R   0x0729      //')'
#define UNICODE_EXCAL       0x0721      //'!'

#define UNICODE_BAR         0x002D      //'-'

#define BACK_SEARCH_MAX 10  /*the max num of char when search backward*/

#define Utl_TStrLen  Utl_UStrLen
#define Utl_TStrCpy  Utl_UStrCpy   

#define _T(_pAnsi_)      Eng_StrMbcs2Ucs_3(_pAnsi_)

typedef struct {
    T_U16       *String[MAX_USTR_LINE];
    T_U16       UnicodeNum[MAX_USTR_LINE];  // unicode char num per line
    T_U16       LineNum;
    T_U16       MaxLen;
} T_CARVED_USTR;

/* unicode string functions*/
T_U32   Utl_UStrLen(const T_U16* strMain);
T_U16   Utl_UByteCount(const T_U16* strMain,T_U16 strLen);
T_S16   Utl_UStrFnd(T_U16*  strMain, const T_U16*  strSub, T_S16 offset);
T_S16   Utl_UStrFndN(T_U16*  strMain, const T_U16* strSub, T_S16 offset, T_U16 count);
T_S16   Utl_UStrFndNL(T_U16* strMain, const T_U16* strSub, T_S16 offset, T_U16 count, T_U16 length);
T_S16   Utl_UStrFndL(T_U16* strMain, const T_U16* strSub, T_S16 offset, T_U16 length);
T_S16   Utl_UStrFndChr(T_U16* strMain, T_U16 chr, T_S16 offset);
T_S16   Utl_UStrRevFnd(const T_U16* strMain, const T_U16* strSub, T_S16 offset);
T_S16   Utl_UStrRevFndChr(const T_U16* strMain, const T_U16 chr, T_S16 offset);
T_S8    Utl_UStrCmp(const T_U16* str1, const T_U16* str2);
T_S8    Utl_UStrRevCmp(T_U16* str1, T_U16* str2);
T_S8    Utl_UStrRevNCmp(T_U16* str1, T_U16* str2, T_U8 length);
T_S8    Utl_UStrCmpN(T_U16* str1, T_U16* str2, T_U16 length);
T_S8    Utl_UStrCmpC(T_U16* str1, T_U16* str2);
T_S8    Utl_UStrCmpNC(T_U16* str1, T_U16* str2, T_U16 length);
T_U16*  Utl_UStrCpy(T_U16* strDest, const T_U16* strSrc);
T_U16*  Utl_UStrCpyN(T_U16* strDest, const T_U16* strSour, T_U32 length);
T_U16*  Utl_UStrCpyChr(T_U16* strDest, T_U16 chr, T_U16 count);
T_U16*  Utl_UStrCat(T_U16* strDest, const T_U16* strSrc);
T_U16*  Utl_UStrCatChr(T_U16* strDest, T_U16 chr, T_S16 count);
T_U16*  Utl_UStrIns(T_U16* strDest, const T_U16* strSub, T_S16 offset);
T_U16*  Utl_UStrInsChr(T_U16* strDest, T_U16 chr, T_S16 offset);
T_U16*  Utl_UStrRep(T_U16* strDest, const T_U16* strSub, T_S16 offset);
T_U16*  Utl_UStrEmpty(T_U16* strDest);
T_U16*  Utl_UStrDel(T_U16* strDest, T_S16 offset, T_U16 count);
T_U16*  Utl_UStrDelChr(T_U16* strDest, T_U16 chr, T_S16 offset, T_S16 count);
T_U16*  Utl_UStrMid(T_U16* strDest, const T_U16* strMain, T_S16 offset, T_S16 end);
T_U16*  Utl_UStrMidL(T_U16* strDest, const T_U16* strSour, T_S16 offset, T_S16 end, T_U16 strlength);
T_U16*  Utl_UStrRight(T_U16* strDest, T_U16* strMain, T_U16 count);
T_U16   Utl_UStrRightChr(const T_U16* strMain);
T_U16*  Utl_UStrUpper(T_U16* strMain);
T_U16*  Utl_UStrLower(T_U16* strMain);
T_U16*  Utl_UStrLTrim(T_U16* strMain);
T_U16*  Utl_UStrRTrim(T_U16* strMain);
T_U16*  Utl_UStrTrim(T_U16* strMain);
T_U16*  Utl_UStrInit(T_U16* strMain,const T_pCSTR pStr);
T_U16*  Utl_UItoa(T_S32 intNum, T_U16* strDest, T_U8 flag);
T_S32   Utl_UAtoi(T_U16* strMain);
T_BOOL  Utl_UStrCarve(const T_U16* strSour, T_U16 WidthLimit, const T_U16* spmark, T_CARVED_USTR *CarvedStr, T_U16 WordSize);
T_BOOL  Utl_UStrCarveInit(T_CARVED_USTR *CarvedStr);
T_BOOL  Utl_UStrCarveFree(T_CARVED_USTR *CarvedStr);
T_BOOL  Utl_UStrIsEmpty(const T_U16* strMain);
T_BOOL  Utl_UStrDigital(const T_U16* strMain);
T_S16   Utl_UIsAlpha(T_U16 chr);
T_BOOL  Utl_UIsDWordChar(const T_U16* strMain, T_RES_LANGUAGE Lang);
T_U16 Utl_CaclSolidas(const T_U16* str);
T_VOID  Utl_USplitFilePath(T_pCWSTR file_path, T_pWSTR path, T_pWSTR name);
T_VOID  Utl_USplitFileName(T_pCWSTR file_name, T_pWSTR name, T_pWSTR ext);
T_U16 GetFilePathDisk(T_USTR_FILE file_path);
T_BOOL FileIsInSD(T_USTR_FILE file_path);

T_VOID  Printf_UC(T_pCWSTR ustr);

#endif // __UTL_UNICODE_STRING_H

