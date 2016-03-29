#ifndef __THUMBS_DB_H__
#define __THUMBS_DB_H__

#include "anyka_types.h"
//#include "vfs.h"
//#include "tstring_api.h"
//#include "string_api.h"
//#include "memop_api.h"
//#include "eng_image.h"
#include "lib_image_api.h"

////////////////////////////////////////////////////////////////////////////////
// Macro Definition
////////////////////////////////////////////////////////////////////////////////

#define LCD_WIDTH              MAIN_LCD_WIDTH// 240
#define LCD_HEIGHT             MAIN_LCD_HEIGHT  // 320
#define PICTURE_BOX_WIDTH       240
#define PICTURE_BOX_HEIGHT      246

#if (LCD_CONFIG_WIDTH==800)
#define THUMBNAIL_WIDTH         160
#define THUMBNAIL_HEIGHT        120
#elif (LCD_CONFIG_WIDTH==480)
#define THUMBNAIL_WIDTH         100
#define THUMBNAIL_HEIGHT        60
#elif (LCD_CONFIG_WIDTH==320)
#define THUMBNAIL_WIDTH         60
#define THUMBNAIL_HEIGHT        60
#endif

#define THUMBNAIL_INTERVAL      6

#define TDB_FILENAME_MAXLEN 	MAX_FILENM_LEN


//#define     THUMB_Debug                 1


////////////////////////////////////////////////////////////////////////////////
// Structure Definition
////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    T_IMAGE_TYPE  type;
    T_U8         *data;
    T_S32         size;
    T_U16         width;
    T_U16         height;
} TDB_ImgInfo;

typedef struct
{
    T_U8    Tag[8];
    T_U32   HeaderSize;
    T_U16   MaxWidth;
    T_U16   MaxHeight;
    T_U32   Count;
    T_S32   FreeHead;
    T_U32   FreeItems;
    T_U32   HashTableCount;
    T_U32   HashTable[1024];
} ThumbsDB_Header;

typedef struct
{
    T_S32   NextItem;
    T_WCHR  Filename[TDB_FILENAME_MAXLEN+1];
    T_U32   nFlag;
    T_U16   Width;
    T_U16   Height;
    T_U32   DataSize;
    T_U8   *Data;
} ThumbsDB_Item;

typedef T_U32 THUMBSDB_HANDLE;

#define THUMBSDB_INVALID_HANDLE 0


#ifdef OS_WIN32
#pragma pack(1)
#endif

#ifdef OS_ANYKA
typedef __packed struct tagBITMAPFILEHEADER
#else 
typedef  struct tagBITMAPFILEHEADER
#endif
{
    T_U16   bfType;
    T_U32   bfSize;
    T_U16   bfReserved1;
    T_U16   bfReserved2;
    T_U32   bfOffBits;
} BITMAPFILEHEADER;


#ifdef OS_ANYKA
typedef __packed struct tagBITMAPINFOHEADER
#else 
typedef  struct tagBITMAPINFOHEADER
#endif
{
    T_U32   biSize;
    T_S32   biWidth;
    T_S32   biHeight;
    T_U16   biPlanes;
    T_U16   biBitCount;
    T_U32   biCompression;
    T_U32   biSizeImage;
    T_S32   biXPelsPerMeter;
    T_S32   biYPelsPerMeter;
    T_U32   biClrUsed;
    T_U32   biClrImportant;
} BITMAPINFOHEADER;

#ifdef OS_WIN32
#pragma pack()
#endif


////////////////////////////////////////////////////////////////////////////////
// Callback Function Definition
////////////////////////////////////////////////////////////////////////////////

typedef T_S32           T_hFILE;    // file handle

typedef T_VOID* (*THUMBSDB_CBFUNC_MALLOC)
(T_U32 size, T_pSTR filename, T_U32 line);

typedef T_VOID* (*THUMBSDB_CBFUNC_FREE)
(T_VOID* mem, T_pSTR filename, T_U32 line);

typedef T_hFILE (*THUMBSDB_CBFUNC_FOPEN)
(T_pCWSTR path, T_U32 flag, T_U32 attr);

typedef T_BOOL (*THUMBSDB_CBFUNC_FCLOSE)
(T_hFILE hFile);

typedef T_U32  (*THUMBSDB_CBFUNC_FREAD)
(T_hFILE hFile, T_pVOID buffer, T_U32 count);

typedef T_U32  (*THUMBSDB_CBFUNC_FWRITE)
(T_hFILE hFile, T_pVOID buffer, T_U32 count);

typedef T_S32  (*THUMBSDB_CBFUNC_FSEEK)
(T_hFILE hFile, T_S32 offset, T_U16 origin);

typedef T_U32  (*THUMBSDB_CBFUNC_FTELL)
(T_hFILE hFile);

typedef T_VOID (*THUMBSDB_CBFUNC_PRINTF)
(T_pCSTR format, ...);


typedef struct
{
    THUMBSDB_CBFUNC_MALLOC  malloc;
    THUMBSDB_CBFUNC_FREE    free;
    THUMBSDB_CBFUNC_FOPEN   fopen;
    THUMBSDB_CBFUNC_FCLOSE  fclose;
    THUMBSDB_CBFUNC_FREAD   fread;
    THUMBSDB_CBFUNC_FWRITE  fwrite;
    THUMBSDB_CBFUNC_FSEEK   fseek;
    THUMBSDB_CBFUNC_FTELL   ftell;
    THUMBSDB_CBFUNC_PRINTF  printf;
} ThumbsDB_CBFuns;


////////////////////////////////////////////////////////////////////////////////
// ThumbsDB API
////////////////////////////////////////////////////////////////////////////////

T_BOOL ThumbsDB_SetCallbackFuns(const ThumbsDB_CBFuns *pCBFuns);

THUMBSDB_HANDLE ThumbsDB_CreateHandle(T_pCWSTR filename);

T_VOID ThumbsDB_DestroyHandle(THUMBSDB_HANDLE hThumbsDB);

T_BOOL ThumbsDB_DisplayThumbs(THUMBSDB_HANDLE hThumbsDB, T_U8* lcdBuffer,
                              T_pCWSTR fileArray[], T_U16 hNum, T_U16 vNum,
                              T_U16 yOffset);

T_U8* ThumbsDB_GetData(THUMBSDB_HANDLE hThumbsDB, T_pCWSTR fileName);

T_BOOL ThumbsDB_InsertImage(THUMBSDB_HANDLE hThumbsDB, T_pCWSTR filename);

T_BOOL ThumbsDB_DeleteImage(THUMBSDB_HANDLE hThumbsDB, T_pCWSTR filename);

T_BOOL ThumbsDB_DeleteAllImages(THUMBSDB_HANDLE hThumbsDB);

T_BOOL ThumbsDB_RenameImage(THUMBSDB_HANDLE hThumbsDB,
                            T_pCWSTR oldFilename, T_pCWSTR newFilename);

T_U8*  ThumbsDB_GetImage(THUMBSDB_HANDLE hThumbsDB, T_pCWSTR filename,
                         T_U32 *size, T_U16 *width, T_U16 *height);

T_BOOL ThumbsDB_Clean(THUMBSDB_HANDLE hThumbsDB,
                      T_pCWSTR fileArray[], T_U16 fileCount);

T_BOOL ThumbsDB_IsImage(THUMBSDB_HANDLE hThumbsDB, T_pCWSTR filename);

#endif //__THUMBS_DB_H__
