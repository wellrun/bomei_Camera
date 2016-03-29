/**
 * @FILENAME: freq.c
 * @BRIEF freq driver file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR liao_zhijun
 * @DATE 2010-05-24
 * @VERSION 1.0
 * @REF
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "timer.h"
#include "l2.h"
#include "drv_module.h"
#include "freq.h"

/*
    ASPEN CPU clock test:
    以执行下面三条指令为例(共执行100兆次，T 表示执行时间):
          ADD      r0,r0,#1
        CMP      r0,r1
        BCC      0x300064d0
    以上三条指令所操作的数据均存放在寄存器中，因此在执行
    时，只会涉及到从内存取指的操作，以下分两种情况测试澹?
1.在打开ICACHE 和DCAHE的情况下
    当使能数据和指令CACHE时，CPU对这三条指令的操作只会涉及
    到寄存器和缓存的操作，速度最快
        CPU clock=60MHZ时，T=8.4s
        CPU clock=100MHZ时，T=5.0s    
    CPU 速度经倍频后的时间为:        
        CPU clock=60MHZ时，T=4.2s
        CPU clock=100MHZ时，T=2.5s

2.在关闭ICACHE 和DCACHE的情况下
    此时CPU需要到内存中取指，这将显著的降低CPU的执行速度
        CPU clock=60MHZ， T = 45.3s
    此外，在关闭ICACHE，打开DCACHE 的情况下，
        CPU clock=60MHZ， T = 44s

    从这两个数据可以看出，CPU在执行这种指令时的大部分时间
    被消耗在取指上

3.功耗
    在小系统中，关闭LCD的情况下测定
    CPU clock=60MHZ， I = 33mA
    CPU clock=200MHZ， I = 45mA

4. 计算方法
    给定特定的指令序列，其执行消耗的时间及占用的CPU clock的
    计算方法为:

    cnt:执行指令的次数
    n:   指令条数
    clk: CPU时钟周期
    每条ARM指令占用两个CPU时钟周期

    指令的执行时间T = n*cnt*2/clk

    每条指令占用的CPU时钟周期:
        t = T*clk/(n * cnt)    
*/

/* define max freqency divider */

#define PLL_CLK_MIN            225
#define PLL_CLK_MAX            (PLL_CLK_MIN + 5 *(0x3F))
#define PLL_CLK_MODERATE_MAX   (280)

#define CPU_3X_ENABLE           (1 << 30)
#define MARK_3X_CFG             (1 << 28)
#define CPU_2X_CFG              (1 << 15)

/* clock divider register */
#define PLL_CHANGE_ENA          (1 << 12)
#define CLOCK_ASIC_ENA          (1 << 14)

#define MAX_AISC_PRE_DIV        16
#define MAX_ASIC_DIV            128

//pll param
static struct pll_param
{
    T_U8    pll_sel;          ///<< M value of pll param
    T_U8    clk168_div;          ///<< N value of pll param 
}pll_param;


typedef enum
{
    FREQ_NORMAL_MODE,
    FREQ_CPU3X_MODE,
    FREQ_BOOST_MODE,
    FREQ_CPU_LOW_MODE
}
E_FREQ_MODE;

static T_BOOL exit_cpu_3x_asic(void);
T_BOOL is_cpu_3x(void);
T_BOOL is_cpu_2x(void);
T_U32 get_real_pll_value(T_VOID);
T_U32 get_clk168_value(T_VOID);
static void store_pll(void);
static void restore_pll(T_U32 *reg);
static T_VOID pll_change(T_U32 ratio);
T_U32 get_mem_freq();

//cpu 2x counter
static T_U16  volatile cpu_at_2x_counter=0;
static T_U8 s_asic_div = 0;

#define MAX_FREQ_CALLBACK       10

//callback func
static T_fFREQ_CHANGE_CALLBACK m_mem_cbk = AK_NULL;
static T_fFREQ_CHANGE_CALLBACK m_uart_cbk = AK_NULL;
static T_fFREQ_CHANGE_CALLBACK m_asic_cbk[MAX_FREQ_CALLBACK] = {0};

static T_U8 index=0;

static T_VOID freq_adjust(T_U32 freq)
{
    T_U32 i;

    //memory_timing adjust function should be called
    if(m_mem_cbk != AK_NULL)
        m_mem_cbk(freq);

    for(i = 0; i < index; i++)
    {
        m_asic_cbk[i](freq);
    }
}

