/**
 * @file camera_ov7670.c
 * @brief camera driver file
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @author xia_wenting 
 * @date 2011-10-27
 * @version 1.0
 * @ref
 */
#include "akdefine.h"
#include "platform_hd_config.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "drv_camera.h"
#include "camera_ov7670.h"
#include "gpio_config.h"
#include "drv_sccb.h"


#ifdef USE_CAMERA_OV7670

#define CAM_EN_LEVEL            0    
#define CAM_RESET_LEVEL         0

#define CAMERA_SCCB_ADDR        0x42
#define CAMERA_OV7670_ID        0x7673

#if (defined(CHIP_AK7801) || defined(CHIP_AK7802))
    #define CAMERA_MCLK_DIV         3              //192Mhz/(2*(3+1))=24Mhz
#elif defined(CHIP_AK322L)
    #define CAMERA_MCLK_DIV         15             //90Mhz/15=6Mhz
#else
    #define CAMERA_MCLK_DIV         14             //84Mhz/14=6Mhz
#endif
#define OV7670_CAMERA_MCLK          24 //16, 20fps

static T_CAMERA_TYPE camera_ov7670_type = CAMERA_P3M;
static T_NIGHT_MODE night_mode = CAMERA_DAY_MODE;
static T_CAMERA_MODE s_ov7670_CurMode = CAMERA_MODE_VGA;

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
                && !((tabParameter[i] == 0xc9) && (tabParameter[i + 1] & 0x60)))
            {                
                temp_value = sccb_read_data(CAMERA_SCCB_ADDR, tabParameter[i]);
                if (temp_value != tabParameter[i + 1])
                {
                    akprintf(C1, M_DRVSYS, "set parameter error!\n");
                    akprintf(C1, M_DRVSYS, "reg 0x%x write data is 0x%x, read data is 0x%x!\n", tabParameter[i], tabParameter[i + 1], temp_value);

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
            sccb_write_data(CAMERA_SCCB_ADDR, tabParameter[i], (T_U8 *)&tabParameter[i + 1], 1);
        }
        i += 2;
    }
}

static T_VOID cam_ov7670_open(T_VOID)
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

static T_BOOL cam_ov7670_close(T_VOID)
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

static T_U32 cam_ov7670_read_id(T_VOID)
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
static T_BOOL cam_ov7670_init()
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
 * @brief: Set camera mode to specify image quality, SXGA/VGA/CIF/ etc 
 * @author: 
 * @date 2004-09-22
 * @param[in] mode mode value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7670_set_mode(T_CAMERA_MODE mode)
{
    s_ov7670_CurMode = mode;
    switch(mode)
    {
        case CAMERA_MODE_VGA:
            camera_setup(VGA_MODE_TAB);
            
            if (CAMERA_NIGHT_MODE == night_mode)
            {
                camera_setup(NIGHT_MODE_TAB);
            }
            break;
        case CAMERA_MODE_CIF:
//            camera_setup(CIF_MODE_TAB);
            break;
        case CAMERA_MODE_QVGA:
//            camera_setup(QVGA_MODE_TAB);
            break;
        case CAMERA_MODE_QCIF:
//            camera_setup(QCIF_MODE_TAB);
            break;
        case CAMERA_MODE_QQVGA:
//            camera_setup(QQVGA_MODE_TAB);
            break;
        case CAMERA_MODE_PREV:                              //preview mode
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
            s_ov7670_CurMode = CAMERA_MODE_VGA;
            akprintf(C1, M_DRVSYS, "set camera mode parameter error!\n");
            break;
        }
}

/**
 * @brief: Set camera exposure mode 
 * @author: 
 * @date 2004-09-22
 * @param[in] exposure exposure mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7670_set_exposure(T_CAMERA_EXPOSURE exposure)
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
 * @date 2004-09-22
 * @param[in] brightness brightness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7670_set_brightness(T_CAMERA_BRIGHTNESS brightness)
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
 * @date 2004-09-22
 * @param[in] contrast contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7670_set_contrast(T_CAMERA_CONTRAST contrast)
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
 * @date 2004-09-22
 * @param[in] saturation saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7670_set_saturation(T_CAMERA_SATURATION saturation)
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
 * @date 2004-09-22
 * @param[in] sharpness sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7670_set_sharpness(T_CAMERA_SHARPNESS sharpness)
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
 * @date 2004-09-22
 * @param[in] awb AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7670_set_AWB(T_CAMERA_AWB awb)
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
 * @brief: Set camera mirror mode 
 * @author: 
 * @date 2004-09-22
 * @param[in] mirror mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7670_set_mirror(T_CAMERA_MIRROR mirror)
{
    switch(mirror)
    {
        case CAMERA_MIRROR_V:
            camera_setbit(0x1e, 4, 1);
            camera_setbit(0x1e, 5, 0);
            break;
        case CAMERA_MIRROR_H:
            camera_setbit(0x1e, 4, 0);
            camera_setbit(0x1e, 5, 1);
            break;
        case CAMERA_MIRROR_NORMAL:
            camera_setbit(0x1e, 4, 0);
            camera_setbit(0x1e, 5, 0);
            break;
        case CAMERA_MIRROR_FLIP:
            camera_setbit(0x1e, 4, 1);
            camera_setbit(0x1e, 5, 1);
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
static T_VOID cam_ov7670_set_effect(T_CAMERA_EFFECT effect)
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
 * @date 2004-09-22
 * @param[in] srcWidth window width
 * @param[in] srcHeight window height
 * @return T_S32
 * @retval 0 if error mode 
 * @retval 1 if success
 * @retval -1 if fail
 */
