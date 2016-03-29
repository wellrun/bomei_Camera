/**************************************************************************
*
* Copyrights (C) 2002, ANYKA software Inc
* All rights reserced.
*
* File name: AKMMIApp.c
* Function: 
* Author:  
* Date:  
* Version: 1.0
*
***************************************************************************/
#include "fwl_vme.h"
#include "AKMMIApp.h"
#include "eng_debug.h"
#include "fwl_evtmailbox.h"
#include "AKMsgDispatch.h"
#include "AKAppMgr.h"

#include "Fwl_public.h"
#include "Eng_KeyTranslate.h"

extern T_BOOL Call_NeedAnswer( T_pCWSTR telenum );
extern unsigned char handle_pub_timer(vT_EvtCode* event, vT_EvtParam* pEventParm);
extern void m_mainloop(vT_EvtCode Event, vT_EvtParam *pParam);
//#################################################################################

//===================================================================================
//              <---- Function Declaration ---->
//===================================================================================
static T_S32 CMMI_ICBThread_Prepare(ICBThread *pICBThread);
static T_S32 CMMI_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam);
static T_S32 CMMI_ICBThread_Free(ICBThread *pICBThread);
//////////////////////////////////////////////////////////////////////////////////////
static const AK_VTABLE(ICBThread) g_ICBMMIFuncs =
{
    CMMI_ICBThread_Prepare,
    CMMI_ICBThread_Handle,
    CMMI_ICBThread_Free
};

/*===================================================================================*/
static __inline vT_EvtCode CMMI_TranslateEvt(T_SYS_EVTID eEvent)
{
     vT_EvtCode eEvt = (vT_EvtCode)eEvent;
     
     switch (eEvent)
     {
        case SYS_EVT_TIMER:
            eEvt = VME_EVT_TIMER;
            break;
        case SYS_EVT_TSCR:
            eEvt = M_EVT_TOUCH_SCREEN/*VME_EVT_TOUCHSCR_ACTION*/;
            break;
        case SYS_EVT_MEDIA:
            eEvt = VME_EVT_MEDIA;
            break;
        case SYS_EVT_SDCB_MESSAGE:
            eEvt = M_EVT_SDCB_MESSAGE;
            break;
        case SYS_EVT_Z05COM_MSG:
            eEvt = M_EVT_Z05COM_MSG;
            break;
        case SYS_EVT_PUB_TIMER:
            eEvt = M_EVT_PUB_TIMER;
            break;
        case SYS_EVT_PINIO:
            eEvt = VME_EVT_PINIO_FLIP;
            break;
        case SYS_EVT_SD_PLUG:
            eEvt = M_EVT_SDCARD_PLUG;
            break;
        case SYS_EVT_USB_PLUG:
            eEvt = VME_EVT_USB_DEVICE_PLUG;
            break;
        case SYS_EVT_USB_SEND_REQUEST:
            eEvt = VME_EVT_USB_SEND_REQUEST;
            break;
        case SYS_EVT_USER_KEY:
            eEvt = M_EVT_USER_KEY;
            break;
        case SYS_EVT_SUBTHREAD_NOTIFY:
            eEvt = VME_EVT_SUBTHREAD_NOTIFY;
            break;
        case SYS_EVT_SUBTHREAD_FINISH:
            eEvt = VME_EVT_SUBTHREAD_FINISH;
            break;
        case SYS_EVT_AUDIO_ABPLAY:
            eEvt = M_EVT_AUDIO_ABPLAY;
            break;
        case SYS_EVT_RTC:
            eEvt = M_EVT_RTC;
            break;
        case SYS_EVT_PRE_EXIT:
            eEvt = M_EVT_PRE_EXIT;
            break;
        case SYS_EVT_PASTE_EXIT:
            eEvt = M_EVT_PASTE_EXIT;
            break;
		case SYS_EVT_SDIO_PLUG:
			eEvt = M_EVT_SDIO_DETECT;
            break;
		case SYS_EVT_SDMMC_PLUG:
			eEvt = M_EVT_SDMMC_DETECT;
            break;
		case SYS_EVT_USB_DETECT:
			eEvt = M_EVT_USB_DETECT;
//			eEvt = SYS_EVT_XXX_MSK_END;//for test
            break;
        default:
            break;
     }

     return eEvt;
}


static T_S32 CMMI_Constructor(CMMI *pMe)
{
    T_S32 lRet = AK_SUCCESS;
    T_AppInfo *pAppInfo = AK_NULL;
    T_APP_INITPARAM sInitParam;

    pAppInfo = AK_GetDefaultAppInfo(AKAPP_CLSID_MMI);
    if (AK_NULL == pAppInfo)
    {
        return AK_EUNSUPPORT;
    }
    sInitParam.base = pAppInfo->sInitparam;
    sInitParam.wnd.dispbuf      = AK_NULL;
    sInitParam.wnd.height       = 320;//Fwl_GetLcdHeight();
    sInitParam.wnd.width        = 240;//Fwl_GetLcdWidth();
    sInitParam.wnd.type         = AK_DISP_RGB;

    Fwl_Print(C3, M_AKFRAME, "---I'm in CMMI_Constructor");
    lRet = CApp_New(&pMe->m_pIBase, &sInitParam);
    Fwl_Print(C3, M_AKFRAME, "---I'm in CMMI_Constructor lRet=%d,stacksize=%d.",lRet,sInitParam.base.ulStackSize);
    
    //Simple Constructor, Add your code here...
    lRet |= IAkApp_Register(pMe->m_pIBase,
                            &pMe->m_ICBThread);
    
    return lRet;
}

static T_S32 CMMI_Destructor(CMMI *pMe)
{
    //Add your code here...
    AK_RELEASEIF(pMe->m_pIBase);
    
    Fwl_Free(pMe);
    
    return AK_SUCCESS;
}

static T_S32 CMMI_ICBThread_Prepare(ICBThread *pICBThread)
{
    //CMMI *pMe = (CMMI *)pICBThread->pData;
    
    Fwl_Print(C3, M_AKFRAME, "---I'm in CMMI_ICBThread_Prepare.");
#ifdef OS_ANYKA
    CMain(0, AK_NULL);
#endif
    return AK_SUCCESS;
}

static T_S32 CMMI_ICBThread_Handle(ICBThread *pICBThread, T_SYS_EVTID eEvent, T_SYS_PARAM *pEvtParam)
{
    CMMI *pMe = (CMMI *)pICBThread->pData;

    //AK_DEBUG_OUTPUT("***********eEvent = 0x%x************\n", eEvent);
    
    //Fwl_Print(C3, M_AKFRAME, "---I'm in CMMI_ICBThread_Handle.eEvent=0x%x.\r\n", eEvent);
    if (eEvent == SYS_EVT_PUB_TIMER)
    {
        vT_EvtCode event = CMMI_TranslateEvt(eEvent);
       
        //handle_pub_timer(&event, (vT_EvtParam*)pEvtParam);
        if (IAppMgr_GetActiveApp(AK_GetAppMgr()) == (IThread*)&pMe->m_myIApp)
        {
            m_mainloop((vT_EvtCode)event, (vT_EvtParam*)pEvtParam);
        }

        return AK_SUCCESS;
    }
    
	if (eEvent == SYS_EVT_USER_KEY)
	{
		T_MMI_KEYPAD keyPad;
		
        keyPad.keyID = pEvtParam->c.Param1;
        keyPad.pressType = pEvtParam->c.Param2;
        Eng_KeyTranslate(&keyPad);
        pEvtParam->c.Param1 = (T_U8)keyPad.keyID;
        pEvtParam->c.Param2 = (T_U8)keyPad.pressType;

#ifdef PLATFORM_DEBUG_VER
		if ((keyPad.keyID == kbUP) && (keyPad.pressType == AK_TRUE))
		{
			AK_List_Task();
		}
#endif
	}

    m_mainloop(CMMI_TranslateEvt(eEvent), (vT_EvtParam*)pEvtParam);

    return AK_SUCCESS;
}

static T_S32 CMMI_ICBThread_Free(ICBThread *pICBThread)
{
    CMMI *pMe = (CMMI *)pICBThread->pData;
    
    Fwl_Print(C3, M_AKFRAME, "---I'm in CMMI_ICBThread_Free.");
    CMMI_Destructor(pMe);
    
    return AK_SUCCESS;
}

T_S32 	CMMI_New(IApp **ppo)
{
    T_S32 nErr = AK_SUCCESS;
    CMMI *pNew = AK_NULL;
    
    if (AK_NULL == ppo)
    {
        return AK_EBADPARAM;
    }
    
    *ppo = AK_NULL;
    
    do 
    {
        pNew = AK_MALLOCRECORD(CMMI);
        AK_BREAKIF(AK_NULL == pNew, nErr, AK_ENOMEMORY);
        
        pNew->m_myIApp.pvt   = AK_NULL;
        pNew->m_myIApp.pData = (T_VOID*)pNew;
        AK_SETVT(&(pNew->m_ICBThread), &g_ICBMMIFuncs);
        pNew->m_ICBThread.pData = (T_VOID*)pNew;
        
        nErr = CMMI_Constructor(pNew);
        if (AK_IS_FAILURE(nErr))
        {
            IAkApp_Register(pNew->m_pIBase,AK_NULL);
            CMMI_Destructor(pNew);
            pNew = AK_NULL;
            break;
        }
        
        *ppo = (IApp*)&pNew->m_myIApp;
        
    } while(AK_FALSE);
    
    return nErr;
}

IThread* CMMI_GetThread(T_VOID)
{
	return IAppMgr_GetApp(AK_GetAppMgr(), AKAPP_CLSID_MMI);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                      <----------               File      End        ----------->
//////////////////////////////////////////////////////////////////////////////////////////////////////////