T_BOOL get_asic_param(T_U32 clk168, T_U32 asic, T_U8 *pre_div, T_U8 *asic_div)
{
    T_U32 div, tmp;
    T_U32 i;
    
    div = clk168 / asic;
    
    if(clk168 % asic != 0)  div++;
    if(div & 0x1) div++;
    
    if(div > MAX_AISC_PRE_DIV*MAX_ASIC_DIV)
    {
        return AK_FALSE;
    }

    tmp = div;
    for(i = 0; i < 7; i++)
    {
        tmp >>= 1;
        if(tmp & 0x1) break;
    }

    tmp = div >> (i+1);
    if(tmp > MAX_AISC_PRE_DIV)
    {
        return AK_FALSE;
    }
    else
    {
        *pre_div = tmp - 1;
        *asic_div = i+1;
    }

    return AK_TRUE;
}

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
T_BOOL get_asic_node(T_U32 pll_freq, T_U32 min_freq, T_U32 **node_list, T_U32 *node_cnt)
{
    T_U32 i;
    T_U32 node_max = 0;
    T_U32 *pNode_list = *node_list;
    T_U32 asic;

    if (((AK_NULL != node_list) && (AK_NULL == *node_list)) || (AK_NULL == node_cnt))
    {
        akprintf(C1, M_DRVSYS, "get_asic_node(): param is null\n");
        return AK_FALSE;
    }

    if (min_freq > (pll_freq >> 1))
    {
        akprintf(C1, M_DRVSYS, "get_asic_node(): min_freq is no min\n");
        return AK_FALSE;
    }
    
    for (i = 1; ; i++)
    {
        if (min_freq > (pll_freq / (i * 2)))
        {
            node_max = i - 1;
            break;
        }
    }
    
    if (AK_NULL != node_list)
    {   
        if (node_max > *node_cnt)
        {
            akprintf(C1, M_DRVSYS, "get_asic_node(): node list memory may be no enough\n");
            return AK_FALSE;
        }

        for (i = 0; i < node_max; i++)
        {
            asic = pll_freq / ((node_max - i) * 2);
            if (0 != (pll_freq % ((node_max - i) * 2)))
            {
                asic++;
            }
            pNode_list[i] = asic;
        }
    }

    *node_cnt = node_max;
    
    return AK_TRUE;
}

/**
 * @brief    set asic frequency.
 *
 * @author    liaozhijun
 * @date     2010-04-06
 * @param    freq [in] the freq value to be set, refer to T_ASIC_FREQ definition
 * @return    T_BOOL
 * @retval    AK_TRUE  set asic frequency successful
 * @retval    AK_FALSE set asic frequency unsuccessful
 */
T_BOOL set_asic_freq(T_U32 freq)
{
    T_U32 ratio;
    T_U8 pre_div, asic_div = 0;
    T_U8 i;
    T_U32 MainFreq;
    T_U32 pre_freq, later_freq = 0;

    pre_freq = get_asic_freq();
    

    if (freq == pre_freq)
    {
        return AK_TRUE;
    }

    if (is_cpu_3x())
    {
        akprintf(C1, M_DRVSYS, "set_asic_freq(): under cpu3x!!\n");
        return AK_FALSE;
    }

    //calculate divide
    MainFreq = get_chip_mainclk(get_clk168_value());
    if(!get_asic_param(MainFreq, freq, &pre_div, &asic_div))
    {
        akprintf(C1, M_DRVSYS, "set_asic_freq(): cannot get proper asic div\n");
        return AK_FALSE;
    }
    
    DrvModule_Protect(DRV_MODULE_FREQ);
    
    //freq low to high, adjust asic module timing first
    if (freq > pre_freq)
    {
        freq_adjust(freq);
    } 

    /* set asic freq div */
    ratio = REG32(CLOCK_DIV_REG);

    ratio &= ~(0x1f << 21);
    if(pre_div != 0)
    {
        //set asic pre div
        ratio |= (pre_div << 22) | (1 << 21);
    }
    
    ratio &= ~(0x07 << 6);
    ratio |= (asic_div << 6);

    ratio |= CLOCK_ASIC_ENA;
    
    REG32(CLOCK_DIV_REG) = ratio;
    
    while (REG32(CLOCK_DIV_REG) & CLOCK_ASIC_ENA);

    later_freq = get_asic_freq();
    
    //adjust asic module clock
    if(m_uart_cbk != AK_NULL)
        m_uart_cbk(later_freq);

    //freq high to low, adjust asic module timing later
    if (freq < pre_freq)
    {
        freq_adjust(later_freq);
    }
    DrvModule_UnProtect(DRV_MODULE_FREQ);

    return AK_TRUE;
}

/**
 * @brief    get current asic frequency.
 *
 * @author    liaozhijun
 * @date     2010-04-06
 * @return    T_U32 the frequency of asic running
 */
T_U32 get_asic_freq(T_VOID)
{
    T_U32 ratio;
    T_U8 pre_div = 0, asic_div = 0;
    T_U32 asic_freq;
    T_U32 clk168_freq;

    //get CLK168 freq
    clk168_freq = get_chip_mainclk(get_clk168_value());

    //just return clk168 if bit[31] is set
    if(REG32(CLOCK3X_CTRL_REG) & (1U<<31))
    {
        return clk168_freq;
    }

    //get ASIC_DIV
    ratio = REG32(CLOCK_DIV_REG);


    //get asic div
    asic_div = (ratio >> 6) & 0x07;
    if(asic_div == 0) asic_div = 1;

    //get pre div
    if(ratio & (1<<21)) pre_div = (ratio >> 22) & 0xF;

    if(is_cpu_3x())         //cpu 3X mode
    {
        asic_freq = clk168_freq / 3;
    }
    else                        //not special clock mode
    {
        asic_freq = (clk168_freq >> asic_div) / (pre_div + 1);
    }

    return asic_freq;
}


/**
 * @brief    get current clk168 frequency.
 *
 * @author    liaozhijun
 * @date     2010-04-06
 * @return    T_U32 the frequency of clk168 running
 */
T_U32 get_clk168_value(T_VOID)
{
    T_U32 ratio;
    T_U8 clk168_div = 0;
    T_U32 clk168_freq;

    ratio = REG32(CLOCK_DIV_REG);
    clk168_div = (ratio >> 17) & 0xF;

    clk168_freq = get_real_pll_value() / (clk168_div+1);

    return clk168_freq;
}

/**
 * @brief    get current memory clock.
 *
 * @author    liaozhijun
 * @date     2010-04-06
 * @return    T_U32 the frequency of memory
 */
T_U32 get_mem_freq()
{
    return get_asic_freq();
}

T_U32 get_real_pll_value(T_VOID)
{
    T_U32 ratio;
    T_U32 pll_sel;
    T_U32 ret;
    
    ratio = REG32(CLOCK_DIV_REG);
    pll_sel = (ratio & 0x3f);
    
    ret = pll_sel*5 + PLL_CLK_MIN;

    return (ret);        
}

/**
 * @brief    get PLL register value.
 *
 * main clock is controlled by pll register.
 * @author    liaozhijun
 * @date     2010-04-06
 * @return    T_U32 the frequency of pll(M)
*/
T_U32 get_pll_value(T_VOID)
{
    return get_clk168_value();
}

T_BOOL get_pll_param(T_U32 pll_value, T_U32 *pll_sel, T_U32 *clk168_div)
{
    T_U32 divdiv = 1, pllpll = pll_value;
    T_U32 div;

    //calc div  &  pll_value
    if(pllpll >= PLL_CLK_MIN)
    {
        *clk168_div = divdiv - 1;
        *pll_sel = (pllpll - PLL_CLK_MIN) / 5;
    }
    else
    {
        div = PLL_CLK_MIN / pllpll;
        if((PLL_CLK_MIN % pllpll) != 0) div++;

        *pll_sel = (div*pllpll - PLL_CLK_MIN) / 5;
        *clk168_div = div*divdiv - 1;

        //the max clk168 div is 0xF
        if(*clk168_div > 0xF) return AK_FALSE;
    }
    return AK_TRUE;
}

/**
 * @brief    set PLL register value.
 *
 * main clock is controlled by pll register.
 * @author    liaozhijun
 * @date     2010-04-06
 * @param    pll_value [in] pll register value(12-372)(M)
 * @return    T_BOOL
 * @retval    AK_TRUE  set pll frequency successful
 * @retval    AK_FALSE set pll frequency unsuccessful
  */
