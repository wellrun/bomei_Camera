/**
 * @FILENAME: gpio_cfg.c
 * @BRIEF gpio configuartion driver file
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR liaozhijun
 * @DATE 2008-06-17
 * @VERSION 1.0
 * @REF
 */
#include "anyka_cpu.h"
#include "anyka_types.h" 
#include "drv_api.h"
#include "gpio.h"
#include "drv_gpio.h"

#define     LINE_ITEM                   6
#define     PIN_ATTE_LINE               6
#define     GPIO_ATTTR_FIXED_1          1
#define     GPIO_ATTTR_FIXED_0          0
#define     GPIO_ATTR_UNSUPPORTED       0xffff
#define     MADD_A                      200
#define     MDAT_A                      201
#define     END_FLAG                    0xff

#define     GPIO_MAX                    89

typedef enum
{
    PULLDOWN = 0,
    PULLUP,
    PULLUPDOWN,
    UNDEFINED
}T_GPIO_TYPE;

static T_VOID gpio_set_group_attribute(E_GPIO_PIN_SHARE_CONFIG PinCfg);

//gpio pullup/pulldown reg
static const T_U32 gpio_pull_set_reg[] = {
    GPIO_PULLUPDOWN_REG1, GPIO_PULLUPDOWN_REG2, 
    GPIO_PULLUPDOWN_REG3
};

//gpio io control reg
static const T_U32 gpio_io_control_reg[] = {
    GPIO_IO_CONTROL_REG1
};

//gpio sharepin config reg
static const T_U32 gpio_sharepin_con_reg[] = {
    GPIO_SHAREPIN_CONTROL1, GPIO_SHAREPIN_CONTROL2, GPIO_SHAREPIN_CONTROL3
};

//rtc gpio polarity select control reg
static const T_U32 rtc_gpio_p_control_reg[] = {
    RTC_WAKEUP_GPIO_P_REG1,RTC_WAKEUP_GPIO_P_REG2
};

//rtc gpio status clear control reg
static const T_U32 rtc_gpio_c_control_reg[] = {
    RTC_WAKEUP_GPIO_C_REG1,RTC_WAKEUP_GPIO_C_REG2
};
//rtc gpio enable control reg
static const T_U32 rtc_gpio_e_control_reg[] = {
    RTC_WAKEUP_GPIO_E_REG1,RTC_WAKEUP_GPIO_E_REG2
};
//rtc gpio status control reg
static const T_U32 rtc_gpio_s_control_reg[] = {
    RTC_WAKEUP_GPIO_S_REG1,RTC_WAKEUP_GPIO_S_REG2
};

T_SHARE_CFG_FUNC_MODULE m_share_cfg[ePIN_AS_DUMMY];

static T_SHARE_COMPONENT m_share_component[] = 
{
    {0,     0,      0,      (1<<0),         (1<<0),         ePIN_AS_PWM1},
    {1,     1,      0,      (1<<1),         (1<<1),         ePIN_AS_PWM2},

    {2,     2,      0,      (1<<4)|(1<<2),  (0<<4)|(1<<2),  ePIN_AS_UART1},
    {2,     2,      0,      (1<<4)|(1<<2),  (1<<4)|(0<<2),  ePIN_AS_CLK25MO},
    {2,     2,      0,      (1<<4)|(1<<2),  (1<<4)|(1<<2),  ePIN_AS_CLK27MO},

    {3,     3,      0,      (1<<3),         (1<<3),         ePIN_AS_SDMMC2},

    {4,     4,      0,      (3<<5),         (2<<5),         ePIN_AS_SDMMC1},
    {4,     4,      0,      (3<<5),         (3<<5),         ePIN_AS_SDMMC2},

    {6,     6,      0,      (1<<7),         (1<<7),         ePIN_AS_SDMMC1},
    {7,     7,      0,      (1<<8),         (1<<8),         ePIN_AS_SPI},

    {8,     8,      0,      (7<<10),        (4<<10),        ePIN_AS_UART2},    
    {8,     8,      0,      (7<<10),        (5<<10),        ePIN_AS_SDMMC1},    
    {8,     8,      0,      (7<<10),        (6<<10),        ePIN_AS_SPI},    
    {8,     8,      0,      (7<<10),        (7<<10),        ePIN_AS_SDMMC2},    

    {9,     9,      0,      (7<<13),        (4<<13),        ePIN_AS_UART3},
    {9,     9,      0,      (7<<13),        (5<<13),        ePIN_AS_SDMMC1},
    {9,     9,      0,      (7<<13),        (6<<13),        ePIN_AS_SPI},
    {9,     9,      0,      (7<<13),        (7<<13),        ePIN_AS_SDMMC2},

    {10,    10,     0,      (3<<16),        (2<<16),        ePIN_AS_SDMMC1},
    {10,    10,     0,      (3<<16),        (3<<16),        ePIN_AS_SPI},

    {11,    11,     0,      (3<<18),        (2<<18),        ePIN_AS_SDMMC1},
    {11,    11,     0,      (3<<18),        (3<<18),        ePIN_AS_SPI},

    {12,    12,     0,      (3<<20),        (2<<20),        ePIN_AS_SDMMC1},
    {12,    12,     0,      (3<<20),        (3<<20),        ePIN_AS_SPI},

    {13,    13,     0,      (1<<22),        (1<<22),        ePIN_AS_SDMMC1},

    {14,    14,     0,      (3<<23),        (2<<23),        ePIN_AS_UART1},
    {14,    14,     0,      (3<<23),        (3<<23),        ePIN_AS_IRDA},

    {15,    15,     0,      (1<<25),        (1<<25),        ePIN_AS_UART2},

    {16,    16,     0,      (3<<27),        (1<<27),        ePIN_AS_UART3},
    {16,    16,     0,      (3<<27),        (2<<27),        ePIN_AS_SDMMC1},
    {16,    16,     0,      (3<<27),        (3<<27),        ePIN_AS_SDMMC2},

    {17,    17,     0,      (3<<30),        (2<<30),        ePIN_AS_SDMMC1},
    {17,    17,     0,      (3<<30),        (3<<30),        ePIN_AS_SDMMC2},

    {33,    33,     1,      (3<<0),         (1<<0),         ePIN_AS_LCD_MPU},
    {33,    33,     1,      (3<<0),         (2<<0),         ePIN_AS_LCD_RGB},
    {33,    33,     1,      (3<<0),         (3<<0),         ePIN_AS_UART1},

    {19,    19,     1,      (3<<2),         (2<<2),         ePIN_AS_UART2},
    {19,    19,     1,      (3<<2),         (3<<2),         ePIN_AS_SDMMC2},

    {20,    20,     1,      (3<<4),         (2<<4),         ePIN_AS_UART3},
    {20,    20,     1,      (3<<4),         (3<<4),         ePIN_AS_SDMMC2},

    {21,    22,     1,      (3<<6),         (2<<6),         ePIN_AS_UART3},
    {21,    22,     1,      (3<<6),         (3<<6),         ePIN_AS_SDMMC2},

    {24,    31,     1,      (3<<9),         (2<<9),         ePIN_AS_LCD_MPU},
    {24,    31,     1,      (3<<9),         (3<<9),         ePIN_AS_LCD_RGB},

    {32,    32,     1,      (3<<11),        (1<<11),        ePIN_AS_LCD_MPU},
    {32,    32,     1,      (3<<11),        (2<<11),        ePIN_AS_LCD_RGB},
    {32,    32,     1,      (3<<11),        (3<<11),        ePIN_AS_UART1},

    {34,    34,     1,      (7<<13),        (4<<13),        ePIN_AS_LCD_MPU},
    {34,    34,     1,      (7<<13),        (5<<13),        ePIN_AS_LCD_RGB},
    {34,    34,     1,      (7<<13),        (6<<13),        ePIN_AS_UART2},
    {34,    34,     1,      (7<<13),        (7<<13),        ePIN_AS_SDMMC2},

    {35,    35,     1,      (7<<16),        (4<<16),        ePIN_AS_LCD_MPU},
    {35,    35,     1,      (7<<16),        (5<<16),        ePIN_AS_LCD_RGB},
    {35,    35,     1,      (7<<16),        (6<<16),        ePIN_AS_UART2},
    {35,    35,     1,      (7<<16),        (7<<16),        ePIN_AS_SDMMC2},

    {36,    36,     1,      (7<<19),        (4<<19),        ePIN_AS_LCD_MPU},
    {36,    36,     1,      (7<<19),        (5<<19),        ePIN_AS_LCD_RGB},
    {36,    36,     1,      (7<<19),        (6<<19),        ePIN_AS_UART3},
    {36,    36,     1,      (7<<19),        (7<<19),        ePIN_AS_SDMMC2},

    {37,    37,     1,      (7<<22),        (4<<22),        ePIN_AS_LCD_MPU},
    {37,    37,     1,      (7<<22),        (5<<22),        ePIN_AS_LCD_RGB},
    {37,    37,     1,      (7<<22),        (6<<22),        ePIN_AS_UART3},
    {37,    37,     1,      (7<<22),        (7<<22),        ePIN_AS_SDMMC2},

    {38,    38,     1,      (7<<25),        (4<<25),        ePIN_AS_LCD_MPU},
    {38,    38,     1,      (7<<25),        (5<<25),        ePIN_AS_LCD_RGB},
    {38,    38,     1,      (7<<25),        (6<<25),        ePIN_AS_UART3},
    {38,    38,     1,      (7<<25),        (7<<25),        ePIN_AS_SDMMC2},

    {39,    39,     1,      (7<<28),        (4<<28),        ePIN_AS_LCD_MPU},
    {39,    39,     1,      (7<<28),        (5<<28),        ePIN_AS_LCD_RGB},
    {39,    39,     1,      (7<<28),        (6<<28),        ePIN_AS_UART3},
    {39,    39,     1,      (7<<28),        (7<<28),        ePIN_AS_SDMMC2},

    {40,    41,     2,      (3<<0),         (2<<0),         ePIN_AS_LCD_MPU},
    {40,    41,     2,      (3<<0),         (3<<0),         ePIN_AS_LCD_RGB},

    {42,    42,     2,      (3<<2),         (1<<2),         ePIN_AS_LCD_MPU},
    {42,    42,     2,      (3<<2),         (2<<2),         ePIN_AS_LCD_RGB},
    {42,    42,     2,      (3<<2),         (3<<2),         ePIN_AS_PWM1},

    {43,    43,     2,      (3<<4),         (1<<4),         ePIN_AS_LCD_MPU},
    {43,    43,     2,      (3<<4),         (2<<4),         ePIN_AS_LCD_RGB},
    {43,    43,     2,      (3<<4),         (3<<4),         ePIN_AS_PWM2},

    {44,    45,     2,      (1<<6),         (1<<6),         ePIN_AS_CAMERA},

    {46,    46,     2,      (1<<7),         (1<<7),         ePIN_AS_CAMERA},
    {47,    47,     2,      (1<<8),         (1<<8),         ePIN_AS_CAMERA},

    {48,    48,     2,      (3<<9),         (2<<9),         ePIN_AS_CAMERA},
    {48,    48,     2,      (3<<9),         (3<<9),         ePIN_AS_JTAG},

    {49,    49,     2,      (3<<11),        (2<<11),        ePIN_AS_CAMERA},
    {49,    49,     2,      (3<<11),        (3<<11),        ePIN_AS_JTAG},

    {50,    53,     2,      (3<<13),        (2<<13),        ePIN_AS_CAMERA},
    {50,    53,     2,      (3<<13),        (3<<13),        ePIN_AS_JTAG},

    {54,    55,     2,      (1<<15),        (1<<15),        ePIN_AS_CAMERA},

    {56,    56,     2,      (1<<16),        (1<<16),        ePIN_AS_DRAM},
    {57,    57,     2,      (1<<17),        (1<<17),        ePIN_AS_DRAM},
    {58,    58,     2,      (1<<18),        (1<<18),        ePIN_AS_DRAM},

    {60,    65,     2,      (3<<20),        (2<<20),        ePIN_AS_MAC},
    {60,    65,     2,      (3<<20),        (3<<20),        ePIN_AS_SDMMC2},

    {66,    76,     2,      (1<<22),        (1<<22),        ePIN_AS_MAC},
    {77,    77,     2,      (1<<23),        (1<<23),        ePIN_AS_DRAM},
    {78,    78,     2,      (1<<25),        (1<<25),        ePIN_AS_CLK25MO},
};

//default gpio setting 

static T_SHARE_UART m_share_uart1 = {
    /*.txd = */14,
    /*.rxd = */2,
    /*.cts = */INVALID_GPIO,
    /*.rts = */INVALID_GPIO,
};

static T_SHARE_UART m_share_uart2 = {
    /*.txd = */8,
    /*.rxd = */19,
    /*.cts = */INVALID_GPIO,
    /*.rts = */INVALID_GPIO,
};

static T_SHARE_UART m_share_uart3 = {
    /*.txd = */9,
    /*.rxd = */20,
    /*.cts = */INVALID_GPIO,
    /*.rts = */INVALID_GPIO,
};

static T_SHARE_SDMMC m_share_sdmmc0 = {
    /*.clk = */6,
    /*.cmd = */4,
    /*.dat = */{10, 11, 12, 13, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO},
};

static T_SHARE_SDMMC m_share_sdmmc1 = {
    /*.clk = */3,
    /*.cmd = */19,
    /*.dat = */{20, 21, 22, 17, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO},
};

static T_SHARE_SPI m_share_spi = {
    /*.cs = */7,
    /*.clk = */11,
    /*.wp = */9,
    /*.hold = */8,
    /* .din = */12,
    /*.dout = */10,
};

static T_SHARE_LCD_MPU m_share_lcd_mpu = {
    /*.rd = */42,
   /* .wr = */43,
   /* .a0 = */40,
   /* .cs = */41,
   /* .dat = */{24, 25, 26, 27, 28, 29, 30, 31, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO},
};

static T_SHARE_LCD_RGB m_share_lcd_rgb = {
    /*.pclk = */43,
    /*.gate = */42,
    /*.vsync = */40,
    /*.hsync = */41,
    /*.dat = */{24, 25, 26, 27, 28, 29, 30, 31, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO, INVALID_GPIO},
};

static T_SHARE_CAMERA m_share_camera = {
    /*.mclk = */54,
    /*.pclk = */55,
    /*.hsync = */44,
    /*.vsync = */45,
    /*.dat = */{46, 47, 48, 49, 50, 51, 52, 53},
};

static T_SHARE_PWM m_share_pwm0 = {
    /*.pin = */0,
};

static T_SHARE_PWM m_share_pwm1 = {
    /*.pin = */1,
};

static T_SHARE_JTAG m_share_jtag = {
    /*.rtck = */48,
    /*.rst = */49,
    /*.tdo = */50,
    /*.tck = */51,
    /*.tdi = */52,
    /*.tms = */53,
};

