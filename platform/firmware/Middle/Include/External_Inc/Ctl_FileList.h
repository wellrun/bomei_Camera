
/**
 * @filename Ctl_FileList.h
 * @brief   FileList definition and function prototype
 *
 * This file declare Anyka FileList Module interfaces.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Li Zhuobin
 * @date    2006-07-15
 * @version 1.0
 * @ref
 */

#ifndef __CTL_FILELIST_H__
#define __CTL_FILELIST_H__

#include "anyka_types.h"
#include "gbl_macrodef.h"
#include "ctl_scrollbar.h"
#include "Ctl_IconExplorer.h"
#include "svc_medialist.h"

/** @{@name Define the FileList refresh flag */
/** Use this flag to set FileList Module to refresh all */
#define FILELIST_REFRESH_ALL            0xffffffff
/** Use this flag to set FileList Module to not refresh */
#define FILELIST_REFRESH_NONE           0x00000000
/** Use this flag to set FileList Module to refresh item */
#define FILELIST_REFRESH_ITEM           0x00000001
/** Use this flag to set FileList Module to refresh scroll bar */
#define FILELIST_REFRESH_SCBAR          0x00000002
/** Use this flag to set FileList Module to refresh focus item */
#define FILELIST_REFRESH_FOCUS          0x00000004
/** Use this flag to set FileList Module to refresh focus title */
#define FILELIST_REFRESH_TITLE          0x00000008
/** @} */

/** @{@name Define the FileList module default display attribute */
/** Use this flag to set FileList Module default frame width equal 2 */
#define FILELIST_FRAMEWIDTH             2
/** Use this flag to set FileList Module default scroll bar width equal 8 */
#define FILELIST_SCROLLBAR_WIDTH        12
/** Use this flag to set FileList Module default item interval equal 1 */
#define FILELIST_ITEM_INTERVAL          1
/** Use this flag to set FileList Module default item height equal font height */
#define FILELIST_ITEM_HEIGHT            (T_U32)(g_Font.CHEIGHT)
/** Use this flag to set FileList Module default back ground color to white color */
#define FILELIST_BACKCOLOR              COLOR_WHITE
/** Use this flag to set FileList Module default back text color to blue color */
#define FILELIST_TEXTCOLOR              COLOR_BLACK
/** Use this flag to set FileList Module default focus back ground color to blue color */
#define FILELIST_FOCUSBACKCOLOR         COLOR_BLUE
/** Use this flag to set FileList Module default focus text color to white color */
#define FILELIST_FOCUSTEXTCOLOR         COLOR_WHITE
/** @} */

#define SAVELIST_MINIMAL_SPACE     50*1024

/** FileList sort item mode */
typedef enum {
    FILELIST_SORT_NONE = 0,             /**< not sort the item */
    FILELIST_SORT_ID,                   /**< sort the item by id */
    FILELIST_SORT_NAME,                 /**< sort the item by file name */
    FILELIST_SORT_PATH,                 /**< sort the item by file path */
    FILELIST_SORT_REVERSE,              /**< reverse the item */
    FILELIST_SORT_RANDOM,               /**< sort the item by random */
    FILELIST_SORT_USER_DEFINE,          /**< sort the item by user define */
    FILELIST_SORTMODE_NUM               /**< quantity of sort item mode */
} T_FILELIST_SORTMODE;

/** FileList fetch item mode */
typedef enum {
    FILELIST_FETCH_SEQUENCE = 0,        /**< fetch item from the FileList module by sequence */
    FILELIST_FETCH_REPEAT_SINGLE,       /**< fetch item from the FileList module by repeat single */
    FILELIST_FETCH_RANDOM,              /**< fetch item from the FileList module by random */
    FILELIST_FETCH_REPEAT,              /**< fetch item from the FileList module by repeat all */
    FILELIST_FETCHMODE_NUM              /**< quantity of fetch item mode */
} T_FILELIST_FETCHMODE;

/** FileList move direction */
typedef enum {
    FILELIST_MOVE_DOWN = 0,             /**< move the focus down */
    FILELIST_MOVE_UP,                   /**< move the focus up */
    FILELIST_MOVE_DIRECTION_NUM         /**< quantity of move focus direction */
} T_FILELIST_DIRECTION;

/** FileList move focus return value */
typedef enum {
    FILELIST_MOVEFOCUS_OVERHEAD,        /**< move the focus over the filelist head */
    FILELIST_MOVEFOCUS_OVERTAIL,        /**< move the focus over the filelist tail */
    FILELIST_MOVEFOCUS_OK,              /**< move the focus success */
    FILELIST_MOVEFOCUS_ERROR,           /**< move the focus error */
    FILELIST_MOVEFOCUS_RET_NUM          /**< quantity of move focus return value */
} T_FILELIST_MOVEFOCUS_RET;

/** FileList search sub Folder mode */
typedef enum {
    FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER = 0,    /**< no search files which is in sub folder and no record the sub folder name when add folder to filelist */
    FILELIST_NO_SEARCH_SUB_RECODE_FOLDER,           /**< no search files which is in sub folder but record the sub folder name when add folder to filelist */
    FILELIST_SEARCH_SUB_NO_RECODE_FOLDER,           /**< search files which is in sub folder but no record the sub folder name when add folder to filelist */
    FILELIST_SEARCH_SUB_RECODE_FOLDER,              /**< search files which is in sub folder and record the sub folder name when add folder to filelist */
    T_FILELIST_SEARCH_SUB_MODE_NUM                 /**< filelist search sub folder mode num */
} T_FILELIST_SEARCH_SUB_MODE;

/** FileList add audio return value */
typedef enum {
    FILELIST_ADD_NONE,                             /**< add none */
    FILELIST_ADD_ERROR,                             /**< add audio error */
    FILELIST_ADD_SUCCESS,                           /**< add audio success */
    FILELIST_ADD_NOSPACE,                           /**< have no memory, can not add audio */
    FILELIST_ADD_OUTPATHDEEP,						/**< Out of Directory Deep, Protect Stack Memory */
    FILELIST_ADD_RET_NUM                            /**< add audio return value num */
} T_FILELIST_ADD_RET;

/** Define FileList item structure */
typedef struct _FILELIST_ITEM {
    T_U32                   ID;                     /**< item id */
    T_pWSTR                  pFilePath;              /**< item file path */
    T_pWSTR                  pText;                  /**< item display content */
    struct _FILELIST_ITEM   *pPrevious;             /**< previous item pointer */
    struct _FILELIST_ITEM   *pNext;                 /**< next item pointer */
} T_FILELIST_ITEM;

/** @{@name Define the callback function type */
/** Callback function for sort compare */
typedef T_BOOL (*T_fFILELIST_COMPARE_CALLBACK)(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2);

/** Callback function for judge the file is support or not */
typedef T_BOOL (*T_fFILELIST_SUPPORT_CALLBACK)(T_pCWSTR pFilePath);

/** @} */

/** Define FileList sort method structure */
typedef struct {
    T_FILELIST_SORTMODE                 SortMode;               /**< item sort mode */
    T_fFILELIST_COMPARE_CALLBACK        SortCompareCallBack;    /**< item sort compare call back function */
} T_FILELIST_SORTMETHOD;

/** Define FileList structure */
typedef struct {
    T_FILELIST_ITEM                     *pItemHead;             /**< item head point */
    T_FILELIST_ITEM                     *pItemShow;             /**< item show first point */
    T_FILELIST_ITEM                     *pItemTail;             /**< item tail point */
    T_FILELIST_ITEM                     *pItemFocus;            /**< item focus point */
    T_FILELIST_ITEM                     *pItemOldFocus;         /**< item old focus point */

    T_FILELIST_FETCHMODE                FetchMode;              /**< item fetch mode */

    T_U32                               ItemQty;                /**< items quantity */
    T_U32                               ItemQtyMax;             /**< items quantity max, 0 is no limit */
    T_U32                               PageItemQty;            /**< one page item quantity */
    T_U32                               FrameWidth;             /**< frame width */
    T_U32                               ItemHeight;             /**< item height */
    T_U32                               ItemVInterval;          /**< item interval */
    T_SCBAR                             ScrollBar;              /**< scroll bar */
    T_U32                               ScrollBarWidth;         /**< scroll bar width */
    T_BOOL                              ScBarShowFlag;          /**< show scroll bar flag */
    T_BOOL                              TitleShowFlag;          /**< show title flag */

    T_RECT                              FileListRect;           /**< file list rect */
    T_RECT                              TitleRect;              /**< title rect */
    T_RECT                              ItemRect;               /**< file list item rect */
    T_COLOR                             ItemBackColor;          /**< item back color */
    T_COLOR                             TextColor;              /**< item text color */
    T_COLOR                             FocusBackColor;         /**< item focus back color */
    T_COLOR                             FocusTextColor;         /**< item focus text color */
    T_U32                               ItemTextOffset;         /**< item text scroll offset */
    T_U32                               TitleTextOffset;        /**< title text scroll offset */

    T_U32                               RefreshFlag;            /**< refresh flag */
    T_BOOL                              ListFlag;               /**< item list flag */
    T_BOOL                              ChangeFlag;
    T_U8                                *pTitleBackData;        /**< title back ground data */
    T_pCDATA                            pFocusBckgrnd;          /**< focus back ground data */

    T_FILELIST_SORTMETHOD               SortMethod;             /**< item sort method struct */
    T_fFILELIST_SUPPORT_CALLBACK        SupportCallBack;        /**< the pointer to the support type call back function */
} T_FILELIST;

