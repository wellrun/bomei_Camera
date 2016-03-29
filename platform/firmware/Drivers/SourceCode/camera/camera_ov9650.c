/**
 * @file camera_ov9650.c
 * @brief camera driver file
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2011-11-01
 * @version 1.0
 * @ref
 */ 
#include "akdefine.h"
#include "platform_hd_config.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "drv_camera.h"
#include "camera_ov9650.h"
#include "gpio_config.h"
#include "drv_sccb.h"

#include "Hal_print.h"

#ifdef USE_CAMERA_OV9650

#define CAM_EN_LEVEL        0    
#define CAM_RESET_LEVEL     1

#define CAMERA_SCCB_ADDR    0x60
#define CAMERA_OV9650_ID    0x9650
#define CAMERA_MCLK_DIV     3
#define OV9650_CAMERA_MCLK  24

static T_CAMERA_TYPE camera_ov9650_type = CAMERA_1P3M;
static T_NIGHT_MODE night_mode = CAMERA_DAY_MODE;
static T_CAMERA_MODE s_ov9650_CurMode = CAMERA_MODE_VGA;

/*
static T_VOID camera_setbit(T_U8 reg, T_U8 bit, T_U8 value)
{
    T_U8 tmp;

    tmp = sccb_read_data(CAMERA_SCCB_ADDR, reg);
    if (value == 1)
        tmp |= 0x1<<bit;
    else
        tmp &= ~(0x1<<bit);
    sccb_write_data(CAMERA_SCCB_ADDR, reg, &tmp, 1);
}
*/

static T_BOOL camera_set_param(const T_U8 tabParameter[])
{    
    int i = 0;
    T_U8 temp_value;

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

            if (!((tabParameter[i] == 0x12) && (tabParameter[i + 1] & 0x80))
                && !((tabParameter[i] == 0x29) && (tabParameter[i + 1] & 0x3f)))
            {                
                temp_value = sccb_read_data(CAMERA_SCCB_ADDR, tabParameter[i]);
                if (temp_value != tabParameter[i + 1])
                {
                    akprintf(C1, M_DRVSYS, "set parameter error!\n");
                    akprintf(C1, M_DRVSYS, "reg 0x%-4x write data is 0x%-4x, read data is 0x%-4x!\n", tabParameter[i], tabParameter[i + 1], temp_value);

                    return AK_FALSE;
                }
            }
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
            sccb_write_data(CAMERA_SCCB_ADDR, tabParameter[i], (T_U8 *)(&tabParameter[i + 1]), 1);
        }
        i += 2;
    }
}

static T_VOID cam_ov9650_open(T_VOID)
{
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

static T_BOOL cam_ov9650_close(T_VOID)
{    
    gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, !CAM_EN_LEVEL);
    gpio_set_pin_level(GPIO_CAMERA_AVDD, !gpio_pin_get_ActiveLevel(GPIO_CAMERA_AVDD));    
    gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_INPUT);

    gpio_set_pin_dir(GPIO_I2C_SCL, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_I2C_SCL, GPIO_LEVEL_LOW);
    gpio_set_pin_dir(GPIO_I2C_SDA, GPIO_DIR_OUTPUT);
    gpio_set_pin_level(GPIO_I2C_SDA, GPIO_LEVEL_LOW);
    
    return AK_TRUE;
}

static T_U32 cam_ov9650_read_id(T_VOID)
{    
    T_U8 value = 0x00;
    T_U32 id = 0;
	T_U32 i;

    sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);        //init sccb first here!!
    
	for(i=0;i<3;++i)
	{
    value = sccb_read_data(CAMERA_SCCB_ADDR, 0x0a);
		if(value)
			break;
	}
	//akprintf(C3, M_DRVSYS, "after %d times,value = 0x%x\n",i+1, value);
    id = value << 8;
    value = sccb_read_data(CAMERA_SCCB_ADDR, 0x0b);
	//akprintf(C3, M_DRVSYS, "value = 0x%x\n", value);
    if (value < 0x55)    //set all to 0x9650
        value = 0x50;
    id |= value;    
	//akprintf(C3, M_DRVSYS, "id = 0x%x\n", id);            
    return id;
}


/**
 * @brief: initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author: 
 * @date 2004-09-22
 * @return T_BOOL
 * @retval AK_TRUE if success, else AK_FALSE
 */
