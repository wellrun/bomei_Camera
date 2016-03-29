/**
@author zhou shangpin
@date   2005.7.5
@file   s_usb_disp.c
@brief  convert usb status
*/

#include "Fwl_public.h"
#include "Fwl_Initialize.h"
#include "Ctl_Msgbox.h"
#include "fwl_usb.h"
#include "Fwl_pfAudio.h"
#include "Fwl_osFS.h"
#include "Ctl_AudioPlayer.h"
#include "Ctl_AVIPlayer.h"
#include "Eng_TopBar.h"
#include "Eng_DynamicFont.h"
#include "Lib_state.h"
#include "Eng_DynamicFont.h"
#include "Eng_AkBmp.h"
#include "Eng_DataConvert.h"
#include "Ctl_Fm.h"
#include "Eng_font.h"
#include "fwl_osfs.h"
#include "Fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "fwl_pfKeypad.h"
#include "Fwl_sys_detect.h"
#include "fwl_oscom.h"
#include "Fwl_usb_host.h"

#define  USB_MOUNT_DRV_TIME  (5*1000) //5s

typedef enum
{
    eUSB_STATE_NULL=0,
    eUSB_STATE_IN,
    eUSB_STATE_MOUNT,
    eUSB_STATE_EXIT,
}E_USB_STATE;

typedef struct
{
    T_MSGBOX    msgbox;
    E_USB_STATE UsbState;
    T_TIMER        timer_id;
    T_BOOL      UpDateMediaList; //is need Update Media List
    T_BOOL      KeyLockState;
}T_PUB_USB_PARAM;

#ifdef OS_ANYKA
extern T_VOID Reset_MMIThread_Queue(T_VOID); // in file : fwl_multitask.c
#endif
extern T_VOID UserCountDownReset(T_VOID);

static T_PUB_USB_PARAM *pPubUsbParam;

static T_BOOL topBarStatusOfPreSM;

static T_VOID usb_interface_display(T_VOID);
static T_VOID update_media_list(T_VOID);
static T_BOOL usb_mount_disk(T_VOID);
static T_VOID usb_UnMount_disk(T_VOID);


//*****************************start to state s_pub_usb ******************************//

void initpub_usb(void)
{
    pPubUsbParam = (T_PUB_USB_PARAM *)Fwl_Malloc(sizeof(T_PUB_USB_PARAM));
    AK_ASSERT_PTR_VOID(pPubUsbParam, "initpub_usb(): malloc error");

    pPubUsbParam->UsbState = eUSB_STATE_NULL;
    pPubUsbParam->timer_id = ERROR_TIMER;
    pPubUsbParam->UpDateMediaList = AK_FALSE;
    //if the key is locked ,when usb pop in clear the locked status
    pPubUsbParam->KeyLockState = gb.KeyLocked;
    gb.KeyLocked = AK_FALSE;

	topBarStatusOfPreSM = TopBar_IsEnableShow();
    TopBar_DisableShow();
    m_regResumeFunc(TopBar_DisableShow);

    /**stop audio when enter usbdisk*/
    TopBar_Show(TB_REFRESH_AUDIO_STATUS);
    
#ifdef OS_ANYKA
    // Clear Queue Event,在进入U盘状态，不再受其他模块残留的影响。
    Reset_MMIThread_Queue();
#endif
}

void exitpub_usb(void)
{
    if (ERROR_TIMER != pPubUsbParam->timer_id)
    {
        Fwl_StopTimer(pPubUsbParam->timer_id);
        pPubUsbParam->timer_id = ERROR_TIMER;    
    }
    //当有其他消息触发退出，这里stop USB-DISK
    if (eUSB_STATE_MOUNT == pPubUsbParam->UsbState)
    {
        usb_UnMount_disk();
    }
    //还原锁键盘状态
    gb.KeyLocked = pPubUsbParam->KeyLockState;
        
    if (pPubUsbParam != AK_NULL)
    {
        Fwl_Free(pPubUsbParam);
        pPubUsbParam = AK_NULL;
    }

    gb.InUsbMessage = AK_FALSE;
   
   if (topBarStatusOfPreSM)
	   TopBar_EnableShow();
   else
	   TopBar_DisableShow();
	Fwl_UsbSetDealProc(USB_DEAL_END);
}

