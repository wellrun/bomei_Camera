#include "Fwl_public.h"
#ifdef SUPPORT_NETWORK

#include "Ctl_MsgBox.h"
#include "Lib_state.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "fwl_net.h"
#include "eng_font.h"
#include "eng_debug.h"
#include "eng_akbmp.h"
#include "eng_screensave.h"
#include "fwl_oscom.h"
#include "ping.h"




#define PING_TEST_SEND_INTERVAL 	1000

#define PING_INFO_RECT_TOP			110
#define PING_INFO_RECT_LEFT			20
#define PING_INFO_RECT_HEIGHT		120
#define PING_INFO_RECT_WIDTH		(Fwl_GetLcdWidth() - 2*PING_INFO_RECT_LEFT)

#define PING_INFO_STRLINE_HEIGHT	20
#define PING_INFO_MAX_STRLINE		6


typedef struct {
	T_ICONEXPLORER  IconExplorer;
	T_TIMER			timer;
	T_U32			remoteIp;
	T_U32			SentNum;
	T_U32			RecvNum;
	T_U32			TotalTime;
	T_WSTR_100		pStr[PING_INFO_MAX_STRLINE];
	T_hSemaphore	semaphore;
	T_pCDATA    	pBkImg;  
} T_PING_TEST_PARM;

static T_PING_TEST_PARM *pPingTestParm;

static T_VOID Ping_Test_CleanInfo(T_VOID)
{
	T_U8 i = 0;
	
	if (AK_NULL == pPingTestParm)
	{
		return;
	}
		
	pPingTestParm->SentNum = 0;
	pPingTestParm->RecvNum = 0;
	pPingTestParm->TotalTime = 0;

	for (i=0; i<PING_INFO_MAX_STRLINE; i++)
	{
		if (0 != pPingTestParm->pStr[i][0])
		{
			memset(pPingTestParm->pStr[i], 0, 100);
		}
	}
}

static T_VOID Ping_Test_Show(T_VOID)
{
	T_U8 i = 0;
	T_RECT rect = {0};
	
	if (AK_NULL == pPingTestParm)
	{
		return;
	}

	IconExplorer_Show(&pPingTestParm->IconExplorer);
	
	RectInit(&rect, PING_INFO_RECT_LEFT, PING_INFO_RECT_TOP, PING_INFO_RECT_WIDTH, PING_INFO_RECT_HEIGHT);

	Fwl_AkBmpDrawPartFromString(HRGB_LAYER, PING_INFO_RECT_LEFT, PING_INFO_RECT_TOP, &rect, 
                    pPingTestParm->pBkImg, &g_Graph.TransColor, AK_FALSE);

	
	//回复信息显示框
	Fwl_DrawRect(HRGB_LAYER, PING_INFO_RECT_LEFT-2, PING_INFO_RECT_TOP-2, PING_INFO_RECT_WIDTH+4, PING_INFO_RECT_HEIGHT+2, COLOR_WHITE);

	for (i=0; i<PING_INFO_MAX_STRLINE; i++)
	{
		if (0 != pPingTestParm->pStr[i][0])
		{
			Fwl_UDispSpeciString(HRGB_LAYER, PING_INFO_RECT_LEFT, PING_INFO_RECT_TOP + i*PING_INFO_STRLINE_HEIGHT, pPingTestParm->pStr[i],
						COLOR_WHITE, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pPingTestParm->pStr[i]));
		}
	}

}


static T_VOID Ping_Test_SetItem(T_VOID)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pPingTestParm)
	{
		return;
	}

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE_IP));
	Fwl_Net_Ip2str(pPingTestParm->remoteIp, tmpstr);
	
	Utl_UStrCat(dispstr, tmpstr);
	
	IconExplorer_AddItemWithOption(&pPingTestParm->IconExplorer, 10, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);// target ip set

	IconExplorer_AddItemWithOption(&pPingTestParm->IconExplorer, 20, AK_NULL, 0, Res_GetStringByID(eRES_STR_START), \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);//start

	IconExplorer_AddItemWithOption(&pPingTestParm->IconExplorer, 30, AK_NULL, 0, Res_GetStringByID(eRES_STR_STOP), \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NONE, AK_NULL);//stop
}

static T_VOID Ping_Test_UpdateItem(T_VOID)
{
	T_WSTR_50 dispstr = {0};
	T_WSTR_20 tmpstr = {0};

	if (AK_NULL == pPingTestParm)
	{
		return;
	}
	
	IconExplorer_DelItem(&pPingTestParm->IconExplorer, 10);

	Utl_UStrCpy(dispstr, Res_GetStringByID(eRES_STR_REMOTE_IP));
	Fwl_Net_Ip2str(pPingTestParm->remoteIp, tmpstr);
	
	Utl_UStrCat(dispstr, tmpstr);

	IconExplorer_AddItemWithOption(&pPingTestParm->IconExplorer, 10, AK_NULL, 0, dispstr, \
                    AK_NULL, AK_NULL, ICONEXPLORER_OPTION_NEXT, AK_NULL);// target ip set
}

