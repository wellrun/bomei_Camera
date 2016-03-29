#include "Fwl_public.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_Initialize.h"
#include "Ctl_TimeSet.h"
#include "Ctl_Msgbox.h"
#include "Ctl_MultiSet.h"
#include "Eng_KeyMapping.h"
#include "Eng_Time.h"
#include "Eng_Alarm.h"
#include "Eng_Font.h"
#include "Eng_AkBmp.h"
#include "Gbl_Global.h"
#include "Eng_AkBmp.h"
#include "Eng_Graph.h"
#include "Lib_geshade.h"
#include "fwl_pfdisplay.h"
#include "Lib_state_api.h"
#include "fwl_oscom.h"
#include "fwl_display.h"


typedef struct {
    T_TIMESET       timeSet;
    T_MSGBOX        msgbox;
} T_SYSCLOCK_PARM;

static T_SYSCLOCK_PARM *pSysclockParm;

static T_VOID resumeSetSysClock_view(T_VOID);
static T_VOID TimeSet_SetKeyPrecessCallBackFunc(T_TIMESET *pTimeSet);
static T_VOID TimeSet_SetEditShowCallBackFunc(T_TIMESET *pTimeSet);
static T_BOOL TimeSet_EditTimeKeyProcessFunc(T_MMI_KEYPAD *pPhyKey);
static T_BOOL TimeSet_ShowEditTimeCallBack(T_TEXT *pText);
static T_BOOL TimeSet_EditDateKeyProcessFunc(T_MMI_KEYPAD *pPhyKey);
static T_BOOL TimeSet_ShowEditDateCallBack(T_TEXT *pText);
static T_BOOL TimeSet_SaveDayFormatCallBack(T_MMI_KEYPAD *pPhyKey);
static T_BOOL TimeSet_SaveDaySeparatorCallBack(T_MMI_KEYPAD *pPhyKey);
static T_BOOL TimeSet_SaveTimeFormatCallBack(T_MMI_KEYPAD *pPhyKey);
static T_BOOL TimeSet_SaveTimeSeparatorCallBack(T_MMI_KEYPAD *pPhyKey);

static T_VOID TimeSet_SetHitButtonCallBackFunc(T_TIMESET *pTimeSet);
static T_BOOL TimeSet_hitButtonCallBack(T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y);
#endif
/*---------------------- BEGIN OF STATE s_set_sysclock ------------------------*/
void initset_sysclock(void)
{
#ifdef SUPPORT_SYS_SET

    pSysclockParm = (T_SYSCLOCK_PARM *)Fwl_Malloc(sizeof(T_SYSCLOCK_PARM));
    AK_ASSERT_PTR_VOID(pSysclockParm, "initset_disp_contrast(): malloc error");


    if (AK_TRUE != TimeSet_Init(&pSysclockParm->timeSet))
    {
        AK_DEBUG_OUTPUT("Time Set init fail!!!");
    }

    TimeSet_GetContent (&pSysclockParm->timeSet);
    TimeSet_SetKeyPrecessCallBackFunc(&pSysclockParm->timeSet);
    TimeSet_SetEditShowCallBackFunc(&pSysclockParm->timeSet);

    TimeSet_SetHitButtonCallBackFunc(&pSysclockParm->timeSet);
    
    m_regResumeFunc(resumeSetSysClock_view);
#endif
}

void exitset_sysclock(void)
{
#ifdef SUPPORT_SYS_SET

    TimeSet_Free(&pSysclockParm->timeSet);
    pSysclockParm = Fwl_Free(pSysclockParm);
#endif
}

void paintset_sysclock(void)
{
#ifdef SUPPORT_SYS_SET
    TimeSet_Show(&pSysclockParm->timeSet);
    GE_StartShade();
    Fwl_RefreshDisplay();
#endif
}

unsigned char handleset_sysclock(T_EVT_CODE event, T_EVT_PARAM *pEventParm)
{
#ifdef SUPPORT_SYS_SET

    T_eBACK_STATE   teditRet;

    if (IsPostProcessEvent(event))
    {
        return 1;
    }

    if (MULTISET_MODE_NORMAL == MultiSet_GetCtlMode(&pSysclockParm->timeSet.multiSet))
    {
        TimeSet_SyncSystemClock(&pSysclockParm->timeSet);
    }

    teditRet = MultiSet_Handler(&pSysclockParm->timeSet.multiSet, event, pEventParm);
    switch (teditRet)
    {
        case eStay:
            break;
        case eReturn:
        default:
            ReturnDefauleProc(teditRet, pEventParm);
            break;
    }
#endif
    return 0;
}

