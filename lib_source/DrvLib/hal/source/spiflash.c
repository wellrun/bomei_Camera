/**
 * @file spi_flash.c
 * @brief spiflash driver source file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */
#include "anyka_types.h"
#include "drv_api.h"
#include "drv_module.h"

#define SPI_FLASH_COM_RDID        0x9f
#define SPI_FLASH_COM_READ        0x03
#define SPI_FLASH_COM_PROGRAM     0x02
#define SPI_FLASH_COM_WR_EN       0x06
#define SPI_FLASH_COM_ERASE       0xd8
#define SPI_FLASH_COM_RD_STUS1    0x05
#define SPI_FLASH_COM_RD_STUS2    0x35
#define SPI_FLASH_COM_WRSR        0x01
#define SPI_FLASH_COM_ERASE_SEC   0x20//擦除4KB sector


//aaai command
#define SPI_FLASH_COM_AAI         0xad
#define SPI_FLASH_COM_WRDI        0x04

//2-wire mode
#define SPI_FLASH_COM_D_READ      0x3B

//4-wire mode
#define SPI_FLASH_COM_Q_READ      0x6B
#define SPI_FLASH_COM_Q_WRITE     0x32

//quad enable
#define SPI_FLASH_QUAD_ENABLE    (1 << 9)

//setor size
#define SPI_SECTOR_SIZE           256

//max read size
#define MAX_RD_SIZE              (32 * 1024)

T_BOOL spi_flash_read_status(T_U16 *status);
T_BOOL spi_flash_write_status(T_U16 status);
static T_VOID write_enable(T_VOID);
static T_BOOL check_status_done(T_VOID);
static T_BOOL write_page(T_U32 page, T_U8 *buf);
static T_BOOL flash_read_standard(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_read_quad(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL write_page_aai(T_U32 page, T_U8 *buf);
static T_BOOL flash_write_standard(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_write_quad(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL write_page_quad(T_U32 page, T_U8 *buf);
static T_BOOL flash_read_dual(T_U32 page, T_U8 *buf, T_U32 page_cnt);
static T_BOOL flash_QE_set(T_BOOL enable);

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
 * @param spi_id choose spi0 or spi1
 * @param bus_width choose bus width: 1 line/2 line/4 line
 * @date 2010-04-29
 * @return T_BOOL
 */
T_BOOL spi_flash_init(T_eSPI_ID spi_id, T_eSPI_BUS bus_wight)
{
    //set clock to 25M, use mode0, coz some spi flash may have data error in mode3
    if(!spi_init(spi_id, SPI_MODE0, SPI_MASTER, 15*1000*1000))
    {
        return AK_FALSE;
    }

    m_sflash.spi_id = spi_id;
    m_sflash.bus_wight = bus_wight;
    m_sflash.bInit = AK_TRUE;

    return AK_TRUE;
}

/**
 * @brief set/clear quad enable
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param enable [in] AK_TRUE: enable QE, AK_FALSE: disable QE
 * @return T_BOOL
 */
static T_BOOL flash_QE_set(T_BOOL enable)
{
    T_U16 status;

    //read status 
    if (!spi_flash_read_status(&status))
    {
        akprintf(C1, M_DRVSYS, "spi QE read status fail\r\n");
        return AK_FALSE;
    }

    if (enable)
    {
        //set quad enable
        status = status | SPI_FLASH_QUAD_ENABLE;
    }
    else
    {
        //clear quad enable
        status = status & (~SPI_FLASH_QUAD_ENABLE);
    }

    //write status
    if (!spi_flash_write_status(status & 0xffff))
    {
        akprintf(C1, M_DRVSYS, "spi QE: write status fail \r\n");
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
                akprintf(C1, M_DRVSYS, "warnning: enable QE fail, will set to one-wire mode\n");
                m_sflash.bus_wight = SPI_BUS1;
            }
            else
            {
                akprintf(C1, M_DRVSYS, "Set QE OK\n");
            }
        }
    }

    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);

}
/**
 * @brief get spiflash id
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_U32
 * @retval T_U32 spiflash id
 */
T_U32 spi_flash_getid(T_VOID)
{
    T_U8 buf1[1];
    T_U32 flash_id = 0;

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return 0;
    }

    buf1[0] = SPI_FLASH_COM_RDID;

    spi_set_protect(m_sflash.spi_id, SPI_BUS1);

    //get id
    spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);
    spi_master_read(m_sflash.spi_id, (T_U8 *)(&flash_id), 3, AK_TRUE);

    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);
    
    akprintf(C1, M_DRVSYS, "the manufacture id is: %x\n", flash_id);

    return flash_id;
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
        //line 4
        ret = flash_read_quad(page, buf, page_cnt); 
    }
    else if ((SPI_BUS2 == m_sflash.bus_wight) && ((m_sflash.param.flag & SFLASH_FLAG_DUAL_READ)))
    {
        //line 2
        ret =  flash_read_dual(page, buf, page_cnt);
    }
    else
    {
        //line 1
        ret = flash_read_standard(page, buf, page_cnt);
    }

    spi_set_unprotect(m_sflash.spi_id, m_sflash.bus_wight);

    return ret;
}


