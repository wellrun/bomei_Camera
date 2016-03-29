
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_gpio.h"

#define PWM_MAX_DUTY_CYCLE  (100)

static const T_U32 pwm_ctrl_reg1_grp[MAX_PWM_NUM] = {PWM_TIMER1_CTRL_REG1, PWM_TIMER2_CTRL_REG1, PWM_TIMER3_CTRL_REG1, PWM_TIMER4_CTRL_REG1, PWM_TIMER5_CTRL_REG1};
static const T_U32 pwm_ctrl_reg2_grp[MAX_PWM_NUM] = {PWM_TIMER1_CTRL_REG2, PWM_TIMER2_CTRL_REG2, PWM_TIMER3_CTRL_REG2, PWM_TIMER4_CTRL_REG2, PWM_TIMER5_CTRL_REG2};

static T_U16 pwm_duty_cycle[MAX_PWM_NUM] = {0};
static T_U32 m_pwm_freq[MAX_PWM_NUM] = {0};

/**
 * @brief   set duty cycle of pwm
 *
 * @author
 * @date
 * @param   [in] pwm_freq frequency of pwm, must in [92HZ--6MHZ] for AK7802
 * @param   [in] duty_cycle duty_cycle of pwm
 * @return  T_BOOL
 * @retval  AK_TRUE set successfully
 * @retval  AK_FALSE set unsuccessfully
 */
T_BOOL pwm_set_duty_cycle(T_PWM_ID pwm_id, T_U32 pwm_freq, T_U16 duty_cycle)
{
    T_U32 freq_div;
    T_U32 high_level_clk, low_level_clk;
    T_U32 temp_freq;

    temp_freq = 25000000;

    if (pwm_id >= MAX_PWM_NUM || pwm_id < uiPWM1)
    {
        akprintf(C1, M_DRVSYS, "pwm_id %d is not in valid range %d \n", pwm_id);
        return AK_FALSE;
    }

    if ((92 >= pwm_freq) || (6000000 < pwm_freq))
    {
        akprintf(C1, M_DRVSYS, "pwm freq %d is not in valid range %d \n", pwm_freq);
        return AK_FALSE;
    }
    if (temp_freq % pwm_freq)
    {
        akprintf(C1, M_DRVSYS, "pwm freq %d can not divide by asic freq %d \n", pwm_freq, temp_freq);
        return AK_FALSE;
    }

    if (0 == duty_cycle)
    {
        high_level_clk = 0x00;
        low_level_clk  = 0xffff;
    }
    else if (PWM_MAX_DUTY_CYCLE == duty_cycle)
    {
        high_level_clk = 0xffff;
        low_level_clk  = 0x00;
    }
    else
    {
        if (duty_cycle > PWM_MAX_DUTY_CYCLE)
        {
            akprintf(C1, M_DRVSYS, "duty_cycle %d is over PWM_MAX_DUTY_CYCLE 100\n", duty_cycle);
            return AK_FALSE;
        }

        freq_div = temp_freq / pwm_freq;
        
        high_level_clk = freq_div * duty_cycle / PWM_MAX_DUTY_CYCLE - 1;    
        if (high_level_clk > 65535)
        {
            akprintf(C1, M_DRVSYS, "high_level_clk %d is out of range 65535\n", high_level_clk);
            return AK_FALSE;
        }
        
        low_level_clk  = freq_div * (PWM_MAX_DUTY_CYCLE -duty_cycle) / PWM_MAX_DUTY_CYCLE - 1;
        if (low_level_clk > 65535)
        {
            akprintf(C1, M_DRVSYS, "low_level_clk %d is out of range 65535\n", low_level_clk);
            return AK_FALSE;
        }

        if ((high_level_clk + low_level_clk) * duty_cycle < PWM_MAX_DUTY_CYCLE)
        {
            akprintf(C1, M_DRVSYS, "pwm freq %d is not support duty cycle %d\n", pwm_freq, duty_cycle);
            return AK_FALSE;
        }
    }
    if (uiPWM1 == pwm_id)
    {
        gpio_pin_group_cfg(ePIN_AS_PWM1);
    }
    else if (uiPWM2 == pwm_id)
    {
        gpio_pin_group_cfg(ePIN_AS_PWM2);
    }    
    else
    {
        akprintf(C1, M_DRVSYS, "pwm %d is not support\n",pwm_id);
    }
    
    //set working mode and enable pwm
    irq_mask();
    REG32(pwm_ctrl_reg1_grp[pwm_id]) = (high_level_clk<<16) | low_level_clk;
    REG32(pwm_ctrl_reg2_grp[pwm_id]) = ((0x2<<24) | (1<<28));

    irq_unmask();
    
    pwm_duty_cycle[pwm_id] = duty_cycle;
    m_pwm_freq[pwm_id] = pwm_freq;
        
    return AK_TRUE;
}

/**
 * @brief  get duty cycle of pwm by id, just use in ak8801 or ak8802 chip 
 *
 * @author liao_zhijun
 * @date 2010_07-28
 * @param   [in] pwm_id pwm id
 * @return  T_U16 duty cycle
 */
T_U16 pwm_get_duty_cycle(T_PWM_ID pwm_id)
{
    if (pwm_id >= MAX_PWM_NUM || pwm_id < uiPWM1)
    {
        akprintf(C3, M_DRVSYS, "pwm_get_duty_cycle_ex pwm_id %d is not in valid range %d \n", pwm_id);
        return 0;
    }
    
    return pwm_duty_cycle[pwm_id];
}

/**
 * @brief  get freq of pwm by id, just use in ak8801 or ak8802 chip 
 *
 * @author liao_zhijun
 * @date 2010_07-28
 * @param   [in] pwm_id pwm id
 * @return  T_U32 freq
 */
T_U32 pwm_get_freq(T_PWM_ID pwm_id)
{
    if (pwm_id >= MAX_PWM_NUM || pwm_id < uiPWM1)
    {
        akprintf(C3, M_DRVSYS, "pwm_get_freq_ex pwm_id %d is not in valid range %d \n", pwm_id);
        return 0;
    }
    
    return m_pwm_freq[pwm_id];
}


