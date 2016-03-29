#ifdef OS_WIN32
#include <windows.h>
#include <Windowsx.h>
#include "string.h"
#endif
#include "AKError.h"
#include "anyka_types.h"

#include "eng_screensave.h"
#include "Eng_Debug.h"
#include "Eng_String.h"
#include "Eng_Time.h"
#include "Eng_DataConvert.h"
#include "file.h"
#include "arch_mmc_sd.h"
#include "fs.h"
#include "Fwl_vme.h"
#include "Fwl_sd.h"
#include "Fwl_public.h"
#include "Fwl_osMalloc.h"
#include "Fwl_osFS.h"
#include "Lib_event.h"
#include "gpio_config.h"
#include "Gbl_Global.h"
#include "hal_timer.h"
#include "hal_gpio.h"
#include "mount.h"
#include "raminit.h"
#include "fwl_usb_host.h"
#include "hal_print.h"
#include "eng_math.h"
#include "fwl_usb_host.h"
#include "fwl_spiflash.h"
#include "Fwl_sys_detect.h"
#include "Arch_init.h"
//#define FS_TEST_API
//#define FS_DEBG_API

#define COPY_THREAD_PRI    110 //优先级
#define COPY_THREAD_TIMESLICE (5UL)

extern T_U16 Uni_ToUpper(T_U16 uni);

#ifdef FS_TEST_API
static T_VOID Fwl_MtdSpeedTest(T_U8 drvId);
static T_BOOL Fwl_FileSpeedTest(T_U8* path);
static T_VOID Fwl_FsTest(T_CHR *rootdir);
static T_VOID Fwl_MtdTest(T_U8 drvId);
static T_VOID FsApi_Dump(T_U8 *tips,T_U8 *data, T_U32 len);
#endif

static T_U8 MoveFolderPathDeep = 0;
typedef struct{
    T_U16            sourFile[FS_MAX_PATH_LEN+1];
    T_U16            destFile[FS_MAX_PATH_LEN+1];
    T_FS_COPY_CALLBACK CallBack;
}T_FOLDER_MOVE_PARM;
T_FOLDER_MOVE_PARM FolderMoveParm;

typedef struct{
    T_U16            sourFile[FS_MAX_PATH_LEN+1];
    T_U16            destFile[FS_MAX_PATH_LEN+1];
    T_VOID            *pCallBackData;
    T_BOOL          replace;
    T_FS_COPY_CALLBACK CallBack;
}T_FILECOPY_PARM;
T_FILECOPY_PARM FileCopyParm;

typedef struct{
    T_U8            sourFile[FS_MAX_PATH_LEN+1];
    T_U8            destFile[FS_MAX_PATH_LEN+1];
    T_VOID            *pCallBackData;
    T_BOOL          replace;
    T_FS_COPY_CALLBACK CallBack;
}T_FILECOPY_ASC_PARM;
T_FILECOPY_ASC_PARM FileCopyAscParm;

typedef struct{
    T_U16            FileName[FS_MAX_PATH_LEN+1];
    T_VOID            *pCallBackData;
    T_FS_DEL_CALLBACK CallBack;
}T_FILEDEL_PARM;
T_FILEDEL_PARM FileDelParm;

typedef struct{
    T_U8            FileName[FS_MAX_PATH_LEN+1];
    T_VOID            *pCallBackData;
    T_FS_DEL_CALLBACK CallBack;
}T_FILEDEL_ASC_PARM;
T_FILEDEL_ASC_PARM FileDelAscParm;


#ifdef ASYN_CLOSE_FILE_CNT
/*
*MODULE:This moudule is for aysn close file or destory file
*AUTHOR:wangxi
*DATE:2011-05-19
*/
#define ASYN_MONITOR_MODE     0 // 1:thread  0: timer
#define ASYN_CLOSE_INFO(tips) Fwl_Print(C3,M_FS,"[AsynClose]%s",tips)
#define ASYN_CLOSE_ERR(tips)  Fwl_Print(C3,M_FS,"[AsynClose]%s_@%d",tips,__LINE__)
    
#define CLOSE_QUE_MAX   (gCloseFileQue.queCnt)
#if (1 == ASYN_MONITOR_MODE)
#define INVALID_CLOSE_MAN_HDL (0)
#define CLOSE_MAN_INTREVAL    (101)
typedef T_U32 ASYN_CLOSE_MAN_HDL;
#else
#define INVALID_CLOSE_MAN_HDL (ERROR_TIMER)
#define CLOSE_MAN_INTREVAL    (2500)
typedef T_TIMER ASYN_CLOSE_MAN_HDL;
#endif


typedef struct
{
    T_pFILE         fp;
    T_ASYN_OPT_MODE optMode;   
} T_OPT_NODE;

typedef enum
{
    eASYN_STATUS_NULL=0,
    eASYN_STATUS_PAUSE,
    eASYN_STATUS_RUN,
} T_ASYN_STATUS;

typedef struct
{
    T_OPT_NODE        *hdlQue; //异步操作队列
    T_U16              head;   //队列头
    T_U16              tail;   //队列尾
    T_U32              validLen; //队列中的结点数
    T_U32              queCnt;
    ASYN_CLOSE_MAN_HDL aysnMonitor;
    T_ASYN_STATUS      monitorStatus;
} T_CLOSE_FILE_QUE;

static T_CLOSE_FILE_QUE gCloseFileQue = {0};
static T_VOID Fwl_AsynCloseFlush_EnableMonitor(T_BOOL isEnable);
#if (1 == ASYN_MONITOR_MODE)
static T_VOID Fwl_AsynCloseFlushMonitor(T_U32 argc, T_pVOID argv);
#else
static T_VOID Fwl_AsynCloseFlushMonitor(T_TIMER timer_id, T_U32 delay);
#endif

#endif





static T_U32 FsApi_MountThread(ThreadFunPTR Fun, T_pVOID pData, T_U32 priority);
static T_PCOPY_CTRL FsApi_FileCopyThread(ThreadFunPTR Fun, T_pVOID pData, T_U32 priority);
static T_VOID FsApi_KillThead(T_U32 ThreadHandle);
static T_VOID FsApi_KillCopyThead(T_COPY_CTRL **ThreadHandle);


#ifdef OS_WIN32

typedef struct tag_FsInitInfo
{
    F_OutStream out;
    F_Instream  in;
    F_GetSecond fGetSecond;
    F_SetSecond fSetSecond;
    F_UniToAsc  fUniToAsc;
    F_AscToUni  fAscToUni;
    F_RamAlloc  fRamAlloc;
    F_RamRealloc fRamRealloc;
    F_RamFree  fRamFree;
    F_OsCrtSem fCrtSem;
    F_OsDelSem fDelSem;
    F_OsObtSem fObtSem;
    F_OsRelSem fRelSem;

    F_MemCpy  fMemCpy;
    F_MemSet  fMemSet;
    F_MemMov  fMemMov;
    F_MemCmp  fMemCmp;
    F_Printf  fPrintf;

    F_GetChipID fGetChipId;
}T_FSINITINFO, *T_PFSINITINFO;


T_U32 Asyn_WriteFatSector(T_PMEDIUM medium, const T_U8 *buf, T_U32 start, T_U32 size)
{
    return medium->write(medium, buf, start, size);
}

T_U32 Asyn_ReadFatSector(T_PMEDIUM medium, T_U8* buf, T_U32 start, T_U32 size)
{
    return medium->read(medium, buf, start, size);
}


void FS_ClearAsyn(T_PMEDIUM medium, T_U32 sector, T_U32 size)
{
    return;
}

T_U32 Asyn_ReadSector(T_PMEDIUM medium, T_U8* buf, T_U32 start, T_U32 size)
{
    return 0;
}

T_U32 Asyn_WriteSector(T_PMEDIUM medium, const T_U8 *buf, T_U32 start, T_U32 size)
{
    return 0;
}

T_BOOL FS_GetNextDriver(PDRIVER_INFO pDriverInfo)
{
    return AK_FALSE;
}

T_BOOL FS_GetFirstDriver(PDRIVER_INFO pDriverInfo)
{
    return AK_FALSE;
}

T_BOOL FS_GetDriver(PDRIVER_INFO pDriverInfo, T_U8 DriverID)
{
    return AK_FALSE;
}

T_BOOL FS_GetObjDriver(PDRIVER_INFO pDriverInfo)
{
    return AK_FALSE;
}

T_VOID Medium_Destroy(T_PMEDIUM obj)
{

}

T_PMEDIUM FS_GetDriverMedium(T_U8 DriverID)
{
    return AK_NULL;
}

T_BOOL FS_SetAsynWriteBufSize(T_U32 BufSize, T_U8 DriverID)
{
    return AK_FALSE;
}

T_BOOL Fwl_FhaInit(T_VOID)
{
    return AK_TRUE;
}

#endif    // End of OS_WIN32


static T_pFILE  FsApi_FileOpen(T_pVOID path, T_FILE_MODE mode, T_BOOL isUnicode, T_BOOL includeFolder,T_BOOL isAsyn)
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

    if (isAsyn)
    {
        mode |= FILE_MODE_ASYN;
    }
    
    if (isUnicode)
    {
        file = File_OpenUnicode(AK_NULL, (T_U16*)path, mode);
    }
    else
    {
        file = File_OpenAsc(AK_NULL, (T_U8*)path, mode);
    }
    
    if (file == (T_U32)AK_NULL)
    {
#ifdef FS_DEBG_API
        Fwl_Print(C1, M_FS, " FsApi_FileOpen open error");
#endif
        return FS_INVALID_HANDLE;
    }


    if (mode != FILE_MODE_CREATE) //(0)//
    {
        if (!File_Exist(file))
        {
#ifdef FS_DEBG_API
            Fwl_Print(C1, M_FS, " FsApi_FileOpen not exist\n");
#endif
            File_Close(file);
            return FS_INVALID_HANDLE;
        }
    }
    
    if (!includeFolder)
    {
        if ((!File_IsFile(file)) || (File_IsFolder(file)))
    {
#ifdef FS_DEBG_API
        Fwl_Print(C1, M_FS, " FsApi_FileOpen Is Not File\n");
#endif
        File_Close(file);
        return FS_INVALID_HANDLE;
    }
    }

    return (T_S32)file;
}




static T_BOOL FsApi_MoveFile(T_pVOID SrcStr, T_pVOID DstStr, T_BOOL isUnicode)
{
    T_PFILE source, dest;
    T_BOOL ret;
    T_U8 srcDrvId = 0, destDrvId = 0;

    srcDrvId = (T_U8)Fwl_GetDriverIdByPath((T_U16*)SrcStr);
    destDrvId = (T_U8)Fwl_GetDriverIdByPath((T_U16*)DstStr);

    if (srcDrvId != destDrvId)
    {
#ifdef FS_DEBG_API
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: FsApi_MoveFile: drv[%d] -> drv[%d]\n", srcDrvId,destDrvId);
#endif
        return AK_FALSE;//不支持不同盘符的文件之间的命名与移动 
    }
    
    if (isUnicode)
    {
        source = File_OpenUnicode(AK_NULL,(T_U16*)SrcStr, FILE_MODE_READ);
    }
    else
    {
        source = File_OpenAsc(AK_NULL,(T_U8*)SrcStr, FILE_MODE_READ);
    }

    if (source == (T_U32)AK_NULL)
    {
#ifdef FS_DEBG_API
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: FsApi_MoveFile: Src Open error\n");
#endif
        return AK_FALSE;
    }
    
    if (!File_Exist(source))
    {
#ifdef FS_DEBG_API
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: FsApi_MoveFile: Src not exist\n");
#endif
        File_Close(source);
        return AK_FALSE;
    }

    if (isUnicode)
    {
        dest = File_OpenUnicode(AK_NULL,(T_U16*)DstStr, FILE_MODE_READ);
    }
    else
    {
        dest = File_OpenAsc(AK_NULL,(T_U8*)DstStr, FILE_MODE_READ);
    }

    if (dest == (T_U32)AK_NULL)
    {
#ifdef FS_DEBG_API
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: FsApi_MoveFile: Dest Open error\n");
#endif
        File_Close(source);
        return AK_FALSE;
    }

   // dest->mode = FILE_MODE_OVERLAY;
    ret = File_RenameTo(source, dest);
   // dest->ExtMode |= FILE_MODE_EXT_FAT_FIXED;

    File_Close(source);
    File_Close(dest);

    return ret;
}

static T_BOOL FsApi_MoveFolder(T_pWSTR oldFile, T_pWSTR newFile,T_FS_COPY_CALLBACK CallBack)
{
    T_hFILESTAT find;
    T_FILE_INFO FileInfo;
    T_USTR_FILE BackPath;
    T_USTR_FILE BackNewPath;
    T_USTR_FILE FindPath;
    T_PFILE source, dest;
    T_BOOL      ret = AK_TRUE;
    T_BOOL      RetValue;
    
    if ((AK_NULL == oldFile) || (AK_NULL == oldFile))
    {
        return AK_FALSE;
    }
    if (MoveFolderPathDeep >= MAX_PATH_DEEP)
    {
        return AK_FALSE;
    }
    
    // find a folder
    if (Utl_UStrLen(oldFile) < MAX_FILENM_LEN)
    {
        Utl_UStrCpy(BackPath, oldFile);
        Utl_UStrCpy(FindPath, oldFile);
        Utl_UStrCpy(BackNewPath, newFile);
        
        if (FindPath[Utl_UStrLen(FindPath) - 1] == UNICODE_SOLIDUS)
        {
            FindPath[Utl_UStrLen(FindPath) - 1] = 0;
        }
        Utl_UStrCat(FindPath, _T("/*.*"));

        if (FS_INVALID_STATHANDLE != (find = Fwl_FsFindFirst(FindPath)))
        {            
            do
            {
                Fwl_FsFindInfo(&FileInfo, find);
                
                // Path Is Too Longer
                if (Utl_UStrLen(BackPath) + Utl_UStrLen(FileInfo.name) >= sizeof(T_STR_FILE) - 2)
                    continue;

                Utl_UStrCpy(oldFile, BackPath);
                Utl_UStrCpy(newFile, BackNewPath);
                
                if (oldFile[Utl_UStrLen(oldFile) - 1] != UNICODE_SOLIDUS)
                {
                    Utl_UStrCat(oldFile, _T("/"));
                }
                if (newFile[Utl_UStrLen(newFile) - 1] != UNICODE_SOLIDUS)
                {
                    Utl_UStrCat(newFile, _T("/"));
                }
                Utl_UStrCat(oldFile, FileInfo.name);
                Utl_UStrCat(newFile,FileInfo.name);    
                
                // Is A Directory
                if ((FileInfo.attrib & 0x10) == 0x10)
                {
                    // folder is "." or "..", continue
                    if (0 == Utl_UStrCmp(FileInfo.name, _T(".")) || 0 == Utl_UStrCmp(FileInfo.name, _T("..")))
                    {
                        continue;
                    }
#if 0                    
                    Printf_UC(newFile);
                    Printf_UC(oldFile);
                    AK_DEBUG_OUTPUT("L:%d\n",__LINE__);
#endif                    
                    if (!Fwl_FsMkDir(newFile))
                    {
                        Fwl_Print(C1, M_FS, "FsApi_MoveFolder:Fwl_FsMkDir fail\n");
                        Printf_UC(newFile);
                        Fwl_FsFindClose(find);
                        return AK_FALSE;
                    }
                    if (AK_NULL != CallBack.Callback) //中断退出
                    {
                        RetValue = CallBack.Callback(AK_NULL, FileInfo.name, 0, 0);
                        if (!RetValue)
                        {
                            Fwl_FsFindClose(find);
                            Fwl_Print(C1, M_FS, "#1:Call Back Return False\n");
                            return AK_FALSE;
                        }
                    }
                        
                    ++MoveFolderPathDeep;
                    ret = FsApi_MoveFolder(oldFile, newFile, CallBack);
                    --MoveFolderPathDeep;
                }
                // Is A File
                else
                {
                    if (AK_NULL != CallBack.Callback) //中断退出
                    {
                        RetValue = CallBack.Callback(AK_NULL, FileInfo.name, 0, 0);
                        if (!RetValue)
                        {
                            Fwl_FsFindClose(find);
                            Fwl_Print(C1, M_FS, "#2:Call Back Return False\n");
                            return AK_FALSE;
                        }
                    }
                    
                    source = File_OpenUnicode(AK_NULL, oldFile, FILE_MODE_READ);
                    if (source == (T_U32)AK_NULL)
                    {
                        Fwl_Print(C1, M_FS, "#1FsApi_MoveFolder:File_OpenUnicode fail Type:%d\n",File_GetLastErrorType());
                        Printf_UC(oldFile);
                        Fwl_FsFindClose(find);
                        return AK_FALSE;
                    }
                    if (!File_Exist(source))
                    {
                        Fwl_Print(C1, M_FS, "FsApi_MoveFolder:File is no Exist\n");
                        Printf_UC(oldFile);
                        File_Close(source);
                        Fwl_FsFindClose(find);
                        return AK_FALSE;
                    }

                    dest = File_OpenUnicode(AK_NULL, newFile, FILE_MODE_READ);
                    if (dest == (T_U32)AK_NULL)
                    {
                        Fwl_Print(C1, M_FS, "#2FsApi_MoveFolder:File_OpenUnicode fail Type:%d\n",File_GetLastErrorType());
                        Printf_UC(newFile);
                        File_Close(source);
                        Fwl_FsFindClose(find);
                        return AK_FALSE;
                    }
                    if (File_Exist(dest))
                    {
                        //AK_DEBUG_OUTPUT("FsApi_MoveFolder:Dst File is Exist,Will To Del\n");
                        if (!File_DelFile(dest))
                        {
                            Fwl_Print(C1, M_FS, "Err:RenameTo Dst file is exist,and del fail\n");
                            File_Close(source);
                            File_Close(dest);
                            Fwl_FsFindClose(find);
                            return AK_FALSE;
                        }
                    }

                    RetValue = File_RenameTo(source, dest);
                    File_Close(source);
                    File_Close(dest);
                    if (!RetValue)
                    {
                        Fwl_Print(C1, M_FS, "FsApi_MoveFolder:File_RenameTo fail Type:%d\n",File_GetLastErrorType());
                        Fwl_FsFindClose(find);
                        return AK_FALSE;
                    }
                }
            } while (Fwl_FsFindNext(find));    // End of do
            
            Fwl_FsFindClose(find);
            find =FS_INVALID_STATHANDLE;
        }
    }
    return ret;
}

