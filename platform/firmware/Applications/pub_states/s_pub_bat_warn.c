
#include "Fwl_public.h"
#include "Ctl_Msgbox.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"

typedef struct {
    T_MSGBOX        msgbox;
} T_BATWARN_PARM;

static T_BATWARN_PARM *pBatWarnParm;
static T_U8 *lcd_buffer;

/*---------------------- BEGIN OF STATE s_pub_bat_warn ------------------------*/
void initpub_bat_warn(void)
{
    AK_DEBUG_OUTPUT("battery charge warning\n");

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

    pBatWarnParm = (T_BATWARN_PARM *)Fwl_Malloc(sizeof(T_BATWARN_PARM));
    AK_ASSERT_PTR_VOID(pBatWarnParm, "initpub_bat_warn(): malloc error");
}

void exitpub_bat_warn(void)
{
#ifdef OS_ANYKA
    memcpy((void *)Fwl_GetDispMemory(), (void *)lcd_buffer, Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*2);
#endif
#ifdef OS_WIN32
    memcpy((void *)Fwl_GetDispMemory(), (void *)lcd_buffer, Fwl_GetLcdWidth()*Fwl_GetLcdHeight()*3);
#endif
    lcd_buffer = Fwl_Free(lcd_buffer);

    pBatWarnParm = Fwl_Free(pBatWarnParm);

    // Before leaving, it should refresh LCD's buffer to previous contents.
    // Calling Fwl_RefreshDisplay() to refresh the buffer of 565.
    Fwl_RefreshDisplay();
}

void paintpub_bat_warn(void)
{
}

unsigned char handlepub_bat_warn(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    MsgBox_InitAfx(&pBatWarnParm->msgbox, 2, ctWARNING, csLOW_BATTERY, MSGBOX_INFORMATION);
    MsgBox_SetDelay(&pBatWarnParm->msgbox, MSGBOX_DELAY_1);
    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pBatWarnParm->msgbox);

    return 0;
}
