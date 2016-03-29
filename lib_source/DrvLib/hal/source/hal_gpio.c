/**
 * @FILENAME: hal_gpio.c
 * @BRIEF config gpio
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-12-19
 * @VERSION 1.0
 * @REF
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_gpio.h"

static T_GPIO_SET *m_pGpioSetting = AK_NULL;


/**
 * @BRIEF gpio pin config init
 * @AUTHOR guoshaofeng
 * @DATE 2007-12-19
 * @PARAM [in] pGpioSetting: config array pointer
 * @RETURN T_VOID
 * @RETVAL
 */
T_VOID gpio_pin_config_init(T_GPIO_SET *pGpioSetting)
{
    T_U32 i;

    for (i=0; ;i++)
    {
        if (GPIO_END_FLAG == pGpioSetting[i].pinNum)
        {
            break;
        }

        if (INVALID_GPIO == pGpioSetting[i].pinNum)
        {
            continue;
        }

        if (ePullUpEn == pGpioSetting[i].pull)
        {
            gpio_set_pull_up_r(pGpioSetting[i].pinNum, AK_TRUE);
        }
        else if (ePullUpDis == pGpioSetting[i].pull)
        {
            gpio_set_pull_up_r(pGpioSetting[i].pinNum, AK_FALSE);
        }
        else if (ePullDownEn == pGpioSetting[i].pull)
        {
            gpio_set_pull_down_r(pGpioSetting[i].pinNum, AK_TRUE);
        }
        else if (ePullDownDis == pGpioSetting[i].pull)
        {
            gpio_set_pull_down_r(pGpioSetting[i].pinNum, AK_FALSE);
        }
        
        gpio_set_pin_dir(pGpioSetting[i].pinNum, pGpioSetting[i].pinDir);
        if (GPIO_DIR_OUTPUT == pGpioSetting[i].pinDir)
        {
            gpio_set_pin_level(pGpioSetting[i].pinNum, pGpioSetting[i].pinDefaultLevel);
        }
    }	

    m_pGpioSetting = pGpioSetting;
}

/**
 * @BRIEF gpio_pin_get_ActiveLevel
 * @AUTHOR guoshaofeng
 * @DATE 2007-12-19
 * @PARAM [in] pin: gpio pin ID.
 * @RETURN T_U8: pin level, 1: high; 0: low; 0xff: invalid_level
 * @RETVAL
 */
T_U8 gpio_pin_get_ActiveLevel(T_U8 pin)
{
    T_U32 i;
    T_U8 activeLevel = 0xff;

    if (AK_NULL == m_pGpioSetting)
    {
        return activeLevel;
    }

    if (INVALID_GPIO == pin)
    {
        return activeLevel;
    }
    else
    {
        for (i=0; ;i++)
        {
            if (GPIO_END_FLAG == m_pGpioSetting[i].pinNum)
            {
                activeLevel = 0xff;
                break;
            }

            if (pin == m_pGpioSetting[i].pinNum)
            {
                activeLevel = m_pGpioSetting[i].pinActiveLevel;
                break;
            }
        }
    }

    return activeLevel;
}


/**
 * @brief get which pin wakeup  
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] reason wake up status.
 * @return T_U8
 * @retval gpio pin
 */
T_U8 gpio_get_wakeup_pin(T_U32 reason)
{
    T_U32 i;    

    for (i = 0 ; i <=31  ; i++)
    {
        if (((1 << i)  & reason) != 0 )
        {
            break;
        }
    }

    return get_wGpio_Pin(i);
}


