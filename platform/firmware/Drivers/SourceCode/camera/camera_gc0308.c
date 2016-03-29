/**
 * @file camera_gc0308.c
 * @brief camera driver file
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2011-09-21
 * @version 1.0
 * @ref
 */ 
#include "akdefine.h"
#include "platform_hd_config.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "drv_camera.h"
#include "camera_gc0308.h"
#include "gpio_config.h"
#include "drv_sccb.h"


#ifdef USE_CAMERA_GC0308

#define CAM_EN_LEVEL            0    
#define CAM_RESET_LEVEL         0
   
#define CAMERA_SCCB_ADDR        0x42
#define CAMERA_GC0308_ID        0x9b

#define GC0308_CAMERA_MCLK      24 //28

#define FREQ_REQUEST_ASIC       30000000
static T_hFreq freq_handle = FREQ_INVALID_HANDLE;

static T_CAMERA_TYPE camera_gc0308_type = CAMERA_P3M;
static T_NIGHT_MODE night_mode = CAMERA_DAY_MODE;
static T_CAMERA_MODE s_gc0308_CurMode = CAMERA_MODE_VGA;


/*
static T_VOID camera_setbit(T_U8 reg, T_U8 bit, T_U8 value)
{
    T_U8 tmp;

    tmp = sccb_read_data(CAMERA_SCCB_ADDR, reg);
    if (value == 1)
        tmp |= 0x1 << bit;
    else
        tmp &= ~(0x1 << bit);
    sccb_write_data(CAMERA_SCCB_ADDR, reg, &tmp, 1);
}
*/

static T_BOOL camera_set_param(const T_U8 tabParameter[])
{ 
    int i = 0;
    //T_U8 temp_value; 

    while (1)
    {
        if ((END_FLAG == tabParameter[i]) && (END_FLAG == tabParameter[i + 1])) 
        {
            break;
        }
        else if (DELAY_FLAG == tabParameter[i])
        {
            mini_delay(tabParameter[i + 1]);
        }
        else
        {
            sccb_write_data(CAMERA_SCCB_ADDR, tabParameter[i], (T_U8 *)(&tabParameter[i + 1]), 1);          
            /*if (!((tabParameter[i] == 0x0e) && (tabParameter[i + 1] & 0x02))
                && !((tabParameter[i] == 0x10) && (tabParameter[i + 1] & 0x26))
                && !((tabParameter[i] == 0x14) && (tabParameter[i + 1] & 0x10))
                && !((tabParameter[i] == 0x17) && (tabParameter[i + 1] & 0x01))
                && !((tabParameter[i] == 0x66) && (tabParameter[i + 1] & 0xe8))
                && !((tabParameter[i] == 0x68) && (tabParameter[i + 1] & 0xa2)))
            {                
                temp_value = sccb_read_data(CAMERA_SCCB_ADDR, tabParameter[i]);
                if (temp_value != tabParameter[i + 1])
                {
                    akprintf(C1, M_DRVSYS, "set parameter error!\n");
                    akprintf(C1, M_DRVSYS, "reg 0x%02x write data is 0x%02x, read data is 0x%02x!\n", tabParameter[i], tabParameter[i + 1], temp_value);

                    return AK_FALSE;
                }
            }*/
        }
        
        i += 2;
    }

    return AK_TRUE;
}

static T_VOID camera_setup(const T_U8 tabParameter[])
{
    int i = 0;

    while (1)
    {
        if ((END_FLAG == tabParameter[i]) && (END_FLAG == tabParameter[i + 1])) 
        {
            break;
        }
        else if (DELAY_FLAG == tabParameter[i])
        {
            mini_delay(tabParameter[i + 1]);
        }
        else
        {
            sccb_write_data(CAMERA_SCCB_ADDR, tabParameter[i], (T_U8 *)&tabParameter[i + 1], 1);
        }
        i += 2;
    }
}

static T_VOID cam_gc0308_open(T_VOID)
{  
    if (FREQ_INVALID_HANDLE == freq_handle)
    {
        freq_handle = FreqMgr_RequestFreq(FREQ_REQUEST_ASIC);
    }
        
    gpio_set_pin_dir(GPIO_CAMERA_AVDD, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_AVDD, gpio_pin_get_ActiveLevel(GPIO_CAMERA_AVDD));   

	
	gpio_set_pin_as_gpio(GPIO_CAMERA_CHIP_ENABLE);
    gpio_set_pin_dir(GPIO_CAMERA_CHIP_ENABLE, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, CAM_EN_LEVEL);    
    mini_delay(10);

	
	gpio_set_pin_as_gpio(GPIO_CAMERA_RESET);
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_CAMERA_RESET, CAM_RESET_LEVEL);
    mini_delay(10);
    gpio_set_pin_level(GPIO_CAMERA_RESET, !CAM_RESET_LEVEL);

    mini_delay(20);
}

