
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Ctl_AudioPlayer.h"
#include "Fwl_oscom.h"
#include "Ctl_MsgBox.h"
#include "Fwl_osFS.h"
#include "Fwl_Initialize.h"
#include "Eng_Math.h"
#include "Eng_FileManage.h"
#include "Eng_DataConvert.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


// don't change this name, some code is dependent on it
#define AUDIO_CUSTOM_LIST_NAME  "MyList_%d.ALT"    

typedef struct {
    T_ICONEXPLORER      IconExplorer;
    T_ICONEXPLORER      *pIconExplorer;
    T_VOID          	*pIndex;
    T_LIST_TYPE         ListType;
    T_MSGBOX            msgbox;
} T_AUDIO_LIST_MENU_PARM;

static T_AUDIO_LIST_MENU_PARM *pAudioListMenu = AK_NULL;

static T_VOID Audio_CreateNewList(T_VOID);
#endif


/*---------------------- BEGIN OF STATE s_audio_list_menu ------------------------*/
void initaudio_list_menu(void)
{

#ifdef SUPPORT_AUDIOPLAYER

    pAudioListMenu = (T_AUDIO_LIST_MENU_PARM *)Fwl_Malloc(sizeof(T_AUDIO_LIST_MENU_PARM));
    AK_ASSERT_PTR_VOID(pAudioListMenu, "initaudio_list_menu(): pAudioListMenu malloc error");

    pAudioListMenu->pIconExplorer = AK_NULL;
    MenuStructInit(&pAudioListMenu->IconExplorer);
    GetMenuStructContent(&pAudioListMenu->IconExplorer, mnAUDIOPLYR_LIST_MENU);

    if (TopBar_GetMenuButtonState() == AK_TRUE)
    {
        TopBar_DisableMenuButton();
    }
#endif
}

void exitaudio_list_menu(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Free(&pAudioListMenu->IconExplorer);
    pAudioListMenu = Fwl_Free(pAudioListMenu);
#endif
}

void paintaudio_list_menu(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Show(&pAudioListMenu->IconExplorer);

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_list_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   IconExplorerRet;
    T_U32           itemqty;
    T_VOID 			*pcontent = AK_NULL;
	T_USTR_FILE		pFilePath = {0};
	
    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudioListMenu->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_MENU)
    {
        pAudioListMenu->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
        pAudioListMenu->pIndex = pEventParm->p.pParam2;
        pAudioListMenu->ListType = (T_LIST_TYPE)(T_U32)pEventParm->p.pParam3;

        pcontent = IconExplorer_GetItemContentFocus(pAudioListMenu->pIconExplorer);
        itemqty = IconExplorer_GetItemQty(pAudioListMenu->pIconExplorer);

        if (LTP_FAVORITELIST == pAudioListMenu->ListType)
        {
            IconExplorer_SetTitleText(&pAudioListMenu->IconExplorer, Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_MYLIST_MENU), ICONEXPLORER_TITLE_TEXTCOLOR); 
            IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 3);
            IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 4);
            IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 5);
            IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 6);

            if (pcontent == AK_NULL)
            {
                IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 1);
                IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 2);
            }
            else if (1 == itemqty)
            {
                IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 2);
            }
        }
        else
        {
            IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 0);
            IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 1);
            IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 2);

            if (AK_NULL != pcontent)
			{
				MList_GetItem(pFilePath, ((T_INDEX_CONTENT *)pcontent)->id, eMEDIA_LIST_AUDIO);
			}

            if (0 == pFilePath[0])
            {
                IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 3);
                IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 4);
                IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 5);
                IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 6);
            }
            else 
            {
                if (1 == itemqty)
                {
                    IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 4);
                    IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 6);
                }
                if (LTP_FAVORITE == pAudioListMenu->ListType)
                {
                    IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 3);
                    IconExplorer_DelItem(&pAudioListMenu->IconExplorer, 4);
                }
            }
        }
    }
        
    IconExplorerRet = IconExplorer_Handler(&pAudioListMenu->IconExplorer, event, pEventParm);
    switch (IconExplorerRet)
    {
        case eNext:
            pEventParm->p.pParam1 = pAudioListMenu->pIconExplorer;
            pEventParm->p.pParam2 = pAudioListMenu->pIndex;
            pEventParm->p.pParam3 = (T_pVOID)pAudioListMenu->ListType;
            
            switch (IconExplorer_GetItemFocusId(&pAudioListMenu->IconExplorer))
            {
                case 0:
                    AK_DEBUG_OUTPUT("\ncreate new list\n");
                    Audio_CreateNewList();
                    break;
                case 1://delete favorite
                    m_triggerEvent(M_EVT_1, pEventParm);
                    break;
                case 2://delete all favorites
                    m_triggerEvent(M_EVT_2, pEventParm);
                    break;
                case 3://add audio to favorite
                    m_triggerEvent(M_EVT_3, pEventParm);
                    break;
                case 4://add all audio to favorite
                    m_triggerEvent(M_EVT_4, pEventParm);
                    break;
                case 5://delete audio
                    m_triggerEvent(M_EVT_5, pEventParm);
                    break;
                case 6://delete all audio 
                    m_triggerEvent(M_EVT_6, pEventParm);
                    break;                    
                default:
                    break;
            }
            break;
        default:
            ReturnDefauleProc(IconExplorerRet, pEventParm);
            break;
    }
