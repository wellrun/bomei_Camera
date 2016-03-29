/**@file hal_common_sd.c
 * @brief Implement sd&sdio commonl operations of how to control sd&sdio.
 *
 * This file implement sd&sdio common bus driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */
#include "sd.h"
#include "hal_sdio.h"
#include "hal_common_sd.h"
#include "sysctl.h"
#include "drv_api.h"

T_pSD_DEVICE g_pCurSdDevice = AK_NULL;

static T_BOOL init_start(T_BOOL bInitMem);
static T_eCARD_TYPE init_finish();
static T_eCARD_TYPE get_card_type();

/**
 * @brief Init sd host controller.
 *
 * Select sd/sdio interface, set share pin,enable sd/sdio module
 * @author Huang Xin
 * @param cif[in] card interface selected
 * @date 2010-07-14
 * @return T_VOID
 */
T_VOID sd_open_controller(T_eCARD_INTERFACE cif)
{
    if(INTERFACE_NOT_SD == cif)
    {
        return;
    }
    else if(INTERFACE_SDIO == cif)
    {
        akprintf(C3, M_DRVSYS, "..Use SDIO interface\r\n");     
        sysctl_clock(CLOCK_MCI2_ENABLE);
        //reset sdio controller
        sysctl_reset(RESET_MCI2);
    }
    else
    {
        akprintf(C3, M_DRVSYS, "..Use SD/MMC interface\r\n");   
        sysctl_clock(CLOCK_MCI1_ENABLE);
        //reset sd controller
        sysctl_reset(RESET_MCI1);
    }
    //set share pin
    set_pin(cif);
    //set negoiate working clock
    set_clock(SD_IDENTIFICATION_MODE_CLK, get_asic_freq(), SD_POWER_SAVE_ENABLE);
}

/**
 * @brief Close sd host controller.
 *
 * Select non sd/sdio interface, restore share pin, close sd/sdio module
 * @author Huang Xin
 * @param cif[in] card interface selected
 * @date 2010-07-14
 * @return T_VOID
 */
T_VOID sd_close_controller(T_eCARD_INTERFACE cif)
{
    if(INTERFACE_NOT_SD == cif)
    {
        return;
    }
    else if(INTERFACE_SDIO == cif)
    {
        akprintf(C3, M_DRVSYS, "..close SDIO interface\r\n");   
        sysctl_clock(~CLOCK_MCI2_ENABLE);
    }
    else
    {
        akprintf(C3, M_DRVSYS, "..close SD/MMC interface\r\n"); 
        sysctl_clock(~CLOCK_MCI1_ENABLE);
    }
}

/**
 * @brief get sd host controller states
 * @author LHS
 * @date 2011-10-26
 * @param cif[in] card interface selected
 * @return T_BOOL: return TURE mean controller is opend.
 */
T_BOOL sd_get_controller_state(T_eCARD_INTERFACE cif)
{
    T_BOOL ret = AK_FALSE;

    if(INTERFACE_NOT_SD == cif)
    {
        return ret;
    }
    else if(INTERFACE_SDIO == cif)
    {
        ret = sysctl_get_clock_state(CLOCK_MCI2_ENABLE);
    }
    else
    {
        ret = sysctl_get_clock_state(CLOCK_MCI1_ENABLE);
    }

    return ret;
}


/**
 * @brief Init sd card.
 *
 * Init card ,get the card type
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
T_eCARD_TYPE init_card(T_BOOL bInitIo,T_BOOL bInitMem)
{
    T_U8 init_io_status = 0;
    T_U8 init_mem_status =0;
    
    if (!init_start(bInitMem))
    {
         return g_pCurSdDevice->enmCardType;
    }
    init_io_status = init_io(bInitIo);
    if (COMMON_SD_INIT_IO_FAIL == init_io_status)
    {    
        if (!g_pCurSdDevice->bMemPresent)
        {
            g_pCurSdDevice->enmCardType = init_finish();
        }
        else
        {
            init_mem_status = init_mem(bInitMem);
            if ((COMMON_SD_SKIP_INIT_MEM == init_mem_status) || 
                (COMMON_SD_INIT_MEM_FAIL == init_mem_status))
            {
                g_pCurSdDevice->enmCardType = init_finish();
            }
            if (COMMON_SD_INIT_FAIL == init_mem_status)
            {
                g_pCurSdDevice->enmCardType = CARD_UNUSABLE;
            }
            
            if (COMMON_SD_INIT_MEM_SUCCESS == init_mem_status)
            {
                g_pCurSdDevice->enmCardType = get_card_type();
            }
        }
    }
    else if (COMMON_SD_INIT_FAIL == init_io_status)
    {
        g_pCurSdDevice->enmCardType = CARD_UNUSABLE;
    }
    else if (COMMON_SD_INIT_IO_SUCCESS == init_io_status)
    {
        if (!g_pCurSdDevice->bMemPresent)
        {
            g_pCurSdDevice->enmCardType = init_finish();
        }
        else
        {
             init_mem_status = init_mem(bInitMem);
             if ((COMMON_SD_SKIP_INIT_MEM == init_mem_status) || 
                (COMMON_SD_INIT_MEM_FAIL == init_mem_status))
             {
                g_pCurSdDevice->enmCardType = init_finish();
             }
             if (COMMON_SD_INIT_FAIL == init_mem_status)
             {
                g_pCurSdDevice->enmCardType = CARD_UNUSABLE;
             }
            
             if (COMMON_SD_INIT_MEM_SUCCESS == init_mem_status)
             {
                g_pCurSdDevice->enmCardType = get_card_type();
             }
        }
    }
    else if (COMMON_SD_SKIP_INIT_IO == init_io_status)
    {
        init_mem_status = init_mem(bInitMem);
        if ((COMMON_SD_SKIP_INIT_MEM == init_mem_status) || 
            (COMMON_SD_INIT_MEM_FAIL == init_mem_status))
        {
            g_pCurSdDevice->enmCardType = init_finish();
        }
        if (COMMON_SD_INIT_FAIL == init_mem_status)
        {
            g_pCurSdDevice->enmCardType = CARD_UNUSABLE;
        }
        if (COMMON_SD_INIT_MEM_SUCCESS == init_mem_status)
        {
            g_pCurSdDevice->enmCardType = get_card_type();
        }
    }
    return g_pCurSdDevice->enmCardType;
}
 


/**
 * @brief get sd relative address.
 *
 * Send CMD3 to get the sd relative address.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cmd_index[in] The command index.
 * @param rsp[in] The command response:no response ,short reponse or long response
 * @param arg[in] The cmd argument.
 * @return T_U32
 * @retval  The RCA
 * @retval  ERROR_INVALID_RCA
 */
T_U32 get_rca()
{
    T_U32 rca=0;
    if (send_cmd( SD_CMD(3), SD_SHORT_RESPONSE, SD_NO_ARGUMENT ))
    {
        rca = get_short_resp();
        rca = rca >> 16;                
        return rca;
    }
    else
        return ERROR_INVALID_RCA;
}

T_U32 set_rca(T_U16 rca)
{ 
    if (send_cmd( SD_CMD(3), SD_SHORT_RESPONSE, rca<<16 ))
    {              
        return rca;
    }
    else
        return ERROR_INVALID_RCA;
}


/**
 * @brief Slect or reject a mmc or sd card.
 *
 * Send CMD7 to select a sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param rca[in] The selected card relative address 
 * @return T_BOOL
 * @retval AK_TRUE Select successful
 * @retval AK_FALSE Select failed
 */
T_BOOL select_card(T_U32 rca)
{
    //deselect card
    if (rca == 0)
    {
        if (send_cmd( SD_CMD(7), SD_NO_RESPONSE, (rca << 16) ))
            return AK_TRUE;         
    }
    //select card
    else
    {
        if (send_cmd( SD_CMD(7), SD_SHORT_RESPONSE, (rca << 16) ))
            return AK_TRUE;           
    }
    return AK_FALSE;
}

/**
 * @brief Init card start
 *
 * Called at the beginning of the init card 
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_BOOL
 * @retval AK_TRUE Start successful
 * @retval AK_FALSE  Start failed
 */
static T_BOOL init_start(T_BOOL bInitMem)
{
    T_U8 i=0;
    T_U32 response=0;
    T_U32 arg_val=0;
    //only init mem,or init all
    if (bInitMem)
    {
        if (!send_cmd( SD_CMD(0), SD_NO_RESPONSE, SD_NO_ARGUMENT ))
        {
            akprintf(C1, M_DRVSYS, "Set the SD CARD idle  failed!\n");  
            return AK_FALSE;
        }
        //NOTE:this delay is necessary ,otherwise re-init will failed while sd is power on for some cards
        mini_delay(3);
        for (i = 0; i<3; i++)
        {
            if (send_cmd( SD_CMD(8), SD_SHORT_RESPONSE, 0x1aa ))
            {                   
                response = get_short_resp();
                if (response != 0x1aa)
                { 
                    g_pCurSdDevice->enmCardType = CARD_UNUSABLE;
                    return AK_FALSE;
                }
                break;
            }
        }
    }
    //only init io
    else
    {
        SDIO_SET_CMD52_ARG(arg_val,CMD52_WRITE,0,0,CCCR_IO_ABORT,(1<<3));
        if(send_cmd(SD_CMD(52), SD_SHORT_RESPONSE, arg_val) == AK_FALSE)
        {
            akprintf(C1, M_DRVSYS, "direct write failed.\n");   
            return AK_FALSE;
        }
    }
    return AK_TRUE;
}

/**
 * @brief Init card finish
 *
 * Called at the end of the init card 
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
static T_eCARD_TYPE init_finish()
{
    T_eCARD_TYPE card_type;
    if(g_pCurSdDevice->bInitIoSuccess)
    {
        card_type = get_card_type();
    }
    else
    {
        card_type = CARD_UNUSABLE;
    }
    return card_type;
}

/**
 * @brief Get card type
 *
 * Called at the end of the init card 
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
static T_eCARD_TYPE get_card_type()
{
    T_U8 type=CARD_UNUSABLE;
    if ((!g_pCurSdDevice->bInitIoSuccess)&&(g_pCurSdDevice->bInitMemSuccess))
    {
        akprintf(C3, M_DRVSYS, "card type :sd card\r\n");
        g_pCurSdDevice->ulRCA = get_rca();
        type = CARD_SD;
    }
    if ((g_pCurSdDevice->bInitIoSuccess)&&(!g_pCurSdDevice->bInitMemSuccess))
    {
        akprintf(C3, M_DRVSYS, "card type :sdio card\r\n");
        g_pCurSdDevice->ulRCA = get_rca();
        type = CARD_SDIO;
    }
    if ((g_pCurSdDevice->bInitIoSuccess)&&(g_pCurSdDevice->bInitMemSuccess))
    {
        akprintf(C3, M_DRVSYS, "card type :combo card\r\n");
        g_pCurSdDevice->ulRCA = get_rca();
        type = CARD_COMBO;
    }
    if ((!g_pCurSdDevice->bInitIoSuccess)&&(!g_pCurSdDevice->bInitMemSuccess))
    {
        akprintf(C3, M_DRVSYS, "card type :mmc card\r\n");
        //card addr is fixed 0x2,but the addr must be different  for supporting multi card 
        g_pCurSdDevice->ulRCA = set_rca(0x2);
        type = CARD_MMC;
    }
    if(ERROR_INVALID_RCA != g_pCurSdDevice->ulRCA)
    {
        return type;
    }
    else
    {
        return CARD_UNUSABLE;
    }
      
}

/**
 * @brief Release card
 *
 * close sd controller and free card device struct, called when init card fail
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_VOID
 */
T_VOID sd_release()
{
    sd_close_controller(g_pCurSdDevice->enmInterface);
    drv_free(g_pCurSdDevice);  
}

