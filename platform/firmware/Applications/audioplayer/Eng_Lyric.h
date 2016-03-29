
#ifndef __LYRIC_H__
#define __LYRIC_H__

#include "anyka_types.h"
#include "Eng_String.h"
#include "Eng_String_uc.h"

#ifndef LYRIC_STRING_LEN
#define LYRIC_STRING_LEN            256
#define LYRIC_CONTENT_NUM           1024
#define LYRIC_HEAD_LEN              32
#define LYRIC_TIME_LEN              8
#endif

typedef enum {
    LYRIC_HEAD_AR = 0,
    LYRIC_HEAD_TI,
    LYRIC_HEAD_AL,
    LYRIC_HEAD_BY,
    LYRIC_HEAD_KEY,
    LYRIC_HEAD_NUM
} T_LYRIC_HEAD;

typedef struct {
    T_U32 time;
    T_U16 lrc[LYRIC_STRING_LEN+1];
} T_LYRIC_CONTENT;

typedef struct {
    T_U16 ar[LYRIC_STRING_LEN+1];
    T_U16 ti[LYRIC_STRING_LEN+1];
    T_U16 al[LYRIC_STRING_LEN+1];
    T_U16 by[LYRIC_STRING_LEN+1];
    T_S32 offset;
    T_U16 key[LYRIC_STRING_LEN+1];
    T_U32 num;
    T_LYRIC_CONTENT *content[LYRIC_CONTENT_NUM];

    T_RECT rect;
    T_U32 htime;
    T_U8 maxline;
    T_U16 *pStr;
    T_CARVED_USTR CarvedStr;
    T_U8 line;
} T_LYRIC_STRUCT;

T_BOOL Lyric_ParseBuf(T_U16 *str, T_LYRIC_STRUCT *lyric, T_RECT rect);

T_VOID Lyric_Free(T_LYRIC_STRUCT *lyric);

T_VOID Lyric_Show(T_LYRIC_STRUCT *lyric, T_U32 time);

T_VOID Lyric_String2Unc(const T_S8 *src, T_U16 *ucBuf);

#endif  /* end of __LYRIC_H__ */

