
#include "Anyka_types.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_FileList.h"
#include "Gbl_Global.h"
#include "Eng_DataConvert.h"
#include "Eng_String_UC.h"
#include "Eng_String.h"
#include "Gbl_MacroDef.h"
#include "Eng_ImgConvert.h"
#include "Eng_ImgDec.h"
#include "Fwl_Public.h"
#include "Fwl_pfAudio.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_APlayerList.h"
#include "svc_medialist.h"



/*****************************************************************************/

T_BOOL AudioPlayer_IsSupportFile(T_pCWSTR pFilePath)
{
    T_MEDIALIB_CHECK_OUTPUT ckOut;	
      
	// Media_QueryInfo(&media_info, pFilePath, AK_TRUE);
	
    switch (Fwl_GetMediatypeByFiletype(Utl_GetFileType(pFilePath)))
    {
    case _SD_MEDIA_TYPE_AAC:
/*
		Media_QueryInfo(&media_info, pFilePath, AK_TRUE);
		if (MEDIALIB_MEDIA_UNKNOWN == media_info.m_MediaType)
	    {
	    	AK_DEBUG_OUTPUT("*.AAC: Unknown Media Type.\n");
	        return AK_FALSE;
	    }
		
		if (!media_info.m_bHasAudio)
	    {
	    	AK_DEBUG_OUTPUT("*.AAC: NOT Has Audio.\n");
	        return AK_FALSE;
	    }
*/
		break;
		
	case _SD_MEDIA_TYPE_RA8LBR:
		Media_CheckFile(&ckOut, (T_pVOID)pFilePath, AK_TRUE);
		if (!ckOut.m_bHasAudio)
	    {
	        return AK_FALSE;
	    }
		break;

	case _SD_MEDIA_TYPE_UNKNOWN:
		return AK_FALSE;
		break;
		
	default:
		break;
    }

    return AK_TRUE;
}


T_VOID AudioPlayer_SaveCurrentPlayList(T_ICONEXPLORER *pIconExplorer)
{
    T_FILELIST FileList;

    if(gb.bInExplorer == AK_FALSE)
    {
        FileList_Init(&FileList, AUDIOPLAYER_MAX_ITEM_QTY, FILELIST_SORT_NONE,  AudioPlayer_IsSupportFile);
        FileList_FromIconExplorer(&FileList, pIconExplorer);
        FileList_SaveFileList(&FileList, _T(AUDIOLIST_CURTPLY_FILE));
        FileList_Free(&FileList);
        //FreqMgr_StateCheckOut(FREQ_FACTOR_DEFAULT);
    }	
}


T_BOOL AudioPlayer_IsSupportListFile(T_pCWSTR pFilePath)
{
    T_USTR_FILE name, ext;
//    T_USTR_FILE filepath;
    T_USTR_FILE ustr_1;

    Eng_StrMbcs2Ucs("alt", ustr_1);
    //Utl_StrCpy((T_pSTR)filepath, pFilePath);

    //Utl_USplitFileName(filepath, name, ext);
    Utl_USplitFileName(pFilePath, name, ext);
    Utl_UStrLower(ext);

    if (0 != Utl_UStrCmp(ext, ustr_1))
    {
        return AK_FALSE;
    }
    
    return AK_TRUE;
}


T_BOOL AudioPlayer_Add(T_pCWSTR pFilePath, T_BOOL SearchSub)
{
    return MList_AddItem(pFilePath, SearchSub, AK_TRUE, eMEDIA_LIST_AUDIO);
}


/**
 * @brief   Inspects pNode according to the play time or play counter info whether to meet the condition
 * 
 * @author  wangwei
 * @date    2008-03-25
 * @param   [T_pINFO_SET] the subset index
 * @param   [T_LIST_CLASS_INFO] classified infomation
 * @param   [T_INFO_NODE] song info node in song info list
 * @return  T_VOID
 * @retval  
 */
/*
static T_VOID AudioPlayer_CheckByTimeOrCounter(T_pINFO_SET pIndex, T_LIST_CLASS_INFO *pClassInfo, T_INFO_NODE *pNode)
{
	
    switch (pClassInfo->attach)
    {
        case METAINFO_ATTACH_OFTEN_PLAY:
        	if (pNode->Node.PlayCounter >= ATTACH_PLAY_COUNT)
        	{
        		pIndex->Index[pIndex->NodeNum++] = pNode;
        	}
        	break;
        case METAINFO_ATTACH_RECENTLY_PLAY:
        	break;
        case METAINFO_ATTACH_RECENTLY_APPEND:
        	break;
        default:
        	break;
    }
            
}
*/







T_pDATA AudioPlayer_GetAudioImage(T_pCWSTR pFilePath, T_LEN width, T_LEN height)
{
	T_pDATA		dis_buf = AK_NULL;

	T_pDATA		pInImgBuf  = AK_NULL;
	T_pDATA		pOutImgBuf = AK_NULL;
	T_U32		InImgSize;
	T_pVOID		ImgHandler = AK_NULL;
	T_U32 		num, size;

	if ((width == 0) || (height == 0))
    {
		return AK_NULL;
	}

#ifdef LCD_MODE_565
	size = 2;
#else
	size = 3;
#endif

	ImgHandler = Media_GetPicMetaInfo(pFilePath, &pInImgBuf, &InImgSize);

	if ((ImgHandler != AK_NULL) && (InImgSize > MIN_PIC_SIZE))
	{
		pOutImgBuf = ImgDec_GetImageDataFromBuf(pInImgBuf, InImgSize);
		
		if (pOutImgBuf != AK_NULL)
		{
			dis_buf = (T_U8 *)Fwl_Malloc(width*height*size+16);

			if (dis_buf == AK_NULL)
			{
			 	AK_DEBUG_OUTPUT("failed to malloc dis_buf\n");
				Media_ReleasePicMetaInfo(ImgHandler);
	            return AK_NULL;
			}

			num = FillAkBmpHead(dis_buf, width, height);
			GetBMP2RGBBuf(pOutImgBuf, dis_buf+num, width, height, 0, 0, 100, 0, AK_NULL, g_Graph.TransColor );
			
			ImgDec_FreeImageData(pOutImgBuf);
		}

		Media_ReleasePicMetaInfo(ImgHandler);
	}
	else
	{
		T_U32 		imgLen;

		if (ImgHandler != AK_NULL)
		{
			Media_ReleasePicMetaInfo(ImgHandler);
		}
		
		AK_DEBUG_OUTPUT("Adplyr_showPic():	Get Audio Image NULL.\n");
		/**Set default background picture*/
        dis_buf = Res_StaticLoad(AK_NULL, eRES_BMP_DEF_MUSIC, &imgLen);      
	}

	return dis_buf;
}



T_U32 AudioPlayer_GetTotalTimeAtStopState(T_VOID)
{
    T_U32 		TotalTime = 0;
    T_USTR_FILE	pFilePath = {0};

	AudioPlayer_GetFocusFilePath(pFilePath);
    if (0 != pFilePath[0]
    	&& Fwl_AudioGetTotalTimeFromFile(pFilePath, &TotalTime))
    {
		return TotalTime;
    }
    
    return 0;
}

T_VOID APList_Update(T_VOID)
{
	MList_AddItem(Fwl_GetDefPath(eAUDIO_PATH), AK_TRUE, AK_FALSE, eMEDIA_LIST_AUDIO);
}


