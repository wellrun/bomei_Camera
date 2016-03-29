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


typedef enum
{
    ITEMID_IP_ADDR = 0,
    ITEMID_MAC_ADDR,            
    ITEMID_SEND_FILE,
    ITEMID_RECV_PATH
}T_NET_SET_ID;

typedef enum
{
    CURSOR_A = 0,
    CURSOR_B,            
    CURSOR_C, 
    CURSOR_D,
    CURSOR_IP_NUM
}T_IP_CURSOR;

typedef enum
{
    CURSOR_0 = 0,
    CURSOR_1,            
    CURSOR_2, 
    CURSOR_3,
    CURSOR_4,
    CURSOR_5,
    CURSOR_MAC_NUM
}T_MAC_CURSOR;

typedef struct {
    T_MULTISET      multiSet;
	T_IP_CURSOR		ipcursor;
	T_MAC_CURSOR	maccursor;
	T_U32			ipaddr;
	T_U8			macaddr[6];
	T_WSTR_20		ipstr;
	T_WSTR_50		macstr;
	T_USTR_FILE     sendfile;
	T_USTR_FILE     recvpath;
	T_MSGBOX    	msgbox;
} T_NET_SET_MENU_PARM;

static T_NET_SET_MENU_PARM *pNetSetMenuParm;

extern T_BOOL IpSet_ModifyIpAddr(T_U32 *ipaddr, T_IP_CURSOR ipCursor, T_BOOL bAdd, T_U32 value);
extern T_BOOL network_enter(T_VOID);

static T_VOID NetworkSet_MacToStr(T_U8* macaddr, T_U16* str)
{
	T_U8    tempString[8];
	T_U16   Ustrtmp[5];
	
	AK_ASSERT_PTR_VOID(str, "NetworkSet_MacToStr(): str");
	AK_ASSERT_PTR_VOID(macaddr, "NetworkSet_MacToStr(): macaddr");

	sprintf(tempString, "%02x:", macaddr[0]);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCpy(str, Ustrtmp);

	sprintf(tempString, "%02x:", macaddr[1]);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCat(str, Ustrtmp);

	sprintf(tempString, "%02x:", macaddr[2]);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCat(str, Ustrtmp);

	sprintf(tempString, "%02x:", macaddr[3]);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCat(str, Ustrtmp);

	sprintf(tempString, "%02x:", macaddr[4]);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCat(str, Ustrtmp);

	sprintf(tempString, "%02x", macaddr[5]);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);
	Utl_UStrCat(str, Ustrtmp);
}

static T_VOID NetworkSet_GetDispStr(T_VOID)
{
    AK_ASSERT_PTR_VOID(pNetSetMenuParm, "NetworkSet_GetDispStr(): pNetSetMenuParm");

	pNetSetMenuParm->ipaddr = gs.ipaddr;
    Fwl_Net_Ip2str(pNetSetMenuParm->ipaddr, pNetSetMenuParm->ipstr);

	memcpy(pNetSetMenuParm->macaddr, gs.macaddr, 6);
	NetworkSet_MacToStr(pNetSetMenuParm->macaddr, pNetSetMenuParm->macstr);


    MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_ALL);
}

