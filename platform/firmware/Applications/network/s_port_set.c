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
#include "fwl_oscom.h"
#include "Eng_akbmp.h"


typedef struct {
    T_MULTISET      multiSet;
	T_U32			port;
	T_U32 			*pPort;
	T_WSTR_20		portstr;
	T_WSTR_20		titlestr;
	T_MSGBOX    	msgbox;
} T_PORT_SET_PARM;

static T_PORT_SET_PARM *pPortSetParm;

T_BOOL PortSet_ModifyPort(T_U32 *port, T_BOOL bAdd)
{
    AK_ASSERT_PTR(port, "PortSet_ModifyPort(): port", AK_FALSE);

    if (bAdd)
    {
		*port = (*port + 1) % 65536;
	}
	else
	{
		*port = (*port + 65536 - 1) % 65536;
	}

	return AK_TRUE;
}

static T_BOOL  PortSet_GetContent (T_VOID)
{	
	AK_ASSERT_PTR(pPortSetParm, "PortSet_GetContent(): pPortSetParm", AK_FALSE);
	

    //main title
    MultiSet_SetTitleText(&pPortSetParm->multiSet, pPortSetParm->titlestr, COLOR_BLACK);

    //port set
    MultiSet_AddItemWithOption(&pPortSetParm->multiSet, 0, pPortSetParm->titlestr, \
                            pPortSetParm->portstr, ITEM_TYPE_EDIT);


    MultiSet_CheckScrollBar(&pPortSetParm->multiSet);
    
    MultiSet_SetRefresh(&pPortSetParm->multiSet, MULTISET_REFRESH_ALL);
    
    return AK_TRUE;
}

