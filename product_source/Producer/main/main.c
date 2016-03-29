#include "anyka_types.h"
#include "anyka_bsp.h"
#include "drv_api.h"
#include "Fwl_osMalloc.h"
#include "version.h"
#include "stdarg.h"
  
#define DBG_TRACE_LOGLEN        2048

static T_CHR printf_buf[DBG_TRACE_LOGLEN];

int printf(T_pCSTR s, ...)
{
    va_list     args;
        
    va_start(args, s);
    vsnprintf(printf_buf, DBG_TRACE_LOGLEN, (const char*)s, args);   
    puts(printf_buf);
    va_end(args); 

    return 0;
}

void CMain()
{
    T_DRIVE_INITINFO drv_info;
    T_U32 asic_freq;
#ifdef CHIP_AK37XX	
	volatile T_U32 *pComAddr = 0x30700000;
#endif

	T_U32 com = 0;
	
    Fwl_MallocInit();

#ifdef CHIP_AK980X    
    drv_info.chip = CHIP_9802;
#endif

#ifdef CHIP_AK37XX
    drv_info.chip = CHIP_3771;
#endif

    drv_info.fRamAlloc = Fwl_MallocAndTrace;
    drv_info.fRamFree = Fwl_FreeAndTrace;
    drv_init(&drv_info);

    MMU_Init(_MMUTT_STARTADDRESS);

#ifdef CHIP_AK37XX
   com = pComAddr[0];
   
    //init console
    switch (com)
    {
		case 0:
			console_init(uiUART0, CONSOLE_UART, 115200);
			break;
		case 1:
			console_init(uiUART1, CONSOLE_UART, 115200);
			break;
		case 2:
			console_init(uiUART2, CONSOLE_UART, 115200);
			break;
		case 3:
			console_init(uiUART3, CONSOLE_UART, 115200);
			break;
		case 4:
			console_init(MAX_UART_NUM, CONSOLE_UART, 115200);
			break;
		default:
			console_init(uiUART1, CONSOLE_UART, 115200);
			break;
					
    }
	
#endif

#ifdef CHIP_AK980X 
    console_init(uiUART0, CONSOLE_UART, 115200);
#endif


#ifdef CHIP_AK37XX
	printf("Addr:%x, com:%x\n", pComAddr, com);
    printf("chip type:AK37XX\n");
#endif

#ifdef CHIP_AK980X  
    printf("chip type:AK980X\r\n");
#endif
    
    printf("producer version:%d.%d.%02d\r\n", MAIN_VERSION, SUB1_VERSION, SUB2_VERSION);

    asic_freq = get_asic_freq();

    printf("asic: %d\n", asic_freq);
    printf("pll: %dMhz\n", get_pll_value());
    
    //event loop
    printf("\r\nEnter transc handle ...... \r\n");
    
    Prod_Transc_Main();
	
}

//for drvlib, otherwise cannot compile
T_VOID lcdbl_set_brightness()
{
}

