/**@file arch_sd.c
 * @brief Implement arch level operations of how to control sd&sdio.
 *
 * This file implement sd&sdio controller driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "l2.h"
#include "sysctl.h"
#include "sd.h"
#include "hal_sd.h"
#include "hal_sdio.h"
#include "hal_common_sd.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "freq.h"
#include "drv_module.h"

/**
 * @brief structure for  maping to device.
 */
typedef struct
{
    T_eCARD_INTERFACE   card_type;  
    T_U32               gpio_num;   
    T_BOOL              benable_dat3;
    T_BOOL              ent_drv_state;
}T_CARD_USE; 

static T_CARD_USE       card_list[INTERFACE_SDIO+1]= {
{0,0,AK_FALSE,0},
{0,0,AK_FALSE,0},
{0,0,AK_FALSE,0},
{0,0,AK_FALSE,0}}; 
static T_U32 s_SdRegClkCtrl    = SD_CLK_CTRL_REG;          // (clock) control reg
static T_U32 s_SdRegArgument   = SD_ARGUMENT_REG;          // command arg reg
static T_U32 s_SdRegCmd        = SD_CMD_REG;               // command control reg
static T_U32 s_SdRegRespCmd    = SD_RESP_CMD_REG;          // command response reg
static T_U32 s_SdRegResp0      = SD_RESP_REG0;             // response reg 0
static T_U32 s_SdRegResp1      = SD_RESP_REG1;             // response reg 1
static T_U32 s_SdRegResp2      = SD_RESP_REG2;             // response reg 2
static T_U32 s_SdRegResp3      = SD_RESP_REG3;             // response reg 3
static T_U32 s_SdRegDataTim    = SD_DATA_TIM_REG;          // data timer reg
static T_U32 s_SdRegDataLen    = SD_DATA_LEN_REG;          // data length reg
static T_U32 s_SdRegDataCtrl   = SD_DATA_CTRL_REG;         // data control reg
static T_U32 s_SdRegDataCount  = SD_DATA_COUT_REG;         // data remain counter reg
static T_U32 s_SdRegStatus     = SD_INT_STAT_REG;          // command/data status reg
static T_U32 s_SdRegIntEnable  = SD_INT_ENABLE;            // interrupt enable reg
static T_U32 s_SdRegDmaMode    = SD_DMA_MODE_REG;          // data buffer configure reg(0x2002x03C), CPU/DMA mode select
static T_U32 s_SdRegCpuMode    = SD_CPU_MODE_REG;          // data reg(0x2002x040)

static T_U32 s_sdio_clok       = 0;
static T_U32 s_sdmmc_clok      = 0;

static T_eCARD_INTERFACE            s_SdInterface = INTERFACE_SDMMC4;
static DEVICE_SELECT                s_L2Select = ADDR_MCI0;
static E_GPIO_PIN_SHARE_CONFIG      s_PinSelect = ePIN_AS_SDMMC1;

static T_VOID set_data_reg(T_U32 len,T_U32 blk_size,T_U8 bus_mode,T_U8 dir);
static T_BOOL sd_rx_data_cpu(T_U8 buf[], T_U32 len);
static T_BOOL sd_tx_data_cpu(T_U8 buf[], T_U32 len);
static T_BOOL sd_tx_data_cpu(T_U8 buf[], T_U32 len);
static T_BOOL sdio_abort_trans(T_U8 func_nbr);
static T_VOID set_sdmmc_if(T_VOID);
static T_VOID set_sdio_if(T_VOID);

static T_VOID set_sdio_if(T_VOID)
{
    s_SdRegClkCtrl      = SDIO_CLK_CTRL_REG;
    s_SdRegArgument     = SDIO_ARGUMENT_REG;
    s_SdRegCmd          = SDIO_CMD_REG;
    s_SdRegRespCmd      = SDIO_RESP_CMD_REG;
    s_SdRegResp0        = SDIO_RESP_REG0;
    s_SdRegResp1        = SDIO_RESP_REG1;
    s_SdRegResp2        = SDIO_RESP_REG2;
    s_SdRegResp3        = SDIO_RESP_REG3;
    s_SdRegDataTim      = SDIO_DATA_TIM_REG;
    s_SdRegDataLen      = SDIO_DATA_LEN_REG;
    s_SdRegDataCtrl     = SDIO_DATA_CTRL_REG;
    s_SdRegDataCount    = SDIO_DATA_COUT_REG;
    s_SdRegStatus       = SDIO_INT_STAT_REG;
    s_SdRegIntEnable    = SDIO_INT_ENABLE;
    s_SdRegDmaMode      = SDIO_DMA_MODE_REG;
    s_SdRegCpuMode      = SDIO_CPU_MODE_REG;
    
    s_L2Select  = ADDR_MCI1;
    s_PinSelect = ePIN_AS_SDMMC2;    
}

