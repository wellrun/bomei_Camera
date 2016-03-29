/**
 * @file svc_medialist.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */


#include "fwl_public.h"
#include "svc_medialist.h"
#include "AKAppMgr.h"
#include "fwl_osfs.h"
#include "file.h"
#include "AKAudioListBGApp.h"
#include "AKVideoListBGApp.h"
#include "eng_string_uc.h"
#include "eng_string.h"
#include "Media_Demuxer_lib.h"
#include "Ctl_APlayerList.h"
#include "Ctl_Aviplayer.h"
#include "eng_time.h"

#define SUSPEND_WAIT_CNT	50

static T_GB_MLIST mList[eMEDIA_LIST_NUM];


#ifdef OS_WIN32
static T_hSemaphore mlist_mutex = 0;
#endif

static T_S32 MList_Init(T_eMEDIA_LIST_TYPE type);
static T_BOOL MList_Destroy(T_eMEDIA_LIST_TYPE type);

static T_BOOL MList_PostEvent(T_eMEDIA_LIST_TYPE type, T_SYS_MAILBOX *pMailbox);


static T_eID3_WRITE_RET MList_BG_ID3_AddClassInfo(T_U16 Id, T_pCWSTR classname, T_eID3_TAGS tag, T_U16 *albumId);
static T_BOOL MList_BG_ID3_AddId3Info(T_pCWSTR path, T_U16 id, T_pFILE fp);
static T_eMEDIALIST_ADD_RET MList_BG_AddFile(T_eMEDIA_LIST_TYPE type, T_pCWSTR path, T_pFILE fp);
static T_eMEDIALIST_ADD_RET MList_BG_AddFolder(T_eMEDIA_LIST_TYPE type, T_pCWSTR path, 
								T_BOOL subFolder, T_pFILE FindParent);

static T_eMEDIALIST_ADD_RET MList_BG_Add(T_eMEDIA_LIST_TYPE type, T_pCWSTR path, T_BOOL subFolder);

static T_eID3_WRITE_RET MList_BG_ID3_DelByClassId(T_U16 id, T_U16 classId, T_eID3_TAGS tag);
static T_eID3_WRITE_RET MList_BG_ID3_DelClassInfo(T_U16 Id, T_pCWSTR classname, T_eID3_TAGS tag, T_U16 *albumId);
static T_BOOL MList_BG_ID3_HasItem(T_U16 albumId, T_pCWSTR classname, T_eID3_TAGS tag);
static T_BOOL MList_BG_ID3_DelAlbum(T_U16 albumId, T_eID3_TAGS tag);

static T_BOOL MList_BG_Del(T_eMEDIA_LIST_TYPE type, T_pCWSTR path);

static T_BOOL MList_ID3_GetTagPath(T_pWSTR path, T_eID3_TAGS tag);
static T_U32 MList_CalcQty(T_U8* pbitmap, T_U32 bitmapSize);
static T_U16 MList_FindFirstHoleId(T_U8* pbitmap, T_U32 bitmapSize);

static T_BOOL MList_SetBitValue(T_U8* pbitmap, T_U32 bitmapSize, T_U32 id, T_BOOL val);
static T_U16 MList_CalcChecksum(T_pCWSTR path);
static T_BOOL MList_GetBitValue(T_U8* pbitmap, T_U32 bitmapSize, T_U32 id);
static T_U16 MList_ID3_GetClassId(T_pCWSTR classname, T_eID3_TAGS tag, T_BOOL bBg);
static T_U16 MList_GetFileId(T_pCWSTR filename, T_eMEDIA_LIST_TYPE type, T_BOOL bBg);
static T_S32 MList_Obtain_Semaphore(T_eMEDIA_LIST_TYPE type);
static T_S32 MList_Release_Semaphore(T_eMEDIA_LIST_TYPE type);
static T_BOOL MList_Suspend(T_eMEDIA_LIST_TYPE type, T_BOOL bNeedWait);

/**
* @brief memset medialist struct
*
* @author Songmengxing
* @date 2011-12-26
* @return T_VOID
* @retval 
*/
T_VOID MList_Memset(T_VOID)
{
	memset(mList, 0, eMEDIA_LIST_NUM * sizeof(T_GB_MLIST));
}

/**
* @brief get add flag
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_eADD_FLAG_TYPE
* @retval 
*/
T_eADD_FLAG_TYPE MList_GetAddFlag(T_eMEDIA_LIST_TYPE type)
{
	T_eADD_FLAG_TYPE ret = eADD_FLAG_NONE;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_GetAddFlag(): type err", ret);

	MList_Obtain_Semaphore(type);
	ret = mList[type].addFlag;
	MList_Release_Semaphore(type);

	return ret;
}

/**
* @brief set add flag
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @param in T_eADD_FLAG_TYPE flag : 0, 1 or 2
* @return T_BOOL
* @retval 
*/
T_BOOL MList_SetAddFlag(T_eMEDIA_LIST_TYPE type, T_eADD_FLAG_TYPE flag)
{
	T_BOOL ret = AK_FALSE;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_SetAddFlag(): type err", ret);

	MList_Obtain_Semaphore(type);

	if (eADD_FLAG_NONE != flag)
	{
		mList[type].addFlag |= flag;
	}
	else
	{
		mList[type].addFlag = eADD_FLAG_NONE;
	}
	
	MList_Release_Semaphore(type);

	return AK_TRUE;
}


/**
* @brief get delete  flag
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL MList_GetDelFlag(T_eMEDIA_LIST_TYPE type)
{
	T_BOOL ret = AK_FALSE;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_GetDelFlag(): type err", ret);

	MList_Obtain_Semaphore(type);
	ret = mList[type].delFlag;
	MList_Release_Semaphore(type);

	return ret;
}

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
T_BOOL MList_SetDelFlag(T_eMEDIA_LIST_TYPE type, T_BOOL flag)
{
	T_BOOL ret = AK_FALSE;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_SetDelFlag(): type err", ret);

	MList_Obtain_Semaphore(type);
	mList[type].delFlag = flag;
	MList_Release_Semaphore(type);

	return AK_TRUE;
}


/**
* @brief Open Media List (Audio / Video) Service for Read Media List (ID3 Information) Mutex
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_S32 count
* @retval -1 open fail; >=0 open success;
*/
T_S32 MList_Open(T_eMEDIA_LIST_TYPE type)
{
	T_S32 ret = -1;

	MList_Obtain_Semaphore(type);
	ret = MList_Init(type);
	MList_Release_Semaphore(type);

	return ret;
}



/**
* @brief Close Media List (Audio / Video) Service Mutex
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL MList_Close(T_eMEDIA_LIST_TYPE type)
{
	T_BOOL ret = AK_FALSE;

	MList_Obtain_Semaphore(type);
	ret = MList_Destroy(type);
	MList_Release_Semaphore(type);

	return ret;
}



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
T_BOOL MList_AddItem(T_pCWSTR path, T_BOOL subFolder, T_BOOL resvOld, T_eMEDIA_LIST_TYPE type)
{
	T_BOOL			ret = AK_FALSE;
	T_SYS_MAILBOX	mailbox;
	T_pWSTR			pathstr = AK_NULL;
	
	AK_ASSERT_PTR(path, "MList_AddItem(): path", ret);
	AK_ASSERT_VAL(*path, "MList_AddItem(): *path", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_AddItem(): type err", ret);

	pathstr = (T_pWSTR)Fwl_Malloc(MEDIA_PATH_SIZE);
	AK_ASSERT_PTR(pathstr, "MList_AddItem(): pathstr", ret);
	memset(pathstr, 0, MEDIA_PATH_SIZE);
	Utl_UStrCpyN(pathstr, path, MAX_FILENM_LEN);

	if (!resvOld)
	{
		MList_Suspend(type, AK_TRUE);
	}

	mailbox.event = eMEDIALIST_EVT_ADD;
	mailbox.param.w.Param1 = (T_U32)pathstr;
	mailbox.param.s.Param3 = subFolder;
	mailbox.param.s.Param4 = resvOld;

	MList_PostEvent(type, &mailbox);

    return AK_TRUE;
}

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
T_BOOL MList_ChangePriority(T_U8 priority, T_eMEDIA_LIST_TYPE type)
{
	T_BOOL			ret = AK_FALSE;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_ChangePriority(): type err", ret);

	if (eMEDIA_LIST_AUDIO == type)
    {
		AK_Change_Priority(IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIOLIST)), priority);
    }
    else if (eMEDIA_LIST_VIDEO == type)
    {
		AK_Change_Priority(IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_VIDEOLIST)), priority);
    }
    else
    {

    }

    return AK_TRUE;
}


/**
* @brief Get Media List (Audio / Video) Quantity
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_S32 count
* @retval -1 get fail; >=0 get success;
*/
T_S32 MList_GetItemQty(T_eMEDIA_LIST_TYPE type)
{
	T_S32			ret = -1;
	T_MEDIA_LIST	*pList = AK_NULL;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_GetItemQty(): type err", ret);

	MList_Obtain_Semaphore(type);

	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pList = mList[type].plist;
	
	ret = (T_S32)MList_CalcQty(pList->bitmap, MEDIA_BITMAP_SIZE);
	MList_Release_Semaphore(type);

	return ret;
}

/**
* @brief Get Media List (Audio / Video) state
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_U8 state
* @retval 
*/
T_U8 MList_State(T_eMEDIA_LIST_TYPE type)
{
	T_U8 			ret = 0;
	T_MEDIA_LIST	*pList = AK_NULL;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_State(): type err", ret);

	MList_Obtain_Semaphore(type);
	
	pList = mList[type].plist;

	if (AK_NULL != pList)
	{
		ret = pList->state;
	}

	MList_Release_Semaphore(type);

	return ret;	
}


/**
* @brief Set Media List  suspend all
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_VOID
* @return T_BOOL
* @retval 
*/
T_BOOL MList_SuspendAll(T_VOID)
{
	T_BOOL 				ret = AK_FALSE;
	T_U32				i = 0;
	T_eMEDIA_LIST_TYPE	type = eMEDIA_LIST_AUDIO;
	T_BOOL 				bCloseOk = AK_TRUE;


	for (type=0; type<eMEDIA_LIST_NUM; type++)
	{
		MList_Suspend(type, AK_FALSE);
	}

	for (i=0; i<SUSPEND_WAIT_CNT; i++)
	{		
		bCloseOk = AK_TRUE;
		
		for (type=0; type<eMEDIA_LIST_NUM; type++)
		{
			if (AK_NULL != mList[type].plist)
			{
				bCloseOk = AK_FALSE;
				break;
			}
		}
		
		if (bCloseOk)
		{
			ret = AK_TRUE;
			break;
		}

		Fwl_Print(C3, M_MLIST, "wait...");
		AK_Sleep(20);
	}

	return ret;	
}

