/**
 * @file hal_spiflash.h
 * @brief spiflash driver interface file.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */
#ifndef _SERIAL_FLASH_H_
#define _SERIAL_FLASH_H_


#include "anyka_types.h"
#include "arch_spi.h"

#define     SPIBOOT_DATA_START_PAGE        (257)


/** @defgroup spiflash group
 *  @ingroup Drv_Lib
 */
/*@{*/

/**
 * @brief  flag define, used in spi flash flag param
 */
#define SFLASH_FLAG_UNDER_PROTECT       (1<<0)
#define SFLASH_FLAG_FAST_READ           (1<<1)
#define SFLASH_FLAG_AAAI                (1<<2)

#define SFLASH_FLAG_DUAL_WRITE          (1<<3)
#define SFLASH_FLAG_DUAL_READ           (1<<4)
#define SFLASH_FLAG_QUAD_WRITE          (1<<5)
#define SFLASH_FLAG_QUAD_READ           (1<<6)
#define SFLASH_FLAG_COM_STATUS2         (1<<7)

/**
 * @brief  serial flash param define
 */
typedef struct tagSFLASH_PARAM
{
    T_U32   id;                     ///< flash id
    T_U32   total_size;             ///< flash total size in bytes
    T_U32   page_size;              ///< bytes per page
    T_U32   program_size;           ///< program size at 02h command
    T_U32   erase_size;             ///< erase size at d8h command 
    T_U32   clock;                  ///< spi clock, 0 means use default clock 

    //chip character bits:
    //bit 0: under_protect flag, the serial flash under protection or not when power on
    //bit 1: fast read flag, the serial flash support fast read or not(command 0Bh)
    //bit 2: AAI flag, the serial flash support auto address increment word programming
    //bit 3: support dual write or no
    //bit 4: support dual read or no
    //bit 5: support quad write or no
    //bit 6: support quad read or no
    //bit 7: the second status command (35h) flag,if use 4-wire(quad) mode,the bit must be is enable
    T_U8    flag;                   ///< chip character bits
    T_U8    protect_mask;           ///< protect mask bits in status register:BIT2:BP0, BIT3:BP1, BIT4:BP2, BIT5:BP3, BIT7:BPL
    T_U8    reserved1;
    T_U8    reserved2;
}T_SFLASH_PARAM;

extern T_SFLASH_PARAM spiflash_param;

typedef struct
{
	T_U32 file_length;
    T_U32 ld_addr;
    T_U32 start_page;
    T_U32 backup_page;        //backup data start page
    T_U8 file_name[16];
}T_FILE_CONFIG;


typedef struct
{
    T_SFLASH_PARAM param;
    T_U8 name[32];
}
T_SFLASH_STRUCT;

/**
 * @brief spi flash init
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param spi_id [in]  spi id, can be spi0 or spi1
 * @return T_BOOL
 */
T_BOOL spi_flash_init(T_eSPI_ID spi_id, T_eSPI_BUS bus_width);

/**
 * @brief set param of serial flash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param sflash_param [in]  serial flash param 
 * @return T_VOID
 */
T_VOID spi_flash_set_param(T_SFLASH_PARAM *sflash_param);

/**
 * @brief read data from one page of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param buf [in]  buffer to store read data 
 * @param page_cnt [in]  the page count to read  
 * @return T_BOOL
 */
T_BOOL spi_flash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt);

/*@}*/


#endif    // _SERIAL_FLASH_H_

