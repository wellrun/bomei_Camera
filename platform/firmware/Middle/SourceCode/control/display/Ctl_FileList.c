
#include "eng_graph.h"
#include "Ctl_FileList.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "fwl_keyhandler.h"
#include "fwl_pfkeypad.h"
#include "Fwl_osFS.h"
#include "Eng_ImgConvert.h"
#include "Eng_Math.h"
#include "Eng_AkBmp.h"
#include "Eng_string_uc.h"
#include "Eng_string.h"
#include "Eng_DataConvert.h"
#include "eng_debug.h"
#include "fwl_pfdisplay.h"
#include "Lib_res_port.h"
#include "fwl_osfs.h"

#include "Fwl_display.h"
#include "file.h"

#define FILELIST_SEPARATOR_CHR      ';'
#define FILELIST_SEPARATOR_STR      ";"
#define FILELIST_AUDIOLIST_EXT      "alt"
#define FILELIST_VIDEOLIST_EXT      "vlt"
#define FILELIST_WRITE_FILE_NUM		32


/**
 * @brief   FileList module set fetch mode.
 *
 * set the way to fetch a file from the FileList module, seauence, repeat all, repeat single and random for example.
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] FetchMode The way to fetch a file from the FileList module
 * @return  T_BOOL
 * @retval  AK_TRUE Successful
 * @retval  AK_FALSE FileList set fetch mode error
 */
static T_BOOL FileList_SetFetchMode(T_FILELIST *pFileList, T_FILELIST_FETCHMODE FetchMode);

/**
 * @brief   delete all item from FileList module.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE delete all item successful
 * @retval  AK_FALSE delete all item error
 */
static T_BOOL FileList_DelALL(T_FILELIST *pFileList);


/**
 * @brief   set max item quantity of the FileList module.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] QtyMax max item quantity
 * @return  T_BOOL
 * @retval  AK_TRUE set max item quantity successful
 * @retval  AK_FALSE set max item quantity error
 */
static T_BOOL FileList_SetMaxQty(T_FILELIST *pFileList, T_U32 QtyMax);

/**
 * @brief   set FileList module refresh flag.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] RefreshFlag FileList module refresh flag
 * @return  T_BOOL
 * @retval  AK_TRUE set FileList module refresh flag successful
 * @retval  AK_FALSE set FileList module refresh flag error
 */
static T_BOOL FileList_SetRefresh(T_FILELIST *pFileList, T_U32 RefreshFlag);


/**
 * @brief   selecte a system define sort mode to sort item.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] SortMode a system sort mode
 * @return  T_BOOL
 * @retval  AK_TRUE set a system sort mode successful
 * @retval  AK_FALSE set a system sort mode error
 */
static T_BOOL FileList_SetSysDefSortMode(T_FILELIST *pFileList, T_FILELIST_SORTMODE SortMode);


/**
 * @brief   sort item by the sort method.
 *
 * @author  lizhuobin
 * @date    2006-07-26
 * @param   [in] pFileList FileList struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE sort item successful
 * @retval  AK_FALSE sort item error
 */
static T_BOOL FileList_SortItem(T_FILELIST *pFileList);


/**
 * @brief   Set FileList Change flag.
 *
 * @author  lizhuobin
 * @date    2006-08-23
 * @param   [in] pFileList FileList struct pointer
 * @param   [in] save:  AK_TRUE:The filelist is changed and should be save at exit audioplayer;
                        AK_FALSE: The filelist is not changedand not need to save at exit audioplayer
 * @return  T_BOOL
 * @retval  AK_TRUE set success
 * @retval  AK_FALSE set error
 */
static T_BOOL FileList_SetChangeFlag(T_FILELIST *pFileList, T_BOOL change);

static T_FILELIST_ADD_RET FileList_AddItemToChain(T_FILELIST *pFileList, const T_U16 *FilePathBuf);
static T_FILELIST_ADD_RET FileList_AddList(T_FILELIST *pFileList, T_USTR_FILE ListPath);
static T_FILELIST_ADD_RET FileList_AddFolder(T_FILELIST *pFileList, T_USTR_FILE FolderPath, T_FILELIST_SEARCH_SUB_MODE SearchSubMode, T_PFILE FindParent);
static T_FILELIST_ADD_RET FileList_AddItem(T_FILELIST *pFileList, T_USTR_FILE FilePath);
static T_BOOL FileList_SortCompareId(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2);
static T_BOOL FileList_SortCompareName(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2);
static T_BOOL FileList_SortComparePath(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2);
static T_BOOL FileList_SortEmpty(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2); // use for sort reverse
static T_BOOL FileList_SortRandom(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2);
static T_BOOL FileList_SwapItemContent(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2);
static T_BOOL FileList_SwapItem(T_FILELIST *pFileList, T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2);
static T_BOOL FileList_CheckScrollBar(T_FILELIST *pFileList);
static T_BOOL FileList_CheckShowFocus(T_FILELIST *pFileList);

// init , the space of file list is malloc outside
// import the file list rect , the sort mode , the import mode , the export mode
T_BOOL FileList_Init(T_FILELIST *pFileList, T_U32 ItemQtyMax, T_FILELIST_SORTMODE SortMode,  T_fFILELIST_SUPPORT_CALLBACK SupportCallBack)
{
    T_U32 len;

    AK_ASSERT_PTR(pFileList, "FileList_Init(): pFileList", AK_FALSE);

    memset (pFileList, 0x0, sizeof(T_FILELIST));    //memset pFileList

    pFileList->pItemHead 		= AK_NULL;
    pFileList->pItemShow 		= AK_NULL;
    pFileList->pItemFocus 		= AK_NULL;
    pFileList->pItemOldFocus 	= AK_NULL;
    pFileList->ItemQty 			= 0;

    //pFileList->TitleShowFlag = TitleShowFlag;
    pFileList->FrameWidth = FILELIST_FRAMEWIDTH;  //set item frame width
    pFileList->ItemVInterval = FILELIST_ITEM_INTERVAL;
    pFileList->ItemHeight = FILELIST_ITEM_HEIGHT;

    //FileList_SetRect(pFileList, FileListRect, TitleShowFlag);      //set file list rect
    FileList_SetSysDefSortMode(pFileList, SortMode);      //set sort mode
    FileList_SetMaxQty(pFileList, ItemQtyMax);      //set file list max item num
    FileList_SetFetchMode(pFileList, FILELIST_FETCH_SEQUENCE);  //set default fetch mode
    pFileList->SupportCallBack = SupportCallBack;

    //set scroll bar
    //FILELIST_ScrollbarInit(pFileList, pFileList->ItemRect);
    //set back color, text color, focus text color, focus back color
    pFileList->ItemBackColor = FILELIST_BACKCOLOR;
    pFileList->TextColor = FILELIST_TEXTCOLOR;
    pFileList->FocusBackColor = FILELIST_FOCUSBACKCOLOR;
    pFileList->FocusTextColor = FILELIST_FOCUSTEXTCOLOR;
    pFileList->ItemTextOffset = 0;              //set scroll text offset
    pFileList->TitleTextOffset = 0;

    //set fresh flag and list flag
    pFileList->RefreshFlag = FILELIST_REFRESH_ALL;
    pFileList->ListFlag = AK_TRUE;

    FileList_SetChangeFlag(pFileList, AK_FALSE);

    pFileList->pFocusBckgrnd = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MENU_FOCUS_BACKGROUND, &len);
    AK_ASSERT_PTR(pFileList->pFocusBckgrnd, "FileList_Init(): pFileList->pFocusBckgrnd null", AK_FALSE);

    return AK_TRUE;
}

// detroy the file list module and exit
T_BOOL FileList_Free(T_FILELIST *pFileList)
{
    AK_ASSERT_PTR(pFileList, "FileList_Free(): pFileList", AK_FALSE);

    // free the item list
    FileList_DelALL(pFileList);

    return AK_TRUE;
}

T_FILELIST_ADD_RET FileList_Add(T_FILELIST *pFileList, T_USTR_FILE ImportPath, T_FILELIST_SEARCH_SUB_MODE SearchSubMode)
{
    T_USTR_FILE         name, ext;
    T_FILELIST_ADD_RET  ret = FILELIST_ADD_ERROR;
	T_PFILE  FindParent;

    AK_ASSERT_PTR(pFileList, "FileList_Add(): pFileList", AK_FALSE);
    AK_ASSERT_PTR(ImportPath, "FileList_Add(): ImportPath", AK_FALSE);

	if (Utl_UStrLen(ImportPath) > MAX_FILENM_LEN)
    {
        Fwl_Print(C3, M_CTRL, "FileList_Add ImportPath too long\r\n");
        return FILELIST_ADD_ERROR;
    }

    if (Fwl_FsIsDir(ImportPath))
    {
    	// add a folder files to the file list
		
		FindParent = File_OpenUnicode(AK_NULL, ImportPath, FILE_MODE_READ);
			
        ret = FileList_AddFolder(pFileList, ImportPath, SearchSubMode,FindParent);

		File_Close(FindParent);
    }
    else
    {
        T_USTR_FILE ustr_1, ustr_2;
        Eng_StrMbcs2Ucs(FILELIST_AUDIOLIST_EXT, ustr_1);
        Eng_StrMbcs2Ucs(FILELIST_VIDEOLIST_EXT, ustr_2);

        Utl_USplitFileName(ImportPath, name, ext);
        Utl_UStrLower(ext);

        if (0 == Utl_UStrCmp(ext, ustr_1) || 0 == Utl_UStrCmp(ext, ustr_2))
        {
            // add the files in the file list file to file list
            ret = FileList_AddList(pFileList, ImportPath);
        }
        else
        {
            // add one audio file to file list
            ret = FileList_AddItem(pFileList, ImportPath);
        }
    }

    if (FILELIST_SORT_NONE != pFileList->SortMethod.SortMode && AK_NULL != pFileList->SortMethod.SortCompareCallBack)
    {
        FileList_SortItem(pFileList);
    }

    if (pFileList->pItemShow == AK_NULL)
    {
        pFileList->pItemShow = pFileList->pItemHead;
    }
    // modify old focus, focus, item quantity
    if (pFileList->pItemFocus == AK_NULL)
    {
        pFileList->pItemOldFocus = pFileList->pItemFocus;
        pFileList->pItemFocus = pFileList->pItemHead;
        FileList_CheckShowFocus(pFileList);
    }

    FileList_CheckScrollBar(pFileList);
    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ALL);

    if (FILELIST_ADD_SUCCESS == ret)
        FileList_SetChangeFlag(pFileList, AK_TRUE);

	return ret;
}

// read filelist to iconexplorer