/**
* @brief Set Media List  suspend by type
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
static T_BOOL MList_Suspend(T_eMEDIA_LIST_TYPE type, T_BOOL bNeedWait)
{
	T_BOOL 				ret = AK_FALSE;
	T_U32				i = 0;
	T_SYS_MAILBOX		mailbox;

	mailbox.event = eMEDIALIST_EVT_CLOSE;

#ifdef OS_ANYKA
	if (eMEDIA_LIST_AUDIO == type)
	{
		if (0 != IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIOLIST)))
		{
			AK_Reset_Queue(IThread_GetQueue(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIOLIST)));
			IAppMgr_PostEvent(AK_GetAppMgr(), AKAPP_CLSID_AUDIOLIST, &mailbox);
		}
		else
		{
			MList_Close(type);
		}
	}
	else if (eMEDIA_LIST_VIDEO == type)
	{
		if (0 != IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_VIDEOLIST)))
		{
			AK_Reset_Queue(IThread_GetQueue(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_VIDEOLIST)));
			IAppMgr_PostEvent(AK_GetAppMgr(), AKAPP_CLSID_VIDEOLIST, &mailbox);
		}
		else
		{
			MList_Close(type);
		}
	}
	else
	{}

#endif

	MList_Obtain_Semaphore(type);
	
	if (AK_NULL != mList[type].plist)
	{
		mList[type].plist->state |= MEDIA_LIST_REQ_SUSPEND;		
	}
	
	MList_Release_Semaphore(type);

	if (bNeedWait)
	{
		for (i=0; i<SUSPEND_WAIT_CNT; i++)
		{		
			if (AK_NULL == mList[type].plist)
			{
				ret = AK_TRUE;
				break;
			}

			Fwl_Print(C3, M_MLIST, "wait...");
			AK_Sleep(20);
		}
	}

	return ret;	
}


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
T_U16 MList_GetItem(T_pWSTR path, T_U16 ItemId, T_eMEDIA_LIST_TYPE type)
{
	T_U16			ret = 0;
	T_USTR_FILE		listpath = {0};
	T_MEDIA_LIST	*pList = AK_NULL;
	T_U32			seeksize = 0;
	
	AK_ASSERT_VAL((type < eMEDIA_LIST_NUM), "MList_GetItem(): type", ret);
	AK_ASSERT_VAL((ItemId < MAX_MEDIA_NUM), "MList_GetItem(): ItemId", ret);
	AK_ASSERT_PTR(path, "MList_GetItem(): path", ret);

	MList_Obtain_Semaphore(type);

	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pList = mList[type].plist;
	
	if (eMEDIA_LIST_AUDIO == type)
	{
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);
	}
	else if (eMEDIA_LIST_VIDEO == type)
	{
		Eng_StrMbcs2Ucs((VIDEOLIST_DEF_FILE), listpath);
	}
	else
	{
		
	}

	if (FS_INVALID_HANDLE == pList->fdFore)
	{		
		pList->fdFore = Fwl_FileOpen(listpath, _FMODE_READ, _FMODE_READ);
		if (FS_INVALID_HANDLE == pList->fdFore)
		{
			Fwl_Print(C2, M_MLIST, "MList_GetItem(): pList->fdFore open fail");
			MList_Release_Semaphore(type);
			return ret;
		}
	}

	if (0 == MList_GetBitValue(pList->bitmap, MEDIA_BITMAP_SIZE, ItemId))
	{
		Fwl_Print(C4, M_MLIST, "MList_GetItem bitval is 0 \r\n");
		MList_Release_Semaphore(type);
		return ret;
	}

	seeksize = MEDIA_FILE_HEAD_SIZE + ItemId * MEDIA_FILEITEM_SIZE;
	
	if (0 < Fwl_FileSeek(pList->fdFore, seeksize, _FSEEK_SET))
	{
		Fwl_FileRead(pList->fdFore, path, MEDIA_PATH_SIZE);
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_GetItem can't find\r\n");
	}

	ret = (T_U16)MList_CalcQty(pList->bitmap, MEDIA_BITMAP_SIZE);

	MList_Release_Semaphore(type);
	
	return ret;
}


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
T_U32 MList_GetPlayInfo(T_U16 ItemId, T_eMEDIA_LIST_TYPE type, T_eMEDIA_PLAYINFO_TYPE infoType)
{
	T_U32 			ret = 0;
	T_MEDIA_LIST	*pList = AK_NULL;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_GetPlayInfo(): type err", ret);
	AK_ASSERT_VAL(infoType < eMEDIA_PLAYINFO_NUM, "MList_GetPlayInfo(): infoType err", ret);
	AK_ASSERT_VAL((ItemId < MAX_MEDIA_NUM), "MList_GetPlayInfo(): ItemId", ret);

	MList_Obtain_Semaphore(type);
	
	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pList = mList[type].plist;

	switch (infoType)
	{
	case eMEDIA_PLAYCNT:
		ret = pList->playInfo[ItemId].Cnt;
		break;
	case eMEDIA_PLAYTIME:
		ret = pList->playInfo[ItemId].Time;
		break;
	case eMEDIA_APPENDTIME:
		ret = pList->playInfo[ItemId].Time;
		break;
	default:
		break;
	}

	

	MList_Release_Semaphore(type);

	return ret;	
}

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
T_BOOL MList_UpdatePlayInfo(T_U16 ItemId, T_eMEDIA_LIST_TYPE type)
{
	T_BOOL 			ret = AK_FALSE;
	T_MEDIA_LIST	*pList = AK_NULL;
	T_USTR_FILE		listpath = {0};
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_UpdatePlayInfo(): type err", ret);
	AK_ASSERT_VAL((ItemId < MAX_MEDIA_NUM), "MList_UpdatePlayInfo(): ItemId", ret);

	MList_Obtain_Semaphore(type);
	
	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pList = mList[type].plist;

	if (eMEDIA_LIST_AUDIO == type)
	{
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);
	}
	else if (eMEDIA_LIST_VIDEO == type)
	{
		Eng_StrMbcs2Ucs((VIDEOLIST_DEF_FILE), listpath);
	}
	else
	{
		
	}

	if (FS_INVALID_HANDLE == pList->fdBack)
	{
		pList->fdBack = Fwl_FileOpen(listpath, _FMODE_WRITE, _FMODE_WRITE);
		if (FS_INVALID_HANDLE == pList->fdBack)
		{
			Fwl_Print(C2, M_MLIST, "MList_BG_AddFile(): pList->fdBack open fail");
			MList_Release_Semaphore(type);
			return ret;
		}
	}

	if (pList->playInfo[ItemId].Cnt < 0xffffffff)
	{
		pList->playInfo[ItemId].Cnt++;
	}

	pList->playInfo[ItemId].Time = GetSysTimeSeconds();
	ret = AK_TRUE;

	MList_Release_Semaphore(type);

	return ret;	
}

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
T_BOOL MList_CleanPlayInfo(T_U16 ItemId, T_eMEDIA_LIST_TYPE type, T_eMEDIA_PLAYINFO_TYPE infoType)
{
	T_BOOL 			ret = AK_FALSE;
	T_MEDIA_LIST	*pList = AK_NULL;
	T_USTR_FILE		listpath = {0};
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_CleanPlayInfo(): type err", ret);
	AK_ASSERT_VAL((ItemId < MAX_MEDIA_NUM), "MList_CleanPlayInfo(): ItemId", ret);
	AK_ASSERT_VAL(infoType < eMEDIA_PLAYINFO_NUM, "MList_CleanPlayInfo(): infoType err", ret);

	MList_Obtain_Semaphore(type);
	
	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pList = mList[type].plist;

	if (eMEDIA_LIST_AUDIO == type)
	{
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);
	}
	else if (eMEDIA_LIST_VIDEO == type)
	{
		Eng_StrMbcs2Ucs((VIDEOLIST_DEF_FILE), listpath);
	}
	else
	{
		
	}

	if (FS_INVALID_HANDLE == pList->fdBack)
	{
		pList->fdBack = Fwl_FileOpen(listpath, _FMODE_WRITE, _FMODE_WRITE);
		if (FS_INVALID_HANDLE == pList->fdBack)
		{
			Fwl_Print(C2, M_MLIST, "MList_BG_AddFile(): pList->fdBack open fail");
			MList_Release_Semaphore(type);
			return ret;
		}
	}

	switch (infoType)
	{
	case eMEDIA_PLAYCNT:
		if (pList->playInfo[ItemId].Cnt >= ATTACH_PLAY_COUNT)
		{
			pList->playInfo[ItemId].Cnt = 1;
			ret = AK_TRUE;
		}
		break;
	case eMEDIA_PLAYTIME:
		if (pList->playInfo[ItemId].Cnt > 0
			&& pList->playInfo[ItemId].Time > ATTACH_ERROR_TIME
			&& GetSysTimeSeconds() - pList->playInfo[ItemId].Time < ATTACH_APPEND_TIME)
		{
			pList->playInfo[ItemId].Time = ATTACH_ERROR_TIME;
			ret = AK_TRUE;
		}
		break;
	case eMEDIA_APPENDTIME:
		if (pList->playInfo[ItemId].Time > ATTACH_ERROR_TIME
			&& GetSysTimeSeconds() - pList->playInfo[ItemId].Time < ATTACH_APPEND_TIME)
		{
			if(0 == pList->playInfo[ItemId].Cnt)
				pList->playInfo[ItemId].Time = ATTACH_ERROR_TIME;
			ret = AK_TRUE;
		}
		break;
	default:
		break;
	}

	MList_Release_Semaphore(type);

	return ret;	
}

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
T_BOOL MList_RemoveMediaItem(T_pCWSTR path, T_BOOL includeFile, T_eMEDIA_LIST_TYPE type)
{
	T_BOOL			ret = AK_FALSE;
	T_SYS_MAILBOX	mailbox;
	T_pWSTR			pathstr = AK_NULL;
	
	AK_ASSERT_PTR(path, "MList_RemoveMediaItem(): path", ret);
	AK_ASSERT_VAL(*path, "MList_RemoveMediaItem(): *path", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_RemoveMediaItem(): type err", ret);

	pathstr = (T_pWSTR)Fwl_Malloc(MEDIA_PATH_SIZE);
	AK_ASSERT_PTR(pathstr, "MList_RemoveMediaItem(): pathstr", ret);
	memset(pathstr, 0, MEDIA_PATH_SIZE);
	Utl_UStrCpyN(pathstr, path, MAX_FILENM_LEN);

	mailbox.event = eMEDIALIST_EVT_DEL;
	mailbox.param.w.Param1 = (T_U32)pathstr;
	mailbox.param.w.Param2 = (T_U32)includeFile;

	MList_PostEvent(type, &mailbox);

    return AK_TRUE;
}

/**
* @brief Remove All Media Item (Audio / Video) From Media List
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval
*/
T_BOOL MList_RemoveAll (T_eMEDIA_LIST_TYPE type)
{
	T_BOOL			ret = AK_FALSE;
	T_SYS_MAILBOX 	mailbox;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_RemoveAll(): type err", ret);

	MList_Suspend(type, AK_TRUE);

	mailbox.event = eMEDIALIST_EVT_DELALL;
	MList_PostEvent(type, &mailbox);
    
    return AK_TRUE;
}