static T_VOID set_sdmmc_if(T_VOID)
{
    s_SdRegClkCtrl      = SD_CLK_CTRL_REG;
    s_SdRegArgument     = SD_ARGUMENT_REG;
    s_SdRegCmd          = SD_CMD_REG;
    s_SdRegRespCmd      = SD_RESP_CMD_REG;
    s_SdRegResp0        = SD_RESP_REG0;
    s_SdRegResp1        = SD_RESP_REG1;
    s_SdRegResp2        = SD_RESP_REG2;
    s_SdRegResp3        = SD_RESP_REG3;
    s_SdRegDataTim      = SD_DATA_TIM_REG;
    s_SdRegDataLen      = SD_DATA_LEN_REG;
    s_SdRegDataCtrl     = SD_DATA_CTRL_REG;
    s_SdRegDataCount    = SD_DATA_COUT_REG;
    s_SdRegStatus       = SD_INT_STAT_REG;
    s_SdRegIntEnable    = SD_INT_ENABLE;
    s_SdRegDmaMode      = SD_DMA_MODE_REG;
    s_SdRegCpuMode      = SD_CPU_MODE_REG;
    
    s_L2Select  = ADDR_MCI0;
    s_PinSelect = ePIN_AS_SDMMC1;
}


/**
 * @brief Set the sd interface.
 *
 * Select the sd interface(INTERFACE_SDMMC4,INTERFACE_SDMMC8 or INTERFACE_SDIO)and select the relevant registers,L2 ,pin.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cif[in] The selected interface,INTERFACE_SDMMC4,INTERFACE_SDMMC8 or INTERFACE_SDIO.
 * @return T_VOID
 */
T_VOID set_interface(T_eCARD_INTERFACE cif)
{
    if(INTERFACE_NOT_SD != cif)
    {
        if(INTERFACE_SDIO == cif)
        {
            set_sdio_if(); 
        }
        else 
        {
            set_sdmmc_if();
        }
        
        s_SdInterface = cif;
        gpio_pin_group_cfg(s_PinSelect);            
    }
}

/**
* @brief enable detect mmc sd or comob card
* @author Yang Xi
* @date 2014-06-11
* @param[in] 
* @return 
*/
T_BOOL card_detect_enable(T_eCARD_INTERFACE card_type,T_BOOL benable)
{
    
    
    if(AK_TRUE == card_list[card_type].benable_dat3)
    {
        if(AK_TRUE == benable)
        {
            gpio_set_pin_as_gpio (card_list[card_type].gpio_num);
            gpio_set_pull_down_r(card_list[card_type].gpio_num, AK_TRUE);
            gpio_set_pin_dir( card_list[card_type].gpio_num, GPIO_DIR_INPUT);

        }
        
    }
    if((INTERFACE_SDMMC4 == card_type)||(INTERFACE_SDMMC8 == card_type))
    {
        return detector_enable("SDMMCC", benable);
    }
    if(INTERFACE_SDIO == card_type)
        return detector_enable("SDIO", benable);
    
    return AK_FALSE;
}

/**
* @brief set state for DRV_PROTECT(interface, true_interface)
* @author Yang Xi
* @date 2014-06-11
* @param[in] 
* @return 
*/
T_BOOL card_set_enter_state(T_eCARD_INTERFACE card_type)
{
    T_BOOL benable = AK_FALSE;
    
    if((INTERFACE_SDMMC4 == card_type)||(INTERFACE_SDMMC8 == card_type))
    {
        detector_is_enabled("SDMMCC", &benable);
    }else if(INTERFACE_SDIO == card_type)
    {
        detector_is_enabled("SDIO", &benable);
    }
    card_list[card_type].ent_drv_state = benable;
    
    return AK_TRUE;
}