static T_BOOL cam_gc0308_close(T_VOID)
{
    //sccb software standby mode
	T_U8 Reg0x1a = 0x2b;
	T_U8 Reg0x25 = 0x00;
	
    sccb_write_data(CAMERA_SCCB_ADDR, 0x1a, &Reg0x1a, 1);
	sccb_write_data(CAMERA_SCCB_ADDR, 0x25, &Reg0x25, 1);

	gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, !CAM_EN_LEVEL);
    gpio_set_pin_level(GPIO_CAMERA_AVDD, !gpio_pin_get_ActiveLevel(GPIO_CAMERA_AVDD));    
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_INPUT);

    gpio_set_pin_dir(GPIO_I2C_SCL, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_I2C_SCL, GPIO_LEVEL_LOW);
    gpio_set_pin_dir(GPIO_I2C_SDA, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_I2C_SDA, GPIO_LEVEL_LOW);

    FreqMgr_CancelFreq(freq_handle);
    freq_handle = FREQ_INVALID_HANDLE;
    
    return AK_TRUE;
}

static T_U32 cam_gc0308_read_id(T_VOID)
{
    T_U8 value = 0x00;
    T_U32 id = 0;

    sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);

    value = sccb_read_data(CAMERA_SCCB_ADDR, 0x00);
    id |= value;   
    
    akprintf(C1, M_DRVSYS, "i2c addr 0x%x, cam_gc0308_read_id = 0x%x\r\n", CAMERA_SCCB_ADDR, id);      
    return id;
}

/**
 * @brief initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author xia_wenting 
 * @date 2011-01-11
 * @return T_BOOL
 * @retval AK_TRUE if success, else AK_FALSE
 */
static T_BOOL cam_gc0308_init()
{
    if (!camera_set_param(INIT_TAB))
    {
        return AK_FALSE;
    }
    else
    {        
        night_mode = CAMERA_DAY_MODE;
        return AK_TRUE;
    }        
}