T_BOOL set_pll_value(T_U32 pll_value)
{
    volatile T_U32 ratio;
    T_U32 pll_sel = 0, clk168_div = 0;
    T_U32 cur_asic_freq;
    T_BOOL ret;
    T_U32 cur_clk168;
    T_U32 cur_mem_freq, mem_div;
    T_U32 asic_div;

    //check param, set clk168 bounds to (12, 372)
    if (pll_value > PLL_CLK_MODERATE_MAX)
    {
        akprintf(C1, M_DRVSYS, "warning:pll value is no moderate:%d,max is %d\n", pll_value, PLL_CLK_MODERATE_MAX);
        if(pll_value > PLL_CLK_MAX)
        {
            akprintf(C1, M_DRVSYS, "pll value %d is out of range\n", pll_value);
            return AK_FALSE;
        }
    }
        
    cur_clk168 = get_clk168_value();

    akprintf(C3, M_DRVSYS, "set pll value:  %d, cur: %d\n", pll_value, cur_clk168);

    if(cur_clk168 == pll_value)
    {
        akprintf(C1, M_DRVSYS,"current pll is already %d\r\n", pll_value);
        return AK_TRUE;
    }

    //get pll sel and clk168 div
    if(!get_pll_param(pll_value, &pll_sel, &clk168_div))
    {
        akprintf(C1, M_DRVSYS, "cannot set pll %d!!\n", pll_value);
        return AK_FALSE;
    }
    
    //freq low to high, adjust asic module timing first
    if(cur_clk168 < pll_value)
    {
        cur_asic_freq = get_asic_freq();
        asic_div = get_chip_mainclk(cur_clk168) / cur_asic_freq;

        freq_adjust(get_chip_mainclk(pll_value) / asic_div);
    }

    DrvModule_Protect(DRV_MODULE_FREQ);
    
    ratio = REG32(CLOCK_DIV_REG);
    
    ratio &= ~(0x3F << 0);          //ratio clear
    ratio |= (pll_sel << 0);        //set pll div    
    ratio &= ~(0xF << 17);          //clk168 div clr  
    ratio |= (clk168_div << 17);    //set clk168 div    
    ratio |= PLL_CHANGE_ENA;        
    
    REG32(CLOCK_DIV_REG) = ratio;
    while (REG32(CLOCK_DIV_REG) & PLL_CHANGE_ENA);
    
    mini_delay(1); //waiting for clock being stable

    //need to adjust asic module, as asic freqency was also changed
    cur_asic_freq = get_asic_freq();
    cur_mem_freq = get_mem_freq();

    if(m_uart_cbk != AK_NULL)
        m_uart_cbk(cur_asic_freq);
        
    //freq high to low, adjust asic module timing later
    if(cur_clk168 > pll_value)
    {
        freq_adjust(cur_asic_freq);
    }
    
    DrvModule_UnProtect(DRV_MODULE_FREQ);
      
    return AK_TRUE;
}

/**
 * @brief    get cpu frequency.
 *
 * cpu frequency can the same as asic frequency or 2X/3X of that
 * @author    liaozhijun
 * @date     2010-04-06
 * @return    T_U32 the cpu frequency
 */
T_U32 get_cpu_freq(T_VOID)
{
    if (is_cpu_2x() || is_cpu_3x())
    {
        return get_chip_mainclk(get_clk168_value());
    }
    else
    {
        return get_asic_freq();
    }
}

/**
 * @brief       get chip main clock.
 *
 * main clock is controlled by pll register.
 * @author      liaozhijun
 * @date         2010-04-06
 * @param       pll_value [in] pll register value
 * @return      T_U32 the main clock frequency = pll_value * 1000000L 
  */
T_U32 get_chip_mainclk(T_U32 pll_value)
{
    return pll_value*1000000L;
}

//check if cpu runs at 3x asic
T_BOOL is_cpu_3x(void)
{
    return (REG32(CLOCK3X_CTRL_REG) & MARK_3X_CFG) ? (AK_TRUE) : (AK_FALSE);
}


//check if cpu runs at 2x asic
T_BOOL is_cpu_2x(void)
{
    if (AK_TRUE == is_cpu_3x()){
        return AK_FALSE;
    }
    
    return (REG32(CLOCK_DIV_REG) & CPU_2X_CFG) ? (AK_TRUE) : (AK_FALSE);
}


/**
 * @brief       judge whether cpu frequency is twice of asic frequency or not
 *          
 * @author      liaozhijun
 * @date         2010-04-06
 * @return      T_BOOL
 * @retval      AK_TRUE cpu frequency is twice of asic frequency
 * @retval      AK_FALSE cpu frequency is not twice of asic frequency
 */
T_BOOL get_cpu_2x_asic(T_VOID)
{
    return is_cpu_2x();
}