/**
* @brief Get Class Quantity in Tag (Artist / Album / Genre / Composer)
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eID3_TAGS tag : Artist / Album / Genre / Composer
* @return T_U16 Class Quantity
* @retval
*/
T_U16 MList_ID3_GetClassQty(T_eID3_TAGS tag)
{
	T_U16 				ret = 0;
	T_eMEDIA_LIST_TYPE	type = eMEDIA_LIST_AUDIO;
	
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_ID3_GetClassQty(): tag", ret);
	MList_Obtain_Semaphore(type);

	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	ret = (T_U16)MList_CalcQty(mList[type].plist->id3[tag].bitmap, MEDIA_BITMAP_SIZE);
	MList_Release_Semaphore(type);

	return ret;
}


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
T_U16 MList_ID3_GetClassName(T_pWSTR name, T_U16 classId, T_eID3_TAGS tag)
{
	T_U16				ret = 0;
	T_USTR_FILE			path = {0};
	T_U32				size = 0;
	T_MEDIA_LIST		*pAList = AK_NULL;
	T_eMEDIA_LIST_TYPE	type = eMEDIA_LIST_AUDIO;
	
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_ID3_GetClassName(): tag", ret);
	AK_ASSERT_VAL((classId < MAX_MEDIA_NUM), "MList_ID3_GetClassName(): classId", ret);
	AK_ASSERT_PTR(name, "MList_ID3_GetClassName(): name", ret);
	
	MList_Obtain_Semaphore(type);

	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pAList = mList[type].plist;

	if (FS_INVALID_HANDLE == pAList->id3[tag].fdFore)
	{
		MList_ID3_GetTagPath(path, tag);
		
		pAList->id3[tag].fdFore = Fwl_FileOpen(path, _FMODE_READ, _FMODE_READ);
		if (FS_INVALID_HANDLE == pAList->id3[tag].fdFore)
		{
			Fwl_Print(C2, M_MLIST, "MList_ID3_GetClassName(): pAList->id3[tag].fdFore open fail");
			MList_Release_Semaphore(type);
			return ret;
		}
	}

	if (0 == MList_GetBitValue(pAList->id3[tag].bitmap, MEDIA_BITMAP_SIZE, classId))
	{
		Fwl_Print(C2, M_MLIST, "MList_ID3_GetClassName bitval is 0 \r\n");
		MList_Release_Semaphore(type);
		return ret;
	}
	
	size = MEDIA_ID3_HEAD_SIZE + classId * MEDIA_CLASSNAME_SIZE;

	if (0 < Fwl_FileSeek(pAList->id3[tag].fdFore, size, _FSEEK_SET))
	{
		Fwl_FileRead(pAList->id3[tag].fdFore, name, MEDIA_CLASSNAME_SIZE);
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_ID3_GetClassName can't find\r\n");
	}

	ret = (T_U16)MList_CalcQty(pAList->id3[tag].bitmap, MEDIA_BITMAP_SIZE);
	MList_Release_Semaphore(type);

	return ret;
	
}


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
T_U16 MList_ID3_GetAlbumQty(T_pCWSTR name, T_eID3_TAGS tag)
{
	T_U16				ret = 0;
	T_U16				id = 0;
	T_eMEDIA_LIST_TYPE	type = eMEDIA_LIST_AUDIO;
	
	AK_ASSERT_PTR(name, "MList_ID3_GetAlbumQty(): name", ret);
	AK_ASSERT_VAL(*name, "MList_ID3_GetAlbumQty(): *name", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_ALBUM), "MList_ID3_GetAlbumQty(): tag", ret);

	MList_Obtain_Semaphore(type);
	
	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	id = MList_ID3_GetClassId(name, tag, AK_FALSE);

	if (id < MAX_MEDIA_NUM)
	{
		ret = mList[type].plist->id3[tag].index[id].qty;
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_ID3_GetAlbumQty can't find the name\r\n");
	}

	MList_Release_Semaphore(type);
	
	return ret;
}


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
T_U16 MList_ID3_GetAlbumName(T_pWSTR T_albumName, T_pCWSTR name, T_U16 id, T_eID3_TAGS tag)
{
	T_U16				ret = 0;
	T_U16				classId = 0;
	T_U16				pos = 0;
	T_MEDIA_LIST		*pAList = AK_NULL;
	T_eMEDIA_LIST_TYPE	type = eMEDIA_LIST_AUDIO;
	
	AK_ASSERT_VAL((tag < eID3_TAGS_ALBUM), "MList_ID3_GetAlbumName(): tag", ret);
	AK_ASSERT_VAL((id < MAX_MEDIA_NUM), "MList_ID3_GetAlbumName(): id", ret);
	AK_ASSERT_PTR(name, "MList_ID3_GetAlbumName(): name", ret);
	AK_ASSERT_VAL(*name, "MList_ID3_GetAlbumName(): *name", ret);
	AK_ASSERT_PTR(T_albumName, "MList_ID3_GetAlbumName(): T_albumName", ret);
	
	MList_Obtain_Semaphore(type);

	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pAList = mList[type].plist;

	classId = MList_ID3_GetClassId(name, tag, AK_FALSE);
	
	if (classId < MAX_MEDIA_NUM)
	{
		if (id < pAList->id3[tag].index[classId].qty)
		{
			pos = pAList->id3[tag].index[classId].pos + id;
			MList_Release_Semaphore(type);
			MList_ID3_GetClassName(T_albumName, pAList->id3[tag].idxmap[pos], eID3_TAGS_ALBUM);
			MList_Obtain_Semaphore(type);
		}
		else
		{
			Fwl_Print(C2, M_MLIST, "MList_ID3_GetAlbumName id err\r\n");
		}

		ret = pAList->id3[tag].index[classId].qty;
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_ID3_GetAlbumName can't find the name\r\n");
	}

	
	MList_Release_Semaphore(type);

	return ret;
}


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
T_U16 MList_ID3_GetSongQty(T_pCWSTR name, T_eID3_TAGS tag)
{
	T_U16				ret = 0;
	T_U16				id = 0;
	T_eMEDIA_LIST_TYPE	type = eMEDIA_LIST_AUDIO;
	
	AK_ASSERT_PTR(name, "MList_ID3_GetSongQty(): name", ret);
	AK_ASSERT_VAL(*name, "MList_ID3_GetSongQty(): *name", ret);
	AK_ASSERT_VAL(((eID3_TAGS_ALBUM==tag)||(eID3_TAGS_COMPOSER==tag)), "MList_ID3_GetSongQty(): tag", ret);

	MList_Obtain_Semaphore(type);
	
	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	id = MList_ID3_GetClassId(name, tag, AK_FALSE);

	if (id < MAX_MEDIA_NUM)
	{
		ret = mList[type].plist->id3[tag].index[id].qty;
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_ID3_GetSongQty can't find the name\r\n");
	}

	MList_Release_Semaphore(type);

	return ret;
}


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
T_U16 MList_ID3_GetSongPath(T_pWSTR path, T_pCWSTR className, T_U16 albumIdx, T_U16 songIdx, T_eID3_TAGS tag)
{
	T_U16				ret = 0;
	T_U16				pos = 0;
	T_U16				classId = 0;
	T_U16				albumId = 0;
	T_U16				songId = 0;
	T_U32				seeksize = 0;
	T_USTR_FILE			listpath = {0};
	T_USTR_FILE			classStr = {0};
	T_MEDIA_LIST		*pAList = AK_NULL;
	T_eMEDIA_LIST_TYPE	type = eMEDIA_LIST_AUDIO;
	
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_ID3_GetSongPath(): tag", ret);
	AK_ASSERT_VAL((albumIdx < MAX_MEDIA_NUM), "MList_ID3_GetSongPath(): albumId", ret);
	AK_ASSERT_VAL((songIdx < MAX_MEDIA_NUM), "MList_ID3_GetSongPath(): songId", ret);
	AK_ASSERT_PTR(className, "MList_ID3_GetSongPath(): className", ret);
	AK_ASSERT_VAL(*className, "MList_ID3_GetSongPath(): *className", ret);
	AK_ASSERT_PTR(path, "MList_ID3_GetSongPath(): path", ret);

	MList_Obtain_Semaphore(type);
	
	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pAList = mList[type].plist;

	classId = MList_ID3_GetClassId(className, tag, AK_FALSE);

	if (classId < MAX_MEDIA_NUM)
	{
		if (eID3_TAGS_ALBUM == tag
		|| eID3_TAGS_COMPOSER == tag)
		{
			if (songIdx < pAList->id3[tag].index[classId].qty)
			{
				pos = pAList->id3[tag].index[classId].pos + songIdx;
				MList_Release_Semaphore(type);
				MList_GetItem(path, pAList->id3[tag].idxmap[pos], eMEDIA_LIST_AUDIO);
				MList_Obtain_Semaphore(type);
			}
			else
			{
				Fwl_Print(C2, M_MLIST, "MList_ID3_GetSongPath songidx err\r\n");
			}
		}
		else
		{
			if (FS_INVALID_HANDLE == pAList->fdFore)
			{		
				Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);
				pAList->fdFore = Fwl_FileOpen(listpath, _FMODE_READ, _FMODE_READ);
				if (FS_INVALID_HANDLE == pAList->fdFore)
				{
					Fwl_Print(C2, M_MLIST, "MList_ID3_GetSongPath(): pAList->fdFore open fail");
					MList_Release_Semaphore(type);
					return ret;
				}
			}

			if (albumIdx < pAList->id3[tag].index[classId].qty)
			{
				pos = pAList->id3[tag].index[classId].pos + albumIdx;
				albumId = pAList->id3[tag].idxmap[pos];

				if (songIdx < pAList->id3[eID3_TAGS_ALBUM].index[albumId].qty)
				{
					pos = pAList->id3[eID3_TAGS_ALBUM].index[albumId].pos + songIdx;
					songId = pAList->id3[eID3_TAGS_ALBUM].idxmap[pos];

					if (0 == MList_GetBitValue(pAList->bitmap, MEDIA_BITMAP_SIZE, songId))
					{
						ret = pAList->id3[tag].index[classId].qty;
						MList_Release_Semaphore(type);
						return ret;
					}

					seeksize = MEDIA_FILE_HEAD_SIZE + songId * MEDIA_FILEITEM_SIZE + MEDIA_PATH_SIZE + tag * MEDIA_CLASSNAME_SIZE;
					
					if (0 < Fwl_FileSeek(pAList->fdFore, seeksize, _FSEEK_SET))
					{
						Fwl_FileRead(pAList->fdFore, classStr, MEDIA_CLASSNAME_SIZE);

						if (0 == Utl_UStrCmp(classStr, className))
						{		
							seeksize = MEDIA_FILE_HEAD_SIZE + songId * MEDIA_FILEITEM_SIZE;
							Fwl_FileSeek(pAList->fdFore, seeksize, _FSEEK_SET);
							Fwl_FileRead(pAList->fdFore, path, MEDIA_PATH_SIZE);
							ret = pAList->id3[tag].index[classId].qty;
							MList_Release_Semaphore(type);

							return ret;
						}
					}
					else
					{
						Fwl_Print(C2, M_MLIST, "MList_ID3_GetSongPath can't find\r\n");
					}
				}
				else
				{
					Fwl_Print(C2, M_MLIST, "MList_ID3_GetSongPath songidx err\r\n");
				}
			}
			else
			{
				Fwl_Print(C2, M_MLIST, "MList_ID3_GetSongPath albumidx err\r\n");
			}
		}

		ret = pAList->id3[tag].index[classId].qty;
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_ID3_GetSongPath can't find the name\r\n");
	}

	
	MList_Release_Semaphore(type);

	return ret;
}


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
T_BOOL MList_ID3_RemoveClass(T_pCWSTR name, T_eID3_TAGS tag)
{
	T_BOOL			ret = AK_FALSE;
	T_SYS_MAILBOX	mailbox;
	T_pWSTR			pathstr = AK_NULL;
	
	AK_ASSERT_PTR(name, "MList_ID3_RemoveClass(): path", ret);
	AK_ASSERT_VAL(*name, "MList_ID3_RemoveClass(): *path", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_ID3_RemoveClass(): tag", ret);

	pathstr = (T_pWSTR)Fwl_Malloc(MEDIA_PATH_SIZE);
	AK_ASSERT_PTR(pathstr, "MList_ID3_RemoveClass(): pathstr", ret);
	memset(pathstr, 0, MEDIA_PATH_SIZE);
	Utl_UStrCpyN(pathstr, name, MAX_FILENM_LEN);

	mailbox.event = eMEDIALIST_EVT_ID3_DELCLASS;
	mailbox.param.w.Param1 = (T_U32)pathstr;
	mailbox.param.w.Param2 = (T_U32)tag;

	MList_PostEvent(eMEDIA_LIST_AUDIO, &mailbox);

    return AK_TRUE;
}


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
T_BOOL MList_ID3_RemoveItem(T_pCWSTR name, T_eID3_TAGS tag)
{
	T_BOOL			ret = AK_FALSE;
	T_SYS_MAILBOX	mailbox;
	T_pWSTR			pathstr = AK_NULL;
	
	AK_ASSERT_PTR(name, "MList_ID3_RemoveItem(): name", ret);
	AK_ASSERT_VAL(*name, "MList_ID3_RemoveItem(): *name", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_ID3_RemoveItem(): tag", ret);

	pathstr = (T_pWSTR)Fwl_Malloc(MEDIA_PATH_SIZE);
	AK_ASSERT_PTR(pathstr, "MList_ID3_RemoveItem(): pathstr", ret);
	memset(pathstr, 0, MEDIA_PATH_SIZE);
	Utl_UStrCpyN(pathstr, name, MAX_FILENM_LEN);

	mailbox.event = eMEDIALIST_EVT_ID3_DELITEM;
	mailbox.param.w.Param1 = (T_U32)pathstr;
	mailbox.param.w.Param2 = (T_U32)tag;

	MList_PostEvent(eMEDIA_LIST_AUDIO, &mailbox);

    return AK_TRUE;
}




/**
* @brief Remove All Media Item (Audio / Video) From Media List  background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval
*/
T_BOOL MList_BG_RemoveAll (T_eMEDIA_LIST_TYPE type)
{
	T_BOOL			ret = AK_FALSE;
	T_U8			i = 0;
	T_USTR_FILE 	listpath = {0};
	T_USTR_FILE		id3path[eID3_TAGS_NUM] = {0};
	T_MEDIA_LIST 	*pList = AK_NULL;
	T_SYS_MAILBOX	mailbox;

	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_BG_RemoveAll(): type err", ret);

	if (eMEDIA_LIST_AUDIO == type)
	{
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);

		Eng_StrMbcs2Ucs((GENRE_INDEX_FILE), id3path[eID3_TAGS_GENRE]);
		Eng_StrMbcs2Ucs((ARTIST_INDEX_FILE), id3path[eID3_TAGS_ARTIST]);
		Eng_StrMbcs2Ucs((ALBUM_INDEX_FILE), id3path[eID3_TAGS_ALBUM]);
		Eng_StrMbcs2Ucs((COMPOSER_INDEX_FILE), id3path[eID3_TAGS_COMPOSER]);
	}
	else if (eMEDIA_LIST_VIDEO == type)
	{
		Eng_StrMbcs2Ucs((VIDEOLIST_DEF_FILE), listpath);
	}
	else
	{
		
	}
	
	MList_Obtain_Semaphore(type);

	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pList = mList[type].plist;

	pList->state |= MEDIA_LIST_STATE_RUNNING;

	if (FS_INVALID_HANDLE == pList->fdBack)
	{
		pList->fdBack = Fwl_FileOpen(listpath, _FMODE_WRITE, _FMODE_WRITE);
		if (FS_INVALID_HANDLE == pList->fdBack)
		{
			pList->state ^= MEDIA_LIST_STATE_RUNNING;
			MList_Release_Semaphore(type);
			return ret;
		}
	}

	memset(pList->bitmap, 0, MEDIA_FILE_HEAD_SIZE - MEDIA_NUM_SIZE);

	if (eMEDIA_LIST_AUDIO == type)
	{
		for (i=0; i<eID3_TAGS_NUM; i++)
		{
			if (FS_INVALID_HANDLE == pList->id3[i].fdBack)
			{
				pList->id3[i].fdBack = Fwl_FileOpen(id3path[i], _FMODE_WRITE, _FMODE_WRITE);
			}

			memset(pList->id3[i].bitmap, 0, MEDIA_ID3_HEAD_SIZE);
		}
	}
	
	pList->state ^= MEDIA_LIST_STATE_RUNNING;

	mList[type].delFlag = AK_TRUE;

	mailbox.event = eMEDIALIST_EVT_CLOSE;

	MList_PostEvent(type, &mailbox);

	MList_Release_Semaphore(type);
	
	return AK_TRUE;
}


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
T_eMEDIALIST_ADD_RET MList_BG_AddPath(T_pWSTR path, T_BOOL subFolder, T_BOOL resvOld, T_eMEDIA_LIST_TYPE type)
{
	T_U8					i = 0;
	T_eMEDIALIST_ADD_RET 	ret = eMEDIALIST_ADD_NONE;
	T_MEDIA_LIST 			*pList = AK_NULL;
	T_USTR_FILE				pathstr = {0};
	T_USTR_FILE				diskpath = {0};
	T_SYS_MAILBOX			mailbox;
	T_USTR_FILE 			listpath = {0};
	T_USTR_FILE				id3path[eID3_TAGS_NUM] = {0};
	
	AK_ASSERT_PTR(path, "MList_BG_AddPath(): path", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_BG_AddPath(): type err", ret);

	Utl_UStrCpyN(pathstr, path, MAX_FILENM_LEN);
	path = Fwl_Free(path);

#ifdef OS_ANYKA
	if (!Fwl_CheckDriverIsValid(pathstr))
    {
        Fwl_Print(C2, M_MLIST, "MList_BG_AddPath invalid path!!!\r\n");
        return ret;
    }

    Eng_StrMbcs2Ucs(DRI_B, diskpath);

    if (!Fwl_CheckDriverIsValid(diskpath))
    {
        Fwl_Print(C2, M_MLIST, "MList_BG_AddPath default path is not mounted!!!\r\n");
        return ret;
    }
#endif

	if (eMEDIA_LIST_AUDIO == type)
	{
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);

		Eng_StrMbcs2Ucs((GENRE_INDEX_FILE), id3path[eID3_TAGS_GENRE]);
		Eng_StrMbcs2Ucs((ARTIST_INDEX_FILE), id3path[eID3_TAGS_ARTIST]);
		Eng_StrMbcs2Ucs((ALBUM_INDEX_FILE), id3path[eID3_TAGS_ALBUM]);
		Eng_StrMbcs2Ucs((COMPOSER_INDEX_FILE), id3path[eID3_TAGS_COMPOSER]);
	}
	else if (eMEDIA_LIST_VIDEO == type)
	{
		Eng_StrMbcs2Ucs((VIDEOLIST_DEF_FILE), listpath);
	}
	else
	{
		
	}
	
	MList_Obtain_Semaphore(type);

	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pList = mList[type].plist;

	if (pList->state & MEDIA_LIST_REQ_SUSPEND)
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_AddPath force to return!!!\r\n");
		MList_Release_Semaphore(type);
		return ret;
	}

	pList->state |= MEDIA_LIST_STATE_RUNNING;
	
    pList->bAdding = AK_TRUE;

	if (FS_INVALID_HANDLE == pList->fdBack)
	{
		pList->fdBack = Fwl_FileOpen(listpath, _FMODE_WRITE, _FMODE_WRITE);
		if (FS_INVALID_HANDLE == pList->fdBack)
		{
			pList->state ^= MEDIA_LIST_STATE_RUNNING;
			pList->bAdding = AK_FALSE;
			MList_Release_Semaphore(type);
			return ret;
		}
	}

	if (eMEDIA_LIST_AUDIO == type)
	{
		for (i=0; i<eID3_TAGS_NUM; i++)
		{
			if (FS_INVALID_HANDLE == pList->id3[i].fdBack)
			{
				pList->id3[i].fdBack = Fwl_FileOpen(id3path[i], _FMODE_WRITE, _FMODE_WRITE);
			}
		}
	}

	if (!resvOld)
	{
		memset(pList->bitmap, 0, MEDIA_FILE_HEAD_SIZE - MEDIA_NUM_SIZE);

		if (eMEDIA_LIST_AUDIO == type)
		{
			for (i=0; i<eID3_TAGS_NUM; i++)
			{
				memset(pList->id3[i].bitmap, 0, MEDIA_ID3_HEAD_SIZE);
			}
		}
	}

	MList_Release_Semaphore(type);

	Fwl_Print(C3, M_MLIST, "type %d ,enter MList_BG_Add!",type);
	ret = MList_BG_Add(type, pathstr, subFolder);
	Fwl_Print(C3, M_MLIST, "type %d ,exit MList_BG_Add!",type);

	MList_Obtain_Semaphore(type);

	pList->bAdding = AK_FALSE;
	pList->state ^= MEDIA_LIST_STATE_RUNNING;
	mList[type].addFlag |= eADD_FLAG_NEW;
	
	mailbox.event = eMEDIALIST_EVT_CLOSE;

	MList_PostEvent(type, &mailbox);

	MList_Release_Semaphore(type);

	return ret;
}

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
T_BOOL MList_BG_DelPath(T_pWSTR path, T_BOOL includeFile, T_eMEDIA_LIST_TYPE type)
{
	T_BOOL			ret = AK_FALSE;
	T_MEDIA_LIST 	*pList = AK_NULL;
	T_USTR_FILE		pathstr = {0};
	T_SYS_MAILBOX	mailbox;
	
	AK_ASSERT_PTR(path, "MList_BG_DelPath(): path", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_BG_DelPath(): type err", ret);

	Utl_UStrCpyN(pathstr, path, MAX_FILENM_LEN);
	path = Fwl_Free(path);
    
	MList_Obtain_Semaphore(type);
	
	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pList = mList[type].plist;

	if (pList->state & MEDIA_LIST_REQ_SUSPEND)
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_DelPath force to return!!!\r\n");
		MList_Release_Semaphore(type);
		return ret;
	}

	pList->state |= MEDIA_LIST_STATE_RUNNING;
	ret = MList_BG_Del(type, pathstr);
	pList->state ^= MEDIA_LIST_STATE_RUNNING;
	mList[type].delFlag = AK_TRUE;

	mailbox.event = eMEDIALIST_EVT_CLOSE;

	MList_PostEvent(type, &mailbox);
	
	MList_Release_Semaphore(type);

	return ret;
}


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
T_BOOL MList_BG_ID3_RemoveClass(T_pWSTR name, T_eID3_TAGS tag)
{
	T_BOOL				ret = AK_FALSE;
	T_U16				id = 0;
	T_U16				i = 0;
	T_USTR_FILE			classname = {0};
	T_U16				pos = 0;
	T_SYS_MAILBOX		mailbox;
	T_MEDIA_LIST		*pAList = AK_NULL;
	T_eMEDIA_LIST_TYPE	type = eMEDIA_LIST_AUDIO;
	
	AK_ASSERT_PTR(name, "MList_BG_ID3_RemoveClass(): name", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_BG_ID3_RemoveClass(): tag", ret);

	Utl_UStrCpyN(classname, name, MAX_FILENM_LEN);
	name = Fwl_Free(name);

	AK_ASSERT_VAL(*classname, "MList_BG_ID3_RemoveClass(): *classname", ret);
	MList_Obtain_Semaphore(type);
	
	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pAList = mList[type].plist;

	if (pAList->state & MEDIA_LIST_REQ_SUSPEND)
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_ID3_RemoveClass force to return!!!\r\n");
		MList_Release_Semaphore(type);
		return ret;
	}

	pAList->state |= MEDIA_LIST_STATE_RUNNING;

	id = MList_ID3_GetClassId(classname, tag, AK_TRUE);

	if (id < MAX_MEDIA_NUM)
	{
		MList_SetBitValue(pAList->id3[tag].bitmap, MEDIA_BITMAP_SIZE, id, 0);
		pAList->id3[tag].ckSum[id]= 0;

		pos = pAList->id3[tag].index[id].pos;
		
		for (i=pos; i<MAX_MEDIA_NUM-pAList->id3[tag].index[id].qty; i++)
		{
			pAList->id3[tag].idxmap[i] = pAList->id3[tag].idxmap[i+pAList->id3[tag].index[id].qty];
		}

		for (i=MAX_MEDIA_NUM-pAList->id3[tag].index[id].qty; i<MAX_MEDIA_NUM; i++)
		{
			pAList->id3[tag].idxmap[i] = 0;
		}

		for (i=id+1; i<MAX_MEDIA_NUM; i++)
		{
			pAList->id3[tag].index[i].pos -= pAList->id3[tag].index[id].qty;
		}

		if (eID3_TAGS_ALBUM == tag)
		{
			MList_BG_ID3_DelAlbum(id, eID3_TAGS_GENRE);
			MList_BG_ID3_DelAlbum(id, eID3_TAGS_ARTIST);
		}
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_ID3_RemoveClass can't find the name\r\n");
	}

	pAList->state ^= MEDIA_LIST_STATE_RUNNING;
	mList[type].delFlag = AK_TRUE;
	
	mailbox.event = eMEDIALIST_EVT_CLOSE;
	MList_PostEvent(type, &mailbox);
	
	MList_Release_Semaphore(type);
	return AK_TRUE;
}



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
T_BOOL MList_BG_ID3_RemoveItem(T_pWSTR name, T_eID3_TAGS tag)
{
	T_BOOL				ret = AK_FALSE;
	T_USTR_FILE			listpath = {0};
	T_U16				id = 0;
	T_U32				size = 0;
	T_eID3_WRITE_RET	writeRet = eID3_WRITE_NONE;
	T_USTR_FILE			classname[eID3_TAGS_NUM] = {0};
	T_U16				albumId = 0;
	T_USTR_FILE			pathstr = {0};
	T_SYS_MAILBOX		mailbox;
	T_MEDIA_LIST		*pAList = AK_NULL;
	T_eMEDIA_LIST_TYPE	type = eMEDIA_LIST_AUDIO;
	
	AK_ASSERT_PTR(name, "MList_BG_ID3_RemoveItem(): name", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_BG_ID3_RemoveItem(): tag", ret);

	Utl_UStrCpyN(pathstr, name, MAX_FILENM_LEN);
	name = Fwl_Free(name);

	AK_ASSERT_VAL(*pathstr, "MList_BG_ID3_RemoveItem(): *pathstr", ret);
	MList_Obtain_Semaphore(type);
	
	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pAList = mList[type].plist;

	if (pAList->state & MEDIA_LIST_REQ_SUSPEND)
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_ID3_RemoveItem force to return!!!\r\n");
		MList_Release_Semaphore(type);
		return ret;
	}

	pAList->state |= MEDIA_LIST_STATE_RUNNING;

	Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);

	if (FS_INVALID_HANDLE == pAList->fdBack)
	{
		pAList->fdBack = Fwl_FileOpen(listpath, _FMODE_WRITE, _FMODE_WRITE);
		if (FS_INVALID_HANDLE == pAList->fdBack)
		{
			Fwl_Print(C2, M_MLIST, "MList_BG_ID3_RemoveItem(): pAList->fdBack open fail");
			pAList->state ^= MEDIA_LIST_STATE_RUNNING;
			MList_Release_Semaphore(type);
			return ret;
		}
	}

	id = MList_GetFileId(pathstr, type, AK_TRUE);

	if (id < MAX_MEDIA_NUM)
	{
		size = MEDIA_FILE_HEAD_SIZE + MEDIA_FILEITEM_SIZE * id + MEDIA_PATH_SIZE;
		Fwl_FileSeek(pAList->fdBack, size, _FSEEK_SET);
		Fwl_FileRead(pAList->fdBack, classname[eID3_TAGS_GENRE], MEDIA_CLASSNAME_SIZE);
		Fwl_FileRead(pAList->fdBack, classname[eID3_TAGS_ARTIST], MEDIA_CLASSNAME_SIZE);
		Fwl_FileRead(pAList->fdBack, classname[eID3_TAGS_ALBUM], MEDIA_CLASSNAME_SIZE);
		Fwl_FileRead(pAList->fdBack, classname[eID3_TAGS_COMPOSER], MEDIA_CLASSNAME_SIZE);

		if (eID3_TAGS_ALBUM == tag 
			|| eID3_TAGS_COMPOSER == tag)
		{
			writeRet = MList_BG_ID3_DelClassInfo(id, classname[tag], tag, &albumId);

			if (eID3_TAGS_ALBUM == tag)
			{
				if (eID3_WRITE_NAME == writeRet)
		    	{
					MList_BG_ID3_DelClassInfo(albumId, classname[eID3_TAGS_ARTIST], eID3_TAGS_ARTIST, AK_NULL);
					MList_BG_ID3_DelClassInfo(albumId, classname[eID3_TAGS_GENRE], eID3_TAGS_GENRE, AK_NULL);
		    	}
		    	else if (eID3_WRITE_HEAD_ONLY == writeRet)
		    	{
					if (!MList_BG_ID3_HasItem(albumId, classname[eID3_TAGS_ARTIST], eID3_TAGS_ARTIST))
					{
						MList_BG_ID3_DelClassInfo(albumId, classname[eID3_TAGS_ARTIST], eID3_TAGS_ARTIST, AK_NULL);
					}

					if (!MList_BG_ID3_HasItem(albumId, classname[eID3_TAGS_GENRE], eID3_TAGS_GENRE))
					{
						MList_BG_ID3_DelClassInfo(albumId, classname[eID3_TAGS_GENRE], eID3_TAGS_GENRE, AK_NULL);
					}
		    	}
	    	}
    	}
    	else if(eID3_TAGS_ARTIST == tag 
			|| eID3_TAGS_GENRE == tag)
    	{
    		T_U8 tmpstr[MEDIA_CLASSNAME_SIZE] = {0};
    		
			size = MEDIA_FILE_HEAD_SIZE + MEDIA_FILEITEM_SIZE * id 
				+ MEDIA_PATH_SIZE + tag * MEDIA_CLASSNAME_SIZE;
				
			Fwl_FileSeek(pAList->fdBack, size, _FSEEK_SET);
			Fwl_FileWrite(pAList->fdBack, tmpstr, MEDIA_CLASSNAME_SIZE);

			albumId = MList_ID3_GetClassId(classname[eID3_TAGS_ALBUM], eID3_TAGS_ALBUM, AK_TRUE);

			if (albumId < MAX_MEDIA_NUM)
			{
				if (!MList_BG_ID3_HasItem(albumId, classname[tag], tag))
				{
					MList_BG_ID3_DelClassInfo(albumId, classname[tag], tag, AK_NULL);
				}
			}
    	}
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_ID3_RemoveItem can't find the name\r\n");
	}

	pAList->state ^= MEDIA_LIST_STATE_RUNNING;
	mList[type].delFlag = AK_TRUE;
	
	mailbox.event = eMEDIALIST_EVT_CLOSE;
	MList_PostEvent(type, &mailbox);
	
	MList_Release_Semaphore(type);
	return AK_TRUE;
}


