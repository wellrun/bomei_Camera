/**
 * @file Fwl_osFS.h
 * @brief This header file is for OS related function prototype
 *
 */
#ifndef __FWL_OSFS_H__
#define __FWL_OSFS_H__

#include "akdefine.h"

//================================================

#define     FS_MAX_PATH_LEN     259 // 260 // unicode len

#define     FS_MAX_DISK_CNT     (26) /*文件系统最多可挂载的盘个数*/

#ifndef T_hFILE
#define     T_hFILE             T_S32
#endif

#ifndef T_hFILESTAT
#define     T_hFILESTAT         T_S32
#endif

#ifndef T_FILE_MODE
#define     T_FILE_MODE         T_U32
#endif

#ifndef T_FILE_FLAG
#define     T_FILE_FLAG         T_U32
#endif

#ifndef FS_INVALID_HANDLE
#define     FS_INVALID_HANDLE -1
#endif

#ifndef FS_INVALID_STATHANDLE
#define     FS_INVALID_STATHANDLE -1
#endif

//================================================
/** FileOpen flag reference
 * @NOTE
 * This flag only works while opening a file
 */
#define FS_PO_RDWR			0x0000 /**< Read/write access allowed. */
#define FS_PO_RDONLY		0x0001 /**< Open for read only. */
#define FS_PO_WRONLY		0x0002 /**< Open for write only. */
#define FS_PO_APPEND		0x0004 /**< Filepointer will be set to end of file on opening the file. */
#define FS_PO_OVERLAY		0x0008 /**< Filepointer will be set to start of file on opening the file. */
#define FS_PO_CREAT			0x0010 /**< Create the file if it does not exist. */
#define FS_PO_TRUNC			0x0020 /**< Truncate the file if it already exists. */
#define FS_PO_EXCL			0x0040 /**< Attempt to create will fail if  the given file already exists.  Used in conjunction with VPO_CREAT*/

/** FileOpen mode reference
 * @NOTE
 * This is the file mode attribute, it is used when creating a file
 */
#define FS_ANORMAL			0x00000000	//normal file
#define FS_AHIDDEN			0x00010000	//hidden file
#define FS_ASYSTEM			0x00020000	//system file
#define FS_UNDELETE			0x00040000	//undelete file

#define FS_S_IWRITE			0x00100000 /**< Write permitted  */
#define FS_S_IREAD			0x00200000 /**< Read permitted. (Always true anyway)*/
#define FS_S_IFMT			0x17000000 /**< type of file mask */
#define FS_S_IFDIR			0x04000000 /**< directory */
#define FS_S_IFREG			0x10000000 /**< regular */


/** 
 *access file in create mode.if not exsit, create it. if exsit, its content will be destoryed 
*/
#ifndef FS_MODE_CREATE
#define FS_MODE_CREATE		(FS_PO_RDWR|FS_PO_CREAT|FS_S_IWRITE|FS_S_IREAD)
#endif

/**
* access file in read-only mode.if the file isn't exsit yet, an error will be returned. 
*/
#ifndef FS_MODE_READ
#define FS_MODE_READ		(FS_PO_RDONLY|FS_S_IWRITE|FS_S_IREAD)
#endif

/**
* access file in read-write mode.you are permitted to read from and write to file in this mode.
*/
#ifndef FS_MODE_WRITE
#define FS_MODE_WRITE		(FS_PO_RDWR|FS_S_IWRITE|FS_S_IREAD)
#endif
/** @} */

#ifndef FS_MODE_OVERLAY
#define FS_MODE_OVERLAY		(FS_PO_OVERLAY|FS_S_IWRITE|FS_S_IREAD)
#endif

#ifndef FS_MODE_APPEND
#define FS_MODE_APPEND		(FS_PO_APPEND|FS_S_IWRITE|FS_S_IREAD)
#endif

 /*
 * @FS_FileSeek reference
 */
#ifndef FS_SEEK_SET
#define FS_SEEK_SET         0
#endif

#ifndef FS_SEEK_CUR
#define FS_SEEK_CUR         1
#endif

#ifndef FS_SEEK_END
#define FS_SEEK_END         2
#endif


//================================================
#define T_pFILE         T_hFILE
#define T_PFILE         T_U32 

#define _FOPEN_FAIL     FS_INVALID_HANDLE

#define _FMODE_READ     FS_MODE_READ
#define _FMODE_WRITE    FS_MODE_APPEND
#define _FMODE_CREATE   FS_MODE_CREATE
#define _FMODE_OVERLAY  FS_MODE_OVERLAY

#define _FSEEK_CUR      FS_SEEK_CUR
#define _FSEEK_END      FS_SEEK_END
#define _FSEEK_SET      FS_SEEK_SET

#define EXPLORER_ISFOLDER 0x10

#define ASYN_CLOSE_FILE_CNT   (10)

#define FILE_COPY_THRD_STACK_SIZE   (256*1024)//深层目录必须这么大

#ifdef ASYN_CLOSE_FILE_CNT
typedef enum {
    eASYN_OPT_NULL = 0,
	eASYN_OPT_CLOSE,
	eASYN_OPT_DELETE,
} T_ASYN_OPT_MODE;
#endif

typedef enum {
	eFS_COPY_ING = 0,
	eFS_COPY_Success,
	eFS_COPY_Fail,
	eFS_COPY_OtherState,
}e_FILE_COPY_STATE;

typedef struct  {
    T_U32       time_create;    /* -1 for FAT file systems */
    T_U32       time_access;    /* -1 for FAT file systems */
    T_U32       time_write;
    T_U8        attrib;
    T_U64_INT       size;
    T_U16       name[FS_MAX_PATH_LEN+1];
}T_FILE_INFO;

typedef struct
{
	T_hTask thread;
	T_U8 Stack[FILE_COPY_THRD_STACK_SIZE];
} T_COPY_CTRL,*T_PCOPY_CTRL;

typedef T_BOOL (*F_CopyCallback)(T_VOID *pData, T_U16 *FileName,T_U32 CurPos, T_U32 FileSize);
typedef e_FILE_COPY_STATE (*F_CopySetState)(e_FILE_COPY_STATE state);

typedef T_BOOL (*F_DelCallback)(T_VOID *pData, T_U16 *FileName);
typedef e_FILE_COPY_STATE (*F_DelSetState)(e_FILE_COPY_STATE state);

typedef struct 
{
	F_CopyCallback    Callback;
	F_CopySetState	  SetState;
}T_FS_COPY_CALLBACK, *T_PFS_COPY_CALLBACK;

typedef struct 
{
	F_DelCallback    Callback;
	F_DelSetState	  SetState;
}T_FS_DEL_CALLBACK, *T_PFS_DEL_CALLBACK;

T_BOOL   Fwl_InitFs(T_VOID);
T_VOID   Fwl_DeInitFs(T_VOID);
T_U8     Fwl_ChkDsk(T_VOID);

T_pCSTR Fwl_FsGetLibVersion(T_VOID);

//path just for file
T_pFILE Fwl_FileOpen(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE mode);
T_pFILE Fwl_FileOpenAsc(T_pSTR path, T_FILE_FLAG flag, T_FILE_MODE mode);

//path can be a dir
T_pFILE Fwl_FileOpen_Ex(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE mode);
T_pFILE Fwl_FileOpenAsc_Ex(T_pSTR path, T_FILE_FLAG flag, T_FILE_MODE mode);

T_BOOL  Fwl_InitAsyn(T_U32 BufSize, T_pCWSTR path);
T_pFILE Fwl_FileOpenAsyn(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE mode);
T_BOOL  Fwl_DeInitAsyn(T_pCWSTR path);

T_BOOL  Fwl_FileClose(T_pFILE hFile);

T_BOOL  Fwl_FileHandleExist(T_pFILE pFile);
T_BOOL  Fwl_FileExist(T_pCWSTR path);
T_BOOL  Fwl_FileExistAsc(T_pSTR path);


T_U32   Fwl_FileRead(T_pFILE hFile, T_pVOID buffer, T_U32 count);
T_U32   Fwl_FileWrite(T_pFILE hFile, T_pCVOID buffer, T_U32 count);