T_FILELIST_ADD_RET FileList_Add_Alt(T_FILELIST *pFileList, T_USTR_FILE ImportPath, T_FILELIST_SEARCH_SUB_MODE SearchSubMode)
{
    T_FILELIST_ADD_RET  ret = FILELIST_ADD_ERROR;
	T_PFILE  FindParent;
    AK_ASSERT_PTR(pFileList, "FileList_Add(): pFileList", AK_FALSE);
    AK_ASSERT_PTR(ImportPath, "FileList_Add(): ImportPath", AK_FALSE);

    if (Utl_UStrLen(ImportPath) > MAX_FILENM_LEN)
    {
        Fwl_Print(C3, M_CTRL, "FileList_Add ImportPath too long\r\n");
        return FILELIST_ADD_ERROR;
    }

    if (Fwl_FsIsDir(ImportPath))
    {
        // add a folder files to the file list
		FindParent = File_OpenUnicode(AK_NULL, ImportPath, FILE_MODE_READ);
        ret = FileList_AddFolder(pFileList, ImportPath, SearchSubMode, FindParent);
		File_Close(FindParent);
    }
   else
    {
                ret = FileList_AddItem(pFileList, ImportPath);
   //     }
    }

    if (FILELIST_SORT_NONE != pFileList->SortMethod.SortMode && AK_NULL != pFileList->SortMethod.SortCompareCallBack)
    {
        FileList_SortItem(pFileList);
    }

    if (pFileList->pItemShow == AK_NULL)
    {
        pFileList->pItemShow = pFileList->pItemHead;
    }
    // modify old focus, focus, item quantity
    if (pFileList->pItemFocus == AK_NULL)
    {
        pFileList->pItemOldFocus = pFileList->pItemFocus;
        pFileList->pItemFocus = pFileList->pItemHead;
        FileList_CheckShowFocus(pFileList);
    }

    FileList_CheckScrollBar(pFileList);
    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ALL);

    if (ret)
        FileList_SetChangeFlag(pFileList, AK_TRUE);

    return ret;
}

T_BOOL FileList_ToIconExplorer(T_FILELIST *pFileList, T_ICONEXPLORER *pIconExplorer, T_eINDEX_TYPE type, T_USTR_FILE path)
{
    T_FILELIST_ITEM *p = AK_NULL;
    T_INDEX_CONTENT pcontent;

    AK_ASSERT_PTR(pFileList, "FileList_ToIconExplorer(): pFileList", AK_FALSE);
    AK_ASSERT_PTR(pIconExplorer, "FileList_ToIconExplorer(): pIconExplorer", AK_FALSE);

    p = pFileList->pItemHead;
    while (p != AK_NULL)
    {
    	pcontent.type = type;
    	pcontent.id = MList_GetPathId(p->pFilePath, pcontent.type);

    	if (AK_NULL != path)
    	{
			Utl_UStrCpyN(path, p->pFilePath, sizeof(T_USTR_FILE)/2);
    	}
    	
        IconExplorer_AddItem(pIconExplorer, p->ID, &pcontent, sizeof(T_INDEX_CONTENT), \
                p->pText, AK_NULL, AK_NULL);

        p = p->pNext;
    }

	IconExplorer_SetListFlag(pIconExplorer);

    return AK_TRUE;
}

// read iconexplorer to filelist
T_FILELIST_ADD_RET FileList_FromIconExplorer(T_FILELIST *pFileList, T_ICONEXPLORER *pIconExplorer)
{
    T_ICONEXPLORER_ITEM *p = AK_NULL;
    T_FILELIST_ADD_RET  ret;
    T_USTR_FILE			path = {0};
    T_INDEX_CONTENT		*pcontent = AK_NULL;

    ret = FILELIST_ADD_ERROR;

    AK_ASSERT_PTR(pFileList, "FileList_FromIconExplorer(): pFileList", AK_FALSE);
    AK_ASSERT_PTR(pIconExplorer, "FileList_FromIconExplorer(): pIconExplorer", AK_FALSE);

    p = pIconExplorer->pItemHead;
    while (p != AK_NULL)
    {
    	pcontent = (T_INDEX_CONTENT *)p->pContent;

    	if (AK_NULL != pcontent)
    	{
    		memset(path, 0, sizeof(T_USTR_FILE));
	    	MList_GetItem(path, pcontent->id, pcontent->type);

	    	if (0 != path[0])
	    	{
		        ret = FileList_AddItem(pFileList, path);
		        if (FILELIST_ADD_SUCCESS != ret)
		        {
		            break;
		        }
	        }
        }
        p = p->pNext;
    }

    return ret;
}


// save file list
T_BOOL FileList_SaveFileList(T_FILELIST *pFileList, T_pCWSTR ListPath)
{
    T_pFILE fp;
    T_FILELIST_ITEM *p;
    T_USTR_FILE tmpstr;
    T_U32 num = 0;
    T_U64_INT freeSize = {0};
    T_U16* pTmpBuf = AK_NULL;
	T_U16 i = 0;
	T_U32 cnt = 0;
	T_U32 charQty = 0;

    AK_ASSERT_PTR(pFileList, "FileList_SaveFileList(): pFileList", AK_FALSE);
    AK_ASSERT_PTR(ListPath, "FileList_SaveFileList(): ListPath", AK_FALSE);
    AK_ASSERT_VAL(*ListPath, "FileList_SaveFileList(): *ListPath", AK_FALSE);

    Fwl_FsGetFreeSize(ListPath[0], &freeSize);
    if ((freeSize.low < SAVELIST_MINIMAL_SPACE) && (freeSize.high < 1))
    {
        return AK_FALSE;
    }

    p = pFileList->pItemHead;
	
    while (p != AK_NULL)
    {
        num++;
        p = p->pNext;
    }

    p = pFileList->pItemHead;
    Eng_StrMbcs2Ucs(FILELIST_SEPARATOR_STR, tmpstr);

	pTmpBuf = (T_U16 *)Fwl_Malloc(FILELIST_WRITE_FILE_NUM*sizeof(T_USTR_FILE));// + (50 << 10));
	AK_ASSERT_PTR(pTmpBuf, "FileList_SaveFileList(): Malloc Fail", AK_FALSE);
		
    fp = Fwl_FileOpen(ListPath, _FMODE_CREATE, _FMODE_CREATE);
    if (_FOPEN_FAIL == fp)
    {
        return AK_FALSE;
    }

	// Get how many times need to write.
	num = num / FILELIST_WRITE_FILE_NUM + 1; 

	for (i = 0; i < num; i++)
	{
		memset(pTmpBuf, 0, FILELIST_WRITE_FILE_NUM*sizeof(T_USTR_FILE));// + (50 << 10));

		while (cnt++ < FILELIST_WRITE_FILE_NUM)
		{
	        if (p != AK_NULL && p->pFilePath)
      		{
                Utl_UStrCat(pTmpBuf, p->pFilePath);
                Utl_UStrCat(pTmpBuf, tmpstr);
                p = p->pNext;
       		}
		}
		
		cnt = 0;
		charQty = Utl_UStrLen(pTmpBuf);
        Fwl_FileWrite(fp, (T_U8 *)pTmpBuf, charQty << 1);
    }
	
    pTmpBuf = Fwl_Free(pTmpBuf);
	Fwl_FileClose(fp);
	
    return AK_TRUE;
}


/****************************************************************/
/**********                 Internal     Function            ************************/
/****************************************************************/

// set file list fetch mode
static T_BOOL FileList_SetFetchMode(T_FILELIST *pFileList, T_FILELIST_FETCHMODE FetchMode)
{
    AK_ASSERT_PTR(pFileList, "FileList_SetFetchMode(): pFileList", AK_FALSE);

    pFileList->FetchMode = FetchMode;

    return AK_TRUE;
}

static T_BOOL FileList_DelALL(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p, *q;

    AK_ASSERT_PTR(pFileList, "FileList_DelALL(): pFileList", AK_FALSE);

    p = pFileList->pItemHead;
    while (p != AK_NULL)
    {
        q = p->pNext;
        p->pFilePath = Fwl_Free(p->pFilePath);
        p = Fwl_Free(p);
        p = q;
    }
    pFileList->pItemHead = AK_NULL;
    pFileList->pItemShow = AK_NULL;
    pFileList->pItemTail = AK_NULL;
    pFileList->pItemFocus = AK_NULL;
    pFileList->pItemOldFocus = AK_NULL;
    pFileList->ItemQty = 0;

    FileList_CheckScrollBar(pFileList);
    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ALL);
    FileList_SetChangeFlag(pFileList, AK_TRUE);

    return AK_TRUE;
}

// parse the file path string and add to the file list chain
static T_FILELIST_ADD_RET FileList_AddItemToChain(T_FILELIST *pFileList, const T_U16 *FilePath)
{
    T_FILELIST_ADD_RET ret = FILELIST_ADD_SUCCESS;
    T_FILELIST_ITEM *p = AK_NULL;
    T_FILELIST_ITEM *pTmp = AK_NULL;
    T_USTR_FILE     text;
    T_USTR_FILE     path;
    T_U32 TextChNum = 0;
    T_U32 FilePahChNum = 0;

    AK_ASSERT_PTR(pFileList, "FileList_AddItemToChain(): pFileList", FILELIST_ADD_ERROR);
    AK_ASSERT_PTR(FilePath, "FileList_AddItemToChain(): FilePathBuf", FILELIST_ADD_ERROR);
    AK_ASSERT_VAL(*FilePath, "FileList_AddItemToChain(): *FilePathBuf", FILELIST_ADD_ERROR);

    //Fwl_Print(C3, M_CTRL, "FileList_AddItemToChain pFileList->ItemQty = %d\r\n", pFileList->ItemQty);

    if (pFileList->ItemQty > pFileList->ItemQtyMax)
    {
        return FILELIST_ADD_NOSPACE;
    }

    if (AK_NULL == pFileList->SupportCallBack || pFileList->SupportCallBack(FilePath))
    {
        p = (T_FILELIST_ITEM *)Fwl_Malloc(sizeof(T_FILELIST_ITEM));
        if (p == AK_NULL)
            return FILELIST_ADD_NOSPACE;
		
        p->ID = ++(pFileList->ItemQty);

        // creat file path
        FilePahChNum = Utl_UStrLen(FilePath);
        p->pFilePath = (T_U16 *)Fwl_Malloc((FilePahChNum + 1)*2);

		if (p->pFilePath == AK_NULL)
		{
			p = Fwl_Free(p);
            return FILELIST_ADD_NOSPACE;
		}
		
        Utl_UStrCpy(p->pFilePath, FilePath);

        // get text pointer
        Utl_USplitFilePath(FilePath, path, text);
        TextChNum = Utl_UStrLen(text);
        p->pText = p->pFilePath + (FilePahChNum - TextChNum);
        //strcpy(p->Text, text);

        // add item to chain
        if (AK_NULL == pFileList->pItemHead)
        {
            p->pPrevious = AK_NULL;
            p->pNext = AK_NULL;
            pFileList->pItemHead = p;
            pFileList->pItemTail = p;
            pFileList->pItemFocus = p;
            pFileList->pItemShow = p;
            pFileList->pItemOldFocus = p;
        }
        else if (AK_NULL != pFileList->pItemTail)
        {
            pTmp = pFileList->pItemTail;
            pTmp->pNext = p;
            p->pPrevious = pTmp;
            p->pNext = AK_NULL;
            pFileList->pItemTail = p;
        }
        else
        {
            Fwl_Print(C3, M_CTRL, "FILELIST ERROR\r\n");
            p = Fwl_Free(p);
            return FILELIST_ADD_ERROR;
        }
        ret = FILELIST_ADD_SUCCESS;
    }

    
    //else
    //{
        //unsurportted type
    //    ret = FILELIST_ADD_ERROR;
    //}

    return ret;
}

