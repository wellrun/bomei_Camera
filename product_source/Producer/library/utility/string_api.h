/**
 * @file Eng_String.h
 * @author Zhusizhe
 * @date 2005-08-24
 * @brief ANYKA software
 * this header file provide application layer string function  
 */

#ifndef __STRING_API_H__
#define __STRING_API_H__

#define UTLLIB_VERSION "V1.8.0C_U"

//#define MAX_STRING_LEN		T_U16_MAX
//
//typedef struct {
//    T_S8        *String[MAX_STR_LINE];
//    T_U16        LineNum;
//    T_U16        MaxLen;
//} T_CARVED_STR;

/* string functions*/
T_S16	Utl_StrLen(T_pCSTR strMain);
T_S32	Utl_StrLen32(T_pCSTR strMain);

T_S16	Utl_StrFnd(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset);
T_S32	Utl_StrFnd32(T_pCSTR strMain, T_pCSTR strSub, T_S32 offset);

T_S16	Utl_StrFndN(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 count);
T_S32	Utl_StrFndN32(T_pCSTR strMain, T_pCSTR strSub, T_S32 offset, T_U32 count);

T_S16   Utl_StrFndNL(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 count, T_U16 length);
T_S32   Utl_StrFndNL32(T_pCSTR strMain, T_pCSTR strSub, T_S32 offset, T_U32 count, T_U32 length);

T_S16	Utl_StrFndL(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 length);
T_S32	Utl_StrFndL32(T_pCSTR strMain, T_pCSTR strSub, T_S32 offset, T_U32 length);

T_S16	Utl_StrFndChr(T_pCSTR strMain, T_S8 chr, T_S16 offset);
T_S32	Utl_StrFndChr32(T_pCSTR strMain, T_S8 chr, T_S32 offset);

T_S16	Utl_StrFndChn(T_pCSTR strMain);
T_S32	Utl_StrFndChn32(T_pCSTR strMain);

T_S16	Utl_StrRevFnd(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset);
T_S32	Utl_StrRevFnd32(T_pCSTR strMain, T_pCSTR strSub, T_S32 offset);

T_S16	Utl_StrRevFndChr(T_pCSTR strMain, T_S8 chr, T_S16 offset);
T_S32	Utl_StrRevFndChr32(T_pCSTR strMain, T_S8 chr, T_S32 offset);

T_S8	Utl_StrCmp(T_pCSTR str1, T_pCSTR str2);

T_S8	Utl_StrRevCmp(T_pCSTR str1, T_pCSTR str2);

T_S8	Utl_StrRevNCmp(T_pCSTR str1, T_pCSTR str2, T_U16 length);
T_S8	Utl_StrRevNCmp32(T_pCSTR str1, T_pCSTR str2, T_U32 length);

T_S8	Utl_StrCmpN(T_pCSTR str1, T_pCSTR str2, T_U16 length);
T_S8	Utl_StrCmpN32(T_pCSTR str1, T_pCSTR str2, T_U32 length);

T_S8	Utl_StrCmpC(T_pCSTR str1, T_pCSTR str2);

T_S8	Utl_StrCmpNC(T_pCSTR str1, T_pCSTR str2, T_U16 length);
T_S8	Utl_StrCmpNC32(T_pCSTR str1, T_pCSTR str2, T_U32 length);

T_pSTR	Utl_StrCpy(T_pSTR strDest, T_pCSTR strSrc);

T_pSTR	Utl_StrCpyN(T_pSTR strDest, T_pCSTR strSour, T_U16 length);
T_pSTR	Utl_StrCpyN32(T_pSTR strDest, T_pCSTR strSour, T_U32 length);

T_pSTR	Utl_StrCpyChr(T_pSTR strDest, T_S8 chr, T_U16 count);
T_pSTR	Utl_StrCpyChr32(T_pSTR strDest, T_S8 chr, T_U32 count);

T_pSTR	Utl_StrCat(T_pSTR strDest, T_pCSTR strSrc);

T_pSTR	Utl_StrCatN(T_pSTR strDest, T_pCSTR strSrc, T_U16 length);
T_pSTR	Utl_StrCatN32(T_pSTR strDest, T_pCSTR strSrc, T_U32 length);

T_pSTR	Utl_StrCatChr(T_pSTR strDest, T_S8 chr, T_U16 count);
T_pSTR	Utl_StrCatChr32(T_pSTR strDest, T_S8 chr, T_U32 count);

T_pSTR	Utl_StrIns(T_pSTR strDest, T_pCSTR strSub, T_S16 offset);
T_pSTR	Utl_StrIns32(T_pSTR strDest, T_pCSTR strSub, T_S32 offset);

