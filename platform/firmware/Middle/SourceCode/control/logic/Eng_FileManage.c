
#include "Gbl_Global.h"
#include "Fwl_public.h"
#include "Ctl_FileList.h"
#include "Eng_FileManage.h"
#include "Fwl_pfKeypad.h"
#include "Fwl_osFS.h"
#include "Ctl_AudioPlayer.h"
#include "Eng_String_UC.h"
#include "Eng_DataConvert.h"
#include "eng_debug.h"
#include "file.h"

//#define FILE_MGR_DBG 
/** define a static variable to store the pasting info */
static T_FILE_MANAGE_PARM File_Manage_Parm = {FILE_NULL};

T_VOID FileMgr_InitFileInfo(T_VOID)
{
    File_Manage_Parm.SrcPath[0] = 0;
    File_Manage_Parm.DestPath[0] = 0;
    File_Manage_Parm.FileName[0] = 0;
    File_Manage_Parm.DeletePath[0] = 0;
    File_Manage_Parm.CutFlag = AK_FALSE;
    File_Manage_Parm.FolderFlag = AK_FALSE; 
}


T_BOOL FileMgr_SaveSrcFileInfo(T_pCWSTR src_path, T_BOOL cutflag)
{
    T_USTR_FILE srcpath,path,name;
    T_U16       UStr_Tmp[MAX_FILENM_LEN + 1];

    if (cutflag)
        File_Manage_Parm.Manage_State = FILE_CUT;
    else
        File_Manage_Parm.Manage_State = FILE_COPY;

    if (src_path == AK_NULL || (Utl_UStrLen(src_path) > MAX_FILENM_LEN))
        return AK_FALSE;

    Utl_UStrCpy(srcpath, (T_U16 *)src_path);
    Utl_USplitFilePath(srcpath, path, name);

    Eng_StrMbcs2Ucs(".", UStr_Tmp);
    if (Utl_UStrCmp(name,UStr_Tmp) == 0)
    {
        srcpath[Utl_UStrLen(srcpath)-2] = 0;
        Utl_USplitFilePath(srcpath, path, name);
    }
    
    if(!FileMgr_CheckFileIsExist(srcpath))
        return AK_FALSE;

    Utl_UStrCpy(File_Manage_Parm.SrcPath,srcpath);
    Utl_UStrCpy(File_Manage_Parm.FileName,name);
	
    File_Manage_Parm.CutFlag = cutflag;

    File_Manage_Parm.FolderFlag = Fwl_FsIsDir(srcpath);
        
    return AK_TRUE; 
}


T_VOID FileMgr_SaveDestFileInfo(T_pCWSTR filepath)
{
    Utl_UStrCpy(File_Manage_Parm.DestPath, (T_U16 *)filepath);
}


T_VOID FileMgr_SetSrcPath(T_pWSTR srcpath)
{
    Utl_UStrCpy(File_Manage_Parm.SrcPath,srcpath);
}


T_VOID FileMgr_GetSrcPath(T_pWSTR srcpath)
{
    if ((File_Manage_Parm.SrcPath == AK_NULL) || Utl_UStrLen(File_Manage_Parm.SrcPath)==0 )
    {
        srcpath[0] = 0;  
        return ;
    }
    Utl_UStrCpy(srcpath, File_Manage_Parm.SrcPath);
}


T_VOID FileMgr_SetDestPath(T_pWSTR destpath)
{
    Utl_UStrCpy(File_Manage_Parm.DestPath,destpath);
}


T_VOID FileMgr_GetDestPath(T_pWSTR destpath)
{
    if ((File_Manage_Parm.DestPath == AK_NULL) || Utl_UStrLen(File_Manage_Parm.DestPath)==0 )
    {
        destpath[0] = 0; 
        return ;
    }
    Utl_UStrCpy(destpath, File_Manage_Parm.DestPath);
}



T_VOID FileMgr_SetDeletetPath(T_pWSTR delpath)
{
    Utl_UStrCpy(File_Manage_Parm.DeletePath,delpath);
    //File_Manage_Parm.Manage_State = FILE_DELETE;

}


T_VOID FileMgr_GetDeletePath(T_pWSTR delpath)
{
    if ((File_Manage_Parm.DeletePath == AK_NULL) || Utl_UStrLen(File_Manage_Parm.DeletePath)==0 )
    {
        delpath[0] = 0;  
        return ;
    }
    Utl_UStrCpy(delpath, File_Manage_Parm.DeletePath);
}


T_VOID FileMgr_GetFileName(T_pWSTR filename)
{
    if ((File_Manage_Parm.FileName == AK_NULL) || Utl_UStrLen(File_Manage_Parm.FileName)==0 )
    {
        filename[0] = 0; 
        return ;
    }
    Utl_UStrCpy(filename, File_Manage_Parm.FileName);
}