static T_FILELIST_ADD_RET FileList_AddList(T_FILELIST *pFileList, T_USTR_FILE ListPath)
{
    T_pFILE fp;
    T_U32   fsize;
    T_U16   *buf;
    T_U16   *pCurBuf;
    T_U16   *pFirst;
    T_FILELIST_ADD_RET ret = FILELIST_ADD_SUCCESS;
    T_USTR_FILE FilePath;
    T_U32       count;

    AK_ASSERT_PTR(pFileList, "FileList_AddList(): pFileList", FILELIST_ADD_ERROR);
    AK_ASSERT_PTR(ListPath, "FileList_AddList(): ListPath", FILELIST_ADD_ERROR);
    AK_ASSERT_VAL(*ListPath, "FileList_AddList(): *ListPath", FILELIST_ADD_ERROR);
    AK_ASSERT_VAL((Utl_UStrLen(ListPath) <= (MAX_FILENM_LEN)), "FileList_AddList(): ListPath too long", FILELIST_ADD_ERROR);


    if (0 < (fsize = Fwl_FileGetSize(ListPath)))
    {
        if (_FOPEN_FAIL != (fp = Fwl_FileOpen(ListPath, _FMODE_READ, _FMODE_READ)))
        {
			buf = (T_U16 *)Fwl_Malloc(fsize + 2);
			if (AK_NULL != buf)
            {
                Fwl_FileRead(fp, (T_U8 *)buf, fsize);
				
                *(buf + (fsize >> 1)) = 0;
                count = 0;
                ret  = FILELIST_ADD_SUCCESS;
				
                pCurBuf = buf;
                pFirst = pCurBuf;
				
                while (*pCurBuf != 0 && ret == FILELIST_ADD_SUCCESS)
                {                
                    if (++count > MAX_FILENM_LEN + 1)
                    {
                        break;
                    }

					// Find ";"
                    if (*pCurBuf == UNICODE_SEPARATOR)
                    {
                        Utl_UStrCpyN(FilePath, pFirst, count-1);
                        FilePath[count - 1] = 0;
                        ret = FileList_AddItemToChain(pFileList, FilePath);
						
                        count = 0;
                        pFirst = pCurBuf+1;
                    }
                    ++pCurBuf;
                }

                buf = Fwl_Free(buf);
            }
            else
            {
            	ret = FILELIST_ADD_NOSPACE;
            }
            Fwl_FileClose(fp);
        }
        else
        {
        	ret = FILELIST_ADD_ERROR;
        }
    }
    else
    {
    	HJH_DEBUG("+++++++++++++++++++++++++++++++\n");
    	HJH_DEBUG("Fwl_FileGetSize(ListPath)) = 0\n");
    	ret = FILELIST_ADD_NONE;
    }

    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ITEM);
    return ret;
}

static T_FILELIST_ADD_RET FileList_AddFolder(T_FILELIST *pFileList, T_USTR_FILE FolderPath, T_FILELIST_SEARCH_SUB_MODE SearchSubMode, T_PFILE FindParent)
{
	
    //T_FILELIST *p;
    //T_STR_FILE folder, path, name, ext;
    T_FILELIST_ADD_RET ret = FILELIST_ADD_ERROR;
    T_FILELIST_ADD_RET ever_ret = FILELIST_ADD_ERROR;
    //T_FOLDERLIST *pFolder = AK_NULL, *f, *g;
    T_FILE_INFO FileInfo;
    T_USTR_FILE TmpPath;
    T_USTR_FILE FindPath;
	T_U32         FindHandle;
	T_PFILEINFO   info;
	T_PFILE       file;

    AK_ASSERT_PTR(pFileList, "FileList_AddFolder(): pFileList", FILELIST_ADD_ERROR);
    AK_ASSERT_PTR(FolderPath, "FileList_AddFolder(): FolderPath", FILELIST_ADD_ERROR);
    AK_ASSERT_VAL(*FolderPath, "FileList_AddFolder(): *FolderPath", FILELIST_ADD_ERROR);
    //AK_ASSERT_VAL(Fwl_FsIsDir(FolderPath), "FileList_AddFolder(): Fwl_FsIsDir(FolderPath)", FILELIST_ADD_ERROR);

	if (Utl_CaclSolidas(FolderPath) > MAX_PATH_DEEP)
    {
		return FILELIST_ADD_OUTPATHDEEP;
    }
	
	// find a folder
    if (Utl_UStrLen(FolderPath) < (MAX_FILENM_LEN - 5))
    {
        Utl_UStrCpy(FindPath, FolderPath);

        ret = FILELIST_ADD_SUCCESS;
		
        if (FILELIST_NO_SEARCH_SUB_RECODE_FOLDER == SearchSubMode || FILELIST_SEARCH_SUB_RECODE_FOLDER == SearchSubMode)
        {
            if (FILELIST_ADD_SUCCESS != (ret = FileList_AddItem(pFileList, FindPath)))
                return ret;

        }

        if (FindPath[Utl_UStrLen(FindPath) - 1] == UNICODE_SOLIDUS)
        {
            FindPath[Utl_UStrLen(FindPath) - 1] = 0;
        }

        Utl_UStrCat(FindPath, _T("/*.*"));

		FindHandle = Fwl_FileFindFirstFromHandle((T_U32)FindParent);

        if (0 != FindHandle)
        {
            ret = FILELIST_ADD_ERROR;
			
            do
            {
            	info = (T_PFILEINFO)Fwl_FsFindInfo(&FileInfo,(T_hFILESTAT)FindHandle);
				if (AK_NULL == info)
				{
					break;
				}
         		file = (T_PFILE)Fwl_FileFindOpen((T_U32)FindParent, (T_U32)info);
#if 1				
				// Path Is Too Longer
				if (Utl_UStrLen(FolderPath) + Utl_UStrLen(FileInfo.name) >= sizeof(T_STR_FILE) - 2)
				{
					continue;
				}

				Utl_UStrCpy(TmpPath, FolderPath);					
            	if (TmpPath[Utl_UStrLen(TmpPath) - 1] != UNICODE_SOLIDUS)
            	{
                	Utl_UStrCat(TmpPath, _T("/"));
            	}
#endif	
				Utl_UStrCat(TmpPath, FileInfo.name);
				
				// Is A Directory
                if ((FileInfo.attrib & 0x10) == 0x10)
                {
					// folder is "." or "..", continue
                	if (0 == Utl_UStrCmp(FileInfo.name, _T(".")) || 0 == Utl_UStrCmp(FileInfo.name, _T("..")))
                	{
						continue;                    
                	}
					// Search Sub Directory
					if (FILELIST_SEARCH_SUB_NO_RECODE_FOLDER == SearchSubMode || FILELIST_SEARCH_SUB_RECODE_FOLDER == SearchSubMode)
                    {					
                        ret = FileList_AddFolder(pFileList, TmpPath, SearchSubMode,file);
                    }
					// Record Folder
					else if (FILELIST_NO_SEARCH_SUB_RECODE_FOLDER == SearchSubMode)
                    {						
                        ret = FileList_AddItem(pFileList, TmpPath);
						continue;
                    }
					// FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER
                    else
                    {
                        continue;
                    }                    
                }
				// Is A File
                else
                {
                    ret = FileList_AddItem(pFileList, TmpPath);					
                }

                //if ever once add successfully, then this function should return SUCCESS
                if (FILELIST_ADD_SUCCESS == ret)
                {
                    ever_ret = FILELIST_ADD_SUCCESS;
                }
                else if (FILELIST_ADD_OUTPATHDEEP == ret)   
				{
				    ever_ret = FILELIST_ADD_OUTPATHDEEP;		// xwz
				}
                else if (FILELIST_ADD_NOSPACE == ret)   
				{
				    /* Added By XieWenzhong in May 19, 2010 */
					ever_ret = FILELIST_ADD_NOSPACE;
					break;
				}
                Fwl_FileClose((T_pFILE)file);
		    }while(Fwl_FsFindNext((T_hFILESTAT)FindHandle) != AK_FALSE);
		    Fwl_FileFindCloseWithHandle(FindHandle);
            ret = ever_ret;
        }
        else
        {
            //ret = FILELIST_ADD_NONE;

            //使用多任务的文件系统后，不再在FindFirst()和FindNext()里搜索上级目录和当前目录。
            //因此空目录里Find时也是返回 FS_INVALID_STATHANDLE.
            ret = FILELIST_ADD_SUCCESS;
        }
    }

	FileList_SetRefresh(pFileList, FILELIST_REFRESH_ITEM);
    return ret;
}

static T_FILELIST_ADD_RET FileList_AddItem(T_FILELIST *pFileList, T_USTR_FILE FilePath)
{
    T_FILELIST_ADD_RET ret = FILELIST_ADD_ERROR;

    AK_ASSERT_PTR(pFileList, "FileList_AddItem(): pFileList", FILELIST_ADD_ERROR);
    AK_ASSERT_PTR(FilePath, "FileList_AddItem(): FilePath", FILELIST_ADD_ERROR);
    AK_ASSERT_VAL(*FilePath, "FileList_AddItem(): *FilePath", FILELIST_ADD_ERROR);
    AK_ASSERT_VAL((Utl_UStrLen(FilePath) <= (MAX_FILENM_LEN)), "FileList_AddItem(): FilePath too long", FILELIST_ADD_ERROR);

    ret = FileList_AddItemToChain(pFileList, FilePath);
    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ITEM);
    return ret;
}

static T_BOOL FileList_SetSysDefSortMode(T_FILELIST *pFileList, T_FILELIST_SORTMODE SortMode)
{
    AK_ASSERT_PTR(pFileList, "FileList_SetSysDefSortMode(): pFileList", AK_FALSE);
    AK_ASSERT_VAL((SortMode < FILELIST_SORT_USER_DEFINE), "FileList_SetSysDefSortMode(): not sys def sort mode", AK_FALSE);

    if (SortMode >= FILELIST_SORT_USER_DEFINE)
    {
        pFileList->SortMethod.SortMode = FILELIST_SORT_NONE;
        pFileList->SortMethod.SortCompareCallBack = AK_NULL;
    }
    else
    {
        pFileList->SortMethod.SortMode = SortMode;
        switch (SortMode)
        {
        case FILELIST_SORT_NONE:
            pFileList->SortMethod.SortCompareCallBack = AK_NULL;
            break;
			
        case FILELIST_SORT_ID:
            pFileList->SortMethod.SortCompareCallBack = (T_fFILELIST_COMPARE_CALLBACK)FileList_SortCompareId;
            break;
			
        case FILELIST_SORT_NAME:
            pFileList->SortMethod.SortCompareCallBack = (T_fFILELIST_COMPARE_CALLBACK)FileList_SortCompareName;
            break;
			
        case FILELIST_SORT_PATH:
            pFileList->SortMethod.SortCompareCallBack = (T_fFILELIST_COMPARE_CALLBACK)FileList_SortComparePath;
            break;
			
        case FILELIST_SORT_REVERSE:
            pFileList->SortMethod.SortCompareCallBack = (T_fFILELIST_COMPARE_CALLBACK)FileList_SortEmpty;
            break;
			
        case FILELIST_SORT_RANDOM:
            pFileList->SortMethod.SortCompareCallBack = (T_fFILELIST_COMPARE_CALLBACK)FileList_SortRandom;
            break;
			
        default:
            pFileList->SortMethod.SortMode = FILELIST_SORT_NONE;
            pFileList->SortMethod.SortCompareCallBack = AK_NULL;
            break;
        }
    }

    return AK_TRUE;
}