static T_BOOL cam_ov9650_init(T_VOID)
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
 * @brief Set camera mode to specify image quality, SXGA/VGA/CIF/ etc. 
 * @author 
 * @date 2008-01-24
 * @param[in] mode mode value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov9650_set_mode(T_CAMERA_MODE mode)
{
    s_ov9650_CurMode = mode;
    
    switch(mode)
    {
        case CAMERA_MODE_SXGA:              //capture mode
            camera_setup(SXGA_MODE_TAB);
            break;        
        case CAMERA_MODE_VGA:               //capture mode
            camera_setup(VGA_MODE_TAB);
            break;
        case CAMERA_MODE_CIF:               //capture mode
            camera_setup(CIF_MODE_TAB);
            break;
        case CAMERA_MODE_QVGA:              //capture mode
            camera_setup(QVGA_MODE_TAB);
            break;
        case CAMERA_MODE_QCIF:              //capture mode
            camera_setup(QCIF_MODE_TAB);
            break;
        case CAMERA_MODE_QQVGA:             //capture mode
            camera_setup(QQVGA_MODE_TAB);
            break;
        case CAMERA_MODE_PREV:              //preview mode
            camera_setup(PREV_MODE_TAB);
            
            if (CAMERA_DAY_MODE == night_mode)
            {
                camera_setup(DAY_MODE_TAB);
            }
            else
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        case CAMERA_MODE_REC:              //record mode
            camera_setup(RECORD_MODE_TAB);
            
            if (CAMERA_DAY_MODE == night_mode)
            {
                camera_setup(DAY_MODE_TAB);
            }
            else
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        default:
            s_ov9650_CurMode = CAMERA_MODE_VGA;
            akprintf(C1, M_DRVSYS, "set camera mode parameter error!\n");
            break;
    }
}

/**
 * @brief: Set camera exposure mode 
 * @author: 
 * @date 2008-01-24
 * @param[in] exposure exposure mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov9650_set_exposure(T_CAMERA_EXPOSURE exposure)
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
 * @brief: Set camera brightness level 
 * @author: 
 * @date 2008-01-24
 * @param[in] brightness brightness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov9650_set_brightness(T_CAMERA_BRIGHTNESS brightness)
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
 * @brief: Set camera contrast level 
 * @author: 
 * @date 2008-01-24
 * @param[in] contrast contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov9650_set_contrast(T_CAMERA_CONTRAST contrast)
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
 * @brief: Set camera saturation level 
 * @author: 
 * @date 2008-01-24
 * @param[in] saturation saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov9650_set_saturation(T_CAMERA_SATURATION saturation)
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
 * @brief: Set camera sharpness level 
 * @author: 
 * @date 2008-01-24
 * @param[in] sharpness sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov9650_set_sharpness(T_CAMERA_SHARPNESS sharpness)
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
 * @brief: Set camera AWB mode 
 * @author: 
 * @date 2008-01-24
 * @param[in] awb AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov9650_set_AWB(T_CAMERA_AWB awb)
{
    switch(awb)
    {
        case AWB_AUTO:
            cam_ov9650_set_contrast(CAMERA_CONTRAST_4);
            cam_ov9650_set_saturation(CAMERA_SATURATION_4);
            cam_ov9650_set_brightness(CAMERA_BRIGHTNESS_3);
            camera_setup(AWB_AUTO_TAB);
            break;
        case AWB_SUNNY:
            cam_ov9650_set_contrast(CAMERA_CONTRAST_5);
            cam_ov9650_set_saturation(CAMERA_SATURATION_5);
            cam_ov9650_set_brightness(CAMERA_BRIGHTNESS_3);
            camera_setup(AWB_SUNNY_TAB);
            break;
        case AWB_CLOUDY:
            cam_ov9650_set_contrast(CAMERA_CONTRAST_5);
            cam_ov9650_set_saturation(CAMERA_SATURATION_4);
            cam_ov9650_set_brightness(CAMERA_BRIGHTNESS_3);
            camera_setup(AWB_CLOUDY_TAB);
            break;
        case AWB_OFFICE:
            cam_ov9650_set_contrast(CAMERA_CONTRAST_4);
            cam_ov9650_set_saturation(CAMERA_SATURATION_4);
            cam_ov9650_set_brightness(CAMERA_BRIGHTNESS_3);
            camera_setup(AWB_OFFICE_TAB);
            break;
        case AWB_HOME:
            cam_ov9650_set_contrast(CAMERA_CONTRAST_4);
            cam_ov9650_set_saturation(CAMERA_SATURATION_3);
            cam_ov9650_set_brightness(CAMERA_BRIGHTNESS_3);
            camera_setup(AWB_HOME_TAB);
            break;
        case AWB_NIGHT:
            /*cam_ov9650_set_contrast(CAMERA_CONTRAST_4);
            cam_ov9650_set_saturation(CAMERA_SATURATION_2);
            cam_ov9650_set_brightness(CAMERA_BRIGHTNESS_5);
            camera_setup(AWB_NIGHT_TAB);*/
            break;
        default:
            akprintf(C1, M_DRVSYS, "set AWB mode parameter error!\n");
            break;            
    }
}

