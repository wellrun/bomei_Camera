/**
 * @file lcd.c
 * @brief LCD driver file,
 * This file provides all the APIs of LCD dirver, such as initial LCD, turn on/off LCD and refresh LCD
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Liangenhui
 * @date 2010-06-18
 * @version 1.0
 */

/*********************************************************
    The following is an example to use lcd driver APIs
    
    lcd_initial();    
    lcd_turn_on(LCD_0);
    lcd_set_brightness(LCD_0, 4);
    lcd_rotate(LCD_0, LCD_90_DEGREE);
    lcd_refresh_RGB(LCD_0, &lcd_disp_rect, RGB_bmp);
    lcd_refresh_RGB_ex (LCD_0, &lcd_dsp_rect, &lcd_vir_rect, RGB_bmp);
    lcd_refresh_YUV1(LCD_0, srcY, srcU, srcV, WIDTH, HEIGHT, &lcd_dsp_rect);
    lcd_refresh_YUV1_ex (LCD_0, srcY, srcU, srcV, YUV_WIDTH, YUV_HEIGHT,&lcd_vir_rect,&lcd_dsp_rect);
    lcd_refresh_dual_YUV (LCD_0, srcY1, srcU1, srcV1, YUV1_WIDTH, YUV1_HEIGHT, &lcd_dsp_rect1,
                    srcY2, srcU2, srcV2, YUV2_WIDTH, YUV2_HEIGHT, &lcd_dsp_rect2);
    lcd_refresh_dual_YUV_ex (LCD_0, srcY1, srcU1, srcV1, YUV1_WIDTH, YUV1_HEIGHT, &lcd_vir_rect1, &lcd_dsp_rect1,
                    srcY2, srcU2, srcV2, YUV2_WIDTH, YUV2_HEIGHT, &lcd_dsp_rect2);
    lcd_YUV_on();
    lcd_YUV_off();
    lcd_osd_set_color_palette(&color);
    lcd_osd_display_on (LCD_0,    &osd_dsp, 8, *dsp_buf);
    lcd_tv_out_open(TV_OUT_TYPE_PAL);
    lcd_tv_out_close(); 
*********************************************************/
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"
#include "hal_probe.h"
#include "sysctl.h"
#include "interrupt.h"
#include "lcd.h"
#include "drv_module.h"
#include "drv_gpio.h"

//yuv2 alpha variable
#define MAX_ALPHA_LEVEL 8

#define FREQ_MIN_TVOUT 60000000

typedef struct 
{
    E_LCD_TYPE LcdType;
    E_TV_OUT_TYPE TvoutType;
    
    T_eLCD_DEGREE CurrentLcdDegree[LCD_MAX_NUMBER];
    T_U8   CurBrightness[LCD_MAX_NUMBER];

    T_BOOL bLcdInit[LCD_MAX_NUMBER];
    T_BOOL bLcd_OnOff[LCD_MAX_NUMBER];
    T_BOOL bDMATransfering;
    T_BOOL bYuv1FullFlag;
    T_BOOL bRgbFlag;           //rgb on flag
    T_BOOL bRgb565;            //rgb565 flag
    T_BOOL bRgbSeq;            //rgb or bgr flag

    T_U32  Yuv2AlphaLvl;
    T_U32  CurBgColor;
    T_U32  YuvOffStatus;         //bit[0]:yuv off flag,bit[1]:YUV2,bit[2]:YUV2

    T_U32  Yuv1OffsetX;
    T_U32  Yuv1OffsetY;
    T_U32  Yuv2OffsetX;
    T_U32  Yuv2OffsetY;



    T_LCD_FUNCTION_HANDLER *pLcdHandler[LCD_MAX_NUMBER]; 
    T_RGBLCD_INFO *pRgblcdPointer;
    T_U8    *pCurrentFrame;
    T_U8    *pNextFrame;

    T_hFreq freq_handle;
}T_LCD_PARAM;

static volatile T_LCD_PARAM m_lcd_param = {0};

/**
 * @brief: LCD channel type define.
 * define the channel type of LCD
 */
typedef enum
{
    EM_CHANNEL_TYPE_RGB = 0,
    EM_CHANNEL_TYPE_YUV1,
    EM_CHANNEL_TYPE_YUV2,
    EM_CHANNEL_TYPE_OSD
}CHANNEL_TYPE;


extern T_U8   lcdbl_set_brightness(T_eLCD lcd, T_U8 brightness);

static T_VOID lcd_rgb_HardInit(const T_RGBLCD_INFO *pRGBlcd);
static T_VOID lcd_rgb_turn_on(T_VOID);
static T_VOID lcd_rgb_turn_off(T_VOID);

static T_VOID lcd_tv_out_Init(E_TV_OUT_TYPE type);
static T_VOID lcd_tv_out_turn_on(T_VOID);
static T_VOID lcd_tv_out_turn_off(T_VOID);
static T_BOOL lcd_intr_handler(T_VOID);
static T_VOID lcd_set_mode();
static T_VOID tv_out_reg_config(E_TV_OUT_TYPE type);
static T_VOID lcd_set_YUV_scaler(T_U16 src_width, T_U16 src_height, T_RECT *dsp_rect,
                                    CHANNEL_TYPE channel, T_BOOL isMPUPannel, T_BOOL bJump, T_eLCD lcd);

/**
 * @brief  lcd handler for interrupt 
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_BOOL
 */
static T_BOOL lcd_intr_handler(T_VOID)
{
    T_U32 reg_value;

    reg_value = REG32(LCD_LCD_STATUS);
    if((reg_value & ALERT_VALID_STAT) == ALERT_VALID_STAT)
    {
        T_U32 t1, t2;

        //disable alert valid interrupt
        REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 17);
        //enable next frame setting
        REG32(LCD_SOFTWARE_CTL_REG) |= (1 << 12);

        //should wait till frame end!!
        t1 = get_tick_count_us();
        while(REG32(LCD_SOFTWARE_CTL_REG)&(1 << 12))
        {
            t2 = get_tick_count_us();
            if (t2 > t1)
            {
                if (t2-t1 > 400) //400us time out
                {
                    akprintf(C3, M_DRVSYS, "check frame timeout!\n");
                    break;
                }
            }
            else
                t1 = t2;
        }
        m_lcd_param.pCurrentFrame = m_lcd_param.pNextFrame;
    }

    if((reg_value & RGB_SYS_ERROR_STAT) == RGB_SYS_ERROR_STAT)
    {
        //disable err valid interrupt
        REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 0);

        akprintf(C2, M_DRVSYS, "refresh error, restart!\n");

        //rgb stop
        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)
            REG32(LCD_LCD_GO_REG) &= ~(LCD_GO_TV);
        else if (E_LCD_TYPE_RGB== m_lcd_param.LcdType)
            REG32(LCD_LCD_GO_REG) &= ~(LCD_GO_RGB);
        else
            akprintf(C3, M_DRVSYS, "unknown lcd type!\n");
        REG32(LCD_LCD_GO_REG) |= (LCD_GO_SYS_STOP);

        //rgb go
        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)
        {
            //tv out reg config
            tv_out_reg_config(m_lcd_param.TvoutType);
            REG32(LCD_LCD_GO_REG) |= (LCD_GO_TV);
        }
        else if (E_LCD_TYPE_RGB== m_lcd_param.LcdType)
            REG32(LCD_LCD_GO_REG) |= (LCD_GO_RGB);
        else
            akprintf(C3, M_DRVSYS, "unknown lcd type!\n");
    }
    
    if((reg_value & FIFO_ALARM_STAT) == FIFO_ALARM_STAT)
    {
        akprintf(C2, M_DRVSYS, "reflesh fifo alarm\n");
        
        //disable empty alarm interrupt
        REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 18);
        //sys stop
        REG32(LCD_LCD_GO_REG) |= (LCD_GO_SYS_STOP);
        
        //rgb go
        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)
        {
            //tv out reg config
            tv_out_reg_config(m_lcd_param.TvoutType);
            REG32(LCD_LCD_GO_REG) |= (LCD_GO_TV);
        }
        else if (E_LCD_TYPE_RGB== m_lcd_param.LcdType)
            REG32(LCD_LCD_GO_REG) |= (LCD_GO_RGB);
        else
            akprintf(C3, M_DRVSYS, "unknown lcd type!\n");
      
    }
    return AK_TRUE;
}

/**
 * @brief  restore lcd channel when lcd initial or lcd to tvout
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_BOOL
 */
static T_VOID restore_channel_setting(T_U32 lcd_channel)
{
    REG32(LCD_TOP_CONFIGURE_REG) = (REG32(LCD_TOP_CONFIGURE_REG) & 0xfffffff0) | (lcd_channel & 0xf);    
}

/**
 * @brief  lcd parameter init
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_BOOL
 */
static T_VOID m_lcd_parameter_init(T_VOID)
{
    T_eLCD lcd;

    m_lcd_param.LcdType = E_LCD_TYPE_MPU;

    for (lcd = LCD_0; lcd < LCD_MAX_NUMBER; lcd++)
    {
        m_lcd_param.pLcdHandler[lcd] = AK_NULL;
        m_lcd_param.bLcdInit[lcd] = AK_FALSE;
        m_lcd_param.bLcd_OnOff[lcd] = AK_FALSE;
        m_lcd_param.CurrentLcdDegree[lcd] = LCD_0_DEGREE;
        m_lcd_param.CurBrightness[lcd] = 0;
    }
    m_lcd_param.bDMATransfering = AK_FALSE;

    m_lcd_param.pRgblcdPointer = AK_NULL;
    m_lcd_param.Yuv2AlphaLvl = MAX_ALPHA_LEVEL;
    m_lcd_param.bRgbFlag = AK_FALSE;
    m_lcd_param.CurBgColor = 0x00;
    m_lcd_param.bRgb565 = AK_FALSE;
    m_lcd_param.bRgbSeq = AK_FALSE;

    m_lcd_param.TvoutType = TV_OUT_TYPE_PAL;

    m_lcd_param.pCurrentFrame = AK_NULL;
    m_lcd_param.pNextFrame = AK_NULL;
    m_lcd_param.bYuv1FullFlag = AK_FALSE;
    m_lcd_param.YuvOffStatus = 0;

    m_lcd_param.freq_handle = FREQ_INVALID_HANDLE;
}

/**
 * @brief set the scaler infomation
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_BOOL
 */
static T_VOID lcd_set_YUV_scaler(T_U16 src_width, T_U16 src_height, T_RECT *dsp_rect,
                                    CHANNEL_TYPE channel, T_BOOL isMPUPannel, T_BOOL bJump, T_eLCD lcd)
{
    T_U32 IDimgH = 0;
    T_U32 IDimgV = 0;
    T_U32 scaleMode = 0;
    T_U32 Alpha = 0;
    T_U32 sNewHeight =  src_height;
    T_U8 hJump = 0;

    if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)//tv out the height must be is 1/2 height
    {
        dsp_rect->top = dsp_rect->top >> 1;
        dsp_rect->height = dsp_rect->height >> 1;
    }

    if(bJump && (src_height >= 36))
    {
        hJump = (src_height - 1) / dsp_rect->height;
        if(hJump > 3) hJump = 3;
    }

    sNewHeight = src_height / (hJump + 1);

    if (src_width * dsp_rect->height == sNewHeight * dsp_rect->width)
        IDimgH = 65536/dsp_rect->width;
    else
        IDimgH = 65536/dsp_rect->width + 1;

    if (IDimgH==0)
        IDimgH = 0xfff;

    IDimgV = 65536/(2 * (dsp_rect->height - 1));
    if (IDimgV==0)
        IDimgV = 0xfff;

    // enable YUV, enable YUV1 scaler and YUV2 scaler
    // only open scaler when need, to improve the quality of picture
    if (src_width==dsp_rect->width && sNewHeight==dsp_rect->height)
        scaleMode = 0;
    else if (src_width == dsp_rect->width)
        scaleMode = YUV1_V_SCALER_ENABLE;
    else if (sNewHeight == dsp_rect->height)
        scaleMode = YUV1_H_SCALER_ENABLE;
    else
        scaleMode = YUV1_H_SCALER_ENABLE | YUV1_V_SCALER_ENABLE;

    scaleMode |= (1<<25);//whenever up scale or down scale, should set bit23 to 1. it consumes less calculation.

    if (channel == EM_CHANNEL_TYPE_YUV1)
    {
        //record yuv1 param
        REG32(LCD_YUV1_H_INFO_REG) = (hJump << 22) | (dsp_rect->width << 11) | src_width;
        REG32(LCD_YUV1_V_INFO_REG) = (dsp_rect->height << 11) | sNewHeight;
        REG32(LCD_YUV1_SCALER_INFO_REG) = (IDimgV << 12) | IDimgH; 

        //pannel fix picture or not
        if(isMPUPannel)
        {
            REG32(LCD_YUV1_DISPLAY_INFO_REG) &= 0xfc000000;
            REG32(LCD_YUV1_DISPLAY_INFO_REG) |= (scaleMode);
            REG32(LCD_PANEL_SIZE_REG) = ((dsp_rect->width << 11) | dsp_rect->height);
        }
        else
        {
            REG32(LCD_YUV1_DISPLAY_INFO_REG) &= 0xfc000000;
            REG32(LCD_YUV1_DISPLAY_INFO_REG) |= (scaleMode)|(dsp_rect->left << 11)|dsp_rect->top;
            if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)//tv out the height must be is 1/2 height
            {
                REG32(LCD_PANEL_SIZE_REG) =  ((lcd_get_hardware_width(lcd) << 11) | (lcd_get_hardware_height(lcd) >> 1));
            }
            else
            {
                REG32(LCD_PANEL_SIZE_REG) =  ((lcd_get_hardware_width(lcd) << 11) | lcd_get_hardware_height(lcd));
            }
        }
    }
    else if(channel == EM_CHANNEL_TYPE_YUV2)
    {
        REG32(LCD_YUV2_H_INFO_REG) = (hJump << 22) | (dsp_rect->width << 11) | src_width;
        REG32(LCD_YUV2_SCALER_INFO_REG) = (IDimgV << 12) | IDimgH;
        REG32(LCD_YUV2_V_INFO_REG) = (dsp_rect->height << 11) | sNewHeight;

        if (m_lcd_param.Yuv2AlphaLvl >= MAX_ALPHA_LEVEL)
            Alpha=0xf;
        else
            Alpha=m_lcd_param.Yuv2AlphaLvl;

        REG32(LCD_YUV2_DISPLAY_INFO_REG) = (Alpha<<26)|(scaleMode)|(dsp_rect->left << 11)|dsp_rect->top;
    }
}

/**
 * @brief check dma finish after refresh one frame, only for MPU lcd
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_BOOL
 */
static T_BOOL check_dma_finish(T_VOID)
{
    volatile T_U32 status;
    T_U32 t1, t2;

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        return AK_TRUE;
    }
    else
    {
        t1 = get_tick_count();
        //3750C 8bit LCD maybe occurred sys_error when quite camera 
        while ((MPU_DISPLAY_OK_STAT != (REG32(LCD_LCD_STATUS)&(MPU_DISPLAY_OK_STAT)))
               || (RGB_SYS_ERROR_STAT != (REG32(LCD_LCD_STATUS)&(RGB_SYS_ERROR_STAT))) )
        {
            t2 = get_tick_count();
            if (t2 > t1)
            {
                if (t2-t1 > 400) //10ms time out
                {
                    akprintf(C3, M_DRVSYS, "check MPU frame timeout!\n");
                    return AK_FALSE;
                }
            }
            else
                t1 = t2;
        }

        return AK_TRUE;
     }

}

/**
 * @brief enable alert valid interrupt and enable sw_en in its interrupt,
 *    to inform next frame is ready to change
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_BOOL
 */
static T_VOID enable_alert_valid_int()
{
    T_U32 status;

    //NOTE:the interrupt mode is only for RGB interface!!!
    if(E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        //read to clear prev frame's status
        status = REG32(LCD_LCD_STATUS);

        //enable alert valid interrupt
        REG32(LCD_LCD_INTERRUPT_MASK) |= (1 << 17);
        //enable system error interrupt
        REG32(LCD_LCD_INTERRUPT_MASK) |= (1 << 0);
        //enable empty alarm interrupt
        REG32(LCD_LCD_INTERRUPT_MASK) |= (1 << 18);
        
    }
}

/**
 * @brief set lcd clock of MPU interface
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_BOOL
 */
static T_VOID lcd_open_clk(T_VOID)
{
    T_U32 status;

    //--select MPU interface--
    lcd_set_mode();

    //panel size and mpu go must be set in initial.
    REG32(LCD_PANEL_SIZE_REG) = ((m_lcd_param.pLcdHandler[LCD_0]->lcd_width << 11) 
                               | (m_lcd_param.pLcdHandler[LCD_0]->lcd_height));

    //this "go" is meaned open the clk , the hardware need to did.
    REG32(LCD_LCD_GO_REG) |= (MPU_REFLASH_START);

    //wait clock valid
    while(1)
    {
        status = REG32(LCD_LCD_STATUS);
        if((status & MPU_DISPLAY_OK_STAT) == MPU_DISPLAY_OK_STAT)
        {
            akprintf(C3, M_DRVSYS, "open lcd clock ok!\n");
            break;
        }
    }
}

/**
 * @brief Initialize the LCD
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_BOOL
 */
