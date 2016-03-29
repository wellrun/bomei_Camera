/**
 * @FILENAME: camera_SIM120B.c
 * @BRIEF camera driver file
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR 
 * @DATE 2008-08-07
 * @VERSION 1.0
 * @REF
 */ 
#include "akdefine.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "camera_SIM120B.h"
#include "platform_hd_config.h"


#ifdef USE_CAMERA_SIM120B

#define CAM_EN_LEVEL		1	
#define CAM_RESET_LEVEL		0

#define CAMERA_SCCB_ADDR        0x6a
#define CAMERA_SIM120B_ID       0x12
#define CAMERA_MCLK_DIV         4             //90Mhz/4=22.5Mhz
#define SIM120B_CAMERA_MCLK     24

	
static T_CAMERA_TYPE camera_SIM120B_type = CAMERA_1P3M;
static T_NIGHT_MODE night_mode = CAMERA_DAY_MODE;
static T_CAMERA_MIRROR mirror_type = CAMERA_MIRROR_NORMAL;
static T_CAMERA_MODE s_SIM120B_CurMode = CAMERA_MODE_VGA;

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

/*			if (!((tabParameter[i] == 0x12) && (tabParameter[i + 1] & 0x80)))
			{				
				temp_value = sccb_read_data(CAMERA_SCCB_ADDR, tabParameter[i]);
				if (temp_value != tabParameter[i + 1])
				{
					akprintf(C1, M_DRVSYS, "set parameter error!\n");
					akprintf(C1, M_DRVSYS, "reg 0x%x write data is 0x%x, read data is 0x%x!\n", tabParameter[i], tabParameter[i + 1], temp_value);

					return AK_FALSE;
				}
			}
*/
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

static T_VOID cam_SIM120B_open(T_VOID)
{
	gpio_set_pin_dir(GPIO_CAMERA_AVDD, GPIO_DIR_OUTPUT);
	gpio_set_pin_level(GPIO_CAMERA_AVDD, gpio_pin_get_ActiveLevel(GPIO_CAMERA_AVDD));   

	gpio_set_pin_dir(GPIO_CAMERA_CHIP_ENABLE, GPIO_DIR_OUTPUT);
	gpio_set_pin_level(GPIO_CAMERA_CHIP_ENABLE, CAM_EN_LEVEL);	
	mini_delay(10);
	
	gpio_set_pin_dir(GPIO_CAMERA_RESET, GPIO_DIR_OUTPUT);
	gpio_set_pin_level(GPIO_CAMERA_RESET, CAM_RESET_LEVEL);
	mini_delay(10);
	gpio_set_pin_level(GPIO_CAMERA_RESET, !CAM_RESET_LEVEL);

	mini_delay(20);
}

static T_BOOL cam_SIM120B_close(T_VOID)
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

static T_U32 cam_SIM120B_read_id(T_VOID)
{	
	T_U32 id = 0;
	T_U8 tmp = 0;

	sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);        //init sccb first here!!

	sccb_write_data(CAMERA_SCCB_ADDR, 0x00, &tmp, 1);
	id = sccb_read_data(CAMERA_SCCB_ADDR, 0x01);	
            
	return id;
}

/**
 * @brief: initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author: 
 * @date 2007-09-14
 * @return int
 * @retval 0 success; 1 fail
 */
static T_BOOL cam_SIM120B_init(T_VOID)
{
	if (!camera_set_param(INIT_TAB))
	{
		return AK_FALSE;
	}
	else
	{		
		night_mode = CAMERA_DAY_MODE;
		mirror_type = CAMERA_MIRROR_NORMAL;
		return AK_TRUE;
	}		
}

