/**
 * @file Eng_Id3Inf.h
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */

#ifndef _ENG_ID3INF_H_
#define _ENG_ID3INF_H_

#include "anyka_types.h"


#define MEDIA_CLASSNAME_SIZE	64


typedef struct{
    T_WCHR          genre[MEDIA_CLASSNAME_SIZE/2 + 1];      /**< genre */                       
    T_WCHR          artist[MEDIA_CLASSNAME_SIZE/2 + 1];  	 /**< artist */
    T_WCHR          album[MEDIA_CLASSNAME_SIZE/2 + 1];      /**< album */
    T_WCHR          composer[MEDIA_CLASSNAME_SIZE/2 + 1];   /**< composer */
} T_SONG_INFO, *T_pSONG_INFO;




/**
* @brief Get metalInfo by path
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pVOID fp : file handle
* @param out T_pSONG_INFO songInfo : info
* @return T_BOOL
* @retval
*/
T_BOOL ID3_GetMetaInfo(T_pVOID fp, T_pSONG_INFO songInfo);

#endif

