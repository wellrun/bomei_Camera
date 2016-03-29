/**@file  hal_skmmc.c
 * @brief Implement sd operations of how to control sd.
 *
 * This file implement sd driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "l2.h"
#include "hal_Print.h"
#include "sysctl.h"
#include "sd.h"
#include "hal_sd.h"
#include "hal_sk_mmc.h"
#include "hal_common_sd.h"
#include "drv_api.h"
#include "drv_module.h"


//The interface shall be INTERFACE_SDMMC4,INTERFACE_SDMMC8,INTERFACE_SDIO
#define SD_DRV_PROTECT(interface) \
        do{ \
            DrvModule_Protect(DRV_MODULE_SDMMC);\
            set_interface(interface);\
            card_set_enter_state(interface);\
            card_detect_enable(interface, AK_FALSE);\
        }while(0)

//The interface shall be INTERFACE_NOT_SD
#define SD_DRV_UNPROTECT(interface, true_interface) \
        do{ \
            set_interface(interface);\
            DrvModule_UnProtect(DRV_MODULE_SDMMC);\
            card_comeback_detect_state(true_interface);\
        }while(0)



T_SKMMC s_tSKMMC = {0};

static T_VOID sk_mmc_power_off_on(T_VOID);
static T_BOOL sk_mmc_init_card(T_VOID);
static T_BOOL sk_mmc_validation_proc(T_VOID);
static T_BOOL sk_mmc_low_level_proc(T_VOID);
static T_BOOL sk_mmc_verify_proc(T_VOID);
static T_BOOL sk_mmc_rw_block(T_U32 blk_num, T_U8 *databuf, T_U32 size, T_U32 mode,T_U8 dir);
static T_BOOL sk_mmc_set_cid(T_VOID);
static T_VOID sk_mmc_char_reverse(T_U8  *s, T_U32 len);


/**
* @brief sk open card
* @author Huang Xin
* @date 2010-06-17
* @param cif[in] card interface selected
* @param bus_mode[in] bus mode selected, can be USE_ONE_BUS or USE_FOUR_BUS
* @return T_pCARD_HANDLE
* @retval NON-NULL  set initial successful,card type is  mmc sd or comob
* @retval NULL set initial fail,card type is not mmc sd or comob card
*/
T_pCARD_HANDLE sk_mmc_open_card(T_eCARD_INTERFACE cif, T_U8 bus_mode)
{
    T_pSD_DEVICE pSdCard = AK_NULL;
    T_U32 i, mmc_spec_version;

    if (INTERFACE_SDMMC4 != cif && INTERFACE_SDMMC8 != cif && INTERFACE_SDIO !=cif)
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_open_card():interface is invalid\n");
        return AK_NULL;
    }
    if (USE_ONE_BUS != bus_mode && USE_FOUR_BUS != bus_mode && USE_EIGHT_BUS != bus_mode)
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_open_card():bus_mode is invalid\n");
        return AK_NULL;
    }
    SD_DRV_PROTECT(cif);
    
    pSdCard = (T_pSD_DEVICE)drv_malloc(sizeof(T_SD_DEVICE));
    if (AK_NULL == pSdCard)
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_open_card():drv_malloc fail\n");
        SD_DRV_UNPROTECT(INTERFACE_NOT_SD, cif); 
        return AK_NULL;
    }
    g_pCurSdDevice = pSdCard;
    memset(g_pCurSdDevice,0,sizeof(T_SD_DEVICE));
    g_pCurSdDevice->enmInterface = cif;
    g_pCurSdDevice->ulVolt = SD_DEFAULT_VOLTAGE;

    g_pCurSdDevice->ulDataBlockLen = SD_DEFAULT_BLOCK_LEN; 
    g_pCurSdDevice->ucSpecVersion = 4;
    //enable sd controller
    sd_open_controller(cif);
    //init card
    if(!sk_mmc_init_card())
    {
        goto ERR_EXIT;
    }
    //validation
    if(!sk_mmc_validation_proc())
    {
        goto ERR_EXIT;
    }
    //low level format
    if(!sk_mmc_low_level_proc())
    {
        goto ERR_EXIT;
    }
    //verify
    if(!sk_mmc_verify_proc())
    {
        goto ERR_EXIT;
    }
    //set cid
    if(!sk_mmc_set_cid())
    {
        goto ERR_EXIT;
    }
    if (!memcmp(((T_U8*)(g_pCurSdDevice->ulCID))+1,((T_U8*)s_tSKMMC.ulCID) + 1,15))
    {
        s_tSKMMC.bOpenOK = AK_TRUE;
        akprintf(C1, M_DRVSYS, "sk_mmc_open_card():open card success\n");
    }
    else
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_open_card():open card fail\n");
        akprintf(C1, M_DRVSYS, "cid[%x %x %x %x]\n",g_pCurSdDevice->ulCID[0],g_pCurSdDevice->ulCID[1],g_pCurSdDevice->ulCID[2],g_pCurSdDevice->ulCID[3]);
        goto ERR_EXIT;
    }

    SD_DRV_UNPROTECT(INTERFACE_NOT_SD, cif);
    //NOTE:g_pCurSdDevice may be changed by other process at this time, "return g_pCurSdDevice" is forbidden
    return (T_pCARD_HANDLE)pSdCard;  