static T_S32 cam_ov7670_set_digital_zoom(T_U32 srcWidth, T_U32 srcHeight)
{
    T_U16 hrefstart = 0, hrefstop = 0, vrefstart = 0, vrefstop = 0;
    T_U8 hbit = 0, lstartbit = 0, lstopbit = 0;
    T_CAMERA_MODE Cammode = s_ov7670_CurMode;
    T_U8 Camera_window_table[] =
    {
        0x17, 0,
        0x18, 0,
        0x32, 0,
        0x19, 0,
        0x1a, 0,
        0x03, 0,
        END_FLAG, END_FLAG
    };
    
    akprintf(C1, M_DRVSYS, "set window size %d, %d, %d\r\n", Cammode, srcWidth, srcHeight);
    
    if(Cammode == CAMERA_MODE_VGA )//VGA_MODE
    {
        if((srcWidth == 640) && (srcHeight == 480))
        {
            Camera_window_table[1] = 0x13;    //0x17;
            Camera_window_table[3] = 0x01;    //0x18
            Camera_window_table[5] = 0xb6;     //0x92;    //0x32 from 0x92 to 0xb6 by lujie @061009
            Camera_window_table[7] = 0x02;    //0x19
            Camera_window_table[9] = 0x7a;    //0x1a
            Camera_window_table[11] = 0x0a;   //0x00;    //0x03 //0x00 from 0x0a to 0xb6 by lujie @061009
            if (camera_set_param(Camera_window_table)  == AK_TRUE)
                return 1;
            else
                return -1;  
        }
        else
        {
            hrefstart = 158 + (640 - srcWidth) /2;   // by lujie 154 to 156 @061009
            hrefstop = hrefstart + srcWidth;

            vrefstart = 8 + (480 - srcHeight) / 2;
            vrefstop = vrefstart + srcHeight; 
        }
    }
    else if(Cammode == CAMERA_MODE_QVGA )//QVGA_MODE
    {
        if((srcWidth == 320) && (srcHeight == 240))
        {
            Camera_window_table[1] = 0x15;    //0x17;
            Camera_window_table[3] = 0x03;    //0x18
            Camera_window_table[5] = 0x36;    //0x32
            Camera_window_table[7] = 0x02;    //0x19
            Camera_window_table[9] = 0x7a;    //0x1a
            Camera_window_table[11]=0x0a;    //0x03     
            if (camera_set_param(Camera_window_table)  == AK_TRUE)
                return 1;
            else
                return -1;                   
        }
        else
        {
            hrefstart = 282 + (320 - srcWidth)/2;//196
            hrefstop = hrefstart + srcWidth;//836

            vrefstart = 8 + (240 - srcHeight)/2 ;
            vrefstop = vrefstart + srcHeight;
        }
    }
    else if(Cammode == CAMERA_MODE_QQVGA )//QQVGA_MODE
    {
        if((srcWidth == 160) && (srcHeight == 120))
        {
            cam_ov7670_set_mode(CAMERA_MODE_QQVGA);
            return 1;
        }
        hrefstart = 282 + (160 - srcWidth)*2;
        hrefstop = hrefstart + srcWidth;

        vrefstart = 8 + (120 - srcHeight)/2 ;
        vrefstop = vrefstart + srcHeight;
    }
    else
    {
        return 0;
    }

    hbit = hrefstart >> 3;         //Horizontal Frame start high 8-bit
    lstartbit = hrefstart & 0x7;   //Horizontal Frame start low 3-bit

    Camera_window_table[1] = hbit;

    if(Cammode != CAMERA_MODE_VGA && Cammode != CAMERA_MODE_SXGA)
    {
        if(hrefstop > 800)hrefstop -=800;
    }
    hbit = hrefstop >> 3;          //Horizontal Frame end high 8-bit
    lstopbit = hrefstop & 0x7;    //Horizontal Frame end low 3-bit

    Camera_window_table[3] = hbit;

    Camera_window_table[5] = 0x80 | lstartbit | (lstopbit << 3) ;

    hbit = vrefstart >> 2;         //Vertical Frame start high 8-bit
    lstartbit = vrefstart & 0x2;  //Vertical Frame start low 2-bit

    Camera_window_table[7] = hbit;

    hbit = vrefstop >> 2;        //Vertical Frame end high 8-bit
    lstopbit = vrefstop & 0x2;  //Vertical Frame end low 2-bit

    Camera_window_table[9] = hbit;

    Camera_window_table[11] = 0x0 | lstartbit | (lstopbit << 2);

    if (camera_set_param(Camera_window_table)  == AK_TRUE)
        return 1;
    else
        return -1;
}