/**
 * @brief: Set camera mode to specify image quality, SXGA/VGA/CIF/ etc 
 * @author: 
 * @date 2007-09-14
 * @param[in] mode: mode value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_SIM120B_set_mode(T_CAMERA_MODE mode)
{
    s_SIM120B_CurMode = mode;
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
		case CAMERA_MODE_PREV:                              //preview mode
			camera_setup(PREV_MODE_TAB);
			
			if (CAMERA_NIGHT_MODE == night_mode)
			{
				camera_setup(NIGHT_MODE_TAB);
			}
			break;
		case CAMERA_MODE_REC:                              //record mode
			camera_setup(RECORD_MODE_TAB);
			break;
		default:
            s_SIM120B_CurMode = CAMERA_MODE_VGA;
			akprintf(C1, M_DRVSYS, "set camera mode parameter error!\n");
			break;
	}
}

/**
 * @brief: Set camera exposure mode 
 * @author: 
 * @date 2007-09-14
 * @param[in] exposure: exposure mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_SIM120B_set_exposure(T_CAMERA_EXPOSURE exposure)
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
 * @date 2007-09-14
 * @param[in] brightness: brightness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_SIM120B_set_brightness(T_CAMERA_BRIGHTNESS brightness)
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
 * @date 2007-09-14
 * @param[in] contrast: contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_SIM120B_set_contrast(T_CAMERA_CONTRAST contrast)
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
 * @date 2007-09-14
 * @param[in] saturation: saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_SIM120B_set_saturation(T_CAMERA_SATURATION saturation)
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
 * @date 2007-09-14
 * @param[in] sharpness: sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_SIM120B_set_sharpness(T_CAMERA_SHARPNESS sharpness)
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
 * @date 2007-09-14
 * @param[in] awb: AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_SIM120B_set_AWB(T_CAMERA_AWB awb)
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
 * @date 2007-09-14
 * @param[in] mirror: mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_SIM120B_set_mirror(T_CAMERA_MIRROR mirror)
{
	T_U8 reg_data = 0x00;
	
	switch(mirror)
	{
		case CAMERA_MIRROR_V:
			sccb_write_data(CAMERA_SCCB_ADDR, 0x00, &reg_data, 1);
			camera_setbit(0x04, 0, 0);
			camera_setbit(0x04, 1, 1);

			mirror_type = CAMERA_MIRROR_V;
			break;	
		case CAMERA_MIRROR_H:
			sccb_write_data(CAMERA_SCCB_ADDR, 0x00, &reg_data, 1);
			camera_setbit(0x04, 0, 1);

			mirror_type = CAMERA_MIRROR_H;
			break;
		case CAMERA_MIRROR_NORMAL:
			sccb_write_data(CAMERA_SCCB_ADDR, 0x00, &reg_data, 1);
			camera_setbit(0x04, 1, 0);
			camera_setbit(0x04, 0, 0);

			mirror_type = CAMERA_MIRROR_NORMAL;
			break;	
		case CAMERA_MIRROR_FLIP:
			sccb_write_data(CAMERA_SCCB_ADDR, 0x00, &reg_data, 1);
			camera_setbit(0x04, 1, 1);
			camera_setbit(0x04, 0, 1);

			mirror_type = CAMERA_MIRROR_FLIP;
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
 * @param[in] effect: effect mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_SIM120B_set_effect(T_CAMERA_EFFECT effect)
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
 * @date 2007-09-14
 * @param[in] width: window width
 * @param[in] height: window height
 * @return int
 * @retval 0 error mode; 1 success; -1 fail
 */
static T_S32 cam_SIM120B_set_digital_zoom(T_U32 width, T_U32 height)
{	
    T_U16 hrefstart = 0, vrefstart = 0;

    T_U8 Camera_window_table[] =
    {
        0x00, 0x03,
        0x90, 0x00,
        0x91, 0x00,
        0x92, 0x00, 
        0x93, 0x00, 
        0x94, 0x00, 
        0x95, 0x00, 
        0x96, 0x00, 
        0x97, 0x00,
        END_FLAG, END_FLAG
    };

    akprintf(C1, M_DRVSYS, "set window size %d, %d\r\n", width, height);

	switch(s_SIM120B_CurMode)
    {
        case CAMERA_MODE_SXGA:
            hrefstart = (1280 - width) / 2;
            vrefstart = (1024 - height) / 2;
            break;

        case CAMERA_MODE_VGA:
            hrefstart = (640 - width) / 2;
            vrefstart = (480 - height) / 2;
            break;

        case CAMERA_MODE_CIF:
            hrefstart = (352 - width) / 2;
            vrefstart = (288 - height) / 2;
            break;

        case CAMERA_MODE_QVGA:
            hrefstart = (320 - width) / 2;
            vrefstart = (240 - height) / 2;
            break;

        case CAMERA_MODE_QCIF:
            hrefstart = (176 - width) / 2;
            vrefstart = (144 - height) / 2;
            break;

    //    case CAMERA_MODE_QQVGA:
    //        break;

        case CAMERA_MODE_PREV:
            hrefstart = (640 - width) / 2;
            vrefstart = (480 - height) / 2;
            break;

        case CAMERA_MODE_REC:                 //CIF
            hrefstart = (352 - width) / 2;
            vrefstart = (288 - height) / 2;
            break;

        default:
            akprintf(C1, M_DRVSYS, "unsupported WINDOWING in mode %d!!\n", s_SIM120B_CurMode);
            return 0;
    }
	
    Camera_window_table[3] = (hrefstart >> 8) & 0x07;    //0x90
    Camera_window_table[5] = hrefstart & 0xff;               //0x91

    Camera_window_table[7] = (vrefstart >> 8) & 0x07;    //0x92
    Camera_window_table[9] = vrefstart & 0xff;               //0x93

    Camera_window_table[11] = (width >> 8) & 0x07;     //0x94
    Camera_window_table[13] = width & 0xff;                //0x95

    Camera_window_table[15] = (height >> 8) & 0x07;    //0x96
    Camera_window_table[17] = height & 0xff;               //0x97

    if (camera_set_param(Camera_window_table)  == AK_TRUE)
        return 1;
    else
        return -1;
}

