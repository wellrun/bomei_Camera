/**
 * @file drv_gpio.h
 * @brief list gpio operation intefaces.
 *
 * This file define gpio macros and APIs: intial, set gpio, get gpio. etc.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liaozhijun
 * @date 2005-07-24
 * @version 1.0
 *
 */

#ifndef __DRV_SHARE_PIN_H__
#define __DRV_SHARE_PIN_H__

/**
 * @brief uart module share pin struct
 * 
 */
typedef struct
{
    T_U8 txd;
    T_U8 rxd;
    T_U8 cts;
    T_U8 rts;
}
T_SHARE_UART;

/**
 * @brief sd/mmc module share pin struct
 * 
 */
typedef struct
{
    T_U8 clk;
    T_U8 cmd;
    T_U8 dat[8];
}
T_SHARE_SDMMC;

/**
 * @brief spi module share pin struct
 * 
 */
typedef struct
{
    T_U8 cs;
    T_U8 clk;
    T_U8 wp;
    T_U8 hold;
    T_U8 din;
    T_U8 dout;
}
T_SHARE_SPI;

/**
 * @brief i2c module share pin struct
 * 
 */
typedef struct
{
    T_U8 clk;
    T_U8 dat;
}
T_SHARE_I2C;

/**
 * @brief pwm module share pin struct
 * 
 */
typedef struct
{
    T_U8 pin;
}
T_SHARE_PWM;


/**
 * @brief mpu lcd module share pin struct
 * 
 */
typedef struct
{
    T_U8 rd;
    T_U8 wr;
    T_U8 a0;
    T_U8 cs;
    T_U8 dat[16];
}
T_SHARE_LCD_MPU;

/**
 * @brief rgb lcd module share pin struct
 * 
 */
typedef struct
{
    T_U8 pclk;
    T_U8 vsync;
    T_U8 hsync;
    T_U8 gate;
    T_U8 dat[16];
}
T_SHARE_LCD_RGB;

/**
 * @brief camera module share pin struct
 * 
 */
typedef struct
{
    T_U8 mclk;
    T_U8 pclk;
    T_U8 hsync;
    T_U8 vsync;
    T_U8 dat[8];
}
T_SHARE_CAMERA;

/**
 * @brief jtag module share pin struct
 * 
 */
typedef struct
{
    T_U8 rtck;
    T_U8 rst;
    T_U8 tdo;
    T_U8 tck;
    T_U8 tdi;
    T_U8 tms;
}
T_SHARE_JTAG;

/**
 * @brief share module
 * 
 */
typedef enum
{
    ePIN_AS_GPIO = 0,           ///< All pin as gpio
    ePIN_AS_PWM1 = 1,               ///< share pin as PWM1
    ePIN_AS_PWM2 = 2,               ///< share pin as PWM2
    ePIN_AS_UART1 = 3,              ///< share pin as UART1
    ePIN_AS_UART2 = 4,              ///< share pin as UART2
    ePIN_AS_UART3 = 5,              ///< share pin as UART3
    ePIN_AS_SDMMC1 = 6,             ///< share pin as MDAT1, 8 lines
    ePIN_AS_SDMMC2 = 7,             ///< share pin as MDAT2, 4lines
    ePIN_AS_SPI = 8,                ///< share pin as SPI
    ePIN_AS_LCD_MPU = 9,            ///< share pin as MPU LCD
    ePIN_AS_LCD_RGB = 10,            ///< share pin as RGB LCD
    ePIN_AS_CAMERA = 11,             ///< share pin as CAMERA
    ePIN_AS_JTAG = 12,               ///< share pin as JTAG
    ePIN_AS_MAC = 13,                ///< share pin as Ethernet MAC
    ePIN_AS_DRAM = 14,               ///< share pin as RAM Controller
    ePIN_AS_IRDA = 15,               ///< share pin as IRDA
    ePIN_AS_I2C = 16,                ///< share pin as I2C
    ePIN_AS_CLK25MO = 17,            
    ePIN_AS_CLK27MO = 18,            

    ePIN_AS_DUMMY = 19
}E_GPIO_PIN_SHARE_CONFIG;


/**
 * @brief change share pin setting for giving module
 * @author  liao_zhijun
 * @date 2014-04-03
 * @param[in] module : which u want change share pin setting for
 * @param[in] gpios : a array contained all the gpio for the module, 0xFF&0xFE means invalid gpio
 * @param[in] gpio_number : number of members the in gpio array
 * @return T_VOID
 * @demo
 *     T_SHARE_UART s_uart1;
        s_uart1.txd = 14;
        s_uart1 .rxd = 2;
        s_uart1.cts = 0xFF;
        s_uart1.rts = 0xFF;

       gpio_share_pin_set(ePIN_AS_UART1, (T_U8 *)&s_uart1, sizeof(s_uart1));
*/
T_VOID gpio_share_pin_set(E_GPIO_PIN_SHARE_CONFIG module, T_U8 gpios[], T_U32 gpio_num);


/**
 * @brief set gpio share pin as gpio 
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param pin [in]  gpio pin ID
 * @return T_BOOL
 * @retval AK_TRUE set successfully
 * @retval AK_FALSE fail to set
 */
T_BOOL  gpio_set_pin_as_gpio(T_U32 pin);

/**
 * @brief set gpio pin group as specified module used
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] PinCfg enum data. the specified module
 * @return T_BOOL
 * @retval AK_TURE setting successful
 * @retval AK_FALSE setting failed
*/
T_BOOL gpio_pin_group_cfg(E_GPIO_PIN_SHARE_CONFIG PinCfg);

/*@}*/

#endif //#ifndef __ARCH_GPIO_H__