T_BOOL lcd_initial(T_VOID)
{
    T_eLCD lcd=LCD_0;
    T_BOOL lcd_init_flags=AK_FALSE;//keep lcd inital flag

    DrvModule_Protect(DRV_MODULE_LCD);

    //if lcd interface mode selection is 0,lcd hasn't initial before
    if(0x00 == (REG32(LCD_TOP_CONFIGURE_REG)&(3 << 5)))
    {
        lcd_init_flags = AK_FALSE;//for bios, first initial
    }
    else
    {
        lcd_init_flags = AK_TRUE;//for MMI, inital before
    }

     //open lcd controller and set share pin
    sysctl_clock(CLOCK_LCD_ENABLE);
    gpio_pin_group_cfg(ePIN_AS_LCD_MPU);

    m_lcd_parameter_init();

    m_lcd_param.pLcdHandler[lcd] = lcd_probe(lcd);

    if (E_LCD_TYPE_MPU == m_lcd_param.pLcdHandler[lcd]->lcd_type)
    {
        akprintf(C3, M_DRVSYS, "it's MPU pannel.\n");

        if(1)//for bios
        {
            //reset lcd interface
            sysctl_reset(RESET_LCD);
            lcd_open_clk();    //open lcd clk
        }
        m_lcd_param.LcdType = E_LCD_TYPE_MPU;

        if (m_lcd_param.pLcdHandler[lcd] != AK_NULL)
        {
            //if(!lcd_init_flags)//for bios  TV OUT -> LCD  init lcd again
            if(1)
            {
                m_lcd_param.pLcdHandler[lcd]->lcd_init_func(lcd);
            }
            m_lcd_param.bLcdInit[lcd] = AK_TRUE;
            m_lcd_param.bLcd_OnOff[lcd] = AK_FALSE;
        }
    }
    else if (E_LCD_TYPE_RGB == m_lcd_param.pLcdHandler[lcd]->lcd_type)
    {
        akprintf(C3, M_DRVSYS, "it's RGB pannel.\n");
        m_lcd_param.LcdType = E_LCD_TYPE_RGB;
        m_lcd_param.pRgblcdPointer = (T_RGBLCD_INFO *)m_lcd_param.pLcdHandler[LCD_0]->lcd_reginfo;
        m_lcd_param.bLcdInit[LCD_0] = AK_TRUE;
        m_lcd_param.bLcd_OnOff[LCD_0] = AK_FALSE; 
        lcd_rgb_HardInit(m_lcd_param.pRgblcdPointer);
    }
    else
    {
        akprintf(C1, M_DRVSYS, "lcd initial error!\n");
        DrvModule_UnProtect(DRV_MODULE_LCD);
        return AK_FALSE;
    }

    DrvModule_UnProtect(DRV_MODULE_LCD);
    return AK_TRUE;
}

/**
 * @brief Turn on the LCD
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @return T_VOID
 */
T_VOID lcd_turn_on(T_eLCD lcd)
{
    if (AK_FALSE == m_lcd_param.bLcdInit[lcd])
    {
        akprintf(C1, M_DRVSYS, "turn on error, lcd is not initial\n");
        return;
    }

    if (AK_TRUE == m_lcd_param.bLcd_OnOff[lcd])
    {
        return;
    }

    DrvModule_Protect(DRV_MODULE_LCD);

    if (E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        if (m_lcd_param.pLcdHandler[lcd]->lcd_turn_on_func != AK_NULL)
        {
            m_lcd_param.pLcdHandler[lcd]->lcd_turn_on_func(lcd);
        }
    }
    else if (E_LCD_TYPE_RGB == m_lcd_param.LcdType)
    {
        lcd_rgb_turn_on();
        if (m_lcd_param.pLcdHandler[lcd]->lcd_turn_on_func != AK_NULL)
        {
            m_lcd_param.pLcdHandler[lcd]->lcd_turn_on_func(lcd);
        }
    }
    else if (E_LCD_TYPE_TVOUT== m_lcd_param.LcdType)
    {
        lcd_tv_out_turn_on();
    }

    m_lcd_param.bLcd_OnOff[lcd] = AK_TRUE;

    DrvModule_UnProtect(DRV_MODULE_LCD);
}

/**
 * @brief Turn off the LCD
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @return T_VOID
*/
T_VOID lcd_turn_off(T_eLCD lcd)
{
    if (AK_FALSE == m_lcd_param.bLcdInit[lcd])
    {
        akprintf(C1, M_DRVSYS, "turn off error, lcd is not initial\n"); 
        return;
    }

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        return;
    }

    DrvModule_Protect(DRV_MODULE_LCD);  

    if (AK_TRUE == m_lcd_param.bDMATransfering)
    {
        check_dma_finish();
        m_lcd_param.bDMATransfering = AK_FALSE;
    }

    m_lcd_param.bLcd_OnOff[lcd] = AK_FALSE;

    if (E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        if (m_lcd_param.pLcdHandler[lcd]->lcd_turn_off_func != AK_NULL)
        {
            m_lcd_param.pLcdHandler[lcd]->lcd_turn_off_func(lcd);   
        }
    }
    else if (E_LCD_TYPE_RGB == m_lcd_param.LcdType)
    {
        if (m_lcd_param.pLcdHandler[lcd]->lcd_turn_off_func != AK_NULL)
        {
            m_lcd_param.pLcdHandler[lcd]->lcd_turn_off_func(lcd);
        }

        lcd_rgb_turn_off();
    }
    else if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)
    {
        lcd_tv_out_turn_off();
    }
    DrvModule_UnProtect(DRV_MODULE_LCD);
}
/*
    
*/
T_BOOL lcd_refresh_output(T_eLCD lcd)
{
    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh error, lcd is not turnon\n");
        return AK_FALSE;
    }
 	 
    if (E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {       
        if (m_lcd_param.bDMATransfering == AK_TRUE)
        {
            check_dma_finish(); 
        }
        REG32(LCD_LCD_GO_REG) |= (MPU_REFLASH_START);
        m_lcd_param.bDMATransfering = AK_TRUE;
    }
    else    //非MPU屏暂时这样配置，还没测试同时刷新YUV通道及RGB通道的情况，
    {
        enable_alert_valid_int();//inform next frame is ready
        INTR_ENABLE(IRQ_MASK_LCD_BIT);
    }
    return AK_TRUE;
}

/**
 * @brief Set brightness value
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] brightness brightness value,use 0 to 7
 * @return T_U8
 * @retval new brightness value after setting
 */
T_U8 lcd_set_brightness(T_eLCD lcd, T_U8 brightness)
{
    /*  may be the blacklight is on first !*/
    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd] && brightness)
    {
        akprintf(C1, M_DRVSYS, "brightness error, lcd is not turnon\n");
        return m_lcd_param.CurBrightness[lcd];
    }

    //TVOUT brightness set, AK98 don't support
    if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)
    {
        return m_lcd_param.CurBrightness[lcd];
    }

    if (m_lcd_param.CurBrightness[lcd] == brightness)
    {
        return m_lcd_param.CurBrightness[lcd];
    }

    DrvModule_Protect(DRV_MODULE_LCD);
    m_lcd_param.CurBrightness[lcd] = lcdbl_set_brightness(lcd,brightness);
    DrvModule_UnProtect(DRV_MODULE_LCD);

    return m_lcd_param.CurBrightness[lcd];
}

/**
 * @brief Get brightness value
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @return T_U8 
 * @retval current brightness value
 */
T_U8 lcd_get_brightness(T_eLCD lcd)
{
    return m_lcd_param.CurBrightness[lcd];
}

/**
 * @brief      rotate the lcd
 * @author    LianGenhui
 * @date    2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] degree rotate degree
 * @return  T_VOID
 */
T_VOID lcd_rotate(T_eLCD lcd, T_eLCD_DEGREE degree)
{
    if(degree >= LCD_MAX_DEGREE)
    {
         akprintf(C1, M_DRVSYS, "rotate error, please refer to T_eLCD_DEGREE(0~3)\n");    
         return;
    }

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "rotate error, lcd is not turnon\n"); 
        return;
    }


    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        akprintf(C2, M_DRVSYS, "driver not support rotate for RGB and TVOUT now !!!!!!!!!!\n");         
        return;
    }

    DrvModule_Protect(DRV_MODULE_LCD);

    if (AK_TRUE == m_lcd_param.bDMATransfering)
    {
        check_dma_finish();
        m_lcd_param.bDMATransfering = AK_FALSE;
    }

    if (m_lcd_param.pLcdHandler[lcd]->lcd_rotate_func)
    {
        if (m_lcd_param.pLcdHandler[lcd]->lcd_rotate_func(lcd, degree) == AK_TRUE)    
        {
            m_lcd_param.CurrentLcdDegree[lcd] = degree;
            akprintf(C3, M_DRVSYS, "current lcd degree = %d\r\n", m_lcd_param.CurrentLcdDegree[lcd]);
        }
        else
        {
            akprintf(C1, M_DRVSYS, "rotate error\r\n");
        }
    }
    DrvModule_UnProtect(DRV_MODULE_LCD);
}

/**
 * @brief      get current lcd degree
 * @author    LianGenhui
 * @date    2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @return  T_eLCD_DEGREE
 * @retval  degree of LCD
 */
T_eLCD_DEGREE lcd_degree(T_eLCD lcd)
{
    return m_lcd_param.CurrentLcdDegree[lcd];
}

/**
 * @brief  	get current lcd type
 * @author	WangGuotian
 * @date	2011-11-25
 * @param[in] T_VOID
 * @return  E_LCD_TYPE
 * @retval  type of LCD
 */
E_LCD_TYPE lcd_get_type(T_VOID)
{
    return m_lcd_param.LcdType;
}


/**
 * @brief  get current lcd hardware width
 * lcd hardware width is determined by the lcd factory, and won't be changed
 * @author    LianGenhui
 * @date    2010-06-18
 * @param[in]     lcd select the lcd, LCD_0 or LCD_1
 * @return  T_U32
 * @retval  lcd hardware width
 */

T_U32 lcd_get_hardware_width(T_eLCD lcd)
{
    T_U32 width = 0;

    if (AK_FALSE == m_lcd_param.bLcdInit[lcd])
    {
        akprintf(C1, M_DRVSYS, "lcd_get_hardware_width error, lcd is not init\n"); 
        return width;
    }

    if (E_LCD_TYPE_RGB == m_lcd_param.LcdType)
    {
        width  = m_lcd_param.pLcdHandler[lcd]->lcd_width;
    }
    else if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)
    {
        width  = TV_WIDTH;
    }
    else if(E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        width  = m_lcd_param.pLcdHandler[lcd]->lcd_width;
    }

    return width;
}

/**
 * @brief  get current lcd's hardware height
 * lcd hardware height is determined by the lcd factory, and won't be changed
 * @author    LianGenhui
 * @date    2010-06-18
 * @param[in]     lcd select the lcd, LCD_0 or LCD_1
 * @return  T_U32
 * @retval  lcd hardware height
 */
T_U32 lcd_get_hardware_height(T_eLCD lcd)
{
    T_U32 height = 0;

    if (AK_FALSE == m_lcd_param.bLcdInit[lcd])
    {
        akprintf(C1, M_DRVSYS, "lcd_get_hardware_height error, lcd is not init\n"); 
        return height;
    }

    if (E_LCD_TYPE_RGB == m_lcd_param.LcdType)
    {
        height  = m_lcd_param.pRgblcdPointer->Tvd;
    }
    else if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)
    {
        if (TV_OUT_TYPE_PAL == m_lcd_param.TvoutType)
        {
            height  = TV_PAL_HEIGHT;
        }
        else
        {
            height = TV_NTSC_HEIGHT;
        }
    }
    else if(E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        height = m_lcd_param.pLcdHandler[lcd]->lcd_height;
    }

    return height;
}

/**
 * @brief   check frame finish,for muti-buffer refresh with RGB interface
 * @author  LianGenhui
 * @date    2010-06-18
 * @param[in]  yuv buffer address
 * @return     T_BOOL
 */
T_BOOL lcd_check_frame_finish(T_U8 *pbuf)
{
    if (pbuf == AK_NULL)
        return AK_TRUE;

    if (m_lcd_param.pCurrentFrame == pbuf)
    {
        return AK_FALSE;
    }
    else
        return AK_TRUE;
}

/**
 * @brief set lcd background color, default is black color.
 * @param BackgroundColor, the value of background color.
 * @return T_VOID
 */
T_VOID lcd_set_BgColor(T_U32 BackgroundColor)
{
    REG32(LCD_GRB_BACKGROUND_REG) = BackgroundColor;
    m_lcd_param.CurBgColor = BackgroundColor;

    REG32(LCD_SOFTWARE_CTL_REG) |= (1<<12);
}

/**
 * @brief Refresh RGB picture to  LCD
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] dsp_rect display rectangle,source picture should lower than 1024*768
 * @param[in] dsp_buf RGB buffer address
 * @return T_BOOL 
 * @retval  AK_TRUE  refresh rgb channel successful
 * @retval  AK_FALSE refresh rgb channel failed.
 * @note  return failed:\n
 *     display size bigger than 1024*768\n
 *     display size smaller than 18*18
 */
T_BOOL lcd_refresh_RGB(T_eLCD lcd, T_RECT *dsp_rect, T_U8 *dsp_buf)
{
    T_U32 status;
    T_U32 i = 0;
    T_U32 lcd_hd_w;
    T_U32 lcd_hd_h;
    T_U32 degree;

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh RGB error, lcd is not turnon\n");
        return AK_FALSE;
    }

    if (dsp_rect->width > 1024 || dsp_rect->height > 768)
    {
        akprintf(C1, M_DRVSYS, "display size bigger than 1024*768\n");
        return AK_FALSE;
    }

    if (dsp_rect->width < 18 || dsp_rect->height < 18 )
    {
        akprintf(C1, M_DRVSYS, "display size smaller than 18*18\n");
        return AK_FALSE;
    }

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        lcd_hd_w = lcd_get_hardware_width(lcd);
        lcd_hd_h = lcd_get_hardware_height(lcd);
    }
    else
    {
        degree = lcd_degree(lcd);
        if ((LCD_0_DEGREE == degree) || (LCD_180_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
        else if ((LCD_90_DEGREE == degree) || (LCD_270_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_height(lcd);
            lcd_hd_h = lcd_get_hardware_width(lcd);
        }
        else
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
    }

    if(((dsp_rect->left + dsp_rect->width) > lcd_hd_w) || ((dsp_rect->top + dsp_rect->height) > lcd_hd_h))
    {
        akprintf(C1, M_DRVSYS, "image out of LCD, error!\n");
        return AK_FALSE;
    }

    //yuv on and yuv1 is full screen
    if((AK_TRUE == m_lcd_param.bYuv1FullFlag) && ((1 << 0) != (m_lcd_param.YuvOffStatus &(1 << 0))))
    {
        akprintf(C2, M_DRVSYS, "yuv1 channel is full screen, not refresh RGB channel\n"); 
        return AK_FALSE;
    }

    //if yuv channel is open and yuv1 not in RGB ,return false 
    if (YUV1_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV1_CHANNEL_EN))
    {
        T_U32 yuv1_left = (REG32(LCD_YUV1_DISPLAY_INFO_REG) >> 11) & 0x7ff;
        T_U32 yuv1_top = REG32(LCD_YUV1_DISPLAY_INFO_REG) & 0x7ff;
        T_U32 yuv1_width = (REG32(LCD_YUV1_H_INFO_REG)>> 11) & 0x7ff  ;
        T_U32 yuv1_height = (REG32(LCD_YUV1_V_INFO_REG) >> 11) & 0x7ff ;

        if (!((dsp_rect->left <= yuv1_left) && (dsp_rect->top <= yuv1_top) 
            && ((dsp_rect->left + dsp_rect->width) >= (yuv1_left + yuv1_width)) 
            && ((dsp_rect->top + dsp_rect->height) >= (yuv1_top + yuv1_height))))
        {
            akprintf(C2, M_DRVSYS, "rgb not include yuv1\n");
            return AK_FALSE;
        }
    }

    DrvModule_Protect(DRV_MODULE_LCD);

    if (AK_TRUE == m_lcd_param.bDMATransfering && E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        check_dma_finish();    //wait for previous frame dma OK
        m_lcd_param.bDMATransfering = AK_FALSE;
    }

    MMU_InvalidateDCache();

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)//rgb or tvout
    {
        //mask lcd interrupt, lcd reg config can't be disturbed
        INTR_DISABLE(IRQ_MASK_LCD_BIT);

        REG32(LCD_RGB_CTL_REG2) = (T_U32)dsp_buf & 0x1fffffff;
        //REG32(LCD_RGB_CTL_REG2) = (((T_U32)dsp_buf & 0x1fffffff) | (1 << 29));

        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)//tv out the height must be is 1/2 height
        {            
            REG32(LCD_RGB_CTL_REG2) |= (RGB_VIR_EN);
            REG32(LCD_RGB_SIZE_REG) = ((dsp_rect->width << 11) | (dsp_rect->height >> 1));
            REG32(LCD_RGB_OFFSET_REG) = ((dsp_rect->left << 11) | (dsp_rect->top >> 1));

            REG32(LCD_RGB_VIRTUAL_SIZE_REG) = ((dsp_rect->width<<1) << 16 | (dsp_rect->height>>1));
            REG32(LCD_RGB_VIRTUAL_OFFSET_REG) = (dsp_rect->left << 16 | dsp_rect->top);
        }
        else
        {
            REG32(LCD_RGB_CTL_REG2) &= ~(RGB_VIR_EN);
            REG32(LCD_RGB_SIZE_REG) = (dsp_rect->width << 11 | dsp_rect->height);
            REG32(LCD_RGB_OFFSET_REG) = (dsp_rect->left << 11 | dsp_rect->top);
        }

        REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//enable rgb channel

        enable_alert_valid_int();//inform next frame is ready
        INTR_ENABLE(IRQ_MASK_LCD_BIT);
    }
    else//mpu
    {
        //Keep last yuv offset value
        if(YUV1_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV1_CHANNEL_EN))
           REG32(LCD_YUV1_DISPLAY_INFO_REG) |= (m_lcd_param.Yuv1OffsetX<< 11) | m_lcd_param.Yuv1OffsetY;
        if(YUV2_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV2_CHANNEL_EN))
           REG32(LCD_YUV2_DISPLAY_INFO_REG) |= (m_lcd_param.Yuv2OffsetX << 11) | m_lcd_param.Yuv2OffsetY;

        REG32(LCD_RGB_CTL_REG2) = ((T_U32)dsp_buf & 0x1fffffff);
        REG32(LCD_RGB_OFFSET_REG) = ((0 << 11) | 0);
        REG32(LCD_RGB_SIZE_REG) = ((dsp_rect->width << 11) | dsp_rect->height);
        REG32(LCD_PANEL_SIZE_REG) = ((dsp_rect->width << 11) | dsp_rect->height);
        
        REG32(LCD_TOP_CONFIGURE_REG) |= RGB_CHANNEL_EN;

        //set display start address
        m_lcd_param.pLcdHandler[lcd]->lcd_set_disp_address_func(lcd, dsp_rect->left, dsp_rect->top, 
                                       dsp_rect->left + dsp_rect->width - 1, dsp_rect->top + dsp_rect->height - 1);
        //start dma
        if (m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func)
            m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func(lcd);

        REG32(LCD_LCD_GO_REG) |= MPU_REFLASH_START;
        
        check_dma_finish();    //wait for previous frame dma OK
        m_lcd_param.bDMATransfering = AK_FALSE;

    }

    m_lcd_param.bRgbFlag = AK_TRUE;//RGB channel is opened

    DrvModule_UnProtect(DRV_MODULE_LCD);
    return AK_TRUE;
}

