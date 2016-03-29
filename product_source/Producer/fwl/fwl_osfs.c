#include "Fwl_osFS.h"
#include <string.h>
#include "Fwl_osMalloc.h"
#include "fs.h"
#include "fsa.h"

//**************************************************************************************************************************
T_hFILE Fwl_FileOpen(T_pCSTR path, T_FILE_MODE mode)
{
    T_PFILE file;
    T_U32 i, len;

    len = strlen(path);
    for (i=len; i !=0; i--)
    {
        if (path[i] == '/' || path[i] == '\\')
            break;
    }

    if (i != 0 && path[i-1] != ':')
    {
        T_U8 buf[300]; 

        if (i > 300)
            return FS_INVALID_HANDLE;

        memcpy(buf, path, 300);
        buf[i] = 0;

        if (!File_MkdirsAsc(buf))
        {
            return FS_INVALID_HANDLE;
        }
    }
    
    file = File_OpenAsc(AK_NULL, path, mode);

    if (!File_IsFile(file))
    {
        File_Close(file);
        
        printf("FWL FILE ERROR: Fwl_FileOpen: \n");
        return FS_INVALID_HANDLE;
    }

    return (T_hFILE)file;
}

T_VOID  Fwl_FileClose(T_hFILE hFile)
{
    if (hFile != FS_INVALID_HANDLE)
    {
        File_Close((T_PFILE)hFile);
    }
}

T_U32   Fwl_FileSeek(T_hFILE hFile, T_S32 offset, T_U16 origin)
{
   T_U32 point;

    if (hFile == FS_INVALID_HANDLE )
    {
        return FS_INVALID_SEEK;
    }

    return File_SetFilePtr((T_PFILE)hFile, offset, origin);    
}

T_U32   Fwl_FileRead(T_hFILE hFile, T_pVOID buffer, T_U32 count)
{
    T_U32 ret = 0;

    if (hFile != FS_INVALID_HANDLE)
    {     
        ret = File_Read((T_PFILE)hFile, (T_U8*)buffer, count);
    }
    
    return ret;
}

T_U32   Fwl_FileWrite(T_hFILE hFile, T_pCVOID buffer, T_U32 count)
{
    T_U32 ret = 0;

    if (hFile != FS_INVALID_HANDLE)
    {
        ret = File_Write((T_PFILE)hFile, (T_U8*)buffer, count);
    }

    return ret;
}

T_BOOL  Fwl_MkDir(T_pCSTR path)
{
    return File_MkdirsAsc(path);
}

//**************************************************************************************

static T_U16 UniToAsc(const T_U16 *pUniStr, T_U32 UniStrLen,
                    T_pSTR pAnsibuf, T_U32 AnsiBufLen)
{
    T_U32 readedUCChars;

    Eng_Unicode2GBK(pUniStr, UniStrLen, &readedUCChars, pAnsibuf, AnsiBufLen, AK_NULL);
    return UniStrLen;
}

static T_U16 AscToUni(const T_pSTR pAnsiStr, T_U32 AnsiStrLen,
                    T_U16 *pUniBuf, T_U32 UniBufLen)
{
    return Eng_GBK2Unicode(pAnsiStr, AnsiStrLen, AK_NULL, pUniBuf, UniBufLen, AK_NULL);
}

static T_U32 fs_getchipId()
{
#ifdef CHIP_AK980X    
    return FS_AK98XX;
#endif

#ifdef CHIP_AK37XX
    return FS_AK37XX;
#endif

}

static T_U8 fs_InStream(T_VOID)
{
    T_U8 ch = 'c';
    //ch = getch() & 0xff;
    return ch;
}

static T_VOID fs_OutStream(T_U16 ch)
{
    printf("%c", ch);
}

static T_VOID fs_SetSecond(T_U32 seconds)
{
	return;
}

static T_S32 fs_fCrtSem(T_U32 initial_count, T_U8 suspend_type, T_S8 *filename, T_U32 fileline)
{
    return 0;
}

static T_S32 fs_fDelSem(T_S32 semaphore, T_S8 *filename, T_U32 fileline)
{
    return 0;
}

