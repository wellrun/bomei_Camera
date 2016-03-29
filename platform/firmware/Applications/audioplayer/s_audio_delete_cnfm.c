
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Ctl_FileList.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Ctl_APlayerList.h"
#include "svc_medialist.h"


typedef struct {
    T_ICONEXPLORER      *pIconExplorer;
    T_LIST_TYPE         ListType;
    T_MSGBOX            msgbox;
} T_AUDIO_DELETE_PARM;

static T_AUDIO_DELETE_PARM *pAudio_Delete_Parm;
#endif

/*---------------------- BEGIN OF STATE s_audio_delete_cnfm ------------------------*/
void initaudio_delete_cnfm(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_Delete_Parm = (T_AUDIO_DELETE_PARM *)Fwl_Malloc(sizeof(T_AUDIO_DELETE_PARM));
    AK_ASSERT_PTR_VOID(pAudio_Delete_Parm, "initaudio_delete_cnfm(): malloc error");

	memset(pAudio_Delete_Parm, 0x0, sizeof(T_AUDIO_DELETE_PARM));
    MsgBox_InitAfx(&pAudio_Delete_Parm->msgbox, 0, ctHINT, csMP3_DELETE_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
#endif
}

void exitaudio_delete_cnfm(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    pAudio_Delete_Parm = Fwl_Free(pAudio_Delete_Parm);
#endif
}

void paintaudio_delete_cnfm(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    MsgBox_Show(&pAudio_Delete_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_delete_cnfm(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   menuRet;
    T_USTR_FILE		pFilePath = {0};
    T_INDEX_CONTENT *pcontent = AK_NULL;
	T_U16			mediaId = 0;
    T_USTR_FILE     name, ext, ustr_1;
    T_BOOL          ret;
    T_S32           focusid;
	T_S16			failStrId = csFAILURE_DONE;

    ret = AK_TRUE;
    
    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pAudio_Delete_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_5 == event)
    {
        pAudio_Delete_Parm->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
		pAudio_Delete_Parm->ListType = (T_LIST_TYPE)(T_U32)pEventParm->p.pParam3;
    }

    menuRet = MsgBox_Handler(&pAudio_Delete_Parm->msgbox, event, pEventParm);
    switch (menuRet)
    {
        case eNext:
        	pcontent = (T_INDEX_CONTENT *)IconExplorer_GetItemContentFocus(pAudio_Delete_Parm->pIconExplorer);

            if (AK_NULL != pcontent)
			{
				mediaId = pcontent->id;
				MList_GetItem(pFilePath, pcontent->id, eMEDIA_LIST_AUDIO);
			}
			focusid = IconExplorer_GetItemFocusId(pAudio_Delete_Parm->pIconExplorer);
            Utl_USplitFileName(pFilePath, name, ext);
            Utl_UStrLower(ext);

            Eng_StrMbcs2Ucs("alt", ustr_1);
            if (0 == Utl_UStrCmp(ext, ustr_1))
            {
                if (AK_TRUE != Fwl_FileDelete(pFilePath))
                {
                    ret  = AK_FALSE;
                }
            }
            else
			{
				if ((LTP_CURPLY != pAudio_Delete_Parm->ListType) \
                    && (LTP_FAVORITE != pAudio_Delete_Parm->ListType))
				{
					if (MList_IsAdding(eMEDIA_LIST_AUDIO))
					{
						ret = AK_FALSE;
						failStrId = csBUSY_CANNOT_DEL;
					}
					else
					{
						if (LTP_TITLE == pAudio_Delete_Parm->ListType)
						{
							ret = MList_RemoveMediaItem(pFilePath, AK_FALSE, eMEDIA_LIST_AUDIO);
						}
						else if (LTP_RECEPLAY == pAudio_Delete_Parm->ListType
							|| LTP_RECEAPPEND == pAudio_Delete_Parm->ListType
							|| LTP_OFTENPLAY == pAudio_Delete_Parm->ListType)
						{
							ret = MList_CleanPlayInfo(mediaId, eMEDIA_LIST_AUDIO, pAudio_Delete_Parm->ListType - LTP_OFTENPLAY);
						}
						else if (pAudio_Delete_Parm->ListType>=LTP_GENRE 
								&& pAudio_Delete_Parm->ListType<=LTP_COMPOSER)
						{
							ret = MList_ID3_RemoveItem(pFilePath, pAudio_Delete_Parm->ListType - LTP_GENRE);
						}
					}
				}

				if (AK_FALSE != ret)
				{
					if (AK_FALSE == IconExplorer_DelItemFocus(pAudio_Delete_Parm->pIconExplorer))
	                {
	                    ret  = AK_FALSE;
	                }
				}
            }
            
            if (AK_TRUE != ret)
            {
                MsgBox_InitAfx(&pAudio_Delete_Parm->msgbox, 3, ctFAILURE, failStrId, MSGBOX_INFORMATION);
            }
            else
            {
                gb.listchanged = AK_TRUE;
                MsgBox_InitAfx(&pAudio_Delete_Parm->msgbox, 3, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
            }
            MsgBox_SetDelay(&pAudio_Delete_Parm->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudio_Delete_Parm->msgbox);

            break;
        default:
            ReturnDefauleProc(menuRet, pEventParm);
            break;
    }
#endif
    return 0;
}