static T_VOID cam_ov7670_set_night_mode(T_NIGHT_MODE mode)
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

T_U32 cam_ov7670_set_framerate(float framerate)
{            
    T_U8 Reg0x92, Reg0x93;
    T_U8 Reg0x9d, Reg0xa5;
    T_U8 Reg0x2a, Reg0x2b;
    T_U8 Reg0x11, Div;
    float dummy_line, mclk;
    //float preview_framerate;    
    float actual_framerate;
    //T_CAMERA_MODE Cammode;    
    T_U16 VGA_maximum_lines = 495;

    Reg0x2a = 0x00;
    Reg0x2b = 0x00;
    sccb_write_data(CAMERA_SCCB_ADDR, 0x2a, &Reg0x2a, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x2b, &Reg0x2b, 1);

    mclk = OV7670_CAMERA_MCLK * 1000000;

    switch(s_ov7670_CurMode)
    {                     
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
            Reg0x9d = (VGA_maximum_lines + dummy_line) * framerate / 100;
            Reg0xa5 = framerate * VGA_maximum_lines / 100;
            break;    
            
        default:                      
            akprintf(C1, M_DRVSYS, "ov7670 unsupport mode %d!\n", s_ov7670_CurMode);
            return 0;
    }
    
    sccb_write_data(CAMERA_SCCB_ADDR, 0x11, &Reg0x11, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x92, &Reg0x92, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x93, &Reg0x93, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0x9d, &Reg0x9d, 1);
    sccb_write_data(CAMERA_SCCB_ADDR, 0xa5, &Reg0xa5, 1);
    
    return 1;
}

static T_BOOL cam_ov7670_set_to_cap(T_U32 srcWidth, T_U32 srcHeight)
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
        akprintf(C1, M_DRVSYS, "ov7670 unsupport %d & %d mode!\n", srcWidth, srcHeight);
        return AK_FALSE;
    }
    
    cam_ov7670_set_mode(Cammode);
    cam_ov7670_set_digital_zoom(srcWidth, srcHeight);    
    mini_delay(200);
    return AK_TRUE;
}

static T_BOOL cam_ov7670_set_to_prev(T_U32 srcWidth, T_U32 srcHeight)
{    
    cam_ov7670_set_mode(CAMERA_MODE_PREV);    
    cam_ov7670_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(200);
    return AK_TRUE;
}

static T_BOOL cam_ov7670_set_to_record(T_U32 srcWidth, T_U32 srcHeight)
{    
    cam_ov7670_set_mode(CAMERA_MODE_REC);
    cam_ov7670_set_digital_zoom(srcWidth, srcHeight);
    mini_delay(200);
    return AK_TRUE;
}

static T_CAMERA_TYPE cam_ov7670_get_type(T_VOID)
{
    return camera_ov7670_type;
} 

static T_CAMERA_FUNCTION_HANDLER ov7670_function_handler = 
{
    OV7670_CAMERA_MCLK,
    cam_ov7670_open,
    cam_ov7670_close,
    cam_ov7670_read_id,
    cam_ov7670_init,
    cam_ov7670_set_mode,
    cam_ov7670_set_exposure,
    cam_ov7670_set_brightness,
    cam_ov7670_set_contrast,
    cam_ov7670_set_saturation,
    cam_ov7670_set_sharpness,
    cam_ov7670_set_AWB,
    cam_ov7670_set_mirror,
    cam_ov7670_set_effect,
    cam_ov7670_set_digital_zoom,
    cam_ov7670_set_night_mode,
    cam_ov7670_set_framerate,
    cam_ov7670_set_to_cap,
    cam_ov7670_set_to_prev,
    cam_ov7670_set_to_record,
    cam_ov7670_get_type
};

static int camera_ov7670_reg(void)
{
    camera_reg_dev(CAMERA_OV7670_ID, &ov7670_function_handler);
    return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_ov7670_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif


