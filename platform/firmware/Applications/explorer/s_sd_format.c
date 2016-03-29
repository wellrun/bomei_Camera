#include "Fwl_public.h"
#include "Fwl_Image.h"
#include "Ctl_Msgbox.h"
#include "Ctl_DisplayList.h"
#include "Ctl_Audioplayer.h"
#include "Eng_DataConvert.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "Gbl_Global.h"
#include "fwl_display.h"
#include "fwl_oscom.h"

#define   TIMER_DISP_TIME    (2*1000)     //2S

typedef enum {
    eSD_INTERFACE_SDMMC = 0,
    eSD_INTERFACE_SDIO,
    eSD_INTERFACE_COUNT
} T_eINTERFACE_TYPE;

typedef struct {
    T_MSGBOX            msgbox;
    T_TIMER             timer_id;
} T_SD_FORMAT_PARM;

static T_SD_FORMAT_PARM *pSD_Format_Parm;

/*---------------------- BEGIN OF STATE s_img_delete_cnfm ------------------------*/
void initsd_format(void)
{
    pSD_Format_Parm = (T_SD_FORMAT_PARM *)Fwl_Malloc(sizeof(T_SD_FORMAT_PARM));
    AK_ASSERT_PTR_VOID(pSD_Format_Parm, "initsd_format(): malloc error");

    pSD_Format_Parm->timer_id = ERROR_TIMER;
}

void exitsd_format(void)
{
    if (ERROR_TIMER != pSD_Format_Parm->timer_id)
    {
        Fwl_StopTimer(pSD_Format_Parm->timer_id);
        pSD_Format_Parm->timer_id = ERROR_TIMER;    
    }
    
    pSD_Format_Parm = Fwl_Free(pSD_Format_Parm);
}

void paintsd_format(void)
{
    MsgBox_Show(&pSD_Format_Parm->msgbox);
    Fwl_RefreshDisplay();
}

unsigned char handlesd_format(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_eBACK_STATE menuRet;
    
    if (IsPostProcessEvent(event))
    {
        MsgBox_SetRefresh(&pSD_Format_Parm->msgbox, CTL_REFRESH_ALL);
        return 1;
    }

    switch (event)
    {
        case M_EVT_SD_MOUNT_STATE:
            if (ERROR_TIMER == pSD_Format_Parm->timer_id)
            {
                pSD_Format_Parm->timer_id = Fwl_SetTimerMilliSecond(TIMER_DISP_TIME, AK_FALSE);
            }
            
            if (eSD_INTERFACE_SDIO == pEventParm->c.Param2 || eSD_INTERFACE_SDMMC == pEventParm->c.Param2)
            {
                switch (pEventParm->c.Param1)
                {
                    case EVT_SD_PLUG_IN:
                        if (AK_TRUE == pEventParm->c.Param3)
                        {
                            MsgBox_InitAfx(&pSD_Format_Parm->msgbox, 0, ctSUCCESS, csEXPLORER_MOUNT_OK, MSGBOX_INFORMATION | MSGBOX_OK);
                        }
                        else
                        {
                            MsgBox_InitAfx(&pSD_Format_Parm->msgbox, 0, ctSUCCESS, csEXPLORER_MOUNT_FAIL, MSGBOX_INFORMATION | MSGBOX_OK);
                        }
                        break;
                        
                    case EVT_SD_PLUG_OUT:
                        if (AK_TRUE == pEventParm->c.Param3)
                        {
                            MsgBox_InitAfx(&pSD_Format_Parm->msgbox, 0, ctSUCCESS, csEXPLORER_UNMOUNT_OK, MSGBOX_INFORMATION | MSGBOX_OK);                        
                        }
                        else
                        {
                            MsgBox_InitAfx(&pSD_Format_Parm->msgbox, 0, ctSUCCESS, csEXPLORER_UNMOUNT_FAIL, MSGBOX_INFORMATION | MSGBOX_OK);
                        }
                        break;
                }
            }
            break;
        case VME_EVT_TIMER:
            m_triggerEvent(M_EVT_EXIT, (T_EVT_PARAM *)&pSD_Format_Parm->msgbox);
            break;
        default:
            break;
    }

    menuRet = MsgBox_Handler(&pSD_Format_Parm->msgbox, event, pEventParm);
    switch(menuRet)
    {
        case eNext:
            MsgBox_SetDelay(&pSD_Format_Parm->msgbox, MSGBOX_DELAY_1);
            m_triggerEvent(M_EVT_EXIT, (T_EVT_PARAM *)&pSD_Format_Parm->msgbox);
            break;
        default:
            ReturnDefauleProc(menuRet, pEventParm);
            break;
    }

    return 0;
}

