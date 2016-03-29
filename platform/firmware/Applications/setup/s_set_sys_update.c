
#include "Fwl_public.h"
#include "Eng_ScreenSave.h"
#include "Eng_String.h"
#include "Ctl_Msgbox.h"
#include "Ctl_Progress.h"
#include "Lib_state_api.h"
#include "update.h"
#include "Eng_TopBar.h"
#include "ctl_displaylist.h"

#if (defined (SPIBOOT) && defined (SUPPORT_SYS_SET))

#include "fwl_power.h"

#define RATE_PROGRESS_LEN  5
#define UPDATE_BUFFERSIZE  24*1024

typedef enum{
    STA_UPDATEIDLE,
	STA_UPDATEPATH_VIEW,
	STA_UPDATEPATH_CONFIRM,
	STA_UPDATEPATH_CONFIRMED,
    STA_UPDATING,
    STA_UPDATEWARNING,
    STA_UPDATERROR,
    STA_UPDATEFINESH,
    STA_UPDATEIXT,
    STA_UPDATEND,
    STA_UPDATELOWBATTERY
}STA_UPDATE;

typedef struct {
	T_DISPLAYLIST			displayList;
    E_UPDATE_RESULT_CODE	info;
    T_U32       progress_rate;
    T_MSGBOX    msgbox;    
    T_PGBAR     pgbar;                      /**< display the updating progress */
    T_U8        Strprogress_rate[RATE_PROGRESS_LEN]; /** the progress rate, in str form */
    T_U8        *Buffer;                   /**< for memory reading and writing */
	T_S32  		nTimes;        
    T_U32  		CurrentState;
	T_USTR_FILE path;
} T_SYSUPDATE_PARM;

static T_SYSUPDATE_PARM *pSysUpdateParm;

T_VOID sys_update_progress(E_UPDATE_RESULT_CODE code, T_U32 param)
{
    pSysUpdateParm->info = code;
    pSysUpdateParm->progress_rate = param;    
    AK_DEBUG_OUTPUT("cur=%d,rate=%d",pSysUpdateParm->info,pSysUpdateParm->progress_rate);
}

T_VOID initset_sys_update(T_VOID)
{
	T_FILE_TYPE FileType[] = {
        FILE_TYPE_ALL,       
        FILE_TYPE_NONE
    };
	
    ScreenSaverDisable();    
    //TopBar_DisableShow();
    pSysUpdateParm = (T_SYSUPDATE_PARM *)Fwl_Malloc(sizeof(T_SYSUPDATE_PARM));
    AK_ASSERT_PTR_VOID(pSysUpdateParm, " malloc pSysUpdateParm error\n");      
	memset(pSysUpdateParm, 0, sizeof(T_SYSUPDATE_PARM));

	pSysUpdateParm->CurrentState = STA_UPDATEPATH_VIEW;  
    pSysUpdateParm->info = 0;//确认初始化为1，避免和更新返回码重复出现问题
    pSysUpdateParm->Buffer = (T_U8 *)Fwl_Malloc(UPDATE_BUFFERSIZE);
    AK_ASSERT_PTR_VOID(pSysUpdateParm->Buffer, "malloc pSysUpdateParm->Buffer error\n");

	MsgBox_InitAfx(&pSysUpdateParm->msgbox, 0, ctHINT, csSYS_UPDATING, MSGBOX_INFORMATION|MSGBOX_OK);
	
	DisplayList_init(&pSysUpdateParm->displayList, Fwl_GetDefPath(eUPDATE_PATH), \
                      Res_GetStringByID(eRES_STR_SYS_UPDATE), FileType);

	DisplayList_CheckItemList(&pSysUpdateParm->displayList);
}

T_VOID exitset_sys_update(T_VOID)
{
    ScreenSaverEnable();

    UpdateTask_Close();
	
    DisplayList_Free(&pSysUpdateParm->displayList);
	
    pSysUpdateParm->Buffer = Fwl_Free(pSysUpdateParm->Buffer);
    pSysUpdateParm = Fwl_Free(pSysUpdateParm);
    TopBar_DisableMenuButton();
}

T_VOID paintset_sys_update(T_VOID)
{
	if (STA_UPDATEPATH_VIEW == pSysUpdateParm->CurrentState)
	{
		T_FILE_INFO *pFileInfo = AK_NULL;

	    pFileInfo = DisplayList_GetItemContentFocus(&pSysUpdateParm->displayList);
	    if ((DisplayList_GetSubLevel(&pSysUpdateParm->displayList) > 0) && pFileInfo)
	    {
	        if ((pFileInfo->attrib&0x10) == 0x10)
	        {
	        	if (0 != Utl_UStrCmp(pFileInfo->name, _T("..")))
		        {
		            TopBar_EnableMenuButton();
		        }
				else
				{
					TopBar_DisableMenuButton();
				}
	        }
	        else
	        {
	            TopBar_DisableMenuButton();
	        }
	    }
	    else
	    {
	        TopBar_DisableMenuButton();
	    }
		
	    DisplayList_Show(&pSysUpdateParm->displayList);
		Fwl_RefreshDisplay();
	}
	else if (STA_UPDATEPATH_CONFIRM == pSysUpdateParm->CurrentState)
	{
		MsgBox_Show(&pSysUpdateParm->msgbox);
		Fwl_RefreshDisplay();
	}
}

T_U8 handleset_sys_update(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    T_U32         Bat_Value;
    T_eBACK_STATE msgRet;    
    T_pRECT       pMBoxRct = AK_NULL;      
    T_U16         uText[50];
	T_eBACK_STATE DisplayListRet;
    T_FILE_INFO *pFileInfo = AK_NULL;
  
    if (IsPostProcessEvent(event))
    {
        return 1;
    }

	if (STA_UPDATEPATH_VIEW == pSysUpdateParm->CurrentState)
	{
		DisplayListRet = DisplayList_Handler(&pSysUpdateParm->displayList, event, pEventParm);
		
    	switch (DisplayListRet)
	    {
	    case eNext:
	        DisplayList_SetRefresh(&pSysUpdateParm->displayList, DISPLAYLIST_REFRESH_ALL);
	        pFileInfo = DisplayList_Operate(&pSysUpdateParm->displayList);
	        break;
	        
	   case eMenu:
	        pFileInfo = DisplayList_GetItemContentFocus(&pSysUpdateParm->displayList);
	        if ((DisplayList_GetSubLevel(&pSysUpdateParm->displayList) > 0) && pFileInfo)
	        {
            	if ((pFileInfo->attrib&0x10) == 0x10)
            	{
            		if (Utl_UStrCmp(pFileInfo->name, _T("..")) == 0)
            		{
            			//上级目录
						break;
					}
					
            		Utl_UStrCpy(pSysUpdateParm->path, DisplayList_GetCurFilePath(&pSysUpdateParm->displayList));
					
					if (Utl_UStrCmp(pFileInfo->name, _T(".")) != 0)
	            	{
						Utl_UStrCat(pSysUpdateParm->path, pFileInfo->name);
						Utl_UStrCat(pSysUpdateParm->path, _T("/"));
					}
				
	                TopBar_DisableMenuButton();
	                DisplayList_SetRefresh(&pSysUpdateParm->displayList, DISPLAYLIST_REFRESH_ALL);
					pSysUpdateParm->CurrentState = STA_UPDATEPATH_CONFIRM;

					MsgBox_InitStr(&pSysUpdateParm->msgbox, 0, GetCustomTitle(ctHINT), Res_GetStringByID(eRES_STR_UPDATE_PATH_CNFM), MSGBOX_QUESTION | MSGBOX_YESNO);
					MsgBox_Show(&pSysUpdateParm->msgbox);
            	}
	        }
	        break;
	    default:
	        ReturnDefauleProc(DisplayListRet, pEventParm);
	        break;
	    }
	}

    if(M_EVT_USER_KEY != event)//按键无影响
    {
        switch(pSysUpdateParm->CurrentState)
        {
        case STA_UPDATEPATH_CONFIRMED:
            pSysUpdateParm->nTimes = 0;
            Bat_Value = Fwl_GetBatteryVoltage();
            if ((Bat_Value <= 3500) && (!Fwl_UseExternCharge()))
            {
                AK_DEBUG_OUTPUT("Bat_Value=%d\n",Bat_Value);
                pSysUpdateParm->CurrentState = STA_UPDATELOWBATTERY;
                MsgBox_InitAfx(&pSysUpdateParm->msgbox, 0, ctHINT, csSYS_LOWBATTERY, MSGBOX_INFORMATION|MSGBOX_OK);
                MsgBox_Show(&pSysUpdateParm->msgbox);    
            }
            else//开始更新
            {
                if(AK_FALSE == SPI_UpdateTask(pSysUpdateParm->path, AK_NULL, sys_update_progress, pSysUpdateParm->Buffer, UPDATE_BUFFERSIZE))
                {
                    AK_DEBUG_OUTPUT("Create SPI_UpdateTask failse!!\n");
                    pSysUpdateParm->CurrentState = STA_UPDATEIDLE;//创建任务失败退出
                    m_triggerEvent(M_EVT_EXIT, pEventParm);
                }
                else
                {
                    AK_DEBUG_OUTPUT("Create SPI_UpdateTask success!!\n");
                    pSysUpdateParm->CurrentState = STA_UPDATING;                    
                    MsgBox_InitAfx(&pSysUpdateParm->msgbox, 0, ctHINT, csSYS_UPDATING, MSGBOX_INFORMATION);
                    MsgBox_Show(&pSysUpdateParm->msgbox);    
                }
            }
            break;
        case STA_UPDATING:
            AK_DEBUG_OUTPUT("pSysUpdateParm->info=%d\n",pSysUpdateParm->info);
            if(UPDATE_SUCCESSFUL == pSysUpdateParm->info)
            {
                pSysUpdateParm->nTimes = pSysUpdateParm->progress_rate;
                if(pSysUpdateParm->nTimes <= 100)
                {
                    Utl_UStrCpy(uText, GetCustomString(csSYS_UPDATING));
                    Utl_Itoa(pSysUpdateParm->nTimes, pSysUpdateParm->Strprogress_rate, RATE_PROGRESS_LEN);
                    Utl_UStrCat(uText, _T(" "));
                    Utl_UStrCat(uText, _T(pSysUpdateParm->Strprogress_rate));
                    Utl_UStrCat(uText, _T(" %"));
                    MsgBox_ClearContent_EX(&pSysUpdateParm->msgbox);
                    MsgBox_AddLine(&pSysUpdateParm->msgbox, uText);    
                    MsgBox_SetRefresh(&pSysUpdateParm->msgbox,CTL_REFRESH_CONTENT);
                    pMBoxRct = &pSysUpdateParm->msgbox.res.MsgBkImgRct; 
                    PgBar_Init(&pSysUpdateParm->pgbar, (T_POS)(pMBoxRct->left + 8),
                                (T_LEN)(pMBoxRct->top + pMBoxRct->height - 40),
                                (T_LEN)(pMBoxRct->width - 16), 12, 0, PGBAR_PILE_ON);
                    PgBar_SetValue(&pSysUpdateParm->pgbar, (T_U16)pSysUpdateParm->nTimes/4, 25);
                    MsgBox_Show(&pSysUpdateParm->msgbox);    
                    PgBar_Show(&pSysUpdateParm->pgbar);
                    if(100 == pSysUpdateParm->nTimes )//完成重启
                    {
                        pSysUpdateParm->nTimes = 0;
                        pSysUpdateParm->CurrentState = STA_UPDATEFINESH;    
                    }
                }
                break;
            }            
            else if(FOLDER_NO_EXIST == pSysUpdateParm->info)
            {
                AK_DEBUG_OUTPUT("the folder is not exit!!\n");
                pSysUpdateParm->CurrentState = STA_UPDATEIXT;
                MsgBox_InitAfx(&pSysUpdateParm->msgbox, 0, ctHINT, csSYS_UPDATING, MSGBOX_INFORMATION|MSGBOX_OK);
                Utl_UStrCpy(uText, GetCustomString(csSYS_UPDATENOFILE));
            }
            else if(FILE_NO_EXIST == pSysUpdateParm->info)
            {
                AK_DEBUG_OUTPUT("the file is not exit!!\n");
                pSysUpdateParm->CurrentState = STA_UPDATEIXT;    
                MsgBox_InitAfx(&pSysUpdateParm->msgbox, 0, ctHINT, csSYS_UPDATING, MSGBOX_INFORMATION|MSGBOX_OK);
                Utl_UStrCpy(uText, GetCustomString(csSYS_UPDATENOFILE));
            }
            else if(OTHER_ERROR == pSysUpdateParm->info)
            {
                AK_DEBUG_OUTPUT("other error !!\n");
                pSysUpdateParm->CurrentState = STA_UPDATERROR;
                Utl_UStrCpy(uText, GetCustomString(csSYS_UPDATERROR));
            }
            else
                break;
            MsgBox_ClearContent_EX(&pSysUpdateParm->msgbox);
            MsgBox_AddLine(&pSysUpdateParm->msgbox, uText); 
            MsgBox_SetRefresh(&pSysUpdateParm->msgbox,CTL_REFRESH_CONTENT);
            MsgBox_Show(&pSysUpdateParm->msgbox);
            break;
        case STA_UPDATEFINESH://需要重启
            AK_DEBUG_OUTPUT("update finished!!\n");//更新
            MsgBox_InitAfx(&pSysUpdateParm->msgbox, 0, ctHINT, csSYS_UPDATEFINISH, MSGBOX_INFORMATION|MSGBOX_OK);
            MsgBox_Show(&pSysUpdateParm->msgbox);
            pSysUpdateParm->CurrentState = STA_UPDATEND;
            break;
        default:
            break;
        }
        Fwl_RefreshDisplay();
    }
    
    AK_DEBUG_OUTPUT("Enter:handleset_sys_update()");
    msgRet = MsgBox_Handler(&pSysUpdateParm->msgbox, event, pEventParm);  
	
    AK_DEBUG_OUTPUT("msgRet = %d\n",msgRet);
    switch(msgRet)
    {
    case eReturn:
    case eNext:        
        AK_DEBUG_OUTPUT("eNextProc()");        
        if(STA_UPDATEIXT == pSysUpdateParm->CurrentState)//文件不存在，退出
        {
            pSysUpdateParm->CurrentState = STA_UPDATEIDLE;
            m_triggerEvent(M_EVT_EXIT, pEventParm);
        }
        else if(STA_UPDATEND == pSysUpdateParm->CurrentState)//更新完成，重启
        {
            AK_DEBUG_OUTPUT("VME_Reset()!!\n");    
			Fwl_SetDefPath(eUPDATE_PATH, pSysUpdateParm->path);
			
			pSysUpdateParm->Buffer = Fwl_Free(pSysUpdateParm->Buffer);
            pSysUpdateParm = Fwl_Free(pSysUpdateParm);//释放内存
            /*power off system*/
            VME_ReTriggerEvent(M_EVT_Z99COM_POWEROFF, AK_NULL);
            gb.ResetAfterPowerOff = AK_TRUE;
            
            return 0;
        }        
        else if(STA_UPDATELOWBATTERY == pSysUpdateParm->CurrentState)//低电 退出
        {
            pSysUpdateParm->CurrentState = STA_UPDATEIDLE;
            m_triggerEvent(M_EVT_EXIT, pEventParm);
        }
        else if(STA_UPDATERROR == pSysUpdateParm->CurrentState)
        {
            pSysUpdateParm->CurrentState = STA_UPDATEIDLE;//再次升级 此时已无法完成升级任务需要usb烧录
        }
		else if(STA_UPDATEPATH_CONFIRM == pSysUpdateParm->CurrentState)
        {
        	if (eNext == msgRet)
        	{
				pSysUpdateParm->CurrentState = STA_UPDATEPATH_CONFIRMED;
			}
			else
			{
				pSysUpdateParm->CurrentState = STA_UPDATEPATH_VIEW;
			}
        }
        break;
    default:    
        ReturnDefauleProc(msgRet, pEventParm);
        break;
    }
    return 0;
    
}
#else
T_VOID initset_sys_update(T_VOID)
{

}
T_VOID exitset_sys_update(T_VOID)
{

}

T_VOID paintset_sys_update(T_VOID)
{

}

T_U8 handleset_sys_update(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
    return 0;
}
#endif




















