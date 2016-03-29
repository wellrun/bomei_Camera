
/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  * @File name: Ctl_AlarmSet.c
  * @Function:  This file is a part of the state machine s_set_alarm
  * @Author:    WangWei
  * @Date:      2008-05-04
  * @Version:   1.0
  */

#ifndef __CTL_ALARM_SET_H__
#define __CTL_ALARM_SET_H__

#include "ctl_global.h"
#include "Ctl_ScrollBar.h"
#include "Ctl_Title.h"
#include "Ctl_MultiSet.h"
#include "eng_time.h"
#include "Ctl_IconExplorer.h"
#include "Eng_GblString.h"


#ifdef __cplusplus
extern "C" {
#endif

#define ITEMID_DAYALARMTYPE     0       /* day alarm type */
#define ITEMID_DAYALARMTIME     1       /* day alarm time */
#define ITEMID_WEEKALARMTIME    2       /* week alarm time */
#define ITEMID_WEEKALARMSET     3       /* week alarm set */
#define ITEMID_ALARMSOUND       4       /* alarm sound */

typedef enum
{
    CURSOR_HOUR = 0,
    CURSOR_MINUTE,            
    CURSOR_SECOND            
}T_ALARM_CURSOR;

typedef struct 
{
    T_SYSTIME       dayAlarmTime;   /* day alarm time */
    T_SYSTIME       weekAlarmTime;  /* week alarm time */
    T_MULTISET      multiSet;       /* MultiSet control struct */
    T_ALARM_CURSOR  alarmCursor;    /* alarm edit cursor */    
	T_USTR_FILE     str_dayAlarm;   /* unicode string of day alarm time */
	T_USTR_FILE     str_weekAlarm;  /* unicode string of week alarm time */
    T_USTR_FILE     alarmSoundName; /* unicode string of alarm sound file name*/
    T_USTR_FILE     weekAlarmState; /* unicode string of week alarm set state */
}T_ALARMSET;

/**
 * @brief   alarm setup init
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARM_PARM *pAlarmSet:T_ALARMSET struct pointer 
 * @return  T_BOOL
 * @retval  AK_TRUE  init success
 * @retval  AK_FALSE init fail 
 */
T_BOOL  AlarmSet_Init(T_ALARMSET *pAlarmSet);

/**
 * @brief   free alarm setup resource
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARM_PARM *pAlarmSet:T_ALARMSET struct pointer 
 * @return  T_VOID
 */
T_VOID  AlarmSet_Free(T_ALARMSET *pAlarmSet);

/**
 * @brief   get alarm setup resource
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARM_PARM *pAlarmSet:T_ALARMSET struct pointer 
 * @return  T_BOOL
 * @retval  AK_TRUE  init success
 * @retval  AK_FALSE init fail 
 */
T_BOOL  AlarmSet_GetContent (T_ALARMSET *pAlarmSet);

/**
 * @brief   alarm setup show function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARMSET *pAlarmSet: T_ALARMSET struct pointer
 * @return  T_VOID
 */
T_VOID  AlarmSet_Show(T_ALARMSET *pAlarmSet);

/**
 * @brief   synchronize alarm sound file name
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARMSET *pAlarmSet: T_ALARMSET struct pointer
 * @return  T_VOID
 */
T_VOID AlarmSet_SyncAlarmSoundFileName(T_ALARMSET *pAlarmSet);

/**
 * @brief   save alarm type
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U32 optionId: alarm type
 * @return  T_VOID
 * @retval  
 */
T_VOID AlarmSet_SaveAlarmType(T_U32 optionId);

/**
 * @brief   check week alarm setup state
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U16 *pWeekAlarmState:week alarm setup item attribute string pointer
 * @return  T_VOID
 */
T_VOID AlarmSet_CheckWeekAlarmSetupState(T_U16 *pWeekAlarmState);

/**
 * @brief   save week alarm setup
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U32 optionId: alarm type
 * @param   T_BOOL chooseState: setup state 
 * @return  T_VOID
 * @retval  
 */
T_VOID AlarmSet_SaveWeekAlarmSetup(T_U32 optionId, T_BOOL chooseState);

/**
 * @brief   movie cursor to right
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARMSET *pAlarmSet: T_ALARMSET struct pointer
 * @return  T_VOID
 */
T_VOID AlarmSet_MoveRightdayAlarmCursor(T_ALARMSET *pAlarmSet);

/**
 * @brief   get alarm time
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARMSET *pAlarmSet : T_ALARMSET struct pointer
 * @return  T_VOID
 */
T_VOID AlarmSet_GetAlarmTime(T_ALARMSET *pAlarmSet);

/**
 * @brief   movie cursor to left
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_ALARMSET *pAlarmSet: T_ALARMSET struct pointer
 * @return  T_VOID
 */
T_VOID AlarmSet_MoveLeftdayAlarmCursor(T_ALARMSET *pAlarmSet);

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
T_BOOL AlarmSet_DecreaseAlarmCursor(T_SYSTIME *pAlarmTime, T_ALARM_CURSOR alarmCursor);

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
T_BOOL AlarmSet_IncreamAlarmTimeCursor(T_SYSTIME *pAlarmTime, T_ALARM_CURSOR alarmCursor);

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
T_BOOL AlarmSet_SaveAlarmTime(T_U32 itemId, T_SYSTIME *pAlarmTime);

#ifdef __cplusplus
}
#endif

#endif

