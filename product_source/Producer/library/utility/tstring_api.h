/**
 * @file tstring_api.h
 * @author Zhusizhe
 * @date 2005-08-24
 * @brief ANYKA software
 * this header file provide application layer string function  
 */

#ifndef __TSTRING_API_H__
#define __TSTRING_API_H__

#ifndef _UNICODE

/********************begin ANSI string operation functions********************/
#include "string_api.h"

#define Utl_TStrCmp			Utl_StrCmp
#define Utl_TStrRevCmp		Utl_StrRevCmp
#define Utl_TStrCmpC		Utl_StrCmpC
#define Utl_TStrCpy			Utl_StrCpy
#define Utl_TStrCat			Utl_StrCat
#define Utl_TStrEmpty		Utl_StrEmpty
#define Utl_TStrRightChr	Utl_StrRightChr
#define Utl_TStrUpper		Utl_StrUpper
#define Utl_TStrLower		Utl_StrLower
#define Utl_TStrLTrim		Utl_StrLTrim
#define Utl_TStrRTrim		Utl_StrRTrim
#define Utl_TStrTrim		Utl_StrTrim
#define Utl_TAtoi			Utl_Atoi
#define Utl_TStrIsEmpty		Utl_StrIsEmpty
#define Utl_TIsDWordChar	Utl_IsDWordChar
#define Utl_TStrDigital		Utl_StrDigital
#define Utl_TIsAlpha		Utl_IsAlpha
#define TIToA2				IToA2
#define TAToI2				AToI2
#define TAToI16				AToI16
#define Utl_TUnchange2		Utl_Unchange2
#define Utl_TUnchange4		Utl_Unchange4
#define Utl_TChange4		Utl_Change4
#define Utl_TChange2		Utl_Change2


#ifdef STR_32BITLEN
/********************begin 32bit length string operation functions************/
#define Utl_TStrLen			Utl_StrLen32
#define Utl_TStrFnd			Utl_StrFnd32
#define Utl_TStrFndN		Utl_StrFndN32
#define Utl_TStrFndNL		Utl_StrFndNL32
#define Utl_TStrFndL		Utl_StrFndL32
#define Utl_TStrFndChr		Utl_StrFndChr32
#define Utl_TStrFndChn		Utl_StrFndChn32
#define Utl_TStrRevFnd		Utl_StrRevFnd32
#define Utl_TStrRevFndChr	Utl_StrRevFndChr32
#define Utl_TStrRevNCmp		Utl_StrRevNCmp32
#define Utl_TStrCmpN		Utl_StrCmpN32
#define Utl_TStrCmpNC		Utl_StrCmpNC32
#define Utl_TStrCpyN		Utl_StrCpyN32
#define Utl_TStrCpyChr		Utl_StrCpyChr32
#define Utl_TStrCatN		Utl_StrCatN32
#define Utl_TStrCatChr		Utl_StrCatChr32
#define Utl_TStrIns			Utl_StrIns32
#define Utl_TStrInsChr		Utl_StrInsChr32
#define Utl_TStrRep			Utl_StrRep32
#define Utl_TStrDel			Utl_StrDel32
#define Utl_TStrDelChr		Utl_StrDelChr32
#define Utl_TStrMid			Utl_StrMid32
#define Utl_TStrMidL		Utl_StrMidL32
#define Utl_TStrRight		Utl_StrRight32
#define Utl_TItoa			Utl_Itoa32
#define Utl_TStrFndC		Utl_StrFndC32
#define Utl_TStrFndNC		Utl_StrFndNC32
#define Utl_TStrFndNLC		Utl_StrFndNLC32
#define Utl_TStrFndLC		Utl_StrFndLC32
#define Utl_TStrRevFndC		Utl_StrRevFndC32
/********************end 32bit string length operation functions**************/
#else
/********************begin 16bit string length operation functions************/
#define Utl_TStrLen			Utl_StrLen
#define Utl_TStrFnd			Utl_StrFnd
#define Utl_TStrFndN		Utl_StrFndN
#define Utl_TStrFndNL		Utl_StrFndNL
#define Utl_TStrFndL		Utl_StrFndL
#define Utl_TStrFndChr		Utl_StrFndChr
#define Utl_TStrFndChn		Utl_StrFndChn
#define Utl_TStrRevFnd		Utl_StrRevFnd
#define Utl_TStrRevFndChr	Utl_StrRevFndChr
#define Utl_TStrRevNCmp		Utl_StrRevNCmp
#define Utl_TStrCmpN		Utl_StrCmpN
#define Utl_TStrCmpNC		Utl_StrCmpNC
#define Utl_TStrCpyN		Utl_StrCpyN
#define Utl_TStrCpyChr		Utl_StrCpyChr
#define Utl_TStrCatN		Utl_StrCatN
#define Utl_TStrCatChr		Utl_StrCatChr
#define Utl_TStrIns			Utl_StrIns
#define Utl_TStrInsChr		Utl_StrInsChr
#define Utl_TStrRep			Utl_StrRep
#define Utl_TStrDel			Utl_StrDel
#define Utl_TStrDelChr		Utl_StrDelChr
#define Utl_TStrMid			Utl_StrMid
#define Utl_TStrMidL		Utl_StrMidL
#define Utl_TStrRight		Utl_StrRight
#define Utl_TItoa			Utl_Itoa
#define Utl_TStrFndC		Utl_StrFndC
#define Utl_TStrFndNC		Utl_StrFndNC
#define Utl_TStrFndNLC		Utl_StrFndNLC
#define Utl_TStrFndLC		Utl_StrFndLC
#define Utl_TStrRevFndC		Utl_StrRevFndC
/********************end  16bit string length operation functions*************/
#endif


/********************end ANSI string operation functions**********************/
#else

/********************begin UNICODE string operation functions*****************/
#include "ustring_api.h"

#define Utl_TStrCmp			Utl_UStrCmp
#define Utl_TStrRevCmp		Utl_UStrRevCmp
#define Utl_TStrCmpC		Utl_UStrCmpC
#define Utl_TStrCpy			Utl_UStrCpy
#define Utl_TStrCat			Utl_UStrCat
#define Utl_TStrEmpty		Utl_UStrEmpty
#define Utl_TStrRightChr	Utl_UStrRightChr
#define Utl_TStrUpper		Utl_UStrUpper
#define Utl_TStrLower		Utl_UStrLower
#define Utl_TStrLTrim		Utl_UStrLTrim
#define Utl_TStrRTrim		Utl_UStrRTrim
#define Utl_TStrTrim		Utl_UStrTrim
#define Utl_TAtoi			Utl_UAtoi
#define Utl_TStrIsEmpty		Utl_UStrIsEmpty
#define Utl_TStrDigital		Utl_UStrDigital
#define Utl_TIsAlpha		Utl_UIsAlpha
#define TIToA2				UIToA2
#define TAToI2				UAToI2
#define TAToI16				UAToI16
#define Utl_TUnchange2		Utl_UUnchange2
#define Utl_TUnchange4		Utl_UUnchange4
#define Utl_TChange4		Utl_UChange4
#define Utl_TChange2		Utl_UChange2


#ifdef STR_32BITLEN
/********************begin 32bit length string operation functions************/
#define Utl_TStrLen			Utl_UStrLen32
#define Utl_TStrFnd			Utl_UStrFnd32
#define Utl_TStrFndN		Utl_UStrFndN32
#define Utl_TStrFndNL		Utl_UStrFndNL32
#define Utl_TStrFndL		Utl_UStrFndL32
#define Utl_TStrFndChr		Utl_UStrFndChr32
#define Utl_TStrFndChn		Utl_UStrFndChn32
#define Utl_TStrRevFnd		Utl_UStrRevFnd32
#define Utl_TStrRevFndChr	Utl_UStrRevFndChr32
#define Utl_TStrRevNCmp		Utl_UStrRevNCmp32
#define Utl_TStrCmpN		Utl_UStrCmpN32
#define Utl_TStrCmpNC		Utl_UStrCmpNC32
#define Utl_TStrCpyN		Utl_UStrCpyN32
#define Utl_TStrCpyChr		Utl_UStrCpyChr32
#define Utl_TStrCatN		Utl_UStrCatN32
#define Utl_TStrCatChr		Utl_UStrCatChr32
#define Utl_TStrIns			Utl_UStrIns32
#define Utl_TStrInsChr		Utl_UStrInsChr32
#define Utl_TStrRep			Utl_UStrRep32
#define Utl_TStrDel			Utl_UStrDel32
#define Utl_TStrDelChr		Utl_UStrDelChr32
#define Utl_TStrMid			Utl_UStrMid32
#define Utl_TStrMidL		Utl_UStrMidL32
#define Utl_TStrRight		Utl_UStrRight32
#define Utl_TItoa			Utl_UItoa32
#define Utl_TStrFndC		Utl_UStrFndC32
#define Utl_TStrFndNC		Utl_UStrFndNC32
#define Utl_TStrFndNLC		Utl_UStrFndNLC32
#define Utl_TStrFndLC		Utl_UStrFndLC32
#define Utl_TStrRevFndC		Utl_UStrRevFndC32
/********************end 32bit string length operation functions**************/
#else
/********************begin 16bit string length operation functions************/
#define Utl_TStrLen			Utl_UStrLen
#define Utl_TStrFnd			Utl_UStrFnd
#define Utl_TStrFndN		Utl_UStrFndN
#define Utl_TStrFndNL		Utl_UStrFndNL
#define Utl_TStrFndL		Utl_UStrFndL
#define Utl_TStrFndChr		Utl_UStrFndChr
#define Utl_TStrFndChn		Utl_UStrFndChn
#define Utl_TStrRevFnd		Utl_UStrRevFnd
#define Utl_TStrRevFndChr	Utl_UStrRevFndChr
#define Utl_TStrRevNCmp		Utl_UStrRevNCmp
#define Utl_TStrCmpN		Utl_UStrCmpN
#define Utl_TStrCmpNC		Utl_UStrCmpNC
#define Utl_TStrCpyN		Utl_UStrCpyN
#define Utl_TStrCpyChr		Utl_UStrCpyChr
#define Utl_TStrCatN		Utl_UStrCatN
#define Utl_TStrCatChr		Utl_UStrCatChr
#define Utl_TStrIns			Utl_UStrIns
#define Utl_TStrInsChr		Utl_UStrInsChr
#define Utl_TStrRep			Utl_UStrRep
#define Utl_TStrDel			Utl_UStrDel
#define Utl_TStrDelChr		Utl_UStrDelChr
#define Utl_TStrMid			Utl_UStrMid
#define Utl_TStrMidL		Utl_UStrMidL
#define Utl_TStrRight		Utl_UStrRight
#define Utl_TItoa			Utl_UItoa
#define Utl_TStrFndC		Utl_UStrFndC
#define Utl_TStrFndNC		Utl_UStrFndNC
#define Utl_TStrFndNLC		Utl_UStrFndNLC
#define Utl_TStrFndLC		Utl_UStrFndLC
#define Utl_TStrRevFndC		Utl_UStrRevFndC
/********************end  16bit string length operation functions*************/
#endif

/********************end UNICODE string operation functions*******************/

#endif

#endif //end of __TSTRING_API_H__
