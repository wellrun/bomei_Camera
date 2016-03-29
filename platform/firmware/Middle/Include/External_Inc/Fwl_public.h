/**
 * @file Fwl_public.h
 * @brief This file is for include some common used header files
 *
 */

#ifndef __Fwl_public_H__
#define __Fwl_public_H__

#include "fwl_vme.h"
#include "Lib_event.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Eng_GblString.h"
#include "Eng_Graph.h"
#include "Fwl_osMalloc.h"
#include "Lib_res_port.h"
#include "Ctl_IconExplorer.h"
#include "ctl_ebook.h"
#include "fwl_sysevent.h"
#include "eng_debug.h"
#include "ctl_waitbox.h"
#include "Lib_geshade.h"
#include "lib_image_api.h"
#include "Eng_DataConvert.h"
#include "Eng_DynamicFont.h"
#include "fwl_pfdisplay.h"
#include "Akos_api.h"



#define AUDIOLIST_BAK_FILE                  DRI_B"system/backup.alt"
#define AUDIOLIST_TMP_FILE                  DRI_B"tmpfile.alt"
#define AUDIOLIST_CURTPLY_FILE              DRI_B"system/currentplay.alt"
#define AUDIO_META_INFO_FILE                DRI_B"system/metaInfo.dat"
#define AUDIOLIST_DEF_FILE                	DRI_B"system/musiclist.alt"

#define ALBUM_INDEX_FILE                	DRI_B"system/album.ind"
#define GENRE_INDEX_FILE                	DRI_B"system/genre.ind"
#define ARTIST_INDEX_FILE                	DRI_B"system/artist.ind"
#define COMPOSER_INDEX_FILE                	DRI_B"system/composer.ind"

#define VIDEOLIST_DEF_FILE                  DRI_B"system/videolist.vlt"
#define VIDEOLIST_BAK_FILE                  DRI_B"system/backup.vlt"
#define VIDEOLIST_TMP_FILE                  DRI_B"tmpfile.vlt"

#define MP3_SUPPORT_TYPE                    "*.mp1; *.mp2; *.mp3; *.mid; *.wav; *.wma; *.mpg; *.mpeg;*.amr;*.aac;*.adif;*.adts;*.m4a;*.flac;*.ogg;*.oga;*.ape;*.mp4;"
#define MP4_SUPPORT_TYPE                    "*.mp1; *.mp2; *.mp3; *.avi; *.3gp; *.akv; *.mpg; *.mpeg;*.amr;*.aac;*.adif;*.adts;*.m4a;*.mp4;*.flv"//;*.rmvb;*.rm"

#define AK_VERSION_PLATFORM                 "Sword37C V1.0.05"

#if (SDRAM_MODE == 8)
#define RAM_VER                             "_8M"
#elif(SDRAM_MODE == 16)
#define RAM_VER                             "_16M"
#elif(SDRAM_MODE == 32)
#define RAM_VER                             "_32M"
#else
#define RAM_VER                             ""

#endif

#ifdef SPIBOOT
#define BOOT_MODE                           "_SPI"
#endif

#ifdef NANDBOOT
#define BOOT_MODE                           "_NAND"
#endif

#ifdef SDMMCBOOT
#define BOOT_MODE                           "_SDMMC"
#endif

#ifdef SDIOBOOT
#define BOOT_MODE                           "_SDIO"
#endif

#define AK_VERSION_SOFTWARE                 AK_VERSION_PLATFORM BOOT_MODE RAM_VER

#if (defined (CHIP_AK3753))
#define AK_VERSION_HARDWARE                 "AK3753_CB_V1.0"
#elif (defined (CHIP_AK3750))
#define AK_VERSION_HARDWARE                 "AK3750_V2.0"
#else
#define AK_VERSION_HARDWARE                 "AK3760_V3.0"
#endif

T_VOID ReturnDefauleProc(T_eBACK_STATE state, T_EVT_PARAM *pEventParm);
T_BOOL IsPostProcessEvent(T_EVT_CODE event);
T_BOOL MenuStructInit(T_ICONEXPLORER *pIconExplorer);

T_VOID Menu_LoadRes(T_VOID);
T_VOID Menu_FreeRes(T_VOID);
T_VOID Standby_LoadUserBkImg(T_VOID);
T_VOID Standby_FreeUserBkImg(T_VOID);

T_BOOL AK_GetScreenSaverStatus(T_VOID);
T_VOID SetKeyLightCount(T_U16 second);
T_VOID PublicTimerStart(T_VOID);
T_VOID PublicTimerStop(T_VOID);
T_VOID VME_Reset(T_VOID);
T_VOID VME_Main(T_VOID);
void CMain(unsigned long argc, void *argv);

#endif
