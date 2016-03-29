/**
 * @file camera.c
 * @brief camera function file
 * This file provides camera APIs: open, capture photo
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2011-03-30
 * @version 1.0
 * @note ref AK980x technical manual.
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "camera.h"
#include "hal_probe.h"
#include "interrupt.h"
#include "sysctl.h"
#include "drv_gpio.h"

//define the image sensor controller register address
#define IMG_CMD_REG            REG32(IMG_CMD_ADDR)
#define IMG_HINFO1_REG         REG32(IMG_HINFO1_ADDR)
#define IMG_HINFO2_REG         REG32(IMG_HINFO2_ADDR)
#define IMG_VINFO1_REG         REG32(IMG_VINFO1_ADDR)
#define IMG_VINFO2_REG         REG32(IMG_VINFO2_ADDR)
#define IMG_YADDR_ODD_REG      REG32(IMG_YADDR_ODD)
#define IMG_UADDR_ODD_REG      REG32(IMG_UADDR_ODD)
#define IMG_VADDR_ODD_REG      REG32(IMG_VADDR_ODD)
#define IMG_RGBADDR_ODD_REG    REG32(IMG_RGBADDR_ODD)
#define IMG_YADDR_EVE_REG      REG32(IMG_YADDR_EVE)
#define IMG_UADDR_EVE_REG      REG32(IMG_UADDR_EVE)
#define IMG_VADDR_EVE_REG      REG32(IMG_VADDR_EVE)
#define IMG_RGBADDR_EVE_REG    REG32(IMG_RGBADDR_EVE)
#define IMG_CONFIG_REG         REG32(IMG_CONFIG_ADDR)
#define IMG_STATUS_REG         REG32(IMG_STATUS_ADDR)
#define IMG_NUM_REG            REG32(IMG_NUM_ADDR)

#define TIME_OUT_VALUE    6000

/**
 * @brief reset camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return T_VOID
 */
T_VOID camctrl_enable(T_VOID)
{
    sysctl_clock(CLOCK_CAMERA_ENABLE);
    gpio_pin_group_cfg(ePIN_AS_CAMERA);

    //reset camera interface
    sysctl_reset(RESET_CAMERA);
    
    //enbale PLL2
    camctrl_open(24);
}

/**
 * @brief open camera, should be done the after reset camera to  initialize 
 * @author xia_wenting  
 * @date 2010-12-06
 * @param[in] mclk camera mclk 
 * @return T_BOOL
 * @retval AK_TRUE if successed
 * @retval AK_FALSE if failed
 */
T_BOOL camctrl_open(T_U32 mclk)
{    
    T_U32 m = 0;
    T_U32 temp = 0;    
    T_U32 Pclk_Div = 0;
    T_U32 Pll2_clk = 0;
    T_U32 mod = 0xff;    
    
    //disable PLL2    
    REG32(MUL_FUN_CTL_REG) |= (1UL << 31); 
    mini_delay(1);
    REG32(MUL_FUN_CTL_REG) |= (1 << 3); //cis clock use clock168
    
    //mclk = clock168/2/(PCD+1)
    Pclk_Div = get_clk168_value()/ 2/ mclk - 1;
    if (get_clk168_value()% (2 * mclk) != 0)
        Pclk_Div += 1;
    
    //config camera PCD
    IMG_CONFIG_REG &= ~(0x07 << 16);
    IMG_CONFIG_REG |= (Pclk_Div << 16);


    REG32(MUL_FUN_CTL_REG) &= ~(1UL << 31);
    mini_delay(1);

    akprintf(C3, M_DRVSYS, "open camera success: %d, %d, %d\n", mclk, get_clk168_value(), Pclk_Div);
    return AK_TRUE;
}

/**
 * @brief close camera 
 * @author xia_wenting  
 * @date 2010-12-06
 * @return T_VOID
 */
T_VOID camctrl_disable(T_VOID)
{    
    INTR_DISABLE(IRQ_MASK_CAMERA_BIT);
    
    //disable PLL2    
    REG32(MUL_FUN_CTL_REG) |= (1UL << 31); 
    mini_delay(1);
        
    sysctl_clock(~CLOCK_CAMERA_ENABLE);
}

