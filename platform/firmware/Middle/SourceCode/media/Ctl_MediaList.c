/**
 * @file ctl_medialist.c
 * @brief ANYKA software
 * 
 * @author songmengxing
 * @date  
 * @version 1,0 
 */


#include "fwl_public.h"
#include "eng_time.h"
#include "Ctl_medialist.h"
#include "eng_string_uc.h"
#include "hal_timer.h"


#define	MLIST_ADD_TIME		(600)
#define	MLIST_ADD_CNT_MASK	(0xF)

/**
* @brief add items to IconExplorer in step when media list is adding
*
* @author Songmengxing
* @date 2012-5-8
* @param in T_U32 count : add count
* @param in T_U32 tickStart : start time
* @return T_BOOL
* @retval 
*/
static T_BOOL Ctl_MList_Add_Enough(T_U32 count, T_U32 tickStart)
{
	if (0 == (count & MLIST_ADD_CNT_MASK))
	{
		if (get_tick_count() - tickStart >= MLIST_ADD_TIME)
		{
			return AK_TRUE;
		}
	}

	return AK_FALSE;
}

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
T_BOOL Ctl_MList_ToIconExplorerStep(T_ICONEXPLORER *pIconExplorer, T_U16* StartId, T_eMEDIA_LIST_TYPE type)
{
	T_BOOL			ret = AK_FALSE;
	T_S32			cnt = 0;
	T_U16			i = 0;
	T_USTR_FILE		path = {0};
	T_USTR_FILE		tmppath = {0};
	T_USTR_FILE		tmpname = {0};
	T_INDEX_CONTENT content;
	T_U32			tickStart = 0;
	T_U32			tmpCnt = 0;

	AK_ASSERT_PTR(pIconExplorer, "Ctl_MList_ToIconExplorerStep(): pIconExplorer", ret);
	AK_ASSERT_PTR(StartId, "Ctl_MList_ToIconExplorerStep(): StartId", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "Ctl_MList_ToIconExplorerStep(): type err", ret);

	tickStart = get_tick_count();
	
	cnt = MList_GetItemQty(type);

	if (cnt < 0 || *StartId >= cnt)
	{
		return ret;
	}

	for (i=*StartId; i<cnt; i++)
	{
		memset(path, 0, sizeof(T_USTR_FILE));
		MList_GetItem(path, i, type);
		
		if (0 == path[0])
		{
			break;
		}

		Utl_USplitFilePath(path, tmppath, tmpname);
		content.type = type;
		content.id = i;
		IconExplorer_AddItem(pIconExplorer, i + 1, &content, sizeof(T_INDEX_CONTENT), tmpname, AK_NULL, AK_NULL);
		ret = AK_TRUE;

		tmpCnt++;

		if (Ctl_MList_Add_Enough(tmpCnt, tickStart))
		{				
			break;
		}
	}

	*StartId = i;

	return ret;
}


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
T_BOOL Ctl_MList_ToIconExplorerComplete(T_ICONEXPLORER *pIconExplorer, T_U16 *StartId, T_S32* firstHoleId, T_eMEDIA_LIST_TYPE type)
{
	T_BOOL		ret = AK_FALSE;
	T_S32		cnt = 0;
	T_U16		addcnt = 0;
	T_U16		i = 0;
	T_USTR_FILE	path = {0};
	T_USTR_FILE	tmppath = {0};
	T_USTR_FILE	tmpname = {0};
	T_U16		emptyCnt = 0;
	T_INDEX_CONTENT content;
	T_U32			tickStart = 0;
	T_U32			tmpCnt = 0;

	AK_ASSERT_PTR(pIconExplorer, "Ctl_MList_ToIconExplorerComplete(): pIconExplorer", ret);
	AK_ASSERT_PTR(StartId, "Ctl_MList_ToIconExplorerComplete(): StartId", ret);
	AK_ASSERT_PTR(firstHoleId, "Ctl_MList_ToIconExplorerComplete(): firstHoleId", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "Ctl_MList_ToIconExplorerComplete(): type err", ret);

	tickStart = get_tick_count();
	
	cnt = MList_GetItemQty(type);

	if (cnt < 0 || *StartId >= cnt)
	{
		return ret;
	}

	addcnt = *StartId;

	for (i=*StartId; (i<MAX_MEDIA_NUM)&&(addcnt<cnt); i++)
	{
		memset(path, 0, sizeof(T_USTR_FILE));
		MList_GetItem(path, i, type);
		
		if (0 == path[0])
		{
			if (0 == emptyCnt)
			{
				*firstHoleId = i;
				*StartId = i;
			}
			emptyCnt++;
			continue;
		}

		Utl_USplitFilePath(path, tmppath, tmpname);
		content.type = type;
		content.id = i;
		IconExplorer_AddItem(pIconExplorer, addcnt + 1, &content, sizeof(T_INDEX_CONTENT), tmpname, AK_NULL, AK_NULL);
		addcnt++;
		ret = AK_TRUE;

		tmpCnt++;

		if (Ctl_MList_Add_Enough(tmpCnt, tickStart))
		{				
			if (addcnt<cnt)
			{
				MList_SetAddFlag(type, eADD_FLAG_STEP);
			}

			*StartId = i;
			return ret;
		}
	}

	if (0 == emptyCnt)
	{
		*StartId = i;
	}

	return ret;
}


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
T_BOOL Ctl_MList_ID3_ToIconExplorerStep(T_ICONEXPLORER *pIconExplorer, T_U16* StartId, T_eID3_TAGS tag)
{
	T_BOOL		ret = AK_FALSE;
	T_U16		cnt = 0;
	T_U16		i = 0;
	T_U16		classname[MEDIA_CLASSNAME_SIZE/2 + 1] = {0};
	T_INDEX_CONTENT content;
	T_U32			tickStart = 0;
	T_U32			tmpCnt = 0;

	AK_ASSERT_PTR(pIconExplorer, "Ctl_MList_ID3_ToIconExplorerStep(): pIconExplorer", ret);
	AK_ASSERT_PTR(StartId, "Ctl_MList_ID3_ToIconExplorerStep(): StartId", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "Ctl_MList_ID3_ToIconExplorerStep(): tag", ret);

	tickStart = get_tick_count();
	
	cnt = MList_ID3_GetClassQty(tag);

	if (*StartId >= cnt)
	{
		return ret;
	}

	for (i=*StartId; i<cnt; i++)
	{
		memset(classname, 0, MEDIA_CLASSNAME_SIZE);
		MList_ID3_GetClassName(classname, i, tag);
		
		if (0 == classname[0])
		{
			break;
		}

		content.type = tag + eINDEX_TYPE_GENRE;
		content.id = i;
		IconExplorer_AddItem(pIconExplorer, i, &content, sizeof(T_INDEX_CONTENT), classname, AK_NULL, AK_NULL);
		ret = AK_TRUE;

		tmpCnt++;

		if (Ctl_MList_Add_Enough(tmpCnt, tickStart))
		{				
			break;
		}
	}

	*StartId = i;

	return ret;
}

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
T_BOOL Ctl_MList_ID3_ToIconExplorerComplete(T_ICONEXPLORER *pIconExplorer, T_U16 *StartId, T_S32* firstHoleId, T_eID3_TAGS tag)
{
	T_BOOL		ret = AK_FALSE;
	T_U16		cnt = 0;
	T_U16		addcnt = 0;
	T_U16		i = 0;
	T_U16		classname[MEDIA_CLASSNAME_SIZE/2 + 1] = {0};
	T_U16		emptyCnt = 0;
	T_INDEX_CONTENT content;
	T_U32			tickStart = 0;
	T_U32			tmpCnt = 0;
	
	AK_ASSERT_PTR(pIconExplorer, "Ctl_MList_ID3_ToIconExplorerComplete(): pIconExplorer", ret);
	AK_ASSERT_PTR(StartId, "Ctl_MList_ID3_ToIconExplorerComplete(): StartId", ret);
	AK_ASSERT_PTR(firstHoleId, "Ctl_MList_ID3_ToIconExplorerComplete(): firstHoleId", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "Ctl_MList_ID3_ToIconExplorerComplete(): tag", ret);

	tickStart = get_tick_count();
	
	cnt = MList_ID3_GetClassQty(tag);

	if (*StartId >= cnt)
	{
		return ret;
	}

	addcnt = *StartId;

	for (i=*StartId; (i<MAX_MEDIA_NUM)&&(addcnt<cnt); i++)
	{
		memset(classname, 0, MEDIA_CLASSNAME_SIZE);
		MList_ID3_GetClassName(classname, i, tag);
		
		if (0 == classname[0])
		{
			if (0 == emptyCnt)
			{
				*firstHoleId = i;
				*StartId = i;
			}
			emptyCnt++;
			continue;
		}

		content.type = tag + eINDEX_TYPE_GENRE;
		content.id = i;
		IconExplorer_AddItem(pIconExplorer, addcnt, &content, sizeof(T_INDEX_CONTENT), classname, AK_NULL, AK_NULL);
		addcnt++;
		ret = AK_TRUE;

		tmpCnt++;

		if (Ctl_MList_Add_Enough(tmpCnt, tickStart))
		{				
			if (addcnt<cnt)
			{
				MList_SetAddFlag(eMEDIA_LIST_AUDIO, eADD_FLAG_STEP);
			}
			
			*StartId = i;
			return ret;
		}
	}

	if (0 == emptyCnt)
	{
		*StartId = i;
	}

	return ret;
}

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
T_BOOL Ctl_MList_ID3_SongToIconExplorer(T_ICONEXPLORER *pIconExplorer, T_U16* StartId, T_pCWSTR className, T_eID3_TAGS tag)
{
	T_BOOL		ret = AK_FALSE;
	T_U16		cnt = 0;
	T_U16		albumCnt = 0;
	T_U16		i = 0;
	T_U16		j = 0;
	T_USTR_FILE	path = {0};
	T_USTR_FILE	tmppath = {0};
	T_USTR_FILE	tmpname = {0};
	T_USTR_FILE	albumname = {0};
	T_INDEX_CONTENT content;
	T_U32			tickStart = 0;
	T_U32			tmpCnt = 0;


	AK_ASSERT_PTR(pIconExplorer, "Ctl_MList_ID3_SongToIconExplorer(): pIconExplorer", ret);
	AK_ASSERT_PTR(StartId, "Ctl_MList_ID3_SongToIconExplorer(): StartId", ret);
	AK_ASSERT_PTR(className, "Ctl_MList_ID3_SongToIconExplorer(): className", ret);
	AK_ASSERT_VAL(*className, "Ctl_MList_ID3_SongToIconExplorer(): *className", ret);
	AK_ASSERT_VAL((tag < eID3_TAGS_NUM), "Ctl_MList_ID3_SongToIconExplorer(): tag", ret);

	tickStart = get_tick_count();
	
	if (eID3_TAGS_ALBUM == tag || eID3_TAGS_COMPOSER == tag)
	{
		cnt = MList_ID3_GetSongQty(className, tag);

		if (*StartId >= cnt)
		{
			return ret;
		}

		for (i=*StartId; i<cnt; i++)
		{
			memset(path, 0, sizeof(T_USTR_FILE));
			
			MList_ID3_GetSongPath(path, className, 0, i, tag);
			
			if (0 == path[0])
			{
				break;
			}

			Utl_USplitFilePath(path, tmppath, tmpname);
			content.type = eINDEX_TYPE_AUDIO;
			content.id = MList_GetPathId(path, eINDEX_TYPE_AUDIO);
			IconExplorer_AddItem(pIconExplorer, i + 1, &content, sizeof(T_INDEX_CONTENT), tmpname, AK_NULL, AK_NULL);
			ret = AK_TRUE;

			tmpCnt++;

			if (Ctl_MList_Add_Enough(tmpCnt, tickStart))
			{				
				MList_SetAddFlag(eMEDIA_LIST_AUDIO, eADD_FLAG_STEP);
				break;
			}
		}

		*StartId = i;
	}
	else if (tag < eID3_TAGS_ALBUM)
	{
		albumCnt = MList_ID3_GetAlbumQty(className, tag);

		for (i=0; i<albumCnt; i++)
		{
			memset(albumname, 0, sizeof(T_USTR_FILE));
			MList_ID3_GetAlbumName(albumname, className, i, tag);

			if (0 != albumname[0])
			{
				cnt = MList_ID3_GetSongQty(albumname, eID3_TAGS_ALBUM);
				
				for (j=StartId[i]; j<cnt; j++)
				{
					memset(path, 0, sizeof(T_USTR_FILE));
					
					MList_ID3_GetSongPath(path, className, i, j, tag);
					
					if (0 == path[0])
					{
						continue;
					}

					Utl_USplitFilePath(path, tmppath, tmpname);
					content.type = eINDEX_TYPE_AUDIO;
					content.id = MList_GetPathId(path, eINDEX_TYPE_AUDIO);
					IconExplorer_AddItem(pIconExplorer, content.id + 1, &content, sizeof(T_INDEX_CONTENT), tmpname, AK_NULL, AK_NULL);
					ret = AK_TRUE;

					tmpCnt++;

					if (Ctl_MList_Add_Enough(tmpCnt, tickStart))
					{				
						MList_SetAddFlag(eMEDIA_LIST_AUDIO, eADD_FLAG_STEP);
						StartId[i] = cnt;
						return ret;
					}
				}

				StartId[i] = cnt;
			}
		}
	}

	return ret;
}



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
T_BOOL Ctl_MList_ToIEByAppendTimeStep(T_ICONEXPLORER *pIconExplorer, T_U16* StartId, T_eMEDIA_LIST_TYPE type)
{
	T_BOOL			ret = AK_FALSE;
	T_S32			cnt = 0;
	T_U32			appendTime = 0;
	T_U16			i = 0;
	T_USTR_FILE		path = {0};
	T_USTR_FILE		tmppath = {0};
	T_USTR_FILE		tmpname = {0};
	T_INDEX_CONTENT content;
	T_U32			tickStart = 0;
	T_U32			tmpCnt = 0;

	AK_ASSERT_PTR(pIconExplorer, "Ctl_MList_ToIEByPlayInfoStep(): pIconExplorer", ret);
	AK_ASSERT_PTR(StartId, "Ctl_MList_ToIEByPlayInfoStep(): StartId", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "Ctl_MList_ToIEByPlayInfoStep(): type err", ret);

	tickStart = get_tick_count();
	
	cnt = MList_GetItemQty(type);

	if (cnt < 0 || *StartId >= cnt)
	{
		return ret;
	}

	for (i=*StartId; i<cnt; i++)
	{
		appendTime = MList_GetPlayInfo(i, type, eMEDIA_APPENDTIME);

		if (0 == appendTime)
		{
			break;
		}
		else if (0 == MList_GetPlayInfo(i, type, eMEDIA_PLAYCNT)
			&& appendTime > ATTACH_ERROR_TIME
			&& GetSysTimeSeconds() - appendTime < ATTACH_APPEND_TIME)
		{
			memset(path, 0, sizeof(T_USTR_FILE));
			MList_GetItem(path, i, type);
			
			if (0 != path[0])
			{
				Utl_USplitFilePath(path, tmppath, tmpname);
				content.type = type;
				content.id = i;
				IconExplorer_AddItem(pIconExplorer, i + 1, &content, sizeof(T_INDEX_CONTENT), tmpname, AK_NULL, AK_NULL);
				ret = AK_TRUE;

				tmpCnt++;

				if (Ctl_MList_Add_Enough(tmpCnt, tickStart))
				{				
					break;
				}
			}
		}
	}

	*StartId = i;

	return ret;
}


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
T_BOOL Ctl_MList_ToIEByPlayInfoComplete(T_ICONEXPLORER *pIconExplorer, T_U16 *StartId, T_S32* firstHoleId, T_eMEDIA_PLAYINFO_TYPE infoType, T_eMEDIA_LIST_TYPE type)
{
	T_BOOL		ret = AK_FALSE;
	T_S32		cnt = 0;
	T_U16		addcnt = 0;
	T_U32		appendTime = 0;
	T_U16		i = 0;
	T_USTR_FILE	path = {0};
	T_USTR_FILE	tmppath = {0};
	T_USTR_FILE	tmpname = {0};
	T_U16		emptyCnt = 0;
	T_INDEX_CONTENT content;
	T_BOOL		bNeedAdd = AK_FALSE;
	T_U32		tickStart = 0;
	T_U32		tmpCnt = 0;

	AK_ASSERT_PTR(pIconExplorer, "Ctl_MList_ToIEByPlayInfoComplete(): pIconExplorer", ret);
	AK_ASSERT_PTR(StartId, "Ctl_MList_ToIEByPlayInfoComplete(): StartId", ret);
	AK_ASSERT_PTR(firstHoleId, "Ctl_MList_ToIEByPlayInfoComplete(): firstHoleId", ret);
	AK_ASSERT_VAL(type < eMEDIA_LIST_NUM, "Ctl_MList_ToIEByPlayInfoComplete(): type err", ret);
	AK_ASSERT_VAL(infoType < eMEDIA_PLAYINFO_NUM, "Ctl_MList_ToIEByPlayInfoComplete(): infoType err", ret);

	tickStart = get_tick_count();
	
	cnt = MList_GetItemQty(type);

	if (cnt < 0 || *StartId >= cnt)
	{
		return ret;
	}

	addcnt = *StartId;

	for (i=*StartId; (i<MAX_MEDIA_NUM)&&(addcnt<cnt); i++)
	{
		bNeedAdd = AK_FALSE;
		
		switch (infoType)
		{
		case eMEDIA_PLAYCNT:
			if (MList_GetPlayInfo(i, type, eMEDIA_PLAYCNT) >= ATTACH_PLAY_COUNT)
			{
				bNeedAdd = AK_TRUE;
			}
			break;
		case eMEDIA_PLAYTIME:
			appendTime = MList_GetPlayInfo(i, type, eMEDIA_APPENDTIME);
			if (MList_GetPlayInfo(i, type, eMEDIA_PLAYCNT) > 0
				&& appendTime > ATTACH_ERROR_TIME
				&& GetSysTimeSeconds() - appendTime < ATTACH_APPEND_TIME)
			{
				bNeedAdd = AK_TRUE;
			}
			break;
		case eMEDIA_APPENDTIME:
			appendTime = MList_GetPlayInfo(i, type, eMEDIA_APPENDTIME);
			if (0 == appendTime)
			{
				if (0 == emptyCnt)
				{
					*firstHoleId = i;
					*StartId = i;
				}
				emptyCnt++;
			}
			else if (0 == MList_GetPlayInfo(i, type, eMEDIA_PLAYCNT)
				&& appendTime > ATTACH_ERROR_TIME
				&& GetSysTimeSeconds() - appendTime < ATTACH_APPEND_TIME)
			{
				bNeedAdd = AK_TRUE;
			}
			break;
		default:
			break;
		}

		if (bNeedAdd)
		{
			memset(path, 0, sizeof(T_USTR_FILE));
			MList_GetItem(path, i, type);
			
			if (0 != path[0])
			{
				Utl_USplitFilePath(path, tmppath, tmpname);
				content.type = type;
				content.id = i;
				IconExplorer_AddItem(pIconExplorer, addcnt + 1, &content, sizeof(T_INDEX_CONTENT), tmpname, AK_NULL, AK_NULL);
				addcnt++;
				ret = AK_TRUE;

				tmpCnt++;

				if (Ctl_MList_Add_Enough(tmpCnt, tickStart))
				{				
					if (addcnt<cnt)
					{
						MList_SetAddFlag(type, eADD_FLAG_STEP);
					}
					
					*StartId = i;
					return ret;
				}
			}
		}
	}

	if (0 == emptyCnt)
	{
		*StartId = i;
	}

	return ret;
}