/**
* @brief Check Media List (Audio / Video) is adding or not
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
T_BOOL MList_IsAdding(T_eMEDIA_LIST_TYPE type)
{
	T_BOOL			ret = AK_FALSE;
	T_MEDIA_LIST	*pList = AK_NULL;

	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_IsAdding(): type err", ret);

	MList_Obtain_Semaphore(type);
	
	pList = mList[type].plist;

	if (AK_NULL != pList)
	{
		ret = pList->bAdding;
	}

	MList_Release_Semaphore(type);

	return ret;
}


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
T_U16 MList_GetPathId(T_pCWSTR filename, T_eMEDIA_LIST_TYPE type)
{
	T_U16 			ret = MAX_MEDIA_NUM;
	T_MEDIA_LIST	*pList = AK_NULL;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_GetPathId(): type err", ret);
	AK_ASSERT_PTR(filename, "MList_GetPathId(): filename", ret);
	AK_ASSERT_VAL(*filename, "MList_GetPathId(): *filename", ret);

	MList_Obtain_Semaphore(type);
	
	if (AK_NULL == mList[type].plist
		&& MList_Init(type) < 0)
	{
		MList_Release_Semaphore(type);
		return ret;
	}

	pList = mList[type].plist;

	ret = MList_GetFileId(filename, type, AK_FALSE);
	
	MList_Release_Semaphore(type);

	return ret;	
}



/**
* @brief Add class info background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U16 id : file id, when tag is Album/Composer; album id, when tag is Artist/Genre
* @param in T_pCWSTR classname : class name
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @param out T_U16 *albumId : album id,when tag is Album; no use when tag is not album
* @return T_eID3_WRITE_RET
* @retval
*/
static T_eID3_WRITE_RET MList_BG_ID3_AddClassInfo(T_U16 Id, T_pCWSTR classname, T_eID3_TAGS tag, T_U16 *albumId)
{
	T_U16				i = 0;
	T_U16				pos = 0;
	T_eID3_WRITE_RET	ret = eID3_WRITE_NONE;
	T_U32				size = 0;
	T_USTR_FILE			path = {0};
	T_U8				ckSum = 0;
	T_U16				emptyId = 0;
	T_U16				classId = 0;
	T_MEDIA_LIST		*pAList = mList[eMEDIA_LIST_AUDIO].plist;
	
	AK_ASSERT_PTR(pAList, "MList_BG_ID3_AddClassInfo(): pAList", ret);
	AK_ASSERT_PTR(pAList->id3, "MList_BG_ID3_AddClassInfo(): pAList->id3", ret);
	AK_ASSERT_VAL((Id < MAX_MEDIA_NUM), "MList_BG_ID3_AddClassInfo(): Id", ret);
	AK_ASSERT_PTR(classname, "MList_BG_ID3_AddClassInfo(): classname", ret);
	AK_ASSERT_VAL(*classname, "MList_BG_ID3_AddClassInfo(): *classname", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_BG_ID3_AddClassInfo(): tag", ret);

	classId = MList_ID3_GetClassId(classname, tag, AK_TRUE);

	if (classId < MAX_MEDIA_NUM)
	{
		if (eID3_TAGS_ALBUM == tag && AK_NULL != albumId)
		{
			*albumId = classId;
		}

		//检查要添加的项是否已经存在
		if (tag < eID3_TAGS_ALBUM)
		{
			pos = pAList->id3[tag].index[classId].pos;
			
			for (i=pos; i<pos + pAList->id3[tag].index[classId].qty;i++)
			{
				if (pAList->id3[tag].idxmap[i] == Id)
				{
					//Fwl_Print(C3, M_MLIST, "it is already here!");
					ret = eID3_WRITE_NONE;
					return ret;
				}
			}
		}

		pos = pAList->id3[tag].index[classId].pos + pAList->id3[tag].index[classId].qty;
		pAList->id3[tag].index[classId].qty++;

		for (i=classId+1; i<MAX_MEDIA_NUM; i++)
		{
			if (0 != pAList->id3[tag].index[i].pos)
			{
				pAList->id3[tag].index[i].pos++;
			}
		}

		for (i=MAX_MEDIA_NUM-1; i>pos; i--)
		{
			pAList->id3[tag].idxmap[i] = pAList->id3[tag].idxmap[i-1];
		}
		
		pAList->id3[tag].idxmap[pos] = Id;

		ret = eID3_WRITE_HEAD_ONLY;
		return ret;
	}

	if (FS_INVALID_HANDLE == pAList->id3[tag].fdBack)
	{
		MList_ID3_GetTagPath(path, tag);
		
		pAList->id3[tag].fdBack = Fwl_FileOpen(path, _FMODE_WRITE, _FMODE_WRITE);
		AK_ASSERT_VAL((FS_INVALID_HANDLE != pAList->id3[tag].fdBack), "MList_BG_ID3_AddClassInfo(): pAList->id3[tag].fdBack", AK_FALSE);
	}

	ckSum = (T_U8)MList_CalcChecksum(classname);

	emptyId = MList_FindFirstHoleId(pAList->id3[tag].bitmap, MEDIA_BITMAP_SIZE);

	if (emptyId < MAX_MEDIA_NUM)
	{
		if (eID3_TAGS_ALBUM == tag && AK_NULL != albumId)
		{
			*albumId = emptyId;
		}

		size = MEDIA_ID3_HEAD_SIZE + emptyId * MEDIA_CLASSNAME_SIZE;
		Fwl_FileSeek(pAList->id3[tag].fdBack, size, _FSEEK_SET);

		//Printf_UC(classname);
		if (0 == Fwl_FileWrite(pAList->id3[tag].fdBack, classname, MEDIA_CLASSNAME_SIZE))
		{
			return ret;
		}
		
		if (0 == emptyId)
		{
			pAList->id3[tag].index[emptyId].pos = 0;
		}
		else
		{
			pAList->id3[tag].index[emptyId].pos = pAList->id3[tag].index[emptyId-1].pos
												+ pAList->id3[tag].index[emptyId-1].qty;
		}

		pAList->id3[tag].index[emptyId].qty = 1;

		for (i=emptyId+1; i<MAX_MEDIA_NUM; i++)
		{
			if (0 != pAList->id3[tag].index[i].pos)
			{
				pAList->id3[tag].index[i].pos++;
			}
		}

		pos = pAList->id3[tag].index[emptyId].pos;
		
		for (i=MAX_MEDIA_NUM-1; i>pos; i--)
		{
			pAList->id3[tag].idxmap[i] = pAList->id3[tag].idxmap[i-1];
		}
			
		pAList->id3[tag].idxmap[pos] = Id;
		
		pAList->id3[tag].ckSum[emptyId] = ckSum;
		MList_SetBitValue(pAList->id3[tag].bitmap, MEDIA_BITMAP_SIZE, emptyId, 1);
				
		ret = eID3_WRITE_NAME;
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_ID3_AddClassInfo can't find empty space\r\n");
	}

	return ret;
}


/**
* @brief Add id3 info background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR path : file path
* @param in T_U16 id : file id
* @param in T_pFILE fp : file handle
* @return T_eID3_WRITE_RET
* @retval
*/
static T_BOOL MList_BG_ID3_AddId3Info(T_pCWSTR path, T_U16 id, T_pFILE fp)
{
	T_U8				i = 0;
	T_USTR_FILE			listpath = {0};
	T_USTR_FILE			id3path[eID3_TAGS_NUM] = {0};
	T_BOOL				ret = AK_FALSE;
	T_SONG_INFO			songInfo ={0};
	T_U16				albumId = 0;
	T_S32				seeksize = 0;
	T_eID3_WRITE_RET	writeRet = eID3_WRITE_NONE;
	T_MEDIA_LIST		*pAList = mList[eMEDIA_LIST_AUDIO].plist;

	AK_ASSERT_PTR(path, "MList_BG_ID3_AddId3Info(): path", ret);
	AK_ASSERT_VAL(*path, "MList_BG_ID3_AddId3Info(): *path", ret);
	AK_ASSERT_PTR(pAList, "MList_BG_ID3_AddId3Info(): pAList", ret);
		
	if (FS_INVALID_HANDLE == pAList->fdBack)
	{
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);
		pAList->fdBack = Fwl_FileOpen(listpath, _FMODE_WRITE, _FMODE_WRITE);
		AK_ASSERT_VAL((FS_INVALID_HANDLE != pAList->fdBack), "MList_BG_ID3_AddId3Info(): pAList->fdBack", ret);
	}

	Eng_StrMbcs2Ucs((GENRE_INDEX_FILE), id3path[eID3_TAGS_GENRE]);
	Eng_StrMbcs2Ucs((ARTIST_INDEX_FILE), id3path[eID3_TAGS_ARTIST]);
	Eng_StrMbcs2Ucs((ALBUM_INDEX_FILE), id3path[eID3_TAGS_ALBUM]);
	Eng_StrMbcs2Ucs((COMPOSER_INDEX_FILE), id3path[eID3_TAGS_COMPOSER]);

	for (i=0; i<eID3_TAGS_NUM; i++)
	{
		if (FS_INVALID_HANDLE == pAList->id3[i].fdBack)
		{
			pAList->id3[i].fdBack = Fwl_FileOpen(id3path[i], _FMODE_WRITE, _FMODE_WRITE);
			AK_ASSERT_VAL((FS_INVALID_HANDLE != pAList->id3[i].fdBack), "MList_BG_ID3_AddId3Info(): pAList->id3[i].fdBack", ret);
		}
	}

	if (ID3_GetMetaInfo((T_pVOID)fp, &songInfo))
    {
		seeksize = MEDIA_FILE_HEAD_SIZE + MEDIA_FILEITEM_SIZE * id + MEDIA_PATH_SIZE;
		Fwl_FileSeek(pAList->fdBack, seeksize, _FSEEK_SET);
		
		Fwl_FileWrite(pAList->fdBack, songInfo.genre, MEDIA_CLASSNAME_SIZE);
		Fwl_FileWrite(pAList->fdBack, songInfo.artist, MEDIA_CLASSNAME_SIZE);
		Fwl_FileWrite(pAList->fdBack, songInfo.album, MEDIA_CLASSNAME_SIZE);
		Fwl_FileWrite(pAList->fdBack, songInfo.composer, MEDIA_CLASSNAME_SIZE);
	
    	writeRet = MList_BG_ID3_AddClassInfo(id, songInfo.album, eID3_TAGS_ALBUM, &albumId);
    	MList_BG_ID3_AddClassInfo(id, songInfo.composer, eID3_TAGS_COMPOSER, AK_NULL);

		MList_BG_ID3_AddClassInfo(albumId, songInfo.artist, eID3_TAGS_ARTIST, AK_NULL);
		MList_BG_ID3_AddClassInfo(albumId, songInfo.genre, eID3_TAGS_GENRE, AK_NULL);


    	if (eID3_WRITE_NAME == writeRet
    	||eID3_WRITE_HEAD_ONLY == writeRet)
    	{
			ret = AK_TRUE;
    	}
    }

	return ret;
}


