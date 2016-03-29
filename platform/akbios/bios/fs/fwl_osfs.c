#ifdef OS_WIN32
#include <windows.h>
#include <Windowsx.h>
#include "string.h"
#endif
#include "anyka_types.h"
#include "Gbl_Global.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osFS.h"
#include "hal_timer.h"

#include "hal_gpio.h"
#include "Eng_Debug.h"

#include "mount.h"
#include "file.h"
#include "fs.h"

#include "fha.h"



//#include "gpio_config.h"


#define FS_DBG(tips)printf("[FS] %s line %d \n",tips,__LINE__)

static T_pFILE  FsApi_FileOpen(T_pVOID path, T_FILE_MODE mode ,T_BOOL isUnicode, T_BOOL includeFolder)
{
    T_PFILE file;

    if (mode & FS_PO_CREAT)
    {
        mode = FILE_MODE_CREATE;
    }
    else if (mode & FS_PO_OVERLAY)
    {
        mode = FILE_MODE_OVERLAY;
    }
    else if (mode & FS_PO_APPEND)
    {
        mode = FILE_MODE_APPEND;
    }
    else if (mode & FS_PO_RDONLY)
    {
        mode = FILE_MODE_READ;
    }
    else
    {
        mode = FILE_MODE_APPEND;
    }

    if (isUnicode)
    {
        file = File_OpenUnicode(AK_NULL, (T_U16*)path, mode);

    }
    else
    {
        file = File_OpenAsc(AK_NULL, (T_U8*)path, mode);
    }
    
    if (file == AK_NULL)
    {
        return FS_INVALID_HANDLE;
    }

    if (mode != FILE_MODE_CREATE)
    {
        if (!File_Exist(file))
        {
            File_Close(file);
            return FS_INVALID_HANDLE;
        }
    }
    
    if ((!includeFolder) && (File_IsFolder(file)))
    {
        File_Close(file);
        return FS_INVALID_HANDLE;
    }

    return (T_S32)file;
}


T_pFILE Fwl_FileOpen(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE mode)
{
    T_pFILE pFile = FS_INVALID_HANDLE;

    if (AK_NULL != path)
    {
        pFile = FsApi_FileOpen(path, mode , AK_TRUE, AK_FALSE);
    }

    if (FS_INVALID_HANDLE == pFile)
    {
        AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FileOpen: %d.\n", pFile);
    }
    
    return pFile;
}

T_pFILE Fwl_FileOpenAsc(T_pSTR path, T_FILE_FLAG flag, T_FILE_MODE mode)
{
    T_pFILE pFile = FS_INVALID_HANDLE;

    if (AK_NULL != path)
    {
        pFile = FsApi_FileOpen(path, mode , AK_FALSE, AK_FALSE);
    }

    if (FS_INVALID_HANDLE == pFile)
    {
        AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FileOpen: %d.\n", pFile);
    }
    
    return pFile;
}


T_S32   Fwl_FileRead(T_pFILE hFile, T_pVOID buffer, T_U32 count)
{
    T_S32 ret = 0;

    if (FS_INVALID_HANDLE != hFile)
    {
        ret = File_Read((T_PFILE)hFile, buffer, count);
    }
    if (ret <= 0){
        AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FileRead: %d\n", ret);
    }

    return ret;
}

T_S32   Fwl_FileWrite(T_pFILE hFile, T_pCVOID buffer, T_U32 count)
{
    T_S32 ret = 0;

    if (FS_INVALID_HANDLE != hFile)
    {
        ret = File_Write((T_PFILE)hFile, buffer, count);
    }
    if (ret <= 0){
        AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FileWrite: %d\n", ret);
    }

    return ret;
}

T_BOOL  Fwl_FileClose(T_pFILE hFile)
{
    T_BOOL ret = AK_FALSE;

    if (FS_INVALID_HANDLE != hFile)
    {
        File_Close((T_PFILE)hFile);
        ret = AK_TRUE;
    }
    
    if (AK_FALSE == ret)
    {
        AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FileClose: %d\n", ret);
    }

    return ret;
}



T_BOOL  Fwl_FileDelete(T_pCWSTR path)
{
    T_BOOL ret = AK_FALSE;

    if (AK_NULL != path)
    {
        ret = File_DelUnicode(path);
    }
    
    if (AK_TRUE != ret)
    {
        AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FileDelete: %d\n", ret);
    }

    return ret;
}


T_U32   Fwl_GetFileLen(T_pFILE hFile)
{
    T_U32 low = 0;
    T_U32 excess;
    
    if (FS_INVALID_HANDLE != hFile)
    {
        low = File_GetLength((T_PFILE)hFile, &excess);
    }

    if (0 == low)
    {
        AK_DEBUG_OUTPUT("FWL FILE: Fwl_GetFileLen: %d\n", low);
    }
    
    return low;
}


//************************************************************************************
//#define FSAPIDBG(tips)printf("[FSAPI] %s line \n",tips)
#define FSAPIDBG(tips)

//#define FSAPIDBG_END(tips)printf("[FSAPI] %s line End \n",tips)
#define FSAPIDBG_END(tips)

//#define FSAPI_DBG_LINE(tips,file,line)printf("[FSAPI] %s: <%s> [line = %d]\n",tips,file,line)
#define FSAPI_DBG_LINE(tips,file,line)

static T_U32 FsApi_GetChipId(T_VOID)
{
    if(CHIP_3771_L == drv_get_chip_version())
    {
       return FS_AK37XXL;
    }
    else
    {
       return FS_AK37XX;
    }
}

static T_VOID FsApi_OutStream(T_U16 ch)
{
    T_U8 ch1;
    
    FSAPIDBG("FsApi_OutStream");
    ch1 = ch & 0xff;
    putch(ch1);
}

static T_U8 FsApi_InStream(T_VOID)
{
    T_U8 ch;
    
    FSAPIDBG("FsApi_InStream");
    ch = getch() & 0xff;
    return ch;
}


static T_U32 FsApi_GetSecond(T_VOID)
{
    FSAPIDBG("FsApi_GetSecond");

    return 0;
}


static T_VOID FsApi_SetSecond(T_U32 seconds)
{
    FSAPIDBG("FsApi_SetSecond");
    return;
}

static T_S32 FsApi_UniStr2AnsiStr(const T_U16 *pUniStr, T_U32 UniStrLen,T_pSTR pAnsibuf, T_U32 AnsiBufLen, T_U32 code)
{
    T_U32 i = 0;

    FSAPIDBG("Unichar2Ansi");
    if (pUniStr != AK_NULL)
    {
        while (i != UniStrLen)
        {
            pAnsibuf[i] = (T_U8)pUniStr[i];
            i++;
        }
        pAnsibuf[i] = 0;
    }
    
    FSAPIDBG_END("Unichar2Ansi");
    
    return UniStrLen;
}


static T_S32 FsApi_AnsiStr2UniStr(const T_pSTR pAnsiStr, T_U32 AnsiStrLen,T_U16 *pUniBuf, T_U32 UniBufLen, T_U32 code)
{
    T_U32 i = 0;
    
    FSAPIDBG("Ansi2Unichar");
    
    if (pUniBuf != AK_NULL)
    {
        while (i != AnsiStrLen)
        {
            pUniBuf[i] = (T_U16)pAnsiStr[i];
            i++;
        }
        pUniBuf[i] = 0;
    }
    
    FSAPIDBG_END("Ansi2Unichar");

    return AnsiStrLen;
}


static T_pVOID    FsApi_Alloc(T_U32 size, T_S8 *filename, T_U32 fileline)
{
    T_pVOID ptr = AK_NULL;
    
    FSAPI_DBG_LINE("FsApi_Alloc",filename,fileline);
    
    //printf("[FSAPI] %s size = %d \n","FsApi_Alloc",size);
    ptr =  (T_pVOID)Fwl_Malloc(size);
    
    FSAPIDBG_END("FsApi_Alloc");
    return ptr;
}


static T_pVOID FsApi_Realloc(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline)
{
    T_pVOID ptr = AK_NULL;
    
    FSAPI_DBG_LINE("FsApi_Realloc",filename,fileline);
    
    ptr = (T_pVOID)Fwl_ReMalloc(var, size);
    
    FSAPIDBG_END("FsApi_Realloc");
    
    return ptr;
}

static T_pVOID  FsApi_Free(T_pVOID var, T_S8 *filename, T_U32 fileline)
{
    T_pVOID ptr = AK_NULL;
    
    FSAPI_DBG_LINE("FsApi_Free",filename,fileline);
    
    ptr = (T_pVOID)Fwl_Free(var);

    FSAPIDBG_END("FsApi_Free");
    return ptr;
}

static T_S32 FsApi_Create_Semaphore(T_U32 initial_count, T_U8 suspend_type, T_S8 *filename, T_U32 fileline)
{
    FSAPI_DBG_LINE("FsApi_Create_Semaphore",filename,fileline);
    return 0;
}

static T_S32 FsApi_Delete_Semaphore(T_S32 semaphore, T_S8 *filename, T_U32 fileline)
{
    FSAPI_DBG_LINE("FsApi_Delete_Semaphore",filename,fileline);
    return 0;
}
static T_S32 FsApi_Obtain_Semaphore(T_S32 semaphore, T_U32 suspend, T_S8 *filename, T_U32 fileline)
{
    FSAPI_DBG_LINE("FsApi_Obtain_Semaphore",filename,fileline);
    return 0;
}
static T_S32 FsApi_Release_Semaphore(T_S32 semaphore, T_S8 *filename, T_U32 fileline)
{
    FSAPI_DBG_LINE("FsApi_Release_Semaphore",filename,fileline);
    return 0;
}

static     T_VOID FsApi_SysReset(T_VOID)
{
    FSAPIDBG("FsApi_SysReset");
    return;
}

static T_VOID FsApi_RandSeed(T_VOID)
{
    FSAPIDBG("FsApi_RandSeed");
    srand(0);
}

