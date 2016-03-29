/**
 * @file
 * @brief ANYKA software
 * this header file provide application layer string function
 *
 * @author PY.Xue, ZouMai
 * @date  2001-08-1
 * @version 1.0
 */

#include <string.h>
#include <ctype.h>
#include "Eng_String.h"
#include "Fwl_osMalloc.h"
#include "Eng_DataConvert.h"
#include "Gbl_Global.h"
#include "eng_string_uc.h"
#include "eng_debug.h"

extern T_S32 Eng_MultiByteToWideChar(T_RES_LANGUAGE lang, const T_S8 *src, T_U32 srcLen, T_U32 *readedBytes, T_U16 *ucBuf, T_U32 ucBufLen, const T_U16 *defaultUChr);

/**
 * @brief search for strSub in strMain from the index element
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR strMain head address of main string to search in
 * @param  T_pSTR strSub head address of sub string to search for
 * @param  T_S16 offset begin address offset
 * @return T_S16
 * @retval -1: not find; else offset in main string
 */
T_S16 Utl_StrFnd(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset)
{
    AK_ASSERT_PTR(strMain, "Utl_StrFnd(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_StrFnd(): strSub", -1);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrFnd()", -1);     /* length can't exceed MAX_STRING_LEN */
#endif

    return Utl_StrFndL(strMain, strSub, offset, Utl_StrLen(strMain));
}

/**
 * @brief search for strSub in strMain from the index element in the special range
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR strMain head address of main string to search in
 * @param  T_pSTR strSub head address of sub string to search for
 * @param  T_S16 offset begin address offset
 * @return T_S16
 * @retval -1: not find; else offset in main string
 */
T_S16 Utl_StrFndN(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 count)
{
    T_U16   curLoc = offset;
    T_U16   i;
    T_U16   subStrLen;
    T_U16   mainStrLen;
    T_pCSTR pMain;

    AK_ASSERT_PTR(strMain, "Utl_StrFndN(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_StrFndN(): strSub", -1);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrFndN()", -1);        /* length can't exceed MAX_STRING_LEN */
    AK_ASSERT_VAL(count < MAX_STRING_LEN, "Utl_StrFndN()", -1);     /* length can't exceed MAX_STRING_LEN */
#endif

    if (offset < 0)
        return -1;

    subStrLen = Utl_StrLen(strSub);
    mainStrLen = Utl_StrLen(strMain);
    if (offset + subStrLen > mainStrLen)
//  if (subStrLen > Utl_StrLen(strMain + offset))   /* if offset > length of strMain, error occur */
        return -1;
    if (subStrLen > count)
        return -1;

    pMain = strMain + offset;

    while (*(pMain + subStrLen - 1) != 0)
    {
        //AK_ASSERT_VAL(curLoc < MAX_STRING_LEN, "Utl_StrFndN()", -1);      /* length can't exceed MAX_STRING_LEN */

        for (i = 0; i < subStrLen; i++)
        {
            if (*(pMain + i) != *(strSub + i))
                break;
        }
        if (i == subStrLen)
        {
            return curLoc;
        }

        curLoc++;
        pMain++;
        if (curLoc + subStrLen > offset + count)
            break;
    }

    return -1;
}

T_S16 Utl_StrFndNL(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 count, T_U16 length)
{
    T_U16   curLoc = offset;
    T_U16   i;
    T_U16   subStrLen;
    T_U16   mainStrLen;
    T_pCSTR pMain;

    AK_ASSERT_PTR(strMain, "Utl_StrFndN(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_StrFndN(): strSub", -1);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrFndN()", -1);        /* length can't exceed MAX_STRING_LEN */
    AK_ASSERT_VAL(count < MAX_STRING_LEN, "Utl_StrFndN()", -1);     /* length can't exceed MAX_STRING_LEN */
#endif

    if (offset < 0)
        return -1;

    subStrLen = Utl_StrLen(strSub);
    //mainStrLen = Utl_StrLen(strMain);
    mainStrLen = length;
    if (offset + subStrLen > mainStrLen)
        return -1;
    if (subStrLen > count)
        return -1;

    pMain = strMain + offset;

    while (*(pMain + subStrLen - 1) != 0)
    {
        //AK_ASSERT_VAL(curLoc < MAX_STRING_LEN, "Utl_StrFndN()", -1);      /* length can't exceed MAX_STRING_LEN */

        for (i = 0; i < subStrLen; i++)
        {
            if (*(pMain + i) != *(strSub + i))
                break;
        }
        if (i == subStrLen)
        {
            return curLoc;
        }

        curLoc++;
        pMain++;
        if (curLoc + subStrLen > offset + count)
            break;
    }

    return -1;
}

/**
 * @brief search for a char in strMain from the index element
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR str head address of main string to search in
 * @param  T_S8 chr the char search for
 * @param  T_S16 index begin address offset
 * @return T_S16
 * @retval -1: not find; else offset in main string
 */
T_S16 Utl_StrFndChr(T_pCSTR strMain, T_S8 chr, T_S16 offset)
{
    T_U16   curLoc = offset;
    T_pCSTR pMain;

    AK_ASSERT_PTR(strMain, "Utl_StrFndChr(): strMain", -1);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrFndChr()", -1);      /* length can't exceed MAX_STRING_LEN */
#endif

    if (offset < 0)
        return -1;

    if (offset >= Utl_StrLen(strMain))
        return -1;

    pMain = strMain + offset;

    while (*pMain != 0)
    {
        //AK_ASSERT_VAL(curLoc < MAX_STRING_LEN, "Utl_StrFndChr(): curLoc", -1);        /* length can't exceed MAX_STRING_LEN */

        if (*pMain == chr)
        {
            return curLoc;
        }

        curLoc++;
        pMain++;
    }

    return -1;
}

/**
 * @brief Check if there are Chinese characters in the string.
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pDATA string
 * @return T_BOOL
 * @retval AK_TRUE: found Chinese character; AK_FALSE: not found.
 */
T_S16 Utl_StrFndChn(T_pCSTR strMain)
{
    T_U16   curLoc = 0;
    T_U16   mainStrLen;
    T_pCDATA    pMain;

    AK_ASSERT_PTR(strMain, "Utl_StrFndChn(): strMain", -1);

    mainStrLen = Utl_StrLen(strMain);
    pMain = (T_pDATA)strMain;

    if (mainStrLen < 2)
        return -1;

    while (*(pMain + 1) != 0)
    {
        if ((*pMain >= 0xa1) && (*(pMain + 1) >= 0xa1))
        {
            return curLoc;
        }
        curLoc++;
        pMain++;
    }

    return -1;
}

/**
 * @brief : look for subStr in the main string from the reverse direction.
 *
 * @author RongDian
 * @date  2001-08-1
 * @param T_pSTR strMain
 * @param  T_pSTR strSub
 * @param  T_S16 offset
 * @return T_S16
 * @retval -1: not found; else: the location of the main string that found the substring.
 */
T_S16   Utl_StrRevFnd(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset)
{
    T_U16 i, j, len1, len2, flag;

    AK_ASSERT_PTR(strMain, "Utl_StrRevFnd(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_StrRevFnd(): strSub", -1);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrRevFnd()", -1);      /* length can't exceed MAX_STRING_LEN */
#endif

    if (offset < 0)
        return -1;

    len1 = Utl_StrLen(strMain);
    len2 = Utl_StrLen(strSub);

    offset = ( offset > len1 - 1 ) ? ( len1 - 1 ) : offset;
    if( offset + 1 < len2 )
        return -1;

    for( i = offset; i >= len2 - 1; i--)
    {
        flag = 1;
        for( j = 0; j < len2; j++)
        {
            if( strMain[i - j] != strSub[len2 - 1 - j] )
            {
                flag = 0;
                break;
            }
        }

        if( flag == 1 )
            return i - len2 + 1;
    }

    return -1;
}

/**
 * @brief: look for 'ch' in the main string from the reverse direction.
 *
 * @author RongDian
 * @date  2001-08-1
 * @param T_pSTR strMain
 * @param  T_S8 chr
 * @param  T_S16 offset
 * @return T_S16
 * @retval -1: not found. else: the location of the main string that found the 'ch'
 */
