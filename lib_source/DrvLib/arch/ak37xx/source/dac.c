/**
 * @file dac.c
 * @brief the source code of DA controller
 * This file is the source code of DA controller
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LianGenhui
 * @date 2010-07-30
 * @version 1.0
 */

#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "hal_sound.h"
#include "dac.h"
#include "sysctl.h"

#define MAX_DACCLK 14000000

#define DAC_HIGH_CLK (60*1000*1000)

#define DAC_SD_NUMER_DENOM                              (ADC_MODULE_BASE_ADDR + 0x150)
#define DAC_SD_OFFSET                                   (ADC_MODULE_BASE_ADDR + 0x154)


//#define DAC_USE_OLD_CLK_DIV

extern T_U32 get_real_pll_value(T_VOID);

/**
 * @brief   Get OSR and DACDIV refer to appointed PLL and sample rate
 * @author  LianGenhui
 * @date    2009-07-30
 * @input   des_sr: destination sample rate
 * @output  osrindex: OSR index
 * @output  mclkdiv: mclk div
 * @return  T_VOID
 */
#ifdef  DAC_USE_OLD_CLK_DIV  
static T_VOID DAC_GetOsrDiv(T_U8 *osrindex, T_U8 *mclkdiv, T_U32 des_sr)
{
    const T_U16 OSR_table[8] = 
        {256, 272, 264, 248, 240, 136, 128, 120};
   
    T_U32 j;
    T_U32 max_div;
    T_S32 k;
    T_U32 SR_save, out_sr=0;
    T_S32 a;
    T_S32 b;
    T_U32 pllclk;
    T_U32 step;
    

    //04V 3771 chip: when the divider of the DAC CLK is even number,
    //the DAC MODULE is more steady
    if(drv_get_chip_version() == CHIP_3771_02SH)
    {
        step = 1 ;
    }
    else
    {
        step = 2;
    }

    pllclk = get_pll_value();
    pllclk *= 1000000;   
      
    max_div = 0x100;
    SR_save = 0;
    *osrindex = 0;
    *mclkdiv = 0xff;
    for(j=0; j<8; j++) //OSR index
    //for(j=0; j<8; j+=6) //OSR index = 0 or 6, ½â¾öºáÎÆÔëÉù
    {
        for(k=max_div-1; k>=0; k = k - step) //DAC_DIV value
        {
            out_sr = pllclk/(k+1);
            if (out_sr > MAX_DACCLK)
                break;
            out_sr = out_sr/OSR_table[j];
            a = out_sr-des_sr;
            a = (a>0)? a : (-a);
            b = SR_save-des_sr;
            b = (b>0)? b : (-b);
            if (a<b)
            {
                SR_save = out_sr;
                *mclkdiv = k;
                *osrindex = j;
            }
        }
    }

    akprintf(C3, M_DRVSYS, "DAC_GetOsrDiv sr=%d, ret_sr=%d\r\n",des_sr, SR_save);

}
#endif
/**
 * @brief  open a dac device 
 * @author LianGenhui
 * @date   2010-07-30
 * @return T_BOOL
 * @retval AK_TRUE  open successful
 * @retval AK_FALSE open failed
 */
T_BOOL dac_open(T_VOID)
{
	T_U32 reg_value;    

    REG32(DAC_CONFIG_REG) |= MUTE;

    // soft reset DAC
    sysctl_reset(RESET_DAC);

    //enable DAC clock
    sysctl_clock(CLOCK_DAC_ENABLE);
    
    // reset DAC
    REG32(ADC_CLK_DIV) &= (~DAC_RST);
    // must set to 1 for quit DAC reset mode.
    REG32(ADC_CLK_DIV) |= (DAC_RST);

    // to enable internal DAC/ADC VIA I2S
    REG32(MUL_FUN_CTL_REG) |= IN_DAAD_EN;  

    //set default dac div and osr, for de pi-pa noise
    reg_value = REG32(ADC_CLK_DIV);
    reg_value &= ~(0xff << 13);    
    reg_value |= (31 << 13);
    REG32(ADC_CLK_DIV) = reg_value;
    
    reg_value = REG32(ANALOG_CTRL_REG2);    
    reg_value |= (0x7 << 14);
    REG32(ANALOG_CTRL_REG2) = reg_value; 

    // to enable DAC CLK
    REG32(ADC_CLK_DIV) |= DAC_CLK_EN;

    //to provide DAC CLK for DAC
    REG32(ADC_CLK_DIV) &= (~DAC_GATE);

    sysctl_reset(RESET_DAC);
    //reset dacs
    REG32(ADC_CLK_DIV) &= ~DAC_RST;
    REG32(ADC_CLK_DIV) |= DAC_RST;
    
    // to enable DACs
    REG32(ANALOG_CTRL_REG2) |= DAC_EN;  

    REG32(I2S_CONFIG_REG) &= (~I2S_CONFIG_WORDLENGTH_MASK);
    //REG32(I2S_CONFIG_REG) |= 0XF;  //16 bit   for  memory saving mode.

    REG32(DAC_CONFIG_REG) |= DAC_CTRL_EN;

    //enable accept data from l2
    reg_value = REG32(DAC_CONFIG_REG);
    reg_value |= L2_EN;
    reg_value |= FORMAT;        //Memory saving format
    reg_value &=~ARM_INT;      
    reg_value &= ~MUTE;
    REG32(DAC_CONFIG_REG) = reg_value;

    //Send data when left channel data shen data lrclk is high 
    // 02es and 04es channel is reciprocative
    if(CHIP_3771_02SH != drv_get_chip_version()) 
    {
        REG32(I2S_CONFIG_REG) &= ~POLARITY_SEL; 
    }
    else
    {
        REG32(I2S_CONFIG_REG) |= POLARITY_SEL; 
    }

	return AK_TRUE;
}

