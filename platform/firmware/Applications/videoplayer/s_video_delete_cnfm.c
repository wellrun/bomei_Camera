
#include "Fwl_public.h"

#ifdef SUPPORT_VIDEOPLAYER
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Ctl_FileList.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "svc_medialist.h"

typedef struct {
    T_ICONEXPLORER          *pIconExplorer;
    T_MSGBOX                msgbox;
} T_VIDEO_DELETE_PARM;

static T_VIDEO_DELETE_PARM *pVideo_Delete_Parm;
#endif
/*---------------------- BEGIN OF STATE s_mp3_delete_cnfm ------------------------*/
void initvideo_delete_cnfm(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    pVideo_Delete_Parm = (T_VIDEO_DELETE_PARM *)Fwl_Malloc(sizeof(T_VIDEO_DELETE_PARM));
    AK_ASSERT_PTR_VOID(pVideo_Delete_Parm, "initmp3_delete_cnfm(): malloc error");

    MsgBox_InitAfx(&pVideo_Delete_Parm->msgbox, 0, ctHINT, csVIDEO_DELETE_NOTE, MSGBOX_QUESTION | MSGBOX_YESNO);
    
    TopBar_DisableMenuButton();
#endif
}

void exitvideo_delete_cnfm(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    pVideo_Delete_Parm = Fwl_Free(pVideo_Delete_Parm);
#endif
}

void paintvideo_delete_cnfm(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    MsgBox_Show(&pVideo_Delete_Parm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlevideo_delete_cnfm(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_eBACK_STATE menuRet;
    T_INDEX_CONTENT *pcontent = AK_NULL;
    T_USTR_FILE		path = {0};
    T_ICONEXPLORER_ITEM *p;
	T_S16			failStrId = csFAILURE_DONE;
	T_BOOL			ret = AK_TRUE;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pVideo_Delete_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    if (M_EVT_4 == event)
        pVideo_Delete_Parm->pIconExplorer = (T_ICONEXPLORER *)pEventParm->p.pParam1;

    menuRet = MsgBox_Handler(&pVideo_Delete_Parm->msgbox, event, pEventParm);
    switch (menuRet)
    {
    case eNext:
    	p = IconExplorer_GetItemFocus(pVideo_Delete_Parm->pIconExplorer);
        if (AK_NULL != p)
        {
        	pcontent = (T_INDEX_CONTENT *)p->pContent;
			if (AK_NULL != pcontent)
			{
        		MList_GetItem(path, pcontent->id, eMEDIA_LIST_VIDEO);
        	}

			if (MList_IsAdding(eMEDIA_LIST_VIDEO))
			{
				ret = AK_FALSE;
				failStrId = csBUSY_CANNOT_DEL;
			}
			else
			{
				ret = MList_RemoveMediaItem(path, AK_FALSE, eMEDIA_LIST_VIDEO);
				IconExplorer_DelItemFocus(pVideo_Delete_Parm->pIconExplorer);
			}
		}

		if (!ret)
        {
            MsgBox_InitAfx(&pVideo_Delete_Parm->msgbox, 3, ctFAILURE, failStrId, MSGBOX_INFORMATION);
        }
        else
        {
            MsgBox_InitAfx(&pVideo_Delete_Parm->msgbox, 3, ctSUCCESS, csCOMMAND_SENT, MSGBOX_INFORMATION);
        }
		
        MsgBox_SetDelay(&pVideo_Delete_Parm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pVideo_Delete_Parm->msgbox);
        break;
    default:
        ReturnDefauleProc(menuRet, pEventParm);
        break;
    }
#endif
    return 0;

}
