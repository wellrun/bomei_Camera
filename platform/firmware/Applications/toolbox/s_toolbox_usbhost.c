/**toolbox menu state machine*/
/*@author zhengwenbo*/
/*@filename s_toolbox_menu.c*/

#ifdef USB_HOST

#include "Fwl_public.h"
#include "Fwl_Initialize.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_Msgbox.h"
#include "Fwl_osMalloc.h"
#include "fwl_usb_host.h"
#include "Fwl_osFS.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_Fm.h"
#include "Eng_ScreenSave.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "Fwl_usb_host.h"
#include "fwl_usb.h"
#include "fwl_oscom.h"
#include "fwl_display.h"
#include "Fwl_oscom.h"
#include "Fwl_sys_detect.h"
#include "Fwl_usb_host.h"


typedef enum {
    UNCONNECTED = 0,
    CONNECTING,
    CONN_FAIL,
    CONN_SUCCESS,
    CONN_DISABLE,
    CONNECTED
} T_USB_CONNECT_STATE;

typedef struct{
    T_MSGBOX            msgbox;
    T_USB_CONNECT_STATE state;
    T_U32               count;
    T_U32               time_num;
    T_U8                DriverNo;
}T_USB_HOST_PARM;

T_BOOL USBHost_InitialDisk(T_USB_HOST_PARM *pUsbHost_Parm);
T_BOOL USBHost_ConnInit(T_USB_HOST_PARM *pUsbHost_Parm);
T_eCSTM_T_ID USBHost_ConnTest(T_USB_HOST_PARM *pUsbHost_Parm);
T_VOID USBHost_ConnExit(T_eCSTM_T_ID state);

static T_USB_HOST_PARM *pUsbHost_Parm = AK_NULL;

void initusb_host(void)
{
    pUsbHost_Parm = (T_USB_HOST_PARM*)Fwl_Malloc(sizeof(T_USB_HOST_PARM));
    AK_ASSERT_PTR_VOID(pUsbHost_Parm, "initusb_host(): pUsbHost_Parm malloc error\n");

    if (AK_FALSE == gb.bUDISKAvail) // USB HOST mode, mount it
    {
//        gb.bUDISKAvail = AK_TRUE;

        pUsbHost_Parm->state = UNCONNECTED;
    }
    else        // if USB DISK(HOST MODE) has been mounted, POP-UP it !
    {
        pUsbHost_Parm->state = CONNECTED;
    }
    pUsbHost_Parm->time_num = 0;
    pUsbHost_Parm->count = 0;
}


void paintusb_host(void)
{
    if (0 != pUsbHost_Parm->msgbox.initFlag)
    {
        MsgBox_Show(&pUsbHost_Parm->msgbox);
        Fwl_RefreshDisplay();
    }
}


void exitusb_host(void)
{
    if ((pUsbHost_Parm->state != CONN_SUCCESS) && (AK_NULL == gb.driverUDISK))
    {
        gb.bUDISKAvail = AK_FALSE;
//        usb_enable_detect(1);

        USBHost_ConnExit(ctFAILURE);
    }

    Fwl_Free(pUsbHost_Parm);
    
    ScreenSaverEnable();
    if (USB_DEAL_BEGIN == Fwl_UsbGetDealProc()) //正常结束
	    Fwl_UsbSetDealProc(USB_DEAL_END);
	else if( USB_DEAL_ABORT == Fwl_UsbGetDealProc() )
	{
		Fwl_UsbDisk_Unload();
	}
}


unsigned char handleusb_host(T_EVT_CODE event, T_EVT_PARAM * pEventParm)
{
    //T_eBACK_STATE   msgRet;
    T_eKEY_ID       keyID;
    T_RECT              msgRect;
    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    if (event == M_EVT_PUB_TIMER)
    {
        T_eCSTM_T_ID state;
        
        Fwl_Print(C3, M_TOOL,"usb host timer. count=%d, state=%d  \n", pUsbHost_Parm->count, pUsbHost_Parm->state);
        if (CONNECTING == pUsbHost_Parm->state)
        {
            pUsbHost_Parm->count++;

            state = USBHost_ConnTest(pUsbHost_Parm);
            Fwl_Print(C3, M_TOOL,"USBHost_ConnTest. state=%d \n", state);
            if (ctNULL == state)
            {   
                if((pUsbHost_Parm->time_num)++ > 6)
                {                    
                    m_triggerEvent(M_EVT_USBHOST_EXIT, pEventParm); // exit, refresh tools menu
                    return 0;                    
                }
                //    coutinue loop        
                pUsbHost_Parm->state = CONNECTING;
                return 0;                               
            }
            else if (ctSUCCESS == state)
            {
                // use host ok, exit. 
                pUsbHost_Parm->state = CONN_SUCCESS;    
				Fwl_UsbSetDealProc(USB_DEAL_END);
                MsgBox_InitStr(&pUsbHost_Parm->msgbox, 3, Res_GetStringByID(eRES_STR_PUB_SUCCESS), \
                            Res_GetStringByID(eRES_STR_USBHOST_MOUNT_OK), MSGBOX_INFORMATION);
                MsgBox_Show(&pUsbHost_Parm->msgbox);
                Fwl_RefreshDisplay();
                AK_Sleep(300);
                m_triggerEvent(M_EVT_USBHOST_EXIT, pEventParm); // exit, refresh tools menu
                return 0;
            }
            else //ctFAILURE
            {
                pUsbHost_Parm->state = CONN_FAIL;       
                MsgBox_InitStr(&pUsbHost_Parm->msgbox, 3, Res_GetStringByID(eRES_STR_PUB_FAILURE), \
                            Res_GetStringByID(eRES_STR_USBHOST_MOUNT_ERROR), MSGBOX_INFORMATION);
                MsgBox_Show(&pUsbHost_Parm->msgbox);
                Fwl_RefreshDisplay();
                Fwl_MiniDelay(3000);
                m_triggerEvent(M_EVT_USBHOST_EXIT, pEventParm); // exit, refresh tools menu
                return 0;
            }
        }
    }
    else if (event == M_EVT_USER_KEY)
    {
        keyID = (T_eKEY_ID)pEventParm->c.Param1;
        Fwl_Print(C3, M_TOOL,"usb host USER_KEY: %d. \n", keyID);
        if (kbCLEAR == keyID)
        {
            if (CONNECTING == pUsbHost_Parm->state)
            {
                m_triggerEvent(M_EVT_EXIT, pEventParm);
                return 0;
            }
            else if (CONN_FAIL == pUsbHost_Parm->state)
            {
                m_triggerEvent(M_EVT_EXIT, pEventParm);
                return 0;
            }
        }
        else if (kbOK == keyID)
        {
            Fwl_Print(C3, M_TOOL,"In USER_KEY=OK state=%d\r\n", pUsbHost_Parm->state);
            if ((CONN_SUCCESS == pUsbHost_Parm->state) || \
                    (CONN_DISABLE == pUsbHost_Parm->state) || \
                    (CONN_FAIL == pUsbHost_Parm->state))
            {
                m_triggerEvent(M_EVT_USBHOST_EXIT, pEventParm); // exit, refresh tools menu
                return 0;
            }
        }
    }
    else
    {
        switch (pUsbHost_Parm->state)
        {
        case UNCONNECTED: //u 盘还未装载
            if (0)//(usb_is_connected())
            {
                Fwl_Print(C3, M_TOOL,"usb host:usb is connected\n");
                MsgBox_InitStr(&pUsbHost_Parm->msgbox, 3, Res_GetStringByID(eRES_STR_PUB_FAILURE), \
                            Res_GetStringByID(eRES_STR_USBHOST_MOUNT_ERROR), MSGBOX_INFORMATION);
                MsgBox_Show(&pUsbHost_Parm->msgbox);
                Fwl_RefreshDisplay();
                AK_Sleep(300);
                m_triggerEvent(M_EVT_EXIT, pEventParm);
                break;
            }
            
			if (!Fwl_UsbHostIsConnect()) //是usb弹出
			{
				Fwl_UsbDisk_Unload();
                m_triggerEvent(M_EVT_EXIT, pEventParm);
             }
			else			
            if (AK_FALSE == USBHost_ConnInit(pUsbHost_Parm))
            {
                m_triggerEvent(M_EVT_EXIT, pEventParm);
            }
            break;
            
        case CONNECTED://u盘已经装载
            pUsbHost_Parm->state = CONN_DISABLE;
			if (Fwl_UsbHostIsConnect()) 
			{
				//先卸载
				Fwl_UnMountUSBDisk();
				//再加载
	            pUsbHost_Parm->state = UNCONNECTED;
	            m_triggerEvent(M_EVT_USBHOST, pEventParm);
			}else
			{
	            MsgBox_InitStr(&pUsbHost_Parm->msgbox, 1, Res_GetStringByID(eRES_STR_PUB_HINT), \
	                            Res_GetStringByID(eRES_STR_USBHOST_POPUP), MSGBOX_INFORMATION);
	            MsgBox_SetDelay(&pUsbHost_Parm->msgbox, MSGBOX_DELAY_2);
	            MsgBox_Show(&pUsbHost_Parm->msgbox);
	            MsgBox_GetRect(&pUsbHost_Parm->msgbox, &msgRect);
	            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);

	            AudioPlayer_Stop();
	            Fwl_UsbDisk_Unload();
	            AK_Sleep(300);//for show 
	            m_triggerEvent(M_EVT_EXIT, pEventParm);
            }
            break;

        default:
            break;
        }

    }
    return 0;
}

T_BOOL USBHost_ConnInit(T_USB_HOST_PARM *pUsbHost_Parm)
{
    T_RECT              msgRect;
    T_BOOL              bRet;
    //The driver 3 is SD Card. And DriverNo 4 is USB Disk.
#if 0    
    pUsbHost_Parm->DriverNo = 4;
    
//    set_cpu_2x_asic(AK_TRUE);
    if (!Global_DriverAvail(pUsbHost_Parm->DriverNo))
    {
        return AK_FALSE;
    }
#endif

    AudioPlayer_Stop();
    ScreenSaverDisable();    
//    bRet = Fwl_Usb_ConnectDisk();

    MsgBox_InitStr(&pUsbHost_Parm->msgbox, 0, Res_GetStringByID(eRES_STR_PUB_HINT), \
            Res_GetStringByID(eRES_STR_USBHOST_CONNECTING), MSGBOX_INFORMATION);

    MsgBox_SetDelay(&pUsbHost_Parm->msgbox, MSGBOX_DELAY_2);
    MsgBox_Show(&pUsbHost_Parm->msgbox);
    MsgBox_GetRect(&pUsbHost_Parm->msgbox, &msgRect);

    Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);
    pUsbHost_Parm->state = CONNECTING;

    return bRet;
}

T_BOOL USBHost_InitialDisk(T_USB_HOST_PARM *pUsbHost_Parm)
{
    T_BOOL ret = AK_TRUE;
    //T_U16 j;
    T_RECT              msgRect;
    
    MsgBox_InitStr(&pUsbHost_Parm->msgbox, 0, Res_GetStringByID(eRES_STR_PUB_HINT), \
                        Res_GetStringByID(eRES_STR_USBHOST_INITIALING), MSGBOX_INFORMATION);
    MsgBox_Show(&pUsbHost_Parm->msgbox);
    
    MsgBox_SetDelay(&pUsbHost_Parm->msgbox, MSGBOX_DELAY_2);
    MsgBox_Show(&pUsbHost_Parm->msgbox);
    MsgBox_GetRect(&pUsbHost_Parm->msgbox, &msgRect);
    Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);
    
    return ret;
}


T_eCSTM_T_ID USBHost_ConnTest(T_USB_HOST_PARM *pUsbHost_Parm)
{
    T_BOOL ret =  AK_TRUE;
    
    if (AK_TRUE == Fwl_UsbHostIsConnect())
    {
        if (AK_TRUE == (ret = USBHost_InitialDisk(pUsbHost_Parm)))
        {
            pUsbHost_Parm->state = CONNECTED;

            //Fwl_MountUSBDisk will change gb.bUDISKAvail
            if (AK_TRUE == (ret = Fwl_MountUSBDisk(pUsbHost_Parm->DriverNo)))
            {
                return ctSUCCESS;
            }
        }

        return ctFAILURE;

    }
    
    return ctNULL;
}

T_VOID USBHost_ConnExit(T_eCSTM_T_ID state)
{
    if (ctSUCCESS != state)
    {
//        Fwl_Usb_DisconnectDisk();
//        set_cpu_2x_asic(AK_FALSE);
        //ScreenSaverEnable();
    }
}


#endif



#ifndef USB_HOST

#include "Fwl_public.h"

void initusb_host(void)
{
}

void paintusb_host(void)
{
}

void exitusb_host(void)
{
}

unsigned char handleusb_host(T_EVT_CODE event, T_EVT_PARAM * pEventParm)
{
    return 0;
}
#endif



