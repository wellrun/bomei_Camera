
#include <string.h>
#include "Gbl_Global.h"
#include "Ctl_DisplayList.h"
#include "Eng_ImgConvert.h"
#include "fwl_keyhandler.h"
#include "Eng_FileManage.h"
#include "Eng_AkBmp.h"
#include "Eng_String.h"
#include "Eng_String_uc.h"
#include "Eng_GblString.h"
#include "Fwl_osMalloc.h"
#include "Eng_DataConvert.h"
#include "Eng_topbar.h"
#include "fwl_pfdisplay.h"
#include "eng_debug.h"
#include "Lib_res_port.h"
#include "mount.h"
#include "Ctl_WaitBox.h"
#include "Fwl_osfs.h"

typedef enum {
    ROOT_DIRECTORY = 0,
    NORMAL_DIRECTORY,
    DIRECTORY_MODE_NUM
}T_DIRECTORY_MODE;

#define DISPLAYLIST_DISKQTY         27
//static T_DISPLAYLIST *p_display_list = AK_NULL;

static T_EXPLORER_ICON_ID DisplayList_GetIconId(T_DISPLAYLIST *pDisplayList, T_U16 *extStr);
static T_VOID DisplayList_SetDispIcon(T_DISPLAYLIST *pDisplayList, T_FILE_TYPE *FileType);
static T_VOID DisplayList_GetDriverList(T_DISPLAYLIST *pDisplayList, T_U32 *focusId);

static T_VOID DisplayList_GetRes(T_DISPLAYLIST *pDisplayList);
static T_VOID DisplayList_InitResRect(T_DISPLAYLIST *pDisplayList);

static T_BOOL DisplayList_EnterFolder(T_U16 *pCurPath, T_U16* pFolderName, T_S32* pSubLevel);
static T_BOOL DisplayList_ExitFolder(T_DISPLAYLIST *pDisplayList, T_U16* pCurPath, T_S32* pSubLevel);
static T_BOOL DisplayList_SortIdCallback(T_U32 Id1, T_U32 Id2);
static T_BOOL DisplayList_SortContentCallback(T_pCVOID pObject, T_VOID *pContent1, T_VOID *pContent2);
static T_BOOL DisplayList_ListCallback(T_pCVOID pObject);
static T_S32  DisplayList_GetSubLevelByPath(T_U16 *pPath);

static T_BOOL DisplayList_EnterRootDirectory(T_DISPLAYLIST *pDisplayList);

static T_BOOL DISPLAYLIST_TmpItemInit(T_DISPLAYLIST_TMP_LIST *pTmpList);
static T_BOOL DISPLAYLIST_AddItemToTmpChain(T_DISPLAYLIST_TMP_LIST *pTmpList, T_FILE_INFO *pFileInfo, T_U32 Id, T_EXPLORER_ICON_ID IconId);
static T_BOOL DISPLAYLIST_TmpDelALL(T_DISPLAYLIST_TMP_LIST *pTmpList);

extern T_pDATA p_menu_bckgrnd;


T_BOOL DisplayList_init(T_DISPLAYLIST *pDisplayList, T_U16 *pCurPath, const T_U16 *pTitleText, T_FILE_TYPE *FileType)
{
    T_U16 i;
    T_USTR_INFO Path;
    T_LEN smallIconW, smallIconH, bigIconW, bigIconH;   
    T_EXPLORER_ICON_ID IconListTbl[FILE_TYPE_NUM] = {
        ICON_BMP, ICON_JPG, ICON_JPG, ICON_JPG, ICON_PNG, ICON_GIF, ICON_UNKNOWN, ICON_UNKNOWN, 
        ICON_UNKNOWN, ICON_AVI, ICON_AKV, ICON_3GP, ICON_MP4, ICON_FLV, /*ICON_RM, ICON_RM,*/ ICON_MP3, ICON_MP3, ICON_MP3, 
        ICON_MP3, /* ICON_MKV, */
        ICON_MID, ICON_MID, ICON_WAV, ICON_WAV, ICON_WAV, ICON_AMR, ICON_WMA, ICON_WMA, ICON_MPEG, 
        ICON_AAC, ICON_AC3, ICON_ADIF, ICON_ADTS, ICON_M4A, ICON_FLAC, ICON_OGG, ICON_OGA, ICON_APE, 
        ICON_LRC, ICON_TXT, ICON_DOC, ICON_PDF, ICON_XLS, ICON_MAP, ICON_NES, ICON_SNES, ICON_GBA,ICON_MD,
        ICON_ALT, ICON_ALT, ICON_SAV, ICON_MFS, ICON_TTF, ICON_TTC, ICON_OTF, ICON_SWF, ICON_UNKNOWN
    };
    
    if (pDisplayList == AK_NULL)
    {
        return AK_FALSE;
    }

    //p_display_list = pDisplayList;
    
    DisplayList_GetRes(pDisplayList);
    DisplayList_InitResRect(pDisplayList);
    AKBmpGetInfo(pDisplayList->pIcon[0], &smallIconW, &smallIconH, AK_NULL);
    AKBmpGetInfo(pDisplayList->pBIcon[0], &bigIconW, &bigIconH, AK_NULL);
    
    for (i=0; i<FILE_TYPE_NUM; i++)
    {
        pDisplayList->DisplayListIconTbl[i].IconId = IconListTbl[i];
        pDisplayList->DisplayListIconTbl[i].IconShowEnable = AK_FALSE;
    }
    
    //pDisplayList->pCurPath = pCurPath;
    /**check if there is the path*/
    if (AK_NULL != pCurPath)
    {
        Utl_UStrCpy(Path, pCurPath);
        if (AK_FALSE == Fwl_FsIsDir(pCurPath))
        {
            memset((T_U8 *)Path, 0, sizeof(T_USTR_INFO));

			if (Fwl_CheckDriverIsValid(pCurPath))
			{
            	Fwl_GetRootDir(pCurPath, Path);
			}
        }

        DisplayList_SetCurFilePath(pDisplayList, Path);
    }
    else
    {
        DisplayList_SetCurFilePath(pDisplayList, AK_NULL);
    }

    pDisplayList->exitFolderFlag = AK_FALSE;
	pDisplayList->bPathTooDeep = AK_FALSE;
	pDisplayList->bNameTooLong = AK_FALSE;
    pDisplayList->subLevel= DisplayList_GetSubLevelByPath(pDisplayList->curPath);
    memset((T_U8 *)pDisplayList->exitFolderName, 0, sizeof(T_USTR_FILE));
    DisplayList_SetDisPostfix(pDisplayList, DISPLAYLIST_DISPOSTFIX);
    DisplayList_SetDisPath(pDisplayList, DISPLAYLIST_DISPATH);
    DisplayList_SetDispFolder(pDisplayList, DISPLAYLIST_DISPFOLDER);
	DisplayList_SetFindSubFolder(pDisplayList, DISPLAYLIST_NOT_FIND_SUBFOLDER);
    DisplayList_SetDispIcon(pDisplayList, FileType);
    
    IconExplorer_Init(&pDisplayList->IconExplorer, pDisplayList->titleRect, pDisplayList->itemRect, ICONEXPLORER_TITLE_ON | ICONEXPLORER_TITLE_TEXT_HCENTER | ICONEXPLORER_TITLE_TEXT_VCENTER | ICONEXPLORER_ITEM_FRAME);
    IconExplorer_SetTitleRect(&pDisplayList->IconExplorer, pDisplayList->titleRect, COLOR_BLUE, pDisplayList->pTitle);
    IconExplorer_SetTitleText(&pDisplayList->IconExplorer, pTitleText, COLOR_BLACK);
    IconExplorer_SetItemRect(&pDisplayList->IconExplorer, pDisplayList->itemRect, COLOR_WHITE, (const T_U8 *)p_menu_bckgrnd);
    IconExplorer_SetItemText(&pDisplayList->IconExplorer, COLOR_WHITE, COLOR_SKYBLUE, COLOR_BLACK);
    IconExplorer_SetItemTransColor(&pDisplayList->IconExplorer, g_Graph.TransColor);
    IconExplorer_SetSmallIcon(&pDisplayList->IconExplorer, smallIconW, smallIconH, 4, 1, 1);
    IconExplorer_SetLargeIcon(&pDisplayList->IconExplorer, bigIconW, bigIconH, 12, 1, 12, 8);
    IconExplorer_SetSortIdCallBack(&pDisplayList->IconExplorer, DisplayList_SortIdCallback);
    IconExplorer_SetSortContentCallBack(&pDisplayList->IconExplorer, DisplayList_SortContentCallback);
    IconExplorer_SetListCallBack(&pDisplayList->IconExplorer, DisplayList_ListCallback);
    IconExplorer_SetSortMode(&pDisplayList->IconExplorer, ICONEXPLORER_SORT_CONTENT);
    IconExplorer_SetScrollBarWidth(&pDisplayList->IconExplorer, g_Graph.BScBarWidth);    

    return AK_TRUE;
}

