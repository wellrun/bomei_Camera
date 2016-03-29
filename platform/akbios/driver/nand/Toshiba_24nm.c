/**
 * @filename toshiba_24nm.c
 * @brief  the Read-Retry algorithms for toshiba 24nm nand
 * Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @author yangyiming
 * @date 2012-05-15
 * @version 1.0
 */
 
//References <<FMMTA32-1 24nm NAND Retry Read Sequence_20110613_For Anyka eyes only.pdf>>

#include "read_retry.h"
#include "toshiba_24nm.h"

#ifdef SUPPORT_TOSHIBA_24NM

static T_U32 set_buf_add[TOSHIBA_RETRY_REGCNT_24NM];
static T_U8 cur_scale;//indicates the current chip

const T_U8 retry_offset_toshiba_24nm[TOSHIBA_RETRY_TIMES_24NM][TOSHIBA_RETRY_REGCNT_24NM] =
{
    {0x04, 0x04, 0x04, 0x04},
    {0x7c, 0x7c, 0x7c, 0x7c},
    {0x78, 0x78, 0x78, 0x78},
    {0x74, 0x74, 0x74, 0x74},
    {0x08, 0x08, 0x08, 0x08}
};


T_NFC_NODE Toshiba_2xnm_Set[] =
{
    { CFG_CMD,  TOSHIBA_PRE_CMD1_24NM},
    { CFG_CMD,  TOSHIBA_PRE_CMD2_24NM},
    { CFG_CMD,  TOSHIBA_SET_CMD_24NM },
    { CFG_ADD,  TOSHIBA_REG1_24NM},//the 1st register
    { CFG_WRITE,  0x1},//writer one byte 
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_CMD,  TOSHIBA_SET_CMD_24NM },
    { CFG_ADD,  TOSHIBA_REG2_24NM},//the 2nd register
    { CFG_WRITE,  0x1},//writer one byte
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_CMD,  TOSHIBA_SET_CMD_24NM },
    { CFG_ADD,  TOSHIBA_REG3_24NM},//the 3rd register
    { CFG_WRITE,  0x1},//writer one byte
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_CMD,  TOSHIBA_SET_CMD_24NM },
    { CFG_ADD,  TOSHIBA_REG4_24NM},//the 4th register
    { CFG_WRITE,  0x1},//writer one byte
    { CFG_BUFADD, 0xFFFFFFFF},//the buff stores the data
    { CFG_CMD,  TOSHIBA_END_CMD1_24NM},
    { CFG_CMD,  TOSHIBA_END_CMD2_24NM}
};

T_NFC_NODE Toshiba_2xnm_Revert[] =
{
    { CFG_CMD,  TOSHIBA_RESET_CMD_24NM},
    { CFG_RB,   0xFF},
};


static T_U8 get_reg_value(T_U8 reg_index)
{
    T_U8 reg_value;

    reg_value = retry_offset_toshiba_24nm[cur_scale][reg_index];
    
    akprintf(C1, "NAND", "S:%d_V:0x%02x\n", cur_scale, reg_value);
    
    return reg_value;
}    

static T_VOID RR_init_24nm(T_U32 chip)
{
    cur_scale = 0;
    Toshiba_2xnm_Set[5].value = &set_buf_add[0];
    Toshiba_2xnm_Set[9].value = &set_buf_add[1];
    Toshiba_2xnm_Set[13].value = &set_buf_add[2];
    Toshiba_2xnm_Set[17].value = &set_buf_add[3];
    
    akprintf(C1, "NAND", "Toshiba Read Retry.\n");
}


static T_VOID RR_modify_scales_24nm(T_U32 chip)
{
    T_U8 i;
    
    if(TOSHIBA_RETRY_TIMES_24NM == cur_scale )
    {
        nfl_cmd_seq(chip, Toshiba_2xnm_Revert, sizeof(Toshiba_2xnm_Revert) / sizeof(Toshiba_2xnm_Revert[0]));
        akprintf(C1, "NAND", "Default Scales.\n");
        cur_scale = 0;
    }
    else
    {
        for(i = 0; i < TOSHIBA_RETRY_REGCNT_24NM; i++)
        {
            set_buf_add[i] = get_reg_value(i);
        }
    }
    
    nfl_cmd_seq(chip, Toshiba_2xnm_Set, sizeof(Toshiba_2xnm_Set) / sizeof(Toshiba_2xnm_Set[0]));
    cur_scale++;

}

static T_VOID RR_revert_scales_24nm(T_U32 chip)
{
    cur_scale = TOSHIBA_RETRY_TIMES_24NM;
    RR_modify_scales_24nm(chip);
}

T_NAND_RETRY_FUNCTION_SET toshiba_24nm_set =
{   
    TOSHIBA_RETRY_TIMES_24NM,
    RR_init_24nm,
    RR_modify_scales_24nm,
    RR_revert_scales_24nm
};

#ifdef RTOS
int RR_toshiba_24nm_reg(T_VOID)
{
    nfl_retry_reg(IDL_64gb_24nm, IDH_64gb_24nm, &toshiba_24nm_set);
    nfl_retry_reg(IDL_32gb_24nm, IDH_32gb_24nm, &toshiba_24nm_set);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(RR_toshiba_24nm_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif //RTOS

#endif