#ifdef SUPPORT_SYS_SET

////////////////////////////////////////////////////////////////////////////////
static T_VOID resumeSetSysClock_view(T_VOID)
{
    T_ITEM_NODE * pFocusItem = AK_NULL;

    MultiSet_LoadImageData(&pSysclockParm->timeSet.multiSet);

    if (MULTISET_MODE_NORMAL == MultiSet_GetCtlMode(&pSysclockParm->timeSet.multiSet))
    {
        MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_ALL);
    }
    else 
    {
        pFocusItem = MultiSet_GetItemFocus(&pSysclockParm->timeSet.multiSet);
        if (ITEM_TYPE_EDIT == pFocusItem->itemType)
        {
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_EDITAREA);
        }
        else
        {
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_OPTION);
        }
    }
}

static T_VOID TimeSet_SetKeyPrecessCallBackFunc(T_TIMESET *pTimeSet)
{
    if (AK_NULL != pTimeSet)
    {
        MultiSet_SetItemKeyProcessCallBack(&pTimeSet->multiSet, ITEMID_TIME,\
                                            TimeSet_EditTimeKeyProcessFunc);
        MultiSet_SetItemKeyProcessCallBack(&pTimeSet->multiSet, ITEMID_DAY, \
                                            TimeSet_EditDateKeyProcessFunc);
        MultiSet_SetItemKeyProcessCallBack(&pTimeSet->multiSet, ITEMID_DAYFORMAT,\
                                            TimeSet_SaveDayFormatCallBack);
        MultiSet_SetItemKeyProcessCallBack(&pTimeSet->multiSet, ITEMID_DAYSEPARATOR,\
                                            TimeSet_SaveDaySeparatorCallBack);
        MultiSet_SetItemKeyProcessCallBack(&pTimeSet->multiSet, ITEMID_TIMEFORMAT, \
                                            TimeSet_SaveTimeFormatCallBack);
        MultiSet_SetItemKeyProcessCallBack(&pTimeSet->multiSet, ITEMID_TIMESEPARATOR, \
                                            TimeSet_SaveTimeSeparatorCallBack);
    }
}

static T_VOID TimeSet_SetEditShowCallBackFunc(T_TIMESET *pTimeSet)
{
    if (AK_NULL != pTimeSet)
    {
        MultiSet_SetEditShowCallBack(&pTimeSet->multiSet, ITEMID_TIME, \
                                       TimeSet_ShowEditTimeCallBack);
        MultiSet_SetEditShowCallBack(&pTimeSet->multiSet, ITEMID_DAY, \
                                        TimeSet_ShowEditDateCallBack);
    }
}

static T_VOID TimeSet_SetHitButtonCallBackFunc(T_TIMESET *pTimeSet)
{
    if (AK_NULL != pTimeSet)
    {
        MultiSet_SetHitButtonCallBack(&pTimeSet->multiSet, ITEMID_TIME, \
                            TimeSet_hitButtonCallBack);
        MultiSet_SetHitButtonCallBack(&pTimeSet->multiSet, ITEMID_DAY, \
                            TimeSet_hitButtonCallBack);

    }
}

static T_BOOL TimeSet_hitButtonCallBack(T_MMI_KEYPAD *pPhyKey, T_POS x, T_POS y)
{
    T_pRECT pRect = AK_NULL;
    //T_ICON_TYPE iconType = ICON_LEFT;
    T_EDIT_AREA *pEditArea = AK_NULL;

    if (AK_NULL == pPhyKey)
    {
        return AK_FALSE;
    }

    pEditArea = &pSysclockParm->timeSet.multiSet.editArea;

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
 * @brief   time edit key process call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MMI_KEYPAD *pPhyKey:press key struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL TimeSet_EditTimeKeyProcessFunc(T_MMI_KEYPAD *pPhyKey)
{
    T_RECT          msgRect;

    AK_ASSERT_PTR(pPhyKey, "TimeSet_KeyProcessFunc(): pPhyKey", AK_FALSE);

    switch (pPhyKey->keyID)
    {
       case kbOK:
            TimeSet_SaveEditTimeCursor(&pSysclockParm->timeSet);
            AlarmRtcChange();

            MsgBox_InitStr(&pSysclockParm->msgbox, 0, GetCustomTitle(ctHINT),\
                GetCustomString(csSUCCESS_DONE), MSGBOX_INFORMATION);
            MsgBox_Show(&pSysclockParm->msgbox);
            MsgBox_GetRect(&pSysclockParm->msgbox, &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, \
                                msgRect.height);
            Fwl_MiniDelay(1000);

       case kbCLEAR:
            TimeSet_SyncSystemClock(&pSysclockParm->timeSet);
            pSysclockParm->timeSet.timeCursor = CURSOR_HOUR;
            MultiSet_SetCtlMode(&pSysclockParm->timeSet.multiSet, MULTISET_MODE_NORMAL);
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_ALL);
            break;                

        case kbUP:
            TimeSet_IncreamTimeCursor(&pSysclockParm->timeSet);
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break;

        case kbDOWN:
            TimeSet_DecreaseTimeCursor(&pSysclockParm->timeSet);
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break;                
            
        case kbLEFT:
            TimeSet_MoveLeftTimeCursor(&pSysclockParm->timeSet);
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break;                

        case kbRIGHT:
            TimeSet_MoveRightTimeCursor(&pSysclockParm->timeSet);
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break; 
           
        default:
            break;
    }

    return AK_TRUE;
}

