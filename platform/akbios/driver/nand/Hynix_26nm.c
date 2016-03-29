/**
 * @filename hynix_26nm.c
 * @brief  the Read-Retry algorithms for hynix 26nm nand
 * Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @author yangyiming
 * @date 2012-05-15
 * @version 1.0
 */

//References <<26nm_64Gb_MLC_RawNAND_ApplicationNote_Rev01_110411.pdf>
//<<26nm_32Gb_MLC_RawNAND_ApplicationNote_Rev04_110411.pdf>

#include "read_retry.h"
#include "hynix_26nm.h"

#ifdef SUPPORT_HYNIX_26NM

static T_S8 (*offset_tbl)[HYNIX_RETRY_REGCNT_26NM];//the pointer to offset table
static T_U32 default_scalse[MAX_RR_CHIPCNT][HYNIX_RETRY_REGCNT_26NM];//to store the default scales in each chip
static T_U32 set_buf_add[HYNIX_RETRY_REGCNT_26NM];
static T_U32 cur_chip;//indicates the current chip 
static T_U8 cur_scale;//indicates the current chip


static T_NFC_NODE Hynix_2xnm_Get[] =
{
    { CFG_CMD ,  HYNIX_GET_CMD_26NM},
    { CFG_ADD,  HYNIX_REG},//just a initial value,the add of  the registers to be read
    { CFG_READ,  0x1},//read one byte
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_ADD,  HYNIX_REG},//just a initial value,the add of  the registers to be read
    { CFG_READ,  0x1},//read one byte
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_ADD,  HYNIX_REG},//just a initial value,the add of  the registers to be read
    { CFG_READ,  0x1},//read one byte
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_ADD,  HYNIX_REG},//just a initial value,the add of  the registers to be read
    { CFG_READ,  0x1},//read one byte
    { CFG_BUFADD, 0xFFFFFFFF}//the buff stores the data
};

static T_NFC_NODE Hynix_2xnm_Set[] =
{
    { CFG_CMD ,  HYNIX_SET1_CMD_26NM},
    { CFG_ADD,  HYNIX_REG},//just a initial value,the add of  the registers to be write
    { CFG_WRITE,  0x1},//writer one byte
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_ADD,  HYNIX_REG},//just a initial value,the add of  the registers to be write
    { CFG_WRITE,  0x1},//writer one byte
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_ADD,  HYNIX_REG},//just a initial value,the add of  the registers to be write
    { CFG_WRITE,  0x1},//writer one byte
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_ADD,  HYNIX_REG},//just a initial value,the add of  the registers to be write
    { CFG_WRITE,  0x1},//writer one byte
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_CMD,  HYNIX_SET2_CMD_26NM}
};

//hynix 26nm 64Gb MLC RawNAND 
static T_S8 offset_hynix_64gb_26nm[HYNIX_RETRY_TIMES_26NM + 1][HYNIX_RETRY_REGCNT_26NM] =
{
    {0x00, 0x00, 0x00, 0x00,},//default scalse offset
    {0x00, 0x06, 0x0A, 0x06},
    {T_S8_MAX, -0x03, -0x07, -0x08},
    {T_S8_MAX, -0x06, -0x0D, -0x0F},
    {T_S8_MAX, -0x0B, -0x14, -0x17},
    {T_S8_MAX, T_S8_MAX, -0x1A, -0x1E},
    {T_S8_MAX, T_S8_MAX, -0x20, -0x25}
};

const T_U8 reg_hynix_64gb_26nm[HYNIX_RETRY_REGCNT_26NM] =
{
    0xAC, 0xAD, 0xAE, 0xAF
};


//hynix 26nm 32Gb MLC RawNAND 
static T_S8 offset_hynix_32gb_26nm[HYNIX_RETRY_TIMES_26NM + 1][HYNIX_RETRY_REGCNT_26NM] =
{
    {0x00, 0x00, 0x00, 0x00,},//default scalse offset
    {0x00, 0x06, 0x0A, 0x06},
    {T_S8_MAX, -0x03, -0x07, -0x08},
    {T_S8_MAX, -0x06, -0x0D, -0x0F},
    {T_S8_MAX, -0x09, -0x14, -0x17},
    {T_S8_MAX, T_S8_MAX, -0x1A, -0x1E},
    {T_S8_MAX, T_S8_MAX, -0x2A, -0x25}
};

const T_U8 reg_hynix_32gb_26nm[HYNIX_RETRY_REGCNT_26NM] =
{
    0xA7, 0xAD, 0xAE, 0xAF
};


/**
 * @brief get a register's new value for hynix retry nand
 * @author yangyiming
 * @date 2012-5-8
 * @param[in] reg_index offset 
 * @return T_U8 new value of one register
 */
static T_U8 get_reg_value(T_U8 reg_index)
{
    T_U8 reg_value;

    if (T_S8_MAX == offset_tbl[cur_scale ][reg_index])
    {
        reg_value = 0;//fixed value
    }
    else
    {
        reg_value = default_scalse[cur_chip][reg_index] + offset_tbl[cur_scale][reg_index];
    }    
    
    akprintf(C1, "NAND", "S:%d_B:0x%02x_V:0x%02x\n", 
        cur_scale, default_scalse[cur_chip][reg_index], reg_value);
    
    return reg_value;
}    