/**
 * @brief Refresh RGB channel data but not display picture on  LCD
 * @author PanMinghua   
 * @date 2014-07-25
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] dsp_rect display rectangle,source picture should lower than 1024*768
 * @param[in] dsp_buf RGB buffer address
 * @return T_BOOL 
 * @retval  AK_TRUE  refresh rgb channel data successful
 * @retval  AK_FALSE refresh rgb channel data failed.
 * @note  return failed:\n
 *     display size bigger than 1024*768\n
 *     display size smaller than 18*18
 */
T_BOOL lcd_refreshdata_RGB(T_eLCD lcd, T_RECT *dsp_rect, T_U8 *dsp_buf)
{
    T_U32 status;
    T_U32 i = 0;
    T_U32 lcd_hd_w;
    T_U32 lcd_hd_h;
    T_U32 degree;

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh RGB error, lcd is not turnon\n");
        return AK_FALSE;
    }

    if (dsp_rect->width > 1024 || dsp_rect->height > 768)
    {
        akprintf(C1, M_DRVSYS, "display size bigger than 1024*768\n");
    }

    if (dsp_rect->width < 18 || dsp_rect->height < 18 )
    {
        akprintf(C1, M_DRVSYS, "display size smaller than 18*18\n");
        return AK_FALSE;
    }

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        lcd_hd_w = lcd_get_hardware_width(lcd);
        lcd_hd_h = lcd_get_hardware_height(lcd);
    }
    else
    {
        degree = lcd_degree(lcd);
        if ((LCD_0_DEGREE == degree) || (LCD_180_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
        else if ((LCD_90_DEGREE == degree) || (LCD_270_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_height(lcd);
            lcd_hd_h = lcd_get_hardware_width(lcd);
        }
        else
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
    }

    if(((dsp_rect->left + dsp_rect->width) > lcd_hd_w) || ((dsp_rect->top + dsp_rect->height) > lcd_hd_h))
    {
        akprintf(C1, M_DRVSYS, "image out of LCD, error!\n");
    }

    //yuv on and yuv1 is full screen
    if((AK_TRUE == m_lcd_param.bYuv1FullFlag) && ((1 << 0) != (m_lcd_param.YuvOffStatus &(1 << 0))))
    {
        akprintf(C2, M_DRVSYS, "yuv1 channel is full screen, not refresh RGB channel\n"); 
        return AK_FALSE;
    }

    //if yuv channel is open and yuv1 not in RGB ,return false 
    if (YUV1_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV1_CHANNEL_EN))
    {
        T_U32 yuv1_left = (REG32(LCD_YUV1_DISPLAY_INFO_REG) >> 11) & 0x7ff;
        T_U32 yuv1_top = REG32(LCD_YUV1_DISPLAY_INFO_REG) & 0x7ff;
        T_U32 yuv1_width = (REG32(LCD_YUV1_H_INFO_REG)>> 11) & 0x7ff  ;
        T_U32 yuv1_height = (REG32(LCD_YUV1_V_INFO_REG) >> 11) & 0x7ff ;

        if (!((dsp_rect->left <= yuv1_left) && (dsp_rect->top <= yuv1_top) 
            && ((dsp_rect->left + dsp_rect->width) >= (yuv1_left + yuv1_width)) 
            && ((dsp_rect->top + dsp_rect->height) >= (yuv1_top + yuv1_height))))
        {
            akprintf(C2, M_DRVSYS, "rgb not include yuv1\n");
            return AK_FALSE;
        }
    }

    DrvModule_Protect(DRV_MODULE_LCD);
	
	if (AK_TRUE == m_lcd_param.bDMATransfering && E_LCD_TYPE_MPU == m_lcd_param.LcdType)
	{
		check_dma_finish();    //wait for previous frame dma OK
		m_lcd_param.bDMATransfering = AK_FALSE;
	}


    MMU_InvalidateDCache();

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)//rgb or tvout
    {
        //mask lcd interrupt, lcd reg config can't be disturbed
        INTR_DISABLE(IRQ_MASK_LCD_BIT);

        REG32(LCD_RGB_CTL_REG2) = (T_U32)dsp_buf & 0x1fffffff;
        //REG32(LCD_RGB_CTL_REG2) = (((T_U32)dsp_buf & 0x1fffffff) | (1 << 29));

        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)//tv out the height must be is 1/2 height
        {            
            REG32(LCD_RGB_CTL_REG2) |= (RGB_VIR_EN);
            REG32(LCD_RGB_SIZE_REG) = ((dsp_rect->width << 11) | (dsp_rect->height >> 1));
            REG32(LCD_RGB_OFFSET_REG) = ((dsp_rect->left << 11) | (dsp_rect->top >> 1));

            REG32(LCD_RGB_VIRTUAL_SIZE_REG) = ((dsp_rect->width<<1) << 16 | (dsp_rect->height>>1));
            REG32(LCD_RGB_VIRTUAL_OFFSET_REG) = (dsp_rect->left << 16 | dsp_rect->top);
        }
        else
        {
            REG32(LCD_RGB_CTL_REG2) &= ~(RGB_VIR_EN);
            REG32(LCD_RGB_SIZE_REG) = (dsp_rect->width << 11 | dsp_rect->height);
            REG32(LCD_RGB_OFFSET_REG) = (dsp_rect->left << 11 | dsp_rect->top);
        }

        REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//enable rgb channel
    }
    else//mpu
    {
        //Keep last yuv offset value
        if(YUV1_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV1_CHANNEL_EN))
           REG32(LCD_YUV1_DISPLAY_INFO_REG) |= (m_lcd_param.Yuv1OffsetX<< 11) | m_lcd_param.Yuv1OffsetY;
        if(YUV2_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV2_CHANNEL_EN))
           REG32(LCD_YUV2_DISPLAY_INFO_REG) |= (m_lcd_param.Yuv2OffsetX << 11) | m_lcd_param.Yuv2OffsetY;

        REG32(LCD_RGB_CTL_REG2) = ((T_U32)dsp_buf & 0x1fffffff);
        REG32(LCD_RGB_OFFSET_REG) = ((0 << 11) | 0);
        REG32(LCD_RGB_SIZE_REG) = ((dsp_rect->width << 11) | dsp_rect->height);
        REG32(LCD_PANEL_SIZE_REG) = ((dsp_rect->width << 11) | dsp_rect->height);

        REG32(LCD_TOP_CONFIGURE_REG) |= RGB_CHANNEL_EN;
        //set display start address
        m_lcd_param.pLcdHandler[lcd]->lcd_set_disp_address_func(lcd, dsp_rect->left, dsp_rect->top, 
                                       dsp_rect->left + dsp_rect->width - 1, dsp_rect->top + dsp_rect->height - 1);
        //start dma
        if (m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func)
            m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func(lcd);
    }

    m_lcd_param.bRgbFlag = AK_TRUE;//RGB channel is opened

    DrvModule_UnProtect(DRV_MODULE_LCD);
    return AK_TRUE;
}

/**
* @brief Refresh RGB picture,use virtual page
* @author LianGenhui
* @date 2010-06-18
* @param[in] lcd selected LCD, must be LCD_0 or LCD_1
* @param[in] dsp_rect display rectangle,display page should lower than 1024*768
* @param[in] vir_rect virture page rectangle(source page),virture page should lower than 1280*1024
* @param[in] dsp_buf RGB buffer
* @return T_BOOL
* @retval  AK_TRUE    refresh rgb channel successful
* @retval  AK_FALSE refresh rgb channel failed
* @note  return failed:\n
*     source picture smaller than display size\n
*     source picture bigger than 1280*1024、smaller than 18*18\n
*     display size smaller than 18*18, bigger than 1024*768
*/
T_BOOL lcd_refresh_RGB_ex (T_eLCD lcd, T_RECT *dsp_rect, T_RECT *vir_rect, T_U8 *dsp_buf)
{
        T_U32 lcd_hd_w;
        T_U32 lcd_hd_h;
        T_U32 degree;

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh RGB error, lcd is not turnon\n");
        return AK_FALSE;;
    }

    if ((dsp_rect->width > 1024) || (dsp_rect->height > 768) || (vir_rect->width > 1280) || (vir_rect->height > 1024) )
    {
        //akprintf(C1, M_DRVSYS, "display size bigger than 1024*768\n");
 //       return AK_FALSE;
    }

    if ((dsp_rect->width < 18) || (dsp_rect->height < 18) || (vir_rect->width < 18) || (vir_rect->height < 18))
    {
        akprintf(C1, M_DRVSYS, "display size smaller than 18*18\n");
        return AK_FALSE;
    }

    if ((dsp_rect->width + dsp_rect->left) > (vir_rect->width) &&
            (dsp_rect->height + dsp_rect->top) > (vir_rect->height))
    {
        akprintf(C1, M_DRVSYS, "display picture bigger than source picture\n"); 
//        return AK_FALSE;
    }

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        lcd_hd_w = lcd_get_hardware_width(lcd);
        lcd_hd_h = lcd_get_hardware_height(lcd);
    }
    else
    {
        degree = lcd_degree(lcd);
        if ((LCD_0_DEGREE == degree) || (LCD_180_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
        else if ((LCD_90_DEGREE == degree) || (LCD_270_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_height(lcd);
            lcd_hd_h = lcd_get_hardware_width(lcd);
        }
        else
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
    }

    if(((dsp_rect->left + dsp_rect->width) > lcd_hd_w) || ((dsp_rect->top + dsp_rect->height) > lcd_hd_h))
    {
        akprintf(C1, M_DRVSYS, "image out of LCD, error!\n");
//        return AK_FALSE;
    }

    //yuv on and yuv1 is full screen
    if((AK_TRUE == m_lcd_param.bYuv1FullFlag) && ((1 << 0) != (m_lcd_param.YuvOffStatus &(1 << 0))))
    {
        akprintf(C2, M_DRVSYS, "yuv1 channel is full screen, not refresh RGB channel\n"); 
        return AK_FALSE;
    }

    //if yuv channel is open and yuv1 not in RGB ,return false
    if (YUV1_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV1_CHANNEL_EN))
    {
        T_U32 yuv1_left = (REG32(LCD_YUV1_DISPLAY_INFO_REG) >> 11) & 0x7ff;
        T_U32 yuv1_top = REG32(LCD_YUV1_DISPLAY_INFO_REG) & 0x7ff;
        T_U32 yuv1_width = (REG32(LCD_YUV1_H_INFO_REG)>> 11) & 0x7ff;
        T_U32 yuv1_height = (REG32(LCD_YUV1_V_INFO_REG) >> 11) & 0x7ff;

        if (!((dsp_rect->left <= yuv1_left) && (dsp_rect->top <= yuv1_top) 
            && ((dsp_rect->left + dsp_rect->width) >= (yuv1_left + yuv1_width)) 
            && ((dsp_rect->top + dsp_rect->height) >= (yuv1_top + yuv1_height))))
        {
            akprintf(C2, M_DRVSYS, "rgb not include yuv1\n");
            return AK_FALSE;
        }
    }

    DrvModule_Protect(DRV_MODULE_LCD);

    if (AK_TRUE == m_lcd_param.bDMATransfering && E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        check_dma_finish();     //wait for previous frame dma OK
        m_lcd_param.bDMATransfering = AK_FALSE;
    }

    MMU_InvalidateDCache();

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        //mask lcd interrupt, lcd reg config can't be disturbed
        INTR_DISABLE(IRQ_MASK_LCD_BIT);

        //set rgb channel buffer and enable virtual page
        REG32(LCD_RGB_CTL_REG2) = (((T_U32)dsp_buf & 0x1fffffff) | (1 << 29));

        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)//tv out the height must be is 1/2 height
        {
            REG32(LCD_RGB_OFFSET_REG) = ((dsp_rect->left << 11) | (dsp_rect->top >> 1));   
            REG32(LCD_RGB_SIZE_REG) = ((dsp_rect->width << 11) | (dsp_rect->height >> 1));
            
            REG32(LCD_RGB_VIRTUAL_SIZE_REG) = ((vir_rect->width<<1) << 16 | vir_rect->height);
            REG32(LCD_RGB_VIRTUAL_OFFSET_REG) = (vir_rect->left << 16 | vir_rect->top);          
        }
        else
        {
            REG32(LCD_RGB_OFFSET_REG) = (dsp_rect->left << 11 | dsp_rect->top);   
            REG32(LCD_RGB_SIZE_REG) = (dsp_rect->width << 11 | dsp_rect->height);
            REG32(LCD_RGB_VIRTUAL_SIZE_REG) = (vir_rect->width << 16 | vir_rect->height);
            REG32(LCD_RGB_VIRTUAL_OFFSET_REG) = (vir_rect->left << 16 | vir_rect->top);           
        }
        


        REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//enable rgb channel

        enable_alert_valid_int();//inform next frame is ready
        INTR_ENABLE(IRQ_MASK_LCD_BIT);
    }
    else//mpu
    {
        if(YUV1_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV1_CHANNEL_EN))
           REG32(LCD_YUV1_DISPLAY_INFO_REG) |= (m_lcd_param.Yuv1OffsetX<< 11) | m_lcd_param.Yuv1OffsetY;
        if(YUV2_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV2_CHANNEL_EN))
           REG32(LCD_YUV2_DISPLAY_INFO_REG) |= (m_lcd_param.Yuv2OffsetX << 11) | m_lcd_param.Yuv2OffsetY;

        //set rgb channel buffer and enable virtual page
        REG32(LCD_RGB_CTL_REG2) = (((T_U32)dsp_buf & 0x1fffffff) | (1 << 29));

        REG32(LCD_RGB_OFFSET_REG) = ((0 << 11) | 0);//offset set to 0 for mpu lcd
        REG32(LCD_RGB_SIZE_REG) = ((dsp_rect->width << 11) | dsp_rect->height);
        REG32(LCD_RGB_VIRTUAL_SIZE_REG) = (vir_rect->width << 16 | vir_rect->height);
        REG32(LCD_RGB_VIRTUAL_OFFSET_REG) = (vir_rect->left << 16 | vir_rect->top);

        //panel size for MPU
        REG32(LCD_PANEL_SIZE_REG) = ((dsp_rect->width << 11) | dsp_rect->height);

        //enable rgb channel
        REG32(LCD_TOP_CONFIGURE_REG) |=  RGB_CHANNEL_EN;

        //set display start address
        m_lcd_param.pLcdHandler[lcd]->lcd_set_disp_address_func(lcd, dsp_rect->left, dsp_rect->top, 
                                                   dsp_rect->left + dsp_rect->width - 1, dsp_rect->top + dsp_rect->height - 1);
        //start dma
        if (m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func)
            m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func(lcd);

        REG32(LCD_LCD_GO_REG) |= MPU_REFLASH_START;

        m_lcd_param.bDMATransfering = AK_TRUE;
    }

    m_lcd_param.bRgbFlag = AK_TRUE;

    DrvModule_UnProtect(DRV_MODULE_LCD);
    return AK_TRUE;
}


