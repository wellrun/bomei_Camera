/**
 * @file Eng_Melody.c
 * @brief ANYKA software
 * Play EMS melody
 * @author ZouMai
 * @date 2002-07-22
 * @version 1.0
 */
#ifdef OS_WIN32

#include "akdefine.h"
#include "w_melody.h"
#include "windows.h"
#include "math.h"

#define MAX_EVENT   2000

typedef struct
{
    unsigned long   delta;      /* delta number */
    unsigned char   command;
    unsigned char   channel;
    unsigned char   note;
    unsigned char   velocity;
} MIDI_EVENT;

typedef struct
{
    T_pDATA         midiData;
    long            tempo;
    int             format;
    int             trackNum;
    unsigned long   tickNum;
    int             eventNum;
    MIDI_EVENT      midiEvent[MAX_EVENT];
} MIDI_INFO;

static T_VOID   ThreadForiMelodyPlay(T_VOID);
static T_S16    GetDgtNote(T_pCSTR strNote, T_S8 octave);
static T_BOOL   ReadMidiInfo(MIDI_INFO *midiInfo);
static T_BOOL   ReadMidiTrackEvents(MIDI_INFO *midiInfo);
static T_BOOL   ReOrganizeMidiInfo(MIDI_INFO *midiInfo);
static T_VOID   MidiInfo2Melody(const MIDI_INFO midiInfo, char *melodybuf);
static T_VOID   GetOctave(const int curOct, const int defOct, char *strOct);
static T_VOID   GetStrNote(const int octave, char *strNote);
static long     WriteMelodyBuf(const char *strOctave, const char *strNote, const long noteLen, char *melodyBuf);
static long     UniteMultiHex(const unsigned char *multiHex, const int begin, const int end);
static unsigned long ReadVarLen(MIDI_INFO *midiInfo);

static T_pSTR   gb_MelodyData = AK_NULL;
static T_S16    gb_MelodyLoop = -1; /* >0: playing, 0: stop, -1: stopped */
static T_fMELODY_CALLBACK   gb_MemodyCallback = AK_NULL;

/**
 * @brief Create a thread to play melody. User can call iMelodyStop() to stop it
 * Two global variable are concerned in this function:
 * @author ZouMai
 * @date 2002-07-22
 * @param T_pCSTR melodyBuf: Standard iMelody format string, end with '\0'
 * @param T_U16 loop: loop number.
 * @param T_fMELODY_CALLBACK callback
 * @return T_BOOL: played or not
 * @retval
 */
T_BOOL iMelodyPlay(T_pSTR melodyBuf, T_U16 loop, T_fMELODY_CALLBACK callback)
{
    T_U32 nThreadID;

    if (gb_MelodyLoop > 0)
        return AK_FALSE;

    gb_MelodyData = melodyBuf;
    gb_MelodyLoop = loop;
    gb_MemodyCallback = callback;

    if (melodyBuf == AK_NULL || loop == 0)
        return AK_FALSE;

    if (strlen(melodyBuf) < 2)
    {
        return AK_FALSE;
    }

    if (CreateThread(AK_NULL, 0, (LPTHREAD_START_ROUTINE)ThreadForiMelodyPlay, NULL, 0, &nThreadID) == NULL)
    {
        if (gb_MemodyCallback != AK_NULL)
        {
            gb_MemodyCallback(AK_FALSE);
            gb_MemodyCallback = AK_NULL;
        }
        gb_MelodyData = AK_NULL;
        gb_MelodyLoop = -1;
        return AK_FALSE;
    }

    return AK_TRUE;
}

/**
 * @brief Stop playing melody
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-07-23
 * @param T_VOID
 * @return T_VOID
 * @retval
 */
T_VOID iMelodyStop(T_VOID)
{
    if (gb_MelodyLoop <= 0)
        return;

    gb_MelodyLoop = 0;

    while (gb_MelodyLoop != -1);

    TerminateThread((HANDLE)ThreadForiMelodyPlay, 0);

    return;
}

