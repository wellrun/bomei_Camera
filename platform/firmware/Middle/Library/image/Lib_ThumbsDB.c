/**
* @FILENAME Lib_ThumbsDB.c
* @BRIEF Thumbs Database Management
* Copyright (C) 2008 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-21
*/
#include "Fwl_Image.h"

#ifdef SUPPORT_IMG_BROWSE

#include "Lib_thumbsdb.h"
#ifdef OS_ANYKA
#include "akos_api.h"
#endif
//#include "eng_jpeg2bmp.h"
#include "Eng_String.h"

#include "Fwl_ImageLib.h"
#include "eng_debug.h"
#include "eng_math.h"
#include "Ctl_ImgBrowse.h"


////////////////////////////////////////////////////////////////////////////////

// Callback Functions
ThumbsDB_CBFuns         g_TDBCBFuns;

#define TDB_Malloc(L)   g_TDBCBFuns.malloc(L, __FILE__, __LINE__)
#define TDB_Free(L)     g_TDBCBFuns.free(L, __FILE__, __LINE__)
#define TDB_Open        g_TDBCBFuns.fopen
#define TDB_Close       g_TDBCBFuns.fclose
#define TDB_Read        g_TDBCBFuns.fread
#define TDB_Write       g_TDBCBFuns.fwrite
#define TDB_Seek        g_TDBCBFuns.fseek
#define TDB_Tell        g_TDBCBFuns.ftell
#define TDB_Printf      g_TDBCBFuns.printf

#define Utl_MemCpy32(strDest, strSour, count)      memcpy((strDest), (strSour), (count))    //?
#define Utl_MemSet32(strDest, chr,count)           Utl_MemSet((strDest), (chr),(count)) //?
#define Utl_TStrCpy                                  Utl_UStrCpy    //ok
#define Utl_TStrLen             Utl_UStrLen     //ok
#define Utl_TStrCmp			 Utl_UStrCmp    // Utl_StrCmp      correction
#define Utl_MemCmp32           Utl_MemCmp   //?


#define OFFSETOF(s, m)  ((T_U32)&(((s*)0)->m))

// ThumbsDB
#if (SDRAM_MODE >= 16)
#define TDB_CACHE_SIZE          64      // 64 ThumbBox Item
#else
#define TDB_CACHE_SIZE          24      // 24 ThumbBox Item
#endif

#define TDB_BKG_COLOR           0x32    // Pure White Color

#define TDB_THUMBSIZE_565       THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT * 2  // RGB565
#define TDB_THUMBSIZE_888       THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT * 3  // RGB888

// Thumbnail Format
#define TDB_RGB_565             0x10uL
#define TDB_RGB_888             0x20uL

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
#define TDB_THUMBBOX_SIZE       TDB_THUMBSIZE_565
#define TDB_COLOR_SIZE			2			// LCD buff color size
#define TDB_RGB_MODE			TDB_RGB_565
#define TDB_RGB_MODE_REVERSE	TDB_RGB_888
#else
#define TDB_THUMBBOX_SIZE       TDB_THUMBSIZE_888
#define TDB_COLOR_SIZE			3			// LCD buff color size
#define TDB_RGB_MODE			TDB_RGB_888
#define TDB_RGB_MODE_REVERSE	TDB_RGB_565
#endif

// Offsets
#define TDB_COUNT_OFFSET        OFFSETOF(ThumbsDB_Header, Count)
#define TDB_FREEHEAD_OFFSET     OFFSETOF(ThumbsDB_Header, FreeHead)
#define TDB_HASHTBL_OFFSET      OFFSETOF(ThumbsDB_Header, HashTable)
#define TDB_ITEM_FLAG_OFFSET    OFFSETOF(ThumbsDB_Item, nFlag)
#define TDB_ITEM_PARAMSIZE      OFFSETOF(ThumbsDB_Item, Data)

// File Status
#define TDB_FS_CLOSE            0
#define TDB_FS_OPEN_READ        1
#define TDB_FS_OPEN_WRITE       2

// Cache
#define TDB_FLUSH_ITEMS         15
#define TDB_CACHE_INVALID_IDX   -1
#define TDB_CACHE_SHOW_BIT      0x01uL  // Show on Screen
#define TDB_CACHE_MEM_BIT       0x02uL  // Only in Memory



// Bitmap Format
#define IMG_BMP_1BIT            1
#define IMG_BMP_4BIT            2
#define IMG_BMP_8BIT            3
#define IMG_BMP_16BIT_555       4
#define IMG_BMP_16BIT_565       5
#define IMG_BMP_24BIT           6
#define IMG_BMP_32BIT           7

// Constants for the biCompression Field
#define BI_RGB                  0uL
#define BI_RLE8                 1uL
#define BI_RLE4                 2uL
#define BI_BITFIELDS            3uL

// Fixed Point
#define Fixed T_S32
#define PRESICION               16
#define ONE                     (1<<PRESICION)
#define HALF                    (ONE>>1)
#define ZERO                    0
#define IntToFixed(a)           (Fixed) ((a)<<PRESICION)
#define FixedToInt(a)           (T_S32) ((a)>>PRESICION)
#define ROUND(a)                FixedToInt((a)+HALF)

////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    T_U8       *data;
    T_U32       nTime;
    T_U32       nFlag;
    T_WCHR      filename[TDB_FILENAME_MAXLEN+1];
} TDB_CacheItem;

typedef struct
{
    T_S32               fp;
    T_U32               fileStatus;
    T_WCHR              filename[TDB_FILENAME_MAXLEN+1];
    ThumbsDB_Header    *header;
    T_U32               cacheTimer;
    T_U32               cacheMemCount;  // Item Only in Memory, Not in File
    TDB_CacheItem       cache[TDB_CACHE_SIZE];
} ThumbsDB_Handle;

#if 0
typedef struct
{
    T_IMAGE_TYPE  type;
    T_U8         *data;
    T_S32         size;
    T_U16         width;
    T_U16         height;
} TDB_ImgInfo;
#endif /* 0 */
typedef struct
{
    T_U16   bmpFormat;
    T_U16   bitCount;
    T_U32   width;
    T_U32   height;
    T_U32   lineBytes;
    T_U8   *bmpPalette;
    T_U8   *bmpData;
    T_BOOL  bTopdown;
} TDB_BmpInfo;

////////////////////////////////////////////////////////////////////////////////

// Search Functions
static T_U32  HashString(const T_U8* str);
static T_S32  FindItem(THUMBSDB_HANDLE hThumbsDB, T_S32 fp, ThumbsDB_Item* pTDB_Item, T_pCWSTR filename,
                       T_S32* prevPos, T_S32* curPos);
static T_S32  GetNewItemPos(T_S32 fp, ThumbsDB_Header* pTDB_Header);

// Create Thumbnail
static T_U8*  CreateThumbnail(T_pCWSTR filename, T_U32 *size,
                              T_U16 *width, T_U16 *height, T_U32 format);
static T_BOOL BmpInfo(const T_U8* bmpBuf, T_U16* width, T_U16* height);
static T_BOOL GetImgInfo(TDB_ImgInfo* pImgInfo);
static T_U8*  DecImage(TDB_ImgInfo* pImgInfo, T_U32* bmpSize, Fixed* factor,
                       T_U16* dstW, T_U16* dstH, T_hGIFENUM* phGif);
static T_U8*  Bmp2Thumbnail(T_U8* bmpData, Fixed factor,
                            T_U16 dstW, T_U16 dstH, T_U32 format);
static T_BOOL BmpInit(T_U8* bmpData, TDB_BmpInfo* pBmpInfo);

// Thumbnail Box Cache
static T_S32  GetThumbnailBox(THUMBSDB_HANDLE hThumbsDB, ThumbsDB_Handle* pTDB_Handle, T_pCWSTR filename);
static T_S32  GetThumbBoxFromCache(ThumbsDB_Handle* pTDB_Handle, T_pCWSTR filename);
static T_S32  UpdateThumbBoxCache(ThumbsDB_Handle* pTDB_Handle, T_pCWSTR filename, T_U8* data);
static T_BOOL FlushThumbBoxCache(ThumbsDB_Handle* pTDB_Handle);
static T_VOID PrepareThumbItem(TDB_CacheItem* pCacheItem, ThumbsDB_Item* pTDB_Item,
                               T_U8* buf, T_U32 nextPos);
static T_U8*  TDB_GetImage(THUMBSDB_HANDLE hThumbsDB, ThumbsDB_Handle *pTDB_Handle, T_pCWSTR filename, T_U32* format);

////////////////////////////////////////////////////////////////////////////////

/**
* @BRIEF initial callback functions
* @PARAM pCBFuns: pointer of callback functions structure
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-09-23
*/
T_BOOL ThumbsDB_SetCallbackFuns(const ThumbsDB_CBFuns *pCBFuns)
{
    // Check Parameters
    if (AK_NULL == pCBFuns
     || AK_NULL == pCBFuns->malloc || AK_NULL == pCBFuns->free
     || AK_NULL == pCBFuns->fopen || AK_NULL == pCBFuns->fclose
     || AK_NULL == pCBFuns->fread || AK_NULL == pCBFuns->fwrite
     || AK_NULL == pCBFuns->fseek || AK_NULL == pCBFuns->ftell)
    {
        return AK_FALSE;
    }

    g_TDBCBFuns = *pCBFuns;
    return AK_TRUE;
}


/**
* @BRIEF create ThumbsDB handle
* @PARAM filename: ThumbsDB filename(Unicode)
* @RETURN ThumbsDB Handle, THUMBSDB_INVALID_HANDLE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-14
*/
THUMBSDB_HANDLE ThumbsDB_CreateHandle(T_pCWSTR filename)
{
    ThumbsDB_Handle *pTDB_Handle = AK_NULL;

#ifdef     THUMB_Debug
    Fwl_Print(C3, M_THUMB, "ThumbsDB_CreateHandle");
#endif

    // Check Parameters
    if (AK_NULL == filename || Utl_TStrLen(filename) > TDB_FILENAME_MAXLEN)
    {
        return (THUMBSDB_HANDLE)THUMBSDB_INVALID_HANDLE;
    }

    // Create ThumbsDB Handle
    pTDB_Handle = (ThumbsDB_Handle*)TDB_Malloc(sizeof(ThumbsDB_Handle)
                + sizeof(ThumbsDB_Header) + sizeof(ThumbsDB_Item));
    if (AK_NULL == pTDB_Handle)
    {
        return (THUMBSDB_HANDLE)THUMBSDB_INVALID_HANDLE;
    }

#ifdef     THUMB_Debug
    //Fwl_Print(C3, M_THUMB, "ThumbsDB_CreateHandle2");
#endif

    Utl_MemSet32(pTDB_Handle, 0, sizeof(ThumbsDB_Handle)
        + sizeof(ThumbsDB_Header) + sizeof(ThumbsDB_Item));
    pTDB_Handle->header = (ThumbsDB_Header*)((T_U8*)pTDB_Handle
                        + sizeof(ThumbsDB_Handle));
    Utl_TStrCpy(pTDB_Handle->filename, filename);


    // Read ThumbsDB Header
    pTDB_Handle->fp = TDB_Open(filename, _FMODE_READ, _FMODE_READ);

#ifdef     THUMB_Debug
    //    Fwl_Print(C3, M_THUMB, "ThumbsDB_CreateHandle4, pTDB_Handle->fp=%d",pTDB_Handle->fp);
#endif


    if (FS_INVALID_HANDLE == pTDB_Handle->fp)   // ThumbsDB File Not Exist!
    {
        // Create ThumbsDB File & Initialize ThumbsDB Header
        pTDB_Handle->fileStatus = TDB_FS_CLOSE;
        if (!ThumbsDB_DeleteAllImages((THUMBSDB_HANDLE)pTDB_Handle))
        {
			pTDB_Handle->fileStatus = TDB_FS_OPEN_READ;
			Utl_MemCpy32(pTDB_Handle->header->Tag, "AKTHUMBS", 8);            
			pTDB_Handle->header->HeaderSize = sizeof(ThumbsDB_Header);
			pTDB_Handle->header->MaxWidth   = THUMBNAIL_WIDTH;
			pTDB_Handle->header->MaxHeight  = THUMBNAIL_HEIGHT;
			pTDB_Handle->header->Count      = 0;
			pTDB_Handle->header->FreeHead   = 0;
			pTDB_Handle->header->FreeItems  = 0;
			pTDB_Handle->header->HashTableCount = 1024;

			//     ThumbsDB_DestroyHandle((THUMBSDB_HANDLE)pTDB_Handle);
			return (THUMBSDB_HANDLE)pTDB_Handle;
        }
        pTDB_Handle->fp = TDB_Open(filename, _FMODE_READ, _FMODE_READ);
        if (FS_INVALID_HANDLE == pTDB_Handle->fp)
        {
            pTDB_Handle->fileStatus = TDB_FS_CLOSE;
            ThumbsDB_DestroyHandle((THUMBSDB_HANDLE)pTDB_Handle);
            return (THUMBSDB_HANDLE)THUMBSDB_INVALID_HANDLE;
        }
    }
    else                                        // ThumbsDB File Exist
    {
        // Read ThumbsDB Header
        TDB_Read(pTDB_Handle->fp, pTDB_Handle->header, sizeof(ThumbsDB_Header));

#ifdef     THUMB_Debug
                Fwl_Print(C3, M_THUMB, "ThumbsDB_CreateHandle5");
#endif

        // Check Validity
        if (Utl_MemCmp32(pTDB_Handle->header->Tag, "AKTHUMBS", 8) != 0
         || pTDB_Handle->header->HeaderSize != sizeof(ThumbsDB_Header)
         || pTDB_Handle->header->MaxWidth  != THUMBNAIL_WIDTH
         || pTDB_Handle->header->MaxHeight != THUMBNAIL_HEIGHT
         || pTDB_Handle->header->HashTableCount != 1024)
        {
			if( ThumbsDB_DeleteAllImages((THUMBSDB_HANDLE)pTDB_Handle) )
			{
				pTDB_Handle->fp = TDB_Open(filename, _FMODE_READ, _FMODE_READ);
			}

            //ThumbsDB_DestroyHandle((THUMBSDB_HANDLE)pTDB_Handle);
            //return (THUMBSDB_HANDLE)THUMBSDB_INVALID_HANDLE;
        }
    }

    // !NOTE: KEEP FILE OPEN (READ ONLY)
    pTDB_Handle->fileStatus = TDB_FS_OPEN_READ;
    return (THUMBSDB_HANDLE)pTDB_Handle;
}


/**
* @BRIEF destroy ThumbsDB handle
* @PARAM hThumbsDB: ThumbsDB handle
* @RETURN none
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-17
*/
T_VOID ThumbsDB_DestroyHandle(THUMBSDB_HANDLE hThumbsDB)
{
    ThumbsDB_Handle *pTDB_Handle = (ThumbsDB_Handle *)hThumbsDB;
    T_S32   i;

    if (AK_NULL != pTDB_Handle)
    {
        // Flush Cached Thumbnail(Only in Memory) to ThumbsDB File
        FlushThumbBoxCache(pTDB_Handle);

        // Free ThumbsDB Cache
        for (i = 0; i < TDB_CACHE_SIZE; ++i)
        {
            if (AK_NULL != pTDB_Handle->cache[i].data)
            {
                TDB_Free(pTDB_Handle->cache[i].data);
                pTDB_Handle->cache[i].data = AK_NULL;
            }
        }

        // Close ThumbsDB File
        if (TDB_FS_CLOSE != pTDB_Handle->fileStatus)
        {
            TDB_Close(pTDB_Handle->fp);
        }

        // Free ThumbsDB Handle
        TDB_Free(pTDB_Handle);
    }

    return;
}


/**
* @BRIEF direct display thumbnails on LCD screen
* @PARAM hThumbsDB: ThumbsDB handle
* @PARAM lcdBuffer: lcdBuffer
* @PARAM fileArray: image file list
* @PARAM hNum:      number of thumbnail in horizontal
* @PARAM vNum:      number of thumbnail in vertical
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-17
*/
T_BOOL ThumbsDB_DisplayThumbs(THUMBSDB_HANDLE hThumbsDB, T_U8* lcdBuffer,
                              T_pCWSTR fileArray[], T_U16 hNum, T_U16 vNum,
                              T_U16 yOffset)
{
    ThumbsDB_Handle *pTDB_Handle = (ThumbsDB_Handle *)hThumbsDB;
    T_S32   i, j, k;
#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
    T_S32   thumbBoxIdx[12], tbOffset, yLine[4];
    T_U8   *thumbBox[12];

	// Check Parameters
    if (AK_NULL == pTDB_Handle || AK_NULL == lcdBuffer
        || AK_NULL == fileArray || 0 == hNum || 0 == vNum
        || yOffset > THUMBNAIL_INTERVAL + THUMBNAIL_HEIGHT)
    {
        return AK_FALSE;
    }
#else
    T_S32   thumbBoxIdx[128], tbOffset, yLine[16];
    T_U8    *thumbBox[128];

    
    if (AK_NULL == pTDB_Handle || AK_NULL == lcdBuffer
        || AK_NULL == fileArray || 0 == hNum || 0 == vNum
        || yOffset > THUMBNAIL_INTERVAL + THUMBNAIL_HEIGHT
        || vNum > 16 || hNum * vNum > 128)
    {
        return AK_FALSE;
    }
#endif

    // Get Thumbnail Box
    Utl_MemSet32(thumbBox, 0, sizeof(thumbBox));
    for (i = 0; i < hNum * vNum; ++i)
    {
        if ((AK_NULL == fileArray[i]) || (Utl_TStrLen(fileArray[i]) == 0))
        {
            thumbBoxIdx[i] = TDB_CACHE_INVALID_IDX;
            thumbBox[i] = AK_NULL;
        }
        else
        {
            thumbBoxIdx[i] = GetThumbnailBox(hThumbsDB, pTDB_Handle, fileArray[i]);
            if (TDB_CACHE_INVALID_IDX != thumbBoxIdx[i])
            {
                thumbBox[i] = pTDB_Handle->cache[thumbBoxIdx[i]].data;
            }
            else
            {
                thumbBox[i] = AK_NULL;
            }
        }
    }
    for (i = 0; i < hNum * vNum; ++i)
    {
        if (TDB_CACHE_INVALID_IDX != thumbBoxIdx[i])
        {
            pTDB_Handle->cache[thumbBoxIdx[i]].nFlag &= ~TDB_CACHE_SHOW_BIT;
        }
    }

    // -------- Display Thumbnail Box --------

    // Display: Top Interval (Not Always Exist)
    yLine[0] = THUMBNAIL_INTERVAL - yOffset;
    if (yLine[0] > 0)
    {
        Utl_MemSet32(lcdBuffer, TDB_BKG_COLOR, yLine[0] * PICTURE_BOX_WIDTH * TDB_COLOR_SIZE);
        lcdBuffer += yLine[0] * PICTURE_BOX_WIDTH * TDB_COLOR_SIZE;
    }

    // Prepare: Top Thumbnail Box (Maybe Partial)
    yLine[0] = (yOffset <= THUMBNAIL_INTERVAL)
        ? (THUMBNAIL_HEIGHT)
        : (THUMBNAIL_HEIGHT + THUMBNAIL_INTERVAL - yOffset);
    // Prepare: Medium Thumbnail Box (Complete)
    for (i = 1; i < vNum - 1; ++i)
    {
        yLine[i] = THUMBNAIL_HEIGHT;
    }
    // Prepare: Bottom Thumbnail Box (Maybe Partial)
    yLine[vNum - 1] = (yOffset <= THUMBNAIL_HEIGHT) ? yOffset : THUMBNAIL_HEIGHT;

    // Display: All Thumbnail Box
    for (i = 0; i < vNum; ++i)
    {
        if (i == 0) {
            tbOffset = (THUMBNAIL_HEIGHT - yLine[0]) * THUMBNAIL_WIDTH * TDB_COLOR_SIZE;//tbOffset is  Thumbnail's data offset!
        } else {
            tbOffset = 0;
        }
        for (k = yLine[i]; k > 0; --k)
        {
            for (j = 0; j < hNum; ++j)
            {
                if (AK_NULL == thumbBox[i * hNum + j])
                {
                    // Left Interval & Thumbnail Box
                    Utl_MemSet32(lcdBuffer, TDB_BKG_COLOR,
                                 (THUMBNAIL_INTERVAL + THUMBNAIL_WIDTH)* TDB_COLOR_SIZE);
                    lcdBuffer += (THUMBNAIL_INTERVAL + THUMBNAIL_WIDTH) * TDB_COLOR_SIZE;
                }
                else
                {
                    // Left Interval
                    Utl_MemSet32(lcdBuffer, TDB_BKG_COLOR, THUMBNAIL_INTERVAL * TDB_COLOR_SIZE);
                    lcdBuffer += THUMBNAIL_INTERVAL * TDB_COLOR_SIZE;

                    // Thumbnail Box
                    Utl_MemCpy32(lcdBuffer, thumbBox[i*hNum+j]+tbOffset, THUMBNAIL_WIDTH * TDB_COLOR_SIZE);
                    lcdBuffer += THUMBNAIL_WIDTH * TDB_COLOR_SIZE;
                }
            }
            tbOffset += THUMBNAIL_WIDTH * TDB_COLOR_SIZE;
            // Right Most Interval
            Utl_MemSet32(lcdBuffer, TDB_BKG_COLOR, THUMBNAIL_INTERVAL * TDB_COLOR_SIZE);
            lcdBuffer += THUMBNAIL_INTERVAL * TDB_COLOR_SIZE;
            }

        // Horizontal Interval
        if ((i + 1) < vNum)
        {
            Utl_MemSet32(lcdBuffer, TDB_BKG_COLOR, (THUMBNAIL_INTERVAL *
            	                              PICTURE_BOX_WIDTH) * TDB_COLOR_SIZE);
            lcdBuffer += (THUMBNAIL_INTERVAL * PICTURE_BOX_WIDTH) * TDB_COLOR_SIZE;
        }
    }

    // Display: Bottom Interval (Not Always Exist)
    yLine[0] = (yOffset <= THUMBNAIL_HEIGHT) ? 0 : (yOffset - THUMBNAIL_HEIGHT);
    if (yLine[0] > 0)
    {
        Utl_MemSet32(lcdBuffer, TDB_BKG_COLOR, yLine[0] * PICTURE_BOX_WIDTH * TDB_COLOR_SIZE);
        lcdBuffer += yLine[0] * PICTURE_BOX_WIDTH * TDB_COLOR_SIZE;
    }

    return AK_TRUE;
}

/**
* @BRIEF direct display thumbnails on LCD screen
* @PARAM hThumbsDB: ThumbsDB handle
* @PARAM lcdBuffer: lcdBuffer
* @PARAM fileArray: image file list
* @PARAM hNum:      number of thumbnail in horizontal
* @PARAM vNum:      number of thumbnail in vertical
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-17
*/
T_U8* ThumbsDB_GetData(THUMBSDB_HANDLE hThumbsDB, T_pCWSTR fileName)
{
    ThumbsDB_Handle *pTDB_Handle = (ThumbsDB_Handle *)hThumbsDB;
    T_S32   thumbBoxIdx;

    // Check Parameters
    if (AK_NULL == pTDB_Handle || AK_NULL == fileName || Utl_TStrLen(fileName) > TDB_FILENAME_MAXLEN)
    {
        return AK_FALSE;
    }

	thumbBoxIdx = GetThumbnailBox(hThumbsDB, pTDB_Handle, fileName);

#ifdef THUMB_Debug
    Fwl_Print(C3, M_THUMB, "ThumbsDB_GetData");
#endif

	if (TDB_CACHE_INVALID_IDX != thumbBoxIdx)
	{
		pTDB_Handle->cache[thumbBoxIdx].nFlag &= ~TDB_CACHE_SHOW_BIT;
		return pTDB_Handle->cache[thumbBoxIdx].data;
	}
	else
	{
		return AK_NULL;
	}
}

/**
* @BRIEF insert a thumbnail into ThumbsDB
* @PARAM hThumbsDB: ThumbsDB handle
* @PARAM filename: image filename(Unicode)
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-17
*/
T_BOOL ThumbsDB_InsertImage(THUMBSDB_HANDLE hThumbsDB, T_pCWSTR filename)
{
    ThumbsDB_Handle *pTDB_Handle = (ThumbsDB_Handle *)hThumbsDB;
    ThumbsDB_Header *pTDB_Header = AK_NULL;
    ThumbsDB_Item   *pTDB_Item = AK_NULL;
    T_S32   fp = FS_INVALID_HANDLE;
    T_U32   nHash;
    T_S32   prevPos, curPos;
    T_U8   *thumbData = AK_NULL;
    T_U32   thumbDataSize;
    T_U16   thumbWidth, thumbHeight;

    // Check Parameters
    if (AK_NULL == pTDB_Handle || AK_NULL == filename || Utl_TStrLen(filename) > TDB_FILENAME_MAXLEN)
    {
        return AK_FALSE;
    }
    pTDB_Header = pTDB_Handle->header;
    pTDB_Item = (ThumbsDB_Item*)((T_U8*)pTDB_Handle
              + sizeof(ThumbsDB_Handle) + sizeof(ThumbsDB_Header));
    Utl_MemSet32(pTDB_Item, 0, sizeof(ThumbsDB_Item));

    // Open ThumbsDB File
    if (TDB_FS_OPEN_WRITE != pTDB_Handle->fileStatus)
    {
        TDB_Close(pTDB_Handle->fp);
        pTDB_Handle->fp = TDB_Open(pTDB_Handle->filename, FS_MODE_WRITE, FS_MODE_WRITE);
        if (FS_INVALID_HANDLE == pTDB_Handle->fp)
        {
            pTDB_Handle->fileStatus = TDB_FS_CLOSE;
            return AK_FALSE;
        }
        pTDB_Handle->fileStatus = TDB_FS_OPEN_WRITE;
    }
    fp = pTDB_Handle->fp;

    // Flush Cached Thumbnail(Only in Memory) to ThumbsDB File
    FlushThumbBoxCache(pTDB_Handle);

    // Calculate Hash Value
    nHash = HashString((T_U8*)filename);
    prevPos = curPos = pTDB_Header->HashTable[nHash];

    // Find the Item According to 'filename'
    if (!FindItem(hThumbsDB, fp, pTDB_Item, filename, &prevPos, &curPos))
    {
        T_S32 newPos;

        // Create Thumbnail
        thumbData = CreateThumbnail(filename, &thumbDataSize,
                                    &thumbWidth, &thumbHeight, TDB_RGB_MODE);
        if (AK_NULL == thumbData)
        {
            // !NOTE: KEEP FILE OPEN (WRITE)
            //return AK_FALSE;

            thumbData = CreateThumbnail(NO_SUPPORT_PIC, &thumbDataSize,
                                    &thumbWidth, &thumbHeight, TDB_RGB_MODE);
        }

        // Obtain an New Item
        newPos = GetNewItemPos(fp, pTDB_Header);

        // Link the New Item to List
        if ((T_U32)prevPos == pTDB_Header->HashTable[nHash]
         && (prevPos == curPos))    // Head Node
        {
            pTDB_Item->NextItem = prevPos;
            pTDB_Header->HashTable[nHash] = newPos;
            TDB_Seek(fp, (T_S32)(TDB_HASHTBL_OFFSET + (nHash << 2)),
                     _FSEEK_SET);
        }
        else
        {
            TDB_Seek(fp, prevPos, _FSEEK_SET);
        }
        TDB_Write(fp, &newPos, sizeof(T_S32));
        curPos = newPos;

        // !NOTE: TDB_Item.NextItem IS READY
        Utl_TStrCpy(pTDB_Item->Filename, filename);

        // Modify ThumbsDB Header
        pTDB_Header->Count++;
        TDB_Seek(fp, TDB_COUNT_OFFSET, _FSEEK_SET);
        TDB_Write(fp, &pTDB_Header->Count, sizeof(T_U32));
    }
    else    // Thumbnail Already Exist!
    {
        // !NOTE: curPos IS READY
        // !NOTE: TDB_Item.NextItem IS READY
        // !NOTE: KEEP FILE OPEN (WRITE)
        return AK_FALSE;
    }

    // Write ThumbsDB Item
    pTDB_Item->nFlag    = 0;
    pTDB_Item->Width    = thumbWidth;
    pTDB_Item->Height   = thumbHeight;
    pTDB_Item->DataSize = thumbDataSize;
    pTDB_Item->Data     = thumbData;
    TDB_Seek(fp, curPos, _FSEEK_SET);
    TDB_Write(fp, pTDB_Item, TDB_ITEM_PARAMSIZE);
    TDB_Write(fp, pTDB_Item->Data, TDB_THUMBBOX_SIZE);

    TDB_Free(pTDB_Item->Data);
    // !NOTE: KEEP FILE OPEN (WRITE)
    return AK_TRUE;
}


/**
* @BRIEF delete a thumbnail in ThumbsDB
* @PARAM hThumbsDB: ThumbsDB handle
* @PARAM filename: image filename(Unicode)
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-17
*/
T_BOOL ThumbsDB_DeleteImage(THUMBSDB_HANDLE hThumbsDB, T_pCWSTR filename)
{
    ThumbsDB_Handle *pTDB_Handle = (ThumbsDB_Handle *)hThumbsDB;
    ThumbsDB_Header *pTDB_Header = AK_NULL;
    ThumbsDB_Item   *pTDB_Item = AK_NULL;
    T_S32   fp = FS_INVALID_HANDLE;
    T_U32   nHash;
    T_S32   prevPos, curPos;
    T_S32   thumbBoxIdx;

    // Check Parameters
    if (AK_NULL == pTDB_Handle || AK_NULL == filename || Utl_TStrLen(filename) > TDB_FILENAME_MAXLEN)
    {
        return AK_FALSE;
    }
    pTDB_Header = pTDB_Handle->header;
    pTDB_Item = (ThumbsDB_Item*)((T_U8*)pTDB_Handle
              + sizeof(ThumbsDB_Handle) + sizeof(ThumbsDB_Header));
    Utl_MemSet32(pTDB_Item, 0, sizeof(ThumbsDB_Item));

    // Open ThumbsDB File
    if (TDB_FS_OPEN_WRITE != pTDB_Handle->fileStatus)
    {
        TDB_Close(pTDB_Handle->fp);
        pTDB_Handle->fp = TDB_Open(pTDB_Handle->filename, FS_MODE_WRITE, FS_MODE_WRITE);
        if (FS_INVALID_HANDLE == pTDB_Handle->fp)
        {
            pTDB_Handle->fileStatus = TDB_FS_CLOSE;
            return AK_FALSE;
        }
        pTDB_Handle->fileStatus = TDB_FS_OPEN_WRITE;
    }
    fp = pTDB_Handle->fp;

    // Free Cache Item of Deleted Thumbnail
    thumbBoxIdx = GetThumbBoxFromCache(pTDB_Handle, filename);
    if (TDB_CACHE_INVALID_IDX != thumbBoxIdx)
    {
        T_U32   nMemFlag;

        nMemFlag = pTDB_Handle->cache[thumbBoxIdx].nFlag & TDB_CACHE_MEM_BIT;
        TDB_Free(pTDB_Handle->cache[thumbBoxIdx].data);
        Utl_MemSet32(&pTDB_Handle->cache[thumbBoxIdx], 0, sizeof(TDB_CacheItem));
        if (nMemFlag)
        {
            pTDB_Handle->cacheMemCount--;
            return AK_TRUE;
        }
    }

    // Flush Cached Thumbnail(Only in Memory) to ThumbsDB File
    FlushThumbBoxCache(pTDB_Handle);

    // Calculate Hash Value
    nHash = HashString((T_U8*)filename);
    prevPos = curPos = pTDB_Header->HashTable[nHash];

    // Find the Item According to 'filename'
    if (FindItem(hThumbsDB, fp, pTDB_Item, filename, &prevPos, &curPos))
    {
        // Pick Off the Item from List
        if ((T_U32)prevPos == pTDB_Header->HashTable[nHash]
         && (prevPos == curPos))    // Head Node
        {
            pTDB_Header->HashTable[nHash] = pTDB_Item->NextItem;
            TDB_Seek(fp, (T_S32)(TDB_HASHTBL_OFFSET + (nHash << 2)),
                     _FSEEK_SET);
            TDB_Write(fp, &pTDB_Header->HashTable[nHash], sizeof(T_S32));
        }
        else
        {
            TDB_Seek(fp, prevPos, _FSEEK_SET);
            TDB_Write(fp, &pTDB_Item->NextItem, sizeof(T_S32));
        }

        // Link the Deleted Item to the Head of Free List
        TDB_Seek(fp, curPos, _FSEEK_SET);
        TDB_Write(fp, &pTDB_Header->FreeHead, sizeof(T_S32));
        pTDB_Header->FreeHead = curPos;
        pTDB_Header->FreeItems++;
        TDB_Seek(fp, TDB_FREEHEAD_OFFSET, _FSEEK_SET);
        TDB_Write(fp, &pTDB_Header->FreeHead, sizeof(T_S32));
        TDB_Write(fp, &pTDB_Header->FreeItems, sizeof(T_U32));

        // Modify ThumbsDB Header
        pTDB_Header->Count--;
        TDB_Seek(fp, TDB_COUNT_OFFSET, _FSEEK_SET);
        TDB_Write(fp, &pTDB_Header->Count, sizeof(T_U32));
    }
    else    // Thumbnail Not Found!
    {
        // !NOTE: KEEP FILE OPEN (WRITE)
        return AK_FALSE;
    }

    // !NOTE: KEEP FILE OPEN (WRITE)
    return AK_TRUE;
}


/**
* @BRIEF delete all thumbnail in ThumbsDB
* @PARAM hThumbsDB: ThumbsDB handle
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-16
*/
T_BOOL ThumbsDB_DeleteAllImages(THUMBSDB_HANDLE hThumbsDB)
{
    ThumbsDB_Handle *pTDB_Handle = (ThumbsDB_Handle *)hThumbsDB;
    ThumbsDB_Header *pTDB_Header = AK_NULL;
    T_S32   fp = FS_INVALID_HANDLE;
    T_S32   i;

    // Check Parameters
    if (AK_NULL == pTDB_Handle)
    {
        return AK_FALSE;
    }
    pTDB_Header = pTDB_Handle->header;

    // Open ThumbsDB File(Truncate)
    if (TDB_FS_CLOSE != pTDB_Handle->fileStatus)
    {
        TDB_Close(pTDB_Handle->fp);
        pTDB_Handle->fp = FS_INVALID_HANDLE;
        pTDB_Handle->fileStatus = TDB_FS_CLOSE;
    }
    fp = TDB_Open(pTDB_Handle->filename, _FMODE_CREATE, _FMODE_CREATE);
    if (FS_INVALID_HANDLE == fp)
    {
        return AK_FALSE;
    }

    // Initialize ThumbsDB Header
    Utl_MemCpy32(pTDB_Header->Tag, "AKTHUMBS", 8);
    pTDB_Header->HeaderSize = sizeof(ThumbsDB_Header);
    pTDB_Header->MaxWidth   = THUMBNAIL_WIDTH;
    pTDB_Header->MaxHeight  = THUMBNAIL_HEIGHT;
    pTDB_Header->Count      = 0;
    pTDB_Header->FreeHead   = 0;
    pTDB_Header->FreeItems  = 0;
    pTDB_Header->HashTableCount = 1024;
    Utl_MemSet32(pTDB_Header->HashTable, 0, sizeof(pTDB_Header->HashTable));

    // Save ThumbsDB Header to File
    TDB_Seek(fp, 0L, _FSEEK_SET);
    TDB_Write(fp, pTDB_Header, sizeof(ThumbsDB_Header));
    TDB_Close(fp);

    // Free ThumbsDB Cache
    for (i = 0; i < TDB_CACHE_SIZE; ++i)
    {
        if (AK_NULL != pTDB_Handle->cache[i].data)
        {
            TDB_Free(pTDB_Handle->cache[i].data);
            Utl_MemSet32(&pTDB_Handle->cache[i], 0, sizeof(TDB_CacheItem));
        }
    }
    pTDB_Handle->cacheTimer = 0;
    pTDB_Handle->cacheMemCount = 0;

    return AK_TRUE;
}


/**
* @BRIEF rename a thumbnail in ThumbsDB
* @PARAM hThumbsDB:   ThumbsDB handle
* @PARAM oldFilename: old image filename(Unicode)
* @PARAM newFilename: new image filename(Unicode)
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-17
*/
T_BOOL ThumbsDB_RenameImage(THUMBSDB_HANDLE hThumbsDB,
                            T_pCWSTR oldFilename, T_pCWSTR newFilename)
{
    ThumbsDB_Handle *pTDB_Handle = (ThumbsDB_Handle *)hThumbsDB;
    ThumbsDB_Header *pTDB_Header = AK_NULL;
    ThumbsDB_Item   *pTDB_Item = AK_NULL;
    T_S32   fp = FS_INVALID_HANDLE;
    T_U32   nHashOld, nHashNew;
    T_S32   prevPosOld, curPosOld;
    T_S32   prevPosNew, curPosNew;
    T_S32   thumbBoxIdx;

    // Check Parameters
    if (AK_NULL == pTDB_Handle || AK_NULL == oldFilename || Utl_TStrLen(oldFilename) > TDB_FILENAME_MAXLEN 
    	|| AK_NULL == newFilename || Utl_TStrLen(newFilename) > TDB_FILENAME_MAXLEN)
    {
        return AK_FALSE;
    }
    pTDB_Header = pTDB_Handle->header;
    pTDB_Item = (ThumbsDB_Item*)((T_U8*)pTDB_Handle
              + sizeof(ThumbsDB_Handle) + sizeof(ThumbsDB_Header));
    Utl_MemSet32(pTDB_Item, 0, sizeof(ThumbsDB_Item));

    // Open ThumbsDB File
    if (TDB_FS_OPEN_WRITE != pTDB_Handle->fileStatus)
    {
        TDB_Close(pTDB_Handle->fp);
        pTDB_Handle->fp = TDB_Open(pTDB_Handle->filename, FS_MODE_WRITE, FS_MODE_WRITE);
        if (FS_INVALID_HANDLE == pTDB_Handle->fp)
        {
            pTDB_Handle->fileStatus = TDB_FS_CLOSE;
            return AK_FALSE;
        }
        pTDB_Handle->fileStatus = TDB_FS_OPEN_WRITE;
    }
    fp = pTDB_Handle->fp;

    // Rename Thumbnail in Cache
    thumbBoxIdx = GetThumbBoxFromCache(pTDB_Handle, oldFilename);
    if (TDB_CACHE_INVALID_IDX != thumbBoxIdx)
    {
        Utl_TStrCpy(pTDB_Handle->cache[thumbBoxIdx].filename, newFilename);
        if (pTDB_Handle->cache[thumbBoxIdx].nFlag & TDB_CACHE_MEM_BIT)
        {
            return AK_TRUE;
        }
    }

    // Flush Cached Thumbnail(Only in Memory) to ThumbsDB File
    FlushThumbBoxCache(pTDB_Handle);

    // Calculate Hash Value
    nHashOld = HashString((T_U8*)oldFilename);
    prevPosOld = curPosOld = pTDB_Handle->header->HashTable[nHashOld];
    nHashNew = HashString((T_U8*)newFilename);
    prevPosNew = curPosNew = pTDB_Handle->header->HashTable[nHashNew];

    // Find the Item According to 'filename'
    if (FindItem(hThumbsDB, fp, pTDB_Item, oldFilename, &prevPosOld, &curPosOld))
    {
        if (nHashNew == nHashOld)
        {
            TDB_Seek(fp, curPosOld + sizeof(T_S32), _FSEEK_SET);
            //!NOTE: Utl_TStrLen count Chars but NOT Bytes
            TDB_Write(fp, (T_VOID*)newFilename,
                      sizeof(T_WCHR) * (Utl_TStrLen(newFilename) + 1));
        }
        else
        {
            ThumbsDB_Item   TDB_ItemNew;

            Utl_MemSet32(&TDB_ItemNew, 0, sizeof(ThumbsDB_Item));
            // Find the Insert Position
            if (!FindItem(hThumbsDB, fp, &TDB_ItemNew, newFilename, &prevPosNew, &curPosNew))
            {
                // Pick Off the Item from Old List
                if ((T_U32)prevPosOld == pTDB_Header->HashTable[nHashOld]
                 && (prevPosOld == curPosOld))     // Head Node
                {
                    pTDB_Header->HashTable[nHashOld] = pTDB_Item->NextItem;
                    TDB_Seek(fp, (T_S32)(TDB_HASHTBL_OFFSET + (nHashOld << 2)),
                             _FSEEK_SET);
                    TDB_Write(fp, &pTDB_Header->HashTable[nHashOld], sizeof(T_S32));
                }
                else
                {
                    TDB_Seek(fp, prevPosOld, _FSEEK_SET);
                    TDB_Write(fp, &pTDB_Item->NextItem, sizeof(T_S32));
                }

                // Link the Item to New List
                if (((T_U32)prevPosNew == pTDB_Header->HashTable[nHashNew])
                 && (prevPosNew == curPosNew))  // Head Node
                {
                    TDB_ItemNew.NextItem = prevPosNew;
                    pTDB_Header->HashTable[nHashNew] = curPosOld;
                    TDB_Seek(fp, (T_S32)(TDB_HASHTBL_OFFSET + (nHashNew << 2)),
                             _FSEEK_SET);
                }
                else
                {
                    TDB_Seek(fp, prevPosNew, _FSEEK_SET);
                }
                TDB_Write(fp, &curPosOld, sizeof(T_S32));

                // !NOTE: TDB_ItemNew.NextItem IS READY
                TDB_Seek(fp, curPosOld, _FSEEK_SET);
                TDB_Write(fp, &TDB_ItemNew.NextItem, sizeof(T_S32));
                TDB_Write(fp, (T_VOID*)newFilename, 256 * sizeof(T_WCHR));
            }
            else    // Conflict: Thumbnail Already Exist!
            {
                // !NOTE: KEEP FILE OPEN (WRITE)
                return AK_FALSE;
            }
        }
    }
    else    // Thumbnail Not Found!
    {
        // !NOTE: KEEP FILE OPEN (WRITE)
        return AK_FALSE;
    }

    // !NOTE: KEEP FILE OPEN (WRITE)
    return AK_TRUE;
}


/**
* @BRIEF obtain a thumbnail from ThumbsDB
* @PARAM hThumbsDB: ThumbsDB handle
* @PARAM filename: image filename(Unicode)
* @PARAM size:     <out> size of thumbnail
* @PARAM width:    <out> width of thumbnail
* @PARAM height:   <out> height of thumbnail
* @RETURN pointer to thumbnail image, AK_NULL if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-17
*/
T_U8*  ThumbsDB_GetImage(THUMBSDB_HANDLE hThumbsDB, T_pCWSTR filename,
                         T_U32 *size, T_U16 *width, T_U16 *height)
{
    ThumbsDB_Handle *pTDB_Handle = (ThumbsDB_Handle *)hThumbsDB;
    ThumbsDB_Header *pTDB_Header = AK_NULL;
    ThumbsDB_Item   *pTDB_Item = AK_NULL;
    T_S32   fp = FS_INVALID_HANDLE;
    T_U32   nHash;
    T_S32   prevPos, curPos;

    // Check Parameters
    if (AK_NULL == pTDB_Handle || AK_NULL == filename || Utl_TStrLen(filename) > TDB_FILENAME_MAXLEN
     || AK_NULL == size || AK_NULL == width || AK_NULL == height)
    {
        return AK_NULL;
    }
    pTDB_Header = pTDB_Handle->header;
    pTDB_Item = (ThumbsDB_Item*)((T_U8*)pTDB_Handle
              + sizeof(ThumbsDB_Handle) + sizeof(ThumbsDB_Header));
    Utl_MemSet32(pTDB_Item, 0, sizeof(ThumbsDB_Item));

    // Open ThumbsDB File
    if (TDB_FS_CLOSE == pTDB_Handle->fileStatus)
    {
        pTDB_Handle->fp = TDB_Open(pTDB_Handle->filename, _FMODE_READ, _FMODE_READ);
        if (FS_INVALID_HANDLE == pTDB_Handle->fp)   // ThumbsDB File Not Exist!
        {
            pTDB_Handle->fileStatus = TDB_FS_CLOSE;
            return AK_NULL;
        }
        pTDB_Handle->fileStatus = TDB_FS_OPEN_READ;
    }
    fp = pTDB_Handle->fp;

    // Flush Cached Thumbnail(Only in Memory) to ThumbsDB File
    FlushThumbBoxCache(pTDB_Handle);

    // Calculate Hash Value
    nHash = HashString((T_U8*)filename);
    prevPos = curPos = pTDB_Header->HashTable[nHash];

    // Find the Item According to 'filename'
    if (!FindItem(hThumbsDB, fp, pTDB_Item, filename, &prevPos, &curPos))  // Thumbnail Not Exist!
    {
        T_BOOL  bRet;

        // Insert Thumbnail
        bRet = ThumbsDB_InsertImage(hThumbsDB, filename);

        // Find the Item According to 'filename'
		prevPos = curPos = pTDB_Header->HashTable[nHash];
        if (!bRet || !FindItem(hThumbsDB, fp, pTDB_Item, filename, &prevPos, &curPos))
        {
            return AK_NULL;
        }
    }

    pTDB_Item->Data = (T_U8*)TDB_Malloc(pTDB_Item->DataSize);
    if (AK_NULL == pTDB_Item->Data)
    {
        return AK_NULL;
    }

    TDB_Seek(fp, curPos + TDB_ITEM_PARAMSIZE, _FSEEK_SET);
    TDB_Read(fp, pTDB_Item->Data, pTDB_Item->DataSize);

    *size = pTDB_Item->DataSize;
    *width  = pTDB_Item->Width;
    *height = pTDB_Item->Height;

    // !NOTE: KEEP FILE OPEN
    return pTDB_Item->Data;
}


