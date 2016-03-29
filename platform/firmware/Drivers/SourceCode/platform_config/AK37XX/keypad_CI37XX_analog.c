/**
 * @FILENAME: keypad_CI37XX.c
 * @BRIEF keypad CI37XX driver file
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR luheshan
 * @DATE 2012-02-28
 */
#ifdef OS_ANYKA
#ifdef CI37XX_PLATFORM
#include "akdefine.h"
#include "keypad_define.h"
#include "platform_hd_config.h"
#include "gbl_macrodef.h"

#if (KEYPAD_TYPE == 1)

////////////key num define
#if (defined (CHIP_AK3760))
#define AD_MAX_KEY_NUM  7  //ad key number
#elif (defined (CHIP_AK3750))
#define AD_MAX_KEY_NUM  7  //ad key number
#else
#error "must define CHIP_AK37XX"	
#endif


/////////////key val define
#if (defined (CHIP_AK3760))
#define AD_KEY1_VAL    50
#define AD_KEY2_VAL    225
#define AD_KEY3_VAL    380
#define AD_KEY4_VAL    480
#define AD_KEY5_VAL    570
#define AD_KEY6_VAL    650
#define AD_KEY7_VAL    715
#define AK_NO_KEY_VAL  1022
#elif (defined (CHIP_AK3750))
#define AD_KEY1_VAL    0
#define AD_KEY2_VAL    62
#define AD_KEY3_VAL    125
#define AD_KEY4_VAL    189
#define AD_KEY5_VAL    250
#define AD_KEY6_VAL    312
#define AD_KEY7_VAL    374
#define AD_KEY8_VAL    435
#define AD_KEY9_VAL    498
#define AD_KEY10_VAL    560
#define AD_KEY11_VAL    623
#define AD_KEY12_VAL    684
#define AD_KEY13_VAL    748
#define AD_KEY14_VAL    808
#define AD_KEY15_VAL    870
#define AD_KEY16_VAL    930

#define AK_NO_KEY_VAL  1023
#endif


#define AD_VAL_OFFSET   30 // ad value offset 


//////////////////////key data 
#if (defined (CHIP_AK3760))
const KEY_DETECT_STR key_detect[]= 
{
    {AD_KEY1_VAL - AD_VAL_OFFSET, AD_KEY1_VAL + AD_VAL_OFFSET, kbNULL},            
    {AD_KEY2_VAL - AD_VAL_OFFSET, AD_KEY2_VAL + AD_VAL_OFFSET, kbUP},            
    {AD_KEY3_VAL - AD_VAL_OFFSET, AD_KEY3_VAL + AD_VAL_OFFSET, kbDOWN},        
    {AD_KEY4_VAL - AD_VAL_OFFSET, AD_KEY4_VAL + AD_VAL_OFFSET, kbMENU},        
    {AD_KEY5_VAL - AD_VAL_OFFSET, AD_KEY5_VAL + AD_VAL_OFFSET, kbLEFT},        
    {AD_KEY6_VAL - AD_VAL_OFFSET, AD_KEY6_VAL + AD_VAL_OFFSET, kbRIGHT},    
    {AD_KEY7_VAL - AD_VAL_OFFSET, AD_KEY7_VAL + AD_VAL_OFFSET, kbCLEAR},
};

static const T_PLATFORM_KEYPAD_ANALOG platform_keypad = {
    key_detect,
    AD_MAX_KEY_NUM,
    AD_VAL_OFFSET,
    AD_KEY1_VAL - AD_VAL_OFFSET,
    AD_KEY7_VAL+AD_VAL_OFFSET
};
#elif (defined (CHIP_AK3750))
const KEY_DETECT_STR key_detect[]= 
{
    {0, AD_KEY1_VAL  + AD_VAL_OFFSET, kbOK},            
    {AD_KEY5_VAL  - AD_VAL_OFFSET, AD_KEY5_VAL  + AD_VAL_OFFSET, kbUP},            
    {AD_KEY9_VAL  - AD_VAL_OFFSET, AD_KEY9_VAL  + AD_VAL_OFFSET, kbDOWN},     
    {AD_KEY13_VAL - AD_VAL_OFFSET, AD_KEY13_VAL + AD_VAL_OFFSET, kbMENU},       
    {AD_KEY14_VAL - AD_VAL_OFFSET, AD_KEY14_VAL + AD_VAL_OFFSET, kbLEFT},       
    {AD_KEY15_VAL - AD_VAL_OFFSET, AD_KEY15_VAL + AD_VAL_OFFSET, kbRIGHT},    
    {AD_KEY16_VAL - AD_VAL_OFFSET, AD_KEY16_VAL + AD_VAL_OFFSET, kbCLEAR},      
};

static const T_PLATFORM_KEYPAD_ANALOG platform_keypad = {
    key_detect,
    AD_MAX_KEY_NUM,
    AD_VAL_OFFSET,
    0,//AD_KEY1_VAL - AD_VAL_OFFSET,
    AD_KEY16_VAL+AD_VAL_OFFSET
};
#endif

/**
 * @取平台键盘设置参数，此接口会被keypad_init接口调用，每个平台分别实现
 *
 * Function it will be call by keypad_init
 * @author luheshan
 * @date 2012-02-28
 * @param[in] T_VOID
 * @return T_PLATFORM_KEYPAD_PARM *
 * @retval the pointer of platform keypad information 
 */
const T_VOID *keypad_get_platform_parm(T_VOID)
{
    return (T_VOID *)&platform_keypad;
}

T_KEYPAD_TYPE keypad_get_platform_type(T_VOID)
{
    return KEYPAD_ANALOG;
}
#endif
#endif  //#ifdef CI37XX_PLATFORM
#endif