T_BOOL  iMelodyIsPlaying(T_VOID)
{
    if (gb_MelodyLoop > 0)
        return AK_TRUE;
    else
        return AK_FALSE;
}

/* input midi data, output molody buffer */
T_BOOL ConvertMIDI2Melody(T_pCDATA midiData, T_pSTR melodyData)
{
    MIDI_INFO midiInfo;

    midiInfo.midiData = (T_pDATA)midiData;
    ReadMidiInfo(&midiInfo);
    ReOrganizeMidiInfo(&midiInfo);
    MidiInfo2Melody(midiInfo, melodyData);

    return AK_TRUE;
}

/**
 * @brief Play melody from the standard iMelody format string
 * Two C runtime functions are called in this function: exp(), log(), So this function can only
 * be used in Windows OS
 *
 * @author @b ZouMai
 *
 * @author
 * @date 2002-07-23
 * @param VOID
 * @return T_VOID
 * @retval
 */
static T_VOID ThreadForiMelodyPlay(T_VOID)
{
    T_S8    octave;
    T_S8    strNote[10];
    T_S16   note;
    T_U8    lenBase[10] = {192, 96, 48, 24, 12, 6, 3, 0};
    T_U8    lenMulti = 10;
    T_S8    lenID;
    T_U8    frqBase = 33;
    T_U16   freq;
    const   T_S8    *p = AK_NULL;

    p = gb_MelodyData;

    while (gb_MelodyLoop > 0)
    {
        if (*p == 0)
        {
            gb_MelodyLoop--;
            if (gb_MelodyLoop == 0)
                break;
            else
                p = gb_MelodyData;
        }

        octave = 4;
        if (*p == '*')
        {
            octave = (T_S8)(*(++p) - 0x30);
            p++;
        }

        strNote[0] = 0;
        if (*p == '#')
        {
            strNote[0] = *p;
            strNote[1] = *(++p);
            if (strNote[1] == 0)
                continue;
        }
        else if (*p >= 'a' && *p <= 'g' || *p == 'r')
        {
            strNote[0] = *p;
        }

        note = GetDgtNote(strNote, octave);

        lenID = 6;
        if (note > -2)
            lenID = (T_S8)(*(++p) - 0x30);

        if (lenID < 0)
            continue;

        if (lenID >= 7)
            lenID = 7;

        if (note >= 0)
        {
            freq = (T_U16)(frqBase * exp(note / 12.0 * log(2)));
            Beep(freq, (T_U16)(lenBase[lenID] * lenMulti));
        }
        else if (note == -1)
        {
            Sleep(lenBase[lenID] * lenMulti);
        }
        p++;
    }

    if (gb_MemodyCallback != AK_NULL)
    {
        gb_MemodyCallback(AK_FALSE);
        gb_MemodyCallback = AK_NULL;
    }
    gb_MelodyData = AK_NULL;
    gb_MelodyLoop = -1;

    return;
}

/**
 * @brief  Get the note
 *
 * @author @b Zouami
 *
 * @author
 * @date 2002-07-25
 * @param T_pSTR strNote
 * @param  const T_S8 octave
 * @return T_S16
 * @retval
 */
static T_S16 GetDgtNote(T_pCSTR strNote, T_S8 octave)
{
    T_S8 chrNote;
    T_S16  note = 0;

    if (strNote[0] == 0)
        return -2;                          /* illegal note */

    if (strNote[0] == 'r')                  /* rest note */
        return -1;

    if (strNote[0] == '#')
        chrNote = strNote[1];
    else
        chrNote = strNote[0];

    switch (chrNote) {                      /* convert tone */
    case 'c':
        note = 0;
        break;
    case 'd':
        note = 2;
        break;
    case 'e':
        note = 4;
        break;
    case 'f':
        note = 5;
        break;
    case 'g':
        note = 7;
        break;
    case 'a':
        note = 9;
        break;
    case 'b':
        note = 11;
        break;
    default:                                /* illegal note */
        return -2;
    }

    if (strNote[0] == '#')
        note++;

    return (note + octave * 12);
}


