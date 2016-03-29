#ifndef _SPIFS_FLASH_H_
#define _SPIFS_FLASH_H_

#include "anyka_types.h"


T_U32 flash_read( T_U32 from, T_U8 *buf, T_U32 len );
T_U32 flash_write( T_U32 to, T_U8 *buf, T_U32 len );
T_BOOL flash_erase_fs_sectors(T_U32 page,T_U32 spi_totalsize);

T_BOOL flash_erase_random4K( T_U32 sector_addr,T_U32 del_4k_num);
T_S32 flash_read_wholepage_direct(T_U32 addr, T_U8 *buf, T_U32 len);
T_S32 flash_write_wholepage_direct(T_U32 addr, T_U8 *buf, T_U32 len);
#endif