ERR_EXIT:
    sd_release();
    SD_DRV_UNPROTECT(INTERFACE_NOT_SD, cif);
    return AK_NULL;
    
}

T_BOOL sk_mmc_init(T_SKMMCBIN erase_bin,T_SKMMCBIN llf1_bin,T_SKMMCBIN llf2_bin,T_U8* llf_param,T_U8*  fdm_bin)
{
    if ((AK_NULL == erase_bin)||(AK_NULL == llf1_bin)||(AK_NULL == llf2_bin)||(AK_NULL == llf_param)||(AK_NULL == fdm_bin))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_init(): param bin null\n");
        return AK_FALSE;
    }
    s_tSKMMC.pEraseBin = (T_SKMMCBIN)erase_bin;
    s_tSKMMC.pLlf1Bin = (T_SKMMCBIN)llf1_bin;
    s_tSKMMC.pLlf2Bin = (T_SKMMCBIN)llf2_bin;
    s_tSKMMC.pLlfParam = llf_param;
    s_tSKMMC.pFdmBin =  fdm_bin;
    s_tSKMMC.bOpenOK = AK_FALSE;
    //default cid after open  card
    s_tSKMMC.ulCID[0] = 0x00008d2a;
    s_tSKMMC.ulCID[1] = 0x20000000;
    s_tSKMMC.ulCID[2] = 0x20202020;
    s_tSKMMC.ulCID[3] = 0x11000020;
    return AK_TRUE;
}

/**
* @brief init sk mmc validation proc
* @author Huang Xin
* @date 2010-06-17
* @param N/A
* @return T_BOOL
* @retval AK_TRUE validation success
* @retval AK_FALSE validation fail
*/
static T_BOOL sk_mmc_validation_proc(T_VOID)
{
    T_U32 buf[128] = {0};

    //step0
    if (!send_cmd(SD_CMD(60), SD_SHORT_RESPONSE, 0x55 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step0 fail\n");
        goto ERR_EXIT;
    }
    //step1-2
    if(!sk_mmc_rw_block(0x68, (T_U8 *)buf,512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_HOST))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step1-2 fail\n");
        goto ERR_EXIT;
    }
    //step3
    else
    {
        akprintf(C1, M_DRVSYS, "HW_VERSION:0x%x\n",buf[0]&0xffff);
    }
    //step4-5
    if(!sk_mmc_rw_block(0x67, (T_U8 *)buf,512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_HOST))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step4-5 fail\n");
        goto ERR_EXIT;
    }
    //step6
    else
    {
        akprintf(C1, M_DRVSYS, "FW_VERSION:0x%x\n",buf[0]&0xffff);
    }
    //step7
    if(!sk_mmc_rw_block(0x65, (T_U8 *)buf,512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_HOST))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step7 fail\n");
        goto ERR_EXIT;
    }
    //step9
    else
    {
        akprintf(C1, M_DRVSYS, "FLASH ID: 0x%x\n",buf[0]);
    }
    //step10
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0xaa ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step10 fail\n");
        goto ERR_EXIT;
    }
    //step11 :set 8bit  bus width
    if(!sd_set_bus_width(USE_EIGHT_BUS))
    {
        akprintf(C3, M_DRVSYS, "set bus mode fail !\n");
    }
    //step13
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x55))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step13 fail\n");
        goto ERR_EXIT;
    }
    //step14-15
    if(!sk_mmc_rw_block(0x65, (T_U8 *)buf,512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_HOST))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step14-15 fail\n");
        goto ERR_EXIT;
    }
    //step16
    else
    {
        akprintf(C1, M_DRVSYS, "FLASH ID: 0x%x\n",buf[0]);
    }
    //step 17
    sk_mmc_power_off_on();

    if (!sk_mmc_init_card())
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step17 fail\n");
        goto ERR_EXIT;
    }
    return AK_TRUE;
