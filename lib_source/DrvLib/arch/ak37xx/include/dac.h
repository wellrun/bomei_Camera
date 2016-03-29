/**
 * @file dac.h
 * @brief the register bit definition of DA controller
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @date 2008-05-30
 * @version 1.0
 */

#ifndef __DAC_H__
#define __DAC_H__

#include "adc.h"

//ANALOG_CONTROL_REG1
#if 0
#define MUXDA_R        (1UL << 31)
#define MUXDA_L        (1 << 30)
#define PD_MICP        (1 << 22)
#define PD_MICN        (1 << 21)
#define PD2_HP         (1 << 20)
#define PD1_HP         (1 << 19)
#define PRE_EN1        (1 << 18)
#define PRE_EN2        (1 << 17)
#define TRIMP_HP       (1 << 16)
#define TRIMN_HP       (1 << 15)
#endif

//ANALOG_CONTROL_REG3

#define Dischg_HP      (29)

#define PD_HP_CTRL     (25)

#define VCM3_SEL       (1 << 23)

#define PRE_EN1        (1 << 17)
#define PRE_EN2        (1 << 16)
#define PD_HP          (1 << 15) 
#define PD2_HP         (1 << 24) 
#define HP_GAIN        (18)
#define HP_IN          (12)

#define RST_DAC_MID    (1 << 11)
#define PD_OP          (1 << 10)
#define PD_CK          (1 << 9)

#define PTM_DCHG       (4)
#define PD_VCM3        (1 << 3)
#define PL_VCM2        (1 << 2)  //pull down to ground.
#define PD_VCM2        (1 << 1)  // power off 
#define PD_BIAS        (1 << 0)  // power off the bias generator

#if 0
#define PTM_D_CHG_EN   (1 << 5)
#define VREF_CHG_FAST  (1 << 4)
#define PD_REF         (1 << 0)
#endif

#define PD_HP_CTRL_MASK (0XF << 23)
#define DISCHG_HP_MASK  (0XF << 23)
#define PTM_D_CHG_MASK  (0XF << 6)


/*  I2S Configuration Register  bit define  0x2002E004  */
#define I2S_CONFIG_WORDLENGTH_MASK (0x1F << 0)
#define POLARITY_SEL    (1 << 5)  
#define LR_CLK          (1 << 6)


// Data configuration Register  0x2002E000 bit define.
#define ARM_INT        (1 << 3)  //ARM interrupt enable
#define MUTE           (1 << 2)  // repeat to sent the Last data to DAC
#define FORMAT         (1 << 4)    //  1 is used memeory saving format.
#define L2_EN          (1 << 1)
#define DAC_CTRL_EN    (1 << 0)

#define DAC_RST         (1 << 24)
#define AUDIO_EN        (1 << 10)
#define DAC_CLK_EN      (1 << 21)
#define DAC_GATE        (1 << 26)
#define DAC_EN          (1 << 13)

#define IN_DAAD_EN      (1 << 23) //ENABLE INTERNAL DAC ADC via i2s

//DAC High Speed Clock Configuration Register(Address: 0x0800, 0174)
#define DAC_HCLK_DIV    (0)
#define DAC_HCLK_VAL    (1 << 8)
#define DAC_HCLK_DIS    (1 << 9)
#define DAC_HCLK_SEL_DIV (1 << 10)


// Power  control function 
//CLK Control and  soft RST control register 0x0800000C
#define L2_CLK_CTRL_EN   (1 << 3)
#define PCM_CLK_CTRL_EN  (1 << 2)
#define DAC_CLK_CTRL_EN  (1 << 1)
#define ADC2_CLK_CTRL_EN (1 << 0)


#define ANALOG_CTRL2_OSR_MASK                   (0x7 << 14)                                // for DACs
#define ANALOG_CTRL2_OSR(value)                 (((value)&0x7) << 14)      // for DACs

#define MASK_CLKDIV2_DAC_DIV                    (0xFF << 13)

/** clock divider reigister 2  bit map*/
#define CLKDIV2_DAC_DIV(val)                    (((val)&0xFF) << 13)    //[20:13]dac clock = PLLCLK/(CLK_DIV2_DAC_DIV+1)

#define I2S_CONFIG_WORDLENGTH_MASK              (0x1F << 0)


#endif //__DAC_H__

