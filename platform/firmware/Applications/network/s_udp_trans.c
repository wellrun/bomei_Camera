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

#define UDP_CHANNEL_START_ID			50

typedef struct {
    T_ICONEXPLORER  	IconExplorer;
    T_MSGBOX        	msgbox;
	T_CONNECT_INFO		ConnInfo;
	T_NET_TRANS			*pNetTrans[NT_CHANNEL_MAX];
} T_UDP_TRANS_PARM;

static T_UDP_TRANS_PARM *pUdpTransParm;


static T_VOID Udp_SetItem(T_VOID)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pUdpTransParm)
	{
		return;
	}

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE_IP));
	Fwl_Net_Ip2str(pUdpTransParm->ConnInfo.RemoteIp, tmpstr);
	Utl_UStrCat(dispstr, tmpstr);
	
	IconExplorer_AddItemWithOption(&pUdpTransParm->IconExplorer, 10, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//remote ip set

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE_PORT));
	Utl_UItoa(pUdpTransParm->ConnInfo.RemotePort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	IconExplorer_AddItemWithOption(&pUdpTransParm->IconExplorer, 20, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//remote port set


	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_LOCAL_PORT));
	Utl_UItoa(pUdpTransParm->ConnInfo.LocalPort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	IconExplorer_AddItemWithOption(&pUdpTransParm->IconExplorer, 30, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//local port set

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_SETUP));

	IconExplorer_AddItemWithOption(&pUdpTransParm->IconExplorer, 40, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);//setup
}

static T_VOID Udp_UpdateItem(T_VOID)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pUdpTransParm)
	{
		return;
	}
	
	IconExplorer_DelItem(&pUdpTransParm->IconExplorer, 10);
	IconExplorer_DelItem(&pUdpTransParm->IconExplorer, 20);
	IconExplorer_DelItem(&pUdpTransParm->IconExplorer, 30);

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE_IP));
	Fwl_Net_Ip2str(pUdpTransParm->ConnInfo.RemoteIp, tmpstr);
	Utl_UStrCat(dispstr, tmpstr);
	
	IconExplorer_AddItemWithOption(&pUdpTransParm->IconExplorer, 10, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//remote ip set

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE_PORT));
	Utl_UItoa(pUdpTransParm->ConnInfo.RemotePort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	IconExplorer_AddItemWithOption(&pUdpTransParm->IconExplorer, 20, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//remote port set


	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_LOCAL_PORT));
	Utl_UItoa(pUdpTransParm->ConnInfo.LocalPort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	IconExplorer_AddItemWithOption(&pUdpTransParm->IconExplorer, 30, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);//local port set

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_SETUP));

	IconExplorer_AddItemWithOption(&pUdpTransParm->IconExplorer, 40, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);//setup
}


static T_VOID Udp_AddChannelItem(T_U32 id)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pUdpTransParm)
	{
		return;
	}

	Fwl_Net_Ip2str(pUdpTransParm->ConnInfo.RemoteIp, dispstr);
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR2));

	Utl_UItoa(pUdpTransParm->ConnInfo.RemotePort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	Eng_StrMbcs2Ucs("  ", tmpstr);
	Utl_UStrCat(dispstr, tmpstr);

	Fwl_Net_Ip2str(pUdpTransParm->ConnInfo.LocalIp, tmpstr);
	Utl_UStrCat(dispstr, tmpstr);
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR2));

	Utl_UItoa(pUdpTransParm->ConnInfo.LocalPort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	IconExplorer_AddItemWithOption(&pUdpTransParm->IconExplorer, UDP_CHANNEL_START_ID + id, AK_NULL, 0, dispstr, \
					AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);
}

static T_BOOL Udp_Setup(T_U32 id)
{
	T_BOOL ret = AK_FALSE;
	
	AK_ASSERT_VAL(id < NT_CHANNEL_MAX, "Udp_Setup(): id err", AK_FALSE);
	AK_ASSERT_PTR(pUdpTransParm, "Udp_Setup(): pUdpTransParm err", AK_FALSE);

	pUdpTransParm->pNetTrans[id] = NetTrans_Init();
	
	if (AK_NULL == pUdpTransParm->pNetTrans[id])
	{
		Fwl_Print(C2, M_NETWORK, "NetTrans_Init failed!");
		return AK_FALSE;
	}

	pUdpTransParm->pNetTrans[id]->pNetConn = Fwl_Net_Conn_New(NETWORKCONN_UDP);

	if (AK_NULL == pUdpTransParm->pNetTrans[id]->pNetConn)
	{
		Fwl_Print(C2, M_NETWORK, "Fwl_Net_Conn_New failed!");
		return AK_FALSE;
	}

	Fwl_Print(C3, M_NETWORK, "Fwl_Net_Conn_New OK!");

	ret = Fwl_Net_Conn_Bind(pUdpTransParm->pNetTrans[id]->pNetConn, 0, pUdpTransParm->ConnInfo.LocalPort);

	if (!ret)
	{
		Fwl_Print(C2, M_NETWORK, "Fwl_Net_Conn_Bind failed!");
		return AK_FALSE;
	}

	Fwl_Print(C3, M_NETWORK, "Fwl_Net_Conn_Bind OK!");

	memcpy(&pUdpTransParm->pNetTrans[id]->pNetConn->info, &pUdpTransParm->ConnInfo, sizeof(T_CONNECT_INFO));

	ret = NetTrans_CreateRecvTask(pUdpTransParm->pNetTrans[id]);

	if (ret)
	{
		Udp_AddChannelItem(id);
	}
	
	return ret;
}