T_U8 m_share_mac[] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76};

T_U8 m_share_dram[] = {56, 57, 58, 77};

T_U8 m_share_irda[] = {14};

/*
    this table contains all the GPIOs whose attribute can be set, the io attribute is one 
    of {IE, PE, PS, SL, DS}, one gpio may support only one or several attributes setting,  
    an element of row 2~5 indicates whether this    attibute setting is aupported by this gpio, 
    if supported, this element tells us how can we set register.

    the value is encoded as:
        bit[0:7]: register addr shift, base addr is CHIP_CONF_BASE_ADDR
        bit[8:15]: start bit

    take value 0x3d4 for example:
    the lower 8-bit is 0xd4, that is the shift, then the address of the register we will configure
    is CHIP_CONF_BASE_ADDR+0x9c

    bit[8:15] of 0x3d4 is 3, that means we shoulde configure the bit[3] of register 
    "CHIP_CONF_BASE_ADDR+0x9c"
*/

static const T_U32 gpio_io_set_table_3771[][PIN_ATTE_LINE] = { 
//                 [bit]    [bit]    [bit]  [bit]   [bit]
    //pin no.      IE         PE       PS     SL     DS      
    13,        0,        1,       2,     3,      4,     
    17,        5,        6,       7,     8,      9,
    39,        10,       11,      12,    13,     14,
    55,        15,       16,      17,    18,     19, 
    INVALID_GPIO,
};
static const T_U32 gpio_set_table_3771[GPIO_MAX][PIN_ATTE_LINE] = { 0 };

/*
    this table contains all the GPIOs attribute: each GPIO is represented by 2-bit,
    which equals to:   01-pullup
                       00-pulldown
                       10-pullup/pulldown
                       11-undefined
*/
const T_U32 gpio_pull_attribute_table_3771[][2] = {  
    0x59555150, /*gpio[0:15]*/          0x55551549, /*gpio[16:31]*/
    0x00559555, /*gpio[32:47]*/         0x553F8000, /*gpio[48:63]*/
    0x0C000005, /*gpio[64:79]*/         0xFFFF4033,/*gpio[80:88]*/
};

static T_BOOL gpio_io_flag; // AK_TRUE:special gpio, AK_FALSE:gennal gpio
static T_VOID share_pin_set_gpio(T_SHARE_CFG_FUNC_MODULE *module, T_U8 gpio)
{
    T_U8 index, offset;

    index = gpio  >> 5;
    offset = gpio & 0x1F;

    module->pin_cfg[index] |= (1<<offset);
}

static T_VOID share_pin_clear_gpio(T_SHARE_CFG_FUNC_MODULE *module, T_U8 gpio)
{
    T_U8 index, offset;

    index = gpio  >> 5;
    offset = gpio & 0x1F;

    module->pin_cfg[index] &= ~(1<<offset);
}

static T_VOID share_pin_update_reg_mask_value_for_gpio(T_SHARE_CFG_FUNC_MODULE *module, T_U8 gpio, T_U32 *reg_mask, T_U32 *reg_value)
{
    T_U32 i, j;

    for(i = 0; i < sizeof(m_share_component)/sizeof(T_SHARE_COMPONENT); i++)
    {
        if((m_share_component[i].gpio_start <= gpio) &&
            (m_share_component[i].gpio_end >= gpio) &&
            (m_share_component[i].module == module->func_module))
        {
            j = m_share_component[i].reg_num;
            reg_mask[j] |= m_share_component[i].bit_mask;
            reg_value[j] |= m_share_component[i].bit_value;

            return;
        }
    }

    //clear bits if this gpio is not suitable for module
    share_pin_clear_gpio(module, gpio);
}

static T_VOID share_pin_update_reg_mask_value(T_SHARE_CFG_FUNC_MODULE *module)
{
    T_U32 i;
    T_U32 index, offset;
    
    //search pin mux list
    for(i = 0; i < GPIO_MAX; i++)
    {
        index = i >> 5;
        offset = i & 0x1F;

        //update reg mask/value for each share component
        if(module->pin_cfg[index] & (1<<offset))
        {
            share_pin_update_reg_mask_value_for_gpio(module, i, module->reg_mask, module->reg_value);
        }
    }
}

static T_VOID share_pin_update_conflict(T_SHARE_CFG_FUNC_MODULE *module1, T_SHARE_CFG_FUNC_MODULE *module2)
{
    T_U32 i;
    
    //just check if they have gpio
    for(i = 0; i < 4; i++)
    {
        if(module1->pin_cfg[i] & module2->pin_cfg[i])
        {
            module1->pin_conflict |= (1<<module2->func_module);
            module2->pin_conflict |= (1<<module1->func_module);
            break;
        }
    }
}

static T_VOID share_pin_update_conflict_module(T_SHARE_CFG_FUNC_MODULE *module)
{
    T_U32 i;

    for(i = ePIN_AS_GPIO; i < ePIN_AS_DUMMY; i++)
    {
        //skip itself
        if(m_share_cfg[i].func_module == module->func_module)
            continue;

        share_pin_update_conflict(module, &m_share_cfg[i]);
    }
}


/**
 * @brief change share pin setting for giving module
 * @author  liao_zhijun
 * @date 2014-04-03
 * @param[in] module : which u want change share pin setting for
 * @param[in] components : a array contained all the share component for the module
 * @param[in] com_number : number of members the in components array
 * @return T_VOID
 * @demo
 *     T_U8 m_mci1_share[] = {eShare_mci1_mcmd_gpio4, eShare_mci1_mck_gpio3, 
                              eShare_mci1_mdata_0_3_mcmd_mck_gpio60_65};

       gpio_share_pin_set(ePIN_AS_SDMMC2, m_mci1_share, sizeof(m_mci1_share));
*/
T_VOID gpio_share_pin_set(E_GPIO_PIN_SHARE_CONFIG module, T_U8 gpios[], T_U32 gpio_num)
{
    T_U32 i;
    T_SHARE_CFG_FUNC_MODULE *pModule;
    
    if((ePIN_AS_GPIO == module) || (AK_NULL == gpios) || (0 == gpio_num))
    {
        return;
    }

    pModule = &m_share_cfg[module];
    memset(pModule, 0, sizeof(T_SHARE_CFG_FUNC_MODULE));
    pModule->func_module = module;
    
    //set share pin
    for(i = 0; i < gpio_num; i++)
    {
        if((0 == gpios[i]) && (module != ePIN_AS_PWM1))
            continue;

        if((0xFF == gpios[i]) || (INVALID_GPIO == gpios[i]))
            continue;

        share_pin_set_gpio(pModule, gpios[i]);
    }

    //update register
    share_pin_update_reg_mask_value(pModule);

    //update conflict
    share_pin_update_conflict_module(pModule);
}

/**
 * @brief init share pin structure and set default share pin for each module
 * @author  liao_zhijun
 * @date 2014-04-03
 * @return T_VOID
*/
T_VOID gpio_share_pin_init(T_VOID)
{
    T_U32 i;

    //gives a default share pin for all
    gpio_share_pin_set(ePIN_AS_PWM1, (T_U8 *)&m_share_pwm0, sizeof(m_share_pwm0));
    gpio_share_pin_set(ePIN_AS_PWM2, (T_U8 *)&m_share_pwm1, sizeof(m_share_pwm1));
    gpio_share_pin_set(ePIN_AS_UART1, (T_U8 *)&m_share_uart1, sizeof(m_share_uart1));
    gpio_share_pin_set(ePIN_AS_UART2, (T_U8 *)&m_share_uart2, sizeof(m_share_uart2));
    gpio_share_pin_set(ePIN_AS_UART3, (T_U8 *)&m_share_uart3, sizeof(m_share_uart3));
    gpio_share_pin_set(ePIN_AS_SDMMC1, (T_U8 *)&m_share_sdmmc0, sizeof(m_share_sdmmc0));
    gpio_share_pin_set(ePIN_AS_SDMMC2, (T_U8 *)&m_share_sdmmc1, sizeof(m_share_sdmmc1));
    gpio_share_pin_set(ePIN_AS_SPI, (T_U8 *)&m_share_spi, sizeof(m_share_spi));
    gpio_share_pin_set(ePIN_AS_LCD_MPU, (T_U8 *)&m_share_lcd_mpu, sizeof(m_share_lcd_mpu));
    gpio_share_pin_set(ePIN_AS_LCD_RGB, (T_U8 *)&m_share_lcd_rgb, sizeof(m_share_lcd_rgb));
    gpio_share_pin_set(ePIN_AS_CAMERA, (T_U8 *)&m_share_camera, sizeof(m_share_camera));
    gpio_share_pin_set(ePIN_AS_JTAG, (T_U8 *)&m_share_jtag, sizeof(m_share_jtag));
    gpio_share_pin_set(ePIN_AS_MAC, (T_U8 *)&m_share_mac, sizeof(m_share_mac));
    gpio_share_pin_set(ePIN_AS_DRAM, (T_U8 *)&m_share_dram, sizeof(m_share_dram));
    gpio_share_pin_set(ePIN_AS_IRDA, (T_U8 *)&m_share_irda, sizeof(m_share_irda));
}

T_BOOL gpio_share_check_conflict(E_GPIO_PIN_SHARE_CONFIG module1, E_GPIO_PIN_SHARE_CONFIG module2)
{
    T_SHARE_CFG_FUNC_MODULE *pModule;


    pModule = &m_share_cfg[module1];

    if(pModule->pin_conflict & (1<<module2))
    {
        return AK_TRUE;
    }

    return AK_FALSE;
}