T_S16 Utl_StrRevFndChr(T_pCSTR strMain, T_S8 chr, T_S16 offset)
{
    T_S16 i;

    AK_ASSERT_PTR(strMain, "Utl_StrRevFndChr(): strMain", -1);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrRevFndChr()", -1);       /* length can't exceed MAX_STRING_LEN */
#endif

    if (offset < 0)
        return -1;

    if(offset >= Utl_StrLen(strMain))
        offset = Utl_StrLen(strMain) -1;

    for (i = offset; i >= 0; i--)
    {
        if (strMain[i] == chr)
            return i;
    }

    return -1;
}

/**
 * @brief Compare two string
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR str1 head address of string1
 * @param  T_pSTR str2 head address of string1
 * @return T_S16
 * @retval compare value
 */
T_S8 Utl_StrCmp(T_pCSTR str1, T_pCSTR str2)
{
    T_S16   len = 0;
    T_pCDATA    pStr1 = (T_pCDATA)str1;
    T_pCDATA    pStr2 = (T_pCDATA)str2;

    AK_ASSERT_PTR(str1, "Utl_StrCmp()", 0);
    AK_ASSERT_PTR(str2, "Utl_StrCmp()", 0);

    while (((T_U8)(*(pStr1)) != 0) || ((T_U8)(*(pStr2)) != 0))
    {
        if ((*(pStr1)) > (*(pStr2)))
            return 1;

        if ((*(pStr1)) < (*(pStr2)))
            return -1;

        pStr1++;
        pStr2++;
        len++;
        //AK_ASSERT_VAL(len < MAX_STRING_LEN, "Utl_StrCmp()", -1);      /* length can't exceed MAX_STRING_LEN */
    }

    return 0;
}

T_S8 Utl_StrRevCmp(T_pCSTR str1, T_pCSTR str2)
{
    T_S16   len = 0, i, j;
    T_pCDATA    pStr1 = (T_pCDATA)str1;
    T_pCDATA    pStr2 = (T_pCDATA)str2;

    AK_ASSERT_PTR(str1, "Utl_StrCmp()", 0);
    AK_ASSERT_PTR(str2, "Utl_StrCmp()", 0);

    i = Utl_StrLen( str1 );
    j = Utl_StrLen( str2 );

    if( i == 0 || j == 0 )
    {
        return 1;
    }

    if( i > j )
    {
        pStr1 += ( i-j );
    }
    else if( i < j )
    {
        pStr2 += ( j-i );
    }

    while ( ( *pStr1 != 0 ) || ( *pStr2 != 0 ) )
    {
        if ((*(pStr1)) > (*(pStr2)))
            return 1;

        if ((*(pStr1)) < (*(pStr2)))
            return -1;

        pStr1++;
        pStr2++;
        len++;
        //AK_ASSERT_VAL(len < MAX_STRING_LEN, "Utl_StrCmp()", -1);      /* length can't exceed MAX_STRING_LEN */
    }

    return 0;
}

T_S8 Utl_StrRevNCmp(T_pCSTR str1, T_pCSTR str2, T_U8 length)
{
    T_S16   len = 0, i, j;

    AK_ASSERT_PTR(str1, "Utl_StrCmp()", 0);
    AK_ASSERT_PTR(str2, "Utl_StrCmp()", 0);

    i = Utl_StrLen( str1 );
    j = Utl_StrLen( str2 );

    while( len < length )
    {
        if( i == 0 && j == 0 )
        {
            break;
        }

        if( i==0 )
        {
            return -1;
        }

        if( j==0 )
        {
            return 1;
        }

        if( str1[ i-1 ] > str2[ j-1 ] )
        {
            return 1;
        }

        if( str1[ i-1 ] < str2[ j-1 ] )
        {
            return -1;
        }

        i--;
        j--;
        len++;
    }

    return 0;
}

/**
 * @brief Compare two string by length
 *
 * @author MiaoBaoLi
 * @date  2001-08-1
 * @param T_pSTR str1 head address of string1
 * @param  T_pSTR str2 head address of string1
 * @return T_S16
 * @retval compare value
 */
T_S8 Utl_StrCmpN(T_pCSTR str1, T_pCSTR str2, T_U16 length)
{
    T_U16   i;
    T_pCDATA    pStr1 = (T_pCDATA)str1;
    T_pCDATA    pStr2 = (T_pCDATA)str2;

    AK_ASSERT_PTR(str1, "Utl_StrCmpN()", 0);
    AK_ASSERT_PTR(str2, "Utl_StrCmpN()", 0);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(length < MAX_STRING_LEN, "Utl_StrCmpN()", 0);     /* length can't exceed MAX_STRING_LEN */
#endif

    for( i=0; i<length; i++ )
    {
        if( pStr1[i] > pStr2[i] )
        {
            return 1;
        }
        else if( pStr1[i] < pStr2[i] )
        {
            return -1;
        }
        if( pStr1[i] == 0 )
        {
            return 0;
        }
    }
    return 0;
}



/**
 * @brief Compare characters of two strings without regard to case by length
 *
 * @author MiaoBaoLi
 * @date  2001-08-1
 * @param T_pDATA s one string pointer
 * @param T_pDATA d another string pointer
 * @param T_U16 length
 * @return T_S16
 * @retval 0:   s substring identical to d substring
 *         1:   s substring greater than d substring
 *         -1: s substring less than d substring
 */
T_S8 Utl_StrCmpNC(T_pCSTR str1, T_pCSTR str2, T_U16 length)
{
    T_U8    c1,c2;
    T_U16  i;
    T_pDATA pStr1 = ( T_pDATA )str1;
    T_pDATA pStr2 = ( T_pDATA )str2;

    AK_ASSERT_PTR(str1, "Utl_StrCmpNC()", 0);
    AK_ASSERT_PTR(str2, "Utl_StrCmpNC()", 0);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(length < MAX_STRING_LEN, "Utl_StrCmpNC()", 0);        /* length can't exceed MAX_STRING_LEN */
#endif

    for( i=0; i<length; i++, pStr1++, pStr2++ )
    {
        c1 = ( T_U8 )( *pStr1 );
        if( c1 >= 'A' && c1 <= 'Z' )
            c1 += 0x20;

        c2 = ( T_U8 )( *pStr2 );
        if( c2 >= 'A' && c2 <= 'Z' )
            c2 += 0x20;

        if(c1 > c2)
        {
            return 1;
        }
        if(c1 < c2)
        {
            return -1;
        }
    }
    return 0;
}



/**
 * @brief copy one or more char to strDest
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_pSTR str
 * @param  T_S8 chr
 * @param  T_S16 count
 * @return T_S8
 * @retval
 */
T_pSTR Utl_StrCpyChr(T_pSTR strDest, T_S8 chr, T_U16 count)
{
    T_U16   i;

    AK_ASSERT_PTR(strDest, "Utl_StrCpyChr()", AK_NULL);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(count < MAX_STRING_LEN, "Utl_StrCpyChr()", AK_NULL);      /* length can't exceed MAX_STRING_LEN */
#endif

    if (strDest == AK_NULL)
        return strDest;

    strDest[0] = 0;
    if (count == 0)
        return strDest;

    for (i = 0; i < count; i++)
        strDest[i] = chr;
    strDest[i] = 0;

    return strDest;
}


/**
 * @brief connect strSub to strDest
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR strDest head address of destination char
 * @param  T_pSTR strSub head address of sub string
 * @return T_S8
 * @retval head address of destination string
 */
T_pSTR Utl_StrCat(T_pSTR strDest, T_pCSTR strSub)
{
    T_S16   i = 0;
    T_S16   len;

    AK_ASSERT_PTR(strDest, "Utl_StrCat()", AK_NULL);
    AK_ASSERT_PTR(strSub, "Utl_StrCat()", AK_NULL);

    len = Utl_StrLen(strDest);

    while (*(strSub + i++) != 0)
    {
        //AK_ASSERT_VAL(i < MAX_STRING_LEN, "Utl_StrCat()", AK_NULL);       /* length can't exceed MAX_STRING_LEN */

        strDest[len + i - 1] = strSub[i - 1];
    }
    strDest[len + i - 1] = 0;

    return strDest;
}


/**
 * @brief connect one or more char to strDest
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR strDest head address of destination string
 * @param  T_S8 chr source char
 * @param  T_U16 count
 * @return T_S8
 * @retval head address of destination string
 */