static T_VOID FsApi_FolderMoveThreadFun(T_U32 argc, T_VOID *argv)
{
    T_BOOL ret;
    T_USTR_FILE BackSourPath;
    T_FOLDER_MOVE_PARM *pFolderMoveParm = (T_FOLDER_MOVE_PARM *)argv;
    
    if (AK_NULL == pFolderMoveParm)
    {
        return;
    }
    
    Utl_UStrCpy(BackSourPath, pFolderMoveParm->sourFile);
    
    Fwl_Print(C3, M_FS, "Folder Cut begin...\n");

    if (AK_NULL != pFolderMoveParm->CallBack.SetState)
    {
        pFolderMoveParm->CallBack.SetState(eFS_COPY_ING);
    }
    
    MoveFolderPathDeep = 0;
    
    ret = FsApi_MoveFolder(pFolderMoveParm->sourFile,pFolderMoveParm->destFile,pFolderMoveParm->CallBack);
    
    Fwl_Print(C3, M_FS, "Folder Cut End ret:%d\n",ret);
    
    if (ret)
    {
        if (!Fwl_FileDelete(BackSourPath))//FsApi_MoveFolder 只是Rename 文件，这里要删除源文件夹。
        {
            if (AK_NULL != pFolderMoveParm->CallBack.SetState)
            {
                pFolderMoveParm->CallBack.SetState(eFS_COPY_Fail);
            }
            return;
        }
        
        if (AK_NULL != pFolderMoveParm->CallBack.SetState)
        {
            pFolderMoveParm->CallBack.SetState(eFS_COPY_Success);
        }
    }
    else
    {
        if (AK_NULL != pFolderMoveParm->CallBack.SetState)
        {
            pFolderMoveParm->CallBack.SetState(eFS_COPY_Fail);
        }
    }
}

T_pFILE Fwl_FileOpen(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE mode)
{
    T_pFILE pFile = FS_INVALID_HANDLE;
    
    if (AK_NULL != path)
    {
#ifdef FS_DEBG_API
        Printf_UC(path);
        FsApi_Dump("Open_Uni", path, 2*Utl_StrLen(path));
#endif
        pFile = FsApi_FileOpen((T_pVOID)path, mode , AK_TRUE, AK_FALSE,AK_FALSE);
    }

    if (FS_INVALID_HANDLE == pFile)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileOpen: %d.\n", pFile);
    }
    
    return pFile;
}

T_pFILE Fwl_FileOpenAsc(T_pSTR path, T_FILE_FLAG flag, T_FILE_MODE mode)
{
    T_pFILE pFile = FS_INVALID_HANDLE;

    if (AK_NULL != path)
    {
#ifdef FS_DEBG_API
        Fwl_Print(C3, M_FS, "%s\n",path);
        FsApi_Dump("Open_Asc", path,strlen(path));
#endif
        pFile = FsApi_FileOpen((T_pVOID)path, mode , AK_FALSE, AK_FALSE,AK_FALSE);
    }

    if (FS_INVALID_HANDLE == pFile)
    {
        Fwl_Print(C3, M_FS, "FWL FILE ERROR: Fwl_FileOpenAsc: %d.\n", pFile);
    }
    
    return pFile;
}


T_pFILE Fwl_FileOpen_Ex(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE mode)
{
    T_pFILE pFile = FS_INVALID_HANDLE;

    if (AK_NULL != path)
    {
#ifdef FS_DEBG_API
        Printf_UC(path);
        FsApi_Dump("OpenEx_Uni", path, 2*Utl_StrLen(path));
#endif
        pFile = FsApi_FileOpen((T_pVOID)path, mode , AK_TRUE, AK_TRUE,AK_FALSE);
    }
    
    if (pFile == FS_INVALID_HANDLE)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileOpen_Ex: %d\n", pFile);
    }

    return pFile;
}


T_pFILE Fwl_FileOpenAsc_Ex(T_pSTR path, T_FILE_FLAG flag, T_FILE_MODE mode)
{
    T_pFILE pFile = FS_INVALID_HANDLE;

    if (AK_NULL != path)
    {
#ifdef FS_DEBG_API
        Fwl_Print(C3, M_FS, "%s\n",path);
        FsApi_Dump("OpenEx_Asc", path,strlen(path));
#endif
        pFile = FsApi_FileOpen((T_pVOID)path, mode , AK_FALSE, AK_TRUE,AK_FALSE);
    }

    if (FS_INVALID_HANDLE == pFile)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileOpenAsc_Ex: %d.\n", pFile);
    }
    
    return pFile;
}

T_BOOL Fwl_InitAsyn(T_U32 BufSize, T_pCWSTR path)
{
    T_U8 driverId = (T_U8)Fwl_GetDriverIdByPath(path);
    T_DRIVER_INFO drvInfo;
    T_BOOL ret = AK_FALSE;
    
    //BufSize = 0;
    if (FS_GetDriver(&drvInfo, driverId))
    {
        ret = FS_SetAsynWriteBufSize(BufSize,driverId);
    }
    Fwl_Print(C3, M_FS, "Fwl_InitAsyn drv[%d] size = 0x%x ret = %d\n",driverId,BufSize,ret);
    return ret;
}


T_BOOL Fwl_DeInitAsyn(T_pCWSTR path)
{
    T_U8 driverId = (T_U8)Fwl_GetDriverIdByPath(path);
    T_DRIVER_INFO drvInfo;
    T_BOOL ret = AK_FALSE;
    
#ifdef ASYN_CLOSE_FILE_CNT
    Fwl_AsynCloseFlushAll();
#endif

    if (FS_GetDriver(&drvInfo, driverId))
    {
        ret = FS_SetAsynWriteBufSize(0,driverId);
    }
    Fwl_Print(C3, M_FS, "Fwl_DeInitAsyn drv[%d] ret = %d\n",driverId,ret);
    return ret;
}

T_VOID Fwl_DestoryAsynWrite(T_VOID)
{
#ifdef OS_ANYKA
    T_U8 driverId = 0;
    T_DRIVER_INFO drvInfo;
    T_BOOL ret = AK_FALSE;
    for (driverId=0; driverId < FS_MAX_DISK_CNT; driverId++)
    {
        if (FS_GetDriver(&drvInfo, driverId))
        {
            ret = FS_SetAsynWriteBufSize(0,driverId);
            Fwl_Print(C3, M_FS, "DestoryAsynWrite drv[%d] ret = %d\n",driverId,ret);
        }
    }
#endif
}


T_pFILE Fwl_FileOpenAsyn(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE mode)
{
    T_pFILE pFile = FS_INVALID_HANDLE;
    
    if (AK_NULL != path)
    {
#ifdef FS_DEBG_API
        Printf_UC(path);
        FsApi_Dump("Open_Uni", path, 2*Utl_StrLen(path));
#endif
        pFile = FsApi_FileOpen((T_pVOID)path, mode , AK_TRUE, AK_FALSE,AK_TRUE);
    }

    if (FS_INVALID_HANDLE == pFile)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileOpen: %d.\n", pFile);
    }
    
    return pFile;
}


T_BOOL Fwl_FileExist(T_pCWSTR path)
{
    T_PFILE pFile = AK_NULL;
    T_BOOL  ret = AK_FALSE;

    if (AK_NULL != path)
    {
        pFile = File_OpenUnicode(AK_NULL, path, FILE_MODE_READ);

        if ((T_U32)AK_NULL == pFile)
        {
            pFile = File_OpenUnicode(AK_NULL, path, FILE_MODE_ASYN | FILE_MODE_READ);
            if ((T_U32)AK_NULL == pFile)
            {
                //AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FileExist:open fail %d\n", pFile);
                return AK_FALSE;
            }
        }
        
        ret  = File_Exist(pFile);
        File_Close(pFile);
    }
    
    return ret;
}

#if 0
T_BOOL Fwl_IsFileExist(T_pCWSTR path)
{
    T_PFILE pFile = AK_NULL;
    T_BOOL  ret = AK_FALSE;

    if (AK_NULL != path)
    {
        pFile = File_OpenUnicode(AK_NULL, path, FILE_MODE_READ);

        if (AK_NULL == pFile)
        {
            return    AK_TRUE;
        }
        
        ret  = File_Exist(pFile);
        File_Close(pFile);
    }
    
    return ret;
}

#endif

T_BOOL Fwl_FileExistAsc(T_pSTR path)
{
    T_PFILE pFile = AK_NULL;
    T_BOOL  ret = AK_FALSE;

    if (AK_NULL != path)
    {
        pFile = File_OpenAsc(AK_NULL, path, FILE_MODE_READ);

        if ((T_U32)AK_NULL == pFile)
        {
            //AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FileExistAsc:open fail %d\n", pFile);
            return    AK_FALSE;
        }
        
        ret  = File_Exist(pFile);
        File_Close(pFile);
    }
    
    return ret;
}

T_BOOL  Fwl_FsIsDir(T_pCWSTR path)
{
    T_PFILE pFile = AK_NULL;
    T_BOOL    ret = AK_FALSE;

    pFile = File_OpenUnicode(AK_NULL, path, FILE_MODE_READ);
    if ((T_U32)AK_NULL == pFile)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FsIsDir:open fail %d\n", pFile);
        return    AK_FALSE;
    }

    ret  = File_IsFolder(pFile);
    File_Close(pFile);
    return ret;
}

#if 0
typedef T_BOOL  (*FS_FileIsFolder)(T_USTR_FILE FolderPath);

T_VOID Fwl_FsIsDirDeeply(T_USTR_FILE FolderPath, const FS_FileIsFolder pCB)
{
    static T_U8 PathDeep = 0;
    
    T_hFILESTAT find;
    T_FILE_INFO FileInfo;
    T_USTR_FILE BackPath;
    T_USTR_FILE TmpPath;
    T_USTR_FILE FindPath;

    if (AK_NULL == FolderPath)
    {
        AK_DEBUG_OUTPUT("ERR:LHS:Fwl_FsIsDirDeeply();FolderPath is null\n");
        return;
    }
    if (PathDeep >= MAX_PATH_DEEP)
    {
        return;
    }
    
    // find a folder
    if (Utl_UStrLen(FolderPath) < MAX_FILENM_LEN)
    {
        Utl_UStrCpy(BackPath, FolderPath);
        Utl_UStrCpy(FindPath, FolderPath);
        
        if (FindPath[Utl_UStrLen(FindPath) - 1] == UNICODE_SOLIDUS)
        {
            FindPath[Utl_UStrLen(FindPath) - 1] = 0;
        }
        Utl_UStrCat(FindPath, _T("/*.*"));

        if (FS_INVALID_STATHANDLE != (find = Fwl_FsFindFirst(FindPath)))
        {            
            do
            {
                Fwl_FsFindInfo(&FileInfo, find);                

                // Path Is Too Longer
                if (Utl_UStrLen(BackPath) + Utl_UStrLen(FileInfo.name) >= sizeof(T_STR_FILE) - 2)
                    continue;

                Utl_UStrCpy(TmpPath, BackPath);                    
                if (TmpPath[Utl_UStrLen(TmpPath) - 1] != UNICODE_SOLIDUS)
                {
                    Utl_UStrCat(TmpPath, _T("/"));
                }
                
                Utl_UStrCat(TmpPath, FileInfo.name);
                
                // Is A Directory
                if ((FileInfo.attrib & 0x10) == 0x10)
                {
                    // folder is "." or "..", continue
                    if (0 == Utl_UStrCmp(FileInfo.name, _T(".")) || 0 == Utl_UStrCmp(FileInfo.name, _T("..")))
                    {
                        continue;
                    }
                    
                    if (AK_NULL != pCB)
                    {
                        //pCB(TmpPath);
                    }
                    
                    //Printf_UC(TmpPath);
                    ++PathDeep;
                    Fwl_FsIsDirDeeply(TmpPath, pCB);
                    --PathDeep;
                }
                // Is A File
                else
                {
                                
                }
            } while (Fwl_FsFindNext(find));    // End of do
            
            Fwl_FsFindClose(find);
            find =FS_INVALID_STATHANDLE;
        }
    }
    return;
}


T_BOOL Fs_find_deepfolderandfile(T_PFILE parenfile, T_PFILE parent)
{
    T_U32 str;
    T_U32 pFindCtrl;
    T_U16 *path;
    T_USTR_FILE Utmpstr;
    T_FINDBUFCTRL bufCtrl;
    T_PFILEINFO fileinfo = AK_NULL;
    T_U32 time;
    time = get_tick_count();
    str = File_GetPathObj(parenfile);
    path = File_GetAbsPath(str);
    //Printf_UC(path);
    //AK_DEBUG_OUTPUT("L%d\n",__LINE__);
    
    Eng_StrMbcs2Ucs("*.*", Utmpstr);
    bufCtrl.NodeCnt      = 1;
    bufCtrl.pattern          = Utmpstr;
    bufCtrl.type = FILTER_NOTITERATE | FILTER_FOLDER;
    bufCtrl.patternLen      = Utl_StrLen(Utmpstr);
    pFindCtrl = File_FindFirst(path,  &bufCtrl);
    //Printf_UC(Utmpstr);
    //AK_DEBUG_OUTPUT("pFindCtrl:%d\n",pFindCtrl);
    if (pFindCtrl == 0)
    {
        return;
    }
    
    do 
    {
        fileinfo = File_FindInfo(pFindCtrl, 0, AK_NULL, AK_NULL);
        if (fileinfo != AK_NULL)
        {
            parenfile = File_FindOpen(parent, fileinfo); 
            if( fileinfo->attr & 0x10)
            {        
                //Printf_UC(path);
                //Printf_UC(fileinfo->LongName);
                Fs_find_deepfolderandfile(parenfile, parent);
            }
        }
    } while (File_FindNext(pFindCtrl, 1) != 0);
    
    File_FindClose(pFindCtrl);
    File_DestroyPathObj(str);
}
#endif

T_BOOL  Fwl_FsIsDirAsc(T_pSTR path)
{
    T_PFILE pFile = AK_NULL;
    T_BOOL    ret = AK_FALSE;

    pFile = File_OpenAsc(AK_NULL, path, FILE_MODE_READ);
    if ((T_U32)AK_NULL == pFile)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FsIsDirAsc:open fail %d\n", pFile);
        return    AK_FALSE;
    }

    ret  = File_IsFolder(pFile);
    File_Close(pFile);
    return ret;
}


T_BOOL  Fwl_FsIsFile(T_pCWSTR path)
{
    T_PFILE pFile = AK_NULL;
    T_BOOL    ret = AK_FALSE;

    pFile = File_OpenUnicode(AK_NULL, path, FILE_MODE_READ);
    if ((T_U32)AK_NULL == pFile)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FsIsFile:open fail %d\n", pFile);
        return    AK_FALSE;
    }

    ret  = File_IsFile(pFile);
    File_Close(pFile);
    return ret;
}


