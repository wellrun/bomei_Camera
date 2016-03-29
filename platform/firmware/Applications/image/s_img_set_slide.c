
#include "Fwl_public.h"

#ifdef SUPPORT_IMG_BROWSE
#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Eng_DynamicFont.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_SLIDE_PARM;

static T_SLIDE_PARM *pSlideParm;
#endif
/*---------------------- BEGIN OF STATE s_img_set_slide ------------------------*/
void initimg_set_slide(void)
{
#ifdef SUPPORT_IMG_BROWSE

    pSlideParm = (T_SLIDE_PARM *)Fwl_Malloc(sizeof(T_SLIDE_PARM));
    AK_ASSERT_PTR_VOID(pSlideParm, "initimg_set_slide(): malloc error");

    MenuStructInit(&pSlideParm->IconExplorer);
    GetMenuStructContent(&pSlideParm->IconExplorer, mnIMG_SLIDE);
    IconExplorer_SetFocus(&pSlideParm->IconExplorer, gs.ImgSlideInterval);
#endif
}

void exitimg_set_slide(void)
{
#ifdef SUPPORT_IMG_BROWSE

    IconExplorer_Free(&pSlideParm->IconExplorer);
    pSlideParm = Fwl_Free(pSlideParm);
#endif
}

void paintimg_set_slide(void)
{
#ifdef SUPPORT_IMG_BROWSE

    IconExplorer_Show(&pSlideParm->IconExplorer);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleimg_set_slide(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_IMG_BROWSE

    T_eBACK_STATE IconExplorerRet;
    T_U16 focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pSlideParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

    IconExplorerRet = IconExplorer_Handler(&pSlideParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        focusID = (T_U16)IconExplorer_GetItemFocusId(&pSlideParm->IconExplorer);
        if (focusID != gs.ImgSlideInterval)
        {
            gs.ImgSlideInterval = (T_U8)focusID;
        }

        m_triggerEvent(M_EVT_EXIT,pEventParm);
        break;
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}
