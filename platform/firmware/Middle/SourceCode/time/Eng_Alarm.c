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
#include "Fwl_public.h"
#include "Eng_Alarm.h"
#include "Fwl_osFs.h"
#include "Fwl_Initialize.h"
#include "Ctl_AudioPlayer.h"
#include "Fwl_rtc.h"
#include "eng_time.h"
#include "fwl_pfaudio.h"

static T_BOOL AlarmPlayingFlag = AK_FALSE;
//static T_BOOL AudioDAChanged = AK_FALSE;
static T_U8 AlarmPlayTimes = ALARM_PALY_TIMES;
static T_U32   alarmnexttime;

/*
 * @judge whether the alarm is playing
 * @auther guohui
 * @date 2006-12-18
 * @return  AK_TURE: playing
 * @        AK_FALSE:not playing
*/
T_BOOL GetAlarmPlayStatus(T_VOID)
{
    return AlarmPlayingFlag;
}

/*
 * get the alarm sound file name and file path
 * @auther guohui
 * @date 2006-12-18
 * @parm:
 *      tmpstr:to store the path and name of the sound file.
 * @return none
*/
T_pWSTR GetAlarmSound(T_VOID)
{
    return gs.AlarmClock.AlarmSoundPathName;
}


/*
 * play the alarm sound
 * @auther guohui
 * @date 2006-12-18
 * @parm:
 *      totalTime: to store the play time of the sound file.
 * @return none
*/
T_VOID GetAlarmSoundTotalTime(T_U32 *totalTime)
{
    T_U16   *tmpstr;

    tmpstr = GetAlarmSound();

    Fwl_AudioGetTotalTimeFromFile(tmpstr, totalTime);
    *totalTime /=1000;

}


/*
 * to check whether the sound file exist
 * @auther guohui
 * @date 2006-12-18
 * @return  AK_FALSE: not exist
 *          AK_TRUE : exist
*/
T_BOOL AlarmSoundExist(T_U16 **alarmPath)
{
    T_pFILE     fid;
 
    *alarmPath = GetAlarmSound();
    fid = Fwl_FileOpen(*alarmPath, _FMODE_READ, _FMODE_READ);
    if ( fid ==_FOPEN_FAIL )
    {
        return AK_FALSE;
    }
        
    Fwl_FileClose(fid);
    return AK_TRUE;
}


/*
 * play the alarm sound
 * @auther guohui
 * @date 2006-12-18
 * @return none
*/
T_VOID PlayAlarmSound(T_VOID)
{
    T_U16   	*tmpstr;
    T_U8        SoundType;
    //T_U32       bitRate, sampleRate;
    T_pCDATA    SoundData = AK_NULL;
    T_U32       audio_len;

    AlarmPlayingFlag = AK_TRUE;

    AK_DEBUG_OUTPUT("alarm play****\n");
    if( AlarmSoundExist(&tmpstr)  
    	&& Fwl_AudioGetMediaTypeFromFile(tmpstr, &SoundType)
        && Fwl_AudioOpenFile(tmpstr) )
    {
        //if the alarm sound set is valid, then play it.	
    }    
    else
    {
    	//if the alarm sound file does not exist, then play the default file.
    	gs.AlarmClock.AlarmSoundPathName[0] = 0;
    	
		SoundData = Res_GetBinResByID(AK_NULL, AK_TRUE, eRES_WAV_CLOCK_ALARM, &audio_len);
		
       	Fwl_AudioOpenBuffer(SoundData, audio_len);
    }
    
    Fwl_AudioSetVolume(Fwl_GetAudioVolume());
	MPlayer_SetEndCB((T_fEND_CB)StopPlayAlarmSound); 

	MPlayer_Play(0);
}


/*
 * stop playing the alarm sound
 * @auther guohui
 * @date 2006-12-18
 * @return none
*/
T_VOID StopPlayAlarmSound(T_VOID)
{
    AlarmPlayingFlag = AK_FALSE;
    
	MPlayer_Close();
}