T_MANAGE_STATE FileMgr_GetManageState(T_VOID)
{
    return File_Manage_Parm.Manage_State;
}

T_BOOL FileMgr_SetManageState(T_MANAGE_STATE mgr_state)
{
    if (mgr_state >= FILE_MANAGE_STATE_NUM)
    {
        Fwl_Print(C3, M_CTRL, " FileMgr_SetManageState(): param error !! ");
        return AK_FALSE;
    }
    File_Manage_Parm.Manage_State = mgr_state;
    return AK_TRUE;
}

T_BOOL FileMgr_GetCutFlag(T_VOID)
{
    return File_Manage_Parm.CutFlag;
}


T_BOOL FileMgr_GetFolderFlag(T_VOID)
{
    return File_Manage_Parm.FolderFlag;
}


T_U16 FileMgr_GetDriverId(T_pCWSTR path)
{
    if (path == AK_NULL)
        return 0xff;
        
    return (T_U16)Fwl_GetDriverIdByPath(path);
}


T_BOOL FileMgr_DetectDiskSpaceIsEnough(T_pCWSTR srcpath,T_pCWSTR destpath)
{
    T_U64_INT free_size = {0};
    T_U32 file_size = 0;
    T_U32 tmp_size = 0;

    if (srcpath == AK_NULL || destpath == AK_NULL)
        return AK_FALSE;
    
    Fwl_FsGetFreeSize(destpath[0], &free_size);

	file_size = Fwl_FileGetSize(srcpath);
	tmp_size  = Fwl_FileGetSize(destpath);

    if ((free_size.low + tmp_size < file_size + DISKSPACE_RESERVE) && (free_size.high < 1))    //SPACE IS NOT ENOUGH
        return AK_FALSE;
    else
        return AK_TRUE;
}


T_FILE_MANAGE_RETURN_VALUE FileMgr_Paste_Pre_Handle(T_pCWSTR srcpath,T_pCWSTR destpath)
{
    T_FILE_MANAGE_RETURN_VALUE ret = PASTE_FILE;
    T_USTR_FILE tmpstr;
    T_USTR_FILE path,name;
    T_U32       path_len, src_len;
    T_U16       driver1,driver2;
    
    driver1 = FileMgr_GetDriverId(srcpath);
    driver2 = FileMgr_GetDriverId(destpath);
	
#ifdef FILE_MGR_DBG
	Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle\n");
	Printf_UC(srcpath);
	Printf_UC(destpath);
	Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle state= %d act = %d  drv[%d]=>drv[%d]\n",
		File_Manage_Parm.Manage_State,ret,driver1,driver2);