T_BOOL DisplayList_CheckItemList(T_DISPLAYLIST *pDisplayList)
{
    if (AK_NULL == pDisplayList)
    {
        return AK_FALSE;
    }

    return IconExplorer_CheckItemList(&pDisplayList->IconExplorer);
}

T_BOOL DisplayList_Free(T_DISPLAYLIST *pDisplayList)
{
    //p_display_list = AK_NULL;
    if (pDisplayList == AK_NULL)
    {
        return AK_FALSE;   
    }
    IconExplorer_Free(&pDisplayList->IconExplorer);

    return AK_TRUE;
}

T_BOOL DisplayList_Show(T_DISPLAYLIST *pDisplayList)
{
    if (Utl_UStrLen(pDisplayList->curPath) > (MAX_FILENM_LEN - FILE_LEN_RESERVE2 + 1))
    {
        DisplayList_ExitDirectory(pDisplayList);
        return AK_FALSE;
    }        
    return IconExplorer_Show(&pDisplayList->IconExplorer);
}

T_BOOL DisplayList_SetRefresh(T_DISPLAYLIST *pDisplayList, T_U32 RefreshFlag)
{
    return IconExplorer_SetRefresh(&pDisplayList->IconExplorer, RefreshFlag);
}

T_U32 DisplayList_GetRefresh(T_DISPLAYLIST *pDisplayList)
{
    if (AK_NULL == pDisplayList)
        return DISPLAYLIST_REFRESH_NONE;

    return IconExplorer_GetRefresh(&pDisplayList->IconExplorer);
}

/*
T_BOOL DisplayList_SelfRefresh()
{
    if (p_display_list== AK_NULL)
    {
        Fwl_Print(C3, M_CTRL, "p_display_list is null, cannot self refresh");
        return AK_FALSE;
    }
    IconExplorer_SetRefresh(&p_display_list->IconExplorer,DISPLAYLIST_REFRESH_ALL);
    IconExplorer_SetListFlag(&p_display_list->IconExplorer);
    IconExplorer_Show(&p_display_list->IconExplorer);
    return AK_TRUE;
}
*/

T_BOOL DisplayList_ListRefresh(T_DISPLAYLIST *pDisplayList)
{
    AK_ASSERT_PTR(pDisplayList,  "pDisplayList is null, cannot self refresh", AK_FALSE);
    
    IconExplorer_SetRefresh(&pDisplayList->IconExplorer,DISPLAYLIST_REFRESH_ALL);
    IconExplorer_SetListFlag(&pDisplayList->IconExplorer);
    return AK_TRUE;
}


T_eBACK_STATE DisplayList_Handler(T_DISPLAYLIST *pDisplayList, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_eBACK_STATE ret;
    T_MMI_KEYPAD phyKey;
    
    if (Event == M_EVT_USER_KEY)
    {
        phyKey.keyID = pParam->c.Param1;
        phyKey.pressType = pParam->c.Param2;

    }
    else
    {
        phyKey.keyID = kbNULL;
    }

    if (Event == M_EVT_USER_KEY && (phyKey.keyID == kbLEFT || phyKey.keyID == kbRIGHT) \
        && phyKey.pressType == PRESS_SHORT)
    {
    	if (!gb_UserkeyValid)
		{
			return IconExplorer_Handler(&pDisplayList->IconExplorer, Event, pParam);
		}
		
        if (DisplayList_GetDisPath(pDisplayList) == DISPLAYLIST_DISPATH)
        {
            if (phyKey.keyID == kbLEFT)
            {
                if (DisplayList_GetSubLevel(pDisplayList) > 0)
                {
                    DisplayList_ExitDirectory(pDisplayList);
                    DisplayList_SetListFlag(pDisplayList);
                }
            }
            else
            {
                DisplayList_Operate(pDisplayList);
            }

            DisplayList_SetRefresh(pDisplayList, DISPLAYLIST_REFRESH_ALL);
        }
        ret = eStay;
    }
    else
    {
        ret = IconExplorer_Handler(&pDisplayList->IconExplorer, Event, pParam);
    }
    
    return ret;
}