/**
 * @brief: Set camera mirror mode 
 * @author: 
 * @date 2008-01-24
 * @param[in] mirror mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov9650_set_mirror(T_CAMERA_MIRROR mirror)
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
            akprintf(C1, M_DRVSYS, "set mirror parameter error!\n");
            break;
    }
}

/**
 * @brief: Set camera effect mode 
 * @author: 
 * @date 2004-09-22
 * @param[in] effect effect mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov9650_set_effect(T_CAMERA_EFFECT effect)
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
        case CAMERA_EFFECT_RED:
            camera_setup(EFFECT_REDDISH_TAB);
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
 * @brief: Set camera windows  
 * @author: 
 * @date 2008-01-24
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_S32
 * @retval 0 if error mode 
 * @retval 1 if success
 * @retval -1 if fail
 */
static T_S32 cam_ov9650_set_digital_zoom(T_U32 srcWidth, T_U32 srcHeight)
{
    T_U16 hrefstart = 0, hrefstop = 0, vrefstart = 0, vrefstop = 0;
    T_U8 hbit = 0, lstartbit = 0, lstopbit = 0;

    T_U8 Camera_window_table[] =
    {
        //1、以下3个寄存器共同构成水平宽度，如1280、640...
        0x17, 0,
        0x18, 0,
        0x32, 0,
        //2、以下3个寄存器共同构成垂直高度，如1024、480...
        0x19, 0,
        0x1A, 0,
        0x03, 0,
        END_FLAG, END_FLAG
    };

    akprintf(C1, M_DRVSYS, "set window size %d, %d\r\n", srcWidth, srcHeight);

    switch(s_ov9650_CurMode)
    {
        case CAMERA_MODE_SXGA:
            hrefstart = 241 + (1280 - srcWidth) / 2;    //水平为0x18-0x17的值*8，故除2
            hrefstop = 1521 - (1280 - srcWidth) / 2;
            
            vrefstart = 10 + (1024 - srcHeight) / 2;    //垂直为0x1a-0x19的值*8，故除2
            vrefstop = 1034 - (1024 - srcHeight) / 2;
            break;
            
        case CAMERA_MODE_VGA:
            hrefstart = 318 + (640 - srcWidth);         //水平为0x18-0x17的值*4，故不除
            hrefstop = 1598 - (640 - srcWidth);
            
            vrefstart = 8 + (480 - srcHeight) / 2;      //垂直为0x1a-0x19的值*8，故除2
            vrefstop = 488 - (480 - srcHeight) / 2;
            break;
            
        case CAMERA_MODE_CIF:
            hrefstart = 308 + (352 - srcWidth);         //水平为0x18-0x17的值*4，故不除
            hrefstop = 1012 - (352 - srcWidth);
            
            vrefstart = 6 + (288 - srcHeight) / 2;      //垂直为0x1a-0x19的值*8，故除2
            vrefstop = 294 - (288 - srcHeight) / 2;
            break;
            
        case CAMERA_MODE_QVGA:
            hrefstart = 312 + (320 - srcWidth) * 2;     //水平为0x18-0x17的值*2，故*2
            hrefstop = 1592 - (320 - srcWidth) * 2;
            
            vrefstart = 6 + (240 - srcHeight) / 2;      //垂直为0x1a-0x19的值*8，故除2
            vrefstop = 246 - (240 - srcHeight) / 2;
            break;
            
        case CAMERA_MODE_QCIF:
            hrefstart = 308 + (176 - srcWidth) * 2;     //水平为0x18-0x17的值*2，故*2
            hrefstop = 1012 - (176 - srcWidth) * 2;
            
            vrefstart = 6 + (144 - srcHeight) / 2;      //垂直为0x1a-0x19的值*8，故除2
            vrefstop = 150 - (144 - srcHeight) / 2;
            break;
            
        case CAMERA_MODE_QQVGA:
            hrefstart = 308 + (160 - srcWidth) * 4;     //水平为0x18-0x17的值*1，故*4
            hrefstop = 1588 - (160 - srcWidth) * 4;
            
            vrefstart = 6 + (120 - srcHeight);          //垂直为0x1a-0x19的值*4，故*1
            vrefstop = 246 - (120 - srcHeight);
            break;
            
        case CAMERA_MODE_PREV:
            hrefstart = 318 + (640 - srcWidth);         //水平为0x18-0x17的值*4，故不除
            hrefstop = 1598 - (640 - srcWidth);
            
            vrefstart = 8 + (480 - srcHeight) / 2;      //垂直为0x1a-0x19的值*8，故除2
            vrefstop = 488 - (480 - srcHeight) / 2;
            break;

        case CAMERA_MODE_REC:                 //CIF
            hrefstart = 318 + (640 - srcWidth);         //水平为0x18-0x17的值*4，故不除
            hrefstop = 1598 - (640 - srcWidth);
            
            vrefstart = 8 + (480 - srcHeight) / 2;      //垂直为0x1a-0x19的值*8，故除2
            vrefstop = 488 - (480 - srcHeight) / 2;
            break;
            
        default:
            akprintf(C1, M_DRVSYS, "unsupported WINDOWING in mode %d!!\n", s_ov9650_CurMode);
            return 0;
    }
    
    hbit = hrefstart >> 3;         //Horizontal Frame start high 8-bit
    lstartbit = hrefstart & 0x7;   //Horizontal Frame start low 3-bit
    
    Camera_window_table[1] = hbit;
        
    hbit = hrefstop >> 3;          //Horizontal Frame end high 8-bit
    lstopbit = hrefstop & 0x7;     //Horizontal Frame end low 3-bit
    
    Camera_window_table[3] = hbit;
    
    Camera_window_table[5] = 0x80 | lstartbit | (lstopbit << 3);
    
    hbit = vrefstart >> 3;         //Vertical Frame start high 8-bit
    lstartbit = vrefstart & 0x7;   //Vertical Frame start low 2-bit
    
    Camera_window_table[7] = hbit;
    
    hbit = vrefstop >> 3;          //Vertical Frame end high 8-bit
    lstopbit = vrefstop & 0x7;     //Vertical Frame end low 2-bit
    
    Camera_window_table[9] = hbit;
    
    Camera_window_table[11] = 0x0 | lstartbit | (lstopbit << 3);
    
    if (camera_set_param(Camera_window_table) == AK_TRUE)
        return 1;
    else
        return -1;
}

