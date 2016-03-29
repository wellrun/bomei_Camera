/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.    
**************************************************************************/

#include "Lib_state.h"
#include "anyka_types.h"
#include "Fwl_public.h"
#include "ctl_msgbox.h"
#include "fwl_pfcamera.h"
#include "eng_gblstring.h"
#include "eng_font.h"
#include "Eng_AkBmp.h"
#include "Eng_ScreenSave.h"
#include "fwl_pfdisplay.h"
#include "eng_debug.h"
#include "Lib_state_api.h"
#include "fwl_usb.h"
#include "fwl_display.h"
#include "Eng_AutoPowerOff.h"
#include "eng_keymapping.h"
#include "Fwl_usb_host.h"

#ifdef SUPPORT_UVC
#ifdef OS_ANYKA

#include "log_uvc_cam.h"
#include "Fwl_sys_detect.h"


static T_HANDLE m_UVCCAMHandle;

typedef struct {
    T_hMSGBOX   pMsgBox;
} T_USB_CAMERA_PARM;

T_USB_CAMERA_PARM *pPcCamera = AK_NULL;

#endif
#endif

T_VOID UsbCam_UI(T_VOID)
{
#ifdef SUPPORT_UVC
#ifdef OS_ANYKA

    T_pCDATA    USBBackground = AK_NULL;
    T_U32       len;
    T_LEN       width,height;
    T_U8        deep;

    USBBackground = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_PC_CAMERA, &len);
    AKBmpGetInfo(USBBackground, &width, &height, &deep);

    Fwl_AkBmpDrawFromString(HRGB_LAYER, 0, 0, USBBackground, (T_COLOR*)AK_NULL, AK_FALSE);

    Fwl_RefreshDisplay();

#endif
#endif
}

void initusb_camera(void)
{
#ifdef SUPPORT_UVC
#ifdef OS_ANYKA
    TopBar_DisableShow();
	ScreenSaverDisable();
	AutoPowerOffDisable(FLAG_UVC);

	pPcCamera = (T_USB_CAMERA_PARM*)Fwl_Malloc(sizeof(T_USB_CAMERA_PARM));
	AK_ASSERT_PTR_VOID(pPcCamera, "initusb_camera(): pPcCamera malloc fail!\n");

	pPcCamera->pMsgBox = (T_MSGBOX*)Fwl_Malloc(sizeof(T_MSGBOX));
	AK_ASSERT_PTR_VOID(pPcCamera->pMsgBox, "initusb_camera(): pPcCamera->pMsgBox malloc fail!\n");

	m_UVCCAMHandle = 0;

#endif
#endif
}

void paintusb_camera(void)
{
	;
}

unsigned char handleusb_camera(T_EVT_CODE event, T_EVT_PARAM * pEventParm)
{
#ifdef SUPPORT_UVC
#ifdef OS_ANYKA

	T_UVCCAM_INI ini;
    T_MMI_KEYPAD phyKey;    
        
	if (IsPostProcessEvent(event)) 
		return 1;

	if (M_EVT_USB_CAMERA == event)
	{
		if(usb_is_connected())
		{
			Fwl_Usb_DisconnectDisk();

		//usb slave init
		    if(!sys_usb_detect())  //charger
		    {
		        return AK_FALSE;
		    }
		
			AK_Feed_Watchdog(0);		

			UVCCam_InitStruct(&ini);

			m_UVCCAMHandle = UVCCam_Open(&ini);
			
			if (AK_FALSE == m_UVCCAMHandle)
			{
				MsgBox_InitStr(pPcCamera->pMsgBox, 2, GetCustomTitle(ctERROR), GetCustomString(csCAMERA_INIT_FAILED), MSGBOX_INFORMATION);
				MsgBox_SetDelay(pPcCamera->pMsgBox, 3);
				m_triggerEvent(M_EVT_MESSAGE, (vT_EvtParam *)pPcCamera->pMsgBox);
				return 0;
			}

            UsbCam_UI();
		}
		else
		{			
			Fwl_Print(C2, M_TOOL, "usb cable connect error.\n");

			MsgBox_InitStr(pPcCamera->pMsgBox, 2, GetCustomTitle(ctERROR), GetCustomString(csPCCAMERA_ENTRY), MSGBOX_INFORMATION);
			MsgBox_SetDelay(pPcCamera->pMsgBox, 3);
			m_triggerEvent( M_EVT_MESSAGE, ( vT_EvtParam* )pPcCamera->pMsgBox );
		}
	}
    else if (M_EVT_USER_KEY == event)
    { 
        phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL)pEventParm->c.Param2;

        if (kbCLEAR == phyKey.keyID && PRESS_SHORT == phyKey.pressType)
        {            
            UVCCam_UserExit(AK_TRUE);
        }        
    }
#endif
#endif

	return 0;
}

void exitusb_camera(void)
{
#ifdef SUPPORT_UVC
#ifdef OS_ANYKA    
    UVCCam_UserExit(AK_TRUE);//让PC Camera逻辑层退出while循环
	UVCCam_Close(m_UVCCAMHandle);
    UVCCam_UserExit(AK_FALSE);//变量复位

	Fwl_Usb_ConnectDisk();
    
	if (AK_NULL != pPcCamera->pMsgBox)
	{
		Fwl_Free(pPcCamera->pMsgBox);
		pPcCamera->pMsgBox = AK_NULL;
	}
	
	if (AK_NULL != pPcCamera)
	{
		Fwl_Free(pPcCamera);
		pPcCamera = AK_NULL;
	}

	ScreenSaverEnable();
	AutoPowerOffEnable(FLAG_UVC);
    TopBar_EnableShow();

	Fwl_UsbSetDealProc(USB_DEAL_END);
    
#endif
#endif
}


