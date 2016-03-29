/**@file hal_sdio.c
 * @brief Implement sdio operations of how to control sdio.
 *
 * This file implement sdio driver.
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
#include "hal_sdio.h"
#include "hal_common_sd.h"
#include "drv_api.h"
#include "drv_module.h"


//The interface shall be INTERFACE_SDIO
#define SDIO_DRV_PROTECT(interface) \
        do{ \
            DrvModule_Protect(DRV_MODULE_SDIO);\
            set_interface(interface);\
            card_set_enter_state(interface);\
            card_detect_enable(interface, AK_FALSE);\
        }while(0)

//The interface shall be INTERFACE_NOT_SD
#define SDIO_DRV_UNPROTECT(interface, true_interface) \
        do{ \
            set_interface(interface);\
            DrvModule_UnProtect(DRV_MODULE_SDIO);\
            card_comeback_detect_state(true_interface);\
        }while(0)


static T_pSD_DEVICE s_pSdioCard = AK_NULL;

static T_BOOL sdio_set_bus_width(T_U8 bus_mode);
static T_U8   sdio_get_ocr(T_U32 *ocr );
static T_U8   sdio_nego_volt(T_U32 volt);

 /**
* @brief initial sdio or combo card
* @author Huang Xin
* @date 2010-06-17
* @param bus_mode[in] bus mode selected, can be USE_ONE_BUS or USE_FOUR_BUS
* @return T_BOOL
* @retval AK_TRUE  set initial successful, card type is sdio or combo
* @retval AK_FALSE set initial fail,card type is not sdio or combo
*/
T_BOOL sdio_initial(T_U8 bus_mode)
{
    T_pSD_DEVICE pSdCard = AK_NULL;
    
    SDIO_DRV_PROTECT(INTERFACE_SDIO);
    pSdCard = (T_pSD_DEVICE)drv_malloc(sizeof(T_SD_DEVICE));
    if (AK_NULL == pSdCard)
    {
        akprintf(C1, M_DRVSYS, "drv_malloc fail\n");
        SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
        return AK_FALSE;
    }
    g_pCurSdDevice = pSdCard;
    memset(g_pCurSdDevice,0,sizeof(T_SD_DEVICE));
    g_pCurSdDevice->enmInterface = INTERFACE_SDIO;
    g_pCurSdDevice->ulVolt = SD_DEFAULT_VOLTAGE;  
    sd_open_controller(INTERFACE_SDIO);
    mini_delay(25); 
    g_pCurSdDevice->enmCardType = init_card(AK_TRUE,AK_TRUE);  
    set_clock(SD_TRANSFER_MODE_CLK, get_asic_freq(), SD_POWER_SAVE_ENABLE);  
    if ((CARD_UNUSABLE== g_pCurSdDevice->enmCardType) || (CARD_SD == g_pCurSdDevice->enmCardType)||(CARD_MMC == g_pCurSdDevice->enmCardType))
    {
        sd_release(g_pCurSdDevice);
        SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
        return AK_FALSE;
    }
    else 
    {
        if (!select_card(g_pCurSdDevice->ulRCA))
        {
            akprintf(C3, M_DRVSYS, "select card fail !\n");
            sd_release(g_pCurSdDevice);
            SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO); 
            return AK_FALSE;
        }

        if(!sdio_set_bus_width(bus_mode))
        {
             akprintf(C3, M_DRVSYS, "set bus mode fail !\n");
             sd_release(g_pCurSdDevice);
             SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
             return AK_FALSE;
        }

        if(!sdio_set_block_len(0,SD_DEFAULT_BLOCK_LEN))
        {
             akprintf(C3, M_DRVSYS, "set bus mode fail !\n");
             sd_release(g_pCurSdDevice);
             SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
             return AK_FALSE;
        }
        
    }
    sd_cfg_buf(INNER_BUF_MODE);

    s_pSdioCard = g_pCurSdDevice;
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
    return AK_TRUE;
}

/**
 * @brief enable specifical fuction in sdio card
 * @author Huang Xin
 * @date 2010-06-17
 * @param func[in] function to enable
 * @return T_BOOL
 * @retval AK_TRUE enable successfully
 * @retval AK_FALSE enable failed
 */