#endif
    return 0;
}


#ifdef SUPPORT_AUDIOPLAYER

/**
**create my favorite file
**/
static T_VOID Audio_CreateNewList(T_VOID)
{
    T_STR_FILE      FileName;
    T_USTR_FILE     FilePath;
    T_USTR_FILE     Ustrtmp;
    T_hFILE         fp;
    T_S32           id;
    T_BOOL          ret;
    T_U64_INT       freeSize;
    T_S8            retvalue = 0;
    ret = AK_FALSE;

    Eng_StrMbcs2Ucs(DRI_D"audio/alt", FilePath);
    //Utl_UStrCpy(FilePath, (const T_U16*)Fwl_GetDefPath(eAUDIOLIST_PATH));
    Fwl_FsGetFreeSize(FilePath[0], &freeSize);
    if (U64cmpU32(&freeSize, 2048) < 0)
    {
        AK_DEBUG_OUTPUT("\nfree size = %d\n", freeSize.low);
        retvalue = 2;
    }
    else
    {
        if (AK_TRUE != FileMgr_CheckFileIsExist(FilePath))
        {
            if (Fwl_FsMkDirTree(FilePath) != AK_TRUE)
            {
                retvalue = -1;
            }
        }
        else
        {
            for (id='1';id<='9';id++)
            {
                if(NULL == IconExplorer_GetItem(pAudioListMenu->pIconExplorer,id))
                {
                    break;
                }
            }
            if (id > '9')
            {
                retvalue = 1;
            }
            else
            {
                sprintf(FileName, AUDIO_CUSTOM_LIST_NAME, (int)(id-'0'));
                Utl_UStrCpy(FilePath, Fwl_GetDefPath(eAUDIOLIST_PATH));
                Utl_UStrCat(FilePath, _T(FileName));
                fp = Fwl_FileOpen(FilePath, _FMODE_CREATE, _FMODE_CREATE);
                if (FS_INVALID_HANDLE == fp)
                {
                    retvalue = -1;
                }
                else
                {
                    ret = Fwl_FileClose(fp);
                    if (AK_FALSE == ret)
                    {
                        retvalue = -1;
                    }
                    else
                    {
                        Utl_UStrCpy(Ustrtmp, Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_MYPLYLST));
                        Utl_UStrCatChr(Ustrtmp, (T_WCHR)id,(T_S16)1);     // because file name is "MyList_%d.ALT"
                        IconExplorer_AddItemWithOption(pAudioListMenu->pIconExplorer, id, FilePath, (Utl_UStrLen(FilePath)+1) << 1, \
                        Ustrtmp, AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);
                        IconExplorer_SortItem(pAudioListMenu->pIconExplorer);

                        retvalue = 0;
                    }
                }
            }
        }
    }
    
    if (0 == retvalue)
    {
        MsgBox_InitAfx(&pAudioListMenu->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
    }
    else if (1 == retvalue)
    {
        MsgBox_InitAfx(&pAudioListMenu->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
        MsgBox_InitStr(&pAudioListMenu->msgbox, 2, Res_GetStringByID(eRES_STR_COM_FAILURE_DONE), 
        Res_GetStringByID(eRES_STR_MP3_NOT_ENOUGH_SPACE), MSGBOX_INFORMATION);
    } 
    else if (2 == retvalue)
    {
        MsgBox_InitAfx(&pAudioListMenu->msgbox, 2, ctFAILURE, csEXPLORER_FREE_SIZE, MSGBOX_INFORMATION);
    }
    else    // fail
    {
        MsgBox_InitAfx(&pAudioListMenu->msgbox, 2, ctFAILURE, csFAILURE_DONE, MSGBOX_INFORMATION);
    }
    MsgBox_SetDelay(&pAudioListMenu->msgbox, MSGBOX_DELAY_1);
    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pAudioListMenu->msgbox);
}

#endif