/**
 * @brief    Refresh YUV1 picture to LCD,use YUV 420 data
 * @author    LianGenhui
 * @date    2010-06-18
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] srcY, srcU, srcV source YUV picture addess
 * @param[in] src_width, src_height the size of source YUV picture
 * @param[in] display rectangle
 * @return T_BOOL 
 * @retval  AK_TRUE  refresh YUV1 channel successful
 * @retval  AK_FALSE refresh YUV1 channel failed
 * @note   return falsed:\n
 *    source picture bigger than 1280*1024\n
 *    display picture bigger than 1024*768\n
 *    source picture and display picture smaller than 18*18\n
 *    upscaled bigger than 16 times or downscaled smaller 1/16\n
 *    if use YUV1 and RBG channel,YUV1 picture not in RGB picture\n
 *    use upscaled,source Horizonial length bigger than 682 
**/ 
T_BOOL lcd_refresh_YUV1 (T_eLCD lcd, T_U8 *srcY, T_U8 *srcU,T_U8 *srcV,
                         T_U16 src_width, T_U16 src_height,
                         T_RECT *dsp_rect)
{
    T_eLCD_DEGREE degree;
    T_U32 lcd_hd_w;
    T_U32 lcd_hd_h;

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh YUV1Fast error, lcd is not turnon\n"); 
        return AK_FALSE;
    }

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        lcd_hd_w = lcd_get_hardware_width(lcd);
        lcd_hd_h = lcd_get_hardware_height(lcd);
    }
    else
    {
        degree = lcd_degree(lcd);
        if ((LCD_0_DEGREE == degree) || (LCD_180_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
        else if ((LCD_90_DEGREE == degree) || (LCD_270_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_height(lcd);
            lcd_hd_h = lcd_get_hardware_width(lcd);
        }
        else
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
    }

    if((srcY == AK_NULL) ||(srcU == AK_NULL) || (srcV == AK_NULL)) 
    {
        akprintf(C1, M_DRVSYS, "YUV buffer is NULL, error!\n");
        return AK_FALSE;;
    }

    if((src_width > 1280) || (src_height > 1024))
    {
        akprintf(C1, M_DRVSYS, "source picture over 10280*1024, error!\n");
        return AK_FALSE;;
    }

    if((dsp_rect->width > 1024) || (dsp_rect->height > 768))
    {
        akprintf(C1, M_DRVSYS, "destination width over 1024 or height over 768, error!\n");
        return AK_FALSE;;
    }

    if ((dsp_rect->width < 18) || (dsp_rect->height < 18) )
    {
        akprintf(C1, M_DRVSYS, "display size smaller than 18*18\n"); 
        return AK_FALSE;
    }

    if(((dsp_rect->left + dsp_rect->width) > lcd_hd_w) || ((dsp_rect->top + dsp_rect->height) > lcd_hd_h))
    {
        akprintf(C1, M_DRVSYS, "image out of LCD, error!\n");
        return AK_FALSE;;
    }

    //if rgb channel is open and yuv1 not in RGB ,return false 
    if (RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & RGB_CHANNEL_EN))
    {
        T_U32 rgb_left = (REG32(LCD_RGB_OFFSET_REG) >> 11) & 0x7ff;
        T_U32 rgb_top = REG32(LCD_RGB_OFFSET_REG) & 0x7ff;
        T_U32 rgb_width = (REG32(LCD_RGB_SIZE_REG) >> 11) & 0x7ff  ;
        T_U32 rgb_height = (REG32(LCD_RGB_SIZE_REG)) & 0x7ff ;

        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)//tv out the height must be is 1/2 height
        {
            rgb_top = rgb_top << 1;
            rgb_height = rgb_height <<1;
        }
        
        if (!((dsp_rect->left >= rgb_left) && (dsp_rect->top >= rgb_top) 
            && ((dsp_rect->left + dsp_rect->width) <= (rgb_left + rgb_width)) 
            && ((dsp_rect->top + dsp_rect->height) <= (rgb_top + rgb_height))))
        {
            akprintf(C2, M_DRVSYS, "yuv1 not in rgb\n"); 
            //return AK_FALSE;
        }
    }


    DrvModule_Protect(DRV_MODULE_LCD);

    if (AK_TRUE == m_lcd_param.bDMATransfering && E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        check_dma_finish();
        m_lcd_param.bDMATransfering = AK_FALSE;
    }
    
    MMU_InvalidateDCache();

    if(E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        //mask lcd interrupt, lcd reg config can't be disturbed
        INTR_DISABLE(IRQ_MASK_LCD_BIT);

        /*yuv addr*/
        REG32(LCD_Y1_ADDR_REG) = (T_U32)srcY;
        REG32(LCD_U1_ADDR_REG) = (T_U32)srcU;
        REG32(LCD_V1_ADDR_REG) = (T_U32)srcV;

        /*scaler*/
        lcd_set_YUV_scaler(src_width, src_height, dsp_rect, EM_CHANNEL_TYPE_YUV1, AK_FALSE, AK_TRUE, lcd);

        m_lcd_param.pNextFrame = srcY; //save srcY for next frame which can compare with this frame

        //configure channel 
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV1_CHANNEL_EN);//enable yuv1 channel
        REG32(LCD_TOP_CONFIGURE_REG) &= ~(YUV2_CHANNEL_EN);//disable yuv2 channel

        
        //disable rgb channel if yuv1 channel is full screen
        if((0 == dsp_rect->left) && (0 == dsp_rect->top) 
            && (dsp_rect->width>= lcd_hd_w) && (dsp_rect->height >= lcd_hd_h))
        {
            m_lcd_param.bYuv1FullFlag = AK_TRUE;
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);//disable rgb channel
        }
        else
        {
            m_lcd_param.bYuv1FullFlag = AK_FALSE;
            if(AK_TRUE == m_lcd_param.bRgbFlag)
                REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//enable rgb channel
        }

        enable_alert_valid_int();//inform next frame is ready
        INTR_ENABLE(IRQ_MASK_LCD_BIT);
    }
    else
    {
        /*yuv addr*/
        REG32(LCD_Y1_ADDR_REG) = (T_U32)srcY;
        REG32(LCD_U1_ADDR_REG) = (T_U32)srcU;
        REG32(LCD_V1_ADDR_REG) = (T_U32)srcV;

        m_lcd_param.Yuv1OffsetX = dsp_rect->left;
        m_lcd_param.Yuv1OffsetY = dsp_rect->top;

        lcd_set_YUV_scaler(src_width, src_height, dsp_rect, EM_CHANNEL_TYPE_YUV1, AK_TRUE, AK_TRUE, lcd);

        //configure channel 
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV1_CHANNEL_EN);//enable yuv1 channel
        REG32(LCD_TOP_CONFIGURE_REG) &= ~(YUV2_CHANNEL_EN);//disable yuv2 channel
        //Judge RGB channel enable or disenable
        if(RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG)&RGB_CHANNEL_EN))
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);

        //set display address for MPU
        m_lcd_param.pLcdHandler[lcd]->lcd_set_disp_address_func(lcd, dsp_rect->left, dsp_rect->top, 
                        dsp_rect->left + dsp_rect->width - 1, dsp_rect->top + dsp_rect->height - 1);
        //start dma
        if (m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func)
            m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func(lcd);

        REG32(LCD_LCD_GO_REG) |= (MPU_REFLASH_START);
        m_lcd_param.bDMATransfering = AK_TRUE;
    }

    DrvModule_UnProtect(DRV_MODULE_LCD);
    return AK_TRUE;
}

/**
 * @brief    Refresh YUV1 channel data but not display picture on LCD,use YUV 420 data
 * @author   PanMinghua
 * @date     2014-07-25
 * @param[in] lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in] srcY, srcU, srcV source YUV picture addess
 * @param[in] src_width, src_height the size of source YUV picture
 * @param[in] display rectangle
 * @return T_BOOL 
 * @retval  AK_TRUE  refresh YUV1 channel data successful
 * @retval  AK_FALSE refresh YUV1 channel data failed
 * @note   return falsed:\n
 *    source picture bigger than 1280*1024\n
 *    display picture bigger than 1024*768\n
 *    source picture and display picture smaller than 18*18\n
 *    upscaled bigger than 16 times or downscaled smaller 1/16\n
 *    if use YUV1 and RBG channel,YUV1 picture not in RGB picture\n
 *    use upscaled,source Horizonial length bigger than 682 
**/ 
T_BOOL lcd_refreshdata_YUV1 (T_eLCD lcd, T_U8 *srcY, T_U8 *srcU,T_U8 *srcV,
                         T_U16 src_width, T_U16 src_height,
                         T_RECT *dsp_rect)
{
    T_eLCD_DEGREE degree;
    T_U32 lcd_hd_w;
    T_U32 lcd_hd_h;

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh YUV1Fast error, lcd is not turnon\n"); 
        return AK_FALSE;
    }

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        lcd_hd_w = lcd_get_hardware_width(lcd);
        lcd_hd_h = lcd_get_hardware_height(lcd);
    }
    else
    {
        degree = lcd_degree(lcd);
        if ((LCD_0_DEGREE == degree) || (LCD_180_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
        else if ((LCD_90_DEGREE == degree) || (LCD_270_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_height(lcd);
            lcd_hd_h = lcd_get_hardware_width(lcd);
        }
        else
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
    }

    if((srcY == AK_NULL) ||(srcU == AK_NULL) || (srcV == AK_NULL)) 
    {
        akprintf(C1, M_DRVSYS, "YUV buffer is NULL, error!\n");
        return AK_FALSE;;
    }

    if((src_width > 1280) || (src_height > 1024))
    {
        akprintf(C1, M_DRVSYS, "source picture over 10280*1024, error!\n");
        return AK_FALSE;;
    }

    if((dsp_rect->width > 1024) || (dsp_rect->height > 768))
    {
        akprintf(C1, M_DRVSYS, "destination width over 1024 or height over 768, error!\n");
        return AK_FALSE;;
    }

    if ((dsp_rect->width < 18) || (dsp_rect->height < 18) )
    {
        akprintf(C1, M_DRVSYS, "display size smaller than 18*18\n"); 
        return AK_FALSE;
    }

    if(((dsp_rect->left + dsp_rect->width) > lcd_hd_w) || ((dsp_rect->top + dsp_rect->height) > lcd_hd_h))
    {
        akprintf(C1, M_DRVSYS, "image out of LCD, error!\n");
        return AK_FALSE;;
    }

    //if rgb channel is open and yuv1 not in RGB ,return false 
    if (RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & RGB_CHANNEL_EN))
    {
        T_U32 rgb_left = (REG32(LCD_RGB_OFFSET_REG) >> 11) & 0x7ff;
        T_U32 rgb_top = REG32(LCD_RGB_OFFSET_REG) & 0x7ff;
        T_U32 rgb_width = (REG32(LCD_RGB_SIZE_REG) >> 11) & 0x7ff  ;
        T_U32 rgb_height = (REG32(LCD_RGB_SIZE_REG)) & 0x7ff ;

        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)//tv out the height must be is 1/2 height
        {
            rgb_top = rgb_top << 1;
            rgb_height = rgb_height <<1;
        }
        
        if (!((dsp_rect->left >= rgb_left) && (dsp_rect->top >= rgb_top) 
            && ((dsp_rect->left + dsp_rect->width) <= (rgb_left + rgb_width)) 
            && ((dsp_rect->top + dsp_rect->height) <= (rgb_top + rgb_height))))
        {
            akprintf(C2, M_DRVSYS, "yuv1 not in rgb\n"); 
        }
    }


    DrvModule_Protect(DRV_MODULE_LCD);
	
    if (AK_TRUE == m_lcd_param.bDMATransfering && E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        check_dma_finish();
        m_lcd_param.bDMATransfering = AK_FALSE;
    }

    
    MMU_InvalidateDCache();

    if(E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        //mask lcd interrupt, lcd reg config can't be disturbed
        INTR_DISABLE(IRQ_MASK_LCD_BIT);

        /*yuv addr*/
        REG32(LCD_Y1_ADDR_REG) = (T_U32)srcY;
        REG32(LCD_U1_ADDR_REG) = (T_U32)srcU;
        REG32(LCD_V1_ADDR_REG) = (T_U32)srcV;

        /*scaler*/
        lcd_set_YUV_scaler(src_width, src_height, dsp_rect, EM_CHANNEL_TYPE_YUV1, AK_FALSE, AK_TRUE, lcd);

        m_lcd_param.pNextFrame = srcY; //save srcY for next frame which can compare with this frame

        //configure channel 
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV1_CHANNEL_EN);//enable yuv1 channel
        REG32(LCD_TOP_CONFIGURE_REG) &= ~(YUV2_CHANNEL_EN);//disable yuv2 channel

        
        //disable rgb channel if yuv1 channel is full screen
        if((0 == dsp_rect->left) && (0 == dsp_rect->top) 
            && (dsp_rect->width>= lcd_hd_w) && (dsp_rect->height >= lcd_hd_h))
        {
            m_lcd_param.bYuv1FullFlag = AK_TRUE;
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);//disable rgb channel
        }
        else
        {
            m_lcd_param.bYuv1FullFlag = AK_FALSE;
            if(AK_TRUE == m_lcd_param.bRgbFlag)
                REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//enable rgb channel
        }
    }
    else
    {
        /*yuv addr*/
        REG32(LCD_Y1_ADDR_REG) = (T_U32)srcY;
        REG32(LCD_U1_ADDR_REG) = (T_U32)srcU;
        REG32(LCD_V1_ADDR_REG) = (T_U32)srcV;

        m_lcd_param.Yuv1OffsetX = dsp_rect->left;
        m_lcd_param.Yuv1OffsetY = dsp_rect->top;

        lcd_set_YUV_scaler(src_width, src_height, dsp_rect, EM_CHANNEL_TYPE_YUV1, AK_TRUE, AK_TRUE, lcd);

        //configure channel 
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV1_CHANNEL_EN);//enable yuv1 channel
        REG32(LCD_TOP_CONFIGURE_REG) &= ~(YUV2_CHANNEL_EN);//disable yuv2 channel
        //Judge RGB channel enable or disenable
        if(RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG)&RGB_CHANNEL_EN))
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);

        //set display address for MPU
        m_lcd_param.pLcdHandler[lcd]->lcd_set_disp_address_func(lcd, dsp_rect->left, dsp_rect->top, 
                        dsp_rect->left + dsp_rect->width - 1, dsp_rect->top + dsp_rect->height - 1);
        //start dma
        if (m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func)
            m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func(lcd);
    }

    DrvModule_UnProtect(DRV_MODULE_LCD);
    return AK_TRUE;
}

/**
 * @brief display YUV1 with virtual page and scaler
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd LCD_0 or LCD1
 * @param[in] display rectangle
 * @param[in] virture page rectangle
 * @param[in] source_width,source_height  the size of source page
 * @param[in] srcY, srcU, srcV    YUV source buffer address
 * @return T_BOOL
 * @retval    AK_TRUE  refresh YUV1 channel successful
 * @retval    AK_FALSE refresh YUV1 channel failed
 * @note   return falsed: refer to lcd_refresh_YUV1
 **/