T_BOOL  Fwl_FsIsFileAsc(T_pSTR path)
{
    T_PFILE pFile = AK_NULL;
    T_BOOL    ret = AK_FALSE;

    pFile = File_OpenAsc(AK_NULL, path, FILE_MODE_READ);
    if ((T_U32)AK_NULL == pFile)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FsIsFileAsc:open fail %d\n", pFile);
        return    AK_FALSE;
    }

    ret  = File_IsFile(pFile);
    File_Close(pFile);
    return ret;
}

/*
  FUNCTION : only for flushing file data which have written.
  ATTENTION: if you want to change this function,
             please communicate with FILE system.
 */
T_VOID Fwl_FileDataFlush(T_pFILE hFile)
{
    if (FS_INVALID_HANDLE != hFile)
    {
        File_Flush((T_PFILE)hFile);
    }
}



T_BOOL Fwl_FileHandleExist(T_pFILE hFile)
{
    if (FS_INVALID_HANDLE != hFile)
    {
        return    File_Exist((T_PFILE)hFile);
    }
    
    return AK_FALSE;
}

T_BOOL Fwl_GetAsynBufInfo(T_U32 *UseSize, T_U32 *BufSize)
{
#ifdef OS_ANYKA
    return FS_GetAsynBufInfo(UseSize, BufSize);
#else
    return AK_FALSE;
#endif
}

T_U32   Fwl_FileRead(T_pFILE hFile, T_pVOID buffer, T_U32 count)
{
    T_U32 ret = 0;

    if (FS_INVALID_HANDLE != hFile)
    {
        ret = File_Read((T_PFILE)hFile, buffer, count);
    }
    if (ret == 0)
    {
        Fwl_Print(C4, M_FS, "Fwl_FileRead: %d\n", ret);
    }

    return ret;
}

T_U32   Fwl_FileWrite(T_pFILE hFile, T_pCVOID buffer, T_U32 count)
{
    T_U32 ret = 0;

    if (FS_INVALID_HANDLE != hFile)
    {
        ret = File_Write((T_PFILE)hFile, (T_pVOID)buffer, count);
    }
#if 0
    if (ret == 0){
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileWrite: %d\n", ret);
    }
#endif
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
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileClose: %d\n", ret);
    }

    return ret;
}

T_BOOL  Fwl_FileTruncate(T_pFILE hFile, T_U32 length)
{
    T_BOOL ret = AK_FALSE;

    if (FS_INVALID_HANDLE != hFile)
    {
        ret = File_Truncate((T_PFILE)hFile, length,0);
    }
    
    if (AK_FALSE == ret)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileTruncate: %d\n", ret);
    }

    return ret;
}


T_S32   Fwl_FileSeek(T_pFILE hFile, T_S32 offset, T_U16 origin)
{
    T_S32 ret = 0;

    if (FS_INVALID_HANDLE != hFile)
    {
        ret = File_SetFilePtr((T_PFILE)hFile,offset,origin);
    }
    
    if (ret < 0)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileSeek: %d\n", ret);
    }

    return ret;
}


/*
 * brief: For Some File Size More Than 2G And Less Than 4G
 */
T_U32   Fwl_FileSeekEx(T_pFILE hFile, T_U32 offset, T_U16 origin)
{
    T_U32 low = 0;
    T_U32 high = 0;
    
     if (FS_INVALID_HANDLE != hFile)
     {
          switch (origin)
          {
          case SEEK_SET:
               low = File_Seek((T_PFILE)hFile, (T_U32)offset, &high);
               break;
            
          case SEEK_END:
               low = File_GetLength((T_PFILE)hFile, &high);
            
             if (high != 0 || low+offset < low)//out of range
               {
                Fwl_Print(C1, M_FS, "SEEK_END ERROR: Fwl_FileSeek: 0x%x, 0x%x, 0x%x\n", low, offset, high);
                return (T_U32)-1;
               }
            
               low = File_Seek((T_PFILE)hFile, low+offset, &high);
               break;
            
          case SEEK_CUR:
               low = File_GetFilePtr((T_PFILE)hFile, &high);
            
            if (high != 0 || low+offset < low)//out of range
            {
                Fwl_Print(C1, M_FS, "SEEK_CUR ERROR: Fwl_FileSeek: 0x%x, 0x%x, 0x%x\n", low, offset, high);
                return (T_U32)-1;
            }
            
            low = File_Seek((T_PFILE)hFile, low+offset, &high);
            break;
        }
    }
 
    return low;
}


T_S32 Fwl_FileTell(T_pFILE hFile)
{
    T_U32 high = 0;

    if (FS_INVALID_HANDLE != hFile)
    {
        return File_GetFilePtr((T_PFILE)hFile, &high);
    }
    else
    {
        return (T_S32)0xffffffff;
    }
}


T_BOOL  Fwl_FileDeltree(T_pFILE hFile)
{
    T_BOOL ret = AK_FALSE;

    if (FS_INVALID_HANDLE != hFile)
    {
        if (File_IsFolder((T_PFILE)hFile))
        {
            if(0 < File_DelDir((T_PFILE)hFile))
            {
                ret = AK_TRUE;
            }
        }
        else
        {
            ret = File_DelFile((T_PFILE)hFile);
        }
    }
    
    if (AK_FALSE == ret)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileDeltree: %d\n", ret);
    }

    return ret;
}


T_BOOL Fwl_SetFileSize(T_pFILE hFile, T_U32 size)
{
   return File_SetFileSize((T_PFILE)hFile,size);
}



T_BOOL Fwl_SetFileBufferSize(T_pFILE hFile, T_U32 size)
{
   return File_SetBufferSize((T_PFILE)hFile,size);
}

T_BOOL  Fwl_IsFile(T_pFILE hFile)
{
    return File_IsFile((T_PFILE)hFile);
}


T_VOID Fwl_FileDestroy(T_pFILE hFile)
{
    if (FS_INVALID_HANDLE != hFile)
    {
        File_DelFile((T_PFILE)hFile);
    }
}


T_BOOL  Fwl_FileDelete (T_pCWSTR path)
{
    T_BOOL ret = AK_FALSE;

    if (AK_NULL != path)
    {
        ret = File_DelUnicode(path);
    }
    
    if (AK_TRUE != ret)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileDelete : %d\n", ret);
    }

    return ret;
}



T_BOOL  Fwl_FileDeleteAsc(T_pSTR path)
{
    T_BOOL ret = AK_FALSE;

    if (AK_NULL != path)
    {
        ret = File_DelAsc(path);
    }
    
    if (AK_TRUE != ret)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileDeleteAsc: %d\n", ret);
    }

    return ret;
}

static T_VOID FsApi_FileDelThreadFun(T_U32 argc, T_VOID *argv)
{
    T_BOOL ret;
    T_FILEDEL_PARM *pFileDelParm = (T_FILEDEL_PARM *)argv;
    
    if (AK_NULL == pFileDelParm)
    {
        return;
    }

    Printf_UC(pFileDelParm->FileName);
    Fwl_Print(C3, M_FS, "File Del begin...\n");

    if (AK_NULL != pFileDelParm->CallBack.SetState)
    {
        pFileDelParm->CallBack.SetState(eFS_COPY_ING);
    }
    
    ret = File_DelUnicode_DelCallBack(pFileDelParm->FileName,pFileDelParm->CallBack.Callback, \
                pFileDelParm->pCallBackData);
        
    Fwl_Print(C3, M_FS, "File Del End ret:%d\n",ret);
    
    if (ret)
    {
        if (AK_NULL != pFileDelParm->CallBack.SetState)
        {
            pFileDelParm->CallBack.SetState(eFS_COPY_Success);
        }
    }
    else
    {
        if (AK_NULL != pFileDelParm->CallBack.SetState)
        {
            pFileDelParm->CallBack.SetState(eFS_COPY_Fail);
        }
    }
}

T_BOOL Fwl_FileDel_CallBack(T_pCWSTR FileName, T_FS_DEL_CALLBACK delCallBack, T_VOID *delCallBackData, T_COPY_CTRL **thrdCtrl)
{
    T_BOOL ret = AK_FALSE;
    
    if (FileName != AK_NULL)
    {
        if (AK_NULL != FileDelParm.CallBack.SetState)
        {
            FileDelParm.CallBack.SetState(eFS_COPY_ING);//init
        }
        
        Utl_UStrCpyN(FileDelParm.FileName, FileName, FS_MAX_PATH_LEN);
        FileDelParm.pCallBackData = delCallBackData;
        FileDelParm.CallBack = delCallBack;
        
        *thrdCtrl = FsApi_FileCopyThread((ThreadFunPTR)FsApi_FileDelThreadFun,(T_pVOID)&FileDelParm,COPY_THREAD_PRI);
    }
    
    if (AK_NULL != *thrdCtrl)
    {
        ret = AK_TRUE;
    }
    
    return ret;
}

static T_VOID FsApi_FileDelAscThreadFun(T_U32 argc, T_VOID *argv)
{
    T_BOOL ret;
    T_FILEDEL_ASC_PARM *pFileDelParm = (T_FILEDEL_ASC_PARM *)argv;
    
    if (AK_NULL == pFileDelParm)
    {
        return;
    }

    Fwl_Print(C3, M_FS, "File Del Asc begin...\n");

    if (AK_NULL != pFileDelParm->CallBack.SetState)
    {
        pFileDelParm->CallBack.SetState(eFS_COPY_ING);
    }
    
    ret = File_DelAsc_DelCallBack(pFileDelParm->FileName,pFileDelParm->CallBack.Callback, \
                pFileDelParm->pCallBackData);
        
    Fwl_Print(C3, M_FS, "File Del Asc End ret:%d\n",ret);
    
    if (ret)
    {
        if (AK_NULL != pFileDelParm->CallBack.SetState)
        {
            pFileDelParm->CallBack.SetState(eFS_COPY_Success);
        }
    }
    else
    {
        if (AK_NULL != pFileDelParm->CallBack.SetState)
        {
            pFileDelParm->CallBack.SetState(eFS_COPY_Fail);
        }
    }
}


T_BOOL Fwl_FileDelAsc_CallBack(T_pCSTR FileName, T_FS_DEL_CALLBACK delCallBack, T_VOID *delCallBackData, T_COPY_CTRL **thrdCtrl)
{
    T_BOOL ret = AK_FALSE;
        
    if (FileName != AK_NULL)
    {
        if (AK_NULL != FileDelAscParm.CallBack.SetState)
        {
            FileDelAscParm.CallBack.SetState(eFS_COPY_ING);//init
        }
        
        Utl_StrCpyN(FileDelAscParm.FileName, FileName, FS_MAX_PATH_LEN);
        FileDelAscParm.pCallBackData = delCallBackData;
        FileDelAscParm.CallBack = delCallBack;
        
        *thrdCtrl = FsApi_FileCopyThread((ThreadFunPTR)FsApi_FileDelAscThreadFun,(T_pVOID)&FileDelAscParm,COPY_THREAD_PRI);
    }
    
    if (AK_NULL != *thrdCtrl)
    {
        ret = AK_TRUE;
    }
    
    return ret;
}

T_BOOL  Fwl_FsRmDir(T_pCWSTR path)
{
    AK_ASSERT_PTR(path, "path Is Invalid", AK_FALSE);

    return File_DelUnicode(path);
}

T_BOOL  Fwl_FsRmDirAsc(T_pSTR path)
{
    AK_ASSERT_PTR(path, "path Is Invalid", AK_FALSE);

    return File_DelAsc(path);
}

T_BOOL  Fwl_FsRmDirTree(T_pCWSTR path)
{
    T_BOOL ret = AK_FALSE;
    T_pFILE fd;

    AK_ASSERT_PTR(path, "path Is Invalid", AK_FALSE);
    
    fd = Fwl_FileOpen_Ex(path, _FMODE_READ, _FMODE_READ);
    AK_ASSERT_VAL(FS_INVALID_HANDLE != fd, "Open Path Failure", AK_FALSE);
    
    ret = Fwl_FileDeltree(fd);
    Fwl_FileClose(fd);
    
    return ret;
}


T_BOOL  Fwl_FsRmDirTreeAsc(T_pSTR path)
{
    T_BOOL ret = AK_FALSE;
    T_pFILE fd;

    AK_ASSERT_PTR(path, "path Is Invalid", AK_FALSE);

    fd = Fwl_FileOpenAsc_Ex(path, _FMODE_READ, _FMODE_READ);
    AK_ASSERT_VAL(FS_INVALID_HANDLE != fd, "Open Path Failure", AK_FALSE);
    
    ret = Fwl_FileDeltree(fd);
    Fwl_FileClose(fd);

    return ret;
}

T_BOOL  Fwl_FsMkDir(T_pCWSTR path)
{
    AK_ASSERT_PTR(path, "path Is Invalid", AK_FALSE);
    
    return File_MkdirsUnicode(path);
}


T_BOOL  Fwl_FsMkDirAsc(T_pSTR path)
{
    AK_ASSERT_PTR(path, "path Is Invalid", AK_FALSE);

    return File_MkdirsAsc(path);
}

T_BOOL  Fwl_FsMkDirTree(T_pCWSTR path)
{
    T_USTR_FILE fullDirPathName, tmpDirName;
    T_U32    curChIdx=0, pathStrLen;
    T_U8    tmpDirCnt = 0;
    T_BOOL ret = AK_FALSE;

    if (AK_NULL == path)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FsMkDirTree:Error Param\n");
        return AK_FALSE;
    }
    
    Utl_UStrCpyN(fullDirPathName, (T_U16 *)path,FS_MAX_PATH_LEN);
    pathStrLen = Utl_UStrLen(fullDirPathName);
    
    if (pathStrLen == 0)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FsMkDirTree:Error Len\n");
        return AK_FALSE;
    }

    if (Fwl_FileExist(path))
    {
        return AK_TRUE;//Folder is exist
    }

    curChIdx = 0;
    while( 1 )
    {
        if((curChIdx >= pathStrLen) || (curChIdx >= FS_MAX_PATH_LEN))
        {
            break;
        }
        
        if(UNICODE_SOLIDUS == fullDirPathName[curChIdx])
        {
            tmpDirCnt++;
            Utl_UStrCpyN(tmpDirName, fullDirPathName, curChIdx);
            tmpDirName[curChIdx] = 0;

            if( tmpDirCnt > 1)
            {
                if (!File_MkdirsUnicode(tmpDirName))
                {
                    Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FsMkDirTree ERROR [%d]!\n", tmpDirCnt);
                    return AK_FALSE;
                }
                else
                {
                    ret = AK_TRUE;
                }
            }
        }

        curChIdx++;

    }
    return ret;
}


T_BOOL  Fwl_FsMkDirTreeAsc(T_pSTR path)
{
    T_STR_FILE fullDirPathName, tmpDirName;
    T_U32    curChIdx=0, pathStrLen;
    T_U8    tmpDirCnt = 0;
    T_BOOL ret = AK_FALSE;

    if (AK_NULL == path)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FsMkDirTreeAsc:Error Param\n");
        return AK_FALSE;
    }
    
    strncpy(fullDirPathName,path,FS_MAX_PATH_LEN);
    pathStrLen = strlen(fullDirPathName);

    if (pathStrLen == 0)
    {
        return AK_FALSE;
    }
    curChIdx = 0;
    while(1)
    {
        if((curChIdx >= pathStrLen) || (curChIdx >= FS_MAX_PATH_LEN))
        {
            break;
        }
        
        if(((T_U8)UNICODE_SOLIDUS) == fullDirPathName[curChIdx])
        {
            tmpDirCnt++;
            strncpy(tmpDirName, fullDirPathName, curChIdx);
            tmpDirName[curChIdx] = 0;

            if( tmpDirCnt > 1)
            {
                if (!File_MkdirsAsc(tmpDirName))
                {
                    Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FsMkDirTreeAsc ERROR [%d]!\n", tmpDirCnt);
                    return AK_FALSE;
                }
        else
        {
                    ret = AK_TRUE;
                }
        }
        }

        curChIdx++;

    }
    return ret;
}



T_U32 Fwl_FileGetSize(T_pCWSTR path)
{
    T_PFILE    fp = AK_NULL;
    T_U32 low = 0;
    T_U32 excess = 0;
    
    fp = File_OpenUnicode(AK_NULL, path, FILE_MODE_READ);
    if ((T_U32)AK_NULL == fp)
    {
        return 0;
    }
#ifdef OS_ANYKA
    low = File_GetLength((T_PFILE)fp, &excess);
#else
    low = 128;
#endif
    File_Close(fp);

    return low;
}



