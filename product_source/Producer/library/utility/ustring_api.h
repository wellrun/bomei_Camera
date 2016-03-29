/**
 * @file Eng_String.h
 * @author Zhusizhe
 * @date 2005-08-24
 * @brief ANYKA software
 * this header file provide application layer string function  
 */

#ifndef __USTRING_API_H__
#define __USTRING_API_H__

/* string functions*/
T_S16	Utl_UStrLen(T_pCWSTR strMain);
T_S32	Utl_UStrLen32(T_pCWSTR strMain);

T_S16	Utl_UStrFnd(T_pCWSTR strMain, T_pCWSTR strSub, T_S16 offset);
T_S32	Utl_UStrFnd32(T_pCWSTR strMain, T_pCWSTR strSub, T_S32 offset);

T_S16	Utl_UStrFndN(T_pCWSTR strMain, T_pCWSTR strSub, T_S16 offset, T_U16 count);
T_S32	Utl_UStrFndN32(T_pCWSTR strMain, T_pCWSTR strSub, T_S32 offset, T_U32 count);

T_S16   Utl_UStrFndNL(T_pCWSTR strMain, T_pCWSTR strSub, T_S16 offset, T_U16 count, T_U16 length);
T_S32   Utl_UStrFndNL32(T_pCWSTR strMain, T_pCWSTR strSub, T_S32 offset, T_U32 count, T_U32 length);

T_S16	Utl_UStrFndL(T_pCWSTR strMain, T_pCWSTR strSub, T_S16 offset, T_U16 length);
T_S32	Utl_UStrFndL32(T_pCWSTR strMain, T_pCWSTR strSub, T_S32 offset, T_U32 length);

T_S16	Utl_UStrFndChr(T_pCWSTR strMain, T_WCHR chr, T_S16 offset);
T_S32	Utl_UStrFndChr32(T_pCWSTR strMain, T_WCHR chr, T_S32 offset);

T_S16	Utl_UStrFndChn(T_pCWSTR strMain);
T_S32	Utl_UStrFndChn32(T_pCWSTR strMain);

T_S16	Utl_UStrRevFnd(T_pCWSTR strMain, T_pCWSTR strSub, T_S16 offset);
T_S32	Utl_UStrRevFnd32(T_pCWSTR strMain, T_pCWSTR strSub, T_S32 offset);

T_S16	Utl_UStrRevFndChr(T_pCWSTR strMain, T_WCHR chr, T_S16 offset);
T_S32	Utl_UStrRevFndChr32(T_pCWSTR strMain, T_WCHR chr, T_S32 offset);

T_S16	Utl_UStrCmp(T_pCWSTR str1, T_pCWSTR str2);

T_S16	Utl_UStrRevCmp(T_pCWSTR str1, T_pCWSTR str2);

T_S16	Utl_UStrRevNCmp(T_pCWSTR str1, T_pCWSTR str2, T_U16 length);
T_S16	Utl_UStrRevNCmp32(T_pCWSTR str1, T_pCWSTR str2, T_U32 length);

T_S16	Utl_UStrCmpN(T_pCWSTR str1, T_pCWSTR str2, T_U16 length);
T_S16	Utl_UStrCmpN32(T_pCWSTR str1, T_pCWSTR str2, T_U32 length);

T_S16	Utl_UStrCmpC(T_pCWSTR str1, T_pCWSTR str2);

T_S16	Utl_UStrCmpNC(T_pCWSTR str1, T_pCWSTR str2, T_U16 length);
T_S16	Utl_UStrCmpNC32(T_pCWSTR str1, T_pCWSTR str2, T_U32 length);

T_pWSTR	Utl_UStrCpy(T_pWSTR strDest, T_pCWSTR strSrc);

T_pWSTR	Utl_UStrCpyN(T_pWSTR strDest, T_pCWSTR strSour, T_U16 length);
T_pWSTR	Utl_UStrCpyN32(T_pWSTR strDest, T_pCWSTR strSour, T_U32 length);

T_pWSTR	Utl_UStrCpyChr(T_pWSTR strDest, T_WCHR chr, T_U16 count);
T_pWSTR	Utl_UStrCpyChr32(T_pWSTR strDest, T_WCHR chr, T_U32 count);

T_pWSTR	Utl_UStrCat(T_pWSTR strDest, T_pCWSTR strSrc);

T_pWSTR	Utl_UStrCatN(T_pWSTR strDest, T_pCWSTR strSrc, T_U16 length);
T_pWSTR	Utl_UStrCatN32(T_pWSTR strDest, T_pCWSTR strSrc, T_U32 length);

T_pWSTR	Utl_UStrCatChr(T_pWSTR strDest, T_WCHR chr, T_U16 count);
T_pWSTR	Utl_UStrCatChr32(T_pWSTR strDest, T_WCHR chr, T_U32 count);

