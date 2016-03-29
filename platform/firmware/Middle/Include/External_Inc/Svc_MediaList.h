/**
 * @file svc_medialist.h
 * @brief ANYKA software
 * 
 * @author 
 * @date  
 * @version 1,0 
 */


#ifndef _SVC_MEDIALIST_H_
#define _SVC_MEDIALIST_H_


#include "anyka_types.h"
#include "Fwl_osmalloc.h"
#include "Fwl_osfs.h"
#include "eng_id3inf.h"


#if (SDRAM_MODE <= 8)
#define MAX_MEDIA_NUM 			(2 << 10)//2K
#else
#define MAX_MEDIA_NUM 			(3 << 10)//3K
#endif

#define MEDIA_NUM_SIZE			2

#define MEDIA_INDEX_SIZE		(MAX_MEDIA_NUM<<2)
#define MEDIA_MAP_SIZE			(MAX_MEDIA_NUM<<1)

#define MEDIA_BITMAP_SIZE		(MAX_MEDIA_NUM>>3)
#define MEDIA_PATH_CKSUM_SIZE	(MAX_MEDIA_NUM<<1)
#define MEDIA_CLASS_CKSUM_SIZE	(MAX_MEDIA_NUM)
#define MEDIA_PLAY_INFO_SIZE	(MAX_MEDIA_NUM<<3)

#define MEDIA_ID3_HEAD_SIZE		(MEDIA_BITMAP_SIZE + MEDIA_MAP_SIZE + MEDIA_INDEX_SIZE + MEDIA_CLASS_CKSUM_SIZE)
#define MEDIA_FILE_HEAD_SIZE	(MEDIA_NUM_SIZE + MEDIA_BITMAP_SIZE + MEDIA_PATH_CKSUM_SIZE + MEDIA_PLAY_INFO_SIZE)



#define MEDIA_PATH_SIZE			(MAX_FILENM_LEN<<1)

#define MEDIA_FILEITEM_SIZE		(MEDIA_PATH_SIZE + 4*MEDIA_CLASSNAME_SIZE)

#define MEDIA_LIST_STATE_OPENED 	0x1		// bit0
#define MEDIA_LIST_STATE_RUNNING	0x2		// bit1
#define MEDIA_LIST_REQ_SUSPEND		0x4		// bit2

#define ATTACH_PLAY_COUNT		5		// 播放次数大于5次就显示在最常播放列表
#define ATTACH_PLAY_TIME        259200  //3*24*60*60,最近3天播放的歌曲显示在最近播放列表
#define ATTACH_APPEND_TIME      259200  //3*24*60*60,最近3天添加的歌曲显示在最近增加列表
#define ATTACH_ERROR_TIME		1


/** Use this flag to set audio player max quantity audio */
#define AUDIOPLAYER_MAX_ITEM_QTY            MAX_MEDIA_NUM
#define MYLIST_MAX_ITEM_QTY            		100

#define	MLIST_HOLE_NONE	-1

typedef enum{
	eMEDIALIST_EVT_ADD,
	eMEDIALIST_EVT_DEL,
	eMEDIALIST_EVT_DELALL,
	eMEDIALIST_EVT_ID3_DELITEM,
	eMEDIALIST_EVT_ID3_DELCLASS,
	eMEDIALIST_EVT_CLOSE,
	eMEDIALIST_EVT_NUM
}T_eMEDIA_LIST_EVENT;


typedef enum tag_eID3_TAGS{
	eID3_TAGS_GENRE 	= 0,
	eID3_TAGS_ARTIST 	= 1,
	eID3_TAGS_ALBUM 	= 2,
	eID3_TAGS_COMPOSER 	= 3,
	eID3_TAGS_NUM	
	
}T_eID3_TAGS;

typedef enum{
	eMEDIA_LIST_AUDIO,
	eMEDIA_LIST_VIDEO,
	eMEDIA_LIST_NUM
}T_eMEDIA_LIST_TYPE;

typedef enum{
	eMEDIA_PLAYCNT,
	eMEDIA_PLAYTIME,
	eMEDIA_APPENDTIME,
	eMEDIA_PLAYINFO_NUM
}T_eMEDIA_PLAYINFO_TYPE;



typedef enum {
    eMEDIALIST_ADD_NONE,			/**< add none */
    eMEDIALIST_ADD_ERROR,			/**< add audio error */
    eMEDIALIST_ADD_SUCCESS,			/**< add audio success */
    eMEDIALIST_ADD_NOSPACE,			/**< have no space, can not add audio */
    eMEDIALIST_ADD_OUTPATHDEEP,		/**< Out of Directory Deep, Protect Stack Memory */
    eMEDIALIST_ADD_RET_NUM			/**< add audio return value num */
} T_eMEDIALIST_ADD_RET;