ERR_EXIT:

    return AK_FALSE;
        
}
T_U8 fdm_bin[65536]={0};

/**
* @brief init sk mmc low level proc
* @author Huang Xin
* @date 2010-06-17
*
* @param N/A
* @return T_BOOL
* @retval AK_TRUE success
* @retval AK_FALSE error occured
*/
static T_BOOL sk_mmc_low_level_proc(T_VOID)
{
    T_U32  i;
    T_U32 tmp_bin[128] = {0};
    
    //step0
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x55 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step0 fail\n");
        goto ERR_EXIT;
    }
    //step1
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x57 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step1 fail\n");
        goto ERR_EXIT;
    }
    //step2-11
    for ( i = 0; i<16; i++)
    {
        if(!sk_mmc_rw_block((0x80000001|(i<<24)), *(s_tSKMMC.pEraseBin + i),512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_CARD))
        {
            akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step2-11 fail\n");
            goto ERR_EXIT;
        }
    }
    //step12
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x56 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step12 fail\n");
        goto ERR_EXIT;
    }
    //step13-14
    if(!sk_mmc_rw_block(0xe0000001, s_tSKMMC.pLlfParam,512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_CARD))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step13-14 fail\n");
        goto ERR_EXIT;
    }

    //step15
    if (!send_cmd( SD_CMD(38), SD_SHORT_RESPONSE, 0xaa))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step15-1 fail\n");
        goto ERR_EXIT;
    }
    mini_delay(500);
    if (!wait_rw_complete())
    {
         akprintf(C3, M_DRVSYS, "sk_mmc_low_level_proc(): step15-2 fail!\n");
         goto ERR_EXIT;
    }
    if (!send_cmd( SD_CMD(38), SD_SHORT_RESPONSE, 0))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step15-3 fail\n");
        goto ERR_EXIT;
    }
    
    akprintf(C1, M_DRVSYS, "erase...\n");
    mini_delay(15000);
    //step16
    while(!wait_rw_complete())
    {
         akprintf(C3, M_DRVSYS, "sk_mmc_low_level_proc(): step16 fail!\n");
         goto ERR_EXIT;
    }

    //step17
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x57))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step17 fail\n");
        goto ERR_EXIT;
    }
    //step 18 
    sk_mmc_power_off_on();
    if (!sk_mmc_init_card())
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step18 fail\n");
        goto ERR_EXIT;
    }
    //step19
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x55))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step19 fail\n");
        goto ERR_EXIT;
    }
    //step20
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x57))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step20 fail\n");
        goto ERR_EXIT;
    }

    //step21-30
    for ( i = 0; i<16; i++)
    {
        if(!sk_mmc_rw_block((0x80000001|(i<<24)), *(s_tSKMMC.pLlf1Bin + i),512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_CARD))
        {
            akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step21-30 fail\n");
            goto ERR_EXIT;
        }
    }
    //step31
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x56 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step31 fail\n");
        goto ERR_EXIT;
    }
    //step32-33
    if(!sk_mmc_rw_block(0xe0000001, s_tSKMMC.pLlfParam,512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_CARD))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step32-33 fail\n");
        goto ERR_EXIT;
    }
    //step34-35
    if(!sk_mmc_rw_block(0x72,(T_U8*)tmp_bin,512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_HOST))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step34 fail\n");
        goto ERR_EXIT;
    }
    //step36
    else
    {
        akprintf(C1, M_DRVSYS, "step36 tmp_bin:0x%x\n",tmp_bin[0]&0xffff);
    }

    //step37
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x57))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step37 fail\n");
        goto ERR_EXIT;
    }

    //step38~47
    for ( i = 0; i<16; i++)
    {
        if(!sk_mmc_rw_block((0x80000001|(i<<24)), *(s_tSKMMC.pLlf2Bin + i), 512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_CARD))
        {
            akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step38-47 fail\n");
            goto ERR_EXIT;
        }  
    }
    //step48
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x56 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step48 fail\n");
        goto ERR_EXIT;
    }
    //step49-50
    if(!sk_mmc_rw_block(0xe0000001, s_tSKMMC.pLlfParam, 512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_CARD))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step49-50 fail\n");
        goto ERR_EXIT;
    }  
    //step51-52
    if(!sk_mmc_rw_block(0x72, (T_U8 *)tmp_bin,512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_HOST))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step51-52 fail\n");
        goto ERR_EXIT;
    }
    //step53
    else
    {
        akprintf(C1, M_DRVSYS, "step53 tmp_bin:0x%x\n",tmp_bin[0]&0xffff);
    }
    //step54
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x57 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step54 fail\n");
        goto ERR_EXIT;
    }
    //step55~64
    for ( i = 0; i<16; i++)
    {
        if(!sk_mmc_rw_block((0x80000001|(i<<24)), *(s_tSKMMC.pEraseBin + i),512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_CARD))
        {
            akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step55-64 fail\n");
            goto ERR_EXIT;
        }

    }
    //step65
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x56 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step65 fail\n");
        goto ERR_EXIT;
    }
    //step66
    if (!send_cmd( SD_CMD(61), SD_SHORT_RESPONSE, 0x600e4224))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_low_level_proc(): step66 fail\n");
        goto ERR_EXIT;
    }
    //step67-69
    if(!sk_mmc_rw_block(0xffffff10, s_tSKMMC.pFdmBin,65536, SD_DATA_MODE_MULTI,SD_DATA_CTL_TO_CARD))
    {
        akprintf(C3, M_DRVSYS, "sk_mmc_low_level_proc(): step67-69 fail\n");
        goto ERR_EXIT;
    }
    //step70-72
    if(!sk_mmc_rw_block(0xffffff10, fdm_bin,65536, SD_DATA_MODE_MULTI,SD_DATA_CTL_TO_HOST))
    {
        akprintf(C3, M_DRVSYS, "sk_mmc_low_level_proc(): step70-72 fail\n");
        goto ERR_EXIT;
    }
    else
    {
        if( memcmp(s_tSKMMC.pFdmBin,fdm_bin,65536))
        {
            akprintf(C3, M_DRVSYS, "sk_mmc_low_level_proc(): step70-72 error\n");
            goto ERR_EXIT;
        }
    }
    //step73-75
    if(!sk_mmc_rw_block(0xffffff20, s_tSKMMC.pFdmBin,65536, SD_DATA_MODE_MULTI,SD_DATA_CTL_TO_CARD))
    {
         akprintf(C3, M_DRVSYS, "sk_mmc_low_level_proc(): step73-75 fail\n");
         goto ERR_EXIT;
    }
    //step76-78
    if(!sk_mmc_rw_block(0xffffff20, fdm_bin,65536, SD_DATA_MODE_MULTI,SD_DATA_CTL_TO_HOST))
    {
         akprintf(C3, M_DRVSYS, "sk_mmc_low_level_proc(): step76-78 fail\n");
         goto ERR_EXIT;
    }
    else
    {
        if( memcmp(s_tSKMMC.pFdmBin,fdm_bin,65536))
        {
            akprintf(C3, M_DRVSYS, "sk_mmc_low_level_proc(): step76-78 error\n");
            goto ERR_EXIT;
        }
    }
    //step79

    sk_mmc_power_off_on();

    if (!sk_mmc_init_card())
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step79 fail\n");
        goto ERR_EXIT;
    }
    return AK_TRUE;
    
