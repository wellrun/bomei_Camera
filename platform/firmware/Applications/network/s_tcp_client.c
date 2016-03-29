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

#define TCP_CLIENT_CHANNEL_START_ID			50

typedef struct {
    T_ICONEXPLORER  	IconExplorer;
    T_MSGBOX        	msgbox;
	T_CONNECT_INFO		ConnInfo;
	T_NET_TRANS			*pNetTrans[NT_CHANNEL_MAX];
} T_TCP_CLT_PARM;

static T_TCP_CLT_PARM *pTcpCltParm;

static T_VOID Tcp_Client_SetItem(T_VOID)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pTcpCltParm)
	{
		return;
	}

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE_IP));
	Fwl_Net_Ip2str(pTcpCltParm->ConnInfo.RemoteIp, tmpstr);
	Utl_UStrCat(dispstr, tmpstr);
	
	IconExplorer_AddItemWithOption(&pTcpCltParm->IconExplorer, 10, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//remote ip set

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE_PORT));
	Utl_UItoa(pTcpCltParm->ConnInfo.RemotePort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	IconExplorer_AddItemWithOption(&pTcpCltParm->IconExplorer, 20, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//remote port set

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_LOCAL_PORT));
	Utl_UItoa(pTcpCltParm->ConnInfo.LocalPort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	//IconExplorer_AddItemWithOption(&pTcpCltParm->IconExplorer, 30, AK_NULL, 0, dispstr,
                    //AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//local port set

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_CONNECT));

	IconExplorer_AddItemWithOption(&pTcpCltParm->IconExplorer, 40, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);//connect
}

static T_VOID Tcp_Client_UpdateItem(T_VOID)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pTcpCltParm)
	{
		return;
	}
	
	IconExplorer_DelItem(&pTcpCltParm->IconExplorer, 10);
	IconExplorer_DelItem(&pTcpCltParm->IconExplorer, 20);
	//IconExplorer_DelItem(&pTcpCltParm->IconExplorer, 30);

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE_IP));
	Fwl_Net_Ip2str(pTcpCltParm->ConnInfo.RemoteIp, tmpstr);
	Utl_UStrCat(dispstr, tmpstr);
	
	IconExplorer_AddItemWithOption(&pTcpCltParm->IconExplorer, 10, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//remote ip set

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE_PORT));
	Utl_UItoa(pTcpCltParm->ConnInfo.RemotePort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	IconExplorer_AddItemWithOption(&pTcpCltParm->IconExplorer, 20, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//remote port set

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_LOCAL_PORT));
	Utl_UItoa(pTcpCltParm->ConnInfo.LocalPort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	//IconExplorer_AddItemWithOption(&pTcpCltParm->IconExplorer, 30, AK_NULL, 0, dispstr,
                    //AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//local port set
}


static T_VOID Tcp_Client_AddConnItem(T_U32 id, T_BOOL bConnected)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pTcpCltParm)
	{
		return;
	}

	Fwl_Net_Ip2str(pTcpCltParm->ConnInfo.RemoteIp, dispstr);
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR2));

	Utl_UItoa(pTcpCltParm->ConnInfo.RemotePort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	Eng_StrMbcs2Ucs("  ", tmpstr);
	Utl_UStrCat(dispstr, tmpstr);

	Fwl_Net_Ip2str(pTcpCltParm->ConnInfo.LocalIp, tmpstr);
	Utl_UStrCat(dispstr, tmpstr);
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR2));

	Utl_UItoa(pTcpCltParm->ConnInfo.LocalPort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	if (bConnected)
	{
		Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CONNECTED));
	}
	else
	{
		Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CONNECTING));
	}

	IconExplorer_AddItemWithOption(&pTcpCltParm->IconExplorer, TCP_CLIENT_CHANNEL_START_ID + id, AK_NULL, 0, dispstr, \
					AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);
}

static T_BOOL Tcp_Client_Connect(T_U32 id)
{
	T_BOOL ret = AK_FALSE;
	
	AK_ASSERT_VAL(id < NT_CHANNEL_MAX, "Tcp_Client_Connect(): id err", AK_FALSE);
	AK_ASSERT_PTR(pTcpCltParm, "Tcp_Client_Connect(): pTcpCltParm err", AK_FALSE);

	Fwl_Print(C3, M_NETWORK, "Tcp_Client_Connect id:%d!", id);

	pTcpCltParm->pNetTrans[id] = NetTrans_Init();
	
	if (AK_NULL == pTcpCltParm->pNetTrans[id])
	{
		Fwl_Print(C2, M_NETWORK, "NetTrans_Init failed!");
		return AK_FALSE;
	}

	pTcpCltParm->pNetTrans[id]->pNetConn = Fwl_Net_Conn_New(NETWORKCONN_TCP);

	if (AK_NULL == pTcpCltParm->pNetTrans[id]->pNetConn)
	{
		Fwl_Print(C2, M_NETWORK, "Fwl_Net_Conn_New failed!");
		return AK_FALSE;
	}

	Fwl_Print(C3, M_NETWORK, "Fwl_Net_Conn_New OK!");

	ret = Fwl_Net_Conn_Bind(pTcpCltParm->pNetTrans[id]->pNetConn, 0, 0);	// 本地端口传0，则由协议随机指定一个能用的，避免端口受限制
	
	pTcpCltParm->ConnInfo.LocalPort = pTcpCltParm->pNetTrans[id]->pNetConn->info.LocalPort;

	if (!ret)
	{
		Fwl_Print(C2, M_NETWORK, "Fwl_Net_Conn_Bind failed!");
		return AK_FALSE;
	}

	Fwl_Print(C3, M_NETWORK, "Fwl_Net_Conn_Bind OK!");

	Tcp_Client_AddConnItem(id, AK_FALSE);
	IconExplorer_Show(&pTcpCltParm->IconExplorer);
    Fwl_RefreshDisplay();

	WaitBox_Start(WAITBOX_RAINBOW, (T_pWSTR)Res_GetStringByID(eRES_STR_CONNECTING));

	ret = Fwl_Net_Conn_Connect(pTcpCltParm->pNetTrans[id]->pNetConn, pTcpCltParm->ConnInfo.RemoteIp, pTcpCltParm->ConnInfo.RemotePort);

	WaitBox_Stop();

	IconExplorer_DelItem(&pTcpCltParm->IconExplorer, TCP_CLIENT_CHANNEL_START_ID + id);
	

	if (!ret)
	{
		Fwl_Print(C2, M_NETWORK, "Fwl_Net_Conn_Connect failed!");
		
		return ret;
	}

	Fwl_Print(C3, M_NETWORK, "Fwl_Net_Conn_Connect OK!");

	memcpy(&pTcpCltParm->pNetTrans[id]->pNetConn->info, &pTcpCltParm->ConnInfo, sizeof(T_CONNECT_INFO));

	ret = NetTrans_CreateRecvTask(pTcpCltParm->pNetTrans[id]);

	if (ret)
	{
		Tcp_Client_AddConnItem(id, AK_TRUE);
	}
	
	return ret;
}

static T_BOOL Tcp_Client_TryConnect(T_VOID)
{
	T_BOOL ret = AK_FALSE;
	T_U32 id = NT_CHANNEL_MAX;
	T_U32 i = 0;
	T_RECT msgRect;
	
	AK_ASSERT_PTR(pTcpCltParm, "Tcp_Client_TryConnect(): pTcpCltParm err", AK_FALSE);

	for (i=0; i<NT_CHANNEL_MAX; i++)
	{
		if (AK_NULL == pTcpCltParm->pNetTrans[i])
		{
			id = i;
			break;
		}
	}

	if (id >= NT_CHANNEL_MAX)
	{
		Fwl_Print(C3, M_NETWORK, "Tcp_Client_TryConnect no empty id!");
		MsgBox_InitStr(&pTcpCltParm->msgbox, 0, GetCustomTitle(ctHINT),\
        	Res_GetStringByID(eRES_STR_ACHIEVED_MAX_CHANNEL), MSGBOX_INFORMATION);
        MsgBox_Show(&pTcpCltParm->msgbox);
        MsgBox_GetRect(&pTcpCltParm->msgbox, &msgRect);
        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
        Fwl_MiniDelay(1000);
		
		return AK_FALSE;
	}

	ret = Tcp_Client_Connect(id);

	if (!ret)
	{
		Fwl_Net_Conn_Delete(pTcpCltParm->pNetTrans[id]->pNetConn);
		pTcpCltParm->pNetTrans[id] = NetTrans_Free(pTcpCltParm->pNetTrans[id]);

		MsgBox_InitStr(&pTcpCltParm->msgbox, 0, GetCustomTitle(ctHINT),\
        	Res_GetStringByID(eRES_STR_CONNECT_FAILED), MSGBOX_INFORMATION);
        MsgBox_Show(&pTcpCltParm->msgbox);
        MsgBox_GetRect(&pTcpCltParm->msgbox, &msgRect);
        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
        Fwl_MiniDelay(1000);
	}
	
	return ret;
}

