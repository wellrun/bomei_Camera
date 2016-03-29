#include "Fwl_public.h"
#ifdef SUPPORT_NETWORK

#include "Ctl_MsgBox.h"
#include "Lib_state.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "ctl_net_trans.h"
#include "eng_font.h"
#include "eng_string_uc.h"
#include "eng_string.h"
#include "eng_akbmp.h"
#include "Eng_KeyMapping.h"
#include "eng_time.h"
#include "eng_screensave.h"
#include "fwl_oscom.h"


#define NT_LOCAL_ADDR_STR_TOP	26
#define NT_REMOTE_ADDR_STR_TOP	44


#define NT_SEND_STR_TOP			60
#define NT_SEND_STR_LEFT		20

#define NT_RECV_STR_TOP			85
#define NT_RECVFILE_IMG_LEFT	20
#define NT_RECVFILE_STR_LEFT	40
#define NT_RECVDATA_IMG_LEFT	120
#define NT_RECVDATA_STR_LEFT	140

#define NT_INFO_RECT_TOP		105
#define NT_INFO_RECT_LEFT		20
#define NT_INFO_RECT_HEIGHT		110
#define NT_INFO_RECT_WIDTH		(Fwl_GetLcdWidth() - 2*NT_INFO_RECT_LEFT)

#define NT_INFO_STRLINE_HEIGHT	18
#define NT_INFO_MAX_STRLINE		6

#define NT_RECVED_STR_TOP		220
#define NT_RECVED_STR_LEFT		0

#define NT_SENT_STR_TOP			NT_RECVED_STR_TOP


typedef struct {
	T_pCDATA    		pBkImg;  
	T_pCDATA    		pRadioImg;  
	T_pCDATA    		pRadioSetupImg;  
	T_MSGBOX        	msgbox;

	T_WSTR_100			pStr[NT_INFO_MAX_STRLINE];
	
	T_NET_TRANS			**ppNetTrans;
	T_hSemaphore		semaphore;
	
	T_BOOL 				bRefresh;
	T_BOOL 				bSendSelect;
	T_U32				focusChnId;
} T_NETWORK_TRANS_PARM;


static T_NETWORK_TRANS_PARM *pNetworkTransParm;


static T_U8 Network_Trans_GetEmpStrId(T_U8 LineNeed)
{
	T_U8 i = 0;
	T_U8 j = NT_INFO_MAX_STRLINE;
	T_U8 k = 0;

	AK_ASSERT_VAL(LineNeed<=NT_INFO_MAX_STRLINE, "Network_Trans_GetEmpStrId(): LineNeed error", j);
	
	if (AK_NULL == pNetworkTransParm)
	{
		return j;
	}

	for (i=0; i<NT_INFO_MAX_STRLINE; i++)
	{
		if (0 == pNetworkTransParm->pStr[i][0])
		{
			j = i;
			break;
		}
	}

	if (j > NT_INFO_MAX_STRLINE - LineNeed)
	{
		k = j - (NT_INFO_MAX_STRLINE - LineNeed);
		
		for (i=0; i<NT_INFO_MAX_STRLINE - LineNeed; i++)
		{
			Utl_UStrCpy(pNetworkTransParm->pStr[i], pNetworkTransParm->pStr[i+k]);
		}

		for (i=NT_INFO_MAX_STRLINE-LineNeed; i<NT_INFO_MAX_STRLINE; i++)
		{
			memset(pNetworkTransParm->pStr[i], 0, 100);
		}

		j = NT_INFO_MAX_STRLINE-LineNeed;
	}

	return j;
}


T_BOOL Network_Trans_DataSet(T_U8* pData, T_U32 len)
{
	T_BOOL ret = AK_FALSE;
	T_U8 id = NT_INFO_MAX_STRLINE;
	T_U16 *pStr = AK_NULL;
	T_U32 width = 0;
	T_U8 lineneed = 0;
	T_U16 dispNum = 0;
	T_U16 remain = 0;
	T_WSTR_100 tmpstr[NT_INFO_MAX_STRLINE] = {0};
	T_U8 i = 0;
	

	AK_ASSERT_PTR(pData, "Network_Trans_DataSet(): pData error", ret);

	if (AK_NULL == pNetworkTransParm)
	{
		return ret;
	}

	pStr = Fwl_Malloc(len * sizeof(T_U16) + 2);
	
	if (AK_NULL == pStr)
	{
		Fwl_Print(C2, M_NETWORK, "pStr malloc failed!");
		return ret;
	}

	memset(pStr, 0, sizeof(T_U16) + 2);
	
	Eng_StrMbcs2Ucs(pData, pStr);

	width = UGetSpeciStringWidth(pStr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pStr));

	if (width <= NT_INFO_RECT_WIDTH)
	{
		lineneed = 1;
		Utl_UStrCpy(tmpstr[0], pStr);
	}
	else
	{
		remain = (T_U16)Utl_UStrLen(pStr);
		
		for (i=0; (i<NT_INFO_MAX_STRLINE)&&(remain>0); i++)
		{
			dispNum = Fwl_GetUStringDispNum(pStr, remain, NT_INFO_RECT_WIDTH, CURRENT_FONT_SIZE);
			Utl_UStrCpyN(tmpstr[i], pStr, dispNum);
			lineneed++;
			pStr += dispNum;
			remain -= dispNum;
		}
	}

	AK_Obtain_Semaphore(pNetworkTransParm->semaphore, AK_SUSPEND);
	
	id = Network_Trans_GetEmpStrId(lineneed);

	if (id < NT_INFO_MAX_STRLINE)
	{
		for (i=0; i<lineneed; i++)
		{
			Utl_UStrCpy(pNetworkTransParm->pStr[id + i], tmpstr[i]);
		}
		
		pNetworkTransParm->bRefresh = AK_TRUE;
	}
	
	AK_Release_Semaphore(pNetworkTransParm->semaphore);

	pStr = Fwl_Free(pStr);
		
	return AK_TRUE;
}

