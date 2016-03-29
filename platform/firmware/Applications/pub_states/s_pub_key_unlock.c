
#include "Fwl_public.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_MSGBOX        msgbox;
} T_KEYUNLOCK_PARM;

static T_KEYUNLOCK_PARM *pKeyUnlockParm;
static T_U8 *lcd_buffer;

/*---------------------- BEGIN OF STATE s_stdb_key_unlock ------------------------*/
void initpub_key_unlock(void)
{
    AK_DEBUG_OUTPUT("key lock or unlock\n");

#ifdef OS_ANYKA
    lcd_buffer = (T_U8 *)Fwl_Malloc(Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*2);
    AK_ASSERT_PTR_VOID(lcd_buffer,"lcd_buffer malloc error");
    memcpy((void *)lcd_buffer, (void *)Fwl_GetDispMemory(), Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*2);
#endif
#ifdef OS_WIN32
    lcd_buffer = (T_U8 *)Fwl_Malloc(Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*3);
    AK_ASSERT_PTR_VOID(lcd_buffer,"lcd_buffer malloc error");
    memcpy((void *)lcd_buffer, (void *)Fwl_GetDispMemory(), Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*3);
#endif

    pKeyUnlockParm = (T_KEYUNLOCK_PARM *)Fwl_Malloc(sizeof(T_KEYUNLOCK_PARM));
    AK_ASSERT_PTR_VOID(pKeyUnlockParm, "initpub_key_unlock(): malloc error");
}

void exitpub_key_unlock(void)
{
#ifdef OS_ANYKA
    memcpy((void *)Fwl_GetDispMemory(), (void *)lcd_buffer, Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*2);
#endif
#ifdef OS_WIN32
    memcpy((void *)Fwl_GetDispMemory(), (void *)lcd_buffer, Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*3);
#endif
    lcd_buffer = Fwl_Free(lcd_buffer);

    pKeyUnlockParm = Fwl_Free(pKeyUnlockParm);

    // Before leaving, it should refresh LCD's buffer to previous contents.
    // Calling Fwl_RefreshDisplay() to refresh the buffer of 565.
    Fwl_RefreshDisplay();
}

void paintpub_key_unlock(void)
{
}

unsigned char handlepub_key_unlock(T_EVT_CODE event, T_EVT_PARAM* pEventParm)
{
    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    if (gb.KeyLocked)
        MsgBox_InitAfx(&pKeyUnlockParm->msgbox, 2, ctHINT, csKEY_LOCKED, MSGBOX_INFORMATION);
    else
        MsgBox_InitAfx(&pKeyUnlockParm->msgbox, 2, ctHINT, csKEY_UNLOCKED, MSGBOX_INFORMATION);
    MsgBox_SetDelay(&pKeyUnlockParm->msgbox, MSGBOX_DELAY_1);
    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pKeyUnlockParm->msgbox);

    return 0;
}
