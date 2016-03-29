
/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  * @File name: Ctl_AlarmSet.h
  * @Function:  This file is a part of the state machine s_set_alarm
  * @Author:    WangWei
  * @Date:      2008-05-04
  * @Version:   1.0
  */

#include "Ctl_AlarmSet.h"
#ifdef SUPPORT_SYS_SET

#include "Fwl_osMalloc.h"
#include "Ctl_MsgBox.h"
#include "Eng_DynamicFont.h"
#include "Eng_Font.h"
#include "Eng_KeyMapping.h"
#include "Eng_Alarm.h"
#include "Eng_AkBmp.h"
#include "Eng_GblString.h"
#include "Eng_String_UC.h"
#include "Eng_DataConvert.h"
#include "Ctl_MultiSet.h"
#include "Lib_res_port.h"
#include "Fwl_rtc.h"

static T_VOID AlarmSet_AlarmSoundFileProcess(T_LEN rectWidth, T_U16 *pFileName);

/**
 * @brief   alarm setup init
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  init success
 * @retval  AK_FALSE init fail 
 */
T_BOOL  AlarmSet_Init(T_ALARMSET *pAlarmSet)
{
    AK_ASSERT_PTR(pAlarmSet, "AlarmSet_Init(): pAlarmSet is AK_NULL", AK_FALSE);

    MultiSet_Init(&pAlarmSet->multiSet);
    pAlarmSet->alarmCursor = CURSOR_HOUR;

    AlarmSet_GetAlarmTime(pAlarmSet);

    return AK_TRUE;
}

/**
 * @brief   get alarm setup resource
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL  AlarmSet_GetContent (T_ALARMSET *pAlarmSet)
{
    //T_U32   strWidth    = 0;
	T_U32	i			= 0;
	//T_U16   strLen      = 0;
    T_LEN   rectWidth   = 0;

    //main title
    MultiSet_SetTitleText(&pAlarmSet->multiSet, \
                        Res_GetStringByID(eRES_STR_ALARM_SETUP), COLOR_BLACK);

    //day alarm type
    MultiSet_AddItemWithOption(&pAlarmSet->multiSet, ITEMID_DAYALARMTYPE, Res_GetStringByID(eRES_STR_DAY_ALARM_TYPE), \
                                AK_NULL, ITEM_TYPE_RADIO);
    MultiSet_AddItemOption(&pAlarmSet->multiSet, ITEMID_DAYALARMTYPE, 0, Res_GetStringByID(eRES_STR_ALARM_TYPE_NULL));
    MultiSet_AddItemOption(&pAlarmSet->multiSet, ITEMID_DAYALARMTYPE, 1, Res_GetStringByID(eRES_STR_ALARM_TYPE_ONCE));
    MultiSet_AddItemOption(&pAlarmSet->multiSet, ITEMID_DAYALARMTYPE, 2, Res_GetStringByID(eRES_STR_ALARM_TYPE_CONTINUE));
    MultiSet_SetOptionFocus(&pAlarmSet->multiSet, ITEMID_DAYALARMTYPE, gs.AlarmClock.DayAlarmType);

    //day alarm time
    MultiSet_AddItemWithOption(&pAlarmSet->multiSet, ITEMID_DAYALARMTIME, Res_GetStringByID(eRES_STR_DAY_ALARM_TIME), \
                            pAlarmSet->str_dayAlarm, ITEM_TYPE_EDIT);

    //week alarm time
    MultiSet_AddItemWithOption(&pAlarmSet->multiSet, ITEMID_WEEKALARMTIME,Res_GetStringByID(eRES_STR_WEEK_ALARM_TIME), \
                            pAlarmSet->str_weekAlarm, ITEM_TYPE_EDIT);

    //week alarm set
    MultiSet_AddItemWithOption(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, Res_GetStringByID(eRES_STR_WEEK_ALARM_SETUP), \
                            pAlarmSet->weekAlarmState, ITEM_TYPE_CHECK);
    MultiSet_AddItemOption(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, 0, Res_GetStringByID(eRES_STR_WEEK_SUNDAY));
    MultiSet_AddItemOption(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, 1, Res_GetStringByID(eRES_STR_WEEK_MONDAY));
    MultiSet_AddItemOption(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, 2, Res_GetStringByID(eRES_STR_WEEK_TUESDAY));
    MultiSet_AddItemOption(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, 3, Res_GetStringByID(eRES_STR_WEEK_WEDNESDAY));
    MultiSet_AddItemOption(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, 4, Res_GetStringByID(eRES_STR_WEEK_THURSDAY));
    MultiSet_AddItemOption(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, 5, Res_GetStringByID(eRES_STR_WEEK_FRIDAY));
    MultiSet_AddItemOption(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, 6, Res_GetStringByID(eRES_STR_WEEK_SATURDAY));
    for (i = 0; i < 7; i++)
    {
        MultiSet_SetCheckBoxState(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, i, gs.AlarmClock.WeekAlarmEnable[i]);
    }
    AlarmSet_CheckWeekAlarmSetupState(pAlarmSet->weekAlarmState);
    MultiSet_SetOptionFocus(&pAlarmSet->multiSet, ITEMID_WEEKALARMSET, 0);

    //alarm sound set
    MultiSet_AddItemWithOption(&pAlarmSet->multiSet, ITEMID_ALARMSOUND, Res_GetStringByID(eRES_STR_ALARM_SOND_SETUP), \
                            pAlarmSet->alarmSoundName, ITEM_TYPE_NONE);
    rectWidth = MultiSet_GetTextRectWidth(&pAlarmSet->multiSet);
    AlarmSet_AlarmSoundFileProcess(rectWidth, pAlarmSet->alarmSoundName);

    MultiSet_CheckScrollBar(&pAlarmSet->multiSet);
    
    MultiSet_SetRefresh(&pAlarmSet->multiSet, MULTISET_REFRESH_ALL);
    
    return AK_TRUE;
}

/**
 * @brief   free alarm setup resource
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARMSET *pAlarmSet: T_ALARMSET struct pointer
 * @return  T_VOID
 */