/**
* @brief set state for UN_DRV_PROTECT(interface, true_interface)
* @author Yang Xi
* @date 2014-06-11
* @param[in] 
* @return 
*/
T_BOOL card_comeback_detect_state(T_eCARD_INTERFACE card_type)
{
    return card_detect_enable(card_type, card_list[card_type].ent_drv_state);
}

/**
* @brief register detect mmc sd or comob card
* @author Yang Xi
* @date 2014-06-11
* @param[in] 
* @return 
*/
T_BOOL card_detect_reg(T_eCARD_INTERFACE card_type, T_U32 gpio_num, T_BOOL benable_dat3, T_fDETECTOR_CALLBACK pcallbackfunc)
{
    if(INTERFACE_NOT_SD == card_type)
        return AK_FALSE;
        
    card_list[card_type].card_type = card_type;
    card_list[card_type].gpio_num = gpio_num;
    card_list[card_type].benable_dat3 = benable_dat3;
    

    if((INTERFACE_SDMMC4 == card_type)||(INTERFACE_SDMMC8 == card_type))
    {

        if(AK_TRUE == benable_dat3)
        {
            //DAT3 fun pin: gpio13, gpio17, gpio39. !!note: gpio63 is dat3, but has not pull-down function(reference 37c datasheet)
            if((13 != gpio_num) && (17 != gpio_num) && (39 != gpio_num))
                return AK_FALSE;
            gpio_set_pull_down_r(gpio_num, AK_TRUE);
            gpio_set_pin_dir( gpio_num, GPIO_DIR_INPUT);
            //when use DAT3 pin, card exist: AK_TRUE(third param)
            if(AK_FALSE == detector_register_gpio("SDMMCC", gpio_num, AK_TRUE, AK_TRUE, 100))
            {
                return AK_FALSE;
            }
            
        }else{
            //when use DAT3 pin, card exist: AK_TRUE(third param)
            if(AK_FALSE == detector_register_gpio("SDMMCC", gpio_num, AK_FALSE, AK_TRUE, 100))
                return AK_FALSE;
        }  
        
        if(AK_FALSE == detector_set_callback("SDMMCC", pcallbackfunc))
        {
            return AK_FALSE;
        }
    }
    if(INTERFACE_SDIO == card_type)
    {

        if(AK_TRUE == benable_dat3)
        {
            //when use DAT3 pin, card exist: AK_TRUE(third param)
            if(AK_FALSE == detector_register_gpio("SDIO", gpio_num, AK_TRUE, AK_TRUE, 100))
                return AK_FALSE;
            gpio_set_pull_down_r(gpio_num, AK_TRUE);
            gpio_set_pin_dir( gpio_num, GPIO_DIR_INPUT);
        }else{
            //when use DAT3 pin, card exist: AK_TRUE(third param)
            if(AK_FALSE == detector_register_gpio("SDIO", gpio_num, AK_FALSE, AK_TRUE, 100))
                return AK_FALSE;
        }

        if(AK_FALSE == detector_set_callback("SDIO", pcallbackfunc))
            return AK_FALSE;
            
    }
    
    if(AK_TRUE == benable_dat3)
    gpio_set_pull_down_r(card_list[card_type].gpio_num, AK_TRUE);
    
    return AK_TRUE;//card_detect_enable(card_type,AK_TRUE);
    
}

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
T_BOOL card_detector_get_state(T_eCARD_INTERFACE card_type, T_BOOL *pState)
{
    
    if((INTERFACE_SDMMC4 == card_type)||(INTERFACE_SDMMC8 == card_type))
    {
        return detector_get_state("SDMMCC", pState);
    }else if(INTERFACE_SDIO == card_type)
    {
        return detector_get_state("SDIO", pState);
    }
    
    return AK_FALSE;
}


/**
 * @brief       when using data3 of SD/MMC CARD to detetor the card, chang the data3 to data bus
 * @author      kejianping
 * @date        2014.08.06
 * @param[in]   card_type
 *                  card type of the device to be detected.
 * @return      T_VOID
 */ 
T_VOID card_detetor_data3_protect(T_eCARD_INTERFACE card_type)
{
	if(card_list[card_type].benable_dat3)
	{
	    card_set_enter_state(card_type);
	    card_detect_enable(card_type, AK_FALSE);
	}
}


/**
 * @brief       when using data3 of SD/MMC CARD to detetor the card, chang the data3 to GPIO
 * @author      kejianping
 * @date        2014.08.06
 * @param[in]   card_type
 *                  card type of the device to be detected.
 * @return      T_VOID
 */ 
