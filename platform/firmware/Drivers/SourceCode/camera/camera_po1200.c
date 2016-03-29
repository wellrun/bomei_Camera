/**
 * @FILENAME: camera_po1200.c
 * @BRIEF camera driver file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR 
 * @DATE 2007-10-29
 * @VERSION 1.0
 * @REF
 */ 
#include "akdefine.h"
#include "drv_api.h"
#include "drv_gpio.h"
#include "camera_po1200.h"
#include "platform_hd_config.h"


#ifdef USE_CAMERA_PO1200

#define CAM_EN_LEVEL		1	
#define CAM_RESET_LEVEL		0

#define CAMERA_I2C_ADDR		0xB8
#define CAMERA_PO1200_ID	0x1200
#define CAMERA_MCLK_DIV     3    //6
#define PO1200_CAMERA_MCLK  24

static T_CAMERA_TYPE camera_po1200_type = CAMERA_1P3M;
static T_CAMERA_MODE s_po1200_CurMode = CAMERA_MODE_VGA;


static T_VOID camera_setbit(T_U8 reg, T_U8 bit, T_U8 value)
{		
	T_U8 tmp;

	i2c_read_data(CAMERA_I2C_ADDR, reg, &tmp, 1);
	if (value == 1)
		tmp |= 0x1 << bit;
	else
		tmp &= ~(0x1 << bit);
	i2c_write_data(CAMERA_I2C_ADDR, reg, &tmp, 1);
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
			i2c_write_data(CAMERA_I2C_ADDR, tabParameter[i], (T_U8 *)&tabParameter[i + 1], 1);

			if(i == 0)
			{
				value = 0xff;
				i2c_read_data(CAMERA_I2C_ADDR, tabParameter[i], &value, 1);
	
	
				if (value != 0)
				{
					akprintf(C1, M_DRVSYS, "set parameter error!\r\n");
					akprintf(C1, M_DRVSYS, "%x", i);
					akprintf(C1, M_DRVSYS, "reg = 0x%x, ", tabParameter[i]);
					akprintf(C1, M_DRVSYS, "data = 0x%x, ", tabParameter[i + 1]);
					akprintf(C1, M_DRVSYS, "read = 0x%x\n", value);
	
					return AK_FALSE;
				}
			}			
		}

		i += 2;
	}

	return AK_TRUE;
}

static T_VOID camera_setup(const T_U8 para_table[])
{
       camera_set_param(para_table);
}

static T_VOID cam_po1200_open(T_VOID)
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

static T_BOOL cam_po1200_close(T_VOID)
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

static T_U32 cam_po1200_read_id(T_VOID)
{
	T_U8 value = 0x0;
	T_U32 id = 0;

	i2c_init(GPIO_I2C_SCL, GPIO_I2C_SDA);
    
 	i2c_read_data(CAMERA_I2C_ADDR, 0x00, &value, 1);
 	id = value << 8;	
 	i2c_read_data(CAMERA_I2C_ADDR, 0x01, &value, 1);	
	id |= value;	

	return id;
}

/**
 * @brief: initialize the parameters of camera, should be done after reset and open camera to initialize   
 * @author: 
 * @date 2004-09-22
 * @return int
 * @retval 0 success; -1 fail
 */
