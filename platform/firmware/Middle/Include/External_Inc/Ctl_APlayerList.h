#ifndef _APLAYER_LIST_H_
#define _APLAYER_LIST_H_

#include "Anyka_types.h"
#include "Ctl_FileList.h"


#define METAINFO_ATTACH_NONE    	0x0
#define METAINFO_ATTACH_GENRE   	0x01
#define METAINFO_ATTACH_ARTIST  	0x02
#define METAINFO_ATTACH_ALBUM   	0x04
#define METAINFO_ATTACH_COMPOSER  	0x08

#define METAINFO_ATTACH_OFTEN_PLAY  	0x10
#define METAINFO_ATTACH_RECENTLY_PLAY  	0x20
#define METAINFO_ATTACH_RECENTLY_APPEND 0x40

#define METAINFO_ATTACH_ALL     	0xff
#define MIN_PIC_SIZE			240 

/** AudioPlayer list type */ 
typedef enum{
    LTP_NONE = 0,           /**< no list type */
    LTP_CURPLY,             /**< current play songs list type*/
    LTP_FAVORITELIST,       /**< favorite list list type*/
    LTP_FAVORITE,           /**< favorite songs list type*/

    LTP_GENRE,              /**< genre list type*/
    LTP_ARTIST,             /**< artist list type*/  
    LTP_ALBUM,              /**< album list type*/
    LTP_COMPOSER,           /**< composer list type */
    
    LTP_TITLE,              /**< title list type */
    LTP_COMPOSER_TITLE,     /**< title list type */
    LTP_OFTENPLAY,          /**< open play type */
    LTP_RECEPLAY,           /**< recently play type */
    LTP_RECEAPPEND,         /**< recently add type */
    LTP_MAXNUM              /**< AudioPlayer list type quantity */
}T_LIST_TYPE;





/**
 * @brief   check the file is support or not
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   [in] pFilePath
 * @return  T_BOOL
 * @retval  AK_TRUE  support
 * @retval  AK_FALSE not support
 */
T_BOOL AudioPlayer_IsSupportFile(T_pCWSTR pFilePath);

T_VOID AudioPlayer_SaveCurrentPlayList(T_ICONEXPLORER *pIconExplorer);
T_BOOL AudioPlayer_IsSupportListFile(T_pCWSTR pFilePath);

/**
 * @brief   AudioPlayer add audio
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   [T_pCSTR] pFilepath path of audio, folder or audio list 
 * @param   [T_BOOL] SearchSub Search sub folder or not, it is in effect if pFilePath is a folder 
 * @return  T_BOOL
 * @retval AK_TRUE:send message success; AK_FALSE:send message fail;
 */
T_BOOL AudioPlayer_Add(T_pCWSTR pFilePath, T_BOOL SearchSub);




/**
 * @brief   get total time at stop state, it will be called in main audio player Interface and stop state
 * 
 * @author  lizhuobin
 * @date    2006-08-22
 * @param   T_VOID
 * @return  T_U32
 * @retval  audio total time
 */
T_U32 AudioPlayer_GetTotalTimeAtStopState(T_VOID);

T_VOID APList_Update(T_VOID);

T_pDATA AudioPlayer_GetAudioImage(T_pCWSTR pFilePath, T_LEN width, T_LEN height);
#endif	// _APLAYER_LIST_H_