/**
* @brief Add file to audio / video list background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @param in T_pCWSTR path : file path
* @param in T_pFILE fp : file handle
* @return T_eMEDIALIST_ADD_RET
* @retval
*/
static T_eMEDIALIST_ADD_RET MList_BG_AddFile(T_eMEDIA_LIST_TYPE type, T_pCWSTR path, T_pFILE fp)
{
	T_eMEDIALIST_ADD_RET	ret = eMEDIALIST_ADD_NONE;
	T_U16					emptyId = 0;
	T_USTR_FILE				listpath = {0};
	T_USTR_FILE				namestr = {0};
	T_USTR_FILE				diskpath = {0};
	T_S32					seeksize = 0;
	T_MEDIA_LIST			*pList = AK_NULL;
	T_U16					maxqty = 0;
	
	
	AK_ASSERT_PTR(path, "MList_BG_AddFile(): path", ret);
	AK_ASSERT_VAL(*path, "MList_BG_AddFile(): *path", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_BG_AddFile(): type err", ret);

#ifdef OS_ANYKA
	if (!Fwl_CheckDriverIsValid(path))
    {
        Fwl_Print(C2, M_MLIST, "MList_BG_AddFile invalid path!!!\r\n");
        return ret;
    }

    Eng_StrMbcs2Ucs(DRI_B, diskpath);

    if (!Fwl_CheckDriverIsValid(diskpath))
    {
        Fwl_Print(C2, M_MLIST, "MList_BG_AddFile default path is not mounted!!!\r\n");
        return ret;
    }
#endif

	pList = mList[type].plist;

	if (eMEDIA_LIST_AUDIO == type)
	{
		if (!AudioPlayer_IsSupportFile(path))
		{
			Fwl_Print(C3, M_MLIST, "MList_BG_AddFile invalid file type!!!\r\n");
			return ret;
		}
		
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);
		maxqty = MAX_MEDIA_NUM;
	}
	else if (eMEDIA_LIST_VIDEO == type)
	{
		if (!AVIPlayer_IsSupportFileType(path))
		{
			Fwl_Print(C3, M_MLIST, "MList_BG_AddFile invalid file type!!!\r\n");
			return ret;
		}
		
		Eng_StrMbcs2Ucs((VIDEOLIST_DEF_FILE), listpath);
		maxqty = VIDEOLIST_MAX_ITEM_QTY;
	}
	else
	{
		
	}

	AK_ASSERT_PTR(pList, "MList_BG_AddFile(): pList", ret);

	MList_Obtain_Semaphore(type);

	if (pList->state & MEDIA_LIST_REQ_SUSPEND)
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_AddFile force to return!!!\r\n");
		MList_Release_Semaphore(type);
		return ret;
	}
		
	if (FS_INVALID_HANDLE == pList->fdBack)
	{
		pList->fdBack = Fwl_FileOpen(listpath, _FMODE_WRITE, _FMODE_WRITE);
		if (FS_INVALID_HANDLE == pList->fdBack)
		{
			Fwl_Print(C2, M_MLIST, "MList_BG_AddFile(): pList->fdBack open fail");
			MList_Release_Semaphore(type);
			return ret;
		}
	}
		
	emptyId = MList_FindFirstHoleId(pList->bitmap, MEDIA_BITMAP_SIZE);

	if (emptyId < maxqty)
	{
		T_U8 emptydata[eID3_TAGS_NUM*MEDIA_CLASSNAME_SIZE] = {0};

		if (MList_GetFileId(path, type, AK_TRUE) < MAX_MEDIA_NUM)
		{
			Fwl_Print(C3, M_MLIST, "MList_BG_AddFile(): it is already here!");
			MList_Release_Semaphore(type);
			return ret;
		}
		
		Utl_UStrCpyN(namestr, path, MAX_FILENM_LEN);
		
		seeksize = MEDIA_FILE_HEAD_SIZE + MEDIA_FILEITEM_SIZE * emptyId;
		Fwl_FileSeek(pList->fdBack, seeksize, _FSEEK_SET);
		
		if (0 < Fwl_FileWrite(pList->fdBack, namestr, MEDIA_PATH_SIZE))
		{
			MList_SetBitValue(pList->bitmap, MEDIA_BITMAP_SIZE, emptyId, 1);
			pList->ckSum[emptyId] = MList_CalcChecksum(path);
			pList->playInfo[emptyId].Cnt = 0;
			pList->playInfo[emptyId].Time = GetSysTimeSeconds();

			Fwl_FileWrite(pList->fdBack, emptydata, eID3_TAGS_NUM*MEDIA_CLASSNAME_SIZE);

			if (eMEDIA_LIST_AUDIO == type)
			{
				MList_BG_ID3_AddId3Info(namestr, emptyId, fp);
			}
			
			ret = eMEDIALIST_ADD_SUCCESS;
		}
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_AddFile can't find empty space\r\n");
		ret = eMEDIALIST_ADD_NOSPACE;
	}

	MList_Release_Semaphore(type);
	return ret;
}