#endif

    switch(File_Manage_Parm.Manage_State)
    {
        case FILE_NULL:
            ret = PASTE_BOARD_NULL;
            break;
            
        case FILE_COPY:
        case FILE_CUT:
			if (!FileMgr_CheckFileIsExist(srcpath))
			{
                ret = FILE_ERROR;
				File_Manage_Parm.Manage_State = FILE_NULL;
#ifdef FILE_MGR_DBG
				Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle Src  not exist\n");
#endif
				break;
			}
			
            Utl_UStrCpy(tmpstr, (T_U16 *)srcpath);
            Utl_USplitFilePath(tmpstr,path,name);
            src_len = Utl_UStrLen(name);
			
            Utl_UStrCpy(tmpstr, (T_U16 *)destpath);
            Utl_USplitFilePath(tmpstr,path,name);
            path_len = Utl_UStrLen(path);

            if  ((src_len+path_len)>  MAX_FILENM_LEN) 
            {
                ret = FILE_LONG_NAME;
#ifdef FILE_MGR_DBG
				Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle Dest Path too long\n");
#endif
				break;
            }
			
            if (!FileMgr_CheckFileIsExist(path))
            {
                ret = FILE_ERROR;
#ifdef FILE_MGR_DBG
				Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle destPath not exist\n");
				Printf_UC(path);
#endif
				break;
            }
        
            
            if (!Utl_UStrCmp((T_U16 *)srcpath, (T_U16 *)destpath)
				|| FileMgr_GetSubPath(srcpath,destpath,tmpstr))
            {
                ret = FILE_SAME_ADDR;
#ifdef FILE_MGR_DBG
				Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle Same Addr\n");
#endif
				break;
            }
			
            //源文件是单文件
            if (Fwl_FsIsFile(srcpath))          
            {
                if (AudioPlayer_IsPlayingFile(srcpath) &&( File_Manage_Parm.Manage_State == FILE_CUT))
                {
                    ret = FILE_BUSY;
#ifdef FILE_MGR_DBG
					Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle DiskSpace Is PlayingFile\n");
#endif
					break;
                }
                if (((File_Manage_Parm.Manage_State != FILE_CUT) || (driver1 !=driver2)) 
					&& (!FileMgr_DetectDiskSpaceIsEnough(srcpath,destpath)))
                {
                    ret = LACK_DISKSPACE;
#ifdef FILE_MGR_DBG
					Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle DiskSpace Is Not Enough\n");
#endif
					break;
                }

				
                if (FileMgr_CheckFileIsExist(destpath))
                {
                    if (Fwl_FsIsDir(destpath))
                   	{
                        ret = FILE_ERROR;
#ifdef FILE_MGR_DBG
						Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle destpath Is folder\n");
#endif
					}
                    else if(AudioPlayer_IsPlayingFile(destpath))
                    {
                        ret = FILE_BUSY;
#ifdef FILE_MGR_DBG
						Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle destpath Is playing\n");
#endif
					}
                    else
                    {
                        ret = FILE_SAME_NAME;
#ifdef FILE_MGR_DBG
						Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle destpath Is existed\n");
#endif
					}
                    break;
                }
				
                if (File_Manage_Parm.CutFlag && (driver1 == driver2))
               	{
                    ret = MOVE_FILE;
#ifdef FILE_MGR_DBG
					Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle Move...\n");
#endif
				}
                else
                {
                    ret = PASTE_FILE;
#ifdef FILE_MGR_DBG
					Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle Paste...\n");
#endif
				}
                break;
            }



            //源文件是文件夹
			if (Fwl_FsIsDir(srcpath))
			{
				if (FileMgr_CheckFileIsExist(destpath))
				{
					if (Fwl_FsIsDir(destpath))
					{
						ret = FILE_SAME_NAME;
#ifdef FILE_MGR_DBG
						Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle Src is folder :same name...\n");
#endif
					}
					else
					{
						ret = FILE_ERROR;
#ifdef FILE_MGR_DBG
						Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle Src is folder :Dest is File\n");
#endif
					}
					break;
				}
				else
				{
						
					//	if (File_Manage_Parm.CutFlag && (driver1 == driver2))
					//		ret = MOVE_FILE;
					//	else
							ret = PASTE_FOLDER; 			
				}

			}
			else
			{
				ret = FILE_ERROR;	
#ifdef FILE_MGR_DBG
				Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle Src is not folder not file\n");
#endif
			}
            break;
            
        default:
            ret = PASTE_BOARD_NULL;
            break;  
    }

#ifdef FILE_MGR_DBG
	Fwl_Print(C3, M_CTRL, "FileMgr_Paste_Pre_Handle state= %d act = %d\n",
		File_Manage_Parm.Manage_State,ret);
#endif
    return ret;
}


T_BOOL FileMgr_CheckFileIsExist(T_pCWSTR filepath)
{
    if (filepath == AK_NULL)
        return AK_FALSE;

	return Fwl_FileExist(filepath);
}


T_BOOL FileMgr_GetSubPath(T_pCWSTR rootpath,T_pCWSTR filepath,T_pWSTR subpath)
{
    T_U32 root_size,full_size;
    T_U32 i;
    T_USTR_FILE root_path;   
    T_USTR_FILE full_path;
    T_USTR_FILE tmp_path;    
    //subpath = "\0";

    if (rootpath == AK_NULL || filepath== AK_NULL || subpath== AK_NULL)
        return AK_FALSE;

    Utl_UStrCpy(root_path, (T_U16 *)rootpath);
    Utl_UStrCpy(full_path, (T_U16 *)filepath);
    root_size = Utl_UStrLen(root_path);
    full_size = Utl_UStrLen(full_path);

    if (root_path[root_size-1] == UNICODE_DOT)
        root_path[root_size-2] = 0;

    root_size = Utl_UStrLen(root_path);
    if (root_size>= full_size)
        return AK_FALSE;
    
    Utl_UStrCpy(tmp_path, (T_U16 *)filepath);
    if (tmp_path[root_size] != UNICODE_SOLIDUS)
        return AK_FALSE;
    tmp_path[root_size] = 0;
    if (Utl_UStrCmp(root_path, tmp_path) != 0)
    //if (strcmp(FileMgr_StrToUp(root_path),FileMgr_StrToUp( tmp_path)) != 0)
        return AK_FALSE;
    
    for(i=root_size;i<full_size;i++)
    {
        subpath[i-root_size] = full_path[i];    
    }
    subpath[full_size - root_size] = 0;
    
    return AK_TRUE;
}