/**
 * @brief Set camera mode to specify image quality, SXGA/VGA/CIF/ etc 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] mode mode value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc0308_set_mode(T_CAMERA_MODE mode)
{
    s_gc0308_CurMode = mode;
    switch(mode)
    {
        case CAMERA_MODE_VGA:
            camera_setup(VGA_MODE_TAB);
            break;
        case CAMERA_MODE_CIF:
            camera_setup(CIF_MODE_TAB);
            break;
        case CAMERA_MODE_QVGA:
            camera_setup(QVGA_MODE_TAB);
            break;
        case CAMERA_MODE_QCIF:
            camera_setup(QCIF_MODE_TAB);
            break;
        case CAMERA_MODE_QQVGA:
            camera_setup(QQVGA_MODE_TAB);
            break;
        case CAMERA_MODE_PREV:              //preview mode
            camera_setup(PREV_MODE_TAB);
            
            if (CAMERA_NIGHT_MODE == night_mode)
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        case CAMERA_MODE_REC:              //record mode
            camera_setup(RECORD_MODE_TAB);

            if (CAMERA_NIGHT_MODE == night_mode)
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        default:
            s_gc0308_CurMode = CAMERA_MODE_VGA;
            akprintf(C1, M_DRVSYS, "set camera mode parameter error!\n");
            break;
        }
}

/**
 * @brief Set camera exposure mode 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] exposure exposure mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc0308_set_exposure(T_CAMERA_EXPOSURE exposure)
{
    switch(exposure)
    {
        case EXPOSURE_WHOLE:
            camera_setup(EXPOSURE_WHOLE_TAB);
            break;
        case EXPOSURE_CENTER:
            camera_setup(EXPOSURE_CENTER_TAB);
            break;
        case EXPOSURE_MIDDLE:
            camera_setup(EXPOSURE_MIDDLE_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set exposure parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera brightness level 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] brightness brightness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc0308_set_brightness(T_CAMERA_BRIGHTNESS brightness)
{
    switch(brightness)
    {
        case CAMERA_BRIGHTNESS_0:
            camera_setup(BRIGHTNESS_0_TAB);
            break;
        case CAMERA_BRIGHTNESS_1:
            camera_setup(BRIGHTNESS_1_TAB);
            break;
        case CAMERA_BRIGHTNESS_2:
            camera_setup(BRIGHTNESS_2_TAB);
            break;
        case CAMERA_BRIGHTNESS_3:
            camera_setup(BRIGHTNESS_3_TAB);
            break;
        case CAMERA_BRIGHTNESS_4:
            camera_setup(BRIGHTNESS_4_TAB);
            break;
        case CAMERA_BRIGHTNESS_5:
            camera_setup(BRIGHTNESS_5_TAB);
            break;
        case CAMERA_BRIGHTNESS_6:
            camera_setup(BRIGHTNESS_6_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set brightness parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera contrast level 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] contrast contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc0308_set_contrast(T_CAMERA_CONTRAST contrast)
{
    switch(contrast)
    {
        case CAMERA_CONTRAST_1:
            camera_setup(CONTRAST_1_TAB);
            break;
        case CAMERA_CONTRAST_2:
            camera_setup(CONTRAST_2_TAB);
            break;
        case CAMERA_CONTRAST_3:
            camera_setup(CONTRAST_3_TAB);
            break;
        case CAMERA_CONTRAST_4:
            camera_setup(CONTRAST_4_TAB);
            break;
        case CAMERA_CONTRAST_5:
            camera_setup(CONTRAST_5_TAB);
            break;
        case CAMERA_CONTRAST_6:
            camera_setup(CONTRAST_6_TAB);
            break;
        case CAMERA_CONTRAST_7:
            camera_setup(CONTRAST_7_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set contrast parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera saturation level 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] saturation saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc0308_set_saturation(T_CAMERA_SATURATION saturation)
{
    switch(saturation)
    {
        case CAMERA_SATURATION_1:
            camera_setup(SATURATION_1_TAB);
            break;
        case CAMERA_SATURATION_2:
            camera_setup(SATURATION_2_TAB);
            break;
        case CAMERA_SATURATION_3:
            camera_setup(SATURATION_3_TAB);
            break;
        case CAMERA_SATURATION_4:
            camera_setup(SATURATION_4_TAB);
            break;
        case CAMERA_SATURATION_5:
            camera_setup(SATURATION_5_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set saturation parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera sharpness level 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] sharpness sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc0308_set_sharpness(T_CAMERA_SHARPNESS sharpness)
{
    switch(sharpness)
    {
        case CAMERA_SHARPNESS_0:
            camera_setup(SHARPNESS_0_TAB);
            break;
        case CAMERA_SHARPNESS_1:
            camera_setup(SHARPNESS_1_TAB);
            break;
        case CAMERA_SHARPNESS_2:
            camera_setup(SHARPNESS_2_TAB);
            break;
        case CAMERA_SHARPNESS_3:
            camera_setup(SHARPNESS_3_TAB);
            break;
        case CAMERA_SHARPNESS_4:
            camera_setup(SHARPNESS_4_TAB);
            break;
        case CAMERA_SHARPNESS_5:
            camera_setup(SHARPNESS_5_TAB);
            break;
        case CAMERA_SHARPNESS_6:
            camera_setup(SHARPNESS_6_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set sharpness parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera AWB mode 
 * @author xia_wenting 
 * @date 2011-01-11
 * @param[in] awb AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc0308_set_AWB(T_CAMERA_AWB awb)
{
    switch(awb)
    {
        case AWB_AUTO:
            camera_setup(AWB_AUTO_TAB);
            break;
        case AWB_SUNNY:
            camera_setup(AWB_SUNNY_TAB);
            break;
        case AWB_CLOUDY:
            camera_setup(AWB_CLOUDY_TAB);
            break;
        case AWB_OFFICE:
            camera_setup(AWB_OFFICE_TAB);
            break;
        case AWB_HOME:
            camera_setup(AWB_HOME_TAB);
            break;
        case AWB_NIGHT:
            camera_setup(AWB_NIGHT_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set AWB mode parameter error!\n");
            break;
    }
}

/**
 * @brief Set camera mirror mode 
 * @author xia_wenting 
 * @date 2011-03-21
 * @param[in] mirror mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc0308_set_mirror(T_CAMERA_MIRROR mirror)
{
    switch(mirror)
    {
       case CAMERA_MIRROR_V:
            camera_setup(MIRROR_V_TAB);
            break;    
        case CAMERA_MIRROR_H:
            camera_setup(MIRROR_H_TAB);
            break;
        case CAMERA_MIRROR_NORMAL:
            camera_setup(MIRROR_NORMAL_TAB);
            break;    
        case CAMERA_MIRROR_FLIP:
            camera_setup(MIRROR_FLIP_TAB);
            break;                    
        default:
            akprintf(C1, M_DRVSYS, "camera gc0308 set mirror parameter error!\n");
            break;    
    }
}


/**
 * @brief Set camera effect mode 
 * @author xia_wenting 
 * @date 2011-03-21
 * @param[in] effect effect mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_gc0308_set_effect(T_CAMERA_EFFECT effect)
{
    switch(effect)
    {
        case CAMERA_EFFECT_NORMAL:
            camera_setup(EFFECT_NORMAL_TAB);
            break;
        case CAMERA_EFFECT_SEPIA:
            camera_setup(EFFECT_SEPIA_TAB);
            break;
        case CAMERA_EFFECT_ANTIQUE:
            camera_setup(EFFECT_ANTIQUE_TAB);
            break;
        case CAMERA_EFFECT_BLUE:
            camera_setup(EFFECT_BLUISH_TAB);
            break;
        case CAMERA_EFFECT_GREEN:
            camera_setup(EFFECT_GREENISH_TAB);
            break;
        case CAMERA_EFFECT_NEGATIVE:
            camera_setup(EFFECT_NEGATIVE_TAB);
            break;
        case CAMERA_EFFECT_BW:
            camera_setup(EFFECT_BW_TAB);
            break;
        case CAMERA_EFFECT_BWN:
            camera_setup(EFFECT_BWN_TAB);
            break;
        default:
            akprintf(C1, M_DRVSYS, "set camer effect parameter error!\n");
            break;
    }
}

/**
 * @brief set camera window
 * @author xia_wenting  
 * @date 2011-03-22
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_S32
 * @retval 0 if error mode 
 * @retval 1 if success
 * @retval -1 if failed
 */
