/**
 * @file ctl_medialist.h
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */

#ifndef _CTL_MEDIALIST_H_
#define _CTL_MEDIALIST_H_

#include "Ctl_IconExplorer.h"
#include "svc_medialist.h"





/**
* @brief add items to IconExplorer in step when media list is adding
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_ICONEXPLORER *pIconExplorer : IconExplorer handle
* @param in/out T_U16* StartId : start id
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL Ctl_MList_ToIconExplorerStep(T_ICONEXPLORER *pIconExplorer, T_U16* StartId, T_eMEDIA_LIST_TYPE type);



/**
* @brief add all items to IconExplorer when media list is not adding
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_ICONEXPLORER *pIconExplorer : IconExplorer handle
* @param in/out T_U16 *StartId : start id
* @param out T_S32* firstHoleId : first empty id
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL Ctl_MList_ToIconExplorerComplete(T_ICONEXPLORER *pIconExplorer, T_U16 *StartId, T_S32* firstHoleId, T_eMEDIA_LIST_TYPE type);



/**
* @brief add id3 classname to IconExplorer in step when media list is adding
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_ICONEXPLORER *pIconExplorer : IconExplorer handle
* @param in/out T_U16* StartId : start id
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @return T_BOOL
* @retval 
*/
T_BOOL Ctl_MList_ID3_ToIconExplorerStep(T_ICONEXPLORER *pIconExplorer, T_U16* StartId, T_eID3_TAGS tag);


/**
* @brief add all id3 classname to IconExplorer when media list is not adding
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_ICONEXPLORER *pIconExplorer : IconExplorer handle
* @param in/out T_U16 *StartId : start id
* @param out T_S32* firstHoleId : first empty id
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @return T_BOOL
* @retval 
*/
T_BOOL Ctl_MList_ID3_ToIconExplorerComplete(T_ICONEXPLORER *pIconExplorer, T_U16 *StartId, T_S32* firstHoleId, T_eID3_TAGS tag);


/**
* @brief add songs to IconExplorer
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_ICONEXPLORER *pIconExplorer : IconExplorer handle
* @param in/out T_U16* StartId : start id
* @param in T_pCWSTR className : class name
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @return T_BOOL
* @retval 
*/
T_BOOL Ctl_MList_ID3_SongToIconExplorer(T_ICONEXPLORER *pIconExplorer, T_U16* StartId, T_pCWSTR className, T_eID3_TAGS tag);




/**
* @brief add items to IconExplorer by append time in step when media list is adding
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_ICONEXPLORER *pIconExplorer : IconExplorer handle
* @param in/out T_U16* StartId : start id
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL Ctl_MList_ToIEByAppendTimeStep(T_ICONEXPLORER *pIconExplorer, T_U16* StartId, T_eMEDIA_LIST_TYPE type);



/**
* @brief add all items to IconExplorer when media list is not adding
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_ICONEXPLORER *pIconExplorer : IconExplorer handle
* @param in/out T_U16 *StartId : start id
* @param out T_S32* firstHoleId : first empty id
* @param in T_eMEDIA_PLAYINFO_TYPE infoType : play count / play time / append time
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL Ctl_MList_ToIEByPlayInfoComplete(T_ICONEXPLORER *pIconExplorer, T_U16 *StartId, T_S32* firstHoleId, T_eMEDIA_PLAYINFO_TYPE infoType, T_eMEDIA_LIST_TYPE type);

#endif