T_BOOL Network_Trans_InfoSet(T_NET_TRANS_INFO *pTransInfo)
{
	T_BOOL ret = AK_FALSE;
	T_U8 id = NT_INFO_MAX_STRLINE;
	T_WSTR_20 tmpstr = {0};
	T_USTR_FILE name = {0};
	T_U16 strDate[30] = {0};
	T_U16 strTime[30] = {0};
	T_U32 width = 0;
	T_U8 lineneed = 0;
	T_U16 dispNum = 0;
	T_U16 remain = 0;
	T_U8 i = 0;
	T_WSTR_100 linestr[NT_INFO_MAX_STRLINE] = {0};
	T_WSTR_800 dispstr = {0};
	T_U16 *pStr = AK_NULL;
	
	AK_ASSERT_PTR(pTransInfo, "Network_Trans_InfoSet(): pTransInfo error", ret);

	if (AK_NULL == pNetworkTransParm)
	{
		return ret;
	}

	if (pTransInfo->bSend)
	{
		Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_SENT));
	}
	else
	{
		Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_RECEIVED));
	}

	Eng_StrMbcs2Ucs(pTransInfo->filename, name);
	Utl_UStrCat(dispstr, name);

	Eng_StrMbcs2Ucs(",", tmpstr);
	Utl_UStrCat(dispstr, tmpstr);
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_TOTAL));
	Utl_UItoa(pTransInfo->filelen, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_EXPLORER_UNIT));

	Eng_StrMbcs2Ucs(",", tmpstr);
	Utl_UStrCat(dispstr, tmpstr);

	width = UGetSpeciStringWidth(dispstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));

	if (width <= NT_INFO_RECT_WIDTH)
	{
		lineneed = 1;
		Utl_UStrCpy(linestr[0], dispstr);
	}
	else
	{
		remain = (T_U16)Utl_UStrLen(dispstr);
		pStr = dispstr;
		
		for (i=0; (i<NT_INFO_MAX_STRLINE)&&(remain>0); i++)
		{
			dispNum = Fwl_GetUStringDispNum(pStr, remain, NT_INFO_RECT_WIDTH, CURRENT_FONT_SIZE);
			Utl_UStrCpyN(linestr[i], pStr, dispNum);
			lineneed++;
			pStr += dispNum;
			remain -= dispNum;
		}
	}

	lineneed += 1;//starttime and stoptime
	

	AK_Obtain_Semaphore(pNetworkTransParm->semaphore, AK_SUSPEND);

	id = Network_Trans_GetEmpStrId(lineneed);

	if (id < NT_INFO_MAX_STRLINE)
	{
		for (i=0; i<lineneed-1; i++)
		{
			Utl_UStrCpy(pNetworkTransParm->pStr[id + i], linestr[i]);
		}

		ConvertTimeS2UcSByFormat(&pTransInfo->starttime, strDate, strTime);

		Utl_UStrCpy(pNetworkTransParm->pStr[id+i], strDate);

		Eng_StrMbcs2Ucs(" ", tmpstr);
		Utl_UStrCat(pNetworkTransParm->pStr[id+i], tmpstr);
		
		Utl_UStrCat(pNetworkTransParm->pStr[id+i], strTime);

		Utl_UStrCat(pNetworkTransParm->pStr[id+i], Res_GetStringByID(eRES_STR_TO));
		ConvertTimeS2UcSByFormat(&pTransInfo->stoptime, strDate, strTime);
		
		Utl_UStrCat(pNetworkTransParm->pStr[id+i], strDate);

		Eng_StrMbcs2Ucs(" ", tmpstr);
		Utl_UStrCat(pNetworkTransParm->pStr[id+i], tmpstr);
		Utl_UStrCat(pNetworkTransParm->pStr[id+i], strTime);

		pNetworkTransParm->bRefresh = AK_TRUE;
	}

	AK_Release_Semaphore(pNetworkTransParm->semaphore);
		
	return AK_TRUE;
}

static T_BOOL Network_Trans_SetInfoCurChn(T_VOID)
{
	T_NET_TRANS *pTrans = AK_NULL;
	T_U32 i = 0;
	
	if (AK_NULL == pNetworkTransParm)
	{
		return AK_FALSE;
	}

	pTrans = pNetworkTransParm->ppNetTrans[pNetworkTransParm->focusChnId];

	if (AK_NULL == pTrans)
	{
		return AK_FALSE;
	}

	for (i=0; i<NT_INFO_MAX_STRLINE; i++)
	{
		memset(pNetworkTransParm->pStr[i], 0, 100);
	}

	for (i=0; i<NT_KEEP_INFO_MAX; i++)
	{
		if (0 != pTrans->transInfo[i].filename[0])
		{
			Network_Trans_InfoSet(&pTrans->transInfo[i]);
		}
	}
	
	pNetworkTransParm->bRefresh = AK_TRUE;
	return AK_TRUE;
}

static T_BOOL Network_Trans_ChangeChannel(T_VOID)
{
	T_NET_TRANS *pTrans = AK_NULL;
	T_U32 count = 0;
	
	if (AK_NULL == pNetworkTransParm)
	{
		return AK_FALSE;
	}

	pTrans = pNetworkTransParm->ppNetTrans[pNetworkTransParm->focusChnId];

	if (AK_NULL != pTrans)
	{
		pTrans->bShowing = AK_FALSE;
	}

	do
	{
		pNetworkTransParm->focusChnId += 1;
		pNetworkTransParm->focusChnId %= NT_CHANNEL_MAX;
		pTrans = pNetworkTransParm->ppNetTrans[pNetworkTransParm->focusChnId];
		count++;
	}while((AK_NULL == pTrans) && (count<NT_CHANNEL_MAX));

	if (AK_NULL != pTrans)
	{
		pTrans->bShowing = AK_TRUE;
		Network_Trans_SetInfoCurChn();

		pNetworkTransParm->bRefresh = AK_TRUE;
		Fwl_Print(C3, M_NETWORK, "Network_Trans_ChangeChannel OK!");
		return AK_TRUE;
	}

	Fwl_Print(C3, M_NETWORK, "Network_Trans_ChangeChannel Failed!");

	return AK_FALSE;
}

static T_BOOL Network_Trans_CheckState(T_VOID)
{
	T_U32 i = 0;
	T_NET_TRANS *pTrans = AK_NULL;
	T_BOOL bFlag = AK_FALSE;
	
	if (AK_NULL == pNetworkTransParm)
	{
		return AK_FALSE;
	}

	for (i=0; i<NT_CHANNEL_MAX; i++)
	{
		pTrans = pNetworkTransParm->ppNetTrans[i];
		
		if (AK_NULL != pTrans)
		{
			if (NET_CLOSE_FLAG_CLOSED == pTrans->pNetConn->closeflag)
			{
				Fwl_Print(C3, M_NETWORK, "channel %d close!", i);
				Fwl_Net_Conn_Delete(pTrans->pNetConn);

				if (pTrans->bShowing)
				{
					bFlag = AK_TRUE;
				}
				
				pTrans = NetTrans_Free(pTrans);
				pNetworkTransParm->ppNetTrans[i] = AK_NULL;

				if (bFlag)
				{
					if (!Network_Trans_ChangeChannel())
					{
						Fwl_Print(C3, M_NETWORK, "No channel, exit!");
						m_triggerEvent(M_EVT_EXIT, AK_NULL);
					}
					
					bFlag = AK_FALSE;
				}
			}
		}
	}

	return AK_TRUE;
}