/**
* @brief Add folder to audio / video list background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @param in T_pCWSTR path : folder path
* @param in T_BOOL subFolder : search subfolder or not
* @param in T_pFILE FindParent : FindParent
* @return T_eMEDIALIST_ADD_RET
* @retval
*/
static T_eMEDIALIST_ADD_RET MList_BG_AddFolder(T_eMEDIA_LIST_TYPE type, T_pCWSTR path, 
								T_BOOL subFolder, T_pFILE FindParent)
{
	
    T_eMEDIALIST_ADD_RET	ret = eMEDIALIST_ADD_NONE;
    T_eMEDIALIST_ADD_RET	ever_ret = eMEDIALIST_ADD_NONE;
    T_FILE_INFO				FileInfo;
    T_USTR_FILE				TmpPath = {0};
    T_USTR_FILE				FindPath = {0};
    T_U16					diskpath[5] = {0};
	T_U32					FindHandle;
	T_PFILEINFO				info;
	T_pFILE					file;
	T_U16					tmpstr1[10] = {0};
	T_U16					tmpstr2[10] = {0};
	T_MEDIA_LIST			*pList = AK_NULL;

	AK_ASSERT_PTR(path, "MList_BG_AddFolder(): path", ret);
	AK_ASSERT_VAL(*path, "MList_BG_AddFolder(): *path", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_BG_AddFolder(): type err", ret);
	AK_ASSERT_VAL(FindParent != FS_INVALID_HANDLE, "MList_BG_AddFolder(): FindParent err", ret);

#ifdef OS_ANYKA
	if (!Fwl_CheckDriverIsValid(path))
    {
        Fwl_Print(C2, M_MLIST, "MList_BG_AddFolder invalid path!!!\r\n");
        return ret;
    }

    Eng_StrMbcs2Ucs(DRI_B, diskpath);

    if (!Fwl_CheckDriverIsValid(diskpath))
    {
        Fwl_Print(C2, M_MLIST, "MList_BG_AddFolder default path is not mounted!!!\r\n");
        return ret;
    }
#endif

	if (Utl_CaclSolidas(path) > MAX_PATH_DEEP)
    {
    	Fwl_Print(C2, M_MLIST, "MList_BG_AddFolder path is too deep\r\n");
		return eMEDIALIST_ADD_OUTPATHDEEP;
    }

    pList = mList[type].plist;

	AK_ASSERT_PTR(pList, "MList_BG_AddFolder(): pList", ret);

    if (pList->state & MEDIA_LIST_REQ_SUSPEND)
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_AddFolder force to return!!!\r\n");
		return ret;
	}
	
	// find a folder
    if (Utl_UStrLen(path) < (MAX_FILENM_LEN - 5))
    {
        Utl_UStrCpy(FindPath, path);

        if (FindPath[Utl_UStrLen(FindPath) - 1] == UNICODE_SOLIDUS)
        {
            FindPath[Utl_UStrLen(FindPath) - 1] = 0;
        }

		Eng_StrMbcs2Ucs("/*.*", tmpstr1);
        Utl_UStrCat(FindPath, tmpstr1);

		FindHandle =Fwl_FileFindFirstFromHandle((T_U32)FindParent);

        if (0 != FindHandle)
        {			
            do
            {
            #ifdef OS_ANYKA
            	if (eMEDIALIST_ADD_NONE == ret)
            	{
	            	if (!Fwl_CheckDriverIsValid(path))
				    {
				        Fwl_Print(C2, M_MLIST, "MList_BG_AddFolder invalid path!!!\r\n");
				        break;
				    }

				    if (!Fwl_CheckDriverIsValid(diskpath))
				    {
				        Fwl_Print(C2, M_MLIST, "MList_BG_AddFolder default path is not mounted!!!\r\n");
				        break;
				    }
			    }
    		#endif

    			if (pList->state & MEDIA_LIST_REQ_SUSPEND)
				{
					Fwl_Print(C2, M_MLIST, "MList_BG_AddFolder force to break!!!\r\n");
					break;
				}
    		
            	info = (T_PFILEINFO)Fwl_FsFindInfo(&FileInfo,(T_hFILESTAT)FindHandle);
				if (AK_NULL == info)
				{
					break;
				}
         		file = (T_pFILE)Fwl_FileFindOpen((T_U32)FindParent, (T_U32)info);
				
				// Path Is Too Longer
				if (Utl_UStrLen(path) + Utl_UStrLen(FileInfo.name) >= MEDIA_PATH_SIZE / 2)
				{
					continue;
				}

				Utl_UStrCpy(TmpPath, path);					
            	if (TmpPath[Utl_UStrLen(TmpPath) - 1] != UNICODE_SOLIDUS)
            	{
            		memset(tmpstr1, 0, 10*2);
            		Eng_StrMbcs2Ucs("/", tmpstr1);
                	Utl_UStrCat(TmpPath, tmpstr1);
            	}

				Utl_UStrCat(TmpPath, FileInfo.name);
				
				// Is A Directory
                if ((FileInfo.attrib & 0x10) == 0x10)
                {
                	memset(tmpstr1, 0, 10*2);
                	Eng_StrMbcs2Ucs(".", tmpstr1);
                	Eng_StrMbcs2Ucs("..", tmpstr2);
                	
					// folder is "." or "..", continue
                	if (0 == Utl_UStrCmp(FileInfo.name, tmpstr1) 
                	|| 0 == Utl_UStrCmp(FileInfo.name, tmpstr2))
                	{
						continue;                    
                	}
					// Search Sub Directory
					if (subFolder)
                    {					
                        ret = MList_BG_AddFolder(type, TmpPath, subFolder,file);
                    }
                    else
                    {
                        continue;
                    }                    
                }
				// Is A File
                else
                {
                    ret = MList_BG_AddFile(type, TmpPath, file);					
                }

                //if ever once add successfully, then this function should return SUCCESS
                if (eMEDIALIST_ADD_SUCCESS == ret)
                {
                    ever_ret = eMEDIALIST_ADD_SUCCESS;
                }
                else if (eMEDIALIST_ADD_OUTPATHDEEP == ret)   
				{
				    ever_ret = eMEDIALIST_ADD_OUTPATHDEEP;		// xwz
				}
                else if (eMEDIALIST_ADD_NOSPACE == ret)   
				{
					ever_ret = eMEDIALIST_ADD_NOSPACE;
					break;
				}
                Fwl_FileClose(file);
		    }while(Fwl_FsFindNext((T_hFILESTAT)FindHandle) != AK_FALSE);
		    Fwl_FileFindCloseWithHandle(FindHandle);
            ret = ever_ret;
        }
        else
        {
            //使用多任务的文件系统后，不再在FindFirst()和FindNext()里搜索上级目录和当前目录。
            //因此空目录里Find时也是返回 FS_INVALID_STATHANDLE.
            ret = eMEDIALIST_ADD_SUCCESS;
        }
    }

    return ret;
}


/**
* @brief Add file / folder to audio / video list background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @param in T_pCWSTR path : file / folder path
* @param in T_BOOL subFolder : search subfolder or not
* @return T_eMEDIALIST_ADD_RET
* @retval
*/
static T_eMEDIALIST_ADD_RET MList_BG_Add(T_eMEDIA_LIST_TYPE type, T_pCWSTR path, T_BOOL subFolder)
{
	T_pFILE					FindParent;
	T_eMEDIALIST_ADD_RET	ret = eMEDIALIST_ADD_NONE;
	T_USTR_FILE				diskpath = {0};
	

	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_BG_Add(): type err", ret);
	AK_ASSERT_PTR(path, "MList_BG_Add(): path", ret);
	AK_ASSERT_VAL(*path, "MList_BG_Add(): *path", ret);

#ifdef OS_ANYKA
	if (!Fwl_CheckDriverIsValid(path))
    {
        Fwl_Print(C2, M_MLIST, "MList_BG_Add invalid path!!!\r\n");
        return ret;
    }

    Eng_StrMbcs2Ucs(DRI_B, diskpath);

    if (!Fwl_CheckDriverIsValid(diskpath))
    {
        Fwl_Print(C2, M_MLIST, "MList_BG_Add default path is not mounted!!!\r\n");
        return ret;
    }
#endif

	if (Utl_UStrLen(path) > MAX_FILENM_LEN)
    {
        Fwl_Print(C3, M_MLIST, "MList_BG_Add path too long\r\n");
        return ret;
    }

    if (Fwl_FsIsDir(path))
    {
    	// add a folder files to the file list
		FindParent = Fwl_FileOpen_Ex(path, _FMODE_READ, _FMODE_READ);

		if (FS_INVALID_HANDLE != FindParent)
		{
			AK_Feed_Watchdog(0);
	        ret = MList_BG_AddFolder(type, path, subFolder, FindParent);
			AK_Feed_Watchdog(4);
			Fwl_FileClose(FindParent);
		}
    }
    else
    {
        // add one audio file to file list
        FindParent = Fwl_FileOpen(path, _FMODE_READ, _FMODE_READ);

		if (FS_INVALID_HANDLE != FindParent)
		{
	        ret = MList_BG_AddFile(type, path, FindParent);
			Fwl_FileClose(FindParent);
		}
    }

	return ret;
}


/**
* @brief Delete class info by class id background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U16 id : file id, when tag is Album/Composer; album id, when tag is Artist/Genre
* @param in T_U16 classId : class id
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @return T_eID3_WRITE_RET
* @retval
*/
static T_eID3_WRITE_RET MList_BG_ID3_DelByClassId(T_U16 id, T_U16 classId, T_eID3_TAGS tag)
{
	T_U16				i = 0;
	T_U16				j = 0;
	T_U16				pos = 0;
	T_eID3_WRITE_RET	ret = eID3_WRITE_NONE;
	T_MEDIA_LIST		*pAList = mList[eMEDIA_LIST_AUDIO].plist;
	
	AK_ASSERT_PTR(pAList, "MList_BG_ID3_DelByClassId(): pAList", ret);
	AK_ASSERT_PTR(pAList->id3, "MList_BG_ID3_DelByClassId(): pAList->id3", ret);
	AK_ASSERT_VAL((id < MAX_MEDIA_NUM), "MList_BG_ID3_DelByClassId(): id", ret);
	AK_ASSERT_VAL((classId < MAX_MEDIA_NUM), "MList_BG_ID3_DelByClassId(): classId", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_BG_ID3_DelByClassId(): tag", ret);

	for (i=0; i<pAList->id3[tag].index[classId].qty; i++)
	{
		pos = pAList->id3[tag].index[classId].pos + i;
		
		if (pAList->id3[tag].idxmap[pos] == id)
		{
			pAList->id3[tag].index[classId].qty--;

			for (j=classId+1; j<MAX_MEDIA_NUM; j++)
			{
				if (0 != pAList->id3[tag].index[j].pos)
				{
					pAList->id3[tag].index[j].pos--;
				}
			}

			for (j=pos; j<MAX_MEDIA_NUM-1; j++)
			{
				pAList->id3[tag].idxmap[j] = pAList->id3[tag].idxmap[j+1];
			}

			pAList->id3[tag].idxmap[MAX_MEDIA_NUM-1] = 0;

			if (0 != pAList->id3[tag].index[classId].qty)
			{
				ret = eID3_WRITE_HEAD_ONLY;
				return ret;
			}
			else
			{
				MList_SetBitValue(pAList->id3[tag].bitmap, MEDIA_BITMAP_SIZE, classId, 0);
				pAList->id3[tag].ckSum[classId]= 0;
				ret = eID3_WRITE_NAME;
				return ret;
			}
		}
	}

	return ret;
}


/**
* @brief Delete album index for Artist/Genre background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U16 albumId : album id
* @param in T_eID3_TAGS tag : Artist/Genre
* @return T_BOOL
* @retval
*/
static T_BOOL MList_BG_ID3_DelAlbum(T_U16 albumId, T_eID3_TAGS tag)
{
	T_BOOL			ret = AK_FALSE;
	T_U16			i = 0;
	T_USTR_FILE		path = {0};
	T_MEDIA_LIST	*pAList = mList[eMEDIA_LIST_AUDIO].plist;

	AK_ASSERT_PTR(pAList, "MList_BG_ID3_DelAlbum(): pAList", ret);
	AK_ASSERT_PTR(pAList->id3, "MList_BG_ID3_DelAlbum(): pAList->id3", ret);
	AK_ASSERT_VAL((albumId < MAX_MEDIA_NUM), "MList_BG_ID3_DelAlbum(): albumId", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_ALBUM), "MList_BG_ID3_DelAlbum(): tag", ret);

	if (FS_INVALID_HANDLE == pAList->id3[tag].fdBack)
	{
		MList_ID3_GetTagPath(path, tag);
		
		pAList->id3[tag].fdBack = Fwl_FileOpen(path, _FMODE_WRITE, _FMODE_WRITE);
		AK_ASSERT_VAL((FS_INVALID_HANDLE != pAList->id3[tag].fdBack), "MList_BG_ID3_DelAlbum(): pAList->id3[tag].fdBack", ret);
	}

	for (i=0; i<MAX_MEDIA_NUM; i++)
	{
		if (MList_GetBitValue(pAList->id3[tag].bitmap, MEDIA_BITMAP_SIZE, i))
		{
			MList_BG_ID3_DelByClassId(albumId, i, tag);
		}
	}

	ret = AK_TRUE;
	return ret;
}



/**
* @brief Delete class info background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U16 id : file id, when tag is Album/Composer; album id, when tag is Artist/Genre
* @param in T_pCWSTR classname : class name
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @param out T_U16 *albumId : album id,when tag is Album; no use when tag is not album
* @return T_eID3_WRITE_RET
* @retval
*/
static T_eID3_WRITE_RET MList_BG_ID3_DelClassInfo(T_U16 Id, T_pCWSTR classname, T_eID3_TAGS tag, T_U16 *albumId)
{
	T_eID3_WRITE_RET	ret = eID3_WRITE_NONE;
	T_U16				classId = 0;
	T_MEDIA_LIST		*pAList = mList[eMEDIA_LIST_AUDIO].plist;
	
	AK_ASSERT_PTR(pAList, "MList_BG_ID3_DelClassInfo(): pAList", ret);
	AK_ASSERT_PTR(pAList->id3, "MList_BG_ID3_DelClassInfo(): pAList->id3", ret);
	AK_ASSERT_VAL((Id < MAX_MEDIA_NUM), "MList_BG_ID3_DelClassInfo(): Id", ret);
	AK_ASSERT_PTR(classname, "MList_BG_ID3_DelClassInfo(): classname", ret);
	AK_ASSERT_VAL(*classname, "MList_BG_ID3_DelClassInfo(): *classname", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_BG_ID3_DelClassInfo(): tag", ret);

	classId = MList_ID3_GetClassId(classname, tag, AK_TRUE);

	if (classId < MAX_MEDIA_NUM)
	{
		if (eID3_TAGS_ALBUM == tag && AK_NULL != albumId)
		{
			*albumId = classId;
		}

		ret = MList_BG_ID3_DelByClassId(Id, classId, tag);
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_ID3_DelClassInfo can't find");
	}

	return ret;
}

/**
* @brief Check the album has item for the classname (Artist/Genre) or not
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U16 albumId : album id
* @param in T_pCWSTR classname : class name
* @param in T_eID3_TAGS tag : Artist/Genre
* @return T_BOOL
* @retval
*/
static T_BOOL MList_BG_ID3_HasItem(T_U16 albumId, T_pCWSTR classname, T_eID3_TAGS tag)
{
	T_BOOL				ret = AK_FALSE;
	T_USTR_FILE			listpath = {0};
	T_U16				i = 0;
	T_U16				pos = 0;
	T_U16				fileId = 0;
	T_U16				size = 0;
	T_USTR_FILE 		namestr = {0};
	T_MEDIA_LIST		*pAList = mList[eMEDIA_LIST_AUDIO].plist;

	
	AK_ASSERT_PTR(pAList, "MList_BG_ID3_HasItem(): pAList", ret);
	AK_ASSERT_PTR(pAList->id3, "MList_BG_ID3_HasItem(): pAList->id3", ret);
	AK_ASSERT_VAL((albumId < MAX_MEDIA_NUM), "MList_BG_ID3_HasItem(): albumId", ret);
	AK_ASSERT_PTR(classname, "MList_BG_ID3_HasItem(): classname", ret);
	AK_ASSERT_VAL(*classname, "MList_BG_ID3_HasItem(): *classname", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_ALBUM), "MList_BG_ID3_HasItem(): tag", ret);

	if (FS_INVALID_HANDLE == pAList->fdBack)
	{
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);
		pAList->fdBack = Fwl_FileOpen(listpath, _FMODE_WRITE, _FMODE_WRITE);
		AK_ASSERT_VAL((FS_INVALID_HANDLE != pAList->fdBack), "MList_BG_ID3_HasItem(): pAList->fdBack", ret);
	}

	pos = pAList->id3[eID3_TAGS_ALBUM].index[albumId].pos;
	
	for (i=pos; i<pos+pAList->id3[eID3_TAGS_ALBUM].index[albumId].qty; i++)
	{
		fileId = pAList->id3[eID3_TAGS_ALBUM].idxmap[i];

		if (MList_GetBitValue(pAList->bitmap, MEDIA_BITMAP_SIZE, fileId))
		{
			size = MEDIA_FILE_HEAD_SIZE	+ MEDIA_FILEITEM_SIZE * fileId 
				+ MEDIA_PATH_SIZE + tag * MEDIA_CLASSNAME_SIZE;
			Fwl_FileSeek(pAList->fdBack, size, _FSEEK_SET);

			Fwl_FileRead(pAList->fdBack, namestr, MEDIA_CLASSNAME_SIZE);

			if (0 == Utl_UStrCmp(namestr, classname))
			{		
				ret = AK_TRUE;
				break;
			}
		}
	}

	return ret;
}