T_pSTR	Utl_StrInsChr(T_pSTR strDest, T_S8 chr, T_S16 offset);
T_pSTR	Utl_StrInsChr32(T_pSTR strDest, T_S8 chr, T_S32 offset);

T_pSTR	Utl_StrRep(T_pSTR strDest, T_pCSTR strSub, T_S16 offset);
T_pSTR	Utl_StrRep32(T_pSTR strDest, T_pCSTR strSub, T_S32 offset);

T_pSTR	Utl_StrEmpty(T_pSTR strDest);

T_pSTR	Utl_StrDel(T_pSTR strDest, T_S16 offset, T_S16 count);
T_pSTR	Utl_StrDel32(T_pSTR strDest, T_S32 offset, T_S32 count);

T_pSTR	Utl_StrDelChr(T_pSTR strDest, T_S8 chr, T_S16 offset, T_S16 count);
T_pSTR	Utl_StrDelChr32(T_pSTR strDest, T_S8 chr, T_S32 offset, T_S32 count);

T_pSTR	Utl_StrMid(T_pSTR strDest, T_pCSTR strMain, T_S16 offset, T_S16 end);
T_pSTR	Utl_StrMid32(T_pSTR strDest, T_pCSTR strMain, T_S32 offset, T_S32 end);

T_pSTR  Utl_StrMidL(T_pSTR strDest, T_pCSTR strSour, T_S16 offset, T_S16 end, T_U16 strlength);
T_pSTR  Utl_StrMidL32(T_pSTR strDest, T_pCSTR strSour, T_S32 offset, T_S32 end, T_U32 strlength);

T_pSTR	Utl_StrRight(T_pSTR strDest, T_pCSTR strMain, T_U16 count);
T_pSTR	Utl_StrRight32(T_pSTR strDest, T_pCSTR strMain, T_U32 count);

T_S16	Utl_StrFndC(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset);
T_S32	Utl_StrFndC32(T_pCSTR strMain, T_pCSTR strSub, T_S32 offset);

T_S16	Utl_StrFndLC(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 length);
T_S32	Utl_StrFndLC32(T_pCSTR strMain, T_pCSTR strSub, T_S32 offset, T_U32 length);

T_S16	Utl_StrFndNC(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 count);
T_S32	Utl_StrFndNC32(T_pCSTR strMain, T_pCSTR strSub, T_S32 offset, T_U32 count);

T_S16   Utl_StrFndNLC(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 count, T_U16 length);
T_S32   Utl_StrFndNLC32(T_pCSTR strMain, T_pCSTR strSub, T_S32 offset, T_U32 count, T_U32 length);

T_S16	Utl_StrRevFndC(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset);
T_S32	Utl_StrRevFndC32(T_pCSTR strMain, T_pCSTR strSub, T_S32 offset);

T_CHR	Utl_StrRightChr(T_pCSTR strMain);

T_pSTR	Utl_StrUpper(T_pSTR strMain);

T_pSTR	Utl_StrLower(T_pSTR strMain);

T_pSTR	Utl_StrLTrim(T_pSTR strMain);

T_pSTR	Utl_StrRTrim(T_pSTR strMain);

T_pSTR	Utl_StrTrim(T_pSTR strMain);

T_pSTR	Utl_Itoa(T_S32 intNum, T_pSTR strDest, T_U8 flag);

T_S32	Utl_Atoi(T_pCSTR strMain);
T_BOOL	Utl_StrIsEmpty(T_pCSTR strMain);
T_BOOL  Utl_IsDWordChar(T_pCSTR strMain);
T_BOOL  Utl_StrDigital(T_pCSTR strMain);
T_S32	Utl_IsAlpha(T_S8 chr);
T_BOOL  IToA2(T_S32 number, T_pSTR string);
T_S32	AToI2(T_pCSTR string);
T_S32	AToI16(T_pCSTR string);
T_S32	Utl_Unchange2(T_pCSTR result1point);
T_S32	Utl_Unchange4(T_pCSTR result1point);
T_BOOL  Utl_Change4(T_S32 length, T_pSTR resultpoint);
T_BOOL  Utl_Change2(T_S32 length, T_pSTR resultpoint);
T_VOID	Utl_ChangPlace(T_pSTR changestring);
//T_BOOL  Utl_StrCarve(T_pCSTR strSour, T_U16 limit, T_pCSTR spmark, T_CARVED_STR *CarvedStr);
//T_BOOL  Utl_StrCarveInit(T_CARVED_STR *CarvedStr);
//T_BOOL  Utl_StrCarveFree(T_CARVED_STR *CarvedStr);

#endif // end of __STRING_API_H__