T_VOID *DisplayList_GetItemContentFocus(T_DISPLAYLIST *pDisplayList)
{
    T_FILE_INFO *pFileInfo = AK_NULL;

    if (pDisplayList->subLevel > 0)
    {
        pFileInfo = (T_FILE_INFO *)IconExplorer_GetItemContentFocus(&pDisplayList->IconExplorer);
        if (pFileInfo == AK_NULL)
            return AK_NULL;

        if (Utl_UStrLen(DisplayList_GetCurFilePath(pDisplayList)) +
            Utl_UStrLen(pFileInfo->name) > MAX_FILENM_LEN)
            return AK_NULL;
        else
            return pFileInfo;
    }
    else
    {
        return AK_NULL;
    }
}

T_VOID DisplayList_SetTopBarMenuIconState(T_DISPLAYLIST *pDisplayList)
{
    T_FILE_INFO *pFileInfo = AK_NULL;

    if (AK_NULL == pDisplayList)
    {
        return;
    }

    pFileInfo = DisplayList_GetItemContentFocus(pDisplayList);
    if ((DisplayList_GetSubLevel(pDisplayList) > 0) && pFileInfo)
    {
        if (!(((pFileInfo->attrib&0x10) == 0x10) \
        && (0 == Utl_UStrCmp(pFileInfo->name, _T("..")))))
        {
            TopBar_EnableMenuButton();
        }
        else
        {
            TopBar_DisableMenuButton();
        }
    }
    else
    {
        TopBar_DisableMenuButton();
    }
}

//the only difference with function above is the _T(".") item don't display the menu icon
T_VOID DisplayList_SetTopBarMenuIconStateInAVAddList(T_DISPLAYLIST *pDisplayList)
{
    T_FILE_INFO *pFileInfo = AK_NULL;

    if (AK_NULL == pDisplayList)
    {
        return;
    }

    pFileInfo = DisplayList_GetItemContentFocus(pDisplayList);
    if ((DisplayList_GetSubLevel(pDisplayList) > 0) && pFileInfo)
    {
        if (!(((pFileInfo->attrib&0x10) == 0x10) \
        && (0 == Utl_UStrCmp(pFileInfo->name, _T("..")) || 0 == Utl_UStrCmp(pFileInfo->name, _T(".")))))
        {
            TopBar_EnableMenuButton();
        }
        else
        {
            TopBar_DisableMenuButton();
        }
    }
    else
    {
        TopBar_DisableMenuButton();
    }
}


T_EXPLORER_ICON_ID DisplayList_GetIconId(T_DISPLAYLIST *pDisplayList, T_U16 *extStr)
{
    T_FILE_TYPE FileType;
    T_EXPLORER_ICON_ID IconId;        
    
    FileType = Utl_GetFileType(extStr);
    IconId = ICON_MAX_ID;
        
    if (pDisplayList->DisplayListIconTbl[FileType].IconShowEnable)
    {
        IconId = pDisplayList->DisplayListIconTbl[FileType].IconId;
    }

    return IconId;
}


static T_BOOL DisplayList_EnterFolder(T_U16* pCurPath, T_U16* pFolderName, T_S32* pSubLevel)
{
    T_U32 count;
    T_BOOL ret = AK_FALSE;
    
    if (pCurPath != AK_NULL && pFolderName != AK_NULL && pSubLevel != AK_NULL\
        && Utl_UStrLen(pFolderName) > 0 \
        && ((Utl_UStrLen(pCurPath) + Utl_UStrLen(pFolderName)) <= (MAX_FILENM_LEN - FILE_LEN_RESERVE2)))
    {
        count = Utl_UStrLen(pCurPath);
        if(count != 0)
        {
            if (pCurPath[count-1] == UNICODE_SOLIDUS|| pCurPath[count-1] == UNICODE_RES_SOLIDUS)
            {
                pCurPath[count-1] = 0;  
                //count--;  
            }
        }
        Utl_UStrCat(pCurPath, _T("/"));
        Utl_UStrCat(pCurPath, pFolderName);
        Utl_UStrCat(pCurPath, _T("/"));
        (*pSubLevel)++;
        
        ret = AK_TRUE;
    }
    
    return ret;
}

static T_BOOL DisplayList_ExitFolder(T_DISPLAYLIST *pDisplayList, T_U16* pCurPath, T_S32* pSubLevel)
{
    T_S32       i, j;
    T_BOOL      flag = AK_FALSE;
    T_U16       *pFolderName;
    T_U16       count;
    T_BOOL      ret = AK_FALSE;
    T_USTR_FILE tmpFolderName;

    if (*pSubLevel > 0)
    {
        pFolderName = pDisplayList->exitFolderName;
        memset((T_U8 *)tmpFolderName, 0, sizeof(T_USTR_FILE));
        
        count = (T_U16)(Utl_UStrLen(pCurPath)-1);
        for (i=count; i>=0; i--)
        {
            if ((pCurPath[i] == UNICODE_SOLIDUS) || (pCurPath[i] == UNICODE_RES_SOLIDUS))
            {
                if (flag == AK_TRUE)
                    break;
                else
                    flag = AK_TRUE;
            }
            // save the name of the folder in inverse
            tmpFolderName[count-i] = pCurPath[i]; 
            pCurPath[i] = '\0';
        }
        (*pSubLevel)--;
    
        // get the name of the exit folder
        // for set focus to the folder
        count = (T_U16)(Utl_UStrLen(tmpFolderName)-1);
        j = 0;
        for(i=0; i<=count; i++)
        {
            if ((tmpFolderName[count - i] != UNICODE_SOLIDUS) && (tmpFolderName[count - i] != UNICODE_RES_SOLIDUS))
            {
                pFolderName[j] = tmpFolderName[count - i];
                j++;
            }
        }
        pDisplayList->exitFolderFlag = AK_TRUE;
        
        ret = AK_TRUE;
    }
    
    return ret;
}


T_VOID DisplayList_GetDriverList(T_DISPLAYLIST *pDisplayList, T_U32 *focusId)
{
    T_U8            i;
    T_U8            driverNo[4];
    T_U16           driverNo_UC[4];
    T_STR_INFO      tmpstr;
    T_USTR_INFO     Utmpstr2;
    T_USTR_INFO     Utmpstr;
    T_pCDATA        pBigDriverIcon;
    T_pCDATA        pDriverIcon;
    T_U32           len;
    T_U32           count = 0;
    T_S8            diskId[DISPLAYLIST_DISKQTY];
    T_U8        	diskType[DISPLAYLIST_DISKQTY];
    T_U16           mountDiskQty;
	T_U8            frist_mount=0;

    pDriverIcon = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MENU_ICONDRIVER, &len);
    pBigDriverIcon = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MENU_ICONDRIVER, &len);
    
    driverNo[1] = ':';
    driverNo[2] = 0;
    driverNo[3] = 0;