T_pSTR Utl_StrCatChr(T_pSTR strDest, T_S8 chr, T_S16 count)
{
    T_U16   i;
    T_U16   len;

    AK_ASSERT_PTR(strDest, "Utl_StrCatChr()", AK_NULL);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(count < MAX_STRING_LEN, "Utl_StrCatChr()", AK_NULL);      /* length can't exceed MAX_STRING_LEN */
#endif

    len = Utl_StrLen(strDest);
    if (count <= 0)
        return strDest;

    for (i = 0; i < count; i++)
        strDest[len + i] = chr;
    strDest[len + i] = 0;

    return strDest;
}

/**
 * @brief Insert one string(strSub) into another string(strDest).
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR strDest destination string
 * @param  T_pSTR strSub sub-string
 * @param  T_S16 offset the location of strDest where the strSub will be added in
 * @return T_S8
 * @retval head of destination string
 */
T_pSTR Utl_StrIns(T_pSTR strDest, T_pCSTR strSub, T_S16 offset)
{
    T_U16   lenDest, lenSub;
    T_S16   i;
    T_S16   curOffset = (T_S16)offset;

    AK_ASSERT_PTR(strDest, "Utl_StrIns()", AK_NULL);
    AK_ASSERT_PTR(strSub, "Utl_StrIns()", AK_NULL);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrIns()", AK_NULL);        /* length can't exceed MAX_STRING_LEN */
#endif

    if (offset < 0)
        return strDest;

    lenDest = Utl_StrLen(strDest);
    lenSub  = Utl_StrLen(strSub);

    if (lenSub == 0)
        return strDest;
    if ((T_U16)curOffset > lenDest)
        curOffset = lenDest;
    strDest[lenDest + lenSub] = 0;
    for (i = lenDest + lenSub - 1; i >= (T_S16)(curOffset + lenSub); i--)
        strDest[i] = strDest[i-lenSub];
    for (i = curOffset + lenSub - 1; i >= curOffset; i--)
        strDest[i] = strSub[i - curOffset];

    return strDest;
}// end Utl_StrIns(T_pSTR strDest, T_pSTR strSub, T_S16 offset)

/**
 * @brief Insert one character into a string(strDest).
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR strDest destination string
 * @param  T_S8 chr sub-character
 * @param  T_S16 offset the location of strDest where the strSub will be added in
 * @return T_S8
 * @retval head of destination string
 */
T_pSTR Utl_StrInsChr(T_pSTR strDest, T_S8 chr, T_S16 offset)
{
    T_S8    sTemp[2];

    AK_ASSERT_PTR(strDest, "Utl_StrInsChr()", AK_NULL);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrInsChr()", AK_NULL);     /* length can't exceed MAX_STRING_LEN */
#endif

    sTemp[0] = chr;
    sTemp[1] = '\0';

    return Utl_StrIns(strDest, sTemp, offset);
}// end Utl_StrIns(T_pSTR strDest, T_pSTR strSub, T_S16 offset)

/**
 * @brief Replace partial characters of one string(strDest) with another string(strSub).
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR strDest destination string
 * @param  T_pSTR strSub sub-string
 * @param  T_S16 offset the location of strDest where the strSub will replace it
 * @return T_S8
 * @retval the head of destination string.
 */
T_pSTR Utl_StrRep(T_pSTR strDest, T_pCSTR strSub, T_S16 offset)
{
    T_U16   lenDest, lenSub;
    T_U16   i;

    AK_ASSERT_PTR(strDest, "Utl_StrRep()", AK_NULL);
    AK_ASSERT_PTR(strSub, "Utl_StrRep()", AK_NULL);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrRep()", AK_NULL);        /* length can't exceed MAX_STRING_LEN */
#endif

    if (offset < 0)
        return strDest;

    lenDest = Utl_StrLen(strDest);
    lenSub  = Utl_StrLen(strSub);

    if (offset < 0)
        offset = 0;
    for (i = offset; i < offset + lenSub; i++)
        strDest[i] = strSub[i - offset];
    if (i > lenDest)
        strDest[i] = 0;

    return strDest;
}// end Utl_StrRep(T_pSTR strDest, T_pSTR strSub, T_S16 offset)

/**
 * @brief Empty a string
 *
 * @author ZouMai
 * @date  2001-11-09
 * @param T_pSTR strDest destination string
 * @return T_pSTR
 * @retval the header of destination string.
 */
T_pSTR Utl_StrEmpty(T_pSTR strDest)
{
    AK_ASSERT_PTR(strDest, "Utl_StrEmpty()", 0);

    strDest[0] = '\0';
    return strDest;
}

/**
 * @brief Delete some characters from string(strDest).
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_pSTR strDest destination string
 * @param  T_S16 offset the location where begin to delete
 * @param  T_S16 iLen the length of deleted characters
 * @return T_S8
 * @retval the header of destination string.
 */
T_pSTR Utl_StrDel(T_pSTR strDest, T_S16 offset, T_U16 count)
{
    T_U16   lenDest;
    T_U16   i;

    AK_ASSERT_PTR(strDest, "Utl_StrDel()", AK_NULL);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrDel()", AK_NULL);        /* length can't exceed MAX_STRING_LEN */
    AK_ASSERT_VAL(count < MAX_STRING_LEN, "Utl_StrDel()", AK_NULL);     /* length can't exceed MAX_STRING_LEN */
#endif

    if (offset < 0)
        return strDest;

    lenDest = Utl_StrLen(strDest);
    if (offset > lenDest - 1)
        offset = lenDest - 1;
    if (count > lenDest - offset)
        count = lenDest - offset;
    for (i = offset; i < lenDest - count; i++)
    {
        //AK_ASSERT_VAL(i < MAX_STRING_LEN, "Utl_StrDel()", AK_NULL);       /* length can't exceed MAX_STRING_LEN */

        strDest[i] = strDest[i + count];
    }
   strDest[i] = '\0';

    return strDest;
}// end Utl_StrDel(T_pSTR strDest, T_S16 offset, T_S16 iLen)


/**
 * @brief Remove 'ch' from the string.
 *
 * @author @b
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_pSTR strDest: header of the main string.
 * @param  T_S8 chr:
 * @param  T_S16 offset
 * @param  T_S16 count: -1: delete all chr
 * @return T_pSTR
 * @retval
 */
T_pSTR Utl_StrDelChr(T_pSTR strDest, T_S8 chr, T_S16 offset, T_S16 count)
{
    T_U16 j, i = offset;
    T_U16 len = Utl_StrLen(strDest);
    T_S16 num = 0;

    AK_ASSERT_PTR(strDest, "Utl_StrDelChr(): strDest", AK_NULL);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrDelChr(): offset", AK_NULL);     /* length can't exceed MAX_STRING_LEN */
    AK_ASSERT_VAL(count < MAX_STRING_LEN, "Utl_StrDelChr(): count1", AK_NULL);      /* length can't exceed MAX_STRING_LEN */
    AK_ASSERT_VAL(count > (-1)*MAX_STRING_LEN, "Utl_StrDelChr(): count2", AK_NULL);     /* length can't exceed MAX_STRING_LEN */
#endif

    if (offset < 0)
        return strDest;

    if (count == 0)
        return strDest;

    while( strDest[i] != '\0' )
    {
        if( strDest[i] == chr )
        {
            for( j=i; j<=len-1; j++ )
                strDest[j] = strDest[j+1];
            len--;
            if( (++num >= count) && (count > 0) )
                break;
        }
        else
        {
            i++;
            #if MAX_STRING_LEN < 0xFFFF
            AK_ASSERT_VAL(i < MAX_STRING_LEN, "Utl_StrDelChr(): i", AK_NULL);       /* length can't exceed MAX_STRING_LEN */
            #endif
        }
    }
    return strDest;
}

/**
 * @brief cur strSour from offset to end and copy to strDest
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR strDest: head address of dest string
 * @param  T_pSTR strSour: head address of source string
 * @param  T_S16 offset: begin index
 * @param  T_S16 end:  end index
 * @return T_S8
 * @retval void
 */
T_pSTR Utl_StrMid(T_pSTR strDest, T_pCSTR strSour, T_S16 offset, T_S16 end)
{
    T_U16   iLength;

    iLength = Utl_StrLen( strSour );

    return Utl_StrMidL( strDest, strSour, offset, end, iLength );
}

/**
 * @brief : get the last character of the string.
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR str
 * @return T_S8
 * @retval Result string pointer
 */
