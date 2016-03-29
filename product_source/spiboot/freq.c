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
#include "l2.h"
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
    T_U32 pll_sel;
    T_U32 asic_freq;
    T_U32 clk168_freq;
    T_U8 pre_div = 0, asic_div = 0;
    T_U8 clk168_div = 0;


    ratio = REG32(CLOCK_DIV_REG);
    clk168_div = (ratio >> 17) & 0xF;

    ratio = REG32(CLOCK_DIV_REG);
    pll_sel = (ratio & 0x3f);

    clk168_freq = ((pll_sel*5 + PLL_CLK_MIN) / (clk168_div+1))*1000000L;


    //just return clk168 if bit[31] is set
    if(REG32(CLOCK3X_CTRL_REG) & (1<<31))
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
    
    asic_freq = (clk168_freq >> asic_div) / (pre_div + 1);

    return asic_freq;
}