static T_VOID cam_ov9650_set_night_mode(T_NIGHT_MODE mode)
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

T_U32 cam_ov9650_set_framerate(float framerate)
{            
    T_U8 Reg0x92, Reg0x93;
    T_U8 Reg0xa2, Reg0x6a;
    T_U8 Reg0x2a, Reg0x2b;
    T_U8 Reg0x11, Div;
    float dummy_line, mclk;
    //float preview_framerate;    
    float actual_framerate;
    T_CAMERA_MODE Cammode;    
    T_U16 SXGA_maximum_lines = 1045;
    T_U16 VGA_maximum_lines = 495;

    Reg0x2a = 0x00;
    Reg0x2b = 0x00;
    sccb_write_data(CAMERA_SCCB_ADDR, 0x2a, &Reg0x2a, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x2b, &Reg0x2b, 1);

    mclk = OV9650_CAMERA_MCLK * 1000000;

    switch(s_ov9650_CurMode)
    {           
        case CAMERA_MODE_SXGA:
            if (framerate <= 7.5)
            {
                Reg0x11 = 0x80;
            }           
            else if ((framerate <= 15) && (OV9650_CAMERA_MCLK > 24))
            {
                Reg0x11 = 0x80;         
            }            
            else
            {
                akprintf(C1, M_DRVSYS, "ov9650 set SXGA framerate error!\n");
                return 0;
            }

            Cammode = CAMERA_MODE_SXGA;
            Div = (T_U8)(Reg0x11 & 0x07) + 1;
            actual_framerate = mclk / Div / (1520 * 1050 * 2);      //0x11 = Ox80, 所以不用除
            dummy_line = 1050 * actual_framerate / framerate - 1050;
            
            Reg0x93 = ((T_U16)dummy_line >> 8) & 0x0f ;
            Reg0x92 = (T_U8)dummy_line;
            Reg0xa2 = (VGA_maximum_lines + dummy_line) * framerate / 100;
            Reg0x6a = framerate * SXGA_maximum_lines / 100;
            break;
            
        case CAMERA_MODE_REC:                
            if (framerate <= 7.5)
            {
                Reg0x11 = 0x82;
            }            
            else if (framerate <= 15)
            {
                Reg0x11 = 0x81;
            }           
            else if (framerate <= 30)
            {
                Reg0x11 = 0x80;
            }            
            else
            {
                akprintf(C1, M_DRVSYS, "ov9650 set VGA framerate error!\n");
                return 0;
            }
            
            Div = (T_U8)(Reg0x11 & 0x07) + 1;
            actual_framerate = mclk / Div / (800 * 500 * 2);    //0x11 = Ox80
            dummy_line = 500 * actual_framerate / framerate - 500;

            Reg0x93 = ((T_U16)dummy_line >> 8) & 0x0f;
            Reg0x92 = (T_U8)dummy_line;
            Reg0xa2 = (VGA_maximum_lines + dummy_line) * framerate / 100;
            Reg0x6a = framerate * VGA_maximum_lines / 100;
            break;    
            
        default:                      
            akprintf(C1, M_DRVSYS, "ov9650 unsupport mode %d!\n", s_ov9650_CurMode);
            return 0;
    }
    
    sccb_write_data(CAMERA_SCCB_ADDR, 0x11, &Reg0x11, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x92, &Reg0x92, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x93, &Reg0x93, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0xa2, &Reg0x93, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x6a, &Reg0x6a, 1);
    
    return 1;
}