T_VOID card_detetor_data3_unprotect(T_eCARD_INTERFACE card_type)
{
	if(card_list[card_type].benable_dat3)
	{
	    card_comeback_detect_state(card_type);
	}
}


/**
 * @brief Set share pin  for the selected interface
 *
 * Config the share pin for selected interface and set the attributes of the share pin,sush as pull up,IO control. 
 * @author Huang Xin
 * @date 2010-07-14
 * @param cif[in] The selected interface,INTERFACE_SDMMC4,INTERFACE_SDMMC8 or INTERFACE_SDIO.
 * @return T_VOID
 */
T_VOID set_pin(T_eCARD_INTERFACE cif)
{
    T_U32  pin_cmd;

    if(INTERFACE_SDIO == cif)
    {
        pin_cmd = 29;
    }
    else
    {
        pin_cmd = 39;
    }
    gpio_pin_group_cfg(s_PinSelect);
    gpio_set_pin_attribute(pin_cmd, GPIO_ATTR_IE, AK_TRUE);
}


/**
 * @brief Set sd card clock.
 *
 * The clock must be less than 400khz when the sd controller in identification mode.
 * @author Huang Xin
 * @date 2010-07-14
 * @param clk[in] The main clock of sd card.
 * @param asic[in] current asic freq
 * @param pwr_save[in] Set this parameter true to enable power save
 * @return T_VOID
 */
T_VOID set_clock(T_U32 clk, T_U32 asic, T_BOOL pwr_save)
{
    T_U8 clk_div_l,clk_div_h;
    T_U32 asic_freq,reg_value,tmp;

    if(0 == clk)
        return;
        
    asic_freq = asic;
    if (asic_freq < clk*2)
    {
        clk_div_l = clk_div_h = 0;
    }
    else
    {
        // clk = asic / ((clk_div_h+1) + (clk_div_l+1))
        //NOTE:clk_div_h and clk_div_l present high and low level cycle time
        tmp = asic_freq / clk;
        if (asic_freq % clk)
            tmp += 1;
        tmp -= 2;
        clk_div_h = tmp/2;
        clk_div_l = tmp - clk_div_h;
    }
    reg_value = (clk_div_l<<CLK_DIV_L_OFFSET)|(clk_div_h<<CLK_DIV_H_OFFSET) | SD_CLK_ENABLE | FALLING_TRIGGER | SD_INTERFACE_ENABLE;
    if (pwr_save)
        reg_value |= PWR_SAVE_ENABLE;

    REG32(s_SdRegClkCtrl) = reg_value;

    //save clock
    tmp = asic_freq/(clk_div_h+clk_div_l+2);
    if(ePIN_AS_SDMMC2 == s_PinSelect)
    {
        s_sdio_clok = clk;
        akprintf(C3, M_DRVSYS, "asic clk = %d, sdio clk = %d\n",asic_freq, tmp);
    }
    else
    {
        s_sdmmc_clok = clk;
        akprintf(C3, M_DRVSYS, "asic clk = %d, sdmmc clk = %d\n",asic_freq, tmp);
    }
}

T_VOID sd_cfg_buf(T_U8 buf_mode)
{
    T_U32 reg_value=0;
    if (buf_mode == L2_DMA_MODE)
       reg_value = BUF_EN|DMA_EN|(128<<BUF_SIZE_OFFSET);     
    else 
       reg_value = BUF_EN|(128<<BUF_SIZE_OFFSET);                  

    REG32(s_SdRegDmaMode) = reg_value;
}


/**
 * @brief send sd command.
 *
 * The clock must be less than 400khz when the sd controller in identification mode.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cmd_index[in] The command index.
 * @param rsp[in] The command response:no response ,short reponse or long response
 * @param arg[in] The cmd argument.
 * @return T_BOOL
 * @retval  AK_TRUE: CMD sent successfully
 * @retval  AK_FALSE: CMD sent failed

 */
T_BOOL send_cmd( T_U8 cmd_index, T_U8 resp ,T_U32 arg)
{
    T_U32 cmd_value = 0;
    T_U32 status;

    if (cmd_index == SD_CMD(1) || cmd_index == SD_CMD(41) || cmd_index == SD_CMD(5))      //R3 is no crc
    {
        cmd_value = CPSM_ENABLE | ( resp << WAIT_REP_OFFSET) | ( cmd_index << CMD_INDEX_OFFSET) | RSP_CRC_NO_CHK;
    }
    else
    {
        cmd_value = CPSM_ENABLE | ( resp << WAIT_REP_OFFSET ) | ( cmd_index << CMD_INDEX_OFFSET) ;
    }

    REG32(s_SdRegArgument) = arg;
    REG32(s_SdRegCmd) = cmd_value;

    if (SD_NO_RESPONSE == resp)
    {
        while(1)
        {
            status = REG32(s_SdRegStatus);
            if (status & CMD_SENT)
            {
                return AK_TRUE;
            }
        }
    }
    else if ((SD_SHORT_RESPONSE == resp) ||(SD_LONG_RESPONSE == resp))
    {
        while(1)
        {
            status = REG32(s_SdRegStatus);
            if ((status & CMD_TIME_OUT)||(status & CMD_CRC_FAIL))
            {
                akprintf(C1, M_DRVSYS, "send cmd %d error, status = %x\n", cmd_index, status);
                return AK_FALSE;       
            }
            else if (status & CMD_RESP_END)
            {
                return AK_TRUE;
            }
        }
    }
    else
    {
        akprintf(C3, M_DRVSYS, "error requeset!\n");
        return AK_FALSE;
    }                   
}

/**
 * @brief Get sd card  short response.
 *
 * Only get register response0 .
 * @author Huang Xin
 * @date 2010-07-14
 * @return The value of register response0
 */
T_U32 get_short_resp()
{
    T_U32 resp_value=0;
    
    resp_value = REG32(s_SdRegResp0);   
    return resp_value;      
}


/**
 * @brief Get sd card  long response.
 *
 *  Get register response0,1,2,3.
 * @author Huang Xin
 * @date 2010-07-14
 * @param resp[in] The buffer address to save long response
 * @return T_VOID
 */
T_VOID get_long_resp(T_U32 resp[])
{
    T_U32 resp_value=0;
    
    resp[3] = REG32(s_SdRegResp0);
    resp[2] = REG32(s_SdRegResp1);
    resp[1] = REG32(s_SdRegResp2);
    resp[0] = REG32(s_SdRegResp3);         
}


/**
 * @brief set sd controller data register.
 *
 * Set timeout value,transfer size,transfer direction,bus mode,data block len
 * @author Huang Xin
 * @date 2010-07-14
 * @param len[in] Transfer size
 * @param blk_size[in] Block length
 * @param dir[in] transfer direction
 * @return T_VOID
 */
static T_VOID set_data_reg(T_U32 len,T_U32 blk_size,T_U8 bus_mode ,T_U8 dir)
{
    T_U32 reg_value;

    REG32(s_SdRegDataTim) = SD_DAT_MAX_TIMER_V;
    REG32(s_SdRegDataLen) = len;

    reg_value = SD_DATA_CTL_ENABLE | ( dir << SD_DATA_CTL_DIRECTION_OFFSET ) \
                | (bus_mode << SD_DATA_CTL_BUS_MODE_OFFSET) \
                | (blk_size << SD_DATA_CTL_BLOCK_LEN_OFFSET );
   
    REG32(s_SdRegDataCtrl) = reg_value;
 
}


/**
 * @brief SD read or write data use l2 dma
 *
 * start l2 dma
 * @author Huang Xin
 * @date 2010-07-14
 * @param ram_addr[in] the ram address used by dma
 * @param size[in] transfer bytes
 * @param dir[in] transfer direction
 * @return T_BOOL
 * @retval  AK_TRUE: transfer successfully
 * @retval  AK_FALSE: transfer failed
 */
T_BOOL sd_trans_data_dma(T_U32 ram_addr,T_U32 size,T_U8 dir)
{
    T_U32 status,data_left,data_to_trans;
    T_U8 buf_id = BUF_NULL;
    T_BOOL ret;
    
    //alloc L2 buffer
    buf_id = l2_alloc(s_L2Select);
    if(BUF_NULL == buf_id)
    {
        akprintf(C3, M_DRVSYS, "alloc L2 buffer failed!, buf=%d\n", buf_id);
        return AK_FALSE;
    }       

    sd_cfg_buf(L2_DMA_MODE);
    //set data reg
    set_data_reg(size, g_pCurSdDevice->ulDataBlockLen, g_pCurSdDevice->enmBusMode, dir);

    //set l2 trans dir
    if (SD_DATA_CTL_TO_HOST == dir)
        dir = BUF2MEM;
    else
        dir = MEM2BUF;
    
    data_left = size;
    //start l2 dma to transfer data     
    do
    {
        //there is a wired problem when we use SD_DMA_SIZE_32K in the following operation:
        //l2 data trans finish, but sd controller stay in RX_ACTIVE status
        //so we change the length to SD_DMA_SIZE_64K. the reason remains to be explored
        //by lzj
        data_to_trans = (data_left >= SD_DMA_SIZE_64K) ? SD_DMA_SIZE_64K : data_left;
        //NOTE: 2011.06.09 by xc
        // 为解决音频播放papa音问题，需要将所有的L2访问改为cpu方式
        #if 0   
        l2_combuf_dma(ram_addr, buf_id, data_to_trans, dir,AK_FALSE);
        if(!l2_combuf_wait_dma_finish(buf_id))
        {
            goto EXIT;
        }
        #else
        if(!l2_combuf_cpu(ram_addr, buf_id, data_to_trans, dir))
        {
            goto EXIT;
        }
        #endif
        data_left -= data_to_trans;
        ram_addr += data_to_trans;
    }while(data_left>0);

EXIT:    
    while(1)
    {
        status = REG32(s_SdRegStatus);
        if (status & DATA_TIME_OUT)
        {
            akprintf(C3, M_DRVSYS, "timeout, status is %x\n", status);
            ret = AK_FALSE;
            break;
        }
        else if (status & DATA_CRC_FAIL)
        {
            akprintf(C3, M_DRVSYS, "crc error, status is %x\n", status);
            ret = AK_FALSE;
            break;
        }
        else if (BUF2MEM == dir && !(status & RX_ACTIVE))
        {
            ret = AK_TRUE;
            break;
        }
        else if (MEM2BUF == dir && !(status & TX_ACTIVE))
        {
            ret = AK_TRUE;
            break;
        }
    }
    l2_free(s_L2Select);
    return ret;
}

/**
 * @brief SD read or write data use cpu mode
 *
 * @author Huang Xin
 * @date 2010-07-14
 * @param ram_addr[in] the ram address used by dma
 * @param size[in] transfer bytes
 * @param dir[in] transfer direction
 * @return T_BOOL
 * @retval  AK_TRUE: transfer successfully
 * @retval  AK_FALSE: transfer failed
 */
T_BOOL sd_trans_data_cpu(T_U32 ram_addr,T_U32 size,T_U8 dir)
{  
    T_BOOL ret = AK_TRUE;
    
    //set data reg
    set_data_reg(size, g_pCurSdDevice->ulDataBlockLen, g_pCurSdDevice->enmBusMode, dir);
    //receive data
    if (SD_DATA_CTL_TO_HOST == dir)
    {    
        if(!sd_rx_data_cpu((T_U8*)ram_addr, size))
        {
            ret = AK_FALSE;
        }
    }
    //send data
    else
    {
        if(!sd_tx_data_cpu((T_U8*)ram_addr, size))
        {
            ret = AK_FALSE;
        }
    }
    return ret;

}

/**
* @brief sdio read multiply bytes
* @author Huang Xin
* @date 2010-07-14
* @param arg[in] the address of block to be selected to read 
* @param data[in] the pointer of array which will be read from card 
* @param size[in] the size of data which will read from card
* @return T_BOOL
* @retval  AK_TRUE: read  successfully
* @retval  AK_FALSE: read failed
*/
T_BOOL sdio_read_multi_byte(T_U32 arg,T_U8 data[],T_U32 size)
{
    T_U8 func=0;
    func = (arg>>CMD53_FUN_NUM)&0x7;
    //config data register
    set_data_reg(size, g_pCurSdDevice->ulFunBlockLen[func],g_pCurSdDevice->enmBusMode, SD_DATA_CTL_TO_HOST);
    //send cmd 53
    if (!send_cmd( SD_CMD(53), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "read sdio failed.\n");
        return  AK_FALSE; 
    }
    //receive data
    if(!sd_rx_data_cpu(data, size))
    {
        return AK_FALSE;
    }
    return AK_TRUE;
}

/**
* @brief sdio write multiply bytes
* @author Huang Xin
* @date 2010-07-14
* @param arg[in] the argument of cmd53 
* @param data[in] the pointer of array which will be wrote to card 
* @param size[in] the size of data which will wrote to card
* @return T_BOOL
* @retval  AK_TRUE: write  successfully
* @retval  AK_FALSE: write failed
*/
T_BOOL sdio_write_multi_byte(T_U32 arg,T_U8 data[],T_U32 size)
{
    T_U8 func=0;
    func = (arg>>CMD53_FUN_NUM)&0x7;

    //config data register
    set_data_reg(size, g_pCurSdDevice->ulFunBlockLen[func],g_pCurSdDevice->enmBusMode, SD_DATA_CTL_TO_CARD);        
    //send cmd 53
    if (!send_cmd( SD_CMD(53), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "read sdio failed.\n");
        return  AK_FALSE; 
    }
    //send data
    if(!sd_tx_data_cpu(data, size))
    {
        return AK_FALSE;
    }
    return AK_TRUE;
}

/**
* @brief sdio read multiply blocks
* @author Huang Xin
* @date 2010-07-14
* @param arg[in] the argument of cmd53 
* @param data[in] the pointer of array which will be wrote to card 
* @param size[in] the size of data which will read from card
* @return T_BOOL
* @retval  AK_TRUE: read  successfully
* @retval  AK_FALSE: read failed
*/
T_BOOL sdio_read_multi_block(T_U32 arg,T_U8 data[],T_U32 size)
{
    T_U8 func=0;
    func = (arg>>CMD53_FUN_NUM)&0x7;
    //config data register
    set_data_reg(size, g_pCurSdDevice->ulFunBlockLen[func], g_pCurSdDevice->enmBusMode,SD_DATA_CTL_TO_HOST);
    //send cmd 53
    if (!send_cmd( SD_CMD(53), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "read sdio failed.\n");
        return  AK_FALSE; 
    }
    //receive data
    if(!sd_rx_data_cpu(data, size))
    {
        return AK_FALSE;
    }
    if(0 == (arg&CMD53_COUNT_MASK))
    {
        sdio_abort_trans(func);
    }
    return AK_TRUE;
}

/**
* @brief sdio write multiply blocks
* @author Huang Xin
* @date 2010-07-14
* @param arg[in] the argument of cmd53 
* @param data[in] the pointer of array which will be wrote to card 
* @param size[in] the size of data which will wrote to card
* @return T_BOOL
* @retval  AK_TRUE: write  successfully
* @retval  AK_FALSE: write failed
*/
T_BOOL sdio_write_multi_block(T_U32 arg,T_U8 data[],T_U32 size)
{
    //send cmd 53
    T_U8 func=0;
    func = (arg>>CMD53_FUN_NUM)&0x7;
    //config data register
    set_data_reg(size, g_pCurSdDevice->ulFunBlockLen[func], g_pCurSdDevice->enmBusMode,SD_DATA_CTL_TO_CARD);
    
    if (!send_cmd( SD_CMD(53), SD_SHORT_RESPONSE, arg))
    {
        akprintf(C3, M_DRVSYS, "read sdio failed.\n");
        return  AK_FALSE; 
    }
    //receive data
    if(!sd_tx_data_cpu(data, size))
    {
        return AK_FALSE;
    }
    if(0 == (arg&CMD53_COUNT_MASK))
    {
        sdio_abort_trans(func);
    }
    return AK_TRUE;
}

/**
* @brief Abort sdio multi blocks transfer
* @author Huang Xin
* @date 2010-07-14
* @param func_nbr[in] the function number of sdio,0~7
* @return T_BOOL
* @retval  AK_TRUE: abort  successfully
* @retval  AK_FALSE: abort failed
*/
static T_BOOL sdio_abort_trans(T_U8 func_nbr)
{
    T_U8 temp = 0;

    if(!sdio_read_byte(0, CCCR_IO_ABORT, &temp))
    {  
        return AK_FALSE;
    }
    temp &= ~(0x7<<0);
    temp |= (func_nbr<<0);

    if(!sdio_write_byte(0, CCCR_IO_ABORT, temp))
    {  
        return AK_FALSE;
    }
    akprintf(C3, M_DRVSYS, "abort tranfer ok!\n");
    return AK_TRUE;     
}

