
#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Eng_DynamicFont.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfAudio.h"
#include "Eng_DataConvert.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_MSGBOX        msgbox;
} T_VERSION_PARM;

static T_VERSION_PARM *pVersionParm;
#endif
/*---------------------- BEGIN OF STATE s_set_disp_version ------------------------*/
void initset_disp_version(void)
{
#ifdef SUPPORT_SYS_SET

    T_USTR_INFO utmpstr;
    T_USTR_INFO utmpstr2;

    pVersionParm = (T_VERSION_PARM *)Fwl_Malloc(sizeof(T_VERSION_PARM));
    AK_ASSERT_PTR_VOID(pVersionParm, "initset_disp_version(): malloc error");

    utmpstr[0] = 0;
    MsgBox_InitStr(&pVersionParm->msgbox, 0, GetCustomTitle(ctHINT), utmpstr, MSGBOX_INFORMATION | MSGBOX_OK);

    Utl_UStrCpy(utmpstr, GetCustomString(csVERSION_SOFTWARE));
    Eng_StrMbcs2Ucs(AK_VERSION_SOFTWARE, utmpstr2);
    Utl_UStrCat(utmpstr, utmpstr2);
    MsgBox_AddLine(&pVersionParm->msgbox, utmpstr);
    Utl_UStrCpy(utmpstr, GetCustomString(csVERSION_HARDWARE));
    Eng_StrMbcs2Ucs(AK_VERSION_HARDWARE, utmpstr2);
    Utl_UStrCat(utmpstr, utmpstr2);
    MsgBox_AddLine(&pVersionParm->msgbox, utmpstr);
#endif
}

void exitset_disp_version(void)
{
#ifdef SUPPORT_SYS_SET
    pVersionParm = Fwl_Free(pVersionParm);
#endif
}

void paintset_disp_version(void)
{
#ifdef SUPPORT_SYS_SET
    MsgBox_Show(&pVersionParm->msgbox);
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_disp_version(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE msgRet;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pVersionParm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    msgRet = MsgBox_Handler(&pVersionParm->msgbox, event, pEventParm);
    switch(msgRet)
    {
    case eNext:
        m_triggerEvent(M_EVT_EXIT, pEventParm);
        break;
    default:
        ReturnDefauleProc(msgRet, pEventParm);
        break;
    }
#endif
    return 0;
}
