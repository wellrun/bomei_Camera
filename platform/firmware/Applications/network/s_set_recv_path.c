#include "Fwl_public.h"

#ifdef SUPPORT_NETWORK
#include <time.h>
#include <string.h>
#include "Ctl_Msgbox.h"
#include "Ctl_DisplayList.h"
#include "Fwl_Initialize.h"
#include "Eng_String.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_DISPLAYLIST           displayList;
    T_MSGBOX                msgbox;
    T_USTR_FILE     		path;
	T_BOOL                  MsgFlag;
} T_RECV_PATH_PARM;

static T_RECV_PATH_PARM *pRecvPathParm = AK_NULL;

#endif
/*---------------------- BEGIN OF STATE s_set_recv_path ------------------------*/
void initset_recv_path(void)
{
#ifdef SUPPORT_NETWORK
	T_FILE_TYPE FileType[] = {
        FILE_TYPE_ALL,
        FILE_TYPE_NONE
    };

    pRecvPathParm = (T_RECV_PATH_PARM *)Fwl_Malloc(sizeof(T_RECV_PATH_PARM));
    AK_ASSERT_PTR_VOID(pRecvPathParm, "initset_recv_path error");
	memset(pRecvPathParm, 0, sizeof(T_RECV_PATH_PARM));
	
    DisplayList_init(&pRecvPathParm->displayList, Fwl_GetDefPath(eNETWORK_PATH), \
            Res_GetStringByID(eRES_STR_DEF_RECV_PATH), FileType);
#endif
}

void exitset_recv_path(void)
{
#ifdef SUPPORT_NETWORK
	TopBar_DisableMenuButton();
    DisplayList_Free(&pRecvPathParm->displayList);
    pRecvPathParm = Fwl_Free(pRecvPathParm);
#endif
}

void paintset_recv_path(void)
{
#ifdef SUPPORT_NETWORK
	TopBar_EnableMenuButton();

	if (pRecvPathParm->MsgFlag)
	{
		MsgBox_Show(&pRecvPathParm->msgbox);
	}
	else
	{
		DisplayList_Show(&pRecvPathParm->displayList);
	}

    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_recv_path(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_eBACK_STATE DisplayListRet;
    T_FILE_INFO *pFileInfo = AK_NULL;
	T_eBACK_STATE msgRet;

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pRecvPathParm->displayList, DISPLAYLIST_REFRESH_ALL);
        return 1;
    }

	if (pRecvPathParm->MsgFlag)
    {
        msgRet = MsgBox_Handler(&pRecvPathParm->msgbox, event, pEventParm);
        switch (msgRet)
        {
        case eNext:
			Fwl_SetDefPath(eNETWORK_PATH, pRecvPathParm->path);
            MsgBox_InitAfx(&pRecvPathParm->msgbox, 2, ctSUCCESS, csSUCCESS_DONE, MSGBOX_INFORMATION);
            MsgBox_SetDelay(&pRecvPathParm->msgbox, MSGBOX_DELAY_0);
            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pRecvPathParm->msgbox);
			break;
        case eReturn:
            pRecvPathParm->MsgFlag = AK_FALSE;
            DisplayList_SetRefresh(&pRecvPathParm->displayList, DISPLAYLIST_REFRESH_ALL);
            break;
        default:
            break;
        }
    }
    else
    {
		DisplayListRet = DisplayList_Handler(&pRecvPathParm->displayList, event, pEventParm);
		switch (DisplayListRet)
	    {
		case eNext:
			DisplayList_SetRefresh(&pRecvPathParm->displayList, DISPLAYLIST_REFRESH_ALL);
			pFileInfo = DisplayList_Operate(&pRecvPathParm->displayList);
			break;

		case eMenu:
			pFileInfo = DisplayList_GetItemContentFocus(&pRecvPathParm->displayList);
			if ((DisplayList_GetSubLevel(&pRecvPathParm->displayList) > 0) && pFileInfo)
			{
				if ((pFileInfo->attrib&0x10) == 0x10)
				{
					if (Utl_UStrCmp(pFileInfo->name, _T("..")) == 0)
					{
						//ÉÏ¼¶Ä¿Â¼
						break;
					}
					
					Utl_UStrCpy(pRecvPathParm->path, DisplayList_GetCurFilePath(&pRecvPathParm->displayList));
					
					if (Utl_UStrCmp(pFileInfo->name, _T(".")) != 0)
			    	{
						Utl_UStrCat(pRecvPathParm->path, pFileInfo->name);
						Utl_UStrCat(pRecvPathParm->path, _T("/"));
					}
				
			        TopBar_DisableMenuButton();
			        DisplayList_SetRefresh(&pRecvPathParm->displayList, DISPLAYLIST_REFRESH_ALL);
					MsgBox_InitStr(&pRecvPathParm->msgbox, 0, GetCustomTitle(ctHINT), Res_GetStringByID(eRES_STR_AS_DEF_RECV_PATH), MSGBOX_QUESTION | MSGBOX_YESNO);
					MsgBox_Show(&pRecvPathParm->msgbox);
					pRecvPathParm->MsgFlag = AK_TRUE;
				}
			}
			break;
		default:
			ReturnDefauleProc(DisplayListRet, pEventParm);
			break;
	    }
    }
#endif
    return 0;
}





