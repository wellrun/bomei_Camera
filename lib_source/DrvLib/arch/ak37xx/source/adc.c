/**
 * @FILENAME: adc.c
 * @BRIEF adc driver file
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @DATE 2010-07-30
 * @VERSION 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "l2.h"
#include "adc.h"
#include "dac.h"
#include "interrupt.h"
#include "hal_sound.h"
#include "sysctl.h"

static void adc_getdiv_osr(T_U8 *mode_sel, T_U8 *save_div, T_U32 sr)
{
    T_U16 k, max_div;
    T_U16 OSR_value=256;
    T_U32 SR_save, out_sr=0;
    T_S32 a, b;
    T_U32 pllclk = get_pll_value() * 1000 * 1000;    

    max_div = 0x100;
    SR_save = 0;
    *mode_sel = 0;
    *save_div = 0;

    SR_save = 0;
    
    if (sr > 24000)
    {
        OSR_value = 256;
        *mode_sel = 1; //48k mode
    }
    else
    {
        OSR_value = 512;
        *mode_sel = 0; //16k mode
    }
    
    for(k=0; k<max_div; k++) //DIV
    {
        out_sr = pllclk*1.0/(k+1)/OSR_value;
        a = out_sr - sr;
        a = (a>0)? a : (-a);
        b = SR_save - sr;
        b = (b>0)? b : (-b);
        if (a<b)
        {
            SR_save = out_sr;
            *save_div = k;
        }
    }
}

/**
 * @brief  open a adc device 
 * @author LianGenhui
 * @date   2010-07-28
 * @return T_BOOL
 * @retval AK_TRUE  open successful
 * @retval AK_FALSE open failed
 */
T_BOOL adc_open(T_VOID)
{
    T_U32 reg_value;
    T_U32 i;
    
    //reset ADC2 and ADC3
    REG32(ADC_CLK_DIV) &= ~(ADC2_RST);
    REG32(ADC_CLK_DIV) |= (ADC2_RST);

    // soft reset ADC23
    sysctl_reset(RESET_ADC2);

    sysctl_clock(CLOCK_ADC2_ENABLE); //enable ADC23 Clock 
    
    //REG32(MUL_FUN_CTL_REG) |= IN_DAAD_EN;   //enable internal   no this function

    //disable vcm2 discharge
    REG32(ANALOG_CTRL_REG3) &= ~(0x1f << 4);

    // SelVcm3
    REG32(ANALOG_CTRL_REG3) |= VCM3_SEL;

    REG32(ANALOG_CTRL_REG4) &= ~(0x1f << 16); //vcm2 charge 200ua

    //PowerOnVcm2/3
    REG32(ANALOG_CTRL_REG3) &= ~PD_VCM3;
    REG32(ANALOG_CTRL_REG3) &= ~PD_VCM2;

    //SetVcmNormal
    REG32(ANALOG_CTRL_REG3) &= ~PL_VCM2;
    REG32(ANALOG_CTRL_REG4) &= ~PL_VCM3;

    //EnableAdc2Limit
    REG32(ANALOG_CTRL_REG4) |= ADC_LIM;

    REG32(ADC2_MODE_CFG) |= ADC2_CTRL_EN;    //enable adc controller
    REG32(ADC2_MODE_CFG) |= L2_EN;    //enable l2
    REG32(ADC2_MODE_CFG) &= ~HOST_RD_INT_EN;
    REG32(ADC2_MODE_CFG) |= CH_POLARITY_SEL;    //Receive the left channel data when the lrclk is high
    //REG32(ADC2_MODE_CFG) &= ~I2S_EN;        //Internal ADC MODE  no this function
    REG32(ADC2_MODE_CFG) &= ~WORD_LENGTH_MASK;
    REG32(ADC2_MODE_CFG) |= (0xF << 8);    //WORD LENGTH IS 16 BIT

    //PowerOnAdc2Conversion
    REG32(ANALOG_CTRL_REG4) &= ~PD_S2D;

    //EnableAdc23Clk
    REG32(ADC_CLK_DIV) |= ADC2_CLK_EN;

    //ProvideAdc23Clk
    REG32(ADC_CLK_DIV) &= ~ADC2_GATE;

    //PowerOnAdc2
    REG32(ANALOG_CTRL_REG4) &= ~PD_ADC2;
    
    REG32(ANALOG_CTRL_REG4) |= VCM3OUT_SEL;  //VCM3 and AVCC must disconnect, if not ,audio will be bad.

    for(i = 0 ; i < 1000; i++);

    //PowerOnCodec
    REG32(ANALOG_CTRL_REG3) &= ~PD_BIAS; 

    return AK_TRUE;
}

/**
 * @brief   Close a adc device
 * @author  LianGenhui
 * @date    2010-07-28
 * @return  T_BOOL
 * @retval  AK_TRUE close successful
 * @retval  AK_FALSE close failed
 */
T_BOOL adc_close(T_VOID)
{
    T_U32 reg_value;
    
    REG32(ADC2_MODE_CFG) &= ~L2_EN;    //disable l2
    //disable ADC2 clk
    REG32(ADC_CLK_DIV) &= ~ADC2_CLK_EN;     

    //disable ADC2 interface
    REG32(ADC2_MODE_CFG) &= ~ADC2_CTRL_EN; 

    //Power off adc2
    REG32(ANALOG_CTRL_REG4) |= PD_ADC2;

    if (LINE_IN != (REG32(ANALOG_CTRL_REG3)&(2<<12)))
    {
        //power off vcm and codec if dac not use
        if(!(REG32(ADC_CLK_DIV) & DAC_CLK_EN))
        {
            reg_value = REG32(ANALOG_CTRL_REG3);
            reg_value |= PD_VCM3;                  //power off vcm3   
            reg_value |= PD_VCM2;                  //power off vcm2  
            reg_value |= PL_VCM2;                  //power off vcm2 
            reg_value |= PD_BIAS;                   //power off codec
            REG32(ANALOG_CTRL_REG3) = reg_value;
        }
        
        akprintf(C3, M_DRVSYS,"poweroff vcm2\n");
        sysctl_clock(~CLOCK_ADC2_ENABLE); //disable ADC23 Clock 
        
    }
    return AK_TRUE;
}

/**
 * @brief   Set adc sample rate, channel, bits per sample of the sound device
 * @author  LianGenhui
 * @date    2010-07-28
 * @param[in] info     refer to SOUND_INFO
 * @return  T_BOOL
 * @retval  AK_TRUE set successful
 * @retval  AK_FALSE set failed
 */
T_BOOL adc_setinfo(SOUND_INFO *info)
{
    T_U8 mode_sel;
    T_U8 save_div;
    T_U32 reg_value;

    adc_getdiv_osr(&mode_sel, &save_div, info->nSampleRate);

    REG32(ADC_CLK_DIV) &= ~(0xff << ADC2_DIV);
    REG32(ADC_CLK_DIV) |= ((save_div&0xff) << ADC2_DIV);
    
    REG32(ANALOG_CTRL_REG2) &= ~(1 << ADC_OSR);
    REG32(ANALOG_CTRL_REG2) |= (mode_sel << ADC_OSR);
    
    return AK_TRUE;
}




