#include "anyka_cpu.h"
#include "hal_spiflash.h"

#ifdef DEBUG_OUTPUT
#define    SPI_DEBUG     printf
#else
#define    SPI_DEBUG     printf
#endif
#define    BUF2MEM                0
#define    MEM2BUF                1
#define    SPI_MASTER_TRANFINISH           (1<<8)
#define    SPI_RXBUF_HALFEMPTY             (1<<6)
#define    SPI_TXBUF_HALFEMPTY             (1<<2)
#define    BUF_NULL     0xff
#define SPIFLASH_PAGE_SIZE 256

static T_SFLASH_PARAM param = {0};

void sflash_init()
{
    T_eSPI_BUS bus_wight;

    param.page_size = spiflash_param.page_size;
    param.erase_size = spiflash_param.erase_size;
    param.program_size = spiflash_param.program_size;
    param.clock = spiflash_param.clock;

    param.flag = spiflash_param.flag;//SFLASH_FLAG_COM_STATUS2|SFLASH_FLAG_DUAL_READ|SFLASH_FLAG_QUAD_READ|SFLASH_FLAG_QUAD_WRITE;   //
    param.protect_mask =  spiflash_param.protect_mask;  //0x0;//(0x7<<2);

    if(param.flag & SFLASH_FLAG_QUAD_READ)
        bus_wight = SPI_BUS4;
    else
        bus_wight = SPI_BUS1;
        
    if (!spi_flash_init(SPI_ID0, bus_wight))  //spi bus line ?
    {
        printf("SPI initialized fail!\n");
    }
    spi_flash_set_param(&param);
//    printf("SPI CLK:%d MHZ\n",param.clock/1000000);
    
}

T_S32  Fwl_SPI_FileRead(T_FILE_CONFIG *pFile, T_U8 *buffer, T_U32 count)
{
	T_S32 ret = -1;
	T_U32 page = 0;
	T_U32 page_cnt = 0;
	T_U32 addr = 0;
	T_U32 i;
    
	if ((AK_NULL == buffer)||(AK_NULL == pFile))
	{
		return ret;
	}	
//	printf("Readcnt:%d, Buffer Add:0x%x, &buff:0x%x\n",count,buffer, &buffer);
	if (count > pFile->file_length)
	{
		count = pFile->file_length;
	}
	
	page = pFile->start_page;	
	page_cnt = (count+SPIFLASH_PAGE_SIZE-1)/SPIFLASH_PAGE_SIZE;

//	printf("page: %d, page_cnt: %d\n",page, page_cnt);
    if(!spi_flash_read(page,buffer,page_cnt))
    {       
        printf("read page:%d info fail\n",page);        
        return -1;
    }

//	printf("SPI Read OK:\n");
	return count;
}