static T_BOOL ReadMidiInfo(MIDI_INFO *midiInfo)
{
    T_U8 strTemp[100];
    int  headLen;
    int  i;

    memcpy(strTemp, midiInfo->midiData, 4);
    midiInfo->midiData += 4;
    strTemp[4] = 0;

    if (strcmp(strTemp, "MThd") != 0)
    {
        //AK_DEBUG_OUTPUT("Not a MIDI file\n");
        return AK_FALSE;
    }

    memcpy(strTemp, midiInfo->midiData, 4);
    midiInfo->midiData += 4;
    headLen = UniteMultiHex(strTemp, 0, 3);

    memcpy(strTemp, midiInfo->midiData, (T_U16)headLen);
    midiInfo->midiData += headLen;

    midiInfo->tempo = 500000;               /* set tempo as default value */
    midiInfo->format = UniteMultiHex(strTemp, 0, 1);
    midiInfo->trackNum = UniteMultiHex(strTemp, 2, 3);
    if ((strTemp[4] & 0x80) == 0)           /* need to consider the MSB late */
    {
        midiInfo->tickNum  = UniteMultiHex(strTemp, 4, 5);
        if (midiInfo->tickNum == 0)
        {
            //AK_DEBUG_OUTPUT("Tick number can\'t be equal zero\n");
            return AK_FALSE;
        }
        else if (midiInfo->tickNum >= 0x8000)
        {
            //AK_DEBUG_OUTPUT("Tick number greater than 0x80\n");
            return AK_FALSE;
        }
    }
    else
    {
        //AK_DEBUG_OUTPUT("Not support tick number format: 2\n");
        return AK_FALSE;
    }

    if (midiInfo->format >= 2)
    {
        //AK_DEBUG_OUTPUT"Not support MIDI format: 2\n");
        return AK_FALSE;
    }

    if (midiInfo->format == 0)
        midiInfo->trackNum = 1;

    if (midiInfo->trackNum > 2)     /* only one track is supported now */
        midiInfo->trackNum = 2;

    midiInfo->eventNum = 0;

    for (i = 0; i < midiInfo->trackNum; i++)
    {
        memcpy(strTemp, midiInfo->midiData, 4);
        midiInfo->midiData += 4;
        strTemp[4] = 0;
        if (strcmp(strTemp, "MTrk") != 0)
        {
            //AK_DEBUG_OUTPUT("MIDI track information not found\n");
            return AK_FALSE;
        }
        memcpy(strTemp, midiInfo->midiData, 4);
        midiInfo->midiData += 4;
        if (!ReadMidiTrackEvents(midiInfo))
            break;
    }
    return AK_TRUE;
}