T_BOOL lcd_refresh_YUV1_ex (T_eLCD lcd,T_U8 *srcY, T_U8 *srcU, T_U8 *srcV,
                                T_U16 src_width, T_U16 src_height,
                                T_RECT *vir_rect,T_RECT *dsp_rect)
{
    T_eLCD_DEGREE degree;
    T_U32 lcd_hd_w;
    T_U32 lcd_hd_h;

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh YUV1 error, lcd is not turnon\n");
        return AK_FALSE;
    }

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        lcd_hd_w = lcd_get_hardware_width(lcd);
        lcd_hd_h = lcd_get_hardware_height(lcd);
    }
    else
    {
        degree = lcd_degree(lcd);
        if ((LCD_0_DEGREE == degree) || (LCD_180_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
        else if ((LCD_90_DEGREE == degree) || (LCD_270_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_height(lcd);
            lcd_hd_h = lcd_get_hardware_width(lcd);
        }
        else
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
    }

    if((srcY == AK_NULL) ||(srcU == AK_NULL) || (srcV == AK_NULL))
    {
        akprintf(C1, M_DRVSYS, "YUV buffer is NULL, error!\n");
        return AK_FALSE;
    }

    if((vir_rect->width > 1280) || (vir_rect->height > 1024))
    {
        akprintf(C1, M_DRVSYS, "source picture over 1280*1024, error!\n");
        return AK_FALSE;
    }

    if ((dsp_rect->width < 18) || (dsp_rect->height < 18))
    {
        akprintf(C1, M_DRVSYS, "display size smaller than 18*18\n");
        return AK_FALSE;
    }

    if((dsp_rect->width > 1024) || (dsp_rect->height > 768))
    {
        akprintf(C1, M_DRVSYS, "destination width over 1024 or height over 768, error!\n");
        return AK_FALSE;
    }


    if(((dsp_rect->left + dsp_rect->width) > lcd_hd_w) || ((dsp_rect->top + dsp_rect->height) > lcd_hd_h))
    {
        akprintf(C1, M_DRVSYS, "image out of LCD, error!\n");
        return AK_FALSE;
    }

    //if rgb channel is open and yuv1 not in RGB ,return false
    if (RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & RGB_CHANNEL_EN))
    {
        T_U32 rgb_left = (REG32(LCD_RGB_OFFSET_REG) >> 11) & 0x7ff;
        T_U32 rgb_top = REG32(LCD_RGB_OFFSET_REG) & 0x7ff;
        T_U32 rgb_width = (REG32(LCD_RGB_SIZE_REG) >> 11) & 0x7ff;
        T_U32 rgb_height = (REG32(LCD_RGB_SIZE_REG)) & 0x7ff;

        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)//tv out the height must be is 1/2 height
        {
            rgb_top = rgb_top << 1;
            rgb_height = rgb_height <<1;
        }


        if (!((dsp_rect->left >= rgb_left) && (dsp_rect->top >= rgb_top) 
            && ((dsp_rect->left + dsp_rect->width) <= (rgb_left + rgb_width)) 
            && ((dsp_rect->top + dsp_rect->height) <= (rgb_top + rgb_height))))
        {
            akprintf(C2, M_DRVSYS, "yuv1 not in rgb\n"); 
            //return AK_FALSE;
        }
    }

    DrvModule_Protect(DRV_MODULE_LCD);

    if (AK_TRUE == m_lcd_param.bDMATransfering && E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        check_dma_finish();
        m_lcd_param.bDMATransfering = AK_FALSE;
    }
    
    MMU_InvalidateDCache();

    /*data refresh*/
    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        //mask lcd interrupt, lcd reg config can't be disturbed
        INTR_DISABLE(IRQ_MASK_LCD_BIT);

        /*yuv addr*/
        REG32(LCD_Y1_ADDR_REG) = (T_U32)srcY;
        REG32(LCD_U1_ADDR_REG) = (T_U32)srcU;
        REG32(LCD_V1_ADDR_REG) = (T_U32)srcV;

        lcd_set_YUV_scaler(vir_rect->width, vir_rect->height, dsp_rect, EM_CHANNEL_TYPE_YUV1, AK_FALSE, AK_FALSE, lcd);

        REG32(LCD_YUV1_VIR_SIZE_REG) = ((src_width << 16) | (src_height));
        REG32(LCD_YUV1_VIR_OFFSET_REG) = ((vir_rect->left << 16) | (vir_rect->top));
        REG32(LCD_YUV1_DISPLAY_INFO_REG) |= (1<<24);//virtual page enable

		m_lcd_param.pNextFrame = srcY; //save srcY for next frame which can compare with this frame

        //configure channel 
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV1_CHANNEL_EN);//enable yuv1 channel
        REG32(LCD_TOP_CONFIGURE_REG) &= ~(YUV2_CHANNEL_EN);//disable yuv2 channel

        //disable rgb channel if yuv1 channel is full screen
        if ((0 == dsp_rect->left) && (0 == dsp_rect->top) && (dsp_rect->width>= lcd_hd_w) 
                                                                        && (dsp_rect->height >= lcd_hd_h))
        {
            m_lcd_param.bYuv1FullFlag = AK_TRUE;
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);//disable rgb channel
        }
        else
        {
            m_lcd_param.bYuv1FullFlag = AK_FALSE;
            if (AK_TRUE == m_lcd_param.bRgbFlag)
                REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//disable rgb channel
        }

        enable_alert_valid_int();//inform next frame is ready
        INTR_ENABLE(IRQ_MASK_LCD_BIT);
    }
    else
    {
        /*yuv addr*/
        REG32(LCD_Y1_ADDR_REG) = (T_U32)srcY;
        REG32(LCD_U1_ADDR_REG) = (T_U32)srcU;
        REG32(LCD_V1_ADDR_REG) = (T_U32)srcV;

        lcd_set_YUV_scaler(vir_rect->width, vir_rect->height, dsp_rect, EM_CHANNEL_TYPE_YUV1, AK_FALSE, AK_FALSE, lcd);

        m_lcd_param.Yuv1OffsetX = dsp_rect->left;
        m_lcd_param.Yuv1OffsetY = dsp_rect->top;

        REG32(LCD_YUV1_VIR_SIZE_REG) = ((src_width << 16) | (src_height));
        REG32(LCD_YUV1_VIR_OFFSET_REG) = ((vir_rect->left << 16) | (vir_rect->top));
        REG32(LCD_YUV1_DISPLAY_INFO_REG) |= (1<<24);//virtual page

        REG32(LCD_PANEL_SIZE_REG) = (dsp_rect->width<< 11) | (dsp_rect->height);

        //configure channel
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV1_CHANNEL_EN);//enable yuv1 channel
        REG32(LCD_TOP_CONFIGURE_REG) &= ~(YUV2_CHANNEL_EN);//disable yuv2 channel

        //Judge RGB channel enable or disenable
        if(RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG)&RGB_CHANNEL_EN))
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);

        //set display address
        m_lcd_param.pLcdHandler[lcd]->lcd_set_disp_address_func(lcd, dsp_rect->left, dsp_rect->top, 
                            dsp_rect->left + dsp_rect->width - 1, dsp_rect->top + dsp_rect->height - 1);

        //start dma
        if (m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func)
            m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func(lcd);

        REG32(LCD_LCD_GO_REG) |= (MPU_REFLASH_START);
        m_lcd_param.bDMATransfering = AK_TRUE;
    }

    DrvModule_UnProtect(DRV_MODULE_LCD);
    return AK_TRUE;
}


/**
 * @brief    Refresh YUV1 and YUV2 picture to LCD
 * @author   LianGenhui
 * @date     2010-06-18
 * @param[in]     lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in]     srcY1, srcU1, srcV1  source YUV1 picture addess;
 * @param[in]     srcY2, srcU2, srcV2  source YUV2 picture addess;
 * @param[in]     src_width1, src_height1 source YUV1 picture size;
 * @param[in]     src_width2, src_height2 source YUV2 picture size;
 * @param[in]     display rectangle of YUV1
 * @param[in]     display rectangle of YUV2
 * @return  T_BOOL
 * @retval  AK_TRUE  refresh YUV1 and YUV2 channel successful
 * @retval  AK_FALSE refresh YUV1 and YUV2 channel failed
 * @note    return falsed:\n
 *    YUV1 source picture bigger than 1280*1024\n
 *    YUV1 display picture bigger than 1024*768\n
 *    YUV1 source and display size smaller than 18*18\n
 *    YUV1 upscaled bigger than 16 times or downscaled smaller 1/16\n
 *    YUV1 picture not in RGB picture if used YUV1 and RBG channel both\n
 *    source Horizonial length bigger than 682 if YUV1 used upscaled\n
 *    YUV2 picture bigger than 320*240
**/ 
T_BOOL lcd_refresh_dual_YUV (T_eLCD lcd, T_U8 *srcY1,T_U8 *srcU1,T_U8 *srcV1,
                     T_U16 src_width1, T_U16 src_height1,
                     T_RECT *dsp_rect1,
                     T_U8 *srcY2,T_U8 *srcU2,T_U8 *srcV2,
                     T_U16 src_width2, T_U16 src_height2,
                     T_RECT *dsp_rect2)
{
    T_eLCD_DEGREE degree;
    T_U32 lcd_hd_w;
    T_U32 lcd_hd_h;
    T_U32 include=0;//include bit of LCD_YUV2_DISPLAY_INFO_REG
    T_U32 status;
    T_U32 i = 0;

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh RGB error, lcd is not turnon\n");
        return AK_FALSE;
    }


    if((srcY1 == AK_NULL) ||(srcU1 == AK_NULL) || (srcV1 == AK_NULL)
                || (srcY2 == AK_NULL) ||(srcU2 == AK_NULL) || (srcV2 == AK_NULL)) 
    {
        akprintf(C1, M_DRVSYS, "YUV buffer is NULL, error!\n");
        return AK_FALSE;
    }

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        lcd_hd_w = lcd_get_hardware_width(lcd);
        lcd_hd_h = lcd_get_hardware_height(lcd);
    }
    else
    {
        degree = lcd_degree(lcd);
        if ((LCD_0_DEGREE == degree) || (LCD_180_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
        else if ((LCD_90_DEGREE == degree) || (LCD_270_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_height(lcd);
            lcd_hd_h = lcd_get_hardware_width(lcd);
        }
        else
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
    }

    if((src_width1 > 1280) || (src_height1 > 1024) || (src_width2 > 1280) || (src_height2 > 1024))
    {
        akprintf(C2, M_DRVSYS, "source picture over 1280*1024, error!\n");
        return AK_FALSE;;
    }

    if((dsp_rect1->width > 1024) || (dsp_rect1->height > 768) || (dsp_rect2->width > 640) || (dsp_rect2->height > 480))
    {
        akprintf(C2, M_DRVSYS, "destination width over  or height over , error!\n");
        return AK_FALSE;
    }

    if ((dsp_rect1->width < 18) || (dsp_rect1->height < 18) || (dsp_rect2->width < 18) || (dsp_rect2->height < 18))
    {
        akprintf(C2, M_DRVSYS, "display size smaller than 18*18\n");
        return AK_FALSE;
    }

    if(((dsp_rect1->left + dsp_rect1->width) > lcd_hd_w) || ((dsp_rect1->top + dsp_rect1->height) > lcd_hd_h)
                || ((dsp_rect2->left + dsp_rect2->width) > lcd_hd_w) || ((dsp_rect2->top + dsp_rect2->height) > lcd_hd_h))
    {
        akprintf(C1, M_DRVSYS, "image out of LCD, error!\n");
        return AK_FALSE;
    }

    /*judge position information*/
    if ((dsp_rect1->left <= dsp_rect2->left) && (dsp_rect1->top <= dsp_rect2->top) 
        && ((dsp_rect1->left + dsp_rect1->width) >= (dsp_rect2->left + dsp_rect2->width)) 
        && ((dsp_rect1->top + dsp_rect1->height) >= (dsp_rect2->top + dsp_rect2->height)))
    {
        include = 0;    //yuv2 in yuv1
    }
    else if (((dsp_rect2->left >= (dsp_rect1->left + dsp_rect1->width)) || (dsp_rect2->top >= (dsp_rect1->top + dsp_rect1->height))
            || ((dsp_rect2->left + dsp_rect2->width) <= dsp_rect1->left) || ((dsp_rect2->top + dsp_rect2->height) <= dsp_rect1->top))  //out yuv2
            && ((dsp_rect2->left + dsp_rect2->width) <= lcd_hd_w)   //in pannel
            && ((dsp_rect2->top + dsp_rect2->height) <= lcd_hd_h))   //in pannel
    {
        include = 1;//yuv2 out yuv1
    }
    else
    {
        akprintf(C1, M_DRVSYS, "YUV1 + YU2 position error\n");
        akprintf(C1, M_DRVSYS, "YUV1,left=%d,top=%d,width=%d,height=%d\n",dsp_rect1->left,dsp_rect1->top,dsp_rect1->width,dsp_rect1->height);
        akprintf(C1, M_DRVSYS, "YUV2,left=%d,top=%d,width=%d,height=%d\n",dsp_rect2->left,dsp_rect2->top,dsp_rect2->width,dsp_rect2->height);
        akprintf(C1, M_DRVSYS, "pannel width=%d,pannel height=%d\n",lcd_get_hardware_width(lcd),lcd_get_hardware_height(lcd));
        return AK_FALSE;
    }

    if (include == 1)
    {
        if (E_LCD_TYPE_MPU == m_lcd_param.LcdType)
        {
            akprintf(C3, M_DRVSYS, "MPU LCD not support YUV2 out YUV1\n");
            return AK_FALSE;
        }
    }

    //if rgb channel is open and yuv1 not in RGB ,return false
    if (RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & RGB_CHANNEL_EN))
    {
        T_U32 rgb_left = (REG32(LCD_RGB_OFFSET_REG) >> 11) & 0x7ff;
        T_U32 rgb_top = REG32(LCD_RGB_OFFSET_REG) & 0x7ff;
        T_U32 rgb_width = (REG32(LCD_RGB_SIZE_REG) >> 11) & 0x7ff;
        T_U32 rgb_height = (REG32(LCD_RGB_SIZE_REG)) & 0x7ff;

        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)//tv out the height must be is 1/2 height
        {
            rgb_top = rgb_top << 1;
            rgb_height = rgb_height <<1;
        }


        if (!((dsp_rect1->left >= rgb_left) && (dsp_rect1->top >= rgb_top) 
            && ((dsp_rect1->left + dsp_rect1->width) <= (rgb_left + rgb_width)) 
            && ((dsp_rect1->top + dsp_rect1->height) <= (rgb_top + rgb_height))))
        {
            akprintf(C2, M_DRVSYS, "yuv1 not in rgb\n"); 
            //return AK_FALSE;
        }
    }

    //if osd channel is opened,osd can't cover up yuv2 and can't display in the same line 
    if (YUV2_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV2_CHANNEL_EN))
    {
        T_U32 osd_left = (REG32(LCD_OSD_OFFSET_REG) >> 11) & 0x7ff;
        T_U32 osd_top = REG32(LCD_OSD_OFFSET_REG) & 0x7ff;
        T_U32 osd_width = REG32(LCD_OSD_SIZE_ALPHA_REG) & 0x7ff;
        T_U32 osd_height = (REG32(LCD_OSD_SIZE_ALPHA_REG) >> 11) & 0x7ff;

        if (!((dsp_rect2->left >= osd_left + osd_width) || (dsp_rect2->top >= osd_top + osd_height) 
          || ((dsp_rect2->left + dsp_rect2->width) <= osd_left) || ((dsp_rect2->top + dsp_rect2->height) <= osd_top)))
        {
            akprintf(C2, M_DRVSYS, "osd not cover up yuv2\n");
            return AK_FALSE;
        }

        if (!((dsp_rect2->top >= osd_top + osd_height)
          || ((dsp_rect2->top + dsp_rect2->height) <= osd_top)) )
        {
            akprintf(C2, M_DRVSYS, "osd and yuv2 can't display in the same line\n"); 
            return AK_FALSE;
        }
    }

    DrvModule_Protect(DRV_MODULE_LCD);

    if (AK_TRUE == m_lcd_param.bDMATransfering && E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        check_dma_finish();
        m_lcd_param.bDMATransfering = AK_FALSE;
    }

    MMU_InvalidateDCache();

    /*transfer data*/
    if(E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        //mask lcd interrupt, lcd reg config can't be disturbed
        INTR_DISABLE(IRQ_MASK_LCD_BIT);

        /*yuv addr*/
        REG32(LCD_Y1_ADDR_REG) = (T_U32)srcY1;
        REG32(LCD_U1_ADDR_REG) = (T_U32)srcU1;
        REG32(LCD_V1_ADDR_REG) = (T_U32)srcV1;
        REG32(LCD_Y2_ADDR_REG) = (T_U32)srcY2;
        REG32(LCD_U2_ADDR_REG) = (T_U32)srcU2;
        REG32(LCD_V2_ADDR_REG) = (T_U32)srcV2;

        /*scaler*/
        lcd_set_YUV_scaler(src_width1, src_height1, dsp_rect1, EM_CHANNEL_TYPE_YUV1, AK_FALSE, AK_TRUE, lcd);
        lcd_set_YUV_scaler(src_width2, src_height2, dsp_rect2, EM_CHANNEL_TYPE_YUV2, AK_FALSE, AK_TRUE, lcd);

        m_lcd_param.Yuv1OffsetX = dsp_rect1->left;
        m_lcd_param.Yuv1OffsetY = dsp_rect1->top;
        m_lcd_param.Yuv2OffsetX = dsp_rect2->left;
        m_lcd_param.Yuv2OffsetY = dsp_rect2->top;

        /*position information*/
        REG32(LCD_YUV2_DISPLAY_INFO_REG) |= (include << 24);

		m_lcd_param.pNextFrame = srcY1; //save srcY for next frame which can compare with this frame

        //configure channel 
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV1_CHANNEL_EN);//enable YUV1 channel
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV2_CHANNEL_EN);//enable YUV2 channe

        //disable rgb channel if yuv1 channel is full screen
        if((0 == dsp_rect1->left) && (0 == dsp_rect1->top) && (dsp_rect1->width>= lcd_hd_w) 
            && (dsp_rect1->height >= lcd_hd_h))
        {
            m_lcd_param.bYuv1FullFlag = AK_TRUE;
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);//disable rgb channel
        }
        else
        {
            m_lcd_param.bYuv1FullFlag = AK_FALSE;
            if(AK_TRUE == m_lcd_param.bRgbFlag)
                REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//disable rgb channel
        }

        enable_alert_valid_int();//inform next frame is ready
        INTR_ENABLE(IRQ_MASK_LCD_BIT);
    }
    else    //yuv2 in yuv1 display on MPU LCD
    {
        /*yuv addr*/
        REG32(LCD_Y1_ADDR_REG) = (T_U32)srcY1;
        REG32(LCD_U1_ADDR_REG) = (T_U32)srcU1;
        REG32(LCD_V1_ADDR_REG) = (T_U32)srcV1;
        REG32(LCD_Y2_ADDR_REG) = (T_U32)srcY2;
        REG32(LCD_U2_ADDR_REG) = (T_U32)srcU2;
        REG32(LCD_V2_ADDR_REG) = (T_U32)srcV2;

        //scaler
        lcd_set_YUV_scaler(src_width1, src_height1, dsp_rect1, EM_CHANNEL_TYPE_YUV1, AK_TRUE, AK_TRUE, lcd);
        lcd_set_YUV_scaler(src_width2, src_height2, dsp_rect2, EM_CHANNEL_TYPE_YUV2, AK_TRUE, AK_TRUE, lcd);

        //position information
        REG32(LCD_YUV2_DISPLAY_INFO_REG) |= (include << 24);

        //configure channel
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV1_CHANNEL_EN);//enable YUV1 channel
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV2_CHANNEL_EN);//enable YUV2 channe
        //Judge RGB channel enable or disenable
        if(RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG)&RGB_CHANNEL_EN))
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);

        //disable rgb channel if yuv1 channel is full screen
        if((0 == dsp_rect1->left) && (0 == dsp_rect1->top) && (dsp_rect1->width>= lcd_hd_w) 
            && (dsp_rect1->height >= lcd_hd_h))
        {
            m_lcd_param.bYuv1FullFlag = AK_TRUE;
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);//disable rgb channel
        }
        else
        {
            m_lcd_param.bYuv1FullFlag = AK_FALSE;
            if(AK_TRUE == m_lcd_param.bRgbFlag)
                REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//disable rgb channel
        }

        //set display address
        m_lcd_param.pLcdHandler[lcd]->lcd_set_disp_address_func(lcd, dsp_rect1->left, dsp_rect1->top,
                                dsp_rect1->left + dsp_rect1->width - 1, dsp_rect1->top + dsp_rect1->height - 1); 

        //start dma
        if (m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func)
            m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func(lcd);

        REG32(LCD_LCD_GO_REG) |= MPU_REFLASH_START;
        m_lcd_param.bDMATransfering = AK_TRUE;
    }

    DrvModule_UnProtect(DRV_MODULE_LCD);
    return AK_TRUE;
}