/**
* @brief Remove a Media (Audio / Video) From Media List background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @param in T_pCWSTR path : a Media (Audio / Video)
* @return T_BOOL
* @retval
*/
static T_BOOL MList_BG_Del(T_eMEDIA_LIST_TYPE type, T_pCWSTR path)
{
	T_BOOL				ret = AK_FALSE;
	T_USTR_FILE			listpath = {0};
	T_USTR_FILE			classname[eID3_TAGS_NUM] = {0};
	T_MEDIA_LIST 		*pList = AK_NULL;
	T_U16				id = 0;
	T_U16				albumId = 0;
	T_eID3_WRITE_RET	writeRet = eID3_WRITE_NONE;
	T_U32				seeksize = 0;
	
	AK_ASSERT_PTR(path, "MList_BG_Del(): path", ret);
	AK_ASSERT_VAL(*path, "MList_BG_Del(): *path", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_BG_Del(): type err", ret);

	pList = mList[type].plist;

	if (eMEDIA_LIST_AUDIO == type)
	{
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);
	}
	else if (eMEDIA_LIST_VIDEO == type)
	{
		Eng_StrMbcs2Ucs((VIDEOLIST_DEF_FILE), listpath);
	}
	else
	{
		
	}

	AK_ASSERT_PTR(pList, "MList_BG_Del(): pList", ret);
		
	if (FS_INVALID_HANDLE == pList->fdBack)
	{
		pList->fdBack = Fwl_FileOpen(listpath, _FMODE_WRITE, _FMODE_WRITE);
		AK_ASSERT_VAL((FS_INVALID_HANDLE != pList->fdBack), "MList_BG_Del(): pList->fdBack", ret);
	}

	id = MList_GetFileId(path, type, AK_TRUE);

	if (id < MAX_MEDIA_NUM)
	{
		MList_SetBitValue(pList->bitmap, MEDIA_BITMAP_SIZE, id, 0);
		pList->ckSum[id] = 0;
		pList->playInfo[id].Cnt = 0;
		pList->playInfo[id].Time = 0;

		if (eMEDIA_LIST_AUDIO == type)
		{
			seeksize = MEDIA_FILE_HEAD_SIZE + MEDIA_FILEITEM_SIZE * id + MEDIA_PATH_SIZE;
			Fwl_FileSeek(pList->fdBack, seeksize, _FSEEK_SET);
			Fwl_FileRead(pList->fdBack, classname[eID3_TAGS_GENRE], MEDIA_CLASSNAME_SIZE);
			Fwl_FileRead(pList->fdBack, classname[eID3_TAGS_ARTIST], MEDIA_CLASSNAME_SIZE);
			Fwl_FileRead(pList->fdBack, classname[eID3_TAGS_ALBUM], MEDIA_CLASSNAME_SIZE);
			Fwl_FileRead(pList->fdBack, classname[eID3_TAGS_COMPOSER], MEDIA_CLASSNAME_SIZE);

			writeRet = MList_BG_ID3_DelClassInfo(id, classname[eID3_TAGS_ALBUM], eID3_TAGS_ALBUM, &albumId);
			MList_BG_ID3_DelClassInfo(id, classname[eID3_TAGS_COMPOSER], eID3_TAGS_COMPOSER, AK_NULL);

			if (eID3_WRITE_NAME == writeRet)
	    	{
				MList_BG_ID3_DelClassInfo(albumId, classname[eID3_TAGS_ARTIST], eID3_TAGS_ARTIST, AK_NULL);
				MList_BG_ID3_DelClassInfo(albumId, classname[eID3_TAGS_GENRE], eID3_TAGS_GENRE, AK_NULL);
	    	}
	    	else if (eID3_WRITE_HEAD_ONLY == writeRet)
	    	{
				if (!MList_BG_ID3_HasItem(albumId, classname[eID3_TAGS_ARTIST], eID3_TAGS_ARTIST))
				{
					MList_BG_ID3_DelClassInfo(albumId, classname[eID3_TAGS_ARTIST], eID3_TAGS_ARTIST, AK_NULL);
				}

				if (!MList_BG_ID3_HasItem(albumId, classname[eID3_TAGS_GENRE], eID3_TAGS_GENRE))
				{
					MList_BG_ID3_DelClassInfo(albumId, classname[eID3_TAGS_GENRE], eID3_TAGS_GENRE, AK_NULL);
				}
	    	}

	    	if (eID3_WRITE_NAME == writeRet
	    	||eID3_WRITE_HEAD_ONLY == writeRet)
	    	{
				ret = AK_TRUE;
	    	}
		}
	}
	else
	{
		Fwl_Print(C2, M_MLIST, "MList_BG_Del can't find the name\r\n");
	}
		
	return ret;
}


/**
* @brief Get the index file path by tag
*
* @author Songmengxing
* @date 2011-12-26
* @param out T_pWSTR path : index file path
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @return T_BOOL
* @retval
*/
static T_BOOL MList_ID3_GetTagPath(T_pWSTR path, T_eID3_TAGS tag)
{
	T_BOOL	ret = AK_FALSE;
	
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_ID3_GetTagPath(): tag", ret);
	AK_ASSERT_PTR(path, "MList_ID3_GetTagPath(): path", ret);

	switch(tag)
	{
	case eID3_TAGS_GENRE:
		Eng_StrMbcs2Ucs((GENRE_INDEX_FILE), path);
		break;
	case eID3_TAGS_ARTIST:
		Eng_StrMbcs2Ucs((ARTIST_INDEX_FILE), path);
		break;
	case eID3_TAGS_ALBUM:
		Eng_StrMbcs2Ucs((ALBUM_INDEX_FILE), path);
		break;
	case eID3_TAGS_COMPOSER:
		Eng_StrMbcs2Ucs((COMPOSER_INDEX_FILE), path);
		break;
	default:
		return AK_FALSE;
		break;
	}

	return AK_TRUE;
}


/**
* @brief Get file id by file name
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR filename : file name
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @param in T_BOOL bBg : the function is called background or not
* @return T_U16 class id
* @retval MAX_MEDIA_NUM can't find
*/
static T_U16 MList_GetFileId(T_pCWSTR filename, T_eMEDIA_LIST_TYPE type, T_BOOL bBg)
{
	T_U16			ret = MAX_MEDIA_NUM;
	T_USTR_FILE		listpath = {0};
	T_U32			size = 0;
	T_U16			ckSum = 0;
	T_U16			i = 0;
	T_USTR_FILE		strName = {0};
	T_hFILE			file = FS_INVALID_HANDLE;
	T_FILE_MODE 	mode = _FMODE_READ;
	T_MEDIA_LIST	*pList = AK_NULL;

	AK_ASSERT_VAL((type < eMEDIA_LIST_NUM), "MList_GetFileId(): type", ret);
	AK_ASSERT_PTR(filename, "MList_GetFileId(): filename", ret);
	AK_ASSERT_VAL(*filename, "MList_GetFileId(): *filename", ret);

	pList = mList[type].plist;

	if (eMEDIA_LIST_AUDIO == type)
	{
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);
	}
	else if (eMEDIA_LIST_VIDEO == type)
	{
		Eng_StrMbcs2Ucs((VIDEOLIST_DEF_FILE), listpath);
	}
	else
	{
		
	}

	AK_ASSERT_PTR(pList, "MList_GetFileId(): pList", ret);

	if (bBg)
	{
		file = pList->fdBack;
		mode = _FMODE_WRITE;
	}
	else
	{
		file = pList->fdFore;
		mode = _FMODE_READ;
	}
	
	if (FS_INVALID_HANDLE == file)
	{		
		file = Fwl_FileOpen(listpath, mode, mode);
		AK_ASSERT_VAL((FS_INVALID_HANDLE != file), "MList_GetFileId(): file", ret);
	}

	
	ckSum = MList_CalcChecksum(filename);
	
	for (i=0; i<MAX_MEDIA_NUM; i++)
	{
		if ((pList->ckSum[i] == ckSum)
		&& MList_GetBitValue(pList->bitmap, MEDIA_BITMAP_SIZE, i))
		{
			size = MEDIA_FILE_HEAD_SIZE + MEDIA_FILEITEM_SIZE * i;
			Fwl_FileSeek(file, size, _FSEEK_SET);

			Fwl_FileRead(file, strName, MEDIA_PATH_SIZE);

			if (0 == Utl_UStrCmp(strName, filename))
			{
				ret = i;
				break;
			}
		}
	}

	if (bBg)
	{
		pList->fdBack = file;
	}
	else
	{
		pList->fdFore = file;
	}

	return ret;
}


/**
* @brief Get class id by class name
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR classname : class name
* @param in T_eID3_TAGS tag : Album/Composer/Artist/Genre
* @param in T_BOOL bBg : the function is called background or not
* @return T_U16 class id
* @retval MAX_MEDIA_NUM can't find
*/
static T_U16 MList_ID3_GetClassId(T_pCWSTR classname, T_eID3_TAGS tag, T_BOOL bBg)
{
	T_U16			ret = MAX_MEDIA_NUM;
	T_USTR_FILE		path = {0};
	T_U32			size = 0;
	T_U8			ckSum = 0;
	T_U16			i = 0;
	T_USTR_FILE		strName = {0};
	T_hFILE			file = FS_INVALID_HANDLE;
	T_FILE_MODE 	mode = _FMODE_READ;
	T_MEDIA_LIST	*pAList = mList[eMEDIA_LIST_AUDIO].plist;

	AK_ASSERT_PTR(pAList, "MList_ID3_GetClassId(): pAList", ret);
	AK_ASSERT_PTR(pAList->id3, "MList_ID3_GetClassId(): pAList->id3", ret);	
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "MList_ID3_GetClassId(): tag", ret);
	AK_ASSERT_PTR(classname, "MList_ID3_GetClassId(): classname", ret);
	AK_ASSERT_VAL(*classname, "MList_ID3_GetClassId(): *classname", ret);

	if (bBg)
	{
		file = pAList->id3[tag].fdBack;
		mode = _FMODE_WRITE;
	}
	else
	{
		file = pAList->id3[tag].fdFore;
		mode = _FMODE_READ;
	}
	
	if (FS_INVALID_HANDLE == file)
	{
		MList_ID3_GetTagPath(path, tag);
		
		file = Fwl_FileOpen(path, mode, mode);
		AK_ASSERT_VAL((FS_INVALID_HANDLE != file), "MList_ID3_GetClassId(): file", ret);
	}

	
	ckSum = (T_U8)MList_CalcChecksum(classname);
	
	for (i=0; i<MAX_MEDIA_NUM; i++)
	{
		if ((pAList->id3[tag].ckSum[i] == ckSum)
		&& MList_GetBitValue(pAList->id3[tag].bitmap, MEDIA_BITMAP_SIZE, i))
		{
			size = MEDIA_ID3_HEAD_SIZE + i * MEDIA_CLASSNAME_SIZE;
			Fwl_FileSeek(file, size, _FSEEK_SET);

			Fwl_FileRead(file, strName, MEDIA_CLASSNAME_SIZE);

			if (0 == Utl_UStrCmp(strName, classname))
			{
				ret = i;
				break;
			}
		}
	}

	if (bBg)
	{
		pAList->id3[tag].fdBack = file;
	}
	else
	{
		pAList->id3[tag].fdFore = file;
	}

	return ret;
}



/**
* @brief Calc qty by bitmap
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U8* pbitmap : bitmap
* @param in T_U32 bitmapSize : size of bitmap
* @return T_U32 qty
* @retval 
*/
static T_U32 MList_CalcQty(T_U8* pbitmap, T_U32 bitmapSize)
{
	T_U32	pos = 0;
	T_U8	bitOffset = 0;
	T_U32	sum = 0;
	T_U32	i = 0;
	T_U32	bitnum = bitmapSize*8;
	
	AK_ASSERT_PTR(pbitmap, "MList_CalcQty(): pbitmap", sum);

	for (i=0; i<bitnum; i++)
	{
		pos = i >> 3;
		bitOffset = (T_U8)(i & 0x7);//i % 8

		sum += (pbitmap[pos] & (1 << bitOffset)) >> bitOffset;
	}

	return sum;
}

/**
* @brief Find the first hole id by bitmap
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U8* pbitmap : bitmap
* @param in T_U32 bitmapSize : size of bitmap
* @return T_U16 id
* @retval 
*/
static T_U16 MList_FindFirstHoleId(T_U8* pbitmap, T_U32 bitmapSize)
{
	T_U32	pos = 0;
	T_U8	bitOffset = 0;
	T_U16	id = MAX_MEDIA_NUM;
	T_U16	i = 0;
	T_U32	bitnum = bitmapSize*8;
	
	AK_ASSERT_PTR(pbitmap, "MList_FindFirstHoleId(): pbitmap", id);

	for (i=0; i<bitnum; i++)
	{
		pos = i >> 3;
		bitOffset = (T_U8)(i & 0x7);

		if (0 == (pbitmap[pos] & (1 << bitOffset)))
		{
			id = i;
			break;
		}
	}

	return id;
}

/**
* @brief Set bit value
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U8* pbitmap : bitmap
* @param in T_U32 bitmapSize : size of bitmap
* @param in T_U32 id : id
* @param in T_BOOL val : 0 or 1
* @return T_BOOL
* @retval 
*/
static T_BOOL MList_SetBitValue(T_U8* pbitmap, T_U32 bitmapSize, T_U32 id, T_BOOL val)
{
	T_U32	pos = 0;
	T_U8	bitOffset = 0;
	T_U32	bitnum = bitmapSize*8;
	T_BOOL	ret = AK_FALSE;
	T_U8	mask = 0;
	T_U8	rmask = 0;
	
	AK_ASSERT_PTR(pbitmap, "MList_SetBitValue(): pbitmap", ret);
	AK_ASSERT_VAL(id < bitnum, "MList_SetBitValue(): id err", ret);

	pos = id >> 3;
	bitOffset = (T_U8)(id & 0x7);

	mask = 1<<bitOffset;
    rmask = ~mask;
    pbitmap[pos] = (pbitmap[pos]&rmask)|(val<<bitOffset);

	ret = AK_TRUE;

	return ret;
}


