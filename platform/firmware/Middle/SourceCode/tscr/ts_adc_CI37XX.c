/**
 * @FILENAME: ts_adc_CI7801.c
 * @BRIEF ts_adc_CI7801 file
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2008-01-15
 * @VERSION 1.0
 * @REF
 */
#ifdef CI37XX_PLATFORM


#include "anyka_types.h"
#include "Gbl_Global.h"
#include "arch_analog.h"
#include "hal_ts.h"
#include "arch_init.h"
    // AVDD是参考电压,应该是3300，3316是补偿
#define AVDD                             3316
#define TS_DEVICE_ID                     0x0656
#define FONT_END_COUNT           (10/TS_TIMER_INTERVAL)  //10 mS  
#define WORD_END_COUNT           (300/TS_TIMER_INTERVAL) //250 mS
#define PEN_DOWN_COUNT           (20/TS_TIMER_INTERVAL-2)  //wait for pen down level off
//ts adc working sample rate
#define TS_ADC_SAMPLE_RATE      (1000)    //1K

#define TS_DIFF_MIN            (2)
#ifdef USE_LCD_RGB_OTA5180A //目前只调试到这个屏需要这个值。
#define TS_THRESHOLD           (16)	 //这个参数影响触摸屏MOVE灵敏度问题
#define TS_TIMER_INTERVAL      (1)   //这个参数影响到飘点，越大越容易飘
#else
#define TS_THRESHOLD           (100)
#define TS_TIMER_INTERVAL      (4)   //timer interval, 4 ms
#endif

T_TS_PARAM tsr_list[TS_MAX_SUPPORT] = 
{
	0xff,                 //PenDownCnt
 	TS_ADC_SAMPLE_RATE,   //TsAdcSampeRate
 	FONT_END_COUNT,       //FontEndCount(unit:TS_TIMER)     
 	WORD_END_COUNT ,      //WordEndCount
 	PEN_DOWN_COUNT,   	  //PenEndCount
 	TS_DEVICE_ID,     // Ts_tpye_ID
 	TS_TIMER_INTERVAL,  //TS_TIMER_INTERVAL(unit:ms)  
 	TS_THRESHOLD,      //it will not return data if move point X or Y bigger than this value
 	TS_DIFF_MIN,        //it will not return data if move point X and Y smaller than this value
 	CTL_TYPE_INSIDE,   //ts conter type

	AK_NULL,
	AK_NULL,
	AK_NULL,
	AK_NULL,
	AK_NULL
};

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

T_VOID tsCalculateCoordinate_point(T_U32 *x, T_U32 *y, T_POINT tsPoint)
{
	T_TSPOINT tsPoint_tmp;

	tsPoint_tmp.x = tsPoint.x;
	tsPoint_tmp.y = tsPoint.y;

	tsCalculateCoordinate(x,y,tsPoint_tmp);
}

T_U32 battary_get_voltage_value(T_VOID)
{
    T_U32 ad_value =0;
    T_U32 voltage =0;
	
#ifdef OS_ANYKA
    ad_value = analog_getvalue_bat();
#endif
    // *2是因为2分频了
    // AVDD是参考电压,应该是3300，3316是补偿
    // ad_value * 2 * AVDD / 1024公式
    // 由于数码加了电阻，需要乘以3
    voltage = (ad_value * 3 * AVDD / 1024);

    return voltage;
}
static int ts_reg(void)
{

    ts_reg_dev(TS_DEVICE_ID, tsr_list);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif

module_init(ts_reg)

#ifdef __CC_ARM
#pragma arm section
#endif

#endif  //#ifdef CI7801_PLATFORM