T_BOOL  Fwl_FileMove(T_pCWSTR oldFile,T_pCWSTR newFile);
T_BOOL  Fwl_FileMoveAsc(T_pSTR oldFile,T_pSTR newFile);

/**
 * @BRIEF Folder Move.this function will create task for Folder Move.When Folder Move is  done,
 				must be to use the function Fwl_KillCopyThead to stop a task. 
 * @AUTHOR Lu Heshan
 * @PARAM [in]T_pCWSTR oldFile   sour file path
 * @PARAM [in]T_pCWSTR newFile   des file path
 * @PARAM [in]T_FS_MOVE_CALLBACK CallBackFun   call back function
 * @PARAM [out]T_COPY_CTRL **thrdCtrl   handle of theFolder Move task. 
 * @RETURN T_BOOL 
 * @RETVAL if return AK_FLASH ,create task fail.
 */
T_BOOL	Fwl_FolderMove(T_pCWSTR oldFile,T_pCWSTR newFile,T_FS_COPY_CALLBACK CallBackFun,T_COPY_CTRL **thrdCtrl);
/**
 * @BRIEF File Copy.this function will create task for copy file.When file copy is  done,
 				must be to use the function Fwl_KillCopyThead to stop a task. 
 * @AUTHOR Lu Heshan
 * @PARAM [in]T_pCWSTR sourFile   sour file path
 * @PARAM [in]T_pCWSTR destFile   des file path
 * @PARAM [in]T_BOOL replace        if it is ak_TRUE, we will replace the same file name
 * @PARAM [in]T_VOID *pCallBackData the date for call back.
 * @PARAM [in]T_FS_COPY_CALLBACK CallBackFun   call back function
 * @PARAM [out]T_COPY_CTRL **thrdCtrl   handle of the copy file task. 
 * @RETURN T_BOOL 
 * @RETVAL if return AK_FLASH ,create task fail.
 */
T_BOOL  Fwl_FileCopy(T_pCWSTR sourFile,T_pCWSTR destFile, T_BOOL replace, T_VOID *pCallBackData, T_FS_COPY_CALLBACK CallBackFun, T_COPY_CTRL **thrdCtrl);

/**
 * @BRIEF File Copy.this function will create task for copy file.When file copy is  done,
 				must be to use the function Fwl_KillCopyThead to stop a task. 
 * @AUTHOR Lu Heshan
 * @DADA    2011-08-11
 * @PARAM [in]T_pCSTR sourFile   sour file path
 * @PARAM [in]T_pCSTR destFile   des file path
 * @PARAM [in]T_BOOL replace        if it is ak_TRUE, we will replace the same file name
 * @PARAM [in]T_VOID *pCallBackData the date for call back.
 * @PARAM [in]T_FS_COPY_CALLBACK CallBackFun   call back function
 * @PARAM [out]T_COPY_CTRL **thrdCtrl   handle of the copy file task. 
 * @RETURN T_BOOL 
 * @RETVAL if return AK_FLASH ,create task fail.
 */
T_BOOL  Fwl_FileCopyAsc(T_pCSTR sourFile,T_pCSTR destFile, T_BOOL replace, T_VOID *pCallBackData, T_FS_COPY_CALLBACK CallBackFun, T_COPY_CTRL **thrdCtrl);
T_S32   Fwl_FileSeek(T_pFILE hFile, T_S32 offset, T_U16 origin);
T_U32   Fwl_FileSeekEx(T_pFILE hFile, T_U32 offset, T_U16 origin);
T_S32 	Fwl_FileTell(T_pFILE hFile);
    
T_U32   Fwl_GetFileLen(T_pFILE hFile);
T_U32   Fwl_FileGetSize(T_pCWSTR path);
T_U32   Fwl_FileGetSizeAsc(T_pSTR path);

T_BOOL  Fwl_FileDelete (T_pCWSTR path);
T_BOOL  Fwl_FileDeleteAsc(T_pSTR path);

T_VOID  Fwl_FileDestroy(T_pFILE hFile);
T_BOOL	Fwl_FileDeltree(T_pFILE hFile);

