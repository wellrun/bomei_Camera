
#include "Fwl_public.h"

#ifdef SUPPORT_AUDIOPLAYER
#include "Fwl_Image.h"
#include "Eng_KeyMapping.h"
#include "Fwl_pfAudio.h"
#include "Eng_ImgConvert.h"
#include "Fwl_Initialize.h"
#include "Fwl_osFS.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_DisplayList.h"
#include "Eng_DataConvert.h"
#include "Lib_state.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"


typedef struct {
    T_ICONEXPLORER      IconExplorer;
    T_ICONEXPLORER      *pGIconExplorer;/*valid in audio player module*/
    T_MSGBOX            msgbox;
} T_AUDIO_LIST_MYFAVORITE_PARM;

static T_AUDIO_LIST_MYFAVORITE_PARM *pAudioListMyFavriteParm = AK_NULL;
static T_VOID resume_audio_mylist(T_VOID);
static T_VOID suspend_audio_mylist(T_VOID);
#endif


void initaudio_list_mylist(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_FILELIST      FileList;
    T_USTR_FILE     tempStr;
    T_FILELIST_ITEM *p;
    T_S32           id;

    pAudioListMyFavriteParm = (T_AUDIO_LIST_MYFAVORITE_PARM *)Fwl_Malloc(sizeof(T_AUDIO_LIST_MYFAVORITE_PARM));
    AK_ASSERT_PTR_VOID(pAudioListMyFavriteParm, "initvideo_list(): malloc error\n");

    if (FileList_Init(&FileList, MYLIST_MAX_ITEM_QTY, FILELIST_SORT_NAME,  AudioPlayer_IsSupportListFile) == AK_FALSE)
    {
        AK_DEBUG_OUTPUT("\nfile list init fail\n");
        return;
    }

    if (FileList_Add_Alt(&FileList, Fwl_GetDefPath(eAUDIOLIST_PATH), FILELIST_NO_SEARCH_SUB_NO_RECODE_FOLDER) == FILELIST_ADD_ERROR)
    {
        AK_DEBUG_OUTPUT("\nfile list is inexist or read fail\n");
    }

    MenuStructInit(&pAudioListMyFavriteParm ->IconExplorer);
    IconExplorer_SetTitleText(&pAudioListMyFavriteParm ->IconExplorer, Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_MYPLYLST), ICONEXPLORER_TITLE_TEXTCOLOR);
        
    p = FileList.pItemHead;
    while (p != AK_NULL)
    {
        Utl_UStrCpy(tempStr,Res_GetStringByID(eRES_STR_AUDIOPLAYER_LIST_MYPLYLST));
        id = p->pText[7];
        Utl_UStrCatChr(tempStr,(T_WCHR)id,(T_S16)1);     // because file name is "MyList_%d.ALT"
        IconExplorer_AddItemWithOption(&pAudioListMyFavriteParm ->IconExplorer, id, p->pFilePath, (Utl_UStrLen(p->pFilePath)+1) << 1, \
            tempStr, AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);
        
        p = p->pNext;
    }
    FileList_Free(&FileList);
    
    TopBar_EnableMenuButton();

    m_regResumeFunc(resume_audio_mylist);
    m_regSuspendFunc(suspend_audio_mylist);
#endif
}

void exitaudio_list_mylist(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    TopBar_DisableMenuButton();

    IconExplorer_Free(&pAudioListMyFavriteParm ->IconExplorer);
    pAudioListMyFavriteParm  = Fwl_Free(pAudioListMyFavriteParm );
#endif
}

void paintaudio_list_mylist(void)
{
#ifdef SUPPORT_AUDIOPLAYER

    IconExplorer_Show(&pAudioListMyFavriteParm ->IconExplorer);

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleaudio_list_mylist(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_AUDIOPLAYER

    T_eBACK_STATE   IconExplorerRet;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pAudioListMyFavriteParm ->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_FAVORITE == event)
    {
        pAudioListMyFavriteParm->pGIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;
    }

    IconExplorerRet = IconExplorer_Handler(&pAudioListMyFavriteParm ->IconExplorer, event, pEventParm);
    switch (IconExplorerRet)
    {
        case eNext:
            if (IconExplorer_GetItemFocus(&pAudioListMyFavriteParm ->IconExplorer) != AK_NULL)
            {
                pEventParm->p.pParam1 = pAudioListMyFavriteParm->pGIconExplorer;
                pEventParm->p.pParam2 = &pAudioListMyFavriteParm ->IconExplorer;
                pEventParm->p.pParam3 = (T_pVOID)LTP_FAVORITE;
                m_triggerEvent(M_EVT_LIST, pEventParm);
            }
            break;
            
        case eMenu:
                IconExplorer_SetRefresh(&pAudioListMyFavriteParm ->IconExplorer, ICONEXPLORER_REFRESH_ALL);
                
                pEventParm->p.pParam1 = &pAudioListMyFavriteParm ->IconExplorer;
                pEventParm->p.pParam2 = AK_NULL;
                pEventParm->p.pParam3 = (T_pVOID)LTP_FAVORITELIST;
                
                m_triggerEvent(M_EVT_MENU, pEventParm);
            break;
            
        default:
            ReturnDefauleProc(IconExplorerRet, pEventParm);
            break;
    }
#endif
    return 0;
}


#ifdef SUPPORT_AUDIOPLAYER

static T_VOID resume_audio_mylist(T_VOID)
{
    TopBar_EnableMenuButton();
}

static T_VOID suspend_audio_mylist(T_VOID)
{
	return;
}
#endif