/**
 * @brief    Refresh YUV1 and YUV2 picture to LCD,display YUV1 with virtual page and scaler
 * @author   LianGenhui
 * @date     2010-06-18
 * @param[in]     lcd selected LCD, must be LCD_0 or LCD_1
 * @param[in]     srcY1, srcU1, srcV1  source YUV1 picture addess;
 * @param[in]     srcY2, srcU2, srcV2  source YUV2 picture addess;
 * @param[in]     src_width1, src_height1 source YUV1 picture size;
 * @param[in]     src_width2, src_height2 source YUV2 picture size;
 * @param[in]     virtual page rectangle of YUV1
 * @param[in]     display rectangle of YUV1
 * @param[in]     display rectangle of YUV2
 * @return  T_BOOL
 * @retval  AK_TRUE  refresh YUV1 and YUV2 channel successful
 * @retval  AK_FALSE refresh YUV1 and YUV2 channel failed
 * @note    return falsed: refer to lcd_refresh_dual_YUV
 **/ 
T_BOOL lcd_refresh_dual_YUV_ex (T_eLCD lcd, T_U8 *srcY1,T_U8 *srcU1,T_U8 *srcV1,
                     T_U16 src_width1, T_U16 src_height1,
                     T_RECT *vir_rect1, T_RECT *dsp_rect1,
                     T_U8 *srcY2,T_U8 *srcU2,T_U8 *srcV2,
                     T_U16 src_width2, T_U16 src_height2,
                     T_RECT *dsp_rect2)
{
    T_eLCD_DEGREE degree;
    T_U32 lcd_hd_w;
    T_U32 lcd_hd_h;
    T_U32 include=0;//include bit of LCD_YUV2_DISPLAY_INFO_REG

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh RGB error, lcd is not turnon\n");
        return AK_FALSE;
    }

    if((srcY1 == AK_NULL) ||(srcU1 == AK_NULL) || (srcV1 == AK_NULL)
                 || (srcY2 == AK_NULL) ||(srcU2 == AK_NULL) || (srcV2 == AK_NULL)) 
    {
        akprintf(C1, M_DRVSYS, "YUV buffer is NULL, error!\n");
        return AK_FALSE;
    }

    if (E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        lcd_hd_w = lcd_get_hardware_width(lcd);
        lcd_hd_h = lcd_get_hardware_height(lcd);
    }
    else
    {
        degree = lcd_degree(lcd);
        if ((LCD_0_DEGREE == degree) || (LCD_180_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
        else if ((LCD_90_DEGREE == degree) || (LCD_270_DEGREE == degree))
        {
            lcd_hd_w = lcd_get_hardware_height(lcd);
            lcd_hd_h = lcd_get_hardware_width(lcd);
        }
        else
        {
            lcd_hd_w = lcd_get_hardware_width(lcd);
            lcd_hd_h = lcd_get_hardware_height(lcd);
        }
    }

    if((src_width1 > 1280) || (src_height1 > 1024) || (src_width2 > 1280) || (src_height2 > 1024))
    {
        akprintf(C2, M_DRVSYS, "source picture over 1280*1024, error!\n");
        return AK_FALSE;
    }

    if((dsp_rect1->width > 1024) || (dsp_rect1->height > 768) || (dsp_rect2->width > 640) || (dsp_rect2->height > 480))
    {
        akprintf(C2, M_DRVSYS, "destination width over  or height over , error!\n");
        return AK_FALSE;
    }

    if ((dsp_rect1->width < 18) || (dsp_rect1->height < 18) || (dsp_rect2->width < 18) || (dsp_rect2->height < 18))
    {
        akprintf(C2, M_DRVSYS, "display size smaller than 18*18\n");
        return AK_FALSE;
    }

    if(((dsp_rect1->left + dsp_rect1->width) > lcd_hd_w) || ((dsp_rect1->top + dsp_rect1->height) > lcd_hd_h)
        || ((dsp_rect2->left + dsp_rect2->width) > lcd_hd_w) || ((dsp_rect2->top + dsp_rect2->height) > lcd_hd_h))
    {
        akprintf(C1, M_DRVSYS, "image out of LCD, error!\n");
        return AK_FALSE;
    }

    /*judge position information*/
    if ((dsp_rect1->left <= dsp_rect2->left) && (dsp_rect1->top <= dsp_rect2->top) 
        && ((dsp_rect1->left + dsp_rect1->width) >= (dsp_rect2->left + dsp_rect2->width)) 
        && ((dsp_rect1->top + dsp_rect1->height) >= (dsp_rect2->top + dsp_rect2->height)))
    {
        include = 0;    //yuv2 in yuv1
    }
    else if (((dsp_rect2->left >= (dsp_rect1->left + dsp_rect1->width)) || (dsp_rect2->top >= (dsp_rect1->top + dsp_rect1->height))
            || ((dsp_rect2->left + dsp_rect2->width) <= dsp_rect1->left) || ((dsp_rect2->top + dsp_rect2->height) <= dsp_rect1->top))  //out yuv2
            &&((dsp_rect2->left + dsp_rect2->width) <= lcd_hd_w)   //in pannel
            &&((dsp_rect2->top + dsp_rect2->height) <= lcd_hd_h))   //in pannel
    {
        include = 1;//yuv2 out yuv1
    }
    else
    {
        akprintf(C1, M_DRVSYS, "YUV1 + YU2 position error\n");
        akprintf(C1, M_DRVSYS, "YUV1,left=%d,top=%d,width=%d,height=%d\n",dsp_rect1->left,dsp_rect1->top,dsp_rect1->width,dsp_rect1->height);
        akprintf(C1, M_DRVSYS, "YUV2,left=%d,top=%d,width=%d,height=%d\n",dsp_rect2->left,dsp_rect2->top,dsp_rect2->width,dsp_rect2->height);
        akprintf(C1, M_DRVSYS, "pannel width=%d,pannel height=%d\n",lcd_get_hardware_width(lcd),lcd_get_hardware_height(lcd));
        return AK_FALSE;
    }

    if (include == 1)
    {
        if(E_LCD_TYPE_MPU == m_lcd_param.LcdType)
        {
            akprintf(C1, M_DRVSYS, "MPU LCD not support YUV2 out of YUV1\n");
            return AK_FALSE;
        }
    }

    //if rgb channel is open and yuv1 not in RGB ,return false
    if (RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & RGB_CHANNEL_EN))
    {
        T_U32 rgb_left = (REG32(LCD_RGB_OFFSET_REG) >> 11) & 0x7ff;
        T_U32 rgb_top = REG32(LCD_RGB_OFFSET_REG) & 0x7ff;
        T_U32 rgb_width = (REG32(LCD_RGB_SIZE_REG) >> 11) & 0x7ff;
        T_U32 rgb_height = (REG32(LCD_RGB_SIZE_REG)) & 0x7ff;

        if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)//tv out the height must be is 1/2 height
        {
            rgb_top = rgb_top << 1;
            rgb_height = rgb_height <<1;
        }


        if (!((dsp_rect1->left >= rgb_left) && (dsp_rect1->top >= rgb_top) 
            && ((dsp_rect1->left + dsp_rect1->width) <= (rgb_left + rgb_width)) 
            && ((dsp_rect1->top + dsp_rect1->height) <= (rgb_top + rgb_height))))
        {
            akprintf(C2, M_DRVSYS, "yuv1 not in rgb\n"); 
            //return AK_FALSE;
        }
    }

    //if osd channel is opened,osd can't cover up yuv2 and can't display in the same line 
    if (YUV2_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV2_CHANNEL_EN))
    {
        T_U32 osd_left = (REG32(LCD_OSD_OFFSET_REG) >> 11) & 0x7ff;
        T_U32 osd_top = REG32(LCD_OSD_OFFSET_REG) & 0x7ff;
        T_U32 osd_width = REG32(LCD_OSD_SIZE_ALPHA_REG) & 0x7ff;
        T_U32 osd_height = (REG32(LCD_OSD_SIZE_ALPHA_REG) >> 11) & 0x7ff;

        if (!((dsp_rect2->left >= osd_left + osd_width) || (dsp_rect2->top >= osd_top + osd_height) 
          || ((dsp_rect2->left + dsp_rect2->width) <= osd_left) || ((dsp_rect2->top + dsp_rect2->height) <= osd_top)))
        {
            akprintf(C2, M_DRVSYS, "osd not cover up yuv2\n");
            return AK_FALSE;
        }

        if (!((dsp_rect2->top >= osd_top + osd_height) 
          || ((dsp_rect2->top + dsp_rect2->height) <= osd_top)) )
        {
            akprintf(C2, M_DRVSYS, "osd and yuv2 can't display in the same line\n"); 
            return AK_FALSE;
        }
    }

    DrvModule_Protect(DRV_MODULE_LCD);

    if (AK_TRUE == m_lcd_param.bDMATransfering && E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        check_dma_finish();
        m_lcd_param.bDMATransfering = AK_FALSE;
    }

    MMU_InvalidateDCache();

    /*transfer data*/
    if(E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        //mask lcd interrupt, lcd reg config can't be disturbed
        INTR_DISABLE(IRQ_MASK_LCD_BIT);

        /*yuv addr*/
        REG32(LCD_Y1_ADDR_REG) = (T_U32)srcY1;
        REG32(LCD_U1_ADDR_REG) = (T_U32)srcU1;
        REG32(LCD_V1_ADDR_REG) = (T_U32)srcV1;
        REG32(LCD_Y2_ADDR_REG) = (T_U32)srcY2;
        REG32(LCD_U2_ADDR_REG) = (T_U32)srcU2;
        REG32(LCD_V2_ADDR_REG) = (T_U32)srcV2;

        /*scaler*/
        lcd_set_YUV_scaler(vir_rect1->width, vir_rect1->height, dsp_rect1, EM_CHANNEL_TYPE_YUV1, AK_FALSE, AK_FALSE, lcd);
        lcd_set_YUV_scaler(src_width2, src_height2, dsp_rect2, EM_CHANNEL_TYPE_YUV2, AK_FALSE, AK_FALSE, lcd);

        REG32(LCD_YUV1_VIR_OFFSET_REG) = ((vir_rect1->left << 16) | (vir_rect1->top));
        REG32(LCD_YUV1_VIR_SIZE_REG) = ((src_width1 << 16) | (src_height1));
        REG32(LCD_YUV1_DISPLAY_INFO_REG) |= (1<<24);//virture page enable

        /*position information*/
        REG32(LCD_YUV2_DISPLAY_INFO_REG) |= (include << 24);

		m_lcd_param.pNextFrame = srcY1; //save srcY for next frame which can compare with this frame

        //configure channel
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV1_CHANNEL_EN);//enable YUV1 channel
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV2_CHANNEL_EN);//enable YUV2 channe

        //disable rgb channel if yuv1 channel is full screen
        if((0 == dsp_rect1->left) && (0 == dsp_rect1->top) && (dsp_rect1->width>= lcd_hd_w) 
            && (dsp_rect1->height >= lcd_hd_h))
        {
            m_lcd_param.bYuv1FullFlag = AK_TRUE;
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);//disable rgb channel
        }
        else
        {
            m_lcd_param.bYuv1FullFlag = AK_FALSE;
            if(AK_TRUE == m_lcd_param.bRgbFlag)
                REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//disable rgb channel
        }

        enable_alert_valid_int();//inform next frame is ready
        INTR_ENABLE(IRQ_MASK_LCD_BIT);
    }
    else
    {
        /*yuv addr*/
        REG32(LCD_Y1_ADDR_REG) = (T_U32)srcY1;
        REG32(LCD_U1_ADDR_REG) = (T_U32)srcU1;
        REG32(LCD_V1_ADDR_REG) = (T_U32)srcV1;
        REG32(LCD_Y2_ADDR_REG) = (T_U32)srcY2;
        REG32(LCD_U2_ADDR_REG) = (T_U32)srcU2;
        REG32(LCD_V2_ADDR_REG) = (T_U32)srcV2;

        //scaler
        lcd_set_YUV_scaler(vir_rect1->width, vir_rect1->height, dsp_rect1, EM_CHANNEL_TYPE_YUV1, AK_TRUE, AK_FALSE, lcd);
        lcd_set_YUV_scaler(src_width2, src_height2, dsp_rect2, EM_CHANNEL_TYPE_YUV2, AK_TRUE, AK_FALSE, lcd);

        REG32(LCD_YUV1_VIR_SIZE_REG) = ((src_width1 << 16) | (src_height1));
        REG32(LCD_YUV1_VIR_OFFSET_REG) = ((vir_rect1->left << 16) | (vir_rect1->top));
        REG32(LCD_YUV1_DISPLAY_INFO_REG) |= (1<<24);//virtual page enable

        //panel size for mpu lcd
        REG32(LCD_PANEL_SIZE_REG) = ((dsp_rect1->width << 11) | dsp_rect1->height);
        
        //position information
        REG32(LCD_YUV2_DISPLAY_INFO_REG) |= (include << 22);

        //configure channel
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV1_CHANNEL_EN);//enable YUV1 channel
        REG32(LCD_TOP_CONFIGURE_REG) |= (YUV2_CHANNEL_EN);//enable YUV2 channe
        //Judge RGB channel enable or disenable
        if(RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG)&RGB_CHANNEL_EN))
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);

        //disable rgb channel if yuv1 channel is full screen
        if((0 == dsp_rect1->left) && (0 == dsp_rect1->top) && (dsp_rect1->width>= lcd_hd_w) 
            && (dsp_rect1->height >= lcd_hd_h))
        {
            m_lcd_param.bYuv1FullFlag = AK_TRUE;
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);//disable rgb channel
        }
        else
        {
            m_lcd_param.bYuv1FullFlag = AK_FALSE;
            if(AK_TRUE == m_lcd_param.bRgbFlag)
                REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//disable rgb channel
        }

        //set display address
        m_lcd_param.pLcdHandler[lcd]->lcd_set_disp_address_func(lcd, dsp_rect1->left, dsp_rect1->top,
                                dsp_rect1->left + dsp_rect1->width - 1, dsp_rect1->top + dsp_rect1->height - 1); 

        //start dma
        if (m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func)
            m_lcd_param.pLcdHandler[lcd]->lcd_start_dma_func(lcd);

        REG32(LCD_LCD_GO_REG) |= MPU_REFLASH_START;
        m_lcd_param.bDMATransfering = AK_TRUE;
    }

    DrvModule_UnProtect(DRV_MODULE_LCD);
    return AK_TRUE;
}

/**
 * @brief open YUV channel
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_VOID
 */
T_VOID  lcd_YUV_on (T_VOID)
{
    T_U32 status;
    T_U32 lcd = LCD_0;
    
    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh RGB error, lcd is not turnon\n");
        return;
    }

    //if rgb channel is open and yuv1 not in RGB ,return false
    if (RGB_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & RGB_CHANNEL_EN))
    {

        T_U32 rgb_left = (REG32(LCD_RGB_OFFSET_REG) >> 11) & 0x7ff;
        T_U32 rgb_top = REG32(LCD_RGB_OFFSET_REG) & 0x7ff;
        T_U32 rgb_width = (REG32(LCD_RGB_SIZE_REG) >> 11) & 0x7ff;
        T_U32 rgb_height = (REG32(LCD_RGB_SIZE_REG)) & 0x7ff;

        T_U32 yuv1_left = (REG32(LCD_YUV1_DISPLAY_INFO_REG) >> 11) & 0x7ff;
        T_U32 yuv1_top = REG32(LCD_YUV1_DISPLAY_INFO_REG) & 0x7ff;
        T_U32 yuv1_width = (REG32(LCD_YUV1_H_INFO_REG)>> 11) & 0x7ff;
        T_U32 yuv1_height = (REG32(LCD_YUV1_V_INFO_REG) >> 11) & 0x7ff;

        if (!((yuv1_left >= rgb_left) && (yuv1_top >= rgb_top) 
            && ((yuv1_left + yuv1_width) <= (rgb_left + rgb_width)) 
            && ((yuv1_top + yuv1_height) <= (rgb_top + rgb_height))))
        {
            akprintf(C2, M_DRVSYS, "yuv1 not in rgb\n");
            return;
        }
    }

    DrvModule_Protect(DRV_MODULE_LCD);
    status = m_lcd_param.YuvOffStatus;
    if((1 << 0) == (status & (1 << 0)))
    {
        INTR_DISABLE(IRQ_MASK_LCD_BIT);

        status &= (3 << 1);
        REG32(LCD_TOP_CONFIGURE_REG) |= status;
        m_lcd_param.YuvOffStatus= 0x0;//clear status

        if(AK_TRUE == m_lcd_param.bYuv1FullFlag)
        {
            REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);//enable RGB channe
        }
        else
        {
            m_lcd_param.bYuv1FullFlag = AK_FALSE;

            if(AK_TRUE == m_lcd_param.bRgbFlag)
                REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//disable rgb channel
        }
        
        if(E_LCD_TYPE_MPU == m_lcd_param.LcdType)
        {
            REG32(LCD_LCD_GO_REG) |= (MPU_REFLASH_START);
            check_dma_finish();
            m_lcd_param.bDMATransfering = AK_FALSE;
        }

        enable_alert_valid_int();//inform next frame is ready
        INTR_ENABLE(IRQ_MASK_LCD_BIT);
    }

    DrvModule_UnProtect(DRV_MODULE_LCD);
}