#if 0
T_VOID gpio_share_print(T_U8 module)
{
    akprintf(C3, M_DRVSYS, "modue: %d\n", m_share_cfg[module].func_module);

    akprintf(C3, M_DRVSYS, "gpio: %x,%x,%x,%x\n", m_share_cfg[module].pin_cfg[0], 
        m_share_cfg[module].pin_cfg[1],
        m_share_cfg[module].pin_cfg[2], 
        m_share_cfg[module].pin_cfg[3]);
    akprintf(C3, M_DRVSYS, "conflict: %x\n", m_share_cfg[module].pin_conflict);

    akprintf(C3, M_DRVSYS, "mask: %x,%x,%x\n", m_share_cfg[module].reg_mask[0], 
        m_share_cfg[module].reg_mask[1],
        m_share_cfg[module].reg_mask[2]);

    akprintf(C3, M_DRVSYS, "value: %x,%x,%x\n", m_share_cfg[module].reg_value[0], 
        m_share_cfg[module].reg_value[1],
        m_share_cfg[module].reg_value[2]);

}
#endif

/**
 * @brief get gpio share pin as uart
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param uart_id [in]  uart id
 * @param clk_pin [in]  clk pin
 * @param data_pin [in]  data pin
 * @return T_BOOL
 * @retval AK_TRUE get successfully
 * @retval AK_FALSE fail to get
 */
T_BOOL gpio_get_uart_pin(T_UART_ID uart_id, T_U32* clk_pin, T_U32* data_pin)
{
    if (uart_id >= MAX_UART_NUM)
    {
        return AK_FALSE;
    }
    switch (uart_id)
    {
        case uiUART0:
            *clk_pin = 1;
            *data_pin = 2;
            break;
        case uiUART1:
            *clk_pin = 3;
            *data_pin = 4;
            break;
        default:
            return AK_FALSE;
    }

    return AK_TRUE;
}


/**
 * @brief set gpio share pin as gpio 
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param pin [in]  gpio pin ID
 * @return T_BOOL
 * @retval AK_TRUE set successfully
 * @retval AK_FALSE fail to set
 */
T_BOOL  gpio_set_pin_as_gpio (T_U32 pin)
{
    T_U32 i;
    T_U32 reg_num, bit_mask;

    //check param
    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return AK_FALSE;
    }

    //loop to find the correct bits to clr in share ping cfg1
    for(i = 0; i < sizeof(m_share_component)/sizeof(T_SHARE_COMPONENT); i++)
    {
        if((pin >= m_share_component[i].gpio_start) && (pin <= m_share_component[i].gpio_end))
        {
            reg_num = gpio_sharepin_con_reg[m_share_component[i].reg_num];
            bit_mask = m_share_component[i].bit_mask;
            REG32(reg_num) &= ~(bit_mask);

            return AK_TRUE;
        }
    }

    return AK_TRUE;
}

static T_VOID gpio_set_group_attribute(E_GPIO_PIN_SHARE_CONFIG PinCfg)
{
    T_U32 pin, start_pin = 0, end_pin = 0;
    
    switch (PinCfg)
    {
        case ePIN_AS_SDMMC1:
            REG32(GPIO_PULLUPDOWN_REG1) &= ~(0xf << 8);
            REG32(GPIO_PULLUPDOWN_REG2) &= ~(0xf << 16);
            REG32(GPIO_IO_CONTROL_REG1) &= ~((1<<11)|(1<<13));
            break;
            
        case ePIN_AS_UART1: 
            start_pin = 1, end_pin = 2;
            for (pin = start_pin; pin <= end_pin; pin++)
                gpio_set_pull_up_r(pin, AK_TRUE);
            break;
            
        case ePIN_AS_UART2: 
            start_pin = 3, end_pin = 4;
            for (pin = start_pin; pin <= end_pin; pin++)
                gpio_set_pull_up_r(pin, AK_TRUE);
            break;

        default:
            break;
    }
}

/**
 * @brief set gpio pin group as specified module used
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] PinCfg enum data. the specified module
 * @return T_BOOL
 * @retval AK_TURE setting successful
 * @retval AK_FALSE setting failed
*/
T_BOOL gpio_pin_group_cfg(E_GPIO_PIN_SHARE_CONFIG PinCfg)
{
    T_U32 i, j;
    T_U32 reg;

    if(ePIN_AS_GPIO == PinCfg)
    {
        //set all pin as gpio except uart1 tx, dram_ba[0,1], dram_addr[11~12], dram_cs
        REG32(gpio_sharepin_con_reg[0]) = (0x2<<23);
        REG32(gpio_sharepin_con_reg[1]) = 0x0;
        REG32(gpio_sharepin_con_reg[2]) = (0x7<<16) | (1<<23);//(0xF<<16) | (1<<23)

        return AK_TRUE;
    }

    for(i = 0; i < sizeof(m_share_cfg)/sizeof(T_SHARE_CFG_FUNC_MODULE); i++)
    {
        if(PinCfg == m_share_cfg[i].func_module)
        {
            //set pull attribute for module
            //gpio_set_group_attribute(PinCfg);
            
            //set share pin cfg reg1
            for(j = 0; j < 3; j++)
            {
                if(m_share_cfg[i].reg_mask[j] != 0)
                {
                    reg = REG32(gpio_sharepin_con_reg[j]);

                    reg &= ~(m_share_cfg[i].reg_mask[j]);
                    reg |= m_share_cfg[i].reg_value[j];

                    REG32(gpio_sharepin_con_reg[j]) = reg;
                }
            }

            return AK_TRUE;
        }
    }

    return AK_FALSE;
}