T_pSTR Utl_StrRight(T_pSTR strDest, T_pCSTR strMain, T_U16 count)
{
    T_U16       length;

    AK_ASSERT_PTR(strDest, "Utl_StrRight()", AK_NULL);
    AK_ASSERT_PTR(strMain, "Utl_StrRight()", AK_NULL);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(count < MAX_STRING_LEN, "Utl_StrRight()", AK_NULL);       /* length can't exceed MAX_STRING_LEN */
#endif

    length = Utl_StrLen(strMain);

    if (count > length)
        count = length;

    Utl_StrCpy(strDest, strMain + length - count);
    return strDest;
}

/**
 * @brief : get the last character of the string.
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR str
 * @return T_S8
 * @retval Result string pointer
 */
T_CHR Utl_StrRightChr(T_pCSTR strMain)
{
    AK_ASSERT_PTR(strMain, "Utl_StrRightChr()", 0);

    if (strMain[0] == '\0')     /* length == 0 */
        return 0;

    return strMain[Utl_StrLen(strMain)-1];  /* length > 0 */
}

/**
 * @brief : convert a string to up case string.
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR str
 * @return T_S8
 * @retval Result string pointer
 */
T_pSTR Utl_StrUpper(T_pSTR strMain)
{
    T_S8    *p;
    T_S16   i = 0;

    AK_ASSERT_PTR(strMain, "Utl_StrUpper()", AK_NULL);

    p = strMain;
    while (*p)
    {
        if (0x61 <= *p && *p <= 0x7a)
            *p -= 0x20;
        p++;
        i++;
        #if MAX_STRING_LEN < 0xFFFF
        AK_ASSERT_VAL(i < MAX_STRING_LEN, "Utl_StrUpper()", AK_NULL);       /* length can't exceed MAX_STRING_LEN */
        #endif
    }
    return strMain;
}

/**
 * @brief Conver string to lower case string.
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR str
 * @return T_S8
 * @retval result string pointer.
 */
T_pSTR Utl_StrLower(T_pSTR strMain)
{
    T_S8    *p = AK_NULL;
    T_S16   i = 0;

    AK_ASSERT_PTR(strMain, "Utl_StrLower()", AK_NULL);

    p = strMain;
    while (*p)
    {
        if (0x41 <= *p && *p <= 0x5a)
            *p += 0x20;
        p++;
        i++;
        #if MAX_STRING_LEN < 0xFFFF
        AK_ASSERT_VAL(i < MAX_STRING_LEN, "Utl_StrLower()", AK_NULL);       /* length can't exceed MAX_STRING_LEN */
        #endif
    }
    return strMain;
}

/**
 * @brief remove the start blank character.
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR str
 * @return T_S8
 * @retval The result string pointer.
 */
T_pSTR Utl_StrLTrim(T_pSTR strMain)
{
    T_S16   blank = 0;
    T_S8    *p = AK_NULL;
    T_S16   i = 0;

    AK_ASSERT_PTR(strMain, "Utl_StrLTrim()", AK_NULL);

    p = strMain;
    while (*(p++) == ' ')
    {
        blank++;
        i++;
        #if MAX_STRING_LEN < 0xFFFF
        AK_ASSERT_VAL(i < MAX_STRING_LEN, "Utl_StrLTrim()", AK_NULL);       /* length can't exceed MAX_STRING_LEN */
        #endif
    }

    p--;
    i = 0;
    while (*(p++ - 1))
    {
        *(p - blank - 1) = *(p - 1);
        i++;
        #if MAX_STRING_LEN < 0xFFFF
        AK_ASSERT_VAL(i < MAX_STRING_LEN, "Utl_StrLTrim()", AK_NULL);       /* length can't exceed MAX_STRING_LEN */
        #endif
    }

    return strMain;
}

/**
 * @brief remove the end blank character.
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR str
 * @return T_S8
 * @retval
 */
T_pSTR Utl_StrRTrim(T_pSTR strMain)
{
    T_U16   len;
    T_S8    *p = AK_NULL;

    AK_ASSERT_PTR(strMain, "Utl_StrRTrim()", AK_NULL);

    len = Utl_StrLen(strMain);
    p = strMain + len - 1;
    while ((*(p--) == ' ') && (len > 0))
        len--;

    strMain[len] = '\0';
    return strMain;
}

/**
 * @brief remove the start and end blank character.
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR str
 * @return T_S8
 * @retval
 */
T_pSTR Utl_StrTrim(T_pSTR strMain)
{
    AK_ASSERT_PTR(strMain, "Utl_StrTrim()", AK_NULL);

    return Utl_StrLTrim(Utl_StrRTrim(strMain));
}

/*
* @brief Carve up on-line  text(ansi) to multi-line(unicode) text according line width limit
* @autor zhengwenbo
* @date 2007-1-23
* @parm
* @return
* @retval
*/
T_BOOL Utl_StrCarve_Ansi2Unic(T_pSTR pStrSour, T_U16 WidthLimit, T_CARVED_USTR *pUCarvedStr, T_U16 WordWith)
{
	T_USTR_INFO UString;
    T_U16 ch = UNICODE_SPACE; //blank char
    T_U32 ustrLen = 0;

    ustrLen = Eng_MultiByteToWideChar(gs.Lang,pStrSour,strlen(pStrSour),AK_NULL,UString,strlen(pStrSour)+1,&ch);

	UString[ustrLen] = 0; // string end flag
	Utl_UStrCarve(UString, WidthLimit, AK_NULL, pUCarvedStr, WordWith);

	return AK_TRUE;
}

/**
 * @brief Carve up one-line text to multi-line text by spmark.
 * if "\n" is found in strSour, this function will carve it
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR strSour: source string (one-line text)
 * @param  T_S16 limit: the upper limit of the character quantity of carved line
 * @param  T_pSTR spmark
 * @param  T_CARVED_STR *CarvedStr: destination string (multi-line text)
 * @return T_BOOL
 * @retval line number after carved up
 */
T_BOOL Utl_StrCarve(T_pCSTR strSour, T_U16 limit, T_CARVED_STR *CarvedStr)
{
    T_U8    *strTemp = 0;
    T_U8    *start=0,*next = 0;
    T_S32 length = 0;
    T_U16 char_no = 0;
    T_U16 ret=0;
    T_BOOL  ret_flag;

    AK_ASSERT_PTR(strSour, "Utl_StrCarve()", AK_FALSE);
    AK_ASSERT_PTR(CarvedStr, "Utl_StrCarve()", AK_FALSE);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(limit < MAX_STRING_LEN, "Utl_StrCarve()", AK_FALSE);      /* length can't exceed MAX_STRING_LEN */
#endif

    CarvedStr->LineNum = 0;
    CarvedStr->MaxLen = limit;
    if (limit == 0)
        return AK_FALSE;

    length = strlen(strSour);
    if (length == 0)
        return AK_FALSE;

    strTemp = (T_U8*)strSour;
    if (strTemp == AK_NULL)
        return AK_FALSE;

    start = strTemp;
    next = strTemp;
    while(length>0)
    {
        ret_flag = AK_FALSE;
        //english char and not "\r\n" or "\n"
        if((*next>0)&&(*next<=0x80)&&(*next!='\r')&&(*next!='\n'))
        {
            next++;
            length--;
            char_no++;
            if(char_no==limit)
                ret = 1;
        }
        //dword char
        else if(*next>0x80)
        {
            if(char_no+2 < limit)
            {
            next += 2;
            length -= 2;
            char_no += 2;
            }
            else if(char_no+2 == limit)
            {
                next += 2;
                length -= 2;
                char_no += 2;
                ret = 1;
            }
            else
                ret = 1;
        }
        //"\r"
        else if(*next == '\r')
        {
            next++;
            length--;
        }
        //"\n"
        else if (*next == '\n')
        {
            next++;
            length--;
            ret = 1;
            ret_flag = AK_TRUE;
        }
        else if(*next==0)
        {
            ret = 1;
        }

        if(ret == 1)
        {
            ret = 0;
            //don't repeat add line,example:"\r\r\r\n"
            while((ret_flag == AK_FALSE) && (*next == '\r' || *next == '\n')){
                if (*next == '\n'){ //break when find the first '\n'
                    next++;
                    length--;
                    break;
                }
                next++;
                length--;
            }
            //don't repeat add line

            CarvedStr->String[CarvedStr->LineNum] = Fwl_Malloc(limit+1);
            if((T_U16)(CarvedStr->LineNum) == MAX_STR_LINE-1)
            {
                char_no = 0;
                return AK_TRUE;
            }
            if((T_U32)(CarvedStr->String[CarvedStr->LineNum])==0)
            {
                char_no = 0;
                return AK_TRUE;
            }
            if(CarvedStr->String[CarvedStr->LineNum] != 0)
            {
                Utl_StrCpyN(CarvedStr->String[CarvedStr->LineNum],start,char_no);
                CarvedStr->LineNum ++;
                start=next;
                char_no = 0;
            }
            char_no = 0;
        }
    }
    if( char_no > 0)
    {
        CarvedStr->String[CarvedStr->LineNum] = Fwl_Malloc(limit+1);
        Utl_StrCpyN(CarvedStr->String[CarvedStr->LineNum],start,char_no);
        CarvedStr->LineNum ++;
    }
    return AK_TRUE;
}/* end Utl_StrCarve(T_pSTR strSour, T_S16 limit, T_CARVED_STR *CarvedStr) */