static T_VOID start_preview(T_CAMERA_MODE Cammode)
{
    T_U8 Reg0x13;

    //Change to preview mode
    cam_ov9650_set_mode(Cammode);    
    
    //Start AG/AE
    Reg0x13 = 0xe7;
    sccb_write_data(CAMERA_SCCB_ADDR, 0x13, &Reg0x13, 1);    
}

/*
static T_VOID start_capture(T_CAMERA_MODE Cammode, float Mclk)
{            
    T_U8 Reg0x13;
    T_U8 Reg0x2a;
    T_U8 Reg0x2b;
    T_U8 Reg0x6a;
    
    T_U8 pre_gain00;
    T_U8 pre_expoA1;
    T_U8 pre_expo10;
    T_U8 pre_expo04;

    T_U32 pre_expo;
    T_U32 pre_gain;
    T_U32 cap_expo;
    T_U32 cap_gain;
    T_U32 cap_expo_gain;

    T_U8 cap_expo04;
    T_U8 cap_expo10;
    T_U8 cap_expoA1;
    T_U8 cap_gain00;

    float temp_data;
    float preview_framerate;    
    float capture_framerate;
    float actual_capture_framerate;
        
    T_U16 SXGA_maximum_lines = 1045;
    T_U16 VGA_maximum_lines = 495;
    T_U16 CIF_maximum_lines = 382;
    T_U16 QVGA_maximum_lines = 248;
    T_U16 QCIF_maximum_lines = 190;
    T_U16 QQVGA_maximum_lines = 248;
        
    T_U16 Lines_10ms;

    //Stop AE/AG
    Reg0x13 = 0xe2;
    sccb_write_data(CAMERA_SCCB_ADDR, 0x13, &Reg0x13, 1);

    //caculate pre_exposure
    pre_gain00 = sccb_read_data(CAMERA_SCCB_ADDR, 0x00);
    pre_expo10 = sccb_read_data(CAMERA_SCCB_ADDR, 0x10);
    pre_expoA1 = sccb_read_data(CAMERA_SCCB_ADDR, 0xA1);
    pre_expo04 = sccb_read_data(CAMERA_SCCB_ADDR, 0x04);
    
    pre_expo = ((pre_expoA1 & 0x3f) << 10) + (pre_expo10 << 2) + (pre_expo04 & 0x03);

//    akprintf(C3, M_DRVSYS, "pre_gain00 = 0x%-8x\n", pre_gain00);
//    akprintf(C3, M_DRVSYS, "pre_expo10 = 0x%-8x\n", pre_expo10);
//    akprintf(C3, M_DRVSYS, "pre_expoA1 = 0x%-8x\n", pre_expoA1);
//    akprintf(C3, M_DRVSYS, "pre_expo04 = 0x%-8x\n", pre_expo04);
//    akprintf(C3, M_DRVSYS, "pre_expo   = 0x%-8x\n", pre_expo);

    //caculate pre_gain
    pre_gain = (pre_gain00 & 0x0f) + 16;
    if(pre_gain00 & 0x10)
    {
        pre_gain = pre_gain << 1;
    }
    if (pre_gain00 & 0x20)
    {
        pre_gain = pre_gain << 1;
    }
    if (pre_gain00 & 0x40)
    {
        pre_gain = pre_gain << 1;
    }
    if (pre_gain00 & 0x80)
    {
        pre_gain = pre_gain << 1;
    }
            
//    akprintf(C3, M_DRVSYS, "pre_gain = 0x%-8x\n", pre_gain);        

    if (CAMERA_DAY_MODE == night_mode)
    {
        preview_framerate = 12.5;
    }
    else
    {
        preview_framerate = 6.25;
    }
            
    switch(Cammode)
    {
        case CAMERA_MODE_SXGA:              //3.125fps
            capture_framerate = 3.125;
            actual_capture_framerate = Mclk * 1000000 / (1520 * 1050 * 2);      //0x11 = Ox80, 所以不用除
            temp_data = 1520 * actual_capture_framerate / capture_framerate - 1520;
            
            Reg0x2a = (((T_U16)temp_data >> 8) & 0x0f) << 4;
            Reg0x2b = (T_U8)temp_data;
            Reg0x6a = capture_framerate * SXGA_maximum_lines / 100;
            
            pre_expo = pre_expo * (capture_framerate * SXGA_maximum_lines) / (preview_framerate * VGA_maximum_lines); 

            Lines_10ms = (Mclk * 1000000 * 10 / 1000) / ((1520 + temp_data) * 2);
            break;    
            
        case CAMERA_MODE_VGA:               //6.25fps
            capture_framerate = 6.25;
            actual_capture_framerate = Mclk * 1000000 / 3 / (800 * 500 * 2);    //0x11 = Ox82, 所以要除以3
            temp_data = 800 * actual_capture_framerate / capture_framerate - 800;
            
            Reg0x2a = (((T_U16)temp_data >> 8) & 0x0f) << 4;
            Reg0x2b = (T_U8)temp_data;
            Reg0x6a = capture_framerate * VGA_maximum_lines / 100;
            
            pre_expo = pre_expo * (capture_framerate * VGA_maximum_lines) / (preview_framerate * VGA_maximum_lines); 

            Lines_10ms = (Mclk * 1000000 * 10 / 1000) / ((800 + temp_data) * 2);
            break;
            
        case CAMERA_MODE_CIF:               //6.25fps
            capture_framerate = 6.25;
            actual_capture_framerate = Mclk * 1000000 / 6 / (520 * 384 * 2);        //0x11 = Ox85, 所以要除以6
            temp_data = 520 * actual_capture_framerate / capture_framerate - 520;
            
            Reg0x2a = (((T_U16)temp_data >> 8) & 0x0f) << 4;
            Reg0x2b = (T_U8)temp_data;
            Reg0x6a = capture_framerate * CIF_maximum_lines / 100;
            
            pre_expo = pre_expo * (capture_framerate * CIF_maximum_lines) / (preview_framerate * VGA_maximum_lines); 

            Lines_10ms = (Mclk * 1000000 * 10 / 1000) / ((520 + temp_data) * 2);
            break;
            
        case CAMERA_MODE_QVGA:              //6.25fps
            capture_framerate = 6.25;
            actual_capture_framerate = Mclk * 1000000 / 4 / (400 * 250 * 2);           //0x11 = Ox83, 所以要除以4
            temp_data = 400 * actual_capture_framerate / capture_framerate - 400;
            
            Reg0x2a = (((T_U16)temp_data >> 8) & 0x0f) << 4;
            Reg0x2b = (T_U8)temp_data;
            Reg0x6a = capture_framerate * QVGA_maximum_lines / 100;
            
            pre_expo = pre_expo * (capture_framerate * QVGA_maximum_lines) / (preview_framerate * VGA_maximum_lines); 

            Lines_10ms = (Mclk * 1000000 * 10 / 1000) / ((400 + temp_data) * 2);
            break;
            
        case CAMERA_MODE_QCIF:              //6.25fps
            capture_framerate = 6.25;
            actual_capture_framerate = Mclk * 1000000 / 8 / (260 * 192 * 2);            //0x11 = Ox87, 所以要除以8
            temp_data = 260 * actual_capture_framerate / capture_framerate - 260;  
            
            Reg0x2a = (((T_U16)temp_data >> 8) & 0x0f) << 4;
            Reg0x2b = (T_U8)temp_data;
            Reg0x6a = capture_framerate * QCIF_maximum_lines / 100;
            
            pre_expo = pre_expo * (capture_framerate * QCIF_maximum_lines) / (preview_framerate * VGA_maximum_lines); 

            Lines_10ms = (Mclk * 1000000 * 10 / 1000) / ((260 + temp_data) * 2);
            break;
            
        case CAMERA_MODE_QQVGA:             //6.25fps
            capture_framerate = 6.25;
            actual_capture_framerate = Mclk * 1000000 / 4 / (400 * 250 * 2);              //0x11 = Ox83, 所以要除以4
            temp_data = 400 * actual_capture_framerate / capture_framerate - 400;
            
            Reg0x2a = (((T_U16)temp_data >> 8) & 0x0f) << 4;
            Reg0x2b = (T_U8)temp_data;
            Reg0x6a = capture_framerate * QQVGA_maximum_lines / 100;
            
            pre_expo04 = 0x24;
            pre_expo = pre_expo * (capture_framerate * QQVGA_maximum_lines) / (preview_framerate * VGA_maximum_lines); 

            Lines_10ms = (Mclk * 1000000 * 10 / 1000) / ((400 + temp_data) * 2);
            break;
            
        default:
            akprintf(C1, M_DRVSYS, "set camera capture mode parameter error!\n");
            break;
    }
//    akprintf(C3, M_DRVSYS, "pre_expo = 0x%-8x\n", pre_expo);
//    akprintf(C3, M_DRVSYS, "actual_capture_framerate = 0x%-8x\n", (T_U16)actual_capture_framerate);
//    akprintf(C3, M_DRVSYS, "Reg0x2a = 0x%-8x\n", Reg0x2a);
//    akprintf(C3, M_DRVSYS, "Reg0x2b = 0x%-8x\n", Reg0x2b);
//    akprintf(C3, M_DRVSYS, "Reg0x6a = 0x%-8x\n", Reg0x6a);
//    akprintf(C3, M_DRVSYS, "Lines_10ms = 0x%-8x\n", Lines_10ms);
            
    //caculate cap_expo, cap_gain 
    cap_expo_gain = pre_gain * pre_expo;
    if (cap_expo_gain < 1048 * 16)
    {
        cap_expo = cap_expo_gain / 16;
        
        if (cap_expo > Lines_10ms) 
        {                        
            cap_expo = (cap_expo + 1) / Lines_10ms * Lines_10ms;
        }        
        //Capture_gain = 16;        
        cap_gain = cap_expo_gain / cap_expo;
    }
    else
    {//if image is dark,//and use the correct exposure time to eliminate 50/60 hz line
        cap_expo = 1048;
        cap_gain = cap_expo_gain / 1048;
    }

//    akprintf(C3, M_DRVSYS, "cap_expo_gain = 0x%-8x\n", cap_expo_gain);
//    akprintf(C3, M_DRVSYS, "cap_expo      = 0x%-8x\n", cap_expo);
//    akprintf(C3, M_DRVSYS, "cap_gain      = 0x%-8x\n", cap_gain);
    
    //caculate cap_gain00, cap_expo04, cap_expo10, cap_expoA1    
    cap_expo04 = (pre_expo04 & 0xfc) | (cap_expo & 0x03);
    cap_expo10 = (cap_expo >> 2) & 0xff;
    cap_expoA1 = cap_expo >> 10;
    
    cap_gain00 = 0;
    if(cap_gain > 31)
    {
        cap_gain00 |= 0x10;
        cap_gain = cap_gain >> 1;
    }
    if(cap_gain > 31)
    {
        cap_gain00 |= 0x20;
        cap_gain = cap_gain >> 1;
    }
    if(cap_gain > 31)
    {
        cap_gain00 |= 0x40;
        cap_gain = cap_gain >> 1;
    }
    if(cap_gain > 31)
    {
        cap_gain00 |= 0x80;
        cap_gain = cap_gain >> 1;
    }
    if(cap_gain > 16)
    {
        cap_gain00 |= ((cap_gain - 16) & 0x0f);
    }

//    akprintf(C3, M_DRVSYS, "cap_gain00 = 0x%-8x\n", cap_gain00);
//    akprintf(C3, M_DRVSYS, "cap_expo10 = 0x%-8x\n", cap_expo10);
//    akprintf(C3, M_DRVSYS, "cap_expoA1 = 0x%-8x\n", cap_expoA1);
//    akprintf(C3, M_DRVSYS, "cap_expo04 = 0x%-8x\n", cap_expo04);    
    
    //change to SXGA mode 
    cam_ov9650_set_mode(Cammode);
    
    sccb_write_data(CAMERA_SCCB_ADDR, 0x2a, &Reg0x2a, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x2b, &Reg0x2b, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x6a, &Reg0x6a, 1);

    //Write registers
    sccb_write_data(CAMERA_SCCB_ADDR, 0x00, &cap_gain00, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0xA1, &cap_expoA1, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x10, &cap_expo10, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x04, &cap_expo04, 1);
}
*/
static T_VOID start_record(T_CAMERA_MODE Cammode)
{
    T_U8 Reg0x13;

    //Change to record mode
    cam_ov9650_set_mode(Cammode);        
    
    //Start AG/AE
    Reg0x13 = 0xe7;
    sccb_write_data(CAMERA_SCCB_ADDR, 0x13, &Reg0x13, 1);    
}

static T_BOOL cam_ov9650_set_to_cap(T_U32 srcWidth, T_U32 srcHeight)
{    
    float Mclk;
    T_CAMERA_MODE Cammode;

    Mclk = (float)(get_asic_freq() / 1000000) / CAMERA_MCLK_DIV;

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
    else if ((srcWidth <= 1280) && (srcHeight <= 1024))
    {
        Cammode = CAMERA_MODE_SXGA;
    }
    else
    {
        akprintf(C1, M_DRVSYS, "ov9650 unsupport %d & %d mode!\n", srcWidth, srcHeight);
        return AK_FALSE;
    } 
     
    //start_capture(Cammode, Mclk); 
    cam_ov9650_set_mode(Cammode);
    cam_ov9650_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(300);
    return AK_TRUE;
}

static T_BOOL cam_ov9650_set_to_prev(T_U32 srcWidth, T_U32 srcHeight)
{    
    start_preview(CAMERA_MODE_PREV);
    cam_ov9650_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(300);
    return AK_TRUE;
}

static T_BOOL cam_ov9650_set_to_record(T_U32 srcWidth, T_U32 srcHeight)
{    
    start_record(CAMERA_MODE_REC);
    cam_ov9650_set_digital_zoom(srcWidth, srcHeight);
    
    mini_delay(300);
    return AK_TRUE;
}

static T_CAMERA_TYPE cam_ov9650_get_type(T_VOID)
{
    return camera_ov9650_type;
}

static T_CAMERA_FUNCTION_HANDLER ov9650_function_handler = 
{
    OV9650_CAMERA_MCLK,
    cam_ov9650_open,
    cam_ov9650_close,
    cam_ov9650_read_id,
    cam_ov9650_init,
    cam_ov9650_set_mode,
    cam_ov9650_set_exposure,
    cam_ov9650_set_brightness,
    cam_ov9650_set_contrast,
    cam_ov9650_set_saturation,
    cam_ov9650_set_sharpness,
    cam_ov9650_set_AWB,
    cam_ov9650_set_mirror,
    cam_ov9650_set_effect,
    cam_ov9650_set_digital_zoom,
    cam_ov9650_set_night_mode,
    cam_ov9650_set_framerate,
    cam_ov9650_set_to_cap,
    cam_ov9650_set_to_prev,
    cam_ov9650_set_to_record,
    cam_ov9650_get_type
};

static int camera_ov9650_reg(void)
{
    camera_reg_dev(CAMERA_OV9650_ID, &ov9650_function_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_ov9650_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif

