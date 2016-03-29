/**
 * @file Eng_DataConvert.h
 * @brief This header file is for data convert function prototype
 * 
 */

#ifndef __ENG_DATA_CONVERT_H__
#define __ENG_DATA_CONVERT_H__

//#include "akdefine.h"
#include "Eng_Type.h"

#define UC2GBK_UNDEF 0xffff		//undefined char in GBK code page when convert UNICODE to GBK
#define GBK2UC_UNDEF 0xffff		//undefined char in GBK code page when convert GBK to UNICODE

T_U16	ConverHexToi(T_pCSTR str_Hex);
T_S16	ConvGSMToUCS2(T_pCSTR strGSM, T_pSTR strUCS2);
T_VOID ConvUCS2ToGSM(T_pCSTR strUCS2, T_pSTR strGSM);
#if 0
T_VOID ConvGBToBig5(T_pDATA srcGB, T_pDATA dstBig5);
T_VOID ConvBig5ToGB(T_pDATA srcBig5, T_pDATA dstGB);
#endif
T_U16	UniCode2GB(T_U16 uniCode, T_pSTR GB);
T_U16	UniCodes2GB(T_U16 *uniCode, T_U16 len, T_pSTR GB);
void BIG5toGBK_Convert(unsigned char* buf,unsigned char* cbuf,unsigned long len);
	
T_S32 Eng_Unicode2GBK(const T_U16 *unicode, T_U32 ucLen, T_U32 *readedUCChars, T_S8 *gbkBuf, T_U32 gbkBufLen, const T_S8 *defaultChar);
T_S32 Eng_GBK2Unicode(const T_S8 *gbk, T_U32 gbkLen, T_U32 *readedGBKBytes, T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr);
T_BOOL Eng_FirstIsGBKChn(const T_S8 *gbk, T_U32 gbkLen);

T_S32 Eng_UTF82Unicode(const T_S8 *utf8, T_U32 utf8BufLen, T_U32 *readedUTF8Bytes, T_U16 *ucBuffer, T_S32 ucBufLen);
T_S32 Eng_Unicode2UTF8(const T_U16 *unicode, T_U32 ucLen, T_U32 *readedUCs, T_S8 *utf8, T_U32 utf8BufLen);

#endif