static T_BOOL ReadMidiTrackEvents(MIDI_INFO *midiInfo)
{
    long delta = 0;
    unsigned char command;

    while(1)
    {
        delta += ReadVarLen(midiInfo);
        command = *midiInfo->midiData;
        midiInfo->midiData++;

        /* The followings belong to MIDI Control Events */
        if ((command & 0xf0) == 0x80 || (command & 0xf0) == 0x90)
        {
            midiInfo->midiEvent[midiInfo->eventNum].delta = delta;
            midiInfo->midiEvent[midiInfo->eventNum].command = (command & 0xf0);
            midiInfo->midiEvent[midiInfo->eventNum].channel = (command & 0x0f);
            midiInfo->midiEvent[midiInfo->eventNum].note = *midiInfo->midiData;
            midiInfo->midiData++;
            midiInfo->midiEvent[midiInfo->eventNum].velocity = *midiInfo->midiData;
            midiInfo->midiData++;
            midiInfo->eventNum++;
            delta = 0;
        }
        else if (command < 0x80)            /* running status: available in MIDI Control Events */
        {
            midiInfo->midiEvent[midiInfo->eventNum].delta = delta;
            midiInfo->midiEvent[midiInfo->eventNum].command = midiInfo->midiEvent[midiInfo->eventNum-1].command;
            midiInfo->midiEvent[midiInfo->eventNum].channel = midiInfo->midiEvent[midiInfo->eventNum-1].channel;
            midiInfo->midiEvent[midiInfo->eventNum].note = command;
            midiInfo->midiEvent[midiInfo->eventNum].velocity = *midiInfo->midiData;
            midiInfo->midiData++;
            midiInfo->eventNum++;
            delta = 0;
        }
        else if ((command & 0xf0) == 0xa0 || (command & 0xf0) == 0xb0 || (command & 0xf0) == 0xe0)
        {                                   /* skip 2 bytes */
            command = *midiInfo->midiData;
            midiInfo->midiData++;
            command = *midiInfo->midiData;
            midiInfo->midiData++;
        }
        else if ((command & 0xf0) == 0xc0 || (command & 0xf0) == 0xd0)
        {                                   /* skip 1 byte */
            command = *midiInfo->midiData;
            midiInfo->midiData++;
        }

        /* The followings belong to SysEx Events */
        else if (command == 0xf0 || command == 0xf7)
        {
            unsigned long length;

            length = ReadVarLen(midiInfo);
            while (length--)
            {
                command = *midiInfo->midiData;
                midiInfo->midiData++;
            }
        }

        /* The followings belong to Meta Events */
        else if (command == 0xff)
        {
            unsigned long length;
            unsigned char metatype;

            metatype = *midiInfo->midiData;
            midiInfo->midiData++;
            length = ReadVarLen(midiInfo);
//          length = *midiInfo->midiData;
//          midiInfo->midiData++;
            if (metatype == 0x2f)           /* end of track */
            {
//              midiInfo->midiEvent[midiInfo->eventNum].delta = delta;
//              midiInfo->midiEvent[midiInfo->eventNum].command = metatype;
//              midiInfo->eventNum++;
                break;
            }
            else if (metatype == 0x51)
            {
                T_U8 strTemp[10];
                memcpy(strTemp, midiInfo->midiData, (T_U16)length);
                midiInfo->midiData += length;
                midiInfo->tempo = UniteMultiHex(strTemp, 0, length - 1);
            }
            else                            /* skip several bytes */
            {
                while (length--)
                {
                    command = *midiInfo->midiData;
                    midiInfo->midiData++;
                }
            }
        }
        if (midiInfo->eventNum >= MAX_EVENT)
            return AK_FALSE;
    }

    return AK_TRUE;
}

static T_BOOL ReOrganizeMidiInfo(MIDI_INFO *midiInfo)
{
    int i;
    int  preNoteOn = 0;
    long delta = 0;

    for (i = 0; i < midiInfo->eventNum; i++)        // 0x90 00 ==> 0x80 00
    {
        if (midiInfo->midiEvent[i].command == 0x90 && midiInfo->midiEvent[i].velocity == 0x00)
            midiInfo->midiEvent[i].command = 0x80;
    }

    for (i = 1; i < midiInfo->eventNum; i++)        // example: 0x90 0x00 0x90 ==> 0x90
    {
        delta += midiInfo->midiEvent[i].delta;

        if (midiInfo->midiEvent[i].command == 0x90)
        {
            if (delta == 0)
            {
                int j;
                int begin;
                int delnum = 1;

                if (midiInfo->midiEvent[preNoteOn].note > midiInfo->midiEvent[i].note)
                    begin = i;
                else
                    begin = preNoteOn;

                for (j = begin; j < midiInfo->eventNum - delnum; j++)
                {
                    if (delnum == 1 && midiInfo->midiEvent[j+1].command == 0x80 &&
                        midiInfo->midiEvent[j+1].note == midiInfo->midiEvent[begin].note)
                    {
                        delnum = 2;
                        midiInfo->midiEvent[j+2].delta += midiInfo->midiEvent[j+1].delta;
                    }
                    midiInfo->midiEvent[j] = midiInfo->midiEvent[j + delnum];
                }
                midiInfo->eventNum -= delnum;
            }
            else
            {
                delta = 0;
                preNoteOn = i;
            }
        }
    }

    return AK_TRUE;
}