ERR_EXIT:
        return AK_FALSE;

}

static T_BOOL sk_mmc_verify_proc(T_VOID)
{
    T_U32 buf[128] = {0};
    T_U32 i = 0;

    //step1
    if (!send_cmd(SD_CMD(60), SD_SHORT_RESPONSE, 0x55 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_verify_proc(): step1 fail\n");
        goto ERR_EXIT;
    }
    //step2
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x57 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_verify_proc(): step2 fail\n");
        goto ERR_EXIT;
    }
    //step3-12
    for ( i = 0; i<16; i++)
    {
        if(!sk_mmc_rw_block((0x80000001|(i<<24)), *(s_tSKMMC.pEraseBin + i),512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_CARD))
        {
            akprintf(C1, M_DRVSYS, "sk_mmc_verify_proc(): step3-12 fail\n");
            goto ERR_EXIT;
        }
    }
    //step13
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x56 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_verify_proc(): step13 fail\n");
        goto ERR_EXIT;
    }
    //step14
    if (!send_cmd( SD_CMD(61), SD_SHORT_RESPONSE, 0x600e4224))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_verify_proc(): step14 fail\n");
        goto ERR_EXIT;
    }
    //step15
    //set bOpenOK ,bHighCapacity temporarily  just for using cmd17 in func sk_mmc_rw_block()
    s_tSKMMC.bOpenOK = g_pCurSdDevice->bHighCapacity = 1;
    if(!sk_mmc_rw_block(0xffffff43, (T_U8 *)buf, 512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_HOST))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_verify_proc(): step15 fail\n");
        s_tSKMMC.bOpenOK = g_pCurSdDevice->bHighCapacity = 0;
        goto ERR_EXIT;
    }
    //step16
    else
    {
        akprintf(C1, M_DRVSYS, "fw_patch version: %x\n",buf[127]);
        s_tSKMMC.bOpenOK = g_pCurSdDevice->bHighCapacity = 0;
    }

    sk_mmc_power_off_on();

    if (!sk_mmc_init_card())
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_validation_proc(): step16 fail\n");
        goto ERR_EXIT;
    }
    return AK_TRUE;
ERR_EXIT:

    return AK_FALSE;
        
}

static T_VOID sk_mmc_power_off_on(T_VOID)
{
    akprintf(C3, M_DRVSYS, "power off card and restart...\n");
    getch();
}

/**
* @brief init sk mmc card
* @author Huang Xin
* @date 2010-06-17
* @param N/A
* @return T_BOOL
* @retval AK_TRUE init success
* @retval AK_FALSE init fail
*/
static T_BOOL sk_mmc_init_card(T_VOID)
{
    //card enumeration
    g_pCurSdDevice->enmCardType = init_card(0, 1);

    if ((CARD_UNUSABLE == g_pCurSdDevice->enmCardType) || 
         (CARD_SDIO == g_pCurSdDevice->enmCardType))
    {
        goto ERR_EXIT;
    }
    if (!select_card(g_pCurSdDevice->ulRCA))
    {
        akprintf(C1, M_DRVSYS, "select card fail !\n");
        goto ERR_EXIT;
    }
    if (!wait_rw_complete())
    {
        akprintf(C1, M_DRVSYS, "check card status fail \n");
        goto ERR_EXIT;
    }
    if(!sd_set_bus_width(USE_ONE_BUS))
    {
        akprintf(C3, M_DRVSYS, "set bus mode fail !\n");
    }
    return AK_TRUE;
 ERR_EXIT:
        return AK_FALSE;
        
}

/**
* @brief set cid of sk mmc card
* @author Huang Xin
* @date 2010-06-17
* @param N/A
* @return T_BOOL
* @retval AK_TRUE set  success
* @retval AK_FALSE set fail
*/
static T_BOOL sk_mmc_set_cid(T_VOID)
{
    T_U32 i = 0;
    T_U32 cid_bin[128] = {0};

    //define cid,  bit0 must be 1,otherwise set cid will fail
    s_tSKMMC.ulCID[0] = 0x04030201;
    s_tSKMMC.ulCID[1] = 0x08070605;
    s_tSKMMC.ulCID[2] = 0x0C0B0A09;
    s_tSKMMC.ulCID[3] = 0x000F0E0D;

    memcpy((T_U8*)cid_bin, (T_U8 *)s_tSKMMC.ulCID, 16);
    //sk need reverse cid before send the cid block
    sk_mmc_char_reverse((T_U8*)cid_bin,16);
    //step1
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x55 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_set_cid(): step1 fail\n");
        goto ERR_EXIT;
    }
    //step2
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x57 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_set_cid(): step2 fail\n");
        goto ERR_EXIT;
    }
    //step3-12
    for ( i = 0; i<16; i++)
    {
        if(!sk_mmc_rw_block((0x80000001|(i<<24)), *(s_tSKMMC.pEraseBin + i),512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_CARD))
        {
            akprintf(C1, M_DRVSYS, "sk_mmc_set_cid(): step3-12 fail\n");
            goto ERR_EXIT;
        }
    }
    //step13
    if (!send_cmd( SD_CMD(60), SD_SHORT_RESPONSE, 0x56 ))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_set_cid(): step13 fail\n");
        goto ERR_EXIT;
    }
    //step14
    if (!send_cmd( SD_CMD(35), SD_SHORT_RESPONSE, SD_NO_ARGUMENT))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_set_cid(): step14 fail\n");
        goto ERR_EXIT;
    }
    //step15-16
    if(!sk_mmc_rw_block(0x80000001, (T_U8 *)cid_bin, 512, SD_DATA_MODE_SINGLE,SD_DATA_CTL_TO_CARD))
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_set_cid(): step15-16 fail\n");
        goto ERR_EXIT;
    }
    //step17
    sk_mmc_power_off_on();
    if (!sk_mmc_init_card())
    {
        akprintf(C1, M_DRVSYS, "sk_mmc_set_cid(): step17 fail\n");
        goto ERR_EXIT;
    }
    return AK_TRUE;
ERR_EXIT:
    return AK_FALSE;
    
}

/**
* @brief sk rw block
* @author Huang Xin
* @date 2010-06-17
* @param blk_num[in] address of block started to rw
* @param databuf[in] data address
* @param size[in] data size
* @param mode[in] single or multi block
* @param dir[in] read or write
* @return T_BOOL
* @retval AK_TRUE rw success
* @retval AK_FALSE rw fail
*/
static T_BOOL sk_mmc_rw_block(T_U32 blk_num, T_U8 *databuf, T_U32 size, T_U32 mode,T_U8 dir)
{       
    T_BOOL ret = AK_FALSE;
    T_U32 cmd;
    T_U32 ram_addr = (T_U32)databuf;

    //Data address is in byte units in a standard capacity sd card and in block(512byte) units in a high capacity sd card
    if ((!g_pCurSdDevice->bHighCapacity) && s_tSKMMC.bOpenOK)        //not HC card
    {
        blk_num<<= 9;
    }

    if(SD_DATA_MODE_SINGLE == mode)
    {
        if (SD_DATA_CTL_TO_HOST == dir)
            cmd = s_tSKMMC.bOpenOK ? SD_CMD(17): SD_CMD(8);
        else 
            cmd = s_tSKMMC.bOpenOK ? SD_CMD(24): SD_CMD(23);

    }
    else
    {
        if (SD_DATA_CTL_TO_HOST == dir)
            cmd = SD_CMD(18);
        else
            cmd = SD_CMD(25);

    }
    //step1: check bus busy
    if( sd_trans_busy())
    {
        return AK_FALSE;
    }
    
    //step2: send card command
    if (send_cmd(cmd, SD_SHORT_RESPONSE, blk_num ) == AK_FALSE )
    {
        akprintf(C1, M_DRVSYS, "block rw command %d is failed!\n", cmd);
        return AK_FALSE;
    }
    //step3: transfer data
    ret = sd_trans_data_dma(ram_addr,size,dir);
    
    //step4: send cmd12 if multi-block operation
    if (mode != SD_DATA_MODE_SINGLE)
    {
        if (!send_cmd( SD_CMD(12), SD_SHORT_RESPONSE, 0 ))
        {
            akprintf(C1, M_DRVSYS, "The stop read multi block command failed!\n");
            ret = AK_FALSE;
        }
    }
    
    //step5: wait card status to idle
    if (!wait_rw_complete())
    {
        akprintf(C1, M_DRVSYS, "sd card program failed!\n");
        ret = AK_FALSE;
    }
    return ret;
}

static T_VOID  sk_mmc_char_reverse(T_U8  *s, T_U32 len)   
{ 
    T_U8   *d; 
    T_U32   i; 
    
    d =  s + len -  1;  
    while(s   <   d)   
    { 
        i = *s; 
        *s++ = *d; 
        *d-- = i; 
    } 
} 