/**
 * @brief Init structrue T_CARVED_STR
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_CARVED_STR *CarvedStr: carved string structure pointer.
 * @return T_BOOL
 * @retval
 */
T_BOOL Utl_StrCarveInit(T_CARVED_STR *CarvedStr)
{
    AK_ASSERT_PTR(CarvedStr, "Utl_StrCarveInit()", AK_FALSE);

    CarvedStr->LineNum = 0;
    CarvedStr->MaxLen = 0;
    return AK_TRUE;
}

/**
 * @brief Free a carve
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_CARVED_STR *CarvedStr: carved string structure pointer.
 * @return T_BOOL
 * @retval
 */
T_BOOL Utl_StrCarveFree(T_CARVED_STR *CarvedStr)
{
    T_U16   i;

    AK_ASSERT_PTR(CarvedStr, "Utl_StrCarveFree(): CarvedStr", AK_FALSE);
    AK_ASSERT_VAL(CarvedStr->LineNum <= MAX_STR_LINE, "Utl_StrCarveFree(): LineNum", AK_FALSE);     /* length can't exceed MAX_STRING_LEN */

    for (i = 0; i < CarvedStr->LineNum; i++)
    {
    	if (CarvedStr->String[i] != AK_NULL)
		{
	        CarvedStr->String[i] = Fwl_Free(CarvedStr->String[i]);
		}
    }
    CarvedStr->LineNum = 0;
    CarvedStr->MaxLen = 0;
    return AK_TRUE;
}

/**
 * @brief the string is empty or not
 *
 * @author ZouMai
 * @date   2001-08-1
 * @param T_pSTR str strMain--head address of string
 * @return T_S16
 * @retval the length
 */
T_BOOL  Utl_StrIsEmpty(T_pCSTR strMain)
{
    AK_ASSERT_PTR(strMain, "Utl_StrIsEmpty(): strMain", AK_FALSE);

    if (*strMain == '\0')
        return AK_TRUE;

    return AK_FALSE;
}

/**
 * @brief Check the first of the string is Chinese character or not.
 *
 * @author Junhua Zhao
 * @date  2005-05-10
 * @param T_pDATA string
 * @return T_BOOL
 * @retval AK_TRUE: found DWORD character; AK_FALSE: not found.
 */
T_BOOL Utl_IsDWordChar(T_pCSTR strMain, T_RES_LANGUAGE Lang)
{
    T_pCDATA pMain;
    T_BOOL ret;

    AK_ASSERT_PTR(strMain, "Utl_IsDWordChar(): strMain", AK_FALSE);

    pMain = (T_pCDATA)strMain;
    switch(Lang)
    {
    case eRES_LANG_CHINESE_SIMPLE:
    case eRES_LANG_CHINESE_TRADITION:
    case eRES_LANG_CHINESE_BIG5:
        ret = Eng_FirstIsGBKChn(strMain,strlen(strMain));
        break;
    case eRES_LANG_ENGLISH:
        ret = AK_FALSE;
    default:
        if (*(pMain) > 0x80) //double word char
            ret = AK_TRUE;
        else ret = AK_FALSE;
        break;
    }

    return ret;
}

/**
 * @brief Check the string is digital character or not.
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pDATA string
 * @return T_BOOL
 * @retval AK_TRUE: found Chinese character; AK_FALSE: not found.
 */
T_BOOL Utl_StrDigital(T_pCSTR strMain)
{
    T_U16   i;
    T_U16   len;

    AK_ASSERT_PTR(strMain, "Utl_StrDigital()", AK_FALSE);

    len = Utl_StrLen(strMain);

    for (i = 0; i < len; i++)
    {
        if ( ( strMain[i] <= '9' && strMain[i] >= '0' ) || strMain[i] == '+' || strMain[i] == '*' )
        {
        }
        else
        {
            return AK_FALSE;
        }
    }

    return AK_TRUE;
}

/**
 * @brief check if the 'ch' belong to alphabet
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_S8 ch
 * @return T_S16
 * @retval 0: yes. 0: no.
 */
T_S16 Utl_IsAlpha(T_S8 chr)
{
    if (((chr >= 'a') && (chr <= 'z')) ||
        ((chr >= 'A') && (chr <= 'Z')) )
        return 0;
    else
        return -1;
}

//add by Jassduke
/*
* @brief  convert a  int64 number to ANSI string
*/

T_pSTR Utl_I64toa(T_S64 intNum , T_pSTR strDest, T_U8 flag)
{
    T_S16       i = 0;
    T_S64       datanew;
    T_S16       index;
    T_STR_100   strTemp;
    T_BOOL      negv = AK_FALSE;

    AK_ASSERT_PTR(strDest, "Utl_I64toa()", AK_NULL);

    strDest[0] = '\0';
    if (intNum < 0)
    {
        intNum *= (-1);
        negv = AK_TRUE;
    }

    if (flag == 16)
    {
        if (intNum < 16)
        {
            if(intNum >= 10)
                strDest[0] = (T_S8)(intNum + 55);
            else
                strDest[0] = (T_S8)(intNum + 48);
            strDest[1] = '\0';
        }
        else
        {
            while (intNum >= 16)
            {
                datanew = intNum;
                intNum = intNum/16;
                if((datanew - intNum*16) >= 10)
                    strTemp[i] = (T_S8)(datanew - intNum * 16 + 55);
                else
                    strTemp[i] = (T_S8)(datanew - intNum * 16 + 48);
                i ++ ;
                if (intNum < 16)
                {
                    if(intNum >= 10)
                        strTemp[i] = (T_S8)(intNum + 55);
                    else if(intNum != 0)
                        strTemp[i] = (T_S8)(intNum + 48);
                    strTemp[i + 1] = 0;
                    break;
                }
            }
            for( index = 0; index <= i; index ++)
                *(strDest + index) = strTemp[i - index];
            *(strDest + index) = 0;
        }
    }
    else
    {
        if (intNum < 10)
        {
            strDest[0] = (T_S8)(intNum + 48);
            strDest[1] = '\0';
        }
        else
        {
            while(intNum >= 10)
            {
                datanew = intNum;
                intNum = intNum/10;
                strTemp[i] = (T_S8)(datanew - intNum * 10 + 48);
                i ++ ;
                if(intNum < 10)
                {
                    strTemp[i] = (T_S8)(intNum + 48);
                    strTemp[i + 1] = 0;
                    break;
                }
            }
            for( index = 0; index <= i; index ++)
                *(strDest + index) = strTemp[i - index];
            *(strDest + index) = 0;
        }
    }

    if (negv)
        Utl_StrIns(strDest, "-", 0);

    return strDest;


}



/**
 * @brief Conver a integer number to string.
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_S16 intNum
 * @param  T_pSTR strDest
 * @param  T_S8 flag
 * @return T_S8
 * @retval result string pointer.
 */