static T_BOOL Udp_TrySetup(T_VOID)
{
	T_BOOL ret = AK_FALSE;
	T_U32 id = NT_CHANNEL_MAX;
	T_U32 i = 0;
	T_RECT msgRect;
	
	AK_ASSERT_PTR(pUdpTransParm, "Udp_TrySetup(): pUdpTransParm err", AK_FALSE);

	for (i=0; i<NT_CHANNEL_MAX; i++)
	{
		if (AK_NULL == pUdpTransParm->pNetTrans[i])
		{
			id = i;
			break;
		}
	}

	if (id >= NT_CHANNEL_MAX)
	{
		Fwl_Print(C3, M_NETWORK, "Udp_TrySetup no empty id!");
		MsgBox_InitStr(&pUdpTransParm->msgbox, 0, GetCustomTitle(ctHINT),\
        	Res_GetStringByID(eRES_STR_ACHIEVED_MAX_CHANNEL), MSGBOX_INFORMATION);
        MsgBox_Show(&pUdpTransParm->msgbox);
        MsgBox_GetRect(&pUdpTransParm->msgbox, &msgRect);
        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
        Fwl_MiniDelay(1000);
		
		return AK_FALSE;
	}

	for (i=0; i<NT_CHANNEL_MAX; i++)
	{
		if ((i != id) && (AK_NULL != pUdpTransParm->pNetTrans[i]))
		{
			if (0 == memcmp(&pUdpTransParm->pNetTrans[i]->pNetConn->info, &pUdpTransParm->ConnInfo, sizeof(T_CONNECT_INFO)))
			{
				//Same channel is already here
				Fwl_Print(C3, M_NETWORK, "Same channel is already here!");
				MsgBox_InitStr(&pUdpTransParm->msgbox, 0, GetCustomTitle(ctHINT),\
		        	Res_GetStringByID(eRES_STR_CHANNEL_EXIST), MSGBOX_INFORMATION);
		        MsgBox_Show(&pUdpTransParm->msgbox);
		        MsgBox_GetRect(&pUdpTransParm->msgbox, &msgRect);
		        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
		        Fwl_MiniDelay(1000);
				
				return AK_FALSE;
			}
		}
	}

	ret = Udp_Setup(id);

	if (!ret)
	{
		Fwl_Net_Conn_Delete(pUdpTransParm->pNetTrans[id]->pNetConn);
		pUdpTransParm->pNetTrans[id] = NetTrans_Free(pUdpTransParm->pNetTrans[id]);

		MsgBox_InitStr(&pUdpTransParm->msgbox, 0, GetCustomTitle(ctHINT),\
        	Res_GetStringByID(eRES_STR_SETUP_FAILED), MSGBOX_INFORMATION);
        MsgBox_Show(&pUdpTransParm->msgbox);
        MsgBox_GetRect(&pUdpTransParm->msgbox, &msgRect);
        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
        Fwl_MiniDelay(1000);
	}
	
	return ret;
}

static T_BOOL Udp_CloseAll(T_VOID)
{
	T_U32 i = 0;
	
	if (AK_NULL == pUdpTransParm)
	{
		return AK_FALSE;
	}

	for (i=0; i<NT_CHANNEL_MAX; i++)
	{
		if (AK_NULL != pUdpTransParm->pNetTrans[i])
		{
			Fwl_Net_Conn_Delete(pUdpTransParm->pNetTrans[i]->pNetConn);
			pUdpTransParm->pNetTrans[i] = NetTrans_Free(pUdpTransParm->pNetTrans[i]);
		}
	}

	return AK_TRUE;
}


#endif
/*---------------------- BEGIN OF STATE s_udp_trans ------------------------*/
void initudp_trans(void)
{
#ifdef SUPPORT_NETWORK

    pUdpTransParm = (T_UDP_TRANS_PARM *)Fwl_Malloc(sizeof(T_UDP_TRANS_PARM));
    AK_ASSERT_PTR_VOID(pUdpTransParm, "initudp_trans(): malloc error");
	memset(pUdpTransParm, 0, sizeof(T_UDP_TRANS_PARM));

	pUdpTransParm->ConnInfo.RemoteIp = gs.udp_rmt_ip;
	pUdpTransParm->ConnInfo.RemotePort = gs.udp_rmt_port;
	pUdpTransParm->ConnInfo.LocalPort = gs.udp_lc_port;
	pUdpTransParm->ConnInfo.LocalIp = gs.ipaddr;
	pUdpTransParm->ConnInfo.bTcpType = AK_FALSE;
	
    MenuStructInit(&pUdpTransParm->IconExplorer);
	IconExplorer_SetTitleText(&pUdpTransParm->IconExplorer, 
		Res_GetStringByID(eRES_STR_UDP_TRANS), ICONEXPLORER_TITLE_TEXTCOLOR);         //UDP´«Êä
	IconExplorer_SetSortIdCallBack(&pUdpTransParm->IconExplorer, IconExplorer_SortIdCallback);
#endif
}

void exitudp_trans(void)
{
#ifdef SUPPORT_NETWORK
	gs.udp_rmt_ip = pUdpTransParm->ConnInfo.RemoteIp;
	gs.udp_rmt_port = pUdpTransParm->ConnInfo.RemotePort;
	gs.udp_lc_port = pUdpTransParm->ConnInfo.LocalPort;

	Udp_CloseAll();

    IconExplorer_Free(&pUdpTransParm->IconExplorer);
    pUdpTransParm = Fwl_Free(pUdpTransParm);
#endif
}

void paintudp_trans(void)
{
#ifdef SUPPORT_NETWORK

    IconExplorer_Show(&pUdpTransParm->IconExplorer);

    Fwl_RefreshDisplay();
#endif
}

unsigned char handleudp_trans(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_eBACK_STATE IconExplorerRet;
	T_U32 focusId = 0;


    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pUdpTransParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

	if (M_EVT_4 == event)
	{
		Udp_SetItem();
	}

	if (M_EVT_EXIT == event)
	{
		Udp_UpdateItem();
	}

    IconExplorerRet = IconExplorer_Handler(&pUdpTransParm->IconExplorer, event, pEventParm);
	focusId = IconExplorer_GetItemFocusId(&pUdpTransParm->IconExplorer);

    switch (IconExplorerRet)
    {
    case eNext:
        switch (focusId)
        {
        case 10: //remote ip set
			pEventParm->p.pParam1 = &pUdpTransParm->ConnInfo.RemoteIp;
			m_triggerEvent(M_EVT_1, pEventParm);
            break;

		case 20: //remote port set
			pEventParm->p.pParam1 = &pUdpTransParm->ConnInfo.RemotePort;
			m_triggerEvent(M_EVT_2, pEventParm);
            break;

		case 30: //local port set
			pEventParm->p.pParam1 = &pUdpTransParm->ConnInfo.LocalPort;
			m_triggerEvent(M_EVT_3, pEventParm);
            break;

		case 40: //setup
			Udp_TrySetup();
            break;

        default:
			if ((focusId >= UDP_CHANNEL_START_ID) && (focusId < UDP_CHANNEL_START_ID+NT_CHANNEL_MAX))
			{
				pEventParm->p.pParam1 = pUdpTransParm->pNetTrans;
				pEventParm->w.Param2 = focusId - UDP_CHANNEL_START_ID;
				m_triggerEvent(M_EVT_NETWORK_TRANS, pEventParm);
			}
            break;
        }
        break;

	case eMenu:
		if ((focusId >= UDP_CHANNEL_START_ID) && (focusId < UDP_CHANNEL_START_ID+NT_CHANNEL_MAX))
		{
			Fwl_Print(C3, M_NETWORK, "Udp Close Channel:%d!", focusId - UDP_CHANNEL_START_ID);
			Fwl_Net_Conn_Delete(pUdpTransParm->pNetTrans[focusId - UDP_CHANNEL_START_ID]->pNetConn);
			pUdpTransParm->pNetTrans[focusId - UDP_CHANNEL_START_ID] = NetTrans_Free(pUdpTransParm->pNetTrans[focusId - UDP_CHANNEL_START_ID]);

			IconExplorer_DelItem(&pUdpTransParm->IconExplorer, focusId);
		}
		break;
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}