/**
* @BRIEF clean ThumbsDB, delete invalid thumbnail
* @PARAM hThumbsDB: ThumbsDB handle
* @PARAM fileArray: image file list
* @PARAM fileCount: total number of image files
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-22
* @UPDATE 2008-10-17
*/
T_BOOL ThumbsDB_Clean(THUMBSDB_HANDLE hThumbsDB,
                      T_pCWSTR fileArray[], T_U16 fileCount)
{
    ThumbsDB_Handle *pTDB_Handle = (ThumbsDB_Handle *)hThumbsDB;
    ThumbsDB_Header *pTDB_Header = AK_NULL;
    ThumbsDB_Item   *pTDB_Item = AK_NULL;
    T_S32   fp = FS_INVALID_HANDLE;
    T_U32   nHash;
    T_S32   prevPos, curPos;
    T_S32   i;
    T_U32   nCount, nFlag;

    // Check Parameters
    if (AK_NULL == pTDB_Handle || AK_NULL == fileArray || 0 == fileCount)
    {
        return AK_FALSE;
    }
    pTDB_Header = pTDB_Handle->header;
    pTDB_Item = (ThumbsDB_Item*)((T_U8*)pTDB_Handle
              + sizeof(ThumbsDB_Handle) + sizeof(ThumbsDB_Header));
    Utl_MemSet32(pTDB_Item, 0, sizeof(ThumbsDB_Item));

    // Open ThumbsDB File
    if (TDB_FS_OPEN_WRITE != pTDB_Handle->fileStatus)
    {
        TDB_Close(pTDB_Handle->fp);
        pTDB_Handle->fp = TDB_Open(pTDB_Handle->filename, FS_MODE_WRITE, FS_MODE_WRITE);
        if (FS_INVALID_HANDLE == pTDB_Handle->fp)
        {
            pTDB_Handle->fileStatus = TDB_FS_CLOSE;
            return AK_FALSE;
        }
        pTDB_Handle->fileStatus = TDB_FS_OPEN_WRITE;
    }
    fp = pTDB_Handle->fp;

    // Flush Cached Thumbnail(Only in Memory) to ThumbsDB File
    FlushThumbBoxCache(pTDB_Handle);

    // Mark The Flag of Exist Thumbnail
    nFlag = 1;
    nCount = 0;
    for (i = (T_S32)fileCount - 1; i >= 0; --i)
    {
        // Calculate Hash Value
        nHash = HashString((T_U8*)fileArray[i]);
        prevPos = curPos = pTDB_Header->HashTable[nHash];

        // Find the Item According to 'filename'
        if (FindItem(hThumbsDB, fp, pTDB_Item, fileArray[i], &prevPos, &curPos))
        {
            TDB_Seek(fp, (T_S32)(curPos + TDB_ITEM_FLAG_OFFSET), _FSEEK_SET);
            TDB_Write(fp, &nFlag, sizeof(T_U32));
            nCount++;
        }
        else
        {
            // ???
        }
    }
    pTDB_Header->Count = nCount;

    // Clear The Not Exit Thumbnail (Move to Free List)
    for (i = pTDB_Header->HashTableCount - 1; i >= 0; --i)
    {
        if (pTDB_Header->HashTable[i] == 0)
        {
            continue;
        }

        prevPos = curPos = pTDB_Header->HashTable[i];
        do {
            // Read the Flag
            TDB_Seek(fp, curPos + TDB_ITEM_FLAG_OFFSET, _FSEEK_SET);
            TDB_Read(fp, &nFlag, sizeof(T_U32));

            if (nFlag != 0)  // Reset Flag of Exist Thumbnail
            {
                nFlag = 0;
                TDB_Seek(fp, curPos + TDB_ITEM_FLAG_OFFSET, _FSEEK_SET);
                TDB_Write(fp, &nFlag, sizeof(T_U32));

                // Search for Next Node
                prevPos = curPos;
                TDB_Seek(fp, (T_S32)curPos, _FSEEK_SET);
                TDB_Read(fp, &curPos, sizeof(T_U32));
            }
            else            // Move The Not Exist Thumbnail to Free List
            {
                T_S32   nextPos;

                // Search for Next Node
                TDB_Seek(fp, (T_S32)curPos, _FSEEK_SET);
                TDB_Read(fp, &nextPos, sizeof(T_U32));

                if ((T_U32)prevPos == pTDB_Header->HashTable[i])    // Head Node
                {
                    pTDB_Header->HashTable[i] = nextPos;
                    prevPos = nextPos;
                }
                else
                {
                    TDB_Seek(fp, prevPos, _FSEEK_SET);
                    TDB_Write(fp, &nextPos, sizeof(T_S32));
                }

                // Link the Item to the Head of Free List
                TDB_Seek(fp, curPos, _FSEEK_SET);
                TDB_Write(fp, &pTDB_Header->FreeHead, sizeof(T_S32));
                pTDB_Header->FreeHead = curPos;
                pTDB_Header->FreeItems++;

                curPos = nextPos;
            }
        } while (curPos != 0);
    }

    // Write ThumbsDB Header to File
    TDB_Seek(fp, TDB_COUNT_OFFSET, _FSEEK_SET);
    TDB_Write(fp, &pTDB_Header->Count, sizeof(T_U32));
    TDB_Seek(fp, TDB_FREEHEAD_OFFSET, _FSEEK_SET);
    TDB_Write(fp, &pTDB_Header->FreeHead, sizeof(T_S32));
    TDB_Write(fp, &pTDB_Header->FreeItems, sizeof(T_U32));
    TDB_Seek(fp, TDB_HASHTBL_OFFSET, _FSEEK_SET);
    TDB_Write(fp, pTDB_Header->HashTable,
              sizeof(T_U32) * pTDB_Header->HashTableCount);

    // !NOTE: KEEP FILE OPEN (WRITE)
    return AK_TRUE;
}

/**
* @BRIEF check whether a file is an image or not
* @PARAM hThumbsDB: ThumbsDB handle
* @PARAM filename: image filename(Unicode)
* @RETURN AK_TRUE if the file is an image, AK_FALSE if not
* @AUTHOR zhang_chengyan
* @DATE 2008-10-07
* @UPDATE 2008-10-07
*/
T_BOOL ThumbsDB_IsImage(THUMBSDB_HANDLE hThumbsDB, T_pCWSTR filename)
{
    T_S32           fp = FS_INVALID_HANDLE;
    T_U8            buf[4];
    T_IMAGE_TYPE    type;

    return AK_TRUE;//由于读取文件的操作影响速度，暂时将下面对文件的操作去掉

    // Read Image Tag From File
    if (AK_NULL == filename || Utl_TStrLen(filename) > TDB_FILENAME_MAXLEN)
    	return AK_FALSE;
    
    fp = TDB_Open(filename, _FMODE_READ, _FMODE_READ);
    if (FS_INVALID_HANDLE == fp)    // File Not Exist!
    {
        return AK_FALSE;
    }
    TDB_Read(fp, buf, sizeof(buf));
    TDB_Close(fp);

    // Check Image Tag
    if ((buf[0]  == (T_U8)0x42) && (buf[1] ==  (T_U8)0x4D))
    {
        type = IMG_IMAGE_BMP;
    }
    else if ((buf[0] == (T_U8)0x00) && (buf[1] ==  (T_U8)0x00))
    {
        type = IMG_IMAGE_WBMP;
    }
    else if ((buf[0] == (T_U8)0xFF) && (buf[1] == (T_U8)0xD8))
    {
        type = IMG_IMAGE_JPG;
    }
    else if ((buf[0] == (T_U8)0x47) && (buf[1] == (T_U8)0x49))
    {
        type = IMG_IMAGE_GIF;
    }
    else if ((buf[0] == (T_U8)0x89) && (buf[1] == (T_U8)0x50)
          && (buf[2] == (T_U8)0x4E) && (buf[3] == (T_U8)0x47))
    {
        type = IMG_IMAGE_PNG;
    }
    else
    {
        type = IMG_IMAGE_UNKNOWN;
    }

    return (type != IMG_IMAGE_UNKNOWN);
}

////////////////////////////////////////////////////////////////////////////////