/**
 * @brief capture an image in YUV420 format
 * @author xia_wenting 
 * @date 2010-12-06
 * @param[out] dstY     Y buffer to save the image data 
 * @param[out] dstU     U buffer to save the image data
 * @param[out] dstV     V buffer to save the image data 
 * @param[in] srcWidth  source width, output width of camera sensor
 * @param[in] srcHeight source height, output height of camera sensor
 * @param[in] dstWidth  desination width, the actual width of image in buffer 
 * @param[in] dstHeight desination height, the actual height of image in buffer 
 * @param[in] timeout   time out value for capture
 * @param[in] bIsCCIR656 whether image sensor compatible with ccir656 protocol
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if time out
 */
T_BOOL camctrl_capture_YUV(T_U8 *dstY, T_U8 *dstU, T_U8 *dstV, T_U32 srcWidth, T_U32 srcHeight, 
                    T_U32 dstWidth, T_U32 dstHeight, T_U32 timeout, T_BOOL bIsCCIR656)
{
    T_U32 IDimgH, IDimgV;
    T_U32 i, count;
    T_U32 tmpValue;

    tmpValue = IMG_STATUS_REG;    //clear status
    
    IDimgH = 65536 / dstWidth;
    IDimgV = 65536 / dstHeight;
    IMG_HINFO1_REG = srcWidth | (dstWidth << 16);
    IMG_HINFO2_REG = IDimgH;
    IMG_VINFO1_REG = srcHeight | (dstHeight << 16);
    IMG_VINFO2_REG = IDimgV;
    
    IMG_YADDR_ODD_REG = (volatile T_U32)dstY;
    IMG_UADDR_ODD_REG = (volatile T_U32)dstU;
    IMG_VADDR_ODD_REG = (volatile T_U32)dstV;
    IMG_YADDR_EVE_REG = (volatile T_U32)dstY;
    IMG_UADDR_EVE_REG = (volatile T_U32)dstU;
    IMG_VADDR_EVE_REG = (volatile T_U32)dstV;

    //bit[12]:0, CCIR 601; 1, CCIR 656. bit[8]:vivref, 0, active low; 1, active hight
    //bit[5]:input data format:0, jpeg; 1, yuv422
    if (!bIsCCIR656)
    {
        IMG_CONFIG_REG &= ~((1 << 12) | (1 << 8));
        IMG_CONFIG_REG |= (1 << 5) | (1<<25);
    }
    else
    {
        IMG_CONFIG_REG &= ~(1 << 8);
        IMG_CONFIG_REG |= (1 << 12) | (1 << 5) | (1<<25);
    }
    
    //bit[5]:range of input yuv data for converting to RGB: 0, 16~235; 1, 0~255
    //bit[4]:0, capture;1, preview. bit[3]:0,disable vertical scaling; 1, enable
    //bit[2]:0, disable horizontal scaling; 1, enable. 
    //bit[1:0]:data format transformed by DMA: 00, rgb888 or jpeg; 01, yuv420
    if ((srcWidth == dstWidth) && (srcHeight == dstHeight))
    {
        IMG_CMD_REG = (0 << 4) | (0 << 3) | (0 << 2) | 0x01 | (0 << 5);
    }
    else if (srcWidth == dstWidth)
    {
        IMG_CMD_REG = (0 << 4) | (1 << 3) | (0 << 2) | 0x01 | (0 << 5);
    }
    else if (srcHeight == dstHeight)
    {
        IMG_CMD_REG = (0 << 4) | (0 << 3) | (1 << 2) | 0x01 | (0 << 5);
    }
    else
    {
        IMG_CMD_REG = (0 << 4) | (1 << 3) | (1 << 2) | 0x01 | (0 << 5);
    }
    
    if (timeout == 0)
        count = TIME_OUT_VALUE;
    else
        count = timeout * TIME_OUT_VALUE;
        
    i = 0;
    while(1)
    {
        tmpValue = IMG_STATUS_REG;
        if ((tmpValue & 0x01) == 0x01)          //capture end
        {
            if ((tmpValue & 0x02) == 0x02)      //capture error
            {
                akprintf(C2, M_DRVSYS, "dma error in capture\n");
                return AK_FALSE;            
            }
            else
            {
                if ((tmpValue & 0x04) == 0x04)  //odd frame
                {
                    break;
                }
                else                            //even frame
                {
                    break;
                }
            }        
        }
            
        if (++i > count)
        {
            akprintf(C2, M_DRVSYS, "time out in capture\n");
            return AK_FALSE;
        }
    }
    
    MMU_InvalidateDCache();
    return AK_TRUE;
}

