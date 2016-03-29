/**toolbox menu state machine*/
/*@author zhengwenbo*/
/*@filename s_toolbox_menu.c*/

#include "Fwl_public.h"

#ifdef SUPPORT_TOOLBOX
#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Fwl_osMalloc.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "Fwl_sys_detect.h"

#ifdef USB_HOST
#include "Fwl_usb_host.h"
#include "fwl_power.h"
#endif

typedef struct{
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX msgbox;
#ifdef CAMERA_SUPPORT
    T_BOOL  msgflag;
#endif
}T_TOOL_MENU_PARM;

static T_TOOL_MENU_PARM *pToolMenu_Parm = AK_NULL;
#endif

void inittool_menu(void)
{
#ifdef SUPPORT_TOOLBOX

    pToolMenu_Parm = (T_TOOL_MENU_PARM*)Fwl_Malloc(sizeof(T_TOOL_MENU_PARM));
    AK_ASSERT_PTR_VOID(pToolMenu_Parm, "inittool_menu(): pToolMenu_Parm malloc error\n");

    MenuStructInit(&pToolMenu_Parm->IconExplorer);
    GetMenuStructContent(&pToolMenu_Parm->IconExplorer, mnTOOLBOX_MENU);

#ifdef CAMERA_SUPPORT
    pToolMenu_Parm->msgflag= AK_FALSE;
#endif
#endif
}

void painttool_menu(void)
{
#ifdef SUPPORT_TOOLBOX

#ifdef CAMERA_SUPPORT
    if (pToolMenu_Parm->msgflag== AK_TRUE)
        MsgBox_Show(&pToolMenu_Parm->msgbox);
    else
#endif
        IconExplorer_Show(&pToolMenu_Parm->IconExplorer);

    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

void exittool_menu(void)
{
#ifdef SUPPORT_TOOLBOX

    IconExplorer_Free(&pToolMenu_Parm->IconExplorer);
    Fwl_Free(pToolMenu_Parm);
#endif
}

unsigned char handletool_menu(T_EVT_CODE event, T_EVT_PARAM * pEventParm)
{
#ifdef SUPPORT_TOOLBOX

    T_eBACK_STATE   IconExplorerRet;
#ifdef CAMERA_SUPPORT
    T_eBACK_STATE   msgRet;
#endif
    T_U16           focusID;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pToolMenu_Parm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

#ifdef USB_HOST
    if (M_EVT_USBHOST_EXIT == event)
    {
        // refresh tools menu
        IconExplorer_DelAllItem(&pToolMenu_Parm->IconExplorer);
        GetMenuStructContent(&pToolMenu_Parm->IconExplorer, mnTOOLBOX_MENU);
        //IconExplorer_SetRefresh(&pToolMenu_Parm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
    }
#endif

#ifdef CAMERA_SUPPORT
    if (pToolMenu_Parm->msgflag == AK_TRUE)
    {
        msgRet = MsgBox_Handler(&pToolMenu_Parm->msgbox, event, pEventParm);
        if((msgRet == eReturn) || (msgRet == eNext))
        {
//            gb.UsbMode = USB_DISK_MODE;             // set  to UsbDisk mode
            pToolMenu_Parm->msgflag = AK_FALSE;
        }
    }
    else
#endif
    {
        IconExplorerRet = IconExplorer_Handler(&pToolMenu_Parm->IconExplorer, event, pEventParm);
        switch (IconExplorerRet)
        {
            case eNext:
                focusID = (T_U16)IconExplorer_GetItemFocusId(&pToolMenu_Parm->IconExplorer);  
                switch (focusID)
                {
#ifdef CAMERA_SUPPORT
                    case 10: //set to  PC Camera mode
						Fwl_UsbSetDealProc(USB_DEAL_BEGIN);
						Fwl_UsbSetMode(USB_CAMERA_MODE);
		                m_triggerEvent(M_EVT_USB_CAMERA, pEventParm);
                        break;
#endif
#ifdef SUPPORT_EMAP
                    case 30: // Emap
                        //GE_ShadeInit();
                        m_triggerEvent(M_EVT_1,pEventParm);
                        break;
#endif
#ifdef USB_HOST
                    case 40:
                        break;
#endif
#ifdef SUPPORT_CALC
                    case 50: // calculator
                        GE_ShadeInit();
                        m_triggerEvent(M_EVT_3,pEventParm);
                        break;
#endif
#ifdef SUPPORT_AUTOTEST

					case 60:
						GE_ShadeInit();
						m_triggerEvent(M_EVT_4, pEventParm);
						break;
#endif
                    default:
                        GE_ShadeInit();
                        m_triggerEvent(M_EVT_EXIT,pEventParm);
                        break;
                }
                break;
            default:
                ReturnDefauleProc(IconExplorerRet, pEventParm);
                break;
        }
    }
#endif
    return 0;
}
