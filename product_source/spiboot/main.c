#include "anyka_types.h"
#include <stdarg.h>
#include "hal_spiflash.h"
#include "arch_uart.h"
#include "freq.h"
#include "memapi.h"
#include "Fwl_compress.h"
#include "anyka_cpu.h"


#define     SPIBOOT_VER                     "spiboot V1.2.00"

#ifdef DEBUG_OUTPUT
#define     DEG_TEST printf
#else
#define     DEG_TEST 
#endif


#define RAM_SIZE_8MB             (8<<20)

#define SYS_STACK_SIZE           (64<<10)
#define SYS_HEAP_SIZE            (2<<20 | 512<<10) //(3<<20)
#define SYS_BOOT_SIZE            (512<<10)
#define _MMUTT_SIZE				 (16<<10)


static T_U32	ram_addr_start_fixed  = 0x30000000U;
static T_U32	sys_boot_start_fixed  = 0x30770000U;
static T_U32	sys_stack_start_fixed = 0x307f0000U;

static T_U32	mmutt_addresss_start = 0;

static T_U32 	sys_heap_start = 0;	//堆起始地址
static T_U32	sys_heap_end   = 0;	//堆起始地址

static T_U32	ram_size = 0;		//内存的实际大小

#ifdef COMPRESS
static T_U32	compress_mmi_bin_size_limit = 0;	// 压缩文件的大小限制
static T_U32	decom_buf_size_limit = 0;			// 解压后的文件占用大小限制
#endif


T_CHIP_CONFIG_INFO chip_config;

typedef void (*pfun)(void);

#if 0   //调试用代码，可以不去掉

T_VOID writezero(T_U8 *data,T_U32 len)
{
    T_U32 i=0;
    
    for(i = 0; i < len; i++)
    {
       data[i] = 0;
    }

}

T_VOID disfun(T_U8 *data,T_U32 len)
{
    T_U32 i=0;
    
    for(i = 0; i < len; i++)
    {
        printf("%02x ",data[i]);
        if((i & 0xF) == 0xF)
            printf("\n");
    }
    printf("Dis len:%d:\n",i);
}
T_VOID writenum_fun(T_U8 *data,T_U32 pos, T_U32 len)
{
    T_U32 i=0;
    len +=pos; 
    for(i = pos; i < len; i++)
    {
       data[i] = i&0xFF;
    }
}
#endif

T_BOOL readcfgbyname(T_FILE_CONFIG *BiosConfig, T_U8 *fileName)
{
    T_U32 *pData=AK_NULL;
    T_U32 file_cnt;
    T_FILE_CONFIG *pFileCfg;
    T_U32 i;
    T_BOOL bMatch;
    
    if (AK_NULL == fileName)
    {
        printf("name is NULL\n");
        return AK_FALSE;
    }
    
    pData = (T_U32 *)ram_addr_start_fixed;
    //read file info;
    if(!spi_flash_read(SPIBOOT_DATA_START_PAGE, (T_U8 *)pData, 1))
    {
        printf("read page:%d info fail\n",SPIBOOT_DATA_START_PAGE);
        while(1);
    }
    
    memcpy(&file_cnt, pData, sizeof(T_U32));
    DEG_TEST("file cnt:%d\r\n",file_cnt);

    bMatch = AK_FALSE;
    pData ++; //第一个字存的文件个数,按照小段模式排列
    //烧录各文件信息列表从第五个字节开始
    pFileCfg = (T_FILE_CONFIG *)(pData);
    DEG_TEST("file name:%s\r\n",pFileCfg->file_name);


    for(i = 0; i < file_cnt; i++)
    {
        bMatch = AK_FALSE;
        
        if (0 == strncmp(pFileCfg->file_name,fileName,16))
        {
            memcpy(BiosConfig, pFileCfg, sizeof(T_FILE_CONFIG));
            bMatch = AK_TRUE;
            break;
        }

        pFileCfg++;
    }
    DEG_TEST("bMatch= %d\r\n",bMatch);

    return bMatch;
}

void boot_v10(T_FILE_CONFIG* BiosConfig)
{
    T_U32 i;
    T_U32 *pSrcData = AK_NULL;
    T_VOID (*biosEntryFun)(T_VOID) = AK_NULL;

    T_U32 address=0,decomLen=0;

    //load bios
    printf("%s loading...\n",BiosConfig->file_name);
	
    DEG_TEST("Burn Info:Page=%d,Len=%d bytes,RunAddr=0x%x\n", 
        BiosConfig->start_page,
        BiosConfig->file_length,
        BiosConfig->ld_addr);
	
 #ifdef COMPRESS
	if (BiosConfig->file_length > compress_mmi_bin_size_limit)
	{
		printf("\n@err:MMI bin file length is:0x%x, but limit_size is:0x%x, abort run.\n", BiosConfig->file_length,
			compress_mmi_bin_size_limit);
		while(1);
	}

	if (RAM_SIZE_8MB == ram_size)
	{
    	pSrcData = (T_U32 *)((sys_heap_start - BiosConfig->file_length + 3) & (~0x03));
    }
    else
    {
		pSrcData = (T_U32 *)((ram_addr_start_fixed + RAM_SIZE_8MB + 3) & (~0x03));
    }
#else
    pSrcData = (T_U32 *)BiosConfig->ld_addr;
#endif

    //read bios data
    if(!Fwl_SPI_FileRead(BiosConfig, (T_U8 *)pSrcData, BiosConfig->file_length))
    {
        printf("read BIOS fail\n");
        while(1);
    }

#ifdef COMPRESS
    decomLen = Fwl_DeComImg((T_pDATA)pSrcData, (T_U32)BiosConfig->file_length,(T_pDATA)BiosConfig->ld_addr, decom_buf_size_limit);
	if (decom_buf_size_limit == decomLen)
	{
		printf("\n@err:isn't Decompressed inextenso, abort run.\n");
		while(1);
	}
#else
    decomLen = 1;
#endif

	
    if (decomLen > 0)
    {
        biosEntryFun = (T_VOID(*)(T_VOID))(BiosConfig->ld_addr);
        
        for (i=0;i<1024/4;i++)
        {
			if (((T_U32*)BiosConfig->ld_addr)[i] == 0x50495053)// 0x50495053=SPIP
            {
                memcpy(((T_U32*)BiosConfig->ld_addr)+i+1, &spiflash_param,
                    sizeof(T_SFLASH_STRUCT)); 
                break;
            }
        }
		
        printf("Decompress %s Ok! Jump To 0x%x\n",
            BiosConfig->file_name,
            biosEntryFun);

        biosEntryFun();
    }


}

/**
 * sdram max clk 162M
 * it doesn't work well now
 */
T_VOID sdram_on_change(T_U32 sys_clk)
{
    /* unit in ns
     * refer to sdram spec.
     * tWTR is 1 MCLK, tWR is 2 MCLK
     */
    #define tRAS    50 //RAS active time, min is 45
    #define tRCD    23 //RAS to CAS delay, min is 18
    #define tRP     23 //RAS precharge time, min is 18
    #define tRFC    80 //auto refresh, RAS cycle time, same as tRC

    T_U32 cycle, value, auto_refresh;
    T_U8  t_ras=15, t_rcd=7, t_rp=7, t_rfc=15, t_wtr=3, t_wr=7;   //in clk cycle

    if(sys_clk > 1000000)
        cycle = 1000/(sys_clk/1000000);
    else
        cycle = 1000;

#if 1    
    t_ras = tRAS / cycle + 1; if (t_ras > 15) t_ras = 15;
    t_rcd = tRCD / cycle + 1; if (t_rcd > 7) t_rcd = 7;
    t_rp  = tRP / cycle + 1; if (t_rp > 7) t_rp = 7;
    t_rfc = tRFC / cycle + 1; if (t_rfc > 15) t_rfc = 15;
    t_wtr = 1;
    t_wr  = 2;
#endif

    //update sdram AC charateristics
    value = REG32(SDRAM_CFG_REG2);
    //value &= ~(0x3<<23); //clear wtr
    //value |= t_wtr<<23;    
    value &= ~(0xf<<20); //clear ras
    value |= t_ras<<20;
    //value &= ~(0x7<<13); //clear wr
    //value |= t_wr<<13;    
    value &= ~(0x7<<11); //clear rcd
    value |= t_rcd<<11;
    value &= ~(0x1f<<6); //clear rfc
    value |= t_rfc<<6;
    value &= ~(0x7<<3); //clear rp
    value |= t_rp<<3;
    REG32(SDRAM_CFG_REG2) = value;

    if ((value & 0x7) == 3)
    {
        //16M SDRAM
        auto_refresh = 64000 / 4096 * 1000; //ns
    }
    else if ((value & 0x7) == 4)
    {
        //32M SDRAM
        auto_refresh = 64000 / 8192 * 1000; //ns
    }
    else
    {
        auto_refresh = 64000 / 8192 * 1000; //ns
    }

    //update auto refresh cycle
    auto_refresh = auto_refresh / cycle;
    if (auto_refresh > 0xffff) 
        auto_refresh = 0xffff;
    
    value = REG32(SDRAM_CFG_REG3);
    value &= ~(0xffff);
    value |= auto_refresh;
    //value |= 0xff<<16;
    REG32(SDRAM_CFG_REG3) = value;

    us_delay(10);

}