/**
 * @brief   init the FileList module.
 *
 * Should be called before use FileList module.
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] FileListRect FileList rect
 * @param   [in] SortMode input a system define sortmode
 * @param   [in] ItemQtyMax FileList module can store max item quantity
 * @param   [in] TitleShowFlag show title or not
 * @param   [in] SupportTypeCallBack support type call back function
 * @return  T_BOOL
 * @retval  AK_TRUE Successful
 * @retval  AK_FALSE FileList init error
 */
T_BOOL   FileList_Init(T_FILELIST *pFileList, T_U32 ItemQtyMax, T_FILELIST_SORTMODE SortMode,  T_fFILELIST_SUPPORT_CALLBACK SupportCallBack);

/**
 * @brief   free the FileList module.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE Successful
 * @retval  AK_FALSE FileList free error
 */
T_BOOL FileList_Free(T_FILELIST *pFileList);


/**
 * @brief   FileList module add item.
 *
 * Add a support file, Add support files from a folder and Add support files from a list.
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] ImportPath The file path of the add file
 * @param   [in] SearchSubFolder search sub folder or not, use in add a folder
 * @return  T_BOOL
 * @retval  AK_TRUE Successful
 * @retval  AK_FALSE FileList add error
 */
T_FILELIST_ADD_RET FileList_Add(T_FILELIST *pFileList, T_USTR_FILE ImportPath, T_FILELIST_SEARCH_SUB_MODE SearchSubMode);

T_FILELIST_ADD_RET FileList_Add_Alt(T_FILELIST *pFileList, T_USTR_FILE ImportPath, T_FILELIST_SEARCH_SUB_MODE SearchSubMode);

T_BOOL FileList_ToIconExplorer(T_FILELIST *pFileList, T_ICONEXPLORER *pIconExplorer, T_eINDEX_TYPE type, T_USTR_FILE path);

//T_BOOL FileList_FromIconExplorer(T_FILELIST *pFileList, T_ICONEXPLORER *pIconExplorer);
T_FILELIST_ADD_RET FileList_FromIconExplorer(T_FILELIST *pFileList, T_ICONEXPLORER *pIconExplorer);


/**
 * @brief   save the FileList items to a list file.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] FilePath list file path
 * @return  T_BOOL
 * @retval  AK_TRUE save successful
 * @retval  AK_FALSE save error
 */
T_BOOL FileList_SaveFileList(T_FILELIST *pFileList, T_pCWSTR FilePath);


#if 0


/**
 * @brief   FileList module get fetch mode.
 *
 * Get the fetch mode of FileList module, seauence, repeat all, repeat single and random for example.
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_FILELIST_FETCHMODE
 * @retval  Successful return fetch mode, such as seauence, repeat all, repeat single and random.
 * @retval  error return FILELIST_FETCHMODE_NUM
 */
T_FILELIST_FETCHMODE FileList_GetFetchMode(T_FILELIST *pFileList);

/**
 * @brief   fetch id item next item, return the file path of the item.
 *
 * input the id, find the item which the id point, fetch next item, return the file path.
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] item id
 * @return  T_pSTR
 * @retval  Successful return id next item file path
 * @retval  can not find the item or fail return AK_NULL
 */
T_pWSTR FileList_FetchNextFilePath(T_FILELIST *pFileList, T_U32 Id);

/**
 * @brief   Ask file list have content or not.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE file list have content
 * @retval  AK_FALSE file list don't have content
 */
T_BOOL FileList_IsListHaveContent(T_FILELIST *pFileList);

/**
 * @brief   fetch the focus's file path from the FileList module.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_pSTR
 * @retval  The pointer of the focus's file path
 * @retval  AK_NULL fectch error
 */
T_pWSTR FileList_GetFocusFilePath(T_FILELIST *pFileList);


/**
 * @brief   delete the focus item from FileList module.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE delete focus successful
 * @retval  AK_FALSE delete focus error
 */
T_BOOL FileList_DelFocus(T_FILELIST *pFileList);

/**
 * @brief   delete the item by the id from FileList module.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] Id the delete item id
 * @return  T_BOOL
 * @retval  AK_TRUE delete the item by the id successful
 * @retval  AK_FALSE delete the item by the id error
 */
T_BOOL FileList_DelById(T_FILELIST *pFileList, T_U32 Id);