void paintpub_usb(void)
{
    if ((eUSB_STATE_MOUNT != pPubUsbParam->UsbState) \
         &&(eUSB_STATE_NULL != pPubUsbParam->UsbState))
    {
        MsgBox_Show(&pPubUsbParam->msgbox);
        Fwl_RefreshDisplay();
    }
}

unsigned char handlepub_usb(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_eBACK_STATE menuRet;
    
    switch (event)
    {
        case M_EVT_USB_IN:
            //start timer,if no key operation in 5 sec,will Mount disk self-motion.
            if (ERROR_TIMER == pPubUsbParam->timer_id)
            {
                pPubUsbParam->timer_id = Fwl_SetTimerMilliSecond(USB_MOUNT_DRV_TIME, AK_FALSE);
            }
            
            MsgBox_InitAfx(&pPubUsbParam->msgbox, 0, ctHINT, csUSB_MOUNT_SHOW, MSGBOX_QUESTION | MSGBOX_YESNO);

            pPubUsbParam->UsbState = eUSB_STATE_IN;
            
            break;
        case M_EVT_USB_OUT:
            if (eUSB_STATE_MOUNT == pPubUsbParam->UsbState) //had mount,need update media list
            {
                pPubUsbParam->UpDateMediaList = AK_TRUE;
                MsgBox_InitAfx(&pPubUsbParam->msgbox, 0, ctHINT, csREFRESH_MEDIA_LIST,\
                                MSGBOX_QUESTION | MSGBOX_YESNO);
            }
            else //had no mount disk,and exit no any operation
            {
                m_triggerEvent(M_EVT_USB_EXIT, pEventParm);
            }
            
            usb_UnMount_disk();
            
            pPubUsbParam->UsbState = eUSB_STATE_EXIT;
            break;
        case VME_EVT_TIMER:
			if (pEventParm->w.Param1 == (T_U32)pPubUsbParam->timer_id)
			{
	            if (eUSB_STATE_IN == pPubUsbParam->UsbState)
	            {
	                if (usb_mount_disk())
	                {
	                    pPubUsbParam->UsbState = eUSB_STATE_MOUNT;
	                }
	                else //U-Disk Mount fail,exit
	                {
	                    usb_UnMount_disk();
	                    pPubUsbParam->UsbState = eUSB_STATE_EXIT;                    
	                    MsgBox_InitAfx(&pPubUsbParam->msgbox, 2, ctHINT, \
	                                   csUSB_MOUNT_FAIL, MSGBOX_INFORMATION);
	                    MsgBox_SetDelay(&pPubUsbParam->msgbox, MSGBOX_DELAY_1); 
	                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pPubUsbParam->msgbox);
	                }
	            }
			}
            break;

        default:
            break;

        return 0;    
    }

    if (eUSB_STATE_MOUNT != pPubUsbParam->UsbState)
    {
        menuRet = MsgBox_Handler(&pPubUsbParam->msgbox, event, pEventParm);
        switch(menuRet)
        {
            case eNext:
                if (eUSB_STATE_IN == pPubUsbParam->UsbState)
                {
                    if (usb_mount_disk())
                    {
                        pPubUsbParam->UsbState = eUSB_STATE_MOUNT;
                    }
                    else //U-Disk Mount fail,exit
                    {
                        usb_UnMount_disk();
                        pPubUsbParam->UsbState = eUSB_STATE_EXIT;
                        MsgBox_InitAfx(&pPubUsbParam->msgbox, 2, ctHINT, \
                                        csUSB_MOUNT_FAIL, MSGBOX_INFORMATION);
                        MsgBox_SetDelay(&pPubUsbParam->msgbox, MSGBOX_DELAY_1); 
                        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pPubUsbParam->msgbox);
                    }
                }
                else if (AK_TRUE == pPubUsbParam->UpDateMediaList)//had mount disk,is need to updtae
                {
                    MsgBox_InitStr(&pPubUsbParam->msgbox, 0, GetCustomTitle(ctHINT), \
                                    GetCustomString(csWAITING), MSGBOX_INFORMATION);
                    MsgBox_Show(&pPubUsbParam->msgbox);
                    Fwl_RefreshDisplay();
                    
                    AK_DEBUG_OUTPUT("Update Media List ...\n");
                    
                    //save song infomation in m_baseSet to file:B:/system/metaInfo.dat
                    update_media_list();            
                    
                    MsgBox_InitStr(&pPubUsbParam->msgbox, 2, GetCustomTitle(ctHINT), \
                                    Res_GetStringByID(eRES_STR_COMMAND_SENT), MSGBOX_INFORMATION);
                    MsgBox_SetDelay(&pPubUsbParam->msgbox, MSGBOX_DELAY_1); 
                    
                    m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pPubUsbParam->msgbox);
                }
                break;
                
            case eReturn:
                pPubUsbParam->UsbState = eUSB_STATE_EXIT;
                m_triggerEvent(M_EVT_USB_EXIT, pEventParm);
                break;
                
            default:
                ReturnDefauleProc(menuRet, pEventParm);
                break;
        }
    }

    return 0;
}

static T_BOOL usb_mount_disk(T_VOID)
{
    T_BOOL ret = AK_FALSE;

	Fwl_Usb_DisconnectDisk();

//usb slave init
    if(!sys_usb_detect())  //charger
    {
        return AK_FALSE;
    }
    
#ifdef OS_ANYKA
    sd_enable_detect(AK_FALSE);
    mmc_enable_detect(AK_FALSE);
#endif

    //stop all Current Access to File System
    MList_SuspendAll();
    AudioPlayer_Stop();
    DynamicFont_Codepage_Free();

    usb_interface_display();
        
#ifdef OS_ANYKA
    gb.InUsbMessage = AK_TRUE;
    
    if(usb_is_connected())
    {    
        ret = Fwl_UsbMountDisk();
    }
#endif    
    return ret;
}

static T_VOID usb_UnMount_disk(T_VOID)
{
    //mest be pledge usb mount disk is successful
    if (eUSB_STATE_MOUNT == pPubUsbParam->UsbState)
    {
        Fwl_UsbDiskStop();
    }
    
#ifdef OS_ANYKA
    UserCountDownReset();
#endif

    Fwl_CreateDefPath();
    Eng_Codepage_Init();

#ifdef OS_ANYKA
    gb.InUsbMessage = AK_FALSE;
#endif

#ifdef OS_ANYKA
    sd_enable_detect(AK_TRUE);
    mmc_enable_detect(AK_TRUE);
#endif

#ifdef OS_ANYKA
    // Clear Queue Event
    Reset_MMIThread_Queue();
#endif
	Fwl_Usb_ConnectDisk();
}

/************************* end of state s_pub_usb******************************* */
static T_VOID usb_interface_display(T_VOID)
{
    T_pCDATA    USBBackground = AK_NULL;
    T_U32       len;
    T_LEN       width,height;
    T_U8        deep;

    USBBackground = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_USB_DISK, &len);
    AKBmpGetInfo(USBBackground, &width, &height, &deep);
    Fwl_AkBmpDrawFromString(HRGB_LAYER, 0, 0, USBBackground, (T_COLOR*)AK_NULL, AK_FALSE);

    Fwl_RefreshDisplay();
}

static T_VOID update_media_list(T_VOID)
{
    APList_Update();
#ifdef SUPPORT_VIDEOPLAYER
    AVIPlayer_UpdateVideoList();
#endif
}