#define FREQ_PLL_VAL    280 //280 M
#define FREQ_M         (1000*1000) ///1M



//!!!!!don't enable interrupt in this function!!!!!
void CMain()
{
	T_U32 i;
	T_U8  fileName[] = "BIOS";
    T_FILE_CONFIG BiosConfig;

    T_U32 reg_v = 0; 
    T_U32 freq_asic;

	chip_config.uart_ID = uiUART0;


    ram_size = drv_get_ram_size();
    // 根据RAM的大小不同，计算得到不同的内存规划
	if (RAM_SIZE_8MB == ram_size)	//A
	{
		mmutt_addresss_start = ram_addr_start_fixed + RAM_SIZE_8MB - _MMUTT_SIZE;
		
		sys_heap_start = (sys_boot_start_fixed - SYS_HEAP_SIZE + 3) & (~0x03);
		sys_heap_end = (sys_boot_start_fixed - 3) & (~0x03);
		
#ifdef COMPRESS
		compress_mmi_bin_size_limit = (2<<20);		//2MB
		decom_buf_size_limit = (3<<20 | 512<<10);	//3.5MB
#endif
	}
	else							//B
	{
		mmutt_addresss_start = ram_addr_start_fixed + RAM_SIZE_8MB - _MMUTT_SIZE;
		
		sys_heap_start = (ram_addr_start_fixed + ram_size - SYS_HEAP_SIZE + 3) & (~0x03);
		sys_heap_end = (ram_addr_start_fixed + ram_size - 3) & (~0x03);
		
#ifdef COMPRESS
		compress_mmi_bin_size_limit = ram_size - RAM_SIZE_8MB - SYS_HEAP_SIZE;
		decom_buf_size_limit = RAM_SIZE_8MB - (1<<20) - SYS_HEAP_SIZE;
#endif
	}
	
#ifdef COMPRESS
    MMU_Init(mmutt_addresss_start);

    Init_MallocMem(sys_heap_start, sys_heap_end);
#endif

    freq_asic = (FREQ_PLL_VAL/2) * FREQ_M; //140M

    //CPU变速
    
    reg_v = REG32(RTC_CLOCK_DIV_REG);
    // set PLL val
    reg_v &= (~(0x3f));
    reg_v |= 11;
    // set 2*asic = PLL
    reg_v &= ~(0x1f << 21); //ASIC_PRE_DIV
    reg_v &= (~(0xf<<17)); //CLK168M_DIV
    reg_v &= (~(0x7<<6)); //ASIC_DIV
    reg_v |= (1<<14);
    //CPU 2X
    reg_v |= (1<<15);
    REG32(RTC_CLOCK_DIV_REG) = reg_v|(1<<12);
    while(((REG32(RTC_CLOCK_DIV_REG))&(1<<12))==1);

    sdram_on_change(freq_asic); //参数是ASIC_CLK    

    uart_init(chip_config.uart_ID, 115200, get_asic_freq());
    
    l2_init();
    memset(&BiosConfig, 0, sizeof(T_FILE_CONFIG));
    
    printf("\n\nSPIBOOT_VER: %s\n",SPIBOOT_VER);  
  
    sflash_init(); 
    
   if(readcfgbyname(&BiosConfig,fileName))
   {
       boot_v10(&BiosConfig);       
   }
   else
   {
       printf("can not find %s burn info\n",fileName);
	   
	   while(1)
	   {
	   	;
	   }
   }

   while(1);   
}




