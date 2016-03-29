#include "Fwl_public.h"
#ifdef SUPPORT_NETWORK

#include "Ctl_MsgBox.h"
#include "Lib_state.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_display.h"
#include "fwl_net.h"
#include "Ctl_MultiSet.h"
#include "Eng_Font.h"
#include "Eng_akbmp.h"
#include "fwl_oscom.h"


typedef enum
{
    CURSOR_A = 0,
    CURSOR_B,            
    CURSOR_C, 
    CURSOR_D,
    CURSOR_IP_NUM
}T_IP_CURSOR;


typedef struct {
    T_MULTISET      multiSet;
	T_IP_CURSOR		ipcursor;
	T_U32			ipaddr;
	T_U32 			*remoteip;
	T_WSTR_20		ipstr;
	T_MSGBOX    	msgbox;
} T_IP_SET_PARM;

static T_IP_SET_PARM *pIpSetParm;

T_BOOL IpSet_ModifyIpAddr(T_U32 *ipaddr, T_IP_CURSOR ipCursor, T_BOOL bAdd, T_U32 value)
{
	T_U8 a,b,c,d;
    AK_ASSERT_PTR(ipaddr, "IpSet_ModifyIpAddr(): ipaddr", AK_FALSE);

	a = (*ipaddr) & 0xff;
	b = ((*ipaddr) >> 8) & 0xff;
	c = ((*ipaddr) >> 16) & 0xff;
	d = ((*ipaddr) >> 24) & 0xff;

    switch(ipCursor)
    {
        case CURSOR_A:
            if (bAdd)
            {
				a = (a + value) % 256;
			}
			else
			{
				a = (a + 256 - value) % 256;
			}
            break;
            
        case CURSOR_B:
            if (bAdd)
            {
				b = (b + value) % 256;
			}
			else
			{
				b = (b + 256 - value) % 256;
			}           
            break;
            
        case CURSOR_C:
            if (bAdd)
            {
				c = (c + value) % 256;
			}
			else
			{
				c = (c + 256 - value) % 256;
			}           
            break;

		case CURSOR_D:
            if (bAdd)
            {
				d = (d + value) % 256;
			}
			else
			{
				d = (d + 256 - value) % 256;
			}            
            break;
            
        default:
            break;
    }

	IPADDR_CALC(ipaddr, a, b, c, d);

	return AK_TRUE;
}

static T_BOOL  IpSet_GetContent (T_VOID)
{	
	AK_ASSERT_PTR(pIpSetParm, "IpSet_GetContent(): pIpSetParm", AK_FALSE);
	

    //main title
    MultiSet_SetTitleText(&pIpSetParm->multiSet, \
                        Res_GetStringByID(eRES_STR_REMOTE_IP), COLOR_BLACK);

    //ip addr set
    MultiSet_AddItemWithOption(&pIpSetParm->multiSet, 0, Res_GetStringByID(eRES_STR_REMOTE_IP), \
                            pIpSetParm->ipstr, ITEM_TYPE_EDIT);


    MultiSet_CheckScrollBar(&pIpSetParm->multiSet);
    
    MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_ALL);
    
    return AK_TRUE;
}

static T_VOID IpSet_MoveIpCursor(T_BOOL bRight)
{
    AK_ASSERT_PTR_VOID(pIpSetParm, "IpSet_MoveIpCursor(): pIpSetParm");

    if (bRight)
    {
		pIpSetParm->ipcursor = (pIpSetParm->ipcursor + 1) % CURSOR_IP_NUM;
	}
	else
	{
		pIpSetParm->ipcursor = (pIpSetParm->ipcursor + CURSOR_IP_NUM - 1) % CURSOR_IP_NUM;
	}
}

