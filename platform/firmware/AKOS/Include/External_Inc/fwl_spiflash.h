/************************************************************************
 * Copyright (c) 2011, Anyka Co., Ltd. 
 * All rights reserved.    
 *  
 * File Name：
 * Function：this header file is for spi flash read.
 *
 * Author：luheshan
**************************************************************************/
#ifdef SPIBOOT

#ifndef _FWL_SPI_FLASH_H_
#define _FWL_SPI_FLASH_H_

#include "hal_spiflash.h"

#define	SPIFLASH_PAGE_SIZE	256
#define	SPI_CFG_PAGE 		257

#define SPI_FSEEK_CUR      1
#define SPI_FSEEK_END      2
#define SPI_FSEEK_SET      0

typedef enum tag_SPIFlashErrorCode
{
    SF_SUCCESS        =    ((T_U16)1),
    SF_FAIL           =    ((T_U16)0),           //FOR DEBUG/
}E_SPIFLASHERRORCODE; 

typedef struct SPIFlash T_SPIFLASH;
typedef struct SPIFlash* T_PSPIFLASH;

typedef E_SPIFLASHERRORCODE  (*fSPIFlash_WritePage)(T_PSPIFLASH spiFlash, T_U32 page, const T_U8 data[]);
typedef E_SPIFLASHERRORCODE  (*fSPIFlash_ReadPage)(T_PSPIFLASH spiFlash, T_U32 page, T_U8 data[]);
typedef E_SPIFLASHERRORCODE  (*fSPIFlash_EraseBlock)(T_PSPIFLASH spiFlash, T_U32 block);

struct SPIFlash
{
    T_U32 chip_id;
    T_U32 page_size;
    T_U32 PagesPerBlock;
    fSPIFlash_WritePage WritePage;
    fSPIFlash_ReadPage ReadPage;
    fSPIFlash_EraseBlock EraseBlock;
};

typedef struct
{
	T_U32 file_length;
    T_U32 ld_addr;
    T_U32 start_page;
    T_U32 backup_page;        //backup data start page
    T_U8 file_name[16];
}
T_FILE_CONFIG;

typedef struct _FILE_CURRENT{
	T_S32					offset;
    T_FILE_CONFIG 			fileCfg;  
}T_FILE_CURRENT;

typedef struct
{
    T_SFLASH_PARAM param;
    T_U8 name[32];
}
T_SFLASH_STRUCT;

T_BOOL Fwl_SPIFlash_Init(T_VOID);

T_SFLASH_STRUCT Fwl_SPIFlash_GetSPI_Inio(T_VOID);


T_BOOL Fwl_SPIFlash_ReadCfg(T_FILE_CONFIG *pFikeConfig, const T_U8 name[]);

E_SPIFLASHERRORCODE Fwl_SPIFlash_WritePage(T_U32 page, const T_U8 data[], T_U32 page_cnt);

E_SPIFLASHERRORCODE Fwl_SPIFlash_ReadPage(T_U32 page, T_U8 *data, T_U32 page_cnt);

T_FILE_CURRENT *Fwl_SPI_FileOpen(T_pCSTR filename);


T_U32  Fwl_SPI_GetFileLen(T_FILE_CURRENT * pFile);


T_S32   Fwl_SPI_FileSeek(T_FILE_CURRENT *pFile, T_S32 offset, T_U16 origin);



T_S32   Fwl_SPI_FileRead(T_FILE_CURRENT *pFile, T_U8* buffer, T_U32 count);


T_BOOL  Fwl_SPI_FileClose(T_FILE_CURRENT **pFile);
T_BOOL Fwl_SPI_Fha_Init(T_VOID);


T_BOOL SpiInitPreUpdate(T_VOID);
T_BOOL  Spi_write_bin_begin(T_U32 data_length, T_U32 ld_addr, T_U8 *file_name,
							   T_BOOL bBackup, T_BOOL bCheck, T_BOOL bUpdateSelf);
T_BOOL  Spi_write_bin(const T_U8 * pData,  T_U32 data_len);
T_BOOL  Spi_write_boot_begin(T_U32 bin_len);
T_BOOL  Spi_write_boot(const T_U8 *pData,  T_U32 data_len);
T_BOOL  Spi_FHA_close(T_VOID);
T_BOOL  SetBootCodeSpiParam(T_U8 *pData, T_U32 len);


 /** 获取空闲块的数量，块大小为64K
* @PARAM 无
* @RETURN  <=0 无空闲块， >0有空闲块
*/  
T_S32 Fwl_SPI_Get_Spare_Block_Num(T_VOID);

 /** 往最后一个块写数据, 块大小为64K
* @PARAM pData[输入] :  写入spiflash的数据指针 
* @PARAM count[输入] :   写入数据的字节数
* @RETURN  AK_TRUE: 成功， AK_FALSE :失败
*/  
T_BOOL Fwl_Write_Last_Block_SafeLy(T_U8 * pData, T_U32 count);

 /**从最后一个块读取数据 , 块大小为64K
* @PARAM pData[输出] : 指向存放读取的spiflash数据的指针 
* @PARAM count[输入] :   读取数据的字节数
* @RETURN  AK_TRUE: 成功， AK_FALSE :失败
*/  
T_BOOL Fwl_Read_Last_Block(T_U8 * pData, T_U32 count);

#endif
#endif