typedef enum {
    eID3_WRITE_NONE,		/**< write none */
    eID3_WRITE_HEAD_ONLY,	/**< write head only */
    eID3_WRITE_NAME,		/**< write id3 name */
    eID3_WRITE_NOSPACE,		/**< have no space can not add id3 */
    eID3_WRITE_RET_NUM		/**< write id3 return value num */
} T_eID3_WRITE_RET;

typedef enum {
	eINDEX_TYPE_AUDIO		= 0,
	eINDEX_TYPE_VIDEO		= 1,
	eINDEX_TYPE_GENRE 		= 2,
	eINDEX_TYPE_ARTIST 		= 3,
	eINDEX_TYPE_ALBUM 		= 4,
	eINDEX_TYPE_COMPOSER 	= 5,
	eINDEX_TYPE_NUM	
	
}T_eINDEX_TYPE;


#define ID3_TAGS_SIZE		(eID3_TAGS_NUM<<1)


/*
 * @brief  ID3 Class, eg. Blues, Disco, Michael Jackson Etc.
 */ 	
typedef struct tag_Id3ClassIndex{
	T_U16 		pos;	//!< Item Start Position In Map
	T_U16 		qty;	//!< Item Number of The Class
			
}T_ID3_CLASS_INDEX;

/*
 * @brief Represent ID3 Group, eg. Artist, Genre Etc.
 */ 	
typedef struct tag_Id3Group{
	T_U8				bitmap[MEDIA_BITMAP_SIZE];	//!< Map Class Index File Name Area
	T_U16				idxmap[MAX_MEDIA_NUM]; 		//!< Album Name/Media Path Map Table
	T_ID3_CLASS_INDEX	index[MAX_MEDIA_NUM];		//!< Album Name/Media Path Index Table
	T_U8				ckSum[MAX_MEDIA_NUM];		//!< Member bitmap Class Name ckecksum, 8 bits
	T_hFILE 			fdFore; 					//!< ID3 Index File Handle For Foreground Read
	T_hFILE 			fdBack; 					//!< ID3 Index File Handle For Background RW
	
}T_ID3_GROUP;

typedef struct 
{
	T_U32	Cnt;    	/**< song's play count */ 
    T_U32	Time;		/**< recently play time or append time(unit s) */ 
			
}T_MEDIA_PLAY_INFO;


typedef struct tag_MediaList{
	T_U8				bitmap[MEDIA_BITMAP_SIZE]; 	//!< Map Media List Index File Name Area
	T_U16				ckSum[MAX_MEDIA_NUM]; 		//!< Member bitmap Path Name ckecksum, 16bits
	T_MEDIA_PLAY_INFO	playInfo[MAX_MEDIA_NUM];	//!< play count and play time
	T_hFILE 			fdFore; 					//!< The Handle of Media File, eg. Audiolist.atl, For Foreground Read
	T_hFILE 			fdBack; 					//!< The Handle of Media File, eg. Audiolist.atl, For Background RW
	T_ID3_GROUP 		*id3;						//!< If Audio List, Has ID3; Use Array
	T_U8				state;						//!< Media List State (bit0: Opened; bit1: Running; bit2: ReqSuspend) 
	T_BOOL				bAdding;
	
}T_MEDIA_LIST, T_pMEDIA_LIST;


typedef enum {
	eADD_FLAG_NONE		= 0x0,
	eADD_FLAG_NEW		= 0x1,	//bit 1
	eADD_FLAG_STEP 		= 0x2,	//bit 2
	
}T_eADD_FLAG_TYPE;


typedef struct
{
    T_MEDIA_LIST		*plist;			/**< media list */            
    T_hSemaphore		mutex;
	T_eADD_FLAG_TYPE	addFlag;
	T_BOOL				delFlag;
	
}T_GB_MLIST;


/** key word struct of search */ 
typedef struct{
    T_U16   *pGenre;       /**< genre string or "\0" */
    T_U16   *pArtist;      /**< artist string or "\0" */
    T_U16   *pAlbum;       /**< album string or "\0" */
    T_U16   *pComposer;    /**< composer string or "\0" */
}T_CLASSINFO;



typedef struct
{
    T_CLASSINFO     ClassInfo;      /**< key word for search */
}T_LIST_CLASS_INFO;

typedef struct
{
    T_U16			type;			/**< T_eINDEX_TYPE */            
    T_U16			id;      		/**< index */
}T_INDEX_CONTENT;


/**
* @brief memset medialist struct
*
* @author Songmengxing
* @date 2011-12-26
* @return T_VOID
* @retval 
*/
T_VOID MList_Memset(T_VOID);


