/**
 * @file pmu.c
 * @brief power manage unit soucr file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */

#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "arch_pmu.h"

#define DCDC12_ENABLE_BIT           (1<<25)
#define LDO12_ENABLE_BIT            (1<<16)
#define USB_DET_N_CTRL              (1<<24)
#define LDO12_SEL_BIT               (1<<15)

#define LDOPLL_CTRL_BIT             (1<<23)

/**
 * @brief set output voltage of ldo33 
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param vol voltage to be set
 * @return T_VOID
 */
T_VOID pmu_set_ldo33(E_LDO33_VOL vol)
{
    T_U32 reg;

    if(vol == LDO33_RESERVER)
        return;

    irq_mask();

    REG32(PMU_CTRL_REG) &= ~(0x7 << 17);
    REG32(PMU_CTRL_REG) |= (vol << 17);

    irq_unmask();
}

/**
 * @brief set output voltage of ldo12
              dcdc12 will be disabled according to chip spec
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param vol voltage to be set
 * @return T_VOID
 */
T_VOID pmu_set_ldo12(E_LDO12_VOL vol)
{
    T_U32 reg;

    irq_mask();

    //enable ldo12
    REG32(PMU_CTRL_REG) |= LDO12_ENABLE_BIT;

    //disable dcdc12
    REG32(PMU_CTRL_REG) &= ~DCDC12_ENABLE_BIT;

    //reset usb detect signal
    REG32(PMU_CTRL_REG) |= USB_DET_N_CTRL;
    REG32(PMU_CTRL_REG) &= ~USB_DET_N_CTRL;
    
    //set ldo12
    REG32(PMU_CTRL_REG) &= ~(0x7<<12);
    REG32(PMU_CTRL_REG) |= (vol<<12);

    irq_unmask();

}

/**
 * @brief set output voltage of dcdc12
              ldo12 will be disabled according to chip spec
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param vol voltage to be set
 * @return T_VOID
 */
T_VOID pmu_set_dcdc12(E_DCDC12_VOL vol)
{
    T_U32 reg;

    akprintf(C1, M_DRVSYS, "dcdc not support\n");
    
#if 0
    irq_mask();

    //enable DCDC12
    REG32(PMU_CTRL_REG) |= DCDC12_ENABLE_BIT;
    
    //disable ldo12
    REG32(PMU_CTRL_REG) &= ~LDO12_ENABLE_BIT;

    //reset usb detect signal
    REG32(PMU_CTRL_REG) |= USB_DET_N_CTRL;
    REG32(PMU_CTRL_REG) &= ~USB_DET_N_CTRL;

    //set ldo12
    REG32(PMU_CTRL_REG) &= ~(0x7<<6);
    REG32(PMU_CTRL_REG) |= (vol<<6);

    irq_unmask();
#endif
}

/**
 * @brief set output voltage of ldopll 
 * @author Liao_Zhijun
 * @date 2010-12-07
 * @param vol voltage to be set
 * @return T_VOID
 */
T_VOID pmu_set_ldopll(E_LDOPLL_VOL vol)
{
    T_U32 reg;
    
    irq_mask();

    if(vol == LDOPLL_125V)
        REG32(PMU_CTRL_REG) &= ~LDOPLL_CTRL_BIT;
    else
        REG32(PMU_CTRL_REG) |= LDOPLL_CTRL_BIT;

    irq_unmask();
}


/**
 * @brief  read and adjust the VREF1V5
 * @author kejianping
 * @date 2014-10-22
 * @return T_U32
 * @retval  1) "0" : read fail, other value :  read success.
 * @          2) "bit7=1 (the first bit is bit0)" means that fuse has not been burnt. 
 * @          3) other values meaning adjust voltage  successfully.
 */

#define EFUSE_CTRL_REG					 0x08000048
T_U32 Efuse_Rd(T_VOID)
{	
	T_U32 rTmp = 0x0;
	
	//pull down VP 
	REG32(ANALOG_CTRL_REG3) |= (1 << 3);
	REG32(ANALOG_CTRL_REG4) |= ((1 << 0) | (1 << 25));
			
	mini_delay(200);   //wait for VP pull down, (>150 ms)
	
	REG32(EFUSE_CTRL_REG) = 0x00000002;//set read	mode
	REG32(EFUSE_CTRL_REG) |= 0x00000001;//start to read	
	us_delay(15);  //(10);	//wait for read finish ( > 2us )

	REG32(EFUSE_CTRL_REG) &= (~0x00000001);	//clear efuse_cfg_rdy bit for next operate
	
	rTmp = REG32(EFUSE_CTRL_REG);		

	return (rTmp >> 8)&0xff;
}



