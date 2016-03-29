/**@file hal_gpio.h
 * @brief gpio pin config
 
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author guoshaofeng
 * @date 2007-12-19
 * @version 1.0
 */

#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

/** @defgroup Hardware Abstract Layer of gpio
 *  @ingroup GPIO
 */
/*@{*/

typedef enum 
{
    ePullUpEn = 0,
    ePullUpDis,
    ePullDownEn,
    ePullDownDis
}T_ePIN_PULL_UP_DOWN;

typedef struct
{
    T_U8 pinNum;
    T_ePIN_PULL_UP_DOWN pull;
    T_U8 pinDir;
    T_U8 pinDefaultLevel;
    T_U8 pinActiveLevel;
}T_GPIO_SET;


#define GPIO_END_FLAG  0xff


/**
 * @brief gpio pin config init
 * @author guoshaofeng
 * @date 2007-12-19
 * @param [in] pGpioSetting config array pointer
 * @return T_VOID
 */
T_VOID gpio_pin_config_init(T_GPIO_SET *pGpioSetting);

/**
 * @brief gpio_pin_get_ActiveLevel
 * @author guoshaofeng
 * @date 2007-12-19
 * @param[in] pin gpio pin ID.
 * @return T_U8 pin level
 * @retval 1 high
 * @retval 0 low
 * @retval 0xff invalid_level
 */
T_U8 gpio_pin_get_ActiveLevel(T_U8 pin);


/**
 * @brief get which pin wakeup  
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] reason wake up status.
 * @return T_U8
 * @retval gpio pin
 */
T_U8 gpio_get_wakeup_pin(T_U32 reason);

/*@}*/
#endif
