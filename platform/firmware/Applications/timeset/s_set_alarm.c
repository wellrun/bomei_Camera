#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_Initialize.h"
#include "Ctl_AlarmSet.h"
#include "Ctl_Msgbox.h"
#include "Eng_KeyMapping.h"
#include "Eng_Time.h"
#include "Eng_DataConvert.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Ctl_IconExplorer.h"
#include "Ctl_MultiSet.h"
#include "Eng_AkBmp.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_oscom.h"
#include "fwl_display.h"


typedef struct {
    T_ALARMSET  alarmSet;
    T_MSGBOX    msgbox;
} T_SETALARM_PARM;

static T_SETALARM_PARM *pSetAlarmParm;

static T_VOID resumeSetAlarm_view(void);
static T_VOID AlarmSet_SetKeyPrecessCallBackFunc(T_ALARMSET *pAlarmSet);
static T_VOID AlarmSet_SetEditShowCallBackFunc(T_ALARMSET *pAlarmSet);
static T_BOOL AlarmSet_DayAlarmTypeSetCallBack(T_MMI_KEYPAD *pPhyKey);
static T_BOOL AlarmSet_WeekAlarmSetCallBack(T_MMI_KEYPAD *pPhyKey);
static T_BOOL AlarmSet_EditAlarmKeyProcessFunc(T_MMI_KEYPAD *pPhyKey);
static T_BOOL AlarmSet_ShowEditTextCallFunc(T_TEXT *pText);

static T_VOID AlarmSet_SetHitButtonCallBackFunc(T_ALARMSET *pAlarmSet);
static T_BOOL AlarmSet_hitButtonCallBack(T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y);
#endif

/*---------------------- BEGIN OF STATE s_set_alarm ------------------------*/
void initset_alarm(void)
{
#ifdef SUPPORT_SYS_SET
    pSetAlarmParm = (T_SETALARM_PARM *)Fwl_Malloc(sizeof(T_SETALARM_PARM));
    AK_ASSERT_PTR_VOID(pSetAlarmParm, "initset_alarm(): malloc error");


    if (AK_TRUE != AlarmSet_Init(&pSetAlarmParm->alarmSet))
    {
        AK_DEBUG_OUTPUT("Time Set init fail!!!");
    }

    AlarmSet_GetContent (&pSetAlarmParm->alarmSet);
    AlarmSet_SetKeyPrecessCallBackFunc(&pSetAlarmParm->alarmSet);
    AlarmSet_SetEditShowCallBackFunc(&pSetAlarmParm->alarmSet);
    
    AlarmSet_SetHitButtonCallBackFunc(&pSetAlarmParm->alarmSet);

    m_regResumeFunc(resumeSetAlarm_view);
#endif
}

void exitset_alarm(void)
{
#ifdef SUPPORT_SYS_SET

    AlarmSet_Free(&pSetAlarmParm->alarmSet);
    pSetAlarmParm = Fwl_Free(pSetAlarmParm);

#endif
}

void paintset_alarm(void)
{
#ifdef SUPPORT_SYS_SET
    AlarmSet_Show(&pSetAlarmParm->alarmSet);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_alarm(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   teditRet;

    if (IsPostProcessEvent(event))
    {
        MultiSet_SetRefresh(&pSetAlarmParm->alarmSet.multiSet, MULTISET_REFRESH_ALL);
        return 1;
    }

    if (MULTISET_MODE_NORMAL == MultiSet_GetCtlMode(&pSetAlarmParm->alarmSet.multiSet))
    {
        AlarmSet_GetAlarmTime(&pSetAlarmParm->alarmSet);
    }

    teditRet = MultiSet_Handler(&pSetAlarmParm->alarmSet.multiSet, event, pEventParm);
    switch (teditRet)
    {
        case eNext:
            m_triggerEvent(M_EVT_3, pEventParm);
            break;

        default:
            ReturnDefauleProc(teditRet, pEventParm);
            break;
    }
#endif
    return 0;
}

#ifdef SUPPORT_SYS_SET

static T_VOID resumeSetAlarm_view(void)
{
    T_ITEM_NODE * pFocusItem = AK_NULL;


    MultiSet_LoadImageData(&pSetAlarmParm->alarmSet.multiSet);
    AlarmSet_SyncAlarmSoundFileName(&pSetAlarmParm->alarmSet);
    
    if (MULTISET_MODE_NORMAL == MultiSet_GetCtlMode(&pSetAlarmParm->alarmSet.multiSet))
    {
        MultiSet_SetRefresh(&pSetAlarmParm->alarmSet.multiSet, MULTISET_REFRESH_ALL);
    }
    else 
    {
        pFocusItem = MultiSet_GetItemFocus(&pSetAlarmParm->alarmSet.multiSet);
        if (ITEM_TYPE_EDIT == pFocusItem->itemType)
        {
            MultiSet_SetRefresh(&pSetAlarmParm->alarmSet.multiSet, MULTISET_REFRESH_EDITAREA);
        }
        else
        {
            MultiSet_SetRefresh(&pSetAlarmParm->alarmSet.multiSet, MULTISET_REFRESH_OPTION);
        }
    }
}

static T_VOID AlarmSet_SetKeyPrecessCallBackFunc(T_ALARMSET *pAlarmSet)
{
    if (AK_NULL != pAlarmSet)
    {
        MultiSet_SetItemKeyProcessCallBack(&pAlarmSet->multiSet, ITEMID_DAYALARMTYPE, \
                                            AlarmSet_DayAlarmTypeSetCallBack);
        MultiSet_SetItemKeyProcessCallBack(&pAlarmSet->multiSet, ITEMID_DAYALARMTIME, \
                                            AlarmSet_EditAlarmKeyProcessFunc);
        MultiSet_SetItemKeyProcessCallBack(&pAlarmSet->multiSet, ITEMID_WEEKALARMTIME, \
                                            AlarmSet_EditAlarmKeyProcessFunc);
        MultiSet_SetItemKeyProcessCallBack(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, \
                                            AlarmSet_WeekAlarmSetCallBack);
    }
}

static T_VOID AlarmSet_SetEditShowCallBackFunc(T_ALARMSET *pAlarmSet)
{
    if (AK_NULL != pAlarmSet)
    {
        MultiSet_SetEditShowCallBack(&pAlarmSet->multiSet, ITEMID_DAYALARMTIME,\
                                    AlarmSet_ShowEditTextCallFunc);
        MultiSet_SetEditShowCallBack(&pAlarmSet->multiSet, ITEMID_WEEKALARMTIME,\
                                    AlarmSet_ShowEditTextCallFunc);
    }
}

static T_VOID AlarmSet_SetHitButtonCallBackFunc(T_ALARMSET *pAlarmSet)
{
    if (AK_NULL != pAlarmSet)
    {
        MultiSet_SetHitButtonCallBack(&pAlarmSet->multiSet, ITEMID_DAYALARMTIME, \
                            AlarmSet_hitButtonCallBack);
        MultiSet_SetHitButtonCallBack(&pAlarmSet->multiSet, ITEMID_WEEKALARMTIME, \
                            AlarmSet_hitButtonCallBack);

    }
}

static T_BOOL AlarmSet_hitButtonCallBack(T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y)
{
    T_pRECT pRect = AK_NULL;
    //T_ICON_TYPE iconType = ICON_LEFT;
    T_EDIT_AREA *pEditArea = AK_NULL;

    if (AK_NULL == pPhyKey)
    {
        return AK_FALSE;
    }

    pEditArea = &pSetAlarmParm->alarmSet.multiSet.editArea;

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

/**
 * @brief   alarm time option key process call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MMI_KEYPAD *pPhyKey: key struct pionter
 * @return  T_BOOL
 * @retval  AK_TRUE: success
 * @retval  AK_FALSE: fail
 */
static T_BOOL AlarmSet_DayAlarmTypeSetCallBack(T_MMI_KEYPAD *pPhyKey)
{
    T_ITEM_NODE *pItemFocus = AK_NULL;
    T_U32        optionFocusId = 0;

    if (AK_NULL != pPhyKey)
    {
        pItemFocus = MultiSet_GetItemFocus(&pSetAlarmParm->alarmSet.multiSet);
        if (AK_NULL != pItemFocus)
        {
            MultiSet_ChangOptionChooseState(pItemFocus);
            optionFocusId = MultiSet_GetOptionFocusId(pItemFocus);
            AlarmSet_SaveAlarmType(optionFocusId);
            AlarmRtcChange();
        }
    }
    return AK_TRUE;
}

/**
 * @brief   alarm time option key process call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MMI_KEYPAD *pPhyKey: key struct pionter
 * @return  T_BOOL
 * @retval  AK_TRUE: success
 * @retval  AK_FALSE: fail
 */
static T_BOOL AlarmSet_WeekAlarmSetCallBack(T_MMI_KEYPAD *pPhyKey)
{
    T_ITEM_NODE *pItemFocus = AK_NULL;
    T_U32       optionFocusId = 0;
    T_BOOL      chooseState = AK_FALSE;
    
    if (AK_NULL != pPhyKey)
    {
        pItemFocus = MultiSet_GetItemFocus(&pSetAlarmParm->alarmSet.multiSet);
        if (AK_NULL != pItemFocus)
        {
            MultiSet_ChangOptionChooseState(pItemFocus);
            optionFocusId = MultiSet_GetOptionFocusId(pItemFocus);

            chooseState = MultiSet_GetOptionFocusChooseState(pItemFocus);
            AlarmSet_SaveWeekAlarmSetup(optionFocusId, chooseState);
            AlarmSet_CheckWeekAlarmSetupState(pSetAlarmParm->alarmSet.weekAlarmState);
            AlarmRtcChange();
        }
    }      

    return AK_TRUE;
}

/**
 * @brief   alarm edit key process call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MMI_KEYPAD *pPhyKey: key struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL AlarmSet_EditAlarmKeyProcessFunc(T_MMI_KEYPAD *pPhyKey)
{
    //T_eBACK_STATE   ret = eStay;
    T_U32           itemId = 0;
    T_SYSTIME       *pAlarmTime = AK_NULL;
    T_ITEM_NODE     *pItemFocus = AK_NULL;
    T_RECT          msgRect;
    
    if (AK_NULL == pPhyKey)
    {
        return AK_FALSE;
    }

    pItemFocus = MultiSet_GetItemFocus(&pSetAlarmParm->alarmSet.multiSet);
    itemId = MultiSet_GetItemFocusId(pItemFocus);
    switch (itemId)
    {
        case ITEMID_DAYALARMTIME:
            pAlarmTime = &pSetAlarmParm->alarmSet.dayAlarmTime;
            break;
        case ITEMID_WEEKALARMTIME:
            pAlarmTime = &pSetAlarmParm->alarmSet.weekAlarmTime;
            break;
        default:
            break;
    }
    
    switch (pPhyKey->keyID)
    {
        case kbOK:
            AlarmSet_SaveAlarmTime(itemId, pAlarmTime);
            pSetAlarmParm->alarmSet.alarmCursor = CURSOR_HOUR;
            AlarmRtcChange();

            MsgBox_InitStr(&pSetAlarmParm->msgbox, 0, GetCustomTitle(ctHINT),\
            GetCustomString(csSUCCESS_DONE), MSGBOX_INFORMATION);
            MsgBox_Show(&pSetAlarmParm->msgbox);
            MsgBox_GetRect(&pSetAlarmParm->msgbox, &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, \
                                msgRect.height);
            Fwl_MiniDelay(1000);

        case kbCLEAR:
            AlarmSet_GetAlarmTime(&pSetAlarmParm->alarmSet);
            MultiSet_SetCtlMode(&pSetAlarmParm->alarmSet.multiSet, MULTISET_MODE_NORMAL);
            MultiSet_SetRefresh(&pSetAlarmParm->alarmSet.multiSet, MULTISET_REFRESH_ALL);
            break;
    
        case kbUP:
            AlarmSet_IncreamAlarmTimeCursor(pAlarmTime, pSetAlarmParm->alarmSet.alarmCursor);
            MultiSet_SetRefresh(&pSetAlarmParm->alarmSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break;

        case kbDOWN:
            AlarmSet_DecreaseAlarmCursor(pAlarmTime, pSetAlarmParm->alarmSet.alarmCursor);
            MultiSet_SetRefresh(&pSetAlarmParm->alarmSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break;
            
        case kbLEFT:
            AlarmSet_MoveLeftdayAlarmCursor(&pSetAlarmParm->alarmSet);
            MultiSet_SetRefresh(&pSetAlarmParm->alarmSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break;

        case kbRIGHT:
            AlarmSet_MoveRightdayAlarmCursor(&pSetAlarmParm->alarmSet);
            MultiSet_SetRefresh(&pSetAlarmParm->alarmSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break;
           
        default:
            break;
    }

    return AK_TRUE;
}

/**
 * @brief   alarm edit show call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TEXT *pText: T_TEXT struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL AlarmSet_ShowEditTextCallFunc(T_TEXT *pText)
{
    T_ITEM_NODE     *pItemFocus = AK_NULL;
    T_SYSTIME       *pAlarmTime = AK_NULL;
    T_U32           itemFocusId = 0;
    T_U16	        strDate[30], strAlarm[30]; 
    T_U32           offset;
    T_S16           PosX, PosY;
    T_U16           Ustrtmp[5];
    T_U16           strLen;
	T_U16           *pStr = AK_NULL;
    T_U16	        *pUStr = AK_NULL;
    T_U8            tempString[8];

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

    pItemFocus = MultiSet_GetItemFocus(&pSetAlarmParm->alarmSet.multiSet);
    itemFocusId = MultiSet_GetItemFocusId(pItemFocus);
    switch (itemFocusId)
    {
        case 1:
            pAlarmTime = &pSetAlarmParm->alarmSet.dayAlarmTime;
            break;
        case 2:
            pAlarmTime = &pSetAlarmParm->alarmSet.weekAlarmTime;
            break;
        default:
            return AK_FALSE;
    }

    ConvertTimeS2UcSByFormat(pAlarmTime, strDate, strAlarm);
    Utl_UStrCpyN(pText->pText, strAlarm, 5);
    pUStr = strAlarm + 8;
    Utl_UStrCat(pText->pText, pUStr);

    Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pText->pText, ~(pText->textColor), CURRENT_FONT_SIZE, \
                            (T_U16)Utl_UStrLen(pText->pText));

    switch (pSetAlarmParm->alarmSet.alarmCursor)
    {
        case CURSOR_HOUR:
            strLen = 0;
            sprintf(tempString, "%02d", pAlarmTime->hour);
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
        case CURSOR_MINUTE:
            strLen = 3;
            sprintf(tempString, "%02d", pAlarmTime->minute);
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
/*            
        case CURSOR_SECOND:
            strLen = 6;
            sprintf(tempString, "%02d", pAlarmTime->second);
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
*/            
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
#endif
