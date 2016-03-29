
/******************************************
 * Copyright (c) 2008, Anyka Co., Ltd. 
 * All rights reserved.
 * @author he_ying_gz
 * @date 2008-3-14
 ******************************************/

#include "Lib_res_port.h"
#include "Lib_state.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osFS.h"
#include "Eng_DataConvert.h"
#include "Gbl_Global.h"
#include "hal_print.h"
#include "Eng_debug.h"
#include "Lib_state_api.h"
#include "Eng_imgConvert.h"
#include "Eng_ImgDec.h"
#include "fwl_spiflash.h"
#include "Fwl_ImageLib.h"
#include "fwl_display.h"

typedef struct
{
    T_U32 offset;
    T_U32 length;
} T_RES_INDEX, *T_pRES_INDEX;   // resource entry index

typedef struct
{
    T_U32 typeNumber;
    T_RES_INDEX indexs[eRES_TYPE_NUM];
} T_RES_TYPE_INFO, *T_pRES_TYPE_INFO;

typedef struct
{
    T_U32 languageNumber;
    T_RES_INDEX indexs[eRES_LANG_NUM];
} T_RES_LANGUAGE_INFO, *T_pRES_LANGUAGE_INFO;

typedef struct
{
    T_U32 stringNumber;
    // NOTE: the offset of entry_index of string  should relative offset !!!!
    T_RES_INDEX indexs[eRES_STR_NUM];
} T_RES_STRING_INFO, *T_pRES_STRING_INFO;

typedef struct
{
    T_U32 binaryNumber;
    T_RES_INDEX indexs[eRES_BINARY_NUM];
} T_RES_BINARY_INFO, *T_pRES_BINARY_INFO;

typedef struct
{
#ifdef SPIBOOT
     T_FILE_CURRENT *fd;
#else
    T_hFILE fd;
#endif
    T_RES_TYPE_INFO typeInfo;
    T_RES_LANGUAGE_INFO languageInfo;
    T_RES_BINARY_INFO binaryInfo;
    T_RES_STRING_INFO stringInfo;
    T_pDATA stringBuf;         // string is not dynamic loading
    T_U32 stringBufSize;
} T_RES_INFO, *T_pRES_INFO;

static T_pRES_INFO g_pResInfo = AK_NULL;

T_BOOL Res_ChkOpenBackFile(T_VOID)
{
#ifndef SPIBOOT //nand and sd boot

	T_BOOL ret = AK_FALSE;

	ret = Fwl_ChkOpenBackFile(RES_FILE_PATH, RES_FILE_PATH_BAK);
    if (!ret)
    {
        AK_DEBUG_OUTPUT("Res_Init: Fwl_ChkOpenBackFile is fail\n");
    }
    
    return ret;
#endif

	return AK_FALSE;
}