T_BOOL  Fwl_FileTruncate(T_pFILE hFile, T_U32 length);
T_VOID  Fwl_FileDataFlush(T_pFILE hFile);

T_BOOL  Fwl_IsFile(T_pFILE hFile);
T_BOOL  Fwl_SetFileSize(T_pFILE hFile, T_U32 size);
T_BOOL  Fwl_SetFileBufferSize(T_pFILE hFile, T_U32 size);

T_BOOL  Fwl_FsMkDir(T_pCWSTR path);
T_BOOL  Fwl_FsMkDirAsc(T_pSTR path);

T_BOOL  Fwl_FsMkDirTree(T_pCWSTR path);
T_BOOL  Fwl_FsMkDirTreeAsc(T_pSTR path);

T_BOOL  Fwl_FsRmDir(T_pCWSTR path);
T_BOOL  Fwl_FsRmDirAsc(T_pSTR path);
T_BOOL  Fwl_FsRmDirTree(T_pCWSTR path);
T_BOOL  Fwl_FsRmDirTreeAsc(T_pSTR path);

T_BOOL  Fwl_FsIsDir(T_pCWSTR path);
T_BOOL  Fwl_FsIsDirAsc(T_pSTR path);

T_BOOL  Fwl_FsIsFile(T_pCWSTR path);
T_BOOL  Fwl_FsIsFileAsc(T_pSTR path);

T_hFILESTAT Fwl_FsFindFirst(T_pCWSTR pattern);
T_BOOL      Fwl_FsFindNext(T_hFILESTAT stat);
T_U32       Fwl_FsFindInfo(T_FILE_INFO *fileInfo, T_hFILESTAT fileStat);
T_pCWSTR    Fwl_FsFindName(T_hFILESTAT stat);
T_VOID      Fwl_FsFindClose(T_hFILESTAT stat);
//Fwl_FileFindFirstFromHandld必须与Fwl_FileFindCloseWithHandle合用
T_U32 Fwl_FileFindFirstFromHandle(T_U32 file);
T_U32 Fwl_FileFindOpen(T_U32 parent, T_U32 FileInfo);
T_VOID Fwl_FileFindCloseWithHandle(T_U32 obj);

T_U16   Fwl_GetDiskList(T_CHR drvDesChar[], T_U8 pDiskType[], T_U16 DiskNum);
T_VOID  Fwl_FsGetSize(T_WCHR drvDesChar, T_U64_INT *size64);
T_VOID  Fwl_FsGetUsedSize(T_WCHR drvDesChar, T_U64_INT *size64);
T_VOID  Fwl_FsGetFreeSize(T_WCHR drvDesChar, T_U64_INT *size64);

T_BOOL Fwl_IsRootDir(T_pCWSTR pFilePath);
T_BOOL Fwl_GetRootDir(T_pCWSTR pFilePath, T_pWSTR pRootDir);
T_BOOL Fwl_CheckDriverIsValid(T_pCWSTR pFilePath);

T_U8   Fwl_GetDriverTypeById(T_U8 DeviceId);
T_U16 Fwl_GetDriverIdByPath(T_pCWSTR path);

T_U32  Fwl_GetOldFileByCreateTime(T_pCWSTR FolderPath,T_BOOL isDir,T_U32 reverseCount,
                                             T_pWSTR findfilelongName,T_U32 *createDate,T_U32 *createTime,
                                             T_U64_INT *totalFileSize);

/*
 *@brief: Is The Device Mobile? e.g. SD/USB
 */
T_BOOL Fwl_IsInMobilMedium(T_pCWSTR pFilePath);

T_VOID Fwl_DiskDump(T_U8 *tips);

T_BOOL Fwl_ChkOpenBackFile(T_U8* path, T_U8* bak_path);

/**
 * @BRIEF  this function will create task . When del file is done,
				must be to use the function Fwl_KillCopyThead to stop a task. 
				it will delete the file with name. it can only delete the file or the empty folder,if delCallBack retruen
                    ak_false, File_DelUnicode_DelCallBack  return AK_FALSE
 * @AUTHOR Lu Heshan
 * @DADA	2011-08-11
 * @PARAM [in]T_pCWSTR FileName,  del file or floder name
 * @PARAM [in]T_FS_DEL_CALLBACK delCallBack, call back function
 * @PARAM [in]T_VOID *delCallBackData,  the date for call back.
 * @PARAM [out]T_COPY_CTRL **thrdCtrl, handle of the copy file task. 
 * @RETURN T_BOOL 
 * @RETVAL if return AK_FLASH ,create task fail.
 */
T_BOOL Fwl_FileDel_CallBack(T_pCWSTR FileName, T_FS_DEL_CALLBACK delCallBack, T_VOID *delCallBackData, T_COPY_CTRL **thrdCtrl);

/**
 * @BRIEF  this function will create task . When del file is done,
				must be to use the function Fwl_KillCopyThead to stop a task. 
				it will delete the file with name. it can only delete the file or the empty folder,if delCallBack retruen
                    ak_false, File_DelUnicode_DelCallBack  return AK_FALSE
 * @AUTHOR Lu Heshan
 * @DADA	2011-08-11
 * @PARAM [in]T_pCSTR FileName ,del file or floder name
 * @PARAM [in]T_FS_DEL_CALLBACK delCallBack, call back function
 * @PARAM [in]T_VOID *delCallBackData,  the date for call back.
 * @PARAM [out]T_COPY_CTRL **thrdCtrl,	handle of the copy file task. 
 * @RETURN T_BOOL 
 * @RETVAL if return AK_FLASH ,create task fail.
 */
T_BOOL Fwl_FileDelAsc_CallBack(T_pCSTR FileName, T_FS_DEL_CALLBACK delCallBack, T_VOID *delCallBackData, T_COPY_CTRL **thrdCtrl);

//to kill FsApi_FileCopyThread function create's task
T_VOID Fwl_KillCopyThead(T_COPY_CTRL **ThreadHandle);


T_BOOL Fwl_ResetDirRoot(T_pWSTR pRootDir, T_pWSTR pFilePath);

/************************************************************************
 * NAME:     Fwl_GetAsynBufInfo
 * FUNCTION  check which file when asyn wirte sector error .
 * PARAM:    UseSize--asyn buffer has used size(unit byte)
 *           BufSize--asyn buffer capacity size(unit byte)
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL Fwl_GetAsynBufInfo(T_U32 *UseSize, T_U32 *BufSize);

#ifdef ASYN_CLOSE_FILE_CNT
T_BOOL Fwl_AsynCloseInit(T_U16 maxCount);
T_VOID Fwl_AsynCloseDeInit(T_VOID);
T_S32  Fwl_AsynCloseFile(T_pFILE hdl);
T_S32  Fwl_AsynDeleteFile(T_pFILE hdl);
T_S32  Fwl_AsynCloseFlush(T_VOID);
T_VOID Fwl_AsynCloseFlushAll(T_VOID);
T_VOID Fwl_AsynClosePause(T_VOID);
T_VOID Fwl_AsynCloseResume(T_VOID);
#endif

#ifdef SPIBOOT //SPI BOOT
#define FWL_FILE_SEEK                  Fwl_SPI_FileSeek
#define FWL_FILE_READ                  Fwl_SPI_FileRead
#define FWL_FILE_CLOSE                 Fwl_SPI_FileClose

#define DRI_A                          "A:/"
#define DRI_B                          "A:/"
#define DRI_C                          "A:/"
#define DRI_D                          "A:/"
#define DRI_E                          "A:/"
#define DRI_F                          "A:/"
#else //nand / sd
#define FWL_FILE_SEEK                  Fwl_FileSeek
#define FWL_FILE_READ                  Fwl_FileRead
#define FWL_FILE_CLOSE                 Fwl_FileClose

#define DRI_A                          "A:/"
#define DRI_B                          "B:/"
#define DRI_C                          "C:/"
#define DRI_D                          "D:/"
#define DRI_E                          "E:/"
#define DRI_F                          "F:/"
#endif


#endif

