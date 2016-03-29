#ifndef _SPIFS_API_H_
#define _SPIFS_API_H_

#include "anyka_types.h"

T_BOOL VME_FsbInit( T_U32 format_startpage,T_U32 spiflash_totalsize);
T_S16  VME_FsbOpen( const T_S8 * path, T_U16 flag, T_U16 mode );
T_S16  VME_FsbClose( T_S16 fd );
T_U32  VME_FsbRead( T_S16 fd, T_U8 *buf, T_U32 count );
T_U32  VME_FsbWrite( T_S16 fd, const T_U8 *buf, T_U32 count);
T_BOOL VME_FsbDelete( const T_S8 * path );
T_S32  VME_FsbLseek( T_S16 fd, T_S32 offset, T_S16 origin);
T_BOOL VME_FsbMkdir( const T_S8 * path );
T_BOOL VME_FsbRmdir( const T_S8 * path );
T_BOOL VME_FsbIsdir( const T_S8 * path );
T_BOOL VME_FsbFormat( T_U32 page,T_U32 spi_totalsize );
T_BOOL VME_FsbGfirst(struct vDSTAT *statobj, const T_S8 * pattern );
T_BOOL VME_FsbGnext ( struct vDSTAT *statobj );

T_S32  VME_FsbFree( T_VOID );
T_S16  VME_FsbStat( const T_S8 * path,struct vSTAT *pstat );
T_S16  VME_FsbFstat( T_S16 fd, struct vSTAT *pstat );
T_S32  VME_GetLastError( T_VOID );
T_BOOL VME_FsbRename( const T_S8 *oldPath, const T_S8 *newPath );
T_U32  VME_Getfilelen( const T_S8 * path, T_U16 flag, T_U16 mode );

#endif