T_BOOL sdio_enable_func(T_U8 func)
{
    T_U8 temp = 0;
    SDIO_DRV_PROTECT(INTERFACE_SDIO);
    g_pCurSdDevice = s_pSdioCard;
    if(!sdio_write_byte(0, CCCR_IO_ENABLE, (1<<func)))
    {
        SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO); 
        return AK_FALSE;
    }
    while(!(temp & (1<<func)))
    {
        if(!sdio_read_byte(0, CCCR_IO_READY, &temp))
        {   
            SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
            return AK_FALSE;
        }
    }
        
    akprintf(C3, M_DRVSYS, "set function enable ok!\n");
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
    return AK_TRUE; 
}
 

/**
* @brief set block length to sdio card
* @author Huang Xin
* @date 2010-06-17
* @param func[in] function to set block length
* @param block_len[in]  block length to set
* @return T_BOOL
* @retval AK_TRUE enable successfully
* @retval AK_FALSE enable failed
*/
T_BOOL sdio_set_block_len(T_U8 func, T_U32 block_len)
{    
    if(func > 7 || block_len > SDIO_MAX_BLOCK_LEN)
    {
        return AK_FALSE;
    }   
    SDIO_DRV_PROTECT(INTERFACE_SDIO); 
    g_pCurSdDevice = s_pSdioCard;
    //block len is stored in  LSB first
    if(!sdio_write_byte(0,(func*0x100+ CCCR_FN0_BLOCK_SIZE+1), (block_len/256)))
    {
        SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO); 
        return AK_FALSE;
    }
        
    if(!sdio_write_byte(0, (func*0x100+ CCCR_FN0_BLOCK_SIZE), (block_len%256)))
    {
        SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO); 
        return AK_FALSE;
    }
    g_pCurSdDevice->ulFunBlockLen[func] = block_len;

    akprintf(C3, M_DRVSYS, "set block length ok!\n");
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
    return AK_TRUE;
}

/**
* @brief  set sdio interrupt callback function
* @author Huang Xin
* @date 2010-06-17
* @param func[in] callback function
* @return T_BOOL
* @retval AK_TRUE set successfully
* @retval AK_FALSE set failed
*/
T_BOOL sdio_set_int_callback(T_SDIO_INT_HANDLER cb)
{
    if (AK_NULL == cb)
    {
        return AK_FALSE;
    }
    SDIO_DRV_PROTECT(INTERFACE_SDIO); 
    g_pCurSdDevice = s_pSdioCard;
    
    //TBD

    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
    return AK_TRUE;
}

 
 /**
  * @brief read one byte  from sdio card 
  * @author Huang Xin
  * @date 2010-06-17
  * @param func[in] function to read
  * @param addr[in] register address to read
  * @param rdata[in] data buffer for read data
  * @return T_BOOL
  * @retval AK_TRUE read successfully
  * @retval AK_FALSE read failed
  */
T_BOOL sdio_read_byte(T_U8 func, T_U32 addr,  T_U8 *rdata)
{
    T_U32 arg_value;
    
    SDIO_DRV_PROTECT(INTERFACE_SDIO); 
    g_pCurSdDevice = s_pSdioCard;
    SDIO_SET_CMD52_ARG(arg_value, CMD52_READ, func, 0, addr, 0x0);
    if(send_cmd(SD_CMD(52), SD_SHORT_RESPONSE, arg_value) == AK_FALSE)
    {
        akprintf(C1, M_DRVSYS, "direct read failed.\n");
        SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
        return AK_FALSE;
    }
    *rdata = (get_short_resp()& 0xff);
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
    return AK_TRUE;
}

 
 /**
  * @brief write one byte to sdio card 
  * @author Huang Xin
  * @date 2010-06-17
  * @param func[in] function to write
  * @param addr[in] register address to write
  * @param rdata[in] the write byte
  * @return T_BOOL
  * @retval AK_TRUE write successfully
  * @retval AK_FALSE write failed
  */