/**
 * @brief capture an image in RGB format
 * @author xia_wenting 
 * @date 2010-12-06
 * @param[out] dst      buffer to save the image data
 * @param[in] srcWidth  source width, output width of camera sensor
 * @param[in] srcHeight source height, output height of camera sensor
 * @param[in] dstWidth  desination width, the actual width of image in buffer 
 * @param[in] dstHeight desination height, the actual height of image in buffer 
 * @param[in] timeout   time out value for capture
 * @param[in] bIsCCIR656 whether image sensor compatible with ccir656 protocol
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if time out
 */
T_BOOL camctrl_capture_RGB(T_U8 *dst, T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth,
                           T_U32 dstHeight, T_U32 timeout, T_BOOL bIsCCIR656)
{
    T_U32 IDimgH, IDimgV;
    T_U32 i, count;
    T_U32 tmpValue;

    tmpValue = IMG_STATUS_REG;    //clear status
    
    IDimgH = 65536 / dstWidth;
    IDimgV = 65536 / dstHeight;
    IMG_HINFO1_REG = srcWidth | (dstWidth << 16);
    IMG_HINFO2_REG = IDimgH;
    IMG_VINFO1_REG = srcHeight | (dstHeight << 16);
    IMG_VINFO2_REG = IDimgV;
    
    IMG_RGBADDR_ODD_REG = (volatile T_U32)dst;
    IMG_RGBADDR_EVE_REG = (volatile T_U32)dst;

    //bit[12]:0, CCIR 601; 1, CCIR 656. bit[8]:vivref, 0, active low; 1, active hight
    //bit[5]:input data format:0, jpeg; 1, yuv422
    if (!bIsCCIR656)
    {
        IMG_CONFIG_REG &= ~((1 << 12) | (1 << 8));
        IMG_CONFIG_REG |= (1 << 5) | (1<<25);
    }
    else
    {
        IMG_CONFIG_REG &= ~(1 << 8);
        IMG_CONFIG_REG |= (1 << 12) | (1 << 5) | (1<<25);
    }
   
    //bit[5]:range of input yuv data for converting to RGB: 0, 16~235; 1, 0~255
    //bit[4]:0, capture;1, preview. bit[3]:0,disable vertical scaling; 1, enable
    //bit[2]:0, disable horizontal scaling; 1, enable. 
    //bit[1:0]:data format transformed by DMA: 00, rgb888 or jpeg; 01, yuv420
    if ((srcWidth == dstWidth) && (srcHeight == dstHeight))
    {
        IMG_CMD_REG = (0 << 4) | (0 << 3) | (0 << 2) | 0x00 | (1 << 5);
    }
    else if (srcWidth == dstWidth)
    {
        IMG_CMD_REG = (0 << 4) | (1 << 3) | (0 << 2) | 0x00 | (1 << 5);
    }
    else if (srcHeight == dstHeight)
    {
        IMG_CMD_REG = (0 << 4) | (0 << 3) | (1 << 2) | 0x00 | (1 << 5);
    }
    else
    {
        IMG_CMD_REG = (0 << 4) | (1 << 3) | (1 << 2) | 0x00 | (1 << 5);
    }
    
    if (timeout == 0)
        count = TIME_OUT_VALUE;
    else
        count = timeout * TIME_OUT_VALUE;
        
    i = 0;
    while(1)
    {
        tmpValue = IMG_STATUS_REG;
        
        if ((tmpValue & 0x01) == 0x01)          //capture end
        {
            if ((tmpValue & 0x02) == 0x02)      //capture error
            {
                akprintf(C2, M_DRVSYS, "dma error in capture\n");
                return AK_FALSE;            
            }
            else
            {
                if ((tmpValue & 0x04) == 0x04)  //odd frame
                {    
                    break;
                }
                else                           //even frame
                {
                    break;
                }
            }        
        }
            
        if(++i > count)
        {
            akprintf(C2, M_DRVSYS, "time out in capture\n");
            return AK_FALSE;
        }
    }
    
    MMU_InvalidateDCache();
    return AK_TRUE;
}