T_U32 Fwl_FileGetSizeAsc(T_pSTR path)
{
    T_PFILE    fp = AK_NULL;
    T_U32 low = 0;
    T_U32 excess = 0;
    
    fp = File_OpenAsc(AK_NULL, path, FILE_MODE_READ);
    if ((T_U32)AK_NULL == fp)
    {
        return 0;
    }
    
    low = File_GetLength((T_PFILE)fp, &excess);
    File_Close(fp);

    return low;
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
        //AK_DEBUG_OUTPUT("FWL FILE: Fwl_GetFileLen: %d\n", low);
    }
    
    return low;
}


//srcStr must like as:path/parttern;
static T_S32 FsApi_GetFolderPath(T_USTR_FILE path, T_pCWSTR srcStr, T_U32 srcLen)
{
    T_U32   len = 0;
    T_pCWSTR psrcStr = AK_NULL;

    if ((AK_NULL == path) || (AK_NULL == srcStr) || (0 == srcLen))
    {
        return -1;
    }

    psrcStr = srcStr;
    len = srcLen;
    
    while ((len > 0) \
        && (psrcStr[len] != UNICODE_RES_SOLIDUS) \
        && (psrcStr[len] != UNICODE_SOLIDUS))
    {
        len--;
    }
    
    if (psrcStr[len] == UNICODE_RES_SOLIDUS || psrcStr[len] == UNICODE_SOLIDUS)
    {
        len++;
    }
    
    Utl_UStrCpyN(path,srcStr,len);

    return (T_S32)len;
}

// 必须和File_FindClose配合使用
T_hFILESTAT Fwl_FsFindFirst(T_pCWSTR pattern)
{
    T_hFILESTAT pFindCtrl = FS_INVALID_STATHANDLE;
    T_FINDBUFCTRL findBuf;
    T_USTR_FILE FindPath;
    T_S32 separteIdx = -1;

    if (pattern == AK_NULL)
    {
        Fwl_Print(C1, M_FS, "Fwl_FsFindFirst Param error\n");
        return FS_INVALID_STATHANDLE;
    }

    if ( pattern[0] == (T_U16)0)
    {
        Fwl_Print(C1, M_FS, "Fwl_FsFindFirst Param error\n");
        return FS_INVALID_STATHANDLE;
    }
    

    Utl_MemSet(FindPath, 0, sizeof(FindPath));
    separteIdx = FsApi_GetFolderPath(FindPath,pattern,Utl_UStrLen(pattern));

    if (separteIdx > 0)
    {
        findBuf.pattern = (T_U16*)&(pattern[separteIdx]);    
    }
    else
    {
        findBuf.pattern = (T_U16*)pattern;
    }
    
    findBuf.patternLen = (T_U8)Utl_UStrLen(findBuf.pattern);

    findBuf.NodeCnt= 1;     
    
    //判断是否查找深层文件夹或一层文件夹
    findBuf.type = FILTER_NOTITERATE;//FILTER_NOTITERATE;//FILTER_COVERNOTMATCH; // FILTER_CMP | FILTER_FOLDER;
    //findBuf.DspCtrl= 0;//DSP_PARENT;//(DSP_PARENT | DSP_CURRENT);     

    pFindCtrl =    File_FindFirst(FindPath, &findBuf);
    if(0 == pFindCtrl)
    {
        Fwl_Print(C1, M_FS, "Fwl_FsFindFirst: %d\n", pFindCtrl);
        return FS_INVALID_STATHANDLE;
    }

    return pFindCtrl;
}

// 必须和File_FindFirst 和File_FindClose配合使用
T_BOOL  Fwl_FsFindNext(T_hFILESTAT pFindCtrl)
{
    T_BOOL ret = AK_FALSE;
    
    if ((pFindCtrl != FS_INVALID_STATHANDLE)\
        && (1 == File_FindNext((T_U32)pFindCtrl, 1)))
    {
        ret = AK_TRUE;
    }
#if 0
    if (AK_TRUE != ret)
    {
        AK_DEBUG_OUTPUT("FWL FILE ERROR: Fwl_FsFindNext: %d\n", ret);
    }
#endif
    return ret;
}

T_VOID  Fwl_FsFindClose(T_hFILESTAT pFindCtrl)
{
    if (pFindCtrl != FS_INVALID_STATHANDLE)
    {
        File_FindClose((T_U32)pFindCtrl);
    }
}

//必须和File_FindFirst配合使用
T_pCWSTR Fwl_FsFindName(T_hFILESTAT pFindCtrl)
{
    T_PFILEINFO info = AK_NULL;
    
    Fwl_Print(C3, M_FS, "Fwl_FsFindName");

    if (FS_INVALID_STATHANDLE != pFindCtrl)
    {
        info = File_FindInfo((T_U32)pFindCtrl, 0, AK_NULL, AK_NULL);

        if (AK_NULL != info)
        {
            return info->LongName;
        }
    }

    return AK_NULL;
}


//必须和File_FindFirst配合使用
T_U32 Fwl_FsFindInfo(T_FILE_INFO *fileInfo, T_hFILESTAT pFindCtrl)
{
    T_PFILEINFO info = AK_NULL;
    
    if ((FS_INVALID_STATHANDLE != pFindCtrl) && (AK_NULL != fileInfo))
    {
        Utl_MemSet(fileInfo, 0, sizeof(T_FILE_INFO));
        
        info = File_FindInfo((T_U32)pFindCtrl, 0, AK_NULL, AK_NULL);
        if (AK_NULL != info)
        {
            fileInfo->time_create = ((info->CreatTime) | (info->CreatDate << 16));
            fileInfo->time_access = fileInfo->time_create;
            fileInfo->time_write  = ((info->ModTime) | (info->ModDate << 16));

            fileInfo->attrib = (T_U8)info->attr;

            //AK_DEBUG_OUTPUT("[Explorer]@@@@@@@@info->excess =%ld \n",info->excess);
            //AK_DEBUG_OUTPUT("[Explorer]@@@@@@@@info->length =%ld\n",info->length);

            fileInfo->size.high = (info->excess);
            fileInfo->size.low= (info->length);
        
    
            Utl_UStrCpyN(fileInfo->name, info->LongName, FS_MAX_PATH_LEN);
        }
    }
    return (T_U32)info;
}

//Fwl_FileFindFirstFromHandld必须与Fwl_FileFindCloseWithHandle合用
T_U32 Fwl_FileFindFirstFromHandle(T_U32 file)
{
    T_FINDBUFCTRL   FindCtrl;
    T_U16           Utmpstr[4];
    T_U32 FindHandle = 0;

    if (0 < file)
    {
        Eng_StrMbcs2Ucs("*.*", Utmpstr);
        FindCtrl.NodeCnt = 1;
        FindCtrl.pattern = Utmpstr;
        FindCtrl.type = FILTER_NOTITERATE;
        FindCtrl.patternLen = Utl_StrLen((T_pCSTR)Utmpstr);

        FindHandle = File_FindFirstFromHandle((T_PFILE)file, &FindCtrl);
    }
    
    return FindHandle;
}

T_U32 Fwl_FileFindOpen(T_U32 parent, T_U32 FileInfo)
{
    T_U32 file;

    if ((0 < parent) && (0 < FileInfo))
    {
        file = (T_U32)File_FindOpen((T_PFILE)parent, (T_PFILEINFO)FileInfo);
    }
    
    return file;
}

T_VOID Fwl_FileFindCloseWithHandle(T_U32 obj)
{
    if (0 < obj)
    {
        File_FindCloseWithHandle(obj);
    }
}

//获取创建日期最早的文件(夹)
T_U32  Fwl_GetOldFileByCreateTime(T_pCWSTR FolderPath,T_BOOL isDir,T_U32 reverseCount,
                                             T_pWSTR findfilelongName,T_U32 *createDate,T_U32 *createTime,
                                             T_U64_INT *totalFileSize)
{
    T_USTR_FILE Utmpstr;
    T_USTR_FILE FindFileName;
    T_USTR_FILE findRootPath;
    //-------------------
    T_FINDBUFCTRL findBuf;
    T_hFILESTAT find;
    T_PFILEINFO info = AK_NULL;
    //-------------------
    T_U32 fileCreateDate,fileCreateTime;
    T_BOOL ret = AK_FALSE;
    T_U32 fileCnt = 0;
    //------------------
    T_U64_INT  fileSize = {0};

    
    if (AK_NULL == FolderPath)
    {
        return 0;
    }
    
    Utl_UStrCpyN(findRootPath, FolderPath, FS_MAX_PATH_LEN);
    Eng_StrMbcs2Ucs("*.*", Utmpstr);
    
    findBuf.NodeCnt     = 1;
    findBuf.pattern     = Utmpstr;

    fileSize.high = 0;
    fileSize.low  = 0;
    if (isDir)
    {
        findBuf.type        = FILTER_NOTITERATE | FILTER_ONLYFOLDER;
    }
    else
    {
        findBuf.type        = FILTER_NOTITERATE | FILTER_ONLYFILE;
    }
    
    findBuf.patternLen     = Utl_StrLen((T_pCSTR)Utmpstr);
    
#ifdef DEBUG_GETOLDFILE
    Printf_UC(findRootPath);
    Printf_UC(findBuf.pattern);
    Fwl_Print(C3, M_FS, "%sclude Subdir,reverse Count %d\n",
        isDir?"In":"Ex",reverseCount);
#endif

    find =  File_FindFirst(findRootPath, &findBuf);
    if (0 == find)
    {
#ifdef DEBUG_GETOLDFILE
        Fwl_Print(C3, M_FS, "Find Noting\n");
#endif
        return 0;
    }
    fileCnt = 0;
    fileCreateDate = 0xFFFFFFFF;
    fileCreateTime = 0xFFFFFFFF;

    do {
        info = File_FindInfo((T_U32)find, 0, AK_NULL, AK_NULL);
        if (AK_NULL == info)
        {
            break;
        }

        // Miss ".",  ".."
        if (0 == Utl_UStrCmp(info->LongName, _T("."))
            || 0 == Utl_UStrCmp(info->LongName, _T("..")))
        {
            continue;
        }


#ifdef DEBUG_GETOLDFILE
        Fwl_Print(C3, M_FS, "attr = 0x%x ,date =%d,time =%d ",
            info->attr,
            info->CreatDate,
            info->CreatTime);
        Printf_UC(info->LongName);
#endif        
        if (FILE_ATTRIBUTE_DIRECTORY != (info->attr & FILE_ATTRIBUTE_DIRECTORY))
        {
            fileCnt++;
            
            if (AK_NULL != totalFileSize)
            {
                U64addU32(&fileSize,info->excess,info->length);
            }
        }

        if (info->CreatDate < fileCreateDate)
        {
            ret = AK_TRUE;
        }
        else if (info->CreatDate == fileCreateDate)
        {
            if(info->CreatTime < fileCreateTime)
            {
                ret = AK_TRUE;
            }
            else
            {
                ret = AK_FALSE;
            }
        }
        else
        {
            ret = AK_FALSE;
        }
        
        if (ret)
        {
            fileCreateDate =  info->CreatDate;
            fileCreateTime =  info->CreatTime;
            if (AK_NULL != findfilelongName)
            {
                Utl_UStrCpyN(FindFileName, info->LongName, FS_MAX_PATH_LEN);
            }
        }
    } while (File_FindNext((T_U32)find, 1)==1); 
    
    Fwl_FsFindClose(find);
    find =FS_INVALID_STATHANDLE;


    if (AK_NULL != totalFileSize)
    {
        totalFileSize->high = fileSize.high;
        totalFileSize->low  = fileSize.low;
    }
    
    if ((fileCnt > 0) && (fileCnt > reverseCount))
    {
        if (AK_NULL != findfilelongName)
        {
            Utl_UStrCpyN(findfilelongName, FindFileName, FS_MAX_PATH_LEN);
        }
        
        
        if (AK_NULL != createDate)
        {
            (*createDate) = fileCreateDate;
        }
        
        if (AK_NULL != createTime)
        {
            (*createTime) = fileCreateTime;
        }
        return fileCnt;
    }
    else
    {
#ifdef DEBUG_GETOLDFILE
        Fwl_Print(C1, M_FS, "Find Failed.cnt=%d\n",fileCnt);
#endif
        return 0;
    }
}


T_BOOL Fwl_FindOldArchiveByStamp(T_pCWSTR FolderPath, T_pWSTR findArchPath, T_BOOL isFindFile)
{
    T_hFILESTAT hFile;
    T_PFILEINFO info=AK_NULL;
    T_FILEINFO OldInfo;
    T_FINDBUFCTRL findBuf;
    T_USTR_FILE Utmpstr;
        
    AK_ASSERT_PTR(FolderPath, "File Old Archive By Stamp, FolderPath Is Invalid", AK_FALSE);
    AK_ASSERT_PTR(findArchPath, "File Old Archive By Stamp, findArchPath Is Invalid", AK_FALSE);

    Eng_StrMbcs2Ucs("*.*", Utmpstr);
    
    findBuf.NodeCnt     = 1;
    findBuf.pattern     = Utmpstr;
    findBuf.type         = FILTER_NOTITERATE;
    findBuf.patternLen     = Utl_StrLen((T_pCSTR)Utmpstr);

    if (FS_INVALID_STATHANDLE == (hFile = File_FindFirst(FolderPath, &findBuf)))
    {
        Fwl_Print(C1, M_FS, "Find First Return 0.\n");
        return AK_FALSE;
    }

    memset(&OldInfo, 0, sizeof(T_FILEINFO));
    OldInfo.CreatTime = 0xFFFFFFFF;
    OldInfo.CreatDate = 0xFFFFFFFF;
    
    do
    {        
        if (AK_NULL == (info = File_FindInfo((T_U32)hFile, 0, AK_NULL, AK_NULL)))
        {
            break;
        }
        
        // Filter File OR Folder
        if ((isFindFile && FILE_ATTRIBUTE_DIRECTORY != (info->attr & FILE_ATTRIBUTE_DIRECTORY))
            || (!isFindFile && info->attr & FILE_ATTRIBUTE_DIRECTORY))
        {
            // Miss ".",  ".."
            if (0 == Utl_UStrCmp(info->LongName, _T("."))
                || 0 == Utl_UStrCmp(info->LongName, _T("..")))
                continue;
            
            if (info->CreatDate < OldInfo.CreatDate
                || (info->CreatDate == OldInfo.CreatDate && info->CreatTime < OldInfo.CreatTime))
            {
                memcpy(&OldInfo, info, sizeof(T_FILEINFO));
                //AK_DEBUG_OUTPUT("find Older Archive.\n");
            }
        }
    } while (Fwl_FsFindNext(hFile));    
    
    Fwl_FsFindClose(hFile);

    if (OldInfo.CreatDate < 0xFFFFFFFF)
    {
        Utl_UStrCpy(findArchPath, FolderPath);
        Utl_UStrCat(findArchPath, OldInfo.LongName);
        
        return AK_TRUE;
    }

    return AK_FALSE;
}

T_U16 Fwl_GetDiskList(T_CHR drvDesChar[], T_U8 pDiskType[], T_U16 DiskNum)
{
    T_U16       diskQty = 0;
#ifdef OS_ANYKA
    T_DRIVER_INFO DriverInfo;
    T_BOOL Ret;
    
    Ret = FS_GetFirstDriver(&DriverInfo);
    for (diskQty = 0; Ret && (diskQty < DiskNum); diskQty++)
    {
        if (AK_NULL != drvDesChar)
        {
            drvDesChar[diskQty] = DriverInfo.DriverID + (T_CHR)UNICODE_A;
        }
        
        if (AK_NULL != pDiskType)
        {
        #ifdef USB_HOST
            if (UsbHost_DriveIsMnt(DriverInfo.DriverID))
            {
                pDiskType[diskQty]    = MEDIUM_USBHOST;
            }
            else
        #endif
            {
                pDiskType[diskQty]    = DriverInfo.nMainType;
            }
        }
#if 0
        Fwl_Print(C3, M_FS, "Drv[%d] MT[%d] ST[%d] \n",DriverInfo.DriverID,
                 DriverInfo.nMainType,
                 DriverInfo.nSubType);
#endif
        Ret = FS_GetNextDriver(&DriverInfo);
    }
#else
    drvDesChar[0] = 'A';
    pDiskType[0] = MEDIUM_NANDFLASH;

    drvDesChar[1] = 'B';
    pDiskType[1] = MEDIUM_NANDFLASH;
    
    drvDesChar[2] = 'C';
    pDiskType[2] = MEDIUM_NANDFLASH;

    drvDesChar[3] = 'D';
    pDiskType[3] = MEDIUM_NANDFLASH;

    drvDesChar[4] = 'E';
    pDiskType[4] = MEDIUM_SD;

    diskQty = 5;
#endif
    return diskQty;
}