T_VOID  AlarmSet_Free(T_ALARMSET *pAlarmSet)
{
    if (AK_NULL != pAlarmSet)
    {
        MultiSet_Free(&pAlarmSet->multiSet);
    }
}

/**
 * @brief   alarm setup show function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARMSET *pAlarmSet: T_ALARMSET struct pointer
 * @return  T_VOID
 */
T_VOID  AlarmSet_Show(T_ALARMSET *pAlarmSet)
{
    if (AK_NULL == pAlarmSet)
    {
        AK_DEBUG_OUTPUT("AlarmSet_Show():pAlarmSet is AK_NULL");
        return;
    }
    else
    {
        MultiSet_Show(&pAlarmSet->multiSet);
        MultiSet_SetRefresh(&pAlarmSet->multiSet, MULTISET_REFRESH_NONE);
    }
}

/**
 * @brief   synchronize alarm sound file name
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_VOID
 * @return  T_VOID
 */
T_VOID AlarmSet_SyncAlarmSoundFileName(T_ALARMSET *pAlarmSet)
{
    T_LEN rectWidth = 0;

    if (AK_NULL == pAlarmSet)
    {
        return;
    }
    rectWidth = MultiSet_GetTextRectWidth(&pAlarmSet->multiSet);
    AlarmSet_AlarmSoundFileProcess(rectWidth, pAlarmSet->alarmSoundName);
}

/**
 * @brief   alarm sound file name process function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_LEN rectWidth; display rect width
 * @param   T_U16 *pFileName; alarm sound file name pointer 
 * @return  T_VOID
 */
static T_VOID AlarmSet_AlarmSoundFileProcess(T_LEN rectWidth, T_U16 *pFileName)
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
		if (0 == (T_U16)Utl_UStrLen(gs.AlarmClock.AlarmSoundPathName))
		{
			Utl_UStrCpy(pFileName, Res_GetStringByID(eRES_STR_ALARM_SOND_SETUP_NULL));
		}
		else
		{
            //adjust whether the file path is valid
            if(!Fwl_FileExist(gs.AlarmClock.AlarmSoundPathName))
            {
                gs.AlarmClock.AlarmSoundPathName[0] = UNICODE_END;
                Utl_UStrCpy(pFileName, Res_GetStringByID(eRES_STR_ALARM_SOND_SETUP_NULL));
            }
            else 
            {                
                Utl_USplitFilePath(gs.AlarmClock.AlarmSoundPathName, filePath, fileName);
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

/**
 * @brief   check week alarm setup state
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U16 *pWeekAlarmState:week alarm setup item attribute string pointer
 * @return  T_VOID
 */
T_VOID AlarmSet_CheckWeekAlarmSetupState(T_U16 *pWeekAlarmState)
{
    T_U32   i, count;

    if (AK_NULL == pWeekAlarmState)
    {
        return;
    }
    else
    {
        count = 0;
        for (i = 0; i < 7; i++)
        {
            if (AK_TRUE == gs.AlarmClock.WeekAlarmEnable[i])
            {
                count++;
            }
        }

        if (7 == count)
        {
    		Utl_UStrCpy(pWeekAlarmState, Res_GetStringByID(eRES_STR_WEEK_ALARM_ALL_ON));
            gs.AlarmClock.WeekAlarmAllType = WEEK_ALARM_ALL_OPEN;
        }
        else if (0 == count)
        {
    		Utl_UStrCpy(pWeekAlarmState, Res_GetStringByID(eRES_STR_WEEK_ALARM_ALL_OFF));
            gs.AlarmClock.WeekAlarmAllType = WEEK_ALARM_ALL_CLOSE;
        }
        else
        { 
    		Utl_UStrCpy(pWeekAlarmState, Res_GetStringByID(eRES_STR_WEEK_ALARM_USER));
            gs.AlarmClock.WeekAlarmAllType = WEEK_ALARM_ALL_USER;
        }           
    }
}

/**
 * @brief   get alarm time
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARMSET *pAlarmSet : T_ALARMSET struct pointer
 * @return  T_VOID
 */
T_VOID AlarmSet_GetAlarmTime(T_ALARMSET *pAlarmSet)
{
    T_U16	strDate[30];
	T_U16	strTime[30];
    T_U32   rtcTime = 0; 
    T_U32   rtc_ymd_time = 0;
    //T_U16   tmpStr[8] = {0};
    T_U16   *pUstr = AK_NULL;


    if (AK_NULL == pAlarmSet)
    {
        AK_DEBUG_OUTPUT("AlarmSet_GetAlarmTime(): pAlarmSet is AK_NULL");
        return;
    }

    rtcTime = Fwl_RTCGetCount();
    rtc_ymd_time = rtcTime - rtcTime % ONE_DAY_SECOND;

    ConvertSecondsToSysTime((rtc_ymd_time + gs.AlarmClock.DayAlarm), &pAlarmSet->dayAlarmTime);
    ConvertSecondsToSysTime((rtc_ymd_time + gs.AlarmClock.WeekAlarm), &pAlarmSet->weekAlarmTime);

    ConvertTimeS2UcSByFormat(&pAlarmSet->dayAlarmTime, strDate, strTime);
    Utl_UStrCpyN(pAlarmSet->str_dayAlarm, strTime, 5);
    pUstr = strTime + 8;
    Utl_UStrCat(pAlarmSet->str_dayAlarm, pUstr);

    ConvertTimeS2UcSByFormat(&pAlarmSet->weekAlarmTime, strDate, strTime);
    Utl_UStrCpyN(pAlarmSet->str_weekAlarm, strTime, 5);
    pUstr = strTime + 8;
    Utl_UStrCat(pAlarmSet->str_weekAlarm, pUstr);

    MultiSet_SetRefresh(&pAlarmSet->multiSet, MULTISET_REFRESH_ALL);
}

/**
 * @brief   movie cursor to left
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARMSET *pAlarmSet: T_ALARMSET struct pointer
 * @return  T_VOID
 */
T_VOID AlarmSet_MoveLeftdayAlarmCursor(T_ALARMSET *pAlarmSet)
{
    if (AK_NULL == pAlarmSet)
    {
        return;
    }

    switch (pAlarmSet->alarmCursor)
    {/*
        case CURSOR_SECOND:
            pAlarmSet->alarmCursor = CURSOR_MINUTE;
            break;
         */   
        case CURSOR_MINUTE:
            pAlarmSet->alarmCursor = CURSOR_HOUR;
            break;

        case CURSOR_HOUR:
        default:
            //pAlarmSet->alarmCursor = CURSOR_SECOND;
            pAlarmSet->alarmCursor = CURSOR_MINUTE;
            break;
    }
}

/**
 * @brief   movie cursor to right
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARMSET *pAlarmSet: T_ALARMSET struct pointer
 * @return  T_VOID
 */
T_VOID AlarmSet_MoveRightdayAlarmCursor(T_ALARMSET *pAlarmSet)
{
    if (AK_NULL == pAlarmSet)
    {
        return;
    }

    switch (pAlarmSet->alarmCursor)
    {
        case CURSOR_HOUR:
            pAlarmSet->alarmCursor = CURSOR_MINUTE;
            break;
/*            
        case CURSOR_MINUTE:
            pAlarmSet->alarmCursor = CURSOR_SECOND;
            break;
*/
        //case CURSOR_SECOND:
        case CURSOR_MINUTE:
        default:
            pAlarmSet->alarmCursor = CURSOR_HOUR;
            break;
    }
}

/**
 * @brief   incream cursor data
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_SYSTIME *pAlarmTime: modify T_SYSTIME struct pointer
 * @param   T_ALARM_CURSOR alarmCursor: cursor
 * @return  T_BOOL
 * @retval  AK_TRUE: success
 * @retval  AK_FALSE: fail
 */
T_BOOL AlarmSet_IncreamAlarmTimeCursor(T_SYSTIME *pAlarmTime, T_ALARM_CURSOR alarmCursor)
{
    AK_ASSERT_PTR(pAlarmTime, "AlarmSet_KeyProcessFunc(): pPhyKey", AK_FALSE);

    switch(alarmCursor)
    {
        case CURSOR_HOUR:
            pAlarmTime->hour++;
            if (pAlarmTime->hour > 23)
            {
                pAlarmTime->hour = 0;
            }
            break;
            
        case CURSOR_MINUTE:
            pAlarmTime->minute++;
            if (pAlarmTime->minute > 59)
            {
                pAlarmTime->minute = 0;
            }            
            break;
            
        case CURSOR_SECOND:
            pAlarmTime->second++; 
            if (pAlarmTime->second > 59)
            {
                pAlarmTime->second = 0;
            }            
            break;
            
        default:
            break;
    }

	return AK_TRUE;
}

/**
 * @brief   decrease cursor data
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_SYSTIME *pAlarmTime: modify T_SYSTIME struct pointer
 * @param   T_ALARM_CURSOR alarmCursor: cursor
 * @return  T_BOOL
 * @retval  AK_TRUE: success
 * @retval  AK_FALSE: fail
 */
T_BOOL AlarmSet_DecreaseAlarmCursor(T_SYSTIME *pAlarmTime, T_ALARM_CURSOR alarmCursor)
{
    AK_ASSERT_PTR(pAlarmTime, "AlarmSet_KeyProcessFunc(): pPhyKey", AK_FALSE);

    switch(alarmCursor)
    {
        case CURSOR_HOUR:
            if (0 == pAlarmTime->hour)
            {
                pAlarmTime->hour = 23;
            }
			else
			{
				pAlarmTime->hour--;
			}
            break;
            
        case CURSOR_MINUTE:
            if (0 == pAlarmTime->minute)
            {
                pAlarmTime->minute = 59;
            }  
			else
			{
				pAlarmTime->minute--;
			}
            break;
            
        case CURSOR_SECOND:
            if (0 == pAlarmTime->second)
            {
                pAlarmTime->second = 59;
            }     
			else
			{
	            pAlarmTime->second--; 
			}
            break;
            
        default:
            break;
    }

	return AK_TRUE;
}

/**
 * @brief   save alarm time
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U32 itemId: item id
 * @param   T_SYSTIME *pAlarmTime: modify T_SYSTIME struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE: success
 * @retval  AK_FALSE: fail
 */
T_BOOL AlarmSet_SaveAlarmTime(T_U32 itemId, T_SYSTIME *pAlarmTime)
{
    AK_ASSERT_PTR(pAlarmTime, "AlarmSet_SaveAlarmTime(): pAlarmTime", AK_FALSE);

    switch (itemId)
    {
        case 1:
            gs.AlarmClock.DayAlarm = (ConvertSysTimeToSeconds(pAlarmTime)%ONE_DAY_SECOND);
            break;
        case 2:
            gs.AlarmClock.WeekAlarm = (ConvertSysTimeToSeconds(pAlarmTime)%ONE_DAY_SECOND);
            break;
        default:
            return AK_FALSE;
    }

    return AK_TRUE;
}

/**
 * @brief   save alarm type
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U8 optionId: alarm type
 * @return  T_VOID
 * @retval  
 */
T_VOID AlarmSet_SaveAlarmType(T_U32 optionId)
{
    switch (optionId)
    {
        case 1:
            gs.AlarmClock.DayAlarmType = 1;
            break;
        case 2:
            gs.AlarmClock.DayAlarmType = 2;
            break;
        default:
            gs.AlarmClock.DayAlarmType = 0;
            break;            
    }
}

/**
 * @brief   save week alarm setup
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U8 optionId: alarm type
 * @param   T_BOOL chooseState: setup state 
 * @return  T_VOID
 * @retval  
 */
T_VOID AlarmSet_SaveWeekAlarmSetup(T_U32 optionId, T_BOOL chooseState)
{
    gs.AlarmClock.WeekAlarmEnable[optionId] = chooseState;
}
#endif