T_BOOL Res_Init(T_VOID)
{
    T_RES_CB_FUNS resCB;
	//T_BOOL ret = AK_FALSE;

    AK_ASSERT_VAL(g_pResInfo == AK_NULL, "g_pResInfo != null\n", AK_FALSE);   // init twice ???

    g_pResInfo = Fwl_Malloc(sizeof(T_RES_INFO));
    if (AK_NULL == g_pResInfo)
    {
        AK_DEBUG_OUTPUT("Fwl_Malloc fail in Res_Init\n");
        return AK_FALSE;
    }

    //init
    memset(g_pResInfo, 0, sizeof(T_RES_INFO));
    g_pResInfo->stringBuf = AK_NULL;
#ifndef SPIBOOT
    g_pResInfo->fd = Fwl_FileOpenAsc(RES_FILE_PATH, _FMODE_READ, _FMODE_READ);

    if (FS_INVALID_HANDLE == g_pResInfo->fd)
    {
        AK_DEBUG_OUTPUT("open file fail in Res_Init\n");
        goto ERROR;
    }
#else
    g_pResInfo->fd = Fwl_SPI_FileOpen(RES_FILE_PATH);
    if (AK_NULL == g_pResInfo->fd)
    {
        AK_DEBUG_OUTPUT("open file fail in Res_Init\n");
        goto ERROR;
    }
#endif

    FWL_FILE_READ(g_pResInfo->fd, &g_pResInfo->typeInfo, sizeof(g_pResInfo->typeInfo));
    if (eRES_TYPE_NUM != g_pResInfo->typeInfo.typeNumber)
    {
        AK_DEBUG_OUTPUT("damaged bin file ??? eRES_TYPE_NUM != typeNumber\n");
        goto ERROR;
    }

    // init string resource 
    // NOTE string is not  dynamic loading

    // because languageInfo is fallow by typeInfo immediately, 
    // omit seeking to eRES_TYPE_STRING

    FWL_FILE_READ(g_pResInfo->fd, &g_pResInfo->languageInfo, sizeof(g_pResInfo->languageInfo));
    if (eRES_LANG_NUM != g_pResInfo->languageInfo.languageNumber)
    {
        AK_DEBUG_OUTPUT("damaged bin file ??? eRES_LANG_NUM != languageNumber\n");
        goto ERROR;
    }
    
    if (AK_FALSE == Res_SetLanguage(gs.Lang))
    {
        AK_DEBUG_OUTPUT("Res_SetLanguage fail\n");
        goto ERROR;
    }

    // init binary resource

    FWL_FILE_SEEK(g_pResInfo->fd, g_pResInfo->typeInfo.indexs[eRES_TYPE_BINARY].offset, _FSEEK_SET);
    FWL_FILE_READ(g_pResInfo->fd, &g_pResInfo->binaryInfo, sizeof(g_pResInfo->binaryInfo));
    if (g_pResInfo->binaryInfo.binaryNumber != eRES_BINARY_NUM)
    {
        AK_DEBUG_OUTPUT("damaged bin file ??? binaryNumber != eRES_BINARY_NUM\n");
        AK_DEBUG_OUTPUT("read binary num = %d, right num is %d\n", g_pResInfo->binaryInfo.binaryNumber, eRES_BINARY_NUM);
        goto ERROR;
    }
    
    //initialize  callback function
    resCB.malloc = (T_RES_CALLBACK_FUN_MALLOC) Fwl_MallocAndTrace;
    resCB.remalloc = (T_RES_CALLBACK_FUN_REMALLOC) Fwl_FreeAndTrace;
    resCB.free = (T_RES_CALLBACK_FUN_FREE) Fwl_FreeAndTrace;
    resCB.printf = (T_RES_CALLBACK_FUN_PRT_S32F) AkDebugOutput;

    if (AK_FALSE == Res_Initial((T_WCHR*)RES_FILE_PATH, MAX_DYNAMIC_CACHE_SIZE, &resCB))
    {
        AK_DEBUG_OUTPUT("Res_Initial fail!!!\n");
        goto ERROR;
    }

    //register callback functions       
    m_addPushFunc(gb_resEnterStateMachine, 0);
    m_addPopFunc(gb_resLeaveStateMachine, 0);

    return AK_TRUE;

ERROR:
#ifndef SPIBOOT
    if (g_pResInfo->fd != FS_INVALID_HANDLE)
    {
        Fwl_FileClose(g_pResInfo->fd);   
        g_pResInfo->fd = FS_INVALID_HANDLE; // nonsense
    }
#else
    if (g_pResInfo->fd != AK_NULL)
    {
        Fwl_SPI_FileClose(g_pResInfo->fd);	 
        g_pResInfo->fd = AK_NULL; // nonsense
    }
#endif
    if (g_pResInfo->stringBuf != AK_NULL)
    {
        Fwl_Free(g_pResInfo->stringBuf);
        g_pResInfo->stringBuf = AK_NULL;
    }
    if (g_pResInfo != AK_NULL)
    {
        Fwl_Free(g_pResInfo);
        g_pResInfo = AK_NULL;
    }
    
    return AK_FALSE;
}

T_BOOL Res_SetLanguage(T_RES_LANGUAGE language)
{
    T_U32 newBufSize;
    T_U32 size;
    AK_ASSERT_PTR(g_pResInfo, "g_pResInfo == null", AK_FALSE);
    AK_ASSERT_VAL(language < eRES_LANG_NUM, "language invalid", AK_FALSE);

    FWL_FILE_SEEK(g_pResInfo->fd, g_pResInfo->languageInfo.indexs[language].offset, _FSEEK_SET);
    size=FWL_FILE_READ(g_pResInfo->fd, &g_pResInfo->stringInfo, sizeof(g_pResInfo->stringInfo));
    
    if (eRES_STR_NUM != g_pResInfo->stringInfo.stringNumber)
    {
        AK_DEBUG_OUTPUT("stringNumber : %d\nsizeof(g_pResInfo->stringInfo)=%d\n", g_pResInfo->stringInfo.stringNumber,sizeof(g_pResInfo->stringInfo));
        AK_DEBUG_OUTPUT("damaged file ??? eRES_STR_NUM != stringNumber\n");
        return AK_FALSE;
    }
    newBufSize = g_pResInfo->languageInfo.indexs[language].length - sizeof(g_pResInfo->stringInfo);
    if (g_pResInfo->stringBufSize < newBufSize)
    {
        T_pDATA newBuf;
        newBuf = Fwl_Malloc(newBufSize);
        if (AK_NULL == newBuf)
        {
            return AK_FALSE;
        }
        if (g_pResInfo->stringBuf != AK_NULL)
        {
            Fwl_Free(g_pResInfo->stringBuf);
        }
        g_pResInfo->stringBuf = AK_NULL;
        g_pResInfo->stringBuf = newBuf;
        g_pResInfo->stringBufSize = newBufSize;
    }

    FWL_FILE_READ(g_pResInfo->fd, g_pResInfo->stringBuf, g_pResInfo->languageInfo.indexs[language].length - sizeof(g_pResInfo->stringInfo));
    return AK_TRUE;
}

T_pCWSTR Res_GetStringByID(T_RES_STRING id)
{
    AK_ASSERT_PTR(g_pResInfo, "g_pResInfo == null", AK_NULL);
    AK_ASSERT_VAL(id < eRES_STR_NUM,"id < eRES_STR_NUM" ,AK_NULL);
    return (T_pCWSTR) (g_pResInfo->stringBuf + g_pResInfo->stringInfo.indexs[id].offset- sizeof(g_pResInfo->stringInfo));
}

T_VOID Res_Free(T_VOID)
{
    AK_ASSERT_PTR_VOID(g_pResInfo, "g_pResInfo == null");
#ifndef SPIBOOT
    Fwl_FileClose(g_pResInfo->fd);
    g_pResInfo->fd = FS_INVALID_HANDLE; // nonsense
#else
    Fwl_SPI_FileClose(g_pResInfo->fd);
    g_pResInfo->fd = AK_NULL; // nonsense
#endif
    Fwl_Free(g_pResInfo);
    g_pResInfo = AK_NULL;
}

extern void *GetHdrResourceEleInfo(const T_WCHR * pthFile)
{
    AK_ASSERT_PTR(g_pResInfo, "g_pResInfo is null", AK_NULL);
    return g_pResInfo;
}

extern unsigned char GetAnyResourceEleInfo(void *hdrInfo, unsigned long resID, unsigned long *startPos, unsigned long *szData)
{
    T_pRES_INFO pResInfo = (T_pRES_INFO) hdrInfo;

    AK_ASSERT_PTR(hdrInfo, "GetAnyResourceEleInfo hdrInfo", AK_FALSE);
    AK_ASSERT_PTR(startPos, "GetAnyResourceEleInfo startPos", AK_FALSE);
    AK_ASSERT_PTR(szData, "GetAnyResourceEleInfo szData", AK_FALSE);
    AK_ASSERT_VAL(resID < eRES_BINARY_NUM, "GetAnyResourceEleInfo resID", AK_FALSE);

    *startPos = pResInfo->binaryInfo.indexs[resID].offset;
    *szData = pResInfo->binaryInfo.indexs[resID].length;
    return AK_TRUE;
}

extern unsigned long GetAnyResourceEleData(void *hdrInfo, unsigned char *mem, unsigned long startPos, unsigned long szData)
{
    T_pRES_INFO pResInfo = (T_pRES_INFO) hdrInfo;
    T_U32 sz;
	T_U32 offset,leng;
    AK_ASSERT_PTR(hdrInfo, "", 0);
    AK_ASSERT_PTR(mem, "", 0);
	
	offset = (T_U32)startPos;
	leng = (T_U32)szData;
    FWL_FILE_SEEK(pResInfo->fd, offset, _FSEEK_SET);
    sz = FWL_FILE_READ(pResInfo->fd, mem, leng);

    if (sz < szData)
    {
        AK_DEBUG_OUTPUT("warning Fwl_FileRead sz < szData");
    }

    return sz;
}



/*********************资源动态加载库里使用该接口**************************/
T_pDATA  Res_PngToBmp(T_pDATA pPngData, T_U32 *len)
{
	T_pDATA pBmp = AK_NULL;
	T_pDATA pBuf = AK_NULL;
	T_U16 width = 0;
	T_U16 height = 0;
	T_U32 num = 0;
	T_U32 scale = 0;
	T_U32 size;
	T_U8 bitCount;
	
	if (AK_NULL == pPngData)
	{
		*len = 0;
		return AK_NULL;
	}
	
	Img_PNGInfo((T_U8 *)pPngData, &width, &height, &bitCount);

	pBmp = Img_PNG2BMP((T_U8 *)pPngData, &size);
	if (AK_NULL == pBmp)
	{
		*len = 0;
		return AK_NULL;
	}

	if (32 != bitCount) //不带alpha通道的png，则进一步转为RGB565 BMP图像格式
	{
#ifdef OS_ANYKA
		size = height*((BYTES_PER_PIXEL*width+3)>>2<<2) + 8;
		pBuf = Fwl_Malloc(size);
#else
		size = height*((2*width+3)>>2<<2) + 8;
		pBuf = Fwl_Malloc(size);
#endif

		if (AK_NULL == pBuf)
		{
			Fwl_Free(pBmp);
			*len = 0;
			return AK_NULL;
		}
		
		num = FillAkBmpHead(pBuf, width, height);

#ifdef OS_ANYKA
		GetBMP2RGBBuf(pBmp, pBuf+num, width, height, 0, 0, 100, 0, &scale, COLOR_BLACK);
#else
		GetBMP2RGBBuf565(pBmp, pBuf+num, width, height, 0, 0, 100, 0, &scale, COLOR_BLACK);
#endif

		Fwl_Free(pBmp);

		*len = size;
		return pBuf;
	}
	else				//否则
	{
		*len = size;
		return pBmp;
	}

	*len = 0;
	return AK_NULL;
}