T_U32 gpio_pin_check(T_U32 pin)
{   
    T_U8 i=0;
    
    while( i < GPIO_MAX)
    {
        if(pin == gpio_io_set_table_3771[i][0])
        {
           gpio_io_flag = AK_TRUE;  //special gpio
           return i;
        }
        else if( INVALID_GPIO == gpio_io_set_table_3771[i][0])
        {
            break;
        }
        i++;
    }
    gpio_io_flag = AK_FALSE;    //general gpio
    if (pin < GPIO_MAX)
        return pin;
    else
        return INVALID_GPIO;
        
}

/*
    attr: to set what kind of attribute of pin
    enable: to enable or disable this pin attribute,1:enable 0:disable

    the gpio pin attribure set is mainly for two kinds of pins: one is general GPIOs, the 
    other is RAM bus(data & addr), for general GPIO, param pin in this function is the 
    GPIO number, for RAM bus, param "pin" should be set as MADD_A or MDAT_A

    NOTE: the following four functions:gpio_set_pin_attribute, gpio_set_pull_up_r,
            gpio_set_pull_down_r, gpio_set_pin_share, you'd better not check the 
            returned value, because it doesn't matter to give a wrong param to a
            specific pin
*/
T_BOOL gpio_set_pin_attribute(T_U32 pin, T_GPIO_PIN_ATTR attr, T_BOOL enable)
{
    T_U32 reg_addr, reg_bit,index; 
    const T_U32 (*gpio_io_set_table)[PIN_ATTE_LINE] = AK_NULL;
    T_U32 i = INVALID_GPIO;
    
    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return AK_FALSE;
    }
      
    if ((i = gpio_pin_check(pin)) == INVALID_GPIO)
    {
        return AK_FALSE;
    }
    
    if(gpio_io_flag)
    {
        gpio_io_set_table = gpio_io_set_table_3771; 
    }    
    else
    {
        gpio_io_set_table = gpio_set_table_3771;
    }    

    /*
        if we want to set this attribute, we should get two things:
        1. which register should be configured
        2. which bit of this register should be configured

        following, we decode corresponding items in gpio_io_set_table to 
        get the above two things
    */
    if (gpio_io_flag == AK_FALSE)
    {
        index = i/32;        
        reg_addr = gpio_pull_set_reg[index];
                   
        if(i<80)    //gpio 80
        {
            reg_bit = i%32;
        }
        else
        {
            reg_bit = i%32 +1 ;
        }
        
        if(enable)
            enable = 0;  //0 is enable
        else
            enable = 1;  //1 is disable
    }
    else
    {
        reg_addr = GPIO_IO_CONTROL_REG1;
        reg_bit = (gpio_io_set_table[i][attr]) & 0xff;
    }
//    printf("GPIO:%d,flag:%d,attr:%d,radd:%x,bit:%d\n",i,gpio_io_flag,attr,reg_addr,reg_bit);
    if (enable)
        *(volatile T_U32*)reg_addr |= 1 << reg_bit;
    else
        *(volatile T_U32*)reg_addr &= ~(1 << reg_bit);

    return AK_TRUE; 
}

/*
    enable: 1:enable pullup 0:disable pullup function

    for 7801,commonly three rules:
    1.  if the pin is attached pullup/pulldown resistor only, then, 
        wrting 0 to the corresponding register bit to enable pullup/pulldown, 
        1 to disable pullup/pulldown!
        
    2.  if the pin is attached pullup and pulldown resistor, then writing 1 to enable
        pullup, 0 to enable pulldown, if you want to disable pullup/pulldown, then 
        disable the PE parameter
*/
T_BOOL gpio_set_pull_up_r(const T_U32 pin, T_BOOL enable)
{
    T_U32 index, reg_data, shift, residual, rs;
    T_GPIO_TYPE reg_bit;
    const T_U32 (*gpio_pull_attribute_table)[2] = AK_NULL;
    T_U8 i=INVALID_GPIO;
    
    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return AK_FALSE;
    }
    if ( (i=gpio_pin_check(pin)) == INVALID_GPIO)
    {
        return AK_FALSE;
    }
    gpio_pull_attribute_table = gpio_pull_attribute_table_3771;
    index = pin / 32;
    residual = pin % 32;
    if (residual < 16)
        shift = 0;
    else
        shift = 1;  
    
    //get pin pullup/pulldown attribute
    reg_data = gpio_pull_attribute_table[index][shift];
    if (shift)
        rs = (residual - 16) * 2;
    else
        rs = residual * 2;
    reg_bit = (reg_data >> rs) & 0x3;   

    if ((reg_bit == PULLDOWN) || (reg_bit == UNDEFINED))
    {
        return AK_FALSE;
    }

    if(enable)
    {
        gpio_set_pin_attribute(pin, GPIO_ATTR_PE, AK_TRUE);
        
        if(gpio_io_flag)
        {
            gpio_set_pin_attribute(pin, GPIO_ATTR_PS, PULLUP);
            gpio_set_pin_attribute(pin, GPIO_ATTR_SL, AK_TRUE);
            gpio_set_pin_attribute(pin, GPIO_ATTR_DS, AK_TRUE);
        }
    }
    else
    {
        gpio_set_pin_attribute(pin, GPIO_ATTR_PE, AK_FALSE);
    }
    return AK_TRUE; 
}


