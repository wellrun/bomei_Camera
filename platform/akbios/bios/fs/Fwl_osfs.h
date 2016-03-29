/**
 * @file Fwl_osFS.h
 * @brief This header file is for OS related function prototype
 *
 */
#ifndef __FWL_OSFS_H__
#define __FWL_OSFS_H__

#include "anyka_types.h"

#define     FS_MAX_PATH_LEN     260 // unicode len

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
#define _FOPEN_FAIL     FS_INVALID_HANDLE

#define _FMODE_READ     FS_MODE_READ
#define _FMODE_WRITE    FS_MODE_APPEND
#define _FMODE_CREATE   FS_MODE_CREATE
#define _FMODE_OVERLAY  FS_MODE_OVERLAY

#define _FSEEK_CUR      FS_SEEK_CUR
#define _FSEEK_END      FS_SEEK_END
#define _FSEEK_SET      FS_SEEK_SET

#define EXPLORER_ISFOLDER 0x10

T_BOOL  Fwl_InitFs(T_VOID);
T_VOID  Fwl_DeInitFs(T_VOID);
T_pFILE Fwl_FileOpen(T_pCWSTR path, T_FILE_FLAG flag, T_FILE_MODE mode);
T_pFILE Fwl_FileOpenAsc(T_pSTR path, T_FILE_FLAG flag, T_FILE_MODE mode);
T_S32   Fwl_FileRead(T_pFILE hFile, T_pVOID buffer, T_U32 count);
T_S32   Fwl_FileWrite(T_pFILE hFile, T_pCVOID buffer, T_U32 count);
T_BOOL  Fwl_FileClose(T_pFILE hFile);
T_U32   Fwl_GetFileLen(T_pFILE hFile);
T_BOOL  Fwl_FileDelete(T_pCWSTR path);





#endif

