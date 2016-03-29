
#include "Fwl_public.h"

#ifdef SUPPORT_IMG_BROWSE
#include "Ctl_ImgBrowse.h"
#include "Lib_state.h"
#include "Ctl_MsgBox.h"
#include "eng_topbar.h"
#include "Lib_state.h"
#include "Fwl_osMalloc.h"
#include "Fwl_Initialize.h"
#include "Fwl_pfaudio.h"
#include "Lib_state_api.h"
#include "Fwl_pfdisplay.h"
#include "fwl_display.h"
#include "eng_font.h"
#include "eng_screensave.h"
#include "Lib_geshade.h"



typedef struct {
    T_DISPLAYLIST           *pDisplayList;
    T_IMGBROWSE             ImgBrowse;
    T_MSGBOX                msgbox;
	T_BOOL					bZ05COM_MSG;
	T_U32					mode;
	T_hSemaphore			semaphore;
	T_BOOL					bShowErrStr;
}T_IMG_BROWSER_PARM;


static T_IMG_BROWSER_PARM *pImg_Browser_Parm = AK_NULL;

void suspendimg_view(void);
void resumeimg_view(void);
#endif
/*---------------------- BEGIN OF STATE s_img_browser ------------------------*/
void initimg_browser(void)
{
#ifdef SUPPORT_IMG_BROWSE
    TopBar_DisableShow();
    Fwl_SetAudioVolumeStatus(AK_FALSE);
	Menu_FreeRes();

    pImg_Browser_Parm = (T_IMG_BROWSER_PARM *)Fwl_Malloc(sizeof(T_IMG_BROWSER_PARM));
    AK_ASSERT_PTR_VOID(pImg_Browser_Parm, "initimg_browser(): malloc error");
    ImgBrowse_Init(&pImg_Browser_Parm->ImgBrowse);

    pImg_Browser_Parm->bZ05COM_MSG = AK_FALSE;
	pImg_Browser_Parm->bShowErrStr = AK_FALSE;
	pImg_Browser_Parm->semaphore = AK_Create_Semaphore(1, AK_PRIORITY);
	
    m_regSuspendFunc(suspendimg_view);
    m_regResumeFunc(resumeimg_view);
    Fwl_SetAudioVolumeStatus(AK_FALSE);

    /**start waitbox*/
    WaitBox_Start(WAITBOX_CLOCK, (T_pWSTR)GetCustomString(csLOADING));
    ScreenSaverDisable();
#endif
}

void exitimg_browser(void)
{
#ifdef SUPPORT_IMG_BROWSE

    // save default image path
    Fwl_SetDefPath(eIMAGE_PATH, DisplayList_GetCurFilePath(pImg_Browser_Parm->pDisplayList));

    TopBar_EnableShow();

    Fwl_SetAudioVolumeStatus(AK_TRUE);
    suspendimg_view();
    ImgBrowse_Free(&pImg_Browser_Parm->ImgBrowse);

    /**kill waitbox*/
    WaitBox_Stop();

	AK_Delete_Semaphore(pImg_Browser_Parm->semaphore);

    pImg_Browser_Parm = Fwl_Free(pImg_Browser_Parm);

    Fwl_SetAudioVolumeStatus(AK_TRUE);
/*
	if(Fwl_TvoutIsOpen())
	{
		Fwl_CleanFrameBuf();	
	}*/
	
	ScreenSaverEnable();
	
	Menu_LoadRes();
#endif
}

void paintimg_browser(void)
{
#ifdef SUPPORT_IMG_BROWSE

    if (ImgBrowse_GetRefresh(&pImg_Browser_Parm->ImgBrowse) != IMG_BROWSE_REFRESH_NONE)
    {
		if (IMG_SLIDE == pImg_Browser_Parm->mode)
		{
			WaitBox_Stop();
        	ImgBrowse_Show(&pImg_Browser_Parm->ImgBrowse);
			pImg_Browser_Parm->bShowErrStr = AK_FALSE;
		}
		
		/*if(Fwl_TvoutIsOpen()&&(IMG_SLIDE != pImg_Browser_Parm->mode))//幻灯片浏览时前端没有做缩放，不走TVOUT快速通道
		{  
			if(pImg_Browser_Parm->ImgBrowse.DecTimerId == ERROR_TIMER)
			{		
				//kill waitbox				
				WaitBox_Stop();
				Fwl_RefreshDisplayTVOUT_Fast();
			    //Fwl_CleanScreen(COLOR_BLACK);
			    	return;
			}		
		}*/
        /*start GE_Shade when the button is not showed*/
        if (IMG_SLIDE == pImg_Browser_Parm->mode)
        {
            GE_StartShade();
			Fwl_RefreshDisplay();
		}
		else
		{
			if (pImg_Browser_Parm->ImgBrowse.largeFlag)
			{
				if (pImg_Browser_Parm->ImgBrowse.DecTimerId == ERROR_TIMER)
				{
					/**kill waitbox*/
					WaitBox_Stop();
					ImgBrowse_StartImgShowFastTimer(&pImg_Browser_Parm->ImgBrowse);					//if(!Fwl_TvoutIsOpen())
					ScreenSaverEnable();
				}
			}
			else
			{
				/**kill waitbox*/
				WaitBox_Stop();
				ImgBrowse_StartImgShowFastTimer(&pImg_Browser_Parm->ImgBrowse);				//if(!Fwl_TvoutIsOpen())
				ScreenSaverEnable();
			}
		}
    }
	else
	{
		if ((IMG_SLIDE == pImg_Browser_Parm->mode) && pImg_Browser_Parm->bShowErrStr)
		{
			T_USTR_FILE Utmpstr;
			T_U32 width = 0;

			WaitBox_Stop();
			
			Fwl_CleanScreen(COLOR_BLACK);
		
			Utl_UStrCpy(Utmpstr, Res_GetStringByID(eRES_STR_COM_FILE_INVALID));	
			width = UGetSpeciStringWidth(Utmpstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));
			Fwl_UDispSpeciString(HRGB_LAYER,
				(T_POS)((Fwl_GetLcdWidth() - width)/2), (T_POS)(Fwl_GetLcdHeight()/2), Utmpstr, COLOR_WHITE, 
				CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Utmpstr));

			Fwl_RefreshDisplay();
		}
	}