static T_U8 Ping_Test_GetEmpStrId(T_U8 LineNeed)
{
	T_U8 i = 0;
	T_U8 j = PING_INFO_MAX_STRLINE;
	T_U8 k = 0;

	AK_ASSERT_VAL(LineNeed<=PING_INFO_MAX_STRLINE, "Ping_Test_GetEmpStrId(): LineNeed error", j);
	
	if (AK_NULL == pPingTestParm)
	{
		return j;
	}

	for (i=0; i<PING_INFO_MAX_STRLINE; i++)
	{
		if (0 == pPingTestParm->pStr[i][0])
		{
			j = i;
			break;
		}
	}

	if (j > PING_INFO_MAX_STRLINE - LineNeed)
	{
		k = j - (PING_INFO_MAX_STRLINE - LineNeed);
		
		for (i=0; i<PING_INFO_MAX_STRLINE - LineNeed; i++)
		{
			Utl_UStrCpy(pPingTestParm->pStr[i], pPingTestParm->pStr[i+k]);
		}

		for (i=PING_INFO_MAX_STRLINE-LineNeed; i<PING_INFO_MAX_STRLINE; i++)
		{
			memset(pPingTestParm->pStr[i], 0, 100);
		}

		j = PING_INFO_MAX_STRLINE-LineNeed;
	}

	return j;
}

T_VOID Ping_Test_Recv(T_U32 addr, T_U32 size, T_U32 time)
{
	T_WSTR_20 tmpstr = {0};
	T_STR_20 tmpstr_asc = {0};
	
	T_U8 id = PING_INFO_MAX_STRLINE;
	
	if (AK_NULL == pPingTestParm)
	{
		return;
	}

	Fwl_Net_Ip2str(addr, tmpstr);
	Eng_StrUcs2Mbcs(tmpstr, tmpstr_asc);

	Fwl_Print(C3, M_NETWORK, "Reply from %s: bytes=%d time=%dms", tmpstr_asc, size, time);

	pPingTestParm->RecvNum++;
	pPingTestParm->TotalTime += time;

	AK_Obtain_Semaphore(pPingTestParm->semaphore, AK_SUSPEND);

	id = Ping_Test_GetEmpStrId(1);

	if (id < PING_INFO_MAX_STRLINE)
	{
		Utl_UStrCpy(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_PACKET));
		Utl_UItoa(size, tmpstr, 10);
		Utl_UStrCat(pPingTestParm->pStr[id], tmpstr);
		Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_EXPLORER_UNIT));

		Eng_StrMbcs2Ucs(",", tmpstr);
		Utl_UStrCat(pPingTestParm->pStr[id], tmpstr);

		Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_TIME));

		if (time < 1)
		{
			Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_LESS_THAN));
			Utl_UItoa(1, tmpstr, 10);
		}
		else
		{
			Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_EQUAL_TO));
			Utl_UItoa(time, tmpstr, 10);
		}

		Utl_UStrCat(pPingTestParm->pStr[id], tmpstr);
		Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_MILLISECOND));
	}

	AK_Release_Semaphore(pPingTestParm->semaphore);
	
}

T_VOID Ping_Test_Result_Set(T_VOID)
{
	T_WSTR_20 tmpstr = {0};
	T_U32	lost = 0;
	T_U32	averagetime = 0;
	
	T_U8 id = PING_INFO_MAX_STRLINE;
	
	if (AK_NULL == pPingTestParm)
	{
		return;
	}
	
	AK_Obtain_Semaphore(pPingTestParm->semaphore, AK_SUSPEND);

	id = Ping_Test_GetEmpStrId(1);

	if (id < PING_INFO_MAX_STRLINE)
	{
		Utl_UStrCpy(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_SENT_NUM));
		Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_EQUAL_TO));		
		Utl_UItoa(pPingTestParm->SentNum, tmpstr, 10);
		Utl_UStrCat(pPingTestParm->pStr[id], tmpstr);

		if (pPingTestParm->SentNum > 0)
		{
			Eng_StrMbcs2Ucs(",", tmpstr);
			Utl_UStrCat(pPingTestParm->pStr[id], tmpstr);

			Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_LOST));
			Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_EQUAL_TO));	
			
			lost = (pPingTestParm->SentNum - pPingTestParm->RecvNum) * 100 / pPingTestParm->SentNum;
			
			Utl_UItoa(lost, tmpstr, 10);
			Utl_UStrCat(pPingTestParm->pStr[id], tmpstr);
			Eng_StrMbcs2Ucs("%", tmpstr);
			Utl_UStrCat(pPingTestParm->pStr[id], tmpstr);

			if (pPingTestParm->RecvNum > 0)
			{
				Eng_StrMbcs2Ucs(",", tmpstr);
				Utl_UStrCat(pPingTestParm->pStr[id], tmpstr);

				Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_AVERAGE_TIME));
				averagetime = pPingTestParm->TotalTime / pPingTestParm->RecvNum;

				if (averagetime < 1)
				{
					Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_LESS_THAN));
					Utl_UItoa(1, tmpstr, 10);
				}
				else
				{
					Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_EQUAL_TO));
					Utl_UItoa(averagetime, tmpstr, 10);
				}

				Utl_UStrCat(pPingTestParm->pStr[id], tmpstr);
				Utl_UStrCat(pPingTestParm->pStr[id], Res_GetStringByID(eRES_STR_MILLISECOND));
			}
		}
	}
	AK_Release_Semaphore(pPingTestParm->semaphore);

	Fwl_Print(C3, M_NETWORK, "sent = %d: lost = %d average = %d ms", pPingTestParm->SentNum, lost, averagetime);
}

