#ifndef __FWL_OSFS_H__
#define __FWL_OSFS_H__

#include "anyka_types.h"
#include "file.h"

#ifndef T_hFILE
#define     T_hFILE             T_S32
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

#ifndef FS_INVALID_SEEK
#define     FS_INVALID_SEEK -1
#endif


T_hFILE Fwl_FileOpen(T_pCSTR path, T_FILE_MODE mode);
T_VOID  Fwl_FileClose(T_hFILE hFile);
T_U32   Fwl_FileSeek(T_hFILE hFile, T_S32 offset, T_U16 origin);
T_U32   Fwl_FileRead(T_hFILE hFile, T_pVOID buffer, T_U32 count);
T_U32   Fwl_FileWrite(T_hFILE hFile, T_pCVOID buffer, T_U32 count);
T_BOOL  Fwl_MkDir(T_pCSTR path);

T_BOOL Fwl_MountInit(T_VOID);
T_BOOL Fwl_FSAInit(T_VOID);

#endif //__FWL_OSFS_H__