static T_BOOL FileList_SortCompareId(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2)
{
    AK_ASSERT_PTR(pItem1, "FileList_SortCompareId(): pItem1", AK_FALSE);
    AK_ASSERT_PTR(pItem2, "FileList_SortCompareId(): pItem2", AK_FALSE);

    if(pItem1->ID > pItem2->ID)
        return AK_TRUE;

   return AK_FALSE;
}

static T_BOOL FileList_SortCompareName(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2)
{
    AK_ASSERT_PTR(pItem1, "FileList_SortCompareName(): pItem1", AK_FALSE);
    AK_ASSERT_PTR(pItem2, "FileList_SortCompareName(): pItem2", AK_FALSE);
    AK_ASSERT_PTR(pItem1->pText, "FileList_SortCompareName(): pItem1->Text", AK_FALSE);
    AK_ASSERT_PTR(pItem2->pText, "FileList_SortCompareName(): pItem2->Text", AK_FALSE);

    if (Utl_UStrCmp(pItem1->pText, pItem2->pText) > 0)
            return AK_TRUE;

    return AK_FALSE;
}

static T_BOOL FileList_SortComparePath(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2)
{
    T_U16    disk_char1, disk_char2;

    AK_ASSERT_PTR(pItem1, "FileList_SortComparePath(): pItem1", AK_FALSE);
    AK_ASSERT_PTR(pItem2, "FileList_SortComparePath(): pItem2", AK_FALSE);
    AK_ASSERT_PTR(pItem1->pFilePath, "FileList_SortComparePath(): pItem1->FilePath", AK_FALSE);
    AK_ASSERT_PTR(pItem2->pFilePath, "FileList_SortComparePath(): pItem2->FilePath", AK_FALSE);

    disk_char1 = pItem1->pFilePath[0];
    disk_char2 = pItem2->pFilePath[0];

    if(UNICODE_A <= disk_char1 && disk_char1 <= UNICODE_Z)
        disk_char1 += 0x20;
    if(UNICODE_A <= disk_char2 && disk_char2 <= UNICODE_Z)
        disk_char2 += 0x20;

    if (disk_char1 > disk_char2)
        return AK_TRUE;

    if (Utl_UStrCmp(pItem1->pFilePath, pItem2->pFilePath) > 0)
            return AK_TRUE;

    return AK_FALSE;
}

static T_BOOL FileList_SortEmpty(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2)
{
    AK_ASSERT_PTR(pItem1, "FileList_SortEmpty(): pItem1", AK_FALSE);
    AK_ASSERT_PTR(pItem2, "FileList_SortEmpty(): pItem2", AK_FALSE);

    return AK_TRUE;
}

static T_BOOL FileList_SortRandom(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2)
{
    T_BOOL ret;

    AK_ASSERT_PTR(pItem1, "FileList_SortRandom(): pItem1", AK_FALSE);
    AK_ASSERT_PTR(pItem2, "FileList_SortRandom(): pItem2", AK_FALSE);

    ret = (T_BOOL)Fwl_GetRand(2);

    return ret;
}

static T_BOOL FileList_SortReverse(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p, *q, *r;
    T_FILELIST_ITEM *pTmp;

    AK_ASSERT_PTR(pFileList, "FileList_SortReverse(): pFileList", AK_FALSE);

    if (pFileList->pItemHead == AK_NULL)
        return AK_FALSE;



    p = pFileList->pItemHead;
    q = pFileList->pItemTail;

    r = pFileList->pItemHead;
    while (r != AK_NULL)
    {
        // reverse pNext and pPrevious
        pTmp = r->pNext;
        r->pNext = r->pPrevious;
        r->pPrevious = pTmp;

        // get next item
        r = pTmp;
    }
    pFileList->pItemHead = q;
    pFileList->pItemTail = p;

    return AK_TRUE;
}

// the file list by the sortmode
// sort all item method
static T_BOOL FileList_SortItem(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p, *q;
    T_BOOL done;

    AK_ASSERT_PTR(pFileList, "FileList_SortItem(): pFileList", AK_FALSE);

    if (pFileList->pItemHead == AK_NULL)
        return AK_FALSE;

    // base on sort mode
    if (FILELIST_SORT_NONE == pFileList->SortMethod.SortMode \
        || AK_NULL == pFileList->SortMethod.SortCompareCallBack)
        return AK_FALSE;

    if (FILELIST_SORT_REVERSE == pFileList->SortMethod.SortMode)
    {
        FileList_SortReverse(pFileList);
    }
    else
    {
        p = pFileList->pItemTail;
        // sort all item
        done = AK_FALSE;
        while ((!done) && (p != AK_NULL))
        {
            done = AK_TRUE;
            q = pFileList->pItemHead;
            while (q != p)
            {
                if (pFileList->SortMethod.SortCompareCallBack(q, q->pNext) == AK_TRUE)
                {
                    done = AK_FALSE;
                    FileList_SwapItem(pFileList, q, q->pNext);
                }
                q = q->pNext;
            }
            p = p->pPrevious;
        }
    }

    // modify old focus & focus point
    pFileList->pItemOldFocus = pFileList->pItemFocus;
    pFileList->pItemFocus = pFileList->pItemHead;

    FileList_CheckShowFocus(pFileList);
    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ALL);

    return AK_TRUE;
}

static T_BOOL FileList_SwapItemContent(T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2)
{
    T_U32 Id;
    T_pWSTR                  pText;
    T_pWSTR                  pFilePath;

    AK_ASSERT_PTR(pItem1, "FileList_SwapItemContent(): pItem1", AK_FALSE);
    AK_ASSERT_PTR(pItem2, "FileList_SwapItemContent(): pItem2", AK_FALSE);

    Id = pItem1->ID;
    pFilePath = pItem1->pFilePath;
    //strcpy(FilePath, pItem1->FilePath);
    pText = pItem1->pText;
    //strcpy(text, pItem1->Text);

    pItem1->ID = pItem2->ID;
    pItem1->pFilePath = pItem2->pFilePath;
    //strcpy(pItem1->FilePath, pItem2->FilePath);
    pItem1->pText = pItem2->pText;
    //strcpy(pItem1->Text, pItem2->Text);

    pItem2->ID = Id;
    pItem2->pFilePath = pFilePath;
    //strcpy(pItem2->FilePath, FilePath);
    pItem2->pText = pText;
    //strcpy(pItem2->Text, text);

    return AK_TRUE;
}

static T_BOOL FileList_SwapItem(T_FILELIST *pFileList, T_FILELIST_ITEM *pItem1, T_FILELIST_ITEM *pItem2)
{
    T_FILELIST_ITEM *p;
    T_BOOL Flag1 = AK_FALSE, Flag2 = AK_FALSE;

    AK_ASSERT_PTR(pFileList, "FileList_SwapItem(): pFileList", AK_FALSE);
    AK_ASSERT_PTR(pItem1, "FileList_SwapItem(): pItem1", AK_FALSE);
    AK_ASSERT_PTR(pItem2, "FileList_SwapItem(): pItem2", AK_FALSE);
    AK_ASSERT_VAL((pItem1 != pItem2), "FileList_SwapItem(): (pItem1 == pItem2)", AK_FALSE);

    // find two item
    p = pFileList->pItemHead;
    while (p != AK_NULL) {
        if (p == pItem1)
            Flag1 = AK_TRUE;
        if (p == pItem2)
            Flag2 = AK_TRUE;

        if ((Flag1 == AK_TRUE) && (Flag2 == AK_TRUE))
            break;

        p = p->pNext;
    }

    if ((Flag1 == AK_FALSE) || (Flag2 == AK_FALSE))
        return AK_FALSE;

    // swap two item content
    FileList_SwapItemContent(pItem1, pItem2);

    //IconList_CheckShowFocus(pIconExplorer);
    //IconExplorer_SetRefresh(pIconExplorer, ICONEXPLORER_REFRESH_ITEM);

    return AK_TRUE;
}


static T_BOOL FileList_SetRefresh(T_FILELIST *pFileList, T_U32 RefreshFlag)
{
    AK_ASSERT_PTR(pFileList, "FileList_SetRefresh(): pFileList", AK_FALSE);

    if (FILELIST_REFRESH_NONE != RefreshFlag)
    {
        pFileList->RefreshFlag |= RefreshFlag;
    }
    else
    {
        pFileList->RefreshFlag = RefreshFlag;
    }

    return AK_TRUE;
}


// check scroll bar show flag
static T_BOOL FileList_CheckScrollBar(T_FILELIST *pFileList)
{
    AK_ASSERT_PTR(pFileList, "FileList_CheckScrollBar(): pFileList", AK_FALSE);

    if (pFileList->ItemQty > pFileList->PageItemQty)
    {
        pFileList->ScBarShowFlag = AK_TRUE;
    }
    else
    {
        pFileList->ScBarShowFlag = AK_FALSE;
    }

    return AK_TRUE;
}

// check show item & focus item
static T_BOOL FileList_CheckShowFocus(T_FILELIST *pFileList)
{
    T_U32 i;
    T_FILELIST_ITEM *p = AK_NULL;
    T_FILELIST_ITEM *pOldItemShow = AK_NULL;
    T_U32 Qty;
    T_BOOL Show = AK_FALSE;

    AK_ASSERT_PTR(pFileList, "FileList_CheckShowFocus(): pFileList", AK_FALSE);
    if (pFileList->pItemHead == AK_NULL)
    {
        return AK_FALSE;
    }

    pOldItemShow = pFileList->pItemShow;

    for (i=0; i<pFileList->ItemQty; i++)
    {
        Qty = 0;
        p = pFileList->pItemFocus;
        while (p != AK_NULL)
        {
            Qty++;

            if (p == pFileList->pItemShow)
            {
                Show = AK_TRUE;
                break;
            }

            if (Qty >= pFileList->PageItemQty)
                break;

            p = p->pPrevious;
        }

        if (Show == AK_FALSE)
        {
            if (pFileList->pItemShow->pNext == AK_NULL)
                pFileList->pItemShow = pFileList->pItemFocus;
            else
                pFileList->pItemShow = pFileList->pItemShow->pNext;
        }
        else
        {
            Qty = 0;
            p = pFileList->pItemShow;
            while (p != AK_NULL)
            {
                Qty++;

                if (Qty >= pFileList->PageItemQty)
                    break;

                p = p->pNext;
            }

            for (; Qty<pFileList->PageItemQty; Qty++)
            {
                if (pFileList->pItemShow->pPrevious == AK_NULL)
                    break;

                pFileList->pItemShow = pFileList->pItemShow->pPrevious;
            }
            break;
        }
    }

    if (pFileList->pItemShow != pOldItemShow)
        FileList_SetRefresh(pFileList, FILELIST_REFRESH_ITEM);

    return AK_TRUE;
}


static T_BOOL FileList_SetMaxQty(T_FILELIST *pFileList, T_U32 QtyMax)
{
   AK_ASSERT_PTR(pFileList, "FileList_SetMaxQty(): pFileList", AK_FALSE);

   pFileList->ItemQtyMax = QtyMax;

   return AK_TRUE;
}

static T_BOOL FileList_SetChangeFlag(T_FILELIST *pFileList, T_BOOL change)
{
    AK_ASSERT_PTR(pFileList, "FileList_SetChangeFlag(): pFileList", AK_FALSE);

    pFileList->ChangeFlag = change;

    return AK_TRUE;
}

#if 0

static T_BOOL FileList_CheckFileType(T_U8 * pFileName, T_STR_FILE FileType);
static T_BOOL FileList_ScrollbarInit(T_FILELIST *pFileList, T_RECT ItemRect);
static T_BOOL FileList_ShowScrollBar(T_FILELIST *pFileList);
static T_BOOL FileList_CheckRect(T_RECT *Rect);

static T_BOOL FileList_RegulateWhenDelOne(T_FILELIST *pFileList, T_FILELIST_ITEM *pDelItem);
static T_BOOL FileList_ScrollItemText(T_FILELIST *pFileList);
static T_BOOL FileList_GetItemTextPos(T_FILELIST *pFileList, T_FILELIST_ITEM *pItem, T_U32 Row, T_S16 *PosX, T_S16 *PosY, T_U16 *MaxLen);
static T_BOOL FileList_ShowItem(T_FILELIST *pFileList);
static T_BOOL FileList_ShowOneItem(T_FILELIST *pFileList, T_FILELIST_ITEM *pItem, T_U32 Row, T_BOOL Focus, T_BOOL ShowBack);
static T_BOOL FileList_ShowFocus(T_FILELIST *pFileList);
static T_FILELIST_MOVEFOCUS_RET FileList_MoveUp(T_FILELIST *pFileList);
static T_FILELIST_MOVEFOCUS_RET FileList_MoveDown(T_FILELIST *pFileList);
static T_BOOL FileList_SetTitleRect(T_FILELIST *pFileList, T_RECT TitleRect);
static T_BOOL FileList_SetItemRect(T_FILELIST *pFileList, T_RECT ItemRect);
static T_pWSTR FileList_RandomFetchFilePath(T_FILELIST *pFileList, T_FILELIST_ITEM *pPlayItem);
static T_BOOL FileList_IsFileInFolder(T_pCWSTR pFilePath, T_pCWSTR pFolderPath);


static T_BOOL FileList_CheckFileType(T_U8 *pFileName, T_STR_FILE FileType)
{
    T_STR_FILE name, ext;

    if (AK_NULL == pFileName || AK_NULL == FileType)
        return AK_FALSE;

    if (strlen(FileType) == 0 || strlen(pFileName) == 0)
        return AK_FALSE;

    if (Utl_StrFnd(FileType, "*.*", 0) != -1)
        return AK_TRUE;

    SplitFileName(pFileName, name, ext);
    if (strlen(ext) == 0)
        return AK_FALSE;

    Utl_StrLower(ext);

    if (Utl_StrFnd(FileType, ext, 0) == -1)
        return AK_FALSE;

    return AK_TRUE;
}

static T_BOOL FileList_GetRes(T_FILELIST *pFileList)
{
    pDisplayList->pTitle = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_TITLE, &len);
}


static T_BOOL FileList_ScrollbarInit(T_FILELIST *pFileList, T_RECT ItemRect)
{
    AK_ASSERT_PTR(pFileList, "FileList_ScrollbarInit(): pFileList", AK_FALSE);

    pFileList->ScrollBarWidth = FILELIST_SCROLLBAR_WIDTH;
    pFileList->ScBarShowFlag = AK_FALSE;
    // set scroll bar parameter again
    ScBar_Init(&pFileList->ScrollBar, \
            (T_S16)(ItemRect.left + ItemRect.width - pFileList->FrameWidth - pFileList->ScrollBarWidth), \
            (T_S16)(ItemRect.top + pFileList->FrameWidth), \
            (T_S16)(pFileList->ScrollBarWidth), \
            (T_S16)(ItemRect.height - pFileList->FrameWidth*2), \
            -10, SCBAR_VERTICAL);

    return AK_TRUE;
}

// show scroll bar
static T_BOOL FileList_ShowScrollBar(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p;
    T_U32 Num = 0;
    //T_U32 MaxQty;

    AK_ASSERT_PTR(pFileList, "FileList_ShowScrollBar(): pFileList", AK_FALSE);
    AK_ASSERT_PTR(pFileList->pItemHead, "FileList_ShowScrollBar(): pFileList->pItemHead", AK_FALSE);
    AK_ASSERT_PTR(pFileList->pItemShow, "FileList_ShowScrollBar(): pFileList->pItemShow", AK_FALSE);

    if (AK_FALSE == pFileList->ScBarShowFlag)
        return AK_FALSE;

    p = pFileList->pItemHead;
    while (p != AK_NULL) {
        if (p == pFileList->pItemShow)
            break;

        Num++;
        p = p->pNext;
    }

    if (AK_NULL == p)
    {
        return AK_FALSE;
    }

    // set scroll bar value, show it
    ScBar_SetValue(&pFileList->ScrollBar, Num, \
            (T_U16)(pFileList->ItemQty), \
            (T_U16)(pFileList->PageItemQty));
    ScBar_Show(&pFileList->ScrollBar);

    return AK_TRUE;
}


// check rect
static T_BOOL FileList_CheckRect(T_RECT *Rect)
{
    AK_ASSERT_PTR(Rect, "FileList_CheckRect(): Rect", AK_FALSE);

    if ((Rect->width < 0) || (Rect->width > Fwl_GetLcdWidth()))
        Rect->width = Fwl_GetLcdWidth();
    if ((Rect->height < 0) || (Rect->height > Fwl_GetLcdHeight()))
        Rect->height = Fwl_GetLcdWidth();
    if (((Rect->left+Rect->width) < 0) || ((Rect->left+Rect->width) > Fwl_GetLcdWidth()))
        Rect->left = Fwl_GetLcdWidth() - Rect->width;
    if (((Rect->top+Rect->height) < 0) || ((Rect->top+Rect->height) > Fwl_GetLcdHeight()))
        Rect->top = Fwl_GetLcdHeight() - Rect->height;

    return AK_TRUE;
}

// get item text pos
static T_BOOL FileList_GetItemTextPos(T_FILELIST *pFileList, T_FILELIST_ITEM *pItem, T_U32 Row, T_S16 *PosX, T_S16 *PosY, T_U16 *MaxLen)
{
    T_S16 OffsetX = 0, OffsetY = 0;
    T_U32 ScBarShowWidth;

    AK_ASSERT_PTR(pFileList, "FileList_GetItemTextPos(): pFileList", AK_FALSE);
    AK_ASSERT_PTR(pItem->pText, "FileList_GetItemTextPos(): pItem->Text", AK_FALSE);
    AK_ASSERT_VAL((Row < pFileList->PageItemQty), "FileList_GetItemTextPos(): (Row >= pFileList->PageItemQty)", AK_FALSE);

    ScBarShowWidth = (pFileList->ScBarShowFlag) ? pFileList->ScrollBarWidth : 0;
    *MaxLen = (T_U16)((pFileList->FileListRect.width-ScBarShowWidth-\
            pFileList->FrameWidth*2-pFileList->ItemVInterval) / g_Font.CWIDTH);

    *PosX = (T_S16)(pFileList->FileListRect.left + pFileList->FrameWidth + \
            pFileList->ItemVInterval + OffsetX);

    if (pFileList->ItemHeight > (T_U32)g_Font.CHEIGHT)
        OffsetY = (T_S16)((pFileList->ItemHeight-g_Font.CHEIGHT)/2);
    *PosY = (T_S16)(pFileList->FileListRect.top + pFileList->FrameWidth + pFileList->ItemVInterval + \
            (pFileList->ItemVInterval+pFileList->ItemHeight)*Row + OffsetY);

    return AK_TRUE;
}

static T_BOOL FileList_RegulateWhenDelOne(T_FILELIST *pFileList, T_FILELIST_ITEM *pDelItem)
{
    T_FILELIST_ITEM *p;

    AK_ASSERT_PTR(pFileList, "FileList_RegulateWhenDelOne(): pFileList", AK_FALSE);
    AK_ASSERT_PTR(pDelItem, "FileList_RegulateWhenDelOne(): pDelItem", AK_FALSE);

    p = pDelItem;
    // modify after delete point
    if (p->pPrevious != AK_NULL)
        p->pPrevious->pNext = p->pNext;
    else
        pFileList->pItemHead = p->pNext;
    if (p->pNext != AK_NULL)
        p->pNext->pPrevious = p->pPrevious;
    else
        pFileList->pItemTail = p->pPrevious;

    // check item focus point
    if (p == pFileList->pItemFocus)
    {
        pFileList->pItemOldFocus = AK_NULL;
        if (p->pNext != AK_NULL)
            pFileList->pItemFocus = p->pNext;
        else
            pFileList->pItemFocus = pFileList->pItemHead;
        //else if (p->pPrevious != AK_NULL)
            //pFileList->pItemFocus = p->pPrevious;
        //else if (pFileList->pItemHead != AK_NULL)
            //pFileList->pItemFocus = pFileList->pItemHead;
        //else
            //pFileList->pItemFocus = AK_NULL;
        FileList_CheckShowFocus(pFileList);
    }
    if (p == pFileList->pItemOldFocus)
    {
        pFileList->pItemOldFocus = AK_NULL;
    }
    if (p == pFileList->pItemShow)
    {
        if (AK_NULL != pFileList->pItemShow->pNext)
            pFileList->pItemShow = pFileList->pItemShow->pNext;
        else if (AK_NULL != pFileList->pItemShow->pPrevious)
            pFileList->pItemShow = pFileList->pItemShow->pPrevious;
        else
            pFileList->pItemShow = AK_NULL;

        FileList_CheckShowFocus(pFileList);
    }

    return AK_TRUE;
}

// scroll long focus item text
static T_BOOL FileList_ScrollItemText(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p;
    T_U32 Row;
    T_RECT Rect;
    T_S16 PosX, PosY;
    T_U16 MaxLen;
    T_U32 ShowScrollBarWidth;

    AK_ASSERT_PTR(pFileList, "FileList_ScrollItemText(): pFileList", AK_FALSE);
    AK_ASSERT_PTR(pFileList->pItemFocus, "FileList_ScrollItemText(): pFileList->pItemFocus", AK_FALSE);
    AK_ASSERT_PTR(pFileList->pItemFocus->pFilePath, "FileList_ScrollItemText(): pFileList->pItemFocus->FilePath", AK_FALSE);
    AK_ASSERT_VAL(*(pFileList->pItemFocus->pFilePath), "FileList_ScrollItemText(): *(pFileList->pItemFocus->FilePath)", AK_FALSE);

    ShowScrollBarWidth = (pFileList->ScBarShowFlag) ? pFileList->ScrollBarWidth : 0;
    // check max length show char
    MaxLen = (T_U16)((pFileList->FileListRect.width-ShowScrollBarWidth-\
            pFileList->FrameWidth*2-pFileList->ItemVInterval) / \
            g_Font.CWIDTH);
    if (Utl_UStrLen(pFileList->pItemFocus->pFilePath) <= MaxLen)
        return AK_FALSE;

    // get focus item row
    p = pFileList->pItemShow;
    for (Row=0; (p!=AK_NULL)&&(Row<pFileList->PageItemQty); Row++)
    {
        if (p == pFileList->pItemFocus)
            break;

        p = p->pNext;
    }
    if (p == AK_NULL)
        return AK_FALSE;

    // show focus item text back
    PosX = (T_S16)(pFileList->FileListRect.left + pFileList->FrameWidth + pFileList->ItemVInterval);
    PosY = (T_S16)(pFileList->FileListRect.top + pFileList->FrameWidth + pFileList->ItemVInterval + \
            (pFileList->ItemVInterval+pFileList->ItemHeight)*Row);
    Rect.width = (T_S16)(pFileList->ItemRect.width - ShowScrollBarWidth - \
            pFileList->FrameWidth*2 - pFileList->ItemVInterval);
    Rect.height = (T_S16)(pFileList->ItemHeight);
    Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pFileList->FocusBackColor);

    // get show focus item text pos
    if (FileList_GetItemTextPos(pFileList, pFileList->pItemFocus, Row, &PosX, &PosY, &MaxLen) == AK_TRUE)
    {
        // check double bytes char
        if (Utl_UIsDWordChar(pFileList->pItemFocus->pText + pFileList->ItemTextOffset, gs.Lang) == AK_TRUE)
        {
            pFileList->ItemTextOffset += 2;
            if ((T_S16)(Utl_UStrLen(pFileList->pItemFocus->pText + pFileList->ItemTextOffset)) < (MaxLen-1))
                pFileList->ItemTextOffset = 0;
        }
        else
        {
            pFileList->ItemTextOffset++;
            if (Utl_UStrLen(pFileList->pItemFocus->pText + pFileList->ItemTextOffset) < MaxLen)
                pFileList->ItemTextOffset = 0;
        }

        // show focus item text
        Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pFileList->pItemFocus->pText + pFileList->ItemTextOffset, \
                pFileList->FocusTextColor, CURRENT_FONT_SIZE, MaxLen);
    }


    return AK_TRUE;
}