static T_BOOL IpSet_EditIpKeyProcessFunc(T_MMI_KEYPAD *pPhyKey)
{
    T_RECT          msgRect;

	AK_ASSERT_PTR(pPhyKey, "IpSet_EditIpKeyProcessFunc(): pPhyKey", AK_FALSE);
	AK_ASSERT_PTR(pIpSetParm, "IpSet_EditIpKeyProcessFunc(): pIpSetParm", AK_FALSE);

    
    switch (pPhyKey->keyID)
    {
        case kbOK:
            *(pIpSetParm->remoteip) = pIpSetParm->ipaddr;
			
            pIpSetParm->ipcursor = CURSOR_A; 
            MsgBox_InitStr(&pIpSetParm->msgbox, 0, GetCustomTitle(ctHINT),\
            GetCustomString(csSUCCESS_DONE), MSGBOX_INFORMATION);
            MsgBox_Show(&pIpSetParm->msgbox);
            MsgBox_GetRect(&pIpSetParm->msgbox, &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, \
                                msgRect.height);
            Fwl_MiniDelay(1000);

        case kbCLEAR:
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
    
        case kbUP:
			if (PRESS_LONG == pPhyKey->pressType)
			{
	            IpSet_ModifyIpAddr(&pIpSetParm->ipaddr, pIpSetParm->ipcursor, AK_TRUE, 50);
	            MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
			}
			else if(PRESS_SHORT == pPhyKey->pressType)
			{
				IpSet_ModifyIpAddr(&pIpSetParm->ipaddr, pIpSetParm->ipcursor, AK_TRUE, 1);
	            MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
			}
            break;

        case kbDOWN:
			if (PRESS_LONG == pPhyKey->pressType)
			{
	            IpSet_ModifyIpAddr(&pIpSetParm->ipaddr, pIpSetParm->ipcursor, AK_FALSE, 50);
	            MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
			}
			else if(PRESS_SHORT == pPhyKey->pressType)
			{
            	IpSet_ModifyIpAddr(&pIpSetParm->ipaddr, pIpSetParm->ipcursor, AK_FALSE, 1);
            	MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
			}
            break;
            
        case kbLEFT:
            IpSet_MoveIpCursor(AK_FALSE);
            MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
            break;

        case kbRIGHT:
            IpSet_MoveIpCursor(AK_TRUE);
            MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
            break;
           
        default:
            break;
    }

    return AK_TRUE;
}


static T_BOOL IpSet_ShowEditIpCallFunc(T_TEXT *pText)
{
    T_U16	        strip[20]; 
    T_U32           offset;
    T_S16           PosX, PosY;
    T_U16           Ustrtmp[5];
    T_U16           strLen;
	T_U16           *pStr = AK_NULL;
    T_U8            tempString[8];
	T_U8 			a,b,c,d;
	
	AK_ASSERT_PTR(pIpSetParm, "IpSet_ShowEditIpCallFunc(): pIpSetParm", AK_FALSE);
	AK_ASSERT_PTR(pText, "IpSet_ShowEditIpCallFunc(): pText", AK_FALSE);

    if (AK_TRUE != MultiSet_GetTextPos(pText->pText, &pText->rect, pText->textStyle, &PosX, &PosY))
    {
        return AK_FALSE;
    }

    pText->cursor.left = PosX;
    pText->cursor.top = PosY;
    pText->cursor.height = g_Font.SCHEIGHT;
	pStr = pText->pText;

	//display background
    if (AK_NULL == pText->pBackData->pImgDt)
    {
        Fwl_FillSolidRect(HRGB_LAYER, pText->rect.left, pText->rect.top, pText->rect.width, pText->rect.height,\
                            pText->backColor);
    }
    else
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pText->rect.left, pText->rect.top, (T_pCDATA)pText->pBackData->pImgDt, \
                            &g_Graph.TransColor, AK_FALSE);
    }
	
	a = pIpSetParm->ipaddr & 0xff;
	b = (pIpSetParm->ipaddr >> 8) & 0xff;
	c = (pIpSetParm->ipaddr >> 16) & 0xff;
	d = (pIpSetParm->ipaddr >> 24) & 0xff;

	sprintf(tempString, "%03d.", a);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCpy(strip, Ustrtmp);

	sprintf(tempString, "%03d.", b);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCat(strip, Ustrtmp);

	sprintf(tempString, "%03d.", c);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCat(strip, Ustrtmp);

	sprintf(tempString, "%03d", d);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCat(strip, Ustrtmp);

    Utl_UStrCpy(pText->pText, strip);


    Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pText->pText, ~(pText->textColor), CURRENT_FONT_SIZE, \
                            (T_U16)Utl_UStrLen(pText->pText));

    switch (pIpSetParm->ipcursor)
    {
        case CURSOR_A:
            strLen = 0;
            sprintf(tempString, "%03d", a);
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
        case CURSOR_B:
            strLen = 4;
            sprintf(tempString, "%03d", b);
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
            
        case CURSOR_C:
            strLen = 8;
            sprintf(tempString, "%03d", c);
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;

		case CURSOR_D:
            strLen = 12;
            sprintf(tempString, "%03d", d);
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;

		default:
			break;
    }

	//calc cursor abscissa
	pStr += strLen; 
    offset = UGetSpeciStringWidth((T_U16 *)pText->pText, \
                                CURRENT_FONT_SIZE, strLen);
	pText->cursor.left = (T_S16)(PosX + offset);

	//calc cursor width
    pText->cursor.width = (T_S16)UGetSpeciStringWidth((T_U16 *)Ustrtmp, \
                                    CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Ustrtmp));

	//display cursor
    Fwl_FillSolidRect(HRGB_LAYER, pText->cursor.left, pText->cursor.top, \
                pText->cursor.width, pText->cursor.height, pText->cursorColor);

    Fwl_UDispSpeciString(HRGB_LAYER, pText->cursor.left, pText->cursor.top, pStr, \
                pText->textColor, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(Ustrtmp));

    return AK_TRUE;
}

static T_BOOL IpSet_hitButtonCallBack(T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y)
{
    T_pRECT pRect = AK_NULL;
    //T_ICON_TYPE iconType = ICON_LEFT;
    T_EDIT_AREA *pEditArea = AK_NULL;

	AK_ASSERT_PTR(pPhyKey, "IpSet_hitButtonCallBack(): pPhyKey", AK_FALSE);
	AK_ASSERT_PTR(pIpSetParm, "IpSet_hitButtonCallBack(): pIpSetParm", AK_FALSE);
	

    pEditArea = &pIpSetParm->multiSet.editArea;

    //hit left button
    pRect = &pEditArea->IconRct[ICON_LEFT];
    if (PointInRect(pRect, x, y))
    {
        pPhyKey->keyID = kbLEFT;
        pPhyKey->pressType = PRESS_SHORT;
        return AK_TRUE;
    }

    //hit right button
    pRect = &pEditArea->IconRct[ICON_RIGHT];
    if (PointInRect(pRect, x, y))
    {
        pPhyKey->keyID = kbRIGHT;
        pPhyKey->pressType = PRESS_SHORT;
        return AK_TRUE;
    }

    //hit up button
    pRect = &pEditArea->IconRct[ICON_UP];
    if (PointInRect(pRect, x, y))
    {
        pPhyKey->keyID = kbUP;
        pPhyKey->pressType = PRESS_SHORT;
        return AK_TRUE;
    }

    //hit down button
    pRect = &pEditArea->IconRct[ICON_DOWN];
    if (PointInRect(pRect, x, y))
    {
        pPhyKey->keyID = kbDOWN;
        pPhyKey->pressType = PRESS_SHORT;
        return AK_TRUE;
    }

    //hit ok button
    pRect = &pEditArea->buttonRct;
    if (PointInRect(pRect, x, y))
    {
        pPhyKey->keyID = kbOK;
        pPhyKey->pressType = PRESS_SHORT;
        return AK_TRUE;
    }

	return AK_FALSE; 
}

static T_VOID resumeIp_set(void)
{
    MultiSet_LoadImageData(&pIpSetParm->multiSet);
    MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
}


#endif
/*---------------------- BEGIN OF STATE s_ip_set ------------------------*/
void initip_set(void)
{
#ifdef SUPPORT_NETWORK

    pIpSetParm = (T_IP_SET_PARM *)Fwl_Malloc(sizeof(T_IP_SET_PARM));
    AK_ASSERT_PTR_VOID(pIpSetParm, "initip_set(): malloc error");
	memset(pIpSetParm, 0, sizeof(T_IP_SET_PARM));	

	Fwl_Net_Ip2str(0xffffffff, pIpSetParm->ipstr);

    MultiSet_Init(&pIpSetParm->multiSet);

    m_regResumeFunc(resumeIp_set);
#endif
}

void exitip_set(void)
{
#ifdef SUPPORT_NETWORK
    MultiSet_Free(&pIpSetParm->multiSet);
    pIpSetParm = Fwl_Free(pIpSetParm);
#endif
}

void paintip_set(void)
{
#ifdef SUPPORT_NETWORK

    MultiSet_Show(&pIpSetParm->multiSet);
    MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_NONE);

    Fwl_RefreshDisplay();
#endif
}

unsigned char handleip_set(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_eBACK_STATE   ret;

    if (IsPostProcessEvent(event))
    {
    	MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_ALL);
        return 1;
    }

	if (M_EVT_1 == event)
	{
		pIpSetParm->remoteip = pEventParm->p.pParam1;
		pIpSetParm->ipaddr = *(pIpSetParm->remoteip);
		
		IpSet_GetContent();
		MultiSet_SetItemKeyProcessCallBack(&pIpSetParm->multiSet, 0, IpSet_EditIpKeyProcessFunc);
		MultiSet_SetEditShowCallBack(&pIpSetParm->multiSet, 0, IpSet_ShowEditIpCallFunc);
	    
		MultiSet_SetHitButtonCallBack(&pIpSetParm->multiSet, 0, IpSet_hitButtonCallBack);
		MultiSet_SetCtlMode(&pIpSetParm->multiSet, MULTISET_MODE_SETTING);
		MultiSet_SetRefresh(&pIpSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
	}

	ret = MultiSet_Handler(&pIpSetParm->multiSet, event, pEventParm);
	
    switch (ret)
    {
        default:
            ReturnDefauleProc(ret, pEventParm);
            break;
    }

#endif
    return 0;
}