static T_VOID cam_SIM120B_set_night_mode(T_NIGHT_MODE mode)
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

static T_VOID start_preview(T_CAMERA_MODE Cammode)
{
	//Change to preview mode
	cam_SIM120B_set_mode(Cammode);
	cam_SIM120B_set_mirror(mirror_type);
}

static T_VOID start_capture(T_CAMERA_MODE Cammode)
{	
	//Write registers, change to QXGA resolution.
	cam_SIM120B_set_mode(Cammode);
	cam_SIM120B_set_mirror(mirror_type);
}

static T_VOID start_record(T_CAMERA_MODE Cammode)
{
	//Change to record mode
	cam_SIM120B_set_mode(Cammode);
	cam_SIM120B_set_mirror(mirror_type);
}

static T_BOOL cam_SIM120B_set_to_cap(T_U32 width, T_U32 height, T_CAMERA_MODE Cammode , T_U16 Zoomlevel)
{		
    T_CAMERA_MODE Cammode;

    if ((width <= 160) && (height <= 120))
    {
        Cammode = CAMERA_MODE_QQVGA;
    }
    else if ((width <= 176) && (height <= 144))
    {
        Cammode = CAMERA_MODE_QCIF;
    }
    else if ((width <= 320) && (height <= 240))
    {
        Cammode = CAMERA_MODE_QVGA;
    }
    else if ((width <= 352) && (height <= 288))
    {
        Cammode = CAMERA_MODE_CIF;
    }
    else if ((width <= 640) && (height <= 480))
    {
        Cammode = CAMERA_MODE_VGA;
    }
    else if ((width <= 1280) && (height <= 1024))
    {
        Cammode = CAMERA_MODE_SXGA;
    }
    else
    {
        akprintf(C1, M_DRVSYS, "SIM120B unsupport %d & %d mode!\n", width, height);
        return AK_FALSE;
    }
    
	start_capture(Cammode);	
	cam_SIM120B_set_digital_zoom(width, height);
	mini_delay(200);
	return AK_TRUE; 
}

static T_BOOL cam_SIM120B_set_to_prev(T_U32 width, T_U32 height)
{
	start_preview(CAMERA_MODE_PREV);
	cam_SIM120B_set_digital_zoom(width, height);
	mini_delay(200);
	return AK_TRUE;
}

static T_BOOL cam_SIM120B_set_to_record(T_U32 width, T_U32 height)
{	
	start_record(CAMERA_MODE_REC);
	cam_SIM120B_set_digital_zoom(width, height);
	mini_delay(200);
	return AK_TRUE;
}

static T_CAMERA_TYPE cam_SIM120B_get_type(T_VOID)
{
	return camera_SIM120B_type;
}

static T_CAMERA_FUNCTION_HANDLER SIM120B_function_handler = 
{
	SIM120B_CAMERA_MCLK,
	cam_SIM120B_open,
	cam_SIM120B_close,
	cam_SIM120B_read_id,
	cam_SIM120B_init,
	cam_SIM120B_set_mode,
	cam_SIM120B_set_exposure,
	cam_SIM120B_set_brightness,
	cam_SIM120B_set_contrast,
	cam_SIM120B_set_saturation,
	cam_SIM120B_set_sharpness,
	cam_SIM120B_set_AWB,
	cam_SIM120B_set_mirror,
	cam_SIM120B_set_effect,
	cam_SIM120B_set_digital_zoom,
	cam_SIM120B_set_night_mode,
	cam_SIM120B_set_to_cap,
	cam_SIM120B_set_to_prev,
	cam_SIM120B_set_to_record,
	cam_SIM120B_get_type
};

static int camera_SIM120B_reg(void)
{
	camera_reg_dev(CAMERA_SIM120B_ID, &SIM120B_function_handler);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_SIM120B_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif

