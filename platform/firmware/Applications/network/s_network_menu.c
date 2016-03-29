#include "Fwl_public.h"
#ifdef SUPPORT_NETWORK

#include "Ctl_MsgBox.h"
#include "Lib_state.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "fwl_net.h"
#include "eng_screensave.h"
#include "Eng_AutoPowerOff.h"

#define MAC_PHY_CLK_OUT 78

typedef struct {
    T_ICONEXPLORER  IconExplorer;
    T_MSGBOX        msgbox;
} T_NET_MENU_PARM;

static T_NET_MENU_PARM *pNetMenuParm;
extern T_VOID mac_phy_mcu_clk_25M(T_U8 pin);
extern T_BOOL  gpio_set_pin_as_gpio(T_U32 pin);
extern T_VOID gpio_set_pin_dir( T_U32 pin, T_U8 dir );

T_BOOL network_enter(T_VOID)
{
	T_U32 netmask = 0;
	T_U32 gw = 0;
	T_U32 len = 6;

	IPADDR_CALC(&netmask, 255, 255, 0, 0);
	IPADDR_CALC(&gw, 172, 16, 10, 10);

	if ((0 == gs.macaddr[0])
		&& (0 == gs.macaddr[1])
		&& (0 == gs.macaddr[2])
		&& (0 == gs.macaddr[3])
		&& (0 == gs.macaddr[4])
		&& (0 == gs.macaddr[5]))
	{
		Fwl_Net_GetMacAddr(gs.macaddr, &len);
	}

#ifdef CHIP_AK3750
	mac_phy_mcu_clk_25M(MAC_PHY_CLK_OUT);
#endif
	
	return Fwl_Net_Init(gs.macaddr, gs.ipaddr, netmask, gw);
}

#endif
/*---------------------- BEGIN OF STATE s_network_menu ------------------------*/
void initnetwork_menu(void)
{
#ifdef SUPPORT_NETWORK

    pNetMenuParm = (T_NET_MENU_PARM *)Fwl_Malloc(sizeof(T_NET_MENU_PARM));
    AK_ASSERT_PTR_VOID(pNetMenuParm, "initnet_menu(): malloc error");
	memset(pNetMenuParm, 0, sizeof(T_NET_MENU_PARM));

    MenuStructInit(&pNetMenuParm->IconExplorer);
    GetMenuStructContent(&pNetMenuParm->IconExplorer, mnNETWORK_MENU);
	ScreenSaverDisable();
	AutoPowerOffDisable(FLAG_NETWORK);
#endif
}

void exitnetwork_menu(void)
{
#ifdef SUPPORT_NETWORK

	#ifdef CHIP_AK3750
	gpio_set_pin_as_gpio (MAC_PHY_CLK_OUT); //saving the power
	gpio_set_pin_dir(MAC_PHY_CLK_OUT, 0);
	#endif
    Fwl_Net_Free();
    IconExplorer_Free(&pNetMenuParm->IconExplorer);

    pNetMenuParm = Fwl_Free(pNetMenuParm);
	ScreenSaverEnable();
	AutoPowerOffEnable(FLAG_NETWORK);

#endif
}

void paintnetwork_menu(void)
{
#ifdef SUPPORT_NETWORK

    IconExplorer_Show(&pNetMenuParm->IconExplorer);

    Fwl_RefreshDisplay();
#endif
}

unsigned char handlenetwork_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_eBACK_STATE IconExplorerRet;

    if (IsPostProcessEvent(event))
    {
        IconExplorer_SetRefresh(&pNetMenuParm->IconExplorer, ICONEXPLORER_REFRESH_ALL);
        return 1;
    }

	if (M_EVT_NETWORK == event)
	{
		if (!network_enter())
		{
			m_triggerEvent(M_EVT_EXIT, pEventParm);
			return 0;
		}
	}


    IconExplorerRet = IconExplorer_Handler(&pNetMenuParm->IconExplorer, event, pEventParm);

    switch (IconExplorerRet)
    {
    case eNext:
        switch (IconExplorer_GetItemFocusId(&pNetMenuParm->IconExplorer))
        {
        case 10: //ping test
			m_triggerEvent(M_EVT_1, pEventParm);
            break;

		case 20: //tcp server
			m_triggerEvent(M_EVT_2, pEventParm);
            break;

		case 30: //tcp client
			m_triggerEvent(M_EVT_3, pEventParm);
            break;

		case 40: //udp trans
			m_triggerEvent(M_EVT_4, pEventParm);
            break;

		case 50: //set
			m_triggerEvent(M_EVT_5, pEventParm);
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

