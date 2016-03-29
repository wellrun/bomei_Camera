
/**
 * @filename Eng_FileManage.h
 * @brief   structure and functions definition,for File Manage Module
 *
 * This file declare Anyka File Manage Module interfaces.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Liu Weijun
 * @date    2006-08-17
 * @version 1.0
 * @ref
 */

#ifndef __ENG_FILE_MANAGE_H__
#define __ENG_FILE_MANAGE_H__


#include "anyka_types.h"
#include "Gbl_Global.h"

#define DISKSPACE_RESERVE   20000 
//define a variable : (MAX_FILE_LEN - FILE_LEN_RESERVE1) is the max length of all kinds of  default path .
#define FILE_LEN_RESERVE1   32

#define FILE_LEN_RESERVE2   12

/** define a enum, the current state management */
typedef enum {
    FILE_NULL = 0,                          /**< no src file be copyed/cut */
    FILE_COPY,                              /**< src file is copyed */
    FILE_CUT,                               /**< src file is cut */
    //FILE_DELETE,                          
    FILE_PASTE,                             /**< src file is pasting */
    FILE_MANAGE_STATE_NUM
} T_MANAGE_STATE;


/** define a structure, for saving file info while file manage */
typedef struct {
    T_MANAGE_STATE  Manage_State;           /**< current mangge state,for example: COPY */
    T_USTR_FILE     SrcPath;                /**< full path of the source file */
    T_USTR_FILE     DestPath;               /**< full path of the dest file */
    T_USTR_FILE     FileName;               /**< file name */
    T_USTR_FILE     DeletePath;             /**< path of the folder that will be deleted*/
    T_BOOL          CutFlag;                /**< true--cut;false--copy */
    T_BOOL          FolderFlag;             /**< data file or folder */
}T_FILE_MANAGE_PARM;


/** define a enum, list the Prev_Paste return value */
typedef enum {
    PASTE_BOARD_NULL = 0,                   /**< no src file be copyed/cut */
    PASTE_FOLDER,                           /**< it's all right,and the src file is a folder */
    PASTE_FILE,                             /**< it's all right,and the src file is a data file */
    MOVE_FILE,                              /**< cut,and the src file and the dest file is in the same driver */
    LACK_DISKSPACE,                         /**< no enough disk space */
    FILE_SAME_NAME,                         /**< same name file is exist in the dest folder */
    FILE_SAME_ADDR,                         /**< src file and dest file is in the same folder */
    FILE_ERROR,                             /**< src file is not exist */
    FILE_BUSY,                              /**< src file is playing */
    FILE_LONG_NAME                          /**< file name is too long*/
} T_FILE_MANAGE_RETURN_VALUE;



/**
 * @brief   initialize the T_FILE_MANAGE_PARM object.
 * 
 * @author  liuweijun
 * @date    2006-08-17
 * @param   [in] T_VOID, because the object is define in Eng Layer.
 * @return  T_VOID
 */
T_VOID FileMgr_InitFileInfo(T_VOID);
 
 
/**
 * @brief   save the source file information,for example,FilePath.
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   [in] srcpath the whole path of the source file
 * @param   [in] cutflag the source file is copyed or cut 
 * @return  T_BOOL
 * @retval  AK_TRUE save successful
 * @retval  AK_FALSE error,while saving
 */
T_BOOL FileMgr_SaveSrcFileInfo(T_pCWSTR srcpath, T_BOOL cutflag);


/**
 * @brief   save the dest file information
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   [in] DestPath  the whole path of the source file
 * @return  T_VOID
 * @retval  
 */
T_VOID FileMgr_SaveDestFileInfo(T_pCWSTR DestPath);


/**
 * @brief   a interface to store the source file Path
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   [in] srcpath path to be stored
 * @return  T_VOID
 * @retval
 */
T_VOID FileMgr_SetSrcPath(T_pWSTR srcpath);


/**
 * @brief   a interface giving the source file Path
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   [out] srcpath store the return path
 * @return  T_VOID
 * @retval
 */
T_VOID FileMgr_GetSrcPath(T_pWSTR srcpath);


/**
 * @brief   a interface to store the dest file Path
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   [in] destpath path to be stored 
 * @return  T_VOID
 * @retval
 */
T_VOID FileMgr_SetDestPath(T_pWSTR destpath);


