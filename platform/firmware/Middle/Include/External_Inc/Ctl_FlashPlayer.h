#ifndef __CTL_FLASHPLAYER_H__
#define __CTL_FLASHPLAYER_H__
#include "fwl_osfs.h"
#include "af_player.h"

#define SHA204_DEVICE_PERSONALIZATION	0

/*flash initilize cb func*/
T_pVOID Fwl_Malloc_flash(T_S32 size);
T_pVOID Fwl_Free_flash(T_pVOID ptr);
T_pVOID Fwl_ReMalloc_flash(T_pVOID ptr, T_S32 size);
T_pFILE	Fwl_FileOpen_flash(T_pCWSTR path, AF_FILE_OPEN_TYPE flags);
T_BOOL	Fwl_FileClose_flash(T_pFILE hFile);
T_S32	Fwl_FileRead_flash(T_pFILE hFile, T_pVOID buffer, T_U32 count);
T_S32	Fwl_FileSeek_flash(T_hFILE hFile, T_S32 offset, T_U16 origin);
T_S32   Fwl_FileWrite_flash(T_hFILE hFile, T_pVOID  *buf, T_U32 count);
T_S32   Fwl_FileTell_flash(T_hFILE hFile);
T_S32   Fwl_FileStat_flash(T_S8 * file_name, T_S16 *st_dev,T_U16 *st_ino, T_U16 *st_mode,T_S16 *st_nlink, T_U16 *st_uid, T_U16 *st_gid,T_S16 *st_rdev, T_S32 *st_size, long int *st_atime_1, long int *st_mtime_1, long int *st_ctime_1);

/*Flash sound output cbfunc*/
T_S32  Fwl_SoundOpen_flash(T_S32 freq,T_S32 channel,T_S32 bits);
T_S32  Fwl_SoundWrite_flash(T_pVOID buf, T_S32 Length);
T_VOID Fwl_SoundClose_flash(T_VOID);

/*Flash sound decode cbfunc*/
T_S32  mp3codec_open(T_S32 freq, T_S32 channel, T_S32 bits);
T_VOID mp3codec_close(T_VOID);
T_S32  mp3codec_fill(T_U8 *src_data, T_S32 size);
T_S32  mp3codec_decode(T_pVOID pcm, T_S32 size);

T_S32 Flash_GetHash(T_U8 *data, T_U8 *haData,const T_U8 key_Id);

T_VOID  Flash_Set_ChipID(T_VOID);

#endif