static T_VOID Ping_Test_Start(T_VOID)
{
	if (AK_NULL == pPingTestParm)
	{
		return;
	}
	
	if (ERROR_TIMER != pPingTestParm->timer)
	{
		return;
	}
	
	pPingTestParm->timer = Fwl_SetTimerMilliSecond(PING_TEST_SEND_INTERVAL, AK_TRUE);
	
	Ping_Test_CleanInfo();
}


static T_VOID Ping_Test_Stop(T_BOOL bNeedSetResult)
{
	if (AK_NULL == pPingTestParm)
	{
		return;
	}
	
	if (ERROR_TIMER == pPingTestParm->timer)
	{
		return;
	}
	
	Fwl_StopTimer(pPingTestParm->timer);
	pPingTestParm->timer = ERROR_TIMER;

	if (bNeedSetResult)
	{
		Ping_Test_Result_Set();
	}
}


#endif
/*---------------------- BEGIN OF STATE s_ping_test ------------------------*/
void initping_test(void)
{
#ifdef SUPPORT_NETWORK

    pPingTestParm = (T_PING_TEST_PARM *)Fwl_Malloc(sizeof(T_PING_TEST_PARM));
    AK_ASSERT_PTR_VOID(pPingTestParm, "initping_test(): malloc error");
	memset(pPingTestParm, 0, sizeof(T_PING_TEST_PARM));

	pPingTestParm->timer = ERROR_TIMER;
	pPingTestParm->remoteIp = gs.ping_rmt_ip;

	
	pPingTestParm->pBkImg = Res_GetBinResByID(AK_NULL, AK_FALSE, eRES_BMP_MAIN_BACKGROUND, AK_NULL);
	
    MenuStructInit(&pPingTestParm->IconExplorer);
	IconExplorer_SetTitleText(&pPingTestParm->IconExplorer, 
		Res_GetStringByID(eRES_STR_PING_TEST), ICONEXPLORER_TITLE_TEXTCOLOR);         // PING测试
	IconExplorer_SetSortIdCallBack(&pPingTestParm->IconExplorer, IconExplorer_SortIdCallback);

	pPingTestParm->semaphore = AK_Create_Semaphore(1, AK_PRIORITY);
	ScreenSaverDisable();
#endif
}

void exitping_test(void)
{
#ifdef SUPPORT_NETWORK
	gs.ping_rmt_ip = pPingTestParm->remoteIp;
	Ping_Test_Stop(AK_FALSE);
	ping_free();
	AK_Delete_Semaphore(pPingTestParm->semaphore);
	IconExplorer_Free(&pPingTestParm->IconExplorer);
    pPingTestParm = Fwl_Free(pPingTestParm);
	ScreenSaverEnable();
#endif
}

void paintping_test(void)
{
#ifdef SUPPORT_NETWORK

	Ping_Test_Show();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleping_test(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_eBACK_STATE IconExplorerRet;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pPingTestParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

	if (M_EVT_1 == event)
	{
		Ping_Test_SetItem();

		ping_init(Ping_Test_Recv);
	}

	if (M_EVT_EXIT == event)
	{
		Ping_Test_UpdateItem();
	}

	if (VME_EVT_TIMER == event)
	{
		if (pEventParm->w.Param1 == (T_U32)pPingTestParm->timer)
		{
			ping_send_now(pPingTestParm->remoteIp);
			pPingTestParm->SentNum++;
			Fwl_Print(C3, M_NETWORK, "Ping send!");
		}
	}

	IconExplorerRet = IconExplorer_Handler(&pPingTestParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        switch (IconExplorer_GetItemFocusId(&pPingTestParm->IconExplorer))
        {
        case 10: // target ip set
        	pEventParm->p.pParam1 = &pPingTestParm->remoteIp;
			m_triggerEvent(M_EVT_1, pEventParm);
            break;

		case 20: //start
			Ping_Test_Start();
			break;

		case 30: //stop
			Ping_Test_Stop(AK_TRUE);
			break;
			
        default:
            break;
        }
        break;
    default:
        ReturnDefauleProc(IconExplorerRet, pEventParm);
        break;
    }
#endif
    return 0;
}

