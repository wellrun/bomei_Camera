/**
 * @file Eng_DataConvert.c
 * @brief ANYKA software
 * this file to process common data convert
 * @author Miaobaoli
 * @date 2001-06-25
 * @author
 */

#include "Gbl_Global.h"
#include "Eng_DataConvert.h"
#include "Eng_String.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osFS.h"
#include "Eng_String_UC.h"
#include <string.h>
#include "Ctl_AudioPlayer.h"
#include "Eng_convert_unicode.h"
#include "Eng_DynamicFont.h"
#include "eng_dynamicfont.h"
#include "Lib_res_port.h"

#ifdef OS_WIN32
#include <windows.h>
#endif

#define UNDEF2              0xA101

/**
 * @brief convert hex string to integer
 *
 * @author @b Miaobaoli
 *
 * @author
 * @date 2001-06-25
 * @param T_pSTR str_Hex:   source hex string pointer.
 * @return T_U16: the integer value.
 * @retval
 */
T_U16 ConverHexToi(T_pCSTR str_Hex)
{
    T_U16 i,c;
    T_S16            length,j;

    AK_FUNCTION_ENTER("ConverHexToi");
    AK_ASSERT_PTR(str_Hex, "ConverHexToi(): str_Hex", 0);

    length = Utl_StrLen((T_pSTR)str_Hex);
    i = 0;

    for( j=length-1; j>=0; j-- )
    {
        c = str_Hex[j];
        if('a' <= c && c <= 'f')
        {
            c -= 'a';
            c += 10;
        }
        else if('A' <= c && c <= 'F')
        {
            c -= 'A';
            c += 10;
        }
        else
            c -= '0';

        c <<= ((length - 1 - j) * 4);
        i |= c;
    }
    AK_FUNCTION_RET_INT("ConverHexToi",i);
    return i;
}

T_VOID Eng_SetLanguage(T_RES_LANGUAGE lang)
{
    AudioPlayer_Stop();

    //free old codepage from memory
    DynamicFont_Codepage_Free();

    Res_SetLanguage(gs.Lang);

    // load new codepage to memory
    Eng_Codepage_Init();

    return;
}


static T_S32 Eng_Unic2Eng(const T_U16 *unicode, T_U32 ucLen, T_S8 *pDestBuf, T_U32 destLen)
{
    T_U32 i;

    if (AK_NULL != pDestBuf && destLen > 0)
    {
        for(i=0; (i<ucLen && i<(destLen-1) && 0 != unicode[0]); i++)
            pDestBuf[i] = (T_U8)(unicode[i]);


        pDestBuf[i] = 0;
        if (i == destLen-1 && i < ucLen)
        i = 0xffffffff;
    }
    else
    {
        i = ucLen;
    }

    return (T_S32)i;
}

 
T_S32 Eng_WideCharToMultiByte(T_RES_LANGUAGE lang, const T_U16 *unicode, T_U32 ucLen, T_U32 *readedBytes, T_S8 *pDestBuf, T_U32 destLen, const T_S8 *defaultChr)
{
    T_S32           ret;

    if (AK_FALSE == dynamic_font.InitFlag)
    {
        return Eng_Unic2Eng(unicode, ucLen, pDestBuf, destLen);
    }

    ret = wine_cp_wcstombs(&dynamic_font.Cptable, WC_COMPOSITECHECK,
                      unicode, ucLen,
                      pDestBuf, destLen, defaultChr, AK_NULL );


    if (destLen && pDestBuf)
    {
        if (ret >= 0)
        {
            if (ret < (T_S32)destLen)
                pDestBuf[ret] = 0;
        }
        else
        {
            pDestBuf[0] = 0;
        }
    }

    return ret;
}



static T_S32 Eng_Eng2Unic(const T_S8 *src, T_U32 srcLen, T_U16 *ucBuf, T_U32 ucBufLen)
{
    T_U32 i;

    if (AK_NULL != ucBuf && ucBufLen > 0)
    {
        for(i=0; (i<srcLen && i<(ucBufLen-1) && 0 != src[i]); i++)
            ucBuf[i] = ((T_U16)(src[i])) & 0xff;

        ucBuf[i] = 0;

        if (i == ucBufLen-1 && i != srcLen)
            i = 0xffffffff;
    }
    else
    {
        i = srcLen;
    }

    return (T_S32)i;
}


T_S32 Eng_MultiByteToWideChar(T_RES_LANGUAGE lang, const T_S8 *src, T_U32 srcLen, T_U32 *convertedBytes, T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr)
{
    T_S32           ret;

    if (AK_FALSE == dynamic_font.InitFlag)
    {      
        return Eng_Eng2Unic(src, srcLen, ucBuf, ucBufLen);
    }

    ret = wine_cp_mbstowcs(&dynamic_font.Cptable, MB_PRECOMPOSED,
                      src, srcLen,
                      ucBuf, ucBufLen, defaultUChr);

    if (ucBufLen && ucBuf)
    {
        if (ret >= 0)
        {
            if (ret < (T_S32)ucBufLen)
                ucBuf[ret] = 0;
        }
        else
        {
            ucBuf[0] = 0;
        }
    }

    return ret;
}

/*************************************************************************
 
    FUNCTION: Eng_MbsToWcs
    AUTHOR: he_ying
    DATE: 2008-1-3
    NOTE:
            cbMultiByte should equal strlen(mbstr) 
            cchWideChar should equal strlen(mbstr)+1 (because '\0' is account in)
            if cchWideChar==0, Eng_MbsToWcs return required buffer size
            cbMultiByte<0 (include -1 !!!) will cause function fail. 
            return -1 on dst buffer overflow, -2 on invalid input char 
 
*************************************************************************/
T_S32 Eng_MbsToWcs(T_S8 *mbstr, T_S32 cbMultiByte, T_U16*wcstr, T_S32 cchWideChar)
{
    T_U16 ch = UNICODE_SPACE;
    T_S32 ret;
    
    ret = Eng_MultiByteToWideChar(gs.Lang,mbstr,cbMultiByte, AK_NULL,wcstr,cchWideChar,&ch);

    return ret;
}

/*************************************************************************
 
    FUNCTION: Eng_WcsToMbs
    AUTHOR: he_ying
    DATE: 2008-1-3
    NOTE: 
    if cbMultiByte==0 returns the number of bytes required for the buffer
    return -1 on dst buffer overflow
 
*************************************************************************/
T_S32 Eng_WcsToMbs(T_U16 *wcstr, T_U32 cchWideChar, T_S8 *mbstr, T_U32 cbMultiByte)
{
    T_S8 ch = UNICODE_SPACE;
    T_S32 ret;
    ret = Eng_WideCharToMultiByte(gs.Lang,wcstr,cchWideChar, AK_NULL,mbstr,cbMultiByte,&ch);
    return ret;
}

/**
 * @date 2007-01-10
 * @param [in] src: ansi string to be converted
 * @param [in] ucBuf: unicode string buffer
 * @return T_S32
 * @retval the size of unicode string
 */
T_S32 Eng_StrMbcs2Ucs(const T_S8 *src, T_U16 *ucBuf)
{
   T_S32 ULen = 0;
   T_U16 ch = UNICODE_SPACE; //space

   ULen = Eng_MultiByteToWideChar(gs.Lang, src, strlen(src), AK_NULL, ucBuf, strlen(src)+1, &ch);
   ucBuf[ULen] = 0;

   return ULen;
}


/**
 * @brief translate ansi string to unicode string 
 *   destination buffer is provided by user
 * 
 * @date 2007-01-10
 * @param [in] src: ansi string to be converted
 * @param [in] ucBuf: unicode string buffer provided by user
 * @return T_U16 * the unicode string point
 * @retval the size of unicode string
 */
T_U16 * Eng_StrMbcs2Ucs_2(const T_S8 *src, T_U16 *ucBuf)
{
   T_S32 ULen = 0;
   T_U16 ch = UNICODE_SPACE; //space

    ULen = Eng_MultiByteToWideChar(gs.Lang, src, strlen(src), AK_NULL, ucBuf, strlen(src)+1, &ch);
    ucBuf[ULen] = 0;

   return ucBuf;
}

/**
 * @brief change ansi string to unicode string
 *
 * @date 2007-01-10
 * @param [in] src: ansi string to be converted
 * @return T_U16 * the unicode string buffer point
 * @retval 
 */
T_U16 * Eng_StrMbcs2Ucs_3(const T_S8 *src)
{
   T_S32 ULen = 0;
   static T_USTR_INFO ucBuf;
   T_U16 ch = UNICODE_SPACE; //space

    ULen = Eng_MultiByteToWideChar(gs.Lang, src, strlen(src), AK_NULL, ucBuf, strlen(src)+1, &ch);
    ucBuf[ULen] = 0;

   return ucBuf;
}

T_S32 Eng_StrMbcs2Ucs_4(const T_S8 *src, T_U16 *ucBuf, T_U32 ucBufLen)
{
	T_S32 ULen = 0;
	T_U16 ch = UNICODE_SPACE; //space

	if (ucBufLen < (strlen(src)+1))
	{
		return -1;
	}
	ULen = Eng_MultiByteToWideChar(gs.Lang, src, strlen(src), AK_NULL, ucBuf, strlen(src)+1, &ch);
	ucBuf[ULen] = 0;

	return ULen;
}



/**
 * @date 2007-01-10
 * @param [in] usrc: unicode string to be converted
 * @param [in] strBuf: ansi string buffer
 * @return T_S32
 * @retval the size of ansi string
 */
T_S32 Eng_StrUcs2Mbcs(const T_U16 *usrc, T_S8 *strBuf)
{
   T_S32 strLen = 0;
   T_U8 ch = ' '; //blank char

    strLen = Eng_WideCharToMultiByte(gs.Lang, usrc, Utl_UStrLen(usrc), AK_NULL, strBuf, 512, &ch);//Utl_UStrLen(usrc)+1
    strBuf[strLen] = 0;

   return strLen;
}


/**
 * @brief Judge first char is a chinese char in GBK buffer
 *
 * @author @b LiaoJianhua
 *
 * @date 2005-07-26
 * @param gbk:[in] source GBK string
 * @param gbkLen:[in] the length of source string, in bytes
 * @return
 * @retval AK_TRUE, first char is a chinese char
 * @retval AK_FALSE, first char is not a chinese char
 * @note
 */
T_BOOL Eng_FirstIsGBKChn(const T_S8 *gbk, T_U32 gbkLen)
{
    T_U8 c1, c2;

    if(gbkLen >= 2)
    {
        c1 = gbk[0];
        c2 = gbk[1];
        if(c2>=0xa1 && c2<=0xfe)
        {
            if(c1>=0xa1 && c1<=0xa9)
            {
                return AK_TRUE;
            }
            else if(c1>=0xb0 && c1<=0xf7)
            {
                return AK_TRUE;
            }
        }
        if(c2>=0x40)
        {
            if(c2<=0xfe)
            {
                if(c1>=0x81 && c1<=0xa0)
                {
                    return AK_TRUE;
                }
            }
            if(c2<=0xa0)
            {
                if(c1>=0xa8 && c1<=0xfe)
                {
                    return AK_TRUE;
                }
            }
        }
    }
    return AK_FALSE;
}

/**
 * @brief Judge first char is a chinese char in BIG5 buffer
 *
 * @author @b Lizhuobin
 *
 * @date 2005-08-25
 * @param big5:[in] source BIG5 string
 * @param big5Len:[in] the length of source string, in bytes
 * @return
 * @retval AK_TRUE, first char is a big5 char
 * @retval AK_FALSE, first char is not a big5 char
 * @note
 */
T_BOOL Eng_FirstIsBIG5Chn(const T_S8 *big5, T_U32 big5Len)
{
    T_U8 c1, c2;

    if(big5Len >= 2)
    {
        c1 = big5[0];
        c2 = big5[1];
        if((c1>=0xa1) && (c1<=0xfe))
        {
            if((c2>=0x40) && (c2<=0x7e))
            {
                return AK_TRUE;
            }
            else if((c2>=0xa1) && (c2<=0xfe))
            {
                return AK_TRUE;
            }
        }
    }
    return AK_FALSE;
}

/************************************************************************/
/*                  update by : he_ying_gz
                    if you want compute readedUTF8Bytes correctly.
                    you will uncomment the last "break" 
                    and comment the last "++i".
                    but, if you do above action, 
                    this function will fail on incorrect utf8 stream, 
                    even mild error!!!
                                                  */
/************************************************************************/ 
T_S32 Eng_UTF82Unicode(const T_S8 *utf8, T_U32 utf8BufLen, T_U32 *readedUTF8Bytes, T_U16 *ucBuffer, T_S32 ucBufLen)
{
    T_U32        i=0, j=0;

    while ((utf8[i]!=0) && (i<utf8BufLen))
    {
        //if((utf8[i]&0x40)==0)        //invalide first-oct
        //{
        //    ucBuffer[j] = 0;
        //    return j;
        //}

        if(0 != ucBufLen)                //if ucBufLen is zero, only calc the needed converted buffer len
        {
            if(j >= (T_U32)ucBufLen)         //otherwise, if the output buffer is not enough, break
                break;
        }
        if((utf8[i]&0x80)==0x0)    //1 bytes
        {
            if ((0 != ucBufLen) && (AK_NULL != ucBuffer))
            {
                ucBuffer[j] = utf8[i];
                ucBuffer[j] &= T_U8_MAX;
            }
            ++j;
            ++i;
        }
        else if (((utf8[i]&0xe0)==0xc0) && ((i+1)<utf8BufLen))   //2 bytes
        {
            if ((0 != ucBufLen) && (AK_NULL != ucBuffer))
            {
                ucBuffer[j] = ((utf8[i]&0x1f)<<6)|(utf8[i+1]&0x3f);
            }
            ++j;
            i+=2;
        }
        else if (((utf8[i]&0xf0)==0xe0) && ((i+2)<utf8BufLen))   //3 bytes
        {
            if ((0 != ucBufLen) && (AK_NULL != ucBuffer))
            {
               ucBuffer[j] = ((T_U16)(utf8[i]&0x0f)<<12)|((utf8[i+1]&0x3f)<<6)|(utf8[i+2]&0x3f);
            }
            ++j;
            i+=3;
        }
        else    //invalide first-oct
        {
//             break;
            //update by ljh, ignore the invalide char byte
            ++i;
        }
    }
    if(readedUTF8Bytes)
    {
        (*readedUTF8Bytes) = i;
    }

    return j;
}


T_S32 Eng_Unicode2UTF8(const T_U16 *unicode, T_U32 ucLen, T_U32 *readedUCs, T_S8 *utf8, T_U32 utf8BufLen)
{
    T_U32        i=0, j=0;
    T_U16       uc = 0;

    while (unicode[i]!=0 && i<ucLen)
    {
        uc = unicode[i];

        if (uc < (T_U16)0x80)
        {
            if(0!=utf8BufLen)
            {
                utf8[j] = (T_U8)uc;
            }
            ++j;
        }
        else if (uc < (T_U16)0x800)
        {
            if(0!=utf8BufLen)
            {
                utf8[j] = (uc>>6)|0xc0;
                utf8[j+1] = (uc&0x3f)|0x80;
            }
            j+=2;
        }
        else
        {
            if(0!=utf8BufLen)
            {
                utf8[j] = ((uc>>12)&0x0f)|0xe0;
                utf8[j+1] = ((uc>>6)&0x3f)|0x80;
                utf8[j+2] = (uc&0x3f)|0x80;
            }
            j+=3;
        }
        ++i;
    }
    if(readedUCs)
    {
        (*readedUCs) = i;
    }
    if(0 != utf8BufLen)
    {
        utf8[j] = 0;
    }
    ++j;
    return j;

}



/**
 * @brief Convert UNICODE string to ansi string, just for file system lib
 * @param[in]   UniStr     source UNICODE string
 * @param[in]   UniStrLen       the length of source string, in UNICODE char unit
 * @param[out]  pAnsibuf        the output ansi string buffer
 * @param[in]   AnsiBufLen indicate the output ansi string buffer size, in bytes
 * @param[in]   code      language
 * @return T_S32
 * @retval if AnsiBufLen is zero, the return value is the required size, in bytes, for a buffer that can receive the translated string
 * @retval if AnsiBufLen is not zero, the return value is the number of bytes written to the buffer pointed to by pAnsi
 */
T_S32 UniStr2AnsiStr(const T_U16 *pUniStr, T_U32 UniStrLen, T_pSTR pAnsibuf, T_U32 AnsiBufLen, T_U32 code)
{
    return Eng_WideCharToMultiByte(code, pUniStr, UniStrLen, AK_NULL, pAnsibuf, AnsiBufLen, AK_NULL);
}

/**
 * @brief Convert ansi string to UNICODE string, just for file system lib
 * @param[in] pAnsiStr       source ansi string
 * @param[in] AnsiStrLen    the length of source string, in bytes
 * @param[out] pUniBuf    the output UNICODE string buffer
 * @param[in] UniBufLen  indicate the output UNICODE string buffer size, in UNICODE unit
 * @return T_S32
 * @retval if ucBufLen is zero, the return value is the required size, in UNICODE char, for a buffer that can receive the translated string
 * @retval if ucBufLen is not zero, the return value is the number of UNICODE chars written to the buffer pointed to by ucBuf
 */
T_S32 AnsiStr2UniStr(T_pSTR pAnsiStr, T_U32 AnsiStrLen, T_U16 *pUniBuf, T_U32 UniBufLen, T_U32 code)
{
    return Eng_MultiByteToWideChar(code, pAnsiStr, AnsiStrLen, AK_NULL, pUniBuf, UniBufLen, AK_NULL);
}


T_U16 Unichar2Ansi(T_U16 uni, T_U32 code)
{
    T_U16 ansi=0;
    //T_U8 str[4];
    T_U16 tmpUni[2];
    T_U32 readedBytes;
    T_U8  ansiBuf[4];

    //str[0] = ansi & 0xFF;
    //str[1] =
    tmpUni[0] = uni;
    tmpUni[1] = 0;


    Eng_WideCharToMultiByte(code, tmpUni, 1, &readedBytes, (T_S8 *)ansiBuf, 4, AK_NULL);
    ansi = ansiBuf[0] << 8;
    ansi += ansiBuf[1];
    return ansi;
}

T_U16 Ansi2Unichar(T_U16 ansi, T_U32 code)
{
    T_U16 uni=0;
    T_U16 tmpUni[4];
    T_U32 readedBytes;
    T_U8 ansiBuf[4];

    ansiBuf[0] = (T_U8)(ansi >> 8);
    ansiBuf[1] = (T_U8)(ansi & 0xff);
    tmpUni[0] = 0;

    Eng_MultiByteToWideChar(code, (const T_S8 *)ansiBuf, 2, &readedBytes, tmpUni, 4, AK_NULL);

    uni = tmpUni[0];
    return uni;
}

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

T_S32 Eng_MbToWc( T_WCHR *pwchar, T_pCSTR pmbchar, T_U32 count)
{
	//T_S32 ret = 0;
	T_U16 ch = UNICODE_SPACE; //space

	union cptable table = dynamic_font.Cptable;
	if ((AK_NULL==pmbchar) || (0 == count))
	{
		return 0;
	} 
	
	if ((table.info.char_size < 2/*MB_MAX*/) || (AK_FALSE == table.dbcs.cp2uni_leadbytes[(T_U8)(*pmbchar)]))
	{
		if (pwchar != AK_NULL)
		{
			*pwchar = (T_U8)(*pmbchar);
		}
		return sizeof(T_CHR);
	}

	if ((count >= table.info.char_size)
		&& (Eng_MultiByteToWideChar(gs.Lang, pmbchar, table.info.char_size, AK_NULL,
		pwchar, ((pwchar != AK_NULL) ? 1 : 0), &ch) != 0))
		
	{
		return table.info.char_size;
	}
	return -1;

}

/**
 * @brief convert utf16be to unicode
 *
 * @author he_ying_gz
 *
 * @date 2008-05-16
 * @param pwchar:[out] resulting wide character 
 * @param pmbchar:[in] utf16be string
 * @param count:[in] the length of utf16be string, in bytes
 * @return
 * @retval converted unicode
 */

T_S32 Eng_UTF16BEToWc(const T_U8 *pbe, T_U32 byteCount, T_WCHR *pwchar, T_U32 wcCount)
{
    T_U32 i=0;
    T_U32 count;
    T_U8 fristByte;
    T_U8 secondByte;

    count = (wcCount < byteCount>>1)? wcCount : byteCount>>1;
    while (i<count)
    {
        fristByte = pbe[i<<1];
        secondByte = pbe[i<<1|0x01];
        pwchar[i] = ((T_WCHR)fristByte)<<8 |secondByte;
        i++;
    }
    return count<<1;
}

T_VOID Eng_GetAspectRatio(T_U16* dstW, T_U16* dstH, T_U16 srcW, T_U16 srcH, T_U16 maxLen)
{
	if (0 == srcW || 0 == srcH)
		return;
	
	if (srcW >= srcH)
	{
		*dstW = maxLen;
		*dstH = maxLen * srcH / srcW;
	}
	else
	{
		*dstH = maxLen;
		*dstW = maxLen * srcW / srcH;
	}
}

/* end of files */



