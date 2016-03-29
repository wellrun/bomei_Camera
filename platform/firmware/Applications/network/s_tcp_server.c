#include "Fwl_public.h"
#ifdef SUPPORT_NETWORK

#include "Ctl_MsgBox.h"
#include "Lib_state.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "ctl_net_trans.h"
#include "fwl_oscom.h"

#define TCP_SERVER_ACCEPT_STACK_SIZE		(10*1024)
#define TCP_SERVER_CHANNEL_START_ID			40

typedef struct {
    T_ICONEXPLORER  	IconExplorer;
    T_MSGBOX        	msgbox;
	T_CONNECT_INFO		ConnInfo;
	T_NETCONN_STRUCT	*pNetconn;
	T_hTask 			accepttask;
	T_pVOID 			pStackAddr;
	T_NET_TRANS			*pNetTrans[NT_CHANNEL_MAX];
} T_TCP_SVR_PARM;

static T_TCP_SVR_PARM *pTcpSvrParm;


static T_VOID Tcp_Server_SetItem(T_VOID)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pTcpSvrParm)
	{
		return;
	}

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_TCP_SERVER_PORT));
	Utl_UItoa(pTcpSvrParm->ConnInfo.LocalPort, tmpstr, 10);
	
	Utl_UStrCat(dispstr, tmpstr);
	
	IconExplorer_AddItemWithOption(&pTcpSvrParm->IconExplorer, 10, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//ListenPort set

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_LISTEN));

	IconExplorer_AddItemWithOption(&pTcpSvrParm->IconExplorer, 20, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);//listen

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_STOP));

	IconExplorer_AddItemWithOption(&pTcpSvrParm->IconExplorer, 30, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);//stop
}

static T_VOID Tcp_Server_UpdateItem(T_VOID)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pTcpSvrParm)
	{
		return;
	}
	
	IconExplorer_DelItem(&pTcpSvrParm->IconExplorer, 10);

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_TCP_SERVER_PORT));
	Utl_UItoa(pTcpSvrParm->ConnInfo.LocalPort, tmpstr, 10);
	
	Utl_UStrCat(dispstr, tmpstr);

	IconExplorer_AddItemWithOption(&pTcpSvrParm->IconExplorer, 10, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//ListenPort set
}

static T_VOID Tcp_Server_AddConnItem(T_U32 id)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pTcpSvrParm)
	{
		return;
	}

	if (AK_NULL == pTcpSvrParm->pNetTrans[id])
	{
		return;
	}

	Fwl_Net_Ip2str(pTcpSvrParm->pNetTrans[id]->pNetConn->info.RemoteIp, dispstr);
	
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR2));

	Utl_UItoa(pTcpSvrParm->pNetTrans[id]->pNetConn->info.RemotePort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	IconExplorer_AddItemWithOption(&pTcpSvrParm->IconExplorer, TCP_SERVER_CHANNEL_START_ID + id, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);
}

static T_VOID Tcp_Server_Accept(T_U32 argc, T_VOID *argv)
{
	T_U32 id = NT_CHANNEL_MAX;
	T_U32 i = 0;
	T_BOOL ret = AK_FALSE;
	T_NETCONN_STRUCT	*pNetConn = AK_NULL;
	
	if (AK_NULL == pTcpSvrParm)
	{
		return;
	}

	if (AK_NULL == pTcpSvrParm->pNetconn)
	{
		return;
	}
	
	while (1)
	{
		id = NT_CHANNEL_MAX;
		
		for (i=0; i<NT_CHANNEL_MAX; i++)
		{
			if (AK_NULL == pTcpSvrParm->pNetTrans[i])
			{
				id = i;
				break;
			}
		}

		if (id >= NT_CHANNEL_MAX)
		{
			continue;
		}

		Fwl_Print(C2, M_NETWORK, "wait for a client connect!");

		pNetConn = AK_NULL;

		if (!Fwl_Net_Conn_Accept(pTcpSvrParm->pNetconn, &pNetConn))
		{
			Fwl_Print(C2, M_NETWORK, "Fwl_Net_Conn_Accept failed!");
			return;
		}

		pTcpSvrParm->pNetTrans[id] = NetTrans_Init();
	
		if (AK_NULL == pTcpSvrParm->pNetTrans[id])
		{
			Fwl_Print(C2, M_NETWORK, "NetTrans_Init failed!");
			Fwl_Net_Conn_Delete(pNetConn);
			return;
		}

		pTcpSvrParm->pNetTrans[id]->pNetConn = pNetConn;

		Fwl_Print(C3, M_NETWORK, "Fwl_Net_Conn_Accept OK!");

		pTcpSvrParm->pNetTrans[id]->pNetConn->info.bTcpType = pTcpSvrParm->ConnInfo.bTcpType;
		pTcpSvrParm->pNetTrans[id]->pNetConn->info.LocalIp = pTcpSvrParm->ConnInfo.LocalIp;
		pTcpSvrParm->pNetTrans[id]->pNetConn->info.LocalPort = pTcpSvrParm->ConnInfo.LocalPort;

		ret = NetTrans_CreateRecvTask(pTcpSvrParm->pNetTrans[id]);

		if (ret)
		{
			Tcp_Server_AddConnItem(id);
		}
		
	}

}


static T_BOOL Tcp_Server_Listen(T_VOID)
{
	T_BOOL ret = AK_FALSE;

	if (AK_NULL == pTcpSvrParm)
	{
		return ret;
	}

	if (AK_NULL != pTcpSvrParm->pNetconn)
	{
		return AK_TRUE;
	}

	pTcpSvrParm->pNetconn = Fwl_Net_Conn_New(NETWORKCONN_TCP);

	if (AK_NULL == pTcpSvrParm->pNetconn)
	{
		Fwl_Print(C2, M_NETWORK, "Fwl_Net_Conn_New failed!");
		return ret;
	}

	Fwl_Print(C3, M_NETWORK, "Fwl_Net_Conn_New OK!");

	ret = Fwl_Net_Conn_Bind(pTcpSvrParm->pNetconn, 0, pTcpSvrParm->ConnInfo.LocalPort);

	if (!ret)
	{
		T_RECT msgRect;
		
		Fwl_Print(C2, M_NETWORK, "Fwl_Net_Conn_Bind failed!");
		
		MsgBox_InitStr(&pTcpSvrParm->msgbox, 0, GetCustomTitle(ctHINT),\
        	Res_GetStringByID(eRES_STR_PORT_INVALID), MSGBOX_INFORMATION);
        MsgBox_Show(&pTcpSvrParm->msgbox);
        MsgBox_GetRect(&pTcpSvrParm->msgbox, &msgRect);
        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
        Fwl_MiniDelay(1000);
		
		return ret;
	}

	Fwl_Print(C3, M_NETWORK, "Fwl_Net_Conn_Bind OK!");

	ret = Fwl_Net_Conn_Listen(pTcpSvrParm->pNetconn);

	if (!ret)
	{
		Fwl_Print(C2, M_NETWORK, "Fwl_Net_Conn_Listen failed!");
		return ret;
	}

	Fwl_Print(C3, M_NETWORK, "Fwl_Net_Conn_Listen OK!");
	
	pTcpSvrParm->pStackAddr = Fwl_Malloc(TCP_SERVER_ACCEPT_STACK_SIZE);
	AK_ASSERT_PTR(pTcpSvrParm->pStackAddr, "pTcpSvrParm->pStackAddr malloc error", ret);
	memset(pTcpSvrParm->pStackAddr, 0, TCP_SERVER_ACCEPT_STACK_SIZE);

	pTcpSvrParm->accepttask = AK_Create_Task((T_VOID*)Tcp_Server_Accept, "accept", 1, 
		AK_NULL, pTcpSvrParm->pStackAddr, TCP_SERVER_ACCEPT_STACK_SIZE, 110, 5, 
		AK_PREEMPT, AK_START);

	return ret;

}


static T_BOOL Tcp_Server_Close(T_VOID)
{
	T_BOOL ret = AK_FALSE;
	T_U32 i = 0;
	
	if (AK_NULL == pTcpSvrParm)
	{
		return ret;
	}

	if (AK_NULL == pTcpSvrParm->pNetconn)
	{
		return ret;
	}

	if (AK_INVALID_TASK != pTcpSvrParm->accepttask)
	{
		AK_Terminate_Task(pTcpSvrParm->accepttask);
		AK_Delete_Task(pTcpSvrParm->accepttask);
		pTcpSvrParm->accepttask = AK_INVALID_TASK;
			
		pTcpSvrParm->pStackAddr = Fwl_Free(pTcpSvrParm->pStackAddr);
	}

	for (i=0; i<NT_CHANNEL_MAX; i++)
	{
		if (AK_NULL != pTcpSvrParm->pNetTrans[i])
		{
			if (AK_NULL != pTcpSvrParm->pNetTrans[i]->pNetConn)
			{
				T_U32 count = 0;
				
				Fwl_Net_Conn_Close(pTcpSvrParm->pNetTrans[i]->pNetConn);
			
				while ((!pTcpSvrParm->pNetTrans[i]->bRecvExit)
					&& (count < 10))
				{
					AK_Sleep(10);
					count++;
				}
				
				Fwl_Net_Conn_Delete(pTcpSvrParm->pNetTrans[i]->pNetConn);
				IconExplorer_DelItem(&pTcpSvrParm->IconExplorer, TCP_SERVER_CHANNEL_START_ID + i);
			}
			
			pTcpSvrParm->pNetTrans[i] = NetTrans_Free(pTcpSvrParm->pNetTrans[i]);
		}
	}
	
	ret = Fwl_Net_Conn_Delete(pTcpSvrParm->pNetconn);
	pTcpSvrParm->pNetconn = AK_NULL;

	return ret;
}


static T_BOOL Tcp_Server_CheckState(T_VOID)
{
	T_U32 i = 0;
	
	if (AK_NULL == pTcpSvrParm)
	{
		return AK_FALSE;
	}

	for (i=0; i<NT_CHANNEL_MAX; i++)
	{
		if (AK_NULL != pTcpSvrParm->pNetTrans[i])
		{
			if (NET_CLOSE_FLAG_CLOSED == pTcpSvrParm->pNetTrans[i]->pNetConn->closeflag)
			{
				Fwl_Print(C3, M_NETWORK, "channel %d close!", i);
				Fwl_Net_Conn_Delete(pTcpSvrParm->pNetTrans[i]->pNetConn);
				pTcpSvrParm->pNetTrans[i] = NetTrans_Free(pTcpSvrParm->pNetTrans[i]);
				IconExplorer_DelItem(&pTcpSvrParm->IconExplorer, TCP_SERVER_CHANNEL_START_ID+i);
			}
		}
		else
		{
			if (AK_NULL != IconExplorer_GetItem(&pTcpSvrParm->IconExplorer, TCP_SERVER_CHANNEL_START_ID+i))
			{
				IconExplorer_DelItem(&pTcpSvrParm->IconExplorer, TCP_SERVER_CHANNEL_START_ID+i);
			}
		}
	}

	return AK_TRUE;
}


#endif
/*---------------------- BEGIN OF STATE s_tcp_server ------------------------*/
void inittcp_server(void)
{
#ifdef SUPPORT_NETWORK

    pTcpSvrParm = (T_TCP_SVR_PARM *)Fwl_Malloc(sizeof(T_TCP_SVR_PARM));
    AK_ASSERT_PTR_VOID(pTcpSvrParm, "inittcp_server(): malloc error");
	memset(pTcpSvrParm, 0, sizeof(T_TCP_SVR_PARM));

	pTcpSvrParm->ConnInfo.LocalPort = gs.listen_port;
	pTcpSvrParm->ConnInfo.LocalIp = gs.ipaddr;
	pTcpSvrParm->ConnInfo.bTcpType = AK_TRUE;
	pTcpSvrParm->accepttask = AK_INVALID_TASK;
	
    MenuStructInit(&pTcpSvrParm->IconExplorer);
	IconExplorer_SetTitleText(&pTcpSvrParm->IconExplorer, 
		Res_GetStringByID(eRES_STR_TCP_SERVER), ICONEXPLORER_TITLE_TEXTCOLOR);         // TCP ·þÎñÆ÷
	IconExplorer_SetSortIdCallBack(&pTcpSvrParm->IconExplorer, IconExplorer_SortIdCallback);
#endif
}

void exittcp_server(void)
{
#ifdef SUPPORT_NETWORK
	gs.listen_port = pTcpSvrParm->ConnInfo.LocalPort;
	Tcp_Server_Close();

    IconExplorer_Free(&pTcpSvrParm->IconExplorer);
    pTcpSvrParm = Fwl_Free(pTcpSvrParm);
#endif
}

void painttcp_server(void)
{
#ifdef SUPPORT_NETWORK

    IconExplorer_Show(&pTcpSvrParm->IconExplorer);

    Fwl_RefreshDisplay();
#endif
}

unsigned char handletcp_server(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_eBACK_STATE IconExplorerRet;	
	T_U32 focusId = 0;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pTcpSvrParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

	if (M_EVT_2 == event)
	{
		Tcp_Server_SetItem();
	}

	if (M_EVT_EXIT == event)
	{
		Tcp_Server_UpdateItem();
	}

	Tcp_Server_CheckState();
	
    IconExplorerRet = IconExplorer_Handler(&pTcpSvrParm->IconExplorer, event, pEventParm);
	focusId = IconExplorer_GetItemFocusId(&pTcpSvrParm->IconExplorer);

    switch (IconExplorerRet)
    {
    case eNext:
        switch (focusId)
        {
        case 10: //ListenPort set
        	if (AK_NULL != pTcpSvrParm->pNetconn)
			{
				T_RECT msgRect;
				
				Fwl_Print(C3, M_NETWORK, "Server is running, don't modify port!");
				MsgBox_InitStr(&pTcpSvrParm->msgbox, 0, GetCustomTitle(ctHINT),\
		        	Res_GetStringByID(eRES_STR_SERVER_RUNNING), MSGBOX_INFORMATION);
		        MsgBox_Show(&pTcpSvrParm->msgbox);
		        MsgBox_GetRect(&pTcpSvrParm->msgbox, &msgRect);
		        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
		        Fwl_MiniDelay(1000);
				break;
			}
        	
			pEventParm->p.pParam1 = &pTcpSvrParm->ConnInfo.LocalPort;
			m_triggerEvent(M_EVT_1, pEventParm);
            break;

		case 20: //listen
			if (!Tcp_Server_Listen())
			{
				T_RECT msgRect;
				
				Tcp_Server_Close();
				
				MsgBox_InitStr(&pTcpSvrParm->msgbox, 0, GetCustomTitle(ctHINT),\
		        	Res_GetStringByID(eRES_STR_COM_FAILURE_DONE), MSGBOX_INFORMATION);
		        MsgBox_Show(&pTcpSvrParm->msgbox);
		        MsgBox_GetRect(&pTcpSvrParm->msgbox, &msgRect);
		        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
		        Fwl_MiniDelay(1000);
			}
			break;

		case 30: //stop
			Tcp_Server_Close();
			break;

        default:
			if ((focusId >= TCP_SERVER_CHANNEL_START_ID) && (focusId < TCP_SERVER_CHANNEL_START_ID+NT_CHANNEL_MAX))
			{
				pEventParm->p.pParam1 = pTcpSvrParm->pNetTrans;
				pEventParm->w.Param2 = focusId - TCP_SERVER_CHANNEL_START_ID;
				m_triggerEvent(M_EVT_NETWORK_TRANS, pEventParm);
			}
            break;
        }
        break;

	case eMenu:
		if ((focusId >= TCP_SERVER_CHANNEL_START_ID) && (focusId < TCP_SERVER_CHANNEL_START_ID+NT_CHANNEL_MAX))
		{
			T_U32 count = 0;
			
			Fwl_Print(C3, M_NETWORK, "Tcp Server Close Channel:%d!", focusId - TCP_SERVER_CHANNEL_START_ID);
			Fwl_Net_Conn_Close(pTcpSvrParm->pNetTrans[focusId - TCP_SERVER_CHANNEL_START_ID]->pNetConn);

			while ((!pTcpSvrParm->pNetTrans[focusId - TCP_SERVER_CHANNEL_START_ID]->bRecvExit)
				&& (count < 10))
			{
				AK_Sleep(10);
				count++;
			}
			
			Fwl_Net_Conn_Delete(pTcpSvrParm->pNetTrans[focusId - TCP_SERVER_CHANNEL_START_ID]->pNetConn);
			pTcpSvrParm->pNetTrans[focusId - TCP_SERVER_CHANNEL_START_ID] = NetTrans_Free(pTcpSvrParm->pNetTrans[focusId - TCP_SERVER_CHANNEL_START_ID]);

			IconExplorer_DelItem(&pTcpSvrParm->IconExplorer, focusId);
		}
		break;
		
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}

