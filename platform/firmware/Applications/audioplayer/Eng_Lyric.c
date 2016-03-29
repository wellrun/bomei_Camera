#include <string.h>

#ifdef SUPPORT_AUDIOPLAYER


#include <stdlib.h>
#include "Eng_Lyric.h"
#include "Fwl_osMalloc.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Eng_String_UC.h"
#include "Eng_DataConvert.h"
#include "eng_debug.h"

#include "fwl_display.h"




#ifndef LYRIC_ID
#define LYRIC_ID                    0
#define LYRIC_ID_HEAD               1
#define LYRIC_ID_HBUF               2
#endif

#ifndef LYRIC_TIME
#define LYRIC_TIME                  0
#define LYRIC_TIME_MM               1
#define LYRIC_TIME_SS               2
#define LYRIC_TIME_FFF              3
#define LYRIC_TIME_NUM              32
#endif


T_BOOL Lyric_ParseBuf(T_U16 *str, T_LYRIC_STRUCT *lyric, T_RECT rect)
{
    T_U16 head[LYRIC_HEAD_LEN+1], hbuf[LYRIC_STRING_LEN+1];
    T_U16 mm[LYRIC_TIME_NUM][LYRIC_TIME_LEN+1], ss[LYRIC_TIME_NUM][LYRIC_TIME_LEN+1], fff[LYRIC_TIME_NUM][LYRIC_TIME_LEN+1];
    T_U16 buf[LYRIC_STRING_LEN+1];
    T_U32 i = 0, j, t = 0;
    T_BOOL tag = AK_FALSE;
    T_U8 id = LYRIC_ID;
    T_U8 time = LYRIC_TIME;
    T_BOOL lflag = AK_FALSE;
    T_BOOL done;
    T_LYRIC_CONTENT tmp_content;
    T_U32 ff_count = 0;
//    T_U32 readedUCChars;
//    T_U16 ucLen;
    T_S8 *gbkBuf = AK_NULL;
//    T_U16 ch = 0x0020; //space 
    T_STR_FILE ar="ar", ti="ti", al="al", by="by", key="key", offset="offset";
    T_U16 uAr[30], uTi[30], uAl[30], uBy[30], uKey[30], uOffset[30];

    Eng_StrMbcs2Ucs(ar, uAr);
    Eng_StrMbcs2Ucs(ti, uTi);
    Eng_StrMbcs2Ucs(al, uAl);
    Eng_StrMbcs2Ucs(by, uBy);
    Eng_StrMbcs2Ucs(key, uKey);
    Eng_StrMbcs2Ucs(offset, uOffset);
    Utl_UStrCarveInit(&lyric->CarvedStr);
    memset((void *)lyric, 0, sizeof(T_LYRIC_STRUCT));
    memset((void *)head, 0, sizeof(head));
    memset((void *)hbuf, 0, sizeof(hbuf));
    memset((void *)mm, 0, sizeof(mm));
    memset((void *)ss, 0, sizeof(ss));
    memset((void *)fff, 0, sizeof(fff));
    memset((void *)buf, 0, sizeof(buf));

    lyric->rect = rect;
    lyric->maxline = lyric->rect.height/g_Font.CHEIGHT;


    while (*str != UNICODE_END) // '\0'
    {
        if (*str == 0xff)
            ff_count++;
        else
            ff_count = 0;
        if (ff_count > 3)
            break;

        if ((*str == UNICODE_LBRACKET) && (!tag))    //'['
        {
            if (id > LYRIC_ID)
            {
                if (Utl_UStrCmp(uAr, head) == 0)
                {
                    Utl_UStrCpy((T_U16 *)lyric->ar, hbuf);
                }
                else if(Utl_UStrCmp(uTi, head) == 0)
                {
                    Utl_UStrCpy(lyric->ti, hbuf);
                }
                else if(Utl_UStrCmp(uAl, head) == 0)
                {
                    Utl_UStrCpy((T_U16 *)lyric->al, hbuf);
                }
                else if(Utl_UStrCmp(uBy, head) == 0)
                {
                    Utl_UStrCpy((T_U16 *)lyric->by, hbuf);
                }
                else if(Utl_UStrCmp(uKey, head) == 0)
                {
                    Utl_UStrCpy((T_U16 *)lyric->key, hbuf);
                }
                else if(Utl_UStrCmp(uOffset, head) == 0)
                {
                    lyric->offset = Utl_UAtoi(hbuf);
                }
                id = LYRIC_ID;
            }
            else if ((t > 0) && (lflag || (Utl_UStrLen(buf) > 0)))
            {
                for (i=0; i<t; i++)
                {
                    if (lyric->num >= LYRIC_CONTENT_NUM)
                        goto SkipGetLyric;

                    lyric->content[lyric->num] = (T_LYRIC_CONTENT *)Fwl_Malloc(sizeof(T_LYRIC_CONTENT));
                    if (!AkAssertCheckPointer(lyric->content[lyric->num]))
                        goto SkipGetLyric;

                    memset((void *)lyric->content[lyric->num], 0x00, sizeof(T_LYRIC_CONTENT));

                    lyric->content[lyric->num]->time = Utl_UAtoi(mm[i])*60*1000 + Utl_UAtoi(ss[i])*1000 + Utl_UAtoi(fff[i]);
                    Utl_UStrCpy((T_U16 *)lyric->content[lyric->num]->lrc, buf);
                    lyric->num++;
                }
                memset((void *)mm, 0, sizeof(mm));
                memset((void *)ss, 0, sizeof(ss));
                memset((void *)fff, 0, sizeof(fff));
                memset((void *)buf, 0, sizeof(buf));
                time = LYRIC_TIME;
                t = 0;
            }

            memset((void *)head, 0, sizeof(head));
            memset((void *)hbuf, 0, sizeof(hbuf));
            i = 0;
            lflag = AK_FALSE;

            tag = AK_TRUE;
            str++;
        }
        else if ((*str == UNICODE_RBRACKET) && tag) //']'
        {
            if (time > LYRIC_TIME)
            {
                t++;
                time = LYRIC_TIME;
            }
            i = 0;
            tag = AK_FALSE;
            str++;
        }
        else if ((*str == UNICODE_COLON) && tag) //':'
        {
            if (id == LYRIC_ID_HEAD)
            {
                id = LYRIC_ID_HBUF;
                i = 0;
            }
            else if (id == LYRIC_ID_HBUF)
            {
                hbuf[i++] = *str;
            }
            else if (time == LYRIC_TIME_MM)
            {
                time = LYRIC_TIME_SS;
                i = 0;
            }
            str++;
        }
        else if ((*str == UNICODE_DOT) && tag) //'.'
        {
            if (id == LYRIC_ID_HBUF)
            {
                hbuf[i++] = *str;
            }
            else if (time == LYRIC_TIME_SS)
            {
                time = LYRIC_TIME_FFF;
                i = 0;
            }
            str++;
        }
        else
        {
            if (tag)
            {
                if ((id == LYRIC_ID) && (time == LYRIC_TIME))
                {
                    if ((*str >= UNICODE_0) && (*str <= UNICODE_9))
                    {
                        time = LYRIC_TIME_MM;
                    }
                    else
                    {
                        id = LYRIC_ID_HEAD;
                    }
                }

                if (id == LYRIC_ID_HEAD)
                {
                    if ((*str >= UNICODE_A) && (*str <= UNICODE_Z))
                    {
                        head[i++] = *str + (UNICODE_a-UNICODE_A);
                    }
                    else
                    {
                        head[i++] = *str;
                    }
                }
                else if (id == LYRIC_ID_HBUF)
                {
                    hbuf[i++] = *str;
                }
                else if (time == LYRIC_TIME_MM)
                {
                    mm[t][i++] = *str;
                }
                else if (time == LYRIC_TIME_SS)
                {
                    ss[t][i++] = *str;
                }
                else if (time == LYRIC_TIME_FFF)
                {
                    fff[t][i++] = *str;
                }
            }
            else if ((*str == UNICODE_N) || (*str == UNICODE_R))
            {
                lflag = AK_TRUE;
            }
            else
            {
                if (i <= LYRIC_STRING_LEN)
                    buf[i++] = *str;
                else
                    break;
            }
            str++;
        }
    }
    if (t > 0)
    {
        for (i=0; i<t; i++)
        {
            if (lyric->num >= LYRIC_CONTENT_NUM)
                goto SkipGetLyric;

            lyric->content[lyric->num] = (T_LYRIC_CONTENT *)Fwl_Malloc(sizeof(T_LYRIC_CONTENT));
            if (!AkAssertCheckPointer(lyric->content[lyric->num]))
                goto SkipGetLyric;

            memset((void *)lyric->content[lyric->num], 0x00, sizeof(T_LYRIC_CONTENT));

            lyric->content[lyric->num]->time = Utl_UAtoi(mm[i])*60*1000 + Utl_UAtoi(ss[i])*1000 + Utl_UAtoi(fff[i]);
            Utl_UStrCpy((T_U16 *)lyric->content[lyric->num]->lrc, buf);
            lyric->num++;
        }
    }

SkipGetLyric:
    j = 1;
    done = AK_FALSE;
    while (!done && (j<lyric->num))
    {
        done = AK_TRUE;
        for (i=0; i<(lyric->num-j); i++)
        {
            if (lyric->content[i]->time > lyric->content[i+1]->time)
            {
                done = AK_FALSE;
                tmp_content = *lyric->content[i];
                *lyric->content[i] = *lyric->content[i+1];
                *lyric->content[i+1] = tmp_content;
            }
        }
        j++;
    }

    if ((lyric->content[0] != AK_NULL) && ((T_S32)lyric->content[0]->time < lyric->offset))
        lyric->offset = lyric->content[0]->time;

    if (lyric->content[0] != AK_NULL)
        lyric->htime = lyric->content[0]->time-lyric->offset;
    else
        lyric->htime = 0;

    if (gbkBuf != AK_NULL)
        gbkBuf = Fwl_Free(gbkBuf);

    return AK_TRUE;
}