/* Convert MIDI_INFO structure to EMS mode string */
static T_VOID MidiInfo2Melody(const MIDI_INFO midiInfo, char *melodyBuf)
{
    int i, j;
    int curTone;
    int curOctave, defOctave = 4;
    long noteLen;
    long remain = 0;
    long curDelta;
    char strNote[10];
    char strOctave[10];

    melodyBuf[0] = 0;

    for (i = 0; i < midiInfo.eventNum; i++)
    {
        if (midiInfo.midiEvent[i].command == 0x90)
        {
            curTone   = midiInfo.midiEvent[i].note % 12;
            curOctave = midiInfo.midiEvent[i].note / 12;
            if (curOctave > 0)
                curOctave--;

            GetOctave(curOctave, defOctave, strOctave); /* get string mode octave */
            GetStrNote(curTone, strNote);               /* get string mode note */

            j = i;                                      /* get string mode note length */
            curDelta = 0;
            do
            {
                curDelta += midiInfo.midiEvent[++j].delta;
                if (midiInfo.midiEvent[j].command == 0x80 &&    /* has rest note */
                    midiInfo.midiEvent[j].note == midiInfo.midiEvent[i].note &&
                    midiInfo.midiEvent[j].channel == midiInfo.midiEvent[i].channel)
                {
                    noteLen = curDelta * 960 / midiInfo.tickNum + remain;
                    remain = WriteMelodyBuf(strOctave, strNote, noteLen, melodyBuf);
                    curDelta = 0;
                    strcpy(strOctave, "");
                    strcpy(strNote, "r");
                }
            }
            while (midiInfo.midiEvent[j].command != 0x90 && j < midiInfo.eventNum - 1);

            noteLen = curDelta * 960 / midiInfo.tickNum + remain;
            remain = WriteMelodyBuf(strOctave, strNote, noteLen, melodyBuf);
        }
    }
    return;
}

static T_VOID GetOctave(const int curOct, const int defOct, char *strOct)
{
    strOct[0] = 0;

    if (curOct != defOct)               /* convert octave */
    {
        strOct[0] = '*';
        strOct[1] = curOct + 0x30;
        strOct[2] = 0;
    }
    return;
}

static T_VOID GetStrNote(const int tone, char *strNote)
{
    switch (tone) {                     /* convert tone */
    case 0:
        strcpy(strNote, "c");
        break;
    case 1:
        strcpy(strNote, "#c");
        break;
    case 2:
        strcpy(strNote, "d");
        break;
    case 3:
        strcpy(strNote, "#d");
        break;
    case 4:
        strcpy(strNote, "e");
        break;
    case 5:
        strcpy(strNote, "f");
        break;
    case 6:
        strcpy(strNote, "#f");
        break;
    case 7:
        strcpy(strNote, "g");
        break;
    case 8:
        strcpy(strNote, "#g");
        break;
    case 9:
        strcpy(strNote, "a");
        break;
    case 10:
        strcpy(strNote, "#a");
        break;
    case 11:
        strcpy(strNote, "b");
        break;
    default:
        break;
    }
    return;
}

static long WriteMelodyBuf(const char *strOctave, const char *strNote, const long noteLen, char *melodyBuf)
{
    long remain = noteLen;

    do
    {
        if (remain > 2880)//>= 3360)                /* 3840 = 4 * 960 = 4 - - - */
        {                               /* 3360 = (3840 + 2880) / 2 = 6720 / 2 */
            strcat(melodyBuf, strOctave);
            strcat(melodyBuf, strNote);
            strcat(melodyBuf, "0");
            remain -= 3840;
        }
//      else if (remain > 1920)//>= 2400)       /* 2880 = 3 * 960 = 3 - - */
//      {                               /* 2400 = (2880 + 1920) / 2 = 4800 / 2 */
//          strcat(melodyBuf, strOctave);
//          strcat(melodyBuf, strNote);
//          strcat(melodyBuf, "1");
//          strcat(melodyBuf, strOctave);
//          strcat(melodyBuf, strNote);
//          strcat(melodyBuf, "2");
//          remain -= 2880;
//      }
        else if (remain > 1440)//>= 1680)       /* 1920 = 2 * 960 = 2 - */
        {                               /* 1680 = (1920 + 1440) / 2 = 3360 / 2 */
            strcat(melodyBuf, strOctave);
            strcat(melodyBuf, strNote);
            strcat(melodyBuf, "1");
            remain -= 1920;
        }
//      else if (remain > 960)//>= 1200)        /* 1440 = (1+1/2) * 960 = 1 . */
//      {                               /* 1200 = (1440 + 960) / 2 = 2400 / 2 */
//          strcat(melodyBuf, strOctave);
//          strcat(melodyBuf, strNote);
//          strcat(melodyBuf, "2");
//          strcat(melodyBuf, strOctave);
//          strcat(melodyBuf, strNote);
//          strcat(melodyBuf, "3");
//          remain -= 1440;
//      }
        else if (remain > 720)//>= 840)         /* 960 = 1 * 960 = 1 */
        {                               /* 840 = (960 + 720) / 2 = 1680 / 2 */
            strcat(melodyBuf, strOctave);
            strcat(melodyBuf, strNote);
            strcat(melodyBuf, "2");
            remain -= 960;
        }
//      else if (remain > 480)//>= 600)         /* 720 = (3/4) * 960 = 1/2 . */
//      {                               /* 600 = (720 + 480) / 2 = 1200 / 2 */
//          strcat(melodyBuf, strOctave);
//          strcat(melodyBuf, strNote);
//          strcat(melodyBuf, "3");
//          strcat(melodyBuf, strOctave);
//          strcat(melodyBuf, strNote);
//          strcat(melodyBuf, "4");
//          remain -= 720;
//      }
        else if (remain > 360)//>= 420)         /* 480 = (1/2) * 960 = 1/2 */
        {                               /* 420 = (480 + 360) / 2 = 840 / 2 */
            strcat(melodyBuf, strOctave);
            strcat(melodyBuf, strNote);
            strcat(melodyBuf, "3");
            remain -= 480;
        }
//      else if (remain > 240)//>= 300)         /* 360 = (3/8) * 960 = 1/4 . */
//      {                               /* 300 = (360 + 240) / 2 = 600 / 2 */
//          strcat(melodyBuf, strOctave);
//          strcat(melodyBuf, strNote);
//          strcat(melodyBuf, "4");
//          strcat(melodyBuf, strOctave);
//          strcat(melodyBuf, strNote);
//          strcat(melodyBuf, "5");
//          remain -= 360;
//      }
        else if (remain > 120)//>= 180)         /* 240 = (1/4) * 960 = 1/2 */
        {                               /* 180 = (240 + 120) / 2 = 360 / 2 */
            strcat(melodyBuf, strOctave);
            strcat(melodyBuf, strNote);
            strcat(melodyBuf, "4");
            remain -= 240;
        }
        else if (remain > 60)//>= 90)           /* 120 = (1/8) * 960 */
        {                               /* 90 = (120 + 60) / 2 = 180 / 2 */
            strcat(melodyBuf, strOctave);
            strcat(melodyBuf, strNote);
            strcat(melodyBuf, "5");
            remain -= 120;
        }
        else
        {
        }
    }
    while (remain > 60);//>= 90);

    return remain;
}

static long UniteMultiHex(const unsigned char *multiHex, const int begin, const int end)
{
    int  i;
    long val = 0;

    for (i = begin; i <= end; i++)
    {
        val = val * 0x100 + multiHex[i];
    }
    return val;
}

static unsigned long ReadVarLen(MIDI_INFO *midiInfo)
{
    unsigned long value;
    unsigned char c;

    value = *midiInfo->midiData;
    midiInfo->midiData++;

    if (value & 0x80)
    {
        value &= 0x7f;
        do
        {
            c = *midiInfo->midiData;
            midiInfo->midiData++;

            value = (value << 7) + (c & 0x7f);
        }
        while (c & 0x80);
    }
    return value;
}
#endif
