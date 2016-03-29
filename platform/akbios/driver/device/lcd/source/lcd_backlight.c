/**
 * @FILENAME: lcd_backlight.c
 * @BRIEF LCD backlight driver file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @VERSION 1.0
 * @REF
 */

#include "akdefine.h"
#include "lcd.h"
#include "gpio.h"
#include "drv_api.h"


static T_VOID lcdbl_aat3140_set_brightness(T_eLCD lcd, T_U8 brightness);
static T_VOID lcdbl_aat3155_set_brightness(T_eLCD lcd, T_U8 brightness);
static T_VOID lcdbl_lm2704_set_brightness(T_eLCD lcd, T_U8 brightness);
static T_VOID lcdbl_tps61061_set_brightness(T_eLCD lcd, T_U8 brightness); 
static T_VOID lcdbl_fan5607_set_brightness(T_eLCD lcd, T_U8 brightness);
static T_VOID lcdbl_1763_set_brightness(T_eLCD lcd, T_U8 brightness);
static T_VOID lcdbl_A30BL_set_brightness(T_eLCD lcd, T_U8 brightness);
static T_VOID lcdbl_gpio_set_brightness(T_eLCD lcd, T_U8 brightness);
static T_VOID lcdbl_pt4401_set_brightness(T_eLCD lcd, T_U8 brightness);


/**
 * @BRIEF Set brightness value
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @PARAM T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @PARAM T_U8 brightness: brightness value
 * @RETURN T_U8: new brightness value after setting
 * @RETVAL
 */
T_U8 lcdbl_set_brightness(T_eLCD lcd, T_U8 brightness)
{
#ifdef USE_LCD_BACKLIGHT_AAT3140
	lcdbl_aat3140_set_brightness(lcd, brightness);
#endif

#ifdef USE_LCD_BACKLIGHT_AAT3155
	lcdbl_aat3155_set_brightness(lcd, brightness);
#endif

#ifdef USE_LCD_BACKLIGHT_LM2704
	lcdbl_lm2704_set_brightness(lcd, brightness);
#endif

#ifdef USE_LCD_BACKLIGHT_TPS61061
	lcdbl_tps61061_set_brightness(lcd, brightness);
#endif

#ifdef USE_LCD_BACKLIGHT_FAN5607
	lcdbl_fan5607_set_brightness(lcd, brightness);
#endif

#ifdef USE_LCD_BACKLIGHT_TL1763
    lcdbl_1763_set_brightness(lcd, brightness);
#endif

#ifdef USE_LCD_BACKLIGHT_A30BL
    lcdbl_A30BL_set_brightness(lcd, brightness);
#endif

#ifdef USE_LCD_BACKLIGHT_PT4401
	lcdbl_pt4401_set_brightness(lcd, brightness);
#endif

#ifdef USE_LCD_BACKLIGHT_TWO_GPIO
	lcdbl_gpio_set_brightness(lcd, brightness);
#endif

	return brightness;
}

/**
 * @BRIEF Set backlight chip brightness value
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @PARAM T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @PARAM T_U8 brightness: brightness value
 * @RETURN
 * @RETVAL
 */
static T_VOID lcdbl_1763_set_brightness(T_eLCD lcd, T_U8 brightness)        //DC8312
{
#ifdef USE_LCD_BACKLIGHT_TL1763

	T_U8 i;

	store_all_int();
	gpio_set_pin_dir( GPIO_LCDBL_CHIP_ENABLE, 1 );
	gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
	us_delay( 1 );
	
	for(i = 0;i<(15-2*brightness);i++)
	{
		gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
		us_delay( 20);
		gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
		us_delay( 20 );
	}

	gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
		//us_delay( 600 );
		//gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );

	restore_all_int(); //restore interrupt

#endif
}

/**
 * @BRIEF Set backlight chip brightness value
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @PARAM T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @PARAM T_U8 brightness: brightness value
 * @RETURN
 * @RETVAL
 */
static T_VOID lcdbl_aat3140_set_brightness(T_eLCD lcd, T_U8 brightness)        //DC8312
{
#ifdef USE_LCD_BACKLIGHT_AAT3140

	T_U8 j;

	gpio_set_pin_dir(GPIO_LCDBL_CHIP_ENABLE, 1);

	//back-light IC has 32 step brightness, MMI has only 7 step brightness.
	j = brightness * 4;

	store_all_int();
	if (j == 0)
	{
		gpio_set_pin_level(GPIO_LCDBL_CHIP_ENABLE, 0);
		us_delay(500);		  
	}
	else
	{
		//off black light with 500us low level
		gpio_set_pin_level(GPIO_LCDBL_CHIP_ENABLE, 0);
		us_delay(500);

		do 
		{
    		gpio_set_pin_level(GPIO_LCDBL_CHIP_ENABLE, 0);                 
		    us_delay(1);
			gpio_set_pin_level(GPIO_LCDBL_CHIP_ENABLE, 1);                     
		    us_delay(1);
		} while (--j);

		//latch time out with 500us High level
		gpio_set_pin_level(GPIO_LCDBL_CHIP_ENABLE, 1);		
		us_delay(500);
	}
	restore_all_int();

#endif
}

/**
 * @BRIEF Set backlight chip brightness value
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @PARAM T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @PARAM T_U8 brightness: brightness value
 * @RETURN
 * @RETVAL
 */
static T_VOID lcdbl_aat3155_set_brightness(T_eLCD lcd, T_U8 brightness)        //CS2303,CI2304
{
#ifdef USE_LCD_BACKLIGHT_AAT3155

	T_U8 i, j;
	
	gpio_set_pin_dir(GPIO_LCDBL_CHIP_ENABLE, 1);
	
	//back-light IC has 16 step brightness, MMI has only 7 step brightness.
	j = ( brightness * 3 )>>1;
	
	store_all_int();
	if (j == 0)
	{		
		gpio_set_pin_level(GPIO_LCDBL_CHIP_ENABLE, 0);
		us_delay( 500 );
	}
	else
	{				
		for (i=15; i>j; i--)
		{
			gpio_set_pin_level(GPIO_LCDBL_CHIP_ENABLE, 0);
			us_delay(1);
			gpio_set_pin_level(GPIO_LCDBL_CHIP_ENABLE, 1);
			us_delay(1);
		}
		us_delay(500);
	}
	restore_all_int();

#endif
}

/**
 * @BRIEF Set backlight chip brightness value
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @PARAM T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @PARAM T_U8 brightness: brightness value
 * @RETURN
 * @RETVAL
 */
static T_VOID lcdbl_lm2704_set_brightness(T_eLCD lcd, T_U8 brightness)        //AM2402
{
#ifdef USE_LCD_BACKLIGHT_LM2704

    gpio_set_pin_dir(GPIO_LCDBL_CHIP_ENABLE, 1);
    
    if (brightness == 0)
        gpio_set_pin_level(GPIO_LCDBL_CHIP_ENABLE, 0);
    else
        gpio_set_pin_level(GPIO_LCDBL_CHIP_ENABLE, 1);

#endif
}

/**
 * @BRIEF Set backlight chip brightness value
 * in here use gpio control;if use PWN control,please add code 
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @PARAM T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @PARAM T_U8 brightness: brightness value
 * @RETURN
 * @RETVAL
 */
static T_VOID lcdbl_tps61061_set_brightness(T_eLCD lcd, T_U8 brightness)        //AI2301,AM2302
{
#ifdef USE_LCD_BACKLIGHT_TPS61061

	T_U8 i, j;

	gpio_set_pin_dir( GPIO_LCDBL_CHIP_ENABLE, 1 );

	//back-light IC has 32 step brightness, MMI has only 7 step brightness.
	j = brightness * 4;

	if ( j == 0 )
	{
		gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
	}
	else
	{
		store_all_int();
		
		gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
		us_delay( 2000 );
		gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
		us_delay( 2000 );

		if( j > 16 )
		{
			for( i=16; i<j; i++ )
			{
				gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
				gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
				us_delay( 20 );
				gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
			}
		}
		else
		{
			for( i=16; i>j; i-- )
			{
				gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
				gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
				us_delay( 240 );
				gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
			}
		}
		restore_all_int();
	}

#endif
}

/**
 * @BRIEF Set backlight chip brightness value
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @PARAM T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @PARAM T_U8 brightness: brightness value
 * @RETURN
 * @RETVAL
 */
static T_VOID lcdbl_fan5607_set_brightness(T_eLCD lcd, T_U8 brightness)        //CI2401
{
#ifdef USE_LCD_BACKLIGHT_FAN5607

    T_U32 pwm_freq = 1000;  //pwm is 1K, duty cycle range is 22 to 78 except 0 and 100
    
    switch(brightness)
    {
		case 0:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 0);
			break;
		case 1:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 30); 
			break;
		case 2:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 40);
			break;
		case 3:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 50);
			break;
		case 4:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 60);
			break;
		case 5:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 65);
			break;
		case 6:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 70);
			break;
		case 7:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 78);
			break;
		default:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 60);
			break;
    }

#endif
}


#define ADDRESS_D1_D6 17
#define ADDRESS_D1_D5 18
#define ADDRESS_D6_CUR 19
#define ADDRESS_MAX_CUR 20
#define ADDRESS_LOW_CUR 20

#define MAX_CUR_20MA 1
#define MAX_CUR_30MA 2
#define MAX_CUR_15MA 3

static T_VOID SetENRisingEdge(T_VOID)
{
   gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );				
   gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
   us_delay( 60 );
   gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
   us_delay( 60 );

}

static T_VOID StopENRisingEdge(T_VOID)
{
	gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
	us_delay( 60 );
	gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
	us_delay( 500 );

}

/**
 * @BRIEF Set backlight chip brightness value
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @PARAM T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @PARAM T_U8 brightness: brightness value
 * @RETURN
 * @RETVAL
 */
static T_VOID lcdbl_A30BL_set_brightness(T_eLCD lcd, T_U8 brightness)        //CI2401
{

	T_U8 i, j;
  /*SIMCOM wangyichun, 07-04-21, Bug 40935, start */
   gpio_set_pin_dir( GPIO_LCDBL_CHIP_ENABLE, 1 );

	//back light level on keypad lock situation
   if( brightness == 0 )
	{
		j = 0;
		gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
	}
  /*SIMCOM wangyichun, 07-04-21, Bug 40935, end */
	else
	{
		//back-light IC has 16 step brightness, MMI has only 7 step brightness.		
		 j = 16 - (( brightness ) * 2 + 1);
			
        for( i=1; i< ADDRESS_MAX_CUR; i++ )
		{	
		  SetENRisingEdge();              
		}
          StopENRisingEdge();
			

	    for( i=1; i< MAX_CUR_20MA; i++ )
		{	
		  SetENRisingEdge();              
		}
          StopENRisingEdge();

        for( i=1; i< ADDRESS_D1_D5; i++ )
		{	
		  SetENRisingEdge();              
		}
          StopENRisingEdge();
		
        for( i=1; i<j; i++ )
		{	
          SetENRisingEdge(); 				
		}
          StopENRisingEdge();					
	}

}

/**
 * @BRIEF Set backlight chip brightness value
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-04
 * @PARAM T_eLCD lcd: selected LCD, must be LCD_0 or LCD_1
 * @PARAM T_U8 brightness: brightness value
 * @RETURN
 * @RETVAL
 */
static T_VOID lcdbl_pt4401_set_brightness(T_eLCD lcd, T_U8 brightness)       
{
#ifdef USE_LCD_BACKLIGHT_PT4401

    T_U32 pwm_freq = 4000;  //pwm is 4K, duty cycle range is 0 to 100
    
    switch(brightness)
    {
		case 0:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 0);
			break;
		case 1:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 30); 
			break;
		case 2:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 40);
			break;
		case 3:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 50);
			break;
		case 4:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 55);
			break;
		case 5:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 60);
			break;
		case 6:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 70);
			break;
		case 7:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 80);
			break;
		default:
			pwm_set_duty_cycle(uiPWM1, pwm_freq, 55);
			break;
    }

#endif
}

static T_VOID lcdbl_gpio_set_brightness(T_eLCD lcd, T_U8 brightness)        //CS2303,CI2304
{
#ifdef USE_LCD_BACKLIGHT_TWO_GPIO
	
	T_U8 bb = brightness>>1;
	if (brightness==1) bb=1;
	store_all_int();
	gpio_set_pin_dir( GPIO_LCDBL_CHIP_ENABLE, 1 );
	gpio_set_pin_dir( GPIO_LCDBL_CHIP_2, 1 );
	switch(bb)
	{
		case 0:
				gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
				gpio_set_pin_level( GPIO_LCDBL_CHIP_2, 0 );
			break;
		case 1:	
				gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 0 );
				gpio_set_pin_level( GPIO_LCDBL_CHIP_2, 1 );
			break;
		case 2:	
				gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
				gpio_set_pin_level( GPIO_LCDBL_CHIP_2, 0 );
			break;
		case 3:	
				gpio_set_pin_level( GPIO_LCDBL_CHIP_ENABLE, 1 );
				gpio_set_pin_level( GPIO_LCDBL_CHIP_2, 1 );
			break;
	}			
	akprintf(C3, M_DRVSYS, "brightness = %d\n", brightness);
	restore_all_int(); //restore interrupt
	
#endif
}