static T_BOOL cam_po1200_init(T_VOID)
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
static T_VOID cam_po1200_set_mode(T_CAMERA_MODE mode)
{
    s_po1200_CurMode = mode;
	cam_set_mirror(CAMERA_MIRROR_NORMAL);
	switch(mode)
	{	
		case CAMERA_MODE_UXGA:
			camera_setup(UXGA_MODE_TAB);
			break;	
		case CAMERA_MODE_SXGA:
			camera_setup(SXGA_MODE_TAB);
			break;
		case CAMERA_MODE_SVGA:
			camera_setup(SVGA_MODE_TAB);
			break;			
		case CAMERA_MODE_VGA:
			camera_setup(VGA_MODE_TAB);
			break;
		case CAMERA_MODE_QSVGA:
			camera_setup(QSVGA_MODE_TAB);
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
            s_po1200_CurMode = CAMERA_MODE_VGA;
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
static T_VOID cam_po1200_set_exposure(T_CAMERA_EXPOSURE exposure)
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
static T_VOID cam_po1200_set_brightness(T_CAMERA_BRIGHTNESS brightness)
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
 * @param[in] contrast: contrast value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_po1200_set_contrast(T_CAMERA_CONTRAST contrast)
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
 * @param[in] saturation: saturation value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_po1200_set_saturation(T_CAMERA_SATURATION saturation)
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
 * @param[in] sharpness: sharpness value
 * @return T_VOID
 * @retval
 */
static T_VOID cam_po1200_set_sharpness(T_CAMERA_SHARPNESS sharpness)
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
static T_VOID cam_po1200_set_AWB(T_CAMERA_AWB awb)
{
	switch(awb)
	{
		case AWB_AUTO:
			cam_po1200_set_contrast(CAMERA_CONTRAST_4);
			cam_po1200_set_saturation(CAMERA_SATURATION_4);
			cam_po1200_set_brightness(CAMERA_BRIGHTNESS_3);
			camera_setup(AWB_AUTO_TAB);
			break;
		case AWB_SUNNY:
			cam_po1200_set_contrast(CAMERA_CONTRAST_5);
			cam_po1200_set_saturation(CAMERA_SATURATION_5);
			cam_po1200_set_brightness(CAMERA_BRIGHTNESS_3);
			camera_setup(AWB_SUNNY_TAB);
			break;
		case AWB_CLOUDY:
			cam_po1200_set_contrast(CAMERA_CONTRAST_5);
			cam_po1200_set_saturation(CAMERA_SATURATION_4);
			cam_po1200_set_brightness(CAMERA_BRIGHTNESS_3);
			camera_setup(AWB_CLOUDY_TAB);
			break;
		case AWB_OFFICE:
			cam_po1200_set_contrast(CAMERA_CONTRAST_4);
			cam_po1200_set_saturation(CAMERA_SATURATION_4);
			cam_po1200_set_brightness(CAMERA_BRIGHTNESS_3);
			camera_setup(AWB_OFFICE_TAB);
			break;
		case AWB_HOME:
			cam_po1200_set_contrast(CAMERA_CONTRAST_4);
			cam_po1200_set_saturation(CAMERA_SATURATION_3);
			cam_po1200_set_brightness(CAMERA_BRIGHTNESS_3);
			camera_setup(AWB_HOME_TAB);
			break;
		case AWB_NIGHT:
			cam_po1200_set_contrast(CAMERA_CONTRAST_4);
			cam_po1200_set_saturation(CAMERA_SATURATION_2);
			cam_po1200_set_brightness(CAMERA_BRIGHTNESS_5);
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
static T_VOID cam_po1200_set_mirror(T_CAMERA_MIRROR mirror)
{
	switch(mirror)
	{
		case CAMERA_MIRROR_V:
				camera_setup(MIRROR_SET_TAB);
				camera_setbit(0x1d, 7, 1);
				camera_setbit(0x1e, 6, 1);
				mini_delay(500);
				camera_setbit(0x1d, 7, 0);
				camera_setup(MIRROR_RESET_TAB);
				break;	
		case CAMERA_MIRROR_H:
				camera_setup(MIRROR_SET_TAB);
				camera_setbit(0x1d, 7, 1);
				camera_setbit(0x1e, 7, 1);
				mini_delay(500);
				camera_setbit(0x1d, 7, 0);
				camera_setup(MIRROR_RESET_TAB);
				break;
		case CAMERA_MIRROR_NORMAL:
				camera_setup(MIRROR_SET_TAB);
			       camera_setbit(0x1d, 7, 1);
				camera_setbit(0x1e, 6, 0);
				camera_setbit(0x1e, 7, 0);
				mini_delay(500);
				camera_setbit(0x1d, 7, 0);
				camera_setup(MIRROR_RESET_TAB);
				break;	
		case CAMERA_MIRROR_FLIP:
				camera_setup(MIRROR_SET_TAB);
				camera_setbit(0x1d, 7, 1);
				camera_setbit(0x1e, 6, 1);
				camera_setbit(0x1e, 7, 1);
				mini_delay(500);
				camera_setbit(0x1d, 7, 0);
				camera_setup(MIRROR_RESET_TAB);
				break;					
		default:
			akprintf(C1, M_DRVSYS, "parameter error!\n");
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
static T_VOID cam_po1200_set_effect(T_CAMERA_EFFECT effect)
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

		case CAMERA_EFFECT_AQUA:
			camera_setup(EFFECT_AQUA_TAB);
			break;				
		case CAMERA_EFFECT_COOL:
			camera_setup(EFFECT_COOL_TAB);
			break;
		case CAMERA_EFFECT_WARM:
			camera_setup(EFFECT_WARM_TAB);
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
static T_S32 cam_po1200_set_focus(T_U32 width, T_U32 height)
{
	unsigned int hrefstart = 0, hrefstop = 0, vrefstart = 0 , vrefstop = 0;

	unsigned char hH8bit = 0, hL8bit = 0 , vH8bit = 0 , vL8bit = 0;
	T_CAMERA_MODE Cammode = s_po1200_CurMode;
	unsigned char Camera_window_table[28] =
	{
		0x03, 0x00,
			
		0x08, 0x00,   // x1
		0x09, 0x00,
		0x0a, 0x00,   //y1
		0x0b, 0x00,
		0x0c, 0x00,  //x2
		0x0d, 0x00,
		0x0e, 0x00,  //y2
		0x0f, 0x00,

		0x9f, 0x00,  //vstart
		0xa0, 0x00,
		0xa1, 0x00,  //vstop
		0xa2, 0x00,
		
	//	0x20, 0xc4,
		END_FLAG, END_FLAG
	};

	akprintf(C3, M_DRVSYS, "reset window size %d ,%d, %d\r\n" , Cammode , width , height);

	if(Cammode == CAMERA_MODE_UXGA)//UXGA mode 1600*1200
	{
		    	hrefstart = 263 + 1600 - width;
		    	hrefstop = 1863 - 1600 + width;
		    	vrefstart = 13 + 1200 - height;
		    	vrefstop = 1213 - 1200 + height;	
	}
	else
	{
	    if(Cammode == CAMERA_MODE_SXGA )//SXGA_MODE 1280*1024
	    {
		    	hrefstart = 263 + 1280 - width;
		    	hrefstop = 1863 - 1280 + width;
		    	vrefstart = 13 + 1024 - height;
		    	vrefstop = 1213 - 1024 + height;	
	    }
	    else
	    {
	       if((Cammode == CAMERA_MODE_SVGA)||(Cammode == CAMERA_MODE_PREV) )//SVGA_MODE 800*600
	        {
	        	/*
        	    akprintf(C1, M_DRVSYS, "enter forcus mode! level: %d \r\n",focusLevel);
		    	hrefstart = 263 + focusLevel * 80;
		    	hrefstop = 1863 - focusLevel * 80;
		    	vrefstart = 13 + focusLevel * 60;
		    	vrefstop = 1213 - focusLevel * 60;	
		    	*/
		    	hrefstart = 263 + 800 - width;
		    	hrefstop = 1863 - 800 + width;
		    	vrefstart = 13 + 600 - height;
		    	vrefstop = 1213 - 600 + height;			    	
	        }
	        else
	        {
	            if(Cammode == CAMERA_MODE_VGA )//VGA_MODE 640*480
	            {
		    		hrefstart = 263 + 640 - width;
		    		hrefstop = 1863 - 640 + width;
		    		vrefstart = 13 + 480 - height;
		    		vrefstop = 1213 - 480 + height;	
	            }
			else
			{
	           		 if(Cammode == CAMERA_MODE_QVGA )//VGA_MODE 640*480
	           		 {
		    			hrefstart = 263 + 320 - width;
		    			hrefstop = 1863 - 320 + width;
		    			vrefstart = 13 + 240- height;
		    			vrefstop = 1213 - 240 + height;	
	          		  }	
				else
				{
					return 0;
				}
			}
	        }
	    }
	}

		hH8bit = (hrefstart & 0xff00) >> 8; //Horizontal Frame start high 8-bit
		hL8bit = hrefstart & 0xff; //Horizontal Frame start low 8-bit

		Camera_window_table[3] = hH8bit;
		Camera_window_table[5] = hL8bit;


		hH8bit = (hrefstop & 0xff00) >> 8;//Horizontal Frame end high 8-bit
		hL8bit = hrefstop & 0xff;//Horizontal Frame end low 8-bit

		Camera_window_table[11] = hH8bit;
		Camera_window_table[13] = hL8bit;


		vH8bit= (vrefstart & 0xff00) >> 8;//Vertical Frame start high 8-bit
		vL8bit= vrefstart & 0xff;//Vertical Frame start low 8-bit

		Camera_window_table[7] = vH8bit;
		Camera_window_table[9] = vL8bit;
		
		Camera_window_table[19] = vH8bit;
		Camera_window_table[21] = vL8bit;


		vH8bit= (vrefstop & 0xff00) >> 8;//Vertical Frame end high 8-bit
		vL8bit= vrefstop & 0xff;//Vertical Frame end low 8-bit

		Camera_window_table[15] = vH8bit;
		Camera_window_table[17] = vL8bit;

		Camera_window_table[23] = vH8bit;
		Camera_window_table[25] = vL8bit;
	if (camera_set_param(Camera_window_table))
	    return 1;
	else
		return -1;
}

static T_VOID cam_po1200_set_night_mode(T_NIGHT_MODE mode)
{
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
}

static T_BOOL cam_po1200_set_to_cap(T_U32 width, T_U32 height)
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
    else if ((width <= 400) && (height <= 300))
    {
        Cammode = CAMERA_MODE_QSVGA;
    }	
    else if ((width <= 640) && (height <= 480))
    {
        Cammode = CAMERA_MODE_VGA;
    }
    else if ((width <= 1280) && (height <= 1024))
    {
        Cammode = CAMERA_MODE_SXGA;
    }
    else if ((width <= 1600) && (height <= 1200))
    {
        Cammode = CAMERA_MODE_UXGA;
    }	
    else
    {
        akprintf(C1, M_DRVSYS, "po1200 unsupport %d & %d mode!\n", width, height);
        return AK_FALSE;
    }
	if(Cammode >= CAMERA_MODE_VGA)
	{
		cam_po1200_set_mode(CAMERA_MODE_SVGA);
		cam_po1200_set_focus(width, height);
	}
	else
	{
		cam_po1200_set_mode(CAMERA_MODE_UXGA);
		cam_po1200_set_focus(width, height);	
	}
	return AK_TRUE;
}

static T_BOOL cam_po1200_set_to_prev(T_U32 width, T_U32 height)
{
	cam_po1200_set_mode(CAMERA_MODE_SVGA);
	cam_po1200_set_focus(width, height);
	return AK_TRUE;
}

static T_BOOL cam_po1200_set_to_record(T_U32 width, T_U32 height)
{	
	cam_po1200_set_mode(CAMERA_MODE_CIF);
	cam_po1200_set_focus(width, height);
	return AK_TRUE;
}

static T_CAMERA_TYPE cam_po1200_get_type(T_VOID)
{
	return camera_po1200_type;
} 

static T_CAMERA_FUNCTION_HANDLER po1200_function_handler = 
{
	PO1200_CAMERA_MCLK,
	cam_po1200_open,
	cam_po1200_close,
	cam_po1200_read_id,
	cam_po1200_init,
	cam_po1200_set_mode,
	cam_po1200_set_exposure,
	cam_po1200_set_brightness,
	cam_po1200_set_contrast,
	cam_po1200_set_saturation,
	cam_po1200_set_sharpness,
	cam_po1200_set_AWB,
	cam_po1200_set_mirror,
	cam_po1200_set_effect,
	cam_po1200_set_focus,
	cam_po1200_set_night_mode,
	cam_po1200_set_to_cap,
	cam_po1200_set_to_prev,
	cam_po1200_set_to_record,
	cam_po1200_get_type
};

static int camera_po1200_reg(void)
{
	camera_reg_dev(CAMERA_PO1200_ID, &po1200_function_handler);
	return 0;
}

#ifdef __CC_ARM
#pragma arm section rwdata = "__initcall_", zidata = "__initcall_"
#endif
module_init(camera_po1200_reg)
#ifdef __CC_ARM
#pragma arm section
#endif

#endif