/**
 * @brief close YUV channel
 * @author LianGenhui
 * @date 2010-06-18
 * @return T_VOID
 */
T_VOID lcd_YUV_off (T_VOID)
{
    T_U32 status;
    T_U32 lcd = LCD_0;

    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh RGB error, lcd is not turnon\n");
        return;
    }

    DrvModule_Protect(DRV_MODULE_LCD);
    status = REG32(LCD_TOP_CONFIGURE_REG);//get low 8 bit
    status &= (3 << 1);//get bit[2:1],yuv1 and yuv2
    status |= (1 << 0);//bit 0 for YUV off flag
    m_lcd_param.YuvOffStatus = status;

    INTR_DISABLE(IRQ_MASK_LCD_BIT);
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(3 << 1);//turn off YUV1 and YUV2 channel
    //#if 0//in case of yuv fullscreen
    REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//enable RGB channe
    //#endif
    
    if(AK_TRUE == m_lcd_param.bRgbFlag)
        REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//enable RGB channe

    if(E_LCD_TYPE_MPU == m_lcd_param.LcdType)
    {
        REG32(LCD_LCD_GO_REG) |= (MPU_REFLASH_START);//off display
        check_dma_finish();
        m_lcd_param.bDMATransfering = AK_FALSE;
    }

    enable_alert_valid_int();//inform next frame is ready
    INTR_ENABLE(IRQ_MASK_LCD_BIT);

    DrvModule_UnProtect(DRV_MODULE_LCD);
}

T_BOOL lcd_set_YUV2AlhpaLvl (T_U32 Level)
{
    if(Level > MAX_ALPHA_LEVEL)
    {
        akprintf(C3, M_DRVSYS, "yuv2 alpha level bigger than %d",MAX_ALPHA_LEVEL);
        return AK_FALSE;
    }

    m_lcd_param.Yuv2AlphaLvl = Level;
    return AK_TRUE;
}

T_U32 lcd_get_YUV2AlhpaCurLvl(T_VOID)
{
    return m_lcd_param.Yuv2AlphaLvl;
}

/**
 * @brief display OSD channel to LCD
 * @author LianGenhui
 * @date 2010-06-18
 * @param lcd LCD_0 or LCD1
 * @param[in] left, top position of display area
 * @param[in] width,height size of display area
 * @param[in] alpha value of alpha blending opration. value between 0 to 8
 * @param[in] dsp_buf display buffer
 * @return T_BOOL
 * @retval  AK_TRUE  refresh OSD channel successful
 * @retval  AK_FALSE refresh OSD channel failed
 * @note return failed:\n
 *    the pixels of line more than 256\n
 *    OSD picture covered the YUV2 picture\n
 *    OSD picture and YUV2 picture are display in the same line
 */
T_BOOL lcd_osd_display_on (T_eLCD lcd, T_RECT *dsp_rect, T_U8 alpha, T_U8 *dsp_buf)
{
    unsigned int status;
    T_U8  temp = 0;
    
    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh RGB error, lcd is not turnon\n");
        return AK_FALSE;
    }

    if(alpha > MAX_ALPHA_LEVEL)
    {
        akprintf(C2, M_DRVSYS, "osd alpha level bigger than %d",MAX_ALPHA_LEVEL);
        return AK_FALSE;
    }

    //if yuv2 channel is opened,osd can't cover up yuv2 and can't display in the same line 
    if (YUV2_CHANNEL_EN == (REG32(LCD_TOP_CONFIGURE_REG) & YUV2_CHANNEL_EN))
    {
        T_U32 yuv2_left = (REG32(LCD_YUV2_DISPLAY_INFO_REG) >> 11) & 0x7ff;
        T_U32 yuv2_top = REG32(LCD_YUV2_DISPLAY_INFO_REG) & 0x7ff;
        T_U32 yuv2_width = (REG32(LCD_YUV2_H_INFO_REG)>> 11) & 0x7ff;
        T_U32 yuv2_height = (REG32(LCD_YUV2_V_INFO_REG) >> 11) & 0x7ff;

        if (!((dsp_rect->left >= yuv2_left + yuv2_width) || (dsp_rect->top >= yuv2_top + yuv2_height) 
          || ((dsp_rect->left + dsp_rect->width) <= yuv2_left) || ((dsp_rect->top + dsp_rect->height) <= yuv2_top)))
        {
            akprintf(C2, M_DRVSYS, "osd not cover up yuv2\n");
            return AK_FALSE;
        }

        if (!((dsp_rect->top >= yuv2_top + yuv2_height) 
          || ((dsp_rect->top + dsp_rect->height) <= yuv2_top)) )
        {
            akprintf(C2, M_DRVSYS, "osd and yuv2 can't display in the same line\n"); 
            return AK_FALSE;
        }
    }


    DrvModule_Protect(DRV_MODULE_LCD);

    if (AK_TRUE == m_lcd_param.bDMATransfering)
    {
        check_dma_finish();
        m_lcd_param.bDMATransfering = AK_FALSE;
    }

    MMU_InvalidateDCache();

    /*transfer data*/
    if(E_LCD_TYPE_MPU != m_lcd_param.LcdType)
    {
        INTR_DISABLE(IRQ_MASK_LCD_BIT);

        REG32(LCD_OSD_ADDR_REG) = (T_U32)(dsp_buf);
        REG32(LCD_OSD_OFFSET_REG) = ((dsp_rect->left << 11) | dsp_rect->top);
        REG32(LCD_OSD_SIZE_ALPHA_REG) = (dsp_rect->width | (dsp_rect->height << 11) | (alpha << 22));
        REG32(LCD_TOP_CONFIGURE_REG) |= OSD_CHANNEL_EN;   

        enable_alert_valid_int();//inform next frame is ready
        INTR_ENABLE(IRQ_MASK_LCD_BIT);
    }
    else
    {
        REG32(LCD_OSD_ADDR_REG) = (T_U32)(dsp_buf);

        //set osd offset again
        REG32(LCD_OSD_OFFSET_REG) = ((dsp_rect->left << 11) | dsp_rect->top);

        REG32(LCD_OSD_SIZE_ALPHA_REG) = (dsp_rect->width | (dsp_rect->height << 11)| (alpha << 22));

        //panel size for mpu lcd
        REG32(LCD_PANEL_SIZE_REG) = ((dsp_rect->width << 11) | dsp_rect->height);

        REG32(LCD_TOP_CONFIGURE_REG) |= OSD_CHANNEL_EN;
    }

    DrvModule_UnProtect(DRV_MODULE_LCD);
    return AK_TRUE;
}

/**
 * @brief turn off OSD
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] lcd LCD_0 or LCD1
 * @return T_VOID
 */
T_VOID lcd_osd_display_off(T_eLCD lcd)
{
    if (AK_FALSE == m_lcd_param.bLcd_OnOff[lcd])
    {
        akprintf(C1, M_DRVSYS, "refresh RGB error, lcd is not turnon\n");
        return;
    }

    DrvModule_Protect(DRV_MODULE_LCD);
    REG32(LCD_TOP_CONFIGURE_REG) &= (~OSD_CHANNEL_EN);
    DrvModule_UnProtect(DRV_MODULE_LCD);
}

/**
 * @brief Set OSD color palette
 * @author LianGenhui
 * @date 2010-06-18
 * @param[in] color color palette,15 kind of colors in this palette,
 * @one color size on two bytes
 * @return T_VOID
 */
T_VOID lcd_osd_set_color_palette(T_U16 *color)
{
    REG32(LCD_OSD_F_COLOR1_REG) = (color[0]<<16) | color[1];
    REG32(LCD_OSD_F_COLOR2_REG) = (color[2]<<16) | color[3];
    REG32(LCD_OSD_F_COLOR3_REG) = (color[4]<<16) | color[5];
    REG32(LCD_OSD_F_COLOR4_REG) = (color[6]<<16) | color[7];
    REG32(LCD_OSD_F_COLOR5_REG) = (color[8]<<16) | color[9];
    REG32(LCD_OSD_F_COLOR6_REG) = (color[10]<<16) | color[11];
    REG32(LCD_OSD_F_COLOR7_REG) = (color[12]<<16) | color[13];
    REG32(LCD_OSD_F_COLOR8_REG) = color[14];
}


/**
 * @brief     open TV-out function
 * @author     LianGenhui
 * @date       2010-06-18
 * @param[in]      type TV_OUT_TYPE_PAL or TV_OUT_TYPE_NTSC
 * @return     T_BOOL
 * @retval  AK_TRUE  open TV out successful
 * @retval  AK_FALSE open TV out failed
 * @note reutrn failed:type not TV_OUT_TYPE_PAL or TV_OUT_TYPE_NTSC
 */
T_BOOL lcd_tv_out_open (E_TV_OUT_TYPE type)
{
    if ((AK_FALSE == m_lcd_param.bLcdInit[LCD_0])
        && (AK_FALSE == m_lcd_param.bLcdInit[LCD_1]))
    {
        m_lcd_parameter_init();
    }
    
    if ((E_LCD_TYPE_TVOUT != m_lcd_param.LcdType) || (m_lcd_param.TvoutType != type))
    {
        lcd_turn_off(LCD_0);
        //delay for LCD to TVOUT
        mini_delay(200);
        m_lcd_param.TvoutType = type;

        m_lcd_param.YuvOffStatus = 0;
        m_lcd_param.bYuv1FullFlag = AK_FALSE;

        if (FREQ_INVALID_HANDLE == m_lcd_param.freq_handle)
        {
            m_lcd_param.freq_handle = FreqMgr_RequestFreq(FREQ_MIN_TVOUT);
        }
        
        lcd_tv_out_Init(type);

        lcd_turn_on(LCD_0);
    }
    return AK_TRUE;
}

/**
 * @brief     close TV-out function
 * @author     LianGenhui
 * @date       2010-06-18
 * @return     T_VOID
 */
T_VOID lcd_tv_out_close(T_VOID)
{
    if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)
    {
        lcd_turn_off(LCD_0);

        FreqMgr_CancelFreq(m_lcd_param.freq_handle);
        m_lcd_param.freq_handle = FREQ_INVALID_HANDLE;

        //delay for TVOUT to LCD
        mini_delay(200);
        lcd_initial();
        lcd_turn_on(LCD_0);
    }
}

/**
 * @BRIEF set lcd mode, config MPU interface
 * @AUTHOR LianGenhui    
 * @DATE 2010-06-18
 */
static T_VOID lcd_set_mode( T_VOID )
{
    T_U32 mode = 0;
    T_U32 A0_polarity = 1;     
    T_U32 IF_Sel;
    T_U16 rgb_or_bgr;
    T_U32 Disply_Color_Sel;
    T_U32 Bus_Sel=0;
    T_U32 lcd = LCD_0;
    T_U32 pll_clk;
    T_U32 pclk;
    T_U32 div;
    T_U32 div_pclk = 4;
    T_U32 W_Len1 = 6;
    T_U32 W_Len2 = 2;
    T_U32 div_tmp;    
    T_U32 i;
    T_U32 j,regval;

    //get interface mode
    IF_Sel = m_lcd_param.pLcdHandler[lcd]->lcd_type;

    // get Disply Color param
    Disply_Color_Sel = m_lcd_param.pLcdHandler[lcd]->lcd_color_sel;

    // get Bus width param 
    if (m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth == 8)
        Bus_Sel = 0;    //8bit
    else if (m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth == 16)
        Bus_Sel = 1;    //16bit
    else if (m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth == 18)
        Bus_Sel = 2;    //18bit
    else if (m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth == 9)
        Bus_Sel = 3;    //9bit
    else
        akprintf(C2, M_DRVSYS, "lcd bus width setting error:%d\n", m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth);

    //get pll clk and lcd pclk
    pll_clk = get_pll_value()*1000000;
    pclk = m_lcd_param.pLcdHandler[lcd]->lcd_PClk_Hz;

    div = (pll_clk/2)/pclk;
    div_tmp = 100;

    //calculate the pclk and cycle len from pll clk and pclk
    if((div < 6) || (div > 100))
    {
        akprintf(C3, M_DRVSYS, "lcd timing param is out of range, use default\n"); 
        
        if((8 == m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth) ||
           (9 == m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth))
        {
            div = 2;    //8 and 9 bit interface
            W_Len1 = 0x16;
            W_Len2 = 0x04;
        }
        else
        {
            div = 2;//16bit and 18 bit
            W_Len1 = 0x08;
            W_Len2 = 0x04;
        }
    }
    else
    {
        for (i=1; i<7; i++)
        {
            for(j=6; j<16;j++)
            {
                if((i *j < div_tmp) && (i*j) > div)
                {
                    div_tmp = i*j;
                    div_pclk = i;
                    W_Len1 = j;
                }
            }
        }
        if((8 == m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth) ||
           (9 == m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth))
        {
            W_Len2 = (W_Len1 >> 1) - 2;//8 and 9 bit interface
        }
        else
        {
            W_Len2 = W_Len1 - 2;        //16bit and 18 bit
        }
    }

    // config pclk value
    REG32(LCD_CLK_CTL_REG) = (1<<8) |(div_pclk<<1) | (1<<0);

    W_Len1 &= 0x7f;
    W_Len2 &= 0x1f;
    
#if 1     
    regval = REG32(LCD_TOP_CONFIGURE_REG);
    
    regval &= ~(3 << 5);
    regval |= (IF_Sel << 5);//lcd interface select
    
    regval &= ~(1 << 14);    //24bits(RGB888)
    
    regval |= (1<<13);     //RGB order
    
    regval &= ~(1 << 12);   //0, RGB
    
    regval &= ~(0xff << 15);
    regval |= (0xa8 << 15); //alarm full
    
    regval &= ~(0xffUL << 24);
    regval |= (0x10 << 24); //alarm empty
    
    REG32(LCD_TOP_CONFIGURE_REG) = regval;
#else    //使用下面这种当时导致需要配置两次，原因暂时不明
    /* config interface: MPU, RGB or TV-out */
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(3 << 5);
    REG32(LCD_TOP_CONFIGURE_REG) |= (IF_Sel << 5);//lcd interface select

    //input data format:16bits or 24bits
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(1 << 14);    //24bits(RGB888)

    //input RGB data sequence:RGB or BGR
    REG32(LCD_TOP_CONFIGURE_REG) |= (1<<13);     //RGB order

    //output sequence:RGB or BGR
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(1 << 12);   //0, RGB

    REG32(LCD_TOP_CONFIGURE_REG) &= ~(0xff << 15);
    REG32(LCD_TOP_CONFIGURE_REG) |= (0xa8 << 15); //alarm full

    REG32(LCD_TOP_CONFIGURE_REG) &= ~(0xffUL << 24);
    REG32(LCD_TOP_CONFIGURE_REG) |= (0x10 << 24); //alarm empty
    
#endif
    /* config bus width, color and write cycle */
    mode = 0;
    mode |=  (Bus_Sel << 14);
    mode |=  (Disply_Color_Sel << 13);
    mode |=  (W_Len1 << 6);
    mode |=  (W_Len2 << 1);
    mode |=  A0_polarity;
    REG32(LCD_MPU_CTL_REG) = mode;
}

static T_VOID switch_bus_width (T_U32 buswidth)
{
    T_U32 mode=0;

    mode = REG32(LCD_MPU_CTL_REG) ;    
    mode &= ~(3<<14);
    
    if (8 == buswidth)
        mode |= (0<<14);
    else if(16 == buswidth)
        mode |= (1<<14);
    else 
        mode |= (3<<14);
        
    REG32(LCD_MPU_CTL_REG) = mode ;
}

/**
 * @brief     send command or data to the lcd device(for MPU)
 * @author    LianGenhui
 * @date      2010-06-18
 * @param[in]     lcd select the lcd, LCD_0 or LCD_1
 * @param[in]     ctl send command or data, refer to T_eLCD_MPU_CTL definition
 * @param     data the data or command want to send
 * @return    T_VOID
 */
T_VOID lcd_MPU_ctl (T_eLCD lcd, T_eLCD_MPU_CTL ctl, T_U32 data)
{
    T_U32 i=0;    
    T_U8 del;
    
    if (LCD_0 == lcd)
    {
        if (LCD_MPU_CMD== ctl)
        {
            //8位接口时必须设置16位接口
            if(8 == m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth)
            {
                switch_bus_width(16);                
                us_delay(1);	//解决高频白屏
			}
			
            REG32(LCD_REG_CONFIG_REG) = MAIN_LCD_MPU_CMD | data;
            
            if(8 == m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth)
            {                
                us_delay(1);	//解决高频白屏
                switch_bus_width(8);
            }
       }
        else if (LCD_MPU_DATA== ctl)
        {
            if(8 == m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth)
            {
                switch_bus_width(16);                
                us_delay(1);	//解决高频白屏
            }

            REG32(LCD_REG_CONFIG_REG) = MAIN_LCD_MPU_DATA | data;
            
            if(8 == m_lcd_param.pLcdHandler[lcd]->lcd_BusWidth)
            {            
                us_delay(1);	//解决高频白屏
                switch_bus_width(8);
            }
       }
    }
    else if (LCD_1 == lcd)
    {
        if (LCD_MPU_CMD == ctl)
        {
            REG32(LCD_REG_CONFIG_REG) = SUB_LCD_MPU_CMD | data;
        }
        else if (LCD_MPU_DATA == ctl)
        {
            REG32(LCD_REG_CONFIG_REG) = SUB_LCD_MPU_DATA | data;
        }
    }
}

