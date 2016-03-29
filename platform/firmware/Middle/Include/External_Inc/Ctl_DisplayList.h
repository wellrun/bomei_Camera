
#ifndef __ENG_DISPLAY_LIST_H__
#define __ENG_DISPLAY_LIST_H__

#include "anyka_types.h"
#include "Eng_String.h"
#include "Ctl_IconExplorer.h"
#include "Fwl_osFS.h"

#define DISPLAYLIST_REFRESH_ALL         ICONEXPLORER_REFRESH_ALL            // refresh all
#define DISPLAYLIST_REFRESH_NONE        ICONEXPLORER_REFRESH_NONE               // no refresh
#define DISPLAYLIST_REFRESH_TITLE       ICONEXPLORER_REFRESH_TITLE                      // refresh title
#define DISPLAYLIST_REFRESH_ITEM        ICONEXPLORER_REFRESH_ITEM                           // refresh item
#define DISPLAYLIST_REFRESH_FOCUS       ICONEXPLORER_REFRESH_FOCUS                      // refresh focus

#define T_DISPLAYLIST_ICONSTYLE         T_ICONEXPLORER_ICONSTYLE
#define DISPLAYLIST_SMALLICON           ICONEXPLORER_SMALLICON
#define DISPLAYLIST_LARGEICON           ICONEXPLORER_LARGEICON
#define DISPLAYLIST_NONEICON            ICONEXPLORER_NONEICON
#define DISPLAYLIST_ICONSTYLE_NUM       ICONEXPLORER_ICONSTYLE_NUM

#define T_DISPLAYLIST_SORTMODE          T_ICONEXPLORER_SORTMODE
#define DISPLAYLIST_SORT_ID             ICONEXPLORER_SORT_ID
#define DISPLAYLIST_SORT_CONTENT        ICONEXPLORER_SORT_CONTENT
#define DISPLAYLIST_SORT_NUM            ICONEXPLORER_SORT_NUM    

#define T_DISPLAYLIST_DIRECTION         T_ICONEXPLORER_DIRECTION
#define DISPLAYLIST_DIRECTION_LEFT      ICONEXPLORER_DIRECTION_LEFT
#define DISPLAYLIST_DIRECTION_RIGHT     ICONEXPLORER_DIRECTION_RIGHT
#define DISPLAYLIST_DIRECTION_UP        ICONEXPLORER_DIRECTION_UP
#define DISPLAYLIST_DIRECTION_DOWN      ICONEXPLORER_DIRECTION_DOWN
#define DISPLAYLIST_DIRECTION_NUM       ICONEXPLORER_DIRECTION_NUM


typedef enum {
    DISLIST_FILE_TYPE_FALSE = 0,
    DISLIST_FILE_TYPE_FOLDER,
    DISLIST_FILE_TYPE_FILE,
    DISLIST_FILE_TYPE_NUM
} T_DISPLAYLIST_FILE_TYPE;

typedef enum {
    ICON_BMP = 0,
    ICON_JPG,
    ICON_PNG,
    ICON_GIF, 
    
    ICON_AVI,
    ICON_AKV,   
    ICON_3GP,
    ICON_MP4,
    ICON_FLV,
    ICON_RM,
    ICON_MKV,
    
    ICON_MP3,
    ICON_MID,
    ICON_WAV,
    ICON_AMR,
    ICON_WMA,
    ICON_MPEG,
    ICON_AAC, 
    ICON_AC3,
    ICON_ADIF,
    ICON_ADTS,
    ICON_M4A,
    ICON_FLAC,
    ICON_OGG,
    ICON_OGA,
    ICON_APE,
    
    ICON_LRC,
    ICON_TXT,
    ICON_DOC,
    ICON_PDF,
    ICON_XLS,
    ICON_MAP,
    
    ICON_NES,
    ICON_SNES,
    ICON_GBA,
    ICON_MD,
    ICON_ALT,
    ICON_SAV,
    
    ICON_MFS,
    ICON_TTF,
    ICON_TTC,
    ICON_OTF,
    ICON_SWF,
    ICON_UNKNOWN,
    ICON_FOLDER0,
    ICON_FOLDER1,
    ICON_UP,

    ICON_MAX_ID
}T_EXPLORER_ICON_ID;

typedef struct {
    T_EXPLORER_ICON_ID  IconId;
    T_BOOL              IconShowEnable;
}T_DISPLAYLIST_ICON_DIS;



typedef enum {
    DISPLAYLIST_NOT_DISPOSTFIX = 0,
    DISPLAYLIST_DISPOSTFIX,
    DISPLAYLIST_DISPOSTFIX_NUM
} T_DISPLAYLIST_DIS_POSTFIX;

typedef enum {
    DISPLAYLIST_NOT_DISPATH = 0,
    DISPLAYLIST_DISPATH,
    DISPLAYLIST_DISPATH_NUM
} T_DISPLAYLIST_DIS_PATH;

typedef enum{
    DISPLAYLIST_NOT_DISPFOLDER,
    DISPLAYLIST_DISPFOLDER,
    DISPLAYLIST_DISPFOLDER_NUM
}T_DISPLAYLIST_DISP_FOLDER;

typedef enum{
    DISPLAYLIST_NOT_FIND_SUBFOLDER,
    DISPLAYLIST_FIND_SUBFOLDER,
    DISPLAYLIST_FIND_SUBFOLDER_NUM
}T_DISPLAYLIST_FIND_SUBFOLDER;

