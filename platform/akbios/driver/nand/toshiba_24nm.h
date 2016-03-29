/**
 * @filename toshiba_24nm.h
 * @brief  some macro needed by the Read-Retry algorithms for toshiba 24nm nand
 * Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @author yangyiming
 * @date 2012-05-15
 * @version 1.0
 */

#ifndef __TOSHIBA_24NM_H_
#define __TOSHIBA_24NM_H_

#define TOSHIBA_PRE_CMD1_24NM   0x5C
#define TOSHIBA_PRE_CMD2_24NM   0xC5
#define TOSHIBA_SET_CMD_24NM    0x55
#define TOSHIBA_RESET_CMD_24NM  0xFF
#define TOSHIBA_END_CMD1_24NM    0x26
#define TOSHIBA_END_CMD2_24NM    0x5D

#define TOSHIBA_RETRY_TIMES_24NM 5
#define TOSHIBA_RETRY_REGCNT_24NM 4

#define TOSHIBA_REG1_24NM 0x04
#define TOSHIBA_REG2_24NM 0x05
#define TOSHIBA_REG3_24NM 0x06
#define TOSHIBA_REG4_24NM 0x07

#define IDL_32gb_24nm   0x3294d798
#define IDH_32gb_24nm   0x5676
#define IDL_64gb_24nm   0x8294de98
#define IDH_64gb_24nm   0x5676

extern T_NAND_RETRY_FUNCTION_SET toshiba_24nm_set;


#endif
