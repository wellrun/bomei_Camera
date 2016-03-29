
#include "akdefine.h"

#ifdef USE_EXTERN_RTC

#include "gpio_config.h"

#include "rtc.h"
#include "drv_in_callback.h"



//#include "anyka_types.h"

#define GPIO_EXTERN_RTC 26  //24  //25

#define  POWER_CHECK_FLAG       11

extern int rtc_read_systime(T_SYSTIME *pt_sys_time);
extern int rtc_write_systime(T_SYSTIME *pt_sys_time);
extern int rtc_clear_alarm(T_U8 alarm_num);
extern int rtc_set_alarm(T_U8 alarm_num,T_SYSTIME *sys_time);
extern int rtc_get_alarm(T_U8 alarm_num,T_SYSTIME *sys_time);
extern int rtc_reset(T_U32 pin_scl, T_U32 pin_sda);
extern T_U8 rtc_CheckPower(void);
extern int rtc_SetPowerFlag(T_U8 num);


/**
 * @brief   get rtc passed count in seconds
 *
 * @author
 * @date
 * @return T_U32
 * @retval the rtc count
 */
T_U32 external_rtc_get_RTCcount(T_VOID)
{
    T_SYSTIME   stctSysTime; 
    T_U32       nSecond;


    if(0 != rtc_read_systime(&stctSysTime))
    {
    	if (AK_NULL != gb_drv_cb_fun.akprintf)
    	{
        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "failed to read sys time");
    	}
    }

#ifdef RTC_DEBUG
	if (AK_NULL != gb_drv_cb_fun.akprintf)
	{
    	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "external_rtc_get_RTCcount year =%d,month=%d,day=%d,hour=%d,minute=%d,second = %d,week=%d",
    stctSysTime.year,stctSysTime.month,stctSysTime.day,stctSysTime.hour,stctSysTime.minute,stctSysTime.second,stctSysTime.week);
	}
#endif 

    nSecond =  ConvertSysTimeToSeconds(&stctSysTime);    



    return nSecond;
}



/**
 * @brief   set rtc start count value in seconds
 *
 * @author
 * @date
 * @param[in]   rtc_value the rtc count to be set
 * @return      T_VOID
 */
T_VOID external_rtc_set_RTCcount(T_U32 rtc_value)
{
    T_SYSTIME   stctSysTime;   
#ifdef RTC_DEBUG
    T_SYSTIME      *systime = &stctSysTime;
#endif

#ifdef RTC_DEBUG
	if (AK_NULL != gb_drv_cb_fun.akprintf)
	{
    	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "external_rtc_set_RTCcount rtc_value=%d",rtc_value);
	}
#endif

    if(AK_FALSE == ConvertSecondsToSysTime(rtc_value, &stctSysTime))
    {
    	if (AK_NULL != gb_drv_cb_fun.akprintf)
    	{
        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "failed to convert second to systime");
    	}
    }

#ifdef RTC_DEBUG
	if (AK_NULL != gb_drv_cb_fun.akprintf)
	{
		gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "external_rtc_set_RTCcount,year =%d,month=%d,day=%d,hour=%d,minute=%d,second=%d,week=%d", \
        systime->year, systime->month,systime->day, systime->hour,systime->minute,systime->second,systime->week);
	}

#endif 

    
    if(0 != rtc_write_systime(&stctSysTime))
    {
    	if (AK_NULL != gb_drv_cb_fun.akprintf)
    	{
        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "failed to set rtc sys time");
    	}
    }




}


/**
 * @brief  clear the alarm set
 *
 * when the rtc count reaches to the alarm  count, 
 * AK chip is woken up if in standby mode and rtc interrupt happens.
 * @author
 * @date
 * @return T_VOID
 */
T_BOOL  external_rtc_clear_alarm(T_VOID)
{
    rtc_clear_alarm(0);
    rtc_clear_alarm(1);
	
	return AK_TRUE;
}

/**
 * @brief   set rtc alarm count.
 *
 * when the rtc count reaches to the alarm  count, 
 * AK chip is woken up if in standby mode and rtc interrupt happens.
 * @author
 * @date
 * @param[in] rtc_wakeup_value alarm count in seconds
 * @return T_VOID
 */
T_VOID  external_rtc_set_alarmcount(T_U32 rtc_wakeup_value)
{
    T_SYSTIME   stctSysTime;  
	
#ifdef RTC_DEBUG
    T_SYSTIME   DataSysTime;
#endif

//    printf("external_rtc_set_alarmcount rtc_wakeup_value =%d",rtc_wakeup_value);

    if(AK_FALSE == ConvertSecondsToSysTime(rtc_wakeup_value, &stctSysTime))
    {
    	if (AK_NULL != gb_drv_cb_fun.akprintf)
    	{
        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "failed to convert second to systime");
    	}
    }

#ifdef RTC_DEBUG
            DataSysTime = stctSysTime;

			if (AK_NULL != gb_drv_cb_fun.akprintf)
	    	{
	        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "external_rtc_set_alarmcount1 year =%d,month=%d,day=%d,hour=%d,minute=%d,second = %d,week=%d",
            DataSysTime.year,DataSysTime.month,DataSysTime.day,DataSysTime.hour,DataSysTime.minute,DataSysTime.second,DataSysTime.week);
	    	}
#endif 

    
    if(0 != rtc_set_alarm(0, &stctSysTime))
    {
    	if (AK_NULL != gb_drv_cb_fun.akprintf)
    	{
        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "failed to set rtc alarm");
    	}
    }

#ifdef RTC_DEBUG

        if(0 != rtc_get_alarm(0,&DataSysTime))
        {
        	if (AK_NULL != gb_drv_cb_fun.akprintf)
	    	{
	        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "failed to get alarm time");
	    	}
        }

		if (AK_NULL != gb_drv_cb_fun.akprintf)
    	{
        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "external_rtc_set_alarmcount2 year =%d,month=%d,day=%d,hour=%d,minute=%d,second = %d,week=%d",
        DataSysTime.year,DataSysTime.month,DataSysTime.day,DataSysTime.hour,DataSysTime.minute,DataSysTime.second,DataSysTime.week);
    	}
#endif 

}

/**
 * @brief get alarm count that has been set.
 * @author
 * @date
 * @return T_U32
 * @retval the alarm count in seconds
 */
T_U32   external_rtc_get_alarmcount(T_VOID)
{
    T_SYSTIME   alarmSysTime;
    T_SYSTIME   DataSysTime;
    T_S32       nSecond ;
    T_U32       nCount;


    if(0 != rtc_get_alarm(0,&alarmSysTime))
    {
    	if (AK_NULL != gb_drv_cb_fun.akprintf)
    	{
        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "failed to get alarm time");
    	}
    }

//    printf("external_rtc_get_alarmcount week=%d,hour =%d,minute=%d",alarmSysTime.week,alarmSysTime.hour,alarmSysTime.minute);
    
    if(0 != rtc_read_systime(&DataSysTime))
    {
    	if (AK_NULL != gb_drv_cb_fun.akprintf)
    	{
        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "failed to get sys time");
    	}
    }

#ifdef RTC_DEBUG
		if (AK_NULL != gb_drv_cb_fun.akprintf)
    	{
        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "external_rtc_get_alarmcount2 year =%d,month=%d,day=%d,hour=%d,minute=%d,second = %d,week=%d",
        DataSysTime.year,DataSysTime.month,DataSysTime.day,DataSysTime.hour,DataSysTime.minute,DataSysTime.second,DataSysTime.week);
    	}
#endif 

    nCount  = ConvertSysTimeToSeconds(&DataSysTime);

    nSecond = (((T_S32)alarmSysTime.week - (T_S32)DataSysTime.week  + 7) % 7 ) * 24 * 3600 + ((T_S32)alarmSysTime.hour - (T_S32)DataSysTime.hour) * 3600 + ((T_S32)alarmSysTime.minute - (T_S32)DataSysTime.minute) * 60;        

    if(nSecond < 0)
    {
        nSecond = nSecond * (-1);
        return (nCount - (T_U32)(nSecond));        
    }
    else
    {
        return (nCount + (T_U32)(nSecond));        
    }
    

}

/**
 * @brief   init rtc module
 *
 * @author
 * @date
 * @return  T_VOID
 */
T_VOID 	external_rtc_init(T_VOID)
{
    rtc_reset(GPIO_I2C_SCL,GPIO_I2C_SDA);  
}


/*
    check whether the power is shut down before start the sys
    AK_TRUE     :  exception occur, system is shut down abnormaly
*/
T_BOOL  external_rtc_CheckPower(T_VOID)
{
    T_U8    uzTmp = 0;
    uzTmp = rtc_CheckPower();
    
   if(POWER_CHECK_FLAG ==  uzTmp)
   {
        return AK_FALSE;
   }
   else
   {
   		if (AK_NULL != gb_drv_cb_fun.akprintf)
    	{
        	gb_drv_cb_fun.akprintf(C1, M_DRVSYS, "external_rtc_CheckPower uzTmp=%d ",uzTmp);
    	}

        return AK_TRUE;
   }
}


T_BOOL external_rtc_SetPowrFlag(T_VOID)
{
   if(0 ==  rtc_SetPowerFlag( POWER_CHECK_FLAG))
   {
        return AK_TRUE;
   }
   else
   {
        return AK_FALSE;
   }    

}
T_VOID 	external_rtc_set_callback(T_fRTC_CALLBACK cb)
{
    gpio_set_pin_as_gpio(GPIO_EXTERN_RTC);
    gpio_set_pin_dir(GPIO_EXTERN_RTC, 0);
    gpio_set_pull_up_r(GPIO_EXTERN_RTC, AK_TRUE);   

    gb_drv_cb_fun.AK_Sleep(100);
    
    //the followed detector interface is overdue,pls use the new interface.
	gb_drv_cb_fun.detector_register_tmp(GPIO_EXTERN_RTC, eDETECT_LOW, cb);
//    detector_set_jitter(GPIO_EXTERN_RTC, 0);
    gb_drv_cb_fun.detector_enable_tmp(GPIO_EXTERN_RTC);

}


/*
 * @brief: check wheather external rtc use pin to wake up sys.
 * @param in: the number of GPIO
 * @retval: AK_TRUE: the rtc use pin to wake up sys.
 * @retval: AK_FALSE: the rtc use another pin or don't use gpio to wake up sys. 
 */
T_BOOL  external_rtc_gpio_check(T_U8 pin)
{
    return pin == GPIO_EXTERN_RTC ? AK_TRUE : AK_FALSE;
}

#endif