/**
 * @brief       set cpu frequency twice of asic frequency or not
 *
 * this function just set cpu_clk = PLL1_clk or set cpu_clk = asic_clk
 * @author      liaozhijun
 * @date         2010-04-06
 * @param       set_value  set twice or not
 * @return      T_BOOL
 * @retval      AK_TRUE setting successful
 * @retval      AK_FALSE setting fail
 */
T_BOOL set_cpu_2x_asic(T_BOOL set_value)
{
    T_BOOL ret = AK_FALSE;
    T_BOOL runing_at_2x = AK_FALSE;
    T_U32 cur_asic_value, asic_div;

    if (AK_TRUE == is_cpu_3x())
    {
        akprintf(C1, M_DRVSYS, "cpu run at 3x mode, don't set 2x now.\n");
        return AK_FALSE;
    }
        
    runing_at_2x = get_cpu_2x_asic();
    
    DrvModule_Protect(DRV_MODULE_FREQ);

    if (set_value) //set cpu 2x
    {
        if (AK_FALSE == runing_at_2x)
        {
            REG32(CLOCK_DIV_REG) |= CPU_2X_CFG;
        }
        
        cpu_at_2x_counter++;
    }
    else //exit cpu 2x
    {
        if (cpu_at_2x_counter>0)
            cpu_at_2x_counter--;
                        
        if ((AK_TRUE == runing_at_2x) && (cpu_at_2x_counter==0))
        {
            REG32(CLOCK_DIV_REG) &= ~CPU_2X_CFG;
        }
    }
    
    DrvModule_UnProtect(DRV_MODULE_FREQ);

    return AK_TRUE;
}


/**
 * @brief Set cpu clock = pll1 clock = 3 * asic clock = 300MHz
 * Warning:
 *      For AK8802 3ES, and only cpu core voltage higher 1.3V use only.
 *      Before entry 3x config, will store old pll1 value and when exit 3x cfg,
 *      it will restore old pll1 value.
 *
 * @author      liaozhijun
 * @date         2010-04-06
 * @param       pll_value: [in]  pll frequency
 * @param       set_value: AK_TRUE is to enable cpu3x
 * @return      T_BOOL
 * @retval      AK_TRUE: cpu frequency three of asic frequency
 * @retval      AK_FALSE: when cpu at 2x or 3x ready.
 */
T_BOOL set_cpu_3x_asic(T_U32 pll_value, T_BOOL set_value)
{
    T_U32 cur_asic_freq;
    T_U32 prev_mem_freq, cur_mem_freq;
    T_BOOL ret;
    T_U32 pll_sel = 0, clk168_div = 0;
    volatile T_U32 ratio;
    
    //check param, set max CPU freq to 300M
    if(set_value && (pll_value > PLL_CLK_MAX))
    {
        akprintf(C1, M_DRVSYS, "cpu freq %d is beyond the chip max value\n", pll_value);
        return AK_FALSE;
    }

    //exit cpu 3x mode
    if (set_value == AK_FALSE)
    {
        DrvModule_Protect(DRV_MODULE_FREQ);
        ret = exit_cpu_3x_asic();
        DrvModule_UnProtect(DRV_MODULE_FREQ);

        return ret;
    }
    
    if(is_cpu_3x() || is_cpu_2x())
    {
        return AK_FALSE;
    }

    ret = get_pll_param(pll_value, &pll_sel, &clk168_div);
    if (AK_FALSE == ret)
    {
        akprintf(C1, M_DRVSYS, "cannot set pll %d!!\n", pll_value);

        return AK_FALSE;
    }
        
    store_pll();

    prev_mem_freq = get_mem_freq();
    if(prev_mem_freq < get_chip_mainclk(pll_value)/3)
    {
        freq_adjust(get_chip_mainclk(pll_value)/3);
    }
    
    DrvModule_Protect(DRV_MODULE_FREQ);
    
    //step 1. enable cpu 3x
    REG32(CLOCK3X_CTRL_REG) |= CPU_3X_ENABLE;

    //step 2. set cpu 3x cfg, this step cannot combined with step1
    REG32(CLOCK3X_CTRL_REG) |= MARK_3X_CFG;

    ratio = REG32(CLOCK_DIV_REG);
    
    ratio &= ~(0x3F << 0);   // ratio clear
    ratio |= (pll_sel << 0);   //set pll div    
    ratio &= ~(0xF << 17);
    ratio |= (clk168_div << 17);
    ratio |= PLL_CHANGE_ENA;

    //change pll
    //store_pclk_state();

    REG32(CLOCK_DIV_REG) = ratio;
    while (REG32(CLOCK_DIV_REG) & PLL_CHANGE_ENA);

    //restore_pclk_state();
                
    //adjust asic module clock
    mini_delay(1);

    cur_asic_freq = get_asic_freq();
    cur_mem_freq = get_mem_freq();
    
    if(prev_mem_freq > cur_mem_freq)
    {
        freq_adjust(cur_mem_freq);
    }

    if(m_uart_cbk != AK_NULL)
        m_uart_cbk(cur_asic_freq);

    DrvModule_UnProtect(DRV_MODULE_FREQ);

    return AK_TRUE;
}


/**
 * @brief        exit cpu 3x mode
 *          
 * @author      liaozhijun
 * @date         2010-04-06
 * @return      T_BOOL
 * @retval      AK_TRUE success to exit cpu 3x mode
 * @retval      AK_FALSE fail to exit cpu 3x mode
 */
T_BOOL exit_cpu_3x_asic(void)
{
    T_U32 cur_asic_freq;
    T_U32 prev_mem_freq, cur_mem_freq;
    T_U32 clk168_freq, mem_div;
    T_U32 ratio, i;
    
    if (AK_FALSE == is_cpu_3x())
    {
        return AK_FALSE;
    }

    prev_mem_freq = get_mem_freq();

    mem_div = (REG32(CLOCK_DIV_REG) >> 9) & 0x07;
    if(mem_div == 0)
        mem_div = 1;

    clk168_freq = (4*pll_param.pll_sel + 180) / (1 + pll_param.clk168_div);
    cur_mem_freq = get_chip_mainclk(clk168_freq) >> mem_div;

    if(prev_mem_freq < cur_mem_freq)
    {
        freq_adjust(cur_mem_freq);
    }
    

    //step1 clr cpu 3x cfg
    REG32(CLOCK3X_CTRL_REG) &= ~MARK_3X_CFG;

    //step2 wait at least 384 clock cycles
    for(i = 0; i < 100; i++);

    //step3 disable cpu 3x
    REG32(CLOCK3X_CTRL_REG) &= ~CPU_3X_ENABLE;

    ratio = REG32(CLOCK_DIV_REG);

    ratio &= ~(0x3F << 0);   // ratio clear
    ratio |= (pll_param.pll_sel << 0);   //set pll div    
    ratio &= ~(0xF << 17);   //set clk168 = pll
    ratio |= (pll_param.clk168_div << 17);   //set clk168 = pll
    ratio |= PLL_CHANGE_ENA;
    
    //change pll clock
    //store_pclk_state();
    
    REG32(CLOCK_DIV_REG) = ratio;
    while (REG32(CLOCK_DIV_REG) & PLL_CHANGE_ENA);
    
    //restore_pclk_state();

    //adjust asic module clock
    mini_delay(1);

    cur_asic_freq = get_asic_freq();
    cur_mem_freq = get_mem_freq();
    
    if(prev_mem_freq > cur_mem_freq)
    {
        freq_adjust(cur_mem_freq);
    }

    if(m_uart_cbk != AK_NULL)
        m_uart_cbk(cur_asic_freq);
        
    //nand_changetiming(cur_asic_freq);
    mini_delay(1);

    return AK_TRUE;
}


// store current pll param
void store_pll(void)
{
    pll_param.pll_sel = (T_U8)(REG32(CLOCK_DIV_REG) & 0x3f);
    pll_param.clk168_div = (T_U8)((REG32(CLOCK_DIV_REG) >> 17) & 0x0f);
}

//recover previous pll param
void restore_pll(T_U32 *reg)
{
    *reg &= ~(0x3F << 0); // M clear
    *reg |= pll_param.pll_sel;
    *reg &= ~(0xf<<17);//n = clear
    *reg |= ((pll_param.clk168_div) << 17);
}

/**
 * @brief        register call back function for freq change
 *          
 * @author      liaozhijun
 * @date         2010-04-06
 * @param      type callback function type, may be memory/uart/nand/sd/spi, and so on
 * @param      callback callback function 
 * @return      T_VOID
 */
T_VOID freq_register(E_FREQ_CALLBACK_TYPE type, T_fFREQ_CHANGE_CALLBACK callback)
{
    if(E_MEMORY_CALLBACK == type)
    {
        m_mem_cbk = callback;
    }
    else if(E_UART_CALLBACK == type)
    {
        m_uart_cbk = callback;
    }
    else
    {
        if(index < MAX_FREQ_CALLBACK)
            m_asic_cbk[index++] = callback;
    }
}