T_pWSTR	Utl_UStrIns(T_pWSTR strDest, T_pCWSTR strSub, T_S16 offset);
T_pWSTR	Utl_UStrIns32(T_pWSTR strDest, T_pCWSTR strSub, T_S32 offset);

T_pWSTR	Utl_UStrInsChr(T_pWSTR strDest, T_WCHR chr, T_S16 offset);
T_pWSTR	Utl_UStrInsChr32(T_pWSTR strDest, T_WCHR chr, T_S32 offset);

T_pWSTR	Utl_UStrRep(T_pWSTR strDest, T_pCWSTR strSub, T_S16 offset);
T_pWSTR	Utl_UStrRep32(T_pWSTR strDest, T_pCWSTR strSub, T_S32 offset);

T_pWSTR	Utl_UStrEmpty(T_pWSTR strDest);

T_pWSTR	Utl_UStrDel(T_pWSTR strDest, T_S16 offset, T_S16 count);
T_pWSTR	Utl_UStrDel32(T_pWSTR strDest, T_S32 offset, T_S32 count);

T_pWSTR	Utl_UStrDelChr(T_pWSTR strDest, T_WCHR chr, T_S16 offset, T_S16 count);
T_pWSTR	Utl_UStrDelChr32(T_pWSTR strDest, T_WCHR chr, T_S32 offset, T_S32 count);

T_pWSTR	Utl_UStrMid(T_pWSTR strDest, T_pCWSTR strMain, T_S16 offset, T_S16 end);
T_pWSTR	Utl_UStrMid32(T_pWSTR strDest, T_pCWSTR strMain, T_S32 offset, T_S32 end);

T_pWSTR  Utl_UStrMidL(T_pWSTR strDest, T_pCWSTR strSour, T_S16 offset, T_S16 end, T_U16 strlength);
T_pWSTR  Utl_UStrMidL32(T_pWSTR strDest, T_pCWSTR strSour, T_S32 offset, T_S32 end, T_U32 strlength);

T_pWSTR	Utl_UStrRight(T_pWSTR strDest, T_pCWSTR strMain, T_U16 count);
T_pWSTR	Utl_UStrRight32(T_pWSTR strDest, T_pCWSTR strMain, T_U32 count);

T_WCHR	Utl_UStrRightChr(T_pCWSTR strMain);

T_pWSTR	Utl_UStrUpper(T_pWSTR strMain);

T_pWSTR	Utl_UStrLower(T_pWSTR strMain);

T_pWSTR	Utl_UStrLTrim(T_pWSTR strMain);

T_pWSTR	Utl_UStrRTrim(T_pWSTR strMain);

T_pWSTR	Utl_UStrTrim(T_pWSTR strMain);

T_pWSTR	Utl_UItoa(T_S32 intNum, T_pWSTR strDest, T_U8 flag);

T_S32	Utl_UAtoi(T_pCWSTR strMain);
T_BOOL	Utl_UStrIsEmpty(T_pCWSTR strMain);
T_BOOL  Utl_UStrDigital(T_pCWSTR strMain);
T_S32	Utl_UIsAlpha(T_WCHR chr);
T_BOOL  UIToA2(T_S32 number, T_pWSTR string);
T_S32	UAToI2(T_pCWSTR string);
T_S32	UAToI16(T_pCWSTR string);
T_BOOL  Utl_UChange4(T_S32 length, T_pWSTR resultpoint);
T_BOOL  Utl_UChange2(T_S32 length, T_pWSTR resultpoint);
T_VOID	Utl_UChangPlace(T_pWSTR changestring);

T_S16	Utl_UStrFndC(T_pCWSTR strMain, T_pCWSTR strSub, T_S16 offset);
T_S32	Utl_UStrFndC32(T_pCWSTR strMain, T_pCWSTR strSub, T_S32 offset);

T_S16	Utl_UStrFndLC(T_pCWSTR strMain, T_pCWSTR strSub, T_S16 offset, T_U16 length);
T_S32	Utl_UStrFndLC32(T_pCWSTR strMain, T_pCWSTR strSub, T_S32 offset, T_U32 length);

T_S16	Utl_UStrFndNC(T_pCWSTR strMain, T_pCWSTR strSub, T_S16 offset, T_U16 count);
T_S32	Utl_UStrFndNC32(T_pCWSTR strMain, T_pCWSTR strSub, T_S32 offset, T_U32 count);

T_S16   Utl_UStrFndNLC(T_pCWSTR strMain, T_pCWSTR strSub, T_S16 offset, T_U16 count, T_U16 length);
T_S32   Utl_UStrFndNLC32(T_pCWSTR strMain, T_pCWSTR strSub, T_S32 offset, T_U32 count, T_U32 length);

T_S16	Utl_UStrRevFndC(T_pCWSTR strMain, T_pCWSTR strSub, T_S16 offset);
T_S32	Utl_UStrRevFndC32(T_pCWSTR strMain, T_pCWSTR strSub, T_S32 offset);

#endif // end of __USTRING_API_H__

