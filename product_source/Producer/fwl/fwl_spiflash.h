#ifndef _FWL_SPI_FLASH_H_
#define _FWL_SPI_FLASH_H_

#include "hal_spiflash.h"

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
    T_U32 total_page;
    T_U32 page_size;
    T_U32 PagesPerBlock;
    fSPIFlash_WritePage WritePage;
    fSPIFlash_ReadPage ReadPage;
    fSPIFlash_EraseBlock EraseBlock;
};

T_BOOL Fwl_SPIHWInit(T_U32* ChipID, T_U32* ChipCnt);
T_PSPIFLASH Fwl_SPIFlash_Init(T_SFLASH_PARAM *param);
E_SPIFLASHERRORCODE Fwl_SPIFlash_WritePage(T_PSPIFLASH spiFlash, T_U32 page, const T_U8 data[]);
E_SPIFLASHERRORCODE Fwl_SPIFlash_ReadPage(T_PSPIFLASH spiFlash, T_U32 page, T_U8 data[]);
E_SPIFLASHERRORCODE Fwl_SPIFlash_EraseBlock(T_PSPIFLASH spiFlash, T_U32 block);


T_U32 FHA_Spi_Erase(T_U32 nChip,  T_U32 nPage);
T_U32 FHA_Spi_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);
T_U32 FHA_Spi_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);
#endif