#ifdef SPIBOOT
    frist_mount = 0;
#else
    frist_mount = 3;
#endif
    mountDiskQty = Fwl_GetDiskList(diskId, diskType, DISPLAYLIST_DISKQTY);

    for(i = frist_mount; i < mountDiskQty; i++)
    {
        driverNo[0] = diskId[i];
        switch (diskType[i])
        {
            case MEDIUM_NANDFLASH:
                Utl_UStrCpy(Utmpstr, GetCustomString(csNANDFLASH));
                break;
            case MEDIUM_SD:
                Utl_UStrCpy(Utmpstr, GetCustomString(csSD_DISK));
                break;
#ifdef USB_HOST
            case MEDIUM_USBHOST:
                Utl_UStrCpy(Utmpstr, GetCustomString(csU_DISK));
                break;
#endif
            default:
                Utl_MemSet((T_U8 *)Utmpstr, 0, sizeof(T_USTR_INFO));
                break;
        }
        sprintf(tmpstr, "%s%s%s", "(", driverNo, ")");
        Eng_StrMbcs2Ucs(tmpstr, Utmpstr2);
        Utl_UStrCat(Utmpstr, Utmpstr2);

        Eng_StrMbcs2Ucs(driverNo, driverNo_UC);
        IconExplorer_AddItem(&pDisplayList->IconExplorer, count, driverNo_UC, sizeof(driverNo_UC), Utmpstr, pDriverIcon, pBigDriverIcon);

		if (AK_NULL != focusId
			&& 0 == Utl_UStrCmpN(driverNo_UC, pDisplayList->exitFolderName, 4))
		{
			*focusId = count;
		}
		
		count++;
    }
}


T_BOOL DisplayList_SortIdCallback(T_U32 Id1, T_U32 Id2)
{
    if (Id1 > Id2)
        return AK_TRUE;
    else
        return AK_FALSE;
}

T_BOOL DisplayList_SortContentCallback(T_pCVOID pObject, T_VOID *pContent1, T_VOID *pContent2)
{
    T_DISPLAYLIST *pDisplayList;

    pDisplayList = (T_DISPLAYLIST *)pObject; 

    if (pDisplayList->subLevel > 0)
    {
        T_FILE_INFO *pFileInfo1, *pFileInfo2;

        pFileInfo1 = pContent1;
        pFileInfo2 = pContent2;

        if (((pFileInfo1->attrib & 0x10) == 0x10) && ((pFileInfo2->attrib & 0x10) != 0x10))
            return AK_FALSE;
        else if (((pFileInfo1->attrib & 0x10) != 0x10) && ((pFileInfo2->attrib & 0x10) == 0x10))
            return AK_TRUE;
        else if (Utl_UStrCmp(pFileInfo1->name, _T("."))== 0 \
                    && Utl_UStrCmp(pFileInfo2->name, _T("..")) != 0)
            return AK_FALSE;
        else if (Utl_UStrCmp(pFileInfo1->name, _T(".."))== 0 \
                    && Utl_UStrCmp(pFileInfo2->name, _T(".")) != 0)
            return AK_FALSE;
        else if (Utl_UStrCmp(pFileInfo2->name, _T("."))== 0 \
                    && Utl_UStrCmp(pFileInfo1->name, _T("..")) != 0)
            return AK_TRUE;
        else if (Utl_UStrCmp(pFileInfo2->name, _T(".."))== 0 \
                    && Utl_UStrCmp(pFileInfo1->name, _T(".")) != 0)
            return AK_TRUE;
        else if (Utl_UStrCmp(pFileInfo1->name, pFileInfo2->name) > 0)
            return AK_TRUE;
        else
            return AK_FALSE;
    }
    else
    {
        T_U16 *pDriverNo1, *pDriverNo2;

        pDriverNo1 = pContent1;
        pDriverNo2 = pContent2;

        if (Utl_UStrCmp(pDriverNo1, pDriverNo2) > 0)
            return AK_TRUE;
        else
            return AK_FALSE;
    }
}

T_VOID DisplayList_FindInSubFolders(T_DISPLAYLIST * pDisplayList, T_DISPLAYLIST_TMP_LIST *TmpList, T_pCWSTR path, T_PFILE parent, T_U32 *count)
{
	T_FILE_INFO     FileInfo;
	T_EXPLORER_ICON_ID iconId;
	T_BOOL			AddItemRet = AK_TRUE;
	T_USTR_FILE 	TmpFileName, TmpFileExt;
	T_USTR_FILE     TotalFileName;
	T_U32           FindHandle;
	T_PFILEINFO 	info;
	T_PFILE			file;

	Fwl_Print(C3, M_CTRL, "DL_FdInSF");

	if (Utl_CaclSolidas(path) > MAX_PATH_DEEP)
    {
    	Fwl_Print(C3, M_CTRL, "PathDeep > MAX_PATH_DEEP_1");
		pDisplayList->bPathTooDeep = AK_TRUE;
		return;
    }
	
	FindHandle = Fwl_FileFindFirstFromHandle((T_U32)parent);
    if (0 != FindHandle)
    {
        do {
			info=(T_PFILEINFO)Fwl_FsFindInfo(&FileInfo,(T_hFILESTAT)FindHandle);
			if (AK_NULL == info)
			{
				break;
			}
         	file = (T_PFILE)Fwl_FileFindOpen((T_U32)parent, (T_U32)info);
			
			Utl_UStrCpyN(TotalFileName, path, MAX_FILENM_LEN);

			if (Utl_UStrLen(TotalFileName) + Utl_UStrLen(FileInfo.name) + Utl_UStrLen(_T("/")) <= MAX_FILENM_LEN)
			{
			    Utl_UStrCat(TotalFileName, _T("/"));
				Utl_UStrCat(TotalFileName, FileInfo.name);
			}
			else
			{
				Fwl_Print(C3, M_CTRL, "1path name is too long!");
				pDisplayList->bNameTooLong = AK_TRUE;
				return;
			}
					
            if ((FileInfo.attrib & 0x10) == 0x10) 
            {
				T_USTR_FILE maxPath = {0};

				Utl_UStrCpy(maxPath, pDisplayList->curPath);
				Utl_UStrCat(maxPath, TotalFileName);

				if (Utl_CaclSolidas(maxPath) > MAX_PATH_DEEP)
    			{
    				Fwl_Print(C3, M_CTRL, "PathDeep > MAX_PATH_DEEP_2");
					pDisplayList->bPathTooDeep = AK_TRUE;
					continue;
    			}
				
            	if (DISPLAYLIST_FIND_SUBFOLDER == DisplayList_GetFindSubFolder(pDisplayList))
            	{		
					DisplayList_FindInSubFolders(pDisplayList, TmpList, TotalFileName, file, count);
				}
            }
            else if ((FileInfo.attrib & 0x08) != 0x08)
            {   
                iconId = DisplayList_GetIconId(pDisplayList, FileInfo.name);
                if (iconId < ICON_MAX_ID)
                {
					Utl_UStrCpy(FileInfo.name, TotalFileName);

                    if (DisplayList_GetDisPostfix(pDisplayList) == DISPLAYLIST_DISPOSTFIX)
                    {
                        AddItemRet = DISPLAYLIST_AddItemToTmpChain(TmpList, &FileInfo, *count, iconId);
                    }
                    else
                    {
                        Utl_USplitFileName(FileInfo.name, TmpFileName, TmpFileExt);
                        AddItemRet = DISPLAYLIST_AddItemToTmpChain(TmpList, &FileInfo, *count, iconId);
                    }
                }
            }
			Fwl_FileClose((T_pFILE)file);
            (*count)++;
        } while (AddItemRet == AK_TRUE && Fwl_FsFindNext((T_hFILESTAT)FindHandle) != AK_FALSE);
		Fwl_FileFindCloseWithHandle(FindHandle);
    }
}