/**
 * @brief capture an image in JPEG format
 * @author xia_wenting 
 * @date 2010-12-06
 * @param[out] dstJPEG     buffer to save the image data 
 * @param[out] JPEGlength   jpeg line length
 * @param[in] timeout time out value for capture
 * @return T_BOOL
 * @retval AK_TRUE if success
 * @retval AK_FALSE if time out
 */
T_BOOL camctrl_capture_JPEG(T_U8 *dstJPEG, T_U32 *JPEGlength, T_U32 timeout)
{
    T_U32 i, count;
    T_U32 tmpValue;

    if (dstJPEG == AK_NULL)
    {
        return AK_FALSE;
    }

    tmpValue = IMG_STATUS_REG;    //clear status
 
    IMG_RGBADDR_ODD_REG = (volatile T_U32)dstJPEG;
    IMG_RGBADDR_EVE_REG = (volatile T_U32)dstJPEG;

    //bit[12]:0, CCIR 601; 1, CCIR 656. bit[8]:vivref, 0, active low; 1, active hight
    //bit[5]:input data format:0, jpeg; 1, yuv422
    IMG_CONFIG_REG &= ~((1 << 12) | (1 << 8) | (1 << 5));
    IMG_CONFIG_REG |= (1<<25);
    
    //bit[4]:0, capture;1, preview. bit[3]:0,disable vertical scaling; 1, enable
    //bit[2]:0, disable horizontal scaling; 1, enable. 
    //bit[1:0]:data format transformed by DMA: 00, rgb888 or jpeg; 01, yuv420    
    IMG_CMD_REG = (0 << 4) | (0 << 3) | (0 << 2) | 0x00;

    if (timeout == 0)
        count = TIME_OUT_VALUE;
    else
        count = timeout * TIME_OUT_VALUE;
        
    i = 0;
    while(1)
    {
        tmpValue = IMG_STATUS_REG;
        
        if ((tmpValue & 0x01) == 0x01)          //capture end
        {
            if ((tmpValue & 0x02) == 0x02)      //capture error
            {
                akprintf(C2, M_DRVSYS, "dma error in capture\n");
                return AK_FALSE;            
            }
            else
            {
                if ((tmpValue & 0x04) == 0x04)  //odd frame
                {
                    *JPEGlength = IMG_NUM_REG;
                    break;                
                }
                else                            //even frame
                {
                    *JPEGlength = IMG_NUM_REG;
                    break;
                }
            }        
        }
            
        if (++i > count)
        {
            akprintf(C2, M_DRVSYS, "time out in capture\n");
            return AK_FALSE;
        }
    }

    MMU_InvalidateDCache();
    return AK_TRUE;
}

/*******************************The following function used in camera interrupt mode********/

static volatile T_U32 m_CameraCaptureCommand;

static T_fCamera_Interrupt_CALLBACK m_CameraInterruptCallback = AK_NULL;

static T_BOOL camctrl_interrupt_handler(T_VOID);

T_VOID camctrl_capture_frame(T_U8 *dstY, T_U8 *dstU, T_U8 *dstV)
{   
    IMG_YADDR_ODD_REG = (volatile T_U32)dstY;
    IMG_UADDR_ODD_REG = (volatile T_U32)dstU;
    IMG_VADDR_ODD_REG = (volatile T_U32)dstV;
    IMG_YADDR_EVE_REG = (volatile T_U32)dstY;
    IMG_UADDR_EVE_REG = (volatile T_U32)dstU;
    IMG_VADDR_EVE_REG = (volatile T_U32)dstV;
    
    IMG_CMD_REG = m_CameraCaptureCommand;
}

T_VOID camctrl_set_info(T_U32 srcWidth, T_U32 srcHeight, T_U32 dstWidth, 
                        T_U32 dstHeight, T_BOOL bIsCCIR656)
{
    T_U32 IDimgH, IDimgV;
    
    IDimgH = 65536 / dstWidth;
    IDimgV = 65536 / dstHeight;
    IMG_HINFO1_REG = srcWidth | (dstWidth << 16);
    IMG_HINFO2_REG = IDimgH;
    IMG_VINFO1_REG = srcHeight | (dstHeight << 16);
    IMG_VINFO2_REG = IDimgV;

    //bit[12]:0, CCIR 601; 1, CCIR 656. bit[8]:vivref, 0, active low; 1, active hight
    //bit[5]:input data format:0, jpeg; 1, yuv422
    if (!bIsCCIR656)
    {
        IMG_CONFIG_REG &= ~((1 << 12) | (1 << 8));
        IMG_CONFIG_REG |= (1 << 5) | (1<<25);
    }
    else
    {
        IMG_CONFIG_REG &= ~(1 << 8);
        IMG_CONFIG_REG |= (1 << 12) | (1 << 5) | (1<<25);
    }
    
    //bit[4]:0, capture;1, preview. bit[3]:0,disable vertical scalling; 1, enable
    //bit[2]:0, disable horizontal scalling; 1, enable. bit[1:0]:00, rgb or jpeg; 01, yuv420
    if ((srcWidth == dstWidth) && (srcHeight == dstHeight))
    {
        m_CameraCaptureCommand = (0 << 4) | (0 << 3) | (0 << 2) | 0x01;
    }
    else if (srcWidth == dstWidth)
    {
        m_CameraCaptureCommand = (0 << 4) | (1 << 3) | (0 << 2) | 0x01;
    }
    else if (srcHeight == dstHeight)
    {
        m_CameraCaptureCommand = (0 << 4) | (0 << 3) | (1 << 2) | 0x01;
    }
    else
    {
        m_CameraCaptureCommand = (0 << 4) | (1 << 3) | (1 << 2) | 0x01;
    }   
}

T_VOID camctrl_set_interrupt_callback(T_fCamera_Interrupt_CALLBACK callback_func)
{
    m_CameraInterruptCallback = callback_func;
    if (callback_func)
    {
        int_register_irq(INT_VECTOR_CAMERA, camctrl_interrupt_handler);
    }
    else
    {
        INTR_DISABLE(IRQ_MASK_CAMERA_BIT);
    }
}

/**
 * @brief read camera controller's register, and check the frame finished or occur errorred
 * @author xia_wenting   
 * @date 2010-12-06
 * @param
 * @return T_BOOL
 * @retval AK_TRUE the frame finished
 * @retval AK_FALSE the frame not finished or occur errorred
 */
T_BOOL camctrl_check_status(T_VOID)
{
    T_U32 tmpValue;

    tmpValue = IMG_STATUS_REG;

    if ((tmpValue & 0x01) == 0x01)             //capture end
    {
        if ((tmpValue & 0x02) == 0x02)         //capture error
        {
            return AK_FALSE;            
        }
        else
        {
            if ((tmpValue & 0x04) == 0x04)     //odd frame
            {
                //akprintf(C3, M_DRVSYS, "odd frame\n");
                return AK_TRUE;                
            }
            else                               //even frame
            {
                //akprintf(C3, M_DRVSYS, "even frame\n");
                return AK_TRUE;
            }
        }        
    }
    else                                       //frame has not been finished
    {
        return AK_FALSE;
    }
}

static T_BOOL camctrl_interrupt_handler(T_VOID)
{    
    if (m_CameraInterruptCallback != AK_NULL)
    {
        m_CameraInterruptCallback();
        //akprintf(C3, M_DRVSYS, "camctrl_interrupt_handler!\n");
    }
    else
    {
        camctrl_check_status();
    }

    return AK_TRUE;
}