static T_U32 FsApi_Rand(T_U32 MaxVal)
{
    T_U32 val = 0;
    
    FSAPIDBG("FsApi_Rand");
    val = (T_U32)rand() % MaxVal;

    return val;
}

static T_VOID FsApi_SleepMs(T_U32 ms)
{
    FSAPIDBG("FsApi_SleepMs");
    return;
}

static T_S32   FsApi_Printf(T_pCSTR s, ...)
{
    FSAPIDBG("FsApi_Printf");
    return 0;
}



static T_BOOL   Fwl_InitFsCb(T_S32 InitMode)
{
    T_FSCallback    fsGlbCb = {0};

    fsGlbCb.out = FsApi_OutStream;
    fsGlbCb.in = FsApi_InStream; 
    fsGlbCb.fGetSecond = FsApi_GetSecond;
    fsGlbCb.fSetSecond = FsApi_SetSecond;
    
    fsGlbCb.fUniToAsc = FsApi_UniStr2AnsiStr;
    fsGlbCb.fAscToUni = FsApi_AnsiStr2UniStr;

    fsGlbCb.fRamAlloc = FsApi_Alloc;
    fsGlbCb.fRamRealloc = FsApi_Realloc;
    fsGlbCb.fRamFree = FsApi_Free;
    
    fsGlbCb.fCrtSem    = FsApi_Create_Semaphore;
    fsGlbCb.fDelSem    = FsApi_Delete_Semaphore;
    fsGlbCb.fObtSem    = FsApi_Obtain_Semaphore;
    fsGlbCb.fRelSem    = FsApi_Release_Semaphore;
    
    (T_pVOID)fsGlbCb.fMemCpy = (T_pVOID)memcpy;
    (T_pVOID)fsGlbCb.fMemSet = (T_pVOID)memset;
    (T_pVOID)fsGlbCb.fMemMov = (T_pVOID)memmove;
    (T_pVOID)fsGlbCb.fMemCmp = (T_pVOID)memcmp;

    if (InitMode)
    {
        fsGlbCb.fPrintf =  printf;
    }
    else
    {
        fsGlbCb.fPrintf = FsApi_Printf;
    }

    fsGlbCb.fGetChipId  = FsApi_GetChipId;


    fsGlbCb.fSysRst= FsApi_SysReset;
    fsGlbCb.fRandSeed    = FsApi_RandSeed;
    fsGlbCb.fGetRand = FsApi_Rand;

    fsGlbCb.fMountThead = AK_NULL;
    fsGlbCb.fKillThead    = AK_NULL;
    fsGlbCb.fSystemSleep = FsApi_SleepMs;

    
    return FS_InitCallBack(&fsGlbCb, 128);
}


static T_BOOL CheckLibVersion(T_VOID)
{    
    typedef struct
    {
        T_U8 *pLibName;
        T_U8 *(*pVerFun)(T_VOID);
    }T_VERSION_INFO;

    T_VERSION_INFO version_info[] = 
    {
        {VER_NAME_DRV,  drvlib_version},
        {VER_NAME_FS,   FSLib_GetVersion},
        {VER_NAME_MOUNT,FS_GetVersion},
        {VER_NAME_MTD,  MtdLib_GetVersion},
        {VER_NAME_FHA,  FHA_get_version}   
    }; 
    
    T_LIB_VER_INFO Lib_version[sizeof(version_info)/sizeof(version_info[0])];
    T_U32 i;
    T_U32 uRet;

    for(i = 0; i < sizeof(version_info)/sizeof(version_info[0]); ++i)
    {
        strncpy(Lib_version[i].lib_name,version_info[i].pLibName,
        sizeof(Lib_version[i].lib_name));
        strncpy(Lib_version[i].lib_version,version_info[i].pVerFun(),
        sizeof(Lib_version[i].lib_version));
        uRet = FHA_check_lib_version(&(Lib_version[i]));
        if(FHA_FAIL == uRet)
        {
            printf("%s no mathc\n", Lib_version[i].lib_name);
            printf("%s->%s\n", Lib_version[i].lib_name, 
            Lib_version[i].lib_version);

            return AK_FALSE;
        }
    }

    return AK_TRUE;

}




T_BOOL Fwl_InitFs(T_VOID)
{

    Fwl_Print(C3, M_INIT, "init fat fs");
    
    printf("Fwl_InitFs Fwl_InitFsCb\n");
    Fwl_InitFsCb(1);
    printf("Fwl_Initfha Nand_MountInit\n");
#ifdef NANDBOOT
    if (!Fwl_FhaInit())
    {
        return AK_FALSE;
    }
    printf("Fwl_InitFs Nand_MountInit\n");

    Nand_MountInit();
        
    if (!CheckLibVersion())
    {
         printf("Check Lib Version Fail\n");
         while(1);
    }
#else
    if (!Fwl_Sd_MountSdBootCard())
    {
        printf("mount sd boot card fail\n");
        while(1);
    }
#endif
    printf("Fwl_InitFs End\n");
    return AK_TRUE;
}


T_VOID Fwl_DeInitFs(T_VOID)
{
    FS_Destroy();
    Nand_DestoryFs();
    return;
}


/* end of file */

