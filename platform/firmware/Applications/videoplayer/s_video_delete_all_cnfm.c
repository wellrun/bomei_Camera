
#include "Fwl_public.h"

#ifdef SUPPORT_VIDEOPLAYER
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfAudio.h"
#include "Ctl_FileList.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "svc_medialist.h"

typedef struct {
    T_ICONEXPLORER          *pIconExplorer;
    T_MSGBOX                msgbox;
} T_VIDEO_DELETE_ALL_PARM;

static T_VIDEO_DELETE_ALL_PARM *pVideo_Delete_All_Parm;
#endif
/*---------------------- BEGIN OF STATE s_mp3_delete_all_cnfm ------------------------*/
void initvideo_delete_all_cnfm(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    pVideo_Delete_All_Parm = (T_VIDEO_DELETE_ALL_PARM *)Fwl_Malloc(sizeof(T_VIDEO_DELETE_ALL_PARM));
    AK_ASSERT_PTR_VOID(pVideo_Delete_All_Parm, "initmp3_delete_all_cnfm(): malloc error");

    MsgBox_InitAfx(&pVideo_Delete_All_Parm->msgbox, 0, ctHINT, csVIDEO_DELETE_ALL_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
    
    TopBar_DisableMenuButton();
#endif
}

void exitvideo_delete_all_cnfm(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    pVideo_Delete_All_Parm = Fwl_Free(pVideo_Delete_All_Parm);
#endif
}

void paintvideo_delete_all_cnfm(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    MsgBox_Show(&pVideo_Delete_All_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlevideo_delete_all_cnfm(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_eBACK_STATE menuRet;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pVideo_Delete_All_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_5 == event)
        pVideo_Delete_All_Parm->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;

    menuRet = MsgBox_Handler(&pVideo_Delete_All_Parm->msgbox, event, pEventParm);
    switch(menuRet)
    {
    case eNext:
        if (pVideo_Delete_All_Parm->pIconExplorer != AK_NULL)
            IconExplorer_DelAllItem(pVideo_Delete_All_Parm->pIconExplorer);

        MList_RemoveAll(eMEDIA_LIST_VIDEO);

        MsgBox_InitAfx(&pVideo_Delete_All_Parm->msgbox, 3, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
        MsgBox_SetDelay(&pVideo_Delete_All_Parm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideo_Delete_All_Parm->msgbox);
        break;
    default:
        ReturnDefauleProc(menuRet, pEventParm);
        break;
    }
#endif
    return 0;
}