T_pSTR Utl_Itoa(T_S32 intNum, T_pSTR strDest, T_U8 flag)
{
    T_S16       i = 0;
    T_S32       datanew;
    T_S16       index;
    T_STR_100   strTemp;
    T_BOOL      negv = AK_FALSE;

    AK_ASSERT_PTR(strDest, "Utl_Itoa()", AK_NULL);

    strDest[0] = '\0';
    if (intNum < 0)
    {
        intNum *= (-1);
        negv = AK_TRUE;
    }

    if (flag == 16)
    {
        if (intNum < 16)
        {
            if(intNum >= 10)
                strDest[0] = (T_S8)(intNum + 55);
            else
                strDest[0] = (T_S8)(intNum + 48);
            strDest[1] = '\0';
        }
        else
        {
            while (intNum >= 16)
            {
                datanew = intNum;
                intNum = intNum/16;
                if((datanew - intNum*16) >= 10)
                    strTemp[i] = (T_S8)(datanew - intNum * 16 + 55);
                else
                    strTemp[i] = (T_S8)(datanew - intNum * 16 + 48);
                i ++ ;
                if (intNum < 16)
                {
                    if(intNum >= 10)
                        strTemp[i] = (T_S8)(intNum + 55);
                    else if(intNum != 0)
                        strTemp[i] = (T_S8)(intNum + 48);
                    strTemp[i + 1] = 0;
                    break;
                }
            }
            for( index = 0; index <= i; index ++)
                *(strDest + index) = strTemp[i - index];
            *(strDest + index) = 0;
        }
    }
    else
    {
        if (intNum < 10)
        {
            strDest[0] = (T_S8)(intNum + 48);
            strDest[1] = '\0';
        }
        else
        {
            while(intNum >= 10)
            {
                datanew = intNum;
                intNum = intNum/10;
                strTemp[i] = (T_S8)(datanew - intNum * 10 + 48);
                i ++ ;
                if(intNum < 10)
                {
                    strTemp[i] = (T_S8)(intNum + 48);
                    strTemp[i + 1] = 0;
                    break;
                }
            }
            for( index = 0; index <= i; index ++)
                *(strDest + index) = strTemp[i - index];
            *(strDest + index) = 0;
        }
    }

    if (negv)
        Utl_StrIns(strDest, "-", 0);

    return strDest;
}

/**
 * @brief : Convert string to integer.
 *
 * @author PY.Xue
 * @date   2001-08-1
 * @param T_pDATA strDest
 * @return T_S16
 * @retval integer value.
 */
T_S32 Utl_Atoi(T_pCSTR strMain)
{
	T_S32		sum;
	T_pCDATA	pMain = AK_NULL;
	T_BOOL		negv = AK_FALSE;
	AK_ASSERT_PTR(strMain, "Utl_Atoi()", 0);
	pMain = (T_pCDATA)strMain;
	sum = 0;
	if ((*pMain) == '-')
	{
		negv = AK_TRUE;
		pMain++;
	}
	while (*pMain)
	{
		if ('0' <= (*pMain) && (*pMain) <= '9')
			sum = sum * 10 + (*pMain - '0');
		else
			break;
		pMain++;
	}
	if (negv)
		sum = 0-sum;
	return sum;
}

/**
 * @brief change a number (less then 16) into a string (four bytes)
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_S16 number: the change number.
 * @param  T_pSTR string: result string pointer.
 * @return T_BOOL
 * @retval
 */
T_BOOL IToA2(T_S16 number, T_pSTR string)
{
    T_S16   temp;
    T_S8    tempString[2];

    AK_ASSERT_PTR(string, "IToA2()", AK_FALSE);

    if(number > 0xf)
    {
        *string = '\0';
        return AK_FALSE;
    }

    temp = number / 8;
    Utl_Itoa(temp,tempString,10);
    *(string) = tempString[0];
    number -= temp * 8;

    temp = number / 4;
    Utl_Itoa(temp,tempString,10);
    *(string + 1) = tempString[0];
    number -= temp * 4;

    temp = number / 2;
    Utl_Itoa(temp,tempString,10);
    *(string + 2) = tempString[0];
    number -= temp * 2;

    Utl_Itoa(number,tempString,10);
    *(string + 3) = tempString[0];

    *(string + 4) = '\0';
    return AK_TRUE;
}

/**
 * @brief change a string into a number, for example, "110001" -->0x31
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_pSTR string: the change string
 * @return T_S16
 * @retval retule value
 */
T_S16 AToI2(T_pCSTR string)
{
    T_S16   result = 0;
    T_U16   i;
    T_U16   length;
    T_S16   Power[9] = {0,1,2,4,8,16,32,64,128};

    AK_ASSERT_PTR(string, "AToI2()", 0);

    result = 0;
    length = (T_U16)Utl_StrLen(string);

    for( i = 0; i < length; i ++)
        result += (*(string + i) - 0x30) * Power[length - i];
    return result;

}


T_S32 AToI16(T_pCSTR string)
{
    T_S32   result = 0;
    T_U16   i;
    T_U16   length;
    T_S8    chartemp;
    T_S32   Power[8] = {1,16,256,4096,65536,1048576,16777216,268435456};

    AK_ASSERT_PTR(string, "AToI16()", 0);

    result = 0;
    length = (T_U16)Utl_StrLen(string);
    if(length > 8) // error format
        return 0;

    for( i = 0; i < length; i ++)
//      result += (*(string + i) - 0x30) * Power[length - i];
    {
        chartemp = *(string + i);
        if(chartemp >= '0' && chartemp <= '9')
            chartemp -= 0x30;
        else if(chartemp >= 'a' && chartemp <= 'f')
            chartemp -= 87;
        else if(chartemp >= 'A' && chartemp <= 'F')
            chartemp -= 55;
        else
            return 0;
        result += chartemp * Power[length - i - 1];

    }
    return result;

}

/**
 * @brief change the string to 2 hexdecimal
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_pSTR result1point: the string to be changed
 * @return T_S16
 * @retval the 2 byte hexdecimal
 */
T_S16 Utl_Unchange2(T_pCSTR result1point)
{
    T_S16 i;
    T_S16 result;
    T_S8    temp[2];                /* used for change  4-byte hex into T_S16*/

    AK_ASSERT_PTR(result1point, "Utl_Unchange2()", 0);

    Utl_MemCpy(temp, result1point, 2);  // total text length

    for(i = 0; i < 2; i ++)
    {
        if(*(temp + i) > '9')
            *(temp + i) -= 55;
        else
            *(temp + i) -= 48;
    }

    result = *(temp) *16  + *(temp + 1) ;
    return result;
}


/**
 * @brief change the string to 4 hexdecimal
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_pSTR result1point: the string to be changed
 * @return T_S16
 * @retval the 4 byte hexdecimal
 */
T_S16 Utl_Unchange4(T_pCSTR result1point)
{
    T_S16   i;
    T_S16   result;
    T_S8    temp[4];                /* used for change  4-byte hex into T_S16*/

    AK_ASSERT_PTR(result1point, "Utl_Unchange4()", 0);

    Utl_MemCpy(temp, result1point, 4);  // total text length

    for(i = 0; i < 4; i ++)
    {
        if(*(temp + i) > '9')
            *(temp + i) -= 55;
        else
            *(temp + i) -= 48;
    }

    result = *(temp) *16 * 16 * 16 + *(temp + 1) * 16 * 16 + *(temp + 2) * 16 + *(temp + 3);
    return result;
}

/**
 * @brief change the 2 byte hexdecimal to a string
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_S16 length: the data to be changed
 * @param T_pSTR resultpoint: the resuklt buffer
 * @return T_BOOL
 * @retval
 */
T_BOOL Utl_Change2(T_S16 length,T_pSTR resultpoint)
{
    T_S16 i;

    AK_ASSERT_PTR(resultpoint, "Utl_Change2()", AK_FALSE);

    *(resultpoint) = (T_S8)(length >> 4);
    *(resultpoint + 1)  = (T_S8)(length - *(resultpoint )  * 16 );

    for(i = 0; i < 2; i ++)
    {
        if(*(resultpoint+i)  <=  9)
            *(resultpoint+i)  += 48;
        else
            *(resultpoint+i)  += 55;

    }
    *(resultpoint+2)  ='\0';
    return AK_TRUE;
}


/**
 * @brief: change the 4 byte hexdecimal to a string
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_S16 length: the data to be changed
 * @param T_pSTR resultpoint: the resuklt buffer
 * @return T_BOOL
 * @retval void
 */
T_BOOL Utl_Change4(T_S16 length, T_pSTR resultpoint)
{
    T_S16 i;

    AK_ASSERT_PTR(resultpoint, "Utl_Change4()", AK_FALSE);

    *(resultpoint) = (T_S8)(length >> 12);
    *(resultpoint + 1)  = (T_S8)((length - *(resultpoint )  * 16 * 16 * 16) >> 8);
    *(resultpoint + 2)  = (T_S8)((length - *(resultpoint)  * 16 * 16 * 16 - *(resultpoint+1) * 16 * 16) >> 4);
    *(resultpoint + 3)  = (T_S8)((length - *(resultpoint)  * 16 * 16 * 16 - *(resultpoint+1)  * 16 * 16 - *(resultpoint+2)  * 16));

    for(i = 0; i < 4; i ++)
    {
        if(*(resultpoint+i)  <= 9)
            *(resultpoint+i)  += 48;
        else
            *(resultpoint+i)  += 55;

    }
    *(resultpoint+4)  ='\0';
    return AK_TRUE;
}