/**
* @BRIEF ELF Hash Function
* @PARAM str: zero terminal string
* @RETURN ELF hash value modulus 1024
* @AUTHOR ELF
* @DATE 2008-09-23
* @UPDATE 2008-10-14
*/
static T_U32 HashString(const T_U8* str)
{
    T_U32 hash = 0;
    T_U32 x    = 0;
#ifdef TDB_TEST
    const T_U8  *ptr = (const T_U8* )str;
#else
    const T_U16 *ptr = (const T_U16*)str;
#endif

    while (*ptr)
    {
        hash = (hash << 4) + (*ptr++);
        if ((x = hash & 0xF0000000uL) != 0)
        {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }
#ifdef TDB_TEST
    return (hash & 0x3FF);          // hash % 1024
#else
    return ((hash >> 14) & 0x3FF);  // (hash >> 14) % 1024
#endif
}

/**
* @BRIEF find an thumbnail item according to filename
* @PARAM fp:        file handle
* @PARAM pTDB_Item: buffer for ThumbsDB Item
* @PARAM filename:  image filename
* @PARAM prevPos:   <in/out> previous position
* @PARAM curPos:    <in/out> current position
* @RETURN AK_TRUE if the item is found (position: prevPos/curPos, value: pTDB_Item)
*         AK_FALSE if the item not found (insert position: prevPos)
* @AUTHOR zhang_chengyan
* @DATE 2008-09-23
* @UPDATE 2008-09-26
*/
static T_S32 FindItem(THUMBSDB_HANDLE hThumbsDB, T_S32 fp, ThumbsDB_Item* pTDB_Item, T_pCWSTR filename,
                      T_S32* prevPos, T_S32* curPos)
{
    T_S32   nCmp;
    
    while (*curPos != 0)
    {
        if(TDB_Seek(fp, *curPos, _FSEEK_SET) != *curPos)
        {
        	return AK_FALSE;
        }
        
        TDB_Read(fp, pTDB_Item, TDB_ITEM_PARAMSIZE);
        
        nCmp = Utl_TStrCmp((T_pCWSTR)pTDB_Item->Filename, filename);
#if 1   // !NOTE: NO SORT
        if (nCmp == 0)
        {
            return AK_TRUE;
        }
#else   // SORT
        if (nCmp > 0)
        {
            return AK_FALSE;
        }
        else if (nCmp == 0)
        {
            return AK_TRUE;
        }
#endif
		
        *prevPos = *curPos;
        *curPos = pTDB_Item->NextItem;

        if(*prevPos == *curPos)
		{
			ThumbsDB_DeleteAllImages(hThumbsDB);
			return AK_FALSE;
		}
    }

    pTDB_Item->NextItem = 0;
    return AK_FALSE;
}

/**
* @BRIEF get new ThumbsDB item's position for data storage
* @PARAM fp:          file handle
* @PARAM pTDB_Header: pointer of ThumbsDB Header
* @RETURN the position for data storage (from either Free List or End of File)
* @AUTHOR zhang_chengyan
* @DATE 2008-09-23
* @UPDATE 2008-09-25
*/
static T_S32 GetNewItemPos(T_S32 fp, ThumbsDB_Header* pTDB_Header)
{
    T_S32   curPos;

    if (pTDB_Header->FreeItems != 0)
    {
        // Pick Up a Free Item
        curPos = pTDB_Header->FreeHead;
        TDB_Seek(fp, curPos, _FSEEK_SET);
        TDB_Read(fp, &pTDB_Header->FreeHead, sizeof(T_S32));
        pTDB_Header->FreeItems--;
        TDB_Seek(fp, TDB_FREEHEAD_OFFSET, _FSEEK_SET);
        TDB_Write(fp, &pTDB_Header->FreeHead, sizeof(T_S32));
        TDB_Write(fp, &pTDB_Header->FreeItems, sizeof(T_U32));
    }
    else
    {
        // Seek to the End of File
        TDB_Seek(fp, 0L, _FSEEK_END);
        curPos = TDB_Tell(fp);
    }

    return curPos;
}


/**
* @BRIEF create thumbnail for large bmp according to filename (RGB565)
* @PARAM filename: image filename
* @PARAM size:     <out> size of thumbnail
* @PARAM width:    <out> width of thumbnail
* @PARAM height:   <out> height of thumbnail
* @PARAM format:   TDB_RGB_565 / TDB_RGB_888
* @RETURN pointer to thumbnail, AK_NULL if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-23
* @UPDATE 2008-10-16
*/
static T_U8*  CreateLargeBmpThumbnail(T_pCWSTR filename, T_U32 *size,
                              T_U16 *width, T_U16 *height, T_U32 format)
{
	Fixed		scaleFactor = ONE;
	T_U16		dstW, dstH;
	T_U8		*thumbnail = AK_NULL;
	T_IMGDEC_LARGE_ATTRIB *pLargeAttrib = AK_NULL;

	// Check Parameter
	if (TDB_RGB_565 != format && TDB_RGB_888 != format)
	{
		return AK_NULL;
	}

	pLargeAttrib = (T_IMGDEC_LARGE_ATTRIB *)Fwl_Malloc(sizeof(T_IMGDEC_LARGE_ATTRIB));

	if (AK_NULL == pLargeAttrib)
	{
		return AK_NULL;
	}

	memset(pLargeAttrib, 0, sizeof(T_IMGDEC_LARGE_ATTRIB));

	if (!ImgDec_LargeBmpDecOpen(pLargeAttrib, filename, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT))
	{
		pLargeAttrib = Fwl_Free(pLargeAttrib);
		return AK_NULL;
	}

	while(DEC_CONTINUE == ImgDec_LargeBmpDecStep(pLargeAttrib));

	if (DEC_COMPLETE == ImgDec_GetLargeDecStatus(pLargeAttrib))
	{

		dstW = (T_U16)pLargeAttrib->ImgAttrib.OutImgW;
		dstH = (T_U16)pLargeAttrib->ImgAttrib.OutImgH;
		
		// Scale & Convert BMP to Thumbnail
	    thumbnail = Bmp2Thumbnail(pLargeAttrib->ImgAttrib.pOutImgBuf, scaleFactor, dstW, dstH, format);

#ifdef THUMB_Debug
	             Fwl_Print(C3, M_THUMB, "thumbnail=%d", thumbnail);
#endif

	    if (AK_NULL != thumbnail)
	    {
	        *size = (TDB_RGB_565 == format) ? TDB_THUMBSIZE_565: TDB_THUMBSIZE_888;
	        *width  = THUMBNAIL_WIDTH;
	        *height = THUMBNAIL_HEIGHT;
	    }
	    else
	    {
	        // !NOTE: DO NOTHING
	    }
	}
	ImgDec_LargeBmpDecClose(pLargeAttrib);
	pLargeAttrib = Fwl_Free(pLargeAttrib);

	return thumbnail;
}

////////////////////////////////////////////////////////////////////////////////

/**
* @BRIEF create thumbnail according to filename (RGB565)
* @PARAM filename: image filename
* @PARAM size:     <out> size of thumbnail
* @PARAM width:    <out> width of thumbnail
* @PARAM height:   <out> height of thumbnail
* @PARAM format:   TDB_RGB_565 / TDB_RGB_888
* @RETURN pointer to thumbnail, AK_NULL if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-23
* @UPDATE 2008-10-16
*/
static T_U8*  CreateThumbnail(T_pCWSTR filename, T_U32 *size,
                              T_U16 *width, T_U16 *height, T_U32 format)
{
    T_S32		fp;
    TDB_ImgInfo	imgInfo;
    T_hGIFENUM	hGif = IMG_INVALID_HANDLE;
    T_U8		*bmpData = AK_NULL;
    T_U32		bmpSize;
    Fixed		scaleFactor;
    T_U16		dstW, dstH;
    T_U8		*thumbnail = AK_NULL;
    T_BOOL		bRet;
    T_U8		typeInfo[IMG_TYPE_INFO_SIZE] = {0};

#ifdef THUMB_Debug
    T_U8        szStr[512];
#endif
    // Check Parameter
    if (TDB_RGB_565 != format && TDB_RGB_888 != format)
    {
        return AK_NULL;
    }

    // Read Image File
    fp = TDB_Open(filename, _FMODE_READ, _FMODE_READ);


#ifdef THUMB_Debug
                Eng_StrUcs2Mbcs(filename, szStr);                
                
                Fwl_Print(C3, M_THUMB, "fp=%d, szStr=%s, sizeof(TDB_ImgInfo)=%d", fp,szStr, sizeof(TDB_ImgInfo));
				
#endif

    if (FS_INVALID_HANDLE == fp)   // Image File Not Exist!
    {
        return AK_NULL;
    }

    // Get Image Size
    Utl_MemSet32(&imgInfo, 0, sizeof(TDB_ImgInfo));
    TDB_Seek(fp, 0L, _FSEEK_END);
    imgInfo.size = TDB_Tell(fp);
    if (imgInfo.size < 0)
    {
    	TDB_Close(fp);
        return AK_NULL;
    }
    
	TDB_Seek(fp, 0, _FSEEK_SET);
    TDB_Read(fp, typeInfo, IMG_TYPE_INFO_SIZE);
    imgInfo.type = Img_ImageType(typeInfo);

    // Allocate Memory for Image Data
    imgInfo.data = (T_U8*)TDB_Malloc(imgInfo.size);

#ifdef THUMB_Debug
                    Fwl_Print(C3, M_THUMB, "imgInfo.data=%d, imgInfo.size=%d", imgInfo.data, imgInfo.size);
#endif
    
    if (AK_NULL == imgInfo.data)
    {
		Fwl_Print(C2, M_THUMB,"Malloc %d Failure\n", imgInfo.size);
		
    	TDB_Close(fp);
    	
    	if (IMG_IMAGE_BMP == imgInfo.type)
    	{
			return CreateLargeBmpThumbnail(filename, size, width, height, format);
    	}
    	else
    	{
        	return AK_NULL;
        }
    }

    // Read Image Data & Get Image Type
    TDB_Seek(fp, 0L, _FSEEK_SET);
    TDB_Read(fp, imgInfo.data, imgInfo.size);
    TDB_Close(fp);
    //imgInfo.type = Img_ImageType(imgInfo.data);

#ifdef THUMB_Debug
     Fwl_Print(C3, M_THUMB, "imgInfo.type=%d, imgInfo.size=%d", imgInfo.type,imgInfo.size);
#endif

    // Get Image Info: width, height, bitCount
    bRet = GetImgInfo(&imgInfo);

#ifdef THUMB_Debug
         Fwl_Print(C3, M_THUMB, "22imgInfo.type=%d, imgInfo.size=%d", imgInfo.type,imgInfo.size);
#endif

    if (!bRet)// || (imgInfo.width*imgInfo.height > 2000000))
    {
        TDB_Free(imgInfo.data);
        return AK_NULL;
    }

    // Decode Image to BMP
    bmpSize = imgInfo.size;

#ifdef THUMB_Debug
         Fwl_Print(C3, M_THUMB, "imgInfo.size=%d", imgInfo.size);
#endif

    
    bmpData = DecImage(&imgInfo, &bmpSize, &scaleFactor, &dstW, &dstH, &hGif);

    
#ifdef THUMB_Debug
             Fwl_Print(C3, M_THUMB, "bmpData=%d", bmpData);
#endif


    if (AK_NULL == bmpData)
    {
        TDB_Free(imgInfo.data);
        return AK_NULL;
    }

    // Scale & Convert BMP to Thumbnail
    thumbnail = Bmp2Thumbnail(bmpData, scaleFactor, dstW, dstH, format);

#ifdef THUMB_Debug
             Fwl_Print(C3, M_THUMB, "thumbnail=%d", thumbnail);
#endif

    if (AK_NULL != thumbnail)
    {
        *size = (TDB_RGB_565 == format) ? TDB_THUMBSIZE_565: TDB_THUMBSIZE_888;
        *width  = THUMBNAIL_WIDTH;
        *height = THUMBNAIL_HEIGHT;
    }
    else
    {
        // !NOTE: DO NOTHING
    }

#ifdef THUMB_Debug
                 Fwl_Print(C3, M_THUMB, "imgInfo.type=%d", imgInfo.type);
#endif

    // Free Resources
    switch (imgInfo.type)
    {
    case IMG_IMAGE_BMP:
        break;
    case IMG_IMAGE_JPG:
    case IMG_IMAGE_PNG:
        TDB_Free(bmpData);
        break;
    case IMG_IMAGE_GIF:
        GIFEnum_Close(hGif);
        break;
    default:
        break;
    }
    TDB_Free(imgInfo.data);

    return thumbnail;
}

/**
* @BRIEF get BMP image info: width, height
* @PARAM bmpBuf: BMP image buffer
* @PARAM width:  <out> width of BMP image
* @PARAM height: <out> height of BMP image
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-25
* @UPDATE 2008-09-25
*/
static T_BOOL BmpInfo(const T_U8* bmpBuf, T_U16* width, T_U16* height)
{
    BITMAPINFOHEADER *pbi = (BITMAPINFOHEADER*)(bmpBuf + sizeof(BITMAPFILEHEADER) );

#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "pbi->biWidth =%d,pbi->biHeight = %d,sizeof bitfilehead = %d, &width=%d , &height =%d", pbi->biWidth, pbi->biHeight, sizeof(BITMAPFILEHEADER), width, height);
#endif

    if (pbi->biWidth == 0 || pbi->biHeight == 0)
    {
        return AK_FALSE;
    }

    *width  = (T_U16)pbi->biWidth;
    *height = (pbi->biHeight > 0) ? (T_U16)pbi->biHeight : (T_U16)(-pbi->biHeight);

#ifdef THUMB_Debug
    Fwl_Print(C3, M_THUMB, "BmpInfo width =%d,height = %d, &width =%d, &height =%d", *width, *height, width, height);
#endif
    return AK_TRUE;
}

#if 0
static T_BOOL BmpInfo_ext(TDB_ImgInfo* pImgInfo)
{
    BITMAPINFOHEADER *pbi = (BITMAPINFOHEADER*)(pImgInfo->data + sizeof(BITMAPFILEHEADER) );

#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "pbi->biWidth =%d,pbi->biHeight = %d,sizeof bitfilehead = %d", pbi->biWidth, pbi->biHeight, sizeof(BITMAPFILEHEADER));
#endif

    if (pbi->biWidth == 0 || pbi->biHeight == 0)
    {
        return AK_FALSE;
    }

    pImgInfo->width = (T_U16)pbi->biWidth;
    pImgInfo->height = (pbi->biHeight > 0) ? (T_U16)pbi->biHeight : (T_U16)(-pbi->biHeight);

#ifdef THUMB_Debug
    Fwl_Print(C3, M_THUMB, "BmpInfo width =%d,height = %d", pImgInfo->width, pImgInfo->height);
#endif
    return AK_TRUE;
}
#endif

/**
* @BRIEF get image info: width, height
* @PARAM pImgInfo: pointer of TDB_ImgInfo structure
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-25
* @UPDATE 2008-09-25
*/
static T_BOOL GetImgInfo(TDB_ImgInfo* pImgInfo)
{
    T_BOOL  bRet;
    T_U8    bitCount;

#ifdef THUMB_Debug
                    Fwl_Print(C3, M_THUMB, "111 pImgInfo->data=%d, pImgInfo->height=%d, pImgInfo->size=%d, pImgInfo->type=%d, pImgInfo->width=%d", 
                    pImgInfo->data,pImgInfo->height, pImgInfo->size, pImgInfo->type, pImgInfo->width);
#endif

    switch (pImgInfo->type)
    {
    case IMG_IMAGE_BMP:
        

//        bRet = BmpInfo_ext(pImgInfo);

#ifdef THUMB_Debug
         Fwl_Print(C3, M_THUMB, "aaa pImgInfo->data=%d, pImgInfo->height=%d, pImgInfo->size=%d, pImgInfo->type=%d, pImgInfo->width=%d, width_addr =%d, height_addr=%d", 
                            pImgInfo->data,pImgInfo->height, pImgInfo->size, pImgInfo->type, pImgInfo->width, &pImgInfo->width, &pImgInfo->height);
#endif

        bRet = BmpInfo(pImgInfo->data, &pImgInfo->width, &pImgInfo->height);

#ifdef THUMB_Debug
         Fwl_Print(C3, M_THUMB, "bb pImgInfo->data=%d, pImgInfo->height=%d, pImgInfo->size=%d, pImgInfo->type=%d, pImgInfo->width=%d, width_addr =%d, height_addr=%d", 
                            pImgInfo->data,pImgInfo->height, pImgInfo->size, pImgInfo->type, pImgInfo->width, &pImgInfo->width, &pImgInfo->height);
#endif

        break;
    case IMG_IMAGE_JPG:
        bRet = Img_JpegInfo(pImgInfo->data, pImgInfo->size,
                   &pImgInfo->width, &pImgInfo->height, AK_NULL, AK_NULL);
        break;
    case IMG_IMAGE_GIF:
        bRet = Img_GIFInfo(pImgInfo->data, &pImgInfo->width, &pImgInfo->height,
                   &bitCount);
        break;
    case IMG_IMAGE_PNG:
        bRet = Img_PNGInfo(pImgInfo->data, &pImgInfo->width, &pImgInfo->height,
                   &bitCount);
        break;
    default:
        return AK_FALSE;
    }

#ifdef THUMB_Debug
            Fwl_Print(C3, M_THUMB, " pImgInfo->data=%d, pImgInfo->height=%d, pImgInfo->size=%d, pImgInfo->type=%d, pImgInfo->width=%d", 
            pImgInfo->data,pImgInfo->height, pImgInfo->size, pImgInfo->type, pImgInfo->width);
#endif

    return bRet;
}

/**
* @BRIEF decode image into BMP format
* @PARAM pImgInfo: pointer of TDB_ImgInfo structure
* @PARAM bmpSize:  <out> size of BMP data
* @PARAM factor:   <out> scale factor
* @PARAM dstW:     <out> weight of thumbnail
* @PARAM dstH:     <out> height of thumbnail
* @PARAM phGif:    <out> pointer of GIF Handle
* @RETURN pointer to BMP data, AK_NULL if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-25
* @UPDATE 2008-10-13
*/
static T_U8* DecImage(TDB_ImgInfo* pImgInfo, T_U32* bmpSize, Fixed* factor,
                      T_U16* dstW, T_U16* dstH, T_hGIFENUM* phGif)
{
    T_U8*   bmpData = AK_NULL;

#ifdef OS_ANYKA
       //AK_Feed_Watchdog(0);
#endif

#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "width =%d, height = %d ,THUMBNAIL_HEIGHT=%d, THUMBNAIL_WIDTH= %d", pImgInfo->width, pImgInfo->height,THUMBNAIL_HEIGHT, THUMBNAIL_WIDTH);    
#endif

    // Calculate Destination Width & Height
    if ((pImgInfo->width <= THUMBNAIL_WIDTH) && (pImgInfo->height <= THUMBNAIL_HEIGHT))
    {
		*dstW = pImgInfo->width;
        *dstH = pImgInfo->height;
		*factor = IntToFixed(pImgInfo->width) / pImgInfo->width;
	}
    else if ((pImgInfo->width * THUMBNAIL_HEIGHT)
		> (pImgInfo->height * THUMBNAIL_WIDTH))
    {
        *factor = IntToFixed(THUMBNAIL_WIDTH) / pImgInfo->width;
        *dstW = (T_U16)THUMBNAIL_WIDTH;
        *dstH = (T_U16)ROUND(pImgInfo->height * (*factor));
		if (0 == *dstH)
		{
			*dstH = 1;
		}
		
        *factor = IntToFixed(pImgInfo->width) / THUMBNAIL_WIDTH ;
    }
    else
    {
        *factor = IntToFixed(THUMBNAIL_HEIGHT) / pImgInfo->height;
        *dstW = (T_U16)ROUND(pImgInfo->width * (*factor));
		if (0 == *dstW)
		{
			*dstW = 1;
		}
		
        *dstH = (T_U16)THUMBNAIL_HEIGHT;
        *factor = IntToFixed(pImgInfo->height) / THUMBNAIL_HEIGHT;
    }

#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "end of DecImage,dstW=%d, dstH=%d, factor=%d, pImgInfo->type=%d", *dstW , *dstH, *factor, pImgInfo->type);
#endif
    // Decode Image into BMP Format
    switch (pImgInfo->type)
    {
    case IMG_IMAGE_BMP:
        bmpData = pImgInfo->data;
        break;
		
    case IMG_IMAGE_WBMP:
        bmpData = Img_WBMP2BMP(pImgInfo->data, *bmpSize, bmpSize);
        break;
		
    case IMG_IMAGE_JPG:
        //bmpData = Img_JpegThumbnail(pImgInfo->data, bmpSize, dstW, dstH);
        bmpData = Img_Jpeg2BMPEx_Mutex(pImgInfo->data, *dstW, *dstH, bmpSize);
		if (AK_NULL == bmpData)
		{
			bmpData = Img_Jpeg2BMPExSoft(pImgInfo->data, *dstW, *dstH, bmpSize);
		}
		
       	*factor = ONE;
        break;
		
    case IMG_IMAGE_GIF:
        {
            T_U8    bitsPerPix;

            *phGif = GIFEnum_New(pImgInfo->data, pImgInfo->size);
            if (IMG_INVALID_HANDLE == *phGif)
            {
                return AK_NULL;
            }
			
            GIFEnum_FirstFrame(*phGif);
            bmpData = (T_U8*)GIFEnum_GetCurBMP(*phGif, bmpSize, &bitsPerPix);
            break;
        }
	
    case IMG_IMAGE_PNG:
		if (AK_NULL == (bmpData = Img_PNG2BMPEx(pImgInfo->data, bmpSize, *dstW, *dstH)))
			Fwl_Print(C3, M_THUMB, "Dec PNG Failure\n");
		
       	*factor = ONE;
        break;
		
    default:
        return AK_NULL;
    }

    return bmpData;
}

#if 0

/**
* @BRIEF scale & convert BMP to thumbnail(RGB565)
* @PARAM bmpData: pointer of BMP data
* @PARAM factor:  scale factor
* @PARAM dstW:    weight of thumbnail
* @PARAM dstH:    height of thumbnail
* @PARAM format:   TDB_RGB_565 / TDB_RGB_888
* @RETURN pointer to thumbnail, AK_NULL if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-25
* @UPDATE 2008-10-16
*/
static T_U8*  Bmp2Thumbnail_FullRect(T_U8* bmpData, Fixed factor,
                            T_U16 dstW, T_U16 dstH, T_U32 format)
{
    T_U8        *thumbnail = AK_NULL;
    T_S32        thumbSize = 0;
    T_U16        x, y;
    Fixed        srcCol, srcRow;
    T_U32        srcX, srcY;
    TDB_BmpInfo  bmpInfo;
    T_U8        *bmpLine = AK_NULL, *bmpPalette = AK_NULL;
    T_U8         R, G, B, *ptrThumb8;
    T_U16        RGB16, *ptrThumb16;
    T_U32        srcIdx;
    T_BOOL       bRet;
    Fixed        xBoundL = 0, yBoundT = 0; // Boundary: Left / Top

#ifdef THUMB_Debug
    Fwl_Print(C3, M_THUMB, "Bmp2Thumbnail dstW=%d ,dstH =%d, format =%d, factor=%d ",dstW, dstH, format, factor );
#endif
    // Initial TDB_BmpInfo Structure
    bRet = BmpInit(bmpData, &bmpInfo);

#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "bRet =%d, bmpInfo->height=%d, bmpInfo->width=%d, bmpInfo->lineBytes=%d",bRet, bmpInfo.height, bmpInfo.width, bmpInfo.lineBytes);
#endif

    if (!bRet)
    {
        return AK_NULL;
    }


    // Calculate Boundary
    if (THUMBNAIL_WIDTH != bmpInfo.width)
    {
        xBoundL = ((dstW - THUMBNAIL_WIDTH + 1) >> 1) * factor;
    }
    
    if (THUMBNAIL_HEIGHT != bmpInfo.height)
    {
        yBoundT = ((dstH - THUMBNAIL_HEIGHT + 1) >> 1) * factor;
    }

#ifdef THUMB_Debug
            Fwl_Print(C3, M_THUMB, "xBoundL=%d, yBoundT=%d", xBoundL, yBoundT);
#endif

    // Allocate Memory for Thumbnail
    thumbSize = (TDB_RGB_565 == format) ? TDB_THUMBSIZE_565: TDB_THUMBSIZE_888;
    thumbnail = (T_U8*)TDB_Malloc(thumbSize);
    
    if (AK_NULL == thumbnail)
    {
        return AK_NULL;
    }
    
    Utl_MemSet32(thumbnail, 0xff, thumbSize);

    // Scale & Convert BMP to Thumbnail
    srcRow = yBoundT;
    bmpPalette = bmpInfo.bmpPalette;
    if (TDB_RGB_565 == format)
    {
        ptrThumb16 = (T_U16*)thumbnail;
        
        for (y = 0; y < THUMBNAIL_HEIGHT; ++y)
        {
            srcCol = xBoundL;
            srcY = ROUND(srcRow);
            if (bmpInfo.bTopdown)
            {
                bmpLine = bmpInfo.bmpData + srcY * bmpInfo.lineBytes;
            }
            else
            {
                bmpLine = bmpInfo.bmpData + (bmpInfo.height - 1 - srcY) * bmpInfo.lineBytes;
            }
            for (x = 0; x < THUMBNAIL_WIDTH; ++x)
            {
                srcX = ROUND(srcCol);
                switch (bmpInfo.bmpFormat)
                {
                case IMG_BMP_32BIT:
                    srcX <<= 2;
                    B = bmpLine[srcX + 0];
                    G = bmpLine[srcX + 1];
                    R = bmpLine[srcX + 2];
                    RGB16 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                    break;
                case IMG_BMP_24BIT:
                    srcX *= 3;
                    B = bmpLine[srcX + 0];
                    G = bmpLine[srcX + 1];
                    R = bmpLine[srcX + 2];
                    RGB16 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                    break;
                case IMG_BMP_16BIT_565:
                    srcX <<= 1;
                    RGB16 = (bmpLine[srcX + 1] << 8) | bmpLine[srcX + 0];
                    break;
                case IMG_BMP_16BIT_555:
                    srcX <<= 1;
                    RGB16 = (bmpLine[srcX + 1] << 9) | ((bmpLine[srcX + 0] & 0xE0) << 1)
                          | (bmpLine[srcX + 0] & 0x1F);
                    break;
                case IMG_BMP_8BIT:
                    srcIdx = (T_U32)(bmpLine[srcX]) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    RGB16 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                    break;
                case IMG_BMP_4BIT:
                    srcX >>= 1;
                    srcIdx = (T_U32)((bmpLine[srcX] >> ((1 - (x & 0x1))>>2)) & 0xF) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    RGB16 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                    break;
                case IMG_BMP_1BIT:
                    srcX >>= 3;
                    srcIdx = (T_U32)((bmpLine[srcX] >> (7 - (x & 0x7))) & 0x1) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    RGB16 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                    break;
                }

                *ptrThumb16++ = RGB16;
                srcCol += factor;
            }
            srcRow += factor;
        }
    }
    else if (TDB_RGB_888 == format)
    {
        ptrThumb8 = thumbnail;
        for (y = 0; y < THUMBNAIL_HEIGHT; ++y)
        {
            srcCol = xBoundL;
            srcY = ROUND(srcRow);
            if (bmpInfo.bTopdown)
            {
                bmpLine = bmpInfo.bmpData + srcY * bmpInfo.lineBytes;
            }
            else
            {
                bmpLine = bmpInfo.bmpData + (bmpInfo.height - 1 - srcY) * bmpInfo.lineBytes;
            }
            for (x = 0; x < THUMBNAIL_WIDTH; ++x)
            {
                srcX = ROUND(srcCol);
                switch (bmpInfo.bmpFormat)
                {
                case IMG_BMP_32BIT:
                    srcX <<= 2;
                    B = bmpLine[srcX + 0];
                    G = bmpLine[srcX + 1];
                    R = bmpLine[srcX + 2];
                    break;
                case IMG_BMP_24BIT:
                    srcX *= 3;
                    B = bmpLine[srcX + 0];
                    G = bmpLine[srcX + 1];
                    R = bmpLine[srcX + 2];
                    break;
                case IMG_BMP_16BIT_565:
                    srcX <<= 1;
                    B = (bmpLine[srcX + 0] << 3);
                    G = (bmpLine[srcX + 1] << 5) | ((bmpLine[srcX + 0] >> 3) & ~3);
                    R = (bmpLine[srcX + 1] & ~7);
                    break;
                case IMG_BMP_16BIT_555:
                    srcX <<= 1;
                    B = (bmpLine[srcX + 0] << 3);
                    G = (bmpLine[srcX + 1] << 6) | ((bmpLine[srcX + 0] >> 2) & ~7);
                    R = (bmpLine[srcX + 1] << 1) & ~7;
                    break;
                case IMG_BMP_8BIT:
                    srcIdx = (T_U32)(bmpLine[srcX]) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    break;
                case IMG_BMP_4BIT:
                    srcX >>= 1;
                    srcIdx = (T_U32)((bmpLine[srcX] >> ((1 - (x & 0x1))>>2)) & 0xF) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    break;
                case IMG_BMP_1BIT:
                    srcX >>= 3;
                    srcIdx = (T_U32)((bmpLine[srcX] >> (7 - (x & 0x7))) & 0x1) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    break;
                }

                *ptrThumb8++ = R;
                *ptrThumb8++ = G;
                *ptrThumb8++ = B;
                srcCol += factor;
            }
            srcRow += factor;
        }
    }
    else
    {
        TDB_Free(thumbnail);
        return AK_NULL;
    }

    return thumbnail;
}
#endif
/**
* @BRIEF scale & convert BMP to thumbnail(RGB565)
* @PARAM bmpData: pointer of BMP data
* @PARAM factor:  scale factor
* @PARAM dstW:    weight of thumbnail
* @PARAM dstH:    height of thumbnail
* @PARAM format:   TDB_RGB_565 / TDB_RGB_888
* @RETURN pointer to thumbnail, AK_NULL if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-25
* @UPDATE 2008-10-16
*/
static T_U8*  Bmp2Thumbnail(T_U8* bmpData, Fixed factor,
                            T_U16 dstW, T_U16 dstH, T_U32 format)
{
    T_U8        *thumbnail = AK_NULL;
    T_S32        thumbSize = 0;
    T_U16        x, y;
    Fixed        srcCol, srcRow;
    T_U32        srcX, srcY;
    TDB_BmpInfo  bmpInfo;
    T_U8        *bmpLine = AK_NULL, *bmpPalette = AK_NULL;
    T_U8         R, G, B, *ptrThumb8;
    T_U16        RGB16, *ptrThumb16;
    T_U32        srcIdx;
    T_BOOL       bRet;
    Fixed        xBoundL = 0, yBoundT = 0; // Boundary: Left / Top

#ifdef THUMB_Debug
    Fwl_Print(C3, M_THUMB, "Bmp2Thumbnail dstW=%d ,dstH =%d, format =%d, factor=%d ",dstW, dstH, format, factor );
#endif
    // Initial TDB_BmpInfo Structure
    bRet = BmpInit(bmpData, &bmpInfo);

#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "bRet =%d, bmpInfo->height=%d, bmpInfo->width=%d, bmpInfo->lineBytes=%d",bRet, bmpInfo.height, bmpInfo.width, bmpInfo.lineBytes);
#endif

    if (!bRet)
    {
        return AK_NULL;
    }


    // Calculate Boundary
    if (THUMBNAIL_WIDTH != bmpInfo.width)
    {
        xBoundL = ((THUMBNAIL_WIDTH - dstW + 1) >> 1);
    }
    
    if (THUMBNAIL_HEIGHT != bmpInfo.height)
    {
        yBoundT = ((THUMBNAIL_HEIGHT-dstH + 1) >> 1);
    }

#ifdef THUMB_Debug
            Fwl_Print(C3, M_THUMB, "xBoundL=%d, yBoundT=%d", xBoundL, yBoundT);
#endif

    // Allocate Memory for Thumbnail
    thumbSize = (TDB_RGB_565 == format) ? TDB_THUMBSIZE_565: TDB_THUMBSIZE_888;
    thumbnail = (T_U8*)TDB_Malloc(thumbSize);
    
    if (AK_NULL == thumbnail)
    {
        return AK_NULL;
    }
    
    Utl_MemSet32(thumbnail, 0xff, thumbSize);

    // Scale & Convert BMP to Thumbnail
    srcRow = 0;
    bmpPalette = bmpInfo.bmpPalette;
    if (TDB_RGB_565 == format)
    {
        ptrThumb16 = (T_U16*)thumbnail;
        for (y = 0; y < THUMBNAIL_HEIGHT; ++y)
        {
			if(y<yBoundT || y>yBoundT+dstH-1)
    		{
				RGB16 = ((TDB_BKG_COLOR >> 3) << 11) | ((TDB_BKG_COLOR >> 2) << 5) | (TDB_BKG_COLOR >> 3);
				for (x = 0; x < THUMBNAIL_WIDTH; ++x)
				{
					*ptrThumb16++ = RGB16;
				}
				continue;
    		}
			srcCol = 0;
            srcY = ROUND(srcRow);
            if (bmpInfo.bTopdown)
            {
                bmpLine = bmpInfo.bmpData + srcY * bmpInfo.lineBytes;
            }
            else
            {
                bmpLine = bmpInfo.bmpData + (bmpInfo.height - 1 - srcY) * bmpInfo.lineBytes;
            }

			AK_ASSERT_PTR(bmpLine, "Bmp2Thumbnail() bmpLine err!", AK_NULL);
			
            for (x = 0; x < THUMBNAIL_WIDTH; ++x)
            {
            	if(x<xBoundL|| x>xBoundL+dstW-1)
        		{
					*ptrThumb16++ =((TDB_BKG_COLOR >> 3) << 11) | ((TDB_BKG_COLOR >> 2) << 5) | (TDB_BKG_COLOR >> 3);
					continue;						
        		}
				
                srcX = ROUND(srcCol);
                switch (bmpInfo.bmpFormat)
                {
                case IMG_BMP_32BIT:
                    srcX <<= 2;
                    B = bmpLine[srcX + 0];
                    G = bmpLine[srcX + 1];
                    R = bmpLine[srcX + 2];
                    RGB16 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                    break;
                case IMG_BMP_24BIT:
                    srcX *= 3;
                    B = bmpLine[srcX + 0];
                    G = bmpLine[srcX + 1];
                    R = bmpLine[srcX + 2];
                    RGB16 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                    break;
                case IMG_BMP_16BIT_565:
                    srcX <<= 1;
                    RGB16 = (bmpLine[srcX + 1] << 8) | bmpLine[srcX + 0];
                    break;
                case IMG_BMP_16BIT_555:
                    srcX <<= 1;
                    RGB16 = (bmpLine[srcX + 1] << 9) | ((bmpLine[srcX + 0] & 0xE0) << 1)
                          | (bmpLine[srcX + 0] & 0x1F);
                    break;
                case IMG_BMP_8BIT:
                    srcIdx = (T_U32)(bmpLine[srcX]) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    RGB16 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                    break;
                case IMG_BMP_4BIT:
                    srcX >>= 1;
                    srcIdx = (T_U32)((bmpLine[srcX] >> ((1 - (x & 0x1))>>2)) & 0xF) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    RGB16 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                    break;
                case IMG_BMP_1BIT:
                    srcX >>= 3;
                    srcIdx = (T_U32)((bmpLine[srcX] >> (7 - (x & 0x7))) & 0x1) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    RGB16 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                    break;
                }

                *ptrThumb16++ = RGB16;
                srcCol += factor;
            }
            srcRow += factor;
        }
    }
    else if (TDB_RGB_888 == format)
    {
        ptrThumb8 = thumbnail;
        for (y = 0; y < THUMBNAIL_HEIGHT; ++y)
        {
        	if(y<yBoundT || y>yBoundT+dstH-1)
    		{
    			for (x = 0; x < THUMBNAIL_WIDTH; ++x)
    			{
					*ptrThumb8++ = TDB_BKG_COLOR;
	                *ptrThumb8++ = TDB_BKG_COLOR;
	                *ptrThumb8++ = TDB_BKG_COLOR;
    			}
				continue;
    		}
			
            srcCol = 0;
            srcY = ROUND(srcRow);
            if (bmpInfo.bTopdown)
            {
                bmpLine = bmpInfo.bmpData + srcY * bmpInfo.lineBytes;
            }
            else
            {
                bmpLine = bmpInfo.bmpData + (bmpInfo.height - 1 - srcY) * bmpInfo.lineBytes;
            }

			AK_ASSERT_PTR(bmpLine, "Bmp2Thumbnail() bmpLine err!", AK_NULL);
			
            for (x = 0; x < THUMBNAIL_WIDTH; ++x)
            {
            	if(x<xBoundL|| x>xBoundL+dstW-1)
	    		{
					*ptrThumb8++ = TDB_BKG_COLOR;
	                *ptrThumb8++ = TDB_BKG_COLOR;
	                *ptrThumb8++ = TDB_BKG_COLOR;
					continue;
	    		}
                srcX = ROUND(srcCol);
                switch (bmpInfo.bmpFormat)
                {
                case IMG_BMP_32BIT:
                    srcX <<= 2;
                    B = bmpLine[srcX + 0];
                    G = bmpLine[srcX + 1];
                    R = bmpLine[srcX + 2];
                    break;
                case IMG_BMP_24BIT:
                    srcX *= 3;
                    B = bmpLine[srcX + 0];
                    G = bmpLine[srcX + 1];
                    R = bmpLine[srcX + 2];
                    break;
                case IMG_BMP_16BIT_565:
                    srcX <<= 1;
                    B = (bmpLine[srcX + 0] << 3);
                    G = (bmpLine[srcX + 1] << 5) | ((bmpLine[srcX + 0] >> 3) & ~3);
                    R = (bmpLine[srcX + 1] & ~7);
                    break;
                case IMG_BMP_16BIT_555:
                    srcX <<= 1;
                    B = (bmpLine[srcX + 0] << 3);
                    G = (bmpLine[srcX + 1] << 6) | ((bmpLine[srcX + 0] >> 2) & ~7);
                    R = (bmpLine[srcX + 1] << 1) & ~7;
                    break;
                case IMG_BMP_8BIT:
                    srcIdx = (T_U32)(bmpLine[srcX]) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    break;
                case IMG_BMP_4BIT:
                    srcX >>= 1;
                    srcIdx = (T_U32)((bmpLine[srcX] >> ((1 - (x & 0x1))>>2)) & 0xF) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    break;
                case IMG_BMP_1BIT:
                    srcX >>= 3;
                    srcIdx = (T_U32)((bmpLine[srcX] >> (7 - (x & 0x7))) & 0x1) << 2;
                    B = bmpPalette[srcIdx + 0];
                    G = bmpPalette[srcIdx + 1];
                    R = bmpPalette[srcIdx + 2];
                    break;
                }

                *ptrThumb8++ = R;
                *ptrThumb8++ = G;
                *ptrThumb8++ = B;
                srcCol += factor;
            }
            srcRow += factor;
        }
    }
    else
    {
        TDB_Free(thumbnail);
        return AK_NULL;
    }

    return thumbnail;
}

/**
* @BRIEF initial TDB_BmpInfo structure
* @PARAM bmpData:  pointer of BMP data
* @PARAM pBmpInfo: pointer of TDB_BmpInfo structure
* @RETURN pointer to thumbnail, AK_NULL if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-26
* @UPDATE 2008-09-26
*/
static T_BOOL BmpInit(T_U8* bmpData, TDB_BmpInfo* pBmpInfo)
{
    BITMAPFILEHEADER *pbf = (BITMAPFILEHEADER*)bmpData;
    BITMAPINFOHEADER *pbi = (BITMAPINFOHEADER*)(bmpData + sizeof(BITMAPFILEHEADER));
	T_U32 mask555RGB[3] = {0x7C00, 0x03e0, 0x001f};

    // Check whether Windows Bitmap or not

#ifdef THUMB_Debug
    Fwl_Print(C3, M_THUMB, "BmpInit pbi->biSize=%d, pBmpInfo->bitCount=%d,  pbi->biBitCount=%d", pbi->biSize, pBmpInfo->bitCount,  pbi->biBitCount);
#endif
    if (pbi->biSize != 0x28)
    {
        return AK_FALSE;
    }

    // Fill TDB_BmpInfo structure
    pBmpInfo->bitCount = pbi->biBitCount;
    pBmpInfo->width  = (T_U32)pbi->biWidth;
    pBmpInfo->height = (pbi->biHeight > 0)
                     ? (T_U32)pbi->biHeight : (T_U32)(-pbi->biHeight);
    pBmpInfo->bmpPalette = bmpData + sizeof(BITMAPFILEHEADER)
                                   + sizeof(BITMAPINFOHEADER);
    pBmpInfo->bmpData = bmpData + pbf->bfOffBits;
    pBmpInfo->bTopdown = (pbi->biHeight < 0) ? AK_TRUE : AK_FALSE;

    switch (pBmpInfo->bitCount)
    {
    case 1:
        pBmpInfo->lineBytes = (((T_U32)pbi->biWidth + 31) >> 5) << 2;
        pBmpInfo->bmpFormat = IMG_BMP_1BIT;
        break;
    case 4:
        pBmpInfo->lineBytes = ((((T_U32)pbi->biWidth << 2) + 31) >> 5) << 2;
        pBmpInfo->bmpFormat = IMG_BMP_4BIT;
        break;
    case 8:
        pBmpInfo->lineBytes = (((T_U32)pbi->biWidth + 3) >> 2) << 2;
        pBmpInfo->bmpFormat = IMG_BMP_8BIT;
        break;
    case 16:
        pBmpInfo->lineBytes = ((((T_U32)pbi->biWidth << 1) + 3) >> 2) << 2;
        if (BI_RGB == pbi->biCompression || 0 == memcmp(pBmpInfo->bmpPalette, mask555RGB, 12))
            pBmpInfo->bmpFormat = IMG_BMP_16BIT_555;
        else
            pBmpInfo->bmpFormat = IMG_BMP_16BIT_565;
        break;
    case 24:
        pBmpInfo->lineBytes = (((T_U32)pbi->biWidth * 3 + 3) >> 2) << 2;
        pBmpInfo->bmpFormat = IMG_BMP_24BIT;
        break;
    case 32:
        pBmpInfo->lineBytes = (T_U32)pbi->biWidth << 2;
        pBmpInfo->bmpFormat = IMG_BMP_32BIT;
        break;
    default:
        return AK_FALSE;
    }

    return AK_TRUE;
}

////////////////////////////////////////////////////////////////////////////////

/**
* @BRIEF get thumbnail box that using for display (RGB888)
* @PARAM pTDB_Handler: pointer of ThumbsDB Handler
* @PARAM filename:     image filename
* @RETURN index of thumbnail box in Cache, TDB_CACHE_INVALID_IDX if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-09-25
* @UPDATE 2008-10-16
*/
static T_S32 GetThumbnailBox(THUMBSDB_HANDLE hThumbsDB, ThumbsDB_Handle* pTDB_Handle, T_pCWSTR filename)
{
    T_U8   *thumbnail = AK_NULL;
    T_U8   *thumbBox  = AK_NULL;
    T_S32   thumbBoxIdx;
//    T_U16  *ptrImg = AK_NULL;
//    T_U8   *ptrBox = AK_NULL;
    T_U32   format, cache_mem_flag;

    // Search Cache for Thumbnail Box
    thumbBoxIdx = GetThumbBoxFromCache(pTDB_Handle, filename);

#ifdef THUMB_Debug
    Fwl_Print(C3, M_THUMB, "thumbBoxIdx =%d",thumbBoxIdx);
#endif

    if (TDB_CACHE_INVALID_IDX != thumbBoxIdx)
    {
        return thumbBoxIdx;
    }

    // Get Thumbnail Image from ThumbsDB File
    thumbnail = TDB_GetImage(hThumbsDB, pTDB_Handle, filename, &format);

#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "thumbnail =%d",thumbnail);
#endif

    
	if (AK_NULL == thumbnail)
    {
        return TDB_CACHE_INVALID_IDX;
    }

	cache_mem_flag = format&TDB_CACHE_MEM_BIT;
	format = format&(~TDB_CACHE_MEM_BIT);
    if (TDB_RGB_MODE == format)
    {
		thumbBox = thumbnail;
        
#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "thumbBox1 =%d",thumbBox);
#endif

        thumbBoxIdx = UpdateThumbBoxCache(pTDB_Handle, filename, thumbBox);

#ifdef THUMB_Debug
                Fwl_Print(C3, M_THUMB, "thumbBoxIdx =%d, cache_mem_flag=%d",thumbBoxIdx, cache_mem_flag);
#endif

		if(cache_mem_flag)
		{
			pTDB_Handle->cache[thumbBoxIdx].nFlag |= TDB_CACHE_MEM_BIT;
			pTDB_Handle->cacheMemCount++;
		}
    }
    else if (TDB_RGB_MODE_REVERSE == format)
    {
		T_U32 i,j;
        thumbBox = (T_U8*)TDB_Malloc(TDB_THUMBBOX_SIZE);
        if (AK_NULL == thumbBox)
        {
            TDB_Free(thumbnail);
            return TDB_CACHE_INVALID_IDX;
        }

#if (defined (LCD_MODE_565) && defined (OS_ANYKA))
		for (i=0,j=0; i<TDB_THUMBBOX_SIZE / 2; i += 3)
		{   
			// 888 to 565
			thumbBox[j++] = ((thumbnail[i+1] & 0x1C) << 3) | ((thumbnail[i+2] & 0xF8) >> 3);       // low 8 bit, G and B
			thumbBox[j++] = (thumbnail[i] & 0xF8) | ((thumbnail[i+1] & 0xE0) >> 5);        // high 8 bit, R and G
		}	
#else
		for (i=0,j=0; i<TDB_THUMBBOX_SIZE / 3; i++)
		{   
			//  565 to 888
			thumbBox[j++] = (thumbnail[i*2+1] & 0xF8) ;       // R
			thumbBox[j++] = ((thumbnail[i*2+1] & 0x07) <<5) |((thumbnail[i*2] & 0xE0) >> 3);        // G
			thumbBox[j++] = (thumbnail[i*2]&0x1F)<<3;       //B
		}
#endif

        thumbBoxIdx = UpdateThumbBoxCache(pTDB_Handle, filename, thumbBox);
 //       pTDB_Handle->cache[thumbBoxIdx].nFlag |= TDB_CACHE_MEM_BIT;
 //       pTDB_Handle->cacheMemCount++;
    }
    else
    {
    	if (AK_NULL != thumbnail)
        {
            TDB_Free(thumbnail);
            return TDB_CACHE_INVALID_IDX;
        }
        
        return TDB_CACHE_INVALID_IDX;
    }

    return thumbBoxIdx;
}

/**
* @BRIEF search cache for thumbnail box
* @PARAM pTDB_Handler: pointer of ThumbsDB Handler
* @PARAM filename: image filename
* @RETURN index of thumbnail box in cache, TDB_CACHE_INVALID_IDX if not found
* @AUTHOR zhang_chengyan
* @DATE 2008-09-26
* @UPDATE 2008-10-16
*/
static T_S32 GetThumbBoxFromCache(ThumbsDB_Handle* pTDB_Handle, T_pCWSTR filename)
{
    T_S32   i;
    TDB_CacheItem *pCache = pTDB_Handle->cache;

#ifdef THUMB_Debug
    T_U8    uzStrA[512], uzStrB[512];
#endif
    
    for (i = 0; i < TDB_CACHE_SIZE; ++i)
    {
#ifdef THUMB_Debug
        Eng_StrUcs2Mbcs(filename, uzStrA);
        Eng_StrUcs2Mbcs(pCache[i].filename, uzStrB);
        //Fwl_Print(C3, M_THUMB, "filename = %s, cache File =%s, i = %d, pCache[i].data=%d,result =%d", uzStrA, uzStrB, i, pCache[i].data, Utl_TStrCmp((T_pCWSTR)pCache[i].filename, filename) );
#endif
        if ((AK_NULL != pCache[i].data)
         && (Utl_TStrCmp((T_pCWSTR)pCache[i].filename, filename) == 0))
        {
            pCache[i].nTime = ++pTDB_Handle->cacheTimer;
            pCache[i].nFlag |= TDB_CACHE_SHOW_BIT;
            return i;
        }
    }

    return TDB_CACHE_INVALID_IDX;
}

/**
* @BRIEF search cache for thumbnail box
* @PARAM pTDB_Handler: pointer of ThumbsDB Handler
* @PARAM filename: image filename
* @PARAM data:     thumbnail box data
* @RETURN index of thumbnail box in cache(always valid)
* @AUTHOR zhang_chengyan
* @DATE 2008-09-26
* @UPDATE 2008-10-16
*/
static T_S32 UpdateThumbBoxCache(ThumbsDB_Handle* pTDB_Handle,
                                 T_pCWSTR filename, T_U8* data)
{
    T_S32   i;
    T_U32   minTime = ~0, minPos = ~0;
    TDB_CacheItem *pCache = pTDB_Handle->cache;

    // Flush Cached Thumbnail(Only in Memory) to ThumbsDB File
    if (pTDB_Handle->cacheMemCount >= TDB_FLUSH_ITEMS)
    {
        FlushThumbBoxCache(pTDB_Handle);
    }

    // Search Cache for Replace Position
    for (i = 0; i < TDB_CACHE_SIZE; ++i)
    {
        if (AK_NULL == pCache[i].data)
        {
            minPos = i;
            break;
        }
    }
    if (minPos == ~0)   // No Free Slot
    {
        for (i = 0; i < TDB_CACHE_SIZE; ++i)
        {
            if (!(pCache[i].nFlag & TDB_CACHE_SHOW_BIT)
             && !(pCache[i].nFlag & TDB_CACHE_MEM_BIT)
             && (minTime > pCache[i].nTime))
            {
                minTime = pCache[i].nTime;
                minPos = i;
            }
        }
    }

    // Update Thumbnail Box in Cache
    if (AK_NULL != pCache[minPos].data)
    {
        TDB_Free(pCache[minPos].data);
        pCache[minPos].data = AK_NULL;
    }
    pCache[minPos].data = data;
    pCache[minPos].nTime = ++pTDB_Handle->cacheTimer;
    pCache[minPos].nFlag |= TDB_CACHE_SHOW_BIT;
    Utl_TStrCpy(pCache[minPos].filename, filename);

    return minPos;
}

/**
* @BRIEF flush cached thumbnail(only in memory) to ThumbsDB file
* @PARAM pTDB_Handler: pointer of ThumbsDB Handler
* @RETURN AK_TRUE if success, AK_FALSE if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-10-16
* @UPDATE 2008-10-21
*/
static T_BOOL FlushThumbBoxCache(ThumbsDB_Handle* pTDB_Handle)
{
    T_U32   i, cnt;
    ThumbsDB_Header *pTDB_Header = pTDB_Handle->header;
    ThumbsDB_Item   *pTDB_Item = AK_NULL;
    TDB_CacheItem   *pCache = pTDB_Handle->cache;
    T_S32   fp = FS_INVALID_HANDLE;
    T_S32   arrIdx[TDB_FLUSH_ITEMS];
    T_U32   arrHash[TDB_FLUSH_ITEMS];
    T_U32   arrNextPos[TDB_FLUSH_ITEMS];
    T_U8   *itemData = AK_NULL, *ptrItem = AK_NULL;
    T_S32   itemOffset, itemSize;
	T_U64_INT freeSize = {0};

    if (pTDB_Handle->cacheMemCount == 0)
    {
        return AK_TRUE;
    }

	Fwl_FsGetFreeSize(pTDB_Handle->filename[0], &freeSize);

	if (U64cmpU32(&freeSize, (TDB_THUMBBOX_SIZE * pTDB_Handle->cacheMemCount + sizeof(ThumbsDB_Header))) <= 0) 
    {
		Fwl_Print(C3, M_THUMB, "disk space is not enough for write thumbs file!!!");
        return AK_FALSE;
    }

    // Initialization
    pTDB_Item = (ThumbsDB_Item*)((T_U8*)pTDB_Handle
              + sizeof(ThumbsDB_Handle) + sizeof(ThumbsDB_Header));
    Utl_MemSet32(pTDB_Item, 0, sizeof(ThumbsDB_Item));
    Utl_MemSet32(arrIdx, 0, sizeof(arrIdx));
    Utl_MemSet32(arrHash, 0, sizeof(arrHash));
    Utl_MemSet32(arrNextPos, 0, sizeof(arrNextPos));
    pTDB_Item->nFlag    = 0;
    pTDB_Item->Width    = THUMBNAIL_WIDTH;
    pTDB_Item->Height   = THUMBNAIL_HEIGHT;
    pTDB_Item->DataSize = TDB_THUMBBOX_SIZE;
    itemSize = (TDB_ITEM_PARAMSIZE + TDB_THUMBBOX_SIZE);

    // Open ThumbsDB File
    if (TDB_FS_CLOSE != pTDB_Handle->fileStatus)
    {
        TDB_Close(pTDB_Handle->fp);
        pTDB_Handle->fp = FS_INVALID_HANDLE;
        pTDB_Handle->fileStatus = TDB_FS_CLOSE;
    }
    fp = TDB_Open(pTDB_Handle->filename, FS_MODE_WRITE, FS_MODE_WRITE);
    if (FS_INVALID_HANDLE == fp)
    {
        return AK_FALSE;
    }

    // Get Index & Calculate Hash Value
    cnt = 0;
    for (i = 0; i < TDB_CACHE_SIZE; ++i)
    {
        if ((AK_NULL != pCache[i].data)
         && (pCache[i].nFlag & TDB_CACHE_MEM_BIT))
        {
            arrIdx[cnt] = i;
            arrHash[cnt] = HashString((T_U8*)pCache[i].filename);
            if (++cnt >= pTDB_Handle->cacheMemCount)
            {
                break;
            }
        }
    }

    // Insert into Free Space
    if (pTDB_Header->FreeItems != 0)
    {
        T_U32   startIdx, endIdx, curPos;

        // Calculate Start Index
        if (pTDB_Header->FreeItems >= pTDB_Handle->cacheMemCount)
        {
            startIdx = 0;
        }
        else
        {
            startIdx = pTDB_Handle->cacheMemCount - pTDB_Header->FreeItems;
        }
        endIdx = pTDB_Handle->cacheMemCount;

        // Allocate Memory Space
        itemData = (T_U8*)TDB_Malloc(itemSize);
        if (AK_NULL == itemData)
        {
        	TDB_Close(fp);
            return AK_FALSE;
        }

        for (i = startIdx; i < endIdx; ++i)
        {
            // Pick Up Free Item
            curPos = pTDB_Header->FreeHead;
            TDB_Seek(fp, curPos, _FSEEK_SET);
            TDB_Read(fp, &pTDB_Header->FreeHead, sizeof(T_S32));            

            // Link to Hash List
            if (pTDB_Header->HashTable[arrHash[i]] == 0)    // List Empty
            {
                arrNextPos[i] = 0;
                pTDB_Header->HashTable[arrHash[i]] = curPos;
            }
            else        // Insert into List Head
            {
                arrNextPos[i] = pTDB_Header->HashTable[arrHash[i]];
                pTDB_Header->HashTable[arrHash[i]] = curPos;
            }

            // Prepare Thumbnail Data
            PrepareThumbItem(&pCache[arrIdx[i]], pTDB_Item,
                             itemData, arrNextPos[i]);

            // Write Thumbnail Data into ThumbsDB File
            TDB_Seek(fp, curPos, _FSEEK_SET);
            TDB_Write(fp, itemData, itemSize);
            pTDB_Handle->cacheMemCount--;
            pTDB_Header->FreeItems--;
            pTDB_Header->Count++;
        }
        TDB_Free(itemData);
    }

    // Append into ThumbsDB
    if (pTDB_Handle->cacheMemCount != 0)
    {
        TDB_Seek(fp, 0L, _FSEEK_END);
        itemOffset = TDB_Tell(fp);

        // Link to Hash List
        for (i = 0; i < pTDB_Handle->cacheMemCount; ++i)
        {
            // !NOTE: ITEM MUST NOT EXIT IN DATABASE
            if (pTDB_Header->HashTable[arrHash[i]] == 0)    // List Empty
            {
                arrNextPos[i] = 0;
                pTDB_Header->HashTable[arrHash[i]] = itemOffset + i * itemSize;
            }
            else        // Insert into List Head
            {
                arrNextPos[i] = pTDB_Header->HashTable[arrHash[i]];
                pTDB_Header->HashTable[arrHash[i]] = itemOffset + i * itemSize;
            }
        }

        // Allocate Memory Space
        itemData = (T_U8*)TDB_Malloc(itemSize * pTDB_Handle->cacheMemCount);
        if (AK_NULL == itemData)
        {
        	TDB_Close(fp);
            return AK_FALSE;
        }

        // Prepare Thumbnail Data
        ptrItem = itemData;
        for (i = 0; i < pTDB_Handle->cacheMemCount; ++i)
        {
            PrepareThumbItem(&pCache[arrIdx[i]], pTDB_Item,
                             ptrItem, arrNextPos[i]);
            ptrItem += TDB_ITEM_PARAMSIZE + TDB_THUMBBOX_SIZE;
        }

        // Write Thumbnail Data into ThumbsDB File
        TDB_Seek(fp, 0L, _FSEEK_END);
        TDB_Write(fp, itemData, itemSize * pTDB_Handle->cacheMemCount);
        pTDB_Header->Count += pTDB_Handle->cacheMemCount;
        TDB_Free(itemData);
    }

    // Update Header of ThumbsDB File
    TDB_Seek(fp, 0L, _FSEEK_SET);
    TDB_Write(fp, pTDB_Header, sizeof(ThumbsDB_Header));
    TDB_Close(fp);

    // Clear MemCount
    pTDB_Handle->cacheMemCount = 0;
    return AK_TRUE;
}

/**
* @BRIEF create a thumbnail item from a cache data
* @PARAM pCacheItem: pointer of ThumbsDB Cache Item
* @PARAM pTDB_Item:  pointer of ThumbsDB Thumbnail Item
* @PARAM ptrItem:    pointer of buffer
* @PARAM nextPos:    pointer to next item's position
* @RETURN pointer to thumbnail image, AK_NULL if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-10-17
* @UPDATE 2008-10-17
*/
static T_VOID PrepareThumbItem(TDB_CacheItem* pCacheItem, ThumbsDB_Item* pTDB_Item,
                               T_U8* buf, T_U32 nextPos)
{
//    T_S32   x, y;
//    T_U8    R, G, B, *ptrRGB888;
//    T_U16  *ptrRGB565;
    
    // TDB_Item Parameters
    pTDB_Item->NextItem = nextPos;
    Utl_TStrCpy(pTDB_Item->Filename, pCacheItem->filename);
    Utl_MemCpy32(buf, pTDB_Item, TDB_ITEM_PARAMSIZE);
    
#if 0
    // Thumbnail: RGB888 ==> RGB565
    ptrRGB888 = pCacheItem->data;
    ptrRGB565 = (T_U16*)(buf + TDB_ITEM_PARAMSIZE);
    for (y = 0; y < THUMBNAIL_HEIGHT; ++y)
    {
        for (x = 0; x < THUMBNAIL_WIDTH; ++x)
        {
            R = *ptrRGB888++;
            G = *ptrRGB888++;
            B = *ptrRGB888++;
            *ptrRGB565++ = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
        }
    }
#endif /* 0 */

   	Utl_MemCpy32(buf + TDB_ITEM_PARAMSIZE, pCacheItem->data, TDB_THUMBBOX_SIZE);

    // Clear TDB_CACHE_MEM_BIT
    pCacheItem->nFlag &= ~TDB_CACHE_MEM_BIT;

    return;
}


/**
* @BRIEF obtain a thumbnail from ThumbsDB (differ from ThumbsDB_GetImage)
* @PARAM pTDB_Handler: pointer of ThumbsDB Handler
* @PARAM filename:     image filename(Unicode)
* @PARAM format:       <out> TDB_RGB_565 / TDB_RGB_888
* @RETURN pointer to thumbnail image, AK_NULL if failure
* @AUTHOR zhang_chengyan
* @DATE 2008-10-16
* @UPDATE 2008-10-16
*/
static T_U8*  TDB_GetImage(THUMBSDB_HANDLE hThumbsDB, ThumbsDB_Handle* pTDB_Handle, T_pCWSTR filename, T_U32* format)
{
    ThumbsDB_Header *pTDB_Header = pTDB_Handle->header;
    ThumbsDB_Item   *pTDB_Item = AK_NULL;
    T_S32   fp = FS_INVALID_HANDLE;
    T_U32   nHash;
    T_S32   prevPos, curPos;

    pTDB_Item = (ThumbsDB_Item*)((T_U8*)pTDB_Handle
              + sizeof(ThumbsDB_Handle) + sizeof(ThumbsDB_Header));
    Utl_MemSet32(pTDB_Item, 0, sizeof(ThumbsDB_Item));

    // Open ThumbsDB File
    if (TDB_FS_CLOSE == pTDB_Handle->fileStatus)
    {
        pTDB_Handle->fp = TDB_Open(pTDB_Handle->filename, _FMODE_READ, _FMODE_READ);
        if (FS_INVALID_HANDLE == pTDB_Handle->fp)   // ThumbsDB File Not Exist!
        {
            pTDB_Handle->fileStatus = TDB_FS_CLOSE;
            return AK_NULL;
        }
        pTDB_Handle->fileStatus = TDB_FS_OPEN_READ;
    }
    fp = pTDB_Handle->fp;

    // Calculate Hash Value
    nHash = HashString((T_U8*)filename);
    prevPos = curPos = pTDB_Header->HashTable[nHash];

    // Find the Item According to 'filename'
    if (!FindItem(hThumbsDB, fp, pTDB_Item, filename, &prevPos, &curPos))  // Thumbnail Not Exist!
    {
        T_U32   thumbDataSize;
        T_U16   thumbWidth, thumbHeight;

#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "get image1");
#endif

        // Create Thumbnail
        pTDB_Item->Data = CreateThumbnail(filename, &thumbDataSize,
                                         &thumbWidth, &thumbHeight, TDB_RGB_MODE);

#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "create  pTDB_Item->Data=%d, thumbDataSize=%d, thumbWidth=%d,thumbHeight=%d", pTDB_Item->Data,thumbDataSize,thumbWidth,thumbHeight);
#endif

        if (AK_NULL == pTDB_Item->Data)
        {
            pTDB_Item->Data = CreateThumbnail(NO_SUPPORT_PIC, &thumbDataSize,
                                    &thumbWidth, &thumbHeight, TDB_RGB_MODE);

#ifdef THUMB_Debug
        Fwl_Print(C3, M_THUMB, "22222create  pTDB_Item->Data=%d, thumbDataSize=%d, thumbWidth=%d,thumbHeight=%d", pTDB_Item->Data,thumbDataSize,thumbWidth,thumbHeight);
#endif
        }
        *format = TDB_RGB_MODE|TDB_CACHE_MEM_BIT;

        // !NOTE: KEEP FILE OPEN
        return pTDB_Item->Data;
    }

    pTDB_Item->Data = (T_U8*)TDB_Malloc(pTDB_Item->DataSize);
	AK_ASSERT_PTR(pTDB_Item->Data, "Malloc Failure", AK_NULL)

	TDB_Seek(fp, curPos + TDB_ITEM_PARAMSIZE, _FSEEK_SET);
	TDB_Read(fp, pTDB_Item->Data, pTDB_Item->DataSize);
	
    *format = TDB_RGB_MODE;

#ifdef THUMB_Debug
     Fwl_Print(C3, M_THUMB, "create3  pTDB_Item->Data=%d", pTDB_Item->Data);
#endif  

    // !NOTE: KEEP FILE OPEN
    return pTDB_Item->Data;
}

#endif
////////////////////////////////////////////////////////////////////////////////
