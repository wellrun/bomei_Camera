/**
 * @file Eng_DataConvert.h
 * @brief This header file is for data convert function prototype
 * 
 */

#ifndef __ENG_DATA_CONVERT_H__
#define __ENG_DATA_CONVERT_H__

#include "anyka_types.h"
#include "Gbl_Global.h"

#define MB_MAX	2
#define UC2GBK_UNDEF 0xffff     //undefined char in GBK code page when convert UNICODE to GBK
#define GBK2UC_UNDEF 0xffff     //undefined char in GBK code page when convert GBK to UNICODE
#define UC2BIG5_UNDEF 0xffff    //undefined char in BIG5 code page when convert UNICODE to BIG5
#define BIG52UC_UNDEF 0xffff    //undefined char in BIG5 code page when convert BIG5 to UNICODE

T_U16   ConverHexToi(T_pCSTR str_Hex);
void BIG5toGBK_Convert(unsigned char* buf,unsigned char* cbuf,unsigned long len);
T_VOID Eng_SetLanguage(T_RES_LANGUAGE lang);
    
T_BOOL Eng_FirstIsGBKChn(const T_S8 *gbk, T_U32 gbkLen);
T_BOOL Eng_FirstIsBIG5Chn(const T_S8 *big5, T_U32 big5Len);
//unicode to gbk
T_S32 Eng_Unicode2GBK(const T_U16 *unicode, T_U32 ucLen, T_U32 *readedUCChars, T_S8 *gbkBuf, T_U32 gbkBufLen, const T_S8 *defaultChar);

T_S32 Eng_MbsToWcs(T_S8 *mbstr, T_S32 cbMultiByte, T_U16*wcstr, T_S32 cchWideChar);
T_S32 Eng_WcsToMbs(T_U16 *wcstr, T_U32 cchWideChar, T_S8 *mbstr, T_U32 cbMultiByte);

/**
 * @brief standard c lib function mbtowc
 *
 * @author he_ying_gz
 *
 * @date 2008-05-16
 * @param pwchar:[out] resulting wide character 
 * @param pmbchar:[in] multibyte string
 * @param count:[in] the length of multibyte string, in bytes
 * @return
 * @retval -1, convert error
 * @retval converted bytes
 * @note refer to standard c document
 */
T_S32 Eng_MbToWc( T_WCHR *pwchar, T_pCSTR pmbchar, T_U32 count);

/**
 * @brief convert utf16be to unicode
 *
 * @author he_ying_gz
 *
 * @date 2008-05-20
 * @param pwchar:[out] resulting wide character 
 * @param pmbchar:[in] utf16be string
 * @param count:[in] the length of utf16be string, in bytes
 * @return
 * @retval converted bytes
 */
T_S32 Eng_UTF16BEToWc(const T_U8 *pbe, T_U32 byteCount, T_WCHR *pwchar, T_U32 wcCount);

//unicode to multiple byte code
T_S32 Eng_WideCharToMultiByte(T_RES_LANGUAGE lang, const T_U16 *unicode, T_U32 ucLen, T_U32 *readedBytes, T_S8 *pDestBuf, T_U32 destLen, const T_S8 *defaultChr);
T_S32 Eng_StrMbcs2Ucs(const T_S8 *src, T_U16 *ucBuf);
T_U16 * Eng_StrMbcs2Ucs_2(const T_S8 *src, T_U16 *ucBuf);
/**
 * @brief change ansi string to unicode string
 *
 * @date 2007-01-10
 * @param [in] src: ansi string to be converted
 * @return T_U16 * the unicode string buffer point
 * @retval 
 */
T_U16 * Eng_StrMbcs2Ucs_3(const T_S8 *src);
T_S32 Eng_StrMbcs2Ucs_4(const T_S8 *src, T_U16 *ucBuf, T_U32 ucBufLen);

T_S32 Eng_StrUcs2Mbcs(const T_U16 *usrc, T_S8 *strBuf);
//multiple byte code to unicode
T_S32 Eng_MultiByteToWideChar(T_RES_LANGUAGE lang, const T_S8 *src, T_U32 srcLen, T_U32 *readedBytes, T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr);

T_S32 Eng_UTF82Unicode(const T_S8 *utf8, T_U32 utf8BufLen, T_U32 *readedUTF8Bytes, T_U16 *ucBuffer, T_S32 ucBufLen);
T_S32 Eng_Unicode2UTF8(const T_U16 *unicode, T_U32 ucLen, T_U32 *readedUCs, T_S8 *utf8, T_U32 utf8BufLen);

T_S32 UniStr2AnsiStr(const T_U16 *pUniStr, T_U32 UniStrLen, T_pSTR pAnsibuf, T_U32 AnsiBufLen, T_U32 code);
T_S32 AnsiStr2UniStr(T_pSTR pAnsiStr, T_U32 AnsiStrLen, T_U16 *pUniBuf, T_U32 UniBufLen, T_U32 code);

T_U16 Unichar2Ansi(T_U16 uni, T_U32 code);
T_U16 Ansi2Unichar(T_U16 ansi, T_U32 code);
T_VOID Eng_GetAspectRatio(T_U16* dstW, T_U16* dstH, T_U16 srcW, T_U16 srcH, T_U16 maxLen);

#endif
