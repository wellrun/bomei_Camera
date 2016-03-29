/**
 * @FILENAME: gpio_pin_CI7801.c
 * @BRIEF config gpio
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR tangjianlong
 * @DATE 2008-01-14
 * @VERSION 1.0
 * @REF
 */
#ifdef OS_ANYKA
#ifdef CI37XX_PLATFORM


#include "keypad_define.h"
#include "gpio_config.h"
#include "drv_in_callback.h"

extern const T_VOID *keypad_get_platform_parm(T_VOID);
extern T_KEYPAD_TYPE keypad_get_platform_type(T_VOID);
extern T_BOOL mmc_is_connected(T_VOID);
extern T_BOOL sd_is_connected(T_VOID);

/* how to set the pin state 
if the default level is high, enable pullup, else disable pullup
 * gpio of keypad should not define in this array.
 */
T_GPIO_SET m_uGpioSetting[] =
{
    //pinNum,                                       pull,                           pinDir,             pinDefaultLevel,        pinActiveLevel
    {GPIO_RING_DETECT,          ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 7    
    {GPIO_LCDBL_CHIP_ENABLE,    ePullDownDis,  GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 10        
    {GPIO_KEYPAD_BL,            ePullDownDis,  GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 11
    {GPIO_AUDIO_AMP,            ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 28
    /**because multi use with GPIO_HEADSET_DETECT*/
    {GPIO_POWER_OFF,            ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_HIGH,  GPIO_LEVEL_HIGH},   //GPIO 3
    {GPIO_FM_RESET,             ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_HIGH,  GPIO_LEVEL_HIGH},   //GPIO 3
    {GPIO_USB_DETECT,           ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //DGPIO 19
    {GPIO_SWITCH_KEY,           ePullDownDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 10
    {GPIO_SD_DETECT,            ePullDownDis,  GPIO_DIR_INPUT,      GPIO_LEVEL_HIGH,GPIO_LEVEL_LOW},      //GPIO 1
    {GPIO_MMC_DETECT,           ePullDownDis,  GPIO_DIR_INPUT,      GPIO_LEVEL_HIGH,GPIO_LEVEL_LOW},      //GPIO 1
    {GPIO_NAND_WP,              ePullUpEn,     GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,  GPIO_LEVEL_HIGH},       //GPIO 40
    {GPIO_DTR,                  ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_HIGH,  GPIO_LEVEL_HIGH},   //GPIO 42
    {GPIO_TSCR_ADC,             ePullDownDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,   GPIO_LEVEL_LOW},   //GPIO 44    
    {GPIO_FLASH_LIGHT1,         ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 46
    {GPIO_FLASH_LIGHT2,         ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 47
    {GPIO_MODULE_RESET,         ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 51
    //{GPIO_DCIN_DETECT,          ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //DGPIO 19
    {GPIO_HEADSET_DETECT,       ePullDownDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 3
    {GPIO_CHARGE_STATUS,        ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 27
    {GPIO_CAMERA_AVDD,          ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 27
    {GPIO_BT_PWREN,             ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 2
    {GPIO_BT_RST,               ePullUpEn,     GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 2
    //{GPIO_BT_WAKEUP,            ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 4
    {GPIO_KEYAPD_ROW0,          ePullDownEn,    GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 7
    {GPIO_KEYAPD_ROW1,          ePullDownEn,    GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 13
    {GPIO_KEYAPD_COLUMN0,        ePullDownDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,    GPIO_LEVEL_LOW},    //GPIO 6
    {GPIO_KEYAPD_COLUMN1,        ePullDownDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,    GPIO_LEVEL_LOW},    //DGPIO28
    {GPIO_KEYAPD_COLUMN2,        ePullDownDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,    GPIO_LEVEL_LOW},    //DGPIO28
    {GPIO_INNO_POWER,           ePullDownDis,   GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO     
    {GPIO_CAMERA_RESET,         ePullUpDis,     GPIO_DIR_OUTPUT,  GPIO_LEVEL_HIGH,  GPIO_LEVEL_HIGH},   //GPIO 
    {GPIO_USB_DRV_BUS,          ePullUpDis,     GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,  GPIO_LEVEL_LOW},   //GPIO 
    {GPIO_HEADSET_MUTE,         ePullUpEn,     GPIO_DIR_OUTPUT,  GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 47
    {GPIO_LCD_RST,             ePullUpEn,     GPIO_DIR_OUTPUT,   GPIO_LEVEL_HIGH,   GPIO_LEVEL_HIGH},     //GPIO5
    {GPIO_POWER_KEY,             ePullUpDis,     GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},  //GPIO20  
    {GPIO_END_FLAG,             ePullUpEn,     GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_LOW} ,    //end

};


T_VOID gpio_set_expand_callback(T_U32 pin, T_U8 level)
{

}

/**
 * @BRIEF get wakeup gpio mask of set
 * @AUTHOR tangjianlong
 * @DATE 2008-01-14
 * @PARAM[out] T_U32 * wgpio_mask: pointer of data mask of get
 * @RETURN T_VOID
 * @RETVAL
 */

 #define INVALID_GPIO                    0xfe
 #define WAKE_CONFIG_REG_NUM             2
 #define ANALOG_KEYPAP_WAKEUP            86
 
 T_VOID get_wGpio_Mask(T_U32 *wgpio_mask)

 {    
    T_U8 i = 0;
	volatile T_U32 ctreg = 0;
	volatile T_U32 tmp = 0;
	
    const T_PLATFORM_KEYPAD_GPIO *keypad_parm = (T_PLATFORM_KEYPAD_GPIO *)keypad_get_platform_parm();

    if (KEYPAD_ANALOG != keypad_get_platform_type())
    {
        // set keypad wake up
        for (i=0 ; i <  keypad_parm->row_qty; i++)
        {
            tmp = (1 << get_wGpio_Bit(keypad_parm->RowGpio[i], &ctreg));

            if ((tmp != INVALID_GPIO) && (ctreg < WAKE_CONFIG_REG_NUM))
            {
				wgpio_mask[ctreg] |= tmp;
				gpio_set_wakeup_p(keypad_parm->RowGpio[i], keypad_parm->active_level);				
            }
                     
        }

        // set power key wake up
        if (GPIO_LEVEL_HIGH == keypad_parm->switch_key_active_level
                && GPIO_LEVEL_LOW == gpio_get_pin_level(keypad_parm->switch_key_id))
        {
            tmp = (1 << get_wGpio_Bit(keypad_parm->switch_key_id, &ctreg));
            
            if ((tmp != INVALID_GPIO) && (ctreg < WAKE_CONFIG_REG_NUM))
            {
	            wgpio_mask[ctreg] |= tmp;
                gpio_set_wakeup_p(keypad_parm->switch_key_id, keypad_parm->switch_key_active_level);
            }
            
        }
    }

    if (KEYPAD_ANALOG == keypad_get_platform_type())//analog key 
    {
    	tmp = (1 << get_wGpio_Bit(ANALOG_KEYPAP_WAKEUP, &ctreg));

	    if ((tmp != INVALID_GPIO) && (ctreg < WAKE_CONFIG_REG_NUM))
	    {
        	wgpio_mask[ctreg] |= tmp;
			gpio_set_wakeup_p(ANALOG_KEYPAP_WAKEUP, AK_FALSE); 
	    }

    }

    
	tmp = (1 << get_wGpio_Bit(GPIO_POWER_KEY, &ctreg));
	
    if ((tmp != INVALID_GPIO) && (ctreg < WAKE_CONFIG_REG_NUM))
    {
        wgpio_mask[ctreg] |= tmp;
        
        if (gpio_pin_get_ActiveLevel(GPIO_POWER_KEY) != gpio_get_pin_level(GPIO_POWER_KEY))   
	    {
			gpio_set_wakeup_p(GPIO_POWER_KEY,  !gpio_pin_get_ActiveLevel(GPIO_POWER_KEY));            
	    }

    }


    //=============== set MMC detect cable wake up================
    
	tmp = (1 << get_wGpio_Bit(GPIO_MMC_DETECT, &ctreg));
	
    if ((tmp != INVALID_GPIO) && (ctreg < WAKE_CONFIG_REG_NUM))
    {
        wgpio_mask[ctreg] |= tmp;
        
        if (gpio_pin_get_ActiveLevel(GPIO_MMC_DETECT) != gpio_get_pin_level(GPIO_MMC_DETECT))   
	    {
			gpio_set_wakeup_p(GPIO_MMC_DETECT,  !gpio_pin_get_ActiveLevel(GPIO_MMC_DETECT));            
	    }
	    else
	    {
			gpio_set_wakeup_p(GPIO_MMC_DETECT,  gpio_pin_get_ActiveLevel(GPIO_MMC_DETECT)); 

	    }

    }

    //==================================================
#ifdef TOUCH_PANEL_WAKE_UP
    // set Touch panel wake up
    if (!gb_stdb.bKeypadLock)
    {
        if (gpio_pin_get_ActiveLevel(GPIO_TSCR_ADC) == GPIO_LEVEL_HIGH
            && gpio_get_pin_level(GPIO_TSCR_ADC) == GPIO_LEVEL_LOW)
        {
            tmp = (1 << get_wGpio_Bit(GPIO_TSCR_ADC, &ctreg));
            
            if ((tmp != INVALID_GPIO) && (ctreg < WAKE_CONFIG_REG_NUM))
	        {
	            wgpio_mask[ctreg] |= tmp;
				gpio_set_wakeup_p(GPIO_TSCR_ADC,  gpio_pin_get_ActiveLevel(GPIO_TSCR_ADC));	            
	        }
            
        }
    }
#endif

#ifdef SUPPORT_FLIP
    // FLIP
    if (gpio_pin_get_ActiveLevel(GPIO_FLIP) == GPIO_LEVEL_HIGH
            && gpio_get_pin_level(GPIO_FLIP) == GPIO_LEVEL_LOW)  
    {
        tmp = (1 << get_wGpio_Bit(GPIO_FLIP, &ctreg));
        
	    if ((tmp != INVALID_GPIO) && (ctreg < WAKE_CONFIG_REG_NUM))
        {
            wgpio_mask[ctreg] |= tmp;
			gpio_set_wakeup_p(GPIO_FLIP,  gpio_pin_get_ActiveLevel(GPIO_FLIP));
        }

    }    
#endif
}

 
/**
 * @BRIEF gpio share pin init, set all pin as gpio
 * @AUTHOR  kejianping
 * @DATE 2014-10-16
 * @NOTE:set all the control signal for peripheral
 */
static T_VOID gpio_share_pin_init(T_VOID)
{
    gpio_pin_group_cfg(ePIN_AS_GPIO);   
}


/**
 * @brief: initialize system gpio pin
 * @note: this function should be called only once! so we usually place it in bios.
 *
 * @author tangjianlong
 * @date 2008-01-14
 */
T_VOID gpio_pin_init(T_VOID)
{
    gpio_init(); 
	gpio_share_pin_init();		
    gpio_pin_config_init(m_uGpioSetting);    
}

#endif  //#ifdef CI7801_PLATFORM
#endif
