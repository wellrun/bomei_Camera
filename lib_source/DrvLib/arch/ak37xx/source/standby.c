/**
 * @file
 * @brief ANYKA software
 * this file will constraint the function of enter & exit standby model
 *
 * @author Zou tianxiang
 * @date 2008-08-30
 * @version 1.0
 */
 
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "standby.h"


/**
    该函数仅在编译standby代码时用到
 */
#ifdef __CC_ARM
#pragma arm section code = "__inner_"
#endif
T_VOID  enter_standby(T_U32 param) 
{
    T_U32 i, value;

    for(i=0; i<20000; i++);

    value = *(volatile unsigned int*)(0x2002d000);
    value &= (1UL<<31);   //keep cas_latency
    
    /*
                            CKE     CS      RAS     CAS     WE 
        bit                 30      19      18      17      16
        auto-refresh        H       L       L       L       H
        enter self-refresh  L       L       L       L       H
        exit self-refresh   H       L       H       H       H
     */
     
    //send precharge command
    *(volatile unsigned int*)(0x2002d000) = value | 0x40120400; // very important!

    //send NOP command
    *(volatile unsigned int*)(0x2002d000) = value | 0x40170000;
        
    //enter self refresh     
    *(volatile unsigned int*)(0x2002d000) = value | 0x00110000; //CKE low

    //after enter self-refreh, wait  serveral clock-cycle before stop dram external clock
    for (i=0; i<0x100; i++);

    //enter standby
    *(volatile unsigned int*)(0x08000004) |= (1<<13);

    for(i=0; i<20000; i++);    //ensure CLK stable, at least 100us

    //exit self refresh and send NOP command
    *(volatile unsigned int*)(0x2002d000) = value | 0x40170000; //CKE high
    
    //send two NOP command
#ifdef __GNUC__
    __asm("nop");
    __asm("nop");
#endif
#ifdef __CC_ARM
    __asm
    {
        nop
        nop
    }
#endif   
        
    //send auto-refresh command
    *(volatile unsigned int*)(0x2002d000) = value | 0x60110000;

    for(i=0; i<5000; i++);

}
#ifdef __CC_ARM
#pragma arm section
#endif

/* end of file */