/**
 * @brief   date edit key process call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MMI_KEYPAD *pPhyKey:press key struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL TimeSet_EditDateKeyProcessFunc(T_MMI_KEYPAD *pPhyKey)
{
    T_RECT          msgRect;

    AK_ASSERT_PTR(pPhyKey, "TimeSet_KeyProcessFunc(): pPhyKey", AK_FALSE);

    switch (pPhyKey->keyID)
    {
        case kbOK:
            TimeSet_SaveEditDateCursor(&pSysclockParm->timeSet);
            AlarmRtcChange();

            MsgBox_InitStr(&pSysclockParm->msgbox, 0, GetCustomTitle(ctHINT),\
                            GetCustomString(csSUCCESS_DONE), MSGBOX_INFORMATION);
            MsgBox_Show(&pSysclockParm->msgbox);
            MsgBox_GetRect(&pSysclockParm->msgbox, &msgRect);
            Fwl_InvalidateRect( msgRect.left, msgRect.top, msgRect.width, msgRect.height);
            Fwl_MiniDelay(1000);

        case kbCLEAR:
            TimeSet_SyncSystemClock(&pSysclockParm->timeSet);
            pSysclockParm->timeSet.dayCursor = CURSOR_FIRST;
            MultiSet_SetCtlMode(&pSysclockParm->timeSet.multiSet, MULTISET_MODE_NORMAL);
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_ALL);
            break;                
            
        case kbUP:
            TimeSet_EditDay_IncreamCursor(&pSysclockParm->timeSet);
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break;

        case kbDOWN:
            TimeSet_EditDay_DecreaseCursor(&pSysclockParm->timeSet);
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break;                
            
        case kbLEFT:
            TimeSet_EditDate_MoveLeftCursor(&pSysclockParm->timeSet);
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break;                

        case kbRIGHT:
            TimeSet_EditDate_MoveRightCursor(&pSysclockParm->timeSet);
            MultiSet_SetRefresh(&pSysclockParm->timeSet.multiSet, MULTISET_REFRESH_EDITAREA);
            break; 

        default:
            break;
    }

    return AK_TRUE;
}

/**
 * @brief   show edit call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TEXT *pText:T_TEXT struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL TimeSet_ShowEditTimeCallBack(T_TEXT *pText)
{
    T_U16	        strDate[30], strTime[30]; 
    T_U32           offset;
    T_S16           PosX, PosY;
    T_U16           Ustrtmp[5];
    T_U16           strLen;
	T_U16           *pStr = AK_NULL;
    //T_U16	        tmpStr[8] = {0};
    T_U8            tempString[8];

    if (AK_TRUE != MultiSet_GetTextPos(pText->pText, &pText->rect, pText->textStyle, &PosX, &PosY))
    {
        return AK_FALSE;
    }

    pText->cursor.left = PosX;
    pText->cursor.top = PosY;
    pText->cursor.height = g_Font.SCHEIGHT;
	pStr = pText->pText;

    /*
	//display background
    if (AK_NULL == pText->pBackData->pImgDt)
    {
        Fwl_FillSolidRect(HRGB_LAYER, pText->rect.left, pText->rect.top, pText->rect.width, pText->rect.height, \
                            pText->backColor);
    }
    else
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pText->rect.left, pText->rect.top, (T_pCDATA)pText->pBackData->pImgDt, &g_Graph.TransColor, AK_FALSE);
    }
    */
    
    ConvertTimeS2UcSByFormat(&pSysclockParm->timeSet.sysTime, strDate, strTime);
    Utl_UStrCpy(pText->pText, strTime);

    Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pText->pText, ~(pText->textColor), CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pText->pText));
    
    switch (pSysclockParm->timeSet.timeCursor)
    {
        case CURSOR_HOUR:
            strLen = 0;
            sprintf(tempString, "%02d", pSysclockParm->timeSet.sysTime.hour);
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
        case CURSOR_MINUTE:
            strLen = 3;
            sprintf(tempString, "%02d", pSysclockParm->timeSet.sysTime.minute);
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
        case CURSOR_SECOND:
            strLen = 6;
            sprintf(tempString, "%02d", pSysclockParm->timeSet.sysTime.second);
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
    }

	//calc cursor abscissa
	pStr += strLen; 
    offset = UGetSpeciStringWidth((T_U16 *)pText->pText, CURRENT_FONT_SIZE, strLen);
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