/**
 * @brief write data to one page of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param buf [in]  buffer to be write
  * @param page_cnt [in]  the page count to write  
* @return T_BOOL
 */
T_BOOL spi_flash_write(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_BOOL ret = AK_FALSE;

    spi_set_protect(m_sflash.spi_id, m_sflash.bus_wight);
        
    if ((SPI_BUS4 == m_sflash.bus_wight) && (m_sflash.param.flag & SFLASH_FLAG_QUAD_WRITE))
    {
        //line 4
        ret = flash_write_quad(page, buf, page_cnt);
    }
    else
    {
        //line 1
        ret = flash_write_standard(page, buf, page_cnt);
    }

    spi_set_unprotect(m_sflash.spi_id, m_sflash.bus_wight);

    return ret;
}


#define SPIFLASH_ERASE_UNIT (4 * 1024)
/**
 * @brief erase one sector(4K) of spiflash 
 * 
 * @author  kejianping
 * @date 2014-09-25
 * @param addr [in]  sector  number
 * @return T_BOOL
 */
T_BOOL spi_flash_erase_sector(T_U32 sector)

{
    T_U8 buf1[4];
    T_U32 addr = sector * SPIFLASH_ERASE_UNIT;
    T_BOOL ret = AK_FALSE;

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return 0;
    }
    
    spi_set_protect(m_sflash.spi_id, SPI_BUS1);

    write_enable();

    buf1[0] = SPI_FLASH_COM_ERASE_SEC;//每次擦写只是擦除4K的大小
    buf1[1] = (addr >> 16) & 0xFF;
    buf1[2] = (addr >> 8)  & 0xFF;
    buf1[3] = (addr >> 0)  & 0xFF;

    //send erase cmd
    if(!spi_master_write(m_sflash.spi_id, buf1, 4, AK_TRUE))
    {
		goto ERASE_EXIT;
    }
    
    if(!check_status_done())
    {
		goto ERASE_EXIT;
    }
    
    ret = AK_TRUE;

ERASE_EXIT:    
    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);
    return ret;
}



/**
 * @brief erase one block(64k) of spiflash 
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  block number
 * @return T_BOOL
 */
 
T_BOOL spi_flash_erase(T_U32 block)
{
    T_U8 buf1[4];
    T_U32 addr = block * m_sflash.param.erase_size;
    T_BOOL ret = AK_FALSE;

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n", 0, AK_TRUE);
        return AK_FALSE;
    }

    spi_set_protect(m_sflash.spi_id, SPI_BUS1);

    //eable write
    write_enable();

    buf1[0] = SPI_FLASH_COM_ERASE;
    buf1[1] = (addr >> 16) & 0xFF;
    buf1[2] = (addr >> 8) & 0xFF;
    buf1[3] = (addr >> 0) & 0xFF;

    //erase
    if(!spi_master_write(m_sflash.spi_id, buf1, 4, AK_TRUE))
    {
        goto ERASE_EXIT;
    }
    
    if(!check_status_done())
    {
        goto ERASE_EXIT;
    }

    ret = AK_TRUE;

ERASE_EXIT:    
    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);
    return ret;
}



/**
 * @brief erase one sector or block of spiflash
 * 
 * @author  kejianping
 * @date 2014-08-29
 * @param addr [in]  erase addr 
 * @param length [in]  erase length 
 * @return T_BOOL
 */