static T_BOOL PortSet_EditPortKeyProcessFunc(T_MMI_KEYPAD *pPhyKey)
{
    T_RECT          msgRect;

	AK_ASSERT_PTR(pPhyKey, "PortSet_EditPortKeyProcessFunc(): pPhyKey", AK_FALSE);
	AK_ASSERT_PTR(pPortSetParm, "PortSet_EditPortKeyProcessFunc(): pPortSetParm", AK_FALSE);

    
    switch (pPhyKey->keyID)
    {
        case kbOK:
            *(pPortSetParm->pPort) = pPortSetParm->port;
			
            MsgBox_InitStr(&pPortSetParm->msgbox, 0, GetCustomTitle(ctHINT),\
            GetCustomString(csSUCCESS_DONE), MSGBOX_INFORMATION);
            MsgBox_Show(&pPortSetParm->msgbox);
            MsgBox_GetRect(&pPortSetParm->msgbox, &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, \
                                msgRect.height);
            Fwl_MiniDelay(1000);

        case kbCLEAR:
            m_triggerEvent(M_EVT_EXIT, AK_NULL);
            break;
    
        case kbUP:
            PortSet_ModifyPort(&pPortSetParm->port, AK_TRUE);
            MultiSet_SetRefresh(&pPortSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
            break;

        case kbDOWN:
            PortSet_ModifyPort(&pPortSetParm->port, AK_FALSE);
            MultiSet_SetRefresh(&pPortSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
            break;
           
        default:
            break;
    }

    return AK_TRUE;
}


static T_BOOL PortSet_ShowEditPortCallFunc(T_TEXT *pText)
{
    T_U16	        str[20]; 
    T_U32           offset;
    T_S16           PosX, PosY;
    T_U16           Ustrtmp[8];
    T_U16           strLen;
	T_U16           *pStr = AK_NULL;
    T_U8            tempString[8];
	
	AK_ASSERT_PTR(pPortSetParm, "PortSet_ShowEditPortCallFunc(): pPortSetParm", AK_FALSE);
	AK_ASSERT_PTR(pText, "PortSet_ShowEditPortCallFunc(): pText", AK_FALSE);

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


	sprintf(tempString, "%05d", (T_U16)pPortSetParm->port);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCpy(str, Ustrtmp);

    Utl_UStrCpy(pText->pText, str);

    Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pText->pText, ~(pText->textColor), CURRENT_FONT_SIZE, \
                            (T_U16)Utl_UStrLen(pText->pText));

    strLen = 0;


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

static T_BOOL PortSet_hitButtonCallBack(T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y)
{
    T_pRECT pRect = AK_NULL;
    //T_ICON_TYPE iconType = ICON_LEFT;
    T_EDIT_AREA *pEditArea = AK_NULL;

	AK_ASSERT_PTR(pPhyKey, "PortSet_hitButtonCallBack(): pPhyKey", AK_FALSE);
	AK_ASSERT_PTR(pPortSetParm, "PortSet_hitButtonCallBack(): pPortSetParm", AK_FALSE);
	

    pEditArea = &pPortSetParm->multiSet.editArea;

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

static T_VOID resumePort_set(void)
{
    MultiSet_LoadImageData(&pPortSetParm->multiSet);
    MultiSet_SetRefresh(&pPortSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
}


#endif
/*---------------------- BEGIN OF STATE s_port_set ------------------------*/
void initport_set(void)
{
#ifdef SUPPORT_NETWORK

    pPortSetParm = (T_PORT_SET_PARM *)Fwl_Malloc(sizeof(T_PORT_SET_PARM));
    AK_ASSERT_PTR_VOID(pPortSetParm, "initport_set(): malloc error");
	memset(pPortSetParm, 0, sizeof(T_PORT_SET_PARM));	

	Utl_UItoa(10000, pPortSetParm->portstr, 10);

    MultiSet_Init(&pPortSetParm->multiSet);

    m_regResumeFunc(resumePort_set);
#endif
}

void exitport_set(void)
{
#ifdef SUPPORT_NETWORK
    MultiSet_Free(&pPortSetParm->multiSet);
    pPortSetParm = Fwl_Free(pPortSetParm);
#endif
}

void paintport_set(void)
{
#ifdef SUPPORT_NETWORK

    MultiSet_Show(&pPortSetParm->multiSet);
    MultiSet_SetRefresh(&pPortSetParm->multiSet, MULTISET_REFRESH_NONE);

    Fwl_RefreshDisplay();
#endif
}

unsigned char handleport_set(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_eBACK_STATE   ret;

    if (IsPostProcessEvent(event))
    {
    	MultiSet_SetRefresh(&pPortSetParm->multiSet, MULTISET_REFRESH_ALL);
        return 1;
    }

	if ((M_EVT_1 == event) || (M_EVT_2 == event) || (M_EVT_3 == event))
	{
		if (M_EVT_1 == event)
		{
			Utl_UStrCpy(pPortSetParm->titlestr, Res_GetStringByID(eRES_STR_TCP_SERVER_PORT));
		}
		else if (M_EVT_2 == event)
		{
			Utl_UStrCpy(pPortSetParm->titlestr, Res_GetStringByID(eRES_STR_REMOTE_PORT));
		}
		else
		{
			Utl_UStrCpy(pPortSetParm->titlestr, Res_GetStringByID(eRES_STR_LOCAL_PORT));
		}
		
		pPortSetParm->pPort = pEventParm->p.pParam1;
		pPortSetParm->port = *(pPortSetParm->pPort);

		PortSet_GetContent();
		MultiSet_SetItemKeyProcessCallBack(&pPortSetParm->multiSet, 0, PortSet_EditPortKeyProcessFunc);
		MultiSet_SetEditShowCallBack(&pPortSetParm->multiSet, 0, PortSet_ShowEditPortCallFunc);
	    
		MultiSet_SetHitButtonCallBack(&pPortSetParm->multiSet, 0, PortSet_hitButtonCallBack);
		MultiSet_SetCtlMode(&pPortSetParm->multiSet, MULTISET_MODE_SETTING);
		MultiSet_SetRefresh(&pPortSetParm->multiSet, MULTISET_REFRESH_EDITAREA);
	}

	ret = MultiSet_Handler(&pPortSetParm->multiSet, event, pEventParm);
	
    switch (ret)
    {
        default:
            ReturnDefauleProc(ret, pEventParm);
            break;
    }

#endif
    return 0;
}