static T_BOOL Tcp_Client_CloseAll(T_VOID)
{
	T_U32 i = 0;
	
	if (AK_NULL == pTcpCltParm)
	{
		return AK_FALSE;
	}

	for (i=0; i<NT_CHANNEL_MAX; i++)
	{
		if (AK_NULL != pTcpCltParm->pNetTrans[i])
		{
			T_U32 count = 0;
			
			Fwl_Net_Conn_Close(pTcpCltParm->pNetTrans[i]->pNetConn);
			
			while ((!pTcpCltParm->pNetTrans[i]->bRecvExit)
				&&(count < 10))
			{
				AK_Sleep(10);
				count++;
			}
			
			Fwl_Net_Conn_Delete(pTcpCltParm->pNetTrans[i]->pNetConn);
			pTcpCltParm->pNetTrans[i] = NetTrans_Free(pTcpCltParm->pNetTrans[i]);
		}
	}

	return AK_TRUE;
}

static T_BOOL Tcp_Client_CheckState(T_VOID)
{
	T_U32 i = 0;
	
	if (AK_NULL == pTcpCltParm)
	{
		return AK_FALSE;
	}

	for (i=0; i<NT_CHANNEL_MAX; i++)
	{
		if (AK_NULL != pTcpCltParm->pNetTrans[i])
		{
			if (NET_CLOSE_FLAG_CLOSED == pTcpCltParm->pNetTrans[i]->pNetConn->closeflag)
			{
				Fwl_Print(C3, M_NETWORK, "channel %d close!", i);
				Fwl_Net_Conn_Delete(pTcpCltParm->pNetTrans[i]->pNetConn);
				pTcpCltParm->pNetTrans[i] = NetTrans_Free(pTcpCltParm->pNetTrans[i]);
				IconExplorer_DelItem(&pTcpCltParm->IconExplorer, TCP_CLIENT_CHANNEL_START_ID+i);
			}
		}
		else
		{
			if (AK_NULL != IconExplorer_GetItem(&pTcpCltParm->IconExplorer, TCP_CLIENT_CHANNEL_START_ID+i))
			{
				IconExplorer_DelItem(&pTcpCltParm->IconExplorer, TCP_CLIENT_CHANNEL_START_ID+i);
			}
		}
	}

	return AK_TRUE;
}

#endif
/*---------------------- BEGIN OF STATE s_tcp_client ------------------------*/
void inittcp_client(void)
{
#ifdef SUPPORT_NETWORK

    pTcpCltParm = (T_TCP_CLT_PARM *)Fwl_Malloc(sizeof(T_TCP_CLT_PARM));
    AK_ASSERT_PTR_VOID(pTcpCltParm, "inittcp_client(): malloc error");
	memset(pTcpCltParm, 0, sizeof(T_TCP_CLT_PARM));

	pTcpCltParm->ConnInfo.RemoteIp = gs.tcpclt_rmt_ip;
	pTcpCltParm->ConnInfo.RemotePort = gs.tcpclt_rmt_port;
	//pTcpCltParm->ConnInfo.LocalPort = gs.tcpclt_lc_port;
	pTcpCltParm->ConnInfo.LocalIp = gs.ipaddr;
	pTcpCltParm->ConnInfo.bTcpType = AK_TRUE;
	
    MenuStructInit(&pTcpCltParm->IconExplorer);
	IconExplorer_SetTitleText(&pTcpCltParm->IconExplorer, 
		Res_GetStringByID(eRES_STR_TCP_CLIENT), ICONEXPLORER_TITLE_TEXTCOLOR);         // TCP 客户端
	IconExplorer_SetSortIdCallBack(&pTcpCltParm->IconExplorer, IconExplorer_SortIdCallback);
#endif
}

void exittcp_client(void)
{
#ifdef SUPPORT_NETWORK
	gs.tcpclt_rmt_ip = pTcpCltParm->ConnInfo.RemoteIp;
	gs.tcpclt_rmt_port = pTcpCltParm->ConnInfo.RemotePort;
	//gs.tcpclt_lc_port = pTcpCltParm->ConnInfo.LocalPort;


	Tcp_Client_CloseAll();

    IconExplorer_Free(&pTcpCltParm->IconExplorer);
    pTcpCltParm = Fwl_Free(pTcpCltParm);
#endif
}

void painttcp_client(void)
{
#ifdef SUPPORT_NETWORK

    IconExplorer_Show(&pTcpCltParm->IconExplorer);

    Fwl_RefreshDisplay();
#endif
}

unsigned char handletcp_client(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_eBACK_STATE IconExplorerRet;
	T_U32 focusId = 0;


    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pTcpCltParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

	if (M_EVT_3 == event)
	{
		Tcp_Client_SetItem();
	}

	if (M_EVT_EXIT == event)
	{
		Tcp_Client_UpdateItem();
	}

	Tcp_Client_CheckState();

    IconExplorerRet = IconExplorer_Handler(&pTcpCltParm->IconExplorer, event, pEventParm);

	focusId = IconExplorer_GetItemFocusId(&pTcpCltParm->IconExplorer);
	
    switch (IconExplorerRet)
    {
    case eNext:
        switch (focusId)
        {
        case 10: //remote ip set
			pEventParm->p.pParam1 = &pTcpCltParm->ConnInfo.RemoteIp;
			m_triggerEvent(M_EVT_1, pEventParm);
            break;

		case 20: //remote port set
			pEventParm->p.pParam1 = &pTcpCltParm->ConnInfo.RemotePort;
			m_triggerEvent(M_EVT_2, pEventParm);
            break;

		//case 30: //local port set
			//pEventParm->p.pParam1 = &pTcpCltParm->ConnInfo.LocalPort;
			//m_triggerEvent(M_EVT_3, pEventParm);
            //break;

		case 40: //connect
			Tcp_Client_TryConnect();
            break;

        default:
			if ((focusId >= TCP_CLIENT_CHANNEL_START_ID) && (focusId < TCP_CLIENT_CHANNEL_START_ID+NT_CHANNEL_MAX))
			{
				pEventParm->p.pParam1 = pTcpCltParm->pNetTrans;
				pEventParm->w.Param2 = focusId - TCP_CLIENT_CHANNEL_START_ID;
				m_triggerEvent(M_EVT_NETWORK_TRANS, pEventParm);
			}
            break;
        }
        break;
		
	case eMenu:
		if ((focusId >= TCP_CLIENT_CHANNEL_START_ID) && (focusId < TCP_CLIENT_CHANNEL_START_ID+NT_CHANNEL_MAX))
		{
			T_U32 count = 0;
			
			Fwl_Print(C3, M_NETWORK, "Tcp Client Close Channel:%d!", focusId - TCP_CLIENT_CHANNEL_START_ID);
			Fwl_Net_Conn_Close(pTcpCltParm->pNetTrans[focusId - TCP_CLIENT_CHANNEL_START_ID]->pNetConn);

			while ((!pTcpCltParm->pNetTrans[focusId - TCP_CLIENT_CHANNEL_START_ID]->bRecvExit)
				&& (count < 10))
			{
				AK_Sleep(10);
				count++;
			}
			
			Fwl_Net_Conn_Delete(pTcpCltParm->pNetTrans[focusId - TCP_CLIENT_CHANNEL_START_ID]->pNetConn);
			pTcpCltParm->pNetTrans[focusId - TCP_CLIENT_CHANNEL_START_ID] = NetTrans_Free(pTcpCltParm->pNetTrans[focusId - TCP_CLIENT_CHANNEL_START_ID]);

			IconExplorer_DelItem(&pTcpCltParm->IconExplorer, focusId);
		}
		break;
		
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}