/**
 * @brief   Close a dac device
 * @author  LianGenhui
 * @date    2010-07-30
 * @return  T_BOOL
 * @retval  AK_TRUE close successful
 * @retval  AK_FALSE close failed
 */
T_BOOL dac_close(T_VOID)
{
    T_U32 reg_value;
    
    REG32(DAC_CONFIG_REG) &= (~L2_EN);

    REG32(ANALOG_CTRL_REG3) &= ~(0x1F << HP_GAIN);
    
    // to power off DACs/DAC clock, 
    REG32(ANALOG_CTRL_REG3) |= PD_OP | PD_CK;        
    REG32(ANALOG_CTRL_REG3) |= RST_DAC_MID;  

    // to disable internal DAC/ADC
    REG32(MUL_FUN_CTL_REG) &= (~(IN_DAAD_EN));
    
    // to disable DAC CLK
    REG32(ADC_CLK_DIV) &= (~DAC_CLK_EN);
    
    //to inhibit DAC CLK for DAC
    REG32(ADC_CLK_DIV) |= DAC_GATE;
    
    // to disable DACs
    REG32(ANALOG_CTRL_REG2) &= ~DAC_EN;

    return AK_TRUE;
}

#ifdef  DAC_USE_OLD_CLK_DIV  

static T_VOID DAC_SetHighSpeed(T_U32 sample_rate)
{
    static T_U32 rate_list[] = {8000, 16000, 24000, 48000, 96000};
    static T_U32 dac_hclk[sizeof(rate_list)/sizeof(rate_list[0])] = 
        {6, 12, 18, 36, 72};

    T_U32        reg_value;
    T_U32        pllclk;
    T_U32        hclk_div;
    T_U32        hclk;
    T_U32        i;


    if(drv_get_chip_version() == CHIP_3771_02SH)
    {
        return ;
    }    

    pllclk = get_real_pll_value();

    for(i=0; i < sizeof(rate_list)/sizeof(rate_list[0]); ++i)
    {
        if(sample_rate <= rate_list[i])
        {
            break;
        }
    } 
    
    if(sizeof(rate_list)/sizeof(rate_list[0]) == i)
    {
        hclk = dac_hclk[i-1];
    }
    else
    {
        hclk = dac_hclk[i];
    }

    hclk_div = pllclk / hclk;  

    //04V 3771 chip: when the divider of the DAC CLK is even number,
    //the DAC MODULE is more steady
    hclk_div &= ~1;

    if(0 != hclk_div)
    {
        hclk_div = hclk_div - 1;
    } 

    akprintf(C3, M_DRVSYS, "pll = %d M, hclk_div = %d\n", pllclk, hclk_div);

#else
static T_VOID DAC_SetHighSpeed(T_U32 sample_rate)
{
    T_U32        reg_value;
    T_U32        pllclk;
    T_U32        hclk_div;
    T_U32        hclk;
    T_U32        i;

    pllclk = get_real_pll_value();
    pllclk = pllclk * 1000000;

    //hclk_div = pllclk/(2*DAC_HIGH_CLK);
	hclk_div = pllclk/(1000*sample_rate);
    if (hclk_div >= 1)
    {
        hclk_div = hclk_div -1;
    }

    i = 0;
    while(REG32(DAC_HS_CLK_REG) & (1<<8))
    {
        ++i;
        if(i > 100000)
        {
            akprintf(C2, M_DRVSYS, "set high speed clk reg fail\n");
            return ;
        }
    }    

#endif
    reg_value = REG32(DAC_HS_CLK_REG);
    reg_value &= ~(0xFF);
    reg_value |= DAC_HCLK_SEL_DIV | DAC_HCLK_VAL | hclk_div;
	REG32(DAC_HS_CLK_REG) = reg_value;
    
    i = 0;
    while(REG32(DAC_HS_CLK_REG) & (1<<8))
    {
        ++i;
        if(i > 100000)
        {
            akprintf(C2, M_DRVSYS, "set high speed clk reg fail\n");
            return ;
        }
    }    
    
}

T_U32 get_max_comdiv(T_U32 large, T_U32 small)
{
    T_U32 diff, div;

    if(small == large)
        return small;

    while(1)
    {
        div = large/small;
        diff = large - small*div;

        if(0 == diff)
        {
            return small;
        }
        else
        {
            large = small;
            small = diff;
        }
    }

    return 1;
}

T_U32 get_frac_div(T_U32 aclk, T_U32 dac_adc_clk, T_U32 *p_denom, T_U32 *p_nume)
{
    T_U32 comdiv, tmp;
    T_U32 denominator, numerator;
    
    comdiv = get_max_comdiv(aclk, dac_adc_clk);

    numerator = dac_adc_clk / comdiv;
    denominator = aclk / comdiv;

    if(denominator & 0x1)
    {
        denominator <<= 1;
        numerator <<= 1;
    }

    if(denominator < 0xFFFF)
    {
        //we got the best frac div
        *p_denom = denominator;
        *p_nume = numerator;
        
        return 0;
    }
    else 
    {
        //calc a nearest value
        tmp = denominator/0x10000;

        numerator = numerator/(tmp+1);
        denominator = denominator /(tmp+1);

        if(denominator &0x1)
        {
            numerator--;
            denominator--;
        }

        tmp = (aclk/denominator)*numerator;
        if(tmp > dac_adc_clk)
        {
            return tmp - dac_adc_clk;
        }
        else
        {
            return dac_adc_clk - tmp;
        }
    }
}


static T_U32 get_dac_clk(T_U32 sample_rate, T_U32 *p_denom, T_U32 *p_nume, T_U32 *p_osr_index, T_U32 *p_osr_val)
{    
    const T_U16 OSR_tbl[] =  {256, 272, 264, 248, 240, 136, 128, 120};
    T_U32 osr_num = sizeof(OSR_tbl)/sizeof(OSR_tbl[0]);
    T_U32 osr_value, osr_index;

    T_U32 i;
    T_U32 aclk, dac_clk;

    T_U32 denom, nume;
    T_U32 cur_diff, min_diff;
    T_U32 min_denom, min_nume;

    T_U32 sr;

    aclk = get_real_pll_value();
    aclk = aclk*1000000;

    min_diff = 0xFFFFFFFF;
    osr_value = OSR_tbl[0];

    for(i = 0; i < osr_num; i++)
    {
        dac_clk = OSR_tbl[i] * sample_rate;
        cur_diff = get_frac_div(aclk, dac_clk, &denom, &nume);

        if(cur_diff < min_diff)
        {
            min_diff = cur_diff;
            
            min_denom = denom;
            min_nume = nume;

            osr_index = i;
            osr_value = OSR_tbl[i];
        }

        if(0 == min_diff)
            break;
    }

    *p_denom = denom;
    *p_nume = nume;
    *p_osr_index = osr_index;
    *p_osr_val = osr_value;

    sr = ( (aclk/denom)*nume ) / osr_value;
	
 
    return osr_value;
}

static T_VOID set_dac_frac_reg(T_U32 denominator, T_U32 numerator)
{
    T_U32 reg;
    T_U32 offset;
    T_U32 to = 0;
    
    #define DAC_DIV_REQ  (1 << 17)
    #define DAC_DIV_EN   (1 << 16) 
    
    offset = (0x10000 - denominator) / 2;
   

    //close old dac clock
    REG32(ADC_CLK_DIV) &= ~(1<<26);

    //disable frac div
    REG32(DAC_SD_OFFSET) &= ~DAC_DIV_EN;


    //set to use frac div
    REG32(DAC_SD_OFFSET) |= (1U <<31);

    //set denomiator, numerator
    REG32(DAC_SD_NUMER_DENOM) = ((denominator) << 16) | (numerator);
    //REG32(DAC_SD_NUMER_DENOM) = (numerator << 16) | denominator;

    //set offset
    reg = REG32(DAC_SD_OFFSET);
    reg &= ~0xFFFF;
    reg |= offset;
    REG32(DAC_SD_OFFSET) = reg;


    //wait until bit[17] is 0
    to = 0;
    while( (REG32(DAC_SD_OFFSET) & DAC_DIV_REQ) )
    {
        if(to++ > 1000000)
        {
            while(1);
        }
    }


    //set div change request
    REG32(DAC_SD_OFFSET) |= DAC_DIV_REQ;

    //wait until bit[17] is 0
    to = 0;
    while( (REG32(DAC_SD_OFFSET) & DAC_DIV_REQ) )
    {
        if(to++ > 1000000)
        {
            while(1);
        }
    }

    //enable frac div
    REG32(DAC_SD_OFFSET) |= DAC_DIV_EN;
}


/**
 * @brief   Set sound sample rate, channel, bits per sample of the sound device
 * @author  LianGenhui
 * @date    2010-07-30
 * @param[in] info     refer to SOUND_INFO
 * @return  T_BOOL
 * @retval  AK_TRUE set successful
 * @retval  AK_FALSE set failed
 */
T_BOOL dac_setinfo(SOUND_INFO *info)
{
    T_U32   temp_sr;
    T_U32   reg_value;
    #ifdef  DAC_USE_OLD_CLK_DIV 
    T_U8    osr, mclkdiv; 
    #else
    T_U32   osr, index;
    T_U32   denom, nume;
    #endif

    
    if(AK_NULL == info)
    {
        akprintf(C2, M_DRVSYS, " DAC_SetInfo : info is null\n");
        return AK_FALSE;
    }

    REG32(ANALOG_CTRL_REG3) |= RST_DAC_MID;  
    REG32(ANALOG_CTRL_REG3) &= ~RST_DAC_MID;  

#ifdef  DAC_USE_OLD_CLK_DIV            
    DAC_GetOsrDiv(&osr, &mclkdiv, info->nSampleRate);
    akprintf(C2, M_DRVSYS, "dac osr is %d, mclkdiv is %d\n", osr, mclkdiv);

    temp_sr = REG32(ANALOG_CTRL_REG2);
    temp_sr &= (ANALOG_CTRL2_OSR_MASK);
    if(temp_sr == (ANALOG_CTRL2_OSR(osr)))
    {
        temp_sr = REG32(ADC_CLK_DIV);
        temp_sr &= (MASK_CLKDIV2_DAC_DIV);
        if(temp_sr == (CLKDIV2_DAC_DIV(mclkdiv)))
        {
            return AK_TRUE;
        }
    }

    //disable HCLK, and disable DAC interface
    REG32(DAC_HS_CLK_REG) |= DAC_HCLK_DIS;
    REG32(DAC_CONFIG_REG) &= ~(DAC_CTRL_EN);
    
    DAC_SetHighSpeed(info->nSampleRate);
#else
    get_dac_clk(info->nSampleRate, &denom, &nume, &index, &osr);


    REG32(DAC_HS_CLK_REG) |= DAC_HCLK_DIS;
    REG32(DAC_CONFIG_REG) &= ~(DAC_CTRL_EN);

    //set high clock
    DAC_SetHighSpeed(info->nSampleRate);

    //set dac clock
    set_dac_frac_reg(denom, nume);


#endif

    //set OSR
    reg_value  = REG32(ANALOG_CTRL_REG2);
    reg_value &= (~ANALOG_CTRL2_OSR_MASK);
    reg_value |= (ANALOG_CTRL2_OSR(osr));
    REG32(ANALOG_CTRL_REG2) = reg_value;


#ifdef  DAC_USE_OLD_CLK_DIV
    //set DIV
    reg_value   = REG32(ADC_CLK_DIV);
    reg_value  &= (~(MASK_CLKDIV2_DAC_DIV));    
    reg_value  |= (CLKDIV2_DAC_DIV(mclkdiv));    
    REG32(ADC_CLK_DIV) = reg_value;   
#endif
    //to reset dac
    //REG32(ADC_CLK_DIV) |= (DAC_RST);

    //reset dacs
    REG32(ADC_CLK_DIV) &= ~DAC_RST;
    REG32(ADC_CLK_DIV) |= DAC_RST;

    //enable HCLK, and enable DAC interface
    REG32(DAC_HS_CLK_REG) &= ~DAC_HCLK_DIS;
    REG32(DAC_CONFIG_REG) |= (DAC_CTRL_EN);
    
    return AK_TRUE;
}