/**
 * @brief   FileList module move focus.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] Direction move focus direction, up or down
 * @return  T_FILELIST_MOVEFOCUS_RET
 * @retval  FILELIST_MOVEFOCUS_OVERHEAD move the focus over the filelist head
 * @retval  FILELIST_MOVEFOCUS_OVERTAIL move the focus over the filelist tail
 * @retval  FILELIST_MOVEFOCUS_OK       move the focus success
 * @retval  FILELIST_MOVEFOCUS_ERROR    move the focus error
 */
T_FILELIST_MOVEFOCUS_RET FileList_MoveFocus(T_FILELIST *pFileList, T_FILELIST_DIRECTION Direction);


/**
 * @brief   set FileList module rect.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] FileListRect FileList module rect
 * @param   [in] TitleShowFlag title to be shown or not
 * @return  T_BOOL
 * @retval  AK_TRUE set FileList module rect successful
 * @retval  AK_FALSE set FileList module rect error
 */
T_BOOL FileList_SetRect(T_FILELIST *pFileList, T_RECT FileListRect, T_BOOL TitleShowFlag);


/**
 * @brief   show the FileList module.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE Successful
 * @retval  AK_FALSE FileList show error
 */
T_BOOL FileList_Show(T_FILELIST *pFileList);

/**
 * @brief   FileList module handle function.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] Event input Event
 * @param   [in] pParam Event parm pointer
 * @return  T_eBACK_STATE
 * @retval  eStay   stay FileList, user do no any operation
 * @retval  eReturn user cancel the FileList
 * @retval  eHome   return to home
 * @retval  eNext   user selected a FileList item
 * @retval  eMenu   go to FileList menu
 */
T_eBACK_STATE FileList_Handler(T_FILELIST *pFileList, T_EVT_CODE Event, T_EVT_PARAM *pParam);



/**
 * @brief   Get FileList module refresh flag.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_U32
 * @retval  FileList module refresh flag
 */
T_U32 FileList_GetRefresh(T_FILELIST *pFileList);

/**
 * @brief   Get FileList module sort mode.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_FILELIST_SORTMODE
 * @retval  FileList module sort mode
 */
T_FILELIST_SORTMODE FileList_GetSortMode(T_FILELIST *pFileList);

/**
 * @brief   Get focus item id.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_U32
 * @retval  focus item id
 */
T_U32 FileList_GetFocusId(T_FILELIST *pFileList);

/**
 * @brief   Get show first item id.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_U32
 * @retval  show first item id
 */
T_U32 FileList_GetShowFirstId(T_FILELIST *pFileList);



/**
 * @brief   set sort method to user define, pass the sort item function pointer to FileList.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] CompareCallBack user define sort call back function pointer
 * @return  T_BOOL
 * @retval  AK_TRUE set sort call back function successful
 * @retval  AK_FALSE set sort call back function error
 */
T_BOOL FileList_SetSortCallBack(T_FILELIST *pFileList, T_fFILELIST_COMPARE_CALLBACK CompareCallBack);


/**
 * @brief   Set Focus to FileList head.
 *
 * @author  lizhuobin
 * @date    2006-08-23
 * @param   [in] pFileList FileList struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE set focus to head successful
 * @retval  AK_FALSE set focus to head error
 */
T_BOOL FileList_SetFocusToHead(T_FILELIST *pFileList);

/**
 * @brief   Get focus item text.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_pCSTR
 * @retval  focus item text
 */
T_pCWSTR FileList_GetFocusText(T_FILELIST *pFileList);

/**
 * @brief   Get item text by id.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] id item id
 * @return  T_pCSTR
 * @retval  item text
 */
T_pCWSTR FileList_GetIdText(T_FILELIST *pFileList, T_U32 Id);

/**
 * @brief   set focus by id.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] id item id
 * @return  T_BOOL
 * @retval  AK_TRUE successful
 * @retval  AK_FALSE fail or can not find the item
 */
T_BOOL FileList_SetFocusById(T_FILELIST *pFileList, T_U32 Id);

/**
 * @brief   del the item from the file list by the folder path.
 *
 * if the item in the folder, it will del from the list
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] pFolderPath folder path
 * @return  T_BOOL
 * @retval  AK_TRUE successful
 * @retval  AK_FALSE error
 */
T_BOOL FileList_DelByFolder(T_FILELIST *pFileList, T_pCWSTR pFolderPath);

/**
 * @brief   Get FileList Change flag.
 *
 * @author  lizhuobin
 * @date    2006-08-23
 * @param   [in] pFileList FileList struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE the filelist has been changed and the user should save the filelist at exit audioplayer
 * @retval  AK_FALSE the filelist is not changed, The filelist is not need to save at exit audioplayer
 */
T_BOOL FileList_GetChangeFlag(T_FILELIST *pFileList);

#endif

#endif