static T_VOID Network_Trans_Show(T_VOID)
{
	T_U32 width = 0;
	T_POS left = 0;
	T_WSTR_100 dispstr = {0};
	T_WSTR_20 tmpstr = {0};
	T_U8		i = 0;
	T_NET_TRANS *pTrans = AK_NULL;

	if (AK_NULL == pNetworkTransParm)
	{
		return;
	}

	pTrans = pNetworkTransParm->ppNetTrans[pNetworkTransParm->focusChnId];

	if (AK_NULL == pTrans)
	{
		return;
	}

	if (!pNetworkTransParm->bRefresh)
	{
		return;
	}

	Fwl_AkBmpDrawFromString(HRGB_LAYER, 0, TOP_BAR_HEIGHT, \
                    pNetworkTransParm->pBkImg, &g_Graph.TransColor, AK_FALSE);

	if (pTrans->pNetConn->info.bTcpType)
	{
		Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_TCP));
	}
	else
	{
		Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_UDP));
	}

	Eng_StrMbcs2Ucs("  ", tmpstr);
	Utl_UStrCat(dispstr, tmpstr);

	//本机
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_LOCAL));
	Fwl_Net_Ip2str(pTrans->pNetConn->info.LocalIp, tmpstr);

	Utl_UStrCat(dispstr, tmpstr);
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR2));

	Utl_UItoa(pTrans->pNetConn->info.LocalPort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);

	width = UGetSpeciStringWidth(dispstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));
	left = (T_POS)((Fwl_GetLcdWidth() - width) >> 1);
	
	Fwl_UDispSpeciString(HRGB_LAYER, left, NT_LOCAL_ADDR_STR_TOP, dispstr,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));

	//远端
	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE));
	Fwl_Net_Ip2str(pTrans->pNetConn->info.RemoteIp, tmpstr);

	Utl_UStrCat(dispstr, tmpstr);
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR2));

	Utl_UItoa(pTrans->pNetConn->info.RemotePort, tmpstr, 10);
	Utl_UStrCat(dispstr, tmpstr);


	width = UGetSpeciStringWidth(dispstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));
	left = (T_POS)((Fwl_GetLcdWidth() - width) >> 1);
	
	Fwl_UDispSpeciString(HRGB_LAYER, left, NT_REMOTE_ADDR_STR_TOP, dispstr,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));


	//发送文件
	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_SEND_FILE));

	width = UGetSpeciStringWidth(dispstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));

	if (pNetworkTransParm->bSendSelect)
	{
		Fwl_FillRect(HRGB_LAYER, NT_SEND_STR_LEFT - 2, NT_SEND_STR_TOP - 2, width + 4, 20, COLOR_LIGHT_BLUE);
	}
	else
	{
		Fwl_FillRect(HRGB_LAYER, NT_SEND_STR_LEFT - 2, NT_SEND_STR_TOP - 2, width + 4, 20, COLOR_SLATEGREY);
	}

	width = UGetSpeciStringWidth(dispstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));
	
	Fwl_DrawRect(HRGB_LAYER, NT_SEND_STR_LEFT - 2, NT_SEND_STR_TOP - 2, width + 4, 20, COLOR_WHITE);

	Fwl_UDispSpeciString(HRGB_LAYER, NT_SEND_STR_LEFT, NT_SEND_STR_TOP, dispstr,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));

	

	//接收文件
	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_RECV_FILE));

	if (pTrans->bRecvdata)
	{
		Fwl_AkBmpDrawFromString(HRGB_LAYER, NT_RECVFILE_IMG_LEFT, NT_RECV_STR_TOP, \
							pNetworkTransParm->pRadioImg, &g_Graph.TransColor, AK_FALSE);
	}
	else
	{
		Fwl_AkBmpDrawFromString(HRGB_LAYER, NT_RECVFILE_IMG_LEFT, NT_RECV_STR_TOP, \
							pNetworkTransParm->pRadioSetupImg, &g_Graph.TransColor, AK_FALSE);
	}

	Fwl_UDispSpeciString(HRGB_LAYER, NT_RECVFILE_STR_LEFT, NT_RECV_STR_TOP, dispstr,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));
	

	//接收数据
	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_RECV_DATA));

	if (pTrans->bRecvdata)
	{
		Fwl_AkBmpDrawFromString(HRGB_LAYER, NT_RECVDATA_IMG_LEFT, NT_RECV_STR_TOP, \
							pNetworkTransParm->pRadioSetupImg, &g_Graph.TransColor, AK_FALSE);
	}
	else
	{
		Fwl_AkBmpDrawFromString(HRGB_LAYER, NT_RECVDATA_IMG_LEFT, NT_RECV_STR_TOP, \
							pNetworkTransParm->pRadioImg, &g_Graph.TransColor, AK_FALSE);
	}

	Fwl_UDispSpeciString(HRGB_LAYER, NT_RECVDATA_STR_LEFT, NT_RECV_STR_TOP, dispstr,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));
	

	//传输信息显示框
	Fwl_DrawRect(HRGB_LAYER, NT_INFO_RECT_LEFT-2, NT_INFO_RECT_TOP-2, NT_INFO_RECT_WIDTH+4, NT_INFO_RECT_HEIGHT+2, COLOR_WHITE);

	AK_Obtain_Semaphore(pNetworkTransParm->semaphore, AK_SUSPEND);

	for (i=0; i<NT_INFO_MAX_STRLINE; i++)
	{
		if (0 != pNetworkTransParm->pStr[i][0])
		{
			Fwl_UDispSpeciString(HRGB_LAYER, NT_INFO_RECT_LEFT, NT_INFO_RECT_TOP + i*NT_INFO_STRLINE_HEIGHT, pNetworkTransParm->pStr[i],
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pNetworkTransParm->pStr[i]));
		}
	}

	AK_Release_Semaphore(pNetworkTransParm->semaphore);

	//已接收
	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_RECEIVED));
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR2));

	if (pTrans->recvTotal >= (1<<30))
	{
		Utl_UItoa((pTrans->recvTotal >> 20), tmpstr, 10);
		Utl_UStrCat(dispstr, tmpstr);
		Eng_StrMbcs2Ucs("M", tmpstr);
		Utl_UStrCat(dispstr, tmpstr);
		Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_EXPLORER_UNIT));
	}
	else if (pTrans->recvTotal >= (1<<20))
	{
		Utl_UItoa((pTrans->recvTotal >> 10), tmpstr, 10);
		Utl_UStrCat(dispstr, tmpstr);
		Eng_StrMbcs2Ucs("K", tmpstr);
		Utl_UStrCat(dispstr, tmpstr);
		Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_EXPLORER_UNIT));
	}
	else
	{
		Utl_UItoa(pTrans->recvTotal, tmpstr, 10);
		Utl_UStrCat(dispstr, tmpstr);
		Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_EXPLORER_UNIT));
	}

	Fwl_UDispSpeciString(HRGB_LAYER, NT_RECVED_STR_LEFT, NT_RECVED_STR_TOP, dispstr,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));

	//已发送
	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_SENT));
	Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_CLK_DAY_SEPARATOR2));

	if (pTrans->sendTotal >= (1<<30))
	{
		Utl_UItoa((pTrans->sendTotal >> 20), tmpstr, 10);
		Utl_UStrCat(dispstr, tmpstr);
		Eng_StrMbcs2Ucs("M", tmpstr);
		Utl_UStrCat(dispstr, tmpstr);
		Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_EXPLORER_UNIT));
	}
	else if (pTrans->sendTotal >= (1<<20))
	{
		Utl_UItoa((pTrans->sendTotal >> 10), tmpstr, 10);
		Utl_UStrCat(dispstr, tmpstr);
		Eng_StrMbcs2Ucs("K", tmpstr);
		Utl_UStrCat(dispstr, tmpstr);
		Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_EXPLORER_UNIT));
	}
	else
	{
		Utl_UItoa(pTrans->sendTotal, tmpstr, 10);
		Utl_UStrCat(dispstr, tmpstr);
		Utl_UStrCat(dispstr, Res_GetStringByID(eRES_STR_EXPLORER_UNIT));
	}

	width = UGetSpeciStringWidth(dispstr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));
	left = Fwl_GetLcdWidth() - width;

	Fwl_UDispSpeciString(HRGB_LAYER, left, NT_SENT_STR_TOP, dispstr,
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(dispstr));
	
	pNetworkTransParm->bRefresh = AK_FALSE;
}


#endif
/*---------------------- BEGIN OF STATE s_network_data_trans ------------------------*/
void initnetwork_data_trans(void)
{
#ifdef SUPPORT_NETWORK

    pNetworkTransParm = (T_NETWORK_TRANS_PARM *)Fwl_Malloc(sizeof(T_NETWORK_TRANS_PARM));
    AK_ASSERT_PTR_VOID(pNetworkTransParm, "initnetwork_data_trans(): malloc error");
	memset(pNetworkTransParm, 0, sizeof(T_NETWORK_TRANS_PARM));

	pNetworkTransParm->bRefresh = AK_TRUE;
	pNetworkTransParm->pBkImg = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_MAIN_BACKGROUND, AK_NULL);
	pNetworkTransParm->pRadioImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_RADIOBT_NOTSETUP, AK_NULL);
    pNetworkTransParm->pRadioSetupImg = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_BMP_MULTISET_RADIOBT_SETUP, AK_NULL);
	pNetworkTransParm->semaphore = AK_Create_Semaphore(1, AK_PRIORITY);
	
	TopBar_SetTitle(Res_GetStringByID(eRES_STR_NETWORK_TRANS));
	TopBar_Show(TB_REFRESH_ALL);
	ScreenSaverDisable();
#endif
}

void exitnetwork_data_trans(void)
{
#ifdef SUPPORT_NETWORK
	AK_Delete_Semaphore(pNetworkTransParm->semaphore);
    pNetworkTransParm = Fwl_Free(pNetworkTransParm);
	ScreenSaverEnable();
#endif
}

void paintnetwork_data_trans(void)
{
#ifdef SUPPORT_NETWORK
	Network_Trans_Show();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handlenetwork_data_trans(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_MMI_KEYPAD    phyKey;
	T_NET_TRANS 	*pTrans = AK_NULL;


    if (IsPostProcessEvent(event))
    {
        return 1;
    }

	if (M_EVT_NETWORK_TRANS == event)
	{
		pNetworkTransParm->ppNetTrans = pEventParm->p.pParam1;
		pNetworkTransParm->focusChnId = pEventParm->w.Param2;

		pTrans = pNetworkTransParm->ppNetTrans[pNetworkTransParm->focusChnId];

		if (AK_NULL == pTrans)
		{
			return 0;
		}
		
		pTrans->bShowing = AK_TRUE;

		Network_Trans_SetInfoCurChn();
		return 0;
	}

	Network_Trans_CheckState();

	pTrans = pNetworkTransParm->ppNetTrans[pNetworkTransParm->focusChnId];

	if (AK_NULL == pTrans)
	{
		return 0;
	}

	if (pTrans->bSending || pTrans->bRecving)
	{
		pNetworkTransParm->bRefresh = AK_TRUE;
	}

	if (M_EVT_USER_KEY == event)
	{
		phyKey.keyID = (T_eKEY_ID)pEventParm->c.Param1;
        phyKey.pressType = (T_BOOL)pEventParm->c.Param2;

		switch(phyKey.keyID)
		{
		case kbCLEAR:
			if (PRESS_SHORT == phyKey.pressType)
			{
				m_triggerEvent(M_EVT_EXIT, pEventParm);
			}
			break;
		case kbOK:
			if (PRESS_SHORT == phyKey.pressType)
			{
				if (pNetworkTransParm->bSendSelect)
				{
					Fwl_Print(C3, M_NETWORK, "send file!");

					if (pTrans->bSending)
					{
						T_RECT msgRect;
						
						Fwl_Print(C3, M_NETWORK, "Sending, please wait for a while!");
						MsgBox_InitStr(&pNetworkTransParm->msgbox, 0, GetCustomTitle(ctHINT),\
				        	Res_GetStringByID(eRES_STR_CHN_SENDING), MSGBOX_INFORMATION);
				        MsgBox_Show(&pNetworkTransParm->msgbox);
				        MsgBox_GetRect(&pNetworkTransParm->msgbox, &msgRect);
				        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
				        Fwl_MiniDelay(1000);
						pNetworkTransParm->bSendSelect = AK_FALSE;
						break;
					}
					NetTrans_TrySend(pTrans);
					pNetworkTransParm->bSendSelect = AK_FALSE;
				}
			}
			break;
		case kbLEFT:
			if (PRESS_SHORT == phyKey.pressType)
			{
				if (pTrans->bRecving)
				{
					T_RECT msgRect;
					
					Fwl_Print(C3, M_NETWORK, "Receiving, don't modify receive type!");
					MsgBox_InitStr(&pNetworkTransParm->msgbox, 0, GetCustomTitle(ctHINT),\
			        	Res_GetStringByID(eRES_STR_CHN_RECVING), MSGBOX_INFORMATION);
			        MsgBox_Show(&pNetworkTransParm->msgbox);
			        MsgBox_GetRect(&pNetworkTransParm->msgbox, &msgRect);
			        Fwl_InvalidateRect(msgRect.left, msgRect.top, msgRect.width, msgRect.height);
			        Fwl_MiniDelay(1000);
					
					break;
				}
				
				pTrans->bRecvdata = !pTrans->bRecvdata;
				pNetworkTransParm->bRefresh = AK_TRUE;
			}
			break;

		case kbRIGHT:
			if (PRESS_SHORT == phyKey.pressType)
			{
				Network_Trans_ChangeChannel();
			}
			break;
		case kbUP:
		case kbDOWN:
			if (PRESS_SHORT == phyKey.pressType)
			{				
				pNetworkTransParm->bSendSelect = !pNetworkTransParm->bSendSelect;
				pNetworkTransParm->bRefresh = AK_TRUE;
			}
			break;
		default:
			break;
		}
	}
	


#endif
    return 0;
}