static T_VOID NetworkSet_SendFileProcess(T_LEN rectWidth, T_U16 *pFileName)
{
    T_USTR_FILE filePath, fileName;
    T_USTR_FILE name, ext;
    T_USTR_FILE tmpStr;
    T_U32       tmpWidth;
    T_U16       tmpNum;

    if (AK_NULL == pFileName)
    {
        return;
    }
    else
    {
		if (0 == (T_U16)Utl_UStrLen(gs.sendfile))
		{
			Utl_UStrCpy(pFileName, Res_GetStringByID(eRES_STR_ALARM_TYPE_NULL));
		}
		else
		{
            //adjust whether the file path is valid
            if(!Fwl_FileExist(gs.sendfile))
            {
                gs.sendfile[0] = UNICODE_END;
                Utl_UStrCpy(pFileName, Res_GetStringByID(eRES_STR_ALARM_TYPE_NULL));
            }
            else 
            {                
                Utl_USplitFilePath(gs.sendfile, filePath, fileName);
                Utl_USplitFileName(fileName, name, ext);
                
                tmpNum = Fwl_GetUStringDispNum(fileName, (T_U16)Utl_UStrLen(fileName), (T_U16)(rectWidth - 5), CURRENT_FONT_SIZE);
                
                if((T_U16)Utl_UStrLen(fileName) > tmpNum)  
                {
                
                    Eng_StrMbcs2Ucs("..>", tmpStr);
                    Utl_UStrCat(tmpStr, ext);
                
                    tmpWidth = UGetSpeciStringWidth(tmpStr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(tmpStr));
                
                    tmpNum = Fwl_GetUStringDispNum( fileName, (T_U16)Utl_UStrLen(fileName), (T_U16)(rectWidth - 5 - tmpWidth), \
                                                CURRENT_FONT_SIZE);
                
                    *(fileName + tmpNum) = 0;
                    Utl_UStrCat(fileName, tmpStr);
                }
                
                Utl_UStrCpy(pFileName, fileName);

            }

		}
    }
}

static T_VOID NetworkSet_RecvPathProcess(T_LEN rectWidth, T_U16 *pFileName)
{
    T_USTR_FILE path;
    T_USTR_FILE tmpStr;
    T_U32       tmpWidth;
    T_U16       tmpNum;

    if (AK_NULL == pFileName)
    {
        return;
    }
    else
    {      
        Utl_UStrCpy(path, Fwl_GetDefPath(eNETWORK_PATH));
        
        tmpNum = Fwl_GetUStringDispNum(path, (T_U16)Utl_UStrLen(path), (T_U16)(rectWidth - 5), CURRENT_FONT_SIZE);
        
        if((T_U16)Utl_UStrLen(path) > tmpNum)  
        {
        
            Eng_StrMbcs2Ucs("..>", tmpStr);
        
            tmpWidth = UGetSpeciStringWidth(tmpStr, CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(tmpStr));
        
            tmpNum = Fwl_GetUStringDispNum( path, (T_U16)Utl_UStrLen(path), (T_U16)(rectWidth - 5 - tmpWidth), \
                                        CURRENT_FONT_SIZE);
        
            *(path + tmpNum) = 0;
            Utl_UStrCat(path, tmpStr);
        }
        
        Utl_UStrCpy(pFileName, path);
    }
}


static T_VOID NetworkSet_GetPathStr(T_VOID)
{
    T_LEN rectWidth = 0;

    AK_ASSERT_PTR_VOID(pNetSetMenuParm, "NetworkSet_SyncAlarmSoundFileName(): pNetSetMenuParm");
	
    rectWidth = MultiSet_GetTextRectWidth(&pNetSetMenuParm->multiSet);
    NetworkSet_SendFileProcess(rectWidth, pNetSetMenuParm->sendfile);
	NetworkSet_RecvPathProcess(rectWidth, pNetSetMenuParm->recvpath);
}


