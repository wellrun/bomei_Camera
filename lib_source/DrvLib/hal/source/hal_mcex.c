/** 
 * @file hal_mcex.c
 * @brief source file for sd card mcex function
 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiXiao
 * @date 2006-04-05
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "drv_api.h"
#include "sd.h"
#include "hal_sd.h"
#include "hal_common_sd.h"
#include "l2.h"
#include "sysctl.h"
#include "hal_mcex.h"
#include "drv_module.h"


//The interface shall be INTERFACE_SDMMC4
#define MCEX_DRV_PROTECT(interface) \
        do{ \
            DrvModule_Protect(DRV_MODULE_SDMMC);\
            set_interface(interface);\
            card_set_enter_state(interface);\
            card_detect_enable(interface, AK_FALSE);\
        }while(0)

//The interface shall be INTERFACE_NOT_SD
#define MCEX_DRV_UNPROTECT(interface, true_interface) \
        do{ \
            set_interface(interface);\
            DrvModule_UnProtect(DRV_MODULE_SDMMC);\
            card_comeback_detect_state(true_interface);\
        }while(0)

//global variabl for mcex
static T_pCARD_HANDLE s_pMcexCard = AK_NULL;


/**
 * @brief mcex init
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return T_BOOL
 * @retval AK_TRUE init success
 * @retval AK_FALSE fail to init
 */
T_BOOL mcex_init()
{
    T_BOOL ret = AK_TRUE;

    //sd initial 
    s_pMcexCard = (T_pCARD_HANDLE)sd_initial(INTERFACE_SDMMC4, USE_ONE_BUS);
    if(AK_NULL == s_pMcexCard )
    {
        akprintf(C1, M_DRVSYS, "sd initial error\n");
        return AK_FALSE;
    }
    MCEX_DRV_PROTECT(INTERFACE_SDMMC4);
    //check if mcex support
    if(!mcex_check())
    {
        akprintf(C1, M_DRVSYS, "mcex check fail!\n");
        ret = AK_FALSE;
        goto EXIT;
    }

    //mode switch
    if(!mcex_open())
    {
        akprintf(C1, M_DRVSYS, "mcex open fail!\n");
        ret = AK_FALSE;
        goto EXIT;
    }

EXIT:
    MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDMMC4);
    return ret;
}

/**
 * @brief mcex close
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return T_VOID
 */
T_VOID mcex_close()
{
}

/**
 * @brief get psi
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param type [in]: psi type
 * @param data [out]: data buffer
 * @return T_BOOL
 * @retval AK_TRUE get psi success
 * @retval AK_FALSE fail to get psi
 */
T_BOOL mcex_get_psi(T_U32 type, T_U8 *data)
{
    T_U32 status;
    T_BOOL ret=AK_FALSE;
    volatile T_U32 reg_value;
    T_U8 buf_id = 0;

    MCEX_DRV_PROTECT(INTERFACE_SDMMC4);
    //step1: check bus busy
    if( sd_trans_busy())
    {
        goto exit;
    }
    //step2: send card command
    if(send_cmd(SD_CMD(MCEX_SEND_PSI), SD_SHORT_RESPONSE, type) == AK_FALSE)
    {
        akprintf(C3, M_DRVSYS, "MCEX_SEND_PSI command failed!\n");
        goto exit;
    }
    //step3: transfer data
    ret = sd_trans_data_dma((T_U32)data,SD_DEFAULT_BLOCK_LEN,SD_DATA_CTL_TO_HOST);
  
    exit:

    MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDMMC4);

    return ret;
}

/**
 * @brief check if the present card support mcex function or not
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return T_BOOL
 * @retval AK_TRUE the present card support mcex
 * @retval AK_FALSE the present card doesn't support mcex
 */
T_BOOL mcex_check()
{
    T_U8 status[512];
    T_U32 i;

    if(!sd_mode_switch(0, 1, 1, (T_U32 *)status))
    {
        return AK_FALSE;
    }
    if (!(status[11] & 0x02)) {
        akprintf(C3, M_DRVSYS, "G&D Functiongroup 2 - Function 1 not supported\n"); // BIT [431:416]
        return AK_FALSE;
    }
    if ((status[16] & 0x0f0) != (1<<4)) {
        akprintf(C3, M_DRVSYS, "G&D Functiongroup 2 - Function 1 Switch failed\n"); // BIT [383:380]
        return AK_FALSE;
    }

    return AK_TRUE;

}

/**
 * @brief open mcex function for the present card
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return T_BOOL
 * @retval AK_TRUE open success
 * @retval AK_FALSE fail to open
 */
T_BOOL mcex_open()
{
    T_U8 status[64];
    T_U32 i;

    if(!sd_mode_switch(1, 1, 1, (T_U32 *)status))
    {
        return AK_FALSE;
    }
    if (!(status[11] & 0x02)) {
        akprintf(C3, M_DRVSYS, "G&D Functiongroup 2 - Function 1 not supported\n"); // BIT [431:416]
        return AK_FALSE;
    }
    if ((status[16] & 0x0f0) != (1<<4)) {
        akprintf(C3, M_DRVSYS, "G&D Functiongroup 2 - Function 1 Switch failed\n"); // BIT [383:380]
        return AK_FALSE;
    }

    return AK_TRUE;
}

/**
 * @brief reset mcex function for the present card
 *
 * @author huang_xin
 * @date 2010-08-31
 * @return T_BOOL
 * @retval AK_TRUE reset success
 * @retval AK_FALSE fail to reset
 */
