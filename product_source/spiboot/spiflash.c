/**
 * @file spi_flash.c
 * @brief spiflash driver source file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */
#include "anyka_types.h"
#include "hal_spiflash.h"

#define SPI_FLASH_COM_RDID        0x9f
#define SPI_FLASH_COM_READ        0x03
#define SPI_FLASH_COM_PROGRAM     0x02
#define SPI_FLASH_COM_WR_EN       0x06
#define SPI_FLASH_COM_ERASE       0xd8
#define SPI_FLASH_COM_RD_STUS1    0x05
#define SPI_FLASH_COM_RD_STUS2    0x35
#define SPI_FLASH_COM_WRSR        0x01

#define SPI_FLASH_COM_AAI         0xad
#define SPI_FLASH_COM_WRDI        0x04

//2-wire mode
#define SPI_FLASH_COM_D_READ      0x3B

//4-wire mode
#define SPI_FLASH_COM_Q_READ      0x6B
#define SPI_FLASH_COM_Q_WRITE     0x32

#define SPI_FLASH_QUAD_ENABLE    (1 << 9)

#define SPI_SECTOR_SIZE           256

#define MAX_RD_SIZE              (32 * 1024)

T_BOOL spi_flash_read_status(T_U16 *status);
T_BOOL spi_flash_write_status(T_U16 status);

static T_BOOL flash_read_standard(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_read_quad(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_QE_set(T_BOOL enable);
static T_VOID write_enable(T_VOID);
static T_BOOL check_status_done(T_VOID);

typedef struct
{
    T_BOOL bInit;

    T_eSPI_ID spi_id;

    T_eSPI_BUS bus_wight;

    T_SFLASH_PARAM param;
}
T_SPI_FLASH;

static T_SPI_FLASH m_sflash = {0};

/**
 * @brief spi flash init
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_BOOL
 */
T_BOOL spi_flash_init(T_eSPI_ID spi_id, T_eSPI_BUS bus_wight)
{

    m_sflash.spi_id = spi_id;
    m_sflash.bus_wight = bus_wight;
    m_sflash.bInit = AK_TRUE;

    return AK_TRUE;
}

static T_BOOL flash_QE_set(T_BOOL enable)
{
    T_U16 status;
    
    if (!spi_flash_read_status(&status))
    {
        printf("spi QE read status fail\r\n");
        return AK_FALSE;
    }

    if (enable)
    {
        status = status | SPI_FLASH_QUAD_ENABLE;
    }
    else
    {
        status = status & (~SPI_FLASH_QUAD_ENABLE);
    }
    
    if (!spi_flash_write_status(status & 0xffff))
    {
        printf("spi QE: write status fail \r\n");
        return AK_FALSE;
    }

    return AK_TRUE;
}

/**
 * @brief set param of serial flash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param sflash_param [in]  serial flash param 
 * @return T_VOID
 */
T_VOID spi_flash_set_param(T_SFLASH_PARAM *sflash_param)
{
    T_U16 status;

    if(AK_NULL == sflash_param)
        return;

    //save param
    memcpy(&m_sflash.param, sflash_param, sizeof(T_SFLASH_PARAM));

    spi_set_protect(m_sflash.spi_id, SPI_BUS1);

    //setup clock
    if(sflash_param->clock > 0)
    {
        spi_init(m_sflash.spi_id, SPI_MODE3, SPI_MASTER, sflash_param->clock);
    }

    //unmask protect
    if(sflash_param->flag & SFLASH_FLAG_UNDER_PROTECT)
    {
        spi_flash_read_status(&status);

        status &= ~sflash_param->protect_mask;

        spi_flash_write_status(status & 0xffff);
    }

    //enable quad
    if((sflash_param->flag & SFLASH_FLAG_QUAD_WRITE) \
        || (sflash_param->flag & SFLASH_FLAG_QUAD_READ))
    {
        if (SPI_BUS4 == m_sflash.bus_wight)
        {
            if (!flash_QE_set(AK_TRUE))
            {
                printf("warnning: enable QE fail, will set to one-wire mode\n");
                m_sflash.bus_wight = SPI_BUS1;
            }
//            else
//            {
//                printf("Set QE OK\n");
//            }
        }
    }

    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);

}

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
T_BOOL spi_flash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_BOOL ret = AK_FALSE;
    
    spi_set_protect(m_sflash.spi_id, m_sflash.bus_wight);
    
    if ((SPI_BUS4 == m_sflash.bus_wight) && (m_sflash.param.flag & SFLASH_FLAG_QUAD_READ))
    {
        ret = flash_read_quad(page, buf, page_cnt); 
    }
    else
    {
        ret = flash_read_standard(page, buf, page_cnt);
    }

    spi_set_unprotect(m_sflash.spi_id, m_sflash.bus_wight);

    return ret;
}