static T_S32 fs_fObtSem(T_S32 semaphore, T_U32 suspend, T_S8 *filename, T_U32 fileline)
{
    return 0;
}

static T_S32 fs_fRelSem(T_S32 semaphore, T_S8 *filename, T_U32 fileline)
{
    return 0;
}

static T_U32 fs_GetSecond(T_VOID)
{
    return 0;//m_file_time;
}

static T_VOID fs_sys_sleep(T_U32 ms)
{
    
}

static T_pVOID fs_Malloc(T_U32 size, T_S8 *filename, T_U32 fileline)
{
    T_pVOID ret;
    
    ret = Fwl_Malloc(size);
    if (AK_NULL == ret)
    {
        printf("fs malloc fail!:size:%d, name:%s, line:%d",size, filename, fileline);
    }

    return ret;
}

static T_pVOID fs_ReMalloc(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline)
{
    T_pVOID ret;
    
    ret = Fwl_ReMalloc(var, size);
    if (AK_NULL == ret)
    {
        printf("fs remalloc fail!:size:%d, name:%s, line:%d",size, filename, fileline);
    }

    return ret;
}

static T_pVOID fs_Free(T_pVOID var, T_S8 *filename, T_U32 fileline)
{
    return Fwl_Free(var);
}

T_BOOL Fwl_MountInit(T_VOID)
{
    T_FSCallback mountinit;

    mountinit.out        = fs_OutStream;
    mountinit.in         = fs_InStream;
    mountinit.fGetSecond = fs_GetSecond;
    mountinit.fSetSecond = fs_SetSecond;
    mountinit.fAscToUni  = AscToUni;
    mountinit.fUniToAsc  = UniToAsc;
    mountinit.fRamAlloc  = fs_Malloc;
    mountinit.fRamRealloc = fs_ReMalloc;
    mountinit.fRamFree   = fs_Free;
    mountinit.fCrtSem    = fs_fCrtSem;
    mountinit.fDelSem    = fs_fDelSem;
    mountinit.fObtSem    = fs_fObtSem;
    mountinit.fRelSem   = fs_fRelSem;
    mountinit.fMemCpy    = memcpy;
    mountinit.fMemSet    = memset;
    mountinit.fMemMov    = memmove;
    mountinit.fMemCmp    = memcmp;

    mountinit.fPrintf    = printf;
    mountinit.fGetChipId = fs_getchipId;
    mountinit.fSysRst    = AK_NULL;
    mountinit.fRandSeed  = AK_NULL;
    mountinit.fGetRand   = AK_NULL;
    mountinit.fMountThead= AK_NULL;
    mountinit.fKillThead = AK_NULL;
    mountinit.fSystemSleep= fs_sys_sleep;
    
    return FS_InitCallBack(&mountinit, 64);
}

//**************************************************************************************
static T_U32 fsa_GeteImageMedium(T_U8 driverID)
{
    DRIVER_INFO DriverInfo;

    if(FS_GetDriver(&DriverInfo, driverID))
    {
        return (T_U32)DriverInfo.medium;
    } 
    else
    {
        return 0;
    }    
}

T_BOOL Fwl_FSAInit(T_VOID)
{
    T_FSA_LIB_CALLBACK fsainit;
    
    fsainit.RamAlloc   = Fwl_Malloc; 
    fsainit.RamFree    = Fwl_Free;
    fsainit.MemSet     = memset;
    fsainit.MemCpy     = memcpy;
    fsainit.MemCmp     = memcmp;
    fsainit.MemMov     = memmove;
    fsainit.Printf     = printf;
    fsainit.fFs.FileOpen   = Fwl_FileOpen;
    fsainit.fFs.FileClose  = Fwl_FileClose;
    fsainit.fFs.FileRead   = Fwl_FileRead;
    fsainit.fFs.FileWrite  = Fwl_FileWrite;
    fsainit.fFs.FileSeek   = AK_NULL;
    fsainit.fFs.FsMkDir    = Fwl_MkDir;
    fsainit.GetImgMedium  = fsa_GeteImageMedium;
 
    return FSA_init(&fsainit);
}