static T_BOOL  NetworkSet_GetContent (T_VOID)
{
    T_LEN   rectWidth   = 0;

	
	AK_ASSERT_PTR(pNetSetMenuParm, "NetworkSet_GetContent(): pNetSetMenuParm", AK_FALSE);
	

    //main title
    MultiSet_SetTitleText(&pNetSetMenuParm->multiSet, \
                        Res_GetStringByID(eRES_STR_NETWORK_SET), COLOR_BLACK);

    //ip addr set
    MultiSet_AddItemWithOption(&pNetSetMenuParm->multiSet, ITEMID_IP_ADDR, Res_GetStringByID(eRES_STR_IP_ADDR), \
                            pNetSetMenuParm->ipstr, ITEM_TYPE_EDIT);

    //mac addr set
    MultiSet_AddItemWithOption(&pNetSetMenuParm->multiSet, ITEMID_MAC_ADDR,Res_GetStringByID(eRES_STR_MAC_ADDR), \
                            pNetSetMenuParm->macstr, ITEM_TYPE_EDIT);


    //send file set
    MultiSet_AddItemWithOption(&pNetSetMenuParm->multiSet, ITEMID_SEND_FILE, Res_GetStringByID(eRES_STR_DEF_SEND_FILE), \
                            pNetSetMenuParm->sendfile, ITEM_TYPE_NONE);
    rectWidth = MultiSet_GetTextRectWidth(&pNetSetMenuParm->multiSet);
    NetworkSet_SendFileProcess(rectWidth, pNetSetMenuParm->sendfile);

	//receive path set
    MultiSet_AddItemWithOption(&pNetSetMenuParm->multiSet, ITEMID_RECV_PATH, Res_GetStringByID(eRES_STR_DEF_RECV_PATH), \
                            pNetSetMenuParm->recvpath, ITEM_TYPE_NONE);
    rectWidth = MultiSet_GetTextRectWidth(&pNetSetMenuParm->multiSet);
    NetworkSet_RecvPathProcess(rectWidth, pNetSetMenuParm->recvpath);

    MultiSet_CheckScrollBar(&pNetSetMenuParm->multiSet);
    
    MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_ALL);
    
    return AK_TRUE;
}

static T_VOID NetworkSet_MoveIpCursor(T_BOOL bRight)
{
    AK_ASSERT_PTR_VOID(pNetSetMenuParm, "NetworkSet_MoveIpCursor(): pNetSetMenuParm");

    if (bRight)
    {
		pNetSetMenuParm->ipcursor = (pNetSetMenuParm->ipcursor + 1) % CURSOR_IP_NUM;
	}
	else
	{
		pNetSetMenuParm->ipcursor = (pNetSetMenuParm->ipcursor + CURSOR_IP_NUM - 1) % CURSOR_IP_NUM;
	}
}

static T_VOID NetworkSet_MoveMacCursor(T_BOOL bRight)
{
    AK_ASSERT_PTR_VOID(pNetSetMenuParm, "NetworkSet_MoveMacCursor(): pNetSetMenuParm");

    if (bRight)
    {
		pNetSetMenuParm->maccursor = (pNetSetMenuParm->maccursor + 1) % CURSOR_MAC_NUM;
	}
	else
	{
		pNetSetMenuParm->maccursor = (pNetSetMenuParm->maccursor + CURSOR_MAC_NUM - 1) % CURSOR_MAC_NUM;
	}
}


static T_BOOL NetworkSet_ModifyMacAddr(T_U8 *macaddr, T_MAC_CURSOR macCursor, T_BOOL bAdd)
{
    AK_ASSERT_PTR(macaddr, "NetworkSet_ModifyMacAddr(): macaddr", AK_FALSE);

	if (bAdd)
    {
		macaddr[macCursor] = (macaddr[macCursor] + 1) % 256;
	}
	else
	{
		macaddr[macCursor] = (macaddr[macCursor] +256 - 1) % 256;
	}

	return AK_TRUE;
}

