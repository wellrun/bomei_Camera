/**
 * @FILENAME: ts_adc_CI3771.c
 * @BRIEF ts_adc_CI3771 file
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2008-01-15
 * @VERSION 1.0
 * @REF
 */
#ifdef CI37XX_PLATFORM

#include "akdefine.h"
#include "ts_adc_list.h"
#include "Gbl_Global.h"



    // AVDD是参考电压,应该是3300，3316是补偿
#define AVDD                             3316

/**
 * @BRIEF select touch screen typ
 * @AUTHOR guoshaofeng
 * @DATE 2008-07-01
 * @RETURN E_TS_TYPE, touch screen type, resistant or capacitive
 * @RETVAL
 */
E_TS_TYPE ts_select_type(T_VOID)
{
    return E_TS_TYPE_RES;
}

/**
 * @BRIEF Calculate coordinate
 * @DATE 2008-01-15
 * @PARAM T_U32 *x, T_U32 *y: x,y coordinate pointer,it is output
 * @PARAM T_TSPOINT tsPoint: x,y coordinate struct,it is input
 * @RETURN T_VOID
 * @RETVAL
 * @DISCRIPTION: the touch screen's x coordinate form left to right  may be positive or negtive
        and so is y coordinate ,
        we diffine the 4 mode in 2 bit **,hight bit represent  x coordinate ,and low bit represent y.
        and for the x coordinate bit,  0   present  the coordinate is postive from left to right, else 1        
        and for the y coordinate bit,  0   present  the coordinate is postive from top to botton, else 1                
            
 */
T_VOID tsCalculateCoordinate(T_U32 *x, T_U32 *y, T_TSPOINT tsPoint)
{         
#ifdef TOUCH_SCR
    T_U32   nXmode = 0;
    T_U32   nYmode = 0;

    nXmode = 0x0c & gs.nADCCoordMode;
    nYmode = 0x03 & gs.nADCCoordMode;

    //  x coordinate fomula
    if(0x0 == nXmode){ // x coordinate is positive from left to right 
        *x = (T_U32)(tsPoint.x);       
    }
    else if(0x04 == nXmode){
        *x = (T_U32)(0x000003ff -  tsPoint.x);
    }
    else if(0x08 == nXmode){
        *x = (T_U32)(tsPoint.y);
    }
    else if(0x0c == nXmode){
        *x = (T_U32)(0x000003ff -  tsPoint.y);
    }

    // y coordinate fomula    
    if(0x0 == nYmode){
        *y = (T_U32)(tsPoint.y);        
    }
    else if(0x01 == nYmode){
        *y = (T_U32)(0x000003ff - tsPoint.y);
    }
    else if(0x02 == nYmode){
        *y = (T_U32)(tsPoint.x);
    }
    else if(0x03 == nYmode){
        *y = (T_U32)(0x000003ff - tsPoint.x);
    }
#endif
}

#define BATTERY_VALUE_AVDD              3300
#define BATTERY_VOL_FORMULA(vol_ad)     ((vol_ad * 2 * BATTERY_VALUE_AVDD) >> 10)
#define BATTERY_CHECK_COUNT             4

T_U32 battery_get_voltage_value(T_VOID)
{
    T_U32 ad_value;
    T_U32 voltage;
	T_U32 uCount;
	
    for(ad_value=0,uCount=0; uCount<BATTERY_CHECK_COUNT; ++uCount)
    {
	    ad_value += analog_getvalue_bat();
    }
	
	ad_value >>= 2;       // ad_value / 4
		
    voltage = BATTERY_VOL_FORMULA(ad_value);
    return voltage;
}

#endif  //#ifdef CI7801_PLATFORM