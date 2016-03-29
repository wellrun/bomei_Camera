/**
 * @file
 * @brief ANYKA software
 * this header file provide application layer string function 
 *
 * @author Junhua Zhao
 * @date  2005-08-10
 * @version 1.0 
 */

#include <string.h>
#include "Fwl_osMalloc.h"
#include "Eng_DataConvert.h"
#include "Gbl_Global.h"
#include "Eng_String_UC.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "eng_debug.h"


extern T_U16 GetCharFontWidth(const T_U16 chr, T_FONT font);

/**
 * @brief search for strSub in strMain from the index element
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * strMain head address of main string to search in
 * @param  T_U16 * strSub head address of sub string to search for
 * @param  T_S16 offset begin address offset
 * @return T_S16
 * @retval -1: not find; else offset in main string
 */
T_S16 Utl_UStrFnd(T_U16* strMain, const T_U16* strSub, T_S16 offset)
{
    AK_ASSERT_PTR(strMain, "Utl_UStrFnd(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_UStrFnd(): strSub", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrFnd()", -1);       /* length can't exceed MAX_USTRING_LEN */
#endif

    return Utl_UStrFndL(strMain, strSub, offset, (T_U16)Utl_UStrLen(strMain));
}

/**
 * @brief search for strSub in strMain from the (strMain+offset) to (strMain+offset+count)
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * strMain head address of main string to search in
 * @param  T_U16 * strSub head address of sub string to search for
 * @param  T_S16 offset begin address offset
 * @param  T_S16 count search char quantity
 * @return T_S16
 * @retval -1: not find; else offset in main string
 */
T_S16 Utl_UStrFndN(T_U16* strMain, const T_U16* strSub, T_S16 offset, T_U16 count)
{
    T_U16   curLoc = offset;
    T_U16   i;
    T_U16   subStrLen;
    T_U16   mainStrLen;
    T_U16*  pMain;

    AK_ASSERT_PTR(strMain, "Utl_UStrFndN(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_UStrFndN(): strSub", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrFndN()", -1);      /* length can't exceed MAX_USTRING_LEN */
    AK_ASSERT_VAL(count < MAX_USTRING_LEN, "Utl_UStrFndN()", -1);       /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return -1;

    subStrLen = (T_U16)Utl_UStrLen(strSub);
    mainStrLen = (T_U16)Utl_UStrLen(strMain);
    if (offset + subStrLen > mainStrLen)
//  if (subStrLen > (T_U16)Utl_UStrLen(strMain + offset))   /* if offset > length of strMain, error occur */
        return -1;
    if (subStrLen > count)
        return -1;

    pMain = strMain + offset;

    while (*(pMain + subStrLen - 1) != 0)
    {
        //AK_ASSERT_VAL(curLoc < MAX_USTRING_LEN, "Utl_UStrFndN()", -1);        /* length can't exceed MAX_USTRING_LEN */

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

T_S16 Utl_UStrFndNL(T_U16* strMain, const T_U16* strSub, T_S16 offset, T_U16 count, T_U16 length)
{
    T_U16   curLoc = offset;
    T_U16   i;
    T_U16   subStrLen;
    T_U16   mainStrLen;
    T_U16*  pMain;

    AK_ASSERT_PTR(strMain, "Utl_UStrFndN(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_UStrFndN(): strSub", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrFndN()", -1);      /* length can't exceed MAX_USTRING_LEN */
    AK_ASSERT_VAL(count < MAX_USTRING_LEN, "Utl_UStrFndN()", -1);       /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return -1;

    subStrLen = (T_U16)Utl_UStrLen(strSub);
    //mainStrLen = (T_U16)Utl_UStrLen(strMain);
    mainStrLen = length;
    if (offset + subStrLen > mainStrLen)
        return -1;
    if (subStrLen > count)
        return -1;

    pMain = strMain + offset;

    while (*(pMain + subStrLen - 1) != 0)
    {
        //AK_ASSERT_VAL(curLoc < MAX_USTRING_LEN, "Utl_UStrFndN()", -1);        /* length can't exceed MAX_USTRING_LEN */

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
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * str head address of main string to search in
 * @param  T_U16 chr the char search for
 * @param  T_S16 index begin address offset
 * @return T_S16
 * @retval -1: not find; else offset in main string
 */
T_S16 Utl_UStrFndChr(T_U16* strMain, T_U16 chr, T_S16 offset)
{
    T_U16   curLoc = offset;
    T_U16*  pMain;

    AK_ASSERT_PTR(strMain, "Utl_UStrFndChr(): strMain", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrFndChr()", -1);        /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return -1;

    if (offset >= (T_U16)Utl_UStrLen(strMain))
        return -1;

    pMain = strMain + offset;

    while (*pMain != 0)
    {
        //AK_ASSERT_VAL(curLoc < MAX_USTRING_LEN, "Utl_UStrFndChr(): curLoc", -1);      /* length can't exceed MAX_USTRING_LEN */

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
 * @brief : look for subStr in the main string from the reverse direction.
 * 
 * @author RongDian 
 * @date  2005-08-10 
 * @param T_U16 * strMain
 * @param  T_U16 * strSub
 * @param  T_S16 offset
 * @return T_S16
 * @retval -1: not found; else: the location of the main string that found the substring.
 */
T_S16   Utl_UStrRevFnd(const T_U16* strMain, const T_U16* strSub, T_S16 offset)
{
    T_U16 i, j, len1, len2, flag;

    AK_ASSERT_PTR(strMain, "Utl_UStrRevFnd(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_UStrRevFnd(): strSub", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrRevFnd()", -1);        /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return -1;

    len1 = (T_U16)Utl_UStrLen(strMain);
    len2 = (T_U16)Utl_UStrLen(strSub);
    
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
 * @date  2005-08-10 
 * @param T_U16 * strMain
 * @param  T_U16 chr
 * @param  T_S16 offset
 * @return T_S16
 * @retval -1: not found. else: the location of the main string that found the 'ch'
 */
T_S16 Utl_UStrRevFndChr(const T_U16* strMain, const T_U16 chr, T_S16 offset)
{
    T_S16 i;

    AK_ASSERT_PTR(strMain, "Utl_UStrRevFndChr(): strMain", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrRevFndChr()", -1);     /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return -1;

    if(offset >= (T_U16)Utl_UStrLen(strMain)) 
        offset = (T_U16)Utl_UStrLen(strMain) -1;

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
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * str1 head address of string1
 * @param  T_U16 * str2 head address of string1
 * @return T_S16
 * @retval compare value
 */
T_S8 Utl_UStrCmp(const T_U16* str1, const T_U16* str2)
{
    T_S16   len = 0;
    const T_U16*  pStr1 = str1;
    const T_U16*  pStr2 = str2;

    AK_ASSERT_PTR(str1, "Utl_UStrCmp() str1", 0);
    AK_ASSERT_PTR(str2, "Utl_UStrCmp() str2", 0);

    while (((*(pStr1)) != 0) || ((*(pStr2)) != 0))
    {
        if ((*(pStr1)) > (*(pStr2)))
            return 1;

        if ((*(pStr1)) < (*(pStr2)))
            return -1;

        pStr1++;
        pStr2++;
        len++;
        //AK_ASSERT_VAL(len < MAX_USTRING_LEN, "Utl_UStrCmp()", -1);        /* length can't exceed MAX_USTRING_LEN */
    }

    return 0;
}

T_S8 Utl_UStrRevCmp(T_U16* str1, T_U16* str2)
{
    T_S16   len = 0, i, j;
    T_U16*  pStr1 = str1;
    T_U16*  pStr2 = str2;

    AK_ASSERT_PTR(str1, "Utl_UStrCmp()", 0);
    AK_ASSERT_PTR(str2, "Utl_UStrCmp()", 0);

    i = (T_U16)Utl_UStrLen( str1 );
    j = (T_U16)Utl_UStrLen( str2 );

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
        //AK_ASSERT_VAL(len < MAX_USTRING_LEN, "Utl_UStrCmp()", -1);        /* length can't exceed MAX_USTRING_LEN */
    }

    return 0;
}

T_S8 Utl_UStrRevNCmp(T_U16* str1, T_U16* str2, T_U8 length)
{
    T_S16   len = 0, i, j;

    AK_ASSERT_PTR(str1, "Utl_UStrCmp()", 0);
    AK_ASSERT_PTR(str2, "Utl_UStrCmp()", 0);

    i = (T_U16)Utl_UStrLen( str1 );
    j = (T_U16)Utl_UStrLen( str2 );

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
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * str1 head address of string1
 * @param  T_U16 * str2 head address of string1
 * @return T_S16
 * @retval compare value
 */
T_S8 Utl_UStrCmpN(T_U16* str1, T_U16* str2, T_U16 length)
{
    T_U16   i;
    T_U16*  pStr1 = str1;
    T_U16*  pStr2 = str2;

    AK_ASSERT_PTR(str1, "Utl_UStrCmpN()", 0);
    AK_ASSERT_PTR(str2, "Utl_UStrCmpN()", 0);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(length < MAX_USTRING_LEN, "Utl_UStrCmpN()", 0);       /* length can't exceed MAX_USTRING_LEN */
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
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_pDATA s one string pointer
 * @param T_pDATA d another string pointer
 * @param T_U16 length
 * @return T_S16
 * @retval 0:   s substring identical to d substring
 *         1:   s substring greater than d substring
 *         -1: s substring less than d substring
 */
T_S8 Utl_UStrCmpNC(T_U16* str1, T_U16* str2, T_U16 length)
{
    T_U16   c1,c2;
    T_U16  i;
    T_U16*  pStr1 = str1;
    T_U16*  pStr2 = str2;
    const T_U16 char_A = 'A';
    const T_U16 char_Z = 'Z';

    AK_ASSERT_PTR(str1, "Utl_UStrCmpNC()", 0);
    AK_ASSERT_PTR(str2, "Utl_UStrCmpNC()", 0);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(length < MAX_USTRING_LEN, "Utl_UStrCmpNC()", 0);      /* length can't exceed MAX_USTRING_LEN */
#endif

    for( i=0; i<length; i++, pStr1++, pStr2++ )
    {
        c1 = *pStr1;
        if( c1 >= char_A && c1 <= char_Z )
            c1 += 0x20;

        c2 = *pStr2;
        if( c2 >= char_A && c2 <= char_Z )
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
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * str
 * @param  T_U16 chr
 * @param  T_S16 count
 * @return T_S8
 * @retval 
 */
T_U16 * Utl_UStrCpyChr(T_U16 * strDest, T_U16 chr, T_U16 count)
{
    T_U16   i;

    AK_ASSERT_PTR(strDest, "Utl_UStrCpyChr()", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(count < MAX_USTRING_LEN, "Utl_UStrCpyChr()", AK_NULL);        /* length can't exceed MAX_USTRING_LEN */
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
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * strDest head address of destination char
 * @param  T_U16 * strSub head address of sub string
 * @return T_S8
 * @retval head address of destination string
 */
T_U16 * Utl_UStrCat(T_U16 * strDest, const T_U16* strSub)
{
    T_S32   i = 0;
    T_S32   len;

    AK_ASSERT_PTR(strDest, "Utl_UStrCat()", AK_NULL);
    AK_ASSERT_PTR(strSub, "Utl_UStrCat()", AK_NULL);

    len = (T_U16)Utl_UStrLen(strDest);

    while (*(strSub + i++) != 0)
    {
        //AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrCat()", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */

        strDest[len + i - 1] = strSub[i - 1];
    }
    strDest[len + i - 1] = 0;
  
    return strDest;
}


/**
 * @brief connect one or more char to strDest
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * strDest head address of destination string
 * @param  T_U16 chr source char
 * @param  T_U16 count
 * @return T_S8
 * @retval head address of destination string
 */
T_U16 * Utl_UStrCatChr(T_U16 * strDest, T_U16 chr, T_S16 count)
{
    T_U16   i;
    T_U16   len;
   
    AK_ASSERT_PTR(strDest, "Utl_UStrCatChr()", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(count < MAX_USTRING_LEN, "Utl_UStrCatChr()", AK_NULL);        /* length can't exceed MAX_USTRING_LEN */
#endif

    len = (T_U16)Utl_UStrLen(strDest);
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
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * strDest destination string
 * @param  T_U16 * strSub sub-string
 * @param  T_S16 offset the location of strDest where the strSub will be added in
 * @return T_S8
 * @retval head of destination string
 */
T_U16 * Utl_UStrIns(T_U16 * strDest, const T_U16* strSub, T_S16 offset)
{
    T_U16   lenDest, lenSub;
    T_S16   i;
    T_S16   curOffset = offset;

    AK_ASSERT_PTR(strDest, "Utl_UStrIns()", AK_NULL);
    AK_ASSERT_PTR(strSub, "Utl_UStrIns()", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrIns()", AK_NULL);      /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return strDest;

    lenDest = (T_U16)Utl_UStrLen(strDest);
    lenSub  = (T_U16)Utl_UStrLen(strSub);

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
}// end Utl_UStrIns(T_U16 * strDest, T_U16 * strSub, T_S16 offset)

/**
 * @brief Insert one character into a string(strDest).
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * strDest destination string
 * @param  T_U16 chr sub-character
 * @param  T_S16 offset the location of strDest where the strSub will be added in
 * @return T_S8
 * @retval head of destination string
 */
T_U16 * Utl_UStrInsChr(T_U16 * strDest, T_U16 chr, T_S16 offset)
{
    T_U16   sTemp[2];

    AK_ASSERT_PTR(strDest, "Utl_UStrInsChr()", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrInsChr()", AK_NULL);       /* length can't exceed MAX_USTRING_LEN */
#endif

    sTemp[0] = chr;
    sTemp[1] = '\0';

    return Utl_UStrIns(strDest, sTemp, offset);
}// end Utl_UStrIns(T_U16 * strDest, T_U16 * strSub, T_S16 offset)

/**
 * @brief Replace partial characters of one string(strDest) with another string(strSub).
 * 
 * @author Junhua Zhao
 * @date  2005-08-10 
 * @param T_U16 * strDest destination string
 * @param  T_U16 * strSub sub-string
 * @param  T_S16 offset the location of strDest where the strSub will replace it
 * @return T_S8
 * @retval the head of destination string.
 */
T_U16 * Utl_UStrRep(T_U16 * strDest, const T_U16* strSub, T_S16 offset)
{
    T_U16   lenDest, lenSub;
    T_U16   i;

    AK_ASSERT_PTR(strDest, "Utl_UStrRep()", AK_NULL);
    AK_ASSERT_PTR(strSub, "Utl_UStrRep()", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrRep()", AK_NULL);      /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return strDest;

    lenDest = (T_U16)Utl_UStrLen(strDest);
    lenSub  = (T_U16)Utl_UStrLen(strSub);

    if (offset < 0)
        offset = 0;
    for (i = offset; i < offset + lenSub; i++)
        strDest[i] = strSub[i - offset];
    if (i > lenDest)
        strDest[i] = 0;
  
    return strDest;
}// end Utl_UStrRep(T_U16 * strDest, T_U16 * strSub, T_S16 offset)

/**
 * @brief Empty a string
 * 
 * @author Junhua Zhao
 * @date  2001-11-09 
 * @param T_U16 * strDest destination string
 * @return T_U16 * 
 * @retval the header of destination string.
 */
T_U16 * Utl_UStrEmpty(T_U16 * strDest)
{
    AK_ASSERT_PTR(strDest, "Utl_UStrEmpty()", 0);

    strDest[0] = '\0';
    return strDest;
}

/**
 * @brief Delete some characters from string(strDest).
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * strDest destination string
 * @param  T_S16 offset the location where begin to delete 
 * @param  T_S16 iLen the length of deleted characters
 * @return T_S8
 * @retval the header of destination string.
 */
T_U16 * Utl_UStrDel(T_U16 * strDest, T_S16 offset, T_U16 count)
{
    T_U16   lenDest;
    T_U16   i;

    AK_ASSERT_PTR(strDest, "Utl_UStrDel()", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrDel()", AK_NULL);      /* length can't exceed MAX_USTRING_LEN */
    AK_ASSERT_VAL(count < MAX_USTRING_LEN, "Utl_UStrDel()", AK_NULL);       /* length can't exceed MAX_USTRING_LEN */
#endif
    
    if (offset < 0)
        return strDest;

    lenDest = (T_U16)Utl_UStrLen(strDest);
    if (offset > lenDest - 1)
        offset = lenDest - 1;
    if (count > lenDest - offset)
        count = lenDest - offset;
    for (i = offset; i < lenDest - count; i++)
    {
        //AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrDel()", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */

        strDest[i] = strDest[i + count];
    }
   strDest[i] = '\0';

    return strDest;
}// end Utl_UStrDel(T_U16 * strDest, T_S16 offset, T_S16 iLen)


/**
 * @brief Remove 'ch' from the string.
 * 
 * @author @b 
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * strDest: header of the main string.
 * @param  T_U16 chr: 
 * @param  T_S16 offset
 * @param  T_S16 count: -1: delete all chr
 * @return T_U16 * 
 * @retval 
 */
T_U16 * Utl_UStrDelChr(T_U16 * strDest, T_U16 chr, T_S16 offset, T_S16 count)
{
    T_U16 j, i = offset;
    T_U16 len = (T_U16)Utl_UStrLen(strDest);
    T_S16 num = 0;

    AK_ASSERT_PTR(strDest, "Utl_UStrDelChr(): strDest", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrDelChr(): offset", AK_NULL);       /* length can't exceed MAX_USTRING_LEN */
    AK_ASSERT_VAL(count < MAX_USTRING_LEN, "Utl_UStrDelChr(): count1", AK_NULL);        /* length can't exceed MAX_USTRING_LEN */
    AK_ASSERT_VAL(count > (-1)*MAX_USTRING_LEN, "Utl_UStrDelChr(): count2", AK_NULL);       /* length can't exceed MAX_USTRING_LEN */
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
            #if MAX_USTRING_LEN < 0xFFFF
            AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrDelChr(): i", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */
            #endif
        }
    }
    return strDest;
}

/**
 * @brief cur strSour from offset to end and copy to strDest
 * 
 * @author Junhua Zhao
 * @date  2005-08-10 
 * @param T_U16 * strDest: head address of dest string
 * @param  T_U16 * strSour: head address of source string
 * @param  T_S16 offset: begin index
 * @param  T_S16 end:  end index
 * @return T_S8
 * @retval void
 */
T_U16 * Utl_UStrMid(T_U16 * strDest, const T_U16* strSour, T_S16 offset, T_S16 end)
{
    T_U16   iLength;

    iLength = (T_U16)Utl_UStrLen( strSour );

    return Utl_UStrMidL( strDest, strSour, offset, end, iLength );
}

/**
 * @brief : get the last character of the string.
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * str
 * @return T_S8
 * @retval Result string pointer
 */
T_U16 * Utl_UStrRight(T_U16 * strDest, T_U16* strMain, T_U16 count)
{
    T_U16       length;

    AK_ASSERT_PTR(strDest, "Utl_UStrRight()", AK_NULL);
    AK_ASSERT_PTR(strMain, "Utl_UStrRight()", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(count < MAX_USTRING_LEN, "Utl_UStrRight()", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */
#endif

    length = (T_U16)Utl_UStrLen(strMain);

    if (count > length)
        count = length;

    Utl_UStrCpy(strDest, strMain + length - count);
    return strDest;
}

/**
 * @brief : get the last character of the string.
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * str
 * @return T_S8
 * @retval Result string pointer
 */
T_U16 Utl_UStrRightChr(const T_U16* strMain)
{
    AK_ASSERT_PTR(strMain, "Utl_UStrRightChr()", 0);

    if (strMain[0] == '\0')     /* length == 0 */
        return 0;

    return strMain[(T_U16)Utl_UStrLen(strMain)-1]; /* length > 0 */
}

/**
 * @brief : convert a string to up case string.
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * str
 * @return T_S8
 * @retval Result string pointer
 */
T_U16 * Utl_UStrUpper(T_U16 * strMain)
{
    T_U16*  p;
    T_S16   i = 0;

    AK_ASSERT_PTR(strMain, "Utl_UStrUpper()", AK_NULL);

    p = strMain;
    while (*p)
    {
        if (0x61 <= *p && *p <= 0x7a)
            *p -= 0x20;
        p++;
        i++;
        #if MAX_USTRING_LEN < 0xFFFF
        AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrUpper()", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */
        #endif
    }
    return strMain;
}

/**
 * @brief Conver string to lower case string.
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * str
 * @return T_S8
 * @retval result string pointer.
 */
T_U16 * Utl_UStrLower(T_U16 * strMain)
{
    T_U16*  p = AK_NULL;
    T_S16   i = 0;

    AK_ASSERT_PTR(strMain, "Utl_UStrLower()", AK_NULL);

    p = strMain;
    while (*p)
    {
        if (0x41 <= *p && *p <= 0x5a)
            *p += 0x20;
        p++;
        i++;
        #if MAX_USTRING_LEN < 0xFFFF
        AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrLower()", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */
        #endif
    }
    return strMain;
}

/**
 * @brief remove the start blank character.
 * 
 * @author Junhua Zhao
 * @date  2005-08-10 
 * @param T_U16 * str
 * @return T_S8
 * @retval The result string pointer.
 */
T_U16 * Utl_UStrLTrim(T_U16 * strMain)
{
    T_S16   blank = 0;
    T_U16*  p = AK_NULL;
    T_S16   i = 0;

    AK_ASSERT_PTR(strMain, "Utl_UStrLTrim()", AK_NULL);

    p = strMain;
    while (*(p++) == ' ')
    {
        blank++;
        i++;
        #if MAX_USTRING_LEN < 0xFFFF
        AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrLTrim()", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */
        #endif
    }

    p--;
    i = 0;
    while (*(p++ - 1))
    {
        *(p - blank - 1) = *(p - 1);
        i++;
        #if MAX_USTRING_LEN < 0xFFFF
        AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrLTrim()", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */
        #endif
    }

    return strMain;
}

/**
 * @brief remove the end blank character.
 * 
 * @author Junhua Zhao
 * @date  2005-08-10 
 * @param T_U16 * str
 * @return T_S8
 * @retval 
 */
T_U16 * Utl_UStrRTrim(T_U16 * strMain)
{
    T_U16   len;
    T_U16*  p = AK_NULL;

    AK_ASSERT_PTR(strMain, "Utl_UStrRTrim()", AK_NULL);

    len = (T_U16)Utl_UStrLen(strMain);
    p = strMain + len - 1;
    while ((*(p--) == ' ') && (len > 0))
        len--;
    
    strMain[len] = '\0';
    return strMain;
}

/**
 * @brief remove the start and end blank character.
 * 
 * @author Junhua Zhao
 * @date  2005-08-10 
 * @param T_U16 * str
 * @return T_S8
 * @retval 
 */
T_U16 * Utl_UStrTrim(T_U16 * strMain)
{
    AK_ASSERT_PTR(strMain, "Utl_UStrTrim()", AK_NULL);

    return Utl_UStrLTrim(Utl_UStrRTrim(strMain));
}

/**
 * @brief Carve up one-line text to multi-line text by spmark.
 * if "\n" is found in strSour, this function will carve it
 *
 * @author zhaojunhua   
 *    modified by zhengwenbo for multilateral language
 * @date  2007-1-24 
 * @param T_U16 * strSour: source string (unicode one-line text)
 * @param  T_U16 WidthLimit: the max displaying width of text per line
 * @param  T_U16 * spmark
 * @param  T_CARVED_USTR *CarvedStr: destination string (multi-line text)
 * @param  T_U16 WordSize: the max size of the word that will not be cut in the end of a line
 * @return T_BOOL
 * @retval AK_TRUE: success AK_FALSE: fail
 */
T_BOOL  Utl_UStrCarve(const T_U16* strSour, T_U16 WidthLimit, const T_U16* spmark, T_CARVED_USTR *CarvedStr, T_U16 WordSize)
{   
    T_U16   *start=0,*next = 0;
    T_U16 length = 0;
    T_U16 char_no = 0;
    T_U16 ret=0;
    T_BOOL  ret_flag;
	T_U16 dis_width = 0;
	T_U16 uni_width = 0; // unicode char width
	T_U16 i = 0;
	T_U16 back_num = 0;

    
    AK_ASSERT_PTR(strSour, "Utl_UStrCarve()", AK_FALSE);
    AK_ASSERT_PTR(CarvedStr, "Utl_UStrCarve()", AK_FALSE);
    //AK_ASSERT_PTR(spmark, "Utl_UStrCarve()", AK_FALSE);
    AK_ASSERT_VAL(WidthLimit < MAX_USTRING_LEN, "Utl_UStrCarve()", AK_FALSE);
    AK_ASSERT_VAL(WordSize, "Utl_UStrCarve(): WordSize is bad parm", AK_FALSE);

    CarvedStr->LineNum = 0;
    CarvedStr->MaxLen = WidthLimit;
    if (WidthLimit == 0)
        return AK_FALSE;
        
    length = (T_U16)Utl_UStrLen(strSour);
    if (length == 0)
        return AK_FALSE;
    
    start = (T_U16 *)strSour;
    next = (T_U16 *)strSour;
	dis_width = 0;
	
    while(length>0)
    {
        ret_flag = AK_FALSE;

		uni_width = GetCharFontWidth(*next, CURRENT_FONT_SIZE);
		
        //english char and not "\r\n" or "\n"
        if((*next > 0)&&(*next <= 0x00ff)&&(*next!= UNICODE_R)&&(*next != UNICODE_N))
        {
            if(dis_width + uni_width >= WidthLimit)
        	{
        	    /*current word must be the last word in the line if total width is equal to limited width 
        	                    and next char is space or "\r" or "\n"*/
        	    if (((dis_width + uni_width == WidthLimit) &&(length > 1)
                    && (((*next+1) == UNICODE_SPACE) || ((*next+1) == UNICODE_R) 
                    || ((*next+1) == UNICODE_N) || ((*next+1) > 0xff))) 
                    || ((dis_width + uni_width == WidthLimit) &&(length == 1)))
                {
                    ++next;
		            --length;
		            ++char_no;
					dis_width += uni_width;
                    ret = 1;
                }
                else
                {
    				/*avoid to cut a integrity word*/
    				back_num = char_no > WordSize ? WordSize : char_no;
    				for (i=0; i<back_num; i++)
    				{
    				    T_U16 tmp;
                                tmp = *(next-i);
    					if (tmp == UNICODE_SPACE)
    					{
    						next -= i;
    						char_no -= i;
    						length += i;
    						break;
    					}
    				}	

    				if (i == back_num) // not find space backward in limit
    				{
    				    if (dis_width + uni_width == WidthLimit)
                        {            
        					++next;
        		            --length;
        		            ++char_no;
        					dis_width += uni_width;
                        }
    				}

                    ret = 1;
                }
        	}
			else
			{
				++next;
	            --length;
	            ++char_no;
				dis_width += uni_width;
			}
        }
        //dword char
        else if(*next>0x00ff)
        {
            if(dis_width + uni_width < WidthLimit)
            {
                ++next;
                --length;
                char_no++;  
				dis_width += uni_width;
            }
            else // >=
			{
				if(dis_width + uni_width == WidthLimit)
	            {
	                ++next;
	                --length;
	                char_no++;   
					dis_width += uni_width;
	                ret = 1;
	            }
	            else // if dis_width > widthlimit
            	{
	                ret = 1;
				}
        	}
        }
        //"\r"
        else if(*next == UNICODE_R)
        {
            ++next;
            --length;
        }
        //"\n"
        else if (*next == UNICODE_N)
        {
            ++next;
            --length;
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
            while((ret_flag == AK_FALSE) && (*next == UNICODE_R || *next == UNICODE_N))
            {
                if (*next == UNICODE_N)
                { //break when find the first '\n'
                    ++next;
                    --length;
                    break;
                }
                ++next;
                --length;
            }
            //don't repeat add line

			CarvedStr->UnicodeNum[CarvedStr->LineNum] = char_no;
            CarvedStr->String[CarvedStr->LineNum] = (T_U16 *)Fwl_Malloc((char_no << 1) + 2); 
            AK_ASSERT_PTR(CarvedStr->String[CarvedStr->LineNum], "Utl_UStrCarve(): string buff malloc fail", AK_FALSE);
            if(CarvedStr->LineNum >= (MAX_USTR_LINE-1))
            {               
                char_no = 0;
                return AK_TRUE;
            }
            if( (CarvedStr->String[CarvedStr->LineNum]) == AK_NULL )            
            {
                char_no = 0;
                return AK_TRUE;
            }
            if(CarvedStr->String[CarvedStr->LineNum] != AK_NULL)
            {
                Utl_UStrCpyN(CarvedStr->String[CarvedStr->LineNum],start,char_no);
                CarvedStr->LineNum ++;
                start=next;
                char_no = 0;
				dis_width = 0;
            }
        }
    }
    if( char_no > 0)
    {        
    	CarvedStr->UnicodeNum[CarvedStr->LineNum] = char_no;
        CarvedStr->String[CarvedStr->LineNum] = (T_U16 *)Fwl_Malloc((char_no << 1) + 2); 
		AK_ASSERT_PTR(CarvedStr->String[CarvedStr->LineNum], "Utl_UStrCarve():CarvedStr->String[CarvedStr->LineNum] malloc fail", AK_FALSE);
        Utl_UStrCpyN(CarvedStr->String[CarvedStr->LineNum],start,char_no);
        CarvedStr->LineNum ++;
    }    
    return AK_TRUE;
}/* end Utl_UStrCarve(T_U16 * strSour, T_S16 WidthLimit, T_U16 * spmark, T_CARVED_USTR *CarvedStr) */


/**
 * @brief Init structrue T_CARVED_USTR
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_CARVED_USTR *CarvedStr: carved string structure pointer.
 * @return T_BOOL
 * @retval 
 */
T_BOOL Utl_UStrCarveInit(T_CARVED_USTR *CarvedStr)
{
    T_S32 i = 0;
    AK_ASSERT_PTR(CarvedStr, "Utl_UStrCarveInit()", AK_FALSE);

    CarvedStr->LineNum = 0;
    CarvedStr->MaxLen = 0;
    for(i=0; i<MAX_USTR_LINE; i++)
    {
        CarvedStr->String[i] = AK_NULL;
    }
    return AK_TRUE;
}

/**
 * @brief Free a carve
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_CARVED_USTR *CarvedStr: carved string structure pointer.
 * @return T_BOOL
 * @retval 
 */
T_BOOL Utl_UStrCarveFree(T_CARVED_USTR *CarvedStr)
{
    T_U16   i;

    AK_ASSERT_PTR(CarvedStr, "Utl_UStrCarveFree(): CarvedStr", AK_FALSE);
    AK_ASSERT_VAL(CarvedStr->LineNum <= (MAX_USTR_LINE - 1), "Utl_UStrCarveFree(): LineNum", AK_FALSE);

    for (i = 0; i < CarvedStr->LineNum; i++)
    {
        if(AK_NULL != CarvedStr->String[i])
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
 * @author Junhua Zhao
 * @date   2005-08-10
 * @param T_U16 * str strMain--head address of string
 * @return T_S16
 * @retval the length
 */
T_BOOL  Utl_UStrIsEmpty(const T_U16* strMain)
{
    AK_ASSERT_PTR(strMain, "Utl_UStrIsEmpty(): strMain", AK_FALSE);

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
T_BOOL Utl_UIsDWordChar(const T_U16* strMain, T_RES_LANGUAGE Lang)
{
    AK_ASSERT_PTR(strMain, "Utl_UIsDWordChar(): strMain", AK_FALSE);
    
    if (*(strMain) <= 0x80) 
        return AK_FALSE;
    else return AK_TRUE;    //double word char
}

/**
 * @brief Check the string is digital character or not.
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_pDATA string
 * @return T_BOOL
 * @retval AK_TRUE: found Chinese character; AK_FALSE: not found.
 */
T_BOOL Utl_UStrDigital(const T_U16* strMain)
{
    T_U16   i;
    T_U16   len;

    AK_ASSERT_PTR(strMain, "Utl_UStrDigital()", AK_FALSE);

    len = (T_U16)Utl_UStrLen(strMain);

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
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_S8 ch
 * @return T_S16
 * @retval 0: yes. 0: no.
 */
T_S16 Utl_UIsAlpha(T_U16 chr)
{
    if (((chr >= 'a') && (chr <= 'z')) ||
        ((chr >= 'A') && (chr <= 'Z')) )
        return 0;
    else 
        return -1;
}

/**
 * @brief Conver a integer number to string.
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_S16 intNum
 * @param  T_U16 * strDest
 * @param  T_S8 flag
 * @return T_S8
 * @retval result string pointer.
 */
T_U16 * Utl_UItoa(T_S32 intNum, T_U16 * strDest, T_U8 flag)
{
    T_S16       i = 0;
    T_S32       datanew;
    T_S16       index;
    T_U16       strTemp[100];
    T_BOOL      negv = AK_FALSE;

    AK_ASSERT_PTR(strDest, "Utl_UItoa()", AK_NULL);

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
                strDest[0] = intNum + 55;
            else 
                strDest[0] = intNum + 48;
            strDest[1] = '\0';
        }
        else
        {
            while (intNum >= 16)
            {
                datanew = intNum;
                intNum = intNum/16;
                if((datanew - intNum*16) >= 10)
                    strTemp[i] = datanew - intNum * 16 + 55;
                else 
                    strTemp[i] = datanew - intNum * 16 + 48;
                i ++ ;
                if (intNum < 16)
                {
                    if(intNum >= 10)
                        strTemp[i] = intNum + 55;
                    else if(intNum != 0)
                        strTemp[i] = intNum + 48;
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
            strDest[0] = intNum + 48;
            strDest[1] = '\0';
        }
        else
        {
            while(intNum >= 10)
            {
                datanew = intNum;
                intNum = intNum/10;
                strTemp[i] = datanew - intNum * 10 + 48;
                i ++ ;
                if(intNum < 10)
                {   
                    strTemp[i] = intNum + 48;
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
    {
        Utl_UStrIns(strDest, _T("-"), 0);
    }
    
    return strDest;
}

T_U16*  Utl_UStrInit(T_U16* strMain,const T_pCSTR pStr)
{
    int i;
    for(i=0;pStr[i]!='\0';i++)
    {
        strMain[i] = pStr[i];
        strMain[i] &= 0xFF;
    }
    return strMain;
}

/**
 * @brief : Convert string to integer.
 * 
 * @author Junhua Zhao 
 * @date   2005-08-10 
 * @param T_pDATA strDest
 * @return T_S16
 * @retval integer value.
 */
	T_S32 Utl_UAtoi(T_U16* strMain)
	{
		T_U16*	pMain = AK_NULL;
		T_S32		sum;
		T_BOOL		negv = AK_FALSE;
		T_S16		i = 0;
		AK_ASSERT_PTR(strMain, "Utl_UAtoi()", 0);
		pMain = strMain;
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
				break;;
			pMain++;
			i++;
        #if MAX_USTRING_LEN < 0xFFFF
			AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrCarve()", 0);		/* length can't exceed MAX_USTRING_LEN */
        #endif
		}
		if (negv)
			sum *= (-1);
		return sum;
	}

/**
 * @brief get length of a string
 *
 * @author @b Junhua Zhao
 * assert the length of the string
 *
 * @author Junhua Zhao 
 * @date   2005-08-10
 * @param T_U16 * str strMain--head address of string
 * @return T_S16
 * @retval the length
 */
__ram T_U32 Utl_UStrLen(const T_U16* strMain)
{
    T_U32 len = 0;

	AK_ASSERT_PTR(strMain, "Utl_UStrLen() ", 0);
	
    if (strMain == 0)
        return 0;

    while(*(strMain+len) != 0x00)
        len++;
    return len;
}

/**
 * @brief get byte count of a string
 *
 * @author Junhua Zhao
 *
 * @author Junhua Zhao 
 * @date   2005-08-10
 * @param T_U16 * str strMain--head address of string
 * @return T_S16
 * @retval the length
 */
T_U16   Utl_UByteCount(const T_U16* strMain,T_U16 strLen)
{
    T_U16 len = 0,i;

    if (strMain == 0)
        return 0;

    for(i=0;(i<strLen) && (*(strMain+i)!=0x00);i++)
    {
        if (Utl_UIsDWordChar(strMain+i,gs.Lang))
            len += 2;
        else len++;
    }
        
    return len;
}

/**
 * @brief search for strSub in strMain from the offset.
 * This fucntion should be called if the string is very long.
 *
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * strMain head address of main string to search in
 * @param  T_U16 * strSub head address of sub string to search for
 * @param  T_S16 offset begin address offset
 * @param  T_U16 length: string length
 * @return T_S16
 * @retval -1: not find; else offset in main string
 */
__ram T_S16 Utl_UStrFndL(T_U16* strMain, const T_U16* strSub, T_S16 offset, T_U16 length)
{
    T_U16   curLoc = offset;
    T_U16   i;
    T_U16   subStrLen;
    T_U16*  pMain;

    AK_ASSERT_PTR(strMain, "Utl_UStrFndL(): strMain", -1);
    AK_ASSERT_PTR(strSub, "Utl_UStrFndL(): strSub", -1);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrFndL()", -1);      /* length can't exceed MAX_USTRING_LEN */
    AK_ASSERT_VAL(length < MAX_USTRING_LEN, "Utl_UStrFndL()", -1);      /* length can't exceed MAX_USTRING_LEN */
#endif

    if (offset < 0)
        return -1;

    subStrLen = (T_U16)Utl_UStrLen(strSub);
    if (offset + subStrLen > length)
        return -1;

    pMain = strMain + offset;

    while (*(pMain + subStrLen - 1) != 0)
    {
        //AK_ASSERT_VAL(curLoc < MAX_USTRING_LEN, "Utl_UStrFndL()", -1);        /* length can't exceed MAX_USTRING_LEN */

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
 * @param T_U16 * strDest: head address of destination string
 * @param  T_U16 * strSour: head address of source string
 * @param  T_S16 offset: begin address offset
 * @param  T_U16 end: end address
 * @param  T_U16 length: string length
 * @return T_S8 head address of destination string
 */
__ram T_U16 * Utl_UStrMidL(T_U16 * strDest, const T_U16* strSour, T_S16 offset, T_S16 end, T_U16 strlength)
{
    T_U16   iLength;
    T_U16   i;

    AK_ASSERT_PTR(strDest, "Utl_UStrMidL()", AK_NULL);
    AK_ASSERT_PTR(strSour, "Utl_UStrMidL()", AK_NULL);
#if MAX_USTRING_LEN < 0xFFFF
    AK_ASSERT_VAL(offset < MAX_USTRING_LEN, "Utl_UStrMidL()", AK_NULL);     /* length can't exceed MAX_USTRING_LEN */
    AK_ASSERT_VAL(end < MAX_USTRING_LEN, "Utl_UStrMidL()", AK_NULL);        /* length can't exceed MAX_USTRING_LEN */
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

    for (i = offset; i <= end; i++)
    {           
        strDest[i - offset] = *(strSour + i);
    }
    strDest[i - offset] = '\0';
    return strDest;
}

/**
 * @brief Compare two string ingore case
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * str1 head address of string1
 * @param  T_U16 * str2 head address of string1
 * @return T_S16
 * @retval compare value
 */
__ram T_S8 Utl_UStrCmpC(T_U16* str1, T_U16* str2)
{
    T_U16   c1, c2;
    T_S16   i=0;
    T_U16   *pStr1 = str1;
    T_U16   *pStr2 = str2;

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
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param T_U16 * strDest head address of source string
 * @param  T_U16 * strSour head address of destination string
 * @return T_S8
 * @retval head address of destination string
 */
__ram T_U16 * Utl_UStrCpy(T_U16 * strDest, const T_U16* strSour)
{
    T_S16   i = 0;
    T_U16   *d;
    T_U16   *s;

    AK_ASSERT_PTR(strDest, "Utl_UStrCpy(): strDest", AK_NULL);
    AK_ASSERT_PTR(strSour, "Utl_UStrCpy(): strSour", AK_NULL);
    
    d = strDest;
    s = (T_U16 *)strSour;

    while (*s)
    {
        *d++ = *s++;
        i++;
        //AK_ASSERT_VAL(i < MAX_USTRING_LEN, "Utl_UStrCpy(): i", AK_NULL);      /* length can't exceed MAX_USTRING_LEN */
    }
    *d = 0;
    
    return strDest;
}

/**
 * @brief Copy 'length' bytes from strSour to strDest.
 * 
 * @author Junhua Zhao 
 * @date  2005-08-10 
 * @param  T_U16 * strDest head address of source string
 * @param  T_U16 * strSour head address of destination string
 * @param  T_S16 length  number of bytes.
 * @return T_S8
 * @retval head address of destination string
 */
__ram T_U16 * Utl_UStrCpyN(T_U16 * strDest, const T_U16* strSour, T_U32 length)
{
    T_U32   i = 0;
    T_U16 *     d;
    const T_U16*  s;

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


T_VOID Utl_USplitFilePath(T_pCWSTR file_path, T_pWSTR path, T_pWSTR name)
{
    T_S32 i, len;

    if ((file_path == AK_NULL) || ((T_U16)Utl_UStrLen(file_path) == 0))
    {
        if (path != AK_NULL)
        {
            path[0] = 0;
        }
        if (name != AK_NULL)
        {
            name[0] = 0;
        }
        return;
    }

    len = (T_U16)Utl_UStrLen(file_path);
    for (i=len-1; i>=0; i--)
    {
        if ((file_path[i] == UNICODE_SOLIDUS) || (file_path[i] == UNICODE_RES_SOLIDUS))
            break;
    }

    if (path != AK_NULL)
    {
        Utl_UStrCpyN(path, file_path, i+1);
        path[i+1] = 0;
    }
    if (name != AK_NULL)
    {
        Utl_UStrCpy(name, &file_path[i+1]);
        name[len-(i+1)] = 0;
    }
}

T_VOID  Utl_USplitFileName(T_pCWSTR file_name, T_pWSTR name, T_pWSTR ext)
{
    T_S32       i, j, len;
    T_USTR_FILE tmp_ext = {0};
    T_U16       ch;

    if ((file_name == AK_NULL) || ((T_U16)Utl_UStrLen(file_name) == 0))
    {
        name[0] = 0;
        if (ext != AK_NULL)
        {
            ext[0] = 0;
        }
        return;
    }

    len = (T_U16)Utl_UStrLen(file_name);
    for (i=len-1, j=0; i>=0; i--)
    {
        ch = file_name[i];
        if (ch == UNICODE_DOT)
            break;
        else
            tmp_ext[j++] = ch;
    }

    if (i > 0)
    {
        Utl_UStrCpyN(name, file_name, i);
        name[i] = 0;
    }
    else
    {
        Utl_UStrCpy(name, file_name);    //don't exist dot
    }

    if (ext != AK_NULL)
    {
        for (i=0; i<j; i++)
            ext[j-1-i] = tmp_ext[i];
        ext[j] = 0;
    }
}

T_U16 Utl_CaclSolidas(const T_U16* str)
{
	T_U16 cnt = 0;
	T_U32 len = Utl_UStrLen(str);	
	
	while (len)
	{
		if (UNICODE_SOLIDUS == str[len--])
			cnt++;
	}
	
	return cnt;
}


/**
 * @brief get the disk from file path.
 * 
 * @author zhengwenbo
 * @date  2007-08-27 
 * @param  T_USTR_FILE file_path: the whole file path
 * @return T_U16
 * @retval the disk
 */
T_U16 GetFilePathDisk(T_USTR_FILE file_path)
{
    T_USTR_FILE path;
    
    if ((file_path == AK_NULL) || ((T_U16)Utl_UStrLen(file_path) == 0))
    {
        return 0;
    }

    Utl_UStrCpy(path, file_path);

    Utl_UStrLTrim(path);
    
    return path[0];
}

/**
 * @brief judge if file is saved in SD card
 * 
 * @author zhengwenbo
 * @date  2007-09-05 
 * @param  T_USTR_FILE file_path: the whole file path
 * @return T_BOOL
 * @retval AK_TRUE: in SD  AK_FALSE: not in SD
 */
T_BOOL FileIsInSD(T_USTR_FILE file_path)
{
    T_U16 disk = UNICODE_c;

    disk = GetFilePathDisk(file_path);
    if ((UNICODE_d == disk) || (UNICODE_D == disk))
    {
        return AK_TRUE;
    }
    else
    {
        return AK_FALSE;
    }
}

T_VOID  Printf_UC(T_pCWSTR ustr)
{
    T_S8    strtmp[1025];
    T_S32   strLen = 0;
    T_U8    ch = ' '; //blank char

    strLen = Eng_WideCharToMultiByte(gs.Lang, ustr, (T_U16)Utl_UStrLen(ustr), AK_NULL, strtmp, 1024, &ch);
    strtmp[strLen] = 0;

    //ConsolePrint(0, "$$$$Print_UC:  %s\n", strtmp);
    AK_DEBUG_OUTPUT("$$$$Print_UC: %s.\n",strtmp);
}