//1.enable pulldown 0.disable pulldown
T_BOOL gpio_set_pull_down_r(const T_U32 pin, T_BOOL enable)
{
    T_U32 index, reg_data, shift, residual, rs;
    T_GPIO_TYPE reg_bit;
    const T_U32 (*gpio_pull_attribute_table)[2] = AK_NULL;
    T_U8 i=INVALID_GPIO;
    
    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return AK_FALSE;
    }
    if ( gpio_pin_check(pin) == INVALID_GPIO)
    {
        return AK_FALSE;
    }
    gpio_pull_attribute_table = gpio_pull_attribute_table_3771;
    index = pin / 32;
    residual = pin % 32;
    if (residual < 16)
        shift = 0;
    else
        shift = 1;  
    
    //get pin pullup/pulldown attribute
    reg_data = gpio_pull_attribute_table[index][shift];
    if (shift)
        rs = (residual - 16) * 2;
    else
        rs = residual * 2;
    reg_bit = (reg_data >> rs) & 0x3;   

    if ((reg_bit == PULLUP) || (reg_bit == UNDEFINED))
    {
        return AK_FALSE;
    }
    


    if(enable)
    {
        gpio_set_pin_attribute(pin, GPIO_ATTR_PE, AK_TRUE);
        
        if(gpio_io_flag)
        {
            gpio_set_pin_attribute(pin, GPIO_ATTR_PS, PULLDOWN);
            gpio_set_pin_attribute(pin, GPIO_ATTR_SL, AK_TRUE);
            gpio_set_pin_attribute(pin, GPIO_ATTR_DS, AK_TRUE);
        }
    }
    else
    {
        gpio_set_pin_attribute(pin, GPIO_ATTR_PE, AK_FALSE);
    }
    return AK_TRUE; 
}

//get wakeup gpio bit mask
T_U8  get_wGpio_Bit(T_U32 pin,T_U32* ctreg)
{
    T_U8 mask_bit = INVALID_GPIO;

    if((AK_FALSE == gpio_assert_legal(pin)) || (pin == 89))
    {
        return INVALID_GPIO;
    }
    
    if (pin < 24)
    {
        mask_bit = pin;
        *ctreg = 0;
    }
    else if ((pin > 31) && (pin < 40))
    {
        mask_bit = pin - 8;        
        *ctreg = 0;
    }
    else if((pin > 39) && (pin < 44))
    {      
        mask_bit = pin - 40;        
        *ctreg = 1;
    }
    else if((pin > 59) && (pin < 77))
    {
        mask_bit = pin - 56;
        *ctreg = 1;
    }    
    else if((pin > 77) && (pin < 81))
    {
        mask_bit = pin - 56;
        *ctreg = 1;
    }    
    else if((pin > 81) && (pin < 87)) //GPIO[86]是没有的，只是用来做AIN KEY判断
    {
        mask_bit = pin - 56;
        *ctreg = 1;
    }    
    else 
    {
        akprintf(C3, M_DRVSYS, "pin %d isn't wakeup GPIO!\n", pin);
        return INVALID_GPIO;
    }

    return mask_bit;

}

T_U32 get_wGpio_Pin(T_U32 mask_bit)
{
    T_U32 pin = INVALID_GPIO;

    if (REG32(RTC_WAKEUP_GPIO_S_REG1) & REG32(RTC_WAKEUP_GPIO_E_REG1))
    {
        if (mask_bit <= 23)
        {
            pin = mask_bit;
        }
        else if ((mask_bit >= 24) && (mask_bit <= 31))
        {
            pin = mask_bit + 8;
        }
    }
    else if(REG32(RTC_WAKEUP_GPIO_S_REG2) & REG32(RTC_WAKEUP_GPIO_E_REG2))
    {
        if (mask_bit <= 3)
            pin = mask_bit + 40;
        else if((mask_bit >= 4) && (mask_bit <= 30))    
            pin = mask_bit + 56;
    }
    else 
    {
        return INVALID_GPIO;
    }
   
    return pin;
}

//polarity: 0: rising edge triggered 1: falling edge triggered
T_VOID gpio_set_wakeup_p(T_U32 pin, T_BOOL polarity)
{
    T_U8 mask_bit;
    T_U32 ctreg;
    if(AK_FALSE == gpio_assert_legal(pin))
    {
        return;
    }

    mask_bit = get_wGpio_Bit(pin,&ctreg);

    if(INVALID_GPIO == mask_bit)
        return;
        
    if (polarity)
    {
        REG32(rtc_gpio_p_control_reg[ctreg]) |= (1 << mask_bit);
    }
    else
    {
        REG32(rtc_gpio_p_control_reg[ctreg]) &= ~(1 << mask_bit);
    }
}