typedef struct {
    T_ICONEXPLORER                  IconExplorer;
    T_BOOL                          exitFolderFlag;
    T_USTR_FILE                     exitFolderName;
    T_pCDATA                        pIcon[ICON_MAX_ID];
    T_pCDATA                        pBIcon[ICON_MAX_ID];
                                    
    T_pCDATA                        pBkImg;
    T_pCDATA                        pTitle;
    T_RECT                          titleRect;          
    T_RECT                          itemRect;
                                    
    T_S32                           subLevel;            /* if in the sub folder level of /ebook/ */
    T_USTR_FILE                     curPath;
    T_DISPLAYLIST_ICON_DIS          DisplayListIconTbl[FILE_TYPE_NUM];
    
    T_DISPLAYLIST_DIS_POSTFIX       DisPostfix;
    T_DISPLAYLIST_DIS_PATH          DisPath;
    T_DISPLAYLIST_DISP_FOLDER       DispFolder;
	T_DISPLAYLIST_FIND_SUBFOLDER	find_subfolder;
	T_BOOL							bPathTooDeep;
	T_BOOL							bNameTooLong;
} T_DISPLAYLIST;

// because Fwl_FsFindFirst will use 0 to 2.5M memory(0-1024 files, every file use 2.5k memory)
// in 8m memory, the memory will cut to two block
// for that, we will malloc some DISPLAYLIST_TMP_ITEM to store the file info
// then, free the memory witch Fwl_FsFindFirst malloc
// move the DISPLAYLIST_TMP_ITEM to icon_explor item
// then free the DISPLAYLIST_TMP_ITEM, that this problem will be solve
typedef struct _DISPLAYLIST_TMP_ITEM {
    T_U32                           Id;                     // item id
    T_EXPLORER_ICON_ID              IconId;
    T_FILE_INFO                     Content;                // item content point, malloc it
    struct _DISPLAYLIST_TMP_ITEM    *pPrevious;             // previous item point
    struct _DISPLAYLIST_TMP_ITEM    *pNext;                 // next item point
} T_DISPLAYLIST_TMP_ITEM;

typedef struct {
    T_DISPLAYLIST_TMP_ITEM                      *pItemHead;             /**< item head point */
    T_DISPLAYLIST_TMP_ITEM                      *pItemTail;             /**< item tail point */ 
    T_U32                                       ItemQty;
} T_DISPLAYLIST_TMP_LIST;

T_BOOL DisplayList_init(T_DISPLAYLIST *pDisplayList, T_U16 *pCurPath, \
                        const T_U16 *pTitleText, T_FILE_TYPE *FileType);
T_BOOL DisplayList_Free(T_DISPLAYLIST *pDisplayList);
T_BOOL DisplayList_Show(T_DISPLAYLIST *pDisplayList);
T_eBACK_STATE DisplayList_Handler(T_DISPLAYLIST *pDisplayList, T_EVT_CODE Event, T_EVT_PARAM *pParam);

T_BOOL DisplayList_CheckItemList(T_DISPLAYLIST *pDisplayList);

T_FILE_INFO *DisplayList_Operate(T_DISPLAYLIST *pDisplayList);

T_BOOL DisplayList_SetRefresh(T_DISPLAYLIST *pDisplayList, T_U32 RefreshFlag);
T_U32  DisplayList_GetRefresh(T_DISPLAYLIST *pDisplayList);


T_BOOL DisplayList_ListRefresh(T_DISPLAYLIST *pDisplayList);

T_VOID *DisplayList_GetItemContentFocus(T_DISPLAYLIST *pDisplayList);
T_VOID DisplayList_SetTopBarMenuIconState(T_DISPLAYLIST *pDisplayList);

T_DISPLAYLIST_ICONSTYLE DisplayList_GetItemIconStyle(T_DISPLAYLIST *pDisplayList);
T_BOOL DisplayList_SetItemIconStyle(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_ICONSTYLE ItemIconStyle);

T_BOOL DisplayList_SetSortMode(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_SORTMODE ItemSortMode);
T_DISPLAYLIST_SORTMODE DisplayList_GetSortMode(T_DISPLAYLIST *pDisplayList);

T_BOOL DisplayList_SetListFlag(T_DISPLAYLIST *pDisplayList);

T_BOOL DisplayList_DelFocusItem(T_DISPLAYLIST *pDisplayList);
T_BOOL DisplayList_MoveFocus(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_DIRECTION Direction);

T_S32 DisplayList_GetSubLevel(T_DISPLAYLIST *pDisplayList);

T_BOOL DisplayList_SetCurFilePath(T_DISPLAYLIST *pDisplayList, T_U16 *path);
T_U16 *DisplayList_GetCurFilePath(T_DISPLAYLIST *pDisplayList);

T_BOOL DisplayList_ExitDirectory(T_DISPLAYLIST *pDisplayList);
T_BOOL DisplayList_EnterDirectory(T_DISPLAYLIST *pDisplayList);

T_BOOL DisplayList_SetDisPostfix(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_DIS_POSTFIX DisPostfix);
T_DISPLAYLIST_DIS_POSTFIX DisplayList_GetDisPostfix(T_DISPLAYLIST *pDisplayList);

T_BOOL DisplayList_SetDisPath(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_DIS_PATH DisPath);
T_DISPLAYLIST_DIS_PATH DisplayList_GetDisPath(T_DISPLAYLIST *pDisplayList);

T_BOOL DisplayList_SetDispFolder(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_DISP_FOLDER DispFolder);
T_DISPLAYLIST_DISP_FOLDER DisplayList_GetDispFolder(T_DISPLAYLIST *pDisplayList);

T_BOOL DisplayList_SetFindSubFolder(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_FIND_SUBFOLDER findsubFolder);
T_DISPLAYLIST_DISP_FOLDER DisplayList_GetFindSubFolder(T_DISPLAYLIST *pDisplayList);

#endif


