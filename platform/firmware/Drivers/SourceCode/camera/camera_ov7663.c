/**
 * @FILENAME: camera_ov7663.c
 * @BRIEF camera driver file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR 
 * @DATE 2007-10-29
 * @VERSION 1.0
 * @REF
 */ 
#include "akdefine.h"
#include "platform_hd_config.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "camera_ov7663.h"


#ifdef USE_CAMERA_OV7663

#define CAM_EN_LEVEL		0	
#define CAM_RESET_LEVEL		1

#define CAMERA_SCCB_ADDR	0x42
#define CAMERA_OV7663_ID	0x7660
#define CAMERA_MCLK_DIV     3    //6
#define OV7663_CAMERA_MCLK  24

static T_CAMERA_TYPE camera_ov7663_type = CAMERA_P3M;
static T_CAMERA_MODE s_ov7663_CurMode = CAMERA_MODE_VGA;

static T_VOID camera_setup(const T_U8 para_table[])
{	
	int i;
	T_U8 reg, cmd;

	for (i=0; ;i++)
	{
    		reg = para_table[2 * i];
    		cmd = para_table[2 * i + 1];

    		if ((END_FLAG == reg) && (END_FLAG == cmd))
    			break;
        	sccb_write_data(CAMERA_SCCB_ADDR, reg, &cmd, 1);
	}
}

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
	T_U8 value;

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

			value = 0xff;
			value = sccb_read_data(CAMERA_SCCB_ADDR, tabParameter[i]);


			if(((tabParameter[i] == 0x17) && (value != tabParameter[i + 1]))
				||((tabParameter[i] == 0x1a) && (value != tabParameter[i + 1])))
			{
				akprintf(C1, M_DRVSYS, "set parameter error!\r\n");
				akprintf(C1, M_DRVSYS, "%x", i);
				akprintf(C1, M_DRVSYS, "reg = 0x%x, ", tabParameter[i]);
				akprintf(C1, M_DRVSYS, "data = 0x%x, ", tabParameter[i + 1]);
				akprintf(C1, M_DRVSYS, "read = 0x%x\n", value);

				return AK_FALSE;
			}			
		}

		i += 2;
	}

	return AK_TRUE;
}

static T_VOID cam_ov7663_open(T_VOID)
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

static T_BOOL cam_ov7663_close(T_VOID)
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

static T_U32 cam_ov7663_read_id(T_VOID)
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

/**
 * @brief: initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author: 
 * @date 2008-01-24
 * @return int
 * @retval 0 success; 1 fail
 */
static T_BOOL cam_ov7663_init(T_VOID)
{
	if(!camera_set_param(RESET_TAB))
	{
		return AK_FALSE;
	}
	return AK_TRUE;
}

/**
 * @brief: Set camera mode to specify image quality, SXGA/VGA/CIF/ etc 
 * @author: 
 * @date 2004-09-22
 * @param[in] mode: mode value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_ov7663_set_mode(T_CAMERA_MODE mode)
{
    s_ov7663_CurMode = mode;
	switch(mode)
	{
		case CAMERA_MODE_SXGA:
			camera_setup(SXGA_MODE_TAB);
			break;
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
		default:
            s_ov7663_CurMode = CAMERA_MODE_VGA;
			akprintf(C2, M_DRVSYS, "set camera mode parameter error!\n");
			return;
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
static T_VOID cam_ov7663_set_exposure(T_CAMERA_EXPOSURE exposure)
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
			akprintf(C2, M_DRVSYS, "set exposure parameter error!\n");
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
static T_VOID cam_ov7663_set_brightness(T_CAMERA_BRIGHTNESS brightness)
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
			akprintf(C2, M_DRVSYS, "set brightness parameter error!\n");
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
static T_VOID cam_ov7663_set_contrast(T_CAMERA_CONTRAST contrast)
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
			akprintf(C2, M_DRVSYS, "set contrast parameter error!\n");
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
static T_VOID cam_ov7663_set_saturation(T_CAMERA_SATURATION saturation)
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
			akprintf(C2, M_DRVSYS, "set saturation parameter error!\n");
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
static T_VOID cam_ov7663_set_sharpness(T_CAMERA_SHARPNESS sharpness)
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
			akprintf(C2, M_DRVSYS, "set sharpness parameter error!\n");
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
static T_VOID cam_ov7663_set_AWB(T_CAMERA_AWB awb)
{
	switch(awb)
	{
		case AWB_AUTO:
			cam_ov7663_set_contrast(CAMERA_CONTRAST_4);
			cam_ov7663_set_saturation(CAMERA_SATURATION_4);
			cam_ov7663_set_brightness(CAMERA_BRIGHTNESS_3);
			camera_setup(AWB_AUTO_TAB);
			break;
		case AWB_SUNNY:
			cam_ov7663_set_contrast(CAMERA_CONTRAST_5);
			cam_ov7663_set_saturation(CAMERA_SATURATION_5);
			cam_ov7663_set_brightness(CAMERA_BRIGHTNESS_3);
			camera_setup(AWB_SUNNY_TAB);
			break;
		case AWB_CLOUDY:
			cam_ov7663_set_contrast(CAMERA_CONTRAST_5);
			cam_ov7663_set_saturation(CAMERA_SATURATION_4);
			cam_ov7663_set_brightness(CAMERA_BRIGHTNESS_3);
			camera_setup(AWB_CLOUDY_TAB);
			break;
		case AWB_OFFICE:
			cam_ov7663_set_contrast(CAMERA_CONTRAST_4);
			cam_ov7663_set_saturation(CAMERA_SATURATION_4);
			cam_ov7663_set_brightness(CAMERA_BRIGHTNESS_3);
			camera_setup(AWB_OFFICE_TAB);
			break;
		case AWB_HOME:
			cam_ov7663_set_contrast(CAMERA_CONTRAST_4);
			cam_ov7663_set_saturation(CAMERA_SATURATION_3);
			cam_ov7663_set_brightness(CAMERA_BRIGHTNESS_3);
			camera_setup(AWB_HOME_TAB);
			break;
		case AWB_NIGHT:
			cam_ov7663_set_contrast(CAMERA_CONTRAST_4);
			cam_ov7663_set_saturation(CAMERA_SATURATION_2);
			cam_ov7663_set_brightness(CAMERA_BRIGHTNESS_5);
			camera_setup(AWB_NIGHT_TAB);
			break;
		default:
			akprintf(C2, M_DRVSYS, "set AWB mode parameter error!\n");
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
static T_VOID cam_ov7663_set_mirror(T_CAMERA_MIRROR mirror)
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
			akprintf(C2, M_DRVSYS, "set mirror parameter error!\n");
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
static T_VOID cam_ov7663_set_effect(T_CAMERA_EFFECT effect)
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
			akprintf(C2, M_DRVSYS, "set camer effect parameter error!\n");
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
static T_S32 cam_ov7663_set_focus(T_U32 width, T_U32 height)
{ 
	unsigned int hrefstart = 0, hrefstop = 0, vrefstart = 0 , vrefstop = 0;

	unsigned char hbit = 0, lstartbit = 0 , lstopbit = 0;
 	T_CAMERA_MODE Cammode = s_ov7663_CurMode;
	unsigned char Camera_window_table[14] =
	{
		0x17, 0,
		0x18, 0,
		0x32, 0,
		0x19, 0,
		0x1A, 0,
		0x03, 0,
		END_FLAG, END_FLAG
	};

	if(Cammode == CAMERA_MODE_VGA || Cammode == CAMERA_MODE_PREV )//VGA_MODE
	{
		if((width == 640) && (height == 480))
		{
			//CameraSetParameter( camera_reset_para );
			//delay1ms(25000);
			Camera_window_table[1] = 0x13;	//0x17;
			Camera_window_table[3] = 0x01;	//0x18
			Camera_window_table[5] = 0x92;	//0x32
			Camera_window_table[7] = 0x02;	//0x19
			Camera_window_table[9] = 0x7a;	//0x1a
			Camera_window_table[11] = 0x00;	//0x03
			if (camera_set_param(Camera_window_table)  == AK_TRUE)
	  			return 1;
			else
				return -1;  
		}

		hrefstart = 154 + (640 - width) /2;
		hrefstop = hrefstart + width;
		//hrefstop = hrefstop%800;
		//hrefstop = 823;
		vrefstart = 8 + (480 - height) / 2;
		vrefstop = vrefstart + height;
		//akprintf(C1, M_DRVSYS, "vga window\n");
	}
	else if(Cammode == CAMERA_MODE_QVGA )//QVGA_MODE
	{
		if((width == 320) && (height == 240))
		{
			//delay1ms(25000);
			Camera_window_table[1] = 0x23;	//0x17;
			Camera_window_table[3] = 0x4b;	//0x18
			Camera_window_table[5] = 0xbf;	//0x32
			Camera_window_table[7] = 0x02;	//0x19
			 Camera_window_table[9] = 0x7a;	//0x1a
			Camera_window_table[11] = 0x00;	//0x03	 
			if (camera_set_param(Camera_window_table)  == AK_TRUE)
	  			return 1;
			else
				return -1;        	   	
		}
		hrefstart = 282 + (320 - width)/2;//196
		hrefstop = hrefstart + width;//836

		vrefstart = 8 + (240 - height)/2 ;

		vrefstop = vrefstart + height;
	}
	else if(Cammode == CAMERA_MODE_QQVGA )//QQVGA_MODE
	{
	   if((width == 160) && (height == 120))
	   	{
	   		return;
	   	}
	   hrefstart = 282 + (160 - width)*2;
	   hrefstop = hrefstart + width;

	   vrefstart = 8 + (120 - height)/2 ;
	   vrefstop = vrefstart + height;
	}
	else
	{
		return 0;
	}

	hbit = hrefstart >> 3;//Horizontal Frame start high 8-bit
    lstartbit = hrefstart & 0x7;//Horizontal Frame start low 3-bit

    Camera_window_table[1] = hbit;

	  if(Cammode != CAMERA_MODE_VGA && Cammode != CAMERA_MODE_SXGA)
	  {
    if(hrefstop > 800)hrefstop -=800;
    }
    hbit = hrefstop >> 3;//Horizontal Frame end high 8-bit
    lstopbit = hrefstop & 0x7;//Horizontal Frame end low 3-bit

    Camera_window_table[3] = hbit;

    Camera_window_table[5] = 0x80 | lstartbit | (lstopbit << 3) ;

    hbit = vrefstart >> 2;//Vertical Frame start high 8-bit
    lstartbit = vrefstart & 0x2;//Vertical Frame start low 2-bit

    Camera_window_table[7] = hbit;

    hbit = vrefstop >> 2;//Vertical Frame end high 8-bit
    lstopbit = vrefstop & 0x2;//Vertical Frame end low 2-bit

    Camera_window_table[9] = hbit;

    Camera_window_table[11] = 0x0 | lstartbit | (lstopbit << 2);

    for (hbit=0 ; hbit<12; hbit+=2)
        akprintf(C3, M_DRVSYS, "%x: 0x%x\n", Camera_window_table[hbit], Camera_window_table[hbit+1]);
        
	if (camera_set_param(Camera_window_table) == AK_TRUE)
	    return 1;
	else
		return -1;
}

static T_VOID cam_ov7663_set_night_mode(T_NIGHT_MODE mode)
{
/*
	switch(mode)
	{
		case CAMERA_NIGHT_MODE:
				camera_setup(NIGHT_MODE_TAB);
			break;
		case CAMERA_DAY_MODE:
				camera_setup(DAY_MODE_TAB);
			break;
		default:
			break;
	}
	*/
	cam_ov7663_set_contrast(CAMERA_CONTRAST_4);
	cam_ov7663_set_saturation(CAMERA_SATURATION_2);
	cam_ov7663_set_brightness(CAMERA_BRIGHTNESS_5);
	camera_setup(AWB_NIGHT_TAB);           
}

static T_BOOL cam_ov7663_set_to_cap(T_U32 width, T_U32 height)
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
    else if ((width <= 1280) && (height <= 1024))
    {
        Cammode = CAMERA_MODE_SXGA;
    }
    else
    {
        akprintf(C1, M_DRVSYS, "ov7663 unsupport %d & %d mode!\n", width, height);
        return AK_FALSE;
    }
	if(Cammode >= CAMERA_MODE_VGA)
	{
		akprintf(C3, M_DRVSYS, "cam_ov7663_set_to_cap1\n");

		cam_ov7663_set_mode(CAMERA_MODE_VGA);
		cam_ov7663_set_focus(width, height);
	}
	else
	{
		akprintf(C3, M_DRVSYS, "cam_ov7663_set_to_cap2\n");	
		cam_ov7663_set_mode(CAMERA_MODE_SXGA);
		cam_ov7663_set_focus(width, height);	
	}

	return AK_TRUE;
}

static T_BOOL cam_ov7663_set_to_prev(T_U32 width, T_U32 height)
{
	cam_ov7663_set_mode(CAMERA_MODE_VGA);
	cam_ov7663_set_focus(width, height);
	
	return AK_TRUE;
}

static T_BOOL cam_ov7663_set_to_record(T_U32 width, T_U32 height)
{	
	cam_ov7663_set_mode(CAMERA_MODE_CIF);
	cam_ov7663_set_focus(width, height);
	return AK_TRUE;
}

static T_CAMERA_TYPE cam_ov7663_get_type(T_VOID)
{
	return camera_ov7663_type;
} 

static T_CAMERA_FUNCTION_HANDLER ov7663_function_handler = 
{
	OV7663_CAMERA_MCLK,
	cam_ov7663_open,
	cam_ov7663_close,
	cam_ov7663_read_id,
	cam_ov7663_init,
	cam_ov7663_set_mode,
	cam_ov7663_set_exposure,
	cam_ov7663_set_brightness,
	cam_ov7663_set_contrast,
	cam_ov7663_set_saturation,
	cam_ov7663_set_sharpness,
	cam_ov7663_set_AWB,
	cam_ov7663_set_mirror,
	cam_ov7663_set_effect,
	cam_ov7663_set_focus,
	cam_ov7663_set_night_mode,
	cam_ov7663_set_to_cap,
	cam_ov7663_set_to_prev,
	cam_ov7663_set_to_record,
	cam_ov7663_get_type
};

static int camera_ov7663_reg(void)
{
	camera_reg_dev(CAMERA_OV7663_ID, &ov7663_function_handler);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_ov7663_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif

