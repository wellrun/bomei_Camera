/*****************************************************************
**
** Copyrights (C) 2006, ANYKA Software Inc.
*  All rights reserced.
**
** File description:Used for alarm sound play control
**
** file name:   Eng_Alarm.c
** Revision:    1.00
** Author:      guohui
** Date:        2006-12-23
******************************************************************/

#ifndef __ENG_ALARM_H__
#define __ENG_ALARM_H__

#include "anyka_types.h"
#include "log_mediaplayer.h"

#define ALARM_PALY_TIMES            5   //5times when not close it

#define ONE_DAY_SECOND              86400
#define START_UP_TIME               15  //s
#define POWER_OFF_ALARM_DELAY       15  //s when power off, we should delay the alarm

#define ALARM_RING_TIME			    45      //15s
#define ALARM_INTERVAL_TIME		    180     //165s


enum {
    SUNDAY = 0,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY
};

enum {
    DAY_ALARM_CLOSE = 0,
    DAY_ALARM_ONCE,
    DAY_ALARM_CONTINUE
};

enum {
    WEEK_ALARM_ALL_CLOSE = 0,
    WEEK_ALARM_ALL_OPEN,
    WEEK_ALARM_ALL_USER
};

enum {
    ALARM_DAY_EDIT_MODE = 1,
    ALARM_WEEK_EDIT_MODE
};

typedef enum {
    MANUAL_POWER_ON =0,
    DAY_ALARM_POWER_ON,
    WEEK_ALARM_POWER_ON
}T_POWER_ON_TYPE;

typedef struct {
    T_U32               DayAlarm;           //the day alarm data
    T_U32               WeekAlarm;          //the week alarm data
    T_U8                WeekAlarmAllType;   //0:close  1:open  2:user
    T_U8                DayAlarmType;       //0:close  1:once  2:continue
    T_BOOL              WeekAlarmEnable[7]; //WeekAlarmEnable[0]:sunday  ... WeekAlarmEnable[6]:saturday
                                            //WeekAlarmEnable[i]>0:open     WeekAlarmEnable[i]=0:close
    T_U16               AlarmSoundPathName[512];
}T_ALARM_TYPE;

/*
 * @judge whether the alarm is playing
 * @auther guohui
 * @date 2006-12-18
 * @return  AK_TURE: playing
 * @        AK_FALSE:not playing
*/
T_BOOL GetAlarmPlayStatus(T_VOID);


/*
 * get the alarm sound file name and file path
 * @auther guohui
 * @date 2006-12-18
 * @parm:
 *      tmpstr:to store the path and name of the sound file.
 * @return none
*/
T_pWSTR GetAlarmSound(T_VOID);

/*
 * play the alarm sound
 * @auther guohui
 * @date 2006-12-18
 * @parm:
 *      totalTime: to store the play time of the sound file.
 * @return none
*/
T_VOID GetAlarmSoundTotalTime(T_U32 *totalTime);

/*
 * play the alarm sound
 * @auther guohui
 * @date 2006-12-18
 * @return none
*/
T_VOID PlayAlarmSound(T_VOID);

/*
 * stop playing the alarm sound
 * @auther guohui
 * @date 2006-12-18
 * @return none
*/
T_VOID StopPlayAlarmSound(T_VOID);


/*
** process some flag and data to after alarmed
** @author guohui
** @date 2006-12-21
** @return void
*/
T_VOID AlarmPostProc(T_VOID);

/*
 * get the min valid alarm data
 * @auther guohui
 * @date 2006-12-18
 * @return T_U32 (seconds)
*/
T_U32 GetAlarmDataMinValid(T_BOOL bTime);


/**
*  @to judge the start up is by manual or alarm
*  @because the register RTC_ALARM_MASK_REG will be clear after auto power on
*  @so we judge by reading the data from saved alarm data.
*  @param: RtcTime: the current rtc time
*          DayAlarm: day alarm data
*          WeekAlarm:week alarm data
*  @return: 0: is manual power on
*           1: is day alarm auto power on
*           2: is week alarm auto power on
*/
T_POWER_ON_TYPE IsAlarmPowerOn(T_U32 RtcTime, T_U32 DayAlarm, T_U32 WeekAlarm);


/*
** select the valid data to write into alarm rtc.
** @author guohui
** @date 2006-12-21
** @return void
*/
T_VOID AlarmRtcChange(T_VOID);

/*
** calculate the weekday accord the date.
** @author guohui
** @date 2006-12-21
** @return  T_U8
** @        0:sunday 1:monday ... 6:saturday
**
*/
T_U8 GetWeekDay(T_VOID);


/*
** Alarm delay process function
**
*/
T_VOID AlarmDelayProc(T_U32 delay_second);

#endif