/**
 * @brief Convert a Hex string to ASCII string.
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_pSTR strPDU
 * @param T_pSTR String2
 * @return T_BOOL
 * @retval
 */
T_BOOL ConvertIntToString(T_pCSTR strPDU, T_pSTR String2)
{
    T_S16 i,j;
    T_S8 StrTemp[5];

    AK_ASSERT_PTR(strPDU, "ConvertIntToString()", AK_FALSE);
    AK_ASSERT_PTR(String2, "ConvertIntToString()", AK_FALSE);

    for(i = 0; *(strPDU + i ) != '\0'; i ++)
    {
        Utl_StrMid(StrTemp,strPDU, i, i);
        j = ConverHexToi(StrTemp);
        IToA2(j, StrTemp);
        Utl_StrCat(String2,StrTemp);
    }
    return AK_TRUE;
}

/**
 * @brief this is a local function, it just changes character's positon
 *          in the input string. For example, string "abcdef" will be
 *          converted to string "badcfe".
 *
 * @author @b miaobaoli
 *
 * @author
 * @date 2001-04-03
 * @param T_pDATA changestring: this is both the source and the destination pointer.
 * @return T_VOID
 * @retval
 */
T_VOID Utl_ChangPlace(T_pSTR changestring)
{
    T_S16 iLength,i;
    T_S8    c,c1;

    AK_FUNCTION_ENTER("Utl_ChangPlace");
    AK_ASSERT_PTR_VOID(changestring, "Utl_ChangPlace(): changestring");

    iLength = Utl_StrLen((T_pCSTR)changestring);
    i = 0;

    while(i < iLength)
    {
        c = *(changestring + i);
        c1 = *(changestring + i + 1);

        *(changestring + i) = c1;
        *(changestring + i + 1) = c;

        i += 2;
    }
    AK_FUNCTION_LEAVE("Utl_ChangPlace");
}


/**
 * @brief toolkit  layer memory manager, call to framework
 *
 * @author PY.Xue
 * @date   2001-08-1
 * @param T_pSTR strDest the dest address
 * @param  T_pSTR strSour the source address
 * @param  T_U32 iLen the length to be copied from source to dest
 * @return T_BOOL
 * @retval the point to the dest
 */
T_BOOL Utl_MemCpy(T_pVOID strDest, T_pCVOID strSour, T_U32 count)
{
    T_U32   i;
    T_pDATA pDest;
    T_pCDATA    pSour;

    //AK_ASSERT_PTR(strDest, "Utl_MemCpy()", AK_FALSE);
    //AK_ASSERT_PTR(strSour, "Utl_MemCpy()", AK_FALSE);
    #if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(count < MAX_STRING_LEN, "Utl_MemCpy()", AK_FALSE);        /* length can't exceed MAX_STRING_LEN */
    #endif
#ifdef OS_ANYKA
    if (count > 128)
    {
        //DMA_Memcpy(strDest,(T_pVOID *)strSour,count);    //DMA memory copy
        memcpy(strDest,(T_pVOID *)strSour,count);
        return AK_TRUE;
    }
#endif // OS_ANYKA

    pDest = (T_pDATA)strDest;
    pSour = (T_pCDATA)strSour;

    for (i = 0; i < count; i++)
    {
        pDest[i] = pSour[i];
    }
    return AK_TRUE;
}

/**
 * @brief toolkit  layer memory manager, call to framework
 *
 * @author PY.Xue
 * @date   2001-08-1
 * @param T_pSTR strDest the dest address
 * @param  T_S8 data
 * @param  T_U32 iLen the length to be copied from source to dest
 * @return T_BOOL
 * @retval the point to the dest
 */
T_BOOL Utl_MemSet(T_pVOID strDest, T_U8 chr, T_U32 count)
{
    T_U32   i;
    T_U8    *pDest;

    AK_ASSERT_PTR(strDest, "Utl_MemSet()", AK_FALSE);
    AK_ASSERT_VAL(count < MAX_STRING_LEN, "Utl_MemSet()", AK_FALSE);        /* length can't exceed MAX_STRING_LEN */

    pDest = (T_pDATA)strDest;
    for (i = 0; i < count; i++)
    {
        pDest[i] = chr;
    }
    return AK_TRUE;
}


T_S8 Utl_MemCmp(T_pCVOID data1, T_pCVOID data2, T_U32 count)
{
    T_U32   i;
    T_pCDATA    p, q;

    AK_ASSERT_PTR(data1, "Utl_MemCmp()", 0);
    AK_ASSERT_PTR(data2, "Utl_MemCmp()", 0);
    AK_ASSERT_VAL(count < MAX_STRING_LEN, "Utl_MemCmp()", 0);       /* length can't exceed MAX_STRING_LEN */

    p = (T_pCDATA)data1;
    q = (T_pCDATA)data2;

    for( i=0; i<count; i++ )
    {
        if( *p < *q )
        {
            return -1;
        }
        else if( *p > *q )
        {
            return 1;
        }

        p++;
        q++;
    }
    return 0;
}

/**
 * @brief get length of a string
 *
 * @author @b ZouMai
 * assert the length of the string
 *
 * @author PY.Xue
 * @date   2001-08-1
 * @param T_pSTR str strMain--head address of string
 * @return T_S16
 * @retval the length
 */
__ram T_U16 Utl_StrLen(T_pCSTR strMain)
{
    T_U16   mainStrLen = 0;
    //T_pCSTR   pMain;

    //AK_ASSERT_PTR(strMain, "Utl_StrLen(): strMain", 0);
    if (strMain == 0)
        return 0;

    while(strMain[mainStrLen]!=0)
        mainStrLen++;
    //pMain = strMain;

    //while (*pMain != 0)
    //{
    //  mainStrLen++;
    //  pMain++;
    //  //AK_ASSERT_VAL(mainStrLen < MAX_STRING_LEN, "Utl_StrLen()", 0);        /* length can't exceed MAX_STRING_LEN */
    //}

    return mainStrLen;
}

/**
 * @brief search for strSub in strMain from the offset.
 * This fucntion should be called if the string is very long.
 *
 * @author ZouMai
 * @date  2001-08-1
 * @param T_pSTR strMain head address of main string to search in
 * @param  T_pSTR strSub head address of sub string to search for
 * @param  T_S16 offset begin address offset
 * @param  T_U16 length: string length
 * @return T_S16
 * @retval -1: not find; else offset in main string
 */
__ram T_S16 Utl_StrFndL(T_pCSTR strMain, T_pCSTR strSub, T_S16 offset, T_U16 length)
{
    T_U16   curLoc = offset;
    T_U16   i;
    T_U16   subStrLen;
    T_pCSTR pMain;

    AK_ASSERT_PTR(strMain, "Utl_StrFndL(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_StrFndL(): strSub", -1);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrFndL()", -1);        /* length can't exceed MAX_STRING_LEN */
    AK_ASSERT_VAL(length < MAX_STRING_LEN, "Utl_StrFndL()", -1);        /* length can't exceed MAX_STRING_LEN */
#endif

    if (offset < 0)
        return -1;

    subStrLen = Utl_StrLen(strSub);
    if (offset + subStrLen > length)
        return -1;

    pMain = strMain + offset;

    while (*(pMain + subStrLen - 1) != 0)
    {
        //AK_ASSERT_VAL(curLoc < MAX_STRING_LEN, "Utl_StrFndL()", -1);      /* length can't exceed MAX_STRING_LEN */

        for (i = 0; i < subStrLen; i++)
        {
            if (*(pMain + i) != *(strSub + i))
                break;
        }
        if (i == subStrLen)
        {
            return curLoc;
        }

        curLoc++;
        pMain++;
    }

    return -1;
}

/**
 * @brief copy sub string from specify start and end place
 *
 * @author Junhua Zhao
 * @date  2005-05-10
 * @param T_pSTR strDest: head address of destination string
 * @param  T_pSTR strSour: head address of source string
 * @param  T_S16 offset: begin address offset
 * @param  T_U16 end: end address
 * @param  T_U16 length: string length
 * @return T_S8 head address of destination string
 */
