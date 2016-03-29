#include "anyka_types.h"
#include "fwl_spiflash.h"
#include "fha.h"

static T_PSPIFLASH m_pSpiflash = AK_NULL;

T_BOOL Fwl_SPIHWInit(T_U32 *ChipID, T_U32 *ChipCnt)
{
    if (AK_NULL == ChipID || AK_NULL == ChipCnt)
    {
        return AK_FALSE;
    }
    
    if (!spi_flash_init(SPI_ID0, SPI_BUS1))
    {
        return AK_FALSE;
    }    
 
    *ChipID  = spi_flash_getid();
    *ChipCnt = 1;
    printf("flash_id is 0x%x\n", *ChipID);

    return AK_TRUE;
}

T_PSPIFLASH Fwl_SPIFlash_Init(T_SFLASH_PARAM *param)
{
    T_PSPIFLASH pSPIFlash = AK_NULL;

    if (AK_NULL == param || 0 == param->page_size)
    {
        printf("Fwl_SPIFlash_Init(): Input para is ERROR!\r\n");
        return AK_NULL;
    }

	printf("id:%d, totalsize:%d, pagesize:%d, program size:%d, secsize:%d, clock:%d, flag:0x%x, mask:0x%x\r\n", 
        param->id,
        param->total_size,
        param->page_size,
		param->program_size,
		param->erase_size,
		param->clock,
		param->flag,
		param->protect_mask);
    
    spi_flash_set_param(param);
    pSPIFlash = Fwl_Malloc(sizeof(T_SPIFLASH));

    if(AK_NULL == pSPIFlash || 0 == param->page_size)
    {
        return AK_NULL;
    }

    pSPIFlash->total_page = param->total_size / param->page_size;
    pSPIFlash->page_size  = param->page_size;
    pSPIFlash->PagesPerBlock = param->erase_size / param->page_size;
    pSPIFlash->ReadPage   = Fwl_SPIFlash_ReadPage;
    pSPIFlash->WritePage  = Fwl_SPIFlash_WritePage;
    pSPIFlash->EraseBlock = Fwl_SPIFlash_EraseBlock;

    m_pSpiflash = pSPIFlash;

    return pSPIFlash;
}

E_SPIFLASHERRORCODE Fwl_SPIFlash_WritePage(T_PSPIFLASH spiFlash, T_U32 page, const T_U8 data[])
{  
	if (!spi_flash_write(page, data, 1))
	{
        return SF_FAIL;
	}    
   
    return SF_SUCCESS;
}

E_SPIFLASHERRORCODE Fwl_SPIFlash_ReadPage(T_PSPIFLASH spiFlash, T_U32 page, T_U8 data[])
{
    if (!spi_flash_read(page, data, 1))
    {
        return SF_FAIL;
    }   
    
    return SF_SUCCESS;
}

E_SPIFLASHERRORCODE  Fwl_SPIFlash_EraseBlock(T_PSPIFLASH spiFlash, T_U32 Block)
{
    if (!spi_flash_erase(Block))
    {
        return SF_FAIL;
    }   
        
    return SF_SUCCESS;
}

//**************************************************************************************************************************

T_U32 FHA_Spi_Erase(T_U32 nChip,  T_U32 nPage)
{
    T_U32 nBlock = nPage / m_pSpiflash->PagesPerBlock;

    if ((nBlock + 1) * m_pSpiflash->PagesPerBlock > m_pSpiflash->total_page)
    {
        printf("erase sector %d over the max space\r\n", nBlock);
        return SF_FAIL;
    } 

    if (!spi_flash_erase(nBlock))
    {
        return FHA_FAIL;
    }   
        
    return FHA_SUCCESS;
}

T_U32 FHA_Spi_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{ 
#if 1

	T_U32 i;

    if ((nPage + nDataLen) > m_pSpiflash->total_page)
    {
        printf("read page %d over the max space\r\n", (nPage + nDataLen));
        return SF_FAIL;
    } 

	for (i = 0; i < nDataLen; i++)
	{
		if (!spi_flash_read(nPage + i, pData + i * m_pSpiflash->page_size, 1))
		{
			return FHA_FAIL;
		}
	}

#else
	
    if (!spi_flash_read(nPage, pData, nDataLen))
    {
        return FHA_FAIL;
    }   
#endif

    return FHA_SUCCESS;
}

T_U32 FHA_Spi_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{	
    if ((nPage + nDataLen) > m_pSpiflash->total_page)
    {
        printf("write page %d over the max space\r\n", (nPage + nDataLen));
        return SF_FAIL;
    } 
        
    if (!spi_flash_write(nPage, pData, nDataLen))
	{
        return FHA_FAIL;
	}    
   
    return FHA_SUCCESS;
}

 