static T_BOOL NetworkSet_EditIpKeyProcessFunc(T_MMI_KEYPAD *pPhyKey)
{
    T_RECT          msgRect;

	AK_ASSERT_PTR(pPhyKey, "NetworkSet_EditIpKeyProcessFunc(): pPhyKey", AK_FALSE);
	AK_ASSERT_PTR(pNetSetMenuParm, "NetworkSet_EditIpKeyProcessFunc(): pNetSetMenuParm", AK_FALSE);

    
    switch (pPhyKey->keyID)
    {
        case kbOK:
            gs.ipaddr = pNetSetMenuParm->ipaddr;
			Fwl_Net_Free();
			network_enter();
			
            pNetSetMenuParm->ipcursor = CURSOR_A; 
            MsgBox_InitStr(&pNetSetMenuParm->msgbox, 0, GetCustomTitle(ctHINT),\
            GetCustomString(csSUCCESS_DONE), MSGBOX_INFORMATION);
            MsgBox_Show(&pNetSetMenuParm->msgbox);
            MsgBox_GetRect(&pNetSetMenuParm->msgbox, &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, \
                                msgRect.height);
            Fwl_MiniDelay(1000);

        case kbCLEAR:
            NetworkSet_GetDispStr();
			pNetSetMenuParm->ipcursor = CURSOR_A;
            MultiSet_SetCtlMode(&pNetSetMenuParm->multiSet, MULTISET_MODE_NORMAL);
            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_ALL);
            break;
    
        case kbUP:
			if (PRESS_LONG == pPhyKey->pressType)
			{
	            IpSet_ModifyIpAddr(&pNetSetMenuParm->ipaddr, pNetSetMenuParm->ipcursor, AK_TRUE, 50);
	            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
			}
			else if(PRESS_SHORT == pPhyKey->pressType)
			{
	            IpSet_ModifyIpAddr(&pNetSetMenuParm->ipaddr, pNetSetMenuParm->ipcursor, AK_TRUE, 1);
	            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
			}
            break;

        case kbDOWN:
			if (PRESS_LONG == pPhyKey->pressType)
			{
	            IpSet_ModifyIpAddr(&pNetSetMenuParm->ipaddr, pNetSetMenuParm->ipcursor, AK_FALSE, 50);
	            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
			}
			else if(PRESS_SHORT == pPhyKey->pressType)
			{
	            IpSet_ModifyIpAddr(&pNetSetMenuParm->ipaddr, pNetSetMenuParm->ipcursor, AK_FALSE, 1);
	            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
			}
            break;
            
        case kbLEFT:
            NetworkSet_MoveIpCursor(AK_FALSE);
            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
            break;

        case kbRIGHT:
            NetworkSet_MoveIpCursor(AK_TRUE);
            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
            break;
           
        default:
            break;
    }

    return AK_TRUE;
}


static T_BOOL NetworkSet_EditMacKeyProcessFunc(T_MMI_KEYPAD *pPhyKey)
{
    T_RECT          msgRect;

	AK_ASSERT_PTR(pPhyKey, "NetworkSet_EditMacKeyProcessFunc(): pPhyKey", AK_FALSE);
	AK_ASSERT_PTR(pNetSetMenuParm, "NetworkSet_EditMacKeyProcessFunc(): pNetSetMenuParm", AK_FALSE);
	
    
    switch (pPhyKey->keyID)
    {
        case kbOK:
            memcpy(gs.macaddr, pNetSetMenuParm->macaddr, 6);
			Fwl_Net_Free();
			network_enter();
			
            pNetSetMenuParm->maccursor = CURSOR_0;
            MsgBox_InitStr(&pNetSetMenuParm->msgbox, 0, GetCustomTitle(ctHINT),\
            GetCustomString(csSUCCESS_DONE), MSGBOX_INFORMATION);
            MsgBox_Show(&pNetSetMenuParm->msgbox);
            MsgBox_GetRect(&pNetSetMenuParm->msgbox, &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, \
                                msgRect.height);
            Fwl_MiniDelay(1000);

        case kbCLEAR:
            NetworkSet_GetDispStr();
			pNetSetMenuParm->maccursor = CURSOR_0;
            MultiSet_SetCtlMode(&pNetSetMenuParm->multiSet, MULTISET_MODE_NORMAL);
            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_ALL);
            break;
    
        case kbUP:
            NetworkSet_ModifyMacAddr(pNetSetMenuParm->macaddr, pNetSetMenuParm->maccursor, AK_TRUE);
            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
            break;

        case kbDOWN:
            NetworkSet_ModifyMacAddr(pNetSetMenuParm->macaddr, pNetSetMenuParm->maccursor, AK_FALSE);
            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
            break;
            
        case kbLEFT:
            NetworkSet_MoveMacCursor(AK_FALSE);
            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
            break;

        case kbRIGHT:
            NetworkSet_MoveMacCursor(AK_TRUE);
            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
            break;
           
        default:
            break;
    }

    return AK_TRUE;
}