static T_S32 cam_gc0308_set_digital_zoom(T_U32 srcWidth, T_U32 srcHeight)
{
    T_U16 hrefstart = 0, vrefstart = 0;
    T_U8 high_bit = 0, low_bit = 0;
    T_CAMERA_MODE Cammode = s_gc0308_CurMode;
    T_U8 Camera_window_table[] =
    {
        0x46, 0,
        0x47, 0,
        0x48, 0,
        0x49, 0,
        0x4a, 0,
        0x4b, 0,
        0x4c, 0,
        END_FLAG, END_FLAG
    };
    
    akprintf(C1, M_DRVSYS, "set window size %d, %d, %d\r\n", Cammode, srcWidth, srcHeight);
       
    if (((srcWidth == 640) && (srcHeight == 480))
        || ((srcWidth == 352) && (srcHeight == 288))
        || ((srcWidth == 320) && (srcHeight == 240))
        || ((srcWidth == 176) && (srcHeight == 144)))
    {  
        return 1;
    }

    switch (s_gc0308_CurMode)
    {         
        case CAMERA_MODE_VGA:
            hrefstart =  (640 - srcWidth) / 2;
            vrefstart = (480 - srcHeight) / 2;
            break;
            
        case CAMERA_MODE_CIF:
            hrefstart =  (352 - srcWidth) / 2;
            vrefstart = (288 - srcHeight) / 2;
            break;
            
        case CAMERA_MODE_QVGA:
            hrefstart =  (320 - srcWidth) / 2;
            vrefstart = (240 - srcHeight) / 2;
            break;
            
        case CAMERA_MODE_QCIF:
            hrefstart =  (176 - srcWidth) / 2;
            vrefstart = (144 - srcHeight) / 2;
            break;
            
        case CAMERA_MODE_QQVGA:
            hrefstart =  (160 - srcWidth) / 2;
            vrefstart = (120 - srcHeight) / 2;
            break;
            
        case CAMERA_MODE_PREV:
            hrefstart =  (640 - srcWidth) / 2;
            vrefstart = (480 - srcHeight) / 2;
            break;

        case CAMERA_MODE_REC:
            hrefstart =  (640 - srcWidth) / 2;
            vrefstart = (480 - srcHeight) / 2;
            break;
            
        default:
            akprintf(C1, M_DRVSYS, "unsupported WINDOWING in mode %d!!\n", s_gc0308_CurMode);
            return 0;
    }
        
    high_bit = hrefstart >> 8;           //horizontal frame start high 3-bit
    low_bit = hrefstart & 0xff;          //horizontal frame start low 8-bit
    Camera_window_table[1] = 0x80 | (high_bit & 0x07);       
    Camera_window_table[5] = low_bit;
    Camera_window_table[11] = srcWidth >> 8;
    Camera_window_table[13] = srcWidth & 0xff;

    high_bit = vrefstart >> 8;          //vertical frame start high 2-bit
    low_bit = vrefstart & 0xff;         //vertical frame start low 8-bit
    Camera_window_table[1] = 0x80 | (high_bit & 0x30);       
    Camera_window_table[3] = low_bit;
    Camera_window_table[7] = srcHeight >> 8;
    Camera_window_table[9] = srcHeight & 0xff;

    if (camera_set_param(Camera_window_table)  == AK_TRUE)
    {
        return 1;
    }
    else
    {    
        return -1;
    }
}