T_BOOL DisplayList_ListCallback(T_pCVOID pObject)
{
    T_USTR_FILE     FindPath;
    T_FILE_INFO     FileInfo;
    T_U32           count = 0;
    T_EXPLORER_ICON_ID iconId;
    T_U32           focusId = 0;
    T_DISPLAYLIST   *pDisplayList;
    T_USTR_FILE     upFolderStr;    //"上级目录"
    T_USTR_FILE     curFolderStr;   //"当前目录"
    T_USTR_FILE     TmpFileName, TmpFileExt;
    T_U16           CurDir[4], UpDir[4];
    T_BOOL          AddItemRet = AK_TRUE;
    T_U32           i = 0;
    T_DISPLAYLIST_TMP_LIST  TmpList;
	T_PFILEINFO     info;
	T_PFILE         FindParent;
	T_U32		   FindHandle;
	T_PFILE         file;
    
    pDisplayList = (T_DISPLAYLIST *)pObject; 
    if (pDisplayList == AK_NULL)
        return AK_FALSE;

    //If too much files in a folder, it need to set the freq to highest to reduce the time of call this function.
//    FreqMgr_StateCheckIn(FREQ_FACTOR_DEFAULT, FREQ_PRIOR_HIGH);

    Eng_StrMbcs2Ucs("..", UpDir);
    Eng_StrMbcs2Ucs(".", CurDir);

    DISPLAYLIST_TmpItemInit(&TmpList);

    
    Utl_UStrCpy(upFolderStr, GetCustomString(csEXPLORER_UPFOLDER));
    Utl_UStrCpy(curFolderStr, GetCustomString(csEXPLORER_CURFOLDER));

    if (pDisplayList->subLevel > 0)
    {
        Utl_UStrCpy(FindPath, pDisplayList->curPath);
        //Utl_UStrCat(FindPath, _T("*.*")); // use File_OpenUnicode to open ,no need this 
        Utl_UStrCat(curFolderStr, pDisplayList->curPath);
        
        if (pDisplayList->subLevel >= 1 && DisplayList_GetDisPath(pDisplayList) == DISPLAYLIST_DISPATH)
        {
            //current folder
            FileInfo.attrib = 0x10;
            Utl_UStrCpy(FileInfo.name, CurDir);
            AddItemRet = DISPLAYLIST_AddItemToTmpChain(&TmpList, &FileInfo, count, ICON_FOLDER0);
            count++;
          
            //up folder
            FileInfo.attrib = 0x10;
            Utl_UStrCpy(FileInfo.name, UpDir);
            AddItemRet = DISPLAYLIST_AddItemToTmpChain(&TmpList, &FileInfo, count, ICON_UP);
            count++;
        }
		
		FindParent = File_OpenUnicode(AK_NULL, FindPath, FILE_MODE_READ);

		FindHandle = Fwl_FileFindFirstFromHandle((T_U32)FindParent);
		
        if (0 != FindHandle)
        {
            do {
				info = (T_PFILEINFO)Fwl_FsFindInfo(&FileInfo, (T_hFILESTAT)FindHandle);
				if (AK_NULL == info)
				{
					break;
				}
				file = (T_PFILE)Fwl_FileFindOpen((T_U32)FindParent, (T_U32)info);
          
                if ((FileInfo.attrib & 0x10) == 0x10) 
                {
					if (Utl_CaclSolidas(FindPath) > MAX_PATH_DEEP)
						continue;
					
                	if (DISPLAYLIST_FIND_SUBFOLDER == DisplayList_GetFindSubFolder(pDisplayList))
                	{
						DisplayList_FindInSubFolders(pDisplayList, &TmpList, FileInfo.name/*FindPath*/, file, &count);
					}
                    else if (DisplayList_GetDispFolder(pDisplayList) == DISPLAYLIST_DISPFOLDER)
                    {
                        AddItemRet = DISPLAYLIST_AddItemToTmpChain(&TmpList, &FileInfo, count, ICON_FOLDER0);
                        if (pDisplayList->exitFolderFlag)
                        {
                            if (Utl_UStrCmp(FileInfo.name, pDisplayList->exitFolderName) == 0)
                            {
                                focusId = count;           
                            }
                        }
                    }
                }
                else if ((FileInfo.attrib & 0x08) != 0x08)
                {   
                    iconId = DisplayList_GetIconId(pDisplayList, FileInfo.name);
                    if (iconId < ICON_MAX_ID)
                    {
                        if (DisplayList_GetDisPostfix(pDisplayList) == DISPLAYLIST_DISPOSTFIX)
                        {
                            AddItemRet = DISPLAYLIST_AddItemToTmpChain(&TmpList, &FileInfo, count, iconId);
                        }
                        else
                        {
                            Utl_USplitFileName(FileInfo.name, TmpFileName, TmpFileExt);
                            AddItemRet = DISPLAYLIST_AddItemToTmpChain(&TmpList, &FileInfo, count, iconId);
                        }
                    }
                }
                count++;
				Fwl_FileClose((T_pFILE)file);
            } while (AddItemRet == AK_TRUE && Fwl_FsFindNext((T_hFILESTAT)FindHandle) != AK_FALSE);
			Fwl_FileFindCloseWithHandle(FindHandle);
        }

		Fwl_FileClose((T_pFILE)FindParent);
		
        // move to icon_explorer
        if (TmpList.ItemQty > 0)
        {
            T_DISPLAYLIST_TMP_ITEM *p = AK_NULL;
            T_FILE_INFO *pTmpFileInfo = AK_NULL;

			if(TmpList.ItemQty>200) //对于文件比较多等待时间比较长的显示waitbox
			{
				WaitBox_Start(WAITBOX_RAINBOW, (T_pWSTR)GetCustomString(csWAITING));
			}
            
            p = TmpList.pItemHead;
            for(i=0; i<TmpList.ItemQty; i++)
            {
                pTmpFileInfo = &p->Content;

                if ((pTmpFileInfo->attrib & 0x10) == 0x10)
                {
                    if (Utl_UStrCmp(pTmpFileInfo->name, UpDir) == 0)
                        IconExplorer_AddItem(&pDisplayList->IconExplorer, p->Id, pTmpFileInfo, \
                            sizeof(T_FILE_INFO), upFolderStr, pDisplayList->pIcon[ICON_UP], pDisplayList->pBIcon[ICON_UP]);
                    else if (Utl_UStrCmp(pTmpFileInfo->name, CurDir) == 0)
                        IconExplorer_AddItem(&pDisplayList->IconExplorer, p->Id, pTmpFileInfo, \
                            sizeof(T_FILE_INFO), curFolderStr, pDisplayList->pIcon[ICON_FOLDER0], pDisplayList->pBIcon[ICON_FOLDER0]);
                    else
                    {
                        IconExplorer_AddItem(&pDisplayList->IconExplorer, p->Id, pTmpFileInfo, sizeof(T_FILE_INFO), \
                            pTmpFileInfo->name, pDisplayList->pIcon[ICON_FOLDER0], pDisplayList->pBIcon[ICON_FOLDER0]);
                    }
                }
                else if ((pTmpFileInfo->attrib & 0x08) != 0x08)
                {
                	T_USTR_FILE     name;
					
					Utl_USplitFilePath(pTmpFileInfo->name, AK_NULL, name);
                    IconExplorer_AddItem(&pDisplayList->IconExplorer, p->Id, pTmpFileInfo, sizeof(T_FILE_INFO), \
                        name, pDisplayList->pIcon[p->IconId], pDisplayList->pBIcon[p->IconId]);
                }

                if (p->pNext != AK_NULL)
                    p = p->pNext;    
            }

            DISPLAYLIST_TmpDelALL(&TmpList);
				
        }
        
//      restore_all_int();
    }
    else
    {
        DisplayList_GetDriverList(pDisplayList, &focusId);
    }

    // exit a folder, set focus to the folder 
    if (focusId != 0)
    {
        IconExplorer_SetFocus(&pDisplayList->IconExplorer, focusId);
    } // enter a folder, if it is not display the drivers and if the quantity of the file is more than 2
    else if (pDisplayList->subLevel > 0)
    {
        if (count > 2)
        {
            if (DisplayList_GetDisPath(pDisplayList) == DISPLAYLIST_DISPATH)
            {
                // set focus to first file 
                IconExplorer_SetFocusByIndex(&pDisplayList->IconExplorer, 2);
            }
            else
            {
                // set focus to first file 
                IconExplorer_SetFocusByIndex(&pDisplayList->IconExplorer, 0);
            }
        }
        else
        {
            if (DisplayList_GetDisPath(pDisplayList) == DISPLAYLIST_DISPATH)
            {
                // set focus to up folder
                IconExplorer_SetFocusByIndex(&pDisplayList->IconExplorer, 1);
            }
            else
            {
                // set focus to first file
                IconExplorer_SetFocusByIndex(&pDisplayList->IconExplorer, 0);
            }
        }
    }
    pDisplayList->exitFolderFlag = AK_FALSE;
    memset(pDisplayList->exitFolderName, 0, sizeof(T_USTR_FILE));

//    FreqMgr_StateCheckOut(FREQ_FACTOR_DEFAULT);    

    DisplayList_SetTopBarMenuIconState(pDisplayList);

	WaitBox_Stop();

    return AK_TRUE;
}

T_S32 DisplayList_GetSubLevelByPath(T_U16 *pPath)
{
    T_S32 i;
    T_S32 count = 0;
    T_USTR_FILE path;
    T_S32 subLevel = 0;

    if (pPath != AK_NULL)
    {
        count = Utl_UStrLen(pPath);
    }
    if(count != 0)
    {
        Utl_UStrCpy(path, pPath);
        if ((path[count-1] != UNICODE_SOLIDUS) && (path[count-1] != UNICODE_RES_SOLIDUS))
        {
            path[count] = UNICODE_SOLIDUS;
            path[count+1] = 0; 
            count++;  
        }
        for(i = 0; i < count; i++)
        {
            if((path[i] == UNICODE_SOLIDUS) || (path[i] == UNICODE_RES_SOLIDUS))
                subLevel++;
        }
    }
    return subLevel;
}

T_FILE_INFO *DisplayList_Operate(T_DISPLAYLIST *pDisplayList)
{
    T_FILE_INFO *pFileInfo = AK_NULL;
    
    if (pDisplayList->subLevel == 0)
    {
        DisplayList_EnterRootDirectory(pDisplayList);
        IconExplorer_SetListFlag(&pDisplayList->IconExplorer);
    }
    else
    {
        pFileInfo = IconExplorer_GetItemContentFocus(&pDisplayList->IconExplorer);
        if (pFileInfo != AK_NULL)
        {
            if ((pFileInfo->attrib & 0x10) == 0x10)
            {
                if (Utl_UStrCmp(pFileInfo->name, _T("..")) == 0)
                {
                    DisplayList_ExitDirectory(pDisplayList);  
                }
                else if (Utl_UStrCmp(pFileInfo->name, _T(".")) != 0)
                {
                    if (Utl_UStrLen(pDisplayList->curPath) + Utl_UStrLen(pFileInfo->name) > (MAX_FILENM_LEN - FILE_LEN_RESERVE2))
                        return AK_NULL;
                    DisplayList_EnterDirectory(pDisplayList);
                }
                
                IconExplorer_SetListFlag(&pDisplayList->IconExplorer);

                //must load the itemlist , or the topbar menu icon would  show wrong when change directory   
                DisplayList_CheckItemList(pDisplayList);

                pFileInfo = AK_NULL;
            }
            else if ((Utl_UStrLen(pDisplayList->curPath) + Utl_UStrLen(pFileInfo->name)) > MAX_FILENM_LEN)
            {
                pFileInfo = AK_NULL;
            }
        }
    }
    
    return pFileInfo;
}

static T_BOOL DisplayList_EnterRootDirectory(T_DISPLAYLIST *pDisplayList)
{
    T_U16       *pDriverId;
    T_BOOL ret = AK_FALSE;
   
    pDriverId = IconExplorer_GetItemContentFocus(&pDisplayList->IconExplorer);
    if (pDriverId != AK_NULL)
    {
        Utl_UStrCpy(pDisplayList->curPath, pDriverId);
        Utl_UStrCat(pDisplayList->curPath, _T("/"));
        (pDisplayList->subLevel)++;
        ret = AK_TRUE;
    }
    
    return ret;
}

T_BOOL DisplayList_ExitDirectory(T_DISPLAYLIST *pDisplayList)
{
   return DisplayList_ExitFolder(pDisplayList, pDisplayList->curPath, &pDisplayList->subLevel);
}

T_BOOL DisplayList_EnterDirectory(T_DISPLAYLIST *pDisplayList)
{
    T_FILE_INFO *pFileInfo;
    T_BOOL ret = AK_FALSE;
    
    pFileInfo = IconExplorer_GetItemContentFocus(&pDisplayList->IconExplorer);
    if (pFileInfo != AK_NULL)
    {
        ret = DisplayList_EnterFolder(pDisplayList->curPath, pFileInfo->name, &pDisplayList->subLevel);
    }
    
    return ret;
}

T_DISPLAYLIST_ICONSTYLE DisplayList_GetItemIconStyle(T_DISPLAYLIST *pDisplayList)
{
    return IconExplorer_GetItemIconStyle(&pDisplayList->IconExplorer);    
}

T_BOOL DisplayList_SetItemIconStyle(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_ICONSTYLE ItemIconStyle)
{
    return IconExplorer_SetItemIconStyle(&pDisplayList->IconExplorer, ItemIconStyle);
}

T_DISPLAYLIST_SORTMODE DisplayList_GetSortMode(T_DISPLAYLIST *pDisplayList)
{
    return pDisplayList->IconExplorer.ItemSortMode;
}

T_BOOL DisplayList_SetSortMode(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_SORTMODE ItemSortMode)
{
    return IconExplorer_SetSortMode(&pDisplayList->IconExplorer, ItemSortMode);
}

T_BOOL DisplayList_SetListFlag(T_DISPLAYLIST *pDisplayList)
{
    return IconExplorer_SetListFlag(&pDisplayList->IconExplorer);
}

T_BOOL DisplayList_DelFocusItem(T_DISPLAYLIST *pDisplayList)
{
    return IconExplorer_DelItemFocus(&pDisplayList->IconExplorer);
}

T_BOOL DisplayList_MoveFocus(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_DIRECTION Direction)
{
    return IconExplorer_MoveFocus(&pDisplayList->IconExplorer, Direction);   
}

T_S32 DisplayList_GetSubLevel(T_DISPLAYLIST *pDisplayList)
{
    return pDisplayList->subLevel;
}

T_BOOL DisplayList_SetCurFilePath(T_DISPLAYLIST *pDisplayList, T_U16 *pCurPath)
{
    if (pDisplayList == AK_NULL)
    {
        return AK_FALSE;   
    }
    if ((pCurPath != AK_NULL) && (Utl_UStrLen(pCurPath) > 0))
    {
        Utl_UStrCpy(pDisplayList->curPath, pCurPath);
    }
    else
    {
        memset((T_U8 *)pDisplayList->curPath, 0, sizeof(T_USTR_FILE));   
    }
    
    return AK_TRUE;
}

T_U16 *DisplayList_GetCurFilePath(T_DISPLAYLIST *pDisplayList)
{
    return pDisplayList->curPath;
}

static T_VOID DisplayList_GetRes(T_DISPLAYLIST *pDisplayList)
{
    T_U32 len;
    T_U32 i;
    
    for (i=0; i<ICON_MAX_ID; i++)
    {
        pDisplayList->pIcon[i] = Res_GetBinResByID(&pDisplayList->pIcon[i], AK_FALSE, eRES_BMP_MENU_ICONBMP + i, &len);
        //pDisplayList->pBIcon[i] = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MENU_BIG_ICON3GP + i, &len);
        pDisplayList->pBIcon[i] = Res_GetBinResByID(&pDisplayList->pBIcon[i], AK_FALSE, eRES_BMP_MENU_ICONBMP + i, &len);
    }
    
    //pDisplayList->pBkImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MENU_BACKGROUND, &len);
    pDisplayList->pTitle = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_TITLE, &len);
}

static T_VOID DisplayList_InitResRect(T_DISPLAYLIST *pDisplayList)
{
    T_pRECT pTitleRect;
    T_pRECT pItemRect;

    pTitleRect = &pDisplayList->titleRect;
    pItemRect = &pDisplayList->itemRect;

    pTitleRect->left = 0;
    pTitleRect->top = 0;
    pTitleRect->width = Fwl_GetLcdWidth();
    pTitleRect->height = TOP_BAR_HEIGHT;
    if (AK_NULL != pTitleRect)
    {
        AKBmpGetInfo(pDisplayList->pTitle, &pTitleRect->width, &pTitleRect->height, AK_FALSE);
    }
    
    pItemRect->left = 0;
    pItemRect->top = pTitleRect->height;
    pItemRect->width = Fwl_GetLcdWidth();
    pItemRect->height = Fwl_GetLcdHeight() - pTitleRect->height;
}

T_BOOL DisplayList_SetDisPostfix(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_DIS_POSTFIX DisPostfix)
{
    if (AK_NULL == pDisplayList)
        return AK_FALSE;

    if (DisPostfix < DISPLAYLIST_DISPOSTFIX_NUM)
    {
        pDisplayList->DisPostfix = DisPostfix;
        return AK_FALSE;
    }

    return AK_FALSE;
}

T_DISPLAYLIST_DIS_POSTFIX DisplayList_GetDisPostfix(T_DISPLAYLIST *pDisplayList)
{
    if (AK_NULL == pDisplayList)
        return DISPLAYLIST_NOT_DISPOSTFIX;

    return pDisplayList->DisPostfix;
}

T_BOOL DisplayList_SetDisPath(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_DIS_PATH DisPath)
{
    if (AK_NULL == pDisplayList)
        return AK_FALSE;

    if (DisPath < DISPLAYLIST_DISPATH_NUM)
    {
        pDisplayList->DisPath = DisPath;
        return AK_FALSE;
    }

    return AK_FALSE;
}

T_DISPLAYLIST_DIS_PATH DisplayList_GetDisPath(T_DISPLAYLIST *pDisplayList)
{
    if (AK_NULL == pDisplayList)
        return DISPLAYLIST_NOT_DISPATH;

    return pDisplayList->DisPath;
}


T_BOOL DisplayList_SetDispFolder(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_DISP_FOLDER DispFolder)
{
    if (AK_NULL == pDisplayList)
        return AK_FALSE;
    
    if (DispFolder < DISPLAYLIST_DISPFOLDER_NUM)
    {
        pDisplayList->DispFolder = DispFolder;
        return AK_TRUE;
    }
    
    return AK_FALSE;
}

T_DISPLAYLIST_DISP_FOLDER DisplayList_GetDispFolder(T_DISPLAYLIST *pDisplayList)
{
    if (AK_NULL == pDisplayList)
    return DISPLAYLIST_NOT_DISPFOLDER;

    return pDisplayList->DispFolder;
}

T_BOOL DisplayList_SetFindSubFolder(T_DISPLAYLIST *pDisplayList, T_DISPLAYLIST_FIND_SUBFOLDER findsubFolder)
{
    if (AK_NULL == pDisplayList)
        return AK_FALSE;
    
    if (findsubFolder < DISPLAYLIST_FIND_SUBFOLDER_NUM)
    {
        pDisplayList->find_subfolder = findsubFolder;
        return AK_TRUE;
    }
    
    return AK_FALSE;
}

T_DISPLAYLIST_DISP_FOLDER DisplayList_GetFindSubFolder(T_DISPLAYLIST *pDisplayList)
{
    if (AK_NULL == pDisplayList)
    	return DISPLAYLIST_NOT_FIND_SUBFOLDER;

    return pDisplayList->find_subfolder;
}

static T_BOOL DISPLAYLIST_TmpItemInit(T_DISPLAYLIST_TMP_LIST *pTmpList)
{
    pTmpList->pItemHead = AK_NULL;
    pTmpList->pItemTail = AK_NULL;
    pTmpList->ItemQty = 0;
    return AK_TRUE;
}

// parse the file path string and add to the file list chain
static T_BOOL DISPLAYLIST_AddItemToTmpChain(T_DISPLAYLIST_TMP_LIST *pTmpList, T_FILE_INFO *pFileInfo, T_U32 Id, T_EXPLORER_ICON_ID IconId)
{
    T_DISPLAYLIST_TMP_ITEM *p = AK_NULL;
    T_DISPLAYLIST_TMP_ITEM *pTmp = AK_NULL;
    
    AK_ASSERT_PTR(pTmpList, "DISPLAYLIST_AddItemToTmpChain(): pTmpList", AK_FALSE);
    AK_ASSERT_PTR(pFileInfo, "DISPLAYLIST_AddItemToTmpChain(): FilePathBuf", AK_FALSE);

    //Fwl_Print(C3, M_CTRL, "FileList_AddItemToChain pFileList->ItemQty = %d\r\n", pFileList->ItemQty);

    if (pTmpList->ItemQty >= ICONEXPLORER_ITEM_QTYMAX)
    {
        return AK_FALSE;
    }

    p = (T_DISPLAYLIST_TMP_ITEM *)Fwl_Malloc(sizeof(T_DISPLAYLIST_TMP_ITEM));
    AK_ASSERT_PTR(p, "DISPLAYLIST_AddItemToTmpChain(): malloc error", AK_FALSE);

    memcpy(&p->Content, pFileInfo, sizeof(T_FILE_INFO));
    pTmpList->ItemQty++;
    
    //memcpy(&p->Content, pFileInfo, sizeof(T_FILE_INFO));
    p->Id = Id;
    p->IconId = IconId;
    
    // add item to chain
    if (AK_NULL == pTmpList->pItemHead)
    {
        p->pPrevious = AK_NULL;
        p->pNext = AK_NULL;
        pTmpList->pItemHead = p;
        pTmpList->pItemTail = p;
    }
    else if (AK_NULL != pTmpList->pItemTail)
    {
        pTmp = pTmpList->pItemTail;
        pTmp->pNext = p;
        p->pPrevious = pTmp;
        p->pNext = AK_NULL;
        pTmpList->pItemTail = p;
    }
    else
    {
        Fwl_Print(C3, M_CTRL, "DISPLAYLIST_AddItemToTmpChain ERROR\r\n");
        p = Fwl_Free(p);
        return AK_FALSE;   
    }
    
    return AK_TRUE;
}

static T_BOOL DISPLAYLIST_TmpDelALL(T_DISPLAYLIST_TMP_LIST *pTmpList)
{
    T_DISPLAYLIST_TMP_ITEM *p, *q;

    AK_ASSERT_PTR(pTmpList, "DISPLAYLIST_TmpDelALL(): pTmpList null", AK_FALSE);

    p = pTmpList->pItemHead;
    while (p != AK_NULL)
    {
        q = p->pNext;
        p = Fwl_Free(p);
        p = q;
    }
    pTmpList->pItemHead = AK_NULL;
    pTmpList->pItemTail = AK_NULL;
    pTmpList->ItemQty = 0;
    
    return AK_TRUE;
}

T_VOID DisplayList_SetDispIcon(T_DISPLAYLIST *pDisplayList, T_FILE_TYPE *FileType)
{
    T_FILE_TYPE i;
    T_FILE_TYPE *FileTypeTmp = FileType;
    
    if (FILE_TYPE_NONE == (*FileTypeTmp))
    {
        return;
    }
    else if (FILE_TYPE_ALL == (*FileTypeTmp))
    {
        for (i=0; i<FILE_TYPE_NUM; i++)
        {
            pDisplayList->DisplayListIconTbl[i].IconShowEnable = AK_TRUE;
        }
        
        return;
    }
    else
    {
        for (i=0; i<FILE_TYPE_NUM; i++)
        {
            pDisplayList->DisplayListIconTbl[i].IconShowEnable = AK_FALSE;
        }
    }
    
    while (FILE_TYPE_NONE != (*FileTypeTmp))
    {
        pDisplayList->DisplayListIconTbl[*FileTypeTmp].IconShowEnable = AK_TRUE;
        FileTypeTmp++;
    }

}

/* end of file */