T_BOOL sdio_write_byte(T_U8 func, T_U32 addr, T_U8 wdata)
{
    T_U32 arg_value;
    
    SDIO_DRV_PROTECT(INTERFACE_SDIO); 
    g_pCurSdDevice = s_pSdioCard;
    SDIO_SET_CMD52_ARG(arg_value, CMD52_WRITE, func, CMD52_NORMAL_WRITE, addr, wdata);
    if (send_cmd(SD_CMD(52), SD_SHORT_RESPONSE, arg_value) == AK_FALSE)
    {
        akprintf(C1, M_DRVSYS, "direct write failed.\n");
        SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);      
        return AK_FALSE;
    }
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
    return AK_TRUE;
}

 
 /**
  * @brief read multiple byte or block from sdio card 
  * @author Huang Xin
  * @date 2010-06-17
  * @param func[in] function to read
  * @param src[in] register address to read
  * @param count[in] data size(number of byte) to read
  * @param opcode[in] fixed address or increasing address
  * @param rdata[in] data buffer for read data
  * @return T_BOOL
  * @retval AK_TRUE read successfully
  * @retval AK_FALSE read failed
  */
T_BOOL sdio_read_multi(T_U8 func, T_U32 src, T_U32 count, T_U8 opcode, T_U8 rdata[])
{
    T_U32 status=0;
    T_U32 arg=0;
    T_U32 bytes=0;
    T_U32 blocks=0;
    T_U8  mode=0;
    T_U32 size=0;
    T_U32 cnt=0;
    T_U32 i = 0;
    
    //param validation
    if(func > 7 )
    {
        akprintf(C1, M_DRVSYS, "error param for cmd53 read: func %d", func);
        return AK_FALSE;
    }
    //check sdio controller inner status
    if (sd_trans_busy())
    {
        akprintf(C3, M_DRVSYS, " The sd card is writing and reading.\n");
        return  AK_FALSE;
    } 

    SDIO_DRV_PROTECT(INTERFACE_SDIO); 
    g_pCurSdDevice = s_pSdioCard;
    //config cmd 53 param
    blocks = count/g_pCurSdDevice->ulFunBlockLen[func];
    if (blocks)
    {
        mode = CMD53_BLOCK_BASIS;
        size = blocks * g_pCurSdDevice->ulFunBlockLen[func];
        //block transferred,blocks value of 0x0 indicates that the count set to infinite
        //in this case ,the I/O blocks shall be transferred until the operation is aborted by writing to
        //the I/O function select bits(ASx) in the CCCR
        blocks = (blocks >= 512) ? 0 : blocks; 
        SDIO_SET_CMD53_ARG(arg, CMD53_READ, func, mode, opcode, src, blocks); 
        if(!sdio_read_multi_block(arg, rdata,size))
        {
           SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
            return AK_FALSE;
        }
        rdata += size;
        if(CMD53_INCR_ADDRESS==opcode)
        {
            src += size;
        }
        
    }
    size = count % g_pCurSdDevice->ulFunBlockLen[func];
    if (size)
    {
        mode = CMD53_BYTE_BASIS;
        cnt = size/512;
        if (cnt)
        {
            //bytes transfered, bytes value of 0x0 shall cause 512 bytes to be read or writen
            bytes = 0;
            for (i = 0 ; i < cnt ; i++)
            {
                SDIO_SET_CMD53_ARG(arg, CMD53_READ, func, mode, opcode, src, bytes);
                if (!sdio_read_multi_byte(arg, rdata + i*512, 512))
                {
                   SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
                    return AK_FALSE;
                }
                size -= 512;
                if(CMD53_INCR_ADDRESS==opcode)
                {
                    src += 512;
                }
            }
        }
        bytes = size;
        if(bytes)
        {
            SDIO_SET_CMD53_ARG(arg, CMD53_READ, func, mode, opcode, src, bytes);
            if ( !sdio_read_multi_byte(arg, rdata + i*512,size))
            {
                SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
                return AK_FALSE;
            }
        }         
    }
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
    return AK_TRUE;
}


 
 /**
  * @brief write multiple byte or block from sdio card 
  * @author Huang Xin
  * @date 2010-06-17
  * @param func[in] function to read
  * @param src[in] register address to read
  * @param count[in] data size(number of byte) to read
  * @param opcode[in] fixed address or increasing address
  * @param wdata[in] the wirte data
  * @return T_BOOL
  * @retval AK_TRUE write successfully
  * @retval AK_FALSE write failed
  */
T_BOOL sdio_write_multi(T_U8 func, T_U32 dest, T_U32 count, T_U8 opcode, T_U8 wdata[])
{
    T_U32 status=0;
    T_U32 arg=0;
    T_U32 bytes=0;
    T_U32 blocks=0;
    T_U8  mode=0;
    T_U32 size=0;
    T_U32 cnt=0;
    T_U32 i = 0;

    //param validation
    if(func > 7 )
    {
        akprintf(C1, M_DRVSYS, "error param for cmd53 read: func %d", func);
        return AK_FALSE;
    }

    //check sdio controller inner status
    if (sd_trans_busy())
    {
        akprintf(C1, M_DRVSYS, " The sd card is writing and reading.\n");
        return  AK_FALSE;
    }
    SDIO_DRV_PROTECT(INTERFACE_SDIO); 
    g_pCurSdDevice = s_pSdioCard;
    //config cmd 53 param
    blocks = count/g_pCurSdDevice->ulFunBlockLen[func];
    if (blocks)
    {
        mode = CMD53_BLOCK_BASIS;
        size = blocks * g_pCurSdDevice->ulFunBlockLen[func];
        //block transferred,blocks value of 0x0 indicates that the count set to infinite
        //in this case ,the I/O blocks shall be transferred until the operation is aborted by writing to
        //the I/O function select bits(ASx) in the CCCR
        blocks = (blocks >= 512) ? 0 : blocks; 
        SDIO_SET_CMD53_ARG(arg, CMD53_WRITE, func, mode, opcode, dest, blocks);
        
        if(!sdio_write_multi_block(arg, (T_U8 *)dest,size))
        {
            SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
            return AK_FALSE;
        }
        wdata += size;
        if(CMD53_INCR_ADDRESS==opcode)
        {
            dest += size;
        }
        
    }
    size = count % g_pCurSdDevice->ulFunBlockLen[func];
    if (size)
    {
        mode = CMD53_BYTE_BASIS;    
        cnt = size/512;
        if (cnt)
        {
            //bytes transfered, bytes value of 0x0 shall cause 512 bytes to be read or writen
            bytes = 0;  
            for (i = 0; i < cnt; i++)
            {
                SDIO_SET_CMD53_ARG(arg, CMD53_WRITE, func, mode, opcode, dest, bytes);
                if (!sdio_write_multi_byte(arg, wdata + i*512, 512))
                {
                    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
                    return AK_FALSE;
                }
                size -= 512;
                
                if(CMD53_INCR_ADDRESS==opcode)
                {
                    dest += 512;
                }
            }
        }
        bytes = size;
        if(bytes)
        {
            SDIO_SET_CMD53_ARG(arg, CMD53_READ, func, mode, opcode, dest, bytes);
            if ( !sdio_write_multi_byte(arg, wdata + i*512,size))
            {
                SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
                return AK_FALSE;
            }
        }        
    }
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
    return AK_TRUE;
}

/**
* @brief select or deselect a sdio device
*
* the card is selected by its own relative address and gets deselected by any other address; address 0 deselects all
* @author Huang Xin
* @date 2010-06-17
* @param addr[in] the rca of  the card which will be selected
* @return T_BOOL
* @retval AK_TRUE  select or deselect successfully
* @retval AK_FALSE  select or deselect failed
*/
T_BOOL sdio_select_card(T_U32 addr)
{
    SDIO_DRV_PROTECT(INTERFACE_SDIO); 
    g_pCurSdDevice = s_pSdioCard;
    
    if(!select_card(addr))
    {
        SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
        return AK_FALSE;
    }
    SDIO_DRV_UNPROTECT(INTERFACE_NOT_SD, INTERFACE_SDIO);
    return AK_TRUE;
}


/**
 * @brief Init the io partion 
 *
 * Called when init card
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCOMMON_SD_STATUS
 */
T_U8 init_io(T_BOOL bInitIo)
{
    T_U32 status,resp,ocr;

    if(!bInitIo)
    {
        akprintf(C1, M_DRVSYS, "skip init sdio\n");
        return COMMON_SD_SKIP_INIT_IO;
    }
    status = sdio_get_ocr(&ocr);
   
    if (SDIO_GET_OCR_FAIL == status)
    {
        akprintf(C1, M_DRVSYS,"no cmd5 resp,skip init sdio\n");
        return COMMON_SD_SKIP_INIT_IO;
    }
 
    if ((SDIO_GET_OCR_INVALID == status)||(SDIO_NO_FUN == status))
    {   
        return COMMON_SD_INIT_IO_FAIL;//mp=1:a_process,   mp=0:b_process,
    }
    if (SDIO_GET_OCR_VALID == status)
    {
        status = sdio_nego_volt(ocr & g_pCurSdDevice->ulVolt);
        if (SDIO_NEGO_FAIL == status)
        {
            return COMMON_SD_INIT_FAIL;
        }
     
        if (SDIO_NEGO_TIMEOUT == status)
        {
            return COMMON_SD_INIT_IO_FAIL;//mp=1:a_process,   mp=0:b_process,
        }
        if(SDIO_NEGO_SUCCESS == status)
        {
            g_pCurSdDevice->bInitIoSuccess = 1;
            return COMMON_SD_INIT_IO_SUCCESS;
        }
    }
    
    akprintf(C1, M_DRVSYS,"init sdio status error!!! \n");
    return COMMON_SD_INIT_IO_ERROR;
}


/**
 * @brief Set sdio card bus width.
 *
 * Usually set the bus width  1 bit or 4 bit  .
 * @author Huang Xin
 * @date 2010-07-14
 * @param bus_mode[in] The bus mode.
 * @return T_BOOL
 * @retval  AK_TRUE: set successfully
 * @retval  AK_FALSE: set failed
 */
static T_BOOL sdio_set_bus_width(T_U8 bus_mode)
{
    T_U8 temp;
    
    if(!sdio_read_byte(0, CCCR_BUS_INTERFACE_CONTOROL, &temp))
        return AK_FALSE;

    if(USE_ONE_BUS == bus_mode)
    {
        temp &= ~(0x3<<0);
        temp |= (0x0<<0);
    }
    else if(USE_FOUR_BUS == bus_mode)
    {
        temp &= ~(0x3<<0);
        temp |= (0x2<<0);
    }
    else
    {
        return AK_FALSE;
    }

    if(!sdio_write_byte(0, CCCR_BUS_INTERFACE_CONTOROL, temp))
        return AK_FALSE;

    return AK_TRUE; 
}

/**
 * @brief Get the sdio card  ocr register
 *
 * Called when init sdio card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param ocr[out] The buffer to save card ocr.
 * @return T_BOOL
 * @retval  AK_TRUE: get successfully
 * @retval  AK_FALSE: get failed
 */
static T_U8 sdio_get_ocr(T_U32 *ocr )
{
    T_U32 response = 0;
    T_U8 fun_num = 0;
    T_BOOL mem_present = 0;
    
    if (send_cmd(SD_CMD(5), SD_SHORT_RESPONSE, SD_NO_ARGUMENT))     
    {   
        response = get_short_resp();
        if(response & SDIO_MP_MASK)
        {
            g_pCurSdDevice->bMemPresent = 1;
        }
        else
        {
            g_pCurSdDevice->bMemPresent = 0;
        }

        if(0 == (response & SDIO_FUN_NUM_MASK))
        {
            return SDIO_NO_FUN;
        }
        else
        {
            g_pCurSdDevice->ulFunNum = (response & SDIO_FUN_NUM_MASK)>>SDIO_FUN_NUM_OFFSET ;
        }
        if(0 == (response & g_pCurSdDevice->ulVolt))
            return SDIO_GET_OCR_INVALID;
        
        *ocr = response & SDIO_OCR_MASK;
        return SDIO_GET_OCR_VALID;
    }
    else    
        return SDIO_GET_OCR_FAIL;
}

/**
 * @brief Negotiation of the sdio card  voltage
 *
 * Called when init sdio card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param volt[in] The voltage to try.
 * @return T_eSDIO_STATUS
 */
static T_U8 sdio_nego_volt(T_U32 volt)
{
    T_U32 response = 0;
    T_U32 i=0;
    do
    {
        if (send_cmd(SD_CMD(5), SD_SHORT_RESPONSE, volt))       
        {      
            response = get_short_resp();    
        }
        else
        {
            return SDIO_NEGO_FAIL;
        }
    }while((!(response & SD_STATUS_POWERUP))&& (i++ < 10000));
    if(i >= 10000)
    {
        akprintf(C1, M_DRVSYS, "sdio nego time out!\n");    
        return SDIO_NEGO_TIMEOUT;
    }

    akprintf(C3, M_DRVSYS, "sdio nego success, ocr value is 0x%x.\n",response&SDIO_OCR_MASK);
    return SDIO_NEGO_SUCCESS;
}

