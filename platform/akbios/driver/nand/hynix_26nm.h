/**
 * @filename hynix_26nm.h
 * @brief  some macro needed by the Read-Retry algorithms for hynix 26nm nand
 * Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @author yangyiming
 * @date 2012-05-15
 * @version 1.0
 */

#ifndef __HYNIX_26NM_H_
#define __HYNIX_26NM_H_

#define HYNIX_GET_CMD_26NM    0x37  
#define HYNIX_SET1_CMD_26NM   0x36
#define HYNIX_SET2_CMD_26NM   0x16
#define HYNIX_RETRY_TIMES_26NM 6
#define HYNIX_RETRY_REGCNT_26NM 4
#define HYNIX_REG 0xff
#define IDL_64gb_26nm   0xd294dead
#define IDH_64gb_26nm   0x4304
#define IDL_32gb_26nm   0xda94d7ad
#define IDH_32gb_26nm   0xc374

extern T_NAND_RETRY_FUNCTION_SET hynix_64gb_26nm_set;
extern T_NAND_RETRY_FUNCTION_SET hynix_32gb_26nm_set;

#endif