T_BOOL spi_flash_read_status(T_U16 *status)
{
    T_U8 buf1[1];
    T_U8 status1, status2;

    buf1[0] = SPI_FLASH_COM_RD_STUS1;

    spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);

    if(!spi_master_read(m_sflash.spi_id, &status1, 1, AK_TRUE))
        return AK_FALSE;

    if (m_sflash.param.flag & SFLASH_FLAG_COM_STATUS2)
    {
        buf1[0] = SPI_FLASH_COM_RD_STUS2;
        
        spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);
        
        if(!spi_master_read(m_sflash.spi_id, &status2, 1, AK_TRUE))
            return AK_FALSE;
        
        *status = (status1 | (status2 << 8));
    }
    else
    {
        *status = status1 && 0xff;
    }

    return AK_TRUE;
}

static T_BOOL check_status_done(T_VOID)
{
    T_U32 timeout = 0;
    T_U16 status = 0;

    while(1)
    {
        spi_flash_read_status(&status);

        if((status & (1 << 0 )) == 0)
            break;

        if(timeout++ > 100000)
        {
            printf("spiflash check_status_done timeout\n");
            return AK_FALSE;
        }
    }

    return AK_TRUE;
}

static T_VOID write_enable(T_VOID)
{
    T_U8 buf1[1];
    
    //write enable
    buf1[0] = SPI_FLASH_COM_WR_EN;
    spi_master_write(m_sflash.spi_id, buf1, 1, AK_TRUE);
}

T_BOOL spi_flash_write_status(T_U16 status)
{
    T_U8 buf[3];
    T_U32 write_cnt;

    write_enable();

    buf[0] = SPI_FLASH_COM_WRSR;
    buf[1] = status & 0xff;
    buf[2] = (status >> 8) & 0xff;

    if (m_sflash.param.flag & SFLASH_FLAG_COM_STATUS2)
    {
        write_cnt = 3;
    }
    else
    {
        write_cnt = 2;
    }
    
    if(!spi_master_write(m_sflash.spi_id, buf, write_cnt, AK_TRUE))
        return AK_FALSE;

    return check_status_done();
}

static T_BOOL flash_read_standard(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf1[4];
    T_U32 i, addr = page * m_sflash.param.page_size;
    T_BOOL bReleaseCS=AK_FALSE;
    T_U32 count, readlen;
    
    if((page > 0xffff) || (AK_NULL == buf))
    {
        printf("spi_flash_read: param error\r\n");
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        printf("spi flash not init\r\n");
        return AK_FALSE;
    }

    buf1[0] = SPI_FLASH_COM_READ;
    buf1[1] = (addr >> 16) & 0xFF;
    buf1[2] = (addr >> 8) & 0xFF;
    buf1[3] = (addr >> 0) & 0xFF;

    spi_master_write(m_sflash.spi_id, buf1, 4, AK_FALSE);

    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? AK_FALSE: AK_TRUE;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
            return AK_FALSE;
        buf += readlen;        
    }
    
    return AK_TRUE;
}


static T_BOOL flash_read_quad(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf_cmd[5];
    T_U16 status;
    T_U32 addr = page * m_sflash.param.page_size;
    T_U32 count;
    T_U32 readlen;
    T_BOOL bReleaseCS=AK_FALSE;
    T_BOOL ret = AK_TRUE;
    
    if((page > 0xffff) || (AK_NULL == buf))
    {
        printf("spi_flash_read: param error\r\n");
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        printf("spi flash not init\r\n");
        return AK_FALSE;
    }

    // cmd
    buf_cmd[0] = SPI_FLASH_COM_Q_READ;
    buf_cmd[1] = (T_U8)((addr >> 16) & 0xFF);
    buf_cmd[2] = (T_U8)((addr >> 8) & 0xFF);
    buf_cmd[3] = (T_U8)(addr & 0xFF);
    buf_cmd[4] = 0;
    if (!spi_master_write(m_sflash.spi_id, buf_cmd, 5, AK_FALSE))
    {
        printf("spi flash_read_quad: write cmd fail\r\n");
        goto EXIT;
    }

    // set spi 4-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS4);

    // read
    count = page_cnt * m_sflash.param.page_size;
    while(count>0)
    {
        readlen = (count > MAX_RD_SIZE)? MAX_RD_SIZE: count;
        count -= readlen;
        //only the last read, release CS
        bReleaseCS = (count>0)? AK_FALSE: AK_TRUE;
        if(!spi_master_read(m_sflash.spi_id, buf, readlen, bReleaseCS))
        {
            ret = AK_FALSE;
            goto EXIT;
        }
        buf += readlen;        
    }

EXIT:   
    // set spi 1-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS1);

    return ret;
}

