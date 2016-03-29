/**
 * @file spi.c
 * @brief SPI driver, define SPI APIs.
 *
 * This file provides SPI APIs: SPI initialization, write data to SPI, read data from SPI
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Huang Xin
 * @date 2010-11-17
 * @version 1.0
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "sysctl.h"
#include "drv_api.h"
#include "l2.h"
#include "drv_module.h"
#include "spi.h"
#include "drv_gpio.h"
#include "freq.h"

//temporarily move the spi protection to hal layer, which is spi flash driver
//coz it's difficulty to handle the cs release/unrelease situation on this layer
#define SPI_DRV_PROTECT(id) 

#define SPI_DRV_UNPROTECT(id) 

static T_SPI s_tSpi[SPI_NUM] = {0};

static T_VOID spi_on_change(T_U32 asic_clk);


T_VOID spi_set_protect(T_U32 spi_id, T_U8 width)
{
    DrvModule_Protect(DRV_MODULE_SPI);
    gpio_pin_group_cfg(ePIN_AS_SPI);
}

T_VOID spi_set_unprotect(T_U32 spi_id, T_U8 width)
{
    DrvModule_UnProtect(DRV_MODULE_SPI);
}

static void set_tx(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~(1<<0);
    reg_value |= (1<<1);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
}

static void set_rx(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value |= (1<<0);
    reg_value &= ~(1<<1);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
}

static void set_rx_tx(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~(3<<0);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value;
}

static void force_cs(T_U8 spi_id)
{
    T_U32 reg_value;

    gpio_pin_group_cfg(ePIN_AS_SPI);

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value |= SPI_CTRL_FORCE_CS;
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
    s_tSpi[spi_id].ucCS = 1;    //cs enable
}

static void unforce_cs(T_U8 spi_id)
{
    T_U32 reg_value;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~SPI_CTRL_FORCE_CS;
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value ;
    s_tSpi[spi_id].ucCS = 0;    //cs disable
}

static T_U8 get_spi_clk_div(T_U32 asic_clk, T_U32 spi_clk)
{
    T_U8 clk_div;

    clk_div = (asic_clk/2 - 1) / spi_clk;

    akprintf(C3, M_DRVSYS, "spi clk: %d\n", asic_clk/(2*(1+clk_div)));

    return clk_div;
}

static T_VOID spi_on_change(T_U32 asic_clk)
{
    T_U32 reg;
    T_U8 div;
    T_U32 i;
    
    DrvModule_Protect(DRV_MODULE_SPI);
    
    for (i = 0; i < SPI_NUM; i++)
    {
        if (s_tSpi[i].bOpen)
        {
            //akprintf(C3, M_DRVSYS, "spi on change: %d\n", s_tSpi[i].clock);

            //calculate clock
            div = get_spi_clk_div(asic_clk, s_tSpi[i].clock);
            if((SPI_SLAVE == s_tSpi[i].ucRole) && (div < 3)) div = 3;
            
            REG32(s_tSpi[i].ulBaseAddr + ASPEN_SPI_CTRL) = (1<<1) | (s_tSpi[i].ucMode<<2) | (s_tSpi[i].ucRole<<4) | SPI_CTRL_ENA_WORK | (div<<8);
        }
    }
    
    DrvModule_UnProtect(DRV_MODULE_SPI);
}

/**
 * @brief spi_data_mode - select spi data mode, set GPIO pin #SPI_D2 #SPI_D3 
 * @author LuHeshan
 * @date 2012-12-24
 * @param spi_id: spi id
 * @param data_mode: 1-2-4wire
 * @return T_BOOL
 * @version 
 */
T_BOOL spi_data_mode(T_eSPI_ID spi_id, T_eSPI_BUS data_mode)
{
    T_U32 reg_value;

    if ((!s_tSpi[spi_id].bOpen) || (SPI_BUS_NUM <= data_mode))
    {
        akprintf(C1, M_DRVSYS, "spi_data_mode(): spi no open or param fail!\n");
        return AK_FALSE;
    }

    s_tSpi[spi_id].ucBusWidth = data_mode;

    reg_value = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL);
    reg_value &= ~(0x3 << 16);
    reg_value |= (data_mode << 16);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = reg_value;

    return AK_TRUE;
}

/**
 * @brief Initialize SPI
 *
 * this func must be called before call any other SPI functions
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param mode[in] spi mode selected 
 * @param role[in] master or slave
 * @param clk_div[in] SPI working frequency = ASICCLK/(2*(clk_div+1)
 * @return T_BOOL
 * @retval AK_TRUE: Successfully initialized SPI.
 * @retval AK_FALSE: Initializing SPI failed.
 */