static T_VOID cam_gc0308_set_night_mode(T_NIGHT_MODE mode)
{
    switch(mode)
    {
        case CAMERA_DAY_MODE:
            camera_setup(DAY_MODE_TAB);
            night_mode = CAMERA_DAY_MODE;
            break;
        case CAMERA_NIGHT_MODE:
            camera_setup(NIGHT_MODE_TAB);
            night_mode = CAMERA_NIGHT_MODE;
            break;
        default:
            akprintf(C1, M_DRVSYS, "set night mode parameter error!\n");
            break;
    }
}

static T_BOOL cam_gc0308_set_to_cap(T_U32 srcWidth, T_U32 srcHeight)
{    
    T_CAMERA_MODE Cammode;

    if ((srcWidth <= 160) && (srcHeight <= 120))
    {
        Cammode = CAMERA_MODE_QQVGA;
    }
    else if ((srcWidth <= 176) && (srcHeight <= 144))
    {
        Cammode = CAMERA_MODE_QCIF;
    }
    else if ((srcWidth <= 320) && (srcHeight <= 240))
    {
        Cammode = CAMERA_MODE_QVGA;
    }
    else if ((srcWidth <= 352) && (srcHeight <= 288))
    {
        Cammode = CAMERA_MODE_CIF;
    }
    else if ((srcWidth <= 640) && (srcHeight <= 480))
    {
        Cammode = CAMERA_MODE_VGA;
    }
    else
    {
        akprintf(C1, M_DRVSYS, "30W camera dose not support such mode");
        return AK_FALSE;
    }
    
    cam_gc0308_set_mode(Cammode);
    cam_gc0308_set_digital_zoom(srcWidth, srcHeight);    
    mini_delay(200);
    return AK_TRUE;
}

static T_BOOL cam_gc0308_set_to_prev(T_U32 srcWidth, T_U32 srcHeight)
{    
    cam_gc0308_set_mode(CAMERA_MODE_PREV);    
    cam_gc0308_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(200);
    return AK_TRUE;
}

static T_BOOL cam_gc0308_set_to_record(T_U32 srcWidth, T_U32 srcHeight)
{
    T_CAMERA_MODE Cammode;

    if ((srcWidth <= 160) && (srcHeight <= 120))
    {
        Cammode = CAMERA_MODE_QQVGA;
    }
    else if ((srcWidth <= 176) && (srcHeight <= 144))
    {
        Cammode = CAMERA_MODE_QCIF;
    }
    else if ((srcWidth <= 320) && (srcHeight <= 240))
    {
        Cammode = CAMERA_MODE_QVGA;
    }
    else if ((srcWidth <= 352) && (srcHeight <= 288))
    {
        Cammode = CAMERA_MODE_CIF;
    }
    else if ((srcWidth <= 640) && (srcHeight <= 480))
    {
        Cammode = CAMERA_MODE_REC;
    }
    else
    {
        akprintf(C1, M_DRVSYS, "30W camera dose not support such mode");
        return AK_FALSE;
    }

    cam_gc0308_set_mode(Cammode);
    cam_gc0308_set_digital_zoom(srcWidth, srcHeight);    
    mini_delay(200);
    return AK_TRUE;
}

static T_CAMERA_TYPE cam_gc0308_get_type(T_VOID)
{
    return camera_gc0308_type;
} 

static T_CAMERA_FUNCTION_HANDLER gc0308_function_handler = 
{
    GC0308_CAMERA_MCLK,
    cam_gc0308_open,
    cam_gc0308_close,
    cam_gc0308_read_id,
    cam_gc0308_init,
    cam_gc0308_set_mode,
    cam_gc0308_set_exposure,
    cam_gc0308_set_brightness,
    cam_gc0308_set_contrast,
    cam_gc0308_set_saturation,
    cam_gc0308_set_sharpness,
    cam_gc0308_set_AWB,
    cam_gc0308_set_mirror,
    cam_gc0308_set_effect,
    cam_gc0308_set_digital_zoom,
    cam_gc0308_set_night_mode,
    AK_NULL,
    cam_gc0308_set_to_cap,
    cam_gc0308_set_to_prev,
    cam_gc0308_set_to_record,
    cam_gc0308_get_type
};

static int camera_gc0308_reg(void)
{
    camera_reg_dev(CAMERA_GC0308_ID, &gc0308_function_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_gc0308_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif

