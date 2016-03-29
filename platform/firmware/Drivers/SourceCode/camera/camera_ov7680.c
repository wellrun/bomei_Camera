/**
 * @FILENAME: camera_ov7680.c
 * @BRIEF camera driver file
 * Copyright (C) 2008 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR 
 * @DATE 2008-03-06
 * @VERSION 1.0
 * @REF
 */ 
#include "akdefine.h"
#include "platform_hd_config.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "camera_ov7680.h"


#ifdef USE_CAMERA_OV7680

#define CAM_EN_LEVEL            0	
#define CAM_RESET_LEVEL         0

#define CAMERA_SCCB_ADDR        0x42
#define CAMERA_OV7680_ID        0x7680

#if (defined(CHIP_AK7801) || defined(CHIP_AK7802))
	#define CAMERA_MCLK_DIV         3              //192Mhz/(2*(3+1))=24Mhz
#elif defined(CHIP_AK322L)
	#define CAMERA_MCLK_DIV         15             //90Mhz/15=6Mhz
#else
	#define CAMERA_MCLK_DIV         14             //84Mhz/14=6Mhz
#endif
#define OV7680_CAMERA_MCLK  24

static T_CAMERA_TYPE camera_ov7680_type = CAMERA_P3M;
static T_NIGHT_MODE night_mode = CAMERA_DAY_MODE;
static T_CAMERA_MODE s_ov7680_CurMode = CAMERA_MODE_VGA;

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
				&& !((tabParameter[i] == 0x5f) && (tabParameter[i + 1] & 0x7e)))
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

static T_VOID cam_ov7680_open(T_VOID)
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

static T_BOOL	cam_ov7680_close(T_VOID)
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

static T_U32 cam_ov7680_read_id(T_VOID)
{
	T_U8 value = 0x00;
	T_U32 id = 0;

	sccb_init(GPIO_I2C_SCL, GPIO_I2C_SDA);        //init sccb first here!!
	
	value = sccb_read_data(CAMERA_SCCB_ADDR, 0x0a);
	id = value << 8;
	value = sccb_read_data(CAMERA_SCCB_ADDR, 0x0b);
	id |= value;	
            
	return id;
}

static T_BOOL cam_ov7680_init()
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
 * @param[in] mode: mode value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7680_set_mode(T_CAMERA_MODE mode)
{
    s_ov7680_CurMode = mode;
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
		case CAMERA_MODE_PREV:                              //preview mode
			camera_setup(PREV_MODE_TAB);
			
			if (CAMERA_NIGHT_MODE == night_mode)
			{
				camera_setup(NIGHT_MODE_TAB);
			}
			break;
		case CAMERA_MODE_REC:                              //record mode
			camera_setup(RECORD_MODE_TAB);
			
			if (CAMERA_NIGHT_MODE == night_mode)
			{
				camera_setup(NIGHT_MODE_TAB);
			}
			break;
		default:
            s_ov7680_CurMode = CAMERA_MODE_VGA;
			akprintf(C1, M_DRVSYS, "set camera mode parameter error!\n");
			break;
		}
}