T_BOOL spi_init(T_eSPI_ID spi_id, T_eSPI_MODE mode, T_eSPI_ROLE role, T_U32 clk)
{
    T_U32 div;
    
    if (spi_id > SPI_ID0 || mode >= SPI_MODE_NUM|| role >= SPI_ROLE_NUM)
    {
        akprintf(C1, M_DRVSYS, "SPI initialized failed!\n");
        return AK_FALSE;
    }
    
    s_tSpi[spi_id].ucRole= role;
    s_tSpi[spi_id].ucMode = mode;  
    s_tSpi[spi_id].clock = clk;  
    s_tSpi[spi_id].bOpen = AK_TRUE;
    
    s_tSpi[spi_id].ulBaseAddr = SPI0_BASE_ADDR;
    s_tSpi[spi_id].ucL2Rx = ADDR_SPI1_RX;
    s_tSpi[spi_id].ucL2Tx = ADDR_SPI1_TX;

    s_tSpi[spi_id].ucBusWidth = SPI_BUS1;

    sysctl_clock(CLOCK_SPI_ENABLE);

    div = get_spi_clk_div(get_asic_freq(), clk);  
    if((SPI_SLAVE == role) && (div < 3)) div = 3;

    s_tSpi[spi_id].ucCS = 0;
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) = (1<<1) | (mode<<2) | (role<<4) | SPI_CTRL_ENA_WORK | (div<<8);

    akprintf(C3, M_DRVSYS, "SPI initialized ok!\n");
    return AK_TRUE;
}

/**
 * @brief close SPI
 *
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @return T_VOID
 */
T_VOID spi_close(T_eSPI_ID spi_id)
{
    T_U32 i = 0;

    if (spi_id > SPI_ID0)
    {
        akprintf(C3, M_DRVSYS, "spi_id %d is invalid!\n",spi_id);
        return;
    }
    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_id %d is not open!\n",spi_id);
        return;
    }

    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_CTRL) &= ~(1<<6);
    s_tSpi[spi_id].bOpen = AK_FALSE;

    //check whether all spi is closed
    sysctl_clock(~CLOCK_SPI_ENABLE);  
        
}

/**
 * @brief reset SPI
 *
 * @author HuangXin
 * @date 2010-11-17
 * @return T_VOID
 */
T_VOID spi_reset()
{
    T_U32 i = 0;
    T_U8 div;
    
    for (i = 0; i < SPI_NUM; i++)
    {
        if (s_tSpi[i].bOpen)
        {
            div = get_spi_clk_div(get_asic_freq(), s_tSpi[i].clock);
            if((SPI_SLAVE == s_tSpi[i].ucRole) && (div < 3)) div = 3;

            REG32(s_tSpi[i].ulBaseAddr + ASPEN_SPI_CTRL) = (1<<1) | (s_tSpi[i].ucMode << 2) | (s_tSpi[i].ucRole << 4) | SPI_CTRL_ENA_WORK | (div << 8);           
        }
    }
}

/**
 * @brief spi master write
 *
 * this func must be called in spi  master mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the write data buffer  
 * @param count[in] the write data count
 * @param bReleaseCS[in] whether pll up cs
 * @return T_BOOL
 * @retval AK_TRUE:  spi write successfully.
 * @retval AK_FALSE: spi write failed.
 */
T_BOOL spi_master_write(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS)
{
    T_BOOL ret = AK_TRUE;
    T_U32 i = 0;
    T_U32 offset = 0;
    T_U32 temp = 0;

    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_master_write(): SPI not initialized!\n");
        return AK_FALSE;
    }

    SPI_DRV_PROTECT(spi_id);

    if ((count < 512)||((T_U32)buf & 0x3))
    {

        force_cs(spi_id);
        set_tx(spi_id);
        
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;
        for(i = 0; i < count; i++)
        {
            temp |= (buf[i] << (offset * 8));

            offset++;
            if((offset == 4) || (i == (count-1)))
            {
                while(!(REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_TXBUF_HALFEMPTY));

                REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_INBUF) = temp;

                temp = 0;
                offset = 0;
            }
        }
        //wait finish status
        while((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) != SPI_MASTER_TRANFINISH );
    }
    else
    {
    
        //alloc L2 buffer
        s_tSpi[spi_id].ucTxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Tx);
        if (BUF_NULL == s_tSpi[spi_id].ucTxBufferID)
        {
            akprintf(C1, M_DRVSYS, "spi_master_write(): allocate L2 buffer failed,  tx=%d\n", s_tSpi[spi_id].ucTxBufferID);
            SPI_DRV_UNPROTECT(spi_id);       
            return AK_FALSE;
        }
        
        force_cs(spi_id);
        set_tx(spi_id);

        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = ((1<<0)|(1<<16));
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;

        //start l2 dma transmit
        l2_combuf_dma((T_U32)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF, AK_FALSE);
        
        if (AK_FALSE == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucTxBufferID))
        {
            akprintf(C1, M_DRVSYS, "spi_master_write(): l2 dma timeout\n");
            ret =  AK_FALSE;
            goto EXIT;
        } 

        //wait data cnt decreased to 0
        while (1)
        {      
            if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR))
                break;
        }
        
        //wait finish status
        while((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) != SPI_MASTER_TRANFINISH );

    EXIT:
    
        //disable l2 dma
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
        l2_clr_status(s_tSpi[spi_id].ucTxBufferID); 
        //free L2 buffer
        l2_free(s_tSpi[spi_id].ucL2Tx);   
    }       

    //pull up CS
    if (bReleaseCS)
    { 
        for (i = 0; i < 30; i++);//improve spi speed 
        unforce_cs(spi_id);
    }

    SPI_DRV_UNPROTECT(spi_id);       
    return ret;
}

/**
 * @brief spi master read
 *
 * this func must be called in spi  master mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the read data buffer  
 * @param count[in] the read data count
 * @param bReleaseCS[in] whether pll up cs
 * @return T_BOOL
 * @retval AK_TRUE:  spi read successfully.
 * @retval AK_FALSE: spi read failed.
 */
T_BOOL spi_master_read(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS)
{
    T_BOOL ret = AK_TRUE;
    T_U32 i = 0;
    T_U32 offset = 0;
    T_U32 temp = 0;

    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_master_read(): SPI not initialized!\n");
        return AK_FALSE;
    }

    SPI_DRV_PROTECT(spi_id);

    if ((count < 512)||((T_U32)buf & 0x3))
    {
        //prepare spi read
        force_cs(spi_id);
        set_rx(spi_id);

        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = 0;
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;   
        for(i = 0; i < count; i++)
        {
            if(offset == 0)
            {
                if((i+4) > count)
                {
                    while(!(REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH));
                }
                else
                {
                    while(!(REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_RXBUF_HALFEMPTY));
                }
                
                temp = REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_INBUF);
            }
            buf[i] = (T_U8)((temp >> (offset * 8)) & 0xff);

            if(++offset == 4)
            {
                offset = 0;
            }
        }
    }
    else 
    {
        //alloc L2 buffer
        s_tSpi[spi_id].ucRxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Rx);
        if (BUF_NULL == s_tSpi[spi_id].ucRxBufferID)
        {
            akprintf(C3, M_DRVSYS, "spi_master_read(): allocate L2 buffer failed,  rx=%d\n", s_tSpi[spi_id].ucRxBufferID);
            SPI_DRV_UNPROTECT(spi_id);       
            return AK_FALSE;
        }
        //prepare spi read
        force_cs(spi_id);
        set_rx(spi_id);

        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = ((1<<0)|(1<<16));
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;   
        
        //start L2 dma
        l2_combuf_dma((T_U32)buf, s_tSpi[spi_id].ucRxBufferID, count, BUF2MEM, AK_FALSE);

        //wait spi trans finish
        while (1)
        {            
            if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && 
                (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH)
                break;
        }

        //wait L2 dma finish, if need frac dma,start frac dma
        if (AK_FALSE == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucRxBufferID))
        {
            akprintf(C1, M_DRVSYS, "spi_master_read(): l2 dma timeout\n");
            ret =  AK_FALSE;
        }
        //disable l2 dma
        REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = 0;    
        l2_clr_status(s_tSpi[spi_id].ucRxBufferID);
        //free L2 buffer
        l2_free(s_tSpi[spi_id].ucL2Rx);   
    }
    
    //pull up CS
    if (bReleaseCS)
    {
        for(i = 0; i < 30; i++);
        unforce_cs(spi_id);
    }
    
    SPI_DRV_UNPROTECT(spi_id);       
    return ret;
}

/**
 * @brief spi slave read
 *
 * this func must be called in spi  slave mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the read data buffer  
 * @param count[in] the read data count
 * @return T_BOOL
 * @retval AK_TRUE:  spi read successfully.
 * @retval AK_FALSE: spi read failed.
 */
T_BOOL spi_slave_read(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count)
{
    T_BOOL ret = AK_TRUE;

    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_slave_read(): SPI not initialized!\n");
        return AK_FALSE;
    }

    SPI_DRV_PROTECT(spi_id);
    
    //alloc L2 buffer
    s_tSpi[spi_id].ucRxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Rx);
    if (BUF_NULL == s_tSpi[spi_id].ucRxBufferID)
    {
        akprintf(C3, M_DRVSYS, "spi_master_read(): allocate L2 buffer failed,  rx=%d\n", s_tSpi[spi_id].ucRxBufferID);
        SPI_DRV_UNPROTECT(spi_id);       
        return AK_FALSE;
    }
    //prepare spi read
    set_rx(spi_id);
    //initial slaver time out max time cycle
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RTIM) = 0xffff;
    //config rx buf in L2
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = ((1<<0)|(1<<16));
    //set the spi send data_cnt
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;   
 
    if(count >= 64)
    {

        l2_combuf_dma((T_U32)buf, s_tSpi[spi_id].ucRxBufferID, count, BUF2MEM, AK_FALSE);    
        //wait spi trans finish
        while (1)
        { 
            if(0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && 
                (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH )
                break;
        }
        //if 4bytes misaligned,wait timeout is necessary
        if (0 != (count & 3) )
        {
            while (1)
            { 
                if((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_SLAVER_TIMEOUT) == SPI_SLAVER_TIMEOUT)
                    break;
            }
        }
            
        //wait L2 dma finish, if need frac dma,start frac dma
        if (AK_FALSE == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucRxBufferID))
        {
            akprintf(C1, M_DRVSYS, "spi_slave_read(): l2 dma timeout\n");
            ret =  AK_FALSE;
        }
    }
    else
    {
        //wait spi trans finish
        while (1)
        { 
            if(0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && 
                (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH )
                break;
        }
        //if 4bytes misaligned,wait timeout is necessary
        if (0 != (count & 3) )
        {
            while (1)
            { 
                if((REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_SLAVER_TIMEOUT) == SPI_SLAVER_TIMEOUT)
                    break;
            }
        }
        //start frac dma
        l2_combuf_dma((T_U32)buf, s_tSpi[spi_id].ucRxBufferID, count, BUF2MEM, AK_FALSE);
        l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucRxBufferID);
    }
    //disable l2 dma
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_RX_EXBUF) = 0;  
    l2_clr_status(s_tSpi[spi_id].ucRxBufferID);
    //free L2 buffer
    l2_free(s_tSpi[spi_id].ucL2Rx);   
    SPI_DRV_UNPROTECT(spi_id);       
    return ret;

}

/**
 * @brief spi slave write
 *
 * this func must be called in spi  slave mode
 * @author HuangXin
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the write data buffer  
 * @param count[in] the write data count
 * @return T_BOOL
 * @retval AK_TRUE:  spi write successfully.
 * @retval AK_FALSE: spi write failed.
 */
T_BOOL spi_slave_write(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count)
{
    T_U32 state;
    T_BOOL ret = AK_TRUE;
    
    if (!s_tSpi[spi_id].bOpen)
    {
        akprintf(C3, M_DRVSYS, "spi_slave_write(): SPI not initialized!\n");
        return AK_FALSE;
    }

    SPI_DRV_PROTECT(spi_id);
    //select the spi L2 buffer
    s_tSpi[spi_id].ucTxBufferID = l2_alloc(s_tSpi[spi_id].ucL2Tx);
    if(BUF_NULL == s_tSpi[spi_id].ucTxBufferID)
    {
        akprintf(C3, M_DRVSYS, "alloc L2 buffer failed!\n");  
        SPI_DRV_UNPROTECT(spi_id);
        return AK_FALSE;
    }
    //prepare spi
    set_tx(spi_id);
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = ((1<<0)|(1<<16));
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) = count;
    
    if((T_U32)buf & 0x3)
    {
        l2_combuf_cpu((T_U32)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF);
    }
    else
    {
        //start l2 dma transmit
        l2_combuf_dma((T_U32)buf, s_tSpi[spi_id].ucTxBufferID, count, MEM2BUF, AK_FALSE);
        if (AK_FALSE == l2_combuf_wait_dma_finish(s_tSpi[spi_id].ucTxBufferID))
        {
            akprintf(C1, M_DRVSYS, "spi_slave_write(): l2 dma timeout\n");
            ret =  AK_FALSE;
            goto EXIT;
        }        
    }
    //wait spi trans finish
    while (1)
    {      
        if (0 == REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_NBR) && (REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_STA) & SPI_MASTER_TRANFINISH) == SPI_MASTER_TRANFINISH)
            break;
    }
        
EXIT:
    //disable l2 dma
    REG32(s_tSpi[spi_id].ulBaseAddr + ASPEN_SPI_TX_EXBUF) = 0;
    l2_clr_status(s_tSpi[spi_id].ucTxBufferID);
    //free L2 buffer
    l2_free(s_tSpi[spi_id].ucL2Tx);   
    SPI_DRV_UNPROTECT(spi_id);       
    return ret;
   
}

static int spi_reg(void)
{
    freq_register(E_ASIC_CALLBACK, spi_on_change);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(spi_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