/**
* @brief sd controller send data ,use cpu mode
* @author Huang Xin
* @date 2010-07-14
* @param buf[in] the pointer of array which will be sent to card 
* @param len[in] the size of data which will be sent to card 
* @return T_BOOL
* @retval  AK_TRUE: send  successfully
* @retval  AK_FALSE: send failed
*/
static T_BOOL sd_tx_data_cpu(T_U8 buf[], T_U32 len)
{
    T_U32 status;
    T_U32 offset = 0;
    
    while(1)
    {
        status = REG32(s_SdRegStatus);
        if ((offset < len) && (status & DATA_BUF_EMPTY))
        {
            REG32(s_SdRegCpuMode) = (buf[offset])|(buf[offset+1]<<8)|(buf[offset+2]<<16)|(buf[offset+3]<<24);
            offset += 4;
        }
        if ((status & DATA_TIME_OUT) || (status & DATA_CRC_FAIL))
        {
            akprintf(C3, M_DRVSYS, "crc error or timeout, status is %x\n",status); 
            return AK_FALSE;
        }
        if (!(status & TX_ACTIVE))
        {
            break;
        }
    }

    return AK_TRUE;
}

/**
* @brief sd controller receive data ,use cpu mode
* @author Huang Xin
* @date 2010-07-14
* @param buf[in] the pointer of array to save received data
* @param len[in] the size of data received
* @return T_BOOL
* @retval  AK_TRUE: receive  successfully
* @retval  AK_FALSE: receive failed
*/
static T_BOOL sd_rx_data_cpu(T_U8 buf[], T_U32 len)
{
    T_U32 status;
    T_U32 buf_tmp;
    T_U32 i;
    T_U32 offset, size;

    offset = 0;
    size = len;
    while(1)
    {
        status = REG32(s_SdRegStatus);
        if ((status & DATA_BUF_FULL))
        {
            buf_tmp  = REG32(s_SdRegCpuMode );   
            for (i = 0; i < 4; i++)
            {
                buf[offset + i] = (T_U8)((buf_tmp >> (i * 8)) & 0xff);
            }
            offset += 4;
            size -= 4;
        }
        if ((size > 0) && (size < 4) && (status & DATA_END))
        {
            buf_tmp = REG32(s_SdRegCpuMode );                
            for ( i = 0; i < size; i++)
            {
                buf[offset + i] = (T_U8)((buf_tmp >> (i * 8)) & 0xff);
            }
            size = 0;           
        }
        if ((status & DATA_TIME_OUT) || (status & DATA_CRC_FAIL))
        {
            akprintf(C3, M_DRVSYS, "crc error or timeout, status is %x\n", status); 
            return AK_FALSE;
        }
        if (!(status & RX_ACTIVE))
        {
            break;
        }
    }

    return AK_TRUE;
}

/**
* @brief Check if sd controller is transferring now
* @author Huang Xin
* @date 2010-07-14
* @return T_BOOL
* @retval  AK_TRUE: sd controller is transferring
* @retval  AK_FALSE: sd controller is not transferring
*/
T_BOOL sd_trans_busy()
{
    T_U32 status;
    status = REG32(s_SdRegStatus);
    if ((status & TX_ACTIVE) || (status & RX_ACTIVE))
    {
        akprintf(C3, M_DRVSYS, " The sd card is writing and reading: %x.\n", status);
        return  AK_TRUE;
    }
    else 
    {
        return AK_FALSE;
    }
}

/**
 * @brief change sd timing when Freq has changed
 *
 * @author liaozhijun
 * @date 2011-6-23
 * @param[in] Freq frequency
 * @return  T_VOID
 */
T_VOID sd_changetiming(T_U32 freq)
{
    DrvModule_Protect(DRV_MODULE_SDMMC);
    
    //check sdio interface
    if(REG32(SDIO_CLK_CTRL_REG) & (1<<20))
    {
        set_sdio_if();
        set_clock(s_sdio_clok, freq, AK_TRUE);
    }

    //check sdmmc interface
    if(REG32(SD_CLK_CTRL_REG) & (1<<20))
    {
        set_sdmmc_if();
        set_clock(s_sdmmc_clok, freq, AK_TRUE);
    }
    
    DrvModule_UnProtect(DRV_MODULE_SDMMC);
}

static int sd_reg(void)
{
    freq_register(E_ASIC_CALLBACK, sd_changetiming);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(sd_reg)
#ifdef __CC_ARM
#pragma arm section
#endif