T_VOID Lyric_Free(T_LYRIC_STRUCT *lyric)
{
    T_U32 i;

    for (i=0; i<lyric->num; i++)
        lyric->content[i] = Fwl_Free(lyric->content[i]);

    Utl_UStrCarveFree(&lyric->CarvedStr);
}

T_VOID Lyric_Show(T_LYRIC_STRUCT *lyric, T_U32 time)
{
    T_U32 i, starttime = 0, onetime = 0;
    T_BOOL new = AK_FALSE;
    T_U16   lineLimit = 0;
    T_POS   LineLeft = 0;
    T_U16   CharNo = 0;
    T_LEN   AddSpace = 0;

    if (time < lyric->htime)
    {
        for (i=0; i<LYRIC_HEAD_NUM; i++)
        {
            if ((time >= i*lyric->htime/LYRIC_HEAD_NUM) && \
                    (time < (i+1)*lyric->htime/LYRIC_HEAD_NUM))
            {
                starttime = i*lyric->htime/LYRIC_HEAD_NUM;
                onetime = lyric->htime/LYRIC_HEAD_NUM;
                break;
            }
        }

        switch (i)
        {
        case LYRIC_HEAD_AR:
            if (lyric->pStr != lyric->ar)
            {
                lyric->pStr = lyric->ar;
                new = AK_TRUE;
            }
            break;
        case LYRIC_HEAD_TI:
            if (lyric->pStr != lyric->ti)
            {
                lyric->pStr = lyric->ti;
                new = AK_TRUE;
            }
            break;
        case LYRIC_HEAD_AL:
            if (lyric->pStr != lyric->al)
            {
                lyric->pStr = lyric->al;
                new = AK_TRUE;
            }
            break;
        case LYRIC_HEAD_BY:
            if (lyric->pStr != lyric->by)
            {
                lyric->pStr = lyric->by;
                new = AK_TRUE;
            }
            break;
        case LYRIC_HEAD_KEY:
            if (lyric->pStr != lyric->key)
            {
                lyric->pStr = lyric->key;
                new = AK_TRUE;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        for (i=0; i<lyric->num; i++)
        {
            if ((time >= lyric->content[i]->time-lyric->offset) && \
                    ((i+1<lyric->num)?(time<lyric->content[i+1]->time-lyric->offset):1))
            {
                starttime = lyric->content[i]->time-lyric->offset;
                onetime = ((i+1<lyric->num)?lyric->content[i+1]->time:lyric->content[i]->time)-lyric->content[i]->time;
                break;
            }
        }

        if ((i < lyric->num) && (lyric->pStr != lyric->content[i]->lrc))
        {
            lyric->pStr = lyric->content[i]->lrc;
            new = AK_TRUE;
        }
    }

    lineLimit = (T_U16)(lyric->rect.width);
    if (lyric->pStr != AK_NULL)
    {
        if (new == AK_TRUE)
        {
            Utl_UStrCarveFree(&lyric->CarvedStr);
            Utl_UStrCarveInit(&lyric->CarvedStr);
 //           lyric->CarvedStr.MaxLen = Fwl_GetLcdWidth();
            Utl_UStrCarve(lyric->pStr, lineLimit, AK_NULL, 
            &lyric->CarvedStr, lineLimit);
            lyric->line = 0;
        }

        if (lyric->CarvedStr.LineNum > lyric->maxline)
        {
            T_U8 num = lyric->CarvedStr.LineNum-lyric->maxline+1;

            for (i=0; i<num; i++)
            {
                if ((time-starttime >= i*onetime/num) && \
                        ((i+1<num)?(time-starttime < (i+1)*onetime/num):1))
                {
                    lyric->line = (T_U8)i;
                    break;
                }
            }
        }

        for (i=lyric->line; ((i<lyric->CarvedStr.LineNum) && (i-lyric->line<lyric->maxline)); i++)
        {
            CharNo = (T_U16)UGetSpeciStringWidth(lyric->CarvedStr.String[i], CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(lyric->CarvedStr.String[i]));

            if (CharNo < lineLimit)
            {
                AddSpace = (lineLimit - (CharNo )) >> 1;
            }
            LineLeft = lyric->rect.left + AddSpace;
                
            Fwl_UDispSpeciString(HRGB_LAYER, \
                    LineLeft, \
                    (T_POS)(lyric->rect.top+g_Font.SCHEIGHT*(i-lyric->line)), \
                    lyric->CarvedStr.String[i], \
                    COLOR_BLACK, CURRENT_FONT_SIZE, \
                    CharNo);
        }
    }
}

T_VOID Lyric_String2Unc(const T_S8 *src, T_U16 *ucBuf)
{
    T_U32   ustrlen = 0;
    //T_U16   ch = UNICODE_SPACE; //space 

    if ((*((T_U8*)src) == 0xff) && ((*((T_U8 *)src)+1) == 0xfe))
    {
        Utl_UStrCpy(ucBuf, ((T_U16 *)src+2));
    }
    else
    {
        //ustrlen = (T_U32)Eng_MultiByteToWideChar(gs.Lang, src, strlen(src), AK_NULL, ucBuf, MAX_FILE_LEN, &ch);
        ustrlen = (T_U32)Eng_StrMbcs2Ucs(src, ucBuf);
        ucBuf[ustrlen] = 0;
     }
}
#endif