T_VOID Fwl_DiskDump(T_U8 *tips)
{
    Fwl_Print(C4, M_FS, "\n[DiskDump]==%s==>>>>>\n",tips);
    Fwl_GetDiskList(AK_NULL, AK_NULL, FS_MAX_DISK_CNT);
    Fwl_Print(C4, M_FS, "[DiskDump]==%s==<<<<<\n",tips);
}


T_VOID Fwl_FsGetSize(T_WCHR drvDesChar, T_U64_INT *size64)
{
    T_U8 drvId = 0;
    
#ifdef OS_ANYKA
    if (AK_NULL != size64)
    {
        //dirverStr[0] = drvDesChar;
        //dirverStr[1] = UNICODE_COLON;
        //dirverStr[2] = 0;
        
        drvId = Uni_ToUpper(drvDesChar) - UNICODE_A;
        size64->low = 0;
        size64->high = 0;
        size64->low = FS_GetDriverCapacity(drvId,&size64->high);
    }
#endif    
}


T_VOID Fwl_FsGetUsedSize(T_WCHR drvDesChar, T_U64_INT *size64)
{
    T_U8 drvId = 0;

#ifdef OS_ANYKA
    if (AK_NULL != size64)
    {
        //dirverStr[0] = drvDesChar;
        //dirverStr[1] = UNICODE_COLON;
        //dirverStr[2] = 0;
        
        drvId = Uni_ToUpper(drvDesChar) - UNICODE_A;
        size64->low = 0;
        size64->high = 0;
        size64->low = FS_GetDriverUsedSize(drvId,&size64->high);

		Fwl_Print(C3, M_FS, "debug -- high:%lu, low:%lu\r\n", size64->high, size64->low);        
    }
#endif    
}

T_VOID Fwl_FsGetFreeSize(T_WCHR drvDesChar, T_U64_INT *size64)
{
    T_U8 drvId = 0;
    
#ifdef OS_ANYKA
    if (AK_NULL != size64)
    {
        //dirverStr[0] = drvDesChar;
        //dirverStr[1] = UNICODE_COLON;
        //dirverStr[2] = 0;
        
        drvId = Uni_ToUpper(drvDesChar) - UNICODE_A;
        size64->low = 0;
        size64->high = 0;
        size64->low = FS_GetDriverFreeSize(drvId,&size64->high);
    }
#endif    
}

T_BOOL  Fwl_FileMove(T_pCWSTR oldFile,T_pCWSTR newFile)
{
    T_BOOL ret = AK_FALSE;
    
    if ((oldFile != AK_NULL) && (newFile != AK_NULL))
    {
        ret = FsApi_MoveFile((T_pVOID)oldFile,(T_pVOID)newFile,AK_TRUE);
    }

    if (AK_TRUE != ret)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileMove: %d\n", ret);
    }

    return ret;
}

T_BOOL    Fwl_FolderMove(T_pCWSTR oldFile,T_pCWSTR newFile,T_FS_COPY_CALLBACK CallBackFun,T_COPY_CTRL **thrdCtrl)
{
    T_BOOL ret = AK_FALSE;
    T_U8 srcDrvId = 0, destDrvId = 0;

    if ((oldFile != AK_NULL) && (newFile != AK_NULL))
    {
        srcDrvId = (T_U8)Fwl_GetDriverIdByPath(oldFile);
        destDrvId = (T_U8)Fwl_GetDriverIdByPath(newFile);
        if (srcDrvId != destDrvId) //目前文件系统只支持同驱动盘reName
        {
            return AK_FALSE;
        }
        
        if (AK_NULL != FolderMoveParm.CallBack.SetState)
        {
            FolderMoveParm.CallBack.SetState(eFS_COPY_ING);//init
        }
        
        Utl_UStrCpyN(FolderMoveParm.sourFile, oldFile, FS_MAX_PATH_LEN);
        Utl_UStrCpyN(FolderMoveParm.destFile, newFile, FS_MAX_PATH_LEN);
        FolderMoveParm.CallBack = CallBackFun;
        
        *thrdCtrl = FsApi_FileCopyThread((ThreadFunPTR)FsApi_FolderMoveThreadFun,(T_pVOID)&FolderMoveParm,COPY_THREAD_PRI);
    }
    
    if (0 != *thrdCtrl)
    {
        ret = AK_TRUE;
    }
    
    return ret;
}


T_BOOL  Fwl_FileMoveAsc(T_pSTR oldFile,T_pSTR newFile)
{
    T_BOOL ret = AK_FALSE;
    
    if ((oldFile != AK_NULL) && (newFile != AK_NULL))
    {
        ret = FsApi_MoveFile(oldFile,newFile,AK_FALSE);
    }

    if (AK_TRUE != ret)
    {
        Fwl_Print(C1, M_FS, "FWL FILE ERROR: Fwl_FileMoveAsc: %d\n", ret);
    }

    return ret;
}
static T_VOID FsApi_FileCopyThreadFun(T_U32 argc, T_VOID *argv)
{
    T_BOOL ret;
    T_FILECOPY_PARM *pFileCopyParm = (T_FILECOPY_PARM *)argv;
    
    if (AK_NULL == pFileCopyParm)
    {
        return;
    }

    Printf_UC(pFileCopyParm->sourFile);
    Printf_UC(pFileCopyParm->destFile);
    Fwl_Print(C3, M_FS, "File Copy begin...\n");

    if (AK_NULL != pFileCopyParm->CallBack.SetState)
    {
        pFileCopyParm->CallBack.SetState(eFS_COPY_ING);
    }
    
    ret = File_CopyUnicode(pFileCopyParm->sourFile,pFileCopyParm->destFile, pFileCopyParm->replace, \
                pFileCopyParm->CallBack.Callback,pFileCopyParm->pCallBackData);

    Fwl_Print(C3, M_FS, "File Copy End ret:%d\n",ret);
    
    if (ret)
    {
        if (AK_NULL != pFileCopyParm->CallBack.SetState)
        {
            pFileCopyParm->CallBack.SetState(eFS_COPY_Success);
        }
    }
    else
    {
        File_GetLastErrorType();
        if (AK_NULL != pFileCopyParm->CallBack.SetState)
        {
            pFileCopyParm->CallBack.SetState(eFS_COPY_Fail);
        }
    }
}

T_BOOL  Fwl_FileCopy(T_pCWSTR sourFile,T_pCWSTR destFile, T_BOOL replace, T_VOID *pCallBackData, T_FS_COPY_CALLBACK CallBackFun, T_COPY_CTRL **thrdCtrl)
{
    T_BOOL ret=AK_FALSE;
    
    if ((sourFile != AK_NULL) && (destFile != AK_NULL))
    {
        
        if (AK_NULL != FileCopyParm.CallBack.SetState)
        {
            FileCopyParm.CallBack.SetState(eFS_COPY_ING);//init
        }
        
        Utl_UStrCpyN(FileCopyParm.sourFile, sourFile, FS_MAX_PATH_LEN);
        Utl_UStrCpyN(FileCopyParm.destFile, destFile, FS_MAX_PATH_LEN);
        FileCopyParm.replace = replace;
        FileCopyParm.pCallBackData = pCallBackData;
        FileCopyParm.CallBack = CallBackFun;
        
        *thrdCtrl = FsApi_FileCopyThread((ThreadFunPTR)FsApi_FileCopyThreadFun,(T_pVOID)&FileCopyParm,COPY_THREAD_PRI);
    }
    
    if (0 != *thrdCtrl)
    {
        ret = AK_TRUE;
    }
    
    return ret;
}

static T_VOID FsApi_FileCopyAscThreadFun(T_U32 argc, T_VOID *argv)
{
    T_BOOL ret;
    T_FILECOPY_ASC_PARM *pFileCopyAscParm = (T_FILECOPY_ASC_PARM *)argv;
    
    if (AK_NULL == pFileCopyAscParm)
    {
        return;
    }

    Fwl_Print(C3, M_FS, "Sour:%s,dest:%s\n",pFileCopyAscParm->sourFile,pFileCopyAscParm->destFile);

    Fwl_Print(C3, M_FS, "File Copy Asc begin...\n");

    if (AK_NULL != pFileCopyAscParm->CallBack.SetState)
    {
        pFileCopyAscParm->CallBack.SetState(eFS_COPY_ING);
    }
    
    ret = File_CopyAsc(pFileCopyAscParm->sourFile,pFileCopyAscParm->destFile, pFileCopyAscParm->replace, \
                pFileCopyAscParm->CallBack.Callback,pFileCopyAscParm->pCallBackData);

    Fwl_Print(C3, M_FS, "File Copy Asc End ret:%d\n",ret);
    
    if (ret)
    {
        if (AK_NULL != pFileCopyAscParm->CallBack.SetState)
        {
            pFileCopyAscParm->CallBack.SetState(eFS_COPY_Success);
        }
    }
    else
    {
        if (AK_NULL != pFileCopyAscParm->CallBack.SetState)
        {
            pFileCopyAscParm->CallBack.SetState(eFS_COPY_Fail);
        }
    }
}

T_BOOL  Fwl_FileCopyAsc(T_pCSTR sourFile,T_pCSTR destFile, T_BOOL replace, T_VOID *pCallBackData, T_FS_COPY_CALLBACK CallBackFun, T_COPY_CTRL **thrdCtrl)
{
    T_BOOL ret=AK_FALSE;
    
    if ((sourFile != AK_NULL) && (destFile != AK_NULL))
    {
        
        if (AK_NULL != FileCopyAscParm.CallBack.SetState)
        {
            FileCopyAscParm.CallBack.SetState(eFS_COPY_ING);//init
        }
        
        Utl_StrCpyN(FileCopyAscParm.sourFile, sourFile, FS_MAX_PATH_LEN);
        Utl_StrCpyN(FileCopyAscParm.destFile, destFile, FS_MAX_PATH_LEN);
        FileCopyAscParm.replace = replace;
        FileCopyAscParm.pCallBackData = pCallBackData;
        FileCopyAscParm.CallBack = CallBackFun;
                                
        *thrdCtrl = FsApi_FileCopyThread((ThreadFunPTR)FsApi_FileCopyAscThreadFun,(T_pVOID)&FileCopyAscParm,COPY_THREAD_PRI);
    }
    
    if (0 != *thrdCtrl)
    {
        ret = AK_TRUE;
    }
    return ret;
}


T_BOOL Fwl_IsRootDir(T_pCWSTR pFilePath)
{
    T_USTR_FILE filePath;
    T_BOOL ret = AK_TRUE;
    T_U32 i;
    T_U32 charNo;
    T_U32 count = 0;

    if (AK_NULL == pFilePath)
    {
        return AK_FALSE;
    }
    
    

    Utl_UStrCpyN(filePath, (T_U16 *)pFilePath,FS_MAX_PATH_LEN);
    charNo = Utl_UStrLen(filePath);

    if (charNo == 0)
    {
        return AK_FALSE;
    }

    if (filePath[charNo - 1] != UNICODE_SOLIDUS && filePath[charNo - 1] != UNICODE_RES_SOLIDUS)
    {
        filePath[charNo] = UNICODE_SOLIDUS;
        filePath[charNo + 1] = 0;
        charNo++;
    }

    for(i=0; i<charNo; i++)
    {
        if (filePath[i] == UNICODE_SOLIDUS || filePath[i] == UNICODE_RES_SOLIDUS)
        {
            count++;
            if (count > 1)
            {
                ret = AK_FALSE;
                break;
            }
        }
    }

    return ret;
}

T_BOOL Fwl_GetRootDir(T_pCWSTR pFilePath, T_pWSTR pRootDir)
{
    T_USTR_FILE Ustr_tmp;

    RAM_ASSERT_PTR(pFilePath, "Fwl_GetRootDir: pFilePath is NULL", AK_FALSE);
    RAM_ASSERT_PTR(pRootDir, "Fwl_GetRootDir: pRootDir is NULL", AK_FALSE);

    Eng_StrMbcs2Ucs(":\\", Ustr_tmp);
    Utl_UStrCpyN(pRootDir, (T_U16 *)pFilePath, 1);
    *(pRootDir + 1) = 0;
    Utl_UStrCat(pRootDir, Ustr_tmp);

    return AK_TRUE;
}

T_BOOL Fwl_ResetDirRoot(T_pWSTR pRootDir, T_pWSTR pFilePath)
{
    RAM_ASSERT_PTR(pFilePath, "Fwl_ResetDirRoot: pFilePath is NULL", AK_FALSE);
    RAM_ASSERT_PTR(pRootDir, "Fwl_ResetDirRoot: pRootDir is NULL", AK_FALSE);

    // change filePath  first 2 Chars as the pRootDir first 2 Chars 
    Utl_MemCpy(pFilePath, pRootDir, sizeof(T_U16) * 2);

    return AK_TRUE;
}

T_U8 Fwl_GetDriverTypeById(T_U8 DriverID)
{
    T_DRIVER_INFO drvInfo;

    if (FS_GetDriver(&drvInfo, DriverID))
    {
        return drvInfo.nMainType;
    }
    else
    {
        return T_U8_MAX;
    }
}
T_U16 Fwl_GetDriverIdByPath(T_pCWSTR path)
{
    T_U16 PathDrvChar;
    T_U16 drvId=0;
    
    memcpy(&PathDrvChar,path,sizeof(T_U16));
    drvId = Uni_ToUpper(PathDrvChar) - UNICODE_A;

    return drvId;
}
/*
 *@brief: Is The Device Mobile. e.g. SD/USB
 */
T_BOOL Fwl_IsInMobilMedium(T_pCWSTR pFilePath)
{
    T_DRIVER_INFO drvInfo;
    
    if (AK_NULL == pFilePath)
    {
        return AK_FALSE;
    }
    if (FS_GetDriver(&drvInfo, (T_U8)Fwl_GetDriverIdByPath(pFilePath)))
    {
        if ((MEDIUM_SD ==drvInfo.nMainType) \
            || (MEDIUM_USBHOST == drvInfo.nMainType))
        {
            return AK_TRUE;
        }
    }
    
    return AK_FALSE;
}

//检测文件对应存储介质的有效性
T_BOOL Fwl_CheckDriverIsValid(T_pCWSTR pFilePath)
{
    T_eINTERFACE_TYPE SdInterface;
    T_DRIVER_INFO drvInfo;
    T_BOOL ret = AK_FALSE;
    
    if (AK_NULL == pFilePath)
    {
        return AK_FALSE;
    }

    ret = FS_GetDriver(&drvInfo, (T_U8)Fwl_GetDriverIdByPath(pFilePath));
    if(!ret)
    {
        return AK_FALSE;
    }
   //在相应的驱动类型下加入其物理介质存在的有效性判断
    switch(drvInfo.nMainType)
    {
    case MEDIUM_SD:
        SdInterface = Fwl_Sd_GetInterfaceByID(drvInfo.DriverID);
        if (eSD_INTERFACE_COUNT > SdInterface)
        {
            ret = AK_TRUE;
        }
        else
        {
            ret = AK_FALSE;
        }
        break;
#ifdef USB_HOST
    case MEDIUM_USBHOST:
        ret = Fwl_UsbHostIsConnect();
        break;
#endif
    case MEDIUM_PARTITION:
        ret = AK_TRUE;
        break;
        
    default:
        break;    
    }
    
    return ret;
    
}

//************************************************************************************
static T_U32 FsApi_GetChipId(T_VOID)
{
#ifdef OS_ANYKA

    return FS_AK37XX_C;
#else	
	return FS_AK37XX;
#endif
}

static T_VOID FsApi_OutStream(T_U16 ch)
{
    Fwl_Print(C3, M_FS, "%c", ch);
}

static T_U8 FsApi_InStream(T_VOID)
{
    T_U8 ch = 'c';
    ch = getch() & 0xff;
    return ch;
}



static T_S32 FsApi_AnsiStr2UniStr(const T_pSTR pAnsiStr, T_U32 AnsiStrLen,T_U16 *pUniBuf, T_U32 UniBufLen, T_U32 code)
{
    T_S32 ret = -1;
    
#ifdef FS_DEBG_API
    //FsApi_Dump("A_U_A", pAnsiStr, AnsiStrLen);
#endif

#if 0
    T_U32 i = 0;
    
    if (pUniBuf != AK_NULL)
    {
        while (i != AnsiStrLen)
        {
            pUniBuf[i] = (T_U16)pAnsiStr[i];
            i++;
        }
        pUniBuf[i] = 0;
    }

    return AnsiStrLen;
#else
    ret = AnsiStr2UniStr(pAnsiStr,AnsiStrLen,pUniBuf,UniBufLen,code);
#endif

#ifdef FS_DEBG_API
    //FsApi_Dump("A_U_U", pUniBuf, 2*UniBufLen);
#endif

    return ret;
}