#endif
}

unsigned char handleimg_browser(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_IMG_BROWSE

    T_S32 ret;
    T_FILE_INFO  *pFileInfo;

    if (IsPostProcessEvent(event))
    {
    	if (M_EVT_Z05COM_MSG == event)
    	{
			pImg_Browser_Parm->bZ05COM_MSG = AK_TRUE;
		}
		
        ImgBrowse_SetRefresh(&pImg_Browser_Parm->ImgBrowse, IMG_BROWSE_REFRESH_ALL);
        return 1;
    }

	if (pImg_Browser_Parm->ImgBrowse.FailReason)
	{
		if (IMG_NO_ENOUGH_MOMORY == pImg_Browser_Parm->ImgBrowse.FailReason)
		{
			MsgBox_InitAfx(&pImg_Browser_Parm->msgbox, 2, ctHINT, csMEM_NOT_ENOUGH, MSGBOX_INFORMATION);
		}
		else if (IMG_NOT_SUPPORT_SIZE == pImg_Browser_Parm->ImgBrowse.FailReason)
		{
			MsgBox_InitAfx(&pImg_Browser_Parm->msgbox, 2, ctHINT, csNOT_SUPT_SIZE, MSGBOX_INFORMATION);
		}
		else
		{
			MsgBox_InitAfx(&pImg_Browser_Parm->msgbox, 2, ctHINT, csFILE_INVALID, MSGBOX_INFORMATION);
		}
		
        MsgBox_SetDelay(&pImg_Browser_Parm->msgbox, MSGBOX_DELAY_1);
        m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pImg_Browser_Parm->msgbox);
		return 0;
	}
	

 	if (M_EVT_USER_KEY == event)
	{
		if ((kbSWX == pEventParm->c.Param1) || (Fwl_TvoutIsOpen() && (kbSWY == pEventParm->c.Param1)))
		{	
			ImgBrowse_SetRefresh(&pImg_Browser_Parm->ImgBrowse, IMG_BROWSE_REFRESH_ALL);
		//	Fwl_CleanFrameBuf();
			return 0;
		}
 	}

    if (M_EVT_1 == event || M_EVT_3 == event)
    {
        AK_ASSERT_PTR(pEventParm, "handleimg_browser(): pEventParm", 0);
        pImg_Browser_Parm->pDisplayList = (T_DISPLAYLIST *)pEventParm;

		if (M_EVT_1 == event)
		{
			pImg_Browser_Parm->mode = IMG_SLIDE;
			ImgBrowse_SetDisMode(&pImg_Browser_Parm->ImgBrowse, IMG_SLIDE);
		}
		else
		{
			pImg_Browser_Parm->mode = IMG_BROWSE;
			ImgBrowse_SetDisMode(&pImg_Browser_Parm->ImgBrowse, IMG_BROWSE);
		}

        if (ImgBrowse_Open(&pImg_Browser_Parm->ImgBrowse, pImg_Browser_Parm->pDisplayList) == AK_FALSE)
        {
            // not support image
            if (IMG_SLIDE == pImg_Browser_Parm->mode)
            {
	            pImg_Browser_Parm->bShowErrStr = AK_TRUE;
            }
			else
			{
				MsgBox_InitAfx(&pImg_Browser_Parm->msgbox, 2, ctHINT, csFILE_INVALID, MSGBOX_INFORMATION);
	            MsgBox_SetDelay(&pImg_Browser_Parm->msgbox, MSGBOX_DELAY_1);
	            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pImg_Browser_Parm->msgbox);
	            return 0;
			}
        }
        else
        {
        	pImg_Browser_Parm->bShowErrStr = AK_FALSE;
            //get the info of the file 
            pFileInfo= DisplayList_GetItemContentFocus(pImg_Browser_Parm->pDisplayList);
            pImg_Browser_Parm->ImgBrowse.fsFileInfo.time_write = pFileInfo->time_write;
        }

        resumeimg_view();
    }

	AK_Obtain_Semaphore(pImg_Browser_Parm->semaphore, AK_SUSPEND);
    ret = ImgBrowse_Handle(&pImg_Browser_Parm->ImgBrowse, event, pEventParm);
	AK_Release_Semaphore(pImg_Browser_Parm->semaphore);

    switch(ret)
    {
        case IMG_RETURN:
            m_triggerEvent(M_EVT_EXIT, pEventParm);
            break;

        case IMG_RETURN_HOME:
            m_triggerEvent(M_EVT_Z09COM_SYS_RESET, pEventParm);
            break;
#if 0	// (SDRAM_MODE >= 16)
        case IMG_MENU:
            pFileInfo = DisplayList_GetItemContentFocus(pImg_Browser_Parm->pDisplayList);
            if (pFileInfo != AK_NULL)
            {
                pEventParm->p.pParam1 = (T_pVOID)pImg_Browser_Parm->pDisplayList;

				if (AK_TRUE == pImg_Browser_Parm->ImgBrowse.bSupportPic)
				{
                	pEventParm->p.pParam2 = (T_pVOID)ImgBrowse_GetOutBuf(&pImg_Browser_Parm->ImgBrowse);
				}
				else
				{
					pEventParm->p.pParam2 = AK_NULL;
				}
                m_triggerEvent(M_EVT_NEXT, pEventParm);
                ImgBrowse_SetRefresh(&pImg_Browser_Parm->ImgBrowse, IMG_BROWSE_REFRESH_ALL);
                Fwl_ShadeEnable(REFRESHSHADE_NUM);
            }
            break;
#endif
        case IMG_OPEN_ERROR:
        case IMG_MULTIMETHODSHOW_ERROR:
			if (IMG_SLIDE == pImg_Browser_Parm->mode)
            {
	            pImg_Browser_Parm->bShowErrStr = AK_TRUE;
            }
			else
			{
				MsgBox_InitAfx(&pImg_Browser_Parm->msgbox, 2, ctHINT, csFILE_INVALID, MSGBOX_INFORMATION);
	            MsgBox_SetDelay(&pImg_Browser_Parm->msgbox, MSGBOX_DELAY_1);
	            m_triggerEvent(M_EVT_MESSAGE, (T_EVT_PARAM *)&pImg_Browser_Parm->msgbox);
			}
            break;

        default:
            break;
    }
#endif
    return 0;
}

#ifdef SUPPORT_IMG_BROWSE

void suspendimg_view(void)
{
    /**kill waitbox*/
    WaitBox_Stop();
	AK_Obtain_Semaphore(pImg_Browser_Parm->semaphore, AK_SUSPEND);

	if (IMG_BROWSE == pImg_Browser_Parm->mode)
	{
    	ImgBrowse_StopImgShowFastTimer(&pImg_Browser_Parm->ImgBrowse);
	}

	if (!pImg_Browser_Parm->bZ05COM_MSG)
	{
    	Fwl_SetAudioVolumeStatus(AK_TRUE);
	}
	
    if (gs.ImgSlideInterval && (IMG_SLIDE == pImg_Browser_Parm->mode))
    {
        ImgBrowse_StopSlideShow(&pImg_Browser_Parm->ImgBrowse);
    }
	AK_Release_Semaphore(pImg_Browser_Parm->semaphore);
}

void resumeimg_view(void)
{
	if (pImg_Browser_Parm->bZ05COM_MSG)
	{
    	pImg_Browser_Parm->bZ05COM_MSG = AK_FALSE;
	}

	//Fwl_CleanScreen(COLOR_BLACK);

	/*if(Fwl_TvoutIsOpen())
	{
		Fwl_CleanFrameBuf();
	}*/
	
    TopBar_DisableShow();

    Fwl_SetAudioVolumeStatus(AK_FALSE);
    if (gs.ImgSlideInterval && (IMG_SLIDE == pImg_Browser_Parm->mode))
    {
        ImgBrowse_StartSlideShow(&pImg_Browser_Parm->ImgBrowse);
    }

	if (IMG_BROWSE == pImg_Browser_Parm->mode)
	{
    	if (pImg_Browser_Parm->ImgBrowse.largeFlag)
		{
			if (pImg_Browser_Parm->ImgBrowse.DecTimerId == ERROR_TIMER)
			{
				ImgBrowse_StartImgShowFastTimer(&pImg_Browser_Parm->ImgBrowse);					//if(!Fwl_TvoutIsOpen())
			}
		}
		else
		{
			ImgBrowse_StartImgShowFastTimer(&pImg_Browser_Parm->ImgBrowse);				//if(!Fwl_TvoutIsOpen())
		}
	}
}



#endif