static T_BOOL NetworkSet_ShowEditIpCallFunc(T_TEXT *pText)
{
    T_U16	        strip[20]; 
    T_U32           offset;
    T_S16           PosX, PosY;
    T_U16           Ustrtmp[5];
    T_U16           strLen;
	T_U16           *pStr = AK_NULL;
    T_U8            tempString[8];
	T_U8 			a,b,c,d;
	
	AK_ASSERT_PTR(pNetSetMenuParm, "NetworkSet_ShowEditIpCallFunc(): pNetSetMenuParm", AK_FALSE);
	AK_ASSERT_PTR(pText, "NetworkSet_ShowEditIpCallFunc(): pText", AK_FALSE);

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
	
	a = pNetSetMenuParm->ipaddr & 0xff;
	b = (pNetSetMenuParm->ipaddr >> 8) & 0xff;
	c = (pNetSetMenuParm->ipaddr >> 16) & 0xff;
	d = (pNetSetMenuParm->ipaddr >> 24) & 0xff;

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

    switch (pNetSetMenuParm->ipcursor)
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

static T_BOOL NetworkSet_ShowEditMacCallFunc(T_TEXT *pText)
{
    T_U32           offset;
    T_S16           PosX, PosY;
    T_U16           Ustrtmp[5];
    T_U16           strLen;
	T_U16           *pStr = AK_NULL;
    T_U8            tempString[8];
	
	AK_ASSERT_PTR(pNetSetMenuParm, "NetworkSet_ShowEditMacCallFunc(): pNetSetMenuParm", AK_FALSE);
	AK_ASSERT_PTR(pText, "NetworkSet_ShowEditMacCallFunc(): pText", AK_FALSE);

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

	NetworkSet_MacToStr(pNetSetMenuParm->macaddr, pText->pText);


    Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pText->pText, ~(pText->textColor), CURRENT_FONT_SIZE, \
                            (T_U16)Utl_UStrLen(pText->pText));

	strLen = 3 * pNetSetMenuParm->maccursor;
    sprintf(tempString, "%02x", pNetSetMenuParm->macaddr[pNetSetMenuParm->maccursor]);
    Eng_StrMbcs2Ucs(tempString, Ustrtmp);

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

static T_BOOL NetworkSet_hitButtonCallBack(T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y)
{
    T_pRECT pRect = AK_NULL;
    //T_ICON_TYPE iconType = ICON_LEFT;
    T_EDIT_AREA *pEditArea = AK_NULL;

	AK_ASSERT_PTR(pPhyKey, "NetworkSet_hitButtonCallBack(): pPhyKey", AK_FALSE);
	AK_ASSERT_PTR(pNetSetMenuParm, "NetworkSet_hitButtonCallBack(): pNetSetMenuParm", AK_FALSE);
	

    pEditArea = &pNetSetMenuParm->multiSet.editArea;

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


static T_VOID NetworkSet_SetKeyPrecessCallBackFunc(T_VOID)
{
	AK_ASSERT_PTR_VOID(pNetSetMenuParm, "NetworkSet_SetKeyPrecessCallBackFunc(): pNetSetMenuParm");
	
    MultiSet_SetItemKeyProcessCallBack(&pNetSetMenuParm->multiSet, ITEMID_IP_ADDR, \
                                        NetworkSet_EditIpKeyProcessFunc);
    MultiSet_SetItemKeyProcessCallBack(&pNetSetMenuParm->multiSet, ITEMID_MAC_ADDR, \
                                        NetworkSet_EditMacKeyProcessFunc);
}

static T_VOID NetworkSet_SetEditShowCallBackFunc(T_VOID)
{
	AK_ASSERT_PTR_VOID(pNetSetMenuParm, "NetworkSet_SetEditShowCallBackFunc(): pNetSetMenuParm");
	
    MultiSet_SetEditShowCallBack(&pNetSetMenuParm->multiSet, ITEMID_IP_ADDR,\
                                    NetworkSet_ShowEditIpCallFunc);
    MultiSet_SetEditShowCallBack(&pNetSetMenuParm->multiSet, ITEMID_MAC_ADDR,\
                                    NetworkSet_ShowEditMacCallFunc);
}

static T_VOID NetworkSet_SetHitButtonCallBackFunc(T_VOID)
{
	AK_ASSERT_PTR_VOID(pNetSetMenuParm, "NetworkSet_SetHitButtonCallBackFunc(): pNetSetMenuParm");
	
    MultiSet_SetHitButtonCallBack(&pNetSetMenuParm->multiSet, ITEMID_IP_ADDR, \
                            NetworkSet_hitButtonCallBack);
    MultiSet_SetHitButtonCallBack(&pNetSetMenuParm->multiSet, ITEMID_MAC_ADDR, \
                            NetworkSet_hitButtonCallBack);
}

static T_VOID resumenetwork_set_menu(void)
{
    T_ITEM_NODE * pFocusItem = AK_NULL;


    MultiSet_LoadImageData(&pNetSetMenuParm->multiSet);
    NetworkSet_GetPathStr();
    
    if (MULTISET_MODE_NORMAL == MultiSet_GetCtlMode(&pNetSetMenuParm->multiSet))
    {
        MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_ALL);
    }
    else 
    {
        pFocusItem = MultiSet_GetItemFocus(&pNetSetMenuParm->multiSet);
        if (ITEM_TYPE_EDIT == pFocusItem->itemType)
        {
            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_EDITAREA);
        }
        else
        {
            MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_OPTION);
        }
    }
}


