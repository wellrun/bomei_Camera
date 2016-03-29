/**
 * @file fwl_gm.h
 * @brief This header file is for defining game framework layer API.
 * Game framework layer define the game APIs which related to OS, firmware, etc.
 * The following is the illustration of the relationship:
 *
 *                    1. platform presentation layer
 *                              |
 *                              V
 *               2. game application presentation layer
 *                        |     |        |     |
 *                        V     |        |     V
 *               3. game engine |        |  5. platform toolkit/control
 *                        |     |        |     |
 *                        V     V        |     |
 *               4. game framework layer |     |
 *                        |              |     |
 *                        V              V     V
 *                    6. platform framework layer
 *                              |
 *                              V
 *                    7. platform OS APIs, firmware drivers, etc.
 *
 * 1. platform presentation layer: for running game application, provided by platform
 * 2. game application presentation layer: include game menu, setting, record, help, etc;
 * 3. game engine(game playing interface): All the source code in this layer are independent of OS and firmware;
 * 4. game framework layer: define the game APIs which related to OS, firmware;
 * 5. platform toolkit/control: provided by platform.
 * 6. platform framework layer: this layer maybe do not exist.
 * 7. OS APIs, firmware drivers, etc.: provided by OS and firmware drivers.
 */

#ifndef __GAME_FWL_H__
#define __GAME_FWL_H__


#ifdef INSTALL_GAME
//#define INSTALL_GAME_PIGBOAT
//#define INSTALL_GAME_RECT

//#define INSTALL_GAME_EGG
//#define INSTALL_GAME_21
//#define INSTALL_GAME_7COLOR
//#define INSTALL_GAME_HERO
//#define INSTALL_GAME_BEAD
//#define INSTALL_GAME_3D
#endif

#define GM_DEFINE_IFILE_API

#ifdef OS_BREW
#else   /* not define OS_BREW */
    #include "Fwl_osFS.h"
    #include "Fwl_osMalloc.h"
    #include "Fwl_pfDisplay.h"
    #include "Eng_String.h"
    #include "Eng_Math.h"
    #include "Eng_Graph.h"
    #include "Eng_String_UC.h"
	#include "Fwl_osCom.h"
	

#endif  /* end of #ifdef OS_BREW */

#ifdef __cplusplus
extern "C" {
#endif

/*********************************** PUBLIC ***********************************/
#ifdef OS_BREW
#else   /* not define OS_BREW */
#endif  /* end of #ifdef OS_BREW */

#define     GAMEDATCNT      10
typedef unsigned long   GameData[GAMEDATCNT];

typedef struct _T_GAME_FWL  *T_GMFWL_HDL;   /* framework layer handle type */

/* game ID */
#define GM_ID_BOAT                      0
#define GM_ID_EGG                       1
#define GM_ID_RACE                      2
#define GM_ID_21                        3
#define GM_ID_BLOCK                     4
#define GM_ID_RECT                      5
#define GM_ID_HERO                      6
#define GM_ID_BEAD                      7
#define GM_ID_3D                        8

/*********************************** KEY ***********************************/
#define GMBOAT_KEY_UP                   1
#define GMBOAT_KEY_DOWN                 2
#define GMBOAT_KEY_LEFT                 3
#define GMBOAT_KEY_RIGHT                4
#define GMBOAT_KEY_OK                   5
#define GMBOAT_KEY_CANCEL               6
#define GMBOAT_KEY_OTHER                7

#define GMEGG_KEY_UP                    1
#define GMEGG_KEY_DOWN                  2
#define GMEGG_KEY_OK                    3
#define GMEGG_KEY_CANCEL                4
#define GMEGG_KEY_OTHER                 5

#define GMBLOCK_KEY_UP                  1
#define GMBLOCK_KEY_DOWN                2
#define GMBLOCK_KEY_LEFT                3
#define GMBLOCK_KEY_RIGHT               4
#define GMBLOCK_KEY_OK                  5
#define GMBLOCK_KEY_CANCEL              6
#define GMBLOCK_KEY_OTHER               7

#define GMRACE_KEY_UP                   1
#define GMRACE_KEY_DOWN                 2
#define GMRACE_KEY_LEFT                 3
#define GMRACE_KEY_RIGHT                4
#define GMRACE_KEY_OK                   5
#define GMRACE_KEY_CANCEL               6
#define GMRACE_KEY_OTHER                7

#define GM21_KEY_UP                     1
#define GM21_KEY_DOWN                   2
#define GM21_KEY_LEFT                   3
#define GM21_KEY_RIGHT                  4
#define GM21_KEY_OK                     5
#define GM21_KEY_CANCEL                 6
#define GM21_KEY_OTHER                  7

#define GMRECT_KEY_UP                   1
#define GMRECT_KEY_DOWN                 2
#define GMRECT_KEY_LEFT                 3
#define GMRECT_KEY_RIGHT                4
#define GMRECT_KEY_OK                   5
#define GMRECT_KEY_CANCEL               6
#define GMRECT_KEY_OTHER                7

#define GMHERO_KEY_UP                   1
#define GMHERO_KEY_DOWN                 2
#define GMHERO_KEY_LEFT                 3
#define GMHERO_KEY_RIGHT                4
#define GMHERO_KEY_OK                   5
#define GMHERO_KEY_CANCEL               6
#define GMHERO_KEY_OTHER                7

#define GMBEAD_KEY_UP                   1
#define GMBEAD_KEY_DOWN                 2
#define GMBEAD_KEY_LEFT                 3
#define GMBEAD_KEY_RIGHT                4
#define GMBEAD_KEY_OK                   5
#define GMBEAD_KEY_CANCEL               6
#define GMBEAD_KEY_OTHER                7

/*********************************** DISPLAY ***********************************/
#ifdef OS_BREW
#else   /* not define OS_BREW */
    typedef T_COLOR                     T_GM_COLOR; /* pixel color */
    #define T_GM_RECT                   T_RECT
#endif  /* end of #ifdef OS_BREW */

/* game screen size */
#define STD_GAME_WIDTH                  320
#define GM_SCREEN_WIDTH                 STD_GAME_WIDTH
#define GM_SCREEN_HEIGHT                240


T_BOOL  GmFwl_Init(T_GMFWL_HDL *phGmFwl, T_VOID *pApp);
T_VOID  GmFwl_Free(T_GMFWL_HDL *phGmFwl);
T_VOID GmFwlAudioResume(T_VOID);
T_U8 GmFwl_GetGameKey(T_U8 gameID, T_U32 userKey);
T_VOID  GmFwl_GetWinRect(T_GMFWL_HDL hGmFwl, T_GM_RECT *winRect);
T_LEN   GmFwl_GetTextHeight(T_GMFWL_HDL hGmFwl);
T_LEN   GmFwl_GetTextWidth(T_GMFWL_HDL hGmFwl, const T_U16 *text, T_U16 textLen);
T_VOID  GmFwl_CleanWinBackground(T_GMFWL_HDL hGmFwl);
T_VOID  GmFwl_VDspCleanRect(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect, T_COLOR color);
T_VOID  GmFwl_VDspDrawImage(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect, T_pCDATA imageData, T_POS sourX, T_POS sourY, T_pCDATA trans);
T_VOID  GmFwl_DisplayVDsp(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect);
T_VOID  GmFwl_DisplayImage(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect, T_pCDATA imageData, T_POS sourX, T_POS sourY);
T_VOID  GmFwl_DrawImage(HLAYER hLayer, T_POS x, T_POS y, T_pCDATA BmpString, T_COLOR *bkColor, T_BOOL Reverse);
T_BOOL  GmFwl_DisplayBitmap(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect, T_pCDATA bitmapData, T_POS sourX, T_POS sourY, T_U32 bitmapDataSize);
T_VOID  GmFwl_DisplayText(DISPLAY_TYPE_DEV LCD, T_POS destX, T_POS destY, const T_U8 *text);
T_VOID  GmFwl_InvalidateRect(T_GMFWL_HDL hGmFwl, const T_GM_RECT *destRect);

/*********************************** SOUND ***********************************/
T_S8    GmFwl_GetVolume(T_GMFWL_HDL hGmFwl);
T_BOOL  GmFwl_SetVolume(T_GMFWL_HDL hGmFwl, T_S8 volume);
T_BOOL  GmFwl_SoundPlay(T_GMFWL_HDL hGmFwl, const T_pDATA soundData, T_U8 loop, T_U32 size);
T_VOID  GmFwl_SoundStop(T_GMFWL_HDL hGmFwl);
T_VOID  GmFwl_KeyToneAlwaysOff(T_GMFWL_HDL hGmFwl);
T_VOID  GmFwl_KeyToneResume(T_GMFWL_HDL hGmFwl);

/*********************************** VIBRATOR ***********************************/
T_VOID  GmFwl_VibraPlay(T_GMFWL_HDL hGmFwl);
T_VOID  GmFwl_VibraStop(T_GMFWL_HDL hGmFwl);

/*********************************** BACKLIGHT ***********************************/
T_VOID GmFwl_BacklightAlwaysOn(T_GMFWL_HDL hGmFwl);
T_VOID GmFwl_BacklightResume(T_GMFWL_HDL hGmFwl);

/*********************************** TIMER ***********************************/
#ifdef OS_BREW
#else   /* not define OS_BREW */
    #define T_GM_TIMER                  T_TIMER
    #define GM_ERROR_TIMER              ERROR_TIMER
#endif  /* end of #ifdef OS_BREW */

T_GM_TIMER  GmFwl_SetTimer(T_GMFWL_HDL hGmFwl, T_U32 milliSeconds, T_U8 gameID);
T_VOID  GmFwl_StopTimer(T_GMFWL_HDL hGmFwl, T_GM_TIMER timerHandle);

/*********************************** STRING AND MEMORY ***********************************/
#ifdef OS_BREW
#else   /* not define OS_BREW */
    #define MALLOC(size)                Fwl_Malloc(size)
    #define FREE(x)                     Fwl_Free(x)
    #define STRLEN(s)                   Utl_StrLen(s)
    #define STRCPY(s1, s2)              Utl_StrCpy(s1, s2)
    #define STRCMP(s1, s2)              Utl_StrCmp(s1, s2)
    #define MEMSET(p, c, s)             Utl_MemSet(p, c, s)
    #define MEMCPY(p, c, s)             Utl_MemCpy(p, c, s)
    #define ITOA(i, s, f)               Utl_Itoa(i, s, f)
#endif  /* end of #ifdef OS_BREW */

/*********************************** FILE SYSTEM ***********************************/
// GM_PROGRESS_SAVE_TO_FILE: save progress data to file system
// GM_PROGRESS_SAVE_TO_BUFF: save progress data to buffer. Data will be lost after switch off
// GM_PROGRESS_NOT_SAVE: do not save progress data
#define GM_PROGRESS_SAVE_TO_FILE

#define PAUSE_NOT_MOVE

#define GM_FILENAME_LEN                 32

#ifdef GM_PROGRESS_SAVE_TO_FILE
    #ifdef OS_ANYKA
		#define	GMBOAT_PROGRESS_FILE	DRI_B"system/gmprogbt"
		#define	GMEGG_PROGRESS_FILE		DRI_B"system/gmprogeg"
		#define	GMBLOCK_PROGRESS_FILE	DRI_B"system/gmprog7c"
		#define	GM21_PROGRESS_FILE		DRI_B"system/gmprog21"
		#define	GMRACE_PROGRESS_FILE	DRI_B"system/gmprograce"
		#define	GMRECT_PROGRESS_FILE	DRI_B"system/gmprogrect"
		#define	GMHERO_PROGRESS_FILE	DRI_B"system/gmproghero"
		#define	GMBEAD_PROGRESS_FILE	DRI_B"system/gmprogbead"
    #endif
    #ifdef OS_WIN32
        #define GMBOAT_PROGRESS_FILE    "system/gmprogbt"
        #define GMEGG_PROGRESS_FILE     "system/gmprogeg"
        #define GMBLOCK_PROGRESS_FILE   "system/gmprog7c"
        #define GM21_PROGRESS_FILE      "system/gmprog21"
        #define GMRACE_PROGRESS_FILE    "system/gmprograce"
        #define GMRECT_PROGRESS_FILE    "system/gmprogrect"
        #define GMHERO_PROGRESS_FILE    "system/gmproghero"
        #define GMBEAD_PROGRESS_FILE    "system/gmprogbead"
    #endif
#endif  /* end of #ifdef GM_PROGRESS_SAVE_TO_FILE */

#ifdef OS_ANYKA
	#define GMBOAT_RECORD_FILE_NAME		DRI_B"system/gmrecbt"
	#define GMEGG_RECORD_FILE_NAME		DRI_B"system/gmreceg"
	#define	GMBLOCK_RECORD_FILE_NAME	DRI_B"system/gmprec7c"
	#define	GM21_RECORD_FILE_NAME		DRI_B"system/gmprec21"
	#define	GMRACE_RECORD_FILE_NAME	    DRI_B"system/gmprecrace"
	#define	GMRECT_RECORD_FILE_NAME	    DRI_B"system/gmprecrect"
	#define	GMHERO_RECORD_FILE_NAME	    DRI_B"system/gmprechero"
	#define	GMBEAD_RECORD_FILE_NAME		DRI_B"system/gmprecbead"
#endif  /* end of #ifdef OS_BREW */
#ifdef OS_WIN32
    #define GMBOAT_RECORD_FILE_NAME     "system/gmrecbt"
    #define GMEGG_RECORD_FILE_NAME      "system/gmreceg"
    #define GMBLOCK_RECORD_FILE_NAME    "system/gmrec7c"
    #define GM21_RECORD_FILE_NAME       "system/gmrec21"
    #define GMRACE_RECORD_FILE_NAME     "system/gmrecrace"
    #define GMRECT_RECORD_FILE_NAME     "system/gmrecrect"
    #define GMHERO_RECORD_FILE_NAME     "system/gmrechero"
    #define GMBEAD_RECORD_FILE_NAME     "system/gmrecbead"
#endif

#ifdef OS_ANYKA
    #define GM_FILE_READ                _FMODE_READ     /**< Open for read only */
    #define GM_FILE_READWRITE           _FMODE_WRITE    /**< Open for read and write */
    #define GM_FILE_CREATE              _FMODE_CREATE       /**< Create the file if it does not exist. */
    #define GM_FILE_TRUNCATE            _FMODE_CREATE       /**< Truncate the file if it already exists. */

    #define T_pGM_FILE                  T_pFILE         /* FILE */
    #define T_GM_FILE_MODE              T_FILE_MODE
    #define GM_FOPEN_FAIL               _FOPEN_FAIL
    #define GM_FSEEK_CURRENT            _FSEEK_CURRENT
    #define GM_FSEEK_END                _FSEEK_END
    #define GM_FSEEK_SET                _FSEEK_SET
#else   /* not define OS_BREW */
    #define GM_FILE_READ                _FMODE_READ     /**< Open for read only */
    #define GM_FILE_READWRITE           _FMODE_WRITE    /**< Open for read and write */
    #define GM_FILE_CREATE              _FMODE_CREATE   /**< Create the file if it does not exist. */
    #define GM_FILE_TRUNCATE            _FMODE_CREATE   /**< Truncate the file if it already exists. */

    #define T_pGM_FILE                  T_pFILE         /* FILE */
    #define T_GM_FILE_MODE              T_FILE_MODE
    #define GM_FOPEN_FAIL               _FOPEN_FAIL
    #define GM_FSEEK_CURRENT            _FSEEK_CURRENT
    #define GM_FSEEK_END                _FSEEK_END
    #define GM_FSEEK_SET                _FSEEK_SET
#endif  /* end of #ifdef OS_BREW */

T_pGM_FILE  GmFwl_FileOpen(T_GMFWL_HDL hGmFwl, const T_pSTR path, T_GM_FILE_MODE mode);
T_U32   GmFwl_FileRead(T_pGM_FILE pFile, T_pDATA buffer, T_U32 count);
T_U32   GmFwl_FileWrite(T_pGM_FILE pFile, T_pCDATA buffer, T_U32 count);
T_BOOL  GmFwl_FileClose(T_pGM_FILE pFile);
T_S32   GmFwl_FileSeek(T_pGM_FILE pFile, T_U32 offset, T_U16 origin);
T_BOOL  GmFwl_FileDelete(T_GMFWL_HDL hGmFwl, const T_pWSTR path);

/* read/save game record */
T_U16   GmFwl_ReadRecord(T_GMFWL_HDL hGmFwl, T_U8 gameID);
T_BOOL  GmFwl_SaveRecord(T_GMFWL_HDL hGmFwl, T_U8 gameID, T_U16 record);
T_BOOL  GmFwl_GameOverScore(T_GMFWL_HDL hGmFwl, T_U8 gameID, T_U16 score);

T_U16 GmScore_GetDataSize(T_VOID);
//T_U16 Gm21Eng_GetDataSize(T_VOID);
T_U16 GmBlockEng_GetDataSize(T_VOID);
T_U16 GmBtEng_GetDataSize(T_VOID);
T_U16 GmEgEng_GetDataSize(T_VOID);
T_U16 GmRacEng_GetDataSize(T_VOID);
T_U16 GmRectEng_GetDataSize(T_VOID);
T_U16 GmHeroEng_GetDataSize(T_VOID);
T_U16 GmBeadEng_GetDataSize(T_VOID);

/*********************************** IFILE ***********************************/
#ifdef GM_DEFINE_IFILE_API
    #define IFILE_FROM_FILE_PTR         0
    #define IFILE_FROM_FILE_NAME        1
    #define IFILE_FROM_BUFF             2
    #define IFILE_FROM_UNKNOWN          0xff

    #define IFILE_SEEK_SET              GM_FSEEK_SET
    #define IFILE_SEEK_CURRENT          GM_FSEEK_CURRENT
    #define IFILE_SEEK_END              GM_FSEEK_END
    #define IFILE_READ(pFile, buffer, count)    GmFwl_FileRead(pFile, buffer, count)
    #define IFILE_WRITE(pFile, buffer, count)   GmFwl_FileWrite(pFile, buffer, count)
    #define IFILE_SEEK(pFile, offset, origin)   GmFwl_FileSeek(pFile, offset, origin)

    typedef union {
        T_pGM_FILE  pFile;
        T_pSTR      filename;   /* read only */
        T_pDATA     pData;      /* read only */
    } T_IFILE_FD;               /* from data */

    typedef struct {
        T_IFILE_FD  fd;         /* from data */
        T_U8        from;       /* from: IFILE_FROM_FILE_PTR, IFILE_FROM_FILE_NAME or IFILE_FROM_BUFF */
        T_U32       curOffset;  /* only for IFILE_FROM_BUFF */
        T_U32       maxLength;  /* only for IFILE_FROM_BUFF */
    } T_IFILE;

    T_VOID  IFile_Init(T_IFILE *piFile, T_U32 maxlen);
    T_U32   IFile_Read(T_IFILE *piFile, T_pDATA buffer, T_U32 count);
    T_U32   IFile_ReadChr(T_IFILE *piFile, T_U8 *chr);
    T_pDATA IFile_ReadAddr(T_IFILE *piFile, T_U32 count);
    T_U32   IFile_Write(T_IFILE *piFile, T_pCDATA buffer, T_U32 count);
    T_U32   IFile_WriteChr(T_IFILE *piFile, T_U8 chr);
    T_S32   IFile_Seek(T_IFILE *piFile, T_U16 origin, T_U32 offset);
    T_U32   IFile_ReadUnit(T_IFILE *piFile, T_U8 bytes);
    T_U32   IFile_ReadUnitR(T_IFILE *piFile, T_U8 bytes);
    T_BOOL  IFile_ReadMultiU16(T_IFILE *piFile, T_U16 *retData, T_U16 count);
#endif

/*********************************** OTHERS ***********************************/
#define GM_MAX(x, y)                    (x > y ? x : y)
#define GM_MIN(x, y)                    (x < y ? x : y)

#ifdef OS_BREW
#else
//  #define GmFwl_InitRand()            Fwl_RandSeed()//Fwl_InitRand()
    #define GmFwl_GetRand(maxval)       Fwl_GetRand(maxval)
    #define GmFwl_SetRect(rect, left, top, width, height)   RectInit(rect, left, top, width, height)
#endif  /* end of #ifdef OS_BREW */

#ifdef __cplusplus
}
#endif

#define GmFwl_double_Img_BitBlt Fwl_Scale_Convert
/*
T_VOID GmFwl_double_Img_BitBlt(T_VOID *dstBuf, T_U16 scaleWidth, T_U16 scaleHeight,
					 T_U16 dstPosX, T_U16 dstPosY, T_U16 dstWidth,
					 T_VOID *srcBuf, T_U16 srcWidth, T_U16 srcHeight, T_U8 srcFormat);
*/
T_VOID GmFwl_double_Img_BitBlt_Free(T_VOID);


#endif  /* __GAME_FWL_H__ */
