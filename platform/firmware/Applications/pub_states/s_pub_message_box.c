
#include "Fwl_public.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"

#define MAX_PUB_MSG_LEVEL       10

T_S8            CurLevel = -1;

static T_MSGBOX     *msgbox[MAX_PUB_MSG_LEVEL];

/*---------------------- BEGIN OF STATE s_pub_message_box ------------------------*/
void initpub_message_box(void)
{
    CurLevel++;
    if (CurLevel < MAX_PUB_MSG_LEVEL && CurLevel >= 0)
    {
        msgbox[CurLevel] = AK_NULL;
    }
    AK_ASSERT_VAL_VOID(CurLevel < MAX_PUB_MSG_LEVEL && CurLevel >= 0,"message box level overflow");
}

void exitpub_message_box(void)
{
    CurLevel--;
}

void paintpub_message_box(void)
{
    T_RECT msgRect;

    if (CurLevel < MAX_PUB_MSG_LEVEL && CurLevel >= 0)
    {
        if (msgbox[CurLevel] != AK_NULL)
        {
            MsgBox_Show(msgbox[CurLevel]);
            MsgBox_GetRect(msgbox[CurLevel], &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);
        }
    }
}

void messagebox_RetProc(T_U16 retLevel, T_EVT_PARAM *pEventParm)
{
    if( retLevel > 10 )
    {
        m_triggerEvent( M_EVT_Z09COM_SYS_RESET, pEventParm );
        return;
    }

    switch( retLevel )
    {
    case 1:
        m_triggerEvent( M_EVT_RETURN, pEventParm );
        break;
    case 2:
        m_triggerEvent( M_EVT_RETURN2, pEventParm );
        break;
    case 3:
        m_triggerEvent( M_EVT_RETURN3, pEventParm );
        break;
    case 4:
        m_triggerEvent( M_EVT_RETURN4, pEventParm );
        break;
    case 5:
        m_triggerEvent( M_EVT_RETURN5, pEventParm );
        break;
    case 6:
        m_triggerEvent( M_EVT_RETURN6, pEventParm );
        break;
    case 7:
        m_triggerEvent( M_EVT_RETURN7, pEventParm );
        break;
    case 8:
        m_triggerEvent( M_EVT_RETURN8, pEventParm );
        break;
    case 9:
        m_triggerEvent( M_EVT_RETURN9, pEventParm );
        break;
    case 10:
        m_triggerEvent( M_EVT_RETURN10, pEventParm );
        break;
    }
}

unsigned char handlepub_message_box(T_EVT_CODE event, T_EVT_PARAM* pEventParm)
{
    T_eBACK_STATE   mboxRet;

    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(msgbox[CurLevel], CTL_REFRESH_ALL);
        return 1;
    }

    if (event == M_EVT_MESSAGE)
    {
        AK_ASSERT_PTR(pEventParm, "handlepub_message_box(): pEventParm", 0);

        if (CurLevel >= MAX_PUB_MSG_LEVEL)
        {
            //pEventParm->s.Param1 = ((T_MSGBOX *)pEventParm)->ReturnLevel;
            //m_triggerEvent(M_EVT_RETURN, pEventParm);
            messagebox_RetProc( ((T_MSGBOX *)pEventParm)->ReturnLevel, pEventParm );
        }
        else
        {
            msgbox[CurLevel] = (T_MSGBOX *)pEventParm;
        }
        return 0;
    }

    AK_ASSERT_VAL(CurLevel < MAX_PUB_MSG_LEVEL && CurLevel >= 0,"message box level overflow-2", 0);

    if (msgbox[CurLevel] == AK_NULL)
    {
        return 0;
    }

    mboxRet = MsgBox_Handler(msgbox[CurLevel], event, pEventParm);
    switch (mboxRet) {
    case eReturn:
    case eNext:
        AK_ASSERT_PTR(pEventParm, "handlepub_message_box(): pEventParm", 0);
        messagebox_RetProc( msgbox[CurLevel]->ReturnLevel, pEventParm );
        return 0;
    case eHome:
        AK_ASSERT_PTR(pEventParm, "handlepub_message_box(): pEventParm", 0);
        m_triggerEvent( M_EVT_Z09COM_SYS_RESET, pEventParm );
        return 0;
    default:
        break;
    }

    return 0;
}
