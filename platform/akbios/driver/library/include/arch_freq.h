/**
 * @file arch_freq.h
 * @brief list frequency operation interfaces.

 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Liao_Zhijun
 * @date 2010-04-15
 * @version 1.0
 */

#ifndef __ARCH_FREQ_H__
#define __ARCH_FREQ_H__

/** @defgroup Frequency Frequency group
 *  @ingroup Drv_Lib
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif

/** @name define the main clock 
 */
#define MAIN_CLK    (get_chip_mainclk(get_pll_value()))

/** @name define the asic clock 
 */
#define ASIC_CLK    (MAIN_CLK>>1)

/** @name asic frequency define
 *  define the asic frequency by divider
 */
/*@{*/
#define ASIC_FREQ_BY_DIV1       (ASIC_CLK/1)
#define ASIC_FREQ_BY_DIV2       (ASIC_CLK/2)
#define ASIC_FREQ_BY_DIV4       (ASIC_CLK/4)
#define ASIC_FREQ_BY_DIV8       (ASIC_CLK/8)
#define ASIC_FREQ_BY_DIV16      (ASIC_CLK/16)
#define ASIC_FREQ_BY_DIV32      (ASIC_CLK/32)
#define ASIC_FREQ_BY_DIV64      (ASIC_CLK/64)
/*@} */

/**
 * @brief    get asic frequency node.
 *
 * @author    Luheshan
 * @date      2012-06-08
 * @param    [in] pll_val: the pll frequency (HZ).
 * @param    [in] min_freq: the min asic frequency(HZ).
 * @param    [in/out] node_list:input a pointer what is note frequency list.can be null,and to get node_cnt
 * @param    [in/out] node_cnt:input a pointer and get the asic frequency count.null is invalidation.
 * @return    T_BOOL
 * @retval    AK_TRUE  set asic frequency successful
 * @retval    AK_FALSE set asic frequency unsuccessful
 */
T_BOOL get_asic_node(T_U32 pll_freq, T_U32 min_freq, T_U32 **node_list, T_U32 *node_cnt);

/**
 * @brief   set asic frequency.
 *
 * @author  liaozhijun
 * @date    2010-04-06
 * @param   freq [in] the freq value to be set, refer to T_ASIC_FREQ definition
 * @return  T_BOOL
 * @retval  AK_TRUE  set asic frequency successful
 * @retval  AK_FALSE set asic frequency unsuccessful
 */
T_BOOL set_asic_freq(T_U32 freq);

/**
 * @brief   get current asic frequency.
 *
 * @author  liaozhijun
 * @date    2010-04-06
 * @return  T_U32 the frequency of asic running
 */
T_U32 get_asic_freq(T_VOID);

/**
 * @brief   set PLL register value.
 *
 * main clock is controlled by pll register.
 * @author  liaozhijun
 * @date    2010-04-06
 * @param   pll_value [in] pll register value(16-376)(Mhz)
 * @return  T_BOOL
 * @retval  AK_TRUE  set pll frequency successful
 * @retval  AK_FALSE set pll frequency unsuccessful
  */
T_BOOL set_pll_value(T_U32 pll_value);

/**
 * @brief   get PLL register value.
 *
 * main clock is controlled by pll register.
 * @author  liaozhijun
 * @date    2010-04-06
 * @return  T_U32 the frequency of pll(Mhz)
*/
T_U32 get_pll_value(T_VOID);

/**
 * @brief   get chip main clock(Hz).
 *
 * main clock is controlled by pll register.
 * @author  liaozhijun
 * @date    2010-04-06
 * @param   pll_value [in] pll register value
 * @return  T_U32 the main clock frequency = pll_value * 1000000L 
  */
T_U32 get_chip_mainclk(T_U32 pll_value);

/**
 * @brief   set cpu frequency twice of asic frequency or not
 *
 * this function just set cpu_clk = PLL1_clk or set cpu_clk = asic_clk
 * @author  liaozhijun
 * @date    2010-04-06
 * @param   set_value  set twice or not
 * @return  T_BOOL
 * @retval  AK_TRUE setting successful
 * @retval  AK_FALSE setting fail
 */
T_BOOL set_cpu_2x_asic(T_BOOL set_value);

/**
 * @brief   judge whether cpu frequency is twice of asic frequency or not
 *          
 * @author  liaozhijun
 * @date    2010-04-06
 * @return  T_BOOL
 * @retval  AK_TRUE cpu frequency is twice of asic frequency
 * @retval  AK_FALSE cpu frequency is not twice of asic frequency
 */
T_BOOL get_cpu_2x_asic(T_VOID);

/**
 * @brief   get cpu frequency.
 *
 * cpu frequency can the same as asic frequency or 2X/3X of that
 * @author  liaozhijun
 * @date    2010-04-06
 * @return  T_U32 the cpu frequency
 */
T_U32 get_cpu_freq(T_VOID);

/**
 * @brief   Set cpu clock = pll1 clock = 3 * asic clock = 300MHz
 * Warning:
 * For AK8802 3ES, and only cpu core voltage higher 1.3V use only.
 * Before entry 3x config, will store old pll1 value and when exit 3x cfg,
 * it will restore old pll1 value.
 *
 * @author  liaozhijun
 * @date    2010-04-06
 * @param   pll_value: [in]  pll frequency
 * @param   set_value: AK_TRUE is to enable cpu3x
 * @return  T_BOOL
 * @retval  AK_TRUE: cpu frequency three of asic frequency
 * @retval  AK_FALSE: when cpu at 2x or 3x ready.
 */
T_BOOL set_cpu_3x_asic(T_U32 pll_value, T_BOOL set_value);


/**
 * @brief       set or exit  boost mode, asic = pll/2.5
 *          
 * @author      liaozhijun
 * @date        2010-09-20
 * @param[in]   pll_value  pll frequency
 * @param[in]   enable AK_TRUE is to enable boost mode
 * @return      T_BOOL
 * @retval      AK_TRUE success to set/exit boost mode
 * @retval      AK_FALSE fail to set/exit boost mode
 */
T_BOOL set_boost_mode(T_U32 pll_value, T_BOOL enable);

/**
 * @brief       set or exit  cpu low mode, cpu = asic
 *          
 * @author      liaozhijun
 * @date        2010-09-20
 * @param[in]   asic  asic frequency
 * @param[in]   enable AK_TRUE is to enable cpu low mode
 * @return      T_BOOL
 * @retval      AK_TRUE success to set/exit cpu low mode
 * @retval      AK_FALSE fail to set/exit cpu low mode
 */
T_U32 set_cpu_low_mode(T_U32 asic, T_BOOL enable);


#ifdef __cplusplus
}
#endif

/*@}*/
#endif //#ifndef __ARCH_FREQ_BASE__