// show all item
static T_BOOL FileList_ShowItem(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p;
    T_U32 Row;
    //T_RECT Rect;

    AK_ASSERT_PTR(pFileList, "FileList_ShowItem(): pFileList", AK_FALSE);

    if ((pFileList->RefreshFlag & FILELIST_REFRESH_ITEM) == FILELIST_REFRESH_ITEM)
    {
        // show item back color
        Fwl_FillSolidRect(HRGB_LAYER, \
                pFileList->ItemRect.left, pFileList->ItemRect.top, \
                pFileList->ItemRect.width, pFileList->ItemRect.height, \
                pFileList->ItemBackColor);

        // show item frame
        Fwl_DialogFrame(HRGB_LAYER, pFileList->ItemRect.left, pFileList->ItemRect.top, \
                pFileList->ItemRect.width, pFileList->ItemRect.height, 0x0f);

        // show item by style
        p = pFileList->pItemShow;
        for (Row=0; (p!=AK_NULL)&&(Row<pFileList->PageItemQty); Row++)
        {
            if (p == pFileList->pItemFocus)
                FileList_ShowOneItem(pFileList, p, Row, AK_TRUE, AK_FALSE);
            else
                FileList_ShowOneItem(pFileList, p, Row, AK_FALSE, AK_FALSE);

            p = p->pNext;
        }
        pFileList->ItemTextOffset = 0;

        // check scroll bar & show it again
        if (pFileList->ScBarShowFlag == AK_TRUE)
            FileList_ShowScrollBar(pFileList);
    }

    return AK_TRUE;
}

// show a item
static T_BOOL FileList_ShowOneItem(T_FILELIST *pFileList, T_FILELIST_ITEM *pItem, T_U32 Row, T_BOOL Focus, T_BOOL ShowBack)
{
    T_RECT Rect;
    T_S16 PosX, PosY;
    T_U16 MaxLen;
    T_U16 *pTmpText;
    T_U32 ScBarShowW;

    AK_ASSERT_PTR(pFileList, "FileList_ShowOneItem(): pFileList", AK_FALSE);
    AK_ASSERT_VAL((Row < pFileList->PageItemQty), "FileList_ShowOneItem(): (Row >= pFileList->PageItemQty)", AK_FALSE);

    if (pFileList == AK_NULL)
        return AK_FALSE;

    if (Row >= pFileList->PageItemQty)
        return AK_FALSE;

    ScBarShowW = (pFileList->ScBarShowFlag) ? pFileList->ScrollBarWidth : 0;
    // show item back
    if (ShowBack == AK_TRUE)
    {
        PosX = (T_S16)(pFileList->ItemRect.left + pFileList->FrameWidth);
        PosY = (T_S16)(pFileList->ItemRect.top + pFileList->FrameWidth + pFileList->ItemVInterval + \
                (pFileList->ItemVInterval+pFileList->ItemHeight)*Row);
        Rect.left = 0;
        Rect.top = 0;
        Rect.width = (T_S16)(pFileList->ItemRect.width - pFileList->FrameWidth*2 - ScBarShowW);
        Rect.height = (T_S16)(pFileList->ItemHeight);
        if (Focus == AK_TRUE)
        {
            Fwl_AkBmpDrawPartFromString(HRGB_LAYER, PosX, PosY, &Rect, pFileList->pFocusBckgrnd, AK_NULL, AK_FALSE);
        }
        else
        {
            Fwl_FillSolidRect(HRGB_LAYER, PosX, PosY, Rect.width, Rect.height, pFileList->ItemBackColor);
        }
    }
    else
    {
        if (Focus == AK_TRUE)
        {
            PosX = (T_S16)(pFileList->ItemRect.left + pFileList->FrameWidth);
            PosY = (T_S16)(pFileList->ItemRect.top + pFileList->FrameWidth + pFileList->ItemVInterval + \
                    (pFileList->ItemVInterval+pFileList->ItemHeight)*Row);
            Rect.left = 0;
            Rect.top = 0;
            Rect.width = (T_S16)(pFileList->ItemRect.width - pFileList->FrameWidth*2 - ScBarShowW);
            Rect.height = (T_S16)(pFileList->ItemHeight);
            Fwl_AkBmpDrawPartFromString(HRGB_LAYER, PosX, PosY, &Rect, pFileList->pFocusBckgrnd, AK_NULL, AK_FALSE);
        }
    }

    // show item text
    if (pItem->pText != AK_NULL)
    {
        // get show item text pos
        if (FileList_GetItemTextPos(pFileList, pItem, Row, &PosX, &PosY, &MaxLen) == AK_TRUE) {
            pTmpText = (T_U16 *)Fwl_Malloc((MaxLen+1) << 1);
            if (pTmpText != AK_NULL)
            {
                memset((void *)pTmpText, 0x00, (MaxLen + 1) << 1);
                // check string length
                if (Utl_UStrLen(pItem->pText) <= MaxLen)
                {
                    Utl_UStrCpy(pTmpText, pItem->pText);
                }
                else
                {
                    if (Focus == AK_TRUE)
                    {
                        Utl_UStrMid(pTmpText, pItem->pText, 0, (T_S16)(MaxLen-1));
                    }
                    else
                    {
                        T_USTR_FILE Ustr;

                        Utl_UStrMid(pTmpText, pItem->pText, 0, (T_S16)(MaxLen-1-1));
                        Eng_StrMbcs2Ucs(">", Ustr);
                        Utl_UStrCat(pTmpText, Ustr);
                    }
                }

                if (Focus == AK_TRUE)
                    Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pTmpText, pFileList->FocusTextColor, CURRENT_FONT_SIZE, MaxLen);
                else
                    Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pTmpText, pFileList->TextColor, CURRENT_FONT_SIZE, MaxLen);

                pTmpText = Fwl_Free(pTmpText);
            }
        }
    }

    return AK_TRUE;
}

// show focus item
static T_BOOL FileList_ShowFocus(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p;
    T_U32 Row;

    AK_ASSERT_PTR(pFileList, "FileList_ShowFocus(): pFileList", AK_FALSE);

    if (((pFileList->RefreshFlag & FILELIST_REFRESH_FOCUS) == FILELIST_REFRESH_FOCUS) && \
            ((pFileList->RefreshFlag & FILELIST_REFRESH_ITEM) != FILELIST_REFRESH_ITEM)) {
        // refresh old focus & focus item
        p = pFileList->pItemShow;
        for (Row=0; (p!=AK_NULL)&&(Row<pFileList->PageItemQty); Row++)
        {
            if (p == pFileList->pItemOldFocus)
                FileList_ShowOneItem(pFileList, p, Row, AK_FALSE, AK_TRUE);
            if (p == pFileList->pItemFocus)
                FileList_ShowOneItem(pFileList, p, Row, AK_TRUE, AK_TRUE);

            p = p->pNext;
        }
        pFileList->ItemTextOffset = 0;
    }

    return AK_TRUE;
}

T_FILELIST_MOVEFOCUS_RET FileList_MoveFocus(T_FILELIST *pFileList, T_FILELIST_DIRECTION Direction)
{
    T_FILELIST_ITEM *pOldItemFocus;
    T_FILELIST_MOVEFOCUS_RET ret = FILELIST_MOVEFOCUS_ERROR;

    AK_ASSERT_PTR(pFileList, "FileList_MoveFocus(): pFileList", FILELIST_MOVEFOCUS_ERROR);
    AK_ASSERT_PTR(pFileList->pItemHead, "FileList_MoveFocus(): pFileList->pItemHead", FILELIST_MOVEFOCUS_ERROR);
    AK_ASSERT_PTR(pFileList->pItemFocus, "FileList_MoveFocus(): pFileList->pItemFocus", FILELIST_MOVEFOCUS_ERROR);

    // save current focus
    pOldItemFocus = pFileList->pItemFocus;

    // move focus by direction
    switch (Direction)
    {
        case FILELIST_MOVE_UP:
            ret = FileList_MoveUp(pFileList);
            break;
        case FILELIST_MOVE_DOWN:
            ret = FileList_MoveDown(pFileList);
            break;
        default:
            return FILELIST_MOVEFOCUS_ERROR;
    }

    // check refresh flag
    if (pFileList->pItemFocus != pOldItemFocus)
        FileList_SetRefresh(pFileList, FILELIST_REFRESH_FOCUS);

    return ret;
}


static T_FILELIST_MOVEFOCUS_RET FileList_MoveUp(T_FILELIST *pFileList)
{
    T_FILELIST_MOVEFOCUS_RET ret = FILELIST_MOVEFOCUS_ERROR;

    AK_ASSERT_PTR(pFileList, "FileList_MoveUp(): pFileList", FILELIST_MOVEFOCUS_ERROR);
    AK_ASSERT_PTR(pFileList->pItemHead, "FileList_MoveUp(): pFileList->pItemHead", FILELIST_MOVEFOCUS_ERROR);
    AK_ASSERT_PTR(pFileList->pItemFocus, "FileList_MoveUp(): pFileList->pItemFocus", FILELIST_MOVEFOCUS_ERROR);

    pFileList->pItemOldFocus = pFileList->pItemFocus;
    if (pFileList->pItemFocus->pPrevious == AK_NULL)
    {
        /*
        while (pFileList->pItemFocus->pNext != AK_NULL)
            pFileList->pItemFocus = pFileList->pItemFocus->pNext;
            */
        ret = FILELIST_MOVEFOCUS_OVERHEAD;
        pFileList->pItemFocus = pFileList->pItemTail;
    }
    else
    {
        ret = FILELIST_MOVEFOCUS_OK;
        pFileList->pItemFocus = pFileList->pItemFocus->pPrevious;
    }

    FileList_CheckShowFocus(pFileList);

    return ret;
}

static T_FILELIST_MOVEFOCUS_RET FileList_MoveDown(T_FILELIST *pFileList)
{
    T_FILELIST_MOVEFOCUS_RET ret = FILELIST_MOVEFOCUS_ERROR;

    AK_ASSERT_PTR(pFileList, "FileList_MoveDown(): pFileList", FILELIST_MOVEFOCUS_ERROR);
    AK_ASSERT_PTR(pFileList->pItemHead, "FileList_MoveDown(): pFileList->pItemHead", FILELIST_MOVEFOCUS_ERROR);
    AK_ASSERT_PTR(pFileList->pItemFocus, "FileList_MoveDown(): pFileList->pItemFocus", FILELIST_MOVEFOCUS_ERROR);

    pFileList->pItemOldFocus = pFileList->pItemFocus;
    if (pFileList->pItemFocus->pNext == AK_NULL)
    {
        ret = FILELIST_MOVEFOCUS_OVERTAIL;
        pFileList->pItemFocus = pFileList->pItemHead;
    }
    else
    {
        ret = FILELIST_MOVEFOCUS_OK;
        pFileList->pItemFocus = pFileList->pItemFocus->pNext;
    }

    FileList_CheckShowFocus(pFileList);

    return ret;
}


static T_BOOL FileList_SetTitleRect(T_FILELIST *pFileList, T_RECT TitleRect)
{
    AK_ASSERT_PTR(pFileList, "FileList_SetTitleRect(): pFileList", AK_FALSE);

    pFileList->TitleRect = TitleRect;
    FileList_CheckRect(&pFileList->TitleRect);

    return AK_TRUE;
}

static T_BOOL FileList_SetItemRect(T_FILELIST *pFileList, T_RECT ItemRect)
{
    AK_ASSERT_PTR(pFileList, "FileList_SetItemRect(): pFileList", AK_FALSE);

    pFileList->ItemRect = ItemRect;
    FileList_CheckRect(&pFileList->ItemRect);

    // set scroll bar parameter again
    FileList_ScrollbarInit(pFileList, pFileList->ItemRect);
    // set one page item row
    pFileList->PageItemQty = (pFileList->ItemRect.height-pFileList->FrameWidth*2-pFileList->ItemVInterval) / \
            (pFileList->ItemHeight+pFileList->ItemVInterval);

    FileList_CheckScrollBar(pFileList);
    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ITEM);

    return AK_TRUE;
}


T_BOOL FileList_SetFocusById(T_FILELIST *pFileList, T_U32 Id)
{
    T_FILELIST_ITEM *p;

    AK_ASSERT_PTR(pFileList, "FileList_SetFocusById(): pFileList", AK_FALSE);

    p = pFileList->pItemHead;
    while (p != AK_NULL)
    {
        if (p->ID == Id)
            break;
        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    pFileList->pItemOldFocus = pFileList->pItemFocus;
    pFileList->pItemFocus = p;

    // check scroll bar, modify refresh flag
    FileList_CheckScrollBar(pFileList);
    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ALL);

    return AK_TRUE;
}


static T_BOOL FileList_IsFileInFolder(T_pCWSTR pFilePath, T_pCWSTR pFolderPath)
{
    T_U32      count;
    T_U32      i;
    T_USTR_FILE FilePath;
    T_USTR_FILE FolderPath;
    T_BOOL  ret = AK_FALSE;

    AK_ASSERT_PTR(pFilePath, "FileList_IsFileInFolder(): pFilePath", AK_FALSE);
    AK_ASSERT_PTR(pFolderPath, "FileList_IsFileInFolder(): pFolderPath", AK_FALSE);

    Utl_UStrCpy(FilePath, (T_U16 *)pFilePath);
    Utl_UStrCpy(FolderPath, (T_U16 *)pFolderPath);
    Utl_UStrLower(FilePath);
    Utl_UStrLower(FolderPath);
    count = Utl_UStrLen(FolderPath);
    for(i=0; i<=count; i++)
    {
        if (FilePath[i] != FolderPath[i])
        {
            break;
        }
    }
    if (i >= count)
    {
        ret = AK_TRUE;
    }

    return ret;
}


// file list show
T_BOOL FileList_Show(T_FILELIST *pFileList)
{
    AK_ASSERT_PTR(pFileList, "FileList_Show(): pFileList", AK_FALSE);

    // show content
    //FileList_ShowTitle(pFileList);
    FileList_ShowItem(pFileList);
    FileList_ShowFocus(pFileList);

    // reset refresh flag
    FileList_SetRefresh(pFileList, FILELIST_REFRESH_NONE);

    return AK_TRUE;
}

// file list handle
T_eBACK_STATE FileList_Handler(T_FILELIST *pFileList, T_EVT_CODE Event, T_EVT_PARAM *pParam)
{
    T_MMI_KEYPAD phyKey;

    AK_ASSERT_PTR(pFileList, "FileList_Handler(): pFileList", eStay);

    switch (Event)
    {
        // user key event process
        case M_EVT_USER_KEY:
            phyKey.keyID = pParam->c.Param1;
            phyKey.pressType = pParam->c.Param2;
            switch (phyKey.keyID)
            {
                case kbOK:
                    if (phyKey.pressType == PRESS_SHORT)
                    {
                        // return affirm
                        return eNext;
                    }
                    if (phyKey.pressType == PRESS_LONG)
                    {
                        Fwl_KeyStop();
                        // return affirm
                        return eNext;
                    }
                    break;
                case kbCLEAR:
                    if (phyKey.pressType == PRESS_SHORT)
                    {
                        // return return
                        return eReturn;
                    }
                    else if (phyKey.pressType == PRESS_LONG)
                    {
                        // return home
                        Fwl_KeyStop();
                        return eHome;
                    }
                    break;
                case kbLEFT:
                    break;
                case kbRIGHT:
                    break;
                case kbUP:
                    if (phyKey.pressType == PRESS_SHORT)
                    {
                        FileList_MoveFocus(pFileList, FILELIST_MOVE_UP);
                    }
                    break;
                case kbDOWN:
                    if (phyKey.pressType == PRESS_SHORT)
                    {
                        FileList_MoveFocus(pFileList, FILELIST_MOVE_DOWN);
                    }
                    break;
                default:
                    break;
            }
            break;
        case M_EVT_PUB_TIMER:
            // scroll title & item text
            //if (IconExplorer_ScrollTitleText(pIconExplorer) == AK_TRUE)
                //Fwl_RefreshDisplay();
            if (FileList_ScrollItemText(pFileList) == AK_TRUE)
                Fwl_RefreshDisplay();
            break;
        default:
            break;
    }
    return eStay;
}


T_FILELIST_FETCHMODE FileList_GetFetchMode(T_FILELIST *pFileList)
{
    AK_ASSERT_PTR(pFileList, "FileList_SetFetchMode(): pFileList", FILELIST_ADD_RET_NUM);

    return pFileList->FetchMode;
}

static T_pWSTR FileList_RandomFetchFilePath(T_FILELIST *pFileList, T_FILELIST_ITEM *pPlayItem)
{
    T_U32 SkipNum;
    T_FILELIST_ITEM *p, *q;
    T_pWSTR      pFilePath = AK_NULL;
    T_U32 i;


    AK_ASSERT_PTR(pFileList, "FileList_FetchOneItem(): pFileList", AK_NULL);
    AK_ASSERT_PTR(pFileList->pItemHead, "FileList_FetchOneItem(): pFileList->pItemHead", AK_NULL);
    AK_ASSERT_PTR(pFileList->pItemFocus, "FileList_FetchOneItem(): pFileList->pItemFocus", AK_NULL);

    if (pPlayItem)
    {
        FileList_SetFocusById(pFileList, pPlayItem->ID);
    }

    if (1 == pFileList->ItemQty)
    {
        p = pFileList->pItemFocus;
        pFilePath = p->pFilePath;
    }
    else
    {
        // random skip
        SkipNum = Fwl_GetRand(pFileList->ItemQty);
        q = pFileList->pItemFocus;
        for(i=0; i<SkipNum; i++)
        {
            FileList_MoveFocus(pFileList, FILELIST_MOVE_DOWN);
        }
        p = pFileList->pItemFocus;

        if (p != q)
        {
            pFilePath = p->pFilePath;
            pFileList->pItemFocus = p;
        }
        else
        {
            // do one more time
            SkipNum = Fwl_GetRand(pFileList->ItemQty);
            for(i=0; i<SkipNum; i++)
            {
                FileList_MoveFocus(pFileList, FILELIST_MOVE_DOWN);
            }
            p = pFileList->pItemFocus;

            if (p != q)
            {
                pFilePath = p->pFilePath;
                pFileList->pItemFocus = p;
            }
            else if (AK_NULL != p->pNext)
            {
                p = p->pNext;
                pFilePath = p->pFilePath;
                pFileList->pItemFocus = p;
            }
            else
            {
                p = pFileList->pItemHead;
                pFilePath = p->pFilePath;
                pFileList->pItemFocus = p;
            }
        }
    }
    return pFilePath;
}


T_pWSTR FileList_FetchNextFilePath(T_FILELIST *pFileList, T_U32 Id)
{
    T_FILELIST_ITEM *p = AK_NULL;
    T_pWSTR      pFilePath = AK_NULL;
    T_FILELIST_ITEM *pOldItemFocus;

    AK_ASSERT_PTR(pFileList, "FileList_FetchOneItem(): pFileList", AK_NULL);
    AK_ASSERT_PTR(pFileList->pItemHead, "FileList_FetchOneItem(): pFileList->pItemHead", AK_NULL);
    AK_ASSERT_PTR(pFileList->pItemFocus, "FileList_FetchOneItem(): pFileList->pItemFocus", AK_NULL);

    pOldItemFocus = pFileList->pItemFocus;

    p = pFileList->pItemHead;
    while (p != AK_NULL)
    {
        if (p->ID == Id)
            break;
        p = p->pNext;
    }

    switch (pFileList->FetchMode)
    {
        case FILELIST_FETCH_SEQUENCE:
            if (AK_NULL != p)
            {
                // The play file not del from list
                p = p->pNext;
                if (AK_NULL != p)
                {
                    pFilePath = p->pFilePath;
                    pFileList->pItemFocus = p;
                }
                else
                {
                    pFilePath = AK_NULL;
                }
            }
            else
            {
                // The play file have been del from list
                pFilePath = pFileList->pItemFocus->pFilePath;
            }
            break;
        case FILELIST_FETCH_REPEAT:
            if (AK_NULL != p)
            {
                p = p->pNext;
                if (AK_NULL != p)
                {
                    pFilePath = p->pFilePath;
                    pFileList->pItemFocus = p;
                }
                else
                {
                    p = pFileList->pItemHead;
                    pFileList->pItemFocus = p;
                    pFilePath = p->pFilePath;
                }
            }
            else
            {
                pFilePath = pFileList->pItemFocus->pFilePath;
            }
            break;
        case FILELIST_FETCH_REPEAT_SINGLE:
            if (AK_NULL != p)
            {
                pFilePath = p->pFilePath;
                pFileList->pItemFocus = p;
            }
            else
            {
                pFilePath = AK_NULL;
            }
            break;
        case FILELIST_FETCH_RANDOM:
            pFilePath = FileList_RandomFetchFilePath(pFileList, p);
            break;
        default:
            pFilePath = AK_NULL;
            break;
    }
    FileList_CheckShowFocus(pFileList);
    // check refresh flag
    if (pFileList->pItemFocus != pOldItemFocus)
        FileList_SetRefresh(pFileList, FILELIST_REFRESH_FOCUS | FILELIST_REFRESH_ITEM);
    return pFilePath;
}

T_BOOL FileList_IsListHaveContent(T_FILELIST *pFileList)
{
    AK_ASSERT_PTR(pFileList, "FileList_IsListHaveContent(): pFileList", AK_FALSE);

    if (pFileList->pItemHead)
    {
        return AK_TRUE;
    }

    return AK_FALSE;
}

// fetch next file by fetch mode
T_pWSTR FileList_FetchFocusNextFilePath(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p;

    AK_ASSERT_PTR(pFileList, "FileList_FetchFocusNextFilePath(): pFileList", AK_NULL);

    p = pFileList->pItemHead;
    while (p != AK_NULL) {
        if (p == pFileList->pItemFocus)
            break;

        p = p->pNext;
    }

    if (AK_NULL == p || AK_NULL == p->pNext)
        return AK_NULL;
    p = p->pNext;

    return p->pFilePath;
}

// fetch next file by fetch mode
T_pWSTR FileList_FetchIdNextFilePath(T_FILELIST *pFileList, T_U32 Id)
{
    T_FILELIST_ITEM *p;

    AK_ASSERT_PTR(pFileList, "FileList_FetchIdNextFilePath(): pFileList", AK_NULL);

    p = pFileList->pItemHead;
    while (p != AK_NULL) {
        if (p->ID == Id)
            break;

        p = p->pNext;
    }

    if (AK_NULL == p || AK_NULL == p->pNext)
        return AK_NULL;
    p = p->pNext;

    return p->pFilePath;
}

// fetch the focus file path
T_pWSTR FileList_GetFocusFilePath(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p;

    AK_ASSERT_PTR(pFileList, "FileList_GetFocusFilePath(): pFileList", AK_NULL);

    p = pFileList->pItemHead;
    while (p != AK_NULL) {
        if (p == pFileList->pItemFocus)
            break;

        p = p->pNext;
    }

    if (AK_NULL == p)
        return AK_NULL;

    return p->pFilePath;
}

T_pCWSTR FileList_GetFocusText(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p;

    AK_ASSERT_PTR(pFileList, "FileList_GetFocusText(): pFileList", AK_NULL);

    if (AK_NULL != pFileList->pItemFocus)
    {
        p = pFileList->pItemFocus;
        return p->pText;
    }

    return AK_NULL;
}



// load file list 
static T_FILELIST_ADD_RET FileList_AddList(T_FILELIST *pFileList, T_USTR_FILE ListPath)
{
    T_pFILE fp;
    T_U32   fsize;
    T_U16   *buf;
    T_U16   *pCurBuf;
    T_U16   *pFirst;
    T_FILELIST  *FileList = AK_NULL;
    T_FILELIST_ADD_RET ret = FILELIST_ADD_ERROR;
    T_USTR_FILE FilePath;
    T_U32       count;

    AK_ASSERT_PTR(pFileList, "FileList_AddList(): pFileList", FILELIST_ADD_ERROR);
    AK_ASSERT_PTR(ListPath, "FileList_AddList(): ListPath", FILELIST_ADD_ERROR);
    AK_ASSERT_VAL(*ListPath, "FileList_AddList(): *ListPath", FILELIST_ADD_ERROR);
    AK_ASSERT_VAL((Utl_UStrLen(ListPath) < (MAX_FILENM_LEN - 2)), "FileList_AddList(): ListPath too long", FILELIST_ADD_ERROR);


    if ((fsize = Fwl_FileGetSize(fhandle)) > 0)
    {
        fp = Fwl_FileOpen(ListPath, _FMODE_READ, _FMODE_READ);
        if (fp != _FOPEN_FAIL)
        {
            buf = (T_U16 *)Fwl_Malloc(fsize + 2);
            if (buf != AK_NULL)
            {
                Fwl_FileRead(fp, (T_U8 *)buf, fsize);
                *(buf + (fsize >> 1)) = 0;
                count = 0;
                ret  = FILELIST_ADD_SUCCESS;
                pCurBuf = buf;
                pFirst = pCurBuf;
                while (*pCurBuf != 0 && ret == FILELIST_ADD_SUCCESS)
                {
                    ++count;
                    if (count >= MAX_FILENM_LEN)
                    {
                        break;
                    }
                    if (*pCurBuf == UNICODE_SEPARATOR)
                    {
                        Utl_UStrCpyN(FilePath, pFirst, count-1);
                        FilePath[count - 1] = 0;
                        ret = FileList_AddItemToChain(pFileList, FilePath);
                        count = 0;
                        pFirst = pCurBuf+1;
                    }
                    ++pCurBuf;
                }

                buf = Fwl_Free(buf);
            }
            Fwl_FileClose(fp);
        }
    }

    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ITEM);
    return ret;
}

// delete the focus from the file list
T_BOOL FileList_DelFocus(T_FILELIST *pFileList)
{
    T_FILELIST_ITEM *p;

    AK_ASSERT_PTR(pFileList, "FileList_DelFocus(): pFileList", AK_FALSE);

    // find item focus
    p = pFileList->pItemHead;
    while (p != AK_NULL) {
        if (p == pFileList->pItemFocus)
            break;

        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    FileList_RegulateWhenDelOne(pFileList, p);
    p->pFilePath = Fwl_Free(p->pFilePath);
    p = Fwl_Free(p);

    // modify item quantity
    pFileList->ItemQty--;

    // check scroll bar, modify refresh flag
    FileList_CheckScrollBar(pFileList);
    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ALL);
    FileList_SetChangeFlag(pFileList, AK_TRUE);
    return AK_TRUE;
}

// delete the id file from the file list
T_BOOL FileList_DelById(T_FILELIST *pFileList, T_U32 Id)
{
    T_FILELIST_ITEM *p;

    AK_ASSERT_PTR(pFileList, "FileList_DelById(): pFileList", AK_FALSE);

    p = pFileList->pItemHead;
    while (p != AK_NULL)
    {
        if (p->ID == Id)
            break;
        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_FALSE;

    FileList_RegulateWhenDelOne(pFileList, p);
    p->pFilePath = Fwl_Free(p->pFilePath);
    p = Fwl_Free(p);
    // modify item quantity
    pFileList->ItemQty--;

    FileList_CheckScrollBar(pFileList);
    FileList_SetRefresh(pFileList, FILELIST_REFRESH_ALL);
    FileList_SetChangeFlag(pFileList, AK_TRUE);

    return AK_TRUE;
}

static T_BOOL FileList_SetCompareCallBack(T_FILELIST *pFileList, T_fFILELIST_COMPARE_CALLBACK CompareCallBack)
{
    if (AK_NULL == pFileList)
        return AK_FALSE;

    pFileList->SortMethod.SortCompareCallBack = CompareCallBack;

    return AK_TRUE;
}


T_BOOL FileList_SetSortCallBack(T_FILELIST *pFileList, T_fFILELIST_COMPARE_CALLBACK CompareCallBack)
{
    AK_ASSERT_PTR(pFileList, "FileList_UserDefSort(): pFileList", AK_FALSE);
    AK_ASSERT_PTR((T_pCVOID)CompareCallBack, "FileList_UserDefSort(): CompareCallBack", AK_FALSE);

    pFileList->SortMethod.SortMode = FILELIST_SORT_USER_DEFINE;
    pFileList->SortMethod.SortCompareCallBack = CompareCallBack;

    return AK_TRUE;
}


T_U32 FileList_GetRefresh(T_FILELIST *pFileList)
{
    AK_ASSERT_PTR(pFileList, "FileList_GetRefresh(): pFileList", FILELIST_REFRESH_NONE);

    return pFileList->RefreshFlag;
}


T_BOOL FileList_SetRect(T_FILELIST *pFileList, T_RECT FileListRect, T_BOOL TitleShowFlag)
{
    T_U32 len;
    T_RECT TitleRect;
    T_RECT ItemRect;

    AK_ASSERT_PTR(pFileList, "FileList_SetRect(): pFileList", AK_FALSE);

    if (pFileList->TitleShowFlag != TitleShowFlag)
        pFileList->TitleShowFlag = TitleShowFlag;

    pFileList->FileListRect = FileListRect;

    if (TitleShowFlag)
    {
        pFileList->pTitleBackData = (T_pDATA)Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PUB_TITLE, &len);
        AKBmpGetInfo(pFileList->pTitleBackData, &TitleRect.width, &TitleRect.height, AK_FALSE);
        TitleRect.left = FileListRect.left;
        TitleRect.top = FileListRect.top;
    }
    else
    {
        TitleRect.left = 0;
        TitleRect.top = 0;
        TitleRect.width = 0;
        TitleRect.height = 0;
    }
    FileList_SetTitleRect(pFileList, TitleRect);

    ItemRect.left = FileListRect.left;
    ItemRect.top = FileListRect.top + TitleRect.height;
    ItemRect.width = FileListRect.width;
    ItemRect.height = FileListRect.height - TitleRect.height;

    FileList_SetItemRect(pFileList, ItemRect);

    return AK_TRUE;
}



T_BOOL FileList_SetFocusToHead(T_FILELIST *pFileList)
{
    AK_ASSERT_PTR(pFileList, "FileList_SetFocusToHead(): pFileList", AK_FALSE);

    pFileList->pItemOldFocus = pFileList->pItemFocus;
    pFileList->pItemFocus = pFileList->pItemHead;
    FileList_CheckShowFocus(pFileList);

    // check refresh flag
    if (pFileList->pItemFocus != pFileList->pItemOldFocus)
        FileList_SetRefresh(pFileList, FILELIST_REFRESH_FOCUS);

    return AK_TRUE;
}

T_U32 FileList_GetFocusId(T_FILELIST *pFileList)
{
    T_U32 Id = 0;

    AK_ASSERT_PTR(pFileList, "FileList_GetFocusId(): pFileList", 0);

    Id = pFileList->pItemFocus ? pFileList->pItemFocus->ID : 0;

    return Id;
}

T_pCWSTR FileList_GetIdText(T_FILELIST *pFileList, T_U32 Id)
{
    T_FILELIST_ITEM *p;

    AK_ASSERT_PTR(pFileList, "FileList_GetIdText(): pFileList", AK_NULL);
  //  AK_ASSERT_VAL((Id != 0), "FileList_GetIdText(): Id == 0", AK_NULL);

    p = pFileList->pItemHead;
    while (p != AK_NULL)
    {
        if (p->ID == Id)
            break;
        p = p->pNext;
    }

    if (p == AK_NULL)
        return AK_NULL;

    return p->pText;
}


T_BOOL FileList_DelByFolder(T_FILELIST *pFileList, T_pCWSTR pFolderPath)
{
    T_FILELIST_ITEM *p;
    T_FILELIST_ITEM *p_next;

    AK_ASSERT_PTR(pFileList, "FileList_DelByFolder(): pFileList", AK_FALSE);

    p = pFileList->pItemHead;

    while (p != AK_NULL)
    {
        p_next = p->pNext;
        if (FileList_IsFileInFolder(p->pFilePath, pFolderPath))
        {
            FileList_DelById(pFileList, p->ID);
        }
        p = p_next;
    }

    return AK_TRUE;
}



T_BOOL FileList_GetChangeFlag(T_FILELIST *pFileList)
{
    AK_ASSERT_PTR(pFileList, "FileList_GetChangeFlag(): pFileList", AK_FALSE);

    return pFileList->ChangeFlag;
}



#endif