__ram T_pSTR Utl_StrMidL(T_pSTR strDest, T_pCSTR strSour, T_S16 offset, T_S16 end, T_U16 strlength)
{
    T_U16   iLength;
    T_U16   i;

    AK_ASSERT_PTR(strDest, "Utl_StrMid()", AK_NULL);
    AK_ASSERT_PTR(strSour, "Utl_StrMid()", AK_NULL);
#if MAX_STRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_STRING_LEN, "Utl_StrMid()", AK_NULL);        /* length can't exceed MAX_STRING_LEN */
    AK_ASSERT_VAL(end < MAX_STRING_LEN, "Utl_StrMid()", AK_NULL);       /* length can't exceed MAX_STRING_LEN */
#endif

    strDest[0] = '\0';
    if (offset < 0 || end < 0)
        return strDest;

    if (offset > end)
        return strDest;

    iLength = strlength;
    if (iLength == 0)
        return strDest;

    if (offset > iLength - 1)
        offset = iLength - 1;
    if (end > iLength - 1)
        end = iLength - 1;

    for (i = offset; i <= end; i++){
        if ((i < iLength-1) &&
            Utl_IsDWordChar(strSour + i, gs.Lang)){ //It isn't last char and it's DWord character
            if (i==end)
                break;
            strDest[i - offset] = *(strSour + i);
            i++;
        }
        strDest[i - offset] = *(strSour + i);
    }
    strDest[i - offset] = '\0';
    return strDest;
}

/**
 * @brief Compare two string ingore case
 *
 * @author MiaoBaoLi
 * @date  2001-08-1
 * @param T_pSTR str1 head address of string1
 * @param  T_pSTR str2 head address of string1
 * @return T_S16
 * @retval compare value
 */
__ram T_S8 Utl_StrCmpC(T_pCSTR str1, T_pCSTR str2)
{
    T_U8    c1, c2;
    T_S16   i=0;
    T_pCDATA    pStr1 = (T_pDATA)str1;
    T_pCDATA    pStr2 = (T_pDATA)str2;

    for( i=0; ; i++ )
    {
        c1 = pStr1[i];
        if( c1 >= 'A' && c1 <= 'Z' )
        {
            c1 += 0x20;
        }

        c2 = pStr2[i];
        if( c2 >= 'A' && c2 <= 'Z' )
        {
            c2 += 0x20;
        }

        if( c1 > c2 )
        {
            return 1;
        }
        else if( c1 < c2 )
        {
            return -1;
        }
        else
        {
            if( c1 == 0 )
            {
                return 0;
            }
        }
    }
}

/**
 * @brief copy strSour to StrDest
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param T_pSTR strDest head address of source string
 * @param  T_pSTR strSour head address of destination string
 * @return T_S8
 * @retval head address of destination string
 */
__ram T_pSTR Utl_StrCpy(T_pSTR strDest, T_pCSTR strSour)
{
    T_S16   i = 0;
    T_pSTR      d;
    T_pCSTR s;

    //AK_ASSERT_PTR(strDest, "Utl_StrCpy(): strDest", AK_NULL);
    //AK_ASSERT_PTR(strSour, "Utl_StrCpy(): strSour", AK_NULL);

    d = strDest;
    s = strSour;

    while (*s)
    {
        *d++ = *s++;
        i++;
        //AK_ASSERT_VAL(i < MAX_STRING_LEN, "Utl_StrCpy(): i", AK_NULL);        /* length can't exceed MAX_STRING_LEN */
    }
    *d = 0;

    return strDest;
}

/**
 * @brief Copy 'length' bytes from strSour to strDest.
 *
 * @author PY.Xue
 * @date  2001-08-1
 * @param  T_pSTR strDest head address of source string
 * @param  T_pSTR strSour head address of destination string
 * @param  T_S16 length  number of bytes.
 * @return T_S8
 * @retval head address of destination string
 */
__ram T_pSTR Utl_StrCpyN(T_pSTR strDest, T_pCSTR strSour, T_U32 length)
{
    T_U32   i = 0;
    T_pSTR      d;
    T_pCSTR s;

    d = strDest;
    s = strSour;

    while ((*s) && (i < length))
    {
        *d++ = *s++;
        i++;
    }
    *d = 0;

    return strDest;
}

T_VOID SplitFilePath(T_STR_FILE file_path, T_pSTR path, T_pSTR name)
{
    T_S32 i, len;

    if ((file_path == AK_NULL) || (strlen(file_path) == 0))
    {
        path[0] = '\0';
        name[0] = '\0';
        return;
    }

    len = strlen(file_path);
    for (i=len-1; i>=0; i--)
    {
        if ((file_path[i] == '/') || (file_path[i] == '\\'))
            break;
    }

    strncpy(path, file_path, i+1);
    path[i+1] = '\0';
    strcpy(name, &file_path[i+1]);
    name[len-(i+1)] = '\0';
}


T_VOID SplitFileName(T_STR_FILE file_name, T_pSTR name, T_pSTR ext)
{
    T_S32 i, j, len;
    T_S8 ch;
    T_STR_FILE tmp_ext = {0};

    if ((file_name == AK_NULL) || (strlen(file_name) == 0))
    {
        name[0] = '\0';
        ext[0] = '\0';
        return;
    }

    len = strlen(file_name);
    for (i=len-1, j=0; i>=0; i--)
    {
        ch = file_name[i];
        if (ch == '.')
            break;
        else
            tmp_ext[j++] = ch;
    }

    if (i > 0)
    {
        strncpy(name, file_name, i);
        name[i] = '\0';
    }
    else
    {
        strcpy(name, file_name);    //don't exist dot
    }

    for (i=0; i<j; i++)
        ext[j-1-i] = tmp_ext[i];
    ext[j] = '\0';
}


T_FILE_TYPE Utl_GetFileType(T_pCWSTR file_path)
{
    T_USTR_FILE name, ext;
    T_STR_FILE  tmpstr;
    T_FILE_TYPE i;

    T_U8 *FileExtStr[FILE_TYPE_NUM] = {
        "bmp", "jpg", "jpeg", "jpe", "png", "gif", "mj", "mjpg", "mjpeg", "avi", "akv",
        "3gp", "mp4", "flv", /*"rmvb", "rm", */"mp1", "mp2", "mp3", "mpg",// "mkv",
        "mid", "midi", "adpcm", "wav", "wave",
        "amr", "asf", "wma", "mpeg", "aac", "ac3", "adif", "adts", "m4a", "flac", "ogg", "oga", 
        "ape", "lrc", "txt", "doc", "pdf", "xls", "map", "nes", "smc", "gba","smd", "alt", 
        "vlt", "sav", "mfs", "ttf", "ttc", "otf", "swf", "???"   //unknown
    };

    if (file_path == AK_NULL || Utl_UStrLen(file_path) == 0)
    {    
        return FILE_TYPE_NONE;
    }

    Utl_USplitFileName(file_path, name, ext);
    Utl_UStrLower(ext);
    Eng_StrUcs2Mbcs(ext, tmpstr);

    for (i = 0; i < FILE_TYPE_NUM; i++)
    {
        if (!Utl_StrCmp(tmpstr, FileExtStr[i]))
        {
            return i;
        }
    }
     
    return FILE_TYPE_NONE;
}

T_BOOL Utl_IsLegalFname(T_U16 *Fname)  
{
    T_U32 strLen = 0;
    T_U32 i;
    T_U16 chara = UNICODE_SPACE;

    strLen = Utl_UStrLen(Fname);

    for (i=0; i < strLen; i++)
    {
        chara = *(Fname++);

        if (chara == UNICODE_QUESTION)
        {
            return AK_FALSE;
        }
    }

    return AK_TRUE;
}

/*************************************************************************
 
    FUNCTION: UTL_isBlankStr
    AUTHOR: he_ying
    DATE: 2008-1-8
    NOTE: class blank is not just include space, \t is blank too! refer to ISO C
 
*************************************************************************/
T_BOOL UTL_isBlankStr(T_pCWSTR wcs)
{
    AK_ASSERT_PTR(wcs,"UTL_isBlankStr(NULL)",AK_FALSE);
    while (*wcs != '\0')
    {
        if (isspace((T_S16)*wcs) == AK_FALSE)
        {
            return AK_FALSE;
        }
        wcs++;
    }

    return AK_TRUE;
}