static T_VOID RR_init_26nm(T_U32 chip)
{
    cur_chip = chip;
    cur_scale = 0;
    
    Hynix_2xnm_Get[3].value = &default_scalse[cur_chip][0];  
    Hynix_2xnm_Get[6].value = &default_scalse[cur_chip][1];    
    Hynix_2xnm_Get[9].value = &default_scalse[cur_chip][2];    
    Hynix_2xnm_Get[12].value = &default_scalse[cur_chip][3];

    Hynix_2xnm_Set[3].value = &set_buf_add[0];
    Hynix_2xnm_Set[6].value = &set_buf_add[1];
    Hynix_2xnm_Set[9].value = &set_buf_add[2];
    Hynix_2xnm_Set[12].value = &set_buf_add[3];
    
    nfl_cmd_seq(chip, Hynix_2xnm_Get, sizeof(Hynix_2xnm_Get) / sizeof(Hynix_2xnm_Get[0]));
    
    akprintf(C1, "NAND", "Default Scales: 0x%2x,0x%2x,0x%2x,0x%2x\n",
        default_scalse[cur_chip][0],default_scalse[cur_chip][1],default_scalse[cur_chip][2],default_scalse[cur_chip][3]);
}

static T_VOID RR_modify_scales_26nm(T_U32 chip)
{
    T_U8 i;
    cur_chip = chip;
    cur_scale++;
    
    if(cur_scale > HYNIX_RETRY_TIMES_26NM)
    {
        cur_scale = 0;
    }
    
    for(i = 0; i < HYNIX_RETRY_REGCNT_26NM; i++)
    {
        set_buf_add[i] = get_reg_value(i);
    }
    
    nfl_cmd_seq(chip, Hynix_2xnm_Set, sizeof(Hynix_2xnm_Set) / sizeof(Hynix_2xnm_Set[0]));

}

static T_VOID RR_revert_scales_26nm(T_U32 chip)
{
    cur_chip = chip;
    cur_scale = HYNIX_RETRY_TIMES_26NM;
    
    RR_modify_scales_26nm(chip);
}

static T_VOID RR_init_64gb_26nm(T_U32 chip)
{
    offset_tbl = offset_hynix_64gb_26nm;
    Hynix_2xnm_Get[1].value = (T_U32)reg_hynix_64gb_26nm[0];
    Hynix_2xnm_Get[4].value  = (T_U32)reg_hynix_64gb_26nm[1];
    Hynix_2xnm_Get[7].value  = (T_U32)reg_hynix_64gb_26nm[2];
    Hynix_2xnm_Get[10].value  = (T_U32)reg_hynix_64gb_26nm[3];

    Hynix_2xnm_Set[1].value = (T_U32)reg_hynix_64gb_26nm[0];
    Hynix_2xnm_Set[4].value  = (T_U32)reg_hynix_64gb_26nm[1];
    Hynix_2xnm_Set[7].value  = (T_U32)reg_hynix_64gb_26nm[2];
    Hynix_2xnm_Set[10].value  = (T_U32)reg_hynix_64gb_26nm[3];
    
    RR_init_26nm(chip);
}
static T_VOID RR_init_32gb_26nm(T_U32 chip)
{
    offset_tbl = offset_hynix_32gb_26nm;
    Hynix_2xnm_Get[1].value = (T_U32)reg_hynix_32gb_26nm[0];
    Hynix_2xnm_Get[4].value = (T_U32)reg_hynix_32gb_26nm[1];
    Hynix_2xnm_Get[7].value = (T_U32)reg_hynix_32gb_26nm[2];
    Hynix_2xnm_Get[10].value = (T_U32)reg_hynix_32gb_26nm[3];

    Hynix_2xnm_Set[1].value = (T_U32)reg_hynix_32gb_26nm[0];
    Hynix_2xnm_Set[4].value  = (T_U32)reg_hynix_32gb_26nm[1];
    Hynix_2xnm_Set[7].value  = (T_U32)reg_hynix_32gb_26nm[2];
    Hynix_2xnm_Set[10].value  = (T_U32)reg_hynix_32gb_26nm[3];
    
    RR_init_26nm(chip);
}

T_NAND_RETRY_FUNCTION_SET hynix_64gb_26nm_set =
{   
    HYNIX_RETRY_TIMES_26NM,
    RR_init_64gb_26nm,
    RR_modify_scales_26nm,
    RR_revert_scales_26nm
};

T_NAND_RETRY_FUNCTION_SET hynix_32gb_26nm_set =
{   
    HYNIX_RETRY_TIMES_26NM,
    RR_init_32gb_26nm,
    RR_modify_scales_26nm,
    RR_revert_scales_26nm
};

#ifdef RTOS
int RR_hynix_26nm_reg(T_VOID)
{
    nfl_retry_reg(IDL_64gb_26nm, IDH_64gb_26nm, &hynix_64gb_26nm_set);
    nfl_retry_reg(IDL_32gb_26nm, IDH_32gb_26nm, &hynix_32gb_26nm_set);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(RR_hynix_26nm_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif //RTOS

#endif

