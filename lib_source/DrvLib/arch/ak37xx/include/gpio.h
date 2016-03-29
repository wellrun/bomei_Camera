/**
 * @file gpio.h
 * @brief gpio function header file
 *
 * This file define gpio macros and APIs: intial, set gpio, get gpio. etc.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Miaobaoli
 * @date 2005-07-24
 * @version 1.0
 *
 * @note:
 * 1. 对于mmi系统中已定义了的gpio，不需要删除相关代码，只需将其定义为INVALID_GPIO
 
 * 2. 如果需要用到扩展io，只需要打开GPIO_MULTIPLE_USE宏，并设置对应的gpio
 *    GPIO_EXPAND_OUT1和GPIO_EXPAND_OUT2，如果只有一组扩展io,可以将GPIO_EXPAND_OUT2
 *	  设为INVALID_GPIO即可
 * 
 * 3. 对于不同的硬件板请以宏隔开并配置好相应宏定义
 *
 */

#ifndef __GPIO_H__
#define __GPIO_H__

#include "drv_gpio.h"

/**
 * @brief share component
 * 
 */
typedef enum
{
    eShare_pwm0_gpio0 = 1,
    eShare_pwm0_gpio42,
    
    eShare_pwm1_gpio1,
    eShare_pwm1_gpio43,
    
    eShare_uart1_rxd_gpio2,
    eShare_uart1_txd_gpio14,
    eShare_uart1_rxd_gpio33,
    eShare_uart1_txd_gpio32,
    
    eShare_uart2_txd_gpio8,
    eShare_uart2_rxd_gpio19,
    eShare_uart2_txd_gpio15,
    eShare_uart2_txd_gpio34,
    eShare_uart2_rxd_gpio35,
    
    eShare_uart3_txd_gpio9,
    eShare_uart3_txd_gpio16,
    eShare_uart3_cts_rts_gpio21_22,
    eShare_uart3_rxd_gpio20,
    eShare_uart3_txd_gpio36,
    eShare_uart3_rxd_gpio37,
    eShare_uart3_rts_gpio38,
    eShare_uart3_cts_gpio39,
    
    eShare_25M_out_gpio2,
    eShare_clk_25M_out_gpio78,
    eShare_27M_out_gpio2,
    
    eShare_mci0_mcmd_gpio4,
    eShare_mci0_mck_gpio6,
    eShare_mci0_mdata_0_gpio10,
    eShare_mci0_mdata_1_gpio11,
    eShare_mci0_mdata_2_gpio12,
    eShare_mci0_mdata_3_gpio13,
    eShare_mci0_mdata_4_gpio8,
    eShare_mci0_mdata_5_gpio9,
    eShare_mci0_mdata_6_gpio16,
    eShare_mci0_mdata_7_gpio17,
    
    eShare_mci1_mcmd_gpio4,
    eShare_mci1_mck_gpio3,
    eShare_mci1_mdata_0_gpio20,
    eShare_mci1_mdata_1_2_gpio21_22,
    eShare_mci1_mdata_3_gpio17,
    eShare_mci1_mdata_0_gpio8,
    eShare_mci1_mdata_1_gpio9,
    eShare_mci1_mdata_2_gpio16,
    eShare_mci1_mcmd_gpio19,
    eShare_mci1_mcmd_gpio35,
    eShare_mci1_mck_gpio34,
    eShare_mci1_mdata_0_gpio36,
    eShare_mci1_mdata_1_gpio37,
    eShare_mci1_mdata_2_gpio38,
    eShare_mci1_mdata_3_gpio39,
    eShare_mci1_mdata_0_3_mcmd_mck_gpio60_65,
    
    eShare_spi_cs_gpio7,
    eShare_spi_hold_gpio8,
    eShare_spi_wp_gpio9,
    eShare_spi_dout_gpio10,
    eShare_spi_clk_gpio11,
    eShare_spi_din_gpio12,
    
    eShare_irda_rx_gpio14,
    
    eShare_lcd_mpu_data_0_7_gpio24_31,
    eShare_lcd_mpu_data_8_gpio32,
    eShare_lcd_mpu_data_9_gpio33,
    eShare_lcd_mpu_data_10_gpio34,
    eShare_lcd_mpu_data_11_gpio35,
    eShare_lcd_mpu_data_12_gpio36,
    eShare_lcd_mpu_data_13_gpio37,
    eShare_lcd_mpu_data_14_gpio38,
    eShare_lcd_mpu_data_15_gpio39,
    eShare_lcd_mpu_a0_dat_cmd_cs_gpio40_41,
    eShare_lcd_mpu_rd_gpio42,
    eShare_lcd_mpu_wr_gpio43,
    
    eShare_lcd_rgb_data_0_7_gpio24_31,
    eShare_lcd_rgb_data_8_gpio32,
    eShare_lcd_rgb_data_9_gpio33,
    eShare_lcd_rgb_data_10_gpio34,
    eShare_lcd_rgb_data_11_gpio35,
    eShare_lcd_rgb_data_12_gpio36,
    eShare_lcd_rgb_data_13_gpio37,
    eShare_lcd_rgb_data_14_gpio38,
    eShare_lcd_rgb_data_15_gpio39,
    eShare_lcd_rgb_vsync_hsync_gpio40_41,
    eShare_lcd_rgb_gate_gpio42,
    eShare_lcd_rgb_pclk_gpio43,
    
    eShare_cis_hsync_vsync_gpio44_45,
    eShare_cis_data_0_gpio46,
    eShare_cis_data_1_gpio47,
    eShare_cis_data_2_gpio48,
    eShare_cis_data_3_gpio49,
    eShare_cis_data_4_7_gpio50_53,
    eShare_cis_sclk_pclk_gpio54_55,
    
    eShare_dbg_rtck_gpio48,
    eShare_dbg_rst_gpio49,
    eShare_dbg_tdo_tck_tdi_tms_gpio50_53,
    
    eShare_dram_ba_0_gpio56,
    eShare_dram_ba_1_gpio57,
    eShare_dram_addr_11_gpio58,
    eShare_dram_addr_12_gpio59,
    eShare_dram_cs_gpio77,
    
    eShare_MAC1_gpio60_65,
    eShare_MAC2_gpio66_76,

    eShare_dummy
}
E_SHARE_COMPONENT;


typedef struct
{
    E_GPIO_PIN_SHARE_CONFIG func_module;
    
    T_U32 pin_cfg[4];
    T_U32 pin_conflict;

    //store reg mask and value here, space for time
    T_U32 reg_mask[3];
    T_U32 reg_value[3];
}
T_SHARE_CFG_FUNC_MODULE;

typedef struct
{
    T_U8            gpio_start;
    T_U8            gpio_end;
    T_U8            reg_num;
    T_U32           bit_mask;
    T_U32           bit_value;
    E_GPIO_PIN_SHARE_CONFIG module;
}
T_SHARE_COMPONENT;

typedef struct
{
    T_U8 gpio_start;
    T_U8 gpio_end;
    T_U8 bit;
}
T_SHARE_CFG_GPIO;

T_U32 gpio_pin_check(T_U32 pin);
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
T_BOOL gpio_get_uart_pin(T_UART_ID uart_id, T_U32* clk_pin, T_U32* data_pin);


/**
 * @brief init share pin structure and set default share pin for each module
 * @author  liao_zhijun
 * @date 2014-04-03
 * @return T_VOID
*/
T_VOID gpio_share_pin_init(T_VOID);

#endif //#ifndef __GPIO_H__