/**
* @brief Open Media List (Audio / Video) Service for Read Media List (ID3 Information) Mutex
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_S32 count
* @retval -1 open fail; >=0 open success;
*/
T_S32 MList_Open(T_eMEDIA_LIST_TYPE type);



/**
* @brief Close Media List (Audio / Video) Service Mutex
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL MList_Close(T_eMEDIA_LIST_TYPE type);

/**
* @brief add a Path (Folder / File) to Audio / Video List
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR path : Path (Folder / File) 
* @param in T_BOOL subFolder : search subfolder or not
* @param in T_BOOL resvOld : reserve old list or not
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval
*/
T_BOOL MList_AddItem(T_pCWSTR path, T_BOOL subFolder, T_BOOL resvOld, T_eMEDIA_LIST_TYPE type);

/**
* @brief Change Updating Media List Thread Priority
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U8 priority : priority
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval
*/
T_BOOL MList_ChangePriority(T_U8 priority, T_eMEDIA_LIST_TYPE type);



/**
* @brief Get Media List (Audio / Video) Quantity
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_S32 count
* @retval -1 get fail; >=0 get success;
*/
T_S32 MList_GetItemQty(T_eMEDIA_LIST_TYPE type);

/**
* @brief Get Media List (Audio / Video) state
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_U8 state
* @retval 
*/
T_U8 MList_State(T_eMEDIA_LIST_TYPE type);


/**
* @brief Set Media List suspend all
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_VOID
* @return T_BOOL
* @retval 
*/
T_BOOL MList_SuspendAll(T_VOID);


/**
* @brief Get Media Path (Audio / Video)
*
* @author Songmengxing
* @date 2011-12-26
* @param out T_pCWSTR path : Path 
* @param in T_U16 ItemId : id of item
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_U16 Media Quantity
* @retval
*/
T_U16 MList_GetItem(T_pWSTR path, T_U16 ItemId, T_eMEDIA_LIST_TYPE type);

/**
* @brief Remove a Media (Audio / Video) From Media List
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR path : a Media (Audio / Video)
* @param in T_BOOL includeFile : remove file or not
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval
*/
T_BOOL MList_RemoveMediaItem(T_pCWSTR path, T_BOOL includeFile, T_eMEDIA_LIST_TYPE type);

/**
* @brief Remove All Media Item (Audio / Video) From Media List
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval
*/
T_BOOL MList_RemoveAll (T_eMEDIA_LIST_TYPE type);





/**
* @brief add a Path (Folder / File) to Audio / Video List  background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pWSTR path : Path (Folder / File) 
* @param in T_BOOL subFolder : search subfolder or not
* @param in T_BOOL resvOld : reserve old list or not
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_eMEDIALIST_ADD_RET
* @retval
*/
T_eMEDIALIST_ADD_RET MList_BG_AddPath(T_pWSTR path, T_BOOL subFolder, T_BOOL resvOld, T_eMEDIA_LIST_TYPE type);

/**
* @brief Remove a Media (Audio / Video) From Media List background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pWSTR path : a Media (Audio / Video)
* @param in T_BOOL includeFile : remove file or not
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval
*/
T_BOOL MList_BG_DelPath(T_pWSTR path, T_BOOL includeFile, T_eMEDIA_LIST_TYPE type);


/**
* @brief Remove All Media Item (Audio / Video) From Media List  background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval
*/
T_BOOL MList_BG_RemoveAll (T_eMEDIA_LIST_TYPE type);


/**
* @brief Get Class Quantity in Tag (Artist / Album / Genre / Composer)
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eID3_TAGS tag : Artist / Album / Genre / Composer
* @return T_U16 Class Quantity
* @retval
*/
T_U16 MList_ID3_GetClassQty(T_eID3_TAGS tag);

/**
* @brief Get Class Initializing Name (Jazz, 张学友 … …)
*
* @author Songmengxing
* @date 2011-12-26
* @param out T_pWSTR name : class name
* @param in T_U16 classId : id of class
* @param in T_eID3_TAGS tag : Artist / Album / Genre / Composer
* @return T_U16 Class Initializing Quantity In This Tag
* @retval
*/
T_U16 MList_ID3_GetClassName(T_pWSTR name, T_U16 classId, T_eID3_TAGS tag);

/**
* @brief Get Artist/Genre's Album Quantity
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR name : class name
* @param in T_eID3_TAGS tag : Artist / Genre 
* @return T_U16 Album Quantity
* @retval
*/
T_U16 MList_ID3_GetAlbumQty(T_pCWSTR name, T_eID3_TAGS tag);

/**
* @brief Get Album Name In Artist/Genre Tag
*
* @author Songmengxing
* @date 2011-12-26
* @param out T_pWSTR T_albumName : album name
* @param in T_pCWSTR name : class name
* @param in T_U16 id : album id in the class
* @param in T_eID3_TAGS tag : Artist / Genre
* @return T_U16 album qty of the class
* @retval
*/
T_U16 MList_ID3_GetAlbumName(T_pWSTR T_albumName, T_pCWSTR name,  T_U16 id, T_eID3_TAGS tag);

/**
* @brief Get Song Quantity of Album/Composers 
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR name : class name
* @param in T_eID3_TAGS tag : Album/Composers 
* @return T_U16 Song Quantity
* @retval
*/
T_U16 MList_ID3_GetSongQty(T_pCWSTR name, T_eID3_TAGS tag);

/**
* @brief Get Song Path from Album/Composer/Artist/Genre
*
* @author Songmengxing
* @date 2011-12-26
* @param out T_pWSTR path : song path
* @param in T_pCWSTR className : class name
* @param in T_U16 albumIdx : album index
* @param in T_U16 songIdx : song index
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @return T_U16 
* @retval
*/
T_U16 MList_ID3_GetSongPath(T_pWSTR path, T_pCWSTR className, T_U16 albumIdx, T_U16 songIdx, T_eID3_TAGS tag);

/**
* @brief Remove A Class From ID3 Index File. 
* @If Remove a Album From ALBUM TAG, All This Index Information About The Album Will Be Remove;
* @Others Will Be ONLY Remove Index Information In THIS TAG Index File
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR name : class name
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @return T_BOOL
* @retval
*/
T_BOOL MList_ID3_RemoveClass(T_pCWSTR name, T_eID3_TAGS tag);

/**
* @brief Delete A Item From ID3 Index File
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR name : file name
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @return T_BOOL
* @retval
*/
T_BOOL MList_ID3_RemoveItem(T_pCWSTR name, T_eID3_TAGS tag);


/**
* @brief Delete A Item From ID3 Index File background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR name : file name
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @return T_BOOL
* @retval
*/
T_BOOL MList_BG_ID3_RemoveItem(T_pWSTR name, T_eID3_TAGS tag);

/**
* @brief Remove A Class From ID3 Index File background. 
* @If Remove a Album From ALBUM TAG, All This Index Information About The Album Will Be Remove;
* @Others Will Be ONLY Remove Index Information In THIS TAG Index File
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pWSTR name : class name
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @return T_BOOL
* @retval
*/
T_BOOL MList_BG_ID3_RemoveClass(T_pWSTR name, T_eID3_TAGS tag);


/**
* @brief Check Media List (Audio / Video) is adding or not
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL MList_IsAdding(T_eMEDIA_LIST_TYPE type);


/**
* @brief Get file id by file name
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR filename : file name
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_U16 class id
* @retval MAX_MEDIA_NUM can't find
*/
T_U16 MList_GetPathId(T_pCWSTR filename, T_eMEDIA_LIST_TYPE type);


/**
* @brief get add flag
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_eADD_FLAG_TYPE
* @retval 
*/
T_eADD_FLAG_TYPE MList_GetAddFlag(T_eMEDIA_LIST_TYPE type);

/**
* @brief set add flag
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @param in T_eADD_FLAG_TYPE flag : 0,1 or 2
* @return T_BOOL
* @retval 
*/
T_BOOL MList_SetAddFlag(T_eMEDIA_LIST_TYPE type, T_eADD_FLAG_TYPE flag);

/**
* @brief get delete flag
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL MList_GetDelFlag(T_eMEDIA_LIST_TYPE type);

/**
* @brief set delete flag
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @param in T_BOOL flag : true or false
* @return T_BOOL
* @retval 
*/
T_BOOL MList_SetDelFlag(T_eMEDIA_LIST_TYPE type, T_BOOL flag);

/**
* @brief Get Media List (Audio / Video) play info
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U16 ItemId : id of item
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @param in T_eMEDIA_PLAYINFO_TYPE infoType : play count / play time / append time
* @return T_U32 play info
* @retval 
*/
T_U32 MList_GetPlayInfo(T_U16 ItemId, T_eMEDIA_LIST_TYPE type, T_eMEDIA_PLAYINFO_TYPE infoType);

/**
* @brief Update Media List (Audio / Video) play info
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U16 ItemId : id of item
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL MList_UpdatePlayInfo(T_U16 ItemId, T_eMEDIA_LIST_TYPE type);


/**
* @brief Clean Media List (Audio / Video) play info
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U16 ItemId : id of item
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL MList_CleanPlayInfo(T_U16 ItemId, T_eMEDIA_LIST_TYPE type, T_eMEDIA_PLAYINFO_TYPE infoType);


#endif	//_SVC_MEDIALIST_H_

