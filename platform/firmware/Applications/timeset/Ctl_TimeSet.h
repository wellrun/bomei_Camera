
/**
  * @Copyrights (C) 2008, ANYKA software Inc
  * @All rights reserved.
  * @File name: Ctl_TimeSet.h
  * @Function:  This file is a part of the state machine s_set_sysclock
  * @Author:    WangWei
  * @Date:      2008-05-04
  * @Version:   1.0
  */

#ifndef __CTL_SYSCLOCK_SET_H__
#define __CTL_SYSCLOCK_SET_H__

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

#define ITEMID_TIME             0   /* set time*/
#define ITEMID_DAY              1   /* set date*/
#define ITEMID_DAYFORMAT        2   /* set date format */
#define ITEMID_DAYSEPARATOR     3   /* set date separator*/
#define ITEMID_TIMEFORMAT       4   /* set time format*/
#define ITEMID_TIMESEPARATOR    5   /* set time separator*/

typedef enum
{
    CURSOR_FIRST = 0,           		
    CURSOR_SECND,            
    CURSOR_THIRD            
}T_CURSOR_DAY;

typedef enum
{
    CURSOR_HOUR = 0,
    CURSOR_MINUTE,            
    CURSOR_SECOND            
}T_CURSOR_TIME;

typedef struct 
{
    T_SYSTIME       sysTime;    /* system time: 非编辑状态下自动更新，编辑状态下可手动修改*/
    T_MULTISET      multiSet;   /* MultiSet struct*/
    T_CURSOR_DAY    dayCursor;  /* date edit cursor*/
    T_CURSOR_TIME   timeCursor; /* time edit cursor*/
	T_USTR_FILE     timeBuf;    /* unicode string*/
	T_USTR_FILE     dayBuf;
}T_TIMESET;

/**
 * @brief   system clock setup init
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  init success
 * @retval  AK_FALSE init fail 
 */
T_BOOL  TimeSet_Init(T_TIMESET *pTimeSet);

/**
 * @brief   free system clock setup resource
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_VOID
 */
T_VOID  TimeSet_Free(T_TIMESET *pTimeSet);

/**
 * @brief   get resource
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  init success
 * @retval  AK_FALSE init fail 
 */
T_BOOL  TimeSet_GetContent (T_TIMESET *pTimeSet);

/**
 * @brief   show function
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_VOID
 * @retval  
 */
T_VOID  TimeSet_Show(T_TIMESET *pTimeSet);

/**
 * @brief   set refresh flag
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @param   T_MULTISET_REFRESH_FLAG refreshFlag: refresh flag
 * @return  T_VOID
 */
T_VOID  TimeSet_SetRefresh(T_TIMESET *pTimeSet, T_MULTISET_REFRESH_FLAG refreshFlag);

/**
 * @brief   synchronize system clock
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_VOID
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_SyncSystemClock(T_TIMESET *pTimeSet);

/**
 * @brief   movie cursor to left
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_MoveLeftTimeCursor(T_TIMESET *pTimeSet);

/**
 * @brief   movie cursor to right
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_MoveRightTimeCursor(T_TIMESET *pTimeSet);

/**
 * @brief   incream the cursor num
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_IncreamTimeCursor(T_TIMESET *pTimeSet);

/**
 * @brief   decrease the cursor num
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_DecreaseTimeCursor(T_TIMESET *pTimeSet);

/**
 * @brief   save edit time
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_SaveEditTimeCursor(T_TIMESET *pTimeSet);

/**
 * @brief   movie cursor to left
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_EditDate_MoveLeftCursor(T_TIMESET *pTimeSet);

/**
 * @brief   movie cursor to right
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_EditDate_MoveRightCursor(T_TIMESET *pTimeSet);

/**
 * @brief   incream cursor date
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_EditDay_IncreamCursor(T_TIMESET *pTimeSet);

/**
 * @brief   decrease cursor date
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_EditDay_DecreaseCursor(T_TIMESET *pTimeSet);

/**
 * @brief   save edit date
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_TIMESET *pTimeSet: T_TIMESET struct pointer
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_BOOL TimeSet_SaveEditDateCursor(T_TIMESET *pTimeSet);

/**
 * @brief   save date format
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U8 optionFocusId: date format
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_VOID TimeSet_SaveDayFormat(T_U32 optionFocusId);

/**
 * @brief   save date separator
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U8 optionFocusId: date separator
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_VOID TimeSet_SaveDaySeparator(T_U32 optionFocusId);

/**
 * @brief   save time format
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U8 optionFocusId: time format
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_VOID TimeSet_SaveTimeFormat(T_U32 optionFocusId);

/**
 * @brief   save time separator
 * @author  WangWei
 * @date    2008-05-04 
 * @param   T_U8 optionFocusId: time separator
 * @return  T_BOOL
 * @retval  AK_TRUE  success
 * @retval  AK_FALSE fail 
 */
T_VOID TimeSet_SaveTimeSeparator(T_U32 optionFocusId);
#ifdef __cplusplus
}
#endif

#endif

