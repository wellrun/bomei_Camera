#ifndef _SPIFS_SCAN_GC_H
#define _SPIFS_SCAN_GC_H

#include "akspifs_fs.h"
#include "anyka_types.h"
//function declare

extern struct AK_superblock superblock;
T_BOOL AK_check_fs(T_U32 format_startpage,T_U32 spiflash_totalsize);
T_BOOL AK_format( T_U32 page,T_U32 spi_totalsize);
T_BOOL AK_SetConfig( T_U32 format_startpage,T_U32 spiflash_totalsize);
T_BOOL AK_build_dirinfo( T_VOID );
T_VOID AK_destroy_dirinfo( T_VOID );
T_VOID AK_FsbDestroy( T_VOID );
T_BOOL AK_scan_inodetable(  T_U8 * inodedata );
T_BOOL inode_gc( T_VOID );

T_BOOL read_superblock( struct AK_superblock *sb );
T_BOOL write_superblock( struct AK_superblock *sb );
T_BOOL flush_superblock();
T_VOID mark_superblock_dirty( T_VOID );
T_BOOL fix_superblock(struct AK_superblock *sb);
T_BOOL check_superblock(T_VOID);

#endif