static T_S32 FsApi_UniStr2AnsiStr(const T_U16 *pUniStr, T_U32 UniStrLen,T_pSTR pAnsibuf, T_U32 AnsiBufLen, T_U32 code)
{
    T_S32 ret = -1;
    
#ifdef FS_DEBG_API
    //FsApi_Dump("U_A_U", pUniStr, 2*UniStrLen);
#endif

#if 0
    T_U32 i = 0;

    if (pUniStr != AK_NULL)
    {
        while (i != UniStrLen)
        {
            pAnsibuf[i] = (T_U8)pUniStr[i];
            i++;
        }
        pAnsibuf[i] = 0;
    }
    return UniStrLen;
#else
    ret = UniStr2AnsiStr(pUniStr,UniStrLen,pAnsibuf, AnsiBufLen, code);
#endif

#ifdef FS_DEBG_API
    //FsApi_Dump("U_A_A", pAnsibuf, AnsiBufLen);
#endif
    return ret;
}

static T_U32 FsApi_GetSecond(T_VOID)
{
    T_U32 current_time = 0;
    T_U16 year         = 0;
    T_U16 TempYear     = 0;
    T_U16 month        = 0;
    T_U16 MonthToDays  = 0;
    T_U16 day          = 0;
    T_U16 hour         = 0;
    T_U32 minute       = 0;
    T_U32 second       = 0;
      T_U16 i            = 0;
#ifdef OS_ANYKA
    T_SYSTIME systime;
#endif

    T_U16 std_month_days[13]  = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    T_U16 leap_month_days[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#ifdef OS_ANYKA
    systime = GetSysTime();

    year = systime.year;
    month = systime.month;
    day = systime.day;
    hour = systime.hour;
    minute = systime.minute;
    second = systime.second;

    if (year < 1980)
    {
        /* Typho2257, Jan.2,07 - Modified the default year to 2007 */
        year = 2007;                                //default
    }
        

    /* BEGIN Typho5369, Dec24,06 - Modified the calculating: It is not true that
       each 4 years has one leap year! */
    for (TempYear = year - 1; TempYear >= 1980; TempYear--)
    {
        /* the case of leap year */
        if ( TempYear % 4 == 0 && (TempYear % 100 != 0 || TempYear % 400 == 0))
        {
            current_time += 31622400; // the seconds of a leap year
        }
        else
        {
            /* not a leap year */
            current_time += 31536000;  // the seconds of a common year(not a leap one)
        }
    }

    /* calculate the current year's seconds */

    if ((month < 1) || (month > 12))
    {
        /* get the default value. */
        month = 1;
    }

    /* the current year is a leap one */
    if ( year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
    {
        for (i = 1; i < month; i++)
        {
            MonthToDays += leap_month_days[i];
        }
    }
    else
    {
        /* the current year is not a leap one */
        for (i = 1; i < month; i++)
        {
            MonthToDays += std_month_days[i];
        }
    }

    if ((day < 1) || (day > 31))
    {
        /* get the default value */
        day = 1;
    }
    MonthToDays += (day - 1);

    /* added the past days of this year(change to seconds) */
    current_time += MonthToDays * 24 * 3600;

    /* added the current day's time(seconds) */
    current_time += hour * 3600 + minute * 60 + second;
#endif // #ifdef OS_ANYKA

    return current_time;
}


static T_VOID FsApi_SetSecond(T_U32 seconds)
{
    return;
}

static T_pVOID    FsApi_Alloc(T_U32 size, T_S8 *filename, T_U32 fileline)
{
    T_pVOID ptr = Fwl_MallocAndTrace(size, (T_pSTR)filename, fileline);
    if(AK_NULL == ptr)
        Fwl_Print(C1, M_FS, "FsApi_Alloc : malloc error");
    return ptr;
}


static T_pVOID FsApi_Realloc(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline)
{
    T_pVOID ptr = Fwl_ReMallocAndTrace(var, size, (T_pSTR)filename, fileline);
    if(AK_NULL == ptr)
        Fwl_Print(C3, M_FS, "FsApi_Realloc : malloc error");
    return ptr;
}

static T_pVOID  FsApi_Free(T_pVOID var, T_S8 *filename, T_U32 fileline)
{
    return Fwl_FreeAndTrace(var, (T_pSTR)filename, fileline);
}

static T_S32 FsApi_Create_Semaphore(T_U32 initial_count, T_U8 suspend_type, T_S8 *filename, T_U32 fileline)
{
    return AK_Create_Semaphore(initial_count, suspend_type);
}

static T_S32 FsApi_Delete_Semaphore(T_S32 semaphore, T_S8 *filename, T_U32 fileline)
{
    return AK_Delete_Semaphore(semaphore);
}
static T_S32 FsApi_Obtain_Semaphore(T_S32 semaphore, T_U32 suspend, T_S8 *filename, T_U32 fileline)
{
    return AK_Obtain_Semaphore(semaphore, suspend);
}
static T_S32 FsApi_Release_Semaphore(T_S32 semaphore, T_S8 *filename, T_U32 fileline)
{
    return AK_Release_Semaphore(semaphore);
}

static     T_VOID FsApi_SysReset(T_VOID)
{
    Fwl_Print(C3, M_FS, "FsApi_SysReset -- entry\n");
}

static T_VOID FsApi_RandSeed(T_VOID)
{
    srand(get_tick_count());
}

static T_U32 FsApi_Rand(T_U32 MaxVal)
{
    T_U32 val = 0;
    
    val = (T_U32)rand() % MaxVal;
    return val;
}

#define MOUNT_THRD_STACK_SIZE   (10*1024)
typedef struct
{
    T_hTask thread;
    T_U8 Stack[MOUNT_THRD_STACK_SIZE];
} T_MNT_CTRL,*T_PMNT_CTRL;

static T_U32 FsApi_MountThread(ThreadFunPTR Fun, T_pVOID pData, T_U32 priority)
{
    T_PMNT_CTRL thrdCtrl = AK_NULL;
        
    thrdCtrl = Fwl_Malloc(sizeof(T_MNT_CTRL));

    if (AK_NULL == thrdCtrl)
    {
        Fwl_Print(C1, M_FS, "FsApi_MountThread Malloc Error\n");
        return 0;
    }

    if (50 == priority)
    {
        thrdCtrl->thread = AK_Create_Task((T_VOID*)Fun, "AsynThread",
                        1, pData, 
                       thrdCtrl->Stack, MOUNT_THRD_STACK_SIZE,
                       100, 5,
                       AK_PREEMPT,AK_START);
    }
    else
    {
    thrdCtrl->thread = AK_Create_Task((T_VOID*)Fun, "MountThread",
                        1, pData, 
                       thrdCtrl->Stack, MOUNT_THRD_STACK_SIZE,
                       100, 1,
                       AK_PREEMPT,AK_START);
    }
    if (AK_IS_INVALIDHANDLE(thrdCtrl->thread))
    {
        Fwl_Print(C2, M_FS, "FsApi_MountThread Create_Task Error\n");
        Fwl_Free(thrdCtrl);
        thrdCtrl = AK_NULL;
        return 0;
    }

    return (T_U32)thrdCtrl;
}
static T_VOID FsApi_KillThead(T_U32 ThreadHandle)
{   
    T_PMNT_CTRL thrdCtrl = (T_PMNT_CTRL)ThreadHandle;

    if (AK_NULL != thrdCtrl)
    {
        AK_Terminate_Task(thrdCtrl->thread);
        AK_Delete_Task(thrdCtrl->thread);
        Fwl_Free(thrdCtrl);
        thrdCtrl = AK_NULL;
    }
}

static T_PCOPY_CTRL FsApi_FileCopyThread(ThreadFunPTR Fun, T_pVOID pData, T_U32 priority)
{
    T_PCOPY_CTRL thrdCtrl = AK_NULL;
        
    thrdCtrl = (T_PCOPY_CTRL)Fwl_Malloc(sizeof(T_COPY_CTRL));
    Fwl_Print(C3, M_FS, "FsApi_FileCopyThread malloc:0x%x\n",thrdCtrl);
    
    if (AK_NULL == thrdCtrl)
    {
        Fwl_Print(C1, M_FS, "FsApi_FileCopyThread Malloc Error\n");
        return 0;
    }

    thrdCtrl->thread = AK_Create_Task((T_VOID*)Fun, "FileCopy",
                    1, pData, 
                   thrdCtrl->Stack, FILE_COPY_THRD_STACK_SIZE,
                   (T_OPTION)priority, COPY_THREAD_TIMESLICE,
                   (T_OPTION)AK_PREEMPT,(T_OPTION)AK_START);

    if (AK_IS_INVALIDHANDLE(thrdCtrl->thread))
    {
        Fwl_Print(C2, M_FS, "FsApi_FileCopyThread Create_Task Error\n");
        Fwl_Free(thrdCtrl);
        thrdCtrl = AK_NULL;
        return 0;
    }

    return thrdCtrl;
}


static T_VOID FsApi_KillCopyThead(T_COPY_CTRL **ThreadHandle)
{   
    T_PCOPY_CTRL thrdCtrl = AK_NULL;

    if (AK_NULL == ThreadHandle)
    {
        return;
    }
    
    thrdCtrl = *ThreadHandle;

    if (AK_NULL != thrdCtrl)
    {
        AK_Terminate_Task(thrdCtrl->thread);
        AK_Delete_Task(thrdCtrl->thread);
        Fwl_Print(C3,M_FS,"FsApi_KillCopyThead free:0x%x\n",thrdCtrl);
        Fwl_Free(thrdCtrl);
        *ThreadHandle = AK_NULL;
    }
}

T_VOID Fwl_KillCopyThead(T_COPY_CTRL **ThreadHandle)
{
    FsApi_KillCopyThead(ThreadHandle);
}

T_VOID FsApi_SleepMs(T_U32 ms)
{
    AK_Sleep((ms + 4)/5);
}

#define FS_MUTI_THRD   0x00000001
#define FS_DBG_PRINT   0x00000004

#ifdef OS_WIN32

T_VOID Global_Initial(T_PFSINITINFO fsInitInfo);

static T_VOID Fwl_InitFsInfo(T_U32 InitMode)
{
    T_FSINITINFO    fsInitInfo = {0};

//    list_head_init(&fs_alloc_list);
    fsInitInfo.out = FsApi_OutStream;
    fsInitInfo.in = FsApi_InStream; 
    fsInitInfo.fGetSecond = FsApi_GetSecond;
    fsInitInfo.fSetSecond = FsApi_SetSecond;
    
    fsInitInfo.fUniToAsc = FsApi_UniStr2AnsiStr;
    fsInitInfo.fAscToUni = FsApi_AnsiStr2UniStr;

    fsInitInfo.fRamAlloc = FsApi_Alloc;
    fsInitInfo.fRamRealloc = FsApi_Realloc;
    fsInitInfo.fRamFree = FsApi_Free;
    
    fsInitInfo.fCrtSem    = FsApi_Create_Semaphore;
    fsInitInfo.fDelSem    = FsApi_Delete_Semaphore;
    fsInitInfo.fObtSem    = FsApi_Obtain_Semaphore;
    fsInitInfo.fRelSem    = FsApi_Release_Semaphore;
    
    (T_pVOID)fsInitInfo.fMemCpy = (T_pVOID)memcpy;
    (T_pVOID)fsInitInfo.fMemSet = (T_pVOID)memset;
    (T_pVOID)fsInitInfo.fMemMov = (T_pVOID)memmove;
    (T_pVOID)fsInitInfo.fMemCmp = (T_pVOID)memcmp;
    
    if (InitMode & FS_DBG_PRINT)
    {
        Fwl_Print(C3, M_FS, "[Fwl_InitFsInfo]Enable DebugPrint\n");
        fsInitInfo.fPrintf = printf;
    }
    else
    {
        Fwl_Print(C3, M_FS, "[Fwl_InitFsInfo]Disable DebugPrint\n");
        fsInitInfo.fPrintf = AK_NULL;
    }


    fsInitInfo.fGetChipId  = FsApi_GetChipId;


    Global_Initial((T_PFSINITINFO)(&fsInitInfo));
}
#endif

static T_BOOL   Fwl_InitFsCb(T_U32 InitMode)
{
#ifdef OS_ANYKA
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

    if (InitMode & FS_DBG_PRINT)
    {
        Fwl_Print(C3, M_FS, "[Fwl_InitFsCb]Enable DebugPrint\n");
        fsGlbCb.fPrintf = AkDebugOutput;
    //    fsGlbCb.fPrintf = printf;
    }
    else
    {
        Fwl_Print(C3, M_FS, "[Fwl_InitFsCb]Disable DebugPrint\n");
        fsGlbCb.fPrintf = AK_NULL;
    }
    
    fsGlbCb.fGetChipId  = FsApi_GetChipId;


    fsGlbCb.fSysRst= FsApi_SysReset;
    fsGlbCb.fRandSeed    = FsApi_RandSeed;
    fsGlbCb.fGetRand = FsApi_Rand;

    if (InitMode & FS_MUTI_THRD)
    {
        Fwl_Print(C3, M_FS, "[Fwl_InitFsCb]Enable MutiThread\n");
        fsGlbCb.fMountThead = FsApi_MountThread;
        fsGlbCb.fKillThead  = FsApi_KillThead;
    }
    else
    {
        Fwl_Print(C3, M_FS, "[Fwl_InitFsCb]Disable MutiThread\n");
        fsGlbCb.fMountThead = AK_NULL;
        fsGlbCb.fKillThead  = AK_NULL;
    }
    fsGlbCb.fSystemSleep = FsApi_SleepMs;

    
    return FS_InitCallBack(&fsGlbCb, 128);
#else
    return AK_FALSE;
#endif
}

#ifdef OS_ANYKA
T_pCSTR Fwl_FsGetLibVersion(T_VOID)
{
    return (T_pCSTR)FS_GetVersion();
}
#endif

T_U8 Fwl_ChkDsk(T_VOID)
{
#ifdef OS_ANYKA
    Fwl_Print(C3, M_FS, "\n#############check disk Begin...\n");
    FS_ChkDsk(0,AK_NULL, AK_NULL);
    FS_ChkDsk(1,AK_NULL, AK_NULL);
    FS_ChkDsk(2,AK_NULL, AK_NULL);
    Fwl_Print(C3, M_FS, "\n#############check disk End\n");
#endif
    return 0;
}

static T_hSemaphore            gFsLock = 0;

T_VOID Fwl_LockFs(T_VOID)
{
    if (AK_IS_VALIDHANDLE(gFsLock))
    {
       AK_Obtain_Semaphore(gFsLock, AK_SUSPEND);
    }
}

T_VOID Fwl_UnLockFs(T_VOID)
{
    if (AK_IS_VALIDHANDLE(gFsLock))
    {
       AK_Release_Semaphore(gFsLock);
    }
}

T_BOOL Fwl_InitFs(T_VOID)
{
    T_U32 i;
    T_eINTERFACE_TYPE SdMount[eSD_INTERFACE_COUNT];
    T_BOOL SdRet = AK_FALSE;

    Fwl_Print(C3, M_FS, "init fs Begin..");
#ifdef OS_WIN32
    Fwl_InitFsInfo(FS_MUTI_THRD|FS_DBG_PRINT);
#endif
    Fwl_InitFsCb(FS_MUTI_THRD|FS_DBG_PRINT);
    Fwl_Print(C3, M_FS, "\n[FSINIT]Ok.<MaxPathLen = %d>\n",FS_MAX_PATH_LEN);

#ifdef NANDBOOT
    Fwl_Print(C3, M_FS, "Fwl_Initfha\n");
    if (!Fwl_FhaInit())
    {
        return AK_FALSE;
    }
    Fwl_Print(C3, M_FS, "Fwl_InitFs Nand_MountInit\n");

    Fwl_MountNand();
    
    if (!CheckLibVersions()) //只是NAND版本需要
    {
        Fwl_Print(C1, M_FS, "Check Lib Version Fail\n");
        while(1);
    }
#endif

#ifdef SPIBOOT
    Fwl_SPIFlash_Init();
#endif
    //Mount Sd 卡，如果有SD Boot,则必须先Mount Sd Boot
    for (i=0; i<eSD_INTERFACE_COUNT; i++)
    {
        SdMount[0] = eSD_INTERFACE_SDMMC;
        SdMount[1] = eSD_INTERFACE_SDIO;
#ifdef SDIOBOOT
        SdMount[0] = eSD_INTERFACE_SDIO;
        SdMount[1] = eSD_INTERFACE_SDMMC;
#endif
        SdRet = Fwl_Sd_HwInit(SdMount[i]);
        if (SdRet)
        {
            SdRet = Fwl_Sd_Mount(SdMount[i]);
            if (!SdRet)
            {
                Fwl_Sd_HwDestory(SdMount[i]);
            }
        }
#if ((defined (SDIOBOOT)) || (defined (SDMMCBOOT)))
        if ((0 == i) && (!SdRet))
        {
            Fwl_Print(C1, M_FS, "Mount sd boot card is fail\n");
            while(1);
        }
#endif
    }

#if (defined (SDIOBOOT))
    if (!Fwl_Fha_SD_Init(eSD_INTERFACE_SDIO))
    {
        Fwl_Print(C1, M_FS, "fha sdio init fail");
        return AK_FALSE;
    }
#elif (defined (SDMMCBOOT))
    if (!Fwl_Fha_SD_Init(eSD_INTERFACE_SDMMC))
    {
        Fwl_Print(C1, M_FS, "fha sdMMC init fail\n");
        return AK_FALSE;
    }
#endif

#ifdef OS_WIN32 
    FS_Sd_SetEmulate('Y');
#endif


#ifdef FS_TEST_API
    Fwl_FsTest(DRI_D);
    Fwl_FsTest(DRI_B);
    Fwl_FsTest(DRI_A);
    Fwl_FsTest(DRI_D);
    //Fwl_MtdTest(2);
    //Fwl_MtdTest(3);
#endif

#ifdef SUPPORT_PANNIC_REBOOT
    if (SYS_PANNIC_RESET == GetSystemStartMode())
    {
        return AK_TRUE;
    }
#endif     // end of #ifdef SUPPORT_PANNIC_REBOOT

    gFsLock = AK_Create_Semaphore(1, AK_PRIORITY);
#ifdef ASYN_CLOSE_FILE_CNT
    if (!Fwl_AsynCloseInit(ASYN_CLOSE_FILE_CNT))
    {
        return AK_FALSE;
    }
#endif
    return AK_TRUE;
}


T_VOID Fwl_DeInitFs(T_VOID)
{
#ifdef OS_ANYKA
#ifdef ASYN_CLOSE_FILE_CNT
    Fwl_AsynCloseDeInit();
#endif
    Fwl_DestoryAsynWrite();
    FS_Destroy();
#ifdef NANDBOOT
    Nand_DestoryFs();
#endif
#endif
    AK_Delete_Semaphore(gFsLock);
    gFsLock = 0;
    return;
}
T_BOOL Fwl_ChkOpenBackFile(T_U8* path, T_U8* bak_path)
{
    T_hFILE fp_new=FS_INVALID_HANDLE;
    T_hFILE fp_bak=FS_INVALID_HANDLE;
    T_U32 file_len;
    T_U32 tmpLen= 0;
    T_U32 file_bak_len;
    T_U32 ret;
    T_pVOID dataBuf = AK_NULL;
    T_BOOL  RET = AK_FALSE;
    const T_U32   buf_size = 500*1024;//500K
    T_U32   file_tmp = 0;
    
    dataBuf = Fwl_Malloc(buf_size);
    if (AK_NULL == dataBuf)
    {
        Fwl_Print(C1, M_FS, "ERR:LHS:Fwl_ChkOpenBackFile: malloc fail\n");
        RET = AK_FALSE;
        goto end;
    }
    
    //Fwl_Print(C3, M_FS, "file path:%s,bak file path:%s\n",path,bak_path);
    
    fp_new = Fwl_FileOpenAsc(path ,_FMODE_READ ,_FMODE_READ);
    
    if( fp_new != FS_INVALID_HANDLE )
    {
        file_len = Fwl_GetFileLen(fp_new);

        //Fwl_Print(C3, M_FS, "file lens is %d\n",file_len);
        
        if(0 == file_len)
        {
            Fwl_Print(C2, M_FS, "get file length error\n");
            Fwl_FileClose(fp_new);
            goto openBak;
        }
        
        file_tmp = 0;
        while (0 != (tmpLen = Fwl_FileRead(fp_new, dataBuf, buf_size)))
        {
            file_tmp += tmpLen;
            if (tmpLen != buf_size)
            {
                break;
            }
        }
        
        //Fwl_Print(C3, M_FS, "\nread len:%d,file len:%d\n",file_tmp,file_len);
        Fwl_FileClose(fp_new); 
        if (file_tmp != file_len)
        {
            Fwl_Print(C2, M_FS, "\nfile read error\n");
            goto openBak;
        }
        else
        {
            RET = AK_TRUE;
            goto end;
        }
    }
openBak:
    Fwl_Print(C3, M_FS, "file %s open failed, try to open back file %s\n", path, bak_path);

    fp_bak = Fwl_FileOpenAsc(bak_path ,_FMODE_READ ,_FMODE_READ);

    if( fp_bak == FS_INVALID_HANDLE )
    {
        Fwl_Print(C2, M_FS, "ERR:LHS:file open backup failed\n");
        RET = AK_FALSE;
        goto end;
    }
    else
    {
        Fwl_Print(C3, M_FS, "open bakeup success, recover it now\n");
        
        file_bak_len = Fwl_GetFileLen(fp_bak);

        if (0 == file_bak_len)
        {
            Fwl_Print(C2, M_FS, "\nERR:LHS:get bakfile length error\n");
            Fwl_FileClose(fp_bak);
            RET = AK_FALSE;
            goto end;
        }
        
        Fwl_FileDeleteAsc(path);
        fp_new = Fwl_FileOpenAsc(path, _FMODE_CREATE, _FMODE_CREATE);
        if (fp_new == FS_INVALID_HANDLE)
        {
            Fwl_Print(C2, M_FS, "\nERR:LHS:file create fail\n");
            Fwl_FileClose(fp_bak);
            RET = AK_FALSE;
            goto end;
        }
    
        Fwl_Print(C3, M_FS, "file_bak_len:%d\n",file_bak_len);

        Fwl_FileSeek(fp_bak, 0, FS_SEEK_SET);
        Fwl_FileSeek(fp_new, 0, FS_SEEK_SET);
        file_tmp = 0;
        while (0 != (tmpLen = Fwl_FileRead(fp_bak, dataBuf, buf_size)))
        {
             ret = Fwl_FileWrite(fp_new, dataBuf, tmpLen);
             file_tmp += ret;
        }

        Fwl_FileClose(fp_bak);
        Fwl_FileClose(fp_new);
        
        if(file_tmp != file_bak_len)
        {
            Fwl_FileDeleteAsc(path);
            Fwl_Print(C2, M_FS, "\nERR:LHS:recover file write fail read:%d,write:%d\n", file_bak_len,file_tmp);
            RET = AK_FALSE;
        }
        else
        { 
            Fwl_Print(C3, M_FS, "recover file success!\n");
            RET = AK_TRUE;
        }
    }    
end:
    //Fwl_Print(C3, M_FS, "end open back file\n");
    dataBuf = Fwl_Free(dataBuf);
    return RET;
}

#ifdef ASYN_CLOSE_FILE_CNT
T_BOOL Fwl_AsynCloseInit(T_U16 maxCount)
{
    T_S32 i = 0;
    
    if (maxCount == 0)
    {
        ASYN_CLOSE_ERR("Param Er");
        return AK_FALSE;
    }
    gCloseFileQue.queCnt = maxCount;

    gCloseFileQue.hdlQue = (T_OPT_NODE*)Fwl_Malloc(gCloseFileQue.queCnt*sizeof(T_OPT_NODE));
    if (AK_NULL == gCloseFileQue.hdlQue)
    {
        ASYN_CLOSE_ERR("Malloc Er");
        return AK_FALSE;
    }
    
    for (i=0;i<(T_S32)CLOSE_QUE_MAX;i++)
    {
        gCloseFileQue.hdlQue[i].fp      = FS_INVALID_HANDLE;
        gCloseFileQue.hdlQue[i].optMode = eASYN_OPT_NULL;
    }
    
    gCloseFileQue.head       = 0;
    gCloseFileQue.tail       = 0;
    gCloseFileQue.validLen = 0;
    gCloseFileQue.aysnMonitor   = INVALID_CLOSE_MAN_HDL;
    gCloseFileQue.monitorStatus = eASYN_STATUS_RUN; 
    Fwl_Print(C3,M_FS,"Asyn Close Init Ok_cnt=%d\n",gCloseFileQue.queCnt);
    return AK_TRUE;
}

T_VOID Fwl_AsynCloseDeInit(T_VOID)
{
    Fwl_AsynCloseFlush_EnableMonitor(AK_FALSE);
    Fwl_AsynCloseFlushAll();
    if (AK_NULL != gCloseFileQue.hdlQue)
    {
        Fwl_Free(gCloseFileQue.hdlQue);
        gCloseFileQue.hdlQue = AK_NULL;
    }
    memset(&gCloseFileQue,sizeof(gCloseFileQue),0);
}

T_S32 Fwl_AsynCloseFlush(T_VOID)
{
    T_pFILE fileHdl = FS_INVALID_HANDLE;
    T_ASYN_OPT_MODE optmode = 0;

    Fwl_LockFs();
    if ((AK_NULL != gCloseFileQue.hdlQue) && (gCloseFileQue.validLen > 0))
    {
        fileHdl = gCloseFileQue.hdlQue[gCloseFileQue.head].fp;
        optmode = gCloseFileQue.hdlQue[gCloseFileQue.head].optMode;
        if (FS_INVALID_HANDLE != fileHdl)
        {
            Fwl_Print(C3,M_FS,"AsynFile Flush(%d)_0x%x,cnt=%d\n",optmode,fileHdl,gCloseFileQue.validLen);
            gCloseFileQue.hdlQue[gCloseFileQue.head].fp      = FS_INVALID_HANDLE;
            gCloseFileQue.hdlQue[gCloseFileQue.head].optMode = eASYN_OPT_NULL;        
            //=============================
            if (eASYN_OPT_DELETE == optmode) //删除操作
            {
                Fwl_FileDestroy(fileHdl);
            }
            
            Fwl_FileClose(fileHdl);
            //=============================
        }

        gCloseFileQue.head = (T_U16)((gCloseFileQue.head + 1)% CLOSE_QUE_MAX);
        gCloseFileQue.validLen--;
    }
    Fwl_UnLockFs();
    return gCloseFileQue.validLen;
}

T_VOID Fwl_AsynCloseFlushAll(T_VOID)
{
    Fwl_LockFs();
    Fwl_AsynCloseFlush_EnableMonitor(AK_FALSE);
    Fwl_UnLockFs();
    while (Fwl_AsynCloseFlush()>0);
}


static T_VOID Fwl_AsynCloseFlush_EnableMonitor(T_BOOL isEnable)
{
    if (INVALID_CLOSE_MAN_HDL != gCloseFileQue.aysnMonitor)
    {
#if (1 == ASYN_MONITOR_MODE)
        FsApi_KillThead(gCloseFileQue.aysnMonitor);
#else
        vtimer_stop(gCloseFileQue.aysnMonitor);
#endif
        gCloseFileQue.aysnMonitor = INVALID_CLOSE_MAN_HDL;
    }

    if (isEnable)
    {
#if (1 == ASYN_MONITOR_MODE)
        gCloseFileQue.aysnMonitor = FsApi_MountThread(Fwl_AsynCloseFlushMonitor,AK_NULL,CLOSE_MAN_INTREVAL);
#else
        gCloseFileQue.aysnMonitor = vtimer_start(CLOSE_MAN_INTREVAL, AK_FALSE, Fwl_AsynCloseFlushMonitor);
#endif
    }
}

#if (1 == ASYN_MONITOR_MODE)
static T_VOID Fwl_AsynCloseFlushMonitor(T_U32 argc, T_pVOID argv)
{
    while (1)
    {
        if (eASYN_STATUS_PAUSE != gCloseFileQue.monitorStatus)
        {
            if (Fwl_AsynCloseFlush() <= 0)
            {
                break;
            }
        }
        AK_Sleep(10);
    }
    Fwl_AsynCloseFlush_EnableMonitor(AK_FALSE);
}
#else
static T_VOID Fwl_AsynCloseFlushMonitor(T_TIMER timer_id, T_U32 delay)
{
    Fwl_AsynCloseFlush_EnableMonitor(AK_FALSE);
    if (Fwl_AsynCloseFlush() > 0)
    {
        Fwl_AsynCloseFlush_EnableMonitor(AK_TRUE);
    }
}
#endif

static T_S32 Fwl_AsynFileOpt(T_pFILE hdl, T_U8 mode)
{
    T_pFILE fileHdl = (T_pFILE)hdl;
    
    if (gCloseFileQue.validLen >= CLOSE_QUE_MAX)
    {
        Fwl_AsynCloseFlush();
    }
    Fwl_LockFs();
    if ((AK_NULL != gCloseFileQue.hdlQue) && (gCloseFileQue.validLen < CLOSE_QUE_MAX) && (FS_INVALID_HANDLE != fileHdl))
    {
        gCloseFileQue.hdlQue[gCloseFileQue.tail].fp      = fileHdl;
        gCloseFileQue.hdlQue[gCloseFileQue.tail].optMode = mode;
        gCloseFileQue.tail = (T_U16)((gCloseFileQue.tail + 1) % CLOSE_QUE_MAX);
        gCloseFileQue.validLen++;
        Fwl_Print(C3,M_FS,"AsynFile Close(mode=%d) fp_0x%x,cnt=%d\n",mode,fileHdl,gCloseFileQue.validLen);
    }
    if ((INVALID_CLOSE_MAN_HDL == gCloseFileQue.aysnMonitor) && (gCloseFileQue.validLen > 0))
    {
        Fwl_AsynCloseFlush_EnableMonitor(AK_TRUE);
    }
    Fwl_UnLockFs();
    return gCloseFileQue.validLen;
}

T_S32 Fwl_AsynCloseFile(T_pFILE hdl)
{
    return Fwl_AsynFileOpt(hdl, eASYN_OPT_CLOSE);
}

T_S32 Fwl_AsynDeleteFile(T_pFILE hdl)
{
    return Fwl_AsynFileOpt(hdl, eASYN_OPT_DELETE);
}

T_VOID Fwl_AsynClosePause(T_VOID)
{
    Fwl_LockFs();
    gCloseFileQue.monitorStatus = eASYN_STATUS_PAUSE;
    Fwl_UnLockFs();
    Fwl_AsynCloseFlush_EnableMonitor(AK_FALSE);
}

T_VOID Fwl_AsynCloseResume(T_VOID)
{
    Fwl_LockFs();
    gCloseFileQue.monitorStatus = eASYN_STATUS_RUN;
    Fwl_UnLockFs();
    Fwl_AsynCloseFlush_EnableMonitor(AK_TRUE);
}


#endif

#ifdef OS_WIN32 //模拟器没有用到异步写，为了编译通过，暂时这样做
void Asyn_FlushFat(T_PFILE file)
{
    return;
}
void Asyn_InsertFileFat(T_PFILE file)
{
    return;
}
#endif


#ifdef FS_TEST_API
#define FS_TEST_TAG ""//[Fs_Test]

#define _OPT_START_ADDR (0)
#define _OPT_SIZE_ (1<<10)
#define _OPT_BUF_   (64)
#define _OPT_BLK_  (_OPT_SIZE_/_OPT_BUF_)

#define _SPEED_START_ADDR (128<<20)
#define _SPEED_SIZE_ (20<<20)
#define _SPEED_SEC_CNT_   (200)

#define _SPEED_FILE_BUF   (400<<10)
#define _SPEED_FILE_BUF_CON (50)

static T_VOID Fwl_MtdSpeedTest(T_U8 drvId)
{
    T_DRIVER_INFO DriverInfo;
    T_U32 addr = 0, secSz = 0,bufSz = 0;
    T_PMEDIUM medium = AK_NULL;
    T_U32 retCnt = 0;
    T_U8 blkcnt = 1;
    T_U8 *buf,*buf2;
    T_U32  systm00,systm11;
    
    if (!FS_GetDriver(&DriverInfo,drvId))
    {
        Fwl_Print(C3, M_FS, "FS_GetDriver[%d] $$$$$$$$$$$$ Error\n",drvId);
        return;
    }
    else
    {
        Fwl_Print(C3, M_FS, "FS_GetDriver[%d] $$$$$$$$$$$$ Ok\n",drvId);
    }

    medium = DriverInfo.medium;
    Fwl_Print(C3, M_FS, "medium $$$$$$$$$$$$  0x%x\n",medium);
    if (AK_NULL == medium)
    {
        return;
    }
    
    Fwl_Print(C3, M_FS, "medium read $$$$$$$$$$$$  0x%x\n", medium->read);
    if (AK_NULL == medium->read)
    {
        return;
    }
    
    Fwl_Print(C3, M_FS, "medium write$$$$$$$$$$$$  0x%x\n",medium->write);
    if (AK_NULL == medium->write)
    {
        return;
    }
    
    //==============================================
    secSz = (1 << medium->SecBit);
    bufSz = secSz* _SPEED_SEC_CNT_;
    buf = (T_U8 *)Fwl_Malloc(bufSz);
    buf2 = (T_U8 *)Fwl_Malloc(bufSz);
    memset(buf, 0x55, bufSz);
    
    Fwl_Print(C3, M_FS, "#W\n");
    systm00 = get_tick_count();
    
    Fwl_Print(C3, M_FS, "[Nandwrite_%d_drv]startTime:[%d] bufSz:%d ,Allsize:%d\n",drvId,\
        systm00,bufSz,_SPEED_SIZE_);
    
    for (addr =  _SPEED_START_ADDR/secSz;\
         addr < (( _SPEED_START_ADDR +  _SPEED_SIZE_)/secSz);\
         addr+= _SPEED_SEC_CNT_)
    {
        medium->write(medium, buf, addr, _SPEED_SEC_CNT_);
#if 0        
        if (_OPT_SEC_CNT_ != tes)
            Fwl_Print(C3, M_FS, "ERR:%d,%d,0x%x\n",tes,_OPT_SEC_CNT_,addr);
#endif        
    }
    systm11 = get_tick_count();
    Fwl_Print(C3, M_FS, "[Nandwrite_%d_drv]endTime:[%d] bufSz:%d ,Allsize:%d\n",drvId,\
        systm11,bufSz,_SPEED_SIZE_);
    Fwl_Print(C3, M_FS, "Mtd write speed :%d K\n",(_SPEED_SIZE_>>10)/((systm11-systm00)/1000));

    //==============================================
    //retCnt = medium->read(medium, buf, addr, sizeof(buf));
    
    Fwl_Print(C3, M_FS, "#R\n");
    systm00 = get_tick_count();
    for (addr =  _SPEED_START_ADDR/secSz;\
         addr < (( _SPEED_START_ADDR +  _SPEED_SIZE_)/secSz);\
         addr+=_SPEED_SEC_CNT_)
    {        
        memset(buf2, 0, bufSz);
        
        medium->read(medium, buf2, addr,  _SPEED_SEC_CNT_);
        
        if(0!=memcmp(buf2,buf,bufSz))
        {
            Fwl_Print(C3, M_FS, "ERR:addr:0x%x,buf[0]:0x%x,buf2[0]:0x%x,\n",addr,buf[0],buf2[0]);
        }
    }
    systm11 = get_tick_count();
    Fwl_Print(C3, M_FS, "[Nandread_%d_drv]S:[%d] <%d> Bytes \n",drvId,\
    systm11, _SPEED_SIZE_);    
    Fwl_Print(C3, M_FS, "Mtd read speed :%d K\n",(_SPEED_SIZE_>>10)/((systm11-systm00)/1000));

    while(1);
    //==============================================

}

static T_BOOL Fwl_FileSpeedTest(T_U8* path)
{
    T_hFILE fp_new=FS_INVALID_HANDLE;
    T_U32 tmpLen=0;
    T_U32 file_tmp=0;
    T_U8 *dataBuf=AK_NULL;
    T_U32 ret;
    T_U32 fileSize;
    T_U32 Time,Time2;

    fileSize = _SPEED_FILE_BUF*_SPEED_FILE_BUF_CON;
    
    dataBuf = (T_U8 *)Fwl_Malloc(_SPEED_FILE_BUF);
    if (AK_NULL==dataBuf)
    {
        Fwl_Print(C3, M_FS, "buf malloc is null\n");
        while(1);
    }
    memset(dataBuf,0x55,_SPEED_FILE_BUF);
    
    fp_new = Fwl_FileOpenAsc(path, _FMODE_CREATE, _FMODE_CREATE);
    
    if (fp_new == FS_INVALID_HANDLE)
    {
        Fwl_Print(C3, M_FS, "\nERR:LHS:file create fail\n");
        while(1);
    }

    Fwl_FileSeek(fp_new, 0, FS_SEEK_SET);
    
    Fwl_Print(C3, M_FS, "file test:fileSize:%d,bufSize:%d\n",fileSize,_SPEED_FILE_BUF);
    
    Time = get_tick_count();
    file_tmp = 0;
    while (fileSize-file_tmp)
    {
         ret = Fwl_FileWrite(fp_new, dataBuf, _SPEED_FILE_BUF);
         file_tmp += ret;
    }
    Time2=get_tick_count();
    
    Fwl_Print(C3, M_FS, "file test:fileSize:%d,bufSize:%d,AllTime:%d\n",fileSize,_SPEED_FILE_BUF,Time2-Time);
    Fwl_Print(C3, M_FS, "file write speed :%d K\n",(fileSize>>10)/((Time2-Time)/1000));
        
    Fwl_FileClose(fp_new);
    
    if(file_tmp != fileSize)
    {
        Fwl_Print(C3, M_FS, "\nERR:LHS:recover file write fail read:%d,write:%d\n", file_tmp,file_tmp);
    }
    else
    { 
        Fwl_Print(C3, M_FS, "write file success!\n");
    }

    Fwl_Print(C3, M_FS, "file read speed test\n");
    
    fp_new = Fwl_FileOpenAsc(path, _FMODE_READ, _FMODE_READ);
    
    if (fp_new == FS_INVALID_HANDLE)
    {
        Fwl_Print(C3, M_FS, "\nERR:LHS:file create fail\n");
        while(1);
    }
    
    fileSize = Fwl_GetFileLen(fp_new);
    
    Fwl_FileSeek(fp_new, 0, FS_SEEK_SET);
    
    Time = get_tick_count();
    Fwl_Print(C3, M_FS, "file test:fileSize:%d,bufSize:%d\n",fileSize,_SPEED_FILE_BUF);
    
    file_tmp = 0;
    while (ret = Fwl_FileRead(fp_new,dataBuf,_SPEED_FILE_BUF))
    {
         file_tmp += ret;
    }
    Time2=get_tick_count();
    
    Fwl_Print(C3, M_FS, "file test:fileSize:%d,bufSize:%d,AllTime:%d\n",fileSize,_SPEED_FILE_BUF,Time2-Time);
    Fwl_Print(C3, M_FS, "file write speed :%d K\n",(fileSize>>10)/((Time2-Time)/1000));
        
    Fwl_FileClose(fp_new);
    
    if(file_tmp != fileSize)
    {
        Fwl_Print(C3, M_FS, "\nERR:LHS:recover file write fail read:%d,write:%d\n", file_tmp,file_tmp);
    }
    else
    { 
        Fwl_Print(C3, M_FS, "read file success!\n");
    }
    while(1);
}

static T_VOID FsApi_Dump(T_U8* tips, T_U8*data,T_U32 Len)
{
#define FS_TEST_DUMP_COL 32
    T_U32 i= 0;
    Fwl_Print(C3, M_FS, "=============== %s =================\n",tips);
    if (AK_NULL != data)
    {
        for (i=0;i<Len;i++)
        {
            Fwl_Print(C3, M_FS, " %02X",data[i]);
            if ((i+1)%FS_TEST_DUMP_COL == 0)
            {
            Fwl_Print(C3, M_FS, "\n");
            }
        }
    }
    Fwl_Print(C3, M_FS, "\n=============================================\n");
}

static T_VOID Fwl_MtdTest(T_U8 drvId)
{
    T_DRIVER_INFO DriverInfo;
    T_U32 i = 0,addr = 0;
    T_PMEDIUM medium = AK_NULL;
    T_U32 retCnt = 0;
    T_U8 buf [_OPT_BUF_],blkcnt = 1;
    T_U32  systm00,systm11;
    
    if (!FS_GetDriver(&DriverInfo,drvId))
    {
        Fwl_Print(C3, M_FS, "FS_GetDriver[%d] $$$$$$$$$$$$ Error\n",drvId);
        return;
    }
    else
    {
        Fwl_Print(C3, M_FS, "FS_GetDriver[%d] $$$$$$$$$$$$ Ok\n",drvId);
    }

    medium = DriverInfo.medium;
    Fwl_Print(C3, M_FS, "medium $$$$$$$$$$$$  0x%x\n",medium);
    if (AK_NULL == medium)
    {
        return;
    }
    
    Fwl_Print(C3, M_FS, "medium read $$$$$$$$$$$$  0x%x\n", medium->read);
    if (AK_NULL == medium->read)
    {
        return;
    }
    
    Fwl_Print(C3, M_FS, "medium write$$$$$$$$$$$$  0x%x\n",medium->write);
    if (AK_NULL == medium->write)
    {
        return;
    }
    
    //==============================================
    memset(buf, 0x12, sizeof(buf));
    systm00 = Fwl_RTCGetCount();
    Fwl_Print(C3, M_FS, "#W\n");
    systm00 = Fwl_RTCGetCount();
    
    Fwl_Print(C3, M_FS, "[Nandwrite_%d_drv(%d)]S:[%d] <%d> Bytes \n",i,drvId,\
        systm00,_OPT_SIZE_);
    
    for (addr = _OPT_START_ADDR;\
         addr < (_OPT_START_ADDR + _OPT_SIZE_);\
         addr = addr + sizeof(buf))
    {
        medium->write(medium, buf, addr, sizeof(buf));
    }
    systm11 = Fwl_RTCGetCount();
    Fwl_Print(C3, M_FS, "[Nandwrite_%d_drv(%d)]S:[%d] <%d> Bytes \n",i,drvId,\
        systm11,_OPT_SIZE_);
    //==============================================
    //retCnt = medium->read(medium, buf, addr, sizeof(buf));
    Fwl_Print(C3, M_FS, "#R\n");
    systm00 = Fwl_RTCGetCount();
    for (addr = _OPT_START_ADDR;\
         addr < (_OPT_START_ADDR + _OPT_SIZE_);\
         addr = addr + sizeof(buf))
    {
        medium->read(medium, buf, addr, sizeof(buf));
    }
    systm11 = Fwl_RTCGetCount();
    Fwl_Print(C3, M_FS, "[Nandread_%d_drv(%d)]S:[%d] <%d> Bytes \n",i,drvId,\
    systm11,_OPT_SIZE_);
    //==============================================

}

#define TST_DRV DRI_B
#define TST_TIMES (3)
static T_VOID Fwl_FsTest(T_CHR *rootdir)
{
    T_pFILE     fp_c,fp_r;
    T_STR_FILE  file = {0};
    T_U8 dataBuf[_OPT_BUF_];
    T_U32 i = 0, len = 0,addr = 0;
    T_U32  systm00,systm11;
    T_CHR *pFileName = AK_NULL;
    
    if (AK_NULL == rootdir)
    {
        rootdir = TST_DRV;
    }
    
    Fwl_Print(C3, M_FS, "\n"FS_TEST_TAG"%s is %s Exist \n",rootdir,Fwl_FileExistAsc(rootdir)?"":"NOT");
    #if 1
    //==================Read======================= 
    Fwl_Print(C3, M_FS, "\n-------------------Read Test-<%s>-----------------\n",rootdir);
    for (i=0;i< TST_TIMES;i++)
    {
        switch(i)
        {
            case 0:pFileName = "UTF-8.txt";break;
            case 1:pFileName = "UTF-16.txt";break;
            case 2:pFileName = "ANSI.txt";break;
            default:pFileName = "UTF-8.txt";break;
        }
        
        sprintf(file, "%s%s",rootdir,pFileName);
        Fwl_Print(C3, M_FS, FS_TEST_TAG"Open[%d] File %s begin.. \n",i,file);
        
        fp_r= Fwl_FileOpenAsc(file, _FMODE_READ,_FMODE_READ);
        if (FS_INVALID_HANDLE == fp_r)
        {
            Fwl_Print(C3, M_FS, FS_TEST_TAG"Open %s Fail\n",file);
            continue;
        }
        else
        {
            Fwl_Print(C3, M_FS, FS_TEST_TAG"Open File %s Ok.. \n",file);
            len = Fwl_FileRead(fp_r, dataBuf, _OPT_BUF_);
            
            Fwl_Print(C3, M_FS, FS_TEST_TAG"Read Len = %d [%d]\n",len ,_OPT_BUF_);
            if (len > 0)
            {
                FsApi_Dump(FS_TEST_TAG"ReadTest",dataBuf,len);
            }
            Fwl_FileClose(fp_r);
        }
    }
    #endif
    #if 1
    //=================Create======================= 
    Fwl_Print(C3, M_FS, "\n-------------------Create Test-<%s>-----------------\n",rootdir);
    for (i=0;i<sizeof(dataBuf);i++)
    {
        dataBuf[i] = i;
    }
    for (i=0;i< TST_TIMES;i++)
    {
        sprintf(file, "%stestFile%03d.txt",rootdir,i);

        Fwl_Print(C3, M_FS, FS_TEST_TAG"Create File %s begin.. \n",file);
        fp_c = Fwl_FileOpenAsc(file,_FMODE_CREATE, _FMODE_CREATE);
        
        if (FS_INVALID_HANDLE == fp_c)
        {
            Fwl_Print(C3, M_FS, FS_TEST_TAG"Create  Fail\n");
            continue;
        }
        else
        {
            Fwl_Print(C3, M_FS, FS_TEST_TAG"Create Ok\n");
        }
        
        Fwl_FileSeek(fp_c, 0, _FSEEK_CUR);
        
        Fwl_Print(C3, M_FS, FS_TEST_TAG"Write<%d> Begin..\n",i);
        systm00 = Fwl_RTCGetCount();
        for (addr = _OPT_START_ADDR;\
             addr < (_OPT_START_ADDR + _OPT_SIZE_);\
             addr = addr + _OPT_BUF_)
        {
            Fwl_FileWrite(fp_c, dataBuf, _OPT_BUF_);
        }
        systm11 = Fwl_RTCGetCount();
        Fwl_Print(C3, M_FS, FS_TEST_TAG"%s is %s Exist \n",file,File_Exist(fp_c)?"":"NOT");
        Fwl_Print(C3, M_FS, FS_TEST_TAG"Write<%d> S:[%d] <%d> Bytes \n",i,(systm11-systm00),Fwl_GetFileLen(fp_c));
        
        Fwl_FileClose(fp_c);

        Fwl_Print(C3, M_FS, FS_TEST_TAG"Close %s is %s Exist \n",file,Fwl_FileExistAsc(file)?"":"NOT");
        //Fwl_FileDeleteAsc(file);
    }
    #endif
    #if 1
    //=================MkDir======================= 
    Fwl_Print(C3, M_FS, "\n-------------------MkDir Test-<%s>-----------------\n",rootdir);
    for (i=0; i< TST_TIMES; i++)
    {
        switch(i)
        {
            case 0: pFileName = "aaabbb/";break;
            case 1: pFileName = "aaabbb/ccccdd";break;
            case 2: pFileName = "ffffff/ccccdd";break;
            default:pFileName = "eeeebbb/cccddd/";break;
        }
        sprintf(file, "%s%s",rootdir,pFileName);
        Fwl_Print(C3, M_FS, FS_TEST_TAG"Mk Tree %s is %s Exist begin..\n",file,Fwl_FileExistAsc(file)?"":"not");
        Fwl_Print(C3, M_FS, FS_TEST_TAG"Mk Tree %s %s\n",file,Fwl_FsMkDirTreeAsc(file)?"Ok":"Failed");
        Fwl_Print(C3, M_FS, FS_TEST_TAG"Mk Tree %s is %s Exist end\n",file,Fwl_FileExistAsc(file)?"":"NOT");
    }
    pFileName = "tttbbb";
    sprintf(file, "%s%s",rootdir,pFileName);
    Fwl_Print(C3, M_FS, FS_TEST_TAG"Mk Dir %s is %s Exist begin..\n",file,Fwl_FileExistAsc(file)?"":"not");
    Fwl_Print(C3, M_FS, FS_TEST_TAG"Mk Dir %s %s\n",file,Fwl_FsMkDirAsc(file)?"Ok":"Failed");
    Fwl_Print(C3, M_FS, FS_TEST_TAG"Mk Dir %s is %s Exist end\n",file,Fwl_FileExistAsc(file)?"":"NOT");
    #endif
}

#endif  // end  #if  FS_TEST_API

/* end of file */