/**
 * @brief: Set camera exposure mode 
 * @author: 
 * @date 2004-09-22
 * @param[in] exposure: exposure mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7680_set_exposure(T_CAMERA_EXPOSURE exposure)
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
 * @param[in] brightness: brightness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7680_set_brightness(T_CAMERA_BRIGHTNESS brightness)
{
	T_U8 temp;
	
	switch(brightness)
	{
		case CAMERA_BRIGHTNESS_0:
			camera_setbit(0xD5, 2, 1);
			camera_setup(BRIGHTNESS_0_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp |= 0x08;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_BRIGHTNESS_1:
			camera_setbit(0xD5, 2, 1);
			camera_setup(BRIGHTNESS_1_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp |= 0x08;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_BRIGHTNESS_2:
			camera_setbit(0xD5, 2, 1);
			camera_setup(BRIGHTNESS_2_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp |= 0x08;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_BRIGHTNESS_3:
			camera_setbit(0xD5, 2, 1);
			camera_setup(BRIGHTNESS_3_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp &= 0xf7;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_BRIGHTNESS_4:
			camera_setbit(0xD5, 2, 1);
			camera_setup(BRIGHTNESS_4_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp &= 0xf7;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_BRIGHTNESS_5:
			camera_setbit(0xD5, 2, 1);
			camera_setup(BRIGHTNESS_5_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp &= 0xf7;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_BRIGHTNESS_6:
			camera_setbit(0xD5, 2, 1);
			camera_setup(BRIGHTNESS_6_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp &= 0xf7;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
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
 * @param[in] contrast: contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7680_set_contrast(T_CAMERA_CONTRAST contrast)
{
	T_U8 temp;

	switch(contrast)
	{
		case CAMERA_CONTRAST_1:
			camera_setup(CONTRAST_1_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp |= 0x04;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_CONTRAST_2:
			camera_setup(CONTRAST_2_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp |= 0x04;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_CONTRAST_3:
			camera_setup(CONTRAST_3_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp |= 0x04;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_CONTRAST_4:
			camera_setup(CONTRAST_4_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp &= 0xfb;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_CONTRAST_5:
			camera_setup(CONTRAST_5_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp &= 0xfb;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_CONTRAST_6:
			camera_setup(CONTRAST_6_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp &= 0xfb;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
			break;
		case CAMERA_CONTRAST_7:
			camera_setup(CONTRAST_7_TAB);
			temp = sccb_read_data(CAMERA_SCCB_ADDR, 0xdf);
			temp &= 0xfb;
			sccb_write_data(CAMERA_SCCB_ADDR, 0xdf, &temp, 1);
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
 * @param[in] saturation: saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7680_set_saturation(T_CAMERA_SATURATION saturation)
{
	switch(saturation)
	{
		case CAMERA_SATURATION_1:
			camera_setbit(0xD5, 1, 1);
			camera_setup(SATURATION_1_TAB);
			break;
		case CAMERA_SATURATION_2:
			camera_setbit(0xD5, 1, 1);
			camera_setup(SATURATION_2_TAB);
			break;
		case CAMERA_SATURATION_3:
			camera_setbit(0xD5, 1, 1);
			camera_setup(SATURATION_3_TAB);
			break;
		case CAMERA_SATURATION_4:
			camera_setbit(0xD5, 1, 1);
			camera_setup(SATURATION_4_TAB);
			break;
		case CAMERA_SATURATION_5:
			camera_setbit(0xD5, 1, 1);
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
 * @param[in] sharpness: sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7680_set_sharpness(T_CAMERA_SHARPNESS sharpness)
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
 * @param[in] awb: AWB mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7680_set_AWB(T_CAMERA_AWB awb)
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
 * @param[in] mirror: mirror mode
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7680_set_mirror(T_CAMERA_MIRROR mirror)
{
	switch(mirror)
	{
		case CAMERA_MIRROR_V:             //flip
			camera_setbit(0x0c, 6, 0);
			camera_setbit(0x0c, 7, 1);
			camera_setbit(0x65, 3, 0);
			break;
		case CAMERA_MIRROR_H:            //mirror
			camera_setbit(0x0c, 6, 1);
			camera_setbit(0x0c, 7, 0);
			camera_setbit(0x65, 3, 1);
			break;
		case CAMERA_MIRROR_NORMAL:
			camera_setbit(0x0c, 6, 0);
			camera_setbit(0x0c, 7, 0);
			camera_setbit(0x65, 3, 0);
			break;
		case CAMERA_MIRROR_FLIP:        //flip and mirror
			camera_setbit(0x0c, 6, 1);
			camera_setbit(0x0c, 7, 1);
			camera_setbit(0x65, 3, 1);
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
static T_VOID cam_ov7680_set_effect(T_CAMERA_EFFECT effect)
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
 * @param[in] width: window width
 * @param[in] height: window height
 * @return int
 * @retval 0 error mode; 1 success; -1 fail
 */
static T_S32 cam_ov7680_set_digital_zoom(T_U32 width, T_U32 height)
{
	T_U16 hrefstart = 0, hrefsize = 0, vrefstart = 0, vrefsize = 0;

	T_U8 Camera_window_table[] =
	{
		0x16, 0x00,
		0x17, 0x00,
		0x18, 0x00,
		0x19, 0x00,
		0x1a, 0x00,

		0xd0, 0x00,
		0xd1, 0x00,
		0xd2, 0x00,
		0xd3, 0x00,
		0xd4, 0x00,
		
		END_FLAG, END_FLAG
	};

	akprintf(C1, M_DRVSYS, "set window size %d, %d\r\n", width, height);

	switch(s_ov7680_CurMode)
	{			
		case CAMERA_MODE_VGA:
			hrefstart = 104 + (640 - width) / 2;        
			hrefsize = 16 + width;
			
			vrefstart = 14 + (480 - height) / 2;   
			vrefsize = 4 + height;
			break;
			
		case CAMERA_MODE_CIF:
			hrefstart = 104 + (640 - 640*width/352) / 2;        
			hrefsize = 656 - (640 - 640*width/352);
			
			vrefstart = 14 + (480 - 480*height/288) / 2;   
			vrefsize = 484 - (480 - 480*height/288);
			break;
			
		case CAMERA_MODE_QVGA:
			hrefstart = 104 + (640 - 640*width/320) / 2;        
			hrefsize = 656 - (640 - 640*width/320);
			
			vrefstart = 14 + (480 - 480*height/240) / 2;   
			vrefsize = 484 - (480 - 480*height/240);
			break;
			
		case CAMERA_MODE_QCIF:
			hrefstart = 104 + (640 - 640*width/176) / 2;        
			hrefsize = 656 - (640 - 640*width/176);
			
			vrefstart = 14 + (480 - 480*height/144) / 2;   
			vrefsize = 484 - (480 - 480*height/144);
			break;
			
		case CAMERA_MODE_QQVGA:
			hrefstart = 104 + (640 - 640*width/160) / 2;        
			hrefsize = 656 - (640 - 640*width/160);
			
			vrefstart = 14 + (480 - 480*height/120) / 2;   
			vrefsize = 484 - (480 - 480*height/120);
			break;
			
		case CAMERA_MODE_PREV:
			hrefstart = 104 + (640 - width) / 2;        
			hrefsize = 16 + width;
			
			vrefstart = 14 + (480 - height) / 2;   
			vrefsize = 4 + height;
			break;

		case CAMERA_MODE_REC:
		#if (defined(CHIP_AK7801) || defined(CHIP_AK7802))
			hrefstart = 104 + (640 - width) / 2;        
			hrefsize = 16 + width;
			
			vrefstart = 14 + (480 - height) / 2;   
			vrefsize = 4 + height;
		#else
			hrefstart = 104 + (640 - 640*width/352) / 2;        
			hrefsize = 656 - (640 - 640*width/352);
			
			vrefstart = 14 + (480 - 480*height/288) / 2;   
			vrefsize = 484 - (480 - 480*height/288);
		#endif
			break;
			
		default:
			akprintf(C1, M_DRVSYS, "unsupported WINDOWING in mode %d!!\n", s_ov7680_CurMode);
			return 0;
	}
		
	Camera_window_table[1] = (hrefstart & 0x03) |((vrefstart & 0x03) << 2) | ((hrefsize & 0x01) << 6);
		
	Camera_window_table[3] = (hrefstart >> 2) & 0xff;	
	Camera_window_table[5] = ((hrefsize / 2) >> 1) & 0xff;
		
	Camera_window_table[7] = (vrefstart >> 2) & 0xff;	
	Camera_window_table[9] = (vrefsize / 2) & 0xff;
	
	Camera_window_table[11] = (hrefsize >> 2) & 0xff;
	Camera_window_table[13] = (vrefsize >> 2) & 0x7f;

	Camera_window_table[15] = (width >> 2) & 0xff;
	Camera_window_table[17] = (height >> 2) & 0x7f;

	Camera_window_table[19] = (height & 0x03) | ((width & 0x03) << 2) | ((vrefsize & 0x03) << 4) | ((hrefsize & 0x03) << 6);

	if (camera_set_param(Camera_window_table) == AK_TRUE)
		return 1;
	else
		return -1;
}

static T_VOID cam_ov7680_set_night_mode(T_NIGHT_MODE mode)
{
	switch(mode)
	{
		case CAMERA_DAY_MODE:
			camera_setup(DAY_MODE_TAB);
			break;
		case CAMERA_NIGHT_MODE:
			camera_setup(NIGHT_MODE_TAB);
			break;
		default:
			akprintf(C1, M_DRVSYS, "set night mode parameter error!\n");
			break;
	}
}

static T_BOOL cam_ov7680_set_to_cap(T_U32 width, T_U32 height)
{	
	float Mclk;
    T_CAMERA_MODE Cammode;

	Mclk = (float)(get_asic_freq() / 1000000) / CAMERA_MCLK_DIV;

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
    else
    {
        akprintf(C1, M_DRVSYS, "ov7680 unsupport %d & %d mode!\n", width, height);
        return AK_FALSE;
    }
	cam_ov7680_set_mode(Cammode);
	cam_ov7680_set_digital_zoom(width, height);	
	mini_delay(300);
	return AK_TRUE;
}

static T_BOOL	cam_ov7680_set_to_prev(T_U32 width, T_U32 height)
{	
	cam_ov7680_set_mode(CAMERA_MODE_PREV);	
	cam_ov7680_set_digital_zoom(width, height);
	mini_delay(300);
	return AK_TRUE;
}

static T_BOOL cam_ov7680_set_to_record(T_U32 width, T_U32 height)
{	
	cam_ov7680_set_mode(CAMERA_MODE_REC);
	cam_ov7680_set_digital_zoom(width, height);
	mini_delay(300);
	return AK_TRUE;
}

static T_CAMERA_TYPE cam_ov7680_get_type(T_VOID)
{
	return camera_ov7680_type;
} 

static T_CAMERA_FUNCTION_HANDLER ov7680_function_handler = 
{
	OV7680_CAMERA_MCLK,
	cam_ov7680_open,
	cam_ov7680_close,
	cam_ov7680_read_id,
	cam_ov7680_init,
	cam_ov7680_set_mode,
	cam_ov7680_set_exposure,
	cam_ov7680_set_brightness,
	cam_ov7680_set_contrast,
	cam_ov7680_set_saturation,
	cam_ov7680_set_sharpness,
	cam_ov7680_set_AWB,
	cam_ov7680_set_mirror,
	cam_ov7680_set_effect,
	cam_ov7680_set_digital_zoom,
	cam_ov7680_set_night_mode,
	cam_ov7680_set_to_cap,
	cam_ov7680_set_to_prev,
	cam_ov7680_set_to_record,
	cam_ov7680_get_type
};

static int camera_ov7680_reg(void)
{
	camera_reg_dev(CAMERA_OV7680_ID, &ov7680_function_handler);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_ov7680_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif

