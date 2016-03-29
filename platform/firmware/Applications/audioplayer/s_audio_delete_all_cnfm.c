
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Image.h"
#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfAudio.h"
#include "Ctl_FileList.h"
#include "Fwl_osFS.h"
#include "Eng_String_UC.h"
#include "Eng_DataConvert.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Ctl_APlayerList.h"
#include "svc_medialist.h"




typedef struct {
    T_ICONEXPLORER      *pIconExplorer;
    T_CLASSINFO 		*pClassInfo;
    T_LIST_TYPE         ListType;
    T_MSGBOX            msgbox;
} T_AUDIO_DELETE_ALL_PARM;

static T_AUDIO_DELETE_ALL_PARM *pAudio_Delete_All_Parm;
#endif

/*---------------------- BEGIN OF STATE s_audio_delete_all_cnfm ------------------------*/
void initaudio_delete_all_cnfm(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_Delete_All_Parm = (T_AUDIO_DELETE_ALL_PARM *)Fwl_Malloc(sizeof(T_AUDIO_DELETE_ALL_PARM));
    AK_ASSERT_PTR_VOID(pAudio_Delete_All_Parm, "initaudio_delete_all_cnfm(): malloc error");
	
	memset(pAudio_Delete_All_Parm, 0x0, sizeof(T_AUDIO_DELETE_ALL_PARM));		// xwz
    MsgBox_InitAfx(&pAudio_Delete_All_Parm->msgbox, 0, ctHINT, csMP3_DELETE_ALL_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
#endif
}

void exitaudio_delete_all_cnfm(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_Delete_All_Parm = Fwl_Free(pAudio_Delete_All_Parm);
#endif
}

void paintaudio_delete_all_cnfm(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    MsgBox_Show(&pAudio_Delete_All_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_delete_all_cnfm(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   menuRet;
    T_USTR_FILE		pFilePath = {0};
	T_INDEX_CONTENT *pcontent = AK_NULL;
    T_hFILESTAT     find;
    T_FILE_INFO     FileInfo;
    T_USTR_FILE     name, ext;
    T_USTR_FILE     FindPath, FilePath;
    T_BOOL          ret;
	T_S16			failStrId = csFAILURE_DONE;

    ret = AK_TRUE;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pAudio_Delete_All_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_6 == event)
    {
		pAudio_Delete_All_Parm->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
		pAudio_Delete_All_Parm->ListType = (T_LIST_TYPE)(T_U32)pEventParm->p.pParam3;

		if (pAudio_Delete_All_Parm->ListType>=LTP_GENRE 
			&& pAudio_Delete_All_Parm->ListType<=LTP_COMPOSER)
		{
			pAudio_Delete_All_Parm->pClassInfo = (T_CLASSINFO *)pEventParm->p.pParam2;
		}
		else
		{
			
		}
    }
    
    menuRet = MsgBox_Handler(&pAudio_Delete_All_Parm->msgbox, event, pEventParm);
    switch(menuRet)
    {
        case eNext:
            gb.listchanged = AK_TRUE;
            pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(pAudio_Delete_All_Parm->pIconExplorer);

            if (AK_NULL != pcontent)
			{
				MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
			}
            
            Utl_USplitFileName(pFilePath, name, ext);
            Utl_UStrLower(ext);

            //ÊÕ²Ø¼ÐÎÄ¼þ
            if (0 == Utl_UStrCmp(ext, _T("alt")))
            {
                Eng_StrMbcs2Ucs("*.alt", ext);
                Utl_UStrCpy(FindPath, (T_pCWSTR)Fwl_GetDefPath(eAUDIOLIST_PATH));
                Utl_UStrCat(FindPath, ext);
               
                find = Fwl_FsFindFirst(FindPath);
                if (find != -1)
                {
                    do
                    {
                        Fwl_FsFindInfo(&FileInfo, find);
                        if ((Utl_UStrCmp(FileInfo.name, _T(".")) == 0) \
                            || (Utl_UStrCmp(FileInfo.name, _T("..")) == 0))
                        {
                            continue;
                        }
                        Utl_UStrCpy(FilePath, (T_pCWSTR)Fwl_GetDefPath(eAUDIOLIST_PATH));
                        Utl_UStrCat(FilePath, FileInfo.name);
                        if (AK_TRUE != Fwl_FileDelete((T_pWSTR)FilePath))
                        {
                            ret  = AK_FALSE;
                            break;
                        }

                        if (AK_FALSE == IconExplorer_DelItemFocus(pAudio_Delete_All_Parm->pIconExplorer))
                        {
                            ret  = AK_FALSE;
                            break;
                        }
                    } while (Fwl_FsFindNext(find));
					Fwl_FsFindClose(find);
					find =FS_INVALID_STATHANDLE;
                }
            }
            else //¸èÇú
            {
				if ((LTP_CURPLY != pAudio_Delete_All_Parm->ListType) \
                    && (LTP_FAVORITE != pAudio_Delete_All_Parm->ListType))
 
			    {
			    	if (MList_IsAdding(eMEDIA_LIST_AUDIO))
					{
						ret = AK_FALSE;
						failStrId = csBUSY_CANNOT_DEL;
					}
					else
					{
						if (LTP_TITLE == pAudio_Delete_All_Parm->ListType)
						{
							ret = MList_RemoveAll(eMEDIA_LIST_AUDIO);
						}
						else if (LTP_RECEPLAY == pAudio_Delete_All_Parm->ListType
							|| LTP_RECEAPPEND == pAudio_Delete_All_Parm->ListType
							|| LTP_OFTENPLAY == pAudio_Delete_All_Parm->ListType)
						{
							T_U16 i = 0;

							for (i=0; i<MAX_MEDIA_NUM; i++)
							{
								MList_CleanPlayInfo(i, eMEDIA_LIST_AUDIO, pAudio_Delete_All_Parm->ListType - LTP_OFTENPLAY);
							}
						}
						else if (pAudio_Delete_All_Parm->ListType>=LTP_GENRE 
								&& pAudio_Delete_All_Parm->ListType<=LTP_COMPOSER)
						{
							switch (pAudio_Delete_All_Parm->ListType)
							{
							case LTP_GENRE:
								ret = MList_ID3_RemoveClass(pAudio_Delete_All_Parm->pClassInfo->pGenre, eID3_TAGS_GENRE);
								break;
							case LTP_ARTIST:
								ret = MList_ID3_RemoveClass(pAudio_Delete_All_Parm->pClassInfo->pArtist, eID3_TAGS_ARTIST);
								break;
							case LTP_ALBUM:
								ret = MList_ID3_RemoveClass(pAudio_Delete_All_Parm->pClassInfo->pAlbum, eID3_TAGS_ALBUM);
								break;
							case LTP_COMPOSER:
								ret = MList_ID3_RemoveClass(pAudio_Delete_All_Parm->pClassInfo->pComposer, eID3_TAGS_COMPOSER);
								break;
							default:
								break;
							}
							
						}
					}
                }

				if (AK_FALSE != ret)
				{
					if (AK_FALSE == IconExplorer_DelAllItem(pAudio_Delete_All_Parm->pIconExplorer))
	                {
	                    ret  = AK_FALSE;
						Fwl_Print(C3, M_FWL, "Delete Play List Failure!\n");
	                }
				}
            }

            if (AK_TRUE != ret)
            {
                MsgBox_InitAfx(&pAudio_Delete_All_Parm->msgbox, 3, ctFAILURE, failStrId, MSGBOX_INFORMATION);
            }
            else
            {
                MsgBox_InitAfx(&pAudio_Delete_All_Parm->msgbox, 3, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
            }
            MsgBox_SetDelay(&pAudio_Delete_All_Parm->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudio_Delete_All_Parm->msgbox);
        
            break;
        default:
            ReturnDefauleProc(menuRet, pEventParm);
            break;
    }

#endif
    return 0;
}


