/**
 * @file Apl_fStandby.h
 * @brief This file is for standby  function ( field strength, battery etc) prototype
 *
 */

#ifndef __FWL_INITIALIZE_H__
#define __FWL_INITIALIZE_H__

#include "anyka_types.h"
#include "gbl_global.h"

#define PON_PIC                     DRI_D"IMAGE/pon.bmp"
#define POFF_PIC                    DRI_D"IMAGE/poff.bmp"
#define STDB_PIC                    DRI_D"IMAGE/stdb.bmp"
#define MENU_PIC                    DRI_D"IMAGE/menu.bmp"
#define PON_CACHE_PIC               DRI_B"SYSTEM/pon_cah.bmp"
#define POFF_CACHE_PIC              DRI_B"SYSTEM/poff_cah.bmp"
#define STDB_CACHE_PIC              DRI_B"SYSTEM/stdb_cah.bmp"
#define MENU_CACHE_PIC              DRI_B"SYSTEM/menu_cah.bmp"
#define USER_PROFILE                DRI_B"SYSTEM/profile.dat"

#define SYSTEM_PATH     DRI_B"SYSTEM/"                       
#define AUDIO_PATH      DRI_D"AUDIO/"
#define AUDIOREC_PATH   DRI_D"AUDIO/RECORD/"
#define FMREC_PATH      DRI_D"AUDIO/RECORD/"
#define AUDIOLIST_PATH  DRI_D"AUDIO/ALT/"
#define VIDEO_PATH      DRI_D"VIDEO/"
#ifdef CAMERA_SUPPORT
#define VIDEOREC_PATH   DRI_D"VIDEO/RECORD/"
#endif
#define VIDEOLIST_PATH  DRI_D"VIDEO/VLT/"
#define IMAGE_PATH      DRI_D"IMAGE/"
#ifdef CAMERA_SUPPORT
#define IMAGEREC_PATH   DRI_D"IMAGE/RECORD/"
#endif
#define EBOOK_PATH      DRI_D"EBOOK/"
#define EMAP_PATH       DRI_D"EMAP/"
#define GAME_PATH       DRI_D"GAME/"
#define GAMENES_PATH    DRI_D"GAME/"
#define GAMESNES_PATH   DRI_D"GAME/"
#define GAMEGBA_PATH    DRI_D"GAME/"
#define GAMEMD_PATH     DRI_D"GAME/"
#ifdef SUPPORT_FLASH
#define FLASH_PATH     DRI_D"FLASH/"
#endif
#define RECIDX_PATH		DRI_B"RECIDX/"

#ifdef SUPPORT_VFONT
#define VFONT_PATH		DRI_D"FONT/"
#endif
#define UPDATE_PATH		DRI_D"UPDATE/"

#ifdef SUPPORT_NETWORK
#define NETWORK_PATH	DRI_D"NETWORK/"
#endif

// define name of whole lib using string
   //the order of the lib list must not change!! 
#if 1   //def OS_ANYKA

typedef enum 
{
    ERR_LIB = -1,

    IMAGE_LIB,

#ifdef SUPPORT_GE_SHADE
    GESHADE_LIB,
#endif

    MEDIA_LIB,
    VIDEO_LIB,
    SDCODEC_LIB,
    SDFILTER_LIB,

//    SMCORE_LIB,
//    AKOS_LIB,    
    FS_LIB,
    MOUNT_LIB,
    //MTD_LIB,
    FHA_LIB,
    DRV_LIB,
#ifdef SUPPORT_NETWORK
	LWIP_LIB,
#endif

    MAX_LIB_NUM
}T_eLIB_NAME;

typedef T_VOID * (*_GetLibVersion)(T_VOID) ;

#endif


#define USE_PROJMODE_FUN


#define POWER_ON_PIC                1
#define POWER_OFF_PIC               2
#define STAND_BY_PIC                3
#define MENU_BK_PIC                 4

#define INVALID_KEY_INFO            0xff

#define VALID_NES_KEYID(nKeyId)    ((nKeyId) < nValidGamKeyNum[GAME_NES] && (nKeyId) >= 0)

#define VALID_SNES_KEYID(nKeyId)    ((nKeyId) < nValidGamKeyNum[GAME_SNES] && (nKeyId) >= 0)

#define VALID_GBA_KEYID(nKeyId)    ((nKeyId) < nValidGamKeyNum[GAME_GBA] && (nKeyId) >= 0)

#define VALID_MD_KEYID(nKeyId)    ((nKeyId) < nValidGamKeyNum[GAME_MD] && (nKeyId) >= 0)


#define VALID_SIM_GAME_KEYID(nGameType, nKeyId)    (GAME_NES <= (nGameType) &&  (nGameType) <= GAME_MD &&  (nKeyId) < nValidGamKeyNum[(nGameType)] && (nKeyId) >= 0)

#define VALID_SIM_GAME_TYPE(nGameType)  ((nGameType) >= GAME_NES && (nGameType) <= GAME_MD)

#define VALID_DEV_KEY_TYPE(nKeyType) ((nKeyType) >=0 && (nKeyType) < MAX_INPUT_TYPE)

#define VALID_DEV_KEY_ID(nKeyType, nKeyId)  (((nKeyType) >=0 && (nKeyType) < MAX_INPUT_TYPE) && (0 <= (nKeyId) && (nKeyId) < nValidPhyKeyNum[(nKeyType)]))




T_VOID  InitVariable(T_VOID);
T_BOOL  ReadUserdata(T_VOID);
T_BOOL  SaveUserdata(T_VOID);
T_VOID  GetDefUserdata(T_VOID);
T_VOID Fwl_InitDefPath(T_VOID);
T_BOOL Fwl_DirIsDefPath(T_pWSTR pPath);
T_BOOL Fwl_SetDefPath(T_DEFPATH_TYPE type, T_pWSTR pPath);
T_BOOL Fwl_CreateDefPath(T_VOID);
T_pWSTR Fwl_GetDefPath(T_DEFPATH_TYPE type);

/**
 * @brief check lib version
 *
 * @author 
 * @date 
 * @param T_VOID
 * @return T_BOOL
 * @retval AK_TRUE: version consistent , AK_FALSE: version disaccord
 */
T_BOOL CheckLibVersion(T_VOID);


T_BOOL    SetDevKey_GameKeyMapping(T_S32 nGameType, T_S32 nDevType, T_S32   nDevKeyId, T_S32  nGameVkeyId);

T_S32 GetGameVkey_DevKeyMapped(T_S32 nGameType , T_S32 nDevType, T_S32 nDevKeyId);

T_BOOL  SetGameKey_DevKeyMapping(T_S32 nGameType, T_S32  nGameVkeyId, T_S32 nDevType, T_S32   nDevKeyId);

T_BOOL    AdjustGameKey_DevKeyMapping(T_S32 nGameType, T_S32  nGameVkeyId, T_S32 nDevType, T_S32   nDevKeyId);

T_U8 * GetDevTypeNameStr(T_S32    nInputKeyType);

T_U8 * GetDevKeyNameStr(T_S32   nDevTye, T_S32  nKeyId);

T_U8 * GetGameVKeyNameStr(T_S32 nGameType, T_S32    nKeyId);

T_S32   GetDevType_SimGameKeyMapped(T_S32 nGameType, T_S32 nVkeyId);

T_S32   GetDevKeyValue_SimGameKeyMapped(T_S32 nGameType, T_S32 nVkeyId);



#endif