/*
** process some flag and data to after alarmed
** @author guohui
** @date 2006-12-21
** @return void
*/
T_VOID AlarmPostProc(T_VOID)
{
    AlarmRtcChange();
}

/*
 * get the min valid alarm data
 * @auther guohui
 * @date 2006-12-18
 * @return T_U32 (seconds)
*/
T_U32 GetAlarmDataMinValid(T_BOOL bTime)
{
    T_U32 RealTime, MinTime, TmpTime = 0;
    T_U8 i, RealWeek;
    T_U32 RealYMD;

    RealTime = Fwl_RTCGetCount();
    RealWeek = GetWeekDay();
    RealYMD = RealTime - RealTime%ONE_DAY_SECOND;
    MinTime = RealTime + ONE_DAY_SECOND*10;

    if (gs.AlarmClock.DayAlarmType != DAY_ALARM_CLOSE)
    {
        TmpTime = RealYMD + gs.AlarmClock.DayAlarm;
        if (TmpTime > RealTime)
            MinTime = TmpTime;
        else
        {
            if (bTime && AlarmPlayTimes > 0 && DAY_ALARM_CONTINUE == gs.AlarmClock.DayAlarmType)
            {
                MinTime = alarmnexttime;
            }
            else
            {
                MinTime = TmpTime + ONE_DAY_SECOND;
            }
        }
    }

    if (gs.AlarmClock.WeekAlarmAllType != WEEK_ALARM_ALL_CLOSE)
    {
        for (i=RealWeek; i<RealWeek+7; i++)
        {
            if (gs.AlarmClock.WeekAlarmEnable[i%7] == AK_TRUE)
            {
                TmpTime = RealYMD + (i-RealWeek)*ONE_DAY_SECOND + gs.AlarmClock.WeekAlarm;
                if ((TmpTime > RealTime) && (TmpTime < MinTime))
                {
                    MinTime = TmpTime;
                    break;
                }
            }
        }
    }

    if (MinTime == (RealTime + ONE_DAY_SECOND*10))
        MinTime = 0;

    return MinTime;
}


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
T_POWER_ON_TYPE IsAlarmPowerOn(T_U32 RtcTime, T_U32 DayAlarm, T_U32 WeekAlarm)
{
    T_U8 Week;
    T_U32 RtcTimeHms, TimeDay = 0, TimeWeek = 0;
    T_BOOL DayAlarmValid = AK_FALSE;
    T_BOOL WeekAlarmValid = AK_FALSE;

    Week = GetWeekDay();
    RtcTimeHms = RtcTime % ONE_DAY_SECOND;
    if (gs.AlarmClock.DayAlarmType != DAY_ALARM_CLOSE)
    {
        TimeDay = DayAlarm;
        DayAlarmValid = AK_TRUE;
    }

    if (gs.AlarmClock.WeekAlarmAllType != WEEK_ALARM_ALL_CLOSE)
    {
        if (gs.AlarmClock.WeekAlarmEnable[Week])
        {
            TimeWeek = WeekAlarm;
            WeekAlarmValid = AK_TRUE;
        }
    }

    if (TimeDay <= TimeWeek)
    {
        if (DayAlarmValid)
        {
            if ((RtcTimeHms > TimeDay) && (RtcTimeHms < TimeDay + START_UP_TIME))
            {
                return DAY_ALARM_POWER_ON;
            }
            else if (RtcTimeHms < START_UP_TIME)
            {
                if((TimeDay > ONE_DAY_SECOND - START_UP_TIME) && (TimeDay < ONE_DAY_SECOND))
                {
                    return DAY_ALARM_POWER_ON;
                }
            }
        }

        if (WeekAlarmValid)
        {
            if ((RtcTimeHms > TimeWeek) && (RtcTimeHms < TimeWeek + START_UP_TIME))
            {
                return WEEK_ALARM_POWER_ON;
            }
            else if (RtcTimeHms < START_UP_TIME)
            {
                if ((TimeWeek > ONE_DAY_SECOND - START_UP_TIME) && (TimeWeek < ONE_DAY_SECOND))
                {
                    return WEEK_ALARM_POWER_ON;
                }
            }

        }
    }
    else
    {
        if (WeekAlarmValid)
        {
            if ((RtcTimeHms > TimeWeek) && (RtcTimeHms < TimeWeek + START_UP_TIME))
            {
                return WEEK_ALARM_POWER_ON;
            }
            else if (RtcTimeHms < START_UP_TIME)
            {
                if ((TimeWeek > ONE_DAY_SECOND - START_UP_TIME) && (TimeWeek < ONE_DAY_SECOND))
                {
                    return WEEK_ALARM_POWER_ON;
                }
            }

        }

        if (DayAlarmValid)
        {
            if ((RtcTimeHms > TimeDay) && (RtcTimeHms < TimeDay + START_UP_TIME))
            {
                return DAY_ALARM_POWER_ON;
            }
            else if (RtcTimeHms < START_UP_TIME)
            {
                if((TimeDay > ONE_DAY_SECOND - START_UP_TIME) && (TimeDay < ONE_DAY_SECOND))
                {
                    return DAY_ALARM_POWER_ON;
                }
            }
        }
    }

    return MANUAL_POWER_ON;
}


/*
** select the valid data to write into alarm rtc.
** @author guohui
** @date 2006-12-21
** @return void
*/
T_VOID AlarmRtcChange(T_VOID)
{
    T_U32 alarm_time;

    AlarmPlayTimes = ALARM_PALY_TIMES;

    alarm_time = GetAlarmDataMinValid(AK_FALSE);
    AK_DEBUG_OUTPUT("alarm_time:%d\n", alarm_time);

    Fwl_SetAlarmRtcCount(alarm_time);
    gb.RtcType = RTC_ALARM;

    if ((Fwl_GetAlmRTCCount() % ONE_DAY_SECOND) == gs.AlarmClock.DayAlarm)
        gs.LatestIsDayAlarm = AK_TRUE;
    else
        gs.LatestIsDayAlarm = AK_FALSE;
}


/*
** calculate the weekday accord the date.
** @author guohui
** @date 2006-12-21
** @return  T_U8
** @        0:sunday 1:monday ... 6:saturday
**
*/
T_U8 GetWeekDay(T_VOID)
{
    T_U8 week;
    T_SYSTIME TimeTmp = GetSysTime();

    T_U16   year    =   TimeTmp.year;
    T_U8    month   =   TimeTmp.month;
    T_U8    day     =   TimeTmp.day;
    if(month==1)
    {
        month = 13;
        year--;
    }
    else if(month==2)
    {
        month = 14;
        year--;
    }

    week = ( day+2*month+3*(month+1)/5+year+year/4-year/100+year/400+1 )%7;
    return week;

}


/*
** Alarm delay process function
**
*/
T_VOID AlarmDelayProc(T_U32 delay_second)
{
    T_U32   rtc_time, alarm_time, curtime;

    AlarmPlayTimes--;
    if(!AlarmPlayTimes)
        return;

    rtc_time   = Fwl_RTCGetCount();
    alarm_time = GetAlarmDataMinValid(AK_FALSE);
    if(rtc_time + delay_second < alarm_time)
    {
        curtime = rtc_time + delay_second;
    }
    else
    {
        if(alarm_time && (alarm_time >= rtc_time))
        {
            curtime = alarm_time;
            AlarmPlayTimes = ALARM_PALY_TIMES;

            if (gs.AlarmClock.DayAlarm == curtime)
                gs.LatestIsDayAlarm = AK_TRUE;
            else
                gs.LatestIsDayAlarm = AK_FALSE;
        }
        else
           curtime = rtc_time + delay_second;
    }
    
    alarmnexttime = curtime;
    Fwl_SetAlarmRtcCount(curtime);
}


/*      end of the file     */
