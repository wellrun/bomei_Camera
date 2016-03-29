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
    T_USTR_FILE     		filename;
} T_SEND_FILE_PARM;

static T_SEND_FILE_PARM *pSendFileParm = AK_NULL;

#endif
/*---------------------- BEGIN OF STATE s_set_send_file ------------------------*/
void initset_send_file(void)
{
#ifdef SUPPORT_NETWORK

    T_USTR_FILE pathstr, namestr;
	T_FILE_TYPE FileType[] = {
        FILE_TYPE_ALL,
        FILE_TYPE_NONE
    };

    pSendFileParm = (T_SEND_FILE_PARM *)Fwl_Malloc(sizeof(T_SEND_FILE_PARM));
    AK_ASSERT_PTR_VOID(pSendFileParm, "initset_send_file error");
	memset(pSendFileParm, 0, sizeof(T_SEND_FILE_PARM));

	if (0 != Utl_UStrLen(gs.sendfile))
	{
    	Utl_USplitFilePath(gs.sendfile, pathstr, namestr);
	}
	else
	{
		Utl_UStrCpy(pathstr, Fwl_GetDefPath(eNETWORK_PATH));
	}
	
    DisplayList_init(&pSendFileParm->displayList, pathstr, \
            Res_GetStringByID(eRES_STR_DEF_SEND_FILE), FileType);
#endif
}

void exitset_send_file(void)
{
#ifdef SUPPORT_NETWORK
	TopBar_DisableMenuButton();
    DisplayList_Free(&pSendFileParm->displayList);
    pSendFileParm = Fwl_Free(pSendFileParm);
#endif
}

void paintset_send_file(void)
{
#ifdef SUPPORT_NETWORK
    DisplayList_Show(&pSendFileParm->displayList);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_send_file(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_eBACK_STATE DisplayListRet;
    T_FILE_INFO *pFileInfo = AK_NULL;
    T_USTR_FILE     FilePath;

    if (IsPostProcessEvent(event))
    {
        DisplayList_SetRefresh(&pSendFileParm->displayList, DISPLAYLIST_REFRESH_ALL);
        return 1;
    }

	DisplayListRet = DisplayList_Handler(&pSendFileParm->displayList, event, pEventParm);
	switch (DisplayListRet)
    {
        case eNext:
            DisplayList_SetRefresh(&pSendFileParm->displayList, DISPLAYLIST_REFRESH_ALL);
            pFileInfo = DisplayList_Operate(&pSendFileParm->displayList);
            if (pFileInfo != AK_NULL)
            {
                Utl_UStrCpy(FilePath, DisplayList_GetCurFilePath(&pSendFileParm->displayList));
                Utl_UStrCat(FilePath, pFileInfo->name);
                Utl_UStrCpy(gs.sendfile, FilePath);
                MsgBox_InitAfx(&pSendFileParm->msgbox, 2, ctSUCCESS, csSELECT_OK, MSGBOX_INFORMATION);
                MsgBox_SetDelay(&pSendFileParm->msgbox, MSGBOX_DELAY_1);
                m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pSendFileParm->msgbox);
            }
            break;
        default:
            ReturnDefauleProc(DisplayListRet, pEventParm);
            break;
    }
    
#endif
    return 0;
}





