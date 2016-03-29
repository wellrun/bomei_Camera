
#include "Fwl_public.h"

#ifdef SUPPORT_VIDEOPLAYER

#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_DynamicFont.h"
#include "Ctl_AudioPlayer.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_ICONEXPLORER      IconExplorer;
    T_MSGBOX            msgbox;
} T_VIDEO_REPEAT_PARM;

static T_VIDEO_REPEAT_PARM *pVideo_RepeatParm;
#endif
/*---------------------- BEGIN OF STATE s_mp3_list_repeat_mode ------------------------*/
void initvideo_list_repeat_mode(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    pVideo_RepeatParm = (T_VIDEO_REPEAT_PARM *)Fwl_Malloc(sizeof(T_VIDEO_REPEAT_PARM));
    AK_ASSERT_PTR_VOID(pVideo_RepeatParm, "initvideo_list_repeat_mode(): malloc error");

    MenuStructInit(&pVideo_RepeatParm->IconExplorer);
    GetMenuStructContent(&pVideo_RepeatParm->IconExplorer, mnMP3_REPEAT_MODE);
    IconExplorer_SetFocus(&pVideo_RepeatParm->IconExplorer, gs.VideoRepMode);
    TopBar_DisableMenuButton();
#endif
}

void exitvideo_list_repeat_mode(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    IconExplorer_Free(&pVideo_RepeatParm->IconExplorer);
    pVideo_RepeatParm = Fwl_Free(pVideo_RepeatParm);
#endif
}

void paintvideo_list_repeat_mode(void)
{
#ifdef SUPPORT_VIDEOPLAYER

    IconExplorer_Show(&pVideo_RepeatParm->IconExplorer);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlevideo_list_repeat_mode(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_VIDEOPLAYER

    T_eBACK_STATE IconExplorerRet;
    T_U32 focusID;
//    T_FILELIST *pFileList = AK_NULL;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pVideo_RepeatParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pVideo_RepeatParm->IconExplorer, event, pEventParm);
    switch (IconExplorerRet)
    {
    case eNext:
        focusID = IconExplorer_GetItemFocusId(&pVideo_RepeatParm->IconExplorer);
        if (focusID != gs.VideoRepMode)
        {
            gs.VideoRepMode = (T_U8)focusID;
//          SaveUserdata();
        }
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}