T_BOOL mcex_reset()
{
    MCEX_DRV_PROTECT(INTERFACE_SDMMC4);

    if(!send_cmd( SD_CMD(MCEX_CONTROL_TRM), SD_SHORT_RESPONSE, 1 ) )
    {
        akprintf(C1, M_DRVSYS, "MCEX_CONTROL_TRM failed!\n");
        MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDMMC4);
        return AK_FALSE;
    }

    MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDMMC4);
    return AK_TRUE;
}

/**
 * @brief get timeout for mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param read_timeout [in]: timeout for read operation
 * @param write_timeout [in]: timeout for write operation
 * @return T_BOOL
 * @retval AK_TRUE reset success
 * @retval AK_FALSE fail to reset
 */
T_BOOL mcex_get_timeout(T_U32 *read_timeout, T_U32 *write_timeout)
{
    T_U8 data[512];

    *read_timeout = 0;
    *write_timeout = 0;

    memset(data, 0, 512);
    if(!mcex_get_psi(MCEX_PSI_PR, data))
    {
        return AK_FALSE;
    }

    *read_timeout = (T_U32)(data[0]) * 250;
    *write_timeout = (T_U32)(data[1]) * 250;

    return AK_TRUE;
}

static T_BOOL mcex_get_status(T_U8 *status, T_U8 *error)
{
    T_U8 data[512];

    *status = 0;
    *error = 0;

    if(!mcex_get_psi(MCEX_PSI_SR, data))
    {
        return AK_FALSE;
    }

    *status = data[0];
    *error = data[1];
    
    return AK_TRUE;
}

static T_BOOL mcex_check_status(MCEX_STATUS stop)
{
    T_U8 status, error;
    T_U32 cnt = 0, errcnt= 0 ;

    do
    {
        if(mcex_get_status(&status, &error))
        {
            if(error != eMCEX_error_none)
            {
                akprintf(C3, M_DRVSYS, "progress error!, status=%x, error=%x\n", status, error);
                return AK_FALSE;
            }
            
            if(status == stop)
            {
                return AK_TRUE;
            }
        }
        else
        {
            akprintf(C3, M_DRVSYS, "get mcex status error!\n");
            return AK_FALSE;
        }
    }
    while(cnt++ < 1000);

    akprintf(C3, M_DRVSYS, "get mcex status time out, status=0x%x, error = 0x%x!\n", status, error);
    return AK_FALSE;
}

/**
 * @brief write data through mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param mode [in]: 
 * @param data [in]: data to be written
 * @param blk_size [in]: block size
 * @return T_BOOL
 * @retval AK_TRUE write success
 * @retval AK_FALSE fail to write
 */
T_BOOL mcex_write(T_U32 mode, T_U8 *data, T_U32 blk_size)
{
    T_U32 arg;
    T_U32 size;
    T_U32 reg_value;
    T_U32 i;
    T_U32 status, ret = AK_FALSE;
    T_U8 buf_id = 0;

    MCEX_DRV_PROTECT(INTERFACE_SDMMC4);

    mode = !!mode;
    arg = mode << 31;
    arg |= blk_size & 0x0000FFFF;
    size = blk_size*SD_DEFAULT_BLOCK_LEN;
    //step1: check bus busy
    if( sd_trans_busy())
    {
        goto exit;
    }
    //step2: send card command
    if (!send_cmd( SD_CMD(MCEX_WRITE_SEC_CMD), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "The MCEX_WRITE_SEC_CMD command failed!\n");
        goto exit; 
    }
    //step3: transfer data
    ret = sd_trans_data_dma((T_U32)data,size,SD_DATA_CTL_TO_CARD);
    //step4: wait card status to idle
    if(!mcex_check_status(eMCEX_status_cmd_complete))
    {
        akprintf(C3, M_DRVSYS, "mcex write error\n");
        ret = AK_FALSE;
    }  
    exit:
    MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDMMC4);

    return ret;

}

/**
 * @brief read data through mcex
 *
 * @author huang_xin
 * @date 2010-08-31
 * @param data [out]: data to be read
 * @param blk_size [in]: block size
 * @return T_BOOL
 * @retval AK_TRUE read success
 * @retval AK_FALSE fail to read
 */
T_BOOL mcex_read(T_U8 *data, T_U32 blk_size)
{
    T_U32 arg;
    T_U32 size;
    T_U32 reg_value;
    T_U32 i;
    T_U32 status, ret = AK_FALSE;
    T_U8 buf_id;

    MCEX_DRV_PROTECT(INTERFACE_SDMMC4);

    size = blk_size * SD_DEFAULT_BLOCK_LEN;

    //step1: check bus busy
    if( sd_trans_busy())
    {
        goto exit;
    }
    arg = blk_size & 0x0000FFFF;
    //step2: send card command
    if (!send_cmd( SD_CMD(MCEX_READ_SEC_CMD), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "The MCEX_READ_SEC_CMD command failed!\n");
        goto exit;
    }

    //step3: transfer data
    ret = sd_trans_data_dma((T_U32)data,size,SD_DATA_CTL_TO_HOST);
    //step4: wait card status to idle
    if(!mcex_check_status(eMCEX_status_idle))
    {
        akprintf(C3, M_DRVSYS, "mcex read error\n");
        ret = AK_FALSE;
    }
    exit:
    MCEX_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDMMC4);
    return ret;

}