#endif
/*---------------------- BEGIN OF STATE s_network_set_menu ------------------------*/
void initnetwork_set_menu(void)
{
#ifdef SUPPORT_NETWORK

    pNetSetMenuParm = (T_NET_SET_MENU_PARM *)Fwl_Malloc(sizeof(T_NET_SET_MENU_PARM));
    AK_ASSERT_PTR_VOID(pNetSetMenuParm, "initnetwork_set_menu(): malloc error");
	memset(pNetSetMenuParm, 0, sizeof(T_NET_SET_MENU_PARM));	

    MultiSet_Init(&pNetSetMenuParm->multiSet);
	NetworkSet_GetDispStr();
	NetworkSet_GetContent();
	NetworkSet_SetKeyPrecessCallBackFunc();
    NetworkSet_SetEditShowCallBackFunc();
    
    NetworkSet_SetHitButtonCallBackFunc();

    m_regResumeFunc(resumenetwork_set_menu);
#endif
}

void exitnetwork_set_menu(void)
{
#ifdef SUPPORT_NETWORK
    MultiSet_Free(&pNetSetMenuParm->multiSet);
    pNetSetMenuParm = Fwl_Free(pNetSetMenuParm);
#endif
}

void paintnetwork_set_menu(void)
{
#ifdef SUPPORT_NETWORK

    MultiSet_Show(&pNetSetMenuParm->multiSet);
    MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_NONE);

    Fwl_RefreshDisplay();
#endif
}

unsigned char handlenetwork_set_menu(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_NETWORK

    T_eBACK_STATE   ret;
	T_ITEM_NODE 	*pItemFocus = AK_NULL;
	T_U32			itemFocusId = 0;


    if (IsPostProcessEvent(event))
    {
    	MultiSet_SetRefresh(&pNetSetMenuParm->multiSet, MULTISET_REFRESH_ALL);
        return 1;
    }


	ret = MultiSet_Handler(&pNetSetMenuParm->multiSet, event, pEventParm);
	
    switch (ret)
    {
        case eNext:
			pItemFocus = MultiSet_GetItemFocus(&pNetSetMenuParm->multiSet);
    		itemFocusId = MultiSet_GetItemFocusId(pItemFocus);
			
			if (ITEMID_SEND_FILE == itemFocusId)
			{
				m_triggerEvent(M_EVT_1, pEventParm);
			}
			else if (ITEMID_RECV_PATH == itemFocusId)
			{
				m_triggerEvent(M_EVT_2, pEventParm);
			}
            break;
        default:
            ReturnDefauleProc(ret, pEventParm);
            break;
    }

#endif
    return 0;
}