/**
* @brief Get bit value
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_U8* pbitmap : bitmap
* @param in T_U32 bitmapSize : size of bitmap
* @param in T_U32 id : id
* @return T_BOOL bit value
* @retval 
*/
static T_BOOL MList_GetBitValue(T_U8* pbitmap, T_U32 bitmapSize, T_U32 id)
{
	T_U32	pos = 0;
	T_U8	bitOffset = 0;
	T_U32	bitnum = bitmapSize*8;
	T_BOOL	ret = 0;
	
	AK_ASSERT_PTR(pbitmap, "MList_GetBitValue(): pbitmap", ret);
	AK_ASSERT_VAL(id < bitnum, "MList_GetBitValue(): id err", ret);

	pos = id >> 3;
	bitOffset = (T_U8)(id & 0x7);

	ret = (pbitmap[pos] & (1 << bitOffset)) >> bitOffset;

	return ret;
}

/**
* @brief Calc check sum by path
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_pCWSTR path : path
* @return T_U16 check sum
* @retval
*/
static T_U16 MList_CalcChecksum(T_pCWSTR path)
{
	T_U16 ret = 0;
	T_U32 i = 0;
	T_U32 sum = 0;

	AK_ASSERT_PTR(path, "MList_CalcChecksum(): path", ret);
	AK_ASSERT_VAL(*path, "MList_CalcChecksum(): *path", ret);

	for (i=0; i<Utl_UStrLen(path); i++)
	{
		sum += path[i];
	}

	ret = (T_U16)sum;

	return ret;
}


/**
* @brief Open Media List (Audio / Video) Service for Read Media List (ID3 Information)
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_S32 count
* @retval -1 open fail; >=0 open success;
*/
static T_S32 MList_Init(T_eMEDIA_LIST_TYPE type)
{
	T_S32			count = -1;
	T_U8			i = 0;
	T_USTR_FILE		listpath = {0};
	T_USTR_FILE		id3path[eID3_TAGS_NUM] = {0};
	T_MEDIA_LIST 	*pList = AK_NULL;
	T_U16			maxQty = 0;
	T_U16			defMaxQty = MAX_MEDIA_NUM;
	T_BOOL			bFileOk = AK_FALSE;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_Init(): type err", count);

	if (eMEDIA_LIST_AUDIO == type)
	{
		Eng_StrMbcs2Ucs((AUDIOLIST_DEF_FILE), listpath);

		Eng_StrMbcs2Ucs((GENRE_INDEX_FILE), id3path[eID3_TAGS_GENRE]);
		Eng_StrMbcs2Ucs((ARTIST_INDEX_FILE), id3path[eID3_TAGS_ARTIST]);
		Eng_StrMbcs2Ucs((ALBUM_INDEX_FILE), id3path[eID3_TAGS_ALBUM]);
		Eng_StrMbcs2Ucs((COMPOSER_INDEX_FILE), id3path[eID3_TAGS_COMPOSER]);
	}
	else if (eMEDIA_LIST_VIDEO == type)
	{
		Eng_StrMbcs2Ucs((VIDEOLIST_DEF_FILE), listpath);
	}
	else
	{
		
	}


	pList = (T_MEDIA_LIST *)Fwl_Malloc(sizeof(T_MEDIA_LIST));
	AK_ASSERT_PTR(pList, "MList_Init(): pList malloc fail", count);
	memset(pList, 0, sizeof(T_MEDIA_LIST));

	pList->fdBack = FS_INVALID_HANDLE;
	pList->fdFore = FS_INVALID_HANDLE;

	pList->fdBack = Fwl_FileOpen(listpath, _FMODE_READ, _FMODE_READ);
	
	if (FS_INVALID_HANDLE != pList->fdBack)
	{
		Fwl_FileRead(pList->fdBack, &maxQty, MEDIA_NUM_SIZE);

		if (defMaxQty == maxQty)
		{
			Fwl_FileRead(pList->fdBack, pList->bitmap, MEDIA_BITMAP_SIZE);
			Fwl_FileRead(pList->fdBack, pList->ckSum, MEDIA_PATH_CKSUM_SIZE);
			Fwl_FileRead(pList->fdBack, pList->playInfo, MEDIA_PLAY_INFO_SIZE);
			bFileOk = AK_TRUE;
		}
		else
		{
			Fwl_Print(C2, M_MLIST, "MList_Init type %d file version err", type);
			Fwl_FileClose(pList->fdBack);
			Fwl_FileDelete(listpath);

			if (eMEDIA_LIST_AUDIO == type)
			{
				for (i=0; i<eID3_TAGS_NUM; i++)
				{
					Fwl_FileDelete(id3path[i]);
				}
			}
		}
	}
	
	if (!bFileOk)
	{
		Fwl_CreateDefPath();
		
		pList->fdBack = Fwl_FileOpen(listpath, _FMODE_CREATE, _FMODE_CREATE);

		if (FS_INVALID_HANDLE == pList->fdBack)
		{
			Fwl_Print(C2, M_MLIST, "MList_Init type %d pList->fdBack open fail", type);
			mList[type].plist = pList;
			MList_Destroy(type);
			return count;
		}

		Fwl_FileWrite(pList->fdBack, &defMaxQty, MEDIA_NUM_SIZE);
		Fwl_FileWrite(pList->fdBack, pList->bitmap, MEDIA_BITMAP_SIZE);
		Fwl_FileWrite(pList->fdBack, pList->ckSum, MEDIA_PATH_CKSUM_SIZE);
		Fwl_FileWrite(pList->fdBack, pList->playInfo, MEDIA_PLAY_INFO_SIZE);
	}

	Fwl_FileClose(pList->fdBack);
	pList->fdBack = FS_INVALID_HANDLE;

	if (eMEDIA_LIST_AUDIO == type)
	{
		pList->id3 = (T_ID3_GROUP *)Fwl_Malloc(eID3_TAGS_NUM*sizeof(T_ID3_GROUP));

		if (AK_NULL == pList->id3)
		{
			Fwl_Print(C2, M_MLIST, "MList_Init pAList->id3 malloc fail");
			mList[type].plist = pList;
			MList_Destroy(type);
			return count;
		}
		
		memset(pList->id3, 0, eID3_TAGS_NUM*sizeof(T_ID3_GROUP));
		
		for (i=0; i<eID3_TAGS_NUM; i++)
		{
			pList->id3[i].fdBack = FS_INVALID_HANDLE;
			pList->id3[i].fdFore = FS_INVALID_HANDLE;

			pList->id3[i].fdBack = Fwl_FileOpen(id3path[i], _FMODE_READ, _FMODE_READ);
		
			if (FS_INVALID_HANDLE != pList->id3[i].fdBack)
			{
				Fwl_FileRead(pList->id3[i].fdBack, pList->id3[i].bitmap, MEDIA_BITMAP_SIZE);
				Fwl_FileRead(pList->id3[i].fdBack, pList->id3[i].idxmap, MEDIA_MAP_SIZE);
				Fwl_FileRead(pList->id3[i].fdBack, pList->id3[i].index, MEDIA_INDEX_SIZE);
				Fwl_FileRead(pList->id3[i].fdBack, pList->id3[i].ckSum, MEDIA_CLASS_CKSUM_SIZE);
			}
			else
			{
				pList->id3[i].fdBack = Fwl_FileOpen(id3path[i], _FMODE_CREATE, _FMODE_CREATE);

				if (FS_INVALID_HANDLE == pList->id3[i].fdBack)
				{
					Fwl_Print(C2, M_MLIST, "MList_Init pAList->id3[%d].fdBack open fail", i);
					mList[type].plist = pList;
					MList_Destroy(type);
					return count;
				}

				Fwl_FileWrite(pList->id3[i].fdBack, pList->id3[i].bitmap, MEDIA_BITMAP_SIZE);
				Fwl_FileWrite(pList->id3[i].fdBack, pList->id3[i].idxmap, MEDIA_MAP_SIZE);
				Fwl_FileWrite(pList->id3[i].fdBack, pList->id3[i].index, MEDIA_INDEX_SIZE);
				Fwl_FileWrite(pList->id3[i].fdBack, pList->id3[i].ckSum, MEDIA_CLASS_CKSUM_SIZE);
			}

			Fwl_FileClose(pList->id3[i].fdBack);
			pList->id3[i].fdBack = FS_INVALID_HANDLE;

		}
	}
	count = MList_CalcQty(pList->bitmap, MEDIA_BITMAP_SIZE);	

	pList->state |= MEDIA_LIST_STATE_OPENED;

	mList[type].plist = pList;

	return count;
}


/**
* @brief Close Media List (Audio / Video) Service
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @return T_BOOL
* @retval 
*/
static T_BOOL MList_Destroy(T_eMEDIA_LIST_TYPE type)
{
	T_BOOL			ret = AK_FALSE;
	T_U8			i = 0;
	T_MEDIA_LIST	*pList = AK_NULL;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_Destroy(): type err", ret);

	pList = mList[type].plist;

	if (AK_NULL == pList)
	{
		return ret;
	}

	if (pList->state & MEDIA_LIST_STATE_RUNNING)
	{
		Fwl_Print(C2, M_MLIST, "type %d ,it is running, can't close it!",type);
		return ret;
	}

	if (AK_NULL != pList->id3)
	{
		for (i=0; i<eID3_TAGS_NUM; i++)
		{
			if (FS_INVALID_HANDLE != pList->id3[i].fdBack)
			{
				Fwl_FileSeek(pList->id3[i].fdBack, 0, _FSEEK_SET);
				Fwl_FileWrite(pList->id3[i].fdBack, pList->id3[i].bitmap, MEDIA_BITMAP_SIZE);
				Fwl_FileWrite(pList->id3[i].fdBack, pList->id3[i].idxmap, MEDIA_MAP_SIZE);
				Fwl_FileWrite(pList->id3[i].fdBack, pList->id3[i].index, MEDIA_INDEX_SIZE);
				Fwl_FileWrite(pList->id3[i].fdBack, pList->id3[i].ckSum, MEDIA_CLASS_CKSUM_SIZE);
				Fwl_FileClose(pList->id3[i].fdBack);
			}

			if (FS_INVALID_HANDLE != pList->id3[i].fdFore)
			{
				Fwl_FileClose(pList->id3[i].fdFore);
			}
		}

		pList->id3 = Fwl_Free(pList->id3);
	}

	if (FS_INVALID_HANDLE != pList->fdBack)
	{
		Fwl_FileSeek(pList->fdBack, MEDIA_NUM_SIZE, _FSEEK_SET);
		Fwl_FileWrite(pList->fdBack, pList->bitmap, MEDIA_BITMAP_SIZE);
		Fwl_FileWrite(pList->fdBack, pList->ckSum, MEDIA_PATH_CKSUM_SIZE);
		Fwl_FileWrite(pList->fdBack, pList->playInfo, MEDIA_PLAY_INFO_SIZE);
		Fwl_FileClose(pList->fdBack);
	}

	if (FS_INVALID_HANDLE != pList->fdFore)
	{
		Fwl_FileClose(pList->fdFore);
	}
	
	pList = Fwl_Free(pList);

	mList[type].plist = pList;

	Fwl_Print(C3, M_MLIST, "type %d close OK!", type);

	return AK_TRUE;	
}

/**
* @brief post event to background
*
* @author Songmengxing
* @date 2011-12-26
* @param in T_eMEDIA_LIST_TYPE type : eMEDIA_LIST_AUDIO / eMEDIA_LIST_VIDEO
* @param in T_SYS_MAILBOX *pMailbox : pMailbox
* @return T_BOOL
* @retval 
*/
static T_BOOL MList_PostEvent(T_eMEDIA_LIST_TYPE type, T_SYS_MAILBOX *pMailbox)
{
	T_BOOL	ret = AK_FALSE;
	
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_PostEvent(): type err", ret);
	AK_ASSERT_PTR(pMailbox, "MList_PostEvent(): pMailbox", ret);

	switch (type)
	{
	case eMEDIA_LIST_AUDIO:
		//获取线程句柄,若需要创建，则创建线程
		if (0 == IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_AUDIOLIST)))
		{
			CAudioListBGApp_New(AK_NULL);
		}

		IAppMgr_PostEvent(AK_GetAppMgr(), AKAPP_CLSID_AUDIOLIST, pMailbox);	
		ret = AK_TRUE;
		break;

	case eMEDIA_LIST_VIDEO:
		if (0 == IThread_GetTask(IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_VIDEOLIST)))
		{
			CVideoListBGApp_New(AK_NULL);
		}
		
		IAppMgr_PostEvent(AK_GetAppMgr(), AKAPP_CLSID_VIDEOLIST, pMailbox);
		ret = AK_TRUE;
		break;

	default:
		break;
	}

	return ret;
}



/**
* @brief Obtain Semaphore
*
* @author Songmengxing
* @date 2011-12-26
* @param T_VOID
* @return T_S32
* @retval
*/
static T_S32 MList_Obtain_Semaphore(T_eMEDIA_LIST_TYPE type)
{
#ifdef OS_ANYKA
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_Obtain_Semaphore(): type err", -1);
	
	if (0 == mList[type].mutex)
	{
		mList[type].mutex = AK_Create_Semaphore(1, AK_PRIORITY);
	}

	return AK_Obtain_Semaphore(mList[type].mutex, AK_SUSPEND);
#else
	if (0 == mlist_mutex)
	{
		mlist_mutex = AK_Create_Semaphore(1, AK_PRIORITY);
	}

	return AK_Obtain_Semaphore(mlist_mutex, AK_SUSPEND);
#endif
}

/**
* @brief Release Semaphore
*
* @author Songmengxing
* @date 2011-12-26
* @param T_VOID
* @return T_S32
* @retval
*/
static T_S32 MList_Release_Semaphore(T_eMEDIA_LIST_TYPE type)
{
#ifdef OS_ANYKA
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "MList_Release_Semaphore(): type err", -1);
	
	if (0 == mList[type].mutex)
	{
		return -1;
	}
	
	return AK_Release_Semaphore(mList[type].mutex);
#else
	if (0 == mlist_mutex)
	{
		return -1;
	}
	
	return AK_Release_Semaphore(mlist_mutex);
#endif
}