T_BOOL spi_flash_erase_addr(T_U32 addr, T_U32 length)
{
    T_U8 buf1[4];
    T_BOOL ret = AK_FALSE;

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return 0;
    }
    
    spi_set_protect(m_sflash.spi_id, SPI_BUS1);

    write_enable();

    buf1[0] = (length > SPIFLASH_ERASE_UNIT)?(SPI_FLASH_COM_ERASE):(SPI_FLASH_COM_ERASE_SEC);
    buf1[1] = (addr >> 16) & 0xFF;
    buf1[2] = (addr >> 8) & 0xFF;
    buf1[3] = (addr >> 0) & 0xFF;

    //erase
    if(!spi_master_write(m_sflash.spi_id, buf1, 4, AK_TRUE))
    {
		goto ERASE_EXIT;
	}
    
    if(!check_status_done())
    {
		goto ERASE_EXIT;
	}
	
    ret = AK_TRUE;

ERASE_EXIT:    
    spi_set_unprotect(m_sflash.spi_id, SPI_BUS1);
    return ret;
}



/**
 * @brief read spi flash status
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param status [out] status buffer
 * @return T_BOOL
 */
T_BOOL spi_flash_read_status(T_U16 *status)
{
    T_U8 buf1[1];
    T_U8 status1, status2;

    //read status1
    buf1[0] = SPI_FLASH_COM_RD_STUS1;

    spi_master_write(m_sflash.spi_id, buf1, 1, AK_FALSE);

    if(!spi_master_read(m_sflash.spi_id, &status1, 1, AK_TRUE))
        return AK_FALSE;

    if (m_sflash.param.flag & SFLASH_FLAG_COM_STATUS2)
    {
        //read status 2
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

T_BOOL spi_flash_write_status(T_U16 status)
{
    T_U8 buf[3];
    T_U32 write_cnt;

    //write enable
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
        akprintf(C1, M_DRVSYS, "spi_flash_read: param error\r\n");
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
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

static T_BOOL flash_read_dual(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf_cmd[5];
    T_U16 status;
    T_U32 addr = page * m_sflash.param.page_size;
    T_U32 count;
    T_U32 readlen;
    T_BOOL bReleaseCS=AK_FALSE;
    
    if((page > 0xffff) || (AK_NULL == buf))
    {
        akprintf(C1, M_DRVSYS, "spi_flash_read: param error\r\n");
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return AK_FALSE;
    }

    buf_cmd[0] = SPI_FLASH_COM_D_READ;
    buf_cmd[1] = (T_U8)((addr >> 16) & 0xFF);
    buf_cmd[2] = (T_U8)((addr >> 8) & 0xFF);
    buf_cmd[3] = (T_U8)(addr & 0xFF);
    buf_cmd[4] = 0;

    // cmd
    if (!spi_master_write(m_sflash.spi_id, buf_cmd, 5, AK_FALSE))
    {
        akprintf(C1, M_DRVSYS, "spi flash_read_quad: write cmd fail\r\n");
        return AK_FALSE;
    }

    // set spi 2-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS2);

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
            // set spi 1-wire
            spi_data_mode(m_sflash.spi_id, SPI_BUS1);
            return AK_FALSE;
        }
        buf += readlen;        
    }
    
    // set spi 1-wire
    spi_data_mode(m_sflash.spi_id, SPI_BUS1);

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
        akprintf(C1, M_DRVSYS, "spi_flash_read: param error\r\n");
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
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
        akprintf(C1, M_DRVSYS, "spi flash_read_quad: write cmd fail\r\n");
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

static T_BOOL flash_write_standard(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U8 buf1[4];
    T_U32 i;
    
    if((page > 0xffff) || (AK_NULL == buf))
    {
        akprintf(C1, M_DRVSYS, "spi_flash_write: param error\r\n");
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return AK_FALSE;
    }

    for(i = 0; i < page_cnt; i++)
    {
        if(m_sflash.param.flag & SFLASH_FLAG_AAAI)
        {
            //auto address increment word program
            if(!write_page_aai(page + i, buf + i*m_sflash.param.page_size))
            {
                akprintf(C1, M_DRVSYS, "write aai fail: %d\n");
                return AK_FALSE;
            }
        }
        else
        {
            //regular page write
            if(!write_page(page + i, buf + i*m_sflash.param.page_size))
            {
                akprintf(C1, M_DRVSYS, "write page fail: %d\n");
                return AK_FALSE;
            }
        }
    }

    return AK_TRUE;
}

static T_BOOL flash_write_quad(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
    T_U32 i;

    if((page > 0xffff) || (AK_NULL == buf))
    {
        akprintf(C1, M_DRVSYS, "spi_flash_write: param error\r\n");
        return AK_FALSE;
    }

    if(!m_sflash.bInit)
    {
        akprintf(C1, M_DRVSYS, "spi flash not init\r\n");
        return AK_FALSE;
    }
    
    // write data
    for(i = 0; i < page_cnt; i++)
    {
        //quad page write
        if(!write_page_quad(page + i, buf + i*m_sflash.param.page_size))
        {
            akprintf(C1, M_DRVSYS, "write page quad fail: %d\n", page+i);
            return AK_FALSE;
        }
    }

    return AK_TRUE;
}

/**
 * @brief write data to one page of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param buf [in]  buffer to be write
 * @return T_BOOL
 */
static T_BOOL write_page(T_U32 page, T_U8 *buf)
{
    T_U8 buf1[4];
    T_U32 addr = page * m_sflash.param.page_size;
    T_U32 i;
    
    for(i = 0; i < m_sflash.param.page_size / m_sflash.param.program_size; i++)
    {
        write_enable();

        buf1[0] = SPI_FLASH_COM_PROGRAM;
        buf1[1] = (addr >> 16) & 0xFF;
        buf1[2] = (addr >> 8) & 0xFF;
        buf1[3] = (addr >> 0) & 0xFF;

        spi_master_write(m_sflash.spi_id, buf1, 4, AK_FALSE);

        if(!spi_master_write(m_sflash.spi_id, buf + i*m_sflash.param.program_size, m_sflash.param.program_size, AK_TRUE))
            return AK_FALSE;
        
        if(!check_status_done())
            return AK_FALSE;

        addr += m_sflash.param.program_size;
    }
    
    return AK_TRUE;
}

static T_BOOL write_page_aai(T_U32 page, T_U8 *buf)
{
    T_U8 buf1[6];
    T_U32 addr = page * m_sflash.param.page_size;
    T_U32 i;
    
    write_enable();

    buf1[0] = SPI_FLASH_COM_AAI;
    buf1[1] = (addr >> 16) & 0xFF;
    buf1[2] = (addr >> 8) & 0xFF;
    buf1[3] = (addr >> 0) & 0xFF;
    buf1[4] = buf[0];
    buf1[5] = buf[1];

    spi_master_write(m_sflash.spi_id, buf1, 6, AK_TRUE);

    if(!check_status_done())
        return AK_FALSE;

    for (i = 2; i < 256; i+=2)
    {
        buf1[1] = buf[i];
        buf1[2] = buf[i+1];

        spi_master_write(m_sflash.spi_id, buf1, 3, AK_TRUE);

        if(!check_status_done())
            return AK_FALSE;
    }

    buf1[0] = SPI_FLASH_COM_WRDI;
    spi_master_write(m_sflash.spi_id, buf1, 1, AK_TRUE);
    return AK_TRUE;
}

static T_BOOL write_page_quad(T_U32 page, T_U8 *buf)
{
    T_U8 buf_cmd[4];
    T_U32 addr = page * m_sflash.param.page_size;
    T_U32 i;
    T_U16 status;
    
    for(i = 0; i < m_sflash.param.page_size / m_sflash.param.program_size; i++)
    {
        write_enable();
        buf_cmd[0] = SPI_FLASH_COM_Q_WRITE;
        buf_cmd[1] = (T_U8)((addr >> 16) & 0xFF);
        buf_cmd[2] = (T_U8)((addr >> 8) & 0xFF);
        buf_cmd[3] = (T_U8)(addr & 0xFF);
        spi_master_write(m_sflash.spi_id, buf_cmd, 4, AK_FALSE);

        // set spi 4-wire
        spi_data_mode(m_sflash.spi_id, SPI_BUS4);

        if(!spi_master_write(m_sflash.spi_id, buf + i*m_sflash.param.program_size, m_sflash.param.program_size, AK_TRUE))
        {
            akprintf(C1, M_DRVSYS, "write page quad write data fail: ");
        }
        
        // set spi 1-wire
        spi_data_mode(m_sflash.spi_id, SPI_BUS1);
        
        if(!check_status_done())
        {
            akprintf(C1, M_DRVSYS, "write page quad check done fail: ");
            return AK_FALSE;
        }

        addr += m_sflash.param.program_size;
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
            akprintf(C1, M_DRVSYS, "spiflash check_status_done timeout\n");
            return AK_FALSE;
        }
    }

    return AK_TRUE;
}


