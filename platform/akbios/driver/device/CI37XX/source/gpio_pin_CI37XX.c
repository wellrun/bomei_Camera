/**
 * @FILENAME: gpio_pin_CI3771.c
 * @BRIEF config gpio
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR tangjianlong
 * @DATE 2008-01-14
 * @VERSION 1.0
 * @REF
 */

#ifdef CI37XX_PLATFORM

#include "akdefine.h"
#include "gbl_global.h"
#include "gpio.h"
#include "drv_api.h"
#include "keypad_type.h"

#ifdef CHIP_AK3771
#include "gpio_config_CI3771.h"
#elif CHIP_AK3753
#include "gpio_config_CI3753.h"
#elif CHIP_AK3750
#include "gpio_config_CI3750.h"
#endif


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
    {GPIO_SD_DETECT,            ePullUpEn,     GPIO_DIR_INPUT,    GPIO_LEVEL_HIGH,GPIO_LEVEL_LOW},    //GPIO 1
    {GPIO_NAND_WP,              ePullUpEn,     GPIO_DIR_OUTPUT,  GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},       //GPIO 40
    {GPIO_DTR,                  ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_HIGH,  GPIO_LEVEL_HIGH},   //GPIO 42
    {GPIO_TSCR_ADC,             ePullDownDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,   GPIO_LEVEL_LOW},   //GPIO 44    
    {GPIO_FLASH_LIGHT1,         ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 46
    {GPIO_FLASH_LIGHT2,         ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 47
    {GPIO_MODULE_RESET,         ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 51
    //{GPIO_DCIN_DETECT,          ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //DGPIO 19
    {GPIO_HEADSET_DETECT,       ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 3
    {GPIO_CHARGE_STATUS,        ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 27
    {GPIO_CAMERA_AVDD,          ePullUpDis,    GPIO_DIR_OUTPUT,  GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 27
    {GPIO_BT_PWREN,             ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 2
    {GPIO_BT_RST,               ePullUpEn,     GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 2
    //{GPIO_BT_WAKEUP,            ePullUpDis,    GPIO_DIR_INPUT,   GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 4
	{GPIO_KEYAPD_ROW0,          ePullDownEn,    GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 7
	{GPIO_KEYAPD_ROW1,          ePullDownEn,    GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_HIGH},   //GPIO 13
	{GPIO_KEYAPD_COLUMN0,		ePullDownDis,	GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,	GPIO_LEVEL_LOW},	//GPIO 6
	{GPIO_KEYAPD_COLUMN1,		ePullDownDis,	GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,	GPIO_LEVEL_LOW},	//DGPIO28
	{GPIO_KEYAPD_COLUMN2,		ePullDownDis,	GPIO_DIR_OUTPUT,  GPIO_LEVEL_LOW,	GPIO_LEVEL_LOW},	//DGPIO28
    {GPIO_HEADSET_MUTE,         ePullUpEn,     GPIO_DIR_OUTPUT,  GPIO_LEVEL_HIGH,  GPIO_LEVEL_LOW},    //GPIO 47
    {GPIO_END_FLAG,             ePullUpEn,     GPIO_DIR_INPUT,   GPIO_LEVEL_LOW,   GPIO_LEVEL_LOW}     //end
};


/**
 * @BRIEF gpio share pin init
 * @AUTHOR tangjianlong
 * @DATE 2008-01-14
 * @NOTE:set all the control signal for peripheral
 */
static T_VOID gpio_share_pin_init(T_VOID)
{
	T_U32 pin;

	printf("enter gpio_share_pin_init\n");

	//reset all except RAM bus to gpio
//	gpio_pin_group_cfg(ePIN_AS_JTAG);  	
    gpio_pin_group_cfg(ePIN_AS_GPIO);

	printf("after gpio_share_pin_init\n");
   
	/*gpio_pin_group_cfg(ePIN_AS_LCD);  	
	gpio_pin_group_cfg(ePIN_AS_CAMERA);  	
	gpio_pin_group_cfg(ePIN_AS_NANDFLASH);  	
	gpio_pin_group_cfg(ePIN_AS_MMC_SD);  	
*/
	//lcd backlight
	
	//gpio_set_pin_dir(9, 1);
	//gpio_set_pin_level(9, 1);
	
	//lcd power control
	//gpio_set_pin_share(81, ePIN_AS_GPIO);
}

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
T_VOID get_wGpio_Mask(T_U32 *wgpio_mask)
{
	T_U8 i = 0;
	const T_PLATFORM_KEYPAD_PARM *keypad_parm = keypad_get_platform_parm();

	// set keypad wake up
	for (i=0 ; i <  keypad_parm->column_qty; i++)
	{
		*wgpio_mask |= (1 << get_wGpio_Bit(keypad_parm->ColumnGpio[i]));
		gpio_set_wakeup_p(keypad_parm->ColumnGpio[i], keypad_parm->active_level);
	}

	// set power key wake up
	if (GPIO_LEVEL_HIGH == keypad_parm->switch_key_active_level
			&& GPIO_LEVEL_LOW == gpio_get_pin_level(keypad_parm->switch_key_id))
	{
		*wgpio_mask |= (1 << get_wGpio_Bit(keypad_parm->switch_key_id));
		gpio_set_wakeup_p(keypad_parm->switch_key_id, keypad_parm->switch_key_active_level);
	}

    // set usb cable wake up
    if (gpio_pin_get_ActiveLevel(GPIO_USB_DETECT) != gpio_get_pin_level(GPIO_USB_DETECT))
    {
       *wgpio_mask |= (1 << get_wGpio_Bit(GPIO_USB_DETECT));
		gpio_set_wakeup_p(GPIO_USB_DETECT,  gpio_pin_get_ActiveLevel(GPIO_USB_DETECT)); 
    }
	// set Charging wake up
//	if (gpio_pin_get_ActiveLevel(GPIO_CHARGING)  == GPIO_LEVEL_HIGH 
//			&& gpio_get_pin_level(GPIO_CHARGING) == GPIO_LEVEL_LOW) 
//	{
//		*wgpio_mask |= (1 << get_wGpio_Bit(GPIO_CHARGING));
//		gpio_set_wakeup_p(GPIO_CHARGING,  gpio_pin_get_ActiveLevel(GPIO_CHARGING));
//	}


	
#ifdef TOUCH_PANEL_WAKE_UP
	// set Touch panel wake up
	if (!gb_stdb.bKeypadLock)
	{
		if (gpio_pin_get_ActiveLevel(GPIO_TSCR_ADC) == GPIO_LEVEL_HIGH
			&& gpio_get_pin_level(GPIO_TSCR_ADC) == GPIO_LEVEL_LOW)
		{
			*wgpio_mask |= (1 << get_wGpio_Bit(GPIO_TSCR_ADC));
			gpio_set_wakeup_p(GPIO_TSCR_ADC,  gpio_pin_get_ActiveLevel(GPIO_TSCR_ADC));
		}
	}
#endif

#ifdef SUPPORT_FLIP
	// FLIP
	if (gpio_pin_get_ActiveLevel(GPIO_FLIP) == GPIO_LEVEL_HIGH
			&& gpio_get_pin_level(GPIO_FLIP) == GPIO_LEVEL_LOW)  
	{
		*wgpio_mask |= (1 << get_wGpio_Bit(GPIO_FLIP));
		gpio_set_wakeup_p(GPIO_FLIP,  gpio_pin_get_ActiveLevel(GPIO_FLIP));
	}	
#endif

//#ifdef USE_GPIO_DETECT_HEADSET
//	// set handset botton wake up
//	if (gpio_pin_get_ActiveLevel(GPIO_HEADSET_DETECT) == GPIO_LEVEL_HIGH
//			&& gpio_get_pin_level(GPIO_HEADSET_DETECT) == GPIO_LEVEL_LOW)    
//	{
//		*wgpio_mask |= (1 << get_wGpio_Bit(GPIO_HEADSET_DETECT));
//		gpio_set_wakeup_p(GPIO_HEADSET_DETECT,  gpio_pin_get_ActiveLevel(GPIO_HEADSET_DETECT));
//	}
//#endif
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
	//gpio init	
	gpio_init();
	printf("after gpio_init\n");
	gpio_share_pin_init();		
	printf("after gpio_share_pin_init\n");

	gpio_pin_config_init(m_uGpioSetting);	
	
}

#endif  //#ifdef CI7801_PLATFORM

