/**
 * @file drv_gpio.h
 * @brief list gpio operation intefaces.
 *
 * This file define gpio macros and APIs: intial, set gpio, get gpio. etc.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liaozhijun
 * @date 2005-07-24
 * @version 1.0
 *
 */

#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

/** @defgroup GPIO GPIO group
 *  @ingroup Drv_Lib
 */
/*@{*/
/*@}*/

/** @defgroup Arch_gpio Architecture of gpio
 *  @ingroup GPIO
 */
/*@{*/

/**@brief total number
 */
#define GPIO_NUMBER                     117

/**@brief invalidate gpio
 */
#define INVALID_GPIO                    0xfe

/** @name gpio output level define
 *  define gpio output level
 */
 /*@{*/
#define GPIO_LEVEL_LOW                  0
#define GPIO_LEVEL_HIGH                 1
/* @} */

/** @name gpio dir define
 *  define gpio dir
 */
 /*@{*/
#define GPIO_DIR_INPUT                  0
#define GPIO_DIR_OUTPUT                 1
/* @} */

/** @name gpio interrupt control define
 *  define gpio interrupt enable/disable
 */
 /*@{*/
#define GPIO_INTERRUPT_DISABLE          0
#define GPIO_INTERRUPT_ENABLE           1
/* @} */

/** @name gpio interrupt active level
 *  define gpio interrupt active level
 */
 /*@{*/
#define GPIO_INTERRUPT_LOW_ACTIVE       0
#define GPIO_INTERRUPT_HIGH_ACTIVE      1   
/* @} */

/** @name gpio interrupt type define
 *  define gpio interrupt type
 */
 /*@{*/
#define GPIO_LEVEL_INTERRUPT            0
#define GPIO_EDGE_INTERRUPT             1
/* @} */

/**
 * @brief share pins
 * 
 */
typedef enum
{
    ePIN_AS_GPIO = 0,           ///< All pin as gpio
    ePIN_AS_NANDFLASH,          ///< share pin as NFC
    ePIN_AS_NANDFLASH_EXT,      ///< share pin as NFC extend data line
    ePIN_AS_SDMMC1,             ///< share pin as MDAT1, 8 lines
    ePIN_AS_SDMMC2,             ///< share pin as MDAT2, 4lines
    ePIN_AS_USB,                ///< share pin as USB OTG
    ePIN_AS_I2S,                ///< share pin as I2S
    ePIN_AS_PWM1,               ///< share pin as PWM1
    ePIN_AS_PWM2,               ///< share pin as PWM2
    ePIN_AS_PWM3,               ///< share pin as PWM3
    ePIN_AS_PWM4,               ///< share pin as PWM4
    ePIN_AS_WATCH_DOG,          ///< share pin as watch dog
    ePIN_AS_RAM,                ///< share pin as RAM Controller
    ePIN_AS_HOSTINTERFACE,      ///< share pin as USB Host

    ePIN_AS_LCD_8BIT,           ///< share pin as LCD for 8bit
    ePIN_AS_LCD,                ///< share pin as LCD
    ePIN_AS_LCD_RGB_8BIT,       ///< share pin as RGBLCD for 8bit
    ePIN_AS_LCD_RGB,            ///< share pin as RGBLCD
    ePIN_AS_LCD_STN,            ///< share pin as RGBLCD
    ePIN_AS_LCD_EXT,            ///<share pin as LCD Extend dataline
    ePIN_AS_SDIO,               ///< share pin as SDIO
    ePIN_AS_UART1,              ///< share pin as UART1
    ePIN_AS_UART2,              ///< share pin as UART2
    ePIN_AS_UART3,              ///< share pin as UART3
    ePIN_AS_UART4,              ///< share pin as UART4
    ePIN_AS_CAMERA,             ///< share pin as CAMERA
    ePIN_AS_SPI1,               ///< share pin as SPI1
    ePIN_AS_SPI2,               ///< share pin as SPI2
    ePIN_AS_PCM,                ///< share pin as PCM
    ePIN_AS_JTAG,               ///< share pin as JTAG
    ePIN_AS_I2C,                ///< share pin as I2C
    ePIN_AS_MAC,                ///< share pin as Ethernet MAC
    ePIN_AS_CLK12MO,            
    ePIN_AS_CLK32KO,            

    ePIN_AS_DUMMY
}E_GPIO_PIN_SHARE_CONFIG;

/**
 * @brief gpio_attr
 * 
 */

typedef enum
{
    GPIO_ATTR_IE = 1,   ///<input enable
    GPIO_ATTR_PE,       ///<pullup/pulldown enable
    GPIO_ATTR_SL,       ///<slew rate
    GPIO_ATTR_DS,       ///<drive strength
    GPIO_ATTR_PS        ///<pullup/pulldown selection
}T_GPIO_PIN_ATTR;

/**
 * @brief GPIO callback define.
 */
typedef T_VOID (*T_fGPIO_CALLBACK)( T_U32 pin, T_U8 polarity );

/**
 * @brief Init gpio.
 * @author  liao_zhijun
 * @date 2010-07-28
 * @return T_VOID
 */
T_VOID gpio_init( T_VOID );

/**
 * @brief Set gpio output level
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] level 0 or 1.
 * @return T_VOID
 */
T_VOID gpio_set_pin_level( T_U32 pin, T_U8 level );

/**
 * @brief Get gpio input level
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @return T_U8
 * @retval 1 level high
 * @retval 0 level low;
 */
T_U8 gpio_get_pin_level( T_U32 pin );

/**
 * @brief Set gpio direction
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] dir 0 means input; 1 means output;
 * @return T_VOID
 */
T_VOID gpio_set_pin_dir( T_U32 pin, T_U8 dir );

/**
 * @brief gpio interrupt control
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] enable 1 means enable interrupt. 0 means disable interrupt.
 * @return T_VOID
 */
T_VOID gpio_int_control( T_U32 pin, T_U8 enable );

/**
 * @brief disable all gpio pin interrupt
 * @author  liao_zhijun
 * @date 2010-07-28
 * @return T_VOID
 */
T_VOID gpio_int_disableall(T_VOID);

/**
 * @brief restore all gpio pin interrupt
 * @author  liao_zhijun
 * @date 2010-07-28
 * @return T_VOID
 */
T_VOID gpio_int_restoreall(T_VOID);

/**
 * @brief set gpio interrupt polarity.
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] polarity 1 means active high interrupt. 0 means active low interrupt.
 * @return T_VOID
 */
T_VOID gpio_set_int_p( T_U32 pin, T_U8 polarity );

/**
 * @brief Register one gpio interrupt callback function.
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] polarity 1 means active high interrupt. 0 means active low interrupt.
 * @param[in] enable Initial interrupt state--enable or disable.
 * @param[in] callback gpio interrupt callback function.
 * @return T_VOID
 */
T_VOID gpio_register_int_callback( T_U32 pin, T_U8 polarity, T_U8 enable, T_fGPIO_CALLBACK callback );

/**
 * @brief gpio pull-up resistance configuration function
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] enable 1 means enable pull up. 0 means disable pull up.
 * @return T_BOOL
 * @retval AK_TURE configuration successful
 * @retval AK_FALSE configuration failed
 */
T_BOOL gpio_set_pull_up_r(T_U32 pin, T_BOOL enable);

/**
 * @brief gpio pull-down resistance configuration function
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] enable 1 means enable pull down. 0 means disable pull down.
 * @return T_BOOL
 * @retval AK_TURE configuration successful
 * @retval AK_FALSE configuration failed
 */
T_BOOL gpio_set_pull_down_r(T_U32 pin, T_BOOL enable);

/**
 * @brief get gpio wakeup pin bit
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @return T_U8
 * @retval gpio pin bit in wakeup register
 */
T_U8  get_wGpio_Bit(T_U8 pin);

/**
 * @brief Set gpio wake up polarity.
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] polarity 1 means high level. 0 means low level.
 * @return T_VOID
 */
T_VOID gpio_set_wakeup_p(T_U32 pin, T_BOOL polarity);

/**
 * @brief set gpio share pin as gpio 
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param pin [in]  gpio pin ID
 * @return T_BOOL
 * @retval AK_TRUE set successfully
 * @retval AK_FALSE fail to set
 */
T_BOOL  gpio_set_pin_as_gpio(T_U32 pin);

/**
 * @brief set gpio pin group as specified module used
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] PinCfg enum data. the specified module
 * @return T_BOOL
 * @retval AK_TURE setting successful
 * @retval AK_FALSE setting failed
*/
T_BOOL gpio_pin_group_cfg(E_GPIO_PIN_SHARE_CONFIG PinCfg);

/**
 * @brief set gpio pin attribute as IE, PE, etc.
 * @note only available for AK3810
 * @author tangjianlong
 * @date 2008-01-15
 * @param[in] pin the pin ID to set
 * @param[in] attr the atrribute to set
 * @param[in] enable enable the attribute or not
 * @return T_BOOL
 * @retval AK_TURE setting successful
 * @retval AK_FALSE setting failed
 */
T_BOOL gpio_set_pin_attribute(T_U32 pin, T_GPIO_PIN_ATTR attr, T_BOOL enable);

/*@}*/

#endif //#ifndef __ARCH_GPIO_H__

