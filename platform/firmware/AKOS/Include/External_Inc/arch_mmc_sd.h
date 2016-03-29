/**
 * @file arch_mmc_sd.h
 * @brief list SD card operation interfaces.
 *
 * This file define and provides functions of SD card
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author HuangXin
 * @date 2010-06-17
 * @version 2.0 for AK322x
 */
#ifndef __ARCH_MMC_SD_H
#define __ARCH_MMC_SD_H

#include"hal_detector.h"

/** @defgroup MMC_SD_SDIO MMC_SD_SDIO
 *  @ingroup Drv_Lib
 */
/*@{*/

typedef T_VOID * T_pCARD_HANDLE;

typedef enum
{    
    PARTITION_USER,
    PARTITION_BOOT1,
    PARTITION_BOOT2,
    PARTITION_RPMB,
    PARTITION_GP1,
    PARTITION_GP2,
    PARTITION_GP3,
    PARTITION_GP4,
    PARTITION_NUM
}
T_eCARD_PARTITION;


typedef enum
{
    INTERFACE_NOT_SD,
    INTERFACE_SDMMC4,
    INTERFACE_SDMMC8,
    INTERFACE_SDIO
}
T_eCARD_INTERFACE;


typedef enum _BUS_MODE
{
    USE_ONE_BUS,
    USE_FOUR_BUS,
    USE_EIGHT_BUS
}T_eBUS_MODE;

/**
 * @brief   mmc4.3 later card switch partition
 *
 * If card spec vers is mmc4.3 later,this func should be called to switch partition 
 * @author Huang Xin
 * @date 2010-07-14
 * @param[in] handle card handle,a pointer of void 
 * @param[in] part the selected partition
 * @return T_BOOL
 * @retval  AK_TRUE: switch successfully
 * @retval  AK_FALSE: switch failed
 */
T_BOOL emmc_switch_partition(T_pCARD_HANDLE handle,T_eCARD_PARTITION part);

/**
 * @brief read data from sd card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] handle card handle,a pointer of void
 * @param[in] block_src source block to read
 * @param[out] databuf data buffer to read
 * @param[in] block_count size of blocks to be readed
 * @return T_BOOL
 * @retval  AK_TRUE: read successfully
 * @retval  AK_FALSE: read failed
 */
T_BOOL sd_read_block(T_pCARD_HANDLE handle,T_U32 block_src, T_U8 *databuf, T_U32 block_count);

/**
 * @brief write data to sd card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] handle card handle,a pointer of void
 * @param[in] block_dest destation block to write
 * @param[in] databuf data buffer to write
 * @param[in] block_count size of blocks to be written
 * @return T_BOOL
 * @retval  AK_TRUE:write successfully
 * @retval  AK_FALSE: write failed
 */
T_BOOL sd_write_block(T_pCARD_HANDLE handle,T_U32 block_dest, const T_U8 *databuf, T_U32 block_count);


/**
 * @brief set the sd interface clk
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] handle card handle,a pointer of void
 * @param[in] clock clock to set
 * @return T_BOOL
 */
T_BOOL sd_set_clock(T_pCARD_HANDLE handle,T_U32 clock);


/**
 * @brief Close sd controller
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] handle card handle,a pointer of void
 * @return T_VOID
 */
T_VOID sd_free(T_pCARD_HANDLE handle);

/**
 * @brief get sd card information
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] handle card handle,a pointer of void
 * @param[out] total_block current sd's total block number
 * @param[out] block_size current sd's block size
 * @a block = 512 bytes
 * @return T_VOID
 */
T_VOID sd_get_info(T_pCARD_HANDLE handle, T_U32 *total_block, T_U32 *block_size);


/**
* @brief initial mmc sd or comob card
* @author Huang Xin
* @date 2010-06-17
* @param[in] cif card interface selected
* @param[in] bus_mode bus mode selected, can be USE_ONE_BUS or USE_FOUR_BUS
* @return T_pCARD_HANDLE
* @retval NON-NULL  set initial successful,card type is  mmc sd or comob
* @retval NULL set initial fail,card type is not mmc sd or comob card
*/
T_pCARD_HANDLE sd_initial(T_eCARD_INTERFACE cif, T_U8 bus_mode);

/**
* @brief register detect mmc sd or comob card, !!notice: this interface only for 37c
* @author Yang xi
* @date 2014-06-11
* @param[in] 
* @return 
*/
T_BOOL card_detect_reg(T_eCARD_INTERFACE card_type, T_U32 gpio_num, 
                      T_BOOL benable_dat3, T_fDETECTOR_CALLBACK pcallbackfunc);

/**
* @brief enable detect mmc sd or comob card
* @author Yang xi
* @date 2014-06-11
* @param[in] 
* @return 
*/
T_BOOL card_detect_enable(T_eCARD_INTERFACE card_type,T_BOOL benable);


/**
 * @brief       Get the connecting state of the card interface by interface.
 * @author      Yang Xi
 * @date        2014.06.13
 * @param[in]   card_type
 *                  card type of the device to be detected.
 * @param[in]   pState
 *                  Pointer to a T_BOOL type variable for fetching the 
 *                  connecting state.
 * @return      T_BOOL
 * @retval      If the function succeeds, the return value is AK_TRUE;
 *              If the function fails, the return value is AK_FALSE.
 */ 
T_BOOL card_detector_get_state(T_eCARD_INTERFACE card_type, T_BOOL *pState);
/*@}*/
#endif //__ARCH_MMC_SD_H  
