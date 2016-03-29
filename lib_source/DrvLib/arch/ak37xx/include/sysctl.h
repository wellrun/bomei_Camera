/**
* @FILENAME sysctl.h
*
* Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
* @DATE  2006-04-19
* @VERSION 1.0
* @REF 
*/

#ifndef __SYSCTL_H__
#define __SYSCTL_H__

/*以下定义的clock 模块必须独立，不能一个模块包含多个可控制clock的子模块*/
#define CLOCK_DEFAULT_ENABLE            (0)
#define CLOCK_CAMERA_ENABLE             (1<<0)
#define CLOCK_LCD_ENABLE                (1<<1)
#define CLOCK_IRDA_ENABLE               (1<<2)
#define CLOCK_SPI_ENABLE                (1<<3)
#define CLOCK_USB_ENABLE                (1<<4)
#define CLOCK_UART0_ENABLE              (1<<5)
#define CLOCK_UART1_ENABLE              (1<<6)
#define CLOCK_UART2_ENABLE              (1<<7)
#define CLOCK_MAC_ENABLE                (1<<8)
#define CLOCK_MCI1_ENABLE               (1<<9)
#define CLOCK_MCI2_ENABLE               (1<<10)
#define CLOCK_L2_ENABLE                 (1<<11)
#define CLOCK_ADC2_ENABLE               (1<<12)
#define CLOCK_DAC_ENABLE                (1<<13)
#define CLOCK_NBITS                     (14)
#define CLOCK_ENABLE_MAX                (1<<CLOCK_NBITS)

/**clock ctrl1 0x0800000c bit map**/

#define CLOCK_CTRL_IMAGE_VIDEO          (1 << 0)
#define CLOCK_CTRL_CAMERA               (1 << 1)
#define CLOCK_CTRL_LCD                  (1 << 2)
#define CLOCK_CTRL_IRDA                 (1 << 3)
#define CLOCK_CTRL_SPI                  (1 << 4)
#define CLOCK_CTRL_USBOTG               (1 << 5)
#define CLOCK_CTRL_UART1                (1 << 6)
#define CLOCK_CTRL_UART2                (1 << 7)
#define CLOCK_CTRL_UART3                (1 << 8)
#define CLOCK_CTRL_SYSTEM_CORE          (1 << 9)
#define CLOCK_CTRL_RAM                  (1 << 10)
#define CLOCK_CTRL_MAC                  (1 << 11)
#define CLOCK_CTRL_MCI1                 (1 << 13)
#define CLOCK_CTRL_MCI2                 (1 << 14)
#define CLOCK_CTRL_L2                   (1 << 15)
#define CLOCK_CTRL_ADC2                 (1 << 16)
#define CLOCK_CTRL_DAC                  (1 << 17)

#define RESET_IMAGE_VIDEO               0
#define RESET_CAMERA                    1
#define RESET_LCD                       2
#define RESET_IRDA                      3
#define RESET_SPI                       4
#define RESET_USB_OTG                   5
#define RESET_UART1                     6
#define RESET_UART2                     7
#define RESET_UART3                     8
#define RESET_SYSTEM_CORE               9
#define RESET_RAM                       10
#define RESET_MAC                       11
#define RESET_MCI1                      13
#define RESET_MCI2                      14
#define RESET_L2                        15
#define RESET_ADC2                      16
#define RESET_DAC                       17

/**
 * @BRIEF Set SleepMode
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM[in] T_U32 module
 * @RETURN T_VOID
 * @RETVAL
 * attention: if you close some parts such as LCD 
            you must init it again when you reopen 
            it 
            some settings may cause serious result
            better not to use it if not familar
 */
T_VOID sysctl_clock(T_U32 module);

/**
 * @brief get module clock states
 * @author LHS
 * @date 2011-10-26
 * @param module [in]: module to be get states
 * @return T_BOOL: return TURE mean clock is enable.
 */
T_BOOL sysctl_get_clock_state(T_U32 module);

/**
 * @brief reset module 
 * @author guoshaofeng
 * @date 2010-07-20
 * @param module [in]: module to be reset
 * @return T_VOID
 */
T_VOID sysctl_reset(T_U32 module);

#endif //__SYSCTL_H__