/**
 * @brief   show edit call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TEXT *pText:T_TEXT struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
static T_BOOL TimeSet_ShowEditDateCallBack(T_TEXT *pText)
{
    T_U16	        strDate[30], strTime[30]; 
    T_U32           offset;
    T_S16           PosX, PosY;
    T_U16           Ustrtmp[5];
    T_U16           strLen;
	T_U16           *pStr;
    T_U8            tempString[8];

    if (AK_TRUE != MultiSet_GetTextPos(pText->pText, &pText->rect, pText->textStyle, &PosX, &PosY))
    {
        return AK_FALSE;
    }

    pText->cursor.left = PosX;
    pText->cursor.top = PosY;
    pText->cursor.height = g_Font.SCHEIGHT;
	pStr = pText->pText;

	/*//display background
    if (AK_NULL == pText->pBackData->pImgDt)
    {
        Fwl_FillSolidRect(HRGB_LAYER, pText->rect.left, pText->rect.top, \
                    pText->rect.width, pText->rect.height, pText->backColor);
    }
    else
    {
        Fwl_AkBmpDrawFromString(HRGB_LAYER, pText->rect.left, pText->rect.top, \
             (T_pCDATA)pText->pBackData->pImgDt, &g_Graph.TransColor, AK_FALSE);
    }
    */
    ConvertTimeS2UcSByFormat(&pSysclockParm->timeSet.sysTime, strDate, strTime);

    Utl_UStrCpy(pText->pText, strDate);
    Fwl_UDispSpeciString(HRGB_LAYER, PosX, PosY, pText->pText, ~(pText->textColor), \
                CURRENT_FONT_SIZE, (T_U16)Utl_UStrLen(pText->pText));

    switch (gs.DayFormat)
    {
        case DAY_FORMAT_MDY://m/d/y
            switch (pSysclockParm->timeSet.dayCursor)
            {
                case CURSOR_FIRST:
                    strLen = 0;
                    sprintf(tempString, "%02d", pSysclockParm->timeSet.sysTime.month);
                    break;
                    
                case CURSOR_SECND:
                    strLen = 3;
                    sprintf(tempString, "%02d", pSysclockParm->timeSet.sysTime.day);
                    break;
                    
                case CURSOR_THIRD:
                    strLen = 6;
                    sprintf(tempString, "%04d", pSysclockParm->timeSet.sysTime.year);
                    break;
            }
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
            
        case DAY_FORMAT_DMY://d/m/y
            switch (pSysclockParm->timeSet.dayCursor)
            {
                case CURSOR_FIRST:
                    strLen = 0;
                    sprintf(tempString, "%02d", pSysclockParm->timeSet.sysTime.day);
                    break;
                    
                case CURSOR_SECND:
                    strLen = 3;
                    sprintf(tempString, "%02d", pSysclockParm->timeSet.sysTime.month);
                    break;
                    
                case CURSOR_THIRD:
                    strLen = 6;
                    sprintf(tempString, "%04d", pSysclockParm->timeSet.sysTime.year);
                    break;
            }
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
            
        default://y/m/r
            switch (pSysclockParm->timeSet.dayCursor)
            {
                case CURSOR_FIRST:
                    strLen = 0;
                    sprintf(tempString, "%04d", pSysclockParm->timeSet.sysTime.year);
                    break;
                    
                case CURSOR_SECND:
                    strLen = 5;
                    sprintf(tempString, "%02d", pSysclockParm->timeSet.sysTime.month);
                    break;
                    
                case CURSOR_THIRD:
                    strLen = 8;
                    sprintf(tempString, "%02d", pSysclockParm->timeSet.sysTime.month);
                    break;
            }
            Eng_StrMbcs2Ucs(tempString, Ustrtmp);
            break;
    }
	
	//calc cursor abscissa
	pStr += strLen; 
    offset = UGetSpeciStringWidth((T_U16 *)pText->pText, CURRENT_FONT_SIZE, strLen);
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

/**
 * @brief   alarm time option key process call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MMI_KEYPAD *pPhyKey: key struct pionter
 * @return  T_eBACK_STATE
 * @retval  AK_TRUE: success
 * @retval  AK_FALSE: fail
 */
static T_BOOL TimeSet_SaveDayFormatCallBack(T_MMI_KEYPAD *pPhyKey)
{
    T_ITEM_NODE *pItemFocus = AK_NULL;
    T_U32       optionFocusId = 0;

    if (AK_NULL != pPhyKey)
    {
        pItemFocus = MultiSet_GetItemFocus(&pSysclockParm->timeSet.multiSet);
        if (AK_NULL != pItemFocus)
        {
            MultiSet_ChangOptionChooseState(pItemFocus);
            optionFocusId = MultiSet_GetOptionFocusId(pItemFocus);
            TimeSet_SaveDayFormat(optionFocusId);
            TimeSet_SyncSystemClock(&pSysclockParm->timeSet);
        }
    }
    return AK_TRUE;
}

/**
 * @brief   alarm time option key process call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MMI_KEYPAD *pPhyKey: key struct pionter
 * @return  T_eBACK_STATE
 * @retval  AK_TRUE: success
 * @retval  AK_FALSE: fail
 */
static T_BOOL TimeSet_SaveDaySeparatorCallBack(T_MMI_KEYPAD *pPhyKey)
{
    T_ITEM_NODE *pItemFocus = AK_NULL;
    T_U32       optionFocusId = 0;
    //T_BOOL      chooseState = AK_FALSE;
    
    if (AK_NULL != pPhyKey)
    {
        pItemFocus = MultiSet_GetItemFocus(&pSysclockParm->timeSet.multiSet);
        if (AK_NULL != pItemFocus)
        {
            MultiSet_ChangOptionChooseState(pItemFocus);
            optionFocusId = MultiSet_GetOptionFocusId(pItemFocus);
            TimeSet_SaveDaySeparator(optionFocusId);
            TimeSet_SyncSystemClock(&pSysclockParm->timeSet);
        }      
    }
    return AK_TRUE;
}

/**
 * @brief   alarm time option key process call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MMI_KEYPAD *pPhyKey: key struct pionter
 * @return  T_eBACK_STATE
 * @retval  AK_TRUE: success
 * @retval  AK_FALSE: fail
 */
static T_BOOL TimeSet_SaveTimeFormatCallBack(T_MMI_KEYPAD *pPhyKey)
{
    T_ITEM_NODE *pItemFocus = AK_NULL;
    T_U32        optionFocusId = 0;

    if (AK_NULL != pPhyKey)
    {
        pItemFocus = MultiSet_GetItemFocus(&pSysclockParm->timeSet.multiSet);
        if (AK_NULL != pItemFocus)
        {
            MultiSet_ChangOptionChooseState(pItemFocus);
            optionFocusId = MultiSet_GetOptionFocusId(pItemFocus);
            TimeSet_SaveTimeFormat(optionFocusId); 
            TimeSet_SyncSystemClock(&pSysclockParm->timeSet);
        }
    }
    return AK_TRUE;
}

/**
 * @brief   alarm time option key process call back function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_MMI_KEYPAD *pPhyKey: key struct pionter
 * @return  T_eBACK_STATE
 * @retval  AK_TRUE: success
 * @retval  AK_FALSE: fail
 */
static T_BOOL TimeSet_SaveTimeSeparatorCallBack(T_MMI_KEYPAD *pPhyKey)
{
    T_ITEM_NODE *pItemFocus = AK_NULL;
    T_U32       optionFocusId = 0;
    //T_BOOL      chooseState = AK_FALSE;
    
    if (AK_NULL != pPhyKey)
    {
        pItemFocus = MultiSet_GetItemFocus(&pSysclockParm->timeSet.multiSet);
        if (AK_NULL != pItemFocus)
        {
            MultiSet_ChangOptionChooseState(pItemFocus);
            optionFocusId = MultiSet_GetOptionFocusId(pItemFocus);
            TimeSet_SaveTimeSeparator(optionFocusId);
            TimeSet_SyncSystemClock(&pSysclockParm->timeSet);
        }
    }      

    return AK_TRUE;
}
#endif