T_U32 lcd_readback(T_eLCD lcd)
{
    T_U32 tmp = 0;
    T_U32 i;

    //to be constructed

    return tmp;
}

static T_VOID lcd_rgb_open_clk(T_VOID)
{
    T_U32 div;
    T_U32 PLLClk,Pclk;

    //get pll value and pclk value, calc pclk div
    PLLClk = get_pll_value()*1000000;
    Pclk = m_lcd_param.pLcdHandler[0]->lcd_PClk_Hz;

    div = (PLLClk/2)/Pclk;
    //make sure the exact Pclk not exceed the desired Pclk
    if ((PLLClk/2/div) <= Pclk)
        div -= 1;

    div &= 0x7f;
    //config lcd pclk value
    REG32(LCD_CLK_CTL_REG) = (1<<8) |(div<<1) | (1<<0);
}

/**
 * @brief set rgb channel input format
              only support in aspen3s
 * @author Liaozhijun
 * @param[in]  bRgb565 AK_TRUE: rgb565, AK_FALSE: rgb888
 * @param[in]  bRgbSeq AK_TRUE: rgb, AK_FALSE: bgr
 * @date   2010-12-20
 * @return T_VOID
 */
T_VOID lcd_set_rgb_data_format(T_BOOL bRgb565, T_BOOL bRgbSeq)
{
    if ((m_lcd_param.bRgb565 != bRgb565) || (m_lcd_param.bRgbSeq != bRgbSeq))
    {
        m_lcd_param.bRgb565 = bRgb565;
        m_lcd_param.bRgbSeq = bRgbSeq;
        
        DrvModule_Protect(DRV_MODULE_LCD);
        
        if((E_LCD_TYPE_MPU != m_lcd_param.LcdType) && m_lcd_param.bRgbFlag)
        {
            store_all_int();

            if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)
            {
                //disable alert valid interrupt
                REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 17);
                //disable system error interrupt
                REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 0);
                //disable empty alarm interrupt
                REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 18);
                
                REG32(LCD_LCD_GO_REG) |= (LCD_GO_SYS_STOP);
            }
            else if (E_LCD_TYPE_RGB == m_lcd_param.LcdType)
            {                
                //turn off brightness
                lcdbl_set_brightness(LCD_0,0);
                                
                //lcd turn off
                lcd_rgb_turn_off();

            }
            
            //input data format:16bits or 24bits
            if (m_lcd_param.bRgb565)
                REG32(LCD_TOP_CONFIGURE_REG) &= ~(1<<14);       //16bits
            else
                REG32(LCD_TOP_CONFIGURE_REG) |= (1<<14);        //24bits

            //input data format: rgb or bgr
            if (m_lcd_param.bRgbSeq)
                REG32(LCD_TOP_CONFIGURE_REG) |= (1<<13);       //rgb
            else
                REG32(LCD_TOP_CONFIGURE_REG) &= ~(1<<13);      //bgr

            
            //rgb go
            if (E_LCD_TYPE_TVOUT == m_lcd_param.LcdType)
            {
                REG32(LCD_LCD_GO_REG) |= (LCD_GO_TV);
            }
            else if (E_LCD_TYPE_RGB == m_lcd_param.LcdType)
            {
                //turn on
                lcd_rgb_turn_on();

                //turn on brightness
                lcdbl_set_brightness(LCD_0, m_lcd_param.CurBrightness[LCD_0]);
            }
            
            restore_all_int();
            
        }
        else
        {
            check_dma_finish();

            //input data format:16bits or 24bits
            if (m_lcd_param.bRgb565)
                REG32(LCD_TOP_CONFIGURE_REG) &= ~(1<<14);       //16bits
            else
                REG32(LCD_TOP_CONFIGURE_REG) |= (1<<14);        //24bits

            //input data format: rgb or bgr
            if (m_lcd_param.bRgbSeq)
                REG32(LCD_TOP_CONFIGURE_REG) |= (1<<13);       //rgb
            else
                REG32(LCD_TOP_CONFIGURE_REG) &= ~(1<<13);      //bgr
        }
        
        DrvModule_UnProtect(DRV_MODULE_LCD);
    }
}

static T_VOID lcd_rgb_HardInit(const T_RGBLCD_INFO *pRGBlcd)
{
    T_U32 reg_value = 0;
    //save channel setting
    T_U32 lcd_channel=REG32(LCD_TOP_CONFIGURE_REG)&0xf;

    //for AK9801, lcd bus width is 24bit
    gpio_pin_group_cfg(ePIN_AS_LCD_RGB);

    //reset lcd interface
    sysctl_reset(RESET_LCD);

    //config interface: RGB
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(3 << 5);
    REG32(LCD_TOP_CONFIGURE_REG) |= (2 << 5);

    //the full status use max value
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(0xff << 15);
    REG32(LCD_TOP_CONFIGURE_REG) |= (0xff << 15);
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(0xffUL << 24);
    REG32(LCD_TOP_CONFIGURE_REG) |= (0x50 << 24);//set empty alarm

    //output sequence:RGB or BGR
    if (pRGBlcd->RGBorGBR == 1)
        REG32(LCD_TOP_CONFIGURE_REG) |= 1<<12;       // 1, BGR
    else
        REG32(LCD_TOP_CONFIGURE_REG) &= ~(1 << 12);  // 0, RGB

    //Pclk polarity
    if (pRGBlcd->PHVG_POL & 0x8)
        REG32(LCD_TOP_CONFIGURE_REG) |= 1<<4;        // 1, positive
    else
        REG32(LCD_TOP_CONFIGURE_REG) &= ~(1 << 4);   // 0, negative

    //input data format:16bits or 24bits
    REG32(LCD_TOP_CONFIGURE_REG) |= (1<<14);         //24bits
    //input RGB data sequence:RGB or BGR
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(1<<13);        //RGB order

    //hsyn,vsyn,gate polarity
    REG32(LCD_RGB_CTL_REG1) |= pRGBlcd->PHVG_POL & 0x7;

    //rgb data bus width
    REG32(LCD_RGB_CTL_REG1) &= ~(3<<21);
    if (m_lcd_param.pLcdHandler[0]->lcd_BusWidth == 24)      //24bit
        REG32(LCD_RGB_CTL_REG1) |= (3<<21); 
    else if (m_lcd_param.pLcdHandler[0]->lcd_BusWidth == 18) //18bit
        REG32(LCD_RGB_CTL_REG1) |= (1<<21);
    else if (m_lcd_param.pLcdHandler[0]->lcd_BusWidth == 8)  //8bit
        REG32(LCD_RGB_CTL_REG1) |= (0<<21);
    else//default use 18 bit
        REG32(LCD_RGB_CTL_REG1) |= (1<<21);

    //interlace
    if (pRGBlcd->isInterlace)
        REG32(LCD_RGB_CTL_REG1) &= ~(1<<20);
    else
        REG32(LCD_RGB_CTL_REG1) |= (1<<20);

    //time setup
    //thp and tvp
    REG32(LCD_RGB_CTL_REG3) = (pRGBlcd->Thpw << 12) | pRGBlcd->Tvpw;
    //thb and thd
    REG32(LCD_RGB_CTL_REG4) = (pRGBlcd->Thb << 12) | pRGBlcd->Thd;
    //thf and thlen
    REG32(LCD_RGB_CTL_REG5) = (pRGBlcd->Thf << 13) | pRGBlcd->Thlen;
    //tvb
    REG32(LCD_RGB_CTL_REG6) = (pRGBlcd->Tvb);
    //tvf
    REG32(LCD_RGB_CTL_REG7) = (pRGBlcd->Tvf << 12);
    //tvd
    REG32(LCD_RGB_CTL_REG8) = (pRGBlcd->Tvd << 15);
    //tvlen
    REG32(LCD_RGB_CTL_REG9) = (pRGBlcd->Tvlen);
    //pannel size
    REG32(LCD_PANEL_SIZE_REG) = ((pRGBlcd->Thd << 11)|pRGBlcd->Tvd);

    //open lcd pclk
    lcd_rgb_open_clk();
    if (m_lcd_param.pLcdHandler[0]->lcd_init_func != AK_NULL)
    {
        m_lcd_param.pLcdHandler[0]->lcd_init_func(0);
    }

    //backgrond color
    lcd_set_BgColor(m_lcd_param.CurBgColor);

    //set RGB_CS to low
    REG32(LCD_SOFTWARE_CTL_REG) &= ~(1<<18);

    /*Resume lcd channel setting*/
    restore_channel_setting(lcd_channel);

    int_register_irq(INT_VECTOR_LCD, lcd_intr_handler);

    //set alert line number
    //ATTENTION: alert line is [0, lcd_height-1]
    reg_value = REG32(LCD_SOFTWARE_CTL_REG);
    reg_value &= ~0x7FF;
    reg_value |= ((pRGBlcd->Tvd & 0x7FF)-2) | (1<<11);
    REG32(LCD_SOFTWARE_CTL_REG) = reg_value;
    REG32(LCD_LCD_INTERRUPT_MASK) |= 1;//enable system error interrupt
}

static T_VOID lcd_rgb_turn_on(T_VOID)
{    
    //rgb go
    REG32(LCD_LCD_GO_REG) |= (LCD_GO_RGB);    
}

static T_VOID lcd_rgb_turn_off(T_VOID)
{
    T_U32 reg_data, reg_buf;
    T_U32 t1,t2;
    
    reg_buf = REG32(LCD_RGB_CTL_REG2);
    
    //disable alert valid interrupt
    REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 17);
    //disable system error interrupt
    REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 0);
    //disable empty alarm interrupt
    REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 18);

    //set RGB_CS to low
    REG32(LCD_SOFTWARE_CTL_REG) &= ~(1<<18);

    reg_data = REG32(LCD_LCD_STATUS);
    
    t1 = get_tick_count();
    while ((reg_data & (1<<17)) != (1<<17))
    {
        reg_data = REG32(LCD_LCD_STATUS);
        
        t2 = get_tick_count();
        if (t2 > t1)
        {
            if ((t2 - t1) > 50) //50 ms
            {
                akprintf(C2, M_DRVSYS, "lcd_rgb_turn_off time out\n");
                break;
            }
        }
        else
        {
            t1 = t2;
        }
    }

    //correctly close lcd,and reset reg refer to RGB 
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(RGB_CHANNEL_EN);//disable rgb channel
    mini_delay(20);     //wait a frame interval and stop again
    REG32(LCD_LCD_GO_REG) |= (LCD_GO_SYS_STOP);

    //lcd init
    lcd_rgb_HardInit(m_lcd_param.pRgblcdPointer);

    REG32(LCD_TOP_CONFIGURE_REG) |= (RGB_CHANNEL_EN);//enable rgb channel
    REG32(LCD_RGB_CTL_REG2) = reg_buf;
}


//**************************TV_OUT interface***************************
static T_VOID lcd_tv_out_Init(E_TV_OUT_TYPE type)
{
    m_lcd_param.LcdType = E_LCD_TYPE_TVOUT;
    m_lcd_param.TvoutType = type;
    m_lcd_param.bLcdInit[LCD_0] = AK_TRUE;
    m_lcd_param.bLcd_OnOff[LCD_0] = AK_FALSE;
    m_lcd_param.bDMATransfering = AK_FALSE;
}

static T_VOID tv_out_reg_config(E_TV_OUT_TYPE type)
{
    //PAL or NTSC mode config
    if (TV_OUT_TYPE_PAL == type)
    {
        REG32(TVOUT_PARA_CONFIG_REG6) &= ~(0xff);
        REG32(TVOUT_PARA_CONFIG_REG6) |= 89;

        REG32(TVOUT_PARA_CONFIG_REG3) &= ~(0x3ff<<18);
        REG32(TVOUT_PARA_CONFIG_REG3) |= 625<<18;

        REG32(LCD_CHROMA_FRQ_CTR_REG) = 0x2A098ACB;

        REG32(TVOUT_PARA_CONFIG_REG1) &= ~(0xff);
        REG32(TVOUT_PARA_CONFIG_REG1) |= 138;

        REG32(TVOUT_PARA_CONFIG_REG5) = (1440<<0)|(24<<24)|(21<<16);

        REG32(TVOUT_PARA_CONFIG_REG2) = (0xE0<<18) | (0x12C<<8) | (44 << 0);

        REG32(TVOUT_PARA_CONFIG_REG3) &= ~(0xff);
        REG32(TVOUT_PARA_CONFIG_REG3) |= 44;

        REG32(TVOUT_PARA_CONFIG_REG4) &= ~(0xffffff);
        REG32(TVOUT_PARA_CONFIG_REG4) |= 137<<8 | 137;

        //after PAL configuring, set soft_rst
        REG32(TVOUT_CTRL_REG2) &= ~(0x1<<8);
    }
    else if (TV_OUT_TYPE_NTSC == type)
    {
        //for NTSC mode, only need to set soft_rst
        REG32(TVOUT_CTRL_REG2) &= ~(0x1<<8);
        REG32(TVOUT_PARA_CONFIG_REG2) = (0xE0<<18) | (0x12C<<8) | (0x2B << 0);
    }
}

static T_VOID lcd_tv_out_turn_on(T_VOID)
{
    T_U32 reg_data;
    T_U32 wLen,hLen;
    T_U32 TvModeCfg;
    T_U32 reg_value;

    sysctl_clock(CLOCK_LCD_ENABLE);

    //reset lcd interface
    sysctl_reset(RESET_LCD);

    wLen = lcd_get_hardware_width(LCD_0);
    hLen = lcd_get_hardware_height(LCD_0)>>1;

    //config spll, tvclk = spll/25;
    //spll = 25 * (M/N) / (2^D), 
    //now we set M = 108, N = 4, D = 0;
    reg_value = REG32(MUL_FUN_CTL_REG2);
    reg_value &= ~(0x3FF);
    reg_value |= (108 << 0) | (4 << 8);
    REG32(MUL_FUN_CTL_REG2) = reg_value;

    //clear spll bypass and power down
    REG32(MUL_FUN_CTL_REG) &= ~((1<<30)|(1<<6));

    //enable clock
    REG32(MUL_FUN_CTL_REG) &= ~(1<<8);
    //REG32(MUL_FUN_CTL_REG) &= ~(1<<21);    //enter powerdown mode


    // config interface: TV_OUT
    REG32(LCD_TOP_CONFIGURE_REG) &= (~(3<<5));
    REG32(LCD_TOP_CONFIGURE_REG) |= (3<<5); // tv out mode

    REG32(LCD_TOP_CONFIGURE_REG) &= ~(0xff << 15);
    REG32(LCD_TOP_CONFIGURE_REG) |= (0xff << 15);
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(0xffUL << 24);
    REG32(LCD_TOP_CONFIGURE_REG) |= (0x10 << 24);
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(1 << 14);
    REG32(LCD_TOP_CONFIGURE_REG) |= (1 << 13);
    REG32(LCD_TOP_CONFIGURE_REG) &= ~(1 << 12);   //0, RGB

    //pannel size
    REG32(LCD_PANEL_SIZE_REG) = ((wLen << 11) | hLen);

    //backgrond color
    lcd_set_BgColor(0x8080);

    //tv out reg config
    tv_out_reg_config(m_lcd_param.TvoutType);

    REG32(MUL_FUN_CTL_REG) |= (1<<21);    //enter powerdown mode

    //set alert line number
    reg_value = REG32(LCD_SOFTWARE_CTL_REG);
    reg_value &= ~0x7FF;
    reg_value |= ((hLen & 0x7FF) - 2) | (1<<11);
    REG32(LCD_SOFTWARE_CTL_REG) = reg_value;

    REG32(LCD_LCD_GO_REG) = (LCD_GO_TV);   //REFRESH

    REG32(MUL_FUN_CTL_REG) &= ~(1<<21);    //exit powerdown mode
    int_register_irq(INT_VECTOR_LCD, lcd_intr_handler);

    REG32(LCD_LCD_INTERRUPT_MASK) |= 1;//enable system error interrupt
}

static T_VOID lcd_tv_out_turn_off(T_VOID)
{
    //disable alert valid interrupt
    REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 17);
    //disable system error interrupt
    REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 0);
    //disable empty alarm interrupt
    REG32(LCD_LCD_INTERRUPT_MASK) &= ~(1 << 18);

    // tv_out stop
    REG32(LCD_LCD_GO_REG) &= ~(LCD_GO_TV);
    REG32(LCD_LCD_GO_REG) |= (LCD_GO_SYS_STOP);

    // tv out power down
    REG32(MUL_FUN_CTL_REG) |= (1<<21);
    // close tv out pclk
    REG32(MUL_FUN_CTL_REG) &= ~(1<<28);
}

T_VOID store_pclk_state()
{
    if((E_LCD_TYPE_RGB != m_lcd_param.LcdType) || (AK_FALSE == m_lcd_param.bLcd_OnOff[0]))
        return;

    //wait current frame finish
    while((REG32(LCD_LCD_STATUS) & ALERT_VALID_STAT) != ALERT_VALID_STAT)
    {
    }

    //stop refresh
    REG32(LCD_LCD_GO_REG) |= (LCD_GO_SYS_STOP);

    //wait one frame time
    mini_delay(50);
    REG32(LCD_LCD_GO_REG) |= (LCD_GO_SYS_STOP);

    //disable interrupt during changing pll
    store_all_int();
}

T_VOID restore_pclk_state()
{
    if((E_LCD_TYPE_RGB != m_lcd_param.LcdType) || (AK_FALSE == m_lcd_param.bLcd_OnOff[0]))
        return;

    //enable interrupt
    restore_all_int();

    //refresh go
    REG32(LCD_LCD_GO_REG) |= (LCD_GO_RGB);
}