/**
 * @brief   a interface giving the dest file Path
 * 
 * @author  Liuweijun       
 * @date    2006-09-4
 * @param   [out] destpath store the delete file path
 * @return  T_VOID
 * @retval
 */
T_VOID FileMgr_GetDeletePath(T_pWSTR delpath);


/**
 * @brief   a interface to store the dest file Path
 * 
 * @author  Liuweijun       
 * @date    2006-09-4
 * @param   [in] delpath path to be deleted 
 * @return  T_VOID
 * @retval
 */
T_VOID FileMgr_SetDeletetPath(T_pWSTR delpath);


/**
 * @brief   a interface giving the dest file Path
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   [out] destpath store the return path
 * @return  T_VOID
 * @retval
 */
T_VOID FileMgr_GetDestPath(T_pWSTR destpath);


/**
 * @brief   a interface giving the source file name
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   [out] filename  store the return filename
 * @return  T_VOID
 * @retval
 */
T_VOID FileMgr_GetFileName(T_pWSTR filename);
 

/**
 * @brief   a interface giving the ManageState
 * 
 * @author  Liuweijun       
 * @date    2006-09-4
 * @param   T_VOID
 * @return  T_MANAGE_STATE
 * @retval  
 */
T_MANAGE_STATE FileMgr_GetManageState(T_VOID);
 

/**
 * @brief   a interface to set  the ManageState
 * 
 * @author  Liuweijun       
 * @date    2006-10-23
 * @param   T_MANAGE_STATE
 * @return  T_BOOL
 * @retval  
 */
T_BOOL FileMgr_SetManageState(T_MANAGE_STATE mgr_state);


/**
 * @brief   a interface giving the cut flag
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  source file is CUT
 * @retval  AK_FALSE source file is copyed
 */
T_BOOL FileMgr_GetCutFlag(T_VOID);


/**
 * @brief   a interface giving the folder flag
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  source file is a folder
 * @retval  AK_FALSE source file is a single file
 */
T_BOOL FileMgr_GetFolderFlag(T_VOID);




/**
 * @brief   get id of a driver 
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   [in]path file path
 * @return  T_BOOL
 * @retval  driver ID
 */
T_U16 FileMgr_GetDriverId(T_pCWSTR path);


/**
 * @brief   detect if the disk space is enough for the new file
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   [in]srcpath  
 * @param   [in]destpath 
 * @return  T_BOOL
 * @retval  AK_TRUE disk space is enough
 * @retval  AK_FALSE disk space is not enough
 */
T_BOOL FileMgr_DetectDiskSpaceIsEnough(T_pCWSTR srcpath,T_pCWSTR destpath);


/**
 * @brief   prev handle before starting paste a file/folder 
 * 
 * @author  Liuweijun       
 * @date    2006-08-17
 * @param   [in]srcpath
 * @param   [in]destpath
 * @return  T_FILE_MANAGE_RETURN_VALUE
 * @retval  
 */
T_FILE_MANAGE_RETURN_VALUE FileMgr_Paste_Pre_Handle(T_pCWSTR srcpath,T_pCWSTR destpath);



/**
 * @brief   check if a file is exist   
 * 
 * @author  Liuweijun
 * @date    2006-08-17
 * @param   [in]filepath  full path of the file
 * @return  T_BOOL
 * @retval  AK_TRUE  file exist
 * @retval  AK_FALSE file not exist
 */
T_BOOL FileMgr_CheckFileIsExist(T_pCWSTR filepath);



/**
 * @brief   get a part of a filepath 
 * 
 * @author  Liuweijun
 * @date    2006-08-17
 * @param   [in]rootpath  the source file
 * @param   [in]filepath the dest file
 * @param   [out]subpath the returned subpath
 * @return  T_U8 * 
 * @retval  AK_TRUE  the part of the filepath
 * @retval  AK_FALSE 
 */
T_BOOL FileMgr_GetSubPath(T_pCWSTR rootpath,T_pCWSTR filepath,T_pWSTR subpath);


/**
 * @brief   change every char of a string to upper;  
 * 
 * @author  Liuweijun
 * @date    2006-09-01
 * @param   [in]filepath  the filepath
 * @return  T_U8 * 
 * @retval  AK_TRUE  the upper string 
 * @retval  AK_FALSE 
 */

//T_pCSTR FileMgr_StrToUp(T_pCSTR string);

#endif